#include "Solution.h"
#include <cmath>
#include "Utils.h"
#include <random>
#include <nlohmann/json.hpp>
using namespace std;

enum class Operation
{
    BestInsert,
    SwapInter,
    SwapIntra,
    SwapOut,
    Stop,
    // Realocate,
    // TwoOpt
};

Solution::Solution(const Instance &instance) {
    visitedVertices.resize(instance.numVertex);

    int id = 0;
    for (int i = 0; i < instance.vehicleTypes.size(); i++) {
        for (int j = 0; j < instance.vehicleTypes[i]; j++) {
            id++;
            Route aux_rota(instance, id, i);
            routes.push(aux_rota);
            improvedRoutes[i] = 0;
            testedRoutes[i] = 0;
        }
    }
}

bool Solution::intraRouteSwap(const Instance &instance, Route &route, bool &isBest)
{
    vector<vector<double>> swap_inter(4, vector<double>(6, -1));
    double best_swap = -1;
    double score_v1;
    double score_v2;

    double dist1_remove_v1;
    double dist2_remove_v1;
    double dist1_remove_v2;
    double dist2_remove_v2;

    double dist1_add_v1;
    double dist2_add_v1;
    double dist1_add_v2;
    double dist2_add_v2;

    int vertice_parada_v1;
    int vertice_parada_v2;
    int anterior1;
    int proximo1;
    int anterior2;
    int proximo2;

    double impacto1;
    double impacto2;
    // cout << endl <<"Entrando Swap Inter Rotas ****" << endl;
    for (int i = 1; i <= route.vertexSequence.size() - 2; i++)
    {
        // if(best_swap!=-1) break;
        // cout<<"Vertice i["<< i << "]: "<<rota.route[i]<< endl;
        anterior1 = route.vertexSequence[i - 1];
        proximo1 = route.vertexSequence[i + 1];
        score_v1 = (route.isStopVertex[i] == 1) ? instance.vertexScores[route.vertexSequence[i]] : instance.vertexScores[route.vertexSequence[i]] / 3;

        dist1_remove_v1 = route.costMatrix[anterior1][route.vertexSequence[i]]; // arestas que serão removidas V1
        dist2_remove_v1 = route.costMatrix[route.vertexSequence[i]][proximo1];
        vertice_parada_v1 = (route.isStopVertex[i] == 1) ? instance.stopTime : 0; // se tiver parada, plus de 15 minutos

        for (int j = i + 2; j <= route.vertexSequence.size() - 2; j++)
        {
            if (i == j)
                break;
            // cout << "Vertice j[" << j << "]: " << rota.route[j] << endl;
            anterior2 = route.vertexSequence[j - 1];
            proximo2 = route.vertexSequence[j + 1];
            score_v2 = (route.isStopVertex[j] == 1) ? instance.vertexScores[route.vertexSequence[j]] : instance.vertexScores[route.vertexSequence[j]] / 3;

            vertice_parada_v2 = (route.isStopVertex[j] == 1) ? instance.stopTime : 0;

            dist1_remove_v2 = route.costMatrix[anterior2][route.vertexSequence[j]]; // arestas que serão removidas V2
            dist2_remove_v2 = route.costMatrix[route.vertexSequence[j]][proximo2];

            bool seguidos = (j == i + 1);
            if (seguidos)
            {
                dist1_remove_v2 = route.costMatrix[anterior2][route.vertexSequence[j]]; // arestas que serão removidas V2
                dist2_remove_v2 = route.costMatrix[route.vertexSequence[j]][proximo2];

                dist1_add_v1 = route.costMatrix[route.vertexSequence[j]][route.vertexSequence[i]];
                dist2_add_v1 = route.costMatrix[route.vertexSequence[i]][proximo2];

                dist1_add_v2 = route.costMatrix[anterior1][route.vertexSequence[j]];
                dist2_add_v2 = route.costMatrix[route.vertexSequence[j]][route.vertexSequence[i]];

                impacto1 = dist1_add_v1 + dist2_add_v1 + dist1_add_v2 - dist1_remove_v1 - dist2_remove_v1 - dist2_remove_v2;
                impacto2 = 0;
            }
            else
            {
                dist1_remove_v2 = route.costMatrix[anterior2][route.vertexSequence[j]]; // arestas que serão removidas V2
                dist2_remove_v2 = route.costMatrix[route.vertexSequence[j]][proximo2];

                dist1_add_v1 = route.costMatrix[anterior2][route.vertexSequence[i]]; // arestas adicionadas v1
                dist2_add_v1 = route.costMatrix[route.vertexSequence[i]][proximo2];

                dist1_add_v2 = route.costMatrix[anterior1][route.vertexSequence[j]]; // arestas adicionadas V2
                dist2_add_v2 = route.costMatrix[route.vertexSequence[j]][proximo1];

                impacto1 = -dist1_remove_v1 - dist2_remove_v1 - vertice_parada_v1 + dist1_add_v2 + dist2_add_v2 + vertice_parada_v2;
                impacto2 = -dist1_remove_v2 - dist2_remove_v2 - vertice_parada_v2 + dist1_add_v1 + dist2_add_v1 + vertice_parada_v1;
            }
            if (route.cost + impacto1 + impacto2 > instance.maxTime)
            {
                continue; // custo ultrapassa o maxTime, nao tem como fazer o swap
            }

            if (impacto1 + impacto2 >= 0 || impacto1 + impacto2 >= best_swap)
            {
                continue; // nao melhorou a soluçao
            }
            int id = route.id;
            int vertice_i = route.vertexSequence[i];
            int vertice_j = route.vertexSequence[j];

            bool local_visita = false;
            double impacto1_if_equals = 0;
            map<double, int> aux_visited_vertice2 = visitedVertices[route.vertexSequence[j]];
            auto it = aux_visited_vertice2.find(route.arrivalTimes[j] + instance.protectionTime);
            if (it != aux_visited_vertice2.end())
            {
                aux_visited_vertice2.erase(it);
            }

            if (aux_visited_vertice2.empty())
            {
                local_visita = true;
            }
            else
            {
                it = aux_visited_vertice2.lower_bound(route.arrivalTimes[i - 1] + dist1_add_v2 + vertice_parada_v2 + instance.protectionTime);
                if (it == aux_visited_vertice2.end())
                {
                    auto it_prev = prev(it);
                    if (Utils::doubleLessOrEqual(it_prev->first, route.arrivalTimes[i - 1] + dist1_add_v2))
                    {
                        local_visita = true;
                    }
                    else
                    {
                        local_visita = false;
                    }
                }
                else if (it == aux_visited_vertice2.begin())
                {
                    if (it->second == route.id)
                    {
                        if (route.arrivalTimes[j] < it->first - instance.protectionTime)
                        {
                            impacto1_if_equals = impacto1 + impacto2;
                        }
                        else
                        {
                            impacto1_if_equals = impacto1; // corrigido
                        }
                    }

                    if (Utils::doubleLessOrEqual(route.arrivalTimes[i - 1] + dist1_add_v2 + vertice_parada_v2 + instance.protectionTime, it->first - instance.protectionTime + impacto1_if_equals))
                    {
                        local_visita = true;
                    }
                    else
                    {
                        local_visita = false;
                    }
                }
                else
                {
                    if (it->second == route.id)
                    {
                        if (route.arrivalTimes[j] < it->first - instance.protectionTime)
                        {
                            impacto1_if_equals = impacto1 + impacto2;
                        }
                        else
                        {
                            impacto1_if_equals = impacto1;
                        }
                    }
                    auto it_prev = prev(it);
                    if (Utils::doubleLessOrEqual(route.arrivalTimes[i - 1] + dist1_add_v2 + vertice_parada_v2 + instance.protectionTime, it->first - instance.protectionTime + impacto1_if_equals) && Utils::doubleLessOrEqual(it_prev->first, route.arrivalTimes[i - 1] + dist1_add_v2))
                    {
                        local_visita = true;
                    }
                }
            }
            if (!local_visita)
                // cout << "---- Erro" << endl;
                continue;

            double temp_visita_custo = (j == i + 1) ? route.arrivalTimes[i - 1] + dist1_add_v2 + vertice_parada_v2 + dist2_add_v2 : route.arrivalTimes[j - 1] + impacto1;
            local_visita = false;
            double impacto2_if_equals = 0;
            map<double, int> aux_visited_vertice1 = visitedVertices[route.vertexSequence[i]];
            it = aux_visited_vertice1.find(route.arrivalTimes[i] + instance.protectionTime);
            if (it != aux_visited_vertice1.end())
            {
                aux_visited_vertice1.erase(it);
            }
            if (aux_visited_vertice1.empty())
            {
                local_visita = true;
            }
            else
            {
                it = aux_visited_vertice1.lower_bound(temp_visita_custo + dist1_add_v1 + vertice_parada_v1 + instance.protectionTime);
                if (it == aux_visited_vertice1.end())
                {
                    auto it_prev = prev(it);
                    double aux_impacto = 0;
                    if (it_prev->second == route.id)
                    {
                        aux_impacto = impacto1;
                        impacto2_if_equals = impacto1 + impacto2;
                    }

                    if (Utils::doubleLessOrEqual(it_prev->first + aux_impacto, temp_visita_custo + dist1_add_v1 + impacto2_if_equals))
                    {
                        local_visita = true;
                    }
                    else
                    {
                        local_visita = false;
                    }
                }
                else if (it == aux_visited_vertice1.begin())
                {
                    if (it->second == route.id)
                        impacto2_if_equals = impacto1 + impacto2;

                    if (Utils::doubleLessOrEqual(temp_visita_custo + dist1_add_v1 + vertice_parada_v1 + instance.protectionTime, it->first - instance.protectionTime + impacto2_if_equals))
                    {
                        local_visita = true;
                    }
                    else
                    {
                        local_visita = false;
                    }
                }
                else
                {
                    if (it->second == route.id)
                        impacto2_if_equals = impacto1 + impacto2;
                    auto it_prev = prev(it);
                    double if_equals_aux = 0;
                    if (it_prev->first - instance.protectionTime > route.arrivalTimes[i])
                        if_equals_aux += impacto1;

                    if (Utils::doubleLessOrEqual(temp_visita_custo + dist1_add_v1 + vertice_parada_v1 + instance.protectionTime, it->first - instance.protectionTime + impacto2_if_equals) && Utils::doubleLessOrEqual(it_prev->first + if_equals_aux, temp_visita_custo + dist1_add_v1))
                    {
                        local_visita = true;
                    }
                }
            }
            if (!local_visita)
                continue;

            double if_prev_equals = 0, if_next_equals = 0;
            double impacto = impacto1;
            bool possibilidade_visita = true;
            for (int n = i + 1; n < route.vertexSequence.size() - 1; n++)
            {
                if (n == j)
                {
                    impacto += impacto2;
                    continue;
                }
                auto it = visitedVertices[route.vertexSequence[n]].find(route.arrivalTimes[n] + instance.protectionTime);
                auto it_next = std::next(it);
                auto it_prev = it != visitedVertices[route.vertexSequence[n]].begin() ? prev(it) : visitedVertices[route.vertexSequence[n]].end();
                if (route.id == it_next->second)
                {
                    // ocorrencia vai sofrer os dois impactos
                    if (route.arrivalTimes[j] <= it_next->first - instance.protectionTime)
                    {
                        if_next_equals = impacto1 + impacto2;
                    }
                    else if (route.arrivalTimes[i] <= it_next->first - instance.protectionTime)
                    { // ocorrencia vai sofrer somente o impacto1
                        if_next_equals = impacto1;
                    }
                }
                else
                {
                    if_next_equals = 0;
                }

                if (route.id == it_prev->second)
                {
                    if (route.arrivalTimes[j] <= it_prev->first - instance.protectionTime)
                    {
                        if_prev_equals = impacto1 + impacto2;
                    }
                    else if (route.arrivalTimes[i] <= it_prev->first - instance.protectionTime)
                    { // ocorrencia vai sofrer somente o impacto1
                        if_prev_equals = impacto1;
                    }
                }
                else
                {
                    if_prev_equals = 0;
                }
                // if_prev_equals = (rota.id == it_prev->second && rota.visita_custo[j] <= it_prev->first - grafo.protectionTime) ? impacto2 : 0;

                if (it_next != visitedVertices[route.vertexSequence[n]].end() && it->first + impacto > it_next->first - instance.protectionTime + if_next_equals)
                {
                    possibilidade_visita = false;
                    break;
                }

                if (it_prev != visitedVertices[route.vertexSequence[n]].end() && it->first + impacto - instance.protectionTime < it_prev->first + if_prev_equals)
                {
                    possibilidade_visita = false;
                    break;
                }
            }
            if (!possibilidade_visita)
            {
                continue;
            }

            if (possibilidade_visita)
            {
                //

                // exclui vertice i rota1
                swap_inter[0][0] = route.vertexSequence[i];
                swap_inter[0][1] = score_v1;
                swap_inter[0][2] = route.costMatrix[anterior1][proximo1] - dist1_remove_v1 - dist2_remove_v1 - vertice_parada_v1;
                swap_inter[0][3] = i;

                // exclui vertice j rota2
                swap_inter[1][0] = route.vertexSequence[j];
                swap_inter[1][1] = score_v2;
                swap_inter[1][2] = route.costMatrix[anterior2][proximo2] - dist1_remove_v2 - dist2_remove_v2 - vertice_parada_v2;
                swap_inter[1][3] = j;

                // adicionar vertice j na rota1
                swap_inter[2][0] = route.vertexSequence[j];                                                                 // id vertice
                swap_inter[2][1] = score_v2;                                                                                // score vertice
                swap_inter[2][2] = dist1_add_v2 + dist2_add_v2 + vertice_parada_v2 - route.costMatrix[anterior1][proximo1]; // impacto insert_v vertice
                swap_inter[2][3] = i;                                                                                       // Local insert rota
                swap_inter[2][4] = route.arrivalTimes[i - 1] + dist1_add_v2 + vertice_parada_v2;                            // Visita_custo
                swap_inter[2][5] = (vertice_parada_v2 == instance.stopTime) ? 1 : 0;

                // adicionar vertice i na rota2
                swap_inter[3][0] = route.vertexSequence[i];                                                                 // id vertice
                swap_inter[3][1] = score_v1;                                                                                // score vertice
                swap_inter[3][2] = dist1_add_v1 + dist2_add_v1 + vertice_parada_v1 - route.costMatrix[anterior2][proximo2]; // impacto insert_v vertice
                swap_inter[3][3] = j;                                                                                       // Local insert rota
                swap_inter[3][4] = route.arrivalTimes[j - 1] + dist1_add_v1 + vertice_parada_v1 + impacto1;                 // Visita_custo
                swap_inter[3][5] = (vertice_parada_v1 == instance.stopTime) ? 1 : 0;

                if (isBest)
                {
                    best_swap = impacto1 + impacto2;
                }
                else
                {
                    break;
                }
                // break;
            }
        }
        if (!isBest && swap_inter[0][0] != -1)
            break;
    }
    if (swap_inter[0][0] == -1)
    {
        return false;
    }
    else
    {
        // cout<< endl<<"ANTES: "<<endl;
        // // cout << "Rota: "<<rota.id<<" - Swap: "<<swap_inter[0][0]<< " | " << swap_inter[1][0]<<endl;
        // cout <<"Rota: " << rota.id << " - Vertice[" << swap_inter[0][3] << "] = " << rota.route[swap_inter[0][3]] << " |Swap| Vertice[" << swap_inter[1][3] << "] = " << rota.route[swap_inter[1][3]] <<endl;
        // cout << "Visita_custo[" << swap_inter[0][3] << "] = " << rota.visita_custo[swap_inter[0][3]] << " | Visita_custo[" << swap_inter[1][3] << "] = " << rota.visita_custo[swap_inter[1][3]] << endl;
        // cout << rota <<endl;
        // S.print_visited(12, 13);

        route.excludeVertex(swap_inter[0], visitedVertices, totalScore, totalCost);
        route.insertVertex(swap_inter[2], visitedVertices, totalScore, totalCost);

        route.excludeVertex(swap_inter[1], visitedVertices, totalScore, totalCost);
        route.insertVertex(swap_inter[3], visitedVertices, totalScore, totalCost);

        // cout << endl<<"DEPOIS: "<<endl;
        // // cout << "Rota: "<<rota.id<<" - Swap: "<<swap_inter[0][0]<< " | " << swap_inter[1][0]<<endl;
        // cout<<"Rota: " << rota.id << " - Vertice[" << swap_inter[0][3] << "] = " << rota.route[swap_inter[0][3]] << " |Swap| Vertice[" << swap_inter[1][3] << "] = " << rota.route[swap_inter[1][3]] <<endl;
        // cout << "Visita_custo[" << swap_inter[0][3] << "] = " << rota.visita_custo[swap_inter[0][3]] << " | Visita_custo[" << swap_inter[1][3] << "] = " << rota.visita_custo[swap_inter[1][3]] << endl;
        // cout << rota << endl;
        return true;
    }
}

