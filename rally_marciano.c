/*
 * Simulação do Rally dos Robôs em Marte
 *
 * Este programa simula a movimentação de robôs em uma arena em Marte,
 * seguindo as regras descritas abaixo:
 * - Robôs com IDs menores têm prioridade de movimentação sobre robôs com IDs maiores.
 * - Quando um robô sem energia tem robôs vizinhos, ele rouba energia do robô com o menor ID.
 * - O roubo de energia ocorre apenas quando o robô vizinho tem pelo menos 2 unidades de energia,
 *   garantindo que o robô roubado não fique sem energia.
 * - Um robô perde energia ao se movimentar, e o movimento depende da energia disponível.
 *
 * A tarefa do código é modificar o comportamento para que o processamento de cada robô
 * ocorra de forma concorrente.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>

/* Tipos de objetos que podem estar presentes nas células da arena */
#define VAZIO '.'     // Célula vazia
#define PILAR 'x'     // Célula contendo um pilar (obstáculo fixo)
#define BATERIA 'b'   // Célula com uma bateria (recarrega energia do robô)
#define FIGURA 'f'    // Célula com uma figura (objetivo que deve ser coletado pelos robôs)

/* Direções de movimento */
#define NORTE 'N'     // Movimento para o Norte
#define LESTE 'L'     // Movimento para o Leste
#define SUL 'S'       // Movimento para o Sul
#define OESTE 'O'     // Movimento para o Oeste

/* Estrutura para representar uma célula da arena */
typedef struct
{
    char obj;  // Objeto presente na célula (VAZIO, PILAR, BATERIA, FIGURA)
    int id;    // ID do robô presente na célula ou -1 se estiver vazia
} CelulaArena;

/* Estrutura para representar a arena */
typedef struct
{
    CelulaArena **cel;  // Matriz que representa as células da arena
    int n_lins;  // Número de linhas da arena
    int n_cols;  // Número de colunas da arena
} Arena;

/* Estrutura para representar um robô */
typedef struct
{
    int id;  // ID único do robô
    int i;   // Linha atual do robô na arena
    int j;   // Coluna atual do robô na arena
    int energia;  // Energia restante do robô
    int figuras_coletadas;  // Quantidade de figuras coletadas pelo robô
    char *sequencia_movimentos;  // Sequência de movimentos programados para o robô
    int tamanho_sequencia;  // Número total de movimentos programados
    int id_movimento;  // Índice do movimento atual na sequência

    int move_i;  // Linha destino onde o robô pretende se mover
    int move_j;  // Coluna destino onde o robô pretende se mover
    int id_roubo_energia;  // ID do robô do qual o robô tentará roubar energia
} Robo;

/* Variáveis globais */
Arena arena;  // Estrutura representando a arena
Robo *robos;  // Array contendo todos os robôs da simulação
int num_robos;  // Número total de robôs
int num_total_turnos;  // Número total de turnos da simulação
int energia_bateria;  // Quantidade de energia fornecida por uma bateria

sem_t sem_iniciar_jogada_robo; 
sem_t sem_proxima_jogada_robo;
sem_t sem_proximo_turno;
sem_t sem_iniciar_jogada_robo_zerado;
sem_t sem_iniciar_movimento_robos;
sem_t sem_iniciar_jogada_robo_com_bateria;

pthread_mutex_t mutex_num_robos_jogaram;
int num_robos_jogaram = 0;

pthread_mutex_t mutex_num_robos_zerados;
int num_robos_zerados = 0;

pthread_mutex_t mutex_num_robos_movimentar;
int num_robos_movimentar = 0;

pthread_mutex_t mutex_num_robos_tentaram_roubar_energia_inicio_turno;
int num_robos_tentaram_roubar_energia_inicio_turno = 0;

