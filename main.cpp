#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <string>
#include <queue>
#include <random>
#include <chrono>
#include <thread>
#include <fstream>
#include <sstream>
#include <cassert>
#include <map>
#include <iterator>
#include "Instance.h"
#include "Route.h"
#include "Solution.h"
#include "Utils.h"

#define NDEBUG

using namespace std;

Solution ILS_Reset(Solution &initialSolution, Instance &instance, mt19937 randomGenerator, double maxExecutionTime, int maxIterationsWithoutImprovement)
{
    Solution currentSolution = initialSolution;
    Solution neighborSolution = initialSolution;
    Solution bestLocalSolution = initialSolution;
    Solution bestGlobalSolution = initialSolution;
    string caller;

    auto start = std::chrono::high_resolution_clock::now();
    instance.totalIterations = 0;
    int iterationsWithoutImprovement = 0;
    // cout << "[ILS_Reset] Iniciando algoritmo ILS com Reset" << endl;
    // cout << "[ILS_Reset] Solução Inicial Score: " << initialSolution.totalScore << endl;


    while (true)
    {
        double porcentagem_perturbacao = static_cast<double>(iterationsWithoutImprovement) / maxIterationsWithoutImprovement;

        neighborSolution = currentSolution;
        neighborSolution.PerturbationWithStrength(instance, randomGenerator, porcentagem_perturbacao);
        neighborSolution.updateSolutionTimeWindows();
        caller = "Pertubação";
        neighborSolution.checkSolution(instance, caller);

        neighborSolution.localSearch(instance, randomGenerator);
        neighborSolution.updateSolutionTimeWindows();
        caller = "Busca Local";
        neighborSolution.checkSolution(instance, caller);

        bool improved = false;
        if (!Utils::doubleGreaterOrEqual(bestLocalSolution.totalScore, neighborSolution.totalScore))
        {
            bestLocalSolution = neighborSolution;
            currentSolution = bestLocalSolution;
            iterationsWithoutImprovement = 0; // Reset ao encontrar uma melhora
            improved = true;
        }
        else
        {
            iterationsWithoutImprovement++;
        }


        if (iterationsWithoutImprovement >= maxIterationsWithoutImprovement)
        {
            // cout << "[ILS_Reset Debug] Reset acionado! Iter s/ Melhora: " << iterationsWithoutImprovement << ". BestLocal: " << bestLocalSolution.totalScore << ", BestGlobal: " << bestGlobalSolution.totalScore << endl;
            if (bestGlobalSolution.totalScore < bestLocalSolution.totalScore)
            {
                // cout << "[ILS_Reset Debug] Atualizando BestGlobal antes do Reset." << endl;
                bestGlobalSolution = bestLocalSolution;
            }
            Solution newSolution(instance);
            newSolution.Constructive(instance, randomGenerator);
            // cout << "[ILS_Reset Debug] Nova solução construtiva gerada. Score: " << newSolution.totalScore << endl;
            bestLocalSolution = newSolution;
            currentSolution = bestLocalSolution;
            iterationsWithoutImprovement = 0;
        }

        // Verifica se o tempo máximo foi atingido
        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = now - start;

        if (duration.count() >= maxExecutionTime)
        {
            // cout << "[ILS_Reset Debug] Tempo máximo atingido: " << duration.count() << "s. Encerrando." << endl;
            if (bestGlobalSolution.totalScore < bestLocalSolution.totalScore)
            {
                //  cout << "[ILS_Reset Debug] Atualizando BestGlobal na saída por tempo." << endl;
                bestGlobalSolution = bestLocalSolution;
            }
            break;
        }

        instance.totalIterations++;
    }
    // cout << "[ILS_Reset] Fim do algoritmo. Melhor Score Global: " << bestGlobalSolution.totalScore << " | Total Iterações: " << instance.totalIterations << endl;
    return bestGlobalSolution;
}