void Solution::Constructive(Instance &instance, mt19937 &randomGenerator)
{
    priority_queue<Route> doneRoutes;

    while (!routes.empty())
    {
        Route route = routes.top();
        routes.pop();

        std::uniform_int_distribution<int> dis(1, 10);
        int randomChoice = dis(randomGenerator);
        route.additionalStopTime = (randomChoice <= 7) ? instance.stopTime : 0;

        vector<int> candidateVertices = Utils::createCandidateList(instance, route, visitedVertices);

        if (candidateVertices.empty())
        {
            route.cost += route.costMatrix[route.vertexSequence.back()][0];
            route.vertexSequence.push_back(0);
            route.isStopVertex.push_back(0);
            route.arrivalTimes.push_back(route.cost);
            route.timeWindows.push_back({999, 999});

            route.updateTimeWindows(visitedVertices);
            doneRoutes.push(route);
            totalCost += route.cost;
            totalScore += route.score;
            continue;
        }

        int selectedVertex;
        string caller;
        randomChoice = dis(randomGenerator);

        if (randomChoice >= 1 && randomChoice < 3)
        {
            shuffle(candidateVertices.begin(), candidateVertices.end(), randomGenerator);
            selectedVertex = candidateVertices[0];
            caller = "Construtivo - Random";
        }
        else if (randomChoice >= 3 && randomChoice < 5)
        {
            selectedVertex = Utils::findMinCost(candidateVertices, route.costMatrix, route.vertexSequence.back());
            caller = "Construtivo - Min_custo";
        }
        else if (randomChoice >= 5 && randomChoice < 7)
        {
            selectedVertex = Utils::findMaxScore(candidateVertices, instance.vertexScores);
            caller = "Construtivo - Max_scores";
        }
        else if (randomChoice >= 7 && randomChoice <= 10)
        {
            selectedVertex = Utils::findBestCostBenefit(candidateVertices, instance.vertexScores, route.costMatrix, route.vertexSequence.back());
            caller = "Construtivo - Cost_benefit";
        }

        double custoAdicional = route.costMatrix[route.vertexSequence.back()][selectedVertex] + route.additionalStopTime;
        route.cost += custoAdicional;

        double vertexScore = instance.vertexScores[selectedVertex];

        if (route.additionalStopTime == instance.stopTime)
        {
            route.score += vertexScore;
            route.isStopVertex.push_back(1);
        }
        else
        {
            vertexScore = vertexScore / 3;
            route.score += vertexScore;
            route.isStopVertex.push_back(0);
        }

        route.arrivalTimes.push_back(route.cost);
        route.vertexSequence.push_back(selectedVertex);
        route.timeWindows.push_back({999, 999});

        visitedVertices[selectedVertex][route.cost + instance.protectionTime] = route.id;

        routes.push(route);
        updateSolutionTimeWindows();
        checkSolution(instance, caller);
    }
    routes = doneRoutes;
}

