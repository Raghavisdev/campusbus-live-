#include <string>
#include <vector>
#include "BusStop.h"

using namespace std;

class Route
{
    private:

        string routeID;
        vector<BusStop> stops;

    public :
        Route(string routeID);

        void addstop(BusStop stop);
        vector<BusStop> getRoute() const;
        string getRouteId() const;

};  