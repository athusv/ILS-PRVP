#ifndef ROUTE_H
#define ROUTE_H

#include <iostream>
#include <vector>
#include <map>
#include <utility>
#include <algorithm>
#include "Instance.h"
using namespace std;

class Route {
public:
    int id;
    double cost = 0;
    double score = 0;
    int additionalStopTime;
    int protectionTime;
    int stopTime;
    int vehicleType;
    int speed;
    vector<vector<double>> costMatrix;
    vector<int> vertexSequence; // Id of the vertices in the route
    vector<bool> isStopVertex;  // Stop or Pass
    vector<double> arrivalTimes;                 // Cost when the vertex was visited
    vector<pair<double, double>> timeWindows;    // Backward and Forward flexibility

    Route(int id, int protectionTime, int stopTime, int vehicleType, int speed, const Instance &instance);
    void insertVertex(vector<double> &bestInsert, vector<map<double, int>> &visitedVertices, double &solutionScore, double &solutionCost);
    void excludeVertex(vector<double> &excludeVertex, vector<map<double, int>> &visitedVertices, double &solutionScore, double &solutionCost);
    void makeStop(vector<double> stopVertex, vector<map<double, int>> &visitedVertices, double &solutionScore, double &solutionCost);
    void makePass(vector<double> passVertex, vector<map<double, int>> &visitedVertices, double &solutionScore, double &solutionCost);
    void updateVisitedVertices(int startIndex, int endIndex, double impact, vector<map<double, int>> &visitedVertices);
    void updateTimeWindows(vector<map<double, int>> &visitedVertices);
    void printTimeWindows();
    friend ostream &operator<<(ostream &os, const Route &route);
    bool operator<(const Route &route) const;
};

#endif // ROUTE_H
