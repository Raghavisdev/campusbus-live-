#include "BusStop.h"

BusStop::BusStop(
    string stopID,
    string stopName,
    double latitude,
    double longitude
) : Location(latitude, longitude)
{
    this->stopID = stopID;
    this->stopName = stopName;
}

string BusStop::getStopID() const
{
    return stopID;
}

string BusStop::getStopName() const
{
    return stopName;
}