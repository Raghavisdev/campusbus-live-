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

Recommendation BusRecommendation::recommendLive(
    const BusLocationManager& manager,
    const vector<Route>& routes,
    string currentStopID,
    string destinationStopID
)
{
    vector<Bus> buses = manager.getAllBuses();

    Recommendation best;
    double bestTotal = 999999999.0;

    for (size_t i = 0; i < buses.size(); i++)
    {
        Bus bus = buses[i];

        if (bus.getOccupancy() >= bus.getCapacity())
        {
            continue;
        }

        // No status row means no known position
        if (bus.getLastUpdated() == "")
        {
            continue;
        }

        for (size_t j = 0; j < routes.size(); j++)
        {
            if (routes[j].getRouteId() != bus.getRouteID())
            {
                continue;
            }

            RouteETA routeETA;

            double rideTime = routeETA.calculate(
                routes[j],
                currentStopID,
                destinationStopID
            );

            if (rideTime < 0)
            {
                continue;
            }

            string busStop = GPS::findNearestStop(
                routes[j],
                bus.getLatitude(),
                bus.getLongitude()
            );

            int busIndex = routes[j].getStopIndex(busStop);
            int studentIndex = routes[j].getStopIndex(currentStopID);

            // Bus has already passed the student's stop
            if (busIndex < 0 || busIndex > studentIndex)
            {
                continue;
            }

            double waitTime = routes[j].timeBetween(busIndex, studentIndex);
            double total = waitTime + rideTime;

            if (total < bestTotal)
            {
                bestTotal = total;
                best.bus = bus;
                best.waitTime = waitTime;
                best.rideTime = rideTime;
            }
        }
    }

    return best;
}