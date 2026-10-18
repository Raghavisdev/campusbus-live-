#ifndef ROUTE_H
#define ROUTE_H

#include <string>
#include <vector>
#include <unordered_map>
#include "BusStop.h"

using namespace std;

class Route
{
private:

    string routeID;
    vector<BusStop> stops;
    vector<double> travelTimes;

    
    unordered_map<string, int> stopIndex;

    
    vector<double> prefixTime;

public:

    Route(string routeID);

    void addstop(BusStop stop);
    void addstop(BusStop stop, double travelTime);

    vector<BusStop> getRoute() const;
    vector<double> getTravelTimes() const;

    string getRouteId() const;

   
    int getStopIndex(string stopID) const;

   
    double timeBetween(int fromIndex, int toIndex) const;
};

#endif