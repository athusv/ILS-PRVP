#include "Instance.h"
#include <vector>
#include <string>

Instance::Instance(const string &filename)
{
    // cout << "Iniciando a leitura do arquivo: " << filename << endl;
    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "Erro ao abrir o arquivo!" << endl;
        exit(1);
    }
    int tprot; double tmax;
    int quantos_tipos_veiculos;
    file >> qt_vertices;
    // cout << "Quantidade de vértices: " << qt_vertices << endl;
    file >> tmax;
    // cout << "Tempo máximo lido: " << tmax << endl;
    file >> tprot;
    // cout << "Tempo de proteção lido: " << tprot << endl;
    file >> quantos_tipos_veiculos;
    // cout << "Quantidade de tipos de veículos: " << quantos_tipos_veiculos << endl;

    for(int i = 0; i < quantos_tipos_veiculos; i++){
        int quantos_cada_tipo;
        file >> quantos_cada_tipo;
        tipo_veiculo.push_back(quantos_cada_tipo);
        // cout << "Tipo de veículo " << i << ": " << quantos_cada_tipo << " unidades" << endl;
    }
    for (int i = 0; i < quantos_tipos_veiculos; i++){
        int velocidade_tipo;
        file >> velocidade_tipo;
        velocidade.push_back(velocidade_tipo);
        // cout << "Velocidade do tipo " << i << ": " << velocidade_tipo << " km/h" << endl;
    }

    veiculos = 0;
    for(int i = 0; i < tipo_veiculo.size(); i++){
        veiculos += tipo_veiculo[i];
    }
    // cout << "Quantidade total de veículos: " << veiculos << endl;

    t_max = tmax * 60;
    t_prot = tprot * 60;
    t_parada = 15 * 60;
    // cout << "t_max: " << t_max << ", t_prot: " << t_prot << ", t_parada: " << t_parada << endl;

    
    int id;
    double score;

    for (int i = 0; i < qt_vertices; i++) {
        file >> id >> score;
        score_vertices.push_back(score);
        // cout << "Vértice " << id << " com score " << score << endl;
    }

    distancia_metros.resize(qt_vertices, vector<double>(qt_vertices, 0));
    double aux;
    for (int i = 0; i < qt_vertices; i++)
    {
        for (int j = 0; j < qt_vertices; j++)
        {
            file >> aux;
            distancia_metros[i][j] = aux;
            // if (i == 0 && j < 5) { // Exemplo: imprime as primeiras 5 distâncias da primeira linha
                // cout << "Distância de " << i << " para " << j << ": " << aux << " metros" << endl;
            // }
        }
    }
    
    iteracoes_totais = 0;
    cout << "Leitura do arquivo concluída." << endl;

    file.close();
}

ostream &operator<<(ostream &os, const Instance &instance)
{
    os << "Quantidade de Vértices: " << instance.qt_vertices << endl;
    os << "Número de Veículos: " << instance.veiculos << endl;
    os << "Tempo Máximo (t_max): " << instance.t_max << endl;
    os << "Tempo de Proteção (t_prot): " << instance.t_prot << endl;
    os << "Tempo de Parada (t_parada): " << instance.t_parada << endl;
    for (int i = 0; i < instance.tipo_veiculo.size(); i++) {
        os << "Tipo de veiculo " << i << ": " << instance.tipo_veiculo[i] << endl;
        os << "Velocidade do tipo " << i << ": " << instance.velocidade[i] << " km/h" << endl;
    }

    os << "Scores dos Vértices: [";
    for (size_t i = 0; i < instance.score_vertices.size(); i++)
    {
        os << instance.score_vertices[i];
        if (i + 1 != instance.score_vertices.size())
        {
            os << ", ";
        }
    }
    os << "]" << endl;

    os << "Matriz de Distâncias: " << endl;
    for (size_t i = 0; i < instance.distancia_metros.size(); i++)
    {
        os << "[" << i << "]: ";
        for (size_t j = 0; j < instance.distancia_metros[i].size(); j++)
        {
            os << "[" << j << "]: " << instance.distancia_metros[i][j];
            if (j + 1 != instance.distancia_metros[i].size())
            {
                os << ", ";
            }
        }
        os << endl;
    }

    return os;
}

