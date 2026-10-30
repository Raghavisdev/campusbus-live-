-- CampusBus Live database schema (SQLite)
-- Rebuild the database with: python database/init_db.py
-- Route and stop data come from database/data/*.csv

PRAGMA foreign_keys = ON;

-- Drop child tables before the tables they reference
DROP TABLE IF EXISTS occupancy_log;
DROP TABLE IF EXISTS bus_locations;
DROP TABLE IF EXISTS devices;
DROP TABLE IF EXISTS route_segments;
DROP TABLE IF EXISTS bus_status;
DROP TABLE IF EXISTS buses;
DROP TABLE IF EXISTS route_stops;
DROP TABLE IF EXISTS routes;
DROP TABLE IF EXISTS bus_stops;
DROP TABLE IF EXISTS users;
DROP TABLE IF EXISTS students;

-- Students log in with their college ID (prototype: no password yet)
CREATE TABLE students (
    college_id TEXT PRIMARY KEY,
    name TEXT NOT NULL
);

-- Coordinates stay NULL until a stop has been pinned on a map
CREATE TABLE bus_stops (
    stop_id TEXT PRIMARY KEY,
    stop_name TEXT NOT NULL UNIQUE,
    latitude REAL,
    longitude REAL,
    verified INTEGER NOT NULL DEFAULT 0
);

-- Every route exists twice: to_geu (morning, from the sheet) and
-- from_geu (evening, same stops reversed, id ends in "R")
CREATE TABLE routes (
    route_id TEXT PRIMARY KEY,
    route_name TEXT NOT NULL,
    direction TEXT NOT NULL CHECK (direction IN ('to_geu', 'from_geu')),
    base_route_id TEXT NOT NULL
);

-- Keyed by position, so a route may pass the same stop twice (R15, R17)
CREATE TABLE route_stops (
    route_id TEXT NOT NULL,
    stop_order INTEGER NOT NULL,
    stop_id TEXT NOT NULL,

    PRIMARY KEY (route_id, stop_order),

    FOREIGN KEY (route_id) REFERENCES routes(route_id),
    FOREIGN KEY (stop_id) REFERENCES bus_stops(stop_id)
);

-- Only filled once both stops of a segment have coordinates
CREATE TABLE route_segments (
    route_id TEXT NOT NULL,
    from_stop_id TEXT NOT NULL,
    to_stop_id TEXT NOT NULL,
    distance_km REAL NOT NULL,
    travel_time_min REAL NOT NULL,
    source TEXT NOT NULL DEFAULT 'estimate',

    PRIMARY KEY (route_id, from_stop_id, to_stop_id),

    FOREIGN KEY (route_id) REFERENCES routes(route_id),
    FOREIGN KEY (from_stop_id) REFERENCES bus_stops(stop_id),
    FOREIGN KEY (to_stop_id) REFERENCES bus_stops(stop_id)
);

-- One bus per route; route_id is its morning (to_geu) route
CREATE TABLE buses (
    bus_id TEXT PRIMARY KEY,
    route_id TEXT NOT NULL,
    capacity INTEGER NOT NULL,
    active INTEGER NOT NULL DEFAULT 0,
    FOREIGN KEY (route_id) REFERENCES routes(route_id)
);

-- Latest known state of each running bus (one row per bus)
CREATE TABLE bus_status (
    bus_id TEXT PRIMARY KEY,
    current_route_id TEXT,
    occupancy INTEGER NOT NULL,
    latitude REAL NOT NULL,
    longitude REAL NOT NULL,
    speed_kmh REAL,
    last_updated TEXT NOT NULL,
    FOREIGN KEY (bus_id) REFERENCES buses(bus_id),
    FOREIGN KEY (current_route_id) REFERENCES routes(route_id)
);

-- Which tracker phone belongs to which bus
CREATE TABLE devices (
    device_id TEXT PRIMARY KEY,
    bus_id TEXT NOT NULL,
    FOREIGN KEY (bus_id) REFERENCES buses(bus_id)
);

-- Every GPS point received (replay, ETA learning, evaluation)
CREATE TABLE bus_locations (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    bus_id TEXT NOT NULL,
    latitude REAL NOT NULL,
    longitude REAL NOT NULL,
    speed_kmh REAL,
    recorded_at TEXT NOT NULL,
    FOREIGN KEY (bus_id) REFERENCES buses(bus_id)
);

CREATE INDEX idx_bus_locations_bus_time ON bus_locations (bus_id, recorded_at);

-- Camera passenger counts over time
CREATE TABLE occupancy_log (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    bus_id TEXT NOT NULL,
    occupancy INTEGER NOT NULL,
    recorded_at TEXT NOT NULL,
    FOREIGN KEY (bus_id) REFERENCES buses(bus_id)
);
