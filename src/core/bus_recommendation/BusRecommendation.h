#ifndef BUS_RECOMMENDATION_H
#define BUS_RECOMMENDATION_H

#include "../models/Bus.h"
#include "../bus_location_manager/BusLocationManager.h"
#include "../datastructures/Graph.h"
#include "../models/Route.h"
#include "../ETA/ETAEngine.h"
#include <vector>
#include <string>

using namespace std;

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
};

#endif