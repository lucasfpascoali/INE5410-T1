# O Rally dos Robôs em Marte

---

<p align="center">
  <img src="figuras/fig01.webp" alt="Figura 1" width="45%">
  <img src="figuras/fig02.webp" alt="Figura 2" width="45%">
</p>

## Descrição do Problema

Em um futuro próximo, a humanidade estabeleceu colônias em Marte. Para entreter os habitantes e testar novas tecnologias, foi criado um esporte emocionante: o **Rally dos Robôs Marcianos**. Neste evento, robôs competem em uma arena gigante na superfície marciana, repleta de obstáculos, desafios e recursos valiosos.

### A Arena Marciana

A arena é uma vasta grade bidimensional de dimensões **N×M**, representando a superfície irregular de Marte. Em cada posição da grade, pode haver:

- **Pilastras de Pedra (`'x'`)**: Formações rochosas intransponíveis que bloqueiam o caminho dos robôs;
- **Terreno Livre (`'.'`)**: Áreas planas e transitáveis onde os robôs podem se movimentar livremente;
- **Depósito de Energia (`'b'`)**: Baterias marcianas que fornecem energia adicional aos robôs que as coletam;
- **Artefatos Antigos (`'f'`)**: Valiosas figuras deixadas por antigas civilizações marcianas, cobiçadas pelos cientistas.

### A Missão dos Robôs

Cada robô participante tem a seguinte missão:

- **Coletar Artefatos Antigos**: Encontrar e coletar o máximo de figuras (`'f'`) possível. Ao entrar em uma célula com uma figura, o robô a coleta, e a célula torna-se terreno livre (`'.'`);
- **Coletar Baterias Marcianas**: Ao entrar em uma célula com uma bateria (`'b'`), o robô a coleta, ganha **P** movimentos adicionais, e a célula torna-se terreno livre (`'.'`).

### Movimentação dos Robôs

Cada robô é programado com uma sequência individual de movimentos, definida pela sua equipe de engenheiros antes do início do rally. Os movimentos possíveis são:

- **Norte (`'N'`)**: Avançar uma célula para o norte (linha anterior);
- **Leste (`'L'`)**: Avançar uma célula para o leste (coluna seguinte);
- **Sul (`'S'`)**: Avançar uma célula para o sul (linha seguinte);
- **Oeste (`'O'`)**: Avançar uma célula para o oeste (coluna anterior).

A simulação da movimentação dos robôs é feita em **turnos**. Em cada turno, cada robô pode fazer uma movimentação, desde que tenha energia suficiente e o movimento seja válido. Se dois ou mais robôs tentarem se mover para a mesma posição, apenas o robô com menor identificador (ID) realizará o movimento. Os demais precisarão esperar o próximo turno.

A simulação termina quando o número total de turnos (`T`) for atingido. O turno 0 é o estado incial da simulação.

### Gestão de Energia

Cada robô começa com uma bateria totalmente carregada, permitindo **P** movimentos. Ao coletar uma bateria marciana (`'b'`), o robô ganha **P** movimentos adicionais. Se um robô ficar sem energia, ele entra em modo de hibernação e não se move até receber energia de um robô vizinho, que transfere 1 movimento de energia ao final do turno.

---

## Formato de Entrada

O programa deve ler a entrada a partir de um arquivo de texto ou do console, no seguinte formato:

1. **Primeira Linha:**
   - Cinco inteiros separados por espaços: `N M R P T`
     - `N`: Número de linhas da arena;
     - `M`: Número de colunas da arena;
     - `R`: Número de robôs;
     - `P`: Quantidade de movimentos fornecidos por uma bateria totalmente carregada;
     - `T`: Número de turnos.

2. **Descrição da Arena:**
   - **N** linhas, cada uma contendo **M** caracteres sem espaços, representando as células da arena:
     - `'x'`: Pilastra;
     - `'b'`: Bateria;
     - `'.'`: Terreno Livre;
     - `'f'`: Artefato Antigo.

3. **Posições Iniciais dos Robôs:**
   - **R** linhas, cada uma contendo dois inteiros `i j` separados por espaço:
     - `i`: Linha inicial do robô (0 ≤ `i` < `N`);
     - `j`: Coluna inicial do robô (0 ≤ `j` < `M`).

4. **Sequências de Movimentos dos Robôs:**
   - **R** linhas, cada uma com um inteiro `S` e `S` caracteres representando os movimentos (`'N'`, `'L'`, `'S'`, `'O'`).

---

## Exemplo de Entrada

```plaintext
5 5 2 10 5
.....
..b..
.x.x.
..f..
.....
0 0
4 4
5 SLLLL
5 OONOO
```

### Explicação do Exemplo:

- **Arena:** 5 linhas x 5 colunas.
- **Robôs:** 2 robôs com IDs 0 e 1.
- **Movimentos por Bateria:** 10 movimentos (P = 10).
- **Número de turnos simulados:** 4 turnos (T = 4).

#### Arena:

- Uma bateria localizada na posição (1,2).
- Uma figura na posição (3,2).

#### Posições Iniciais:

- Robô 0: Linha 0, Coluna 0.
- Robô 1: Linha 4, Coluna 4.