/* Declaração das funções auxiliares */
void sincroniza_proximo_turno();
void sincroniza_execucao_movimento();
void resolve_conflito_celulas();
void le_entrada();
void imprime_estado();
void *rotina_robo(void *arg);
void processa_robo(Robo *robo);
void calcula_roubo_energia(Robo *robot);
void calcula_movimento(Robo *robo);
void realiza_movimento(Robo *robo);
void realiza_roubo_energia(Robo *robot);
int eh_posicao_valida(int i, int j);
void imprime_resultados();

/* Funções para alocação e destruição de memória */
void cria_arena(Arena *a, int linhas, int colunas);
void destroi_arena(Arena *arena);
void destroi_robos(Robo *robos, int num_robos);
void inicializacao_globais();
void destruicao_globais();


int main()
{
    /* Leitura da entrada e inicialização da arena e dos robôs */
    le_entrada();

    /* Inicialização Semáforos e Mutexes */
    inicializacao_globais();

    /* Criação threads para cada robô */
    pthread_t *threads = (pthread_t *)malloc(sizeof(pthread_t) * num_robos);
    for (int i = 0; i < num_robos; i++) pthread_create(&threads[i], NULL, rotina_robo, &robos[i]);

    /* Simulação dos turnos. O turno 0 é o estado inicial. */
    for (int turno = 0; turno < num_total_turnos; turno++)
    {  
        printf("Turno %d:\n", turno);
        imprime_estado();

        for (int i = 0; i < num_robos; i++) sem_post(&sem_iniciar_jogada_robo); // Libera robôs para jogarem, garante que robôs só joguem depois de exibir estado
        
        sem_wait(&sem_proximo_turno); // Espera último robô jogar para ir para próximo turno.
    }

    /* Finalização threads dos robôs */
    for (int i = 0; i < num_robos; i++) pthread_join(threads[i], NULL);

    /* Imprime os resultados da simulação */
    printf("Turno %d:\n", num_total_turnos);
    imprime_estado();
    imprime_resultados();

    /* Liberação de memória alocada */
    destroi_arena(&arena);
    destroi_robos(robos, num_robos);

    /* Liberação de Semáforos e Mutexes */
    destruicao_globais();

    return 0;
}

/* Função que inicializa semáforos e mutexes */
void inicializacao_globais() {
    sem_init(&sem_iniciar_jogada_robo, 0, 0); // Começa em zero, função main libera num_robos vezes.
    sem_init(&sem_proxima_jogada_robo, 0, 0); // Começa em zero, último thread de robô libera num_robos-1 vezes.
    sem_init(&sem_proximo_turno, 0, 0); // Começa em 0, última thread de robô libera 1 vez, função main consome e inicia novo turno.
    sem_init(&sem_iniciar_jogada_robo_zerado, 0, 0); // Começa em 0, último robô com energia, libera robôs sem energia para jogar.
    sem_init(&sem_iniciar_movimento_robos, 0, 0); // Começa em 0, último robô a planejar seu movimento (logo, possui energia), libera robôs que aguardam execução de movimento.
    sem_init(&sem_iniciar_jogada_robo_com_bateria, 0, 0); // Começa em 0
    pthread_mutex_init(&mutex_num_robos_jogaram, NULL); // Mutex para área crítica num_robos_jogaram (quantidade de robôs que já jogaram no turno).
    pthread_mutex_init(&mutex_num_robos_zerados, NULL); // Mutex para área crítica num_robos_zerados (quantidade de robôs sem bateria no turno).
    pthread_mutex_init(&mutex_num_robos_movimentar, NULL); // Mutex para área crítica num_robos_movimentar (quantidade de robôs que vão realizar movimento).
    pthread_mutex_init(&mutex_num_robos_tentaram_roubar_energia_inicio_turno, NULL); // Mutex para área crítica num_robos_tentaram_roubar_energia_inicio_turno.
}

