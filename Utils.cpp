#include "Utils.h"

bool Utils::doubleEquals(double a, double b, double epsilon) {
    return fabs(a - b) < epsilon;
}

bool Utils::doubleGreaterOrEqual(double a, double b, double epsilon) {
    return (a > b) || doubleEquals(a, b, epsilon);
}

bool Utils::doubleLessOrEqual(double a, double b, double epsilon) {
    return (a < b) || doubleEquals(a, b, epsilon);
}

int Utils::max_score(const std::vector<int>& lista_candidatos, const std::vector<double>& score_vertices) {
    double max = -1;
    int index_max = 0;
    for (auto candidato : lista_candidatos) {
        if (score_vertices[candidato] > max) {
            max = score_vertices[candidato];
            index_max = candidato;
        }
    }
    return index_max;
}

int Utils::min_custo(const std::vector<int> &lista_candidatos, const vector<vector<double>> &distancia_matriz, int &last_vertice){
    double min = distancia_matriz[last_vertice][lista_candidatos[0]];
    int index_min = lista_candidatos[0];
    for (auto candidato : lista_candidatos){
        if (distancia_matriz[last_vertice][candidato] < min){
            min = distancia_matriz[last_vertice][candidato];
            index_min = candidato;
        }
    }
    return index_min;
}

int Utils::cost_benefit(const std::vector<int> &lista_candidatos,const vector<double>& score_vertices, const vector<vector<double>> &distancia_matriz, int &last_vertice){
    double benefit = -1;
    int index_benefit = 0;
    for (auto candidato : lista_candidatos){
        if (score_vertices[candidato]/distancia_matriz[last_vertice][candidato] > benefit){
            benefit = score_vertices[candidato] / distancia_matriz[last_vertice][candidato];
            index_benefit = candidato;
        }
    }
    return index_benefit;
}

std::vector<int> Utils::make_lista(Instance& grafo, Caminho& rota, std::vector<std::map<double, int>>& visited_vertices) {
    std::vector<int> lista_de_candidatos;
    for (int i = 1; i < grafo.qt_vertices; i++) {
        if (visited_vertices[i].empty() || doubleLessOrEqual(visited_vertices[i].rbegin()->first, rota.custo)) {
            double distancia_1 = rota.distancia_matriz[rota.route.back()][i];
            double distancia_2 = rota.distancia_matriz[i][0];
            if (rota.custo + distancia_1 + distancia_2 + rota.plus_parada < grafo.t_max) {
                lista_de_candidatos.push_back(i);
            }
        }
    }
    return lista_de_candidatos;
}

std::vector<double> Utils::p_excluir(Instance& grafo, std::vector<std::map<double, int>>& visited_vertices, Caminho& rota, int i) {
    std::vector<double> exclui_vertice = {-1, -1, -1, -1};
    // cout << "p_excluir" << endl;
    double dist1 = rota.distancia_matriz[rota.route[i - 1]][rota.route[i]];
    double dist2 = rota.distancia_matriz[rota.route[i]][rota.route[i + 1]];
    double dist3 = rota.distancia_matriz[rota.route[i - 1]][rota.route[i + 1]];

    double impacto = dist3 - dist2 - dist1;
    if (rota.paradas[i] == 1)
        impacto -= grafo.t_parada;

    if (rota.custo + impacto > grafo.t_max)
        return exclui_vertice;

    bool possibilidade_visita;
    if (impacto < 0) {
        possibilidade_visita = (Utils::doubleGreaterOrEqual(rota.push_hotspots[i].first, impacto * -1));
    } else {
        possibilidade_visita = (Utils::doubleGreaterOrEqual(rota.push_hotspots[i].second, impacto));
    }

    if (possibilidade_visita) {
        exclui_vertice[0] = rota.route[i];
        exclui_vertice[1] = (rota.paradas[i] == 1)
                                ? grafo.score_vertices[rota.route[i]]
                                : grafo.score_vertices[rota.route[i]]/3;
        exclui_vertice[2] = impacto;
        exclui_vertice[3] = i;
    }
    return exclui_vertice;
}