#### Sequências de Movimentos:

- Robô 0: 5 movimentos (`S`, `L`, `L`, `L`, `L`).
- Robô 1: 5 movimentos (`O`, `O`, `N`, `O`, `O`).

### Simulação da Entrada:

```plaintext
Turno 0:
(0)  .   .   .   .  
 .   .   b   .   .  
 .   x   .   x   .  
 .   .   f   .   .  
 .   .   .   .  (1) 
Turno 1:
 .   .   .   .   .  
(0)  .   b   .   .  
 .   x   .   x   .  
 .   .   f   .   .  
 .   .   .  (1)  .  
Turno 2:
 .   .   .   .   .  
 .  (0)  b   .   .  
 .   x   .   x   .  
 .   .   f   .   .  
 .   .  (1)  .   .  
Turno 3:
 .   .   .   .   .  
 .   .  (0)  .   .  
 .   x   .   x   .  
 .   .  (1)  .   .  
 .   .   .   .   .  
Turno 4:
 .   .   .   .   .  
 .   .   .  (0)  .  
 .   x   .   x   .  
 .  (1)  .   .   .  
 .   .   .   .   .  
Turno 5:
 .   .   .   .   .  
 .   .   .   .  (0) 
 .   x   .   x   .  
(1)  .   .   .   .  
 .   .   .   .   .  
```

### Estado Final dos Robôs:

- **Robô 0:**
  - Figuras coletadas: 0
  - Energia restante: 15
  - Posição final: (1, 4)
  
- **Robô 1:**
  - Figuras coletadas: 1
  - Energia restante: 5
  - Posição final: (3, 0)

---

## Implementação da Solução do Trabalho

O carregamento da matriz, os movimentos dos robôs e a leitura dos parâmetros de entrada já estão implementados. Todavia, a simulação atualmente não é feita de forma concorrente. A sua tarefa será implementar uma versão **concorrente** do simulador, conforme orientações a seguir.

### Threads:

- Cada robô deve ser controlado por uma **thread** separada.
- Utilize mecanismos de sincronização (como mutexes e/ou semáforos) para controlar o acesso à arena e resolver conflitos entre robôs.
- Garanta que as seguintes regras sejam respeitadas:
  1. Robôs com IDs menores têm prioridade de movimentação sobre robôs com IDs maiores.
  2. Quando um robô sem energia tem robôs vizinhos, ele rouba energia do robô com o menor ID.
  3. O roubo de energia ocorre apenas quando o robô vizinho tem pelo menos 2 unidades de energia, garantindo que o robô roubado não fique sem energia.
  4. Um robô perde energia ao se movimentar, e o movimento depende da energia disponível.

### Condições de Término da Simulação:

A simulação acaba após o número total de turnos (`T`) ser atingido.

### Saída do Programa:

Ao final da simulação, o programa deve exibir para cada robô:

- ID do Robô;
- Figuras coletadas: Número de figuras coletadas;
- Energia Restante: Unidades de energia restante;
- Posição Final: (linha, coluna).

#### Exemplo de Saída:

```plaintext
Robô 0:
  Figuras coletadas: 0
  Energia restante: 15
  Posição final: (1, 4)

Robô 1:
  Figuras coletadas: 1
  Energia restante: 5
  Posição final: (3, 0)
```

---

## Considerações Sobre o Código Disponibilizado

O código fornecido processa os robôs de forma sequencial através da função `processa_robo()`. A execução possui um loop externo para os turnos e um loop interno que executa a ação de cada robô sequencialmente, iniciando sempre pelo robô de menor ID (Robô 0). No entanto, essa solução não resolve todos os problemas. Como o turno de cada robô é realizado de forma sequencial, o robô 0 não tem conhecimento do resultado do turno dos robôs de IDs maiores, o que causa problemas no roubo de energia dos robôs vizinhos.

### Exemplo do Problema:

Compare as seguintes entradas:

#### Entrada 1:

```plaintext
5 5 3 2 4
.....
b...b
b...b
.....
.....
0 2
0 0
0 4
3 SSS
3 SSL
3 SSO
```

#### Entrada 2:

```plaintext
5 5 3 2 4
.....
b...b
b...b
.....
.....
0 0
0 4
0 2
3 SSL
3 SSO
3 SSS
```

- Na **Entrada 1**, o robô que rouba energia é o 0, o que resulta em um problema. 
- Na **Entrada 2**, o robô que rouba energia é o 2, que vê o resultado dos robôs 0 e 1 no início do seu turno, funcionando como esperado.

Você deve solucionar este problema modificando o código de forma que todos robôs sejam executados de forma concorrente.

---

## Teste de Execução Concorrente

Para testar a execução concorrente, você pode inserir `sleep()` nas funções de movimentação dos robôs `realiza_movimento()` e de roubo de energia `realiza_roubo_energia()`. Isso simulará o ganho de performance ao paralelizar a movimentação dos robôs.

## Instruções de Compilação do Código

Para compilar o código, basta rodar o comando `make`, que compilará o arquivo `rally_marciano.c`. 

Para executar o código, utilize o seguinte comando:

```bash
./rally_marciano < input.txt
```

---

Boa sorte no desafio, e que vença o melhor robô!