void Solution::PerturbationWithStrength(Instance &instance, mt19937 &randomGenerator, double perturbationIntensity)
{
    std::string caller;
    bool bestImprovement = false;

    std::priority_queue<Route> auxRoutes;
    while (!routes.empty())
    {
        Route route = routes.top();
        routes.pop();
        int verticesToExclude = static_cast<int>(route.vertexSequence.size() * perturbationIntensity);

        std::vector<int> indexRoute;
        for (int i = 1; i < route.vertexSequence.size() - 1; i++)
            indexRoute.push_back(i);
        std::shuffle(indexRoute.begin(), indexRoute.end(), randomGenerator);

        int n = 0;
        int index;

        while (true)
        {
            if (verticesToExclude == 0 || n == indexRoute.size())
                break;

            index = indexRoute[n];
            std::vector<double> excludeVertex = Utils::tryExcludeVertex(instance, visitedVertices, route, index);
            if (excludeVertex[0] != -1)
            {
                route.excludeVertex(excludeVertex, visitedVertices, totalScore, totalCost);
                updateSolutionTimeWindows();

                caller = "Perturbação EXCLUIR";
                assert(checkSolution(instance, caller));

                indexRoute.pop_back();
                for (int j = 0; j < indexRoute.size(); j++)
                {
                    if (indexRoute[j] == route.vertexSequence.size() - 1)
                    {
                        indexRoute.erase(indexRoute.begin() + j);
                        break;
                    }
                }
                std::shuffle(indexRoute.begin(), indexRoute.end(), randomGenerator);
                verticesToExclude--;
                n = 0;
                continue;
            }
            if (route.outOfRouteSwap(instance.vertexScores, instance.numVertex, visitedVertices, totalScore, totalCost, index, index + 1, bestImprovement))
            {
                updateSolutionTimeWindows();

                caller = "Perturbação - Swap Out";
                assert(checkSolution(instance, caller));

                std::shuffle(indexRoute.begin(), indexRoute.end(), randomGenerator);
                verticesToExclude--;
                n = 0;
                continue;
            }
            n++;
        }
        auxRoutes.push(route);
    }
    routes = auxRoutes;

    return;
}

