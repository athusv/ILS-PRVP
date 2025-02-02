#include "Perturbacao.h"

Sol &Perturbacao::perturbacao_strength(Instance &grafo, Sol &S, std::mt19937 &gen, double porcentagem_perturbacao)
{
    std::string chamou;
    bool best_improvement = false;

    for (auto &rota : S.rotas){
        int quantos_excluir = static_cast<int>(rota.route.size() * porcentagem_perturbacao);
        // std::cout << "Porcentagem Perturbação: " << porcentagem_perturbacao << std::endl;
        // std::cout << "Quantos excluir = " << quantos_excluir << std::endl;
        // std::cout << "Rota ID: " << rota.id << std::endl;

        std::vector<int> indice_route;
        for (int i = 1; i < rota.route.size() - 1; i++)
        {
            indice_route.push_back(i);
        }

        std::shuffle(indice_route.begin(), indice_route.end(), gen);

        int n = 0;
        int i;

        while (true)
        {
            if (quantos_excluir == 0 || n == indice_route.size())
                break;

            i = indice_route[n];

            std::vector<double> exclui_vertice = Utils::p_excluir(grafo, S.visited_vertices, rota, i);
            if (exclui_vertice[0] != -1)
            {
                // cout << endl<<"Efetuando p_excluir" << endl;
                // cout<<rota<<endl;
                // std::cout << "Rota:" << rota.id << " - Vertice[" << i << "]: " << exclui_vertice[0] << " - Score: " << exclui_vertice[1] << " - Impacto: " << exclui_vertice[2] << std::endl;

                rota.excluir(exclui_vertice, S.visited_vertices, S.score, S.custo);
                // cout << rota << endl;
                S.atualiza_push(grafo);

                chamou = "Perturbação EXCLUIR";
                assert(S.checa_solucao(grafo, chamou));

                indice_route.pop_back();
                for (int j = 0; j < indice_route.size(); j++)
                {
                    if (indice_route[j] == rota.route.size() - 1)
                    {
                        indice_route.erase(indice_route.begin() + j);
                        break;
                    }
                }
                std::shuffle(indice_route.begin(), indice_route.end(), gen);
                quantos_excluir--;
                n = 0;
                continue;
            }
            vector<vector<double>> swap_out = Utils::swap_out(grafo, S, rota, i, i + 1, best_improvement);
            if (swap_out[0][0] != -1)
            {
                // cout <<endl<< "Efetuando Swap perturbaçao" << endl;
                // cout<<rota<<endl;
                // cout << "Rota: " << rota.id << " - Vertice[" << swap_out[0][3] << "] = " << swap_out[0][0] << " (SAI) " << " |Swap| Vertice[" << swap_out[0][3]<< "] = " << swap_out[1][0] << " (ENTRA)" << std::endl;
                rota.excluir(swap_out[0], S.visited_vertices, S.score, S.custo);
                rota.incert(swap_out[1], S.visited_vertices, S.score, S.custo);
                S.atualiza_push(grafo);

                chamou = "Perturbação - Swap Out";
                assert(S.checa_solucao(grafo, chamou));

                std::shuffle(indice_route.begin(), indice_route.end(), gen);
                quantos_excluir--;
                n = 0;
                continue;
            }

            n++;
        }

    }

    return S;
}

