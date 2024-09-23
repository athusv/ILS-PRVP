#ifndef Instance_H
#define Instance_H

#include <iostream>
#include <vector>
#include <fstream>
#include <string>

using namespace std;

class Instance {
public:
    int qt_vertices;
    int veiculos;
    double tmax;
    int t_prot;
    int t_parada;
    vector<double> score_vertices;
    vector<vector<double> > distancia_matriz;

    Instance(const string &filename, int &tprot, int &tparada, double &velocidade);
};

#endif // Instance_H
