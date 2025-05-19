#include "Utils.h"

bool Utils::doubleEquals(double a, double b, double epsilon) {
    return fabs(a - b) < epsilon;
}

bool Utils::doubleGreaterOrEqual(double a, double b, double epsilon) {
    return (a > b) || doubleEquals(a, b, epsilon);
}

bool Utils::doubleLessOrEqual(double a, double b, double epsilon) {
    return (a < b) || doubleEquals(a, b, epsilon);
}

int Utils::findMaxScore(const std::vector<int> &candidates, const std::vector<double> &vertexScores){
    double max = -1;
    int index_max = 0;
    for (auto candidate : candidates) {
        if (vertexScores[candidate] > max){
            max = vertexScores[candidate];
            index_max = candidate;
        }
    }
    return index_max;
}

int Utils::findMinCost(const std::vector<int> &candidates, const vector<vector<double>> &costMatrix, int &lastVertex){
    double min = costMatrix[lastVertex][candidates[0]];
    int index_min = candidates[0];
    for (auto candidate : candidates){
        if (costMatrix[lastVertex][candidate] < min){
            min = costMatrix[lastVertex][candidate];
            index_min = candidate;
        }
    }
    return index_min;
}

int Utils::findBestCostBenefit(const std::vector<int> &candidates, const vector<double> &vertexScores, const vector<vector<double>> &costMatrix, int &lastVertex){
    double benefit = -1;
    int index_benefit = 0;
    for (auto candidate : candidates){
        if (vertexScores[candidate] / costMatrix[lastVertex][candidate] > benefit)
        {
            benefit = vertexScores[candidate] / costMatrix[lastVertex][candidate];
            index_benefit = candidate;
        }
    }
    return index_benefit;
}

std::vector<int> Utils::createCandidateList(Instance &instance, Route &route, std::vector<std::map<double, int>> &visitedVertices){
    std::vector<int> candidateList;
    
    for (int i = 1; i < instance.numVertices; i++) {
        if (visitedVertices[i].empty() || doubleLessOrEqual(visitedVertices[i].rbegin()->first, route.cost)) {
            double dist1 = route.costMatrix[route.vertexSequence.back()][i];
            double dist2 = route.costMatrix[i][0];
            if (route.cost + dist1 + dist2 + route.additionalStopTime < instance.maxTime) {
                candidateList.push_back(i);
            }
        }
    }
    
    return candidateList;
}

std::vector<double> Utils::tryExcludeVertex(Instance &instance, std::vector<std::map<double, int>> &visitedVertices, Route &route, int vertexIndex) {
    std::vector<double> excludeVertex = {-1, -1, -1, -1};
    double dist1 = route.costMatrix[route.vertexSequence[vertexIndex - 1]][route.vertexSequence[vertexIndex]];
    double dist2 = route.costMatrix[route.vertexSequence[vertexIndex]][route.vertexSequence[vertexIndex + 1]];
    double dist3 = route.costMatrix[route.vertexSequence[vertexIndex - 1]][route.vertexSequence[vertexIndex + 1]];

    double impacto = dist3 - dist2 - dist1;
    if (route.isStopVertex[vertexIndex] == 1)
        impacto -= instance.stopTime;

    if (route.cost + impacto > instance.maxTime)
        return excludeVertex;

    bool possibilityVisit;
    if (impacto < 0) {
        possibilityVisit = (Utils::doubleGreaterOrEqual(route.timeWindows[vertexIndex].first, impacto * -1));
    } else {
        possibilityVisit = (Utils::doubleGreaterOrEqual(route.timeWindows[vertexIndex].second, impacto));
    }

    if (possibilityVisit) {
        excludeVertex[0] = route.vertexSequence[vertexIndex];
        excludeVertex[1] = (route.isStopVertex[vertexIndex] == 1)
                                ? instance.vertexScores[route.vertexSequence[vertexIndex]]
                                : instance.vertexScores[route.vertexSequence[vertexIndex]] / 3;
        excludeVertex[2] = impacto;
        excludeVertex[3] = vertexIndex;
    }
    return excludeVertex;
}
