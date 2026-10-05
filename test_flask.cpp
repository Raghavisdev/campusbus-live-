#include <iostream>
#include "src/core/models/Bus.h"
#include "src/core/bus_location_manager/BusLocationManager.h"

using namespace std;

int main()
{
    Bus bus("B1", "R1", 50);

    BusLocationManager manager;

    manager.addBus(bus);

    manager.updateBusOccupancy("B1");

    Bus updatedBus = manager.getBus("B1");

    cout << "Bus ID: " << updatedBus.getBusID() << endl;
    cout << "Occupancy: " << updatedBus.getOccupancy() << endl;

    return 0;
}