#include "LocalSearch.h"
#include <iostream>
#include <limits>
#include <numeric>
#include <set>

using namespace std;

enum class Operation
{
    BestInsert,
    SwapInter,
    SwapIntra,
    SwapOut,
    Para,
    Realocate,
    TwoOpt
};

void LocalSearch::localSearch(Instance &instance, Solution &solution, mt19937 &randomGenerator)
{
    priority_queue<Route> doneRoutes;
    string caller;
    bool improvementFound;
    bool bestImprovement = true;

    vector<Operation> operations = {Operation::BestInsert, Operation::SwapInter, Operation::SwapIntra, Operation::SwapOut, Operation::Para, Operation::Realocate};
    while (!solution.routes.empty()){
        Route route = solution.routes.top(); 
        solution.routes.pop();
        improvementFound = false;

        shuffle(operations.begin(), operations.end(), randomGenerator);
        for (const auto &operation : operations){
            if (operation == Operation::TwoOpt){
                twoOpt(instance, solution, route, bestImprovement);
            }
            else if (operation == Operation::Realocate){
                // Count debug
                solution.cont_vizinhanca_total["realocate"] += 1;
                solution.testedRoutes[route.id] += 1;
                if (realocate(instance, solution, route, bestImprovement)){
                    solution.routes.push(route);
                    solution.updateSolutionTimeWindows();
                    caller = "Busca Local - realocate";

                    assert(solution.checkSolution(instance, caller));
                    improvementFound = true;

                    // Count debug
                    solution.cont_vizinhanca["realocate"] += 1;
                    solution.improvedRoutes[route.id] += 1;
                    break;
                }
            }
            else if (operation == Operation::BestInsert){
                // Count debug
                solution.cont_vizinhanca_total["best_insert"] += 1;
                solution.testedRoutes[route.id] += 1;
                if (bestInsert(instance, solution, route, bestImprovement)){
                    solution.routes.push(route);
                    solution.updateSolutionTimeWindows();

                    caller = "Busca Local - insert";
                    assert(solution.checkSolution(instance, caller));
                    improvementFound = true;

                    // Count debug
                    solution.cont_vizinhanca["best_insert"] += 1;
                    solution.improvedRoutes[route.id] += 1;
                    break;
                }
            }
            else if (operation == Operation::SwapIntra){
                // Count debug
                solution.cont_vizinhanca_total["swap_intra"] += 1;
                solution.testedRoutes[route.id] += 1;
                if (intraRouteSwap(instance, solution, route, bestImprovement)){
                    solution.routes.push(route);
                    solution.updateSolutionTimeWindows();

                    caller = "Busca Local - Swap Intra";
                    assert(solution.checkSolution(instance, caller));
                    improvementFound = true;

                    // Count debug
                    solution.cont_vizinhanca["swap_intra"] += 1;
                    solution.improvedRoutes[route.id] += 1;
                    break;
                }
            }
            else if (operation == Operation::Para){
                // Count debug
                solution.cont_vizinhanca_total["para"] += 1;
                solution.testedRoutes[route.id] += 1;
                if (tryMakeStop(instance, solution, route, bestImprovement)){
                    solution.routes.push(route);
                    solution.updateSolutionTimeWindows();
                    caller = "Busca Local - Para";
                    assert(solution.checkSolution(instance, caller));
                    improvementFound = true;

                    // Count debug
                    solution.cont_vizinhanca["para"] += 1;
                    solution.improvedRoutes[route.id] += 1;
                    break;
                }
            }
            else if (operation == Operation::SwapOut){
                // Count debug
                solution.cont_vizinhanca_total["swap_out"] += 1;
                solution.testedRoutes[route.id] += 1;
                if (outOfRouteSwap(instance, solution, route, 1, route.vertexSequence.size() - 1, bestImprovement)){
                    solution.routes.push(route);
                    solution.updateSolutionTimeWindows();

                    caller = "Busca Local - Swap Out";
                    assert(solution.checkSolution(instance, caller));
                    improvementFound = true;

                    // Count debug
                    solution.cont_vizinhanca["swap_out"] += 1;
                    solution.improvedRoutes[route.id] += 1;
                    break;
                }
            }
            else if (operation == Operation::SwapInter)
            {
                if (solution.routes.size() > 0){
                    Route route2 = solution.routes.top();
                    solution.routes.pop();

                    // Count debug
                    solution.cont_vizinhanca_total["swap_inter"] += 1;
                    solution.testedRoutes[route.id] += 1;
                    solution.testedRoutes[route2.id] += 1;
                    if (interRouteSwap(instance, solution, route, route2, 1, route.vertexSequence.size() - 1, bestImprovement)){
                        solution.routes.push(route);
                        solution.routes.push(route2);
                        solution.updateSolutionTimeWindows();

                        caller = "Busca Local - Swap Inter";
                        assert(solution.checkSolution(instance, caller));
                        improvementFound = true;

                        // Count debug
                        solution.cont_vizinhanca["swap_inter"] += 1;
                        solution.improvedRoutes[route.id] += 1;
                        solution.improvedRoutes[route2.id] += 1;
                        break;
                    }
                    solution.routes.push(route2);
                }
            }
        }

        if (!improvementFound)
        {
            doneRoutes.push(route);
        }
    }
    solution.routes = doneRoutes;

    return;
}

