cmake_minimum_required(VERSION 3.16)
project(MyProject)

# Define o padrão C++17
set(CMAKE_CXX_STANDARD 17)

# Adiciona os arquivos de código-fonte ao executável
add_executable(MyProject
    local_search_patrulhamento.cpp
    Instance.cpp
    Caminho.cpp
    Sol.cpp
    Busca_local.cpp
    Perturbacao.cpp
    Utils.cpp
)

# Adiciona uma regra customizada para executar o programa com os argumentos fornecidos
add_custom_target(run
    COMMAND MyProject output_farol.txt 120 15 20 120
    DEPENDS MyProject
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
)

# Remover esta linha incorreta
# add_executable(MyProject ${C:\Users\athus\Faculdade\6 periodo\PIBIT\solucao\pibit-rotas-pm})
