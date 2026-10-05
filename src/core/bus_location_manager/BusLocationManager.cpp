#include "BusLocationManager.h"

void BusLocationManager::addBus(Bus bus)
{
    buses.emplace(bus.getBusID(), bus);
}
void BusLocationManager::updateBusLocation(
    string busID,
    double latitude,
    double longitude,
     string lastUpdated
)
{
    buses.at(busID).updateLocation(latitude, longitude);
    buses.at(busID).setLastUpdated(lastUpdated);

}

Bus BusLocationManager::getBus(string busID) const
{
     return  buses.at(busID);
}

vector<Bus> BusLocationManager::getAllBuses() const
{
    vector<Bus>  allBuses;
    for (auto bus : buses)
    {
          allBuses.push_back(bus.second);
    }
    return allBuses;
}