#ifndef CAMINHO_H
#define CAMINHO_H

#include <iostream>
#include <vector>
#include <map>
#include <utility>
#include <algorithm>

using namespace std;

class Caminho {
public:
    int id;
    double custo = 0;
    double score = 0;
    int z = 0;
    int plus_parada;
    int t_prot;
    int t_parada;
    vector<int> route; // Id dos vertices da rota
    vector<bool> paradas; // Se Para ou Passa
    vector<double> visita_custo; // Custo de quando o vertice foi visitado
    vector<pair<double, double> > push_hotspots; // Para tras e para frente.

    Caminho(int id, int t_prot, int t_parada);
    void insert_v(vector<double> &best_insert, vector<map<double, int> > &visited_vertices, double &score_s, double &custo_s);
    void excluir(vector<double> &exclui_vertice, vector<map<double, int> > &visited_vertices, double &score_s, double &custo_s);
    void parar(vector<double> vertice_parar, vector<map<double, int> > &visited_vertices, double &score_s, double &custo_s);
    void passar(vector<double> vertice_passa, vector<map<double, int> > &visited_vertices, double &score_s, double &custo_s);
    void atualizar_visited_vertices(int start_index, int final_index, double impacto, vector<map<double, int> > &visited_vertices);
    void atualizar_push_hotspots(vector<map<double, int> > &visited_vertices);
    void print_push();
    friend ostream &operator<<(ostream &os, const Caminho &caminho);
    bool operator<(const Caminho &c) const;
};

#endif // CAMINHO_H
