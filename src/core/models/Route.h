#ifndef ROUTE_H
#define ROUTE_H

#include <string>
#include <vector>
#include "BusStop.h"

using namespace std;

class Route
{
private:

    string routeID;
    vector<BusStop> stops;
    vector<double> travelTimes;

public:

    Route(string routeID);

    void addstop(BusStop stop);
    void addstop(BusStop stop, double travelTime);

    vector<BusStop> getRoute() const;
    vector<double> getTravelTimes() const;

    string getRouteId() const;
};

#endif