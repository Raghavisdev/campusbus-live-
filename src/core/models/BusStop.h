#ifndef BUSSTOP_H
#define BUSSTOP_H

#include <string>

using namespace std;

class BusStop
{
private:

    string stopID;
    string stopName;
    double latitude;
    double longitude;

public:

    BusStop(
        string stopID,
        string stopName,
        double latitude,
        double longitude
    );

    string getStopID() const;
    string getStopName() const;

    double getLatitude() const;
    double getLongitude() const;
};

#endif