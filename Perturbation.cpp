#include "Perturbation.h"

void Perturbation::applyPerturbationWithStrength(Instance &instance, Solution &solution, std::mt19937 &randomGenerator, double perturbationIntensity){
    std::string caller;
    bool bestImprovement = false;

    std::priority_queue<Route> auxRoutes;
    while (!solution.routes.empty()){
        Route route = solution.routes.top();
        solution.routes.pop();
        int verticesToExclude = static_cast<int>(route.vertexSequence.size() * perturbationIntensity);
        
        std::vector<int> indexRoute;
        for (int i = 1; i < route.vertexSequence.size() - 1; i++) indexRoute.push_back(i);
        std::shuffle(indexRoute.begin(), indexRoute.end(), randomGenerator);

        int n = 0;
        int i;

        while (true){
            if (verticesToExclude == 0 || n == indexRoute.size())
                break;

            i = indexRoute[n];
            std::vector<double> excludeVertex = Utils::tryExcludeVertex(instance, solution.visitedVertices, route, i);
            if (excludeVertex[0] != -1){
                route.excludeVertex(excludeVertex, solution.visitedVertices, solution.totalScore, solution.totalCost);
                solution.updateSolutionTimeWindows();

                caller = "Perturbação EXCLUIR";
                assert(solution.checkSolution(instance, caller));

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
            if (LocalSearch::outOfRouteSwap(instance, solution, route, i, i + 1, bestImprovement)){
                solution.updateSolutionTimeWindows();

                caller = "Perturbação - Swap Out";
                assert(solution.checkSolution(instance, caller));

                std::shuffle(indexRoute.begin(), indexRoute.end(), randomGenerator);
                verticesToExclude--;
                n = 0;
                continue;
            }
            n++;
        }
        auxRoutes.push(route);
    }
    solution.routes = auxRoutes;

    return;
}
