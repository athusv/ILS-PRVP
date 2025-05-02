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
public:
    double totalScore = 0;
    double totalCost = 0;
    int iterationCount = 0;
    priority_queue<Route> routes;
    vector<map<double, int>> visitedVertices; // numero_vertice - quando/quem visitou
    map<string, int> cont_vizinhanca = {{"best_insert", 0},{"swap_inter", 0}, {"swap_intra", 0}, {"swap_out", 0}, {"para", 0}, {"realocate", 0}};
    unordered_map<string, int> cont_vizinhanca_total = {{"best_insert", 0}, {"swap_inter", 0}, {"swap_intra", 0}, {"swap_out", 0}, {"para", 0}, {"realocate", 0}};
    unordered_map<int, int> improvedRoutes;
    unordered_map<int, int> testedRoutes;

    Solution(const Instance &instance);
    void Constructive(Instance &instance, mt19937 &randomGenerator);
    void PerturbationWithStrength(Instance &instance, mt19937 &randomGenerator, double perturbationIntensity);
    void printVisited(int startIndex, int endIndex) const;
    bool checkSequenceVertex(Instance &instance, string &caller);
    bool checkVisited(const Instance &instance, string &caller) const;
    bool checkScore(Instance &instance, string &caller);
    bool checkCost(Instance &instance, string &caller);
    bool checkSolution(Instance &instance, string &caller);
    bool checkTimeWindows(Instance &instance, string &caller);
    bool checkArrivalTime(Instance &instance, string &caller);
    void updateSolutionTimeWindows();
    void printSolution(Instance &instance);
    bool operator<(const Solution &solution) const;
    friend ostream &operator<<(ostream &os, const Solution &solution);
};

#endif // SOL_H
