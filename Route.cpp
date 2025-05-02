#include "Route.h"
#include "Utils.h"

Route::Route(int id, int protectionTime, int stopTime, int vehicleType, int speed, const Instance &instance)
{
    // cout << "Iniciando a construção do objeto Caminho..." << endl;
    this->id = id;
    this->protectionTime = protectionTime;
    this->stopTime = stopTime;
    this->vehicleType = vehicleType;
    this->speed = speed;

    // cout << "ID: " << id << ", Tipo de Veículo: " << tipo_veiculo << ", Velocidade: " << velocidade << " km/h" << endl;
    // cout << "Tempo de Proteção: " << t_prot << ", Tempo de Parada: " << t_parada << endl;

    vertexSequence.push_back(0);
    isStopVertex.push_back(0);
    arrivalTimes.push_back(0);
    timeWindows.push_back({999, 999});

    costMatrix.resize(instance.numVertices, vector<double>(instance.numVertices, 0));
    double metersPerSecond = speed / 3.6;
    for (int i = 0; i < instance.numVertices; i++)
    {
        for (int j = 0; j < instance.numVertices; j++)
        {
            costMatrix[i][j] = instance.distanceMatrix[i][j] / metersPerSecond;
            // if (i == 0 && j < 5) { // Exemplo: imprime as primeiras 5 distâncias da primeira linha
            //     cout << "Distância ajustada de " << i << " para " << j << ": " << distancia_matriz[i][j] << " segundos" << endl;
            // }
        }
    }
    // cout << "Construção do objeto Caminho concluída." << endl;
}

void Route::insertVertex(vector<double> &bestInsert, vector<map<double, int>> &visitedVertices, double &solutionScore, double &solutionCost)
{
    // id, score, impacto, local insert, visita_custo, para ou nao

    score += bestInsert[1];
    solutionScore += bestInsert[1]; // somando score
    cost += bestInsert[2];
    solutionCost += bestInsert[2]; // somando impacto

    vertexSequence.insert(vertexSequence.begin() + bestInsert[3], static_cast<int>(bestInsert[0])); // inserindo vertice
    isStopVertex.emplace(isStopVertex.begin() + bestInsert[3], bestInsert[5]);
    arrivalTimes.insert(arrivalTimes.begin() + bestInsert[3], bestInsert[4]);
    timeWindows.insert(timeWindows.begin() + bestInsert[3], {999, 999});

    // Inserindo informações na tabela de vértices visitados
    visitedVertices[static_cast<int>(bestInsert[0])][bestInsert[4] + protectionTime] = id;

    // Atualizando a tabela de vértices visitados para vértices subsequentes na rota
    updateVisitedVertices(bestInsert[3] + 1, vertexSequence.size() - 1, bestInsert[2], visitedVertices);
    updateTimeWindows(visitedVertices);
}

void Route::excludeVertex(vector<double> &excludeVertex, vector<map<double, int>> &visitedVertices, double &solutionScore, double &solutionCost)
{
    int indice = static_cast<int>(excludeVertex[3]);
    double score_vertice = excludeVertex[1];
    // id, score, impacto, indice_rota, indice_visited
    score -= score_vertice; // Convertendo para inteiro
    solutionScore -= score_vertice;
    cost += excludeVertex[2];
    solutionCost += excludeVertex[2];

    auto it = visitedVertices[static_cast<int>(excludeVertex[0])].find(arrivalTimes[indice] + protectionTime);
    visitedVertices[static_cast<int>(excludeVertex[0])].erase(it);
    // cout << "vertice indicado - " << route[indice] << endl;
    vertexSequence.erase(vertexSequence.begin() + indice);
    isStopVertex.erase(isStopVertex.begin() + indice);
    arrivalTimes.erase(arrivalTimes.begin() + indice);
    timeWindows.erase(timeWindows.begin() + indice);
    updateVisitedVertices(indice, vertexSequence.size() - 1, excludeVertex[2], visitedVertices);
    updateTimeWindows(visitedVertices);
}