bool LocalSearch::twoOpt(const Instance &instance, Solution &solution, Route &route, bool &isBest)
{
    cout << "two_opt | Rota: " << route.id << endl;
    cout << "custo: " << route.cost << endl;
    cout << " antes rota: ";
    cout << "Veiculo " << route.id << " - Score: " << route.score << " - Custo: " << route.cost << " Rota: [";
    for (int v = 0; v < route.vertexSequence.size(); v++)
    {
        cout << "[" << v << "]:";
        if (route.isStopVertex[v] == true)
        {
            cout << "(*" << route.vertexSequence[v] << "*)"
                 << ", ";
        }
        else if (v + 1 == route.vertexSequence.size())
        {
            cout << "(" << route.vertexSequence[v] << ")";
        }
        else
        {
            cout << "(" << route.vertexSequence[v] << ")"
                 << ", ";
        }
    }
    cout << "]";
    cout << endl;
    double best_custo = route.cost;
    double best_impacto = 0;
    vector<double> best_impacto_vector;
    vector<int> new_route;
    vector<bool> new_paradas;
    int best_i, best_j;
    for (int i = 1; i < route.vertexSequence.size() - 2; i++)
    {
        for (int j = i + 3; j < route.vertexSequence.size() - 1; j++)
        {
            // Cálculo das distâncias removidas
            double dist_remove1 = route.costMatrix[route.vertexSequence[i]][route.vertexSequence[i + 1]];
            double dist_remove2 = route.costMatrix[route.vertexSequence[j]][route.vertexSequence[j + 1]];

            // Cálculo das novas distâncias
            double dist_add1 = route.costMatrix[route.vertexSequence[i]][route.vertexSequence[j]];
            double dist_add2 = route.costMatrix[route.vertexSequence[i + 1]][route.vertexSequence[j + 1]];
            vector<int> aux_rota = route.vertexSequence;
            vector<bool> aux_paradas = route.isStopVertex;
            vector<double> aux_impacto_vector;
            reverse(aux_rota.begin() + i + 1, aux_rota.begin() + j + 1);
            reverse(aux_paradas.begin() + i + 1, aux_paradas.begin() + j + 1);
            double dist_remove;
            double dist_add;
            double impacto = 0;
            // cout << "Verificando impacto" << endl;
            for (int k = i; k < j + 1; k++)
            {
                // cout << "k: " << k << " - [k]: " << rota.route[k] << " - [k+1]: " << rota.route[k+1] << " - paradas[k]: " << rota.paradas[k] << " - paradas[k+1]: " << rota.paradas[k+1] << endl;
                dist_remove = -route.costMatrix[route.vertexSequence[k]][route.vertexSequence[k + 1]] - route.isStopVertex[k + 1] * instance.stopTime;
                // cout << "dist_remove: " << dist_remove << endl;

                // cout << "k: " << k << " - [k]: " << aux_rota[k] << " - [k+1]: " << aux_rota[k + 1] << " - paradas[k]: " << aux_paradas[k] << " - paradas[k+1]: " << aux_paradas[k + 1] << endl;
                dist_add = route.costMatrix[aux_rota[k]][aux_rota[k + 1]] + aux_paradas[k + 1] * instance.stopTime;
                // cout << "dist_add: " << dist_add << endl;
                impacto += dist_add + dist_remove;
                aux_impacto_vector.push_back(impacto);
            }
            // cout << "impacto: " << impacto << endl;
            // cout << "aux_impacto: " << aux_impacto_vector.size() << endl;
            if (route.cost + impacto >= best_custo)
            {
                continue;
            }

            bool possibilidade_visita;
            possibilidade_visita = (Utils::doubleGreaterOrEqual(route.timeWindows[j + 1].first, impacto * -1));
            if (!possibilidade_visita)
            {
                continue;
            }

            if (possibilidade_visita)
            {
                best_custo = route.cost + impacto;
                best_i = i;
                best_j = j;
                new_route = aux_rota;
                new_paradas = aux_paradas;
                best_impacto = impacto;
                best_impacto_vector = aux_impacto_vector;
                break;
            }

            // i + 1=  j
            // i + 2 = j - 1
            // i + 3 = j - 2
            // i + 4 = j - 3
            // i + 5 = j - 4
            // i + 6 = j - 5
            // i + 7 = j - 6
            // i + 8 = j - 7
            // i + 9 = j - 8
        }
    }
    cout << "best_custo: " << best_custo << endl;
    cout << "best_i: " << best_i << " best_j: " << best_j << endl;
    cout << "depois rota: ";
    cout << "impacto: " << best_impacto << endl;
    cout << "best_impacto_vector: ";
    for (int i = 0; i < best_impacto_vector.size(); i++)
    {
        cout << best_impacto_vector[i] << " ";
    }
    cout << endl;
    cout << "Veiculo " << route.id << " - Score: " << route.score << " - Custo: " << best_custo << " Rota: [";
    for (int v = 0; v < new_route.size(); v++)
    {
        cout << "[" << v << "]:";
        if (new_paradas[v] == true)
        {
            cout << "(*" << new_route[v] << "*)"
                 << ", ";
        }
        else if (v + 1 == new_route.size())
        {
            cout << "(" << new_route[v] << ")";
        }
        else
        {
            cout << "(" << new_route[v] << ")"
                 << ", ";
        }
    }
    cout << "]";
    cout << endl;
    cout << endl
         << "_________________________" << endl;
    route.vertexSequence = new_route;
    route.cost = best_custo;
    return true;
}