void Solution::localSearch(Instance &instance, mt19937 &randomGenerator)
{
    priority_queue<Route> doneRoutes;
    string caller;
    bool improvementFound;
    bool bestImprovement = true;

    // cout << "Iniciando busca local" << endl;

    vector<Operation> operations = {Operation::BestInsert, Operation::SwapInter, Operation::SwapIntra, Operation::SwapOut, Operation::Stop, Operation::Realocate};
    while (!routes.empty())
    {
        Route route = routes.top();
        routes.pop();
        improvementFound = false;

        // cout << "Rota: " << route.id << " Score: " << route.score << endl;

        shuffle(operations.begin(), operations.end(), randomGenerator);
        for (const auto &operation : operations)
        {
            if (operation == Operation::TwoOpt)
            {
                route.twoOpt();
            }
            else if (operation == Operation::Realocate)
            {
                // Count debug
                totalNeighborhoodOperations["realocate"] += 1;
                testedRoutes[route.id] += 1;
                // cout << "***Tentando realocate rota " << route.id << endl;
                if (route.realocate(instance.vertexScores, visitedVertices, totalScore, totalCost, bestImprovement))
                {
                    // cout << "---Realocate melhorou: " << totalScore << endl;
                    routes.push(route);
                    updateSolutionTimeWindows();
                    caller = "Busca Local - realocate";

                    assert(checkSolution(instance, caller));
                    improvementFound = true;

                    // Count debug
                    neighborhoodUsageCount["realocate"] += 1;
                    improvedRoutes[route.id] += 1;
                    break;
                }
            }
            else if (operation == Operation::BestInsert)
            {
                // Count debug
                totalNeighborhoodOperations["bestInsert"] += 1;
                testedRoutes[route.id] += 1;
                // cout << "***Tentando bestInsert rota " << route.id << endl;
                if (route.bestInsertNearestVertices(instance.vertexScores, instance.numVertex, visitedVertices, totalScore, totalCost, bestImprovement))
                {
                    // cout << "---BestInsert melhorou: " << totalScore << endl;
                    routes.push(route);
                    updateSolutionTimeWindows();

                    caller = "Busca Local - insert";
                    assert(checkSolution(instance, caller));
                    improvementFound = true;

                    // Count debug
                    neighborhoodUsageCount["bestInsert"] += 1;
                    improvedRoutes[route.id] += 1;
                    break;
                }
            }
            else if (operation == Operation::SwapIntra)
            {
                // Count debug
                totalNeighborhoodOperations["swapIntra"] += 1;
                testedRoutes[route.id] += 1;
                // cout << "***Tentando swapIntra rota " << route.id << endl;
                if (intraRouteSwap(instance, route, bestImprovement))
                // if (route.intraRouteSwap(instance, visitedVertices, totalScore, totalCost, bestImprovement))
                {
                    // cout << "---SwapIntra melhorou: " << totalScore << endl;
                    routes.push(route);
                    updateSolutionTimeWindows();

                    caller = "Busca Local - Swap Intra";
                    assert(checkSolution(instance, caller));
                    improvementFound = true;

                    // Count debug
                    neighborhoodUsageCount["swapIntra"] += 1;
                    improvedRoutes[route.id] += 1;
                    break;
                }
            }
            else if (operation == Operation::Stop)
            {
                // Count debug
                totalNeighborhoodOperations["stop"] += 1;
                testedRoutes[route.id] += 1;
                // cout << "***Tentando stop rota " << route.id << endl;
                if (route.tryMakeStop(instance.vertexScores, visitedVertices, totalScore, totalCost, bestImprovement))
                {
                    // cout << "---Stop melhorou: " << totalScore << endl;
                    routes.push(route);
                    updateSolutionTimeWindows();
                    caller = "Busca Local - Para";
                    assert(checkSolution(instance, caller));
                    improvementFound = true;

                    // Count debug
                    neighborhoodUsageCount["stop"] += 1;
                    improvedRoutes[route.id] += 1;
                    break;
                }
            }
            else if (operation == Operation::SwapOut)
            {
                // Count debug
                totalNeighborhoodOperations["swapOut"] += 1;
                testedRoutes[route.id] += 1;
                // cout << "***Tentando swapOut rota " << route.id << endl;
                if (route.outOfRouteSwap(instance.vertexScores, instance.numVertex, visitedVertices, totalScore, totalCost, 1, route.vertexSequence.size() - 1, bestImprovement))
                {
                    // cout << "---SwapOut melhorou: " << totalScore << endl;
                    routes.push(route);
                    updateSolutionTimeWindows();

                    caller = "Busca Local - Swap Out";
                    assert(checkSolution(instance, caller));
                    improvementFound = true;

                    // Count debug
                    neighborhoodUsageCount["swapOut"] += 1;
                    improvedRoutes[route.id] += 1;
                    break;
                }
            }
            else if (operation == Operation::SwapInter)
            {
                if (routes.size() > 0)
                {
                    Route route2 = routes.top();
                    routes.pop();

                    // Count debug
                    totalNeighborhoodOperations["swapInter"] += 1;
                    testedRoutes[route.id] += 1;
                    testedRoutes[route2.id] += 1;
                    // cout << "Tentando SwapInter entre rota " << route.id << " e rota " << route2.id << endl;
                    // cout << "Rota: " << route.id << " Velocidade: " << route.speed << endl;
                    // cout << "Rota: " << route2.id << " Velocidade: " << route2.speed << endl;
                    if (route.interRouteSwap(instance.vertexScores, visitedVertices, totalScore, totalCost, route2, 1, route.vertexSequence.size() - 1, bestImprovement))
                    {
                        // cout << "---SwapInter melhorou: " << totalScore << endl;
                        routes.push(route);
                        routes.push(route2);
                        updateSolutionTimeWindows();

                        caller = "Busca Local - Swap Inter";
                        assert(checkSolution(instance, caller));
                        improvementFound = true;

                        // Count debug
                        neighborhoodUsageCount["swapInter"] += 1;
                        improvedRoutes[route.id] += 1;
                        improvedRoutes[route2.id] += 1;
                        break;
                    }
                    routes.push(route2);
                }
            }
        }

        if (!improvementFound)
        {
            doneRoutes.push(route);
        }
    }
    routes = doneRoutes;

    // cout << "Busca local finalizada" << endl;
    return;
}

