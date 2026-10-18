#include "Graph.h"
#include <algorithm>
void Graph::addRoute(const Route& route)  
{
    vector<BusStop> stops = route.getRoute();
    vector<double> travelTimes =  route.getTravelTimes();
    string routeID=  route.getRouteId();
    for (size_t i=0;i<stops.size();i++)
    {
        vector<string>& routes = routesAtStop[stops[i].getStopID()];
        if (find(routes.begin(), routes.end(), routeID) == routes.end())
        {
            routes.push_back(routeID);
        }
    }
    for (size_t i=0;i+1< stops.size();i++)
    {
        string from = stops[i].getStopID();
        string to = stops[i + 1].getStopID();
        Edge edge;
        edge.destination = to;
        edge.traveltime  = travelTimes[i];
        adjacencyList[from].push_back(edge);
    }
}

vector<Edge> Graph::getNeighbours(string stopID) const  
{
    auto it = adjacencyList.find(stopID);
    if (it == adjacencyList.end())
    {
        return {};
    }
    return it->second;
}
vector<string> Graph::findRoutes(string stopID) const
{
    auto it = routesAtStop.find(stopID);
    if (it == routesAtStop.end())
    {
        return {};
    }
    return it->second;
}

vector<string> Graph::findRoutes(string fromStopID, string toStopID) const
{
    vector<string> fromRoutes = findRoutes(fromStopID);
    vector<string> toRoutes = findRoutes(toStopID);
    vector<string> commonRoutes;
    for (size_t i = 0; i < fromRoutes.size(); i++)
    {
        if (find(toRoutes.begin(), toRoutes.end(), fromRoutes[i]) != toRoutes.end())
        {
            commonRoutes.push_back(fromRoutes[i]);
        }
    }
    return commonRoutes;
}