bool LocalSearch::realocate(const Instance &instance, Solution &solution, Route &route, bool &isBest)
{
    // cout << "Iniciando realocate" << endl;
    vector<vector<double>> relocData(2, vector<double>(6, -1));
    double vertexStopTime;
    double bestRelocateImpact = 1;

    double dist1RemovePrevToActual;
    double dist2RemoveActualToNext;
    double dist3AddPrevToNext;

    double dist1AddPrevToActual;
    double dist2AddActualToNext;
    double dist3RemovePrevToNext;

    double removalImpact = 0;
    double insertionImpact = 0;

    double vertexScore;
    for (int i = 1; i < route.vertexSequence.size() - 2; i++)
    {
        int actualVertex = route.vertexSequence[i];
        int previousVertex = route.vertexSequence[i - 1];
        int nextVertex = route.vertexSequence[i + 1];

        dist1RemovePrevToActual = route.costMatrix[previousVertex][actualVertex];
        dist2RemoveActualToNext = route.costMatrix[actualVertex][nextVertex];
        dist3AddPrevToNext = route.costMatrix[previousVertex][nextVertex];

        vertexStopTime = (route.isStopVertex[i] == 1) ? instance.stopTime : 0;
        removalImpact = -dist1RemovePrevToActual - dist2RemoveActualToNext - vertexStopTime + dist3AddPrevToNext;
        vertexScore = (route.isStopVertex[i] == 1) ? instance.vertexScores[actualVertex] : instance.vertexScores[actualVertex] / 3;

        for (int j = 0; j < route.vertexSequence.size() - 2; j++)
        {
            if (j == i || j == i - 1)
                continue;
            int previousVertex2 = route.vertexSequence[j];
            int nextVertex2 = route.vertexSequence[j + 1];
            if (previousVertex2 == actualVertex || nextVertex2 == actualVertex)
                continue;
            dist1AddPrevToActual = route.costMatrix[previousVertex2][actualVertex];
            dist2AddActualToNext = route.costMatrix[actualVertex][nextVertex2];
            dist3RemovePrevToNext = route.costMatrix[previousVertex2][nextVertex2];
            insertionImpact = dist1AddPrevToActual + dist2AddActualToNext + vertexStopTime - dist3RemovePrevToNext;

            if (route.cost + removalImpact + insertionImpact > instance.maxTime || removalImpact + insertionImpact >= 0 || removalImpact + insertionImpact >= bestRelocateImpact)
            {
                continue;
            }

            bool localVisitation = false;     // iniciando como se ele nao pudesse vizitar
            double impacto1_if_equals = 0; // caso seja igual, deve ser considerado esse impacto
            double impacto1_i_minus_j = 0;
            map<double, int> auxVisitedVertice1 = solution.visitedVertices[actualVertex];

            auto it = auxVisitedVertice1.find(route.arrivalTimes[i] + instance.protectionTime);
            // cout << "  Find realizado" << endl;
            if (it != auxVisitedVertice1.end())
            {
                auxVisitedVertice1.erase(it);
            }

            if (auxVisitedVertice1.empty())
            {
                localVisitation = true;
            }
            else
            {

                if (i < j)
                {
                    impacto1_i_minus_j = removalImpact;
                }
                it = auxVisitedVertice1.lower_bound(route.arrivalTimes[j] + dist1AddPrevToActual + instance.protectionTime);
                if (it == auxVisitedVertice1.end())
                {
                    // caso nao tenha vizinhos para frente, verificar se a visita anterior é possivel

                    auto it_prev = prev(it);
                    if (it_prev->second == route.id)
                    {
                        if (route.arrivalTimes[i] < it_prev->first - instance.protectionTime)
                        {
                            impacto1_if_equals = removalImpact;
                        }
                    }

                    if (Utils::doubleLessOrEqual(it_prev->first + impacto1_if_equals, route.arrivalTimes[j] + impacto1_i_minus_j + dist1AddPrevToActual))
                    {
                        localVisitation = true;
                    }
                    else
                    {
                        localVisitation = false;
                    }
                }
                else if (it == auxVisitedVertice1.begin())
                {
                    // caso nao tenha vizinhos para tras, verificar se a visita para frente é possivel
                    if (it->second == route.id)
                    {
                        // caso a visita analisada seja a mesma rota, verificar se a visita para frente é possivel
                        if (route.arrivalTimes[j] < it->first - instance.protectionTime && route.arrivalTimes[i] < it->first - instance.protectionTime)
                        {
                            impacto1_if_equals = removalImpact + insertionImpact;
                        }
                        else if (route.arrivalTimes[j] < it->first - instance.protectionTime)
                        {
                            impacto1_if_equals = insertionImpact; 
                        }
                    }

                    if (Utils::doubleLessOrEqual(route.arrivalTimes[j] + impacto1_i_minus_j + dist1AddPrevToActual + vertexStopTime + instance.protectionTime, it->first - instance.protectionTime + impacto1_if_equals))
                    {
                        localVisitation = true;
                    }
                    else
                    {
                        localVisitation = false;
                    }
                }
                else{
                    if (it->second == route.id)
                    {
                        if (route.arrivalTimes[j] < it->first - instance.protectionTime && route.arrivalTimes[i] < it->first - instance.protectionTime)
                        {
                            impacto1_if_equals = removalImpact + insertionImpact;
                        }
                        else if (route.arrivalTimes[j] < it->first - instance.protectionTime)
                        {
                            impacto1_if_equals = insertionImpact; // corrigido
                        }
                    }
                    auto it_prev = prev(it);
                    double impacto2_if_equals = 0;
                    if (it_prev->second == route.id)
                    {
                        if (route.arrivalTimes[i] < it_prev->first - instance.protectionTime)
                        {
                            impacto2_if_equals = removalImpact;
                        }
                    }
                    if (Utils::doubleLessOrEqual(route.arrivalTimes[j] + impacto1_i_minus_j + dist1AddPrevToActual + vertexStopTime + instance.protectionTime, it->first - instance.protectionTime + impacto1_if_equals) && Utils::doubleLessOrEqual(it_prev->first + impacto2_if_equals, route.arrivalTimes[j] + impacto1_i_minus_j + dist1AddPrevToActual))
                    {
                        localVisitation = true;
                    }
                }
            }
            if (!localVisitation)
                continue;

            double cumulativeImpact = 0;
            bool possibilityVisitation = true;
            for (int n = (i < j) ? i + 1 : j + 1; n < route.vertexSequence.size() - 1; n++)
            {
                double if_prev_equals = 0, if_next_equals = 0;

                if (n == i + 1)
                {
                    cumulativeImpact += removalImpact;
                }
                else if (n == j + 1)
                {
                    cumulativeImpact += insertionImpact;
                }

                auto it = solution.visitedVertices[route.vertexSequence[n]].find(route.arrivalTimes[n] + instance.protectionTime);

                if (it == solution.visitedVertices[route.vertexSequence[n]].end()){
                    possibilityVisitation = false;
                    break;
                }

                auto it_next = std::next(it);
                auto it_prev = it != solution.visitedVertices[route.vertexSequence[n]].begin() ? prev(it) : solution.visitedVertices[route.vertexSequence[n]].end();

                if (it_next != solution.visitedVertices[route.vertexSequence[n]].end())
                {
                    if (route.id == it_next->second)
                    {
                        if_next_equals += insertionImpact; // corrigido
                    }
                    if (route.arrivalTimes[i] < it_next->first - instance.protectionTime)
                    {
                        if_next_equals += removalImpact;
                    }
                }
                else
                {
                    if_next_equals = 0;
                }

                if (route.id == it_prev->second)
                {
                    if (route.arrivalTimes[j] < it_prev->first - instance.protectionTime)
                    {
                        if_prev_equals += insertionImpact; 
                    }
                    if (route.arrivalTimes[i] < it_prev->first - instance.protectionTime)
                    {
                        if_prev_equals += removalImpact;
                    }
                }
                else{
                    if_prev_equals = 0;
                }

                if (it_next != solution.visitedVertices[route.vertexSequence[n]].end() && it->first + cumulativeImpact > it_next->first - instance.protectionTime + if_next_equals){
                    possibilityVisitation = false;
                    break;
                }

                if (it_prev != solution.visitedVertices[route.vertexSequence[n]].end() && it->first + cumulativeImpact - instance.protectionTime < it_prev->first + if_prev_equals){
                    possibilityVisitation = false;
                    break;
                }
            }

            if (!possibilityVisitation)
            {
                continue;
            }

            if (possibilityVisitation){
                relocData[0][0] = actualVertex;
                relocData[0][1] = vertexScore;
                relocData[0][2] = dist3AddPrevToNext - dist1RemovePrevToActual - dist2RemoveActualToNext - vertexStopTime;
                relocData[0][3] = (j < i) ? i + 1 : i;

                // adicionar vertice i na rota
                relocData[1][0] = actualVertex;                                                 // id vertice
                relocData[1][1] = vertexScore;                                                 // score vertice
                relocData[1][2] = dist1AddPrevToActual + dist2AddActualToNext + vertexStopTime - dist3RemovePrevToNext;   // impacto insert_v vertice
                relocData[1][3] = j + 1;                                                   // Local insert rota
                relocData[1][4] = route.arrivalTimes[j] + dist1AddPrevToActual + vertexStopTime;        // arrivalTimes
                relocData[1][5] = (vertexStopTime == instance.stopTime) ? 1 : 0;

                if (isBest){
                    bestRelocateImpact = removalImpact + insertionImpact;
                }
                else{
                    break;
                }
            }
        }
        if (!isBest && relocData[0][0] != -1)
            break;
    }

    if (relocData[0][0] == -1){
        return false;
    }
    else{
        route.insertVertex(relocData[1], solution.visitedVertices, solution.totalScore, solution.totalCost);
        route.excludeVertex(relocData[0], solution.visitedVertices, solution.totalScore, solution.totalCost);
        return true;
    }
}

