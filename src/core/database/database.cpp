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