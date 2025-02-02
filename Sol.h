#ifndef SOL_H
#define SOL_H

#include <iostream>
#include <vector>
#include <map>
#include <utility>
#include <algorithm>
#include <queue>
#include <cassert>
#include "Caminho.h"  // Inclui o cabeçalho da classe Caminho
#include "Instance.h"  // Inclui o cabeçalho da classe Instance

using namespace std;

class Sol {
private:
public:
    double score = 0;
    double custo = 0;
    vector<Caminho> rotas;
    vector<map<double, int>> visited_vertices;  // numero_vertice - quando/quem visitou
    map<string, int> cont_vizinhanca = {{"best_insert", 0},{"swap_inter", 0}, {"swap_intra", 0}, {"swap_out", 0}, {"para", 0}};

    Sol(const Instance &grafo);
    void print_visited(int inicio, int final) const;
    bool checa_rota(const Instance &grafo, string &chamou);
    bool checa_visited(const Instance &grafo, string &chamou) const;
    bool checa_score(const Instance &grafo, string &chamou);
    bool checa_custo(const Instance &grafo, string &chamou);
    bool checa_solucao(const Instance &grafo, string &chamou);
    bool checa_push(const Instance &grafo, string &chamou);
    bool checa_visita_custo(const Instance &grafo, string &chamou);
    void atualiza_push(const Instance &grafo);
    void print_solucao(const Instance &grafo);
    bool operator<(const Sol &s) const;
    friend ostream &operator<<(ostream &os, const Sol &sol);
};

#endif // SOL_H
