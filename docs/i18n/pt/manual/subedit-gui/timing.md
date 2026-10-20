# Sincronização a partir do vídeo

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revista por nenhum leitor do idioma.

*Pressupõe-se que o filme está aberto e que o reprodutor é conhecido: ver [O reprodutor](player.md).*

Vê-se o filme, pára-se no fotograma em que uma linha deve começar, e **essa posição passa a ser
o início da legenda**. Cinco gestos, no menu **Vídeo**, com os nomes que o Gaupol lhes dá. Apoiam-se
no que já existe: definir uma posição é **o mesmo comando que escrever o valor na
célula**, e inserir é **o mesmo que `Editar ▸ Inserir legendas…`**.

| Comando | Atalho | O que faz |
| :------ | :----- | :-------- |
| Vídeo ▸ Definir o início a partir da posição do vídeo | `Alt+U` | define o **início** da legenda selecionada na posição do filme |
| Vídeo ▸ Definir o fim a partir da posição do vídeo | `Alt+K` | define o seu **fim** na posição do filme |
| Vídeo ▸ Inserir legenda na posição do vídeo | `Alt+J` | insere uma legenda que **começa** na posição e seleciona-a |
| Vídeo ▸ Selecionar a legenda anterior à posição do vídeo | `Alt+Up` | seleciona a última legenda que **começa antes** da posição |
| Vídeo ▸ Selecionar a legenda seguinte à posição do vídeo | `Alt+Down` | seleciona a primeira que **começa depois** da posição |

**Todos ficam desativados sem vídeo aberto**, como os gestos do reprodutor. `Set Start…` e `Set End…`
também ficam desativados enquanto nada estiver selecionado: atuam sobre a **primeira linha** da
seleção.

**Os atalhos são os do Gaupol, com `Alt`**: lá são `U`, `K` e `J` — letras simples, que
a janela tomaria antes da célula em edição, de modo que escrever um `k` num texto
definiria um fim. `Select Previous…` e `Select Next…` são `Ctrl+Y` e `Ctrl+U` no Gaupol; `Ctrl+Y`
é um *refazer* em alguns sistemas, e as setas com `Alt` dizem *antes* e *depois* melhor do que
duas letras. Uma célula em edição é **confirmada** antes de `Set Start…`, `Set End…` ou
`Insert Subtitle…`, como antes de qualquer modificação.

## Definir um limite

- **Uma entrada do histórico por gesto**: `Editar ▸ Anular` devolve a posição anterior, e nada mais.
- **As mesmas regras que a célula.** Um início definido depois do fim da legenda, ou um fim antes do seu
  início, **não é recusado**: é definido, e a tabela marca a legenda como marca aquela
  cujo valor foi escrito à mão — ver [As anomalias](table.md#as-anomalias). Nada é corrigido em silêncio.
- **Uma posição que já é a do limite não faz nada**, e não acrescenta nada ao histórico.
- **A posição é sempre o início de um fotograma**: o reprodutor nunca comunica uma posição
  dentro de um fotograma. Definir um limite e depois voltar a ele — com `Seek Selection Start`, por exemplo —
  **mostra o mesmo fotograma**.

## Inserir na posição

A nova legenda **começa na posição**, e dura **três segundos**, ou até ao início da seguinte
quando esta vem mais cedo. O seu texto está vazio. Ocupa o seu lugar na ordem do ficheiro: depois de
todas as que começam nessa posição ou antes dela. A inserção é **uma entrada do histórico**,
e a legenda inserida fica selecionada: pode escrever-se logo.

## Selecionar a partir da posição

Ambos seguem a ordem do ficheiro, não a seleção atual. **Sem vizinha desse lado**, a
seleção vai para a última legenda do ficheiro no caso de `Select Next…`, e para a primeira no de
`Select Previous…` — o comportamento do Gaupol. Estes dois gestos não movem o filme.

## Fotograma a fotograma

Avançar e recuar **de um fotograma** — ou de N, ver abaixo —, para definir um limite **no fotograma exato**.
Dois gestos, e dois botões na barra de reprodução, de um lado e do outro do botão de reprodução:

| Comando | Atalho | O que faz |
| :------ | :----- | :-------- |
| Vídeo ▸ Passo para trás | `Alt+Left` | recua o filme do passo de fotogramas |
| Vídeo ▸ Passo em frente | `Alt+Right` | avança-o do passo de fotogramas |

- **O passo é um número de fotogramas, no mínimo um**: é a definição `Frame step` das
  [preferências](preferences.md#o-reprodutor). Um passo de N fotogramas mostra o fotograma **N mais adiante**, a 25 fotogramas por
  segundo como a 23,976; **nunca se exprime em milissegundos**.
- **O filme pára nos seus limites**: recuar a partir do primeiro fotograma deixa-o lá, avançar a partir do último
  também. A reprodução é parada após um passo.
- **Os botões da barra** — dois ícones, de um lado e do outro de reproduzir/pausa — **acionam as mesmas ações que o menu**: o mesmo passo, desativados
  sem vídeo como as entradas do menu, e a mesma dica de ferramenta, que dá o atalho. **Um dedo que fica num
  botão, ou na tecla, encadeia os passos.**
- **Um passo de cada vez.** Uma tecla mantida premida repete mais depressa do que se faz um passo: a primeira repetição sai logo,
  as que chegam enquanto o passo é executado **não se acumulam** — partilham um só lugar, o
  último, que sai depois. Largar a tecla pára, portanto, a imagem onde se está, e não deixa que o filme
  continue com o balanço adquirido.

## Ajustar um limite de um fotograma

Para ajustar um limite com o teclado: **o início ou o fim da legenda selecionada**, de um passo, mais cedo ou mais tarde.

| Comando | Atalho | O que faz |
| :------ | :----- | :-------- |
| Vídeo ▸ Antecipar ligeiramente o início | `Alt+Q` | a legenda **começa um passo mais cedo** |
| Vídeo ▸ Atrasar ligeiramente o início | `Alt+W` | **começa um passo mais tarde** |
| Vídeo ▸ Antecipar ligeiramente o fim | `Alt+Shift+Q` | **termina um passo mais cedo** |
| Vídeo ▸ Atrasar ligeiramente o fim | `Alt+Shift+W` | **termina um passo mais tarde** |

`Q` e `W` são os do Gaupol para *mais cedo* e *mais tarde*, com `Alt` pela razão indicada acima; o fim
leva `Alt+Shift`, porque o `E` do Gaupol é aqui a letra de `Edit`.

- **Uma entrada do histórico por ajuste**, anulável. A legenda vizinha e a sobreposição são comunicadas
  **como ao escrever**: o limite é definido, e a tabela marca o que deixou de se sustentar — ver
  [As anomalias](table.md#as-anomalias).
- **O mesmo passo que o reprodutor**, portanto um fotograma por defeito. **O que vale um fotograma** lê-se, por esta ordem,
  da **taxa de fotogramas que o vídeo declara**, da do **documento quando conta em fotogramas** (MicroDVD), e depois
  da **grelha que as posições desenham**. **Sem nenhuma das três, o gesto recusa e di-lo**: escolher uma
  taxa de fotogramas ao acaso deslocaria qualquer limite de algo que não é um fotograma.
- **Não exige um vídeo**, apenas uma seleção: usa-se também num ficheiro contado em fotogramas, sem
  filme. Quando um filme está aberto, **a reprodução desloca-se para a nova posição**, para mostrar o fotograma que o limite
  acabou de tomar.
- Um limite não passa antes da origem: recuar de um passo a partir de menos de um fotograma põe-no em zero.
