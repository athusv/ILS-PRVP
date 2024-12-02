#ifndef PERTURBACAO_H
#define PERTURBACAO_H

#include <queue>
#include <random>
#include <vector>
#include <iostream>
#include "Instance.h"
#include "Sol.h"
#include "Caminho.h"
#include "Utils.h"
#include "Busca_local.h"

class Perturbacao {
public:
    static Sol& perturbacao(Instance& grafo, Sol& S, std::mt19937& gen);
    static Sol &perturbacao_strength(Instance &grafo, Sol &S, std::mt19937 &gen, double porcentagem_perturbacao);
};

#endif // PERTURBACAO_H
