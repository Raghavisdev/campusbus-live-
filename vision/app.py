import json
import re
import sqlite3
import subprocess
import sys
from datetime import datetime, timedelta, timezone
from pathlib import Path

from flask import Flask, request, jsonify, send_from_directory

REPO_DIR = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO_DIR / "database"))

from geo import project_near  # noqa: E402

MOVE_THRESHOLD_KM = 0.03   # smaller changes are GPS jitter, not movement
OFF_ROUTE_KM = 0.2         # further than this from the road line = off route

# Pace = actual time / planned time over a recent window (see README "How the ETA is calculated")
PACE_WINDOW_MIN = 5.0      # look back this far
PACE_MIN_HISTORY = 1.0     # need at least this much history before measuring
PACE_SMOOTHING = 0.3       # weight of the newest measurement
PACE_LIMITS = (0.6, 3.0)   # ignore implausible values
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

def locate_on_route(bus_id, latitude, longitude):
    """Project a GPS point onto the bus's road line and work out direction,
    next stop and progress. Returns the bus_status columns to update."""

    base_route = query_database("SELECT route_id FROM buses WHERE bus_id = ?", (bus_id,))[0][0]
    shape = query_database(
        "SELECT latitude, longitude, km FROM route_shapes WHERE route_id = ? ORDER BY seq", (base_route,)
    )

    # No road line yet: C++ falls back to the nearest stop
    if not shape:
        return {"current_route_id": None, "base_km": None, "next_stop_order": None,
                "segment_progress": None, "off_route_m": None, "planned_min": None}

    points = [(lat, lon) for lat, lon, _ in shape]
    kms = [km for _, _, km in shape]
    total_km = kms[-1]

    previous = query_database("SELECT base_km, current_route_id FROM bus_status WHERE bus_id = ?", (bus_id,))
    previous_km, previous_route = previous[0] if previous else (None, None)

    base_km, off_km, _ = project_near(points, kms, latitude, longitude, previous_km)

    # Direction from consecutive positions: km increasing = towards GEU
    current_route = previous_route or base_route
    if previous_km is not None:
        if base_km - previous_km > MOVE_THRESHOLD_KM:
            current_route = base_route
        elif previous_km - base_km > MOVE_THRESHOLD_KM:
            current_route = base_route + "R"

    route_km = base_km if current_route == base_route else total_km - base_km

    stops = query_database(
        "SELECT rs.stop_order, rs.km, s.radius_m, rs.stop_id FROM route_stops rs JOIN bus_stops s USING (stop_id) "
        "WHERE rs.route_id = ? ORDER BY rs.stop_order",
        (current_route,),
    )

    # 0 = not available for students (off route, or past the last stop)
    next_stop, progress = 0, 0.0

    if off_km <= OFF_ROUTE_KM:
        for index, (order, stop_km, radius_m, _) in enumerate(stops):
            if abs(route_km - stop_km) * 1000 <= radius_m:
                next_stop, progress = order, 1.0          # at this stop now
                break
            if stop_km > route_km:
                next_stop = order
                if index > 0:
                    previous_stop_km = stops[index - 1][1]
                    progress = (route_km - previous_stop_km) / (stop_km - previous_stop_km)
                break

    return {"current_route_id": current_route, "base_km": round(base_km, 4), "next_stop_order": next_stop,
            "segment_progress": round(progress, 3), "off_route_m": round(off_km * 1000),
            "planned_min": planned_minutes(current_route, stops, route_km)}


def planned_minutes(route_id, stops, route_km):
    """P(s): planned minutes from the route start to position route_km,
    interpolated linearly inside a segment. None if segment times are missing."""

    times = dict(
        ((from_stop, to_stop), minutes)
        for from_stop, to_stop, minutes in query_database(
            "SELECT from_stop_id, to_stop_id, travel_time_min FROM route_segments WHERE route_id = ?", (route_id,)
        )
    )

    elapsed = 0.0
    for (_, km_a, _, stop_a), (_, km_b, _, stop_b) in zip(stops, stops[1:]):
        minutes = times.get((stop_a, stop_b))
        if minutes is None or km_a is None or km_b is None:
            return None
        if route_km <= km_b:
            fraction = 0.0 if km_b == km_a else max(0.0, (route_km - km_a) / (km_b - km_a))
            return round(elapsed + fraction * minutes, 3)
        elapsed += minutes

    return round(elapsed, 3)


def parse_time(text):
    return datetime.strptime(text, "%Y-%m-%d %H:%M:%S").replace(tzinfo=timezone.utc)


