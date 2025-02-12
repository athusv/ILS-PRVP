#include "Instance.h"
#include <vector>
#include <string>

Instance::Instance(const string &filename)
{
    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "Erro ao abrir o arquivo!" << endl;
        exit(1);
    }
    int tprot; double tmax;

    file >> qt_vertices;
    //cout << "Quantidade de vértices: " << qt_vertices << endl;
    file >> veiculos;
    // veiculos = 4;
    file >> tmax;
    // tmax = 60 * 8;
    file >> tprot;
    file >> velocidade;
    t_max = tmax*60;
    t_prot = tprot * 60;
    t_parada = 15 * 60;

    distancia_matriz.resize(qt_vertices, vector<double>(qt_vertices, 0));
    int id;
    double score;

    for (int i = 0; i < qt_vertices; i++) {
        file >> id >> score;
        //cout << "Vértice " << id << " com score " << score << endl;
        score_vertices.push_back(score);
    }

	double aux;
    double metros_p_segundo = velocidade / 3.6;
    for (int i = 0; i < qt_vertices; i++)
    {
        for (int j = 0; j < qt_vertices; j++)
        {
            file >> aux;
            distancia_matriz[i][j] = aux/metros_p_segundo;
        }
    }
    
    iteracoes_totais = 0;
    //for (int i = 0; i < qt_vertices; i++) {
        //for (int j = 0; j < qt_vertices; j++) {
            //file >> distancia_matriz[i][j];
            //if(i == 0){
                //cout << "Distância de " << i << " para " << j << ": " << distancia_matriz[i][j] << endl;
            //}
        //}
    //}

    file.close();
}

ostream &operator<<(ostream &os, const Instance &instance)
{
    os << "Quantidade de Vértices: " << instance.qt_vertices << endl;
    os << "Número de Veículos: " << instance.veiculos << endl;
    os << "Tempo Máximo (t_max): " << instance.t_max << endl;
    os << "Tempo de Proteção (t_prot): " << instance.t_prot << endl;
    os << "Tempo de Parada (t_parada): " << instance.t_parada << endl;
    os << "Velocidade: " << instance.velocidade << " km/h" << endl;

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
    for (size_t i = 0; i < instance.distancia_matriz.size(); i++)
    {
        os << "[" << i << "]: ";
        for (size_t j = 0; j < instance.distancia_matriz[i].size(); j++)
        {
            os << "[" << j << "]: " << instance.distancia_matriz[i][j];
            if (j + 1 != instance.distancia_matriz[i].size())
            {
                os << ", ";
            }
        }
        os << endl;
    }

    return os;
}

