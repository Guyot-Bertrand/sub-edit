# Cortar, copiar e colar textos

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revista por nenhum leitor do idioma.

Três entradas do menu `Edit`, por baixo de `Undo` e `Redo`, e acima de
`Find and Replace…`.

| Entrada | Atalho | O que faz |
| :------ | :----- | :-------- |
| Cortar textos | `Ctrl+X` | copia os textos da seleção e depois esvazia-os |
| Copiar textos | `Ctrl+C` | copia os textos da seleção |
| Colar textos | `Ctrl+V` | escreve os textos copiados a partir da primeira linha selecionada |

**São os textos que viajam, e mais nada**: nem início, nem fim, nem linha
inteira. Colar não move, portanto, nenhuma posição — a sincronização da linha que
recebe um texto continua a ser a sua. É o que o Gaupol faz.

## Sobre o que atuam

**Com uma tradução, o texto da coluna atual**: a tradução se a célula atual
estiver na coluna `Translation`, o texto principal nas restantes —
ver [A tabela](table.md#o-texto-a-que-as-operações-se-destinam). `Cut Texts` esvazia, portanto,
a tradução e deixa o texto principal, e `Paste Texts` escreve na coluna em que
se está. **A área de transferência não leva nenhum documento**: pode copiar-se
uma célula do texto principal e colá-la na tradução, e
vice-versa.

**A seleção, e nunca o ficheiro inteiro.** As três entradas ficam desativadas enquanto
nada estiver selecionado. Noutros pontos da janela, “nada selecionado” quer dizer
“o ficheiro inteiro”; aqui, um `Ctrl+X` infeliz esvaziaria todos os textos
do documento, e uma colagem não teria linha por onde começar.

Enquanto uma célula está aberta, `Ctrl+X`, `Ctrl+C` e `Ctrl+V` pertencem
ao campo de entrada: cortam, copiam e colam carateres, tal como `Del`
apaga ali um caráter e não uma legenda. Lançadas a partir do menu, as entradas começam por confirmar
a entrada: `Copy Texts` copia então o texto tal como acabou de ser
escrito. Ver [Editar uma célula](editing.md).

## Copiar

`Copy Texts` copia o texto de cada linha selecionada, da primeira à
última. **Uma seleção descontínua conserva a sua forma**: uma linha deixada fora
da seleção torna-se um buraco, e a colagem não a sobrescreverá.

**Uma linha selecionada cujo texto está vazio também se torna um buraco**:
colada, deixa a linha de destino como está. Copiar um único texto vazio não
cola, portanto, nada.

A área de transferência do sistema recebe estes textos **como texto simples, separados por uma
linha em branco** — um buraco é aí escrito como um texto vazio, e é por isso que um texto
vazio não pode ser senão um buraco: a colagem dá o mesmo resultado
quer a cópia venha desta janela quer de outro programa. O que se copia aqui pode
ser colado em qualquer programa, e o que se copia noutro lado pode ser colado aqui.

Copiar não modifica o documento e não entra no histórico.

## Cortar

`Cut Texts` copia e depois **esvazia os textos** das linhas selecionadas. As linhas
ficam, com as suas posições: cortar não é remover — para remover
linhas, é [`Remove Subtitles`](lines.md#remover).

Cortar textos que já estão todos vazios não faz nada e não entra no
histórico.

## Colar

`Paste Texts` escreve os textos da área de transferência **a partir da primeira linha
selecionada**, uma linha por texto, para baixo.

| O que a área de transferência contém | O que acontece |
| :----------------------------------- | :------------- |
| um texto | substitui o da linha |
| um buraco, ou um texto vazio copiado | a linha mantém o seu texto |
| mais textos do que linhas restantes | acrescentam-se linhas no fim para os receber |

As linhas acrescentadas nascem como linhas inseridas depois da última: três
segundos cada uma, uma a seguir à outra. **A janela di-lo**, numa caixa de informação:

```text
inserted 2 subtitles to fit the clipboard
```

Depois da colagem, **as linhas escritas ficam selecionadas**.

Uma colagem que nada altera — os mesmos textos nas mesmas linhas — não entra
no histórico.

### As etiquetas de outro formato são convertidas

Uma cópia feita nesta janela **recorda o formato do seu documento**, e
essa memória sobrevive à abertura de outro ficheiro. Colar num documento de
outro formato converte, portanto, as etiquetas, tal como `Save As…` faz:
`{\i1}Bonjour{\i0}` copiado de um Advanced SSA é colado como `<i>Bonjour</i>` num
SubRip.

**O que o formato de destino não consegue escrever perde-se, e a janela di-lo**, nas
palavras de `Save As…`:

```text
pasting SubRip texts into LRC: 1 tag dropped
```

Ao contrário de `Save As…`, nada é perguntado antes: a colagem anula-se com
`Ctrl+Z`, que devolve os textos como estavam.

**Um texto copiado fora do `subedit` não tem formato** e é colado tal como está:
não há nada a converter a partir do nada. É também o caso de uma cópia desta
janela que a área de transferência do sistema entretanto substituiu — o que conta é o
conteúdo da área de transferência do sistema.

Quando as duas mensagens são devidas, cabem numa única caixa, separadas por
um ponto e vírgula.

## O que a ação de anular diz a este respeito

Cada gesto é **uma** entrada do histórico, qualquer que seja o número de
linhas que escreve ou acrescenta.

| Operação | O que `Undo` indica |
| :------- | :------------------ |
| cortar | `Undo: cutting texts` |
| colar | `Undo: pasting texts` |

Ver [Anular e refazer](undo.md).
