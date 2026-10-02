#ifndef ROUTE_H
#define ROUTE_H

#include <string>
#include <vector>
#include "BusStop.h"

using namespace std;

class Route {

private:

    string routeID;
    string routeName;
    vector<BusStop> stops;

public:

    Route(string routeID, string routeName);

    void addStop(const BusStop& stop);
    void removeStop(int index);

    string getRouteID() const;
    string getRouteName() const;

    int getStopCount() const;
    const BusStop& getStop(int index) const;
};

#endif