int main(int argc, char *argv[])
{
    unsigned int seed_value;
    std::random_device rd;
    double maxExecutionTime = 10.0;
    int maxIterationsWithoutImprovement = 1000;

    string fileName = string(argv[1]);
    // Instance grafo("C:/Users/athus/Faculdade/6 periodo/PIBIT/solucao/pibit-rotas-pm/misc/ILS-algoritm/" + instancia, t_prot, t_parada, velocidade);
    if (argc > 2){

    }
    if (argc > 3)
    {
        seed_value = stoul(argv[2]);
        // cout << "Seed fornecida: " << seed_value << std::endl;
    }
    else
    {
        random_device rd;
        seed_value = rd();
        // cout << "Seed aleatória gerada: " << seed_value << std::endl;
    }
    std::ofstream outputFile("resultados_" + fileName);

    // Verifica se o arquivo foi aberto corretamente
    if (!outputFile)
    {
        std::cerr << "Erro ao abrir o arquivo!" << std::endl;
        return 1;
    }
    
    Instance instance("Instancias/" + fileName);
    Solution best_s = Solution(instance);
    double mean_score = 0;
    long long int mean_it = 0;
    double b_score_construtivo;
    unsigned int seed_best;
    int n;
    for (n = 0; n < 3; n++)
    {

        random_device rd;
        seed_value = rd();
        mt19937 gen(seed_value);
        Solution s0(instance);
        // cout << s0 << endl;
        // cout << "Construtivo" << endl;
        s0.Constructive(instance, gen);
        // cout << s0 << endl;
        s0.updateSolutionTimeWindows();
        string chamou = "Construtivo";
        s0.checkSolution(instance, chamou);

        Solution s1 = ILS_Reset(s0, instance, gen, maxExecutionTime, maxIterationsWithoutImprovement);
        mean_score += s1.totalScore;
        if (best_s.totalScore < s1.totalScore)
        {
            best_s = s1;
            seed_best = seed_value;
            b_score_construtivo = s0.totalScore;
        }
        double percentual_melhora = (static_cast<double>(s1.totalScore - s0.totalScore) / s0.totalScore) * 100;

        outputFile << "Tempo Máximo: " << instance.maxTime * instance.numVehicles << std::endl;
        outputFile << "Seed: " << seed_value << " | Tempo de Execução: " << maxExecutionTime << "s | Max Itereções sem melhora: " << maxIterationsWithoutImprovement << endl;
        outputFile << "T_prot: " << instance.protectionTime / 60 << "min | T_parada: " << instance.stopTime / 60 << "min | Velocidade: " << "Km/h" << endl;
        outputFile << "Instância: " << fileName << " | Vértices: " << instance.numVertices << " | Veículos: " << instance.numVehicles << endl;
        outputFile << "Solução Construtivo - Score: " << s0.totalScore << " | Custo: " << s0.totalCost << endl;
        outputFile << "Solução ILS - Score: " << s1.totalScore << " | Custo: " << s1.totalCost << endl;
        outputFile << "Melhoria de " << percentual_melhora << "%" << endl
                   << endl;

        outputFile << "Contagem Estruturas de Vizinhança: " << endl;
        outputFile << "Best Insert = " << s1.neighborhoodUsageCount["bestInsert"] << "/" << s1.totalNeighborhoodOperations["bestInsert"] << endl;
        outputFile << "Swap Inter = " << s1.neighborhoodUsageCount["swapInter"] << "/" << s1.totalNeighborhoodOperations["swapInter"] << endl;
        outputFile << "Swap Intra = " << s1.neighborhoodUsageCount["swapIntra"] << "/" << s1.totalNeighborhoodOperations["swapIntra"] << endl;
        outputFile << "Swap Out = " << s1.neighborhoodUsageCount["swapOut"] << "/" << s1.totalNeighborhoodOperations["swapOut"] << endl;
        outputFile << "Para = " << s1.neighborhoodUsageCount["stop"] << "/" << s1.totalNeighborhoodOperations["stop"] << endl;
        outputFile << "Realocate = " << s1.neighborhoodUsageCount["realocate"] << "/" << s1.totalNeighborhoodOperations["realocate"] << endl;
        outputFile << "Contagem de Melhorias por rota" << endl;
        priority_queue<Route> rotas_prontas;
        while (!s0.routes.empty())
        {
            Route rota = s0.routes.top();
            s0.routes.pop();
            outputFile << "Rota " << rota.id << "= " << s1.improvedRoutes[rota.id] << "/" << s1.testedRoutes[rota.id] << endl;
            rotas_prontas.push(rota);
        }
        s0.routes = rotas_prontas;

        outputFile << "Iterações totais: " << instance.totalIterations << endl;
        outputFile << "***********************************************" << std::endl
                   << endl;
    }

    mean_score = mean_score / n;
    mean_it = static_cast<int>(instance.totalIterations / n);
    outputFile << "-- Seed da melhor execução: " << seed_best << std::endl;
    outputFile << "-- Média de iterações: " << mean_it << std::endl;
    outputFile << "-- Pontuação do Construtivo da melhor execução: " << b_score_construtivo << std::endl;
    outputFile << "-- Média de pontuação ILS: " << mean_score << std::endl;
    outputFile << "-- Melhor pontuação: " << best_s.totalScore << std::endl;
    outputFile.close();
    // instancia, seed_best, tempo, total_iterações, it_reset, mean_score, best_score, custo,
    // cout << fileName << ", " << seed_best << ", " << maxExecutionTime << ", " << mean_it << ", " << maxIterationsWithoutImprovement << ", " << mean_score << ", " << best_s.totalScore << endl;

    // exportar um .TXT
}