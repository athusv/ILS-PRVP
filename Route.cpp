#include "Route.h"
#include "Utils.h"

Route::Route(const Instance &instance, int id, int vehicleType)
{
    // cout << "Iniciando a construção do objeto Caminho..." << endl;
    this->id = id;
    this->protectionTime = instance.protectionTime;
    this->stopTime = instance.stopTime;
    this->vehicleType = vehicleType;
    this->maxTime = instance.maxTime;
    this->speed = instance.speed[vehicleType];

    // cout << "ID: " << id << ", Tipo de Veículo: " << tipo_veiculo << ", Velocidade: " << velocidade << " km/h" << endl;
    // cout << "Tempo de Proteção: " << t_prot << ", Tempo de Parada: " << t_parada << endl;

    vertexSequence.push_back(0);
    isStopVertex.push_back(0);
    arrivalTimes.push_back(0);
    timeWindows.push_back({999, 999});

    costMatrix.resize(instance.numVertex, vector<double>(instance.numVertex, 0));
    double metersPerSecond = speed / 3.6;
    for (int i = 0; i < instance.numVertex; i++)
    {
        for (int j = 0; j < instance.numVertex; j++)
        {
            costMatrix[i][j] = instance.distanceMatrix[i][j] / metersPerSecond;
            // if (i == 0 && j < 5) { // Exemplo: imprime as primeiras 5 distâncias da primeira linha
            //     cout << "Distância ajustada de " << i << " para " << j << ": " << distancia_matriz[i][j] << " segundos" << endl;
            // }
        }
    }
    // cout << "Rota " << id << " do veículo " << vehicleType << ":" << endl
    //      << endl;
    // Inicializar nearestVertices ajustando os tempos para a velocidade do veículo
    nearestVertices.resize(instance.numVertex);
    for (int i = 0; i < instance.numVertex; i++)
    {
        nearestVertices[i] = instance.nearestVertices[i];

        // Converter distâncias para custos (tempo) baseado na velocidade do veículo
        for (auto &vertexDist : nearestVertices[i])
        {
            vertexDist.distance = vertexDist.distance / metersPerSecond;
        }

        // cout << "Vértice " << i << " tem " << nearestVertices[i].size() << " vértices próximos: [";
        // for (const auto &vertex : nearestVertices[i])
        // {
        //     cout << "(" << vertex.index << ", " << vertex.distance << "), ";
        // }
        // cout << "]" << endl;
    }

    // cout << "Construção do objeto Caminho concluída." << endl;
}

bool Route::realocate(const vector<double> &vertexScores, vector<map<double, int>> &visitedVertices, double &solutionScore, double &solutionCost, bool &isBest)
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
    for (int i = 1; i < vertexSequence.size() - 2; i++)
    {
        int actualVertex = vertexSequence[i];
        int previousVertex = vertexSequence[i - 1];
        int nextVertex = vertexSequence[i + 1];

        dist1RemovePrevToActual = costMatrix[previousVertex][actualVertex];
        dist2RemoveActualToNext = costMatrix[actualVertex][nextVertex];
        dist3AddPrevToNext = costMatrix[previousVertex][nextVertex];

        vertexStopTime = (isStopVertex[i] == 1) ? stopTime : 0;
        removalImpact = -dist1RemovePrevToActual - dist2RemoveActualToNext - vertexStopTime + dist3AddPrevToNext;
        vertexScore = (isStopVertex[i] == 1) ? vertexScores[actualVertex] : vertexScores[actualVertex] / 3;

        for (int j = 0; j < vertexSequence.size() - 2; j++)
        {
            if (j == i || j == i - 1)
                continue;
            int previousVertex2 = vertexSequence[j];
            int nextVertex2 = vertexSequence[j + 1];
            if (previousVertex2 == actualVertex || nextVertex2 == actualVertex)
                continue;
            dist1AddPrevToActual = costMatrix[previousVertex2][actualVertex];
            dist2AddActualToNext = costMatrix[actualVertex][nextVertex2];
            dist3RemovePrevToNext = costMatrix[previousVertex2][nextVertex2];
            insertionImpact = dist1AddPrevToActual + dist2AddActualToNext + vertexStopTime - dist3RemovePrevToNext;

            if (cost + removalImpact + insertionImpact > maxTime || removalImpact + insertionImpact >= 0 || removalImpact + insertionImpact >= bestRelocateImpact)
            {
                continue;
            }

            bool localVisitation = false;  // iniciando como se ele nao pudesse vizitar
            double impacto1_if_equals = 0; // caso seja igual, deve ser considerado esse impacto
            double impacto1_i_minus_j = 0;
            map<double, int> auxVisitedVertice1 = visitedVertices[actualVertex];

            auto it = auxVisitedVertice1.find(arrivalTimes[i] + protectionTime);
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
                it = auxVisitedVertice1.lower_bound(arrivalTimes[j] + dist1AddPrevToActual + protectionTime);
                if (it == auxVisitedVertice1.end())
                {
                    // caso nao tenha vizinhos para frente, verificar se a visita anterior é possivel

                    auto it_prev = prev(it);
                    if (it_prev->second == id)
                    {
                        if (arrivalTimes[i] < it_prev->first - protectionTime)
                        {
                            impacto1_if_equals = removalImpact;
                        }
                    }

                    if (Utils::doubleLessOrEqual(it_prev->first + impacto1_if_equals, arrivalTimes[j] + impacto1_i_minus_j + dist1AddPrevToActual))
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
                    if (it->second == id)
                    {
                        // caso a visita analisada seja a mesma rota, verificar se a visita para frente é possivel
                        if (arrivalTimes[j] < it->first - protectionTime && arrivalTimes[i] < it->first - protectionTime)
                        {
                            impacto1_if_equals = removalImpact + insertionImpact;
                        }
                        else if (arrivalTimes[j] < it->first - protectionTime)
                        {
                            impacto1_if_equals = insertionImpact;
                        }
                    }

                    if (Utils::doubleLessOrEqual(arrivalTimes[j] + impacto1_i_minus_j + dist1AddPrevToActual + vertexStopTime + protectionTime, it->first - protectionTime + impacto1_if_equals))
                    {
                        localVisitation = true;
                    }
                    else
                    {
                        localVisitation = false;
                    }
                }
                else
                {
                    if (it->second == id)
                    {
                        if (arrivalTimes[j] < it->first - protectionTime && arrivalTimes[i] < it->first - protectionTime)
                        {
                            impacto1_if_equals = removalImpact + insertionImpact;
                        }
                        else if (arrivalTimes[j] < it->first - protectionTime)
                        {
                            impacto1_if_equals = insertionImpact; // corrigido
                        }
                    }
                    auto it_prev = prev(it);
                    double impacto2_if_equals = 0;
                    if (it_prev->second == id)
                    {
                        if (arrivalTimes[i] < it_prev->first - protectionTime)
                        {
                            impacto2_if_equals = removalImpact;
                        }
                    }
                    if (Utils::doubleLessOrEqual(arrivalTimes[j] + impacto1_i_minus_j + dist1AddPrevToActual + vertexStopTime + protectionTime, it->first - protectionTime + impacto1_if_equals) && Utils::doubleLessOrEqual(it_prev->first + impacto2_if_equals, arrivalTimes[j] + impacto1_i_minus_j + dist1AddPrevToActual))
                    {
                        localVisitation = true;
                    }
                }
            }
            if (!localVisitation)
                continue;

            double cumulativeImpact = 0;
            bool possibilityVisitation = true;
            for (int n = (i < j) ? i + 1 : j + 1; n < vertexSequence.size() - 1; n++)
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

                auto it = visitedVertices[vertexSequence[n]].find(arrivalTimes[n] + protectionTime);

                if (it == visitedVertices[vertexSequence[n]].end())
                {
                    possibilityVisitation = false;
                    break;
                }

                auto it_next = std::next(it);
                auto it_prev = it != visitedVertices[vertexSequence[n]].begin() ? prev(it) : visitedVertices[vertexSequence[n]].end();

                if (it_next != visitedVertices[vertexSequence[n]].end())
                {
                    if (id == it_next->second)
                    {
                        if_next_equals += insertionImpact; // corrigido
                    }
                    if (arrivalTimes[i] < it_next->first - protectionTime)
                    {
                        if_next_equals += removalImpact;
                    }
                }
                else
                {
                    if_next_equals = 0;
                }

                if (id == it_prev->second)
                {
                    if (arrivalTimes[j] < it_prev->first - protectionTime)
                    {
                        if_prev_equals += insertionImpact;
                    }
                    if (arrivalTimes[i] < it_prev->first - protectionTime)
                    {
                        if_prev_equals += removalImpact;
                    }
                }
                else
                {
                    if_prev_equals = 0;
                }

                if (it_next != visitedVertices[vertexSequence[n]].end() && it->first + cumulativeImpact > it_next->first - protectionTime + if_next_equals)
                {
                    possibilityVisitation = false;
                    break;
                }

                if (it_prev != visitedVertices[vertexSequence[n]].end() && it->first + cumulativeImpact - protectionTime < it_prev->first + if_prev_equals)
                {
                    possibilityVisitation = false;
                    break;
                }
            }

            if (!possibilityVisitation)
            {
                continue;
            }

            if (possibilityVisitation)
            {
                relocData[0][0] = actualVertex;
                relocData[0][1] = vertexScore;
                relocData[0][2] = dist3AddPrevToNext - dist1RemovePrevToActual - dist2RemoveActualToNext - vertexStopTime;
                relocData[0][3] = (j < i) ? i + 1 : i;

                // adicionar vertice i na rota
                relocData[1][0] = actualVertex;                                                                         // id vertice
                relocData[1][1] = vertexScore;                                                                          // score vertice
                relocData[1][2] = dist1AddPrevToActual + dist2AddActualToNext + vertexStopTime - dist3RemovePrevToNext; // impacto insert_v vertice
                relocData[1][3] = j + 1;                                                                                // Local insert rota
                relocData[1][4] = arrivalTimes[j] + dist1AddPrevToActual + vertexStopTime;                              // arrivalTimes
                relocData[1][5] = (vertexStopTime == stopTime) ? 1 : 0;

                if (isBest)
                {
                    bestRelocateImpact = removalImpact + insertionImpact;
                }
                else
                {
                    break;
                }
            }
        }
        if (!isBest && relocData[0][0] != -1)
            break;
    }

    if (relocData[0][0] == -1)
    {
        return false;
    }
    else
    {
        insertVertex(relocData[1], visitedVertices, solutionScore, solutionCost);
        excludeVertex(relocData[0], visitedVertices, solutionScore, solutionCost);
        return true;
    }
}

