#include "Perturbacao.h"

// Sol &Perturbacao::perturbacao(Instance &grafo, Sol &s0, std::mt19937 &gen)
// { // rotornar
//     std::string chamou;
//     std::uniform_int_distribution<int> dis(1, 8);
//     int numero_aleatorio = dis(gen);
//     double porcentagem_perturbacao = static_cast<double>(numero_aleatorio) * 0.1;
//     // std::cout << "Porcentagem de perturbação: " << porcentagem_perturbacao << std::endl;

//     std::priority_queue<Caminho> aux_rotas;

//     while (!s0.rotas.empty())
//     {
//         Caminho rota = s0.rotas.top();
//         s0.rotas.pop();
//         int quantos_excluir = static_cast<int>(rota.route.size() * porcentagem_perturbacao);
//         // int quantos_excluir = static_cast<int>(rota.route.size() * 0.1);
//         // std::cout << "Quantos excluir = " << quantos_excluir << std::endl;
//         for (int i = 1; i < rota.route.size() - 1; i++)
//         {
//             int id = rota.id;
//             int vertice = rota.route[i];

//             std::vector<double> exclui_vertice = Utils::p_excluir(grafo, s0.visited_vertices, rota, i);
//             if (exclui_vertice[0] != -1)
//             {
//                 // cout << rota << endl;
//                 // s0.print_visited(8, 9);
//                 std::cout << "Rota:" << rota.id << " - Vertice: " << exclui_vertice[0] << " - Score: " << exclui_vertice[1]  << " - Impacto: " << exclui_vertice[2] << std::endl;
//                 rota.excluir(exclui_vertice, s0.visited_vertices, s0.score, s0.custo);
//                 s0.atualiza_push(grafo);
//                 quantos_excluir--;
//                 chamou = "Pertubaçao EXCLUIR";
//                 // cout<<rota<<endl;
//                 assert(s0.checa_solucao(grafo, chamou));
//             }
//             else{
//                 vector<vector<double>> swap_out = Utils::swap_perturbacao(grafo, s0, rota, i);
//                 if (swap_out[0][0] != -1)
//                 {
//                     // cout << "SWAP FORA DA ROTA" << endl;
//                     // std::cout << s0 << std::endl;
//                     cout << "Rota: " << rota.id << " - Vertice[" << swap_out[0][3] << "] = " << swap_out[0][0] << " (SAI) " << " |Swap| Vertice[" << swap_out[0][3] << "] = " << swap_out[1][0] << " (ENTRA) " << endl;
//                     rota.excluir(swap_out[0], s0.visited_vertices, s0.score, s0.custo);
//                     rota.incert(swap_out[1], s0.visited_vertices, s0.score, s0.custo);
//                     s0.rotas.push(rota);
//                     s0.atualiza_push(grafo);
//                     quantos_excluir--;
//                     chamou = "Perturbaçao - Swap Out";
//                     assert(s0.checa_solucao(grafo, chamou));
//                 }
//             }
//             if (quantos_excluir == 0)
//                 break;
//         }
//         aux_rotas.push(rota);
//     }
//     s0.rotas = aux_rotas;

//     return s0;
// }

Sol &Perturbacao::perturbacao(Instance &grafo, Sol &s0, std::mt19937 &gen)
{
    std::string chamou;
    std::uniform_int_distribution<int> dis(1, 8);
    int numero_aleatorio = dis(gen);
    double porcentagem_perturbacao = static_cast<double>(numero_aleatorio) * 0.1;

    std::priority_queue<Caminho> aux_rotas;

    while (!s0.rotas.empty())
    {
        Caminho rota = s0.rotas.top();
        s0.rotas.pop();
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

            std::vector<double> exclui_vertice = Utils::p_excluir(grafo, s0.visited_vertices, rota, i);
            if (exclui_vertice[0] != -1)
            {
                // std::cout << "Rota:" << rota.id << " - Vertice[" << i << "]: " << exclui_vertice[0] << " - Score: " << exclui_vertice[1] << " - Impacto: " << exclui_vertice[2] << std::endl;

                rota.excluir(exclui_vertice, s0.visited_vertices, s0.score, s0.custo);
                s0.atualiza_push(grafo);

                chamou = "Perturbação EXCLUIR";
                assert(s0.checa_solucao(grafo, chamou));

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

            // std::vector<std::vector<double>> swap_out = Utils::swap_perturbacao(grafo, s0, rota, i);
            // if (swap_out[0][0] != -1)
            // {
            //     std::cout << "Rota: " << rota.id << " - Vertice[" << swap_out[0][3] << "] = "
            //               << swap_out[0][0] << " (SAI) " << " |Swap| Vertice[" << swap_out[0][3]
            //               << "] = " << swap_out[1][0] << " (ENTRA)" << std::endl;

            //     rota.excluir(swap_out[0], s0.visited_vertices, s0.score, s0.custo);
            //     rota.incert(swap_out[1], s0.visited_vertices, s0.score, s0.custo);
            //     s0.rotas.push(rota);
            //     s0.atualiza_push(grafo);

            //     std::cout << std::endl
            //               << rota << std::endl
            //               << std::endl;

            //     chamou = "Perturbação - Swap Out";
            //     assert(s0.checa_solucao(grafo, chamou));

            //     indice_route.pop_back();
            //     for (int j = 0; j < indice_route.size(); j++)
            //     {
            //         if (indice_route[j] == rota.route.size() - 1)
            //         {
            //             indice_route.erase(indice_route.begin() + j);
            //             break;
            //         }
            //     }
            //     std::shuffle(indice_route.begin(), indice_route.end(), gen);
            //     quantos_excluir--;
            //     n = 0;
            //     continue;
            // }

            n++;
        }

        aux_rotas.push(rota);
    }
    s0.rotas = aux_rotas;

    return s0;
}
