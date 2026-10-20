# `convert`

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revista por nenhum leitor do idioma.

```
subedit-cli convert --to <format>
                    [--line-endings unix|windows|mac] [--to-encoding NAME]
                    [--bom | --no-bom] [--sort] [--disjoint]
                    [--join-repeats] [--repair]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    <file>...
```

Escreve cada ficheiro no formato e na forma pedidos. **Nunca altera
a entrada**, a menos que `--in-place` o peça explicitamente.

<!-- example: subedit-cli convert --help -->
```console
$ subedit-cli convert --help
Write a subtitle file out in another format or shape
Usage: subedit-cli convert [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to convert

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --to TEXT:{srt,vtt,subviewer2,ssa,ass,microdvd,mpl2,tmplayer,lrc} REQUIRED
                              Format to write
  --line-endings TEXT:{unix,windows,mac}
                              Line endings to write; the source's by default
  --to-encoding NAME          Encoding to write; the source's by default
  --frame-rate RATE           Frame rate of a file counted in frames: 25, 23.976
  --bom                       Write a byte order mark
  --no-bom                    Write no byte order mark
  --sort                      Put the subtitles in order of their start before writing
  --disjoint                  Cut overlapping subtitles so that no range overlaps another, before writing
  --join-repeats              Join the subtitles that repeat each other into one, before writing
  --repair                    Join the repeats, sort, then make the ranges disjoint, before writing
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Argumentos e opções

| Opção | Obrigatório | Valores | Predefinição |
| :----- | :----- | :------ | :----- |
| `<file>...` | sim | um ou mais caminhos | — |
| `--recursive`, `-r` | não | um indicador | desativado — ver [Processar uma árvore de pastas](lots.md) |
| `--to` | **sim** | um nome da tabela abaixo, e nada mais | — |
| `--line-endings` | não | `unix`, `windows` ou `mac` | os do ficheiro lido |
| `--to-encoding` | não | qualquer codificação que o ICU saiba escrever, exceto as que escrevem a sua própria marca | a do ficheiro lido |
| `--bom` / `--no-bom` | não | indicadores, mutuamente exclusivos | o que o ficheiro lido trazia |
| `--sort` | não | um indicador: põe as legendas pela ordem do seu início **antes de escrever**, ver [Ordenar ao escrever](#ordenar-ao-escrever) | desativado |
| `--join-repeats` | não | um indicador: faz uma única legenda das partes de um mesmo texto que se seguem, **antes de tudo o resto**, ver [Tornar os intervalos disjuntos](#tornar-os-intervalos-disjuntos) | desativado |
| `--repair` | não | um indicador: o mesmo que `--join-repeats --sort --disjoint`, por esta ordem | desativado |
| `--disjoint` | não | um indicador: corta as legendas sobrepostas **antes de escrever**, ver [Tornar os intervalos disjuntos](#tornar-os-intervalos-disjuntos) | desativado |
| `--output` / `--output-dir` / `--in-place` | **uma das três**, exceto com `--dry-run` | ver [Invocação](invocation.md#o-destino) | — |
| `--dry-run` | não | um indicador: calcula e diz, não escreve nada — ver [Ver antes de escrever](invocation.md#ver-antes-de-escrever) | desativado |

`mac` designa o retorno de carro isolado (`\r`), o fim de linha do Mac OS clássico.

## Os formatos que `--to` aceita

| Nome | Formato | Extensão escrita |
| :-- | :----- | :--------------- |
| `srt` | SubRip | `.srt` |
| `vtt` | WebVTT | `.vtt` |
| `subviewer2` | SubViewer 2.0 | `.sub` |
| `ssa` | Sub Station Alpha | `.ssa` |
| `ass` | Advanced SSA | `.ass` |
| `mpl2` | MPL2 | `.txt` |
| `microdvd` | MicroDVD | `.sub` |
| `tmplayer` | TMPlayer | `.txt` |
| `lrc` | LRC | `.lrc` |

**Um nome, e não uma extensão.** Duas extensões designam dois formatos
cada uma — `.sub` é também a do MicroDVD, `.txt` a do MPL2 e a do
TMPlayer — pelo que uma opção que aceitasse extensões não saberia qual
se pede. Os nomes acima não são ambíguos e coincidem com a
extensão sempre que a extensão também não é ambígua.

**A lista é fechada.** É a do Gaupol, e os nove nela constam.

## Os dois formatos que não têm fim

**O TMPlayer e o LRC escrevem apenas uma posição por linha**, o seu início. Isto
nota-se dos dois lados.

**Ao ler**, os fins são deduzidos: o de uma linha é o início da
seguinte, e a última recebe cinco segundos. É o que o Gaupol faz.
Também se diz, através de um diagnóstico que `--verbose` mostra nos três
subcomandos:

```
file.lrc: carries no end times; each one was taken from the next start, settled by the reader
```

**Ao escrever**, os fins não são escritos, por falta de lugar onde os pôr.
Converter para um destes dois formatos perde-os, portanto, e uma ida e volta não
os encontra de novo — deduz-os outra vez.

**O LRC perde mais uma coisa**: não tem maneira de conter uma quebra de linha.
Uma fala em duas linhas é escrita numa só, com as duas linhas unidas por
um espaço. Também essa perda não se pode reparar.

Nenhum dos dois tem vocabulário para o itálico.

## `--frame-rate` e o único formato que conta em fotogramas

**O MicroDVD não conta em segundos**: as suas posições *são* números
de fotograma, e nenhum ficheiro MicroDVD declara a taxa a que foram
contados. `--frame-rate` fornece-a e serve dos dois lados: é a
taxa a que se lê um `.sub` em fotogramas e aquela a que uma
conversão para MicroDVD conta os fotogramas.

| Opção | Obrigatório | Valores | Predefinição |
| :----- | :----- | :------ | :----- |
| `--frame-rate` | não | uma taxa em fotogramas por segundo: `25`, `23.976`, `29.97` | ver abaixo |

**Ao ler, sem `--frame-rate`, o ficheiro é lido a 23,976** — o valor do
Gaupol — e o nível `-vvv` di-lo:
`counts in frames and states no rate; it was read at`. É o único lugar da ferramenta em que todas as posições mostradas
assentam num pressuposto que o ficheiro não pode confirmar.

**Ao escrever, sem `--frame-rate`, três casos e três respostas:**

| O ficheiro de partida | O que acontece |
| :------------------- | :-------------- |
| já é MicroDVD | a taxa que o leu volta a escrevê-lo, e os fotogramas regressam idênticos |
| é baseado em tempo e as suas posições caem numa grelha | a grelha é adotada, e `-vv` diz qual |
| é baseado em tempo, sem grelha | **recusa**, código 2: `writing frames needs a frame rate, and the positions fall on no grid to take one from — give --frame-rate` |

A recusa é deliberada. O único gesto que restaria seria inventar um valor, e
cada linha do ficheiro se deslocaria na medida do erro desse valor.

## O que uma conversão perde

**Converter para outro formato nunca é sem perdas**, e o que não sobrevive
depende do par:

| O que não passa | Porquê |
| :--------------------- | :------- |
| o cabeçalho | só faz sentido no seu próprio formato: `[INFORMATION]` não é um cabeçalho WebVTT |
| os dados específicos do formato | as coordenadas do SubRip, o identificador e as definições de uma entrada WebVTT não têm equivalente noutros formatos |
| a precisão | o SubViewer 2, os dois Sub Station Alpha e o LRC escrevem ao centésimo, o MPL2 ao décimo, o TMPlayer ao segundo, o MicroDVD ao fotograma; `01:00:00,017` volta como `01:00:00,020` |
| o fim de uma linha | o TMPlayer e o LRC não têm campo para ele; a leitura seguinte deduz-o de novo |
| as quebras de linha | só o LRC: as suas linhas cabem numa única linha, e as duas são coladas por um espaço |
| as etiquetas que o formato de destino não sabe escrever | o WebVTT não tem `<font color>`, o TMPlayer e o LRC não têm nada |
| a disposição | `{\pos(x,y)}`, `{\an8}`, `<v Marie>`, `<ruby>`: sobrevivem a uma ida e volta no seu próprio formato, e desaparecem na conversão |

**Escrever um ficheiro de volta no seu próprio formato não perde nada** — é a garantia
que sustenta todas as outras, e `--to srt` num `.srt` devolve os mesmos bytes. Nem sequer
se descodifica: o texto continua a ser a cadeia bruta que foi lida.

## As etiquetas são convertidas, não copiadas

**Cada formato escreve o itálico à sua maneira**, e converter sem traduzir
deixaria um `{\i1}` num `.srt`: texto que o utilizador vê, num
ficheiro que nunca saberá interpretá-lo. O que passa são **seis
coisas** — negrito, itálico, sublinhado, cor, tipo de letra e tamanho:

| Formato | Negrito | Itálico | Sublinhado | Cor | Tipo de letra | Tamanho |
| :----- | :--: | :------: | :------: | :-----: | :----: | :----: |
| SubRip | `<b>` | `<i>` | `<u>` | `<font color="#RRGGBB">` | — | — |
| WebVTT | `<b>` | `<i>` | `<u>` | — | — | — |
| SubViewer 2 | `<b>` | `<i>` | `<u>` | `<font color="#RRGGBB">` | — | — |
| Sub Station Alpha | `{\b1}` | `{\i1}` | — | `{\c&HBBGGRR&}` | `{\fnNAME}` | `{\fsN}` |
| Advanced SSA | `{\b1}` | `{\i1}` | `{\u1}` | `{\c&HBBGGRR&}` | `{\fnNAME}` | `{\fsN}` |
| MicroDVD | `{Y:b}` | `{Y:i}` | `{Y:u}` | `{C:$BBGGRR}` | `{F:NAME}` | `{S:N}` |
| MPL2 | `\` | `/` | `_` | `{C:$BBGGRR}` | `{F:NAME}` | `{S:N}` |
| TMPlayer | — | — | — | — | — | — |
| LRC | — | — | — | — | — | — |

**Um “—” é uma perda anunciada.** Converter um `.srt` com cor para WebVTT retira
a cor e di-lo; o ficheiro produzido é WebVTT, não SubRip disfarçado.

**O MicroDVD e o MPL2 não conseguem estilizar parte de uma linha.** As suas etiquetas
não têm fim: `{y:i}` inclina tudo o que se segue até ao fim da linha, e
`{Y:i}` até ao fim da legenda. Uma linha em que apenas metade estava
a negrito é, portanto, escrita sem negrito nenhum, em vez de com um negrito que transborda.

## O que a conversão diz ter perdido

**Di-lo quando perde, e cala-se quando não perde nada.** Um relatório
que aparece em todas as chamadas é um relatório que ninguém lê.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,500\n<i>sur deux</i>\nlignes\n\n' > a.srt; subedit-cli convert --to lrc --output b.lrc a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,500\n<i>sur deux</i>\nlignes\n\n' > a.srt; subedit-cli convert --to lrc --output b.lrc a.srt
a.srt: 1 subtitle written as LRC -> b.lrc
a.srt: ends are not carried by LRC, line breaks were joined in 1 subtitle, 1 tag dropped
```

