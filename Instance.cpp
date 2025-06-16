#include "Instance.h"
#include <vector>
#include <string>
#include <algorithm>
#include <nlohmann/json.hpp>

// Construtor para leitura de arquivo (detecta automaticamente se é JSON ou formato texto)
Instance::Instance(const string &filename)
{
    // Verifica se o arquivo tem extensão .json
    bool isJsonFormat = false;
    size_t jsonPos = filename.find(".json");
    if (jsonPos != string::npos)
    {
        isJsonFormat = true;
    }

    // Abre o arquivo
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Erro ao abrir o arquivo: " << filename << endl;
        exit(1);
    }

    if (isJsonFormat)
    {
        // Leitura de arquivo JSON
        nlohmann::json jsonData;
        file >> jsonData;
        file.close();

        // Extrair os dados do JSON
        numVertex = jsonData["numVertex"].get<int>();
        maxTime = jsonData["maxTime"].get<int>() * 60;               // Converter para segundos
        protectionTime = jsonData["protectionTime"].get<int>() * 60; // Converter para segundos
        stopTime = 15 * 60;                                          // Padrão de 15 minutos em segundos

        // Ler tipos de veículos e velocidades
        vehicleTypes = jsonData["vehicleTypes"].get<vector<int>>();
        speed = jsonData["speed"].get<vector<int>>();

        // Calcular número total de veículos
        numVehicles = 0;
        for (int i = 0; i < vehicleTypes.size(); i++)
        {
            numVehicles += vehicleTypes[i];
        }

        // Ler scores dos vértices
        vertexScores = jsonData["scores"].get<vector<double>>();

        // Ler matriz de distâncias e calcular vértices mais próximos
        auto jsonCosts = jsonData["costs"];
        distanceMatrix.resize(numVertex, vector<double>(numVertex, 0));
        nearestVertices.resize(numVertex);

        for (int i = 0; i < numVertex; i++)
        {
            for (int j = 0; j < numVertex; j++)
            {
                distanceMatrix[i][j] = jsonCosts[i][j].get<double>();

                // Adicionar à lista de vértices próximos (exceto o próprio vértice)
                if (i != j)
                {
                    nearestVertices[i].push_back(VertexDistance(j, distanceMatrix[i][j]));
                }
            }

            // Ordenar a lista de vértices próximos por distância
            sort(nearestVertices[i].begin(), nearestVertices[i].end(),
                 [](const VertexDistance &a, const VertexDistance &b)
                 {
                     return a.distance < b.distance;
                 });

            // cout << "Vértice " << i << " tem " << nearestVertices[i].size() << " vértices próximos: [";
            // for (const auto &vertex : nearestVertices[i])
            // {
            //     cout << "(" << vertex.index << ", " << vertex.distance << "), ";
            // }
            // cout << "]" << endl;
        }
    }
    else
    {
        // Leitura de arquivo formato texto original
        file >> numVertex;
        // cout << "Quantidade de vértices: " << qt_vertices << endl;
        file >> maxTime;
        // cout << "Tempo máximo lido: " << tmax << endl;
        file >> protectionTime;
        int numVehicleTypes;
        file >> numVehicleTypes;
        // cout << "Quantidade de tipos de veículos: " << quantos_tipos_veiculos << endl;

        for (int i = 0; i < numVehicleTypes; i++)
        {
            int numVehiclesType;
            file >> numVehiclesType;
            vehicleTypes.push_back(numVehiclesType);
            // cout << "Tipo de veículo " << i << ": " << quantos_cada_tipo << " unidades" << endl;
        }
        for (int i = 0; i < numVehicleTypes; i++)
        {
            int speed_type;
            file >> speed_type;
            speed.push_back(speed_type);
            // cout << "Velocidade do tipo " << i << ": " << speed_type << " km/h" << endl;
        }

        numVehicles = 0;
        for (int i = 0; i < vehicleTypes.size(); i++)
        {
            numVehicles += vehicleTypes[i];
        }
        // cout << "Quantidade total de veículos: " << veiculos << endl;

        maxTime = maxTime * 60;
        protectionTime = protectionTime * 60;
        stopTime = 15 * 60;
        // cout << "t_max: " << t_max << ", t_prot: " << t_prot << ", t_parada: " << t_parada << endl;

        int id;
        double score;

        for (int i = 0; i < numVertex; i++)
        {
            file >> id >> score;
            vertexScores.push_back(score);
            // cout << "Vértice " << id << " com score " << score << endl;
        }

        distanceMatrix.resize(numVertex, vector<double>(numVertex, 0));
        nearestVertices.resize(numVertex);

        double aux;
        for (int i = 0; i < numVertex; i++)
        {
            for (int j = 0; j < numVertex; j++)
            {
                file >> aux;
                distanceMatrix[i][j] = aux;

                // Adicionar à lista de vértices próximos (exceto o próprio vértice)
                if (i != j)
                {
                    nearestVertices[i].push_back(VertexDistance(j, aux));
                }
                // if (i == 0 && j < 5) { // Exemplo: imprime as primeiras 5 distâncias da primeira linha
                // cout << "Distância de " << i << " para " << j << ": " << aux << " metros" << endl;
                // }
            }

            // Ordenar a lista de vértices próximos por distância
            sort(nearestVertices[i].begin(), nearestVertices[i].end(),
                 [](const VertexDistance &a, const VertexDistance &b)
                 {
                     return a.distance < b.distance;
                 });
        }
    }

    totalIterations = 0;
    file.close();
}

ostream &operator<<(ostream &os, const Instance &instance)
{
    os << "Quantidade de Vértices: " << instance.numVertex << endl;
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

    os << "\nVértices Mais Próximos (ordenados por distância): " << endl;
    for (size_t i = 0; i < instance.nearestVertices.size(); i++)
    {
        os << "Vértice " << i << ": ";
        for (size_t j = 0; j < instance.nearestVertices[i].size(); j++)
        {
            os << "[" << instance.nearestVertices[i][j].index << ":"
               << instance.nearestVertices[i][j].distance << "]";
            if (j + 1 != instance.nearestVertices[i].size())
            {
                os << ", ";
            }
        }
        os << endl;
    }

    return os;
}

