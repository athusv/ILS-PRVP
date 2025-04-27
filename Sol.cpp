#include "Sol.h"
#include <cmath>
#include "Utils.h"

using namespace std;

Sol::Sol(const Instance &grafo) {
    // cout << "Iniciando a construção do objeto Sol..." << endl;
    visited_vertices.resize(grafo.qt_vertices);
    // cout << "Tamanho de visited_vertices ajustado para: " << grafo.qt_vertices << endl;

    // Todos já visitaram o vértice 0 (início)
    int id = 0;
    // cout << "Inicializando rotas para cada tipo de veículo..." << endl;
    for (int i = 0; i < grafo.tipo_veiculo.size(); i++) {
        // cout << "Tipo de veículo " << i << " com " << grafo.tipo_veiculo[i] << " unidades." << endl;
        for (int j = 0; j < grafo.tipo_veiculo[i]; j++) {
            id++;
            Caminho aux_rota(id, grafo.t_prot, grafo.t_parada, i, grafo.velocidade[i], grafo);
            rotas.push(aux_rota);
            // cout << "Rota adicionada para veículo do tipo " << i << ", unidade " << j << "." << endl;
            improved_rotas[i] = 0;
            teste_rotas[i] = 0;
        }
    }
    // cout << "Construção do objeto Sol concluída." << endl;
}

void Sol::print_visited(int inicio, int final) const{
    cout << endl << "Visited vertices: " << endl;
    for (int i = inicio; i < final; i++) {
        cout <<"T: " << visited_vertices[i].size() << " Vertice " << i << ": [";
        for (const auto &visita: visited_vertices[i]) {
            cout << "(" << visita.first << ", " << visita.second << "), ";
        }
        cout << "]" << endl;
    }
}

bool Sol::checa_rota(Instance &grafo, string &chamou) {
    bool achou = true;
    priority_queue<Caminho> rotasCopy = rotas;
    while (!rotasCopy.empty()) {
        Caminho rota = rotasCopy.top();
        rotasCopy.pop();
        for (int i = 1; i < rota.route.size() - 1; i++) {
            auto it = visited_vertices[rota.route[i]].find(rota.visita_custo[i] + grafo.t_prot);
            if (it == visited_vertices[rota.route[i]].end()) {
                cout << chamou << endl;
                auto it_find = visited_vertices[rota.route[i]].upper_bound(rota.visita_custo[i] + grafo.t_prot);
                cout << "Vertice["<<i<<"]: " << rota.route[i] << " nao encontrado na Rota: " << rota.id << endl;
                cout << "Visita_custo: [" << i << "]: " << rota.visita_custo[i] + grafo.t_prot << " | Vertice[" << rota.route[i] << "]: Id = " << it_find->second << ", custo = " << it_find->first << endl;
                cout << rota << endl << endl;

                return false;
            }
        }
    }
    return achou;
}

bool Sol::checa_visited(const Instance &grafo, string &chamou) const {
    bool check = true;
    for (int i = 1; i < grafo.qt_vertices; i++) {
        if (visited_vertices[i].empty() || visited_vertices[i].size() == 1) {
            continue;
        }

        auto it1 = visited_vertices[i].begin();
        auto it2 = next(it1);

        while (it2 != visited_vertices[i].end()) {
            if (it1->first > it2->first - grafo.t_prot)
            {

                cout << endl << "Visited - " << chamou << ": Vertice " << i << " - Rota " << it1->second << endl;
                cout << "Vertice: " << i << " Diferença: " << it2->first - it1->first << " | Ideal: "<< grafo.t_prot <<endl;
                cout << "[" << distance(visited_vertices[i].begin(), it1) << "] Rota: " << it1->second <<
                        " - Custo: " << it1->first << endl;
                cout << "[" << distance(visited_vertices[i].begin(), it2) << "] Rota: " << it2->second <<
                        " - Custo: " << it2->first << endl;
                
                print_visited(i, i+1);
                
                check = false;
            }
            ++it1;
            ++it2;
        }
    }
    return check;
}