Sol &Perturbacao::perturbacao(Instance &grafo, Sol &S, std::mt19937 &gen)
{
    std::string chamou;
    std::uniform_int_distribution<int> dis(1, 8);
    int numero_aleatorio = dis(gen);
    double porcentagem_perturbacao = static_cast<double>(numero_aleatorio) * 0.1;
    bool best_improvement = false;

    for (auto &rota : S.rotas){
        int quantos_excluir = static_cast<int>(rota.route.size() * porcentagem_perturbacao);

        // std::cout << "Quantos excluir = " << quantos_excluir << std::endl;
        // std::cout << std::endl
        //           << rota << std::endl
        //           << std::endl;

        std::vector<int> indice_route;
        for (int i = 1; i < rota.route.size() - 1; i++)
        {
            indice_route.push_back(i);
        }

        std::shuffle(indice_route.begin(), indice_route.end(), gen);

        int n = 0;
        int i;

        while (true)
        {
            if (quantos_excluir == 0 || n == indice_route.size())
                break;

            i = indice_route[n];

            std::vector<double> exclui_vertice = Utils::p_excluir(grafo, S.visited_vertices, rota, i);
            if (exclui_vertice[0] != -1)
            {
                // cout << endl<<"Efetuando p_excluir" << endl;
                // cout<<rota<<endl;
                // std::cout << "Rota:" << rota.id << " - Vertice[" << i << "]: " << exclui_vertice[0] << " - Score: " << exclui_vertice[1] << " - Impacto: " << exclui_vertice[2] << std::endl;
                
                rota.excluir(exclui_vertice, S.visited_vertices, S.score, S.custo);
                // cout << rota << endl;
                S.atualiza_push(grafo);
            
                chamou = "Perturbação EXCLUIR";
                assert(S.checa_solucao(grafo, chamou));
                
                indice_route.pop_back();
                for (int j = 0; j < indice_route.size(); j++)
                {
                    if (indice_route[j] == rota.route.size() - 1)
                    {
                        indice_route.erase(indice_route.begin() + j);
                        break;
                    }
                }
                std::shuffle(indice_route.begin(), indice_route.end(), gen);
                quantos_excluir--;
                n = 0;
                continue;
            }

            // if (Busca_local::swap_Out_rotas(grafo, S, rota, i, i + 1, best_improvement))
            // {
            //     // cout <<endl<< "Efetuando Swap perturbaçao" << endl;
            //     // cout<<rota<<endl;
            //     // cout << "Rota: " << rota.id << " - Vertice[" << swap_out[0][3] << "] = " << swap_out[0][0] << " (SAI) " << " |Swap| Vertice[" << swap_out[0][3]<< "] = " << swap_out[1][0] << " (ENTRA)" << std::endl;

            //     S.atualiza_push(grafo);

            //     chamou = "Perturbação - Swap Out";
            //     assert(S.checa_solucao(grafo, chamou));

            //     std::shuffle(indice_route.begin(), indice_route.end(), gen);
            //     quantos_excluir--;
            //     n = 0;
            //     continue;
            // }

            // // swap_intra


            // if(S.rotas.size() != 0){
            //     Caminho rota2 = S.rotas.top();
            //     S.rotas.pop();

            //     if (Busca_local::swap_intra_rotas(grafo, S, rota, rota2, i, i + 1, best_improvement))
            //     {

            //         // S.rotas.push(rota);
            //         S.rotas.push(rota2);
            //         S.atualiza_push(grafo);
            //         chamou = "Perturbação - Swap Intra";
            //         assert(S.checa_solucao(grafo, chamou));

            //         std::shuffle(indice_route.begin(), indice_route.end(), gen);
            //         quantos_excluir--;
            //         n = 0;
            //         continue;
            //     }
            // }else if(aux_rotas.size() != 0){
            //     Caminho rota2 = aux_rotas.top();
            //     aux_rotas.pop();

            //     if (Busca_local::swap_intra_rotas(grafo, S, rota, rota2, i, i + 1, best_improvement))
            //     {

            //         // S.rotas.push(rota);
            //         aux_rotas.push(rota2);
            //         S.atualiza_push(grafo);
            //         chamou = "Perturbação - Swap Intra";
            //         assert(S.checa_solucao(grafo, chamou));

            //         std::shuffle(indice_route.begin(), indice_route.end(), gen);
            //         quantos_excluir--;
            //         n = 0;
            //         continue;
            //     }
            // }



            n++;
        }
    }

    return S;
}