/* Função que destrói semáforos e mutexes */
void destruicao_globais() {
    sem_destroy(&sem_iniciar_jogada_robo);
    sem_destroy(&sem_proxima_jogada_robo);
    sem_destroy(&sem_proximo_turno);
    sem_destroy(&sem_iniciar_jogada_robo_zerado);
    sem_destroy(&sem_iniciar_movimento_robos);
    sem_destroy(&sem_iniciar_jogada_robo_com_bateria);
    pthread_mutex_destroy(&mutex_num_robos_jogaram);
    pthread_mutex_destroy(&mutex_num_robos_zerados);
    pthread_mutex_destroy(&mutex_num_robos_movimentar);
    pthread_mutex_destroy(&mutex_num_robos_tentaram_roubar_energia_inicio_turno);
}

/* Função que calcula o número de robôs sem energia */
void calcula_robos_zerados() {
    // Não é necessário o uso de mutex, pois apenas o último robô de cada turno acessa essa função.
    num_robos_zerados = 0;
    for (int i = 0; i < num_robos; i++) if (robos[i].energia == 0) num_robos_zerados++;
}

/* Função que sincroniza robôs no fim do turno (todos devem ter concluído seu lance) */
void sincroniza_proximo_turno() {
    // Contador de robôs que jogram no turno
    pthread_mutex_lock(&mutex_num_robos_jogaram);
    num_robos_jogaram++;
    pthread_mutex_unlock(&mutex_num_robos_jogaram);

    // Último robô com energia a jogar, libera robôs zerados para jogarem.
    if (num_robos_jogaram == num_robos - num_robos_zerados) for (int i = 0 ; i < num_robos_zerados; i++) sem_post(&sem_iniciar_jogada_robo_zerado);

    // Faltam robôs para jogarem
    if (num_robos_jogaram != num_robos) {
        sem_wait(&sem_proxima_jogada_robo);
        return;
    }

    // Último robô a jogar no turno
    calcula_robos_zerados();
    num_robos_jogaram = 0; // Reinicia número de robôs que jogaram no turno
    for (int i = 0; i < num_robos - 1; i++) sem_post(&sem_proxima_jogada_robo); // Libera robôs em espera para irem p/ próxima iteração
    sem_post(&sem_proximo_turno); // Libera função main ir para próximo turno
}

/* Função que sincroniza execução do movimento de cada robô, garante que conflitos de disputa de célula sejam resolvidos */
void sincroniza_execucao_movimento() {
    // Contador de robôs que vão executar movimento.
    pthread_mutex_lock(&mutex_num_robos_movimentar);
    num_robos_movimentar++;
    pthread_mutex_unlock(&mutex_num_robos_movimentar);

    // Faltam robôs planejarem movimento. Robôs sem energia, não podem realizar movimento, logo estão fora do cálculo.
    if (num_robos_movimentar != num_robos - num_robos_zerados) {
        sem_wait(&sem_iniciar_movimento_robos);
        return;
    }

    // Último robô a planejar seu movimento
    resolve_conflito_celulas();

    num_robos_movimentar = 0; // Reinicia número de robôs a movimentar.
    for (int i = 0; i < num_robos - num_robos_zerados - 1; i++) sem_post(&sem_iniciar_movimento_robos); // Libera robôs executarem seus movimentos.
}


void resolve_conflito_celulas() {
    for (int i = 0; i < num_robos; i++) {
        for (int j = i + 1; j < num_robos; j++) {
            if (robos[i].move_i == robos[j].move_i && robos[i].move_j == robos[j].move_j) {
                if (robos[i].id < robos[j].id) {
                    robos[j].move_i = robos[j].i;
                    robos[j].move_j = robos[j].j;
                } else {
                    robos[i].move_i = robos[i].i;
                    robos[i].move_j = robos[i].j;
                }
            }
        }
    }
}

