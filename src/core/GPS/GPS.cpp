#include "GPS.h"
#include <cmath>

string GPS::findNearestStop(
    const Route& route,
    double latitude,
    double longitude
)
{
    vector<BusStop> stops = route.getRoute();

    string nearestStop = "";
    double minDistance = 999999999.0;

    for (size_t i = 0; i < stops.size(); i++)
    {
        double latDiff = latitude - stops[i].getLatitude();
        double longDiff = longitude - stops[i].getLongitude();

        double distance = sqrt(
            latDiff * latDiff +
            longDiff * longDiff
        );

        if (distance < minDistance)
        {
            minDistance = distance;
            nearestStop = stops[i].getStopID();
        }
    }

    return nearestStop;
}