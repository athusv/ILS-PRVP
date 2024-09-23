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
    static Sol& perturbacao(Instance& grafo, Sol& s0, std::mt19937& gen);
};

#endif // PERTURBACAO_H
