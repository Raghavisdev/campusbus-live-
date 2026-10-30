#ifndef BUS_RECOMMENDATION_H
#define BUS_RECOMMENDATION_H

#include "../models/Bus.h"
#include "../bus_location_manager/BusLocationManager.h"
#include "../datastructures/Graph.h"
#include "../models/Route.h"
#include "../ETA/ETAEngine.h"
#include "../GPS/GPS.h"
#include <vector>
#include <string>

using namespace std;

struct Recommendation
{
    Bus bus;
    double waitTime;
    double rideTime;

    Recommendation() : bus("", "", 0), waitTime(-1.0), rideTime(-1.0) {}
};

class BusRecommendation
{
public:

    static Bus recommend(
        const BusLocationManager& manager,
        const Graph& graph,
        const vector<Route>& routes,
        string currentStopID,
        string destinationStopID
    );

    // Uses each bus's position: skips buses that have already passed the
    // student's stop and picks the lowest wait + ride time.
    static Recommendation recommendLive(
        const BusLocationManager& manager,
        const vector<Route>& routes,
        string currentStopID,
        string destinationStopID
    );
};

#endif