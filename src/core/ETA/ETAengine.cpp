#include "ETAEngine.h"

double ETA::calculateETA(
    const Graph& graph,
    const Route& route,
    string currentStopID,
    string destinationStopID
)
{
    vector<BusStop> stops = route.getRoute();

    double totalTime = 0.0;
    bool started = false;

    for (int i = 0; i < stops.size() - 1; i++)
    {
        string from = stops[i].getStopID();
        string to = stops[i + 1].getStopID();

        if (from == currentStopID)
        {
            started = true;
        }

        if (started)
        {
            vector<Edge> neighbours = graph.getNeighbours(from);

            for (int j = 0; j < neighbours.size(); j++)
            {
                if (neighbours[j].destination == to)
                {
                    totalTime += neighbours[j].traveltime;
                    break;
                }
            }
        }

        if (to == destinationStopID)
        {
            return totalTime;
        }
    }

    return -1.0;
}