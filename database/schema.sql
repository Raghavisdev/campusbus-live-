-- CampusBus Live database schema (SQLite)
-- Rebuild the database with: python database/init_db.py

PRAGMA foreign_keys = ON;

-- Drop child tables before the tables they reference
DROP TABLE IF EXISTS route_segments;
DROP TABLE IF EXISTS bus_status;
DROP TABLE IF EXISTS buses;
DROP TABLE IF EXISTS route_stops;
DROP TABLE IF EXISTS routes;
DROP TABLE IF EXISTS bus_stops;
DROP TABLE IF EXISTS users;

CREATE TABLE users (
    user_id INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    username TEXT NOT NULL UNIQUE,
    password_hash TEXT NOT NULL
);

CREATE TABLE bus_stops (
    stop_id TEXT PRIMARY KEY,
    stop_name TEXT NOT NULL,
    latitude REAL NOT NULL,
    longitude REAL NOT NULL
);

CREATE TABLE routes (
    route_id TEXT PRIMARY KEY,
    route_name TEXT NOT NULL
);

CREATE TABLE route_stops (
    route_id TEXT NOT NULL,
    stop_id TEXT NOT NULL,
    stop_order INTEGER NOT NULL,

    PRIMARY KEY (route_id, stop_id),

    FOREIGN KEY (route_id) REFERENCES routes(route_id),
    FOREIGN KEY (stop_id) REFERENCES bus_stops(stop_id)
);

CREATE TABLE buses (
    bus_id TEXT PRIMARY KEY,
    route_id TEXT NOT NULL,
    capacity INTEGER NOT NULL,
    FOREIGN KEY (route_id) REFERENCES routes(route_id)
);

CREATE TABLE bus_status (
    bus_id TEXT PRIMARY KEY,
    occupancy INTEGER NOT NULL,
    latitude REAL NOT NULL,
    longitude REAL NOT NULL,
    last_updated TEXT NOT NULL,
    FOREIGN KEY (bus_id) REFERENCES buses(bus_id)
);

CREATE TABLE route_segments (
    route_id TEXT NOT NULL,
    from_stop_id TEXT NOT NULL,
    to_stop_id TEXT NOT NULL,
    distance_km REAL NOT NULL,
    travel_time_min REAL NOT NULL,

    PRIMARY KEY (route_id, from_stop_id, to_stop_id),

    FOREIGN KEY (route_id) REFERENCES routes(route_id),
    FOREIGN KEY (from_stop_id) REFERENCES bus_stops(stop_id),
    FOREIGN KEY (to_stop_id) REFERENCES bus_stops(stop_id)
);

