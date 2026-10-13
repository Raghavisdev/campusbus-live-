-- CampusBus Live mock data (prototype phase: GPS and occupancy values are simulated)

-- users
INSERT INTO users (user_id, name, username, password_hash) VALUES
    (1, 'Raj', 'raj123', 'test_hash');

-- bus_stops
INSERT INTO bus_stops (stop_id, stop_name, latitude, longitude) VALUES
    ('S1', 'Graphic Era Main Gate', 30.2679993, 77.9960119),
    ('S2', 'ISBT', 30.2878207, 77.9980253),
    ('S3', 'Majra', 30.2945, 77.9995),
    ('S4', 'Niranjanpur', 30.302, 78.006),
    ('S5', 'Patel Nagar', 30.309, 78.015),
    ('S6', 'Saharanpur Chowk', 30.31717, 78.02817),
    ('S7', 'Doordarshan Kendra', 30.2747, 78.0007),
    ('S8', 'Ajabpur Chowk', 30.2798, 78.0028),
    ('S9', 'Mata Mandir Chowk', 30.2834, 78.0008),
    ('S10', 'Kargi Chowk', 30.2789, 77.9885),
    ('S11', 'Brahmanwala Chowk', 30.2848, 77.9874);

-- routes
INSERT INTO routes (route_id, route_name) VALUES
    ('R1', 'Graphic Era - Saharanpur Chowk'),
    ('R2', 'Graphic Era - Haridwar Bypass - ISBT');

-- route_stops
INSERT INTO route_stops (route_id, stop_id, stop_order) VALUES
    ('R1', 'S1', 1),
    ('R1', 'S2', 2),
    ('R1', 'S3', 3),
    ('R1', 'S4', 4),
    ('R1', 'S5', 5),
    ('R1', 'S6', 6),
    ('R2', 'S1', 1),
    ('R2', 'S7', 2),
    ('R2', 'S8', 3),
    ('R2', 'S9', 4),
    ('R2', 'S10', 5),
    ('R2', 'S11', 6),
    ('R2', 'S2', 7);

-- buses
INSERT INTO buses (bus_id, route_id, capacity) VALUES
    ('B1', 'R1', 50),
    ('B2', 'R1', 50),
    ('B3', 'R2', 40);

-- bus_status
INSERT INTO bus_status (bus_id, occupancy, latitude, longitude, last_updated) VALUES
    ('B1', 30, 30.2855, 77.998, '2026-10-08 18:10:00'),
    ('B2', 50, 30.276, 77.9968, '2026-10-08 18:10:00'),
    ('B3', 20, 30.2798, 78.0028, '2026-10-08 18:10:00');

-- route_segments
INSERT INTO route_segments (route_id, from_stop_id, to_stop_id, distance_km, travel_time_min) VALUES
    ('R1', 'S1', 'S2', 2.213, 5.0),
    ('R1', 'S2', 'S3', 0.756, 3.0),
    ('R1', 'S3', 'S4', 1.042, 4.0),
    ('R1', 'S4', 'S5', 1.163, 4.0),
    ('R1', 'S5', 'S6', 1.557, 5.0),
    ('R2', 'S1', 'S7', 0.871, 3.0),
    ('R2', 'S7', 'S8', 0.602, 2.0),
    ('R2', 'S8', 'S9', 0.444, 2.0),
    ('R2', 'S9', 'S10', 1.283, 4.0),
    ('R2', 'S10', 'S11', 0.664, 3.0),
    ('R2', 'S11', 'S2', 1.074, 4.0);