void Solution::printVisited(int startIndex, int endIndex) const{
    // cout << endl << "Visited vertices: " << endl;
    for (int i = startIndex; i < endIndex; i++) {
        // cout << "T: " << visitedVertices[i].size() << " Vertice " << i << ": [";
        for (const auto &visit : visitedVertices[i]){
            // cout << "(" << visit.first << ", " << visit.second << "), ";
        }
        // cout << "]" << endl;
    }
}

bool Solution::checkSequenceVertex(Instance &instance, string &caller)
{
    bool found = true;
    priority_queue<Route> routesCopy = routes;
    while (!routesCopy.empty()) {
        Route route = routesCopy.top();
        routesCopy.pop();
        for (int i = 1; i < route.vertexSequence.size() - 1; i++) {
            auto it = visitedVertices[route.vertexSequence[i]].find(route.arrivalTimes[i] + instance.protectionTime);
            if (it == visitedVertices[route.vertexSequence[i]].end()){
                // cout << caller << endl;
                auto itFind = visitedVertices[route.vertexSequence[i]].upper_bound(route.arrivalTimes[i] + instance.protectionTime);
                // cout << "Vertice["<<i<<"]: " << route.vertexSequence[i] << " nao encontrado na Rota: " << route.id << endl;
                // cout << "Visita_custo: [" << i << "]: " << route.arrivalTimes[i] + instance.protectionTime << " | Vertice[" << route.vertexSequence[i] << "]: Id = " << itFind->second << ", custo = " << itFind->first << endl;
                // cout << route << endl << endl;
                return false;
            }
        }
    }
    return found;
}

