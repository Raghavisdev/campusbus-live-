#ifndef ETA_H
#define ETA_H

#include <string>
#include "../datastructures/Graph.h"
#include "../models/Route.h"

using namespace std;

class ETA
{
public:

    static double calculateETA(
        const Graph& graph,
        const Route& route,
        string currentStopID,
        string destinationStopID
    );
};

#endif