import subprocess
import csv

# Lista de instâncias (substitua pelos nomes dos arquivos ou parâmetros corretos)
instancias = ["47_1_360_120_20.txt", "47_2_360_120_20.txt","47_5_360_120_20.txt","47_7_360_120_20.txt", "47_10_360_120_20.txt", "51_1_360_120_20.txt", "51_2_360_120_20.txt","51_5_360_120_20.txt","51_7_360_120_20.txt", "51_10_360_120_20.txt", "107_1_360_120_20.txt", "107_2_360_120_20.txt","107_5_360_120_20.txt","107_7_360_120_20.txt", "107_10_360_120_20.txt", "146_1_360_120_20.txt", "146_2_360_120_20.txt","146_5_360_120_20.txt","146_7_360_120_20.txt", "146_10_360_120_20.txt", "10_1_360_90_20.txt", "16_1_360_90_20.txt", "21_1_360_90_20.txt", "10_1_360_120_20.txt", "16_1_360_120_20.txt", "21_1_360_120_20.txt", "10_1_360_180_20.txt", "16_1_360_180_20.txt", "21_1_360_180_20.txt", "10_1_360_360_20.txt", "16_1_360_360_20.txt", "21_1_360_360_20.txt",]  # Exemplo com arquivos

# Nome do arquivo CSV de saída
csv_filename = "resultados_cond_15.csv"

# Abre o CSV e escreve os resultados
with open(csv_filename, "w", newline="") as csvfile:
    writer = csv.writer(csvfile)
    
    # Escreve o cabeçalho (ajuste conforme necessário)
    # instancia, seed_best, tempo, total_iterações, it_reset, mean_score, best_score,
    writer.writerow(["Instância", "Seed", "Tempo", "It_total", "It_reset", "Mean", "Best"])  

    for instancia in instancias:
        print(f"Rodando para {instancia}...")

        # Executa o programa C++ passando a instância como argumento
        process = subprocess.run(["./my_program", instancia], capture_output=True, text=True)
        
        # Divide a saída em linhas
        output_lines = process.stdout.strip().split("\n")

        # Extrai os dados da saída (ajuste conforme o formato da saída do seu C++)
        for line in output_lines:
            valores = line.split(",")  # Supondo que a saída seja separada por vírgulas
            writer.writerow(valores)  # Adiciona a instância no início da linha

print(f"Resultados salvos em {csv_filename}")
