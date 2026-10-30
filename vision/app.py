import json
import re
import sqlite3
import subprocess
from datetime import datetime, timezone
from pathlib import Path

from flask import Flask, request, jsonify, send_from_directory

REPO_DIR = Path(__file__).resolve().parent.parent
FRONTEND_DIR = REPO_DIR / "frontend"
DATABASE_PATH = REPO_DIR / "database" / "campusbus.db"
RECOMMEND_EXE = REPO_DIR / "output" / "recommend.exe"

app = Flask(__name__, static_folder=str(FRONTEND_DIR), static_url_path="")

occupancy_data = {}


def query_database(sql, params=()):
    connection = sqlite3.connect(DATABASE_PATH)
    try:
        return connection.execute(sql, params).fetchall()
    finally:
        connection.close()


def write_database(statements):
    connection = sqlite3.connect(DATABASE_PATH)
    try:
        for sql, params in statements:
            connection.execute(sql, params)
        connection.commit()
    finally:
        connection.close()


def now_iso():
    return datetime.now(timezone.utc).strftime("%Y-%m-%d %H:%M:%S")


# ---------- Frontend ----------

@app.route("/", methods=["GET", "POST"])
def index():
    # Some tracker apps send GPS to the bare server address
    if "lat" in request.values or request.method == "POST":
        return gps()
    return send_from_directory(FRONTEND_DIR, "index.html")


# ---------- API used by the frontend ----------

@app.route("/api/login", methods=["POST"])
def login():

    data = request.get_json(silent=True) or {}
    college_id = str(data.get("college_id", "")).strip()

    if not re.fullmatch(r"\d{10}", college_id):
        return jsonify({"ok": False, "error": "College ID must be 10 digits"}), 400

    rows = query_database(
        "SELECT name FROM students WHERE college_id = ?",
        (college_id,)
    )

    if not rows:
        return jsonify({"ok": False, "error": "Unknown college ID"}), 401

    return jsonify({"ok": True, "name": rows[0][0]})


@app.route("/api/stops", methods=["GET"])
def stops():

    rows = query_database(
        "SELECT stop_id, stop_name FROM bus_stops ORDER BY CAST(SUBSTR(stop_id, 2) AS INTEGER)"
    )

    return jsonify([{"id": stop_id, "name": name} for stop_id, name in rows])


@app.route("/api/recommend", methods=["GET"])
def recommend():

    from_stop = request.args.get("from", "")
    to_stop = request.args.get("to", "")

    # Only known stop IDs are passed to the C++ program
    known_stops = {row[0] for row in query_database("SELECT stop_id FROM bus_stops")}

    if from_stop not in known_stops or to_stop not in known_stops:
        return jsonify({"ok": False, "error": "Unknown stop"}), 400

    if not RECOMMEND_EXE.exists():
        return jsonify({"ok": False, "error": "Backend not built. Run build.bat"}), 500

    try:
        result = subprocess.run(
            [str(RECOMMEND_EXE), from_stop, to_stop, str(DATABASE_PATH)],
            capture_output=True,
            text=True,
            timeout=5,
            cwd=REPO_DIR
        )
    except subprocess.TimeoutExpired:
        return jsonify({"ok": False, "error": "Backend timed out"}), 504

    # The JSON answer is the last line the C++ program prints
    lines = [line for line in result.stdout.splitlines() if line.strip()]

    try:
        answer = json.loads(lines[-1])
    except (IndexError, ValueError):
        return jsonify({"ok": False, "error": "Invalid backend response"}), 500

    return jsonify(answer)


# ---------- Live GPS from tracker phones ----------

