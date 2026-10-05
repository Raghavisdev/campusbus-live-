#include "Bus.h"

Bus::Bus(string busID, string routeID, int capacity)
{
    this->busID = busID;
    this->routeID = routeID;
    this->capacity = capacity;

    latitude = 0.0;
    longitude = 0.0;
    occupancy = 0;
    dispatchTime = "";
    lastUpdated = "";
}

void Bus::updateLocation(double latitude, double longitude)
{
    this->latitude = latitude;
    this->longitude = longitude;
}

void Bus::updateOccupancy(int occupancy)
{
    this->occupancy = occupancy;
}

void Bus::setDispatchTime(string dispatchTime)
{
    this->dispatchTime = dispatchTime;
}

void Bus::setLastUpdated(string lastUpdated)
{
    this->lastUpdated = lastUpdated;
}

string Bus::getDispatchTime() const
{
    return dispatchTime;
}

string Bus::getLastUpdated() const
{
    return lastUpdated;
}

string Bus::getBusID() const
{
    return busID;
}

int Bus::getCapacity() const
{
    return capacity;
}

int Bus::getOccupancy() const
{
    return occupancy;
}

string Bus::getRouteID() const
{
    return routeID;
}

double Bus::getLatitude() const
{
    return latitude;
}

double Bus::getLongitude() const
{
    return longitude;
}