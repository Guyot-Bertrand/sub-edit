# `convert`

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revisada por um leitor do idioma.

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

Grava cada arquivo no formato e na forma pedidos. **Nunca altera
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

| Opção | Obrigatória | Valores | Padrão |
| :----- | :----- | :------ | :----- |
| `<file>...` | sim | um ou mais caminhos | — |
| `--recursive`, `-r` | não | uma opção sem valor | desativada — veja [Processar uma árvore de diretórios](lots.md) |
| `--to` | **sim** | um nome da tabela abaixo, e nada mais | — |
| `--line-endings` | não | `unix`, `windows` ou `mac` | os do arquivo lido |
| `--to-encoding` | não | qualquer codificação que o ICU possa escrever, exceto as que escrevem a própria marca | a do arquivo lido |
| `--bom` / `--no-bom` | não | opções sem valor, mutuamente exclusivas | o que o arquivo lido trazia |
| `--sort` | não | uma opção sem valor: coloca as legendas na ordem de seus inícios **antes de gravar**, veja [Ordenar ao escrever](#ordenar-ao-escrever) | desativada |
| `--join-repeats` | não | uma opção sem valor: forma uma única legenda com os pedaços de um mesmo texto que se sucedem, **antes de qualquer outra coisa**, veja [Tornar os intervalos disjuntos](#tornar-os-intervalos-disjuntos) | desativada |
| `--repair` | não | uma opção sem valor: o mesmo que `--join-repeats --sort --disjoint`, nessa ordem | desativada |
| `--disjoint` | não | uma opção sem valor: corta as legendas que se sobrepõem **antes de gravar**, veja [Tornar os intervalos disjuntos](#tornar-os-intervalos-disjuntos) | desativada |
| `--output` / `--output-dir` / `--in-place` | **uma das três**, exceto com `--dry-run` | veja [Invocação](invocation.md#o-destino) | — |
| `--dry-run` | não | uma opção sem valor: calcula e informa, não grava nada — veja [Ver antes de escrever](invocation.md#ver-antes-de-escrever) | desativada |

`mac` designa o retorno de carro sozinho (`\r`), o final de linha do Mac OS clássico.

## Os formatos que `--to` aceita

| Nome | Formato | Extensão gravada |
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
cada uma — `.sub` também é a do MicroDVD, `.txt` a do MPL2 e a do
TMPlayer —, de modo que uma opção que recebesse extensões não saberia qual
delas se pede. Os nomes acima são inequívocos e coincidem com a
extensão onde a extensão também é inequívoca.

**A lista é fechada.** É a do Gaupol, e os nove estão nela.

## Os dois formatos que não trazem fim

**TMPlayer e LRC escrevem apenas uma posição por linha**, o início. Isso
aparece nos dois lados.

**Na leitura**, os fins são deduzidos: o de uma linha é o início da
seguinte, e a última recebe cinco segundos. É o que o Gaupol faz.
Também informamos isso, por meio de um diagnóstico que `--verbose` mostra nos três
subcomandos:

```
file.lrc: carries no end times; each one was taken from the next start, settled by the reader
```

**Na escrita**, os fins não são gravados, por falta de lugar onde colocá-los.
Converter para um desses dois formatos os perde, portanto, e uma viagem de ida e volta não
os recupera — ela os deduz de novo.

**O LRC perde mais uma coisa**: ele não tem como carregar uma quebra de linha.
Uma fala em duas linhas é gravada em uma só, com as duas linhas unidas por
um espaço. Essa perda também não pode ser reparada.

Nenhum dos dois tem vocabulário para o itálico.

## `--frame-rate` e o único formato que conta em quadros

**O MicroDVD não conta em segundos**: suas posições *são* números de
quadros, e nenhum arquivo MicroDVD declara a taxa com que foram
contados. `--frame-rate` a fornece e serve nos dois lados: é a
taxa com que um `.sub` em quadros é lido e aquela com que uma
conversão para MicroDVD conta os quadros.

| Opção | Obrigatória | Valores | Padrão |
| :----- | :----- | :------ | :----- |
| `--frame-rate` | não | uma taxa em quadros por segundo: `25`, `23.976`, `29.97` | veja abaixo |

**Na leitura, sem `--frame-rate`, o arquivo é lido a 23.976** — o valor do
Gaupol — e o nível `-vvv` informa isso:
`counts in frames and states no rate; it was read at`. É o único lugar da ferramenta em que toda posição exibida
repousa sobre uma suposição que o arquivo não pode confirmar.

**Na escrita, sem `--frame-rate`, três casos e três respostas:**

| O arquivo de partida | O que acontece |
| :------------------- | :-------------- |
| já é MicroDVD | a taxa que o leu o grava de volta, e os quadros voltam idênticos |
| é baseado em tempo, e suas posições caem sobre uma grade | a grade é adotada, e `-vv` diz qual |
| é baseado em tempo, sem grade | **recusa**, código 2: `writing frames needs a frame rate, and the positions fall on no grid to take one from — give --frame-rate` |

A recusa é deliberada. O único gesto restante seria inventar um número, e
cada linha do arquivo se deslocaria na medida do erro desse número.

## O que uma conversão perde

**Converter para outro formato nunca é sem perdas**, e o que não sobrevive
depende do par:

| O que não passa | Por quê |
| :--------------------- | :------- |
| o cabeçalho | só faz sentido em seu próprio formato: `[INFORMATION]` não é um cabeçalho WebVTT |
| os dados específicos do formato | as coordenadas do SubRip, o identificador e as configurações de uma cue WebVTT não têm equivalente em outros formatos |
| a precisão | SubViewer 2, os dois Sub Station Alpha e LRC gravam em centésimos, MPL2 em décimos, TMPlayer em segundos, MicroDVD em quadros; `01:00:00,017` volta como `01:00:00,020` |
| o fim de uma linha | TMPlayer e LRC não têm campo para ele; a leitura seguinte o deduz de novo |
| as quebras de linha | só o LRC: suas linhas cabem em uma única linha, e as duas são coladas por um espaço |
| as marcações que o formato de destino não pode escrever | o WebVTT não tem `<font color>`, TMPlayer e LRC não têm nada |
| a disposição | `{\pos(x,y)}`, `{\an8}`, `<v Marie>`, `<ruby>`: sobrevivem a uma viagem de ida e volta em seu próprio formato e desaparecem na conversão |

**Gravar um arquivo de volta em seu próprio formato não perde nada** — é a garantia
que sustenta todas as outras, e `--to srt` sobre um `.srt` devolve os mesmos bytes. Nada
chega a ser decodificado: o texto continua sendo a string bruta que foi lida.

## As marcações são traduzidas, não copiadas

**Cada formato escreve o itálico à sua maneira**, e converter sem traduzir
deixaria um `{\i1}` em um `.srt`: texto que o usuário vê, em um
arquivo que nunca saberá interpretá-lo. O que passa são **seis
coisas** — negrito, itálico, sublinhado, cor, fonte e tamanho:

| Formato | Negrito | Itálico | Sublinhado | Cor | Fonte | Tamanho |
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

**Um travessão é uma perda anunciada.** Converter um `.srt` colorido para WebVTT remove
a cor e informa isso; o arquivo produzido é WebVTT, não um SubRip disfarçado.

**MicroDVD e MPL2 não conseguem estilizar parte de uma linha.** Suas marcações
não têm fim: `{y:i}` inclina tudo o que se segue até o fim da linha, e
`{Y:i}` até o fim da legenda. Uma linha em que só metade estava
em negrito é, portanto, gravada sem negrito algum, em vez de com um negrito que transborda.

## O que a conversão declara ter perdido

**Ela informa quando perde e se cala quando não perde nada.** Um relatório
que aparece a cada chamada é um relatório que ninguém lê.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,500\n<i>sur deux</i>\nlignes\n\n' > a.srt; subedit-cli convert --to lrc --output b.lrc a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,500\n<i>sur deux</i>\nlignes\n\n' > a.srt; subedit-cli convert --to lrc --output b.lrc a.srt
a.srt: 1 subtitle written as LRC -> b.lrc
a.srt: ends are not carried by LRC, line breaks were joined in 1 subtitle, 1 tag dropped
```

A linha aparece no nível de detalhamento padrão e em todos os acima dele.
Os itens, nesta ordem:

| Item | O que ele diz |
| :---- | :----------- |
| `ends are not carried by <format>` | o formato de destino não grava fim |
| `line breaks were joined in N subtitles` | só o LRC: N legendas estavam em várias linhas |
| `N tags dropped` | N marcações não puderam ser gravadas, disposição incluída |
| `the header was dropped` | o cabeçalho não atravessa a fronteira entre formatos |
| `the <format> fields of N subtitles were dropped` | N legendas traziam dados específicos de seu formato |
| `positions moved by up to N ms` | a maior distância pela qual uma posição se moveu |

## Mudar a codificação

**`--to-encoding` escolhe a codificação gravada; `--encoding` indica a que é
lida.** As duas juntas recodificam um arquivo:

```console
$ subedit-cli convert --to srt --encoding cp1252 --to-encoding utf-8 \
      --output-dir sortie film.srt
```

Sem `--to-encoding`, o arquivo é gravado na **codificação em que foi lido**,
como acontece com seus finais de linha e sua marca: um arquivo convertido sem
instrução sobre codificação devolve os mesmos caracteres, nos mesmos bytes.

**Um caractere que a codificação de destino não consegue escrever interrompe o arquivo**, e
nada é gravado. Um `ł` não tem lugar no Latin-1; substituí-lo por um `?`
perderia texto entre a leitura e a escrita, sem que ninguém visse.

**Uma marca de ordem de bytes só existe para as codificações Unicode.**
`--bom` com um `--to-encoding` que não tem nenhuma é recusado em vez de ignorado
— veja a tabela de erros abaixo.

**E a codificação precisa nomear sua ordem de bytes.** `UTF-16` e `UTF-32` são
nomes que o ICU conhece, cujo conversor escreve **uma marca própria**, seja o que for
que se peça: `--no-bom` seria então aceito e desobedecido. Os dois são,
portanto, recusados, e a mensagem nomeia a saída:

```console
$ subedit-cli convert --to srt --to-encoding UTF-16 --output copie.srt film.srt
"UTF-16" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do
```

`UTF-16LE`, `UTF-16BE`, `UTF-32LE` e `UTF-32BE` gravam exatamente os mesmos
bytes, marca incluída — mas sob o controle de `--bom` e `--no-bom`. A
recusa não custa, portanto, nada além de um nome mais preciso, e vale tanto para
a leitura quanto para a escrita: `--encoding UTF-16` recebe a mesma resposta.

## O destino

As três opções e suas regras são comuns aos subcomandos que
gravam: veja [Invocação](invocation.md#o-destino). Nada é gravado
sem uma delas.

O que é específico de `convert`: **`--output-dir` muda a extensão conforme o
formato.** `convert --to vtt` sobre `a.srt` grava `a.vtt`, nunca um `a.srt` contendo
WebVTT — um arquivo cujo nome mente atrapalha todas as outras ferramentas.

## Converter no próprio arquivo é recusado

`--in-place` com um `--to` que muda o formato resulta em código `1` e não grava
nada: no próprio arquivo, não há um segundo nome para carregar o novo formato, e
o arquivo ficaria com uma extensão que seu conteúdo não justifica mais.

`--in-place` ainda pode ser usado para mudar **a forma sem o formato** —
gravar um `.srt` de volta como `.srt` com outros finais de linha, por exemplo.

A recusa é decidida **só pela extensão**, antes de qualquer leitura: um erro
de uso nunca deve deixar um lote gravado pela metade.

## Finais de linha e marca de ordem de bytes

Por padrão, **o arquivo gravado retoma o que o arquivo lido trazia**. O modelo
guarda ambos na leitura; impô-los por padrão perderia, a cada
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

`--sort` grava o arquivo **na ordem dos inícios**, seja qual for o formato de destino. A ordenação é feita **sobre uma cópia**: o arquivo lido não
é tocado, e o que é gravado é o que [`sort`](sort.md) teria feito dele — de forma estável, duas legendas que começam juntas mantêm sua ordem.

**Nunca por padrão**: a ordem de um arquivo é a de seu autor enquanto nada mais for pedido. Com a opção, cada arquivo informa isso, no
primeiro nível de narração, `<path>: N subtitles moved` ou `<path>: already in order`, e o registro de `--format json` traz uma
contagem `moved` — **ausente sem a opção**, para não mudar a forma do que os scripts existentes leem.

## Tornar os intervalos disjuntos

`--disjoint` corta as legendas que se sobrepõem para que nenhum intervalo se sobreponha mais a outro, **sobre a mesma cópia da ordenação**: o arquivo lido não é tocado.

Nunca por padrão. A opção faz o que [`disjoint`](disjoint.md) faz, **com seus valores padrão**, antes de gravar, depois da ordenação. Ela exige um arquivo em ordem: caso contrário é recusada, e `--sort` — que vem primeiro — o coloca em ordem antes. O arquivo informa isso no primeiro nível de narração (`<path>: N subtitles split, M merged`), e o registro de `--format json` traz as contagens `split` e `merged`, **ausentes sem a opção**.

**`--join-repeats` vem antes da ordenação e do corte**: um texto cortado em pedaços que se sucedem é reconstruído primeiro (veja
[`join-repeats`](join-repeats.md)), depois as legendas são ordenadas e, por fim, os intervalos são tornados disjuntos. **`--repair` é os três de uma vez**,
como [`repair`](repair.md), com os valores padrão do corte. Com `--join-repeats`, cada arquivo informa
`<path>: N repeated subtitles joined` no primeiro nível de narração, e o registro de `--format json` traz `counts.joined` — **ausente sem a opção**.

## Saída

**Saída padrão** — nada: o resultado é o arquivo gravado. Com `--dry-run`, também nada: não há arquivo, e a linha de narração diz isso — veja [Ver antes de escrever](invocation.md#ver-antes-de-escrever).

No erro padrão, conforme o nível:

| Nível | O que se acrescenta |
| :----- | :-------------- |
| 1 | `<path>: N subtitles written as WebVTT -> <destination>` |
| 2 | `<path>: SubRip -> WebVTT, UTF-8, no BOM, LF line endings` — a codificação nomeada é a que foi gravada |
| 3 | `<path>: N bytes read, M written`, depois **todos os diagnósticos de leitura** — veja [Invocação](invocation.md#diagnósticos-de-leitura) |

### Em JSON

Com [`--format json`](invocation.md#uma-saída-que-um-script-pode-ler), o objeto de cada arquivo traz `counts.subtitles` e depois o que a conversão perdeu: `lost_ends` e `lost_header` (`0` ou `1`), `joined_lines`, `lost_tags`, `lost_fields` e `furthest_ms` — os itens da [frase de perda](#o-que-a-conversão-declara-ter-perdido). A tabela completa é a de [Invocação](invocation.md#o-que-cada-subcomando-coloca-em-counts).

## Códigos de saída

Os da ferramenta: `0` se todos os arquivos foram gravados, `2` se nenhum, `3` se
apenas alguns, `1` em caso de erro de uso.

## Erros

| O que o provoca | Mensagem |
| :------------------ | :------ |
| `--bom` com `--no-bom` | `--bom and --no-bom ask for opposite things; give one or the other` |
| `--in-place` que muda o formato | `--in-place cannot change the format: the file would keep a name its content no longer matches` |
| `--bom` em uma codificação que não tem nenhuma | `<path>: <encoding> has no byte order mark to write` |
| `--to-encoding` nomeando uma codificação desconhecida | `no encoding is named "<name>"` |
| `--to-encoding` nomeando uma codificação que escreve a própria marca | `"<name>" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do` |
| caractere ausente da codificação gravada | `<path>: holds a character the chosen encoding cannot write` |

**Recusar `--bom` em uma codificação que não tem nenhuma não é
preciosismo.** Uma marca de ordem de bytes existe para as codificações Unicode e
para nenhuma outra: pedir uma em um arquivo em Windows-1252 é pedir
algo que não existe. O arquivo seria gravado sem ela, e ninguém
saberia que o pedido não foi atendido.

Os do destino são comuns aos subcomandos que gravam:
veja [Invocação](invocation.md#o-destino).

Os erros de leitura são os de [`inspect`](inspect.md): dizem respeito
ao arquivo de entrada e têm as mesmas mensagens.
