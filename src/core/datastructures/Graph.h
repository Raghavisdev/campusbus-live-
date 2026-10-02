#include <vector>
#include <unordered_map>
#include <string>
#include "Route.h"

using namespace std;

class Graph{
    private:
        unordered_map<string , vector<string>> adjacencyList;
    public:
        void addRoute(const Route& route);
};