//Excluir e adaptar a função q ja existe
bool Utils::swap_perturbacao(Instance &grafo, Sol &S, Caminho &rota, int i)
{
    vector<vector<double>> swap(2, vector<double>(6, -1));
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

    int v1 = rota.route[i];
    anterior1 = rota.route[i - 1];
    proximo1 = rota.route[i + 1];
    score_v1 = (rota.paradas[i]) ? grafo.score_vertices[v1] : grafo.score_vertices[v1]/3;

    dist1_remove_v1 = rota.distancia_matriz[anterior1][v1]; // Arestas que serão removidas V1
    dist2_remove_v1 = rota.distancia_matriz[v1][proximo1];
    vertice_parada_v1 = (rota.paradas[i]) ? grafo.t_parada : 0; // Se tiver parada, plus de 15 minutos
    for (int n = 0; n < 2; n++)
    {
        rota.plus_parada = (n == 1) ? 0 : grafo.t_parada;
        // Itera sobre todos os vértices
        for (int v2 = 1; v2 < grafo.qt_vertices; v2++)
        {
            if (anterior1 == v2 || proximo1 == v2 || v1 == v2)
                continue;

            score_v2 = (rota.plus_parada == grafo.t_parada) ? grafo.score_vertices[v2] : grafo.score_vertices[v2] / 3;

            dist1_add_v2 = rota.distancia_matriz[anterior1][v2]; // Arestas que serão adicionadas
            dist2_add_v2 = rota.distancia_matriz[v2][proximo1];

            impacto1 = -dist1_remove_v1 - dist2_remove_v1 - vertice_parada_v1 + dist1_add_v2 + dist2_add_v2 + rota.plus_parada;

            // Verifica se o novo custo ultrapassa o T_max
            if (!Utils::doubleLessOrEqual(rota.custo + impacto1, grafo.t_max))
            {
                // cout << "EXCEDEU TEMPO LIMITE" << endl;
                continue; // Custo ultrapassa o T_max, não tem como fazer o swap
            }

            double impacto1_if_equals = 0;
            bool local_visita = false;
            if (S.visited_vertices[v2].empty())
            {
                local_visita = true;
            }
            else
            {
                auto it = S.visited_vertices[v2].lower_bound(
                    rota.visita_custo[i - 1] + dist1_add_v2 + rota.plus_parada + grafo.t_prot);
                if (it == S.visited_vertices[v2].end())
                {
                    auto it_prev = prev(it);
                    if (Utils::doubleLessOrEqual(it_prev->first, rota.visita_custo[i - 1] + dist1_add_v2))
                    {
                        local_visita = true;
                    }
                    else
                    {
                        local_visita = false;
                    }
                }
                else if (it == S.visited_vertices[v2].begin())
                {
                    if (it->second == rota.id) impacto1_if_equals += impacto1;
                    if (Utils::doubleLessOrEqual(rota.visita_custo[i - 1] + dist1_add_v2 + rota.plus_parada + grafo.t_prot, it->first - grafo.t_prot + impacto1_if_equals))
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
                    if (it->second == rota.id)
                        impacto1_if_equals += impacto1;
                    auto it_prev = prev(it);
                    if (Utils::doubleLessOrEqual(rota.visita_custo[i - 1] + dist1_add_v2 + rota.plus_parada + grafo.t_prot, it->first - grafo.t_prot + impacto1_if_equals) &&
                        Utils::doubleLessOrEqual(it_prev->first, rota.visita_custo[i - 1] + dist1_add_v2))
                    {
                        local_visita = true;
                    }
                }
            }

            if (!local_visita)
                continue;

            // possibilidade visita rota
            bool possibilidade_visita = true;
            if (impacto1 < 0)
            {
                possibilidade_visita = (Utils::doubleGreaterOrEqual(rota.push_hotspots[i + 1].first, impacto1 * -1));
            }
            else
            {
                possibilidade_visita = (Utils::doubleGreaterOrEqual(rota.push_hotspots[i + 1].second, impacto1));
            }

            if (possibilidade_visita)
            {
                // Dados do vértice removido da rota
                swap[0][0] = v1; // ID do vértice v1 que será removido
                swap[0][1] = score_v1; // Score do vértice
                swap[0][2] = rota.distancia_matriz[anterior1][proximo1] - dist1_remove_v1 - dist2_remove_v1 - vertice_parada_v1; // Alteração no custo total ao remover o vértice
                swap[0][3] = i;// Posição do vértice na rota original

                // Dados do vértice inserido na rota
                swap[1][0] = v2;      // ID do vértice que será inserido
                swap[1][1] = score_v2;// Score do vértice
                swap[1][2] = dist1_add_v2 + dist2_add_v2 + rota.plus_parada - rota.distancia_matriz[anterior1][proximo1]; // Alteração no custo total ao adicionar o vértice
                swap[1][3] = i;// Posição onde o vértice será inserido na rota
                swap[1][4] = rota.visita_custo[i - 1] + dist1_add_v2 + rota.plus_parada; // Novo tempo de visita
                swap[1][5] = (rota.plus_parada == grafo.t_parada) ? 1 : 0; // Indica se o vértice é uma parada

                rota.excluir(swap[0], S.visited_vertices, S.score, S.custo);
                rota.insert_v(swap[1], S.visited_vertices, S.score, S.custo);
                return true;
                // return swap;
            }
        }
    }
    return false;
    // return swap;
}