bool Solution::checkVisited(const Instance &instance, string &caller) const{
    bool check = true;
    for (int i = 1; i < instance.numVertex; i++)
    {
        if (visitedVertices[i].empty() || visitedVertices[i].size() == 1) continue;

        auto it1 = visitedVertices[i].begin();
        auto it2 = next(it1);

        while (it2 != visitedVertices[i].end()){
            if (it1->first > it2->first - instance.protectionTime){
                // cout << endl << "Visited - " << caller << ": Vertice " << i << " - Rota " << it1->second << endl;
                // cout << "Vertice: " << i << " Diferença: " << it2->first - it1->first << " | Ideal: " << instance.protectionTime << endl;
                // cout << "[" << distance(visitedVertices[i].begin(), it1) << "] Rota: " << it1->second << " - Custo: " << it1->first << endl;
                // cout << "[" << distance(visitedVertices[i].begin(), it2) << "] Rota: " << it2->second << " - Custo: " << it2->first << endl;
                printVisited(i, i+1);
                check = false;
            }
            ++it1;
            ++it2;
        }
    }
    return check;
}

bool Solution::checkScore(Instance &instance, string &caller)
{
    priority_queue<Route> routesCopy = routes;
    while (!routesCopy.empty()) {
        Route route = routesCopy.top();
        routesCopy.pop();
        double auxScore = 0;
        for (int i = 0; i < route.vertexSequence.size(); i++) {
            double s;
            if (route.isStopVertex[i] == 1) {
                s = instance.vertexScores[route.vertexSequence[i]];
            } else {
                s = instance.vertexScores[route.vertexSequence[i]] / 3;
            }
            auxScore += s;
        }
        if (!Utils::doubleEquals(auxScore, route.score)){
            // cout << caller << endl;
            // cout << "Rota " << route.id << " - Score incompativel | aux_score = " << auxScore << " - Score = " << route.score << endl;
            return false;
        }
    }
    return true;
}

