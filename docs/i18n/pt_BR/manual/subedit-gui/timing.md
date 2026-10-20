# Sincronizar a partir do vídeo

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revisada por um leitor do idioma.

*Supõe-se que o filme esteja aberto e que o reprodutor seja conhecido: veja [O reprodutor](player.md).*

Assiste-se ao filme, para-se no quadro em que uma linha deve começar, e **essa posição se torna
o início da legenda**. Cinco gestos, no menu **Vídeo**, com os nomes que o Gaupol lhes dá. Eles
se apoiam no que já existe: definir uma posição é **o mesmo comando que digitar o valor na
célula**, e inserir é **o mesmo que `Editar ▸ Inserir legendas…`**.

| Comando | Atalho | O que faz |
| :------ | :----- | :-------- |
| Vídeo ▸ Definir início pela posição do vídeo | `Alt+U` | define o **início** da legenda selecionada como a posição do filme |
| Vídeo ▸ Definir fim pela posição do vídeo | `Alt+K` | define o seu **fim** como a posição do filme |
| Vídeo ▸ Inserir legenda na posição do vídeo | `Alt+J` | insere uma legenda que **começa** na posição, e a seleciona |
| Vídeo ▸ Selecionar a legenda anterior à posição do vídeo | `Alt+Up` | seleciona a última legenda que **começa antes** da posição |
| Vídeo ▸ Selecionar a legenda seguinte à posição do vídeo | `Alt+Down` | seleciona a primeira que **começa depois** da posição |

**Todos ficam desativados sem vídeo aberto**, como os gestos do reprodutor. `Set Start…` e `Set End…`
também ficam desativados enquanto nada estiver selecionado: eles atuam sobre a **primeira linha** da
seleção.

**Os atalhos são os do Gaupol, com `Alt`**: lá eles são `U`, `K` e `J` — letras soltas, que
a janela tomaria antes da célula em edição, de modo que digitar um `k` num texto
definiria um fim. `Select Previous…` e `Select Next…` são `Ctrl+Y` e `Ctrl+U` no Gaupol; `Ctrl+Y`
é um *refazer* em alguns sistemas, e as setas com `Alt` dizem *antes* e *depois* melhor que
duas letras. Uma célula em edição é **confirmada** antes de `Set Start…`, `Set End…` ou
`Insert Subtitle…`, como antes de qualquer modificação.

## Definir uma extremidade

- **Uma entrada de histórico por gesto**: `Editar ▸ Desfazer` devolve a posição anterior, e nada mais.
- **As mesmas regras da célula.** Um início definido depois do fim da legenda, ou um fim antes do seu
  início, **não é recusado**: é definido, e a tabela marca a legenda como marca aquela
  cujo valor foi digitado à mão — veja [As anomalias](table.md#as-anomalias). Nada é corrigido em silêncio.
- **Uma posição que já é a da extremidade não faz nada**, e não acrescenta nada ao histórico.
- **A posição é sempre o início de um quadro**: o reprodutor nunca informa uma posição
  dentro de um quadro. Definir uma extremidade e depois voltar a ela — com `Seek Selection Start`, por exemplo —
  **mostra o mesmo quadro**.

## Inserir na posição

A nova legenda **começa na posição**, e dura **três segundos**, ou até o início da seguinte
quando esta vem antes. Seu texto é vazio. Ela ocupa seu lugar na ordem do arquivo: depois de
todas as que começam nessa posição ou antes dela. A inserção é **uma entrada de histórico**,
e a legenda inserida é selecionada: pode-se digitá-la imediatamente.

## Selecionar a partir da posição

Ambos seguem a ordem do arquivo, e não a seleção atual. **Sem vizinha desse lado**, a
seleção vai para a última legenda do arquivo no caso de `Select Next…`, e para a primeira no caso de
`Select Previous…` — o comportamento do Gaupol. Esses dois gestos não movem o filme.

## Quadro a quadro

Avançar e recuar **um quadro** — ou N, veja abaixo —, para definir uma extremidade **no quadro exato**.
Dois gestos, e dois botões na barra de reprodução, de cada lado do botão de reproduzir:

| Comando | Atalho | O que faz |
| :------ | :----- | :-------- |
| Vídeo ▸ Recuar um quadro | `Alt+Left` | recua o filme pelo passo em quadros |
| Vídeo ▸ Avançar um quadro | `Alt+Right` | avança-o pelo passo em quadros |

- **O passo é um número de quadros, pelo menos um**: é a configuração `Frame step` das
  [preferências](preferences.md#o-reprodutor). Um passo de N quadros mostra o quadro **N adiante**, a 25 quadros por
  segundo como a 23,976; ele **nunca é expresso em milissegundos**.
- **O filme para em seus limites**: recuar a partir do primeiro quadro o deixa ali, avançar a partir do último
  também. A reprodução é interrompida após um passo.
- **Os botões da barra** — dois ícones, de cada lado de reproduzir/pausar — **acionam as mesmas ações que o menu**: mesmo passo, desativados
  sem vídeo como as entradas do menu, e a mesma dica de ferramenta, que indica o atalho. **Um dedo que permanece num
  botão, ou na tecla, encadeia os passos.**
- **Um passo de cada vez.** Uma tecla mantida pressionada repete mais depressa do que um passo é feito: a primeira repetição sai
  imediatamente, as que chegam enquanto o passo está sendo feito **não se acumulam** — elas compartilham um lugar, o
  último, que sai depois. Soltar a tecla interrompe portanto a imagem onde se está, e não deixa o filme
  seguir sob o próprio impulso.

## Ajustar uma extremidade por um quadro

Para ajustar uma extremidade com o teclado: **o início ou o fim da legenda selecionada**, por um passo, para antes ou para depois.

| Comando | Atalho | O que faz |
| :------ | :----- | :-------- |
| Vídeo ▸ Antecipar o início | `Alt+Q` | a legenda **começa um passo mais cedo** |
| Vídeo ▸ Atrasar o início | `Alt+W` | ela **começa um passo mais tarde** |
| Vídeo ▸ Antecipar o fim | `Alt+Shift+Q` | ela **termina um passo mais cedo** |
| Vídeo ▸ Atrasar o fim | `Alt+Shift+W` | ela **termina um passo mais tarde** |

`Q` e `W` são os do Gaupol para *mais cedo* e *mais tarde*, com `Alt` pela razão dada acima; o fim
usa `Alt+Shift`, porque o `E` do Gaupol é aqui a letra de `Edit`.

- **Uma entrada de histórico por ajuste**, que pode ser desfeita. A legenda vizinha e a sobreposição são relatadas
  **como ao digitar**: a extremidade é definida, e a tabela marca o que deixou de se sustentar — veja
  [As anomalias](table.md#as-anomalias).
- **O mesmo passo do reprodutor**, portanto um quadro por padrão. **O que vale um quadro** é lido, nesta ordem,
  da **taxa de quadros que o vídeo declara**, da do **documento quando ele conta em quadros** (MicroDVD), depois da
  **grade que as posições desenham**. **Sem nenhuma das três, o gesto se recusa e diz isso**: escolher uma
  taxa de quadros ao acaso deslocaria qualquer extremidade por algo que não é um quadro.
- **Ele não exige um vídeo**, apenas uma seleção: também é usado num arquivo contado em quadros, sem
  filme. Quando um filme está aberto, **a reprodução se move para a nova posição**, para mostrar o quadro que a extremidade
  acabou de assumir.
- Uma extremidade não vai antes da origem: recuar um passo a partir de menos de um quadro a coloca em zero.
