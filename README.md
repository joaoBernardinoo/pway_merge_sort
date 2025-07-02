Análise e Explicação do Trabalho de Ordenação Externa

Este documento detalha a implementação de um algoritmo de ordenação externa em Python, conforme as especificações do trabalho da disciplina MATA54 - Estrutura de Dados e Algoritmos II. O código implementa o método de mesclagem balanceada de p-caminhos (p-way merge sort), utilizando seleção por substituição para a geração das sequências iniciais ordenadas (runs).
1. Visão Geral do Problema

A ordenação externa é necessária quando o volume de dados a ser ordenado é maior do que a memória principal (RAM) disponível. A abordagem consiste em quebrar o arquivo grande em pedaços menores que caibam na memória, ordená-los individualmente (criando "runs" ou sequências ordenadas) e, em seguida, mesclar esses "runs" de volta em um único arquivo final ordenado.

O trabalho especifica duas técnicas cruciais para essa implementação:

    Seleção por Substituição (Replacement Selection): Um algoritmo inteligente para criar os "runs" iniciais. Sua principal vantagem é que, em média, ele produz "runs" com o dobro do tamanho da memória disponível, otimizando o processo de mesclagem subsequente.

    Mesclagem de p-caminhos (p-way Merge): O processo de combinar até p "runs" ordenados em um novo "run" maior e ordenado. O uso de uma heap mínima (min-heap) é fundamental para tornar essa mesclagem eficiente.

2. Estrutura do Código: A Classe ExternalSorter

O código está encapsulado na classe ExternalSorter, que gerencia todo o ciclo de vida do processo de ordenação.
__init__(self, p_ways, input_file, output_file)

    Objetivo: Inicializar o ordenador.

    Funcionalidade:

        Armazena os parâmetros essenciais: p (número de caminhos), input_file (arquivo de entrada) e output_file (arquivo de saída).

        Valida se p >= 2, uma restrição lógica do método.

        Cria um diretório temporário (temp_dir) para armazenar os arquivos de "run" intermediários. Isso mantém o diretório de trabalho limpo.

        Inicializa variáveis para as estatísticas finais: total_regs, initial_runs, passes.

sort(self)

    Objetivo: Orquestrar o processo de ordenação em fases.

    Fluxo de Trabalho:

        Fase 1: Geração de Runs (_generate_initial_runs): Invoca o método que lê o arquivo de entrada e cria os "runs" iniciais ordenados usando seleção por substituição.

        Fase 2: Mesclagem (_merge_runs): Pega a lista de "runs" iniciais e os mescla iterativamente, em passagens, até que reste apenas um arquivo.

        Fase 3: Finalização (_finalize_output): Copia o arquivo final mesclado (que está no diretório temporário) para o caminho de saída especificado pelo usuário.

        Fase 4: Estatísticas (_print_stats): Imprime as estatísticas coletadas no formato exigido.

        Fase 5: Limpeza (_cleanup): Remove o diretório temporário e todos os seus conteúdos.

3. Geração de Runs Iniciais: _generate_initial_runs

Este é o aalgoritmo de Seleção por Substituição.

    Simulação de Memória: A especificação restringe a memória a p registros. O código simula isso com duas estruturas:

        main_memory_heap: Uma heap mínima (min-heap) de tamanho máximo p. Ela representa a memória principal e sempre nos dá o menor elemento disponível com eficiência.

        secondary_storage: Uma lista simples que armazena temporariamente os números lidos do arquivo de entrada que são menores que o último número escrito no "run" atual. Esses números não podem fazer parte do "run" corrente e são "congelados" para o próximo.

    Como Funciona:

        A heap é preenchida com os primeiros p números do arquivo de entrada.

        Um loop principal começa, que continuará enquanto houver elementos na heap ou no armazenamento secundário.

        Dentro do loop, um novo arquivo de "run" é criado.

        O menor elemento é extraído da heap (heapq.heappop) e escrito no arquivo de "run" atual.

        Um novo número é lido do arquivo de entrada.

        A decisão chave acontece aqui:

            Se o novo número for maior ou igual ao último número escrito, ele pode fazer parte do "run" atual e é inserido na heap (heapq.heappush).

            Se o novo número for menor, ele quebraria a sequência ordenada. Portanto, ele é adicionado ao secondary_storage para ser usado no próximo "run".

        Quando a heap principal fica vazia, o "run" atual está completo. O processo recomeça movendo todos os elementos do secondary_storage para a heap, iniciando um novo "run".

