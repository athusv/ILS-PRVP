#ifndef LOCALSEARCH_H
#define LOCALSEARCH_H

#include "Instance.h"
#include "Solution.h"
#include "Utils.h"
#include <queue>
#include <vector>
#include <random>

class LocalSearch {
public:
    static void localSearch(Instance &instance, Solution &solution, mt19937 &randomGenerator);
    static bool outOfRouteSwap(Instance &instance, Solution &solution, Route &route, int startIndex, int endIndex, bool &best);

private:
    static bool bestInsert(const Instance &instance, Solution &solution, Route &route, bool &isBest);
    static bool intraRouteSwap(const Instance &instance, Solution &solution, Route &route, bool &isBest);
    static bool interRouteSwap(const Instance &instance, Solution &solution, Route &route1, Route &route2, int startIndex, int endIndex, bool &isBest);
    static bool tryMakeStop(const Instance &instance, Solution &solution, Route &route, bool &isBest);
    static bool realocate(const Instance &grafo, Solution &S, Route &rota, bool &best);
    static bool twoOpt(const Instance &instance, Solution &solution, Route &route, bool &best);
};

#endif 
