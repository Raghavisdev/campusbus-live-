#include "Bus.h"

Bus::Bus(string busID, string routeID, int capacity)
{
    this->busID = busID;
    this->routeID = routeID;
    this->capacity = capacity;

    occupancy = 0;
    dispatchTime = "";
    lastUpdated = "";

    nextStopOrder = -1;
    segmentProgress = 0.0;
    paceRatio = 1.0;
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

void Bus::setRoutePosition(int nextStopOrder, double segmentProgress)
{
    this->nextStopOrder = nextStopOrder;
    this->segmentProgress = segmentProgress;
}

int Bus::getNextStopOrder() const
{
    return nextStopOrder;
}

double Bus::getSegmentProgress() const
{
    return segmentProgress;
}

void Bus::setPaceRatio(double paceRatio)
{
    this->paceRatio = paceRatio;
}

double Bus::getPaceRatio() const
{
    return paceRatio;
}