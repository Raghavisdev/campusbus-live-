#include "BusRecommendation.h"

Bus BusRecommendation::recommend(
    const BusLocationManager& manager,
    const Graph& graph,
    const vector<Route>& routes,
    string currentStopID,
    string destinationStopID
)
{
    vector<Bus> buses = manager.getAllBuses();

    Bus bestBus("", "", 0);
    double bestETA = 999999999.0;

    for (size_t i = 0; i < buses.size(); i++)
    {
        Bus bus = buses[i];

        if (bus.getOccupancy() >= bus.getCapacity())
        {
            continue;
        }

        for (size_t j = 0; j < routes.size(); j++)
        {
            if (routes[j].getRouteId() != bus.getRouteID())
            {
                continue;
            }

            double eta = ETA::calculateETA(
                graph,
                routes[j],
                currentStopID,
                destinationStopID
            );

            if (eta >= 0 && eta < bestETA)
            {
                bestETA = eta;
                bestBus = bus;
            }
        }
    }

    return bestBus;
}