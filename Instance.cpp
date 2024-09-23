#include "Instance.h"
#include <vector>
#include <string>

Instance::Instance(const string &filename, int &tprot, int &tparada, double &velociade)
{
    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "Erro ao abrir o arquivo!" << endl;
        exit(1);
    }

    t_prot = tprot * 60;
    t_parada = tparada * 60;

    file >> qt_vertices;
    //cout << "Quantidade de vértices: " << qt_vertices << endl;
    file >> veiculos;
    // veiculos = 4;
    file >> tmax;
    // tmax = 60 * 8;

    distancia_matriz.resize(qt_vertices, vector<double>(qt_vertices, 0));
    int id;
    double score;

    for (int i = 0; i < qt_vertices; i++) {
        file >> id >> score;
        //cout << "Vértice " << id << " com score " << score << endl;
        score_vertices.push_back(score);
    }

	double aux;
    double metros_p_segundo = velociade / 3.6;
    for (int i = 0; i < qt_vertices; i++)
    {
        for (int j = 0; j < qt_vertices; j++)
        {
            file >> aux;
            distancia_matriz[i][j] = aux/metros_p_segundo;
        }
    }

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