@app.route("/api/gps", methods=["GET", "POST"])
def gps():
    """Accepts the Traccar Client (OsmAnd) format:
    ?id=<device>&lat=<deg>&lon=<deg>&timestamp=<unix seconds>&speed=<knots>
    Parameters may be in the URL, a form body or a JSON body.
    """

    data = dict(request.values)
    data.update(request.get_json(silent=True) or {})

    device_id = str(data.get("id", "")).strip()

    try:
        latitude = float(data.get("lat"))
        longitude = float(data.get("lon"))
    except (TypeError, ValueError):
        return jsonify({"ok": False, "error": "lat and lon are required"}), 400

    if not (-90 <= latitude <= 90 and -180 <= longitude <= 180):
        return jsonify({"ok": False, "error": "lat/lon out of range"}), 400

    rows = query_database("SELECT bus_id FROM devices WHERE device_id = ?", (device_id,))

    if not rows:
        return jsonify({"ok": False, "error": "Unknown device id"}), 404

    bus_id = rows[0][0]

    speed_kmh = None
    if data.get("speed") not in (None, ""):
        try:
            speed_kmh = round(float(data["speed"]) * 1.852, 1)  # knots -> km/h
        except ValueError:
            pass

    recorded_at = now_iso()
    if data.get("timestamp"):
        try:
            recorded_at = datetime.fromtimestamp(float(data["timestamp"]), timezone.utc).strftime("%Y-%m-%d %H:%M:%S")
        except (ValueError, OverflowError, OSError):
            pass

    write_database([
        (
            "INSERT INTO bus_locations (bus_id, latitude, longitude, speed_kmh, recorded_at) VALUES (?, ?, ?, ?, ?)",
            (bus_id, latitude, longitude, speed_kmh, recorded_at),
        ),
        (
            "INSERT INTO bus_status (bus_id, occupancy, latitude, longitude, speed_kmh, last_updated) "
            "VALUES (?, 0, ?, ?, ?, ?) "
            "ON CONFLICT(bus_id) DO UPDATE SET latitude = excluded.latitude, longitude = excluded.longitude, "
            "speed_kmh = excluded.speed_kmh, last_updated = excluded.last_updated",
            (bus_id, latitude, longitude, speed_kmh, recorded_at),
        ),
    ])

    return jsonify({"ok": True, "bus": bus_id, "recorded_at": recorded_at})


@app.route("/api/buses", methods=["GET"])
def buses():
    """Live view of active buses (handy for checking that phones are reporting)."""

    rows = query_database(
        "SELECT b.bus_id, COALESCE(s.current_route_id, b.route_id), s.latitude, s.longitude, "
        "s.speed_kmh, s.occupancy, b.capacity, s.last_updated "
        "FROM buses b LEFT JOIN bus_status s ON s.bus_id = b.bus_id WHERE b.active = 1 ORDER BY b.bus_id"
    )

    keys = ["bus", "route", "lat", "lon", "speed_kmh", "occupancy", "capacity", "last_updated"]
    return jsonify([dict(zip(keys, row)) for row in rows])


# ---------- Occupancy (camera -> Flask -> C++) ----------

@app.route("/update-occupancy", methods=["POST"])
def update_occupancy():

    data = request.get_json(silent=True) or {}

    bus_id = data.get("busID")
    occupancy = data.get("occupancy")

    if not isinstance(bus_id, str) or not isinstance(occupancy, int) or occupancy < 0:
        return jsonify({
            "status": "error",
            "message": "Expected {\"busID\": string, \"occupancy\": non-negative integer}"
        }), 400

    occupancy_data[bus_id] = occupancy

    # Also keep it in SQLite so the C++ recommendation sees live occupancy
    if query_database("SELECT 1 FROM buses WHERE bus_id = ?", (bus_id,)):
        recorded_at = now_iso()
        write_database([
            ("INSERT INTO occupancy_log (bus_id, occupancy, recorded_at) VALUES (?, ?, ?)",
             (bus_id, occupancy, recorded_at)),
            ("UPDATE bus_status SET occupancy = ? WHERE bus_id = ?", (occupancy, bus_id)),
        ])

    print("Bus ID:", bus_id)
    print("Occupancy:", occupancy)

    return jsonify({
        "status": "success",
        "busID": bus_id,
        "occupancy": occupancy
    })


@app.route("/get-occupancy/<bus_id>", methods=["GET"])
def get_occupancy(bus_id):

    if bus_id not in occupancy_data:
        return jsonify({
            "status": "error",
            "message": "Bus not found"
        }), 404

    return jsonify({
        "busID": bus_id,
        "occupancy": occupancy_data[bus_id]
    })


@app.route("/health", methods=["GET"])
def health():

    return jsonify({
        "status": "running"
    })


if __name__ == "__main__":
    import sys

    # --lan lets phones on the same Wi-Fi reach the server (needed for GPS tracking)
    host = "0.0.0.0" if "--lan" in sys.argv else "127.0.0.1"
    app.run(host=host, port=5000)