/* Função para ler a entrada e configurar a arena e os robôs */
void le_entrada()
{
    int N, M, R, T;

    /* Lê as dimensões da arena, número de robôs, energia por bateria e o número de turnos */
    scanf("%d %d %d %d %d", &N, &M, &R, &energia_bateria, &T);

    /* Cria a arena */
    cria_arena(&arena, N, M);

    /* Cria os robôs */
    robos = (Robo *) malloc(sizeof(Robo) * R);

    arena.n_lins = N;
    arena.n_cols = M;
    num_robos = R;
    num_total_turnos = T;

    /* Lê a configuração da arena */
    char line[102];
    for (int i = 0; i < N; i++)
    {
        scanf("%s", line);
        for (int j = 0; j < M; j++)
        {
            arena.cel[i][j].obj = line[j];
            arena.cel[i][j].id = -1;  // Inicialmente, nenhuma célula contém robôs
        }
    }

    /* Lê as posições iniciais dos robôs */
    for (int i = 0; i < R; i++)
    {
        scanf("%d %d", &robos[i].i, &robos[i].j);  // Posição inicial do robô
        robos[i].id = i;
        robos[i].energia = energia_bateria;
        robos[i].figuras_coletadas = 0;
        robos[i].id_movimento = 0;
        arena.cel[robos[i].i][robos[i].j].id = robos[i].id;  // Atualiza a posição na arena
    }

    char format[20];

    /* Lê a sequência de movimentos de cada robô */
    for (int i = 0; i < R; i++)
    {
        int n_mov = 0;
        scanf("%d", &n_mov);
        robos[i].tamanho_sequencia = n_mov;

        /* Aloca memória para armazenar a sequência de movimentos */
        robos[i].sequencia_movimentos = (char *) malloc(sizeof(char) * (n_mov + 1));

        /* Lê a sequência de movimentos */
        sprintf(format, "%%%ds", n_mov);
        scanf(format, robos[i].sequencia_movimentos);
    }
}

/* Função para imprimir o estado atual da arena */
void imprime_estado()
{
    for (int i = 0; i < arena.n_lins; i++)
    {
        for (int j = 0; j < arena.n_cols; j++)
        {
            if (arena.cel[i][j].id == -1)
            {
                printf(" %c  ", arena.cel[i][j].obj);  // Imprime o objeto na célula
            } else
            {
                printf("(%d) ", arena.cel[i][j].id);  // Imprime o ID do robô presente
            }
        }
        printf("\n");
    }
    fflush(stdout);
}

/* Função que controla a rotina da thread de cada robô */
void *rotina_robo(void *arg) {
    Robo *robo = (Robo *)arg;
    for (int turno = 0; turno < num_total_turnos; turno++) {
        sem_wait(&sem_iniciar_jogada_robo); // Robôs aguardam função main exibir estado e liberar robôs jogarem
        
        if (num_robos_zerados && robo->energia > 0) {
            sem_wait(&sem_iniciar_jogada_robo_com_bateria);
        } else if (robo->energia == 0) {
            processa_robo(robo);

            pthread_mutex_lock(&mutex_num_robos_tentaram_roubar_energia_inicio_turno);
            num_robos_tentaram_roubar_energia_inicio_turno++;
            pthread_mutex_unlock(&mutex_num_robos_tentaram_roubar_energia_inicio_turno);
            
            // Último robô sem bateria a tentar roubar no início do turno
            if (num_robos_tentaram_roubar_energia_inicio_turno == num_robos_zerados) {
                num_robos_tentaram_roubar_energia_inicio_turno = 0;
                for (int i = 0; i < num_robos - num_robos_zerados; i++) sem_post(&sem_iniciar_jogada_robo_com_bateria);
            }

            if (robo->energia > 0) {
                sincroniza_proximo_turno();
                continue;
            } else { // Robôs que continuam sem bateria, tentam roubar energia de novo
                sem_wait(&sem_iniciar_jogada_robo_zerado); // Robôs sem bateria aguardam que todos os robôs joguem 
            }
        }

        processa_robo(robo);

        sincroniza_proximo_turno(); // Garante que todos os robôs tenham jogado antes de iniciar novo turno
    }
    pthread_exit(NULL);
}