void Route::makeStop(vector<double> stopVertex, vector<map<double, int>> &visitedVertices, double &solutionScore, double &solutionCost)
{
    cost += stopTime;
    solutionCost += stopTime;

    //    pontuaçao parcial
    score -= stopVertex[1];
    solutionScore -= stopVertex[1];

    //   pontuaçao_completa
    score += stopVertex[2];
    solutionScore += stopVertex[2];

    isStopVertex[stopVertex[0]] = 1;

    updateVisitedVertices(static_cast<int>(stopVertex[0]), vertexSequence.size() - 1, stopTime, visitedVertices);
    updateTimeWindows(visitedVertices);
}

void Route::makePass(vector<double> passVertex, vector<map<double, int>> &visitedVertices, double &solutionScore, double &solutionCost)
{
    cost -= passVertex[2];
    solutionCost -= passVertex[2];
    //     pontuaçao_parcial
    score += passVertex[1];
    solutionScore += passVertex[1];

    //     pontuaçao_completa
    score -= passVertex[2];
    solutionScore -= passVertex[2];

    isStopVertex[passVertex[0]] = 0;
    arrivalTimes[passVertex[0]] -= stopTime;

    updateVisitedVertices(passVertex[0], vertexSequence.size() - 1, -stopTime, visitedVertices);
    updateTimeWindows(visitedVertices);
}

void Route::updateVisitedVertices(int startIndex, int endIndex, double impact, vector<map<double, int>> &visitedVertices)
{
    for (int a = startIndex; a < endIndex; a++) {
        auto it = visitedVertices[vertexSequence[a]].find(arrivalTimes[a] + protectionTime);
        if (it != visitedVertices[vertexSequence[a]].end()) {
            visitedVertices[vertexSequence[a]].erase(it);
        }

        arrivalTimes[a] += impact;
        visitedVertices[vertexSequence[a]][arrivalTimes[a] + protectionTime] = id;
    }
}

void Route::updateTimeWindows(vector<map<double, int>> &visitedVertices) {
    double gapPull;
    double gapPush;
    for (int i = timeWindows.size() - 2; i >= 0; i--) {
        if (i == 0) {
            timeWindows[i].first = timeWindows[i + 1].first;
            timeWindows[i].second = timeWindows[i + 1].second;
            break;
        }
        auto it = visitedVertices[vertexSequence[i]].find(arrivalTimes[i] + protectionTime);

        auto nextIt = next(it);
        auto prevIt = prev(it);

        gapPull = (it == visitedVertices[vertexSequence[i]].begin()) ? 999 : (it->first - protectionTime) - prevIt->first;
        // gapPull = (it->first - grafo.t_prot) - prev_it->first;
        gapPush = (nextIt == visitedVertices[vertexSequence[i]].end()) ? 999 : (nextIt->first - protectionTime) - it->first;

        timeWindows[i].first = (Utils::doubleLessOrEqual(timeWindows[i + 1].first, gapPull)) ? timeWindows[i+1].first : gapPull;
        timeWindows[i].second = (Utils::doubleLessOrEqual(timeWindows[i + 1].second, gapPush)) ? timeWindows[i+1].second :gapPush;
        // push_hotspots[i].first = min(push_hotspots[i + 1].first, folga_puxar);
        // push_hotspots[i].second = min(push_hotspots[i + 1].second, folga_empurrar);
    }
}

void Route::printTimeWindows() {
    cout << "Veiculo " << id << " - Score: " << score << " - Custo: " << cost << endl << "Push: [";
    for (int v = 0; v < vertexSequence.size(); v++) {
        cout << "(" << timeWindows[v].first << ", " << timeWindows[v].second << "),";
    }
    cout << "]" << endl;
}

ostream &operator<<(ostream &os, const Route &route) {
    os <<endl<< "Veiculo " << route.id << " - Score: " << route.score << " - Custo: " << route.cost << " - Velocidade: " << route.speed << "km/h Rota: [";
    for (int v = 0; v < route.vertexSequence.size(); v++) {
        os << "[" << v<<"]:";
        if (route.isStopVertex[v] == true) {
            os << "(*" << route.vertexSequence[v] << "*)"
                    << ", ";
        } else if (v + 1 == route.vertexSequence.size()) {
            os << "(" << route.vertexSequence[v] << ")";
        } else {
            os << "(" << route.vertexSequence[v] << ")"
                    << ", ";
        }
    }
    os << "]";
    return os;
}

bool Route::operator<(const Route &c) const {
    if (cost == c.cost) {
        return id > c.id; // Adiciona comparação pelo ID para garantir ordem consistente
    }
    return cost > c.cost;
}