bool Solution::checkCost(Instance &instance, string &caller){
    double totalCost = 0;
    priority_queue<Route> routesCopy = routes;
    while (!routesCopy.empty()) {
        Route route = routesCopy.top();
        routesCopy.pop();
        double auxCost = 0;
        for (int i = 1; i < route.vertexSequence.size(); i++) {
            auxCost += route.costMatrix[route.vertexSequence[i - 1]][route.vertexSequence[i]];
            if (route.isStopVertex[i] == 1) {
                auxCost += instance.stopTime;
            }
        }
        
        if(!Utils::doubleEquals(auxCost, route.cost)){
            // cout << caller<<endl;
            // cout<< "Rota " << route.id<< " - Custo incompativel | aux_custo = "<< auxCost << " - Custo = "<< route.cost<<endl;
            return false;
        }
        if(!Utils::doubleLessOrEqual(route.cost, instance.maxTime)){
            // cout << caller << endl;
            // cout << "Rota " << route.id << " Ultrapassou T_max | T_max = "<<instance.maxTime<< " Custo = " << route.cost << endl;
            return false;
        }
        totalCost += auxCost;
    }
    return true;
}
bool Solution::checkTimeWindows(Instance &instance, string &caller)
{
    priority_queue<Route> routesCopy = routes;
    while (!routesCopy.empty())
    {
        Route route = routesCopy.top();
        routesCopy.pop();
        for (int i = route.timeWindows.size() - 2; i >= 0; i--){
            if (i == 0){
                if(route.timeWindows[i].first != route.timeWindows[i + 1].first) return false;
                if(route.timeWindows[i].second != route.timeWindows[i + 1].second) return false;
                break;
            }
            double gapPull;
            double gapPush;

            auto it = visitedVertices[route.vertexSequence[i]].find(route.arrivalTimes[i] + instance.protectionTime);
            auto nextIt = next(it);
            auto prevIt = prev(it);
            gapPull = (it == visitedVertices[route.vertexSequence[i]].begin()) ? 999 : (it->first - instance.protectionTime) - prevIt->first;
            // gapPull = (it->first - grafo.protectionTime) - prev_it->first;
            gapPush = (nextIt == visitedVertices[route.vertexSequence[i]].end()) ? 999 : (nextIt->first - instance.protectionTime) - it->first;

            double minPull = (Utils::doubleLessOrEqual(route.timeWindows[i + 1].first, gapPull)) ? route.timeWindows[i + 1].first : gapPull;
            if (!Utils::doubleEquals(route.timeWindows[i].first, minPull)){
                // cout << caller << endl;
                return false;
            }

            double minPush = (Utils::doubleLessOrEqual(route.timeWindows[i + 1].second, gapPush)) ? route.timeWindows[i + 1].second : gapPush;
            if (!Utils::doubleEquals(route.timeWindows[i].second, minPush)){
                // cout << caller << endl;
                return false;
            }

            if (minPull < 0 || minPush < 0){
                // cout << endl
                // << "Push_rotas - " << caller << endl<<" Rota: " << route.id << " - Vertice: " << route.vertexSequence[i] << endl;
                // cout << "Folga_puxar: " << minPull << "|" << route.timeWindows[i].first << " - Folga_empurrar: " << minPush << "|" << route.timeWindows[i].second << endl;;
                printVisited(route.vertexSequence[i], route.vertexSequence[i] + 1);
                // cout<<route<<endl;
                return false;
            }
        
        }
    }
    return true;
}

