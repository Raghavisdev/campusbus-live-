#include "Route.h"

Route::Route(string routeID)
{
    this->routeID = routeID;
}

void Route::addstop(BusStop stop)
{
    stops.push_back(stop);
    stopIndex[stop.getStopID()] = stops.size() - 1;
  
    if (prefixTime.empty())
    {
        prefixTime.push_back(0.0);
    }
    else
    {
        prefixTime.push_back(prefixTime.back());
    }
}

void Route::addstop(BusStop stop, double travelTime)
{
    stops.push_back(stop);
    stopIndex[stop.getStopID()] = stops.size() - 1;

    // Travel time is required only between stops.
    // The first stop does not have a previous stop.
    if (stops.size() > 1)
    {
        travelTimes.push_back(travelTime);
        prefixTime.push_back(prefixTime.back() + travelTime);
    }
    else
    {
        prefixTime.push_back(0.0);
    }
}

vector<BusStop> Route::getRoute() const
{
    return stops;
}

vector<double> Route::getTravelTimes() const
{
    return travelTimes;
}

string Route::getRouteId() const
{
    return routeID;
}

int Route::getStopIndex(string stopID) const
{
    auto it = stopIndex.find(stopID);

    if (it == stopIndex.end())
    {
        return -1;
    }

    return it->second;
}

double Route::timeBetween(int fromIndex, int toIndex) const
{
    int lastIndex = prefixTime.size() - 1;

    if (fromIndex < 0 || toIndex > lastIndex || fromIndex > toIndex)
    {
        return -1.0;
    }

    return prefixTime[toIndex] - prefixTime[fromIndex];
}