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

            int studentIndex = routes[j].getStopIndex(currentStopID);
            int nextStopOrder = bus.getNextStopOrder();
            double waitTime;

            // Off route, finished its trip, or no recent GPS
            if (nextStopOrder == 0)
            {
                continue;
            }

            if (nextStopOrder > 0)
            {
                // Position on the road line: the bus is heading to stop "next",
                // a fraction "progress" of the way from the stop before it
                int next = nextStopOrder - 1;

                // Next stop is already past the student's stop
                if (next > studentIndex)
                {
                    continue;
                }

                waitTime = routes[j].timeBetween(next, studentIndex);

                if (next > 0)
                {
                    double progress = bus.getSegmentProgress();
                    waitTime += (1.0 - progress) * routes[j].timeBetween(next - 1, next);
                }
            }
            else
            {
                // No road line for this route yet: use the nearest stop
                string busStop = GPS::findNearestStop(
                    routes[j],
                    bus.getLatitude(),
                    bus.getLongitude()
                );

                int busIndex = routes[j].getStopIndex(busStop);

                // Bus has already passed the student's stop
                if (busIndex < 0 || busIndex > studentIndex)
                {
                    continue;
                }

                waitTime = routes[j].timeBetween(busIndex, studentIndex);
            }

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