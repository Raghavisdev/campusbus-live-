"""Build database/campusbus.db from schema.sql, the CSV files in data/ and seed.sql.

Run from the repo root:
    python database/init_db.py

Re-running it resets the database. Live GPS history is lost.

data/stops.csv               every stop (coordinates may be blank)
data/routes.csv              stops of each route in order, as on the route sheet (towards GEU)
data/coordinates_to_fill.csv pinned coordinates; filled rows override stops.csv
data/segments.csv            optional: distance and time between neighbouring stops
"""
import csv
import sqlite3
import sys
from pathlib import Path

DB_DIR = Path(__file__).resolve().parent
DATA_DIR = DB_DIR / "data"

BUS_CAPACITY = 40  # placeholder until real capacities are known


def read_csv(name):
    path = DATA_DIR / name
    if not path.exists():
        return []
    with open(path, encoding="utf-8", newline="") as f:
        return list(csv.DictReader(f))


def to_float(value):
    value = (value or "").strip()
    return float(value) if value else None


def import_stops(conn):
    pinned = {
        row["stop_id"]: (to_float(row["latitude"]), to_float(row["longitude"]))
        for row in read_csv("coordinates_to_fill.csv")
        if to_float(row["latitude"]) is not None and to_float(row["longitude"]) is not None
    }

    for row in read_csv("stops.csv"):
        latitude, longitude = to_float(row["latitude"]), to_float(row["longitude"])
        verified = int(row.get("verified") or 0)

        if row["stop_id"] in pinned:
            latitude, longitude = pinned[row["stop_id"]]
            verified = 1

        conn.execute(
            "INSERT INTO bus_stops (stop_id, stop_name, latitude, longitude, verified) VALUES (?, ?, ?, ?, ?)",
            (row["stop_id"], row["stop_name"], latitude, longitude, verified),
        )

    return len(pinned)


def import_routes(conn):
    routes = {}
    for row in read_csv("routes.csv"):
        routes.setdefault(row["route_id"], []).append((int(row["stop_order"]), row["stop_id"], row["stop_name"]))

    for route_id, stops in routes.items():
        stops.sort()
        first, last = stops[0][2], stops[-1][2]

        # Morning trip, exactly as on the route sheet
        conn.execute(
            "INSERT INTO routes VALUES (?, ?, 'to_geu', ?)",
            (route_id, f"{first} to {last}", route_id),
        )
        # Evening trip: same stops, reversed
        conn.execute(
            "INSERT INTO routes VALUES (?, ?, 'from_geu', ?)",
            (route_id + "R", f"{last} to {first}", route_id),
        )

        for order, (_, stop_id, _) in enumerate(stops, start=1):
            conn.execute("INSERT INTO route_stops VALUES (?, ?, ?)", (route_id, order, stop_id))

        for order, (_, stop_id, _) in enumerate(reversed(stops), start=1):
            conn.execute("INSERT INTO route_stops VALUES (?, ?, ?)", (route_id + "R", order, stop_id))

        bus_id = "B" + route_id[1:]
        conn.execute("INSERT INTO buses (bus_id, route_id, capacity) VALUES (?, ?, ?)", (bus_id, route_id, BUS_CAPACITY))

    return len(routes)


def import_segments(conn):
    count = 0
    for row in read_csv("segments.csv"):
        values = (float(row["distance_km"]), float(row["travel_time_min"]), row.get("source") or "estimate")

        conn.execute(
            "INSERT INTO route_segments VALUES (?, ?, ?, ?, ?, ?)",
            (row["route_id"], row["from_stop_id"], row["to_stop_id"]) + values,
        )
        # Same road in the opposite direction for the evening route
        conn.execute(
            "INSERT INTO route_segments VALUES (?, ?, ?, ?, ?, ?)",
            (row["route_id"] + "R", row["to_stop_id"], row["from_stop_id"]) + values,
        )
        count += 1

    return count


def main():
    db_path = Path(sys.argv[1]) if len(sys.argv) > 1 else DB_DIR / "campusbus.db"
    schema = (DB_DIR / "schema.sql").read_text(encoding="utf-8")
    seed = (DB_DIR / "seed.sql").read_text(encoding="utf-8")

    conn = sqlite3.connect(db_path)
    try:
        conn.executescript(schema)
        pinned = import_stops(conn)
        routes = import_routes(conn)
        segments = import_segments(conn)
        conn.executescript(seed)
        conn.commit()
    finally:
        conn.close()

    print(f"Database created at {db_path}")
    print(f"  {routes} routes (+ reverse), {pinned} pinned stops, {segments} segments")


if __name__ == "__main__":
    main()
