#include "Route.h"

Route::Route(string routeID)
{
    this->routeID = routeID;
}

void Route::addstop(BusStop stop)
{
    stops.push_back(stop);
}

vector<BusStop> Route::getRoute() const
{
    return stops;
}

string Route::getRouteId() const
{
    return routeID;
}

