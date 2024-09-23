# Nome do executável
TARGET = my_program

# Compilador
CXX = g++

# Flags de compilação
CXXFLAGS = -std=c++17 -Wall

# Arquivos de origem (.cpp)
SRCS = local_search_patrulhamento.cpp Instance.cpp Caminho.cpp Sol.cpp Busca_local.cpp Perturbacao.cpp Utils.cpp

# Arquivos objeto (.o), gerados após a compilação
OBJS = $(SRCS:.cpp=.o)

# Número de execuções
NUM_RUNS = 10

# Lista de instâncias (arquivos de entrada)
INSTANCES = input12.txt input16.txt input18.txt input23.txt

# Lista de valores do segundo parâmetro
PARAMS = 180 120 90

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

# Executa o programa com os argumentos fornecidos e calcula a média
run:
	@echo "Salvando resultados em resultados.txt"; \
	echo "Resultados das execuções:" > resultados.txt; \
	for param in $(PARAMS); do \
		for instance in $(INSTANCES); do \
			echo "Rodando para a instância: $$instance | T_prot: $$param"; \
			total_score=0; \
			for i in $$(seq 1 $(NUM_RUNS)); do \
				echo "  Execução $$i para $$instance | T_prot $$param"; \
				output=$$(./$(TARGET) $$instance $$param 15 20); \
				echo "$$output"; \
				score=$$(echo "$$output" | grep "Solução ILS - Score" | awk -F ": " '{print $$2}' | awk '{print $$1}'); \
				total_score=$$(echo "$$total_score + $$score" | bc); \
			done; \
			media=$$(echo "scale=3; $$total_score / $(NUM_RUNS)" | bc); \
			echo "."; \
			echo "***** Instância: $$instance | T_prot: $$param -- Média de pontuação: $$media"; \
			echo "***** Instância: $$instance | T_prot: $$param -- Média de pontuação: $$media" >> resultados.txt; \
			echo "."; \
		done; \
	done


# Executa o programa com os argumentos fornecidos e calcula a média
# run:
# 	@for param in $(PARAMS); do \
# 		for instance in $(INSTANCES); do \
# 			echo "Rodando para a instância: $$instance | T_prot: $$param"; \
# 			total_score=0; \
# 			for i in $$(seq 1 $(NUM_RUNS)); do \
# 				echo "  Execução $$i para $$instance | T_prot $$param"; \
# 				output=$$(./$(TARGET) $$instance $$param 15 20); \
# 				echo "$$output"; \
# 				score=$$(echo "$$output" | grep "Solução ILS - Score" | awk -F ": " '{print $$2}' | awk '{print $$1}'); \
# 				total_score=$$(echo "$$total_score + $$score" | bc); \
# 			done; \
# 			media=$$(echo "scale=3; $$total_score / $(NUM_RUNS)" | bc); \
# 			echo "."; \
# 			echo "***** Instancia: $$instance | T_prot: $$param -- Média de pontuação: $$media"; \
# 			echo ".";\
# 		done; \
# 	done

# # Executa o programa com os argumentos fornecidos e calcula a média
# run:
# 	@echo "Executando $(NUM_RUNS) vezes e calculando a média de pontuação"
# 	@total_score=0; \
# 	for i in $$(seq 1 $(NUM_RUNS)); do \
# 		echo "Execução $$i"; \
# 		output=$$(./$(TARGET) input23.txt 120 15 20); \
# 		echo "$$output"; \
# 		score=$$(echo "$$output" | grep "Solução ILS - Score" | awk -F ": " '{print $$2}' | awk '{print $$1}'); \
# 		# echo "$$score";\
# 		total_score=$$(echo "$$total_score + $$score" | bc); \
# 	done; \
# 	media=$$(echo "scale=3; $$total_score / $(NUM_RUNS)" | bc); \
# 	echo"."; \
# 	echo "**** Média de pontuação: $$media"


# Executa o programa com os argumentos fornecidos
# run: $(TARGET)
# #               instancia     T_prot(min) / T_parada(min) / Velocidade(Km/h) 
# 	./$(TARGET) input23.txt 90 15 20
	

