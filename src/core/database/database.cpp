#include "Database.h"
#include <iostream>

using namespace std;

Database::Database(string databasePath)
{
    db = NULL;

    int result = sqlite3_open(databasePath.c_str(), &db);

    if (result != SQLITE_OK)
    {
        cout << "Database could not be opened." << endl;
        db = NULL;
    }
}

Database::~Database()
{
    if (db != NULL)
    {
        sqlite3_close(db);
    }
}

sqlite3* Database::getDatabase() const
{
    return db;
}

Bus Database::getBus(string busID)
{
    string query =
        "SELECT bus_id, route_id, capacity "
        "FROM buses "
        "WHERE bus_id = ?;";

    sqlite3_stmt* statement;

    int result = sqlite3_prepare_v2(
        db,
        query.c_str(),
        -1,
        &statement,
        NULL
    );

    if (result != SQLITE_OK)
    {
        return Bus("", "", 0);
    }

    sqlite3_bind_text(
        statement,
        1,
        busID.c_str(),
        -1,
        SQLITE_STATIC
    );

    result = sqlite3_step(statement);

    if (result != SQLITE_ROW)
    {
        sqlite3_finalize(statement);
        return Bus("", "", 0);
    }

    string id = (const char*)sqlite3_column_text(statement, 0);
    string routeID = (const char*)sqlite3_column_text(statement, 1);
    int capacity = sqlite3_column_int(statement, 2);

    sqlite3_finalize(statement);

    return Bus(id, routeID, capacity);
}

vector<Bus> Database::getAllBuses()
{
    vector<Bus> buses;

    string query =
        "SELECT bus_id, route_id, capacity "
        "FROM buses;";

    sqlite3_stmt* statement;

    int result = sqlite3_prepare_v2(
        db,
        query.c_str(),
        -1,
        &statement,
        NULL
    );

    if (result != SQLITE_OK)
    {
        return buses;
    }

    while (sqlite3_step(statement) == SQLITE_ROW)
    {
        string busID = (const char*)sqlite3_column_text(statement, 0);
        string routeID = (const char*)sqlite3_column_text(statement, 1);
        int capacity = sqlite3_column_int(statement, 2);

        Bus bus(busID, routeID, capacity);

        buses.push_back(bus);
    }

    sqlite3_finalize(statement);

    return buses;
}

Bus Database::getBusStatus(string busID)
{
    string query =
        "SELECT buses.bus_id, buses.route_id, buses.capacity, "
        "bus_status.occupancy, bus_status.latitude, "
        "bus_status.longitude, bus_status.last_updated "
        "FROM buses "
        "JOIN bus_status "
        "ON buses.bus_id = bus_status.bus_id "
        "WHERE buses.bus_id = ?;";
    sqlite3_stmt* statement;

    int result = sqlite3_prepare_v2(
        db,
        query.c_str(),
        -1,
        &statement,
        NULL
    );

    if (result != SQLITE_OK)
    {
        return Bus("", "", 0);
    }

    sqlite3_bind_text(
        statement,
        1,
        busID.c_str(),
        -1,
        SQLITE_STATIC
    );

    result=sqlite3_step(statement);

    if (result !=SQLITE_ROW)
    {
        sqlite3_finalize(statement);
        return Bus("", "", 0);
    }
    string id = (const char*)sqlite3_column_text(statement, 0);
    string routeID = (const char*)sqlite3_column_text(statement, 1);
    int capacity = sqlite3_column_int(statement, 2);
    int occupancy = sqlite3_column_int(statement, 3);

    double latitude = sqlite3_column_double(statement, 4);
    double longitude = sqlite3_column_double(statement, 5);

    string lastUpdated =(const char*)sqlite3_column_text(statement, 6);

    Bus bus(id, routeID, capacity);

    bus.updateOccupancy(occupancy);
    bus.updateLocation(latitude, longitude);
    bus.setLastUpdated(lastUpdated);

    sqlite3_finalize(statement);

    return bus;
}
vector<BusStop> Database::getAllStops()
{
    vector<BusStop> stops;

    string query =
        "SELECT stop_id, stop_name, latitude, longitude "
        "FROM bus_stops;";

    sqlite3_stmt* statement;

    int result = sqlite3_prepare_v2(
        db,
        query.c_str(),
        -1,
        &statement,
        NULL
    );

    if (result != SQLITE_OK)
    {
        return stops;
    }

    while (sqlite3_step(statement) == SQLITE_ROW)
    {
        string stopID = (const char*)sqlite3_column_text(statement, 0);
        string stopName = (const char*)sqlite3_column_text(statement, 1);
        double latitude = sqlite3_column_double(statement, 2);
        double longitude = sqlite3_column_double(statement, 3);

        stops.push_back(BusStop(stopID, stopName, latitude, longitude));
    }

    sqlite3_finalize(statement);

    return stops;
}

