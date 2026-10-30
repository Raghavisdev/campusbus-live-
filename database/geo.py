"""Small geometry helpers shared by the database scripts and the Flask server.

A route's road is stored as a polyline: points with the distance (km) from the
start of the route. A GPS point is projected onto that line to find how far
along the route the bus is.
"""
import math

KM_PER_DEG_LAT = 110.574


def _to_xy(lat, lon, lat0):
    """Flat x/y in km around latitude lat0 (accurate enough within a city)."""
    return lon * 111.320 * math.cos(math.radians(lat0)), lat * KM_PER_DEG_LAT


def haversine_km(lat1, lon1, lat2, lon2):
    to_rad = math.pi / 180
    dlat = (lat2 - lat1) * to_rad
    dlon = (lon2 - lon1) * to_rad
    a = math.sin(dlat / 2) ** 2 + math.cos(lat1 * to_rad) * math.cos(lat2 * to_rad) * math.sin(dlon / 2) ** 2
    return 6371.0 * 2 * math.atan2(math.sqrt(a), math.sqrt(1 - a))


def cumulative_km(points):
    """[(lat, lon), ...] -> km from the first point for each point."""
    km = [0.0]
    for (lat1, lon1), (lat2, lon2) in zip(points, points[1:]):
        km.append(km[-1] + haversine_km(lat1, lon1, lat2, lon2))
    return km


def project(points, km, lat, lon, start_index=0):
    """Closest position on the polyline to (lat, lon).

    Returns (km_along, distance_off_line_km, segment_index).
    Only segments from start_index onward are considered, so stops on a loop
    can be matched in travel order.
    """
    lat0 = points[0][0]
    px, py = _to_xy(lat, lon, lat0)
    best = None

    for i in range(start_index, len(points) - 1):
        ax, ay = _to_xy(points[i][0], points[i][1], lat0)
        bx, by = _to_xy(points[i + 1][0], points[i + 1][1], lat0)
        dx, dy = bx - ax, by - ay
        length_sq = dx * dx + dy * dy

        t = 0.0 if length_sq == 0 else max(0.0, min(1.0, ((px - ax) * dx + (py - ay) * dy) / length_sq))
        cx, cy = ax + t * dx, ay + t * dy
        off = math.hypot(px - cx, py - cy)
        along = km[i] + t * (km[i + 1] - km[i])

        if best is None or off < best[1]:
            best = (along, off, i)

    return best


def project_near(points, km, lat, lon, previous_km=None, tolerance_km=0.03):
    """Like project(), but when the line passes close to the point more than once
    (a loop), prefer the match nearest to the bus's previous position."""
    if previous_km is None:
        return project(points, km, lat, lon)

    lat0 = points[0][0]
    px, py = _to_xy(lat, lon, lat0)
    candidates = []

    for i in range(len(points) - 1):
        ax, ay = _to_xy(points[i][0], points[i][1], lat0)
        bx, by = _to_xy(points[i + 1][0], points[i + 1][1], lat0)
        dx, dy = bx - ax, by - ay
        length_sq = dx * dx + dy * dy
        t = 0.0 if length_sq == 0 else max(0.0, min(1.0, ((px - ax) * dx + (py - ay) * dy) / length_sq))
        off = math.hypot(px - (ax + t * dx), py - (ay + t * dy))
        candidates.append((km[i] + t * (km[i + 1] - km[i]), off, i))

    closest = min(c[1] for c in candidates)
    near = [c for c in candidates if c[1] <= closest + tolerance_km]
    return min(near, key=lambda c: abs(c[0] - previous_km))
