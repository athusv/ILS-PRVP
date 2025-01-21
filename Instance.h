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
    double t_max;
    int t_prot;
    int t_parada;
    int velocidade;
    vector<double> score_vertices;
    vector<vector<double> > distancia_matriz;

    enum class Operacao
    {
        Insert,
        SwapInter,
        SwapIntra,
        SwapOut,
        Para,
        Vazio
    };

    Instance(const string &filename);
    friend ostream &operator<<(ostream &os, const Instance &instance);
};

#endif // Instance_H