bool Solution::checkArrivalTime(Instance &instance, string &caller){
    priority_queue<Route> routesCopy = routes;
    while (!routesCopy.empty()){
        Route route = routesCopy.top();
        routesCopy.pop();
        double auxArrivalTime = 0;
        for(int i = 1 ; i < route.vertexSequence.size()-1; i++){
            int plus_parada = (route.isStopVertex[i]) ? instance.stopTime : 0;
            auxArrivalTime+=route.costMatrix[route.vertexSequence[i-1]][route.vertexSequence[i]] + plus_parada;
            if(!Utils::doubleEquals(auxArrivalTime, route.arrivalTimes[i])){
                // cout << caller << endl;
                // cout << "Rota: " << route.id << " - Vertice[" << i << "] = " << route.vertexSequence[i]<<endl;
                // cout << "Visita_custo: "<<route.arrivalTimes[i] << " | Aux_visita_custo: "<<auxArrivalTime<<endl;
                return false;
            }
        }
    }
    return true;
}

bool Solution::checkSolution(Instance &instance, string &caller){
    assert(checkArrivalTime(instance, caller));
    assert(checkScore(instance, caller));
    assert(checkCost(instance, caller));
    
    assert(checkVisited(instance, caller));
    assert(checkSequenceVertex(instance, caller));
    assert(checkTimeWindows(instance, caller));
    return true;
}

void Solution::updateSolutionTimeWindows()
{
    priority_queue<Route> rotasCopy;
    while (!routes.empty())
    {
        Route route = routes.top();
        routes.pop();
        route.updateTimeWindows(visitedVertices);
        rotasCopy.push(route);
    }
    routes = rotasCopy;
}

void Solution::printSolution(Instance &instance)
{
    priority_queue<Route> routesCopy = routes;
    // cout << totalScore << endl;
    // cout << instance.numVehicles << endl;
    while (!routesCopy.empty()){
        Route route = routesCopy.top();
        routesCopy.pop();
        // cout << endl << "Viatura " << route.id << ":" << endl;
        for (int i = 0; i < route.vertexSequence.size(); i++){
            if (i == 0)
            {
                // cout << "Base," << route.arrivalTimes[i] << "," << route.isStopVertex[i] << endl;
                continue;
            }else if(i == route.vertexSequence.size()-1){
                // cout << "Base," << route.arrivalTimes[i] << "," << route.isStopVertex[i] << endl;
                continue;
            }
            // cout << route.vertexSequence[i] << "," << route.arrivalTimes[i] << "," << route.isStopVertex[i] << endl;
        }
    }
    // cout <<endl;
}

void Solution::printJson(Instance &instance){
    nlohmann::json result;
    result["totalIterations"] = instance.totalIterations;
    result["totalScore"] = totalScore;
    result["totalCost"] = totalCost;
    result["numVertex"] = instance.numVertex;
    result["maxTime"] = instance.maxTime;
    result["protectionTime"] = instance.protectionTime;
    result["stopTime"] = instance.stopTime;
    
    priority_queue<Route> auxRoutes = routes;
    while(!auxRoutes.empty()){
        Route route = auxRoutes.top();
        nlohmann::json routeJson;
        routeJson["score"] = route.score;
        routeJson["cost"] = route.cost;
        routeJson["speed"] = route.speed;
        routeJson["route"] = route.vertexSequence;
        routeJson["stop"] = route.isStopVertex;
        result["routes"].push_back(routeJson);
        auxRoutes.pop();
    }

    cout << setw(1) << result << endl;
}


bool Solution::operator<(const Solution &solution) const{
    return totalScore > solution.totalScore;
}

ostream &operator<<(ostream &os, const Solution &solution){
    priority_queue<Route> routesCopy = solution.routes;
    // os << "Score: " << solution.totalScore << ", Custo: " << solution.totalCost << endl;
    while (!routesCopy.empty()) {
        os << routesCopy.top() << endl;
        routesCopy.pop();
    }
    return os;
}