bool Sol::checa_score(Instance &grafo, string &chamou) {
    priority_queue<Caminho> rotasCopy = rotas;
    while (!rotasCopy.empty()) {
        Caminho rota = rotasCopy.top();
        rotasCopy.pop();
        double aux_score = 0;
        for (int i = 0; i < rota.route.size(); i++) {
            double s;
            if (rota.paradas[i] == 1) {
                s = grafo.score_vertices[rota.route[i]];
            } else {
                s = grafo.score_vertices[rota.route[i]]/3;
            }
            aux_score += s;
        }
        if (!Utils::doubleEquals(aux_score, rota.score))
        {
            cout << chamou << endl;
            cout << "Rota " << rota.id << " - Score incompativel | aux_score = " << aux_score << " - Score = " << rota.score << endl;
            return false;
        }
    }
    return true;
}

bool Sol::checa_custo(Instance &grafo, string &chamou) {
    double total_custo = 0;
    priority_queue<Caminho> rotasCopy = rotas;
    while (!rotasCopy.empty()) {
        Caminho rota = rotasCopy.top();
        rotasCopy.pop();
        double aux_custo = 0;
        for (int i = 1; i < rota.route.size(); i++) {
            aux_custo += rota.distancia_matriz[rota.route[i - 1]][rota.route[i]];
            if (rota.paradas[i] == 1) {
                aux_custo += grafo.t_parada;
            }
        }
        
        if(!Utils::doubleEquals(aux_custo, rota.custo)){
            cout << chamou<<endl;
            cout<< "Rota " << rota.id<< " - Custo incompativel | aux_custo = "<< aux_custo << " - Custo = "<< rota.custo<<endl;
            return false;
        }
        if(!Utils::doubleLessOrEqual(rota.custo, grafo.t_max)){
            cout << chamou << endl;
            cout << "Rota " << rota.id << " Ultrapassou T_max | T_max = "<<grafo.t_max<< " Custo = " << rota.custo << endl;
            return false;
        }
        total_custo += aux_custo;
    }
    return true;
}
bool Sol::checa_push(Instance &grafo, string &chamou){
    priority_queue<Caminho> rotasCopy = rotas;
    while (!rotasCopy.empty())
    {
        Caminho rota = rotasCopy.top();
        rotasCopy.pop();
        for (int i = rota.push_hotspots.size() - 2; i >= 0; i--)
        {
            if (i == 0)
            {
                if(rota.push_hotspots[i].first != rota.push_hotspots[i + 1].first) return false;
                if(rota.push_hotspots[i].second != rota.push_hotspots[i + 1].second) return false;
                break;
            }
            double folga_puxar;
            double folga_empurrar;

            auto it = visited_vertices[rota.route[i]].find(rota.visita_custo[i] + grafo.t_prot);
            auto next_it = next(it);
            auto prev_it = prev(it);
            folga_puxar = (it == visited_vertices[rota.route[i]].begin()) ? 999 : (it->first - grafo.t_prot) - prev_it->first;
            // folga_puxar = (it->first - grafo.t_prot) - prev_it->first;
            folga_empurrar = (next_it == visited_vertices[rota.route[i]].end()) ? 999 : (next_it->first - grafo.t_prot) - it->first;

            double min_puxar = (Utils::doubleLessOrEqual(rota.push_hotspots[i + 1].first, folga_puxar)) ? rota.push_hotspots[i + 1].first : folga_puxar;
            if (!Utils::doubleEquals(rota.push_hotspots[i].first, min_puxar)){
                cout << chamou << endl;
                return false;
            }

            double min_empurrar = (Utils::doubleLessOrEqual(rota.push_hotspots[i + 1].second, folga_empurrar)) ? rota.push_hotspots[i + 1].second : folga_empurrar;
            if (!Utils::doubleEquals(rota.push_hotspots[i].second, min_empurrar)){
                cout << chamou << endl;
                return false;
            }

            if (min_puxar < 0 || min_empurrar < 0)
            {
                cout << endl
                     << "Push_rotas - " << chamou << endl<<" Rota: " << rota.id << " - Vertice: " << rota.route[i] << endl;
                cout << "Folga_puxar: " << min_puxar << "|" << rota.push_hotspots[i].first << " - Folga_empurrar: " << min_empurrar << "|" << rota.push_hotspots[i].second << endl;;
                print_visited(rota.route[i], rota.route[i] + 1);
                cout<<rota<<endl;
                return false;
            }
        
        }
    }
    return true;
}