std::vector<double> Utils::p_insert(Instance& grafo, std::vector<std::map<double, int>>& visited_vertices, const Caminho& rota, int i, int vertice_insert) {
    std::vector<double> vert_incert = {-1, -1, -1, -1, -1, -1};

    double dist1 = rota.distancia_matriz[rota.route[i - 1]][vertice_insert];
    double dist2 = rota.distancia_matriz[vertice_insert][rota.route[i + 1]];
    double dist3 = rota.distancia_matriz[rota.route[i - 1]][rota.route[i]];
    double dist4 = rota.distancia_matriz[rota.route[i]][rota.route[i + 1]];

    int parada = (rota.paradas[i]) ? grafo.t_parada : 0;
    double impacto = dist1 + dist2 + grafo.t_parada - dist3 - dist4 - parada;

    if (rota.custo + impacto > grafo.t_max)
        return vert_incert;

    int local_visita = -1;
    if (visited_vertices[vertice_insert].empty()) {
        local_visita = 1;
    } else {
        auto it1 = visited_vertices[i].begin();
        auto it2 = next(it1);
        if (it2 == visited_vertices[i].end()) {
            if (it1->first <= rota.visita_custo[i - 1] + dist1) {
                local_visita = 1;
            }
            else if (it1->first - grafo.t_prot >= rota.visita_custo[i - 1] + dist1 + grafo.t_parada + grafo.t_prot)
            {
                local_visita = 1;
            }
        }
        while (it2 != visited_vertices[i].end()) {
            if (next(it2) == visited_vertices[i].end()) {
                if (it2->first <= rota.visita_custo[i - 1] + dist1)
                    local_visita = 1;
            }

            if (it2->first - grafo.t_prot >= rota.visita_custo[i - 1] + dist1 + grafo.t_parada + grafo.t_prot)
            {
                if (it1->first <= rota.visita_custo[i - 1] + dist1) {
                    local_visita = 1;
                    break;
                }
            }
            ++it1;
            ++it2;
        }
    }
    if (local_visita == -1)
        return vert_incert;

    bool possibilidade_visita;
    if (impacto < 0) {
        possibilidade_visita = (Utils::doubleGreaterOrEqual(rota.push_hotspots[i + 1].first, impacto * -1));
    } else {
        possibilidade_visita = (Utils::doubleGreaterOrEqual(rota.push_hotspots[i + 1].second, impacto));
    }

    if (possibilidade_visita) {
        vert_incert[0] = vertice_insert; // id vertice
        vert_incert[1] = grafo.score_vertices[vertice_insert]; // score vertice
        vert_incert[2] = impacto; // impacto incert vertice
        vert_incert[3] = i + 1; // Local insert rota
        vert_incert[4] = rota.visita_custo[i - 1] + dist1 + grafo.t_parada; // Visita_custo
        vert_incert[5] = local_visita;
    }

    return vert_incert;
}

