#include "ETAEngine.h"

double RouteETA::calculate(
    const Route& route,
    string currentStopID,
    string destinationStopID
) const
{
    int from = route.getStopIndex(currentStopID);
    int to = route.getStopIndex(destinationStopID);

    if (from == -1 || to == -1)
    {
        return -1.0;
    }

    if (from >= to)
    {
        return -1.0;
    }

    return route.timeBetween(from, to);
}

double ETA::calculateETA(
    const Graph& graph,
    const Route& route,
    string currentStopID,
    string destinationStopID
)
{

    RouteETA routeETA;
    const ETACalculator& calculator = routeETA;

    return calculator.calculate(route, currentStopID, destinationStopID);
}