bool Route::tryMakeStop(const vector<double> &vertexScores, vector<map<double, int>> &visitedVertices, double &solutionScore, double &solutionCost, bool &isBest)
{
    // O melhor vertice marcado como passada q for possivel ser parada, sera uma parada

    vector<double> vertice_para = {-1, -1, -1};
    if (cost + stopTime > maxTime)
    {
        return false;
    }

    for (int i = 1; i < vertexSequence.size() - 1; i++)
    {
        if (isStopVertex[i] == 1)
            continue;

        if (vertexScores[vertexSequence[i]] < vertice_para[1])
            continue;

        bool possibilidade_parada = (Utils::doubleGreaterOrEqual(timeWindows[i].second, stopTime));

        if (possibilidade_parada)
        {
            vertice_para = {static_cast<double>(i), vertexScores[vertexSequence[i]] / 3, vertexScores[vertexSequence[i]]};

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
        makeStop(vertice_para, visitedVertices, solutionScore, solutionCost);
        return true;
    }
}

bool Route::bestInsert(const vector<double> &vertexScores, int numVertices, vector<map<double, int>> &visitedVertices, double &solutionScore, double &solutionCost, bool &isBest)
{
    //                 id, score, custo, local insert, custo local insert, local visita
    std::vector<double> b_vert_insert = {-1, -1, -1, -1, -1, -1};

    double dist1 = 0;
    double dist2 = 0;
    double dist3 = 0;

    for (int n = 0; n < 2; n++)
    {
        additionalStopTime = (n == 1) ? 0 : stopTime;

        for (int v = 1; v < numVertices; v++)
        {
            if (vertexScores[v] < b_vert_insert[1])
                continue;

            double score_v = (additionalStopTime == stopTime) ? vertexScores[v] : vertexScores[v] / 3;

            for (int j = 0; j < vertexSequence.size() - 1; j++)
            {
                int anterior = vertexSequence[j];
                int proximo = vertexSequence[j + 1];

                if (anterior == v || proximo == v)
                {
                    continue;
                }

                dist1 = costMatrix[anterior][v];
                dist2 = costMatrix[v][proximo];
                dist3 = costMatrix[anterior][proximo];
                double impacto = dist1 + dist2 + additionalStopTime - dist3;

                if (cost + impacto > maxTime)
                    continue;

                if (b_vert_insert[1] == score_v && b_vert_insert[2] < impacto)
                {
                    continue;
                }

                bool local_visita = false;
                if (visitedVertices[v].empty())
                {
                    local_visita = true;
                }
                else
                {
                    auto it = visitedVertices[v].lower_bound(
                        arrivalTimes[j] + dist1 + additionalStopTime + protectionTime);
                    if (it == visitedVertices[v].end())
                    {
                        auto it_prev = prev(it);
                        if (Utils::doubleLessOrEqual(it_prev->first, arrivalTimes[j] + dist1))
                        {
                            local_visita = true;
                        }
                        else
                        {
                            local_visita = false;
                        }
                    }
                    else if (it == visitedVertices[v].begin())
                    {
                        if (Utils::doubleLessOrEqual(arrivalTimes[j] + dist1 + additionalStopTime + protectionTime, it->first - protectionTime))
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
                        if (Utils::doubleLessOrEqual(arrivalTimes[j] + dist1 + additionalStopTime + protectionTime, it->first - protectionTime) &&
                            Utils::doubleLessOrEqual(
                                it_prev->first, arrivalTimes[j] + dist1))
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
                    possibilidade_visita = (Utils::doubleGreaterOrEqual(timeWindows[j + 1].first, impacto * -1));
                }
                else
                {
                    possibilidade_visita = (Utils::doubleGreaterOrEqual(timeWindows[j + 1].second, impacto));
                }

                if (possibilidade_visita)
                {
                    b_vert_insert[0] = v;                                            // id vertice
                    b_vert_insert[1] = score_v;                                      // score vertice
                    b_vert_insert[2] = impacto;                                      // impacto insert_v vertice
                    b_vert_insert[3] = j + 1;                                        // Local insert rota
                    b_vert_insert[4] = arrivalTimes[j] + dist1 + additionalStopTime; // Visita_custo
                    b_vert_insert[5] = (additionalStopTime == stopTime) ? 1 : 0;

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
        insertVertex(b_vert_insert, visitedVertices, solutionScore, solutionCost);
        return true;
    }
}

bool Route::bestInsertNearestVertices(const vector<double> &vertexScores, int numVertices, vector<map<double, int>> &visitedVertices, double &solutionScore, double &solutionCost, bool &isBest)
{
    //                 id, score, custo, local insert, custo local insert, local visita
    std::vector<double> b_vert_insert = {-1, -1, -1, -1, -1, -1};

    double dist1 = 0;
    double dist2 = 0;
    double dist3 = 0;

    for (int n = 0; n < 2; n++)
    {
        additionalStopTime = (n == 1) ? 0 : stopTime;

        for (int j = 0; j < vertexSequence.size() - 1; j++)
        {
            int anterior = vertexSequence[j];
            int proximo = vertexSequence[j + 1];

            for (auto &vertex : nearestVertices[anterior])
            {
                int v = vertex.index;
                if (v == 0)
                    continue;
                if (v == anterior || v == proximo)
                    continue;
                if (cost + vertex.distance > maxTime)
                    continue;
                if (vertexScores[v] < b_vert_insert[1])
                    continue;

                double score_v = (additionalStopTime == stopTime) ? vertexScores[v] : vertexScores[v] / 3;

                dist1 = costMatrix[anterior][v];
                dist2 = costMatrix[v][proximo];
                dist3 = costMatrix[anterior][proximo];
                double impacto = dist1 + dist2 + additionalStopTime - dist3;

                if (cost + impacto > maxTime)
                    continue;

                if (b_vert_insert[1] == score_v && b_vert_insert[2] < impacto)
                {
                    continue;
                }

                bool local_visita = false;
                if (visitedVertices[v].empty())
                {
                    local_visita = true;
                }
                else
                {
                    auto it = visitedVertices[v].lower_bound(
                        arrivalTimes[j] + dist1 + additionalStopTime + protectionTime);
                    if (it == visitedVertices[v].end())
                    {
                        auto it_prev = prev(it);
                        if (Utils::doubleLessOrEqual(it_prev->first, arrivalTimes[j] + dist1))
                        {
                            local_visita = true;
                        }
                        else
                        {
                            local_visita = false;
                        }
                    }
                    else if (it == visitedVertices[v].begin())
                    {
                        if (Utils::doubleLessOrEqual(arrivalTimes[j] + dist1 + additionalStopTime + protectionTime, it->first - protectionTime))
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
                        if (Utils::doubleLessOrEqual(arrivalTimes[j] + dist1 + additionalStopTime + protectionTime, it->first - protectionTime) &&
                            Utils::doubleLessOrEqual(
                                it_prev->first, arrivalTimes[j] + dist1))
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
                    possibilidade_visita = (Utils::doubleGreaterOrEqual(timeWindows[j + 1].first, impacto * -1));
                }
                else
                {
                    possibilidade_visita = (Utils::doubleGreaterOrEqual(timeWindows[j + 1].second, impacto));
                }

                if (possibilidade_visita)
                {
                    b_vert_insert[0] = v;                                            // id vertice
                    b_vert_insert[1] = score_v;                                      // score vertice
                    b_vert_insert[2] = impacto;                                      // impacto insert_v vertice
                    b_vert_insert[3] = j + 1;                                        // Local insert rota
                    b_vert_insert[4] = arrivalTimes[j] + dist1 + additionalStopTime; // Visita_custo
                    b_vert_insert[5] = (additionalStopTime == stopTime) ? 1 : 0;

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
        insertVertex(b_vert_insert, visitedVertices, solutionScore, solutionCost);
        return true;
    }
}

bool Route::intraRouteSwap(const Instance &instance, vector<map<double, int>> &visitedVertices, double &solutionScore, double &solutionCost, bool &isBest)
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
    for (int i = 1; i <= vertexSequence.size() - 2; i++)
    {
        // if(best_swap!=-1) break;
        // cout<<"Vertice i["<< i << "]: "<<rota.route[i]<< endl;
        anterior1 = vertexSequence[i - 1];
        proximo1 = vertexSequence[i + 1];
        score_v1 = (isStopVertex[i] == 1) ? instance.vertexScores[vertexSequence[i]] : instance.vertexScores[vertexSequence[i]] / 3;

        dist1_remove_v1 = costMatrix[anterior1][vertexSequence[i]]; // arestas que serão removidas V1
        dist2_remove_v1 = costMatrix[vertexSequence[i]][proximo1];
        vertice_parada_v1 = (isStopVertex[i] == 1) ? instance.stopTime : 0; // se tiver parada, plus de 15 minutos

        for (int j = i + 2; j <= vertexSequence.size() - 2; j++)
        {
            if (i == j)
                break;
            // cout << "Vertice j[" << j << "]: " << rota.route[j] << endl;
            anterior2 = vertexSequence[j - 1];
            proximo2 = vertexSequence[j + 1];
            score_v2 = (isStopVertex[j] == 1) ? instance.vertexScores[vertexSequence[j]] : instance.vertexScores[vertexSequence[j]] / 3;

            vertice_parada_v2 = (isStopVertex[j] == 1) ? instance.stopTime : 0;

            dist1_remove_v2 = costMatrix[anterior2][vertexSequence[j]]; // arestas que serão removidas V2
            dist2_remove_v2 = costMatrix[vertexSequence[j]][proximo2];

            bool seguidos = (j == i + 1);
            if (seguidos)
            {
                dist1_remove_v2 = costMatrix[anterior2][vertexSequence[j]]; // arestas que serão removidas V2
                dist2_remove_v2 = costMatrix[vertexSequence[j]][proximo2];

                dist1_add_v1 = costMatrix[vertexSequence[j]][vertexSequence[i]];
                dist2_add_v1 = costMatrix[vertexSequence[i]][proximo2];

                dist1_add_v2 = costMatrix[anterior1][vertexSequence[j]];
                dist2_add_v2 = costMatrix[vertexSequence[j]][vertexSequence[i]];

                impacto1 = dist1_add_v1 + dist2_add_v1 + dist1_add_v2 - dist1_remove_v1 - dist2_remove_v1 - dist2_remove_v2;
                impacto2 = 0;
            }
            else
            {
                dist1_remove_v2 = costMatrix[anterior2][vertexSequence[j]]; // arestas que serão removidas V2
                dist2_remove_v2 = costMatrix[vertexSequence[j]][proximo2];

                dist1_add_v1 = costMatrix[anterior2][vertexSequence[i]]; // arestas adicionadas v1
                dist2_add_v1 = costMatrix[vertexSequence[i]][proximo2];

                dist1_add_v2 = costMatrix[anterior1][vertexSequence[j]]; // arestas adicionadas V2
                dist2_add_v2 = costMatrix[vertexSequence[j]][proximo1];

                impacto1 = -dist1_remove_v1 - dist2_remove_v1 - vertice_parada_v1 + dist1_add_v2 + dist2_add_v2 + vertice_parada_v2;
                impacto2 = -dist1_remove_v2 - dist2_remove_v2 - vertice_parada_v2 + dist1_add_v1 + dist2_add_v1 + vertice_parada_v1;
            }
            if (cost + impacto1 + impacto2 > instance.maxTime)
            {
                continue; // custo ultrapassa o maxTime, nao tem como fazer o swap
            }

            if (impacto1 + impacto2 >= 0 || impacto1 + impacto2 >= best_swap)
            {
                continue; // nao melhorou a soluçao
            }
            int id = id;
            int vertice_i = vertexSequence[i];
            int vertice_j = vertexSequence[j];

            bool local_visita = false;
            double impacto1_if_equals = 0;
            map<double, int> aux_visited_vertice2 = visitedVertices[vertexSequence[j]];
            auto it = aux_visited_vertice2.find(arrivalTimes[j] + instance.protectionTime);
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
                it = aux_visited_vertice2.lower_bound(arrivalTimes[i - 1] + dist1_add_v2 + vertice_parada_v2 + instance.protectionTime);
                if (it == aux_visited_vertice2.end())
                {
                    auto it_prev = prev(it);
                    if (Utils::doubleLessOrEqual(it_prev->first, arrivalTimes[i - 1] + dist1_add_v2))
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
                    if (it->second == id)
                    {
                        if (arrivalTimes[j] < it->first - instance.protectionTime)
                        {
                            impacto1_if_equals = impacto1 + impacto2;
                        }
                        else
                        {
                            impacto1_if_equals = impacto1; // corrigido
                        }
                    }

                    if (Utils::doubleLessOrEqual(arrivalTimes[i - 1] + dist1_add_v2 + vertice_parada_v2 + instance.protectionTime, it->first - instance.protectionTime + impacto1_if_equals))
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
                    if (it->second == id)
                    {
                        if (arrivalTimes[j] < it->first - instance.protectionTime)
                        {
                            impacto1_if_equals = impacto1 + impacto2;
                        }
                        else
                        {
                            impacto1_if_equals = impacto1;
                        }
                    }
                    auto it_prev = prev(it);
                    if (Utils::doubleLessOrEqual(arrivalTimes[i - 1] + dist1_add_v2 + vertice_parada_v2 + instance.protectionTime, it->first - instance.protectionTime + impacto1_if_equals) && Utils::doubleLessOrEqual(it_prev->first, arrivalTimes[i - 1] + dist1_add_v2))
                    {
                        local_visita = true;
                    }
                }
            }
            if (!local_visita)
                // cout << "---- Erro" << endl;
                continue;

            double temp_visita_custo = (j == i + 1) ? arrivalTimes[i - 1] + dist1_add_v2 + vertice_parada_v2 + dist2_add_v2 : arrivalTimes[j - 1] + impacto1;
            local_visita = false;
            double impacto2_if_equals = 0;
            map<double, int> aux_visited_vertice1 = visitedVertices[vertexSequence[i]];
            it = aux_visited_vertice1.find(arrivalTimes[i] + instance.protectionTime);
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
                    if (it_prev->second == id)
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
                    if (it->second == id)
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
                    if (it->second == id)
                        impacto2_if_equals = impacto1 + impacto2;
                    auto it_prev = prev(it);
                    double if_equals_aux = 0;
                    if (it_prev->first - instance.protectionTime > arrivalTimes[i])
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
            for (int n = i + 1; n < vertexSequence.size() - 1; n++)
            {
                if (n == j)
                {
                    impacto += impacto2;
                    continue;
                }
                auto it = visitedVertices[vertexSequence[n]].find(arrivalTimes[n] + instance.protectionTime);
                auto it_next = std::next(it);
                auto it_prev = it != visitedVertices[vertexSequence[n]].begin() ? prev(it) : visitedVertices[vertexSequence[n]].end();
                if (id == it_next->second)
                {
                    // ocorrencia vai sofrer os dois impactos
                    if (arrivalTimes[j] <= it_next->first - instance.protectionTime)
                    {
                        if_next_equals = impacto1 + impacto2;
                    }
                    else if (arrivalTimes[i] <= it_next->first - instance.protectionTime)
                    { // ocorrencia vai sofrer somente o impacto1
                        if_next_equals = impacto1;
                    }
                }
                else
                {
                    if_next_equals = 0;
                }

                if (id == it_prev->second)
                {
                    if (arrivalTimes[j] <= it_prev->first - instance.protectionTime)
                    {
                        if_prev_equals = impacto1 + impacto2;
                    }
                    else if (arrivalTimes[i] <= it_prev->first - instance.protectionTime)
                    { // ocorrencia vai sofrer somente o impacto1
                        if_prev_equals = impacto1;
                    }
                }
                else
                {
                    if_prev_equals = 0;
                }
                // if_prev_equals = (rota.id == it_prev->second && rota.visita_custo[j] <= it_prev->first - grafo.protectionTime) ? impacto2 : 0;

                if (it_next != visitedVertices[vertexSequence[n]].end() && it->first + impacto > it_next->first - instance.protectionTime + if_next_equals)
                {
                    possibilidade_visita = false;
                    break;
                }

                if (it_prev != visitedVertices[vertexSequence[n]].end() && it->first + impacto - instance.protectionTime < it_prev->first + if_prev_equals)
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
                swap_inter[0][0] = vertexSequence[i];
                swap_inter[0][1] = score_v1;
                swap_inter[0][2] = costMatrix[anterior1][proximo1] - dist1_remove_v1 - dist2_remove_v1 - vertice_parada_v1;
                swap_inter[0][3] = i;

                // exclui vertice j rota2
                swap_inter[1][0] = vertexSequence[j];
                swap_inter[1][1] = score_v2;
                swap_inter[1][2] = costMatrix[anterior2][proximo2] - dist1_remove_v2 - dist2_remove_v2 - vertice_parada_v2;
                swap_inter[1][3] = j;

                // adicionar vertice j na rota1
                swap_inter[2][0] = vertexSequence[j];                                                                 // id vertice
                swap_inter[2][1] = score_v2;                                                                          // score vertice
                swap_inter[2][2] = dist1_add_v2 + dist2_add_v2 + vertice_parada_v2 - costMatrix[anterior1][proximo1]; // impacto insert_v vertice
                swap_inter[2][3] = i;                                                                                 // Local insert rota
                swap_inter[2][4] = arrivalTimes[i - 1] + dist1_add_v2 + vertice_parada_v2;                            // Visita_custo
                swap_inter[2][5] = (vertice_parada_v2 == instance.stopTime) ? 1 : 0;

                // adicionar vertice i na rota2
                swap_inter[3][0] = vertexSequence[i];                                                                 // id vertice
                swap_inter[3][1] = score_v1;                                                                          // score vertice
                swap_inter[3][2] = dist1_add_v1 + dist2_add_v1 + vertice_parada_v1 - costMatrix[anterior2][proximo2]; // impacto insert_v vertice
                swap_inter[3][3] = j;                                                                                 // Local insert rota
                swap_inter[3][4] = arrivalTimes[j - 1] + dist1_add_v1 + vertice_parada_v1 + impacto1;                 // Visita_custo
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

        excludeVertex(swap_inter[0], visitedVertices, solutionScore, solutionCost);
        insertVertex(swap_inter[2], visitedVertices, solutionScore, solutionCost);

        excludeVertex(swap_inter[1], visitedVertices, solutionScore, solutionCost);
        insertVertex(swap_inter[3], visitedVertices, solutionScore, solutionCost);

        // cout << endl<<"DEPOIS: "<<endl;
        // // cout << "Rota: "<<rota.id<<" - Swap: "<<swap_inter[0][0]<< " | " << swap_inter[1][0]<<endl;
        // cout<<"Rota: " << rota.id << " - Vertice[" << swap_inter[0][3] << "] = " << rota.route[swap_inter[0][3]] << " |Swap| Vertice[" << swap_inter[1][3] << "] = " << rota.route[swap_inter[1][3]] <<endl;
        // cout << "Visita_custo[" << swap_inter[0][3] << "] = " << rota.visita_custo[swap_inter[0][3]] << " | Visita_custo[" << swap_inter[1][3] << "] = " << rota.visita_custo[swap_inter[1][3]] << endl;
        // cout << rota << endl;
        return true;
    }
}

bool Route::outOfRouteSwap(const vector<double> &vertexScores, int numVertices, vector<map<double, int>> &visitedVertices, double &solutionScore, double &solutionCost, int startIndex, int endIndex, bool &isBest)
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
        int v1 = vertexSequence[i];
        anterior1 = vertexSequence[i - 1];
        proximo1 = vertexSequence[i + 1];
        score_v1 = (isStopVertex[i]) ? vertexScores[v1] : vertexScores[v1] / 3;

        dist1_remove_v1 = costMatrix[anterior1][v1]; // Arestas que serão removidas V1
        dist2_remove_v1 = costMatrix[v1][proximo1];
        vertice_parada_v1 = (isStopVertex[i]) ? stopTime : 0; // Se tiver parada, plus de 15 minutos

        for (int n = 0; n < 2; n++)
        {
            additionalStopTime = (n == 1) ? 0 : stopTime;
            // Itera sobre todos os vértices
            for (int v2 = 1; v2 < numVertices; v2++)
            {
                if (anterior1 == v2 || proximo1 == v2 || v1 == v2)
                    continue;

                score_v2 = (additionalStopTime == stopTime) ? vertexScores[v2] : vertexScores[v2] / 3;

                if (best_swap >= -score_v1 + score_v2 || score - score_v1 + score_v2 <= score)
                    continue; // ja tem a melhor swap

                dist1_add_v2 = costMatrix[anterior1][v2]; // Arestas que serão adicionadas
                dist2_add_v2 = costMatrix[v2][proximo1];

                impacto1 = -dist1_remove_v1 - dist2_remove_v1 - vertice_parada_v1 + dist1_add_v2 + dist2_add_v2 + additionalStopTime;

                // Verifica se o novo custo ultrapassa o T_max
                if (!Utils::doubleLessOrEqual(cost + impacto1, maxTime))
                {
                    // cout << "EXCEDEU TEMPO LIMITE" << endl;
                    continue; // Custo ultrapassa o T_max, não tem como fazer o swap
                }

                double impacto1_if_equals = 0;
                bool local_visita = false;
                if (visitedVertices[v2].empty())
                {
                    local_visita = true;
                }
                else
                {
                    auto it = visitedVertices[v2].lower_bound(
                        arrivalTimes[i - 1] + dist1_add_v2 + additionalStopTime + protectionTime);
                    if (it == visitedVertices[v2].end())
                    {
                        auto it_prev = prev(it);
                        if (Utils::doubleLessOrEqual(it_prev->first, arrivalTimes[i - 1] + dist1_add_v2))
                        {
                            local_visita = true;
                        }
                        else
                        {
                            local_visita = false;
                        }
                    }
                    else if (it == visitedVertices[v2].begin())
                    {
                        if (it->second == id)
                            impacto1_if_equals += impacto1;
                        if (Utils::doubleLessOrEqual(arrivalTimes[i - 1] + dist1_add_v2 + additionalStopTime + protectionTime, it->first - protectionTime + impacto1_if_equals))
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
                        if (it->second == id)
                            impacto1_if_equals += impacto1;
                        auto it_prev = prev(it);
                        if (Utils::doubleLessOrEqual(arrivalTimes[i - 1] + dist1_add_v2 + additionalStopTime + protectionTime, it->first - protectionTime + impacto1_if_equals) &&
                            Utils::doubleLessOrEqual(it_prev->first, arrivalTimes[i - 1] + dist1_add_v2))
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
                    possibilidade_visita = (Utils::doubleGreaterOrEqual(timeWindows[i + 1].first, impacto1 * -1));
                }
                else
                {
                    possibilidade_visita = (Utils::doubleGreaterOrEqual(timeWindows[i + 1].second, impacto1));
                }

                if (possibilidade_visita)
                {
                    // Dados do vértice removido da rota
                    swap_out[0][0] = v1;                                                                                      // ID do vértice v1 que será removido
                    swap_out[0][1] = score_v1;                                                                                // Score do vértice
                    swap_out[0][2] = costMatrix[anterior1][proximo1] - dist1_remove_v1 - dist2_remove_v1 - vertice_parada_v1; // Alteração no custo total ao remover o vértice
                    swap_out[0][3] = i;                                                                                       // Posição do vértice na rota original

                    // Dados do vértice inserido na rota
                    swap_out[1][0] = v2;                                                                                 // ID do vértice que será inserido
                    swap_out[1][1] = score_v2;                                                                           // Score do vértice
                    swap_out[1][2] = dist1_add_v2 + dist2_add_v2 + additionalStopTime - costMatrix[anterior1][proximo1]; // Alteração no custo total ao adicionar o vértice
                    swap_out[1][3] = i;                                                                                  // Posição onde o vértice será inserido na rota
                    swap_out[1][4] = arrivalTimes[i - 1] + dist1_add_v2 + additionalStopTime;                            // Novo tempo de visita
                    swap_out[1][5] = (additionalStopTime == stopTime) ? 1 : 0;                                           // Indica se o vértice é uma parada

                    if (isBest)
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
            if (!isBest && swap_out[0][0] != -1)
                break;
        }
        if (!isBest && swap_out[0][0] != -1)
            break;
    }
    if (swap_out[0][0] == -1)
    {
        return false;
    }
    else
    {
        // cout << "Rota: " << rota.id << " - Vertice[" << swap_out[0][3] << "] = " << swap_out[0][0] << " (SAI) " << " |Swap| Vertice[" << swap_out[0][3] << "] = " << swap_out[1][0] << " (ENTRA) " << endl;

        excludeVertex(swap_out[0], visitedVertices, solutionScore, solutionCost);
        insertVertex(swap_out[1], visitedVertices, solutionScore, solutionCost);
        return true;
    }
}

bool Route::interRouteSwap(const vector<double> &vertexScores, vector<map<double, int>> &visitedVertices, double &solutionScore, double &solutionCost, Route &route2, int i_inicial, int i_final, bool &isBest)
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
        anterior1 = this->vertexSequence[i - 1];
        proximo1 = this->vertexSequence[i + 1];
        score_r1 = (this->isStopVertex[i] == 1) ? vertexScores[this->vertexSequence[i]] : vertexScores[this->vertexSequence[i]] / 3;

        dist1_remove_r1 = this->costMatrix[anterior1][this->vertexSequence[i]]; // arestas que serão removidas da rota 1
        dist2_remove_r1 = this->costMatrix[this->vertexSequence[i]][proximo1];
        vertice_parada_r1 = (this->isStopVertex[i] == 1) ? stopTime : 0; // se tiver parada, plus de 15 minutos
        // cout << "**** Rota: "<<rota1.id<<" - Vertice[" << i << "] = "<< rota1.route[i] <<" | "<< i <<" de " << rota1.route.size()-1<<" ****"<<endl;
        for (int j = 1; j < route2.vertexSequence.size() - 1; j++)
        {
            anterior2 = route2.vertexSequence[j - 1];
            proximo2 = route2.vertexSequence[j + 1];
            score_r2 = (route2.isStopVertex[j] == 1) ? vertexScores[route2.vertexSequence[j]] : vertexScores[route2.vertexSequence[j]] / 3;
            // cout << "Rota: " << rota2.id << " - Vertice[" << j << "] = " << rota2.route[j] << " | " << j << " de " << rota2.route.size()-1 << endl;

            if (anterior1 == route2.vertexSequence[j] || proximo1 == route2.vertexSequence[j] || anterior2 == this->vertexSequence[i] || proximo2 == this->vertexSequence[i])
            {
                // cout << "Vertice repetido" << endl;
                continue; // nao pode ter o mesmo vertice em seguida
            }

            vertice_parada_r2 = (route2.isStopVertex[j] == 1) ? stopTime : 0; // se tiver parada, plus de 15 minutos

            dist3_remove_r2 = route2.costMatrix[anterior2][route2.vertexSequence[j]]; // arestas que serão removidas da rota 2
            dist4_remove_r2 = route2.costMatrix[route2.vertexSequence[j]][proximo2];

            dist1_add_r1 = this->costMatrix[anterior1][route2.vertexSequence[j]]; // arestas adicionadas na rota1
            dist2_add_r1 = this->costMatrix[route2.vertexSequence[j]][proximo1];

            dist3_add_r2 = route2.costMatrix[anterior2][this->vertexSequence[i]]; // arestas adicionadas na rota2
            dist4_add_r2 = route2.costMatrix[this->vertexSequence[i]][proximo2];

            impacto1 = dist1_add_r1 + dist2_add_r1 + vertice_parada_r2 - dist1_remove_r1 - dist2_remove_r1 - vertice_parada_r1;
            impacto2 = dist3_add_r2 + dist4_add_r2 + vertice_parada_r1 - dist3_remove_r2 - dist4_remove_r2 - vertice_parada_r2;

            if (this->cost + impacto1 > maxTime || route2.cost + impacto2 > maxTime)
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
            map<double, int> aux_visited_vertice_j = visitedVertices[route2.vertexSequence[j]];
            auto it = aux_visited_vertice_j.find(route2.arrivalTimes[j] + protectionTime);
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
                it = aux_visited_vertice_j.lower_bound(this->arrivalTimes[i - 1] + dist1_add_r1 + vertice_parada_r2 + protectionTime);
                if (it == aux_visited_vertice_j.end())
                {
                    auto it_prev = prev(it);
                    if (it_prev->second == this->id && it_prev->first - protectionTime > this->arrivalTimes[i])
                        impacto1_if_equals += impacto1;
                    if (it_prev->second == route2.id && it_prev->first - protectionTime > route2.arrivalTimes[j])
                        impacto1_if_equals += impacto2;
                    if (Utils::doubleLessOrEqual(it_prev->first + impacto1_if_equals, this->arrivalTimes[i - 1] + dist1_add_r1))
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
                    if (it->second == this->id && it->first - protectionTime > this->arrivalTimes[i])
                        impacto1_if_equals += impacto1;
                    if (it->second == route2.id && it->first - protectionTime > route2.arrivalTimes[j])
                        impacto1_if_equals += impacto2;
                    if (Utils::doubleLessOrEqual(this->arrivalTimes[i - 1] + dist1_add_r1 + vertice_parada_r2 + protectionTime, it->first - protectionTime + impacto1_if_equals))
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
                    if (it->second == this->id && it->first - protectionTime > this->arrivalTimes[i])
                        impacto1_if_equals += impacto1;
                    if (it->second == route2.id && it->first - protectionTime > route2.arrivalTimes[j])
                        impacto1_if_equals += impacto2;
                    auto it_prev = prev(it);
                    if (Utils::doubleLessOrEqual(this->arrivalTimes[i - 1] + dist1_add_r1 + vertice_parada_r2 + protectionTime, it->first - protectionTime + impacto1_if_equals) && Utils::doubleLessOrEqual(it_prev->first, this->arrivalTimes[i - 1] + dist1_add_r1))
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
            map<double, int> aux_visited_vertice_i = visitedVertices[this->vertexSequence[i]];
            it = aux_visited_vertice_i.find(this->arrivalTimes[i] + protectionTime);
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
                it = aux_visited_vertice_i.lower_bound(route2.arrivalTimes[j - 1] + dist3_add_r2 + vertice_parada_r1 + protectionTime);
                if (it == aux_visited_vertice_i.end())
                {
                    auto it_prev = prev(it);
                    if (it_prev->second == this->id && it_prev->first - protectionTime > this->arrivalTimes[i])
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
                    if (it->second == this->id && it->first - protectionTime > this->arrivalTimes[i])
                        impacto2_if_equals += impacto1;
                    if (it->second == route2.id && it->first - protectionTime > route2.arrivalTimes[j])
                        impacto2_if_equals += impacto2;
                    if (Utils::doubleLessOrEqual(route2.arrivalTimes[j - 1] + dist3_add_r2 + vertice_parada_r1 + protectionTime, it->first - protectionTime + impacto2_if_equals))
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
                    if (it->second == this->id && it->first - protectionTime > this->arrivalTimes[i])
                        impacto2_if_equals += impacto1;
                    if (it->second == route2.id && it->first - protectionTime > route2.arrivalTimes[j])
                        impacto2_if_equals = impacto2;
                    auto it_prev = prev(it);
                    if (Utils::doubleLessOrEqual(route2.arrivalTimes[j - 1] + dist3_add_r2 + vertice_parada_r1 + protectionTime, it->first - protectionTime + impacto2_if_equals) && Utils::doubleLessOrEqual(it_prev->first, route2.arrivalTimes[j - 1] + dist3_add_r2))
                    {
                        local_visita = true;
                    }
                }
            }
            if (!local_visita)
                continue;

            double if_prev_equals = 0, if_next_equals = 0;

            bool possibilidade_visita = true;
            for (int a = i + 1; a < this->vertexSequence.size() - 1; a++)
            {

                int vertice = this->vertexSequence[a];

                it = visitedVertices[this->vertexSequence[a]].find(this->arrivalTimes[a] + protectionTime);
                auto it_next = it != visitedVertices[this->vertexSequence[a]].end() ? next(it) : visitedVertices[this->vertexSequence[a]].end();
                auto it_prev = it != visitedVertices[this->vertexSequence[a]].begin() ? prev(it) : visitedVertices[this->vertexSequence[a]].end();

                // auto it_next = next(it);
                // auto it_prev = prev(it);
                // if (it_next != S.visitedVertices[rota1.route[a]].end())
                if_next_equals = (route2.id == it_next->second && route2.arrivalTimes[j] <= it_next->first - protectionTime) ? impacto2 : 0;
                // if (it_prev != S.visitedVertices[rota1.route[a]].end())
                if_prev_equals = (route2.id == it_prev->second && route2.arrivalTimes[j] <= it_prev->first - protectionTime) ? impacto2 : 0;

                if (it_next != visitedVertices[this->vertexSequence[a]].end() && it->first + impacto1 > it_next->first - protectionTime + if_next_equals)
                {
                    possibilidade_visita = false;
                    break;
                }
                if (it_prev != visitedVertices[this->vertexSequence[a]].end() && it->first + impacto1 - protectionTime < it_prev->first + if_prev_equals)
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

                it = visitedVertices[route2.vertexSequence[a]].find(route2.arrivalTimes[a] + protectionTime);
                auto it_next = it != visitedVertices[route2.vertexSequence[a]].end() ? next(it) : visitedVertices[route2.vertexSequence[a]].end();
                auto it_prev = it != visitedVertices[route2.vertexSequence[a]].begin() ? prev(it) : visitedVertices[route2.vertexSequence[a]].end();

                if_next_equals = (this->id == it_next->second && this->arrivalTimes[i] <= it_next->first - protectionTime) ? impacto1 : 0;
                if_prev_equals = (this->id == it_prev->second && this->arrivalTimes[i] <= it_prev->first - protectionTime) ? impacto1 : 0;

                if (it_next != visitedVertices[route2.vertexSequence[a]].end() && it->first + impacto2 > it_next->first - protectionTime + if_next_equals)
                {
                    possibilidade_visita = false;
                    break;
                }
                if (it_prev != visitedVertices[route2.vertexSequence[a]].end() && it->first + impacto2 - protectionTime < it_prev->first + if_prev_equals)
                {
                    possibilidade_visita = false;
                    break;
                }
            }

            if (possibilidade_visita)
            {

                // exclui vertice i rota1
                swap_intra[0][0] = this->vertexSequence[i];
                swap_intra[0][1] = score_r1;
                swap_intra[0][2] = this->costMatrix[anterior1][proximo1] - dist1_remove_r1 - dist2_remove_r1 - vertice_parada_r1;
                swap_intra[0][3] = i;

                // exclui vertice j rota2
                swap_intra[1][0] = route2.vertexSequence[j];
                swap_intra[1][1] = score_r2;
                swap_intra[1][2] = route2.costMatrix[anterior2][proximo2] - dist3_remove_r2 - dist4_remove_r2 - vertice_parada_r2;
                swap_intra[1][3] = j;

                swap_intra[2][0] = route2.vertexSequence[j];
                swap_intra[2][1] = score_r2;
                swap_intra[2][2] = dist1_add_r1 + dist2_add_r1 + vertice_parada_r2 - this->costMatrix[anterior1][proximo1];
                swap_intra[2][3] = i;
                swap_intra[2][4] = this->arrivalTimes[i - 1] + dist1_add_r1 + vertice_parada_r2;
                swap_intra[2][5] = (vertice_parada_r2 == stopTime) ? 1 : 0;

                swap_intra[3][0] = this->vertexSequence[i];
                swap_intra[3][1] = score_r1;
                swap_intra[3][2] = dist3_add_r2 + dist4_add_r2 + vertice_parada_r1 - route2.costMatrix[anterior2][proximo2];
                swap_intra[3][3] = j;
                swap_intra[3][4] = route2.arrivalTimes[j - 1] + dist3_add_r2 + vertice_parada_r1;
                swap_intra[3][5] = (vertice_parada_r1 == stopTime) ? 1 : 0;

                if (isBest)
                {
                    best_swap = impacto1 + impacto2;
                }
                else
                {
                    break;
                }
            }
        }
        if (!isBest && swap_intra[0][0] != -1)
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
        this->excludeVertex(swap_intra[0], visitedVertices, solutionScore, solutionCost);
        route2.excludeVertex(swap_intra[1], visitedVertices, solutionScore, solutionCost);

        // Adicionando
        this->insertVertex(swap_intra[2], visitedVertices, solutionScore, solutionCost);
        route2.insertVertex(swap_intra[3], visitedVertices, solutionScore, solutionCost);

        // cout << "Depois: "<<endl<<rota1<<endl<< rota2<<endl;
        return true;
    }
}

bool Route::twoOpt()
{
    // cout << "two_opt | Rota: " << id << endl;
    // cout << "custo: " << cost << endl;
    // cout << " antes rota: ";
    // cout << "Veiculo " << id << " - Score: " << score << " - Custo: " << cost << " Rota: [";
    // for (int v = 0; v < vertexSequence.size(); v++)
    // {
    //     cout << "[" << v << "]:";
    //     if (isStopVertex[v] == true)
    //     {
    //         cout << "(*" << vertexSequence[v] << "*)"
    //              << ", ";
    //     }
    //     else if (v + 1 == vertexSequence.size())
    //     {
    //         cout << "(" << vertexSequence[v] << ")";
    //     }
    //     else
    //     {
    //         cout << "(" << vertexSequence[v] << ")"
    //              << ", ";
    //     }
    // }
    // cout << "]";
    // cout << endl;
    double best_custo = cost;
    double best_impacto = 0;
    vector<double> best_impacto_vector;
    vector<int> new_route;
    vector<bool> new_paradas;
    int best_i, best_j;
    for (int i = 1; i < vertexSequence.size() - 2; i++)
    {
        for (int j = i + 3; j < vertexSequence.size() - 1; j++)
        {
            // Cálculo das distâncias removidas
            double dist_remove1 = costMatrix[vertexSequence[i]][vertexSequence[i + 1]];
            double dist_remove2 = costMatrix[vertexSequence[j]][vertexSequence[j + 1]];

            // Cálculo das novas distâncias
            double dist_add1 = costMatrix[vertexSequence[i]][vertexSequence[j]];
            double dist_add2 = costMatrix[vertexSequence[i + 1]][vertexSequence[j + 1]];
            vector<int> aux_rota = vertexSequence;
            vector<bool> aux_paradas = isStopVertex;
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
                dist_remove = -costMatrix[vertexSequence[k]][vertexSequence[k + 1]] - isStopVertex[k + 1] * stopTime;
                // cout << "dist_remove: " << dist_remove << endl;

                // cout << "k: " << k << " - [k]: " << aux_rota[k] << " - [k+1]: " << aux_rota[k + 1] << " - paradas[k]: " << aux_paradas[k] << " - paradas[k+1]: " << aux_paradas[k + 1] << endl;
                dist_add = costMatrix[aux_rota[k]][aux_rota[k + 1]] + aux_paradas[k + 1] * stopTime;
                // cout << "dist_add: " << dist_add << endl;
                impacto += dist_add + dist_remove;
                aux_impacto_vector.push_back(impacto);
            }
            // cout << "impacto: " << impacto << endl;
            // cout << "aux_impacto: " << aux_impacto_vector.size() << endl;
            if (cost + impacto >= best_custo)
            {
                continue;
            }

            bool possibilidade_visita;
            possibilidade_visita = (Utils::doubleGreaterOrEqual(timeWindows[j + 1].first, impacto * -1));
            if (!possibilidade_visita)
            {
                continue;
            }

            if (possibilidade_visita)
            {
                best_custo = cost + impacto;
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
    cout << "Veiculo " << id << " - Score: " << score << " - Custo: " << best_custo << " Rota: [";
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
    vertexSequence = new_route;
    cost = best_custo;
    return true;
}

void Route::printVisited(vector<map<double, int>> &visitedVertices, int startIndex, int endIndex) const
{
    // cout << endl
    // << "Visited vertices: " << endl;
    for (int i = startIndex; i < endIndex; i++)
    {
        // cout << "T: " << visitedVertices[i].size() << " Vertice " << i << ": [";
        for (const auto &visit : visitedVertices[i])
        {
            // cout << "(" << visit.first << ", " << visit.second << "), ";
        }
        // cout << "]" << endl;
    }
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
        else
        {
            cout << "ERRO: Não foi possível encontrar entrada para o vértice " << vertexSequence[a]
                 << " com tempo " << arrivalTimes[a] + protectionTime << " na rota " << id << endl;
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

void Route::printArrivalTimes()
{
    // cout << "Arrival times: " << endl;
    // for (int i = 0; i < arrivalTimes.size(); i++) {
    //     cout << "[" << i << "] = " << arrivalTimes[i];
    // }
    // cout << endl;
}

void Route::printTimeWindows() {
    // cout << "Veiculo " << id << " - Score: " << score << " - Custo: " << cost << endl << "Push: [";
    // for (int v = 0; v < vertexSequence.size(); v++) {
    //     cout << "(" << timeWindows[v].first << ", " << timeWindows[v].second << "),";
    // }
    // cout << "]" << endl;
}

ostream &operator<<(ostream &os, const Route &route) {
    // os <<endl<< "Veiculo " << route.id << " - Score: " << route.score << " - Custo: " << route.cost << " - Velocidade: " << route.speed << "km/h Rota: [";
    for (int v = 0; v < route.vertexSequence.size(); v++) {
        // os << "[" << v<<"]:";
        if (route.isStopVertex[v] == true) {
            // os << "(*" << route.vertexSequence[v] << "*)"
            // << ", ";
        } else if (v + 1 == route.vertexSequence.size()) {
            // os << "(" << route.vertexSequence[v] << ")";
        } else {
            // os << "(" << route.vertexSequence[v] << ")"
            // << ", ";
        }
    }
    // os << "]";
    return os;
}

bool Route::operator<(const Route &c) const {
    if (cost == c.cost) {
        return id > c.id; // Adiciona comparação pelo ID para garantir ordem consistente
    }
    return cost > c.cost;
}
