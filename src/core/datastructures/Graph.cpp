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
        edge.routeID     = routeID;
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


const double TRANSFER_MINUTES = 5.0;

static string makeState(const string& stopID, const string& routeID)
{
    return stopID + "|" + routeID;
}

static string stopOf(const string& state)
{
    return state.substr(0, state.find('|'));
}

static string routeOf(const string& state)
{
    return state.substr(state.find('|') + 1);
}

Journey Graph::shortestJourney(string fromStopID, string toStopID) const
{
    Journey journey;

    auto startRoutes = routesAtStop.find(fromStopID);

    if (startRoutes == routesAtStop.end())
    {
        return journey;    
    }

    if (fromStopID == toStopID)
    {
        journey.found = true;
        journey.stops.push_back(fromStopID);
        return journey;
    }

    unordered_map<string, double> dist;
    unordered_map<string, string> previousState;    

    MinHeap heap;

    auto relax = [&](const string& state, double candidate, const string& cameFrom)
    {
        auto known = dist.find(state);

        if (known == dist.end() || candidate < known->second)
        {
            dist[state] = candidate;
            previousState[state] = cameFrom;
            heap.push(candidate, state);
        }
    };

    for (size_t i = 0; i < startRoutes->second.size(); i++)
    {
        string state = makeState(fromStopID, startRoutes->second[i]);
        dist[state] = 0.0;
        heap.push(0.0, state);
    }

    string bestState = "";

    while (!heap.empty())
    {
        HeapItem current = heap.pop();

        if (current.key > dist[current.value])
        {
            continue;
        }

        string stop = stopOf(current.value);
        string route = routeOf(current.value);

        if (stop == toStopID)
        {
            bestState = current.value;
            break;
        }

        auto edges = adjacencyList.find(stop);

        if (edges != adjacencyList.end())
        {
            for (size_t i = 0; i < edges->second.size(); i++)
            {
                const Edge& edge = edges->second[i];

                if (edge.routeID == route)
                {
                    relax(makeState(edge.destination, route), current.key + edge.traveltime, current.value);
                }
            }
        }

        auto here = routesAtStop.find(stop);

        if (here != routesAtStop.end())
        {
            for (size_t i = 0; i < here->second.size(); i++)
            {
                if (here->second[i] != route)
                {
                    relax(makeState(stop, here->second[i]), current.key + TRANSFER_MINUTES, current.value);
                }
            }
        }
    }

    if (bestState == "")
    {
        return journey;    
    }

    vector<string> chain;
    string state = bestState;

    while (true)
    {
        chain.push_back(state);

        auto before = previousState.find(state);

        if (before == previousState.end())
        {
            break;
        }

        state = before->second;
    }

    reverse(chain.begin(), chain.end());

    journey.stops.push_back(stopOf(chain[0]));

    for (size_t i = 1; i < chain.size(); i++)
    {
        if (stopOf(chain[i]) != stopOf(chain[i - 1]))
        {
            journey.stops.push_back(stopOf(chain[i]));
            journey.legRoutes.push_back(routeOf(chain[i]));
        }
    }

    journey.found = true;
    journey.minutes = dist[bestState];

    return journey;
}