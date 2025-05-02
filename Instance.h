#ifndef Instance_H
#define Instance_H

#include <iostream>
#include <vector>
#include <fstream>
#include <string>

using namespace std;

class Instance {
public:
    long long int totalIterations;
    int numVertices;
    int numVehicles;
    vector<int> vehicleTypes;
    double maxTime;
    int protectionTime;
    int stopTime;
    vector<int> speed;
    vector<double> vertexScores;
    vector<vector<double>> distanceMatrix;

    Instance(const string &filename);
    friend ostream &operator<<(ostream &os, const Instance &instance);
};

#endif // Instance_H
