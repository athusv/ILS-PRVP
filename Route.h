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
    double maxTime;
    int speed;
    vector<vector<double>> costMatrix;
    vector<int> vertexSequence; // Id of the vertices in the route
    vector<bool> isStopVertex;  // Stop or Pass
    vector<double> arrivalTimes;                 // Cost when the vertex was visited
    vector<pair<double, double>> timeWindows;    // Backward and Forward flexibility

    Route(const Instance &instance, int id, int vehicleType);

    bool realocate(const vector<double> &vertexScores, vector<map<double, int>> &visitedVertices, double &solutionScore, double &solutionCost, bool &isBest);
    bool tryMakeStop(const vector<double> &vertexScores, vector<map<double, int>> &visitedVertices, double &solutionScore, double &solutionCost, bool &isBest);
    bool bestInsert(const vector<double> &vertexScores, int numVertices, vector<map<double, int>> &visitedVertices, double &solutionScore, double &solutionCost, bool &isBest);
    bool intraRouteSwap(const Instance &instance, vector<map<double, int>> &visitedVertices, double &solutionScore, double &solutionCost, bool &isBest);
    bool outOfRouteSwap(const vector<double> &vertexScores, int numVertices, vector<map<double, int>> &visitedVertices, double &solutionScore, double &solutionCost, int startIndex, int endIndex, bool &isBest);

    bool interRouteSwap(const vector<double> &vertexScores, vector<map<double, int>> &visitedVertices, double &solutionScore, double &solutionCost, Route &route2, int i_inicial, int i_final, bool &isBest);
    bool twoOpt();
    void printVisited(vector<map<double, int>> &visitedVertices, int startIndex, int endIndex) const;
    void insertVertex(vector<double> &bestInsert, vector<map<double, int>> &visitedVertices, double &solutionScore, double &solutionCost);
    void excludeVertex(vector<double> &excludeVertex, vector<map<double, int>> &visitedVertices, double &solutionScore, double &solutionCost);
    void makeStop(vector<double> stopVertex, vector<map<double, int>> &visitedVertices, double &solutionScore, double &solutionCost);
    void makePass(vector<double> passVertex, vector<map<double, int>> &visitedVertices, double &solutionScore, double &solutionCost);
    void updateVisitedVertices(int startIndex, int endIndex, double impact, vector<map<double, int>> &visitedVertices);
    void updateTimeWindows(vector<map<double, int>> &visitedVertices);
    void printTimeWindows();
    void printArrivalTimes();
    friend ostream &operator<<(ostream &os, const Route &route);
    bool operator<(const Route &route) const;
};

#endif // ROUTE_H
