import json
import re
import sqlite3
import subprocess
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


# ---------- Frontend ----------

@app.route("/")
def index():
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
    app.run(host="127.0.0.1", port=5000)
