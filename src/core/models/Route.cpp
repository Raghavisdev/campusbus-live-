#include "Route.h"

Route::Route(string routeID)
{
    this->routeID = routeID;
}

void Route::addstop(BusStop stop)
{
    stops.push_back(stop);
}

void Route::addstop(BusStop stop, double travelTime)
{
    stops.push_back(stop);

    // Travel time is required only between stops.
    // The first stop does not have a previous stop.
    if (stops.size() > 1)
    {
        travelTimes.push_back(travelTime);
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