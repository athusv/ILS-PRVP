#ifndef SOLUTION_H
#define SOLUTION_H

#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
#include <utility>
#include <algorithm>
#include <queue>
#include <cassert>
#include "Route.h"  // Inclui o cabeçalho da classe Caminho
#include "Instance.h"  // Inclui o cabeçalho da classe Instance

#include <random>
using namespace std;

class Solution {
private:
    
    void printVisited(int startIndex, int endIndex) const;
    bool checkSequenceVertex(Instance &instance, string &caller);
    bool checkVisited(const Instance &instance, string &caller) const;
    bool checkScore(Instance &instance, string &caller);
    bool checkCost(Instance &instance, string &caller);
    
    bool checkTimeWindows(Instance &instance, string &caller);
    bool checkArrivalTime(Instance &instance, string &caller);
    

public:
    double totalScore = 0;
    double totalCost = 0;
    int iterationCount = 0;
    priority_queue<Route> routes;
    vector<map<double, int>> visitedVertices; // numero_vertice - quando/quem visitou
    map<string, int> neighborhoodUsageCount = {{"bestInsert", 0}, {"swapInter", 0}, {"swapIntra", 0}, {"swapOut", 0}, {"stop", 0}, {"realocate", 0}};
    unordered_map<string, int> totalNeighborhoodOperations = {{"bestInsert", 0}, {"swapInter", 0}, {"swapIntra", 0}, {"swapOut", 0}, {"stop", 0}, {"realocate", 0}};
    unordered_map<int, int> improvedRoutes;
    unordered_map<int, int> testedRoutes;

    Solution(const Instance &instance);
    bool intraRouteSwap(const Instance &instance, Route &route, bool &isBest);
    void Constructive(Instance &instance, mt19937 &randomGenerator);
    void PerturbationWithStrength(Instance &instance, mt19937 &randomGenerator, double perturbationIntensity);
    void localSearch(Instance &instance, mt19937 &randomGenerator);
    void printSolution(Instance &instance);
    void updateSolutionTimeWindows();
    bool checkSolution(Instance &instance, string &caller);
    bool operator<(const Solution &solution) const;
    friend ostream &operator<<(ostream &os, const Solution &solution);
};

#endif // SOL_H
