"""Fill data/segments.csv with road distance and drive time between neighbouring stops.

Run after pinning coordinates in data/coordinates_to_fill.csv:
    python database/compute_segments.py
    python database/init_db.py

Only routes whose stops all have coordinates are computed. Times come from the
public OSRM demo server (OpenStreetMap roads, car profile), so they are estimates;
replace them with times measured from recorded bus trips when available
(set source to "measured" so they are not overwritten).
"""
import csv
import json
import time
import urllib.request
from pathlib import Path

DATA_DIR = Path(__file__).resolve().parent / "data"
OSRM_URL = "https://router.project-osrm.org/route/v1/driving/{},{};{},{}?overview=false"


def read_csv(name):
    path = DATA_DIR / name
    if not path.exists():
        return []
    with open(path, encoding="utf-8", newline="") as f:
        return list(csv.DictReader(f))


def coordinates():
    coords = {}
    for name in ("stops.csv", "coordinates_to_fill.csv"):
        for row in read_csv(name):
            if row.get("latitude") and row.get("longitude"):
                coords[row["stop_id"]] = (float(row["latitude"]), float(row["longitude"]))
    return coords


def road_segment(start, end):
    url = OSRM_URL.format(start[1], start[0], end[1], end[0])  # OSRM expects lon,lat
    request = urllib.request.Request(url, headers={"User-Agent": "CampusBusLive-student-project"})
    with urllib.request.urlopen(request, timeout=20) as response:
        route = json.load(response)["routes"][0]
    return round(route["distance"] / 1000, 3), round(route["duration"] / 60, 1)


def main():
    coords = coordinates()

    routes = {}
    for row in read_csv("routes.csv"):
        routes.setdefault(row["route_id"], []).append((int(row["stop_order"]), row["stop_id"]))

    # Keep measured times; recompute only estimates
    kept = [row for row in read_csv("segments.csv") if row.get("source") == "measured"]
    done = {(row["route_id"], row["from_stop_id"], row["to_stop_id"]) for row in kept}
    rows = list(kept)

    for route_id, stops in sorted(routes.items()):
        stop_ids = [stop_id for _, stop_id in sorted(stops)]

        if not all(stop_id in coords for stop_id in stop_ids):
            continue

        for start, end in zip(stop_ids, stop_ids[1:]):
            if (route_id, start, end) in done:
                continue
            distance_km, minutes = road_segment(coords[start], coords[end])
            rows.append({
                "route_id": route_id, "from_stop_id": start, "to_stop_id": end,
                "distance_km": distance_km, "travel_time_min": minutes, "source": "osrm",
            })
            time.sleep(1)  # be polite to the free server

        print(f"{route_id}: computed")

    with open(DATA_DIR / "segments.csv", "w", encoding="utf-8", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=["route_id", "from_stop_id", "to_stop_id", "distance_km", "travel_time_min", "source"])
        writer.writeheader()
        writer.writerows(rows)

    print(f"{len(rows)} segments written to data/segments.csv")


if __name__ == "__main__":
    main()
