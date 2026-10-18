#include "BusLocationManager.h"

void BusLocationManager::addBus(Bus bus)
{
    if (hasBus(bus.getBusID()))
    {
        return;
    }

    buses.emplace(bus.getBusID(), bus);
    busesOnRoute[bus.getRouteID()].push_back(bus.getBusID());
}

void BusLocationManager::loadBuses(Database& database)
{
    vector<Bus> allBuses = database.getAllBuses();

    for (size_t i = 0; i < allBuses.size(); i++)
    {
        Bus bus = database.getBusStatus(
            allBuses[i].getBusID()
        );

        if (bus.getBusID() != "")
        {
            addBus(bus);
        }
        else
        {
            addBus(allBuses[i]);
        }
    }
}

bool BusLocationManager::hasBus(string busID) const
{
    return buses.find(busID) != buses.end();
}

bool BusLocationManager::updateBusLocation(
    string busID,
    double latitude,
    double longitude,
    string lastUpdated
)
{
    if (!hasBus(busID))
    {
        return false;
    }

    if (latitude < -90 || latitude > 90 ||
        longitude < -180 || longitude > 180)
    {
        return false;
    }

    buses.at(busID).updateLocation(latitude, longitude);
    buses.at(busID).setLastUpdated(lastUpdated);

    return true;
}

bool BusLocationManager::updateBusOccupancy(string busID)
{
    if (!hasBus(busID))
    {
        return false;
    }

    int occupancy = FlaskClient::getOccupancy(busID);

    if (occupancy < 0)
    {
        return false;
    }

    int capacity = buses.at(busID).getCapacity();

    if (occupancy > capacity)
    {
        occupancy = capacity;
    }

    buses.at(busID).updateOccupancy(occupancy);

    return true;
}

Bus BusLocationManager::getBus(string busID) const
{
    if (!hasBus(busID))
    {
        return Bus("", "", 0);
    }

    return buses.at(busID);
}

vector<Bus> BusLocationManager::getAllBuses() const
{
    vector<Bus> allBuses;

    for (auto bus : buses)
    {
        allBuses.push_back(bus.second);
    }

    return allBuses;
}

vector<string> BusLocationManager::getBusesOnRoute(string routeID) const
{
    auto it = busesOnRoute.find(routeID);

    if (it == busesOnRoute.end())
    {
        return {};
    }

    return it->second;
}