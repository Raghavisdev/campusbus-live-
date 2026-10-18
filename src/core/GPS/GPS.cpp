#include "GPS.h"

string GPS::findNearestStop(
    const Route& route,
    double latitude,
    double longitude
)
{
    vector<BusStop> stops = route.getRoute();

    string nearestStop = "";
    double minDistance = 999999999.0;

    Location busPosition(latitude, longitude);

    for (size_t i = 0; i < stops.size(); i++)
    {
        double distance = busPosition.distanceTo(stops[i]);

        if (distance < minDistance)
        {
            minDistance = distance;
            nearestStop = stops[i].getStopID();
        }
    }

    return nearestStop;
}