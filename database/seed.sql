-- CampusBus Live seed data that is not in database/data/*.csv
-- (stops, routes and buses are imported from the CSV files by init_db.py)

-- students (mock college IDs for testing)
INSERT INTO students (college_id, name) VALUES
    ('2512510001', 'Test Student One'),
    ('2512510002', 'Test Student Two'),
    ('2512510003', 'Test Student Three');

-- Buses running in the GPS test: one per test route
UPDATE buses SET active = 1 WHERE bus_id IN ('B03', 'B04', 'B30');

-- Tracker phones (device ID set in the phone app) -> bus
INSERT INTO devices (device_id, bus_id) VALUES
    ('bus-r03', 'B03'),
    ('bus-r04', 'B04'),
    ('bus-r30', 'B30');
