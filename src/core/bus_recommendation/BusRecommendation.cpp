#include "BusRecommendation.h"
#include <cmath>

// How quickly the bus's current pace stops mattering further ahead (minutes)
const double PACE_FADE_MINUTES = 5.0;

// Planned minutes ahead, corrected by the bus's current pace r. The correction
// applies fully to the road just ahead and fades exponentially further on:
//   adjusted = planned + (r - 1) * T0 * (1 - e^(-planned / T0))
static double paceAdjusted(double plannedMinutes, double paceRatio)
{
    return plannedMinutes
        + (paceRatio - 1.0) * PACE_FADE_MINUTES
        * (1.0 - exp(-plannedMinutes / PACE_FADE_MINUTES));
}

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

            // waitTime and rideTime so far are planned minutes; adjust both for
            // how the bus is actually moving (the ride starts after the wait)
            double pace = bus.getPaceRatio();
            double adjustedWait = paceAdjusted(waitTime, pace);
            double total = paceAdjusted(waitTime + rideTime, pace);

            if (total < bestTotal)
            {
                bestTotal = total;
                best.bus = bus;
                best.waitTime = adjustedWait;
                best.rideTime = total - adjustedWait;
            }
        }
    }

    return best;
}