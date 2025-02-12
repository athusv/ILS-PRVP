# Nome do executável
TARGET = my_program

# Compilador
CXX = g++

# Flags de compilação
# CXXFLAGS = -std=c++17 -Wall
CXXFLAGS = -std=c++17 -w


# Arquivos de origem (.cpp)
SRCS = local_search_patrulhamento.cpp Instance.cpp Caminho.cpp Sol.cpp Busca_local.cpp Perturbacao.cpp Utils.cpp

# Arquivos objeto (.o), gerados após a compilação
OBJS = $(SRCS:.cpp=.o)

# Número de execuções
NUM_RUNS = 3

# Lista de instâncias (arquivos de entrada)
# INSTANCES = 21_1_360_180_20.txt 10_1_360_120_20.txt 16_1_360_120_20.txt 21_1_360_120_20.txt 10_1_360_90_20.txt 16_1_360_90_20.txt 21_1_360_90_20.txt 

INSTANCES = 107_1_360_120_20.txt 107_2_360_120_20.txt 107_5_360_120_20.txt 107_7_360_120_20.txt 47_1_360_120_20.txt 47_2_360_120_20.txt 47_5_360_120_20.txt 47_7_360_120_20.txt 10_1_360_90_20.txt 16_1_360_90_20.txt 21_1_360_90_20.txt 10_1_360_120_20.txt 16_1_360_120_20.txt 21_1_360_120_20.txt 10_1_360_180_20.txt 16_1_360_180_20.txt 21_1_360_180_20.txt 10_1_360_360_20.txt 16_1_360_360_20.txt 21_1_360_360_20.txt 

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


# run:
# 	@echo "Executando $(NUM_RUNS) vezes e calculando a média e a melhor pontuação para cada instância"
# 	@for instance in $(INSTANCES); do \
# 		total_score=0; \
# 		total_iterations=0; \
# 		total_construtivo=0; \
# 		best_score=0; \
# 		best_seed=0; \
# 		best_construtivo_score=0; \
# 		best_melhoria=0; \
# 		output_file="resultados_$${instance}"; \
# 		echo "Resultados da instância $$instance" > $$output_file; \
# 		for i in $$(seq 1 $(NUM_RUNS)); do \
# 			echo "Execução $$i - Instância: $$instance"; \
# 			output=$$(./$(TARGET) $$instance); \
# 			echo "$$output" | tee -a $$output_file; \
# 			# Verifica se a saída é nula \
# 			if [ -z "$$output" ]; then \
# 				echo "Erro: Saída nula para $$instance" | tee -a $$output_file; \
# 				continue; \
# 			fi; \
# 			# Captura da seed \
# 			seed=$$(echo "$$output" | grep "Seed: " | awk -F "Seed: " '{print $$2}' | awk '{print $$1}'); \
# 			# Captura da pontuação do ILS \
# 			score=$$(echo "$$output" | grep "Solução ILS - Score" | awk -F ": " '{print $$2}' | awk '{print $$1}'); \
# 			# Captura da pontuação do Construtivo \
# 			construtivo_score=$$(echo "$$output" | grep "Solução Construtivo - Score" | awk -F ": " '{print $$2}' | awk '{print $$1}'); \
# 			# Captura das iterações \
# 			iterations=$$(echo "$$output" | grep "Iteração" | awk -F ": " '{print $$2}'); \
# 			# Captura da melhoria \
# 			melhoria=$$(echo "$$output" | grep "Melhoria de" | awk -F " " '{print $$3}' | sed 's/%//'); \
# 			# Verifica se os valores estão nulos \
# 			if [ -z "$$score" ] || [ -z "$$construtivo_score" ]; then \
# 				echo "Erro: Valores de pontuação nulos para $$instance" | tee -a $$output_file; \
# 				continue; \
# 			fi; \
# 			# Somando score, iterações e construtivo \
# 			total_score=$$(echo "$$total_score + $$score" | bc); \
# 			total_iterations=$$(echo "$$total_iterations + $$iterations" | bc); \
# 			total_construtivo=$$(echo "$$total_construtivo + $$construtivo_score" | bc); \
# 			# Comparando a melhor pontuação \
# 			if [ $$(echo "$$score > $$best_score" | bc -l) -eq 1 ]; then \
# 				best_score=$$score; \
# 				best_seed=$$seed; \
# 				best_construtivo_score=$$construtivo_score; \
# 				best_melhoria=$$melhoria; \
# 			fi; \
# 			echo "Execução $$i completa." | tee -a $$output_file; \
# 		done; \
# 		# Calculando médias \
# 		media_score=$$(echo "scale=3; $$total_score / $(NUM_RUNS)" | bc); \
# 		media_iterations=$$(echo "scale=3; $$total_iterations / $(NUM_RUNS)" | bc); \
# 		media_construtivo=$$(echo "scale=3; $$total_construtivo / $(NUM_RUNS)" | bc); \
# 		# Exibindo resultados e salvando no arquivo \
# 		echo "**** Instância: $$instance -- Seed da melhor execução: $$best_seed" | tee -a $$output_file; \
# 		echo "**** Instância: $$instance -- Média de iterações: $$media_iterations" | tee -a $$output_file; \
# 		echo "**** Instância: $$instance -- Pontuação do Construtivo da melhor execução: $$best_construtivo_score" | tee -a $$output_file; \
# 		echo "**** Instância: $$instance -- Média de pontuação ILS: $$media_score" | tee -a $$output_file; \
# 		echo "**** Instância: $$instance -- Melhor pontuação: $$best_score" | tee -a $$output_file; \
# 		echo "**** Instância: $$instance -- Melhoria na melhor execução: $$best_melhoria%" | tee -a $$output_file; \
# 		echo "."; \
# 	done;

# run:
# 	@echo "Executando $(NUM_RUNS) vezes e calculando a média e a melhor pontuação para cada instância"
# 	@for instance in $(INSTANCES); do \
# 		total_score=0; \
# 		best_score=0; \
# 		for i in $$(seq 1 $(NUM_RUNS)); do \
# 			echo "Execução $$i - Instância: $$instance"; \
# 			output=$$(./$(TARGET) $$instance); \
# 			echo "$$output"; \
# 			score=$$(echo "$$output" | grep "Solução ILS - Score" | awk -F ": " '{print $$2}' | awk '{print $$1}'); \
# 			# echo "$$score"; \
# 			total_score=$$(echo "$$total_score + $$score" | bc); \
# 			# Comparação para ponto flutuante usando bc \
# 			if [ $$(echo "$$score > $$best_score" | bc) -eq 1 ]; then \
# 				best_score=$$score; \
# 			fi; \
# 			echo "."; \
# 		done; \
# 		media=$$(echo "scale=3; $$total_score / $(NUM_RUNS)" | bc); \
# 		echo "."; \
# 		echo "**** Instância: $$instance -- Média de pontuação: $$media"; \
# 		echo "**** Instância: $$instance -- Melhor pontuação: $$best_score"; \
# 		echo "."; \
# 	done;




# Executa o programa com os argumentos fornecidos
run: $(TARGET)
	./$(TARGET) 47_5_360_120_20.txt
	# ./$(TARGET) 16_1_360_180_20.txt
	# ./$(TARGET) 21_1_360_90_20.txt
	# ./$(TARGET) 107_5_360_120_20.txt


