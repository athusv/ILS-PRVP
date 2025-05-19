# Nome do executável
TARGET = main

# Compilador
CXX = g++

# Flags de compilação
# CXXFLAGS = -std=c++17 -Wall
CXXFLAGS = -std=c++17 -w


# Arquivos de origem (.cpp)
SRCS = main.cpp Instance.cpp Route.cpp Solution.cpp Utils.cpp

# Arquivos objeto (.o), gerados após a compilação
OBJS = $(SRCS:.cpp=.o)

# Número de execuções
NUM_RUNS = 1

# Lista de instâncias (arquivos de entrada)
INSTANCES = 74_2_360_360_5.txt 94_2_360_360_5.txt 

# Lista de valores do segundo parâmetro
PARAMS = 120

# Regra padrão: compilar o projeto
all: $(TARGET)

# Regra para gerar o executável
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Regra para compilar arquivos .cpp em .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Limpa os arquivos objeto (.o) e o executável
clean:
	rm -f $(OBJS) $(TARGET)

# Executa o programa com os argumentos fornecidos
run: $(TARGET)
            #   instancia     T_prot(min) / T_parada(min) / Velocidade(Km/h) 
	./$(TARGET) 47_5_360_120_20_teste.txt 120
	# ./$(TARGET) 16_1_360_120_20.txt

	