/* Função que controla o processamento de cada robô */
void processa_robo(Robo *robo)
{
    if (robo->energia == 0) {
        calcula_roubo_energia(robo); // Etapa de planejamento: Robô sem energia tenta roubar energia de um vizinho.
        realiza_roubo_energia(robo); // Etapa de execução: Robô sem energia tenta roubar energia.
        return;
    }   
    
    calcula_movimento(robo); // Etapa de planejamento: Robô com energia planeja o próximo movimento.

    sincroniza_execucao_movimento(); // Garante que todos os robôs com energia tenham planejado seu movimento.

    realiza_movimento(robo); // Etapa de execução: Robô com energia realiza o movimento planejado.
}

/* Função que define a intenção de roubo de energia */
void calcula_roubo_energia(Robo *robot)
{
    // Arrays para representar as direções Norte, Sul, Leste e Oeste
    int di[] = {-1, 1, 0, 0}; // Movimentos verticais
    int dj[] = { 0, 0, 1,-1}; // Movimentos horizontais

    int id_robo_roubo = -1;  // Inicializa a variável que armazenará o ID do robô a ser roubado

    // Verifica robôs vizinhos para decidir de quem roubar energia
    for (int i = 0; i < 4; i++)
    {
        int ni = robot->i + di[i];  // Calcula a nova linha do vizinho
        int nj = robot->j + dj[i];  // Calcula a nova coluna do vizinho

        // Verifica se a posição do vizinho é válida na arena
        if (eh_posicao_valida(ni, nj))
        {
            int robo_vizinho = arena.cel[ni][nj].id;

            // Se houver um robô vizinho com mais de 1 unidade de energia, ele é um alvo
            if (robo_vizinho >= 0 && robos[robo_vizinho].energia > 1)
            {
                // Prioriza o robô de menor ID para ser roubado
                if (id_robo_roubo < 0 || robo_vizinho < id_robo_roubo)
                {
                    id_robo_roubo = robo_vizinho;
                }
            }
        }
    }

    // Define a intenção de roubo de energia do robô vizinho
    robot->id_roubo_energia = id_robo_roubo;
    return;
}

/* Função para planejar o próximo movimento do robô */
void calcula_movimento(Robo *robo)
{
    // Inicialmente, a intenção de movimento é permanecer na mesma posição
    robo->move_i = robo->i;
    robo->move_j = robo->j;

    // Verifica se o robô ainda tem movimentos programados
    if (robo->id_movimento >= robo->tamanho_sequencia)
        return;

    // Obtém a direção do próximo movimento a partir da sequência programada
    char direcao = robo->sequencia_movimentos[robo->id_movimento];
    robo->id_movimento++;  // Atualiza o índice para o próximo movimento

    // Atualiza a posição pretendida com base na direção
    switch (direcao)
    {
        case NORTE:
            robo->move_i -= 1;  // Movimenta para o norte
            break;
        case SUL:
            robo->move_i += 1;  // Movimenta para o sul
            break;
        case LESTE:
            robo->move_j += 1;  // Movimenta para o leste
            break;
        case OESTE:
            robo->move_j -= 1;  // Movimenta para o oeste
            break;
        default:
            break;  // Direção inválida, o robô permanece na mesma posição
    }

    // Se posição futura é inválida, não modifica
    if (!eh_posicao_valida(robo->move_i, robo->move_j)) {
        robo->move_i = robo->i;
        robo->move_j = robo->j;
    }
}

