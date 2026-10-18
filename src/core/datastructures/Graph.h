#ifndef GRAPH_H
#define GRAPH_H
#include <vector>
#include <unordered_map>
#include <string>
#include "../models/Route.h"
using namespace std;
struct Edge
{
    string destination;
    double traveltime;
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
};
#endif