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
    static Sol &best_improvement(Instance &grafo, Sol &s0, mt19937 gen);
    static bool swap_out_rotas(Instance &grafo, Sol &S, bool &best);
    static bool swap_intra_rotas(Instance &grafo, Sol &S, bool &best);

private:
    static bool best_insert(Instance &grafo, Sol &S, bool &best);
    static bool swap_inter_rotas(Instance &grafo, Sol &S, bool &best);
    static bool para(Instance &grafo, Sol &S, bool &best);
    static bool swap_paradas_inter_rota(Instance &grafo, Sol &S, Caminho &rota, std::tuple<int, int, int, double, double> &best_swap_info);
    bool efetuar_melhor_troca(Instance &grafo, Sol &S, Caminho &rota, std::tuple<int, int, int, double, double> &best_swap_info, int &score_s, double &custo_s);
};

#endif 
