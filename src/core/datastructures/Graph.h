#ifndef GRAPH_H
#define GRAPH_H
#include <vector>
#include <unordered_map>
#include <string>
#include "../models/Route.h"
#include "MinHeap.h"
using namespace std;
struct Edge
{
    string destination;
    double traveltime;
    string routeID;     // the route (bus line) this edge belongs to
};

// Result of Graph::shortestJourney
struct Journey
{
    bool found;
    double minutes;             // total travel time
    vector<string> stops;       // stop IDs from start to destination
    vector<string> legRoutes;   // legRoutes[i] is the route used from stops[i] to stops[i + 1]

    Journey() : found(false), minutes(0.0) {}
};

class Graph
{
private:
    unordered_map<string, vector<Edge>> adjacencyList;
    unordered_map<string, vector<string>> routesAtStop;
public:
    void addRoute(const Route& route);
    vector<Edge> getNeighbours(string stopID) const;
    vector<string> findRoutes(string stopID) const;
    vector<string> findRoutes(string fromStopID, string toStopID) const;

    // Dijkstra's algorithm with a min-heap: fastest way between two stops,
    // possibly changing buses where routes meet (each change costs TRANSFER_MINUTES).
    Journey shortestJourney(string fromStopID, string toStopID) const;
};
#endif