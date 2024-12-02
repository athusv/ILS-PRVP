#include "Caminho.h"
#include "Utils.h"

Caminho::Caminho(int id, int t_prot, int t_parada) {
    this->id = id;
    this->t_prot = t_prot;
    this->t_parada = t_parada;
}

void Caminho::incert(vector<double> &best_insert, vector<map<double, int> > &visited_vertices, double &score_s,
                     double &custo_s) {
    // id, score, impacto, local insert, visita_custo, para ou nao

    score += best_insert[1];
    score_s += best_insert[1]; // somando score
    custo += best_insert[2];
    custo_s += best_insert[2]; // somando impacto

    route.insert(route.begin() + best_insert[3], static_cast<int>(best_insert[0])); // inserindo vertice
    paradas.emplace(paradas.begin() + best_insert[3], best_insert[5]);
    visita_custo.insert(visita_custo.begin() + best_insert[3], best_insert[4]);
    push_hotspots.insert(push_hotspots.begin() + best_insert[3], {999, 999});

    // Inserindo informações na tabela de vértices visitados
    visited_vertices[static_cast<int>(best_insert[0])][best_insert[4] + t_prot] = id;

    // Atualizando a tabela de vértices visitados para vértices subsequentes na rota
    atualizar_visited_vertices(best_insert[3] + 1, route.size() - 1, best_insert[2], visited_vertices);
    atualizar_push_hotspots(visited_vertices);
}

void Caminho::excluir(vector<double> &exclui_vertice, vector<map<double, int>> &visited_vertices, double &score_s, double &custo_s) 
{
    int indice = static_cast<int>(exclui_vertice[3]);
    double score_vertice = exclui_vertice[1];
    // id, score, impacto, indice_rota, indice_visited
    score -= score_vertice; // Convertendo para inteiro
    score_s -= score_vertice;
    custo += exclui_vertice[2];
    custo_s += exclui_vertice[2];

    auto it = visited_vertices[static_cast<int>(exclui_vertice[0])].find(visita_custo[indice] + t_prot);
    visited_vertices[static_cast<int>(exclui_vertice[0])].erase(it);
    // cout << "vertice indicado - " << route[indice] << endl;
    route.erase(route.begin() + indice);
    paradas.erase(paradas.begin() + indice);
    visita_custo.erase(visita_custo.begin() + indice);
    push_hotspots.erase(push_hotspots.begin() + indice);
    atualizar_visited_vertices(indice, route.size() - 1, exclui_vertice[2], visited_vertices);
    atualizar_push_hotspots(visited_vertices);
}

void Caminho::parar(vector<double> vertice_parar, vector<map<double, int> > &visited_vertices, double &score_s, double &custo_s) {
    custo += t_parada;
    custo_s += t_parada;

    //    pontuaçao parcial
    score -= vertice_parar[1];
    score_s -= vertice_parar[1];

    //   pontuaçao_completa
    score += vertice_parar[2];
    score_s += vertice_parar[2];

    paradas[vertice_parar[0]] = 1;

    atualizar_visited_vertices(static_cast<int>(vertice_parar[0]), route.size() - 1, t_parada, visited_vertices);
    atualizar_push_hotspots(visited_vertices);
}

void Caminho::passar(vector<double> vertice_passa, vector<map<double, int> > &visited_vertices, double &score_s, double &custo_s) {
    custo -= t_parada;
    custo_s -= t_parada;
    //     pontuaçao_parcial
    score += vertice_passa[1];
    score_s += vertice_passa[1];

    //     pontuaçao_completa
    score -= vertice_passa[2];
    score_s -= vertice_passa[2];

    paradas[vertice_passa[0]] = 0;
    visita_custo[vertice_passa[0]] -= t_parada;

    atualizar_visited_vertices(vertice_passa[0], route.size() - 1, -t_parada, visited_vertices);
    atualizar_push_hotspots(visited_vertices);
}

void Caminho::atualizar_visited_vertices(int start_index, int final_index, double impacto, vector<map<double, int> > &visited_vertices) {
    for (int a = start_index; a < final_index; a++) {
        auto it = visited_vertices[route[a]].find(visita_custo[a] + t_prot);
        if (it != visited_vertices[route[a]].end()) {
            visited_vertices[route[a]].erase(it);
        }

        visita_custo[a] += impacto;
        visited_vertices[route[a]][visita_custo[a] + t_prot] = id;
    }
}

void Caminho::atualizar_push_hotspots(vector<map<double, int>> &visited_vertices) {
    double folga_puxar;
    double folga_empurrar;
    for (int i = push_hotspots.size() - 2; i >= 0; i--) {
        if (i == 0) {
            push_hotspots[i].first = push_hotspots[i + 1].first;
            push_hotspots[i].second = push_hotspots[i + 1].second;
            break;
        }
        int vertice = route[i];

        auto it = visited_vertices[route[i]].find(visita_custo[i] + t_prot);

        auto next_it = next(it);
        auto prev_it = prev(it);

        folga_puxar = (it == visited_vertices[route[i]].begin()) ? 999 : (it->first - t_prot) - prev_it->first;
        // folga_puxar = (it->first - grafo.t_prot) - prev_it->first;
        folga_empurrar = (next_it == visited_vertices[route[i]].end()) ? 999 : (next_it->first - t_prot) - it->first;

        push_hotspots[i].first = (Utils::doubleLessOrEqual(push_hotspots[i + 1].first, folga_puxar)) ? push_hotspots[i+1].first : folga_puxar;
        push_hotspots[i].second = (Utils::doubleLessOrEqual(push_hotspots[i + 1].second, folga_empurrar)) ? push_hotspots[i+1].second :folga_empurrar;
        // push_hotspots[i].first = min(push_hotspots[i + 1].first, folga_puxar);
        // push_hotspots[i].second = min(push_hotspots[i + 1].second, folga_empurrar);
    }
}

void Caminho::print_push() {
    cout << "Veiculo " << id << " - Score: " << score << " - Custo: " << custo << endl << "Push: [";
    for (int v = 0; v < route.size(); v++) {
        cout << "(" << push_hotspots[v].first << ", " << push_hotspots[v].second << "),";
    }
    cout << "]" << endl;
}

ostream &operator<<(ostream &os, const Caminho &caminho) {
    os << "Veiculo " << caminho.id << " - Score: " << caminho.score << " - Custo: " << caminho.custo << " Rota: [";
    for (int v = 0; v < caminho.route.size(); v++) {
        os << "[" << v<<"]:";
        if (caminho.paradas[v] == true) {
            os << "(*" << caminho.route[v] << "*)"
                    << ", ";
        } else if (v + 1 == caminho.route.size()) {
            os << "(" << caminho.route[v] << ")";
        } else {
            os << "(" << caminho.route[v] << ")"
                    << ", ";
        }
    }
    os << "]";
    return os;
}

bool Caminho::operator<(const Caminho &c) const {
    if (custo == c.custo) {
        return id > c.id; // Adiciona comparação pelo ID para garantir ordem consistente
    }
    return custo > c.custo;
}
