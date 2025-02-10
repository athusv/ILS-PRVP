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

void Construtivo(Instance &grafo, Sol &s0, mt19937 &gen)
{
    // Variavel de parada do while, todas as rotas estao prontas
    vector<bool> rota_pronta(s0.rotas.size(), false);
    int cont = 0;
    
    while(cont < s0.rotas.size()){

        for(Caminho &rota : s0.rotas)
        {
            if(rota_pronta[rota.id])
            {
                continue;
            }
            // decidindo se vai parar ou passar
            std::uniform_int_distribution<int> dis(1, 10);
            int rand_num = dis(gen);
            rota.plus_parada = (rand_num <= 7) ? grafo.t_parada : 0;
            // Lista de vertices possiveis
            vector<int> lista_candidatos = Utils::make_lista(grafo, rota, s0.visited_vertices);
            if (lista_candidatos.empty())
            {
                // cout<<"Lista de candidatos vazia"<<" Rota: " <<rota.id<<endl;

                rota.custo += grafo.distancia_matriz[rota.route.back()][0];
                rota.route.push_back(0);
                rota.paradas.push_back(0);
                rota.visita_custo.push_back(rota.custo);
                rota.push_hotspots.push_back({999, 999});

                rota.atualizar_push_hotspots(s0.visited_vertices);
                rota_pronta[rota.id] = true;
                cont+=1;
                s0.custo += rota.custo;
                s0.score += rota.score;
                // cout << rota << endl;
                // rota.print_push();
                continue;
            }
            // - (20%) Aleatório,
            // - (20%) menor custo,
            // - (20%) maior pontuação,
            // - (40%) melhor custo-benefício (pontuação/tempo).
            // cout << "aleatorio: " << rand_num << endl;
            int index; string chamou;
            rand_num = dis(gen);
            // cout << "rand_num: "<<rand_num<<endl;
            if(rand_num >=1 && rand_num < 3){
                shuffle(lista_candidatos.begin(), lista_candidatos.end(), gen);
                index = lista_candidatos[0];
                // cout << "Random: " << index << endl;
                chamou = "Construtivo - Random";
            }else if(rand_num >= 3 && rand_num < 5){
                // cout << rota <<endl<<rota.route.back()<<endl;
                index = Utils::min_custo(lista_candidatos, grafo.distancia_matriz, rota.route.back());
                // cout << "Min_custo: " << index << endl;
                chamou = "Construtivo - Min_custo";
            }else if(rand_num >= 5 && rand_num < 7){
                index = Utils::max_score(lista_candidatos, grafo.score_vertices);
                // cout << "Max_score: " << index << endl;
                chamou = "Construtivo - Max_scores";
            }else if(rand_num >= 7 && rand_num <=10){
                index = Utils::cost_benefit(lista_candidatos, grafo.score_vertices, grafo.distancia_matriz, rota.route.back());
                // cout << "Cost_benefit: " << index << endl;
                chamou = "Construtivo - Cost_benefit";
            }


            rota.custo += grafo.distancia_matriz[rota.route.back()][index] + rota.plus_parada;
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

            s0.atualiza_push(grafo);
            // s0.checa_solucao(grafo, chamou);
        }
    }
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

        s1 = Busca_local::best_improvement(grafo, s1, gen);
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
    int it_total = 1;
    int it_sem_melhora = 0;

    while (true)
    {
        grafo.iteracoes_totais++;
        double porcentagem_perturbacao = static_cast<double>(it_sem_melhora) / max_it_sem_melhora;
        // cout << "Iteraçao sem melhora: " << it_sem_melhora << " || Max iteraçao: " << max_it_sem_melhora << endl;
        s1 = s;
        // s1 = Perturbacao::perturbacao(grafo, s1, gen);
        s1 = Perturbacao::perturbacao_strength(grafo, s1, gen, porcentagem_perturbacao);
        s1.atualiza_push(grafo);
        chamou = "Pertubação";
        s1.checa_solucao(grafo, chamou);
        // cout << "|||||||||||Pertubação " << it_total << endl;
        // cout << "Score: " << s1.score << ", Custo: " << s1.custo << endl;

        s1 = Busca_local::best_improvement(grafo, s1, gen);
        s1.atualiza_push(grafo);
        chamou = "Busca Local";
        s1.checa_solucao(grafo, chamou);
        // cout << "||||||||||||Busca Local " << it_total << endl;
        // cout << "Score: " << s1.score << ", Custo: " << s1.custo << endl;

        if (!Utils::doubleGreaterOrEqual(best_s.score, s1.score))
        {
            // cout << "Melhor solucão Local " << it_sem_melhora << endl;
            // cout << "Score: " << s1.score << endl;
            best_s = s1;
            s = best_s;
            it_sem_melhora = 0; // Reset ao encontrar uma melhora
        }
        else
        {
            it_sem_melhora++;
        }

        // Reinicia se não houver melhora após 50 iterações
        if (it_sem_melhora >= max_it_sem_melhora)
        {
            // std::cout << "Reiniciando com Construtivo após " << max_it_sem_melhora << " iterações sem melhora.\n";
            if(best_global.score < best_s.score){
                best_global = best_s;
                // cout <<endl<< "***** Melhor solucão Global ***** "<< endl;
                // cout << "Score: " << best_global.score <<endl<<endl;
            }
            Sol s2(grafo);
            Construtivo(grafo, s2, gen);
            best_s = s2;
            s = best_s;

            // cout <<endl<< "Novo Construtivo:" << endl;
            // cout << "Score: " << s.score << endl;
            it_sem_melhora = 0;
        }

        // Verifica se o tempo máximo foi atingido
        auto agora = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duracao = agora - inicio;

        if (duracao.count() >= tempo_maximo)
        {
            // std::cout << "Tempo máximo de execução atingido: " << duracao.count() << " segundos.\n";
            if(best_global.score < best_s.score){
                best_global = best_s;
                // cout <<endl<< "***** Melhor solucão Global ***** "<< endl;
                // cout << "Score: " << best_global.score <<endl<<endl;
            }
            // std::cout << "Iteração: " << it_total << "\n";
            break;
        }

        it_total++;
    }

    return best_global;
}

