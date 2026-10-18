#ifndef LOCATION_H
#define LOCATION_H

#include <cmath>

class Location
{
protected:

    double latitude;
    double longitude;

public:

    Location(double latitude = 0.0, double longitude = 0.0)
    {
        this->latitude = latitude;
        this->longitude = longitude;
    }

    double getLatitude() const
    {
        return latitude;
    }

    double getLongitude() const
    {
        return longitude;
    }

    double distanceTo(const Location& other) const
    {
        const double EARTH_RADIUS_KM = 6371.0;
        const double DEG_TO_RAD = 3.14159265358979 / 180.0;

        double lat1 = latitude * DEG_TO_RAD;
        double lat2 = other.latitude * DEG_TO_RAD;
        double latDiff = (other.latitude - latitude) * DEG_TO_RAD;
        double longDiff = (other.longitude - longitude) * DEG_TO_RAD;

        double a = sin(latDiff / 2) * sin(latDiff / 2) +
                   cos(lat1) * cos(lat2) *
                   sin(longDiff / 2) * sin(longDiff / 2);

        return EARTH_RADIUS_KM * 2 * atan2(sqrt(a), sqrt(1 - a));
    }
};

#endif