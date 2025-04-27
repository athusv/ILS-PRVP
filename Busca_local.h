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
    static bool swap_Out_rotas(Instance &grafo, Sol &S, Caminho &rota, int i_inicial, int i_final, bool &best);
    static bool swap_intra_rotas(Instance &grafo, Sol &S, Caminho &rota1, Caminho &rota2, int i_inicial, int i_final, bool &best);

private:
    static bool best_insert(const Instance &grafo, Sol &S, Caminho &rota, bool &best);
    static bool swap_inter_rotas(Instance &grafo, Sol &S, Caminho &rota, bool &best);
    static bool para(const Instance &grafo, Sol &S, Caminho &rota, bool &best);
    static bool realocate(const Instance &grafo, Sol &S, Caminho &rota, bool &best);
    static bool two_opt(const Instance &grafo, Sol &S, Caminho &rota, bool &best);
    static bool swap_paradas_inter_rota(Instance &grafo, Sol &S, Caminho &rota, std::tuple<int, int, int, double, double> &best_swap_info);
    bool efetuar_melhor_troca(Instance &grafo, Sol &S, Caminho &rota, std::tuple<int, int, int, double, double> &best_swap_info, int &score_s, double &custo_s);
};

#endif 