int main(int argc, char *argv[])
{
    
    unsigned int seed_value;
    std::random_device rd;
    double tempo_maximo = 1 * 60.0;
    int max_it_sem_melhora = 1000;

    string instancia = string(argv[1]);
    // Instance grafo("C:/Users/athus/Faculdade/6 periodo/PIBIT/solucao/pibit-rotas-pm/misc/ILS-algoritm/" + instancia, t_prot, t_parada, velocidade);

    std::ofstream outputFile("resultados_"+instancia);

    // Verifica se o arquivo foi aberto corretamente
    if (!outputFile)
    {
        std::cerr << "Erro ao abrir o arquivo!" << std::endl;
        return 1;
    }

    Instance grafo("Instancias/" + instancia);
    Sol best_s = Sol(grafo);
    double mean_score = 0;
    double mean_it = 0;
    double b_score_construtivo;
    unsigned int seed_best;
    int n;
    for(n = 0; n<3;n++){

        random_device rd;
        seed_value = rd();
        // cout << "Iteração: "<<n<<" | Seed: "<<seed_value<<endl;
        mt19937 gen(seed_value);
        // Solução inicial
        Sol s0(grafo);
        Construtivo(grafo, s0, gen);
        s0.atualiza_push(grafo);
        string chamou = "Construtivo";
        // s0.checa_solucao(grafo, chamou);

        Sol s1 = ILS_Reset(s0, grafo, gen, tempo_maximo, max_it_sem_melhora);
        mean_score += s1.score;
        if(best_s.score < s1.score){
            best_s = s1;
            seed_best = seed_value;
            b_score_construtivo = s0.score;
        }
        double percentual_melhora = (static_cast<double>(s1.score - s0.score) / s0.score) * 100;
        
        outputFile << "Tempo Máximo: " << grafo.t_max * grafo.veiculos << std::endl;
        outputFile << "Seed: "<<seed_value<< " | Tempo de Execução: "<<tempo_maximo<<"s | Max Itereções sem melhora: "<< max_it_sem_melhora <<endl;
        outputFile << "T_prot: "<< grafo.t_prot/60 << "min | T_parada: " <<grafo.t_parada/60<< "min | Velocidade: "<<grafo.velocidade<<"Km/h"<< endl;
        outputFile << "Instância: " << instancia << " | Vértices: " <<grafo.qt_vertices << " | Veículos: " <<grafo.veiculos<<endl;
        outputFile << "Solução Construtivo - Score: " << s0.score << " | Custo: " << s0.custo << endl;
        outputFile << "Solução ILS - Score: " << s1.score << " | Custo: "<<s1.custo<<endl;
        outputFile << "Melhoria de " << percentual_melhora << "%"<<endl<<endl;

        outputFile << "Contagem Estruturas de Vizinhança: " <<endl;
        outputFile << "Best Insert = " << s1.cont_vizinhanca["best_insert"] << "/" << s1.cont_vizinhanca_total["best_insert"] << endl;
        outputFile << "Swap Inter = " << s1.cont_vizinhanca["swap_inter"] << "/" << s1.cont_vizinhanca_total["swap_inter"] << endl;
        outputFile << "Swap Intra = " << s1.cont_vizinhanca["swap_intra"] << "/" << s1.cont_vizinhanca_total["swap_intra"] << endl;
        outputFile << "Swap Out = " << s1.cont_vizinhanca["swap_out"] << "/" << s1.cont_vizinhanca_total["swap_out"] << endl;
        outputFile << "Para = " << s1.cont_vizinhanca["para"] << "/" << s1.cont_vizinhanca_total["para"] << endl;

        outputFile << "Contagem de Melhorias por rota" << endl;
        for (int i = 0; i < s1.improved_rotas.size(); i++)
        {
            outputFile << "Rota " << i << "= " << s1.improved_rotas[i] << "/" << s1.teste_rotas[i] << endl;
        }

        outputFile << "Iterações totais: " << grafo.iteracoes_totais << endl;
        outputFile << "***********************************************" << std::endl<<endl;
    }

    mean_score = mean_score/n;
    mean_it = grafo.iteracoes_totais/n;
    outputFile << "-- Seed da melhor execução: " << seed_best << std::endl;
    outputFile << "-- Média de iterações: " << mean_it << std::endl;
    outputFile << "-- Pontuação do Construtivo da melhor execução: " << b_score_construtivo << std::endl;
    outputFile << "-- Média de pontuação ILS: " << mean_score << std::endl;
    outputFile << "-- Melhor pontuação: " << best_s.score << std::endl;
    // double percentual_melhora = (static_cast<double>(best_s.score - b_score_construtivo) / b_score_construtivo) * 100;
    // outputFile << "-- Melhoria na melhor execução: " << best_s.score << "%"<<std::endl;
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