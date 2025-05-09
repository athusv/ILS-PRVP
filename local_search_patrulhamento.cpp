#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <string>
#include <queue>
#include <random>
#include <chrono>
#include <thread>
#include <fstream>
#include <sstream>
#include <cassert>
#include <map>
#include <iterator>
#include "Instance.h"
#include "Caminho.h"
#include "Sol.h"
#include "Utils.h"
#include "Perturbacao.h"
#include "Busca_local.h"

#define NDEBUG

using namespace std;

void Guloso(Instance &grafo, Sol &s0, mt19937 &gen)
{
    // Variavel de parada do while, todas as rotas estao prontas
    priority_queue<Caminho> rotas_prontas;
    while (!s0.rotas.empty()) {
        Caminho rota = s0.rotas.top();
        s0.rotas.pop();

        //decidindo se vai parar ou passar
        std::uniform_int_distribution<int> dis(1, 10);
        int numero_aleatorio = dis(gen);
        if(numero_aleatorio <= 8){
            rota.plus_parada = grafo.t_parada;
        }else{
            rota.plus_parada = 0;
        }

        vector<int> lista_candidatos = Utils::make_lista(grafo, rota, s0.visited_vertices);
        if (lista_candidatos.size() == 0) {
            rota.custo += rota.distancia_matriz[rota.route.back()][0];
            rota.route.push_back(0);
            rota.paradas.push_back(0);
            rota.visita_custo.push_back(rota.custo);
            rota.push_hotspots.push_back({999, 999});

            rota.atualizar_push_hotspots(s0.visited_vertices);
            rotas_prontas.push(rota);
            s0.custo += rota.custo;
            s0.score += rota.score;

            s0.cont_vizinhanca["best_incert"] = 0;
            s0.cont_vizinhanca["swap_inter"] = 0;
            s0.cont_vizinhanca["swap_intra"] = 0;
            s0.cont_vizinhanca["swap_out"] = 0;
            s0.cont_vizinhanca["para"] = 0;

            // rota.print_push();
            continue;
        }

        int index_max = Utils::max_score(lista_candidatos, grafo.score_vertices);

        rota.custo += rota.distancia_matriz[rota.route.back()][index_max] + rota.plus_parada;
        double s = grafo.score_vertices[index_max];

        if (rota.plus_parada == grafo.t_parada)
        {
            s = grafo.score_vertices[index_max];
            rota.score += s;
            rota.paradas.push_back(1);
        }
        else
        {
            s = grafo.score_vertices[index_max]/3;
            rota.score += s;
            rota.paradas.push_back(0);
        }
        rota.visita_custo.push_back(rota.custo);
        rota.route.push_back(index_max);
        rota.push_hotspots.push_back({999, 999});

        s0.visited_vertices[index_max][rota.custo + grafo.t_prot] = rota.id;

        // if (rota.z == 0) {
        //     rota.plus_parada = 0;
        //     rota.z += 1;
        // } else if (rota.z == 1) {
        //     rota.z += 1;
        // } else if (rota.z == 2) {
        //     rota.plus_parada = grafo.t_parada;
        //     rota.z = 0;
        // }

        // cout << rota << endl;

        s0.rotas.push(rota);
    }
    s0.rotas = rotas_prontas;
}

