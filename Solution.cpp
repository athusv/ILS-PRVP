#include "Solution.h"
#include <cmath>
#include "Utils.h"
#include <random>
#include "LocalSearch.h"
using namespace std;

Solution::Solution(const Instance &instance) {
    visitedVertices.resize(instance.numVertices);

    int id = 0;
    for (int i = 0; i < instance.vehicleTypes.size(); i++) {
        for (int j = 0; j < instance.vehicleTypes[i]; j++) {
            id++;
            Route aux_rota(id, instance.protectionTime, instance.stopTime, i, instance.speed[i], instance);
            routes.push(aux_rota);
            improvedRoutes[i] = 0;
            testedRoutes[i] = 0;
        }
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

        route.cost += route.costMatrix[route.vertexSequence.back()][selectedVertex] + route.additionalStopTime;
        double vertexScore = instance.vertexScores[selectedVertex];

        if (route.additionalStopTime == instance.stopTime)
        {
            vertexScore = instance.vertexScores[selectedVertex];
            route.score += vertexScore;
            route.isStopVertex.push_back(1);
        }
        else
        {
            vertexScore = instance.vertexScores[selectedVertex] / 3;
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
        int i;

        while (true)
        {
            if (verticesToExclude == 0 || n == indexRoute.size())
                break;

            i = indexRoute[n];
            std::vector<double> excludeVertex = Utils::tryExcludeVertex(instance, visitedVertices, route, i);
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
            if (LocalSearch::outOfRouteSwap(instance, *this, route, i, i + 1, bestImprovement))
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

void Solution::printVisited(int startIndex, int endIndex) const{
    cout << endl << "Visited vertices: " << endl;
    for (int i = startIndex; i < endIndex; i++) {
        cout << "T: " << visitedVertices[i].size() << " Vertice " << i << ": [";
        for (const auto &visit : visitedVertices[i]){
            cout << "(" << visit.first << ", " << visit.second << "), ";
        }
        cout << "]" << endl;
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
                cout << caller << endl;
                auto itFind = visitedVertices[route.vertexSequence[i]].upper_bound(route.arrivalTimes[i] + instance.protectionTime);
                cout << "Vertice["<<i<<"]: " << route.vertexSequence[i] << " nao encontrado na Rota: " << route.id << endl;
                cout << "Visita_custo: [" << i << "]: " << route.arrivalTimes[i] + instance.protectionTime << " | Vertice[" << route.vertexSequence[i] << "]: Id = " << itFind->second << ", custo = " << itFind->first << endl;
                cout << route << endl << endl;
                return false;
            }
        }
    }
    return found;
}

bool Solution::checkVisited(const Instance &instance, string &caller) const{
    bool check = true;
    for (int i = 1; i < instance.numVertices; i++) {
        if (visitedVertices[i].empty() || visitedVertices[i].size() == 1) continue;

        auto it1 = visitedVertices[i].begin();
        auto it2 = next(it1);

        while (it2 != visitedVertices[i].end()){
            if (it1->first > it2->first - instance.protectionTime){
                cout << endl << "Visited - " << caller << ": Vertice " << i << " - Rota " << it1->second << endl;
                cout << "Vertice: " << i << " Diferença: " << it2->first - it1->first << " | Ideal: " << instance.protectionTime << endl;
                cout << "[" << distance(visitedVertices[i].begin(), it1) << "] Rota: " << it1->second << " - Custo: " << it1->first << endl;
                cout << "[" << distance(visitedVertices[i].begin(), it2) << "] Rota: " << it2->second << " - Custo: " << it2->first << endl;
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
            cout << caller << endl;
            cout << "Rota " << route.id << " - Score incompativel | aux_score = " << auxScore << " - Score = " << route.score << endl;
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
            cout << caller<<endl;
            cout<< "Rota " << route.id<< " - Custo incompativel | aux_custo = "<< auxCost << " - Custo = "<< route.cost<<endl;
            return false;
        }
        if(!Utils::doubleLessOrEqual(route.cost, instance.maxTime)){
            cout << caller << endl;
            cout << "Rota " << route.id << " Ultrapassou T_max | T_max = "<<instance.maxTime<< " Custo = " << route.cost << endl;
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
                cout << caller << endl;
                return false;
            }

            double minPush = (Utils::doubleLessOrEqual(route.timeWindows[i + 1].second, gapPush)) ? route.timeWindows[i + 1].second : gapPush;
            if (!Utils::doubleEquals(route.timeWindows[i].second, minPush)){
                cout << caller << endl;
                return false;
            }

            if (minPull < 0 || minPush < 0){
                cout << endl
                     << "Push_rotas - " << caller << endl<<" Rota: " << route.id << " - Vertice: " << route.vertexSequence[i] << endl;
                cout << "Folga_puxar: " << minPull << "|" << route.timeWindows[i].first << " - Folga_empurrar: " << minPush << "|" << route.timeWindows[i].second << endl;;
                printVisited(route.vertexSequence[i], route.vertexSequence[i] + 1);
                cout<<route<<endl;
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
                cout << caller << endl;
                cout << "Rota: " << route.id << " - Vertice[" << i << "] = " << route.vertexSequence[i]<<endl;
                cout << "Visita_custo: "<<route.arrivalTimes[i] << " | Aux_visita_custo: "<<auxArrivalTime<<endl;
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
    cout << totalScore << endl;
    cout << instance.numVehicles << endl;
    while (!routesCopy.empty()){
        Route route = routesCopy.top();
        routesCopy.pop();
        cout << endl << "Viatura " << route.id << ":" << endl;
        for (int i = 0; i < route.vertexSequence.size(); i++){
            if (i == 0)
            {
                cout << "Base," << route.arrivalTimes[i] << "," << route.isStopVertex[i] << endl;
                continue;
            }else if(i == route.vertexSequence.size()-1){
                cout << "Base," << route.arrivalTimes[i] << "," << route.isStopVertex[i] << endl;
                continue;
            }
            cout << route.vertexSequence[i] << "," << route.arrivalTimes[i] << "," << route.isStopVertex[i] << endl;
        }
    }
    cout <<endl;
}

bool Solution::operator<(const Solution &solution) const{
    return totalScore > solution.totalScore;
}

ostream &operator<<(ostream &os, const Solution &solution){
    priority_queue<Route> routesCopy = solution.routes;
    os << "Score: " << solution.totalScore << ", Custo: " << solution.totalCost << endl;
    while (!routesCopy.empty()) {
        os << routesCopy.top() << endl;
        routesCopy.pop();
    }
    return os;
}
