# Recortar, copiar e colar textos

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revisada por um leitor do idioma.

Três entradas do menu `Edit`, abaixo de `Undo` e `Redo`, e acima de
`Find and Replace…`.

| Entrada | Atalho | O que faz |
| :------ | :----- | :-------- |
| Recortar textos | `Ctrl+X` | copia os textos da seleção, depois os esvazia |
| Copiar textos | `Ctrl+C` | copia os textos da seleção |
| Colar textos | `Ctrl+V` | escreve os textos copiados a partir da primeira linha selecionada |

**São textos que viajam, e nada mais**: nenhum início, nenhum fim, nenhuma linha
inteira. Colar não move portanto nenhuma posição — a sincronização da linha que
recebe um texto continua sendo a sua. É o que o Gaupol faz.

## Sobre o que atuam

**Com uma tradução, o texto da coluna atual**: a tradução se a
célula atual estiver na coluna `Translation`, o texto principal nos demais casos —
veja [A tabela](table.md#o-texto-sobre-o-qual-as-operações-atuam). `Cut Texts` esvazia portanto
a tradução e deixa o texto principal, e `Paste Texts` escreve na
coluna em que se está. **A área de transferência não carrega nenhum documento**: pode-se
copiar uma célula do texto principal e colá-la na tradução, e
o contrário também.

**A seleção, e nunca o arquivo inteiro.** As três entradas ficam desativadas enquanto
nada estiver selecionado. Em outras partes da janela, “nada selecionado” significa
“o arquivo inteiro”; aqui, um `Ctrl+X` infeliz esvaziaria todos os textos
do documento, e uma colagem não teria linha em que começar.

Enquanto uma célula está aberta, `Ctrl+X`, `Ctrl+C` e `Ctrl+V` pertencem
ao campo de entrada: eles recortam, copiam e colam caracteres, assim como `Del`
apaga um caractere ali e não uma legenda. Acionadas pelo menu, as entradas primeiro confirmam
a entrada: `Copy Texts` copia então o texto como acabou de ser
digitado. Veja [Editar uma célula](editing.md).

## Copiar

`Copy Texts` copia o texto de cada linha selecionada, da primeira à
última. **Uma seleção descontínua conserva sua forma**: uma linha deixada fora
da seleção se torna um buraco, e a colagem não a sobrescreverá.

**Uma linha selecionada cujo texto é vazio também se torna um buraco**:
colada, ela deixa a linha de destino como está. Copiar um único texto vazio não
cola portanto nada.

A área de transferência do sistema recebe esses textos **como texto simples, separados por uma
linha vazia** — um buraco é escrito ali como um texto vazio, e é por isso que um texto
vazio não pode ser outra coisa: a colagem dá o mesmo resultado
quer a cópia venha desta janela, quer de outro programa. O que se copia aqui pode
portanto ser colado em qualquer programa, e o que se copia em outro lugar pode ser colado aqui.

Copiar não modifica o documento, e não entra no histórico.

## Recortar

`Cut Texts` copia, depois **esvazia os textos** das linhas selecionadas. As linhas
permanecem, com suas posições: recortar não é remover — para remover
linhas, usa-se [`Remove Subtitles`](lines.md#remoção).

Recortar textos que já estão todos vazios não faz nada, e não entra no
histórico.

## Colar

`Paste Texts` escreve os textos da área de transferência **a partir da primeira linha
selecionada**, uma linha por texto, para baixo.

| O que a área de transferência contém | O que acontece |
| :----------------------------------- | :------------- |
| um texto | ele substitui o da linha |
| um buraco, ou um texto vazio copiado | a linha mantém seu texto |
| mais textos do que linhas restantes | linhas são acrescentadas no fim para recebê-los |

As linhas acrescentadas nascem como as linhas inseridas depois da última: três
segundos cada uma, uma após a outra. **A janela avisa**, numa caixa de informação:

```text
inserted 2 subtitles to fit the clipboard
```

Após a colagem, **as linhas escritas ficam selecionadas**.

Uma colagem que não muda nada — os mesmos textos nas mesmas linhas — não entra
no histórico.

### As marcações de outro formato são traduzidas

Uma cópia feita nesta janela **lembra o formato do seu documento**, e
essa lembrança sobrevive à abertura de outro arquivo. Colar num documento de
outro formato traduz portanto as marcações, como faz `Save As…`:
`{\i1}Bonjour{\i0}` copiado de um Advanced SSA é colado como `<i>Bonjour</i>` num
SubRip.

**O que o formato de destino não pode escrever é perdido, e isso é dito**, nas
palavras de `Save As…`:

```text
pasting SubRip texts into LRC: 1 tag dropped
```

Ao contrário de `Save As…`, nada é perguntado antes: a colagem é desfeita com
`Ctrl+Z`, que devolve os textos como estavam.

**Um texto copiado fora do `subedit` não tem formato**, e é colado como está:
não há nada a traduzir a partir do nada. É também o caso de uma cópia desta
janela que a área de transferência do sistema substituiu desde então — é o
conteúdo da área de transferência do sistema que conta.

Quando as duas mensagens são devidas, elas cabem numa única caixa, separadas por
um ponto e vírgula.

## O que a ação de desfazer diz a respeito

Cada gesto é **uma** entrada de histórico, qualquer que seja o número de
linhas que ele escreve ou acrescenta.

| Operação | O que `Undo` lê |
| :------- | :-------------- |
| recortar | `Undo: cutting texts` |
| colar | `Undo: pasting texts` |

Veja [Desfazer e refazer](undo.md).
