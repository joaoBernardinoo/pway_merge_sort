import sys
import heapq
import os
import math
import tempfile
import shutil

class ExternalSorter:
    """
    Implementa ordenação externa utilizando o método de mesclagem balanceada p-way com
    seleção por substituição para gerar sequências iniciais (runs).
    """

    def __init__(self, p_ways, input_file, output_file):
        """
        Inicializa o ordenador externo.

        Args:
            p_ways (int): O número de caminhos (p) para a mesclagem.
            input_file (str): Caminho para o arquivo de entrada.
            output_file (str): Caminho para o arquivo de saída.
        """
        if p_ways < 2:
            raise ValueError("O valor de 'p' (caminhos) deve ser pelo menos 2.")
        
        self.p = p_ways
        self.input_file = input_file
        self.output_file = output_file
        self.temp_dir = tempfile.mkdtemp(prefix="ext_sort_")
        self.total_regs = 0
        self.initial_runs = 0
        self.passes = 0

    def sort(self):
        """
        Executa o processo completo de ordenação externa em fases distintas.
        Este método orquestra todo o fluxo de trabalho da ordenação, desde a geração
        de sequências ordenadas iniciais (runs) até a mesclagem final no arquivo de saída.

        O processo inclui:
        1. Geração de sequências iniciais ordenadas a partir do arquivo de entrada.
        2. Mesclagem dessas sequências iterativamente até que reste apenas um arquivo ordenado.
        3. Finalização copiando o resultado para o arquivo de saída.
        4. Impressão de estatísticas sobre o processo de ordenação.
        5. Limpeza dos arquivos temporários.

        Returns:
            None
        """
        try:
            # Fase 1: Gera sequências iniciais ordenadas (runs)
            run_files = self._generate_initial_runs()
            self.initial_runs = len(run_files)

            if not run_files:
                print("Arquivo de entrada está vazio ou não pôde ser lido.")
                # Cria um arquivo de saída vazio
                with open(self.output_file, 'w') as f:
                    f.close()
                return

            # Fase 2: Mescla as sequências iterativamente
            final_file = self._merge_runs(run_files)

            # Fase 3: Finaliza copiando o resultado para o arquivo de saída
            self._finalize_output(final_file)

            # Fase 4: Imprime as estatísticas da ordenação
            self._print_stats()

        finally:
            # Fase 5: Limpa os arquivos e diretórios temporários criados durante a ordenação
            self._cleanup()

    def _generate_initial_runs(self):
        """
        Gera sequências iniciais ordenadas (runs) a partir do arquivo de entrada utilizando
        o algoritmo de seleção por substituição. Este método simula a memória principal com
        uma heap de tamanho 'p', onde 'p' é o número de caminhos. Ele lê o arquivo de entrada,
        cria runs ordenados e os grava em arquivos temporários.

        Returns:
            list: Uma lista de caminhos de arquivos temporários criados para os runs.
        """
        run_files = []
        try:
            with open(self.input_file, 'r') as input_file:
                # Inicializa a heap de memória principal com os primeiros 'p' registros
                main_memory_heap = []
                for _ in range(self.p):
                    line = input_file.readline()
                    if not line:
                        break
                    heapq.heappush(main_memory_heap, int(line.strip()))
                    self.total_regs += 1
                
                if not main_memory_heap:
                    return run_files

                # O armazenamento secundário guarda os elementos para o próximo run
                secondary_storage = []
                run_index = 0

                while main_memory_heap or secondary_storage:
                    if not main_memory_heap:
                        # Inicia um novo run com os elementos da memória secundária
                        main_memory_heap = secondary_storage
                        secondary_storage = []
                        heapq.heapify(main_memory_heap)

                    # Cria um novo arquivo temporário para o run
                    run_file_path = os.path.join(self.temp_dir, f"run_{run_index}.tmp")
                    run_files.append(run_file_path)
                    run_index += 1

                    with open(run_file_path, 'w') as run_file:
                        last_written_value = -math.inf
                        
                        while main_memory_heap:
                            # Extrai o menor elemento da heap
                            smallest_value = heapq.heappop(main_memory_heap)
                            run_file.write(f"{smallest_value}\n")
                            last_written_value = smallest_value

                            # Lê o próximo elemento do arquivo de entrada
                            next_line = input_file.readline()
                            if next_line:
                                self.total_regs += 1
                                next_value = int(next_line.strip())
                                
                                # Seleção por Substituição:
                                # Se o novo valor for maior ou igual ao último valor escrito,
                                # ele pode fazer parte do run atual.
                                if next_value >= last_written_value:
                                    heapq.heappush(main_memory_heap, next_value)
                                else:
                                    # Caso contrário, armazena-o para o próximo run.
                                    secondary_storage.append(next_value)
                            
        except FileNotFoundError:
            print(f"Erro: Arquivo de entrada '{self.input_file}' não encontrado.", file=sys.stderr)
            return []
        except ValueError:
            print(f"Erro: Arquivo de entrada '{self.input_file}' contém dados não numéricos.", file=sys.stderr)
            return []
            
        return run_files

    def _merge_runs(self, run_files):
        """
        Mescla os runs iniciais através de passagens sucessivas até que reste somente um run.
        Este método implementa o processo de mesclagem p-way, agrupando os runs em conjuntos de tamanho 'p'
        e mesclando cada conjunto em um único arquivo de run até que um arquivo ordenado final seja produzido.

        Args:
            run_files (list): Lista de caminhos para os arquivos temporários dos runs iniciais.

        Returns:
            str ou None: Caminho para o arquivo final mesclado, ou None se não houver arquivos para mesclar.
        """
        if not run_files:
            return None
        if len(run_files) == 1:
            self.passes = 1 if self.total_regs > 0 else 0
            return run_files[0]
            
        # Calcula teoricamente o número de passes necessários
        if self.initial_runs > 0:
            self.passes = math.ceil(math.log(self.initial_runs, self.p))

        current_run_files = run_files
        pass_number = 0
        while len(current_run_files) > 1:
            pass_number += 1
            merged_run_files = []
            run_group_index = 0
            
            # Agrupa os runs atuais em conjuntos de tamanho 'p'
            for i in range(0, len(current_run_files), self.p):
                run_group = current_run_files[i:i + self.p]
                
                # Define o nome do arquivo de saída para esta operação de mesclagem
                merged_file_path = os.path.join(self.temp_dir, f"pass_{pass_number}_merged_{run_group_index}.tmp")
                merged_run_files.append(merged_file_path)
                run_group_index += 1
                
                # Realiza a mesclagem p-way para o grupo
                self._merge_group(run_group, merged_file_path)
            
            current_run_files = merged_run_files
            
        return current_run_files[0]

    def _merge_group(self, group_files, output_file_path):
        """
        Mescla um grupo de até 'p' arquivos de run em um único arquivo de saída.
        Este método realiza uma mesclagem p-way mantendo uma heap de mínimos dos menores elementos
        de cada arquivo do grupo, escrevendo o menor elemento no arquivo de saída e lendo o próximo
        elemento do arquivo correspondente.

        Args:
            group_files (list): Lista de caminhos dos arquivos de run no grupo.
            output_file_path (str): Caminho para o arquivo de saída do resultado mesclado.

        Returns:
            None
        """
        min_heap = []
        file_pointers = []

        try:
            # Abre todos os arquivos do grupo e lê a primeira linha de cada um
            for index, file_path in enumerate(group_files):
                file_handle = open(file_path, 'r')
                file_pointers.append(file_handle)
                first_line = file_handle.readline()
                if first_line:
                    heapq.heappush(min_heap, (int(first_line.strip()), index))

            with open(output_file_path, 'w') as output_file:
                while min_heap:
                    # Pega o menor elemento global e o índice do arquivo de onde ele veio
                    smallest_value, file_index = heapq.heappop(min_heap)
                    output_file.write(f"{smallest_value}\n")
                    
                    # Lê o próximo elemento do mesmo arquivo
                    next_line = file_pointers[file_index].readline()
                    if next_line:
                        heapq.heappush(min_heap, (int(next_line.strip()), file_index))
        finally:
            # Garante que todos os arquivos sejam fechados
            for file_handle in file_pointers:
                file_handle.close()

    def _finalize_output(self, final_file):
        """
        Finaliza o processo de ordenação copiando o conteúdo do arquivo final mesclado para o arquivo de saída.
        Se o arquivo final existir, ele é copiado para o local de saída e, em seguida, removido.

        Args:
            final_file (str ou None): Caminho para o arquivo final mesclado, ou None se não existir arquivo.

        Returns:
            None
        """
        if final_file:
            shutil.copy2(final_file, self.output_file)
            os.remove(final_file)

    def _print_stats(self):
        """
        Imprime as estatísticas finais do processo de ordenação no formato especificado.
        As estatísticas incluem o número total de registros, o número de caminhos (p),
        o número de runs iniciais e o número de passes de mesclagem.

        Returns:
            None
        """
        print(f"#Regs\tWays\t#Runs\t#Passes")
        print(f"{self.total_regs}\t{self.p}\t{self.initial_runs}\t{self.passes}")

    def _cleanup(self):
        """
        Remove os arquivos e diretórios temporários criados durante o processo de ordenação.
        Este método garante que todos os recursos temporários sejam limpos após a ordenação.

        Returns:
            None
        """
        if os.path.exists(self.temp_dir):
            for temp_file in os.listdir(self.temp_dir):
                os.remove(os.path.join(self.temp_dir, temp_file))
            os.rmdir(self.temp_dir)


def main():
    """
    Ponto de entrada do script. Faz o parse dos argumentos da linha de comando e inicia o processo de ordenação.
    """
    if len(sys.argv) != 4:
        print("Usage: python pways.py <p> <input_file> <output_file>", file=sys.stderr)
        sys.exit(1)

    try:
        p_ways = int(sys.argv[1])
        input_file = sys.argv[2]
        output_file = sys.argv[3]

        sorter = ExternalSorter(p_ways, input_file, output_file)
        sorter.sort()

    except ValueError as e:
        print(f"Erro: O valor de 'p' deve ser um inteiro. {e}", file=sys.stderr)
        sys.exit(1)
    except Exception as e:
        print(f"Ocorreu um erro inesperado: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
