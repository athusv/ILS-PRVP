#ifndef Instance_H
#define Instance_H

#include <iostream>
#include <vector>
#include <fstream>
#include <string>

using namespace std;

// Estrutura para armazenar índice do vértice e sua distância
struct VertexDistance
{
    int index;
    double distance;

    VertexDistance(int idx, double dist) : index(idx), distance(dist) {}
};

class Instance {
public:
    long long int totalIterations;
    int numVertex;
    int numVehicles;
    vector<int> vehicleTypes;
    double maxTime;
    int protectionTime;
    int stopTime;
    vector<int> speed;
    vector<double> vertexScores;
    vector<vector<double>> distanceMatrix;

    // Para cada vértice, lista ordenada dos vértices mais próximos (índice e distância)
    vector<vector<VertexDistance>> nearestVertices;

    Instance(const string &filename);
    friend ostream &operator<<(ostream &os, const Instance &instance);
};

#endif // Instance_H