/* Função para realizar o movimento do robô */
void realiza_movimento(Robo *robo)
{
    // Verifica se o robô ainda tem energia para se mover
    if (robo->energia == 0)
        return;

    // Posição de destino é a posição atual
    if (robo->move_i == robo->i && robo->move_j == robo->j) 
        return;

    // Verifica se a posição para onde o robô deseja se mover é válida
    if (!eh_posicao_valida(robo->move_i, robo->move_j))
        return;

    // Obtenção das células atuais e de destino
    CelulaArena *nova_cel = &arena.cel[robo->move_i][robo->move_j];
    CelulaArena *cel = &arena.cel[robo->i][robo->j];

    // Se há outro robô na posição futura do robô dessa thread que não irá se mover, o robô dessa thread não anda
    if (nova_cel->id >= 0 && robos[nova_cel->id].move_i == robos[nova_cel->id].i && robos[nova_cel->id].move_j == robos[nova_cel->id].j)   
        return;

    // Verifica se a célula de destino está vazia e não é um obstáculo (pilar)
    if (nova_cel->obj != PILAR)
    {
        // Coleta o objeto presente na célula de destino (se houver)
        switch (nova_cel->obj)
        {
            case BATERIA:
                robo->energia += energia_bateria;  // Recarrega energia com a bateria
                break;
            case FIGURA:
                robo->figuras_coletadas++;  // Coleta a figura
                break;
        }
    //
        // Limpa o objeto da célula de destino após coleta
        nova_cel->obj = VAZIO;

        // Atualiza as células da arena com a nova posição do robô
        if (cel->id == robo->id)
            cel->id = -1;  // Remove o robô da célula atual

        // Define o ID do robô na nova célula
        nova_cel->id = robo->id;

        // Atualiza a posição do robô na arena
        robo->i = robo->move_i;
        robo->j = robo->move_j;

        // Reduz a energia do robô após o movimento
        robo->energia--;
    }
}

/* Função que realiza o roubo de energia de um robô vizinho */
void realiza_roubo_energia(Robo *robot)
{
    // Verifica se o robô já possui energia suficiente
    if (robot->energia > 0)
        return;  // Robô não precisa roubar energia

    int id_roubo = robot->id_roubo_energia;

    // Verifica se há um robô válido para roubar e se o alvo ainda tem energia
    if (id_roubo == -1 || robos[id_roubo].energia == 0)
        return;  // Nenhum robô disponível para roubo ou sem energia

    // Rouba uma unidade de energia do robô alvo
    robos[id_roubo].energia--;
    robot->energia++;

    // Reseta a intenção de roubo após o sucesso
    robot->id_roubo_energia = -1;
}

/* Função que verifica se a posição está dentro dos limites da arena */
int eh_posicao_valida(int i, int j)
{
    return (i >= 0 && i < arena.n_lins && j >= 0 && j < arena.n_cols);
}

/* Função para imprimir os resultados finais da simulação */
void imprime_resultados()
{
    for (int i = 0; i < num_robos; i++)
    {
        printf("Robô %d:\n", robos[i].id);
        printf("  Figuras coletadas: %d\n", robos[i].figuras_coletadas);
        printf("  Energia restante: %d\n", robos[i].energia);
        printf("  Posição final: (%d, %d)\n", robos[i].i, robos[i].j);
    }
}

/* Função para criar a arena com o número de linhas e colunas fornecido */
void cria_arena(Arena *arena, int linhas, int colunas)
{
    arena->n_lins = linhas;
    arena->n_cols = colunas;

    // Aloca a matriz de ponteiros para as linhas
    arena->cel = (CelulaArena **)malloc(linhas * sizeof(CelulaArena *));

    // Aloca memória para cada linha de células e inicializa as células
    for (int i = 0; i < linhas; i++)
    {
        arena->cel[i] = (CelulaArena *)malloc(colunas * sizeof(CelulaArena));

        // Inicializa as células como vazias
        for (int j = 0; j < colunas; j++)
        {
            arena->cel[i][j].obj = VAZIO;  // Célula vazia
            arena->cel[i][j].id = -1;      // Sem robô inicialmente
        }
    }

    return;
}

/* Função para desalocar a memória utilizada pela arena */
void destroi_arena(Arena *arena)
{
    // Libera a memória de cada linha da matriz de células
    for (int i = 0; i < arena->n_lins; i++)
    {
        free(arena->cel[i]);
    }

    // Libera a matriz de ponteiros
    free(arena->cel);
}

/* Função para desalocar a memória utilizada pelos robôs */
void destroi_robos(Robo *robos, int num_robos)
{
    // Libera a memória alocada para a sequência de movimentos de cada robô
    for (int r = 0; r < num_robos; r++)
        free(robos[r].sequencia_movimentos);

    // Libera a memória do array de robôs
    free(robos);
}
