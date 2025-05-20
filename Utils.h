#ifndef UTILS_H
#define UTILS_H

#include <cmath>
#include <vector>
#include <map>
#include "Route.h"
#include "Instance.h"
#include "Solution.h"

class Utils {
public:
    static bool doubleEquals(double a, double b, double epsilon = 1e-9);
    static bool doubleGreaterOrEqual(double a, double b, double epsilon = 1e-9);
    static bool doubleLessOrEqual(double a, double b, double epsilon = 1e-9);
    static std::vector<int> createCandidateList(Instance &instance, Route &route, std::vector<std::map<double, int>> &visitedVertices);
    static int findMaxScore(const std::vector<int> &candidates, const std::vector<double> &vertexScores);
    static int findMinCost(const std::vector<int> &candidates, const vector<vector<double>> &costMatrix, int &lastVertex);
    static int findBestCostBenefit(const std::vector<int> &candidates, const vector<double> &vertexScores, const vector<vector<double>> &costMatrix, int &lastVertex);
    static std::vector<double> tryExcludeVertex(Instance &instance, std::vector<std::map<double, int>> &visitedVertices, Route &route, int vertexIndex);
};

#endif // UTILS_H

