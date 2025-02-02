#ifndef UTILS_H
#define UTILS_H

#include <cmath>
#include <vector>
#include <map>
#include "Caminho.h"
#include "Instance.h"
#include "Sol.h"

class Utils {
public:
    static bool doubleEquals(double a, double b, double epsilon = 1e-9);
    static bool doubleGreaterOrEqual(double a, double b, double epsilon = 1e-9);
    static bool doubleLessOrEqual(double a, double b, double epsilon = 1e-9);
    static int max_score(const std::vector<int>& lista_candidatos, const std::vector<double>& score_vertices);
    static int min_custo(const std::vector<int> &lista_candidatos, const vector<vector<double>> &distancia_matriz, int &last_vertice);
    static int cost_benefit(const std::vector<int> &lista_candidatos,const vector<double>& score_vertices, const vector<vector<double>> &distancia_matriz, int &last_vertice);
    static std::vector<int> make_lista(Instance& grafo, Caminho& rota, std::vector<std::map<double, int>>& visited_vertices);
    static std::vector<double> p_excluir(Instance& grafo, std::vector<std::map<double, int>>& visited_vertices, Caminho& rota, int i);
    static bool swap_perturbacao(Instance &grafo, Sol &S, Caminho &rota, int i);
    static std::vector<double> p_insert(Instance& grafo, std::vector<std::map<double, int>>& visited_vertices, const Caminho& rota, int i, int vertice_insert);
    static vector<vector<double>> swap_out(Instance &grafo, Sol &S, Caminho &rota, int i_inicial, int i_final, bool &best);
};

#endif // UTILS_H