bool LocalSearch::tryMakeStop(const Instance &instance, Solution &solution, Route &route, bool &isBest)
{
    // O melhor vertice marcado como passada q for possivel ser parada, sera uma parada

    vector<double> vertice_para = {-1, -1, -1};
    if (route.cost + instance.stopTime > instance.maxTime)
    {
        return false;
    }

    for (int i = 1; i < route.vertexSequence.size() - 1; i++)
    {
        if (route.isStopVertex[i] == 1)
            continue;

        if (instance.vertexScores[route.vertexSequence[i]] < vertice_para[1])
            continue;

        bool possibilidade_parada = (Utils::doubleGreaterOrEqual(route.timeWindows[i].second, instance.stopTime));

        if (possibilidade_parada)
        {
            vertice_para = {static_cast<double>(i), instance.vertexScores[route.vertexSequence[i]] / 3, instance.vertexScores[route.vertexSequence[i]]};

            if (!isBest)
            {
                break;
            }
        }
    }

    if (vertice_para[0] == -1)
    {
        return false;
    }
    else
    {
        // std::cout << "Rota " << rota.id << " - Vertice[" << vertice_para[0] << "] = " << rota.route[vertice_para[0]] << " Score passada: " << vertice_para[1] << " | Score parada: "<<vertice_para[2]<<std::endl;

        route.makeStop(vertice_para, solution.visitedVertices, solution.totalScore, solution.totalCost);
        return true;
    }
}

bool LocalSearch::bestInsert(const Instance &instance, Solution &solution, Route &route, bool &isBest)
{
    //                 id, score, custo, local insert, custo local insert, local visita
    std::vector<double> b_vert_insert = {-1, -1, -1, -1, -1, -1};

    double dist1 = 0;
    double dist2 = 0;
    double dist3 = 0;

    for (int n = 0; n < 2; n++)
    {
        route.additionalStopTime = (n == 1) ? 0 : instance.stopTime;

        for (int v = 1; v < instance.numVertices; v++)
        {
            if (instance.vertexScores[v] < b_vert_insert[1])
                continue;

            double score_v = (route.additionalStopTime == instance.stopTime) ? instance.vertexScores[v] : instance.vertexScores[v] / 3;

            for (int j = 0; j < route.vertexSequence.size() - 1; j++)
            {
                int anterior = route.vertexSequence[j];
                int proximo = route.vertexSequence[j + 1];

                if (anterior == v || proximo == v)
                {
                    continue;
                }

                dist1 = route.costMatrix[anterior][v];
                dist2 = route.costMatrix[v][proximo];
                dist3 = route.costMatrix[anterior][proximo];
                double impacto = dist1 + dist2 + route.additionalStopTime - dist3;

                if (route.cost + impacto > instance.maxTime)
                    continue;

                if (b_vert_insert[1] == score_v && b_vert_insert[2] < impacto)
                {
                    continue;
                }

                bool local_visita = false;
                if (solution.visitedVertices[v].empty())
                {
                    local_visita = true;
                }
                else
                {
                    auto it = solution.visitedVertices[v].lower_bound(
                        route.arrivalTimes[j] + dist1 + route.additionalStopTime + instance.protectionTime);
                    if (it == solution.visitedVertices[v].end())
                    {
                        auto it_prev = prev(it);
                        if (Utils::doubleLessOrEqual(it_prev->first, route.arrivalTimes[j] + dist1))
                        {
                            local_visita = true;
                        }
                        else
                        {
                            local_visita = false;
                        }
                    }
                    else if (it == solution.visitedVertices[v].begin())
                    {
                        if (Utils::doubleLessOrEqual(route.arrivalTimes[j] + dist1 + route.additionalStopTime + instance.protectionTime, it->first - instance.protectionTime))
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
                        auto it_prev = prev(it);
                        if (Utils::doubleLessOrEqual(route.arrivalTimes[j] + dist1 + route.additionalStopTime + instance.protectionTime, it->first - instance.protectionTime) &&
                            Utils::doubleLessOrEqual(
                                it_prev->first, route.arrivalTimes[j] + dist1))
                        {
                            local_visita = true;
                        }
                    }
                }

                if (!local_visita)
                    continue;

                bool possibilidade_visita;
                if (impacto < 0)
                {
                    possibilidade_visita = (Utils::doubleGreaterOrEqual(route.timeWindows[j + 1].first, impacto * -1));
                }
                else
                {
                    possibilidade_visita = (Utils::doubleGreaterOrEqual(route.timeWindows[j + 1].second, impacto));
                }

                if (possibilidade_visita)
                {
                    b_vert_insert[0] = v;                                               // id vertice
                    b_vert_insert[1] = score_v;                                         // score vertice
                    b_vert_insert[2] = impacto;                                         // impacto insert_v vertice
                    b_vert_insert[3] = j + 1;                                           // Local insert rota
                    b_vert_insert[4] = route.arrivalTimes[j] + dist1 + route.additionalStopTime; // Visita_custo
                    b_vert_insert[5] = (route.additionalStopTime == instance.stopTime) ? 1 : 0;

                    if (!isBest)
                    {
                        break;
                    }
                }
            }
            if (!isBest && b_vert_insert[0] != -1)
                break;
        }
        if (!isBest && b_vert_insert[0] != -1)
            break;
    }

    // realizar a mudança aqui dentro
    if (b_vert_insert[0] == -1)
    {
        return false;
    }
    else
    {
        // std::cout << "Rota " << rota.id << " - Best_insert: Vertice["<<b_vert_insert[3]<<"] = " << b_vert_insert[0] << " - Score: " << b_vert_insert[1] << " - Impacto: " << b_vert_insert[2] << std::endl;

        route.insertVertex(b_vert_insert, solution.visitedVertices, solution.totalScore, solution.totalCost);
        return true;
    }
}