A linha aparece ao nível de verbosidade predefinido, e em todos os que lhe são superiores.
Os itens, por esta ordem:

| Item | O que diz |
| :---- | :----------- |
| `ends are not carried by <format>` | o formato de destino não escreve fim |
| `line breaks were joined in N subtitles` | só o LRC: N legendas estavam em várias linhas |
| `N tags dropped` | N etiquetas não puderam ser escritas, a disposição incluída |
| `the header was dropped` | o cabeçalho não atravessa a fronteira entre formatos |
| `the <format> fields of N subtitles were dropped` | N legendas traziam dados específicos do seu formato |
| `positions moved by up to N ms` | a maior distância em que uma posição se moveu |

## Alterar a codificação

**`--to-encoding` escolhe a codificação escrita; `--encoding` indica a que é
lida.** As duas juntas recodificam um ficheiro:

```console
$ subedit-cli convert --to srt --encoding cp1252 --to-encoding utf-8 \
      --output-dir sortie film.srt
```

Sem `--to-encoding`, o ficheiro é escrito na **codificação em que foi lido**,
como acontece com os seus fins de linha e a sua marca: um ficheiro convertido sem uma
instrução sobre a codificação devolve os mesmos carateres, nos mesmos bytes.

**Um caráter que a codificação de destino não consegue escrever interrompe o ficheiro**, e
nada é escrito. Um `ł` não tem lugar em Latin-1; substituí-lo por um `?`
perderia texto entre a leitura e a escrita, sem que ninguém o visse.

