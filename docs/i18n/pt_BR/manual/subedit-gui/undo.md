# Desfazer e refazer

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revisada por um leitor do idioma.

Toda operação entra num histórico e pode ser desfeita. Uma célula editada conta
como **uma** entrada: o que se digita não é registrado tecla a tecla, só a
confirmação conta.

| Ação | Atalho | Onde |
| :--- | :----- | :--- |
| Desfazer | `Ctrl+Z` | menu **Editar**, barra de ferramentas |
| Refazer | `Ctrl+Shift+Z` | menu **Editar**, barra de ferramentas |

## A ação nomeia o que vai desfazer

O menu não diz “Desfazer”, mas **“Desfazer: edição de um texto”**, conforme a
operação que será desfeita. O botão da barra de ferramentas mantém a palavra sozinha — sua
largura passaria, caso contrário, a seguir a última operação, e ele se moveria sob o
ponteiro — mas sua dica de ferramenta traz o rótulo completo.

| Operação | O que a ação indica |
| :------- | :------------------ |
| uma célula de texto editada | `editing a text` |
| um início, um fim | `editing a start`, `editing an end` |
| uma duração | `editing a duration` |
| uma inserção de linhas | `inserting` |
| uma remoção de linhas | `removing` |
| um deslocamento, `Shift Whole File onto Grid` incluído | `shifting` |
| uma transformação | `transforming` |
| uma conversão de taxa de quadros | `converting the frame rate` |
| um alinhamento a uma taxa de quadros | `aligning on the frame rate` |
| uma remoção de menções | `removing hearing-impaired mentions` |
| colocar em itálico | `putting in italics` |
| tirar o itálico | `taking italics out` |
| uma mudança de maiúsculas e minúsculas | `changing the case` |
| travessões acrescentados, removidos | `adding dialogue dashes`, `removing dialogue dashes` |
| uma união de linhas | `merging` |
| uma divisão de uma linha | `splitting` |
| textos recortados, colados | `cutting texts`, `pasting texts` |
| um ajuste de durações | `adjusting durations` |
| a abertura de uma tradução | `opening a translation` |
| o acréscimo de um arquivo | `appending a file` |
| a divisão do projeto | `splitting the project` |
| uma correção aceita em `Correct Texts…`, ou feita por `Check Spelling…` | `correcting texts` |
| uma substituição, todas as substituições | `replacing`, `replacing all` |

**Vinte e sete rótulos, e é tudo o que a janela pode produzir.** O núcleo nomeia
um vigésimo oitavo — `sorting` — que nenhuma ação da janela alcança: a ordenação
só acontece sob uma política de ordem estrita, ao passo que a janela abre seus documentos
sob a política tolerante, que relata a desordem em vez de repará-la.

**`Shift Whole File onto Grid` não se distingue de um deslocamento**, e isso é
correto: é um deslocamento, cuja quantidade foi medida e não digitada.

São as mesmas palavras da linha de comando: elas são escritas uma única vez,
no núcleo.

Quando não há nada a desfazer, a ação diz apenas “Desfazer” **e fica desativada**: seu
atalho não faz nada.

## A marca de modificação

O título da janela traz uma marca enquanto o arquivo difere do que está no
disco — um asterisco, ou o que a plataforma usar no lugar. **O rótulo da aba
também**, `premier.srt*`: o título fala apenas do projeto que se está olhando,
o rótulo informa sobre os que não se estão olhando.

**Cada aba tem seu próprio histórico.** Desfazer só desfaz o que o projeto da
aba atual recebeu — inclusive um `Replace All` feito sobre todos os projetos:
ele deixa uma entrada em cada projeto que tocou.

**Ela desaparece se se desfaz até o ponto de partida.** Isso não é um detalhe
de exibição: o número de modificações é contado, e não registrado como um sim ou não,
de modo que voltar atrás pode trazê-lo de volta a zero, e a marca some.

## Duas coisas a saber

**O ponto de referência é o último salvamento**, ou a abertura do arquivo se
não houve nenhum. Salvar o move: a marca desaparece, e desfazer depois
a traz de volta.

Uma operação feita após um desfazer **apaga o que havia a refazer**: refazer
reproduziria um comando cujo estado de partida não existe mais. É o
comportamento de todo editor, e é deliberado.

**Enquanto uma célula está aberta**, `Undo` e `Redo` — pelo menu ou pela barra de ferramentas —
primeiro confirmam a entrada, como os outros gestos de
[Editar uma célula](editing.md). O que acabou de ser digitado se torna então a última
entrada do histórico: `Undo` o desfaz, e não a operação anterior; `Redo` não tem
mais nada a refazer, já que uma entrada apaga o que havia a refazer. Uma célula
aberta em que nada foi digitado não deixa entrada: `Undo` desfaz então a operação
anterior.

O histórico guarda mil entradas e esquece as mais antigas além disso.
