#include "Busca_local.h"
#include <iostream>
#include <limits>
#include <numeric>
#include <set>

using namespace std;

enum class Operacao
{
    Insert,
    SwapInter,
    SwapIntra,
    SwapOut,
    Para
};

Sol &Busca_local::busca_local(Instance &grafo, Sol &S, mt19937 &gen){
    std::priority_queue<Caminho> rotas_prontas;
    std::string chamou;
    bool realizou_melhora;
    bool best_improvement = true;

    vector<Operacao> operacoes = {Operacao::Insert, Operacao::SwapInter, Operacao::SwapIntra, Operacao::SwapOut, Operacao::Para};
    // cout << "Busca Local" << endl;
    while (!S.rotas.empty()) {
        Caminho rota = S.rotas.top(); //colocar para vector
        S.rotas.pop();

        realizou_melhora = false;
        // Embaralha a ordem das operações
        shuffle(operacoes.begin(), operacoes.end(), gen);
        for (const auto &operacao : operacoes)
        {
            if (operacao == Operacao::Insert){
                S.cont_vizinhanca_total["best_insert"] += 1;
                S.teste_rotas[rota.id] += 1;
                // insert
                if (Busca_local::best_insert(grafo, S, rota, best_improvement))
                {
                    S.rotas.push(rota);
                    S.atualiza_push(grafo);

                    chamou = "Busca Local - insert";

                    assert(S.checa_solucao(grafo, chamou));
                    realizou_melhora = true;
                    S.cont_vizinhanca["best_insert"]+=1;
                    S.improved_rotas[rota.id] += 1;
                    break;
                }
            }
            else if(operacao == Operacao::SwapInter){
                S.cont_vizinhanca_total["swap_inter"] += 1;
                S.teste_rotas[rota.id] += 1;
                if (swap_inter_rotas(grafo, S, rota, best_improvement))
                {
                    
                    S.rotas.push(rota);
                    S.atualiza_push(grafo);

                    chamou = "Busca Local - Swap Inter";
                    assert(S.checa_solucao(grafo, chamou));
                    realizou_melhora = true;
                    S.cont_vizinhanca["swap_inter"] += 1;
                    S.improved_rotas[rota.id] += 1;
                    break;
                }
            }
            else if(operacao == Operacao::Para){
                S.cont_vizinhanca_total["para"] += 1;
                S.teste_rotas[rota.id] += 1;
                if(para(grafo, S, rota, best_improvement)){
                    
                    S.rotas.push(rota);
                    S.atualiza_push(grafo);
                    chamou = "Busca Local - Para";
                    assert(S.checa_solucao(grafo, chamou));
                    realizou_melhora = true;
                    S.cont_vizinhanca["para"] += 1;
                    S.improved_rotas[rota.id] += 1;
                    break;
                }
            }
            else if (operacao == Operacao::SwapOut){
                // Swap vértice fora da rota
                S.cont_vizinhanca_total["swap_out"] += 1;
                S.teste_rotas[rota.id] += 1;
                if (swap_Out_rotas(grafo, S, rota, 1, rota.route.size() - 1, best_improvement))
                {   
                    S.rotas.push(rota);
                    S.atualiza_push(grafo);
                    chamou = "Busca Local - Swap Out";
                    assert(S.checa_solucao(grafo, chamou));
                    realizou_melhora = true;
                    S.cont_vizinhanca["swap_out"] += 1;
                    S.improved_rotas[rota.id] += 1;
                    break;
                }
            }
            else if(operacao == Operacao::SwapIntra){
                //fazer o classico 
                // sempre tentar trocar com todas as rotas
                //fazer um cache "caso nao seja "
                // Fazer o best_improvement classico 
                if (S.rotas.size() > 0)
                {
                    Caminho rota2 = S.rotas.top();
                    S.rotas.pop();
                    S.cont_vizinhanca_total["swap_intra"] += 1;
                    S.teste_rotas[rota.id] += 1;
                    S.teste_rotas[rota2.id] += 1;
                    if (swap_intra_rotas(grafo, S, rota, rota2, 1, rota.route.size()-1, best_improvement))
                    {
                        
                        S.rotas.push(rota);
                        S.rotas.push(rota2);
                        S.atualiza_push(grafo);
                        chamou = "Busca Local - Swap Intra";
                        assert(S.checa_solucao(grafo, chamou));
                        realizou_melhora = true;
                        S.cont_vizinhanca["swap_intra"] += 1;
                        S.improved_rotas[rota.id] += 1;
                        S.improved_rotas[rota2.id] += 1;
                        break;
                    }

                    // Se não houver melhoria, insere novamente a rota2
                    S.rotas.push(rota2);
                }
            }
        }

        if (!realizou_melhora) {
            rotas_prontas.push(rota);
        }
    }
    S.rotas = rotas_prontas;

    return S;
}

bool Busca_local::para(const Instance &grafo, Sol &S, Caminho &rota, bool &best)
{
    // O melhor vertice marcado como passada q for possivel ser parada, sera uma parada

    vector<double> vertice_para = {-1, -1, -1};
    if (rota.custo + grafo.t_parada > grafo.t_max)
    {
        return false;
    }
    
    for (int i = 1; i < rota.route.size() - 1; i++)
    {
        if (rota.paradas[i] == 1)
            continue;

        if (grafo.score_vertices[rota.route[i]] < vertice_para[1])
            continue;

        bool possibilidade_parada = (Utils::doubleGreaterOrEqual(rota.push_hotspots[i].second, grafo.t_parada));

        if (possibilidade_parada)
        {
            vertice_para = {static_cast<double>(i), grafo.score_vertices[rota.route[i]] / 3, grafo.score_vertices[rota.route[i]]};

            if(!best){
                break;
            }
        }
    }

    if(vertice_para[0] == -1){
        return false;
    }else{
        // std::cout << "Rota " << rota.id << " - Vertice[" << vertice_para[0] << "] = " << rota.route[vertice_para[0]] << " Score passada: " << vertice_para[1] << " | Score parada: "<<vertice_para[2]<<std::endl;

        rota.parar(vertice_para, S.visited_vertices, S.score, S.custo);
        return true;
    }
}

