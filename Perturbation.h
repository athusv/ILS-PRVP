#ifndef PERTURBATION_H
#define PERTURBATION_H

#include <queue>
#include <random>
#include <vector>
#include <iostream>
#include "Instance.h"
#include "Solution.h"
#include "Route.h"
#include "Utils.h"
#include "LocalSearch.h"

class Perturbation {
public:
    static void applyPerturbationWithStrength(Instance &instance, Solution &solution, std::mt19937 &randomGenerator, double perturbationIntensity);
};

#endif // PERTURBATION_H