void Construtivo(Instance &grafo, Sol &s0, mt19937 &gen)
{
    // cout << "Iniciando o método Construtivo..." << endl;
    priority_queue<Caminho> rotas_prontas;
    while (!s0.rotas.empty())
    {
        Caminho rota = s0.rotas.top();
        s0.rotas.pop();
        // cout << "Processando rota com ID: " << rota.id << endl;

        // Decidindo se vai parar ou passar
        std::uniform_int_distribution<int> dis(1, 10);
        int rand_num = dis(gen);
        rota.plus_parada = (rand_num <= 7) ? grafo.t_parada : 0;
        // cout << "Decisão de parada: " << (rota.plus_parada ? "Parar" : "Não parar") << endl;

        // Lista de vertices possiveis
        vector<int> lista_candidatos = Utils::make_lista(grafo, rota, s0.visited_vertices);
        // cout << "Número de candidatos: " << lista_candidatos.size() << endl;
        if (lista_candidatos.empty())
        {
            // cout << "Nenhum candidato disponível, finalizando rota." << endl;
            rota.custo += rota.distancia_matriz[rota.route.back()][0];
            rota.route.push_back(0);
            rota.paradas.push_back(0);
            rota.visita_custo.push_back(rota.custo);
            rota.push_hotspots.push_back({999, 999});

            rota.atualizar_push_hotspots(s0.visited_vertices);
            rotas_prontas.push(rota);
            s0.custo += rota.custo;
            s0.score += rota.score;
            continue;
        }

        int index; string chamou;
        rand_num = dis(gen);
        // cout << "rand_num: " << rand_num << endl;
        if(rand_num >=1 && rand_num < 3){
            shuffle(lista_candidatos.begin(), lista_candidatos.end(), gen);
            index = lista_candidatos[0];
            chamou = "Construtivo - Random";
        }else if(rand_num >= 3 && rand_num < 5){
            index = Utils::min_custo(lista_candidatos, rota.distancia_matriz, rota.route.back());
            chamou = "Construtivo - Min_custo";
        }else if(rand_num >= 5 && rand_num < 7){
            index = Utils::max_score(lista_candidatos, grafo.score_vertices);
            chamou = "Construtivo - Max_scores";
        }else if(rand_num >= 7 && rand_num <=10){
            index = Utils::cost_benefit(lista_candidatos, grafo.score_vertices, rota.distancia_matriz, rota.route.back());
            chamou = "Construtivo - Cost_benefit";
        }
        // cout << "Escolha do candidato: " << index << " usando método: " << chamou << endl;

        rota.custo += rota.distancia_matriz[rota.route.back()][index] + rota.plus_parada;
        double s = grafo.score_vertices[index];

        if (rota.plus_parada == grafo.t_parada)
        {
            s = grafo.score_vertices[index];
            rota.score += s;
            rota.paradas.push_back(1);
        }
        else
        {
            s = grafo.score_vertices[index] / 3;
            rota.score += s;
            rota.paradas.push_back(0);
        }
        rota.visita_custo.push_back(rota.custo);
        rota.route.push_back(index);
        rota.push_hotspots.push_back({999, 999});

        s0.visited_vertices[index][rota.custo + grafo.t_prot] = rota.id;

        s0.rotas.push(rota);
        s0.atualiza_push(grafo);
        s0.checa_solucao(grafo, chamou);
    }
    s0.rotas = rotas_prontas;
}


Sol ILS(Sol &s0, Instance &grafo, mt19937 gen, double tempo_maximo)
{
    // cout << "ILS" << endl;
    Sol s = s0; Sol s1 = s0;
    Sol best_s = s0;
    string chamou;

    auto inicio = std::chrono::high_resolution_clock::now();
    int i = 1;
    while(true) {
        s1 = s;
        s1 = Perturbacao::perturbacao(grafo, s1, gen);
        s1.atualiza_push(grafo);
        chamou = "Pertubação";
        s1.checa_solucao(grafo, chamou);
        // cout << "|||||||||||Pertubação " << i << endl;
        // cout << "Score: " << s1.score << ", Custo: " << s1.custo << endl;

        s1 = Busca_local::busca_local(grafo, s1, gen);
        s1.atualiza_push(grafo);

        chamou = "Busca Local";
        s1.checa_solucao(grafo, chamou);
        // cout << "||||||||||||Busca Local " << i << endl;
        // cout << "Score: " << s1.score << ", Custo: " << s1.custo << endl;
        if (!Utils::doubleGreaterOrEqual(best_s.score, s1.score))
        {

            // cout << "Melhor solucão encontrada ***** " << i << endl;
            // cout << "Score: " << s1.score << ", Custo: " << s1.custo << endl;
            best_s = s1;
            s = best_s;
        }
        // Verifica se o tempo máximo foi atingido
        auto agora = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duracao = agora - inicio;

        if (duracao.count() >= tempo_maximo)
        {
            // std::cout << "Tempo máximo de execução atingido: " << duracao.count() << " segundos.\n";
            std::cout << "Iteração: "<<i<<"\n";
            break;
        }

        i++;
    }

    return best_s;
}

