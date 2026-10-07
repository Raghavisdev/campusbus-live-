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