Este método retorna uma lista com os caminhos para todos os arquivos de "run" criados.
4. Mesclagem dos Runs: _merge_runs e _merge_group
_merge_runs(self, run_files)

    Objetivo: Gerenciar as passagens de mesclagem.

    Lógica:

        Recebe a lista de "runs" iniciais.

        Calcula o número teórico de passagens necessárias: ceil(log_p(initial_runs)).

        Entra em um while loop que continua até que reste apenas um arquivo na lista de "runs".

        A cada iteração (uma "passagem"), ele agrupa os "runs" atuais em conjuntos de tamanho p.

        Para cada grupo, ele chama _merge_group para mesclá-los em um novo arquivo de "run".

        A lista de "runs" é atualizada com os novos arquivos mesclados, e o processo se repete.

_merge_group(self, group_files, output_file_path)

    Objetivo: Realizar a mesclagem de um grupo de até p "runs".

    Implementação:

        Abre todos os arquivos do grupo.

        Cria uma min_heap. Cada item na heap é uma tupla (valor, indice_do_arquivo).

        Lê a primeira linha de cada arquivo e insere na heap.

        Enquanto a heap não estiver vazia:
        a. Extrai o menor elemento global da heap (heapq.heappop). Este é o menor entre todos os elementos atuais de todos os arquivos.
        b. Escreve esse valor no arquivo de saída da mesclagem.
        c. Lê a próxima linha do mesmo arquivo de onde o valor foi extraído.
        d. Se uma linha foi lida, insere o novo (valor, indice_do_arquivo) na heap.

        Ao final, todos os arquivos de entrada são fechados.

5. Finalização e Execução
_finalize_output, _print_stats, _cleanup

Esses métodos auxiliares são diretos:

    _finalize_output: Usa shutil.copy2 para mover o resultado final para o local correto.

    _print_stats: Formata a saída das estatísticas exatamente como pedido na especificação.

    _cleanup: Usa os.remove e os.rmdir para limpar os arquivos temporários, garantindo que nenhum lixo seja deixado para trás.

main()

    Função: Ponto de entrada do script.

    Responsabilidades:

        Verificar se o número correto de argumentos de linha de comando foi fornecido.

        Converter o valor de p para inteiro.

        Instanciar ExternalSorter.

        Chamar o método sorter.sort() para iniciar o processo.

        Capturar exceções (ex: ValueError se p não for um número, FileNotFoundError, etc.) e exibir mensagens de erro amigáveis.

6. Conformidade com a Especificação

Requisito da Especificação
	

Como o Código Atende

Método p-way merge sort
	

Implementado através da classe ExternalSorter, com a lógica de mesclagem em _merge_runs e _merge_group.

Memória principal com no máximo p registros
	

Simulado em _generate_initial_runs pelo uso de uma min-heap (main_memory_heap) de tamanho p.

Seleção por Substituição para runs iniciais
	

Implementado em _generate_initial_runs com a main_memory_heap e o secondary_storage.

Intercalação baseada em heap mínima
	

Implementado em _merge_group com a min_heap para encontrar o menor elemento entre p arquivos.

Interface de linha de comando
	

A função main processa sys.argv para obter p, input_file e output_file.

Formato da saída de estatísticas
	

O método _print_stats imprime #Regs, Ways, #Runs, #Passes exatamente como solicitado.

Uso de arquivos temporários
	

O módulo tempfile é usado para criar um diretório seguro para os arquivos de "run" (.tmp).

Não usar ordenação interna
	

O código não carrega o arquivo inteiro para a memória em nenhum momento. A ordenação ocorre via manipulação de streams de arquivos.

Sem restrição de tamanho de arquivo
	

O processamento é feito linha a linha, permitindo que os arquivos de entrada e saída sejam arbitrariamente grandes.

Linguagem Python
	

O código foi desenvolvido em Python, uma das linguagens permitidas.
