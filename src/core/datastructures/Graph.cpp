#include "Graph.h"

void Graph::addRoute(const Route& route)
{
    vector<BusStop> stops = route.getRoute();

    for (int i = 0; i < stops.size() - 1; i++)
    {
        string from = stops[i].getStopID();
        string to = stops[i + 1].getStopID();

        Edge edge;
        edge.destination = to;
        edge.traveltime = 5.0;

        adjacencyList[from].push_back(edge);
    }
}

vector<Edge> Graph::getNeighbours(string stopID) const
{
    return adjacencyList.at(stopID);
}
