#include "BusStop.h"

BusStop::BusStop(string stopID, string stopName, double latitude, double longitude){
    this->latitude=latitude;
    this->longitude=longitude;
    this->stopID=stopID;
    this->stopName=stopName;
}

string BusStop::getStopID() const
{
    return stopID;
}

string BusStop::getStopName() const
{
    return stopName;
}

double BusStop::getLatitude() const
{
    return latitude;
}

double BusStop::getLongitude() const
{
    return longitude;
}