# Anular e refazer

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revista por nenhum leitor do idioma.

Cada operação entra num histórico e pode ser anulada. Uma célula editada conta
como **uma** entrada: o que se escreve não é registado tecla a tecla, só conta a
confirmação.

| Ação | Atalho | Onde |
| :--- | :----- | :--- |
| Anular | `Ctrl+Z` | menu **Editar**, barra de ferramentas |
| Refazer | `Ctrl+Shift+Z` | menu **Editar**, barra de ferramentas |

## A ação diz o que vai anular

O menu não diz “Anular”, mas **“Anular: edição de um texto”**, segundo a
operação que vai ser anulada. O botão da barra de ferramentas mantém a palavra
simples — a sua largura acompanharia, de outro modo, a última operação, e deslocar-se-ia
debaixo do ponteiro —, mas a sua dica de ferramenta traz a etiqueta completa.

| Operação | O que a ação indica |
| :------- | :------------------ |
| uma célula de texto editada | `editing a text` |
| um início, um fim | `editing a start`, `editing an end` |
| uma duração | `editing a duration` |
| uma inserção de linhas | `inserting` |
| uma remoção de linhas | `removing` |
| um deslocamento, `Shift Whole File onto Grid` incluído | `shifting` |
| uma transformação | `transforming` |
| uma conversão de taxa de fotogramas | `converting the frame rate` |
| um alinhamento por uma taxa de fotogramas | `aligning on the frame rate` |
| uma remoção de menções | `removing hearing-impaired mentions` |
| uma colocação em itálico | `putting in italics` |
| uma retirada do itálico | `taking italics out` |
| uma mudança de maiúsculas e minúsculas | `changing the case` |
| hífens de diálogo acrescentados, removidos | `adding dialogue dashes`, `removing dialogue dashes` |
| uma união de linhas | `merging` |
| uma divisão de uma linha | `splitting` |
| textos cortados, colados | `cutting texts`, `pasting texts` |
| um ajuste de durações | `adjusting durations` |
| a abertura de uma tradução | `opening a translation` |
| o anexo de um ficheiro | `appending a file` |
| a divisão do projeto | `splitting the project` |
| uma correção aceite em `Correct Texts…`, ou feita por `Check Spelling…` | `correcting texts` |
| uma substituição, todas as substituições | `replacing`, `replacing all` |

**Vinte e sete etiquetas, e é tudo o que a janela pode produzir.** O núcleo dá
nome a uma vigésima oitava — `sorting` — que nenhuma ação da janela alcança: a ordenação
só acontece sob uma política de ordem estrita, ao passo que a janela abre os seus documentos
sob a política tolerante, que comunica a desordem em vez de a reparar.

**`Shift Whole File onto Grid` não se distingue de um deslocamento**, e isso
é exato: é um deslocamento, cuja quantidade foi medida e não escrita.

São as mesmas palavras que as da linha de comandos: estão escritas uma só vez,
no núcleo.

Quando não há nada a anular, a ação diz apenas “Anular” **e está desativada**: o seu
atalho não faz nada.

## A marca de modificação

O título da janela leva uma marca enquanto o ficheiro diferir do que está
no disco — um asterisco, ou o que a plataforma usar em seu lugar. **O mesmo acontece com a etiqueta
do separador**, `premier.srt*`: o título fala apenas do projeto que se está a ver,
a etiqueta informa sobre aqueles que não se estão a ver.

**Cada separador tem o seu próprio histórico.** Anular só anula o que o projeto do
separador atual recebeu — incluindo um `Replace All` feito sobre todos os projetos:
deixa uma entrada em cada projeto que atingiu.

**A marca desaparece se se anular até ao ponto de partida.** Não é um pormenor
de apresentação: o número de modificações é contado, não registado como um sim ou não,
pelo que voltar atrás pode trazê-lo de novo a zero e a marca desaparece.

## Duas coisas a saber

**O ponto de referência é a última vez que se guardou**, ou a abertura do ficheiro se
não houve nenhuma. Guardar desloca-o: a marca desaparece, e anular depois
volta a trazê-la.

Uma operação feita depois de uma anulação **apaga o que havia para refazer**:
refazer repetiria um comando cujo estado de partida já não existe. É o
comportamento de todos os editores, e é deliberado.

**Enquanto uma célula está aberta**, `Undo` e `Redo` — pelo menu ou pela barra de ferramentas —
confirmam primeiro a entrada, como os outros gestos de
[Editar uma célula](editing.md). O que acabou de ser escrito torna-se então a última
entrada do histórico: `Undo` anula-a, não a operação anterior; `Redo` já não tem
nada para refazer, visto que uma entrada apaga o que havia para refazer. Uma
célula aberta onde nada foi escrito não deixa entrada: `Undo` anula então a operação
anterior.

O histórico guarda mil entradas e esquece as mais antigas para além disso.
