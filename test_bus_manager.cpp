#include <iostream>
#include "src/core/models/Bus.h"
#include "src/core/models/BusStop.h"
#include "src/core/bus_location_manager/BusLocationManager.h"

using namespace std;

int main()
{
    BusLocationManager manager;

    manager.addBus(Bus("B1", "R1", 50));
    manager.addBus(Bus("B2", "R1", 50));
    manager.addBus(Bus("B3", "R2", 40));

    vector<string> r1Buses = manager.getBusesOnRoute("R1");

    cout << "Buses on R1: ";
    for (size_t i = 0; i < r1Buses.size(); i++)
    {
        cout << r1Buses[i] << " ";
    }
    cout << endl;

    cout << "Buses on R9: " << manager.getBusesOnRoute("R9").size() << endl;

    cout << "Update unknown bus: "
         << manager.updateBusLocation("B9", 30.27, 77.99, "10:00") << endl;
    cout << "Update invalid coordinates: "
         << manager.updateBusLocation("B1", 200, 77.99, "10:00") << endl;
    cout << "Update valid location: "
         << manager.updateBusLocation("B1", 30.27, 77.99, "10:00") << endl;
    cout << "Unknown bus ID returned: \""
         << manager.getBus("B9").getBusID() << "\"" << endl;

    Bus bus = manager.getBus("B1");
    BusStop stop("S2", "ISBT", 30.29, 77.99);

    cout << "Distance from B1 to S2: "
         << bus.distanceTo(stop) << " km" << endl;

    return 0;
}