bool LocalSearch::intraRouteSwap(const Instance &instance, Solution &solution, Route &route, bool &isBest)
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
            map<double, int> aux_visited_vertice2 = solution.visitedVertices[route.vertexSequence[j]];
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
            map<double, int> aux_visited_vertice1 = solution.visitedVertices[route.vertexSequence[i]];
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
                auto it = solution.visitedVertices[route.vertexSequence[n]].find(route.arrivalTimes[n] + instance.protectionTime);
                auto it_next = std::next(it);
                auto it_prev = it != solution.visitedVertices[route.vertexSequence[n]].begin() ? prev(it) : solution.visitedVertices[route.vertexSequence[n]].end();
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

                if (it_next != solution.visitedVertices[route.vertexSequence[n]].end() && it->first + impacto > it_next->first - instance.protectionTime + if_next_equals)
                {
                    possibilidade_visita = false;
                    break;
                }

                if (it_prev != solution.visitedVertices[route.vertexSequence[n]].end() && it->first + impacto - instance.protectionTime < it_prev->first + if_prev_equals)
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
                swap_inter[2][0] = route.vertexSequence[j];                                                                                // id vertice
                swap_inter[2][1] = score_v2;                                                                                     // score vertice
                swap_inter[2][2] = dist1_add_v2 + dist2_add_v2 + vertice_parada_v2 - route.costMatrix[anterior1][proximo1]; // impacto insert_v vertice
                swap_inter[2][3] = i;                                                                                            // Local insert rota
                swap_inter[2][4] = route.arrivalTimes[i - 1] + dist1_add_v2 + vertice_parada_v2;                                  // Visita_custo
                swap_inter[2][5] = (vertice_parada_v2 == instance.stopTime) ? 1 : 0;

                // adicionar vertice i na rota2
                swap_inter[3][0] = route.vertexSequence[i];                                                                                // id vertice
                swap_inter[3][1] = score_v1;                                                                                     // score vertice
                swap_inter[3][2] = dist1_add_v1 + dist2_add_v1 + vertice_parada_v1 - route.costMatrix[anterior2][proximo2]; // impacto insert_v vertice
                swap_inter[3][3] = j;                                                                                            // Local insert rota
                swap_inter[3][4] = route.arrivalTimes[j - 1] + dist1_add_v1 + vertice_parada_v1 + impacto1;                       // Visita_custo
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

        route.excludeVertex(swap_inter[0], solution.visitedVertices, solution.totalScore, solution.totalCost);
        route.insertVertex(swap_inter[2], solution.visitedVertices, solution.totalScore, solution.totalCost);

        route.excludeVertex(swap_inter[1], solution.visitedVertices, solution.totalScore, solution.totalCost);
        route.insertVertex(swap_inter[3], solution.visitedVertices, solution.totalScore, solution.totalCost);

        // cout << endl<<"DEPOIS: "<<endl;
        // // cout << "Rota: "<<rota.id<<" - Swap: "<<swap_inter[0][0]<< " | " << swap_inter[1][0]<<endl;
        // cout<<"Rota: " << rota.id << " - Vertice[" << swap_inter[0][3] << "] = " << rota.route[swap_inter[0][3]] << " |Swap| Vertice[" << swap_inter[1][3] << "] = " << rota.route[swap_inter[1][3]] <<endl;
        // cout << "Visita_custo[" << swap_inter[0][3] << "] = " << rota.visita_custo[swap_inter[0][3]] << " | Visita_custo[" << swap_inter[1][3] << "] = " << rota.visita_custo[swap_inter[1][3]] << endl;
        // cout << rota << endl;
        return true;
    }
}

