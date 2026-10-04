#include <vector>
#include <unordered_map>
#include <string>
#include "Route.h"

using namespace std;

struct Edge{
    string destination;
    double traveltime;
};

class Graph{
    private:
        unordered_map<string , vector<Edge>> adjacencyList;
    public:
        void addRoute(const Route& route);
        vector<Edge> getNeighbours(string stopID) const;
};