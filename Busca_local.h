#ifndef BUSCA_LOCAL_H
#define BUSCA_LOCAL_H

#include "Instance.h"
#include "Sol.h"
#include "Utils.h"
#include <queue>
#include <vector>
#include <random>

class Busca_local {
public:
    static Sol &busca_local(Instance &grafo, Sol &s0, mt19937 &gen);
    static Sol &best_improvement(Instance &grafo, Sol &s0);
    static vector<vector<double>> swap_Out_rotas(Instance &grafo, Sol &S, Caminho &rota, int i_inicial, int i_final, bool &best);
    static vector<vector<double>> swap_intra_rotas(Instance &grafo, Sol &S, Caminho &rota1, Caminho &rota2, int i_inicial, int i_final, bool &best);

private:
    static vector<double> best_insert(const Instance &grafo, Sol &S, Caminho &rota, bool &best);
    static vector<vector<double>> swap_inter_rotas(Instance &grafo, Sol &S, Caminho &rota, bool &best);
    static vector<double> para(const Instance &grafo, Sol &S, Caminho &rota, bool &best);
    static bool swap_paradas_inter_rota(Instance &grafo, Sol &S, Caminho &rota, std::tuple<int, int, int, double, double> &best_swap_info);
    bool efetuar_melhor_troca(Instance &grafo, Sol &S, Caminho &rota, std::tuple<int, int, int, double, double> &best_swap_info, int &score_s, double &custo_s);
};

#endif 
