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

    file >> numVertices;
    // cout << "Quantidade de vértices: " << qt_vertices << endl;
    file >> maxTime;
    // cout << "Tempo máximo lido: " << tmax << endl;
    file >> protectionTime;
    int numVehicleTypes;
    file >> numVehicleTypes;
    // cout << "Quantidade de tipos de veículos: " << quantos_tipos_veiculos << endl;

    for(int i = 0; i < numVehicleTypes; i++){
        int numVehiclesType;
        file >> numVehiclesType;
        vehicleTypes.push_back(numVehiclesType);
        // cout << "Tipo de veículo " << i << ": " << quantos_cada_tipo << " unidades" << endl;
    }
    for (int i = 0; i < numVehicleTypes; i++){
        int speed_type;
        file >> speed_type;
        speed.push_back(speed_type);
        // cout << "Velocidade do tipo " << i << ": " << speed_type << " km/h" << endl;
    }

    numVehicles = 0;
    for(int i = 0; i < vehicleTypes.size(); i++){
        numVehicles += vehicleTypes[i];
    }
    // cout << "Quantidade total de veículos: " << veiculos << endl;

    maxTime = maxTime * 60;
    protectionTime = protectionTime * 60;
    stopTime = 15 * 60;
    // cout << "t_max: " << t_max << ", t_prot: " << t_prot << ", t_parada: " << t_parada << endl;

    
    int id;
    double score;

    for (int i = 0; i < numVertices; i++) {
        file >> id >> score;
        vertexScores.push_back(score);
        // cout << "Vértice " << id << " com score " << score << endl;
    }

    distanceMatrix.resize(numVertices, vector<double>(numVertices, 0));
    double aux;
    for (int i = 0; i < numVertices; i++)
    {
        for (int j = 0; j < numVertices; j++)
        {
            file >> aux;
            distanceMatrix[i][j] = aux;
            // if (i == 0 && j < 5) { // Exemplo: imprime as primeiras 5 distâncias da primeira linha
                // cout << "Distância de " << i << " para " << j << ": " << aux << " metros" << endl;
            // }
        }
    }
    
    totalIterations = 0;
    cout << "Leitura do arquivo concluída." << endl;

    file.close();
}

ostream &operator<<(ostream &os, const Instance &instance)
{
    os << "Quantidade de Vértices: " << instance.numVertices << endl;
    os << "Número de Veículos: " << instance.numVehicles << endl;
    os << "Tempo Máximo (t_max): " << instance.maxTime << endl;
    os << "Tempo de Proteção (t_prot): " << instance.protectionTime << endl;
    os << "Tempo de Parada (t_parada): " << instance.stopTime << endl;
    for (int i = 0; i < instance.vehicleTypes.size(); i++) {
        os << "Tipo de veiculo " << i << ": " << instance.vehicleTypes[i] << endl;
        os << "Velocidade do tipo " << i << ": " << instance.speed[i] << " km/h" << endl;
    }

    os << "Scores dos Vértices: [";
    for (size_t i = 0; i < instance.vertexScores.size(); i++)
    {
        os << instance.vertexScores[i];
        if (i + 1 != instance.vertexScores.size())
        {
            os << ", ";
        }
    }
    os << "]" << endl;

    os << "Matriz de Distâncias: " << endl;
    for (size_t i = 0; i < instance.distanceMatrix.size(); i++)
    {
        os << "[" << i << "]: ";
        for (size_t j = 0; j < instance.distanceMatrix[i].size(); j++)
        {
            os << "[" << j << "]: " << instance.distanceMatrix[i][j];
            if (j + 1 != instance.distanceMatrix[i].size())
            {
                os << ", ";
            }
        }
        os << endl;
    }

    return os;
}

