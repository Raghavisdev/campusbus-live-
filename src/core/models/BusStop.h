#ifndef BUSSTOP_H
#define BUSSTOP_H

#include <string>
#include "Location.h"

using namespace std;

class BusStop : public Location
{
private:

    string stopID;
    string stopName;

public:

    BusStop(
        string stopID,
        string stopName,
        double latitude,
        double longitude
    );

    string getStopID() const;
    string getStopName() const;
};

#endif