bool Busca_local::best_insert(const Instance &grafo, Sol &S, Caminho &rota, bool &best)
{
    //                 id, score, custo, local insert, custo local insert, local visita
    std::vector<double> b_vert_insert = {-1, -1, -1, -1, -1, -1};

    double dist1 = 0;
    double dist2 = 0;
    double dist3 = 0;

    for (int n = 0; n < 2; n++)
    {
        rota.plus_parada = (n == 1) ? 0 : grafo.t_parada;

        for (int i = 1; i < grafo.qt_vertices; i++)
        {
            if (grafo.score_vertices[i] < b_vert_insert[1])
                continue;

            double score_v = (rota.plus_parada == grafo.t_parada) ? grafo.score_vertices[i] : grafo.score_vertices[i]/3;

            for (int j = 0; j < rota.route.size() - 1; j++)
            {
                int anterior = rota.route[j];
                int proximo = rota.route[j + 1];

                if (anterior == i || proximo == i)
                {
                    continue;
                }

                dist1 = grafo.distancia_matriz[anterior][i];
                dist2 = grafo.distancia_matriz[i][proximo];
                dist3 = grafo.distancia_matriz[anterior][proximo];
                double impacto = dist1 + dist2 + rota.plus_parada - dist3;

                if (rota.custo + impacto > grafo.t_max)
                    continue;

                if (b_vert_insert[1] == score_v && b_vert_insert[2] < impacto)
                {
                    continue;
                }

                bool local_visita = false;
                if (S.visited_vertices[i].empty())
                {
                    local_visita = true;
                }
                else
                {
                    auto it = S.visited_vertices[i].lower_bound(
                        rota.visita_custo[j] + dist1 + rota.plus_parada + grafo.t_prot);
                    if (it == S.visited_vertices[i].end())
                    {
                        auto it_prev = prev(it);
                        if (Utils::doubleLessOrEqual(it_prev->first, rota.visita_custo[j] + dist1))
                        {
                            local_visita = true;
                        }
                        else
                        {
                            local_visita = false;
                        }
                    }
                    else if (it == S.visited_vertices[i].begin())
                    {
                        if (Utils::doubleLessOrEqual(rota.visita_custo[j] + dist1 + rota.plus_parada + grafo.t_prot, it->first - grafo.t_prot))
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
                        if (Utils::doubleLessOrEqual(rota.visita_custo[j] + dist1 + rota.plus_parada + grafo.t_prot, it->first - grafo.t_prot) &&
                            Utils::doubleLessOrEqual(
                                it_prev->first, rota.visita_custo[j] + dist1))
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
                    possibilidade_visita = (Utils::doubleGreaterOrEqual(rota.push_hotspots[j+1].first, impacto * -1));
                }
                else
                {
                    possibilidade_visita = (Utils::doubleGreaterOrEqual(rota.push_hotspots[j+1].second, impacto));
                }

                if (possibilidade_visita)
                {
                    b_vert_insert[0] = i;                                               // id vertice
                    b_vert_insert[1] = score_v;                                         // score vertice
                    b_vert_insert[2] = impacto;                                         // impacto incert vertice
                    b_vert_insert[3] = j + 1;                                           // Local insert rota
                    b_vert_insert[4] = rota.visita_custo[j] + dist1 + rota.plus_parada; // Visita_custo
                    b_vert_insert[5] = (rota.plus_parada == grafo.t_parada) ? 1 : 0;

                    if (!best)
                    {
                        break;
                    }
                }
            }
            if(!best && b_vert_insert[0]!=-1) break;
        }
        if (!best && b_vert_insert[0] != -1) break;
    }

    //realizar a mudança aqui dentro 
    if(b_vert_insert[0] == -1){
        return false;
    }else{
        // std::cout << "Rota " << rota.id << " - Best_insert: Vertice["<<b_vert_insert[3]<<"] = " << b_vert_insert[0] << " - Score: " << b_vert_insert[1] << " - Impacto: " << b_vert_insert[2] << std::endl;

        rota.incert(b_vert_insert, S.visited_vertices, S.score, S.custo);
        return true;
    }
    
}

bool Busca_local::swap_inter_rotas(Instance &grafo, Sol &S, Caminho &rota, bool &best)
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
    for(int i = 1; i <= rota.route.size()-2; i++){
        // if(best_swap!=-1) break;
        // cout<<"Vertice i["<< i << "]: "<<rota.route[i]<< endl;
        anterior1 = rota.route[i-1];
        proximo1 = rota.route[i+1];
        score_v1 = (rota.paradas[i] == 1) ? grafo.score_vertices[rota.route[i]] : grafo.score_vertices[rota.route[i]]/3 ;

        dist1_remove_v1 = grafo.distancia_matriz[anterior1][rota.route[i]]; // arestas que serão removidas V1
        dist2_remove_v1 = grafo.distancia_matriz[rota.route[i]][proximo1];
        vertice_parada_v1 = (rota.paradas[i] == 1) ? grafo.t_parada : 0; // se tiver parada, plus de 15 minutos

        for(int j = i+2;j <= rota.route.size()-2; j++){
            if(i == j) break;
            // cout << "Vertice j[" << j << "]: " << rota.route[j] << endl;
            anterior2 = rota.route[j - 1];
            proximo2 = rota.route[j + 1];
            score_v2 = (rota.paradas[j] == 1) ? grafo.score_vertices[rota.route[j]] : grafo.score_vertices[rota.route[j]] / 3;

            vertice_parada_v2 = (rota.paradas[j] == 1) ? grafo.t_parada : 0;

            dist1_remove_v2 = grafo.distancia_matriz[anterior2][rota.route[j]]; // arestas que serão removidas V2
            dist2_remove_v2 = grafo.distancia_matriz[rota.route[j]][proximo2];

            bool seguidos = (j == i+1);
            if(seguidos){
                dist1_remove_v2 = grafo.distancia_matriz[anterior2][rota.route[j]]; // arestas que serão removidas V2
                dist2_remove_v2 = grafo.distancia_matriz[rota.route[j]][proximo2];

                dist1_add_v1 = grafo.distancia_matriz[rota.route[j]][rota.route[i]];
                dist2_add_v1 = grafo.distancia_matriz[rota.route[i]][proximo2];

                dist1_add_v2 = grafo.distancia_matriz[anterior1][rota.route[j]];
                dist2_add_v2 = grafo.distancia_matriz[rota.route[j]][rota.route[i]];

                impacto1 = dist1_add_v1 + dist2_add_v1 + dist1_add_v2 - dist1_remove_v1 - dist2_remove_v1 - dist2_remove_v2;
                impacto2 = 0;
            }else{
                dist1_remove_v2 = grafo.distancia_matriz[anterior2][rota.route[j]]; // arestas que serão removidas V2
                dist2_remove_v2 = grafo.distancia_matriz[rota.route[j]][proximo2];
                
                dist1_add_v1 = grafo.distancia_matriz[anterior2][rota.route[i]]; // arestas adicionadas v1
                dist2_add_v1 = grafo.distancia_matriz[rota.route[i]][proximo2];

                dist1_add_v2 = grafo.distancia_matriz[anterior1][rota.route[j]]; // arestas adicionadas V2
                dist2_add_v2 = grafo.distancia_matriz[rota.route[j]][proximo1];

                impacto1 = - dist1_remove_v1 - dist2_remove_v1 - vertice_parada_v1 + dist1_add_v2 + dist2_add_v2 + vertice_parada_v2;
                impacto2 = - dist1_remove_v2 - dist2_remove_v2 - vertice_parada_v2 + dist1_add_v1 + dist2_add_v1 + vertice_parada_v1;
            }
            if(rota.custo + impacto1 + impacto2 > grafo.t_max){
                continue; // custo ultrapassa o T_max, nao tem como fazer o swap
            }

            if(impacto1+impacto2 >= 0 || impacto1+impacto2 >=best_swap){
                continue; // nao melhorou a soluçao
            }
            int id = rota.id;
            int vertice_i = rota.route[i];
            int vertice_j = rota.route[j];

            bool local_visita = false;
            double impacto1_if_equals = 0;
            map<double, int> aux_visited_vertice2 = S.visited_vertices[rota.route[j]];
            auto it = aux_visited_vertice2.find(rota.visita_custo[j] + grafo.t_prot);
            if (it != aux_visited_vertice2.end()){
                aux_visited_vertice2.erase(it);
            }

            if (aux_visited_vertice2.empty()){
                local_visita = true;

            }else{
                it = aux_visited_vertice2.lower_bound(rota.visita_custo[i - 1] + dist1_add_v2 + vertice_parada_v2 + grafo.t_prot);
                if (it == aux_visited_vertice2.end()){
                    auto it_prev = prev(it);
                    if (Utils::doubleLessOrEqual(it_prev->first, rota.visita_custo[i - 1] + dist1_add_v2)){
                        local_visita = true;
                    }else{
                        local_visita = false;

                    }
                }else if (it == aux_visited_vertice2.begin()){
                    if(it->second == rota.id){
                        if (rota.visita_custo[j] < it->first - grafo.t_prot)
                        {
                            impacto1_if_equals = impacto1+impacto2;
                        }
                        else
                        {
                            impacto1_if_equals = impacto1; //corrigido
                        }
                    }

                    if (Utils::doubleLessOrEqual(rota.visita_custo[i - 1] + dist1_add_v2 + vertice_parada_v2 + grafo.t_prot, it->first - grafo.t_prot + impacto1_if_equals))
                    {
                        local_visita = true;
                    }
                    else
                    {
                        local_visita = false;
                    }
                }else{
                    if (it->second == rota.id)
                    {
                        if (rota.visita_custo[j] < it->first - grafo.t_prot)
                        {
                            impacto1_if_equals = impacto1 + impacto2;
                        }
                        else
                        {
                            impacto1_if_equals = impacto1;
                        }
                    }
                    auto it_prev = prev(it);
                    if (Utils::doubleLessOrEqual(rota.visita_custo[i - 1] + dist1_add_v2 + vertice_parada_v2 + grafo.t_prot, it->first - grafo.t_prot + impacto1_if_equals) && Utils::doubleLessOrEqual(it_prev->first, rota.visita_custo[i - 1] + dist1_add_v2))
                    {
                        local_visita = true;
                    }
                }
            }
            if (!local_visita)
                // cout << "---- Erro" << endl;
                continue;

            double temp_visita_custo = (j == i+1)? rota.visita_custo[i-1]+dist1_add_v2 + vertice_parada_v2 + dist2_add_v2: rota.visita_custo[j - 1] + impacto1;
            local_visita = false;
            double impacto2_if_equals = 0;
            map<double, int> aux_visited_vertice1 = S.visited_vertices[rota.route[i]];
            it = aux_visited_vertice1.find(rota.visita_custo[i] + grafo.t_prot);
            if (it != aux_visited_vertice1.end()){
                aux_visited_vertice1.erase(it);
            }
            if (aux_visited_vertice1.empty()){
                local_visita = true;

            }else{
                it = aux_visited_vertice1.lower_bound(temp_visita_custo + dist1_add_v1 + vertice_parada_v1 + grafo.t_prot);
                if (it == aux_visited_vertice1.end()){
                    auto it_prev = prev(it);
                    double aux_impacto = 0;
                    if(it_prev->second == rota.id) {
                        aux_impacto = impacto1;
                        impacto2_if_equals = impacto1 + impacto2;
                    }

                    if (Utils::doubleLessOrEqual(it_prev->first+aux_impacto,  temp_visita_custo + dist1_add_v1+impacto2_if_equals)){
                        local_visita = true;
                    }else{
                        local_visita = false;
                    }
                }else if (it == aux_visited_vertice1.begin()){
                    if(it->second == rota.id) impacto2_if_equals = impacto1 + impacto2;

                    if (Utils::doubleLessOrEqual(temp_visita_custo + dist1_add_v1 + vertice_parada_v1 + grafo.t_prot, it->first - grafo.t_prot + impacto2_if_equals))
                    {
                        local_visita = true;
                    }
                    else
                    {
                        local_visita = false;
                    }
                }else{
                    if(it->second == rota.id) impacto2_if_equals = impacto1 + impacto2;
                    auto it_prev = prev(it);
                    double if_equals_aux = 0;
                    if(it_prev->first - grafo.t_prot > rota.visita_custo[i]) if_equals_aux += impacto1;

                    if (Utils::doubleLessOrEqual(temp_visita_custo + dist1_add_v1 + vertice_parada_v1 + grafo.t_prot, it->first - grafo.t_prot + impacto2_if_equals) && Utils::doubleLessOrEqual(it_prev->first + if_equals_aux, temp_visita_custo + dist1_add_v1))
                    {
                        local_visita = true;
                    }
                }
            }if (!local_visita)
                continue;

            double if_prev_equals = 0, if_next_equals = 0;
            double impacto = impacto1;
            bool possibilidade_visita = true;
            for (int n = i + 1; n < rota.route.size()-1; n++)
            {
                if (n == j){
                    impacto+=impacto2;
                    continue;
                }
                auto it = S.visited_vertices[rota.route[n]].find(rota.visita_custo[n] + grafo.t_prot);
                auto it_next = std::next(it);
                auto it_prev = it != S.visited_vertices[rota.route[n]].begin() ? prev(it) : S.visited_vertices[rota.route[n]].end();
                if(rota.id == it_next->second){
                    //ocorrencia vai sofrer os dois impactos
                    if (rota.visita_custo[j] <= it_next->first - grafo.t_prot)
                    {
                        if_next_equals = impacto1+impacto2;
                    }
                    else if (rota.visita_custo[i] <= it_next->first - grafo.t_prot)
                    { // ocorrencia vai sofrer somente o impacto1
                        if_next_equals = impacto1;
                    }
                }else{
                    if_next_equals = 0;
                }
                
                if(rota.id == it_prev->second){
                    if (rota.visita_custo[j] <= it_prev->first - grafo.t_prot)
                    {
                        if_prev_equals = impacto1 + impacto2;
                    }
                    else if (rota.visita_custo[i] <= it_prev->first - grafo.t_prot)
                    { // ocorrencia vai sofrer somente o impacto1
                        if_prev_equals = impacto1;
                    }
                }else{
                    if_prev_equals=0;
                }
                // if_prev_equals = (rota.id == it_prev->second && rota.visita_custo[j] <= it_prev->first - grafo.t_prot) ? impacto2 : 0;

                if (it_next != S.visited_vertices[rota.route[n]].end() && it->first + impacto > it_next->first - grafo.t_prot + if_next_equals)
                {
                    possibilidade_visita = false;
                    break;
                }

                if (it_prev != S.visited_vertices[rota.route[n]].end() && it->first + impacto - grafo.t_prot < it_prev->first + if_prev_equals)
                {
                    possibilidade_visita = false;
                    break;
                }
            }
            if (!possibilidade_visita)
            {
                continue;
            }

            if(possibilidade_visita){
                // 

                // exclui vertice i rota1
                swap_inter[0][0] = rota.route[i];
                swap_inter[0][1] = score_v1;
                swap_inter[0][2] = grafo.distancia_matriz[anterior1][proximo1] - dist1_remove_v1 - dist2_remove_v1 - vertice_parada_v1;
                swap_inter[0][3] = i;

                // exclui vertice j rota2   
                swap_inter[1][0] = rota.route[j];
                swap_inter[1][1] = score_v2;
                swap_inter[1][2] = grafo.distancia_matriz[anterior2][proximo2] - dist1_remove_v2 - dist2_remove_v2 - vertice_parada_v2;
                swap_inter[1][3] = j;

                
                //adicionar vertice j na rota1
                swap_inter[2][0] = rota.route[j];                                 // id vertice
                swap_inter[2][1] = score_v2; // score vertice
                swap_inter[2][2] = dist1_add_v2 + dist2_add_v2 + vertice_parada_v2 - grafo.distancia_matriz[anterior1][proximo1]; // impacto incert vertice
                swap_inter[2][3] = i;                             // Local insert rota
                swap_inter[2][4] = rota.visita_custo[i - 1] + dist1_add_v2 + vertice_parada_v2; // Visita_custo
                swap_inter[2][5] = (vertice_parada_v2 == grafo.t_parada) ? 1 : 0;

                // adicionar vertice i na rota2
                swap_inter[3][0] = rota.route[i];                                                                // id vertice
                swap_inter[3][1] = score_v1; // score vertice
                swap_inter[3][2] = dist1_add_v1 + dist2_add_v1 + vertice_parada_v1 - grafo.distancia_matriz[anterior2][proximo2]; // impacto incert vertice
                swap_inter[3][3] = j;                                                            // Local insert rota
                swap_inter[3][4] = rota.visita_custo[j - 1] + dist1_add_v1 + vertice_parada_v1 + impacto1; // Visita_custo
                swap_inter[3][5] = (vertice_parada_v1 == grafo.t_parada) ? 1 : 0;

                if (best)
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
        if(!best && swap_inter[0][0] != -1) break;
    }
    if (swap_inter[0][0] == -1){
        return false;
    }else{
        // cout<< endl<<"ANTES: "<<endl;
        // // cout << "Rota: "<<rota.id<<" - Swap: "<<swap_inter[0][0]<< " | " << swap_inter[1][0]<<endl;
        // cout <<"Rota: " << rota.id << " - Vertice[" << swap_inter[0][3] << "] = " << rota.route[swap_inter[0][3]] << " |Swap| Vertice[" << swap_inter[1][3] << "] = " << rota.route[swap_inter[1][3]] <<endl;
        // cout << "Visita_custo[" << swap_inter[0][3] << "] = " << rota.visita_custo[swap_inter[0][3]] << " | Visita_custo[" << swap_inter[1][3] << "] = " << rota.visita_custo[swap_inter[1][3]] << endl;
        // cout << rota <<endl;
        // S.print_visited(12, 13);

        rota.excluir(swap_inter[0], S.visited_vertices, S.score, S.custo);
        rota.incert(swap_inter[2], S.visited_vertices, S.score, S.custo);

        rota.excluir(swap_inter[1], S.visited_vertices, S.score, S.custo);
        rota.incert(swap_inter[3], S.visited_vertices, S.score, S.custo);


        // cout << endl<<"DEPOIS: "<<endl;
        // // cout << "Rota: "<<rota.id<<" - Swap: "<<swap_inter[0][0]<< " | " << swap_inter[1][0]<<endl;
        // cout<<"Rota: " << rota.id << " - Vertice[" << swap_inter[0][3] << "] = " << rota.route[swap_inter[0][3]] << " |Swap| Vertice[" << swap_inter[1][3] << "] = " << rota.route[swap_inter[1][3]] <<endl;
        // cout << "Visita_custo[" << swap_inter[0][3] << "] = " << rota.visita_custo[swap_inter[0][3]] << " | Visita_custo[" << swap_inter[1][3] << "] = " << rota.visita_custo[swap_inter[1][3]] << endl;
        // cout << rota << endl;
        return true;
    }
}
    
bool Busca_local::swap_intra_rotas(Instance &grafo, Sol &S, Caminho &rota1, Caminho &rota2, int i_inicial, int i_final, bool &best) {
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

    for(int i = i_inicial; i < i_final; i++){
        anterior1 = rota1.route[i - 1];
        proximo1 = rota1.route[i + 1];
        score_r1 = (rota1.paradas[i] == 1) ? grafo.score_vertices[rota1.route[i]] : grafo.score_vertices[rota1.route[i]]/3;

        dist1_remove_r1 = grafo.distancia_matriz[anterior1][rota1.route[i]]; // arestas que serão removidas da rota 1
        dist2_remove_r1 = grafo.distancia_matriz[rota1.route[i]][proximo1];
        vertice_parada_r1 = (rota1.paradas[i] == 1) ? grafo.t_parada : 0; // se tiver parada, plus de 15 minutos
        // cout << "**** Rota: "<<rota1.id<<" - Vertice[" << i << "] = "<< rota1.route[i] <<" | "<< i <<" de " << rota1.route.size()-1<<" ****"<<endl;
        for (int j = 1; j < rota2.route.size()-1; j++){ 
            anterior2 = rota2.route[j - 1];
            proximo2 = rota2.route[j + 1];
            score_r2 = (rota2.paradas[j] == 1) ? grafo.score_vertices[rota2.route[j]] : grafo.score_vertices[rota2.route[j]] / 3;
            // cout << "Rota: " << rota2.id << " - Vertice[" << j << "] = " << rota2.route[j] << " | " << j << " de " << rota2.route.size()-1 << endl;

            if(anterior1 == rota2.route[j] || proximo1 == rota2.route[j] || anterior2 == rota1.route[i] || proximo2 == rota1.route[i]){
                // cout << "Vertice repetido" << endl;
                continue; // nao pode ter o mesmo vertice em seguida
            }

            vertice_parada_r2 = (rota2.paradas[j] == 1) ? grafo.t_parada : 0; // se tiver parada, plus de 15 minutos

            dist3_remove_r2 = grafo.distancia_matriz[anterior2][rota2.route[j]]; // arestas que serão removidas da rota 1
            dist4_remove_r2 = grafo.distancia_matriz[rota2.route[j]][proximo2];

            dist1_add_r1 = grafo.distancia_matriz[anterior1][rota2.route[j]]; // arestas adicionadas na rota1
            dist2_add_r1 = grafo.distancia_matriz[rota2.route[j]][proximo1];

            dist3_add_r2 = grafo.distancia_matriz[anterior2][rota1.route[i]]; // arestas adicionadas na rota2
            dist4_add_r2 = grafo.distancia_matriz[rota1.route[i]][proximo2];

            impacto1 = dist1_add_r1 + dist2_add_r1 + vertice_parada_r2 - dist1_remove_r1 - dist2_remove_r1 - vertice_parada_r1;
            impacto2 = dist3_add_r2 + dist4_add_r2 + vertice_parada_r1 - dist3_remove_r2 - dist4_remove_r2 - vertice_parada_r2;

            if(rota1.custo + impacto1 > grafo.t_max || rota2.custo + impacto2 > grafo.t_max){
                // cout << "Ultrapassa T_MAX" << endl;
                continue; // custo ultrapassa o T_max, nao tem como fazer o swap
            }

            if(impacto1 + impacto2 > 0 || impacto1 + impacto2 > best_swap){
                // cout << "Aumentou o custo, piorou a solução" << endl;
                continue; // aumentou o custo, piorou a solução
            }

            // se ha espaço para visita do vertice J na rota1
            // cout << "Se é possivel inserir a o vertice 'J' na rota1" << endl;
            bool local_visita = false;
            double impacto1_if_equals = 0;
            map<double, int> aux_visited_vertice_j = S.visited_vertices[rota2.route[j]];
            auto it = aux_visited_vertice_j.find(rota2.visita_custo[j] + grafo.t_prot);
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
                it = aux_visited_vertice_j.lower_bound(rota1.visita_custo[i - 1] + dist1_add_r1 + vertice_parada_r2 + grafo.t_prot);
                if (it == aux_visited_vertice_j.end())
                {
                    auto it_prev = prev(it);
                    if(it_prev->second == rota1.id && it_prev->first - grafo.t_prot > rota1.visita_custo[i]) impacto1_if_equals += impacto1;
                    if(it_prev->second == rota2.id && it_prev->first - grafo.t_prot > rota2.visita_custo[j]) impacto1_if_equals += impacto2;
                    if (Utils::doubleLessOrEqual(it_prev->first + impacto1_if_equals, rota1.visita_custo[i - 1] + dist1_add_r1))
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
                    if(it->second == rota1.id && it->first - grafo.t_prot > rota1.visita_custo[i]) impacto1_if_equals += impacto1;
                    if(it->second == rota2.id && it->first - grafo.t_prot > rota2.visita_custo[j]) impacto1_if_equals += impacto2;
                    if (Utils::doubleLessOrEqual(rota1.visita_custo[i - 1] + dist1_add_r1 + vertice_parada_r2 + grafo.t_prot, it->first - grafo.t_prot + impacto1_if_equals))
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
                    if(it->second == rota1.id && it->first - grafo.t_prot > rota1.visita_custo[i]) impacto1_if_equals += impacto1;
                    if(it->second == rota2.id && it->first - grafo.t_prot > rota2.visita_custo[j]) impacto1_if_equals+= impacto2;
                    auto it_prev = prev(it);
                    if (Utils::doubleLessOrEqual(rota1.visita_custo[i - 1] + dist1_add_r1 + vertice_parada_r2 + grafo.t_prot, it->first - grafo.t_prot + impacto1_if_equals) && Utils::doubleLessOrEqual(it_prev->first, rota1.visita_custo[i - 1] + dist1_add_r1))
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
            map<double, int> aux_visited_vertice_i = S.visited_vertices[rota1.route[i]];
            it = aux_visited_vertice_i.find(rota1.visita_custo[i] + grafo.t_prot);
            if (it != aux_visited_vertice_i.end())
            {
                aux_visited_vertice_i.erase(it);
            }else{
                cout << "ERROOOO"<<endl;
            }
            if (aux_visited_vertice_i.empty())
            {
                local_visita = true;
            }
            else
            {
                it = aux_visited_vertice_i.lower_bound(rota2.visita_custo[j - 1] + dist3_add_r2 + vertice_parada_r1 + grafo.t_prot);
                if (it == aux_visited_vertice_i.end())
                {
                    auto it_prev = prev(it);
                    if(it_prev->second == rota1.id && it_prev->first-grafo.t_prot > rota1.visita_custo[i]) impacto2_if_equals = impacto1;
                    if (Utils::doubleLessOrEqual(it_prev->first + impacto2_if_equals, rota2.visita_custo[j - 1] + dist3_add_r2))
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
                    if(it->second == rota1.id && it->first - grafo.t_prot > rota1.visita_custo[i]) impacto2_if_equals+= impacto1;
                    if(it->second == rota2.id && it->first - grafo.t_prot > rota2.visita_custo[j]) impacto2_if_equals += impacto2;
                    if (Utils::doubleLessOrEqual(rota2.visita_custo[j - 1] + dist3_add_r2 + vertice_parada_r1 + grafo.t_prot, it->first - grafo.t_prot + impacto2_if_equals))
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
                    if(it->second == rota1.id && it->first - grafo.t_prot > rota1.visita_custo[i]) impacto2_if_equals += impacto1;
                    if(it->second == rota2.id && it->first - grafo.t_prot > rota2.visita_custo[j]) impacto2_if_equals = impacto2;
                    auto it_prev = prev(it);
                    if (Utils::doubleLessOrEqual(rota2.visita_custo[j - 1] + dist3_add_r2 + vertice_parada_r1 + grafo.t_prot, it->first - grafo.t_prot + impacto2_if_equals) && Utils::doubleLessOrEqual(it_prev->first, rota2.visita_custo[j - 1] + dist3_add_r2))
                    {
                        local_visita = true;
                    }
                }
            }
            if (!local_visita)
                continue;
                
            double if_prev_equals = 0, if_next_equals = 0;

            bool possibilidade_visita = true;
            for (int a = i + 1; a < rota1.route.size() - 1; a++)
            {

                int vertice = rota1.route[a];

                it = S.visited_vertices[rota1.route[a]].find(rota1.visita_custo[a] + grafo.t_prot);
                auto it_next = it != S.visited_vertices[rota1.route[a]].end() ? next(it) : S.visited_vertices[rota1.route[a]].end();
                auto it_prev = it != S.visited_vertices[rota1.route[a]].begin() ? prev(it) : S.visited_vertices[rota1.route[a]].end();

                // auto it_next = next(it);
                // auto it_prev = prev(it);
                // if (it_next != S.visited_vertices[rota1.route[a]].end())
                if_next_equals = (rota2.id == it_next->second && rota2.visita_custo[j] <= it_next->first - grafo.t_prot) ? impacto2 : 0;
                // if (it_prev != S.visited_vertices[rota1.route[a]].end())
                if_prev_equals = (rota2.id == it_prev->second && rota2.visita_custo[j] <= it_prev->first - grafo.t_prot) ? impacto2 : 0;

                if (it_next != S.visited_vertices[rota1.route[a]].end() && it->first + impacto1 > it_next->first - grafo.t_prot + if_next_equals)
                {
                    possibilidade_visita = false;
                    break;
                }
                if (it_prev != S.visited_vertices[rota1.route[a]].end() && it->first + impacto1 - grafo.t_prot < it_prev->first + if_prev_equals)
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
            for (int a = j + 1; a < rota2.route.size() - 1; a++)
            {
                int vertice = rota2.route[a];

                it = S.visited_vertices[rota2.route[a]].find(rota2.visita_custo[a] + grafo.t_prot);
                auto it_next = it != S.visited_vertices[rota2.route[a]].end() ? next(it) : S.visited_vertices[rota2.route[a]].end();
                auto it_prev = it != S.visited_vertices[rota2.route[a]].begin() ? prev(it) : S.visited_vertices[rota2.route[a]].end();

                if_next_equals = (rota1.id == it_next->second && rota1.visita_custo[i] <= it_next->first - grafo.t_prot) ? impacto1 : 0;
                if_prev_equals = (rota1.id == it_prev->second && rota1.visita_custo[i] <= it_prev->first - grafo.t_prot) ? impacto1 : 0;

                if (it_next != S.visited_vertices[rota2.route[a]].end() && it->first + impacto2 > it_next->first - grafo.t_prot + if_next_equals)
                {
                    possibilidade_visita = false;
                    break;
                }
                if (it_prev != S.visited_vertices[rota2.route[a]].end() && it->first + impacto2 - grafo.t_prot < it_prev->first + if_prev_equals)
                {
                    possibilidade_visita = false;
                    break;
                }
            }

            
            if(possibilidade_visita){
                
                // exclui vertice i rota1
                swap_intra[0][0] = rota1.route[i];
                swap_intra[0][1] = score_r1;
                swap_intra[0][2] = grafo.distancia_matriz[anterior1][proximo1] - dist1_remove_r1 - dist2_remove_r1 - vertice_parada_r1;
                swap_intra[0][3] = i;

                // exclui vertice j rota2
                swap_intra[1][0] = rota2.route[j];
                swap_intra[1][1] = score_r2;
                swap_intra[1][2] = grafo.distancia_matriz[anterior2][proximo2] - dist3_remove_r2 - dist4_remove_r2 - vertice_parada_r2;
                swap_intra[1][3] = j;

                
                //adicionar vertice j na rota1
                swap_intra[2][0] = rota2.route[j];                                 // id vertice
                swap_intra[2][1] = score_r2; // score vertice
                swap_intra[2][2] = dist1_add_r1 + dist2_add_r1 + vertice_parada_r2 - grafo.distancia_matriz[anterior1][proximo1]; // impacto incert vertice
                swap_intra[2][3] = i;                             // Local insert rota
                swap_intra[2][4] = rota1.visita_custo[i - 1] + dist1_add_r1 + vertice_parada_r2; // Visita_custo
                swap_intra[2][5] = (vertice_parada_r2 == grafo.t_parada) ? 1 : 0;

                // adicionar vertice i na rota2
                swap_intra[3][0] = rota1.route[i];                                                                // id vertice
                swap_intra[3][1] = score_r1; // score vertice
                swap_intra[3][2] = dist3_add_r2 + dist4_add_r2 + vertice_parada_r1 - grafo.distancia_matriz[anterior2][proximo2]; // impacto incert vertice
                swap_intra[3][3] = j;                                                            // Local insert rota
                swap_intra[3][4] = rota2.visita_custo[j - 1] + dist3_add_r2 + vertice_parada_r1; // Visita_custo
                swap_intra[3][5] = (vertice_parada_r1 == grafo.t_parada) ? 1 : 0;

                if(best){
                    best_swap = impacto1 + impacto2;
                }else{
                    break;
                }
            }
            
        }
        if(!best && swap_intra[0][0] != -1) break;
        
    }

    if(swap_intra[0][0] == -1){
        return false;
    }else{
        // cout << "Rota: " << rota1.id << " - Vertice[" << swap_intra[0][3] << "] = " << swap_intra[0][0] << " |Swap| Rota: " << rota2.id << " - Vertice[" << swap_intra[1][3] << "] = " << swap_intra[1][0] << " | Impacto total: " << impacto1 + impacto2 << endl << "Impacto1: " << impacto1 << " | Impacto2: " << impacto2 << endl;
        // cout << "Rota: " << rota1.id << " - Visita[" << swap_intra[0][3] << "] = " << rota1.visita_custo[swap_intra[0][3]] << " |Swap| Rota: " << rota2.id << " - Visita[" << swap_intra[1][3] << "] = " << rota2.visita_custo[swap_intra[1][3]] << endl;



        // S.print_visited(209, 210);
        // S.print_visited(2, 3);

        // cout << "Antes: "<<endl<<rota1<<endl<< rota2<<endl;

        // S.print_visited(7, 8);

        // Excluindo
        rota1.excluir(swap_intra[0], S.visited_vertices, S.score, S.custo);
        rota2.excluir(swap_intra[1], S.visited_vertices, S.score, S.custo);

        // Adicionando
        rota1.incert(swap_intra[2], S.visited_vertices, S.score, S.custo);
        rota2.incert(swap_intra[3], S.visited_vertices, S.score, S.custo);

        // cout << "Depois: "<<endl<<rota1<<endl<< rota2<<endl;
        return true;
    }
}

bool Busca_local::swap_Out_rotas(Instance &grafo, Sol &S, Caminho &rota, int i_inicial, int i_final, bool &best)
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
    for (int i = i_inicial; i < i_final; i++)
    {
        // cout << "Best swap: " << best_swap << endl;
        int v1 = rota.route[i];
        anterior1 = rota.route[i - 1];
        proximo1 = rota.route[i + 1];
        score_v1 = (rota.paradas[i]) ? grafo.score_vertices[v1] : grafo.score_vertices[v1]/3;

        dist1_remove_v1 = grafo.distancia_matriz[anterior1][v1]; // Arestas que serão removidas V1
        dist2_remove_v1 = grafo.distancia_matriz[v1][proximo1];
        vertice_parada_v1 = (rota.paradas[i]) ? grafo.t_parada : 0; // Se tiver parada, plus de 15 minutos

        for (int n = 0; n < 2; n++)
        {
            rota.plus_parada = (n == 1) ? 0 : grafo.t_parada;
            // Itera sobre todos os vértices 
            for (int v2 = 1; v2 < grafo.qt_vertices; v2++)
            {
                if (anterior1 == v2 || proximo1 == v2 || v1 == v2) continue;

                score_v2 = (rota.plus_parada == grafo.t_parada) ? grafo.score_vertices[v2] : grafo.score_vertices[v2]/3;

                if(best_swap >= -score_v1 + score_v2 || rota.score-score_v1+score_v2 <= rota.score ) continue; //ja tem a melhor swap 

                dist1_add_v2 = grafo.distancia_matriz[anterior1][v2]; // Arestas que serão adicionadas
                dist2_add_v2 = grafo.distancia_matriz[v2][proximo1];

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
                        if(it->second == rota.id) impacto1_if_equals += impacto1;
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


                int if_next_equals = 0; int if_prev_equals = 0;
                // colocar o nova funçao de possibilidade visita
                // possibilidade visita rota
                bool possibilidade_visita = true;
                if (impacto1 < 0)
                {
                    possibilidade_visita = (Utils::doubleGreaterOrEqual(rota.push_hotspots[i+1].first, impacto1 * -1));
                }
                else
                {
                    possibilidade_visita = (Utils::doubleGreaterOrEqual(rota.push_hotspots[i+1].second, impacto1));
                }

                if (possibilidade_visita)
                {
                    // Dados do vértice removido da rota
                    swap_out[0][0] = v1; // ID do vértice v1 que será removido
                    swap_out[0][1] = score_v1; // Score do vértice
                    swap_out[0][2] = grafo.distancia_matriz[anterior1][proximo1] - dist1_remove_v1 - dist2_remove_v1 - vertice_parada_v1; // Alteração no custo total ao remover o vértice
                    swap_out[0][3] = i;// Posição do vértice na rota original

                    // Dados do vértice inserido na rota
                    swap_out[1][0] = v2; // ID do vértice que será inserido
                    swap_out[1][1] = score_v2;// Score do vértice
                    swap_out[1][2] = dist1_add_v2 + dist2_add_v2 + rota.plus_parada - grafo.distancia_matriz[anterior1][proximo1]; // Alteração no custo total ao adicionar o vértice
                    swap_out[1][3] = i; // Posição onde o vértice será inserido na rota
                    swap_out[1][4] = rota.visita_custo[i - 1] + dist1_add_v2 + rota.plus_parada; // Novo tempo de visita 
                    swap_out[1][5] = (rota.plus_parada == grafo.t_parada) ? 1 : 0; // Indica se o vértice é uma parada

                    if(best){
                        best_swap = -score_v1+score_v2;
                    }else{
                        break;
                    }
                    // return swap;
                }
            }
            if(!best && swap_out[0][0]!=-1) break;
        }
        if(!best && swap_out[0][0]!=-1) break;
    }
    if(swap_out[0][0] == -1){
        return false;
    }else{
        // cout << "Rota: " << rota.id << " - Vertice[" << swap_out[0][3] << "] = " << swap_out[0][0] << " (SAI) " << " |Swap| Vertice[" << swap_out[0][3] << "] = " << swap_out[1][0] << " (ENTRA) " << endl;

        rota.excluir(swap_out[0], S.visited_vertices, S.score, S.custo);
        rota.incert(swap_out[1], S.visited_vertices, S.score, S.custo);
        return true;
    }
}

bool Busca_local::swap_paradas_inter_rota(Instance &grafo, Sol &S, Caminho &rota, std::tuple<int, int, int, double, double> &best_swap_info)
{
    best_swap_info = std::make_tuple(-1, -1, -1, std::numeric_limits<double>::max(), 0.0); // Iniciar com valores padrão
    double impacto, aumento_score;

    for (int i = 1; i < rota.route.size() - 1; i++)
    {
        for (int j = i + 1; j < rota.route.size() - 1; j++)
        {
            if (rota.paradas[i] == rota.paradas[j])
                continue;

            // Verificando se vale a pena o swap
            if (rota.paradas[i])
            {
                // Ele já tem o maior score
                if (grafo.score_vertices[rota.route[i]] >= grafo.score_vertices[rota.route[j]])
                    continue;
                impacto = -15;
                aumento_score = grafo.score_vertices[rota.route[i]] - grafo.score_vertices[rota.route[j]];
            }
            else
            {
                if (grafo.score_vertices[rota.route[i]] <= grafo.score_vertices[rota.route[j]])
                    continue;
                impacto = 15;
                aumento_score = grafo.score_vertices[rota.route[j]] - grafo.score_vertices[rota.route[i]];
            }

            // verificar se ele empurra o primeiro e o último
            bool possibilidade_visita = true;

            for (int n = i; n <= j; n++)
            {
                auto it = S.visited_vertices[rota.route[n]].find(rota.visita_custo[n] + grafo.t_prot);
                auto it_prox = std::next(it);
                auto it_ant = std::prev(it);
                if (it_prox != S.visited_vertices[rota.route[n]].end() && it->first + impacto > it_prox->first - grafo.t_prot)
                {
                    possibilidade_visita = false;
                    break;
                }
                if (it_ant != S.visited_vertices[rota.route[n]].end() && it->first + impacto - grafo.t_prot < it_ant->first)
                {
                    possibilidade_visita = false;
                    break;
                }
            }

            if (possibilidade_visita)
            {
                if (impacto < std::get<3>(best_swap_info))
                { // Verificar se o impacto é melhor que o atual
                    best_swap_info = std::make_tuple(i, j, rota.paradas[i], impacto, aumento_score);
                }
            }
        }
    }

    cout << aumento_score << endl;
    return std::get<0>(best_swap_info) != -1; // Retorna true se encontrou uma troca válida
}

bool Busca_local::efetuar_melhor_troca(Instance &grafo, Sol &S, Caminho &rota, std::tuple<int, int, int, double, double> &best_swap_info, int &score_s, double &custo_s)
{
    if (std::get<0>(best_swap_info) == -1)
    {
        return false; // Nenhuma troca válida encontrada
    }

    int i = std::get<0>(best_swap_info);
    int j = std::get<1>(best_swap_info);
    double impacto = std::get<3>(best_swap_info);
    double aumento_score = std::get<4>(best_swap_info);

    // Efetuar a troca
    bool temp_parada = rota.paradas[i];
    rota.paradas[i] = rota.paradas[j];
    rota.paradas[j] = temp_parada;

    // Atualizar a tabela de vértices visitados para vértices subsequentes na rota
    rota.atualizar_visited_vertices(i, j, impacto, S.visited_vertices);
    rota.atualizar_push_hotspots(S.visited_vertices);

    // Atualizar o score e o custo da rota
    rota.score += aumento_score;
    score_s += aumento_score;

    // Imprimir informações da troca
    std::cout << "Troca efetuada na rota " << rota.id << ": (" << i << ", " << j << ") com impacto " << impacto << " e score enviar " << aumento_score << std::endl;

    return true;
}