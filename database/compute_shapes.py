"""Fetch the road shape of each route and the position (km) of every stop on it.

Run after pinning coordinates (and before init_db.py):
    python database/compute_shapes.py

Writes:
    data/shapes.csv          route_id, seq, latitude, longitude, km
    data/stop_positions.csv  route_id, stop_order, km

Only routes whose stops all have coordinates are fetched (towards GEU; the
reverse route reuses the same line). Uses the public OSRM demo server.
"""
import csv
import json
import time
import urllib.request

from compute_segments import DATA_DIR, coordinates, read_csv
from geo import cumulative_km, project

OSRM_URL = "https://router.project-osrm.org/route/v1/driving/{}?overview=full&geometries=geojson"


def fetch_shape(stop_points):
    path = ";".join(f"{lon},{lat}" for lat, lon in stop_points)  # OSRM expects lon,lat
    request = urllib.request.Request(OSRM_URL.format(path), headers={"User-Agent": "CampusBusLive-student-project"})
    with urllib.request.urlopen(request, timeout=30) as response:
        data = json.load(response)
    line = [(lat, lon) for lon, lat in data["routes"][0]["geometry"]["coordinates"]]
    snapped = [(lat, lon) for lon, lat in (w["location"] for w in data["waypoints"])]
    return line, snapped


def main():
    coords = coordinates()

    routes = {}
    for row in read_csv("routes.csv"):
        routes.setdefault(row["route_id"], []).append((int(row["stop_order"]), row["stop_id"]))

    shape_rows, position_rows = [], []

    for route_id, stops in sorted(routes.items()):
        stops.sort()
        if not all(stop_id in coords for _, stop_id in stops):
            continue

        line, snapped = fetch_shape([coords[stop_id] for _, stop_id in stops])
        km = cumulative_km(line)

        for seq, ((lat, lon), k) in enumerate(zip(line, km)):
            shape_rows.append({"route_id": route_id, "seq": seq, "latitude": round(lat, 6),
                               "longitude": round(lon, 6), "km": round(k, 4)})

        # Stops in travel order: search forward from the previous stop (handles loops)
        segment = 0
        for (order, _), (lat, lon) in zip(stops, snapped):
            along, _, segment = project(line, km, lat, lon, start_index=segment)
            position_rows.append({"route_id": route_id, "stop_order": order, "km": round(along, 4)})

        print(f"{route_id}: {len(line)} points, {km[-1]:.2f} km")
        time.sleep(1)  # be polite to the free server

    with open(DATA_DIR / "shapes.csv", "w", encoding="utf-8", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=["route_id", "seq", "latitude", "longitude", "km"])
        writer.writeheader()
        writer.writerows(shape_rows)

    with open(DATA_DIR / "stop_positions.csv", "w", encoding="utf-8", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=["route_id", "stop_order", "km"])
        writer.writeheader()
        writer.writerows(position_rows)


if __name__ == "__main__":
    main()
