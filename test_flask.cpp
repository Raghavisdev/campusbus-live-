#include <iostream>
#include "src/core/models/Bus.h"
#include "src/core/flask_client/FlaskClient.h"

using namespace std;

int main()
{
    Bus bus("B1", "R1", 50);

    int occupancy = FlaskClient::getOccupancy(
        bus.getBusID()
    );

    if (occupancy != -1)
    {
        bus.updateOccupancy(occupancy);
    }

    cout << "Bus ID: " << bus.getBusID() << endl;
    cout << "Occupancy: " << bus.getOccupancy() << endl;

    return 0;
}