Route Database::getRoute(string routeID)
{
    string query =
        "SELECT bus_stops.stop_id, bus_stops.stop_name, "
        "bus_stops.latitude, bus_stops.longitude, "
        "route_segments.travel_time_min "
        "FROM route_stops "
        "JOIN bus_stops "
        "ON route_stops.stop_id = bus_stops.stop_id "
        "LEFT JOIN route_stops AS previous "
        "ON previous.route_id = route_stops.route_id "
        "AND previous.stop_order = route_stops.stop_order - 1 "
        "LEFT JOIN route_segments "
        "ON route_segments.route_id = route_stops.route_id "
        "AND route_segments.from_stop_id = previous.stop_id "
        "AND route_segments.to_stop_id = route_stops.stop_id "
        "WHERE route_stops.route_id = ? "
        "ORDER BY route_stops.stop_order;";

    sqlite3_stmt* statement;

    int result = sqlite3_prepare_v2(
        db,
        query.c_str(),
        -1,
        &statement,
        NULL
    );

    if (result != SQLITE_OK)
    {
        return Route("");
    }

    sqlite3_bind_text(
        statement,
        1,
        routeID.c_str(),
        -1,
        SQLITE_STATIC
    );

    Route route(routeID);
    bool firstStop = true;

    while (sqlite3_step(statement) == SQLITE_ROW)
    {
        string stopID = (const char*)sqlite3_column_text(statement, 0);
        string stopName = (const char*)sqlite3_column_text(statement, 1);
        double latitude = sqlite3_column_double(statement, 2);
        double longitude = sqlite3_column_double(statement, 3);

        BusStop stop(stopID, stopName, latitude, longitude);

        if (firstStop)
        {
            route.addstop(stop);
            firstStop = false;
            continue;
        }

        // A missing segment means the database is incomplete for this route.
        if (sqlite3_column_type(statement, 4) == SQLITE_NULL)
        {
            cout << "Missing travel time for route " << routeID
                 << " at stop " << stopID << endl;

            sqlite3_finalize(statement);
            return Route("");
        }

        double travelTime = sqlite3_column_double(statement, 4);

        route.addstop(stop, travelTime);
    }

    sqlite3_finalize(statement);

    return route;
}

vector<Route> Database::getAllRoutes()
{
    vector<Route> routes;
    vector<string> routeIDs;

    string query =
        "SELECT route_id "
        "FROM routes;";

    sqlite3_stmt* statement;

    int result = sqlite3_prepare_v2(
        db,
        query.c_str(),
        -1,
        &statement,
        NULL
    );

    if (result != SQLITE_OK)
    {
        return routes;
    }

    while (sqlite3_step(statement) == SQLITE_ROW)
    {
        routeIDs.push_back((const char*)sqlite3_column_text(statement, 0));
    }

    sqlite3_finalize(statement);

    for (size_t i = 0; i < routeIDs.size(); i++)
    {
        Route route = getRoute(routeIDs[i]);

        // Graph::addRoute needs at least two stops.
        if (route.getRoute().size() < 2)
        {
            cout << "Skipping route " << routeIDs[i]
                 << ": fewer than two stops." << endl;
            continue;
        }

        routes.push_back(route);
    }

    return routes;
}