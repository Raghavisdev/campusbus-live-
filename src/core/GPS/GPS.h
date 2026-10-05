#include "../models/Route.h"

using namespace std;

class GPS
{
    public:

        static string findNearestStop(
            const Route& route,
            double latitude,
            double longitude
        );
};