Sol ILS_Reset(Sol &s0, Instance &grafo, mt19937 gen, double tempo_maximo, int max_it_sem_melhora)
{
    Sol s = s0;
    Sol s1 = s0;
    Sol best_s = s0;
    Sol best_global = s0;
    string chamou;

    auto inicio = std::chrono::high_resolution_clock::now();
    grafo.iteracoes_totais = 0;
    int it_sem_melhora = 0;
    // cout << "[ILS_Reset] Iniciando algoritmo ILS com Reset" << endl;

    while (true)
    {
        double porcentagem_perturbacao = static_cast<double>(it_sem_melhora) / max_it_sem_melhora;

        s1 = s;
        s1 = Perturbacao::perturbacao_strength(grafo, s1, gen, porcentagem_perturbacao);
        s1.atualiza_push(grafo);
        chamou = "Pertubação";
        s1.checa_solucao(grafo, chamou);

        s1 = Busca_local::busca_local(grafo, s1, gen);
        s1.atualiza_push(grafo);
        chamou = "Busca Local";
        s1.checa_solucao(grafo, chamou);

        if (!Utils::doubleGreaterOrEqual(best_s.score, s1.score))
        {
            best_s = s1;
            s = best_s;
            it_sem_melhora = 0; // Reset ao encontrar uma melhora
        }
        else
        {
            it_sem_melhora++;
        }

        if (it_sem_melhora >= max_it_sem_melhora)
        {
            if(best_global.score < best_s.score){
                best_global = best_s;
            }
            Sol s2(grafo);
            Construtivo(grafo, s2, gen);
            best_s = s2;
            s = best_s;
            it_sem_melhora = 0;
        }

        // Verifica se o tempo máximo foi atingido
        auto agora = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duracao = agora - inicio;

        if (duracao.count() >= tempo_maximo)
        {
            if(best_global.score < best_s.score){
                best_global = best_s;
            }
            break;
        }

        grafo.iteracoes_totais++;
    }

    return best_global;
}

