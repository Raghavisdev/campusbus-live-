#include <iostream>
#include <vector>

#include "src/core/database/Database.h"
#include "src/core/bus_location_manager/BusLocationManager.h"

using namespace std;

int main()
{
    Database database("database/campusbus.db");

    BusLocationManager manager;

    manager.loadBuses(database);

    vector<Bus> buses = manager.getAllBuses();

    for (size_t i = 0; i < buses.size(); i++)
    {
        cout << "Bus ID: "
             << buses[i].getBusID() << endl;

        cout << "Route: "
             << buses[i].getRouteID() << endl;

        cout << "Capacity: "
             << buses[i].getCapacity() << endl;

        cout << "Occupancy: "
             << buses[i].getOccupancy() << endl;

        cout << "Latitude: "
             << buses[i].getLatitude() << endl;

        cout << "Longitude: "
             << buses[i].getLongitude() << endl;

        cout << "Last Updated: "
             << buses[i].getLastUpdated() << endl;

        cout << endl;
    }

    return 0;
}