bool Sol::checa_visita_custo(Instance &grafo, string &chamou){
    priority_queue<Caminho> rotasCopy = rotas;
    while (!rotasCopy.empty())
    {
        Caminho rota = rotasCopy.top();
        rotasCopy.pop();
        double aux_visita_custo = 0;
        for(int i = 1 ; i < rota.route.size()-1; i++){
            int plus_parada = (rota.paradas[i]) ? grafo.t_parada : 0;
            aux_visita_custo+=rota.distancia_matriz[rota.route[i-1]][rota.route[i]] + plus_parada;
            if(!Utils::doubleEquals(aux_visita_custo, rota.visita_custo[i])){
                cout << chamou << endl;
                cout << "Rota: " << rota.id << " - Vertice[" << i << "] = " << rota.route[i]<<endl;
                cout << "Visita_custo: "<<rota.visita_custo[i] << " | Aux_visita_custo: "<<aux_visita_custo<<endl;
                return false;
            }
        }
    }
    return true;
}

bool Sol::checa_solucao(Instance &grafo, string &chamou) {
    // cout << endl << "********** Validando soluçao: **********" << endl;
    assert(checa_visita_custo(grafo, chamou));
    assert(checa_score(grafo, chamou));
    assert(checa_custo(grafo, chamou));
    
    assert(checa_visited(grafo, chamou));
    assert(checa_rota(grafo, chamou));
    assert(checa_push(grafo, chamou));
    // cout << "Solução Correta!" << endl << endl;
    return true;
}

void Sol::atualiza_push(Instance &grafo) {
    priority_queue<Caminho> rotasCopy;
    while (!rotas.empty()) {
        Caminho rota = rotas.top();
        rotas.pop();
        rota.atualizar_push_hotspots(visited_vertices);
        rotasCopy.push(rota);
    }
    rotas = rotasCopy;
}
void Sol::print_solucao(Instance &grafo)
{
    priority_queue<Caminho> rotasCopy = rotas;
    cout << score << endl;
    cout << grafo.veiculos << endl;
    while (!rotasCopy.empty())
    {
        Caminho r = rotasCopy.top();
        rotasCopy.pop();
        cout << endl
             << "Viatura " << r.id << ":" << endl;
            //  << "Vertice, Tempo visita, Para_Passa" << endl;
        for (int i = 0; i < r.route.size(); i++)
        {
            if (i == 0)
            {
                cout << "Base," << r.visita_custo[i] << "," << r.paradas[i] << endl;
                continue;
            }else if(i == r.route.size()-1){
                cout << "Base," << r.visita_custo[i] << "," << r.paradas[i] << endl;
                continue;
            }
            cout << r.route[i] << "," << r.visita_custo[i] << "," << r.paradas[i] << endl;
        }
    }
    cout <<endl;
    // print_visited(0, grafo.qt_vertices);
}

bool Sol::operator<(const Sol &s) const {
    return score > s.score;
}

ostream &operator<<(ostream &os, const Sol &sol) {
    priority_queue<Caminho> rotasCopy = sol.rotas;
    os << "Score: " << sol.score << ", Custo: " << sol.custo << endl;
    while (!rotasCopy.empty()) {
        os << rotasCopy.top() << endl;
        rotasCopy.pop();
    }
    return os;
}