**Uma marca de ordem de bytes só existe para as codificações Unicode.**
`--bom` com um `--to-encoding` que não tem nenhuma é recusado em vez de ignorado
— ver a tabela de erros abaixo.

**E a codificação tem de indicar a sua ordem de bytes.** `UTF-16` e `UTF-32` são
nomes que o ICU conhece, cujo conversor escreve **uma marca própria**, seja o que for
que se peça: `--no-bom` seria então aceite e desobedecido. Ambos são,
portanto, recusados, e a mensagem nomeia a saída:

```console
$ subedit-cli convert --to srt --to-encoding UTF-16 --output copie.srt film.srt
"UTF-16" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do
```

`UTF-16LE`, `UTF-16BE`, `UTF-32LE` e `UTF-32BE` escrevem exatamente os mesmos
bytes, a marca incluída — mas sob o controlo de `--bom` e `--no-bom`. A
recusa não custa, portanto, mais do que um nome mais preciso, e vale tanto para
a leitura como para a escrita: `--encoding UTF-16` recebe a mesma resposta.

## O destino

As três opções e as suas regras são comuns aos subcomandos que
escrevem: ver [Invocação](invocation.md#o-destino). Nada é escrito
sem uma delas.

O que é específico de `convert`: **`--output-dir` muda a extensão com o
formato.** `convert --to vtt` em `a.srt` escreve `a.vtt`, nunca um `a.srt` com
WebVTT — um ficheiro cujo nome mente baralha todas as outras ferramentas.

## Converter no próprio ficheiro é recusado

`--in-place` com um `--to` que muda o formato dá o código `1` e não escreve
nada: no próprio ficheiro, não há um segundo nome que transporte o novo formato, e
o ficheiro ficaria com uma extensão que o seu conteúdo já não justifica.

`--in-place` pode, ainda assim, usar-se para mudar **a forma sem o formato** —
escrever um `.srt` de volta como `.srt` com outros fins de linha, por exemplo.

A recusa decide-se **apenas pela extensão**, antes de qualquer leitura: um erro de
utilização nunca deve deixar um lote escrito a meio.

## Fins de linha e marca de ordem de bytes

Por predefinição, **o ficheiro escrito retoma o que o ficheiro lido trazia**. O modelo
guarda ambos ao ler; impô-los por predefinição perderia, em cada
conversão, uma informação conservada de propósito.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli convert --to vtt --output b.vtt a.srt; cat b.vtt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli convert --to vtt --output b.vtt a.srt; cat b.vtt
a.srt: 1 subtitle written as WebVTT -> b.vtt
WEBVTT

00:01.000 --> 00:03.000
Bonjour.
```

## Ordenar ao escrever

`--sort` escreve o ficheiro **pela ordem dos inícios**, seja qual for o formato de destino. A ordenação faz-se **sobre uma cópia**: o ficheiro lido não
é tocado, e o que se escreve é o que [`sort`](sort.md) dele teria feito — estável, duas legendas que começam em simultâneo mantêm a sua ordem.

**Nunca por predefinição**: a ordem de um ficheiro é a do seu autor enquanto nada mais for pedido. Com a opção, cada ficheiro di-lo, ao
primeiro nível de narração, `<path>: N subtitles moved` ou `<path>: already in order`, e o registo de `--format json` traz uma
contagem `moved` — **ausente sem a opção**, para não alterar a forma do que os scripts existentes leem.

## Tornar os intervalos disjuntos

`--disjoint` corta as legendas sobrepostas para que nenhum intervalo se sobreponha a outro, **sobre a mesma cópia que a ordenação**: o ficheiro lido não é tocado.

Nunca por predefinição. A opção faz o que [`disjoint`](disjoint.md) faz, **com as suas definições predefinidas**, antes de escrever, depois da ordenação. Precisa de um ficheiro ordenado: caso contrário, é recusada, e `--sort` — que vem primeiro — ordena-o previamente. O ficheiro di-lo ao primeiro nível de narração (`<path>: N subtitles split, M merged`), e o registo de `--format json` traz as contagens `split` e `merged`, **ausentes sem a opção**.

**`--join-repeats` vem antes da ordenação e do corte**: um texto cortado em partes que se seguem é primeiro reconstruído (ver
[`join-repeats`](join-repeats.md)), depois as legendas são ordenadas e, por fim, os intervalos tornados disjuntos. **`--repair` é as três de uma vez**,
como [`repair`](repair.md), com as definições predefinidas do corte. Com `--join-repeats`, cada ficheiro diz
`<path>: N repeated subtitles joined` ao primeiro nível de narração, e o registo de `--format json` traz `counts.joined` — **ausente sem a opção**.

## Saída

**Saída padrão** — nada: o resultado é o ficheiro escrito. Com `--dry-run`, também nada: não há ficheiro, e a linha de narração di-lo — ver [Ver antes de escrever](invocation.md#ver-antes-de-escrever).

Na saída de erros, consoante o nível:

| Nível | O que se acrescenta |
| :----- | :-------------- |
| 1 | `<path>: N subtitles written as WebVTT -> <destination>` |
| 2 | `<path>: SubRip -> WebVTT, UTF-8, no BOM, LF line endings` — a codificação nomeada é a escrita |
| 3 | `<path>: N bytes read, M written`, depois **cada diagnóstico de leitura** — ver [Invocação](invocation.md#ler-os-diagnósticos) |

### Em JSON

Com [`--format json`](invocation.md#uma-saída-que-um-script-pode-ler), o objeto de cada ficheiro traz `counts.subtitles`, depois o que a conversão perdeu: `lost_ends` e `lost_header` (`0` ou `1`), `joined_lines`, `lost_tags`, `lost_fields` e `furthest_ms` — os itens da [frase da perda](#o-que-a-conversão-diz-ter-perdido). A tabela completa é a de [Invocação](invocation.md#o-que-cada-subcomando-põe-em-counts).

## Códigos de saída

Os da ferramenta: `0` se todos os ficheiros foram escritos, `2` se nenhum, `3` se
apenas alguns, `1` em caso de erro de utilização.

## Erros

| O que os desencadeia | Mensagem |
| :------------------ | :------ |
| `--bom` com `--no-bom` | `--bom and --no-bom ask for opposite things; give one or the other` |
| `--in-place` que muda o formato | `--in-place cannot change the format: the file would keep a name its content no longer matches` |
| `--bom` numa codificação que não tem nenhuma | `<path>: <encoding> has no byte order mark to write` |
| `--to-encoding` que nomeia uma codificação desconhecida | `no encoding is named "<name>"` |
| `--to-encoding` que nomeia uma codificação que escreve a sua própria marca | `"<name>" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do` |
| caráter em falta na codificação escrita | `<path>: holds a character the chosen encoding cannot write` |

**Recusar `--bom` numa codificação que não tem nenhuma não é
pedantismo.** Uma marca de ordem de bytes existe para as codificações Unicode e
para mais nenhuma: pedir uma num ficheiro em Windows-1252 é pedir
algo que não existe. O ficheiro seria escrito sem ela, e ninguém
saberia que o pedido não foi cumprido.

Os do destino são comuns aos subcomandos que escrevem:
ver [Invocação](invocation.md#o-destino).

Os erros de leitura são os de [`inspect`](inspect.md): dizem respeito
ao ficheiro de entrada e têm as mesmas mensagens.
