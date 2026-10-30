# Campus Bus Live

## Overview

Campus Bus Live is a smart campus transportation and bus monitoring system designed to help students find the most suitable bus for their journey.

The system combines a C++ backend, Data Structures and Algorithms, GPS-based bus tracking, computer vision-based occupancy detection, Flask communication, database management, and a frontend interface.

The main purpose of the system is to allow a student to select their current bus stop and destination and receive a suitable bus recommendation based on the bus route, available seats, current location, and estimated arrival time.

---

## Problem Statement

Students using campus transportation may know the available bus routes but may not know which bus is the most suitable at a particular moment.

The system aims to solve problems such as:

- Not knowing the current location of a bus.
- Not knowing whether a bus has available seats.
- Not knowing which bus will reach a particular stop first.
- Difficulty selecting the correct bus when multiple buses operate on the same or intersecting routes.
- Lack of real-time occupancy information.

Campus Bus Live brings these different pieces of information together to provide a more useful bus recommendation.

---

## Objective

The primary objective is to recommend the closest suitable bus to a student based on:

- Current bus stop
- Destination
- Bus route
- Current bus location
- Bus occupancy
- Available seats
- Estimated arrival time

The system is designed around predefined campus routes, where multiple buses can operate on the same route and different routes can intersect.

---

## How the System Works

A student provides their current stop and destination.

The backend checks the available buses and determines which buses can serve the requested journey.

The database layer can identify routes containing both the current stop and destination, verify the direction using stop order, find buses assigned to those routes, and filter buses that have no available seats.

It then checks whether those buses have available capacity and considers their current position and estimated arrival time.

The most suitable bus is then returned to the user.

The general process is:

1. User logs into the system.
2. User selects their current stop.
3. User selects their destination.
4. Backend identifies suitable routes.
5. Available buses on those routes are checked.
6. Bus occupancy is checked.
7. Current bus location is obtained through GPS.
8. ETA is calculated.
9. The most suitable bus is recommended.
10. The result is displayed through the frontend.

---

## System Architecture

The project is divided into several major components.

### C++ Backend

The C++ backend contains the core bus-management and recommendation logic.

It includes:

- Bus management
- Bus stop management
- Route management
- Graph representation
- GPS processing
- ETA calculation
- Bus location management
- Bus recommendation

### Computer Vision

The computer vision component estimates the number of people inside a bus.

It uses Python, OpenCV, and YOLO11n.

The detected occupancy is sent to the Flask server.

### Flask Communication Layer

Flask acts as the communication layer between the Python computer vision component and the C++ backend.

The Python system sends the latest occupancy to Flask, and the C++ backend can request that information when required.

### Database

SQLite is being used to store persistent backend information such as users, buses, bus stops, routes, and related data.

The SQLite database currently contains tables for users, buses, bus stops, routes, route-stop relationships, route segments,  and live bus status.

Foreign keys are used to maintain relationships between buses, routes, and stops.
Route stop order is stored so  that the backend can verify whether a destination occurs after the user's current stop on a particular route.

### C++ and SQLite Integration

The C++ backend has been connected to SQLite using the SQLite C API. A `Database` class is used to open and access `campusbus.db`.

The current database integration can retrieve an individual bus as well as all buses stored in the database. Database records are converted into C++ `Bus` objects, which will gradually replace the hardcoded bus data currently present in `main.cpp`.

The SQLite integration has been successfully tested with buses B1, B2, and B3.

### Frontend

The frontend will provide the user interface for login, selecting stops and destinations, and viewing bus recommendations and related information.

---

## Bus Model

The `Bus` class represents an individual bus in the system.

A bus contains information such as:

- Bus ID
- Route ID
- Capacity
- Occupancy
- Latitude
- Longitude
- Dispatch time
- Last updated time

The class uses encapsulation by keeping its internal data private and providing public functions to update and retrieve the required information.

---

## Bus Stops

A `BusStop` represents a physical location where passengers can board or leave a bus.

Each bus stop contains:

- Stop ID
- Stop name
- Latitude
- Longitude

The GPS coordinates allow the system to compare the current location of a bus with predefined campus stops.

---

## Route Management

Campus buses operate on predefined routes.

A route contains an ordered sequence of bus stops and the estimated travel time between consecutive stops.

For example, a route may contain:

```text
S1 → S2 → S3 → S4 → S5
```

Route information is also stored in SQLite using `routes`, `route_stops`, and `route_segments`.

The `route_stops` table stores the order of stops on each route, while `route_segments` stores the distance and estimated travel time between consecutive stops.

## How to Run

Requirements: Python 3.11 and MinGW `g++` on PATH (Windows).

```bash
pip install -r requirements.txt
python database/init_db.py
build.bat
python vision/app.py
```

Then open http://127.0.0.1:5000 and log in with a test college ID: `2512510001`, `2512510002` or `2512510003`.

Request flow: browser -> Flask (`/api/login`, `/api/stops`, `/api/recommend`) -> `output/recommend.exe <from> <to>` -> SQLite -> JSON back to the browser.

Optional camera occupancy: `python vision/person_detection.py` (posts counts to `/update-occupancy`).

Data: stops and the 30 routes from the route sheet are in `database/data/` (`stops.csv`, `routes.csv`). Every route is stored twice: towards GEU (`R30`) and back from GEU (`R30R`). Edit the CSV files, then rerun `python database/init_db.py`.

## Live GPS Test (phones as buses)

1. Pin each stop of the test routes in `database/data/coordinates_to_fill.csv` (Google Maps: right-click the stop and copy the coordinates).
2. `python database/compute_segments.py`: road distance and drive time between stops (OSRM estimates).
3. `python database/init_db.py`
4. `python vision/app.py --lan` (phones must be on the same Wi-Fi; allow Python through Windows Firewall).
5. On each phone install Traccar Client. Server URL: `http://<laptop-ip>:5000/api/gps`. Device identifier: `bus-r30` or `bus-r04` (see `devices` in `database/seed.sql`). Frequency: 5 seconds.
6. Check the phones are reporting: http://127.0.0.1:5000/api/buses

Prototype limits: segment times are road estimates until replaced with times measured from recorded trips; GPS updates are not yet checked for staleness.