def measure_pace(bus_id, position, recorded_at):
    """Pace ratio r = actual minutes / planned minutes of road covered,
    over the last PACE_WINDOW_MIN minutes of the same trip, smoothed."""

    previous = query_database("SELECT pace_ratio, current_route_id FROM bus_status WHERE bus_id = ?", (bus_id,))
    previous_ratio, previous_route = previous[0] if previous else (None, None)

    # New trip direction: start again from "on plan"
    if previous_ratio is None or previous_route != position["current_route_id"]:
        previous_ratio = 1.0

    now = parse_time(recorded_at)
    oldest = query_database(
        "SELECT recorded_at, planned_min FROM bus_locations "
        "WHERE bus_id = ? AND route_id = ? AND planned_min IS NOT NULL AND recorded_at >= ? "
        "ORDER BY recorded_at LIMIT 1",
        (bus_id, position["current_route_id"],
         (now - timedelta(minutes=PACE_WINDOW_MIN)).strftime("%Y-%m-%d %H:%M:%S")),
    )

    if not oldest or position["planned_min"] is None:
        return previous_ratio

    elapsed_min = (now - parse_time(oldest[0][0])).total_seconds() / 60
    if elapsed_min < PACE_MIN_HISTORY:
        return previous_ratio

    planned_covered = max(position["planned_min"] - oldest[0][1], 0.05)
    ratio = min(max(elapsed_min / planned_covered, PACE_LIMITS[0]), PACE_LIMITS[1])

    return round(PACE_SMOOTHING * ratio + (1 - PACE_SMOOTHING) * previous_ratio, 3)

@app.route("/api/gps", methods=["GET", "POST"])
def gps():
    """Accepts the Traccar Client (OsmAnd) format:
    ?id=<device>&lat=<deg>&lon=<deg>&timestamp=<unix seconds>&speed=<knots>
    Parameters may be in the URL, a form body or a JSON body.
    """

    data = dict(request.values)
    body = request.get_json(silent=True) or {}

    if isinstance(body.get("location"), dict):
        # Newer Traccar Client versions post JSON: speed in m/s, ISO timestamp
        location = body["location"]
        coords = location.get("coords") or {}
        data["id"] = body.get("device_id", data.get("id"))
        data["lat"], data["lon"] = coords.get("latitude"), coords.get("longitude")
        if coords.get("speed") is not None and coords["speed"] >= 0:
            data["speed"] = coords["speed"] * 1.943844  # m/s -> knots, converted to km/h below
        if location.get("timestamp"):
            try:
                data["timestamp"] = datetime.fromisoformat(location["timestamp"].replace("Z", "+00:00")).timestamp()
            except ValueError:
                pass
    else:
        data.update(body)

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

    position = locate_on_route(bus_id, latitude, longitude)
    position["pace_ratio"] = measure_pace(bus_id, position, recorded_at)

    # Time standing at a stop is not counted in the pace (only moving between stops)
    at_stop = position["segment_progress"] == 1.0
    pace_point = None if at_stop else position["planned_min"]

    write_database([
        (
            "INSERT INTO bus_locations (bus_id, latitude, longitude, speed_kmh, recorded_at, route_id, planned_min) "
            "VALUES (?, ?, ?, ?, ?, ?, ?)",
            (bus_id, latitude, longitude, speed_kmh, recorded_at, position["current_route_id"], pace_point),
        ),
        (
            "INSERT INTO bus_status (bus_id, occupancy, latitude, longitude, speed_kmh, last_updated, "
            "current_route_id, base_km, next_stop_order, segment_progress, off_route_m, planned_min, pace_ratio) "
            "VALUES (?, 0, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?) "
            "ON CONFLICT(bus_id) DO UPDATE SET latitude = excluded.latitude, longitude = excluded.longitude, "
            "speed_kmh = excluded.speed_kmh, last_updated = excluded.last_updated, "
            "current_route_id = excluded.current_route_id, base_km = excluded.base_km, "
            "next_stop_order = excluded.next_stop_order, segment_progress = excluded.segment_progress, "
            "off_route_m = excluded.off_route_m, planned_min = excluded.planned_min, "
            "pace_ratio = excluded.pace_ratio",
            (bus_id, latitude, longitude, speed_kmh, recorded_at, position["current_route_id"], position["base_km"],
             position["next_stop_order"], position["segment_progress"], position["off_route_m"],
             position["planned_min"], position["pace_ratio"]),
        ),
    ])

    return jsonify({"ok": True, "bus": bus_id, "recorded_at": recorded_at, **position})


@app.route("/api/buses", methods=["GET"])
def buses():
    """Live view of active buses (handy for checking that phones are reporting)."""

    rows = query_database(
        "SELECT b.bus_id, COALESCE(s.current_route_id, b.route_id), s.latitude, s.longitude, "
        "s.speed_kmh, s.occupancy, b.capacity, s.last_updated, s.next_stop_order, s.segment_progress, s.off_route_m "
        "FROM buses b LEFT JOIN bus_status s ON s.bus_id = b.bus_id WHERE b.active = 1 ORDER BY b.bus_id"
    )

    keys = ["bus", "route", "lat", "lon", "speed_kmh", "occupancy", "capacity", "last_updated",
            "next_stop_order", "segment_progress", "off_route_m"]
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
