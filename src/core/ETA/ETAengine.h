#ifndef ETA_H
#define ETA_H

#include <string>
#include "../datastructures/Graph.h"
#include "../models/Route.h"

using namespace std;

class ETACalculator
{
public:

    virtual double calculate(
        const Route& route,
        string currentStopID,
        string destinationStopID
    ) const = 0;

    virtual ~ETACalculator() {}
};

class RouteETA : public ETACalculator
{
public:

    double calculate(
        const Route& route,
        string currentStopID,
        string destinationStopID
    ) const override;
};

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