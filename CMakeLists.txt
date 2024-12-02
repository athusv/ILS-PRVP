# Versão mínima do CMake necessária
cmake_minimum_required(VERSION 3.10)

# Nome do projeto e definição do padrão de C++
project(my_program LANGUAGES CXX)

# Definindo o padrão de compilação C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Habilitando avisos
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

# Lista de arquivos fonte
set(SRCS
        local_search_patrulhamento.cpp
        Instance.cpp
        Caminho.cpp
        Sol.cpp
        Busca_local.cpp
        Perturbacao.cpp
        Utils.cpp
)

# Adicionando o executável
add_executable(${PROJECT_NAME} ${SRCS})

# Definindo o alvo e flags de compilação
target_compile_options(${PROJECT_NAME} PRIVATE -std=c++17 -Wall)

# Regra de limpeza dos arquivos objeto e executáveis
add_custom_target(clean_files
        COMMAND ${CMAKE_COMMAND} -E remove ${PROJECT_NAME}
        COMMAND ${CMAKE_COMMAND} -E remove *.o
        COMMENT "Limpeza dos arquivos objeto e executáveis."
)

# Adicionando um comando para rodar o programa sem parâmetros fixos
add_custom_target(run
        COMMAND ${CMAKE_COMMAND} -E echo "Executando ${PROJECT_NAME}"
        COMMAND ${PROJECT_NAME}
        DEPENDS ${PROJECT_NAME}
        COMMENT "Executando o programa."
)