int main(int argc, char *argv[])
{

    unsigned int seed_value;
    std::random_device rd;
    double tempo_maximo = 180.0;
    int max_it_sem_melhora = 1000;

    string instancia = string(argv[1]);
    // Instance grafo("C:/Users/athus/Faculdade/6 periodo/PIBIT/solucao/pibit-rotas-pm/misc/ILS-algoritm/" + instancia, t_prot, t_parada, velocidade);
    if (argc > 2)
    {
        seed_value = stoul(argv[2]);
        // cout << "Seed fornecida: " << seed_value << std::endl;
    }
    else
    {
        random_device rd;
        seed_value = rd();
        // cout << "Seed aleatória gerada: " << seed_value << std::endl;
    }
    std::ofstream outputFile("resultados_" + instancia);

    // Verifica se o arquivo foi aberto corretamente
    if (!outputFile)
    {
        std::cerr << "Erro ao abrir o arquivo!" << std::endl;
        return 1;
    }
    
    Instance grafo("Instancias/" + instancia);
    Sol best_s = Sol(grafo);
    double mean_score = 0;
    long long int mean_it = 0;
    double b_score_construtivo;
    unsigned int seed_best;
    int n;
    for (n = 0; n < 3; n++)
    {

        random_device rd;
        seed_value = rd();
        mt19937 gen(seed_value);
        Sol s0(grafo);
        cout << s0 << endl;
        cout << "Construtivo" << endl;
        Construtivo(grafo, s0, gen);
        cout << s0 << endl;
        s0.atualiza_push(grafo);
        string chamou = "Construtivo";
        s0.checa_solucao(grafo, chamou);

        Sol s1 = ILS_Reset(s0, grafo, gen, tempo_maximo, max_it_sem_melhora);
        mean_score += s1.score;
        if (best_s.score < s1.score)
        {
            best_s = s1;
            seed_best = seed_value;
            b_score_construtivo = s0.score;
        }
        double percentual_melhora = (static_cast<double>(s1.score - s0.score) / s0.score) * 100;

        outputFile << "Tempo Máximo: " << grafo.t_max * grafo.veiculos << std::endl;
        outputFile << "Seed: " << seed_value << " | Tempo de Execução: " << tempo_maximo << "s | Max Itereções sem melhora: " << max_it_sem_melhora << endl;
        outputFile << "T_prot: " << grafo.t_prot / 60 << "min | T_parada: " << grafo.t_parada / 60 << "min | Velocidade: " << "Km/h" << endl;
        outputFile << "Instância: " << instancia << " | Vértices: " << grafo.qt_vertices << " | Veículos: " << grafo.veiculos << endl;
        outputFile << "Solução Construtivo - Score: " << s0.score << " | Custo: " << s0.custo << endl;
        outputFile << "Solução ILS - Score: " << s1.score << " | Custo: " << s1.custo << endl;
        outputFile << "Melhoria de " << percentual_melhora << "%" << endl
                   << endl;

        outputFile << "Contagem Estruturas de Vizinhança: " << endl;
        outputFile << "Best Insert = " << s1.cont_vizinhanca["best_insert"] << "/" << s1.cont_vizinhanca_total["best_insert"] << endl;
        outputFile << "Swap Inter = " << s1.cont_vizinhanca["swap_inter"] << "/" << s1.cont_vizinhanca_total["swap_inter"] << endl;
        outputFile << "Swap Intra = " << s1.cont_vizinhanca["swap_intra"] << "/" << s1.cont_vizinhanca_total["swap_intra"] << endl;
        outputFile << "Swap Out = " << s1.cont_vizinhanca["swap_out"] << "/" << s1.cont_vizinhanca_total["swap_out"] << endl;
        outputFile << "Para = " << s1.cont_vizinhanca["para"] << "/" << s1.cont_vizinhanca_total["para"] << endl;
        outputFile << "Realocate = " << s1.cont_vizinhanca["realocate"] << "/" << s1.cont_vizinhanca_total["realocate"] << endl;
        outputFile << "Contagem de Melhorias por rota" << endl;
        priority_queue<Caminho> rotas_prontas;
        while (!s0.rotas.empty())
        {
            Caminho rota = s0.rotas.top();
            s0.rotas.pop();
            outputFile << "Rota " << rota.id << "= " << s1.improved_rotas[rota.id] << "/" << s1.teste_rotas[rota.id] << endl;
            rotas_prontas.push(rota);
        }
        s0.rotas = rotas_prontas;

        outputFile << "Iterações totais: " << grafo.iteracoes_totais << endl;
        outputFile << "***********************************************" << std::endl
                   << endl;
    }

    mean_score = mean_score / n;
    mean_it = static_cast<int>(grafo.iteracoes_totais / n);
    outputFile << "-- Seed da melhor execução: " << seed_best << std::endl;
    outputFile << "-- Média de iterações: " << mean_it << std::endl;
    outputFile << "-- Pontuação do Construtivo da melhor execução: " << b_score_construtivo << std::endl;
    outputFile << "-- Média de pontuação ILS: " << mean_score << std::endl;
    outputFile << "-- Melhor pontuação: " << best_s.score << std::endl;
    outputFile.close();
    // instancia, seed_best, tempo, total_iterações, it_reset, mean_score, best_score, custo,
    cout << instancia << ", " << seed_best << ", " << tempo_maximo << ", " << mean_it << ", " << max_it_sem_melhora << ", " << mean_score << ", " << best_s.score << endl;

    // if (argc > 2)
    // {
    //     seed_value = stoul(argv[2]);
    // }
    // else
    // {
    //     random_device rd;
    //     seed_value = rd();
    // }

    // exportar um .TXT
}