bool LocalSearch::interRouteSwap(const Instance &instance, Solution &solution, Route &route1, Route &route2, int i_inicial, int i_final, bool &best)
{
    std::vector<std::vector<double>> swap_intra(4, std::vector<double>(6, -1));
    int best_swap = -1;

    double score_r1;
    double score_r2;

    double dist1_remove_r1;
    double dist2_remove_r1;
    double dist3_remove_r2;
    double dist4_remove_r2;

    double dist1_add_r1;
    double dist2_add_r1;
    double dist3_add_r2;
    double dist4_add_r2;

    int vertice_parada_r1;
    int vertice_parada_r2;
    int anterior1;
    int proximo1;
    int anterior2;
    int proximo2;

    double impacto1;
    double impacto2;

    for (int i = i_inicial; i < i_final; i++)
    {
        anterior1 = route1.vertexSequence[i - 1];
        proximo1 = route1.vertexSequence[i + 1];
        score_r1 = (route1.isStopVertex[i] == 1) ? instance.vertexScores[route1.vertexSequence[i]] : instance.vertexScores[route1.vertexSequence[i]] / 3;

        dist1_remove_r1 = route1.costMatrix[anterior1][route1.vertexSequence[i]]; // arestas que serão removidas da rota 1
        dist2_remove_r1 = route1.costMatrix[route1.vertexSequence[i]][proximo1];
        vertice_parada_r1 = (route1.isStopVertex[i] == 1) ? instance.stopTime : 0; // se tiver parada, plus de 15 minutos
        // cout << "**** Rota: "<<rota1.id<<" - Vertice[" << i << "] = "<< rota1.route[i] <<" | "<< i <<" de " << rota1.route.size()-1<<" ****"<<endl;
        for (int j = 1; j < route2.vertexSequence.size() - 1; j++)
        {
            anterior2 = route2.vertexSequence[j - 1];
            proximo2 = route2.vertexSequence[j + 1];
            score_r2 = (route2.isStopVertex[j] == 1) ? instance.vertexScores[route2.vertexSequence[j]] : instance.vertexScores[route2.vertexSequence[j]] / 3;
            // cout << "Rota: " << rota2.id << " - Vertice[" << j << "] = " << rota2.route[j] << " | " << j << " de " << rota2.route.size()-1 << endl;

            if (anterior1 == route2.vertexSequence[j] || proximo1 == route2.vertexSequence[j] || anterior2 == route1.vertexSequence[i] || proximo2 == route1.vertexSequence[i])
            {
                // cout << "Vertice repetido" << endl;
                continue; // nao pode ter o mesmo vertice em seguida
            }

            vertice_parada_r2 = (route2.isStopVertex[j] == 1) ? instance.stopTime : 0; // se tiver parada, plus de 15 minutos

            dist3_remove_r2 = route2.costMatrix[anterior2][route2.vertexSequence[j]]; // arestas que serão removidas da rota 2
            dist4_remove_r2 = route2.costMatrix[route2.vertexSequence[j]][proximo2];

            dist1_add_r1 = route1.costMatrix[anterior1][route2.vertexSequence[j]]; // arestas adicionadas na rota1
            dist2_add_r1 = route1.costMatrix[route2.vertexSequence[j]][proximo1];

            dist3_add_r2 = route2.costMatrix[anterior2][route1.vertexSequence[i]]; // arestas adicionadas na rota2
            dist4_add_r2 = route2.costMatrix[route1.vertexSequence[i]][proximo2];

            impacto1 = dist1_add_r1 + dist2_add_r1 + vertice_parada_r2 - dist1_remove_r1 - dist2_remove_r1 - vertice_parada_r1;
            impacto2 = dist3_add_r2 + dist4_add_r2 + vertice_parada_r1 - dist3_remove_r2 - dist4_remove_r2 - vertice_parada_r2;

            if (route1.cost + impacto1 > instance.maxTime || route2.cost + impacto2 > instance.maxTime)
            {
                // cout << "Ultrapassa T_MAX" << endl;
                continue; // custo ultrapassa o T_max, nao tem como fazer o swap
            }

            if (impacto1 + impacto2 > 0 || impacto1 + impacto2 > best_swap)
            {
                // cout << "Aumentou o custo, piorou a solução" << endl;
                continue; // aumentou o custo, piorou a solução
            }

            // se ha espaço para visita do vertice J na rota1
            // cout << "Se é possivel inserir a o vertice 'J' na rota1" << endl;
            bool local_visita = false;
            double impacto1_if_equals = 0;
            map<double, int> aux_visited_vertice_j = solution.visitedVertices[route2.vertexSequence[j]];
            auto it = aux_visited_vertice_j.find(route2.arrivalTimes[j] + instance.protectionTime);
            if (it != aux_visited_vertice_j.end())
            {
                aux_visited_vertice_j.erase(it);
            }
            if (aux_visited_vertice_j.empty())
            {
                local_visita = true;
            }
            else
            {
                it = aux_visited_vertice_j.lower_bound(route1.arrivalTimes[i - 1] + dist1_add_r1 + vertice_parada_r2 + instance.protectionTime);
                if (it == aux_visited_vertice_j.end())
                {
                    auto it_prev = prev(it);
                    if (it_prev->second == route1.id && it_prev->first - instance.protectionTime > route1.arrivalTimes[i])
                        impacto1_if_equals += impacto1;
                    if (it_prev->second == route2.id && it_prev->first - instance.protectionTime > route2.arrivalTimes[j])
                        impacto1_if_equals += impacto2;
                    if (Utils::doubleLessOrEqual(it_prev->first + impacto1_if_equals, route1.arrivalTimes[i - 1] + dist1_add_r1))
                    {

                        local_visita = true;
                    }
                    else
                    {
                        local_visita = false;
                    }
                }
                else if (it == aux_visited_vertice_j.begin())
                {
                    if (it->second == route1.id && it->first - instance.protectionTime > route1.arrivalTimes[i])
                        impacto1_if_equals += impacto1;
                    if (it->second == route2.id && it->first - instance.protectionTime > route2.arrivalTimes[j])
                        impacto1_if_equals += impacto2;
                    if (Utils::doubleLessOrEqual(route1.arrivalTimes[i - 1] + dist1_add_r1 + vertice_parada_r2 + instance.protectionTime, it->first - instance.protectionTime + impacto1_if_equals))
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
                    if (it->second == route1.id && it->first - instance.protectionTime > route1.arrivalTimes[i])
                        impacto1_if_equals += impacto1;
                    if (it->second == route2.id && it->first - instance.protectionTime > route2.arrivalTimes[j])
                        impacto1_if_equals += impacto2;
                    auto it_prev = prev(it);
                    if (Utils::doubleLessOrEqual(route1.arrivalTimes[i - 1] + dist1_add_r1 + vertice_parada_r2 + instance.protectionTime, it->first - instance.protectionTime + impacto1_if_equals) && Utils::doubleLessOrEqual(it_prev->first, route1.arrivalTimes[i - 1] + dist1_add_r1))
                    {
                        local_visita = true;
                    }
                }
            }
            if (!local_visita)
                // cout << "---- Erro" << endl;
                continue;

            // se ha espaço para visita do vertice I na rota2
            // cout << "Se é possivel inserir a o vertice 'I' na rota2" << endl;
            local_visita = false;
            double impacto2_if_equals = 0;
            map<double, int> aux_visited_vertice_i = solution.visitedVertices[route1.vertexSequence[i]];
            it = aux_visited_vertice_i.find(route1.arrivalTimes[i] + instance.protectionTime);
            if (it != aux_visited_vertice_i.end())
            {
                aux_visited_vertice_i.erase(it);
            }
            else
            {
                cout << "ERROOOO" << endl;
            }
            if (aux_visited_vertice_i.empty())
            {
                local_visita = true;
            }
            else
            {
                it = aux_visited_vertice_i.lower_bound(route2.arrivalTimes[j - 1] + dist3_add_r2 + vertice_parada_r1 + instance.protectionTime);
                if (it == aux_visited_vertice_i.end())
                {
                    auto it_prev = prev(it);
                    if (it_prev->second == route1.id && it_prev->first - instance.protectionTime > route1.arrivalTimes[i])
                        impacto2_if_equals = impacto1;
                    if (Utils::doubleLessOrEqual(it_prev->first + impacto2_if_equals, route2.arrivalTimes[j - 1] + dist3_add_r2))
                    {
                        local_visita = true;
                    }
                    else
                    {
                        local_visita = false;
                    }
                }
                else if (it == aux_visited_vertice_i.begin())
                {
                    if (it->second == route1.id && it->first - instance.protectionTime > route1.arrivalTimes[i])
                        impacto2_if_equals += impacto1;
                    if (it->second == route2.id && it->first - instance.protectionTime > route2.arrivalTimes[j])
                        impacto2_if_equals += impacto2;
                    if (Utils::doubleLessOrEqual(route2.arrivalTimes[j - 1] + dist3_add_r2 + vertice_parada_r1 + instance.protectionTime, it->first - instance.protectionTime + impacto2_if_equals))
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
                    if (it->second == route1.id && it->first - instance.protectionTime > route1.arrivalTimes[i])
                        impacto2_if_equals += impacto1;
                    if (it->second == route2.id && it->first - instance.protectionTime > route2.arrivalTimes[j])
                        impacto2_if_equals = impacto2;
                    auto it_prev = prev(it);
                    if (Utils::doubleLessOrEqual(route2.arrivalTimes[j - 1] + dist3_add_r2 + vertice_parada_r1 + instance.protectionTime, it->first - instance.protectionTime + impacto2_if_equals) && Utils::doubleLessOrEqual(it_prev->first, route2.arrivalTimes[j - 1] + dist3_add_r2))
                    {
                        local_visita = true;
                    }
                }
            }
            if (!local_visita)
                continue;

            double if_prev_equals = 0, if_next_equals = 0;

            bool possibilidade_visita = true;
            for (int a = i + 1; a < route1.vertexSequence.size() - 1; a++)
            {

                int vertice = route1.vertexSequence[a];

                it = solution.visitedVertices[route1.vertexSequence[a]].find(route1.arrivalTimes[a] + instance.protectionTime);
                auto it_next = it != solution.visitedVertices[route1.vertexSequence[a]].end() ? next(it) : solution.visitedVertices[route1.vertexSequence[a]].end();
                auto it_prev = it != solution.visitedVertices[route1.vertexSequence[a]].begin() ? prev(it) : solution.visitedVertices[route1.vertexSequence[a]].end();

                // auto it_next = next(it);
                // auto it_prev = prev(it);
                // if (it_next != S.visitedVertices[rota1.route[a]].end())
                if_next_equals = (route2.id == it_next->second && route2.arrivalTimes[j] <= it_next->first - instance.protectionTime) ? impacto2 : 0;
                // if (it_prev != S.visitedVertices[rota1.route[a]].end())
                if_prev_equals = (route2.id == it_prev->second && route2.arrivalTimes[j] <= it_prev->first - instance.protectionTime) ? impacto2 : 0;

                if (it_next != solution.visitedVertices[route1.vertexSequence[a]].end() && it->first + impacto1 > it_next->first - instance.protectionTime + if_next_equals)
                {
                    possibilidade_visita = false;
                    break;
                }
                if (it_prev != solution.visitedVertices[route1.vertexSequence[a]].end() && it->first + impacto1 - instance.protectionTime < it_prev->first + if_prev_equals)
                {
                    possibilidade_visita = false;
                    break;
                }
            }

            if (!possibilidade_visita)
            {
                continue;
            }

            // utilizar um algoritmo de busca melhor
            for (int a = j + 1; a < route2.vertexSequence.size() - 1; a++)
            {
                int vertice = route2.vertexSequence[a];

                it = solution.visitedVertices[route2.vertexSequence[a]].find(route2.arrivalTimes[a] + instance.protectionTime);
                auto it_next = it != solution.visitedVertices[route2.vertexSequence[a]].end() ? next(it) : solution.visitedVertices[route2.vertexSequence[a]].end();
                auto it_prev = it != solution.visitedVertices[route2.vertexSequence[a]].begin() ? prev(it) : solution.visitedVertices[route2.vertexSequence[a]].end();

                if_next_equals = (route1.id == it_next->second && route1.arrivalTimes[i] <= it_next->first - instance.protectionTime) ? impacto1 : 0;
                if_prev_equals = (route1.id == it_prev->second && route1.arrivalTimes[i] <= it_prev->first - instance.protectionTime) ? impacto1 : 0;

                if (it_next != solution.visitedVertices[route2.vertexSequence[a]].end() && it->first + impacto2 > it_next->first - instance.protectionTime + if_next_equals)
                {
                    possibilidade_visita = false;
                    break;
                }
                if (it_prev != solution.visitedVertices[route2.vertexSequence[a]].end() && it->first + impacto2 - instance.protectionTime < it_prev->first + if_prev_equals)
                {
                    possibilidade_visita = false;
                    break;
                }
            }

            if (possibilidade_visita)
            {

                // exclui vertice i rota1
                swap_intra[0][0] = route1.vertexSequence[i];
                swap_intra[0][1] = score_r1;
                swap_intra[0][2] = route1.costMatrix[anterior1][proximo1] - dist1_remove_r1 - dist2_remove_r1 - vertice_parada_r1;
                swap_intra[0][3] = i;

                // exclui vertice j rota2
                swap_intra[1][0] = route2.vertexSequence[j];
                swap_intra[1][1] = score_r2;
                swap_intra[1][2] = route2.costMatrix[anterior2][proximo2] - dist3_remove_r2 - dist4_remove_r2 - vertice_parada_r2;
                swap_intra[1][3] = j;

                // adicionar vertice j na rota1
                swap_intra[2][0] = route2.vertexSequence[j];                                                                                // id vertice
                swap_intra[2][1] = score_r2;                                                                                      // score vertice
                swap_intra[2][2] = dist1_add_r1 + dist2_add_r1 + vertice_parada_r2 - route1.costMatrix[anterior1][proximo1]; // impacto insert_v vertice
                swap_intra[2][3] = i;                                                                                             // Local insert rota
                swap_intra[2][4] = route1.arrivalTimes[i - 1] + dist1_add_r1 + vertice_parada_r2;                                  // Visita_custo
                swap_intra[2][5] = (vertice_parada_r2 == instance.stopTime) ? 1 : 0;

                // adicionar vertice i na rota2
                swap_intra[3][0] = route1.vertexSequence[i];                                                                                // id vertice
                swap_intra[3][1] = score_r1;                                                                                      // score vertice
                swap_intra[3][2] = dist3_add_r2 + dist4_add_r2 + vertice_parada_r1 - route2.costMatrix[anterior2][proximo2]; // impacto insert_v vertice
                swap_intra[3][3] = j;                                                                                             // Local insert rota
                swap_intra[3][4] = route2.arrivalTimes[j - 1] + dist3_add_r2 + vertice_parada_r1;                                  // arrivalTimes
                swap_intra[3][5] = (vertice_parada_r1 == instance.stopTime) ? 1 : 0;

                if (best)
                {
                    best_swap = impacto1 + impacto2;
                }
                else
                {
                    break;
                }
            }
        }
        if (!best && swap_intra[0][0] != -1)
            break;
    }

    if (swap_intra[0][0] == -1)
    {
        return false;
    }
    else
    {
        // cout << "Rota: " << rota1.id << " - Vertice[" << swap_intra[0][3] << "] = " << swap_intra[0][0] << " |Swap| Rota: " << rota2.id << " - Vertice[" << swap_intra[1][3] << "] = " << swap_intra[1][0] << " | Impacto total: " << impacto1 + impacto2 << endl << "Impacto1: " << impacto1 << " | Impacto2: " << impacto2 << endl;
        // cout << "Rota: " << rota1.id << " - Visita[" << swap_intra[0][3] << "] = " << rota1.visita_custo[swap_intra[0][3]] << " |Swap| Rota: " << rota2.id << " - Visita[" << swap_intra[1][3] << "] = " << rota2.visita_custo[swap_intra[1][3]] << endl;

        // S.print_visited(209, 210);
        // S.print_visited(2, 3);

        // cout << "Antes: "<<endl<<rota1<<endl<< rota2<<endl;

        // S.print_visited(7, 8);

        // Excluindo
        route1.excludeVertex(swap_intra[0], solution.visitedVertices, solution.totalScore, solution.totalCost);
        route2.excludeVertex(swap_intra[1], solution.visitedVertices, solution.totalScore, solution.totalCost);

        // Adicionando
        route1.insertVertex(swap_intra[2], solution.visitedVertices, solution.totalScore, solution.totalCost);
        route2.insertVertex(swap_intra[3], solution.visitedVertices, solution.totalScore, solution.totalCost);

        // cout << "Depois: "<<endl<<rota1<<endl<< rota2<<endl;
        return true;
    }
}

bool LocalSearch::outOfRouteSwap(Instance &instance, Solution &solution, Route &route, int startIndex, int endIndex, bool &best)
{
    vector<vector<double>> swap_out(2, vector<double>(6, -1));
    double best_swap = -1;
    double score_v1;
    double score_v2;

    double dist1_remove_v1;
    double dist2_remove_v1;

    double dist1_add_v2;
    double dist2_add_v2;

    int vertice_parada_v1;
    int anterior1;
    int proximo1;

    double impacto1;

    // Itera sobre os vértices na rota
    for (int i = startIndex; i < endIndex; i++)
    {
        // cout << "Best swap: " << best_swap << endl;
        int v1 = route.vertexSequence[i];
        anterior1 = route.vertexSequence[i - 1];
        proximo1 = route.vertexSequence[i + 1];
        score_v1 = (route.isStopVertex[i]) ? instance.vertexScores[v1] : instance.vertexScores[v1] / 3;

        dist1_remove_v1 = route.costMatrix[anterior1][v1]; // Arestas que serão removidas V1
        dist2_remove_v1 = route.costMatrix[v1][proximo1];
        vertice_parada_v1 = (route.isStopVertex[i]) ? instance.stopTime : 0; // Se tiver parada, plus de 15 minutos

        for (int n = 0; n < 2; n++)
        {
            route.additionalStopTime = (n == 1) ? 0 : instance.stopTime;
            // Itera sobre todos os vértices
            for (int v2 = 1; v2 < instance.numVertices; v2++)
            {
                if (anterior1 == v2 || proximo1 == v2 || v1 == v2)
                    continue;

                score_v2 = (route.additionalStopTime == instance.stopTime) ? instance.vertexScores[v2] : instance.vertexScores[v2] / 3;

                if (best_swap >= -score_v1 + score_v2 || route.score - score_v1 + score_v2 <= route.score)
                    continue; // ja tem a melhor swap

                dist1_add_v2 = route.costMatrix[anterior1][v2]; // Arestas que serão adicionadas
                dist2_add_v2 = route.costMatrix[v2][proximo1];

                impacto1 = -dist1_remove_v1 - dist2_remove_v1 - vertice_parada_v1 + dist1_add_v2 + dist2_add_v2 + route.additionalStopTime;

                // Verifica se o novo custo ultrapassa o T_max
                if (!Utils::doubleLessOrEqual(route.cost + impacto1, instance.maxTime))
                {
                    // cout << "EXCEDEU TEMPO LIMITE" << endl;
                    continue; // Custo ultrapassa o T_max, não tem como fazer o swap
                }

                double impacto1_if_equals = 0;
                bool local_visita = false;
                if (solution.visitedVertices[v2].empty())
                {
                    local_visita = true;
                }
                else
                {
                    auto it = solution.visitedVertices[v2].lower_bound(
                        route.arrivalTimes[i - 1] + dist1_add_v2 + route.additionalStopTime + instance.protectionTime);
                    if (it == solution.visitedVertices[v2].end())
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
                    else if (it == solution.visitedVertices[v2].begin())
                    {
                        if (it->second == route.id)
                            impacto1_if_equals += impacto1;
                        if (Utils::doubleLessOrEqual(route.arrivalTimes[i - 1] + dist1_add_v2 + route.additionalStopTime + instance.protectionTime, it->first - instance.protectionTime + impacto1_if_equals))
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
                            impacto1_if_equals += impacto1;
                        auto it_prev = prev(it);
                        if (Utils::doubleLessOrEqual(route.arrivalTimes[i - 1] + dist1_add_v2 + route.additionalStopTime + instance.protectionTime, it->first - instance.protectionTime + impacto1_if_equals) &&
                            Utils::doubleLessOrEqual(it_prev->first, route.arrivalTimes[i - 1] + dist1_add_v2))
                        {
                            local_visita = true;
                        }
                    }
                }

                if (!local_visita)
                    continue;

                int if_next_equals = 0;
                int if_prev_equals = 0;
                // colocar o nova funçao de possibilidade visita
                // possibilidade visita rota
                bool possibilidade_visita = true;
                if (impacto1 < 0)
                {
                    possibilidade_visita = (Utils::doubleGreaterOrEqual(route.timeWindows[i + 1].first, impacto1 * -1));
                }
                else
                {
                    possibilidade_visita = (Utils::doubleGreaterOrEqual(route.timeWindows[i + 1].second, impacto1));
                }

                if (possibilidade_visita)
                {
                    // Dados do vértice removido da rota
                    swap_out[0][0] = v1;                                                                                                 // ID do vértice v1 que será removido
                    swap_out[0][1] = score_v1;                                                                                           // Score do vértice
                    swap_out[0][2] = route.costMatrix[anterior1][proximo1] - dist1_remove_v1 - dist2_remove_v1 - vertice_parada_v1; // Alteração no custo total ao remover o vértice
                    swap_out[0][3] = i;                                                                                                  // Posição do vértice na rota original

                    // Dados do vértice inserido na rota
                    swap_out[1][0] = v2;                                                                                          // ID do vértice que será inserido
                    swap_out[1][1] = score_v2;                                                                                    // Score do vértice
                    swap_out[1][2] = dist1_add_v2 + dist2_add_v2 + route.additionalStopTime - route.costMatrix[anterior1][proximo1]; // Alteração no custo total ao adicionar o vértice
                    swap_out[1][3] = i;                                                                                           // Posição onde o vértice será inserido na rota
                    swap_out[1][4] = route.arrivalTimes[i - 1] + dist1_add_v2 + route.additionalStopTime;                           // Novo tempo de visita
                    swap_out[1][5] = (route.additionalStopTime == instance.stopTime) ? 1 : 0;                                         // Indica se o vértice é uma parada

                    if (best)
                    {
                        best_swap = -score_v1 + score_v2;
                    }
                    else
                    {
                        break;
                    }
                    // return swap;
                }
            }
            if (!best && swap_out[0][0] != -1)
                break;
        }
        if (!best && swap_out[0][0] != -1)
            break;
    }
    if (swap_out[0][0] == -1)
    {
        return false;
    }
    else
    {
        // cout << "Rota: " << rota.id << " - Vertice[" << swap_out[0][3] << "] = " << swap_out[0][0] << " (SAI) " << " |Swap| Vertice[" << swap_out[0][3] << "] = " << swap_out[1][0] << " (ENTRA) " << endl;

        route.excludeVertex(swap_out[0], solution.visitedVertices, solution.totalScore, solution.totalCost);
        route.insertVertex(swap_out[1], solution.visitedVertices, solution.totalScore, solution.totalCost);
        return true;
    }
}