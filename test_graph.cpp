// Tests for MinHeap and Graph::shortestJourney (Dijkstra).
// Build with build.bat, run: output\test_graph.exe

#include <iostream>
#include <vector>

#include "src/core/datastructures/MinHeap.h"
#include "src/core/datastructures/Graph.h"

using namespace std;

int failures = 0;

void check(bool ok, string what)
{
    cout << (ok ? "PASS  " : "FAIL  ") << what << endl;

    if (!ok)
    {
        failures++;
    }
}

BusStop stop(string id)
{
    return BusStop(id, id, 30.0, 78.0);
}

int main()
{
    // ---- MinHeap: items must come out in ascending order ----
    MinHeap heap;
    unsigned int seed = 12345;

    for (int i = 0; i < 200; i++)
    {
        seed = seed * 1103515245u + 12345u;
        heap.push((seed >> 8) % 1000, "x");
    }

    bool sorted = true;
    double previous = -1.0;
    size_t popped = 0;

    while (!heap.empty())
    {
        double key = heap.pop().key;

        if (key < previous)
        {
            sorted = false;
        }

        previous = key;
        popped++;
    }

    check(sorted && popped == 200, "MinHeap pops 200 random keys in ascending order");

    MinHeap small;
    small.push(5, "e");
    small.push(1, "a");
    small.push(3, "c");
    check(small.pop().value == "a" && small.pop().value == "c" && small.pop().value == "e",
          "MinHeap returns the smallest key first");

    // ---- Dijkstra on a small graph with known answers ----
    //   R1:  A -2-> B -3-> C
    //   R2:  A -10-> C            (slower direct road)
    //   R3:  C -4-> D
    // Changing from one bus to another costs 5 minutes.
    Route r1("R1");
    r1.addstop(stop("A"));
    r1.addstop(stop("B"), 2.0);
    r1.addstop(stop("C"), 3.0);

    Route r2("R2");
    r2.addstop(stop("A"));
    r2.addstop(stop("C"), 10.0);

    Route r3("R3");
    r3.addstop(stop("C"));
    r3.addstop(stop("D"), 4.0);

    Graph graph;
    graph.addRoute(r1);
    graph.addRoute(r2);
    graph.addRoute(r3);

    Journey toC = graph.shortestJourney("A", "C");
    check(toC.found && toC.minutes == 5.0, "A to C takes 5 min (via B on R1, not the 10 min direct road)");
    check(toC.stops.size() == 3 && toC.stops[1] == "B", "A to C goes through B");

    Journey toD = graph.shortestJourney("A", "D");
    check(toD.found && toD.minutes == 14.0, "A to D takes 14 min: 9 min of riding + 5 min to change bus at C");
    check(toD.stops.size() == 4 && toD.legRoutes.size() == 3, "A to D has 4 stops and 3 legs");
    check(toD.legRoutes[0] == "R1" && toD.legRoutes[1] == "R1" && toD.legRoutes[2] == "R3",
          "A to D uses R1, R1, then changes to R3");

    // A direct bus that is slower to ride but needs no change beats the connection
    Route r4("R4");
    r4.addstop(stop("A"));
    r4.addstop(stop("D"), 12.0);
    graph.addRoute(r4);

    Journey direct = graph.shortestJourney("A", "D");
    check(direct.found && direct.minutes == 12.0 && direct.legRoutes.size() == 1 && direct.legRoutes[0] == "R4",
          "with a 12 min direct bus R4, A to D takes it (no change needed)");
    check(graph.shortestJourney("A", "C").minutes == 5.0, "adding R4 does not change A to C");

    check(!graph.shortestJourney("D", "A").found, "D to A is not reachable (one-way routes)");
    check(!graph.shortestJourney("Z", "A").found, "an unknown start stop gives no journey");

    Journey same = graph.shortestJourney("B", "B");
    check(same.found && same.minutes == 0.0 && same.stops.size() == 1, "B to B takes 0 min");

    cout << endl << (failures == 0 ? "All tests passed." : "Some tests FAILED.") << endl;
    return failures == 0 ? 0 : 1;
}
