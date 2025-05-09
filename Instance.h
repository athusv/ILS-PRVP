#ifndef Instance_H
#define Instance_H

#include <iostream>
#include <vector>
#include <fstream>
#include <string>

using namespace std;

class Instance {
public:
    long long int iteracoes_totais;
    int qt_vertices;
    int veiculos;
    vector<int> tipo_veiculo;
    double t_max;
    int t_prot;
    int t_parada;
    vector<int> velocidade;
    vector<double> score_vertices;
    vector<vector<double> > distancia_metros;

    Instance(const string &filename);
    friend ostream &operator<<(ostream &os, const Instance &instance);
};

#endif // Instance_H
