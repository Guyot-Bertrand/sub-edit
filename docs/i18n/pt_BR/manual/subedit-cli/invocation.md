# Invocação

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revisada por um leitor do idioma.

```
subedit-cli [global options] <subcommand> [options] <file>...
```

Sem um subcomando, a ferramenta escreve sua ajuda e para.

<!-- example: subedit-cli -->
```console
$ subedit-cli
Read, inspect and retime subtitle files.
Usage: subedit-cli [OPTIONS] [SUBCOMMAND]

Options:
  -h,--help                   Print this help message and exit
  --version                   Display program version information and exit
  -v                          Say more: -v is the default, -vv details, -vvv debugs
  -q,--quiet                  Say nothing but errors
  --encoding NAME             Encoding to read the files in; detected by default
  --format text|json          Form of the result on standard output: text or json
  --config FILE               Read the operation settings from FILE instead of the user's settings file
  --no-config Excludes: --config
                              Ignore any settings file: use the built-in defaults
  --list-encodings            Write the encodings that can be read and written, one a line, and stop

Subcommands:
  inspect                     Report what a subtitle file is made of
  convert                     Write a subtitle file out in another format or shape
  shift                       Move every position of a file by a fixed amount
  transform                   Correct every position from two points known to be right
  framerate                   Re-time a file mastered at one frame rate for another
  snap                        Move every position onto the nearest frame of a frame rate (see framerate)
  hearing-impaired            Remove the sounds described between square brackets
  adjust                      Bring the duration of every subtitle within a reading speed and limits
  replace                     Replace a text in the subtitles, without breaking a tag
  case                        Put the texts in title, sentence, upper or lower case, tags intact
  italics                     Put the texts in italics, or take their italics out
  dialogue-dashes             Put dialogue dashes on the lines, or take them off
  sort                        Put the subtitles in the order of their start, keeping ties as they are
  join-repeats                Join the subtitles that repeat each other, one after the other, into one
  disjoint                    Cut overlapping subtitles so that no range overlaps another, merging what overlaps
  repair                      Join the repeated subtitles, put them in order, then make the ranges disjoint
  correct                     Correct the texts with the patterns of the correction assistant
  append                      Put subtitle files one after another into a single file
  split-file                  Cut a subtitle file in two, a head and a tail
  pair                        Write a translation at the positions of its main file, and say how it lined up
  config                      List, show and change the operation settings
```

## Idioma

A linha de comando fala o idioma do seu ambiente: **sua ajuda, sua narração e seus
erros** seguem o idioma pedido. Os exemplos deste manual são executados em inglês — os
blocos são gerados por `make manual`, nunca copiados à mão.

**Como o idioma é escolhido.** O primeiro entre `LC_ALL`, `LC_MESSAGES` e `LANG` que estiver definido e
não vazio é a localidade; `LANGUAGE`, quando definido, lista então os idiomas desejados, separados por
dois-pontos (`LANGUAGE=pt_BR:fr`), e é lido primeiro. Esta é a ordem que o gettext segue, lida pelo
próprio programa: a localidade não precisa ter sido gerada na máquina.

| Regra | Efeito |
| :--- | :----- |
| codificação e modificador | ignorados: `fr_FR.UTF-8@euro` procura `fr_FR`, depois `fr` |
| `LC_ALL=C`, localidade `C` ou `POSIX`, ou nenhuma localidade | inglês, e `LANGUAGE` não é lido |
| uma entrada que nomeia o inglês (`en`, `en_GB`) | inglês: o que a segue em `LANGUAGE` não é lido |
| um idioma cujo catálogo não está instalado | o seguinte pedido, e o inglês depois do último, **sem uma palavra** |
| uma mensagem que o catálogo não traduz | exibida em inglês |

**O nome de um padrão de correção** que `correct` diz não ter podido aplicar também segue o
idioma, quando o catálogo o conhece — os padrões distribuídos com o programa; um padrão seu é
nomeado como seu arquivo o escreve. Os próprios arquivos de padrões nunca são reescritos.

Os catálogos são instalados em `<prefix>/share/subedit/locale/<language>/LC_MESSAGES/subedit.mo`
e encontrados a partir do próprio diretório do executável. O inglês é integrado. Esta versão instala `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR`, `ru`, `sr`, `tr` e `zh_CN` — catalão,
tcheco, alemão, espanhol, finlandês, francês, galego, hebraico, húngaro, interlingue, islandês, italiano, holandês, polonês, português, português do Brasil, russo, sérvio, turco e chinês simplificado — traduzidos por completo. O francês foi revisado
por um leitor do idioma; o catalão, o tcheco, o alemão, o espanhol, o finlandês, o galego, o hebraico, o húngaro, o interlingue, o islandês, o italiano, o holandês, o polonês, o português, o português do Brasil, o russo, o sérvio, o turco e o chinês simplificado são
traduzidos automaticamente e ainda não foram revisados por um leitor do idioma.
<!-- machine-translated: ca -->
<!-- machine-translated: cs -->
<!-- machine-translated: de -->
<!-- machine-translated: es -->
<!-- machine-translated: fi -->
<!-- machine-translated: gl -->
<!-- machine-translated: he -->
<!-- machine-translated: hu -->
<!-- machine-translated: ie -->
<!-- machine-translated: is -->
<!-- machine-translated: it_IT -->
<!-- machine-translated: nl -->
<!-- machine-translated: pl -->
<!-- machine-translated: pt -->
<!-- machine-translated: pt_BR -->
<!-- machine-translated: ru -->
<!-- machine-translated: sr -->
<!-- machine-translated: tr -->
<!-- machine-translated: zh_CN -->
Com `LANG=fr_FR.UTF-8`, `subedit-cli --help` começa por "Lire,
inspecter et recaler des fichiers de sous-titres." Um script que lê as palavras da saída
em texto define `LANGUAGE=en` (ou `LC_ALL=C`) — ou lê `--format json`, que nunca muda.

**O que nunca segue o idioma** — o que um script lê:

| Nunca traduzido | Por quê |
| :--------------- | :-- |
| a saída de `--format json`: chaves, valores, e o `message` e o `detail` das falhas e dos avisos | os mesmos bytes em todos os idiomas; as palavras são em inglês |
| os códigos de saída | os quatro de [Códigos de saída](#códigos-de-saída), em todos os idiomas |
| os nomes digitados: subcomandos, opções, seus valores (`off`, `srt`), metavariáveis (`FILE`, `TEXT`, `OPTIONS`) | uma linha de comando escrita uma vez funciona em todos os idiomas |
| os nomes dos padrões de correção que `correct --enable` e `--disable` recebem | um padrão é chamado pelo nome em inglês que seu arquivo contém |
| os nomes dos formatos e das codificações | um formato é chamado pelo seu nome |
| os diagnósticos próprios do CLI11 — um argumento obrigatório ausente, uma opção desconhecida, um valor que uma opção não aceita — e seu `Run with --help for more information.` | redigidos dentro do analisador sobre o qual a linha de comando é construída; permanecem em inglês |

Pedir o inglês, qualquer que seja o ambiente:

<!-- example: LC_ALL=C LANGUAGE=fr subedit-cli shift --output-dir out a.srt; echo "code=$?" -->
```console
$ LC_ALL=C LANGUAGE=fr subedit-cli shift --output-dir out a.srt; echo "code=$?"
shift needs --by, or --to-grid to work the amount out from the positions
code=1
```

## Opções globais

**Elas são escritas antes do nome do subcomando, e somente ali.**
`subedit-cli --encoding cp1252 inspect film.srt` é a forma correta;
`subedit-cli inspect --encoding cp1252 film.srt` é recusada, com
`The following argument was not expected: --encoding`. O mesmo vale para
`--version`, `-v` e `--quiet`.

**`--help` é a exceção, e não é realmente uma**:
`subedit-cli inspect --help` é aceito porque cada subcomando traz a sua, e é a
ajuda do subcomando que ele escreve, não a da ferramenta.

| Opção | Efeito |
| :----- | :---- |
| `-h`, `--help` | escreve a ajuda e para; a do subcomando, se um for nomeado |
| `--version` | escreve `subedit <version>`, as bibliotecas com as quais a ferramenta é executada e os idiomas instalados, e para — veja [`--version`](#--version) |
| `-q`, `--quiet` | nível 0 — nenhuma narração |
| `-v`, `-vv`, `-vvv` | níveis 1 a 3; o nível 1 é o padrão |
| `--encoding NAME` | lê os arquivos nessa codificação, em vez de adivinhá-la |
| `--list-encodings` | escreve as codificações que a ferramenta sabe ler e escrever, uma por linha, e **para** (código `0`) — veja [`--list-encodings`](#--list-encodings) |
| `--format text\|json` | a forma do resultado na saída padrão: o texto, ou um objeto JSON por arquivo — veja [Uma saída que um script pode ler](#uma-saída-que-um-script-pode-ler) |
| `--config FILE` | lê as configurações de operação de `FILE` em vez do arquivo de configurações do usuário — veja [O arquivo de configurações](#o-arquivo-de-configurações) |
| `--no-config` | não lê nenhum arquivo de configurações: toda configuração de operação não indicada na linha assume seu padrão embutido — veja [O arquivo de configurações](#o-arquivo-de-configurações) |

`--quiet` e `-v` na mesma invocação são recusados: duas intenções opostas
não são resolvidas em favor da última escrita.

`--config` e `--no-config` na mesma invocação também são recusados, com
`--config excludes --no-config`, e o mesmo vale para `--config` com um nome vazio, com
`--config needs a file name`: um nome vazio não designa nenhum arquivo, e o arquivo do próprio usuário não é lido
em seu lugar.

### O arquivo de configurações

**[`adjust`](adjust.md) e [`correct`](correct.md) leem por padrão as configurações de operação do
arquivo de configurações** — os valores que `Adjust Durations…` e `Correct Texts…` lembram na
janela, ou que [`config set`](config.md) gravou. Nenhum outro subcomando o lê. O arquivo é:

```
$XDG_CONFIG_HOME/subedit/settings.conf
```

ou `~/.config/subedit/settings.conf` quando `XDG_CONFIG_HOME` não está definido, está vazio ou não é um caminho
absoluto — o arquivo da janela, descrito em [seu manual](../subedit-gui/preferences.md#o-arquivo).

**Prioridade, da mais forte à mais fraca: a opção na linha de comando, depois o arquivo, depois o padrão
embutido.** Uma opção sempre vence: `--speed off` desativa uma velocidade que o arquivo define. Somente o
**bloco de operação** do arquivo é lido; as preferências de exibição (geometria, colunas, tema…)
nunca o são. [`config list`](config.md#config-list) nomeia as chaves, e quais delas a linha de
comando lê: as nove chaves `duration-adjust.*`, `correction.human`, `correction.ocr`,
`correction.remove-blank` e `correction.line-break.max-lines`. **Cada uma delas tem uma opção
que a substitui por uma invocação**, seja qual for a maneira como o arquivo a define — as tabelas de
[`adjust`](adjust.md#o-que-o-arquivo-de-configurações-fornece) e
[`correct`](correct.md#o-que-é-escolhido-e-o-que-não-é) as nomeiam.

**Ler o arquivo do usuário nunca faz um comando falhar.** Um arquivo ausente dá os padrões; um arquivo
que não pode ser lido dá os padrões e diz isso; um valor que não pode ser lido mantém seu padrão e é nomeado; uma
chave desconhecida é ignorada. O arquivo nunca é gravado por `adjust` ou `correct`.

**O que foi lido é dito**, no nível 1 padrão, na saída de erro, antes do primeiro arquivo:

| Caso | Linha |
| :--- | :--- |
| o arquivo foi lido | `settings: read <path>` |
| um valor nele não pôde ser lido | `settings: <key>: cannot read '<value>', default kept` |
| o arquivo do usuário não existe | `settings: <path> not found, defaults used` |
| o arquivo não pode ser lido | `settings: <path> unreadable, defaults used` |
| `--no-config` | nada |

Com [`--format json`](#uma-saída-que-um-script-pode-ler), o objeto de cada arquivo de `adjust` e
`correct` traz `settings`: o caminho que foi lido, ou `null` quando nenhum foi.


**`--no-config` torna uma execução reproduzível**: a mesma linha dá o mesmo resultado em todas as
máquinas, seja o que for que seu usuário definiu na janela. Use-o em scripts, testes e relatos de erro — ou
`--config FILE` com um arquivo que os acompanhe.

**`--config FILE` nomeia outro arquivo**, lido da mesma maneira — **exceto que ele deve existir**. Um
nome dado na linha de comando não é um pedido pelos padrões: quando `FILE` não existe,
`adjust` e `correct` param antes de qualquer entrada ser lida, escrevem `settings file not found: <FILE>` na
saída de erro e terminam com o código de uso `1`. Um `FILE` que existe e não pode ser lido
dá os padrões e diz isso, como o arquivo do usuário. [`config show`](config.md#config-show)
ainda mostra um `FILE` ausente como `(not found)`, e [`config set`](config.md#config-set-e-config-unset) ainda
o cria.

<!-- example: printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine.conf adjust --dry-run a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine.conf adjust --dry-run a.srt; echo "code=$?"
settings file not found: mine.conf
code=1
```

### `--version`

**A versão do programa, sozinha na primeira linha** — `subedit <version>`, o que a saída era até
0.15.38 e o que um script lê. Depois **uma linha por biblioteca com a qual a ferramenta é executada**, depois os idiomas
de interface instalados, na saída padrão; a ferramenta para, com o código `0`. Um subcomando indicado junto não
é executado, e `-q` e `-v` não mudam nada.

<!-- example: subedit-cli --version | head -n 1 -->
```console
$ subedit-cli --version | head -n 1
subedit 1.0.7
```

O conjunto completo, em uma máquina com ICU 74.2 e Enchant 2.3.3 — os números são os da máquina:

```console
$ subedit-cli --version
subedit <version>
  ICU      74.2 (built with 74.2)
  Enchant  2.3.3 (built with 2.3.3)
  CLI11    2.4.1 (built with 2.4.1)
  Languages: en, fr
```

| Biblioteca | Para que a ferramenta a usa |
| :------ | :------------------------ |
| `ICU` | ler e escrever as codificações, detectá-las, mudar a caixa das letras |
| `Enchant` | o corretor ortográfico de `correct` |
| `CLI11` | ler esta linha de comando |

- **Cada linha dá duas versões**: a encontrada em tempo de execução, primeiro, e a com a qual o programa foi
  compilado, entre parênteses. Elas diferem quando a biblioteca da máquina foi atualizada depois que o
  programa foi compilado — a primeira informação a dar em um relato de erro.
- **Os cabeçalhos do Enchant não trazem versão**: a que está entre parênteses é a que o `pkg-config` informou
  quando a compilação foi configurada.
- **O CLI11 é compilado dentro do programa**, seus cabeçalhos sendo tudo o que há dele: suas duas versões são
  sempre iguais.
- **Nem Qt nem libmpv**: a linha de comando não se liga a nenhuma das duas, e lista apenas aquilo com que é executada.
  A janela as lista — veja [seu manual](../subedit-gui/invocation.md#--version).
- **`Languages`** lista `en`, o idioma de origem, que dispensa catálogo, depois cada catálogo
  instalado em `<prefix>/share/subedit/locale` que seja legível, por código — os idiomas que
  `Editar ▸ Preferências…` oferece.
- **O que segue o idioma**: `built with` e `Languages`. A primeira linha, os nomes, os
  números e os códigos de idioma nunca o seguem.
- **Com `--format json`**, antes ou depois de `--version`, **um objeto em uma linha**: `schema`
  (`1`), `command` (`"version"`), `version`, `libraries` — uma matriz de
  `{"name":"ICU","loaded":"74.2","built":"74.2"}`, na ordem do texto — e `languages`, uma
  matriz de códigos. Nada nele é traduzido.

<!-- example: subedit-cli --version --format json | grep -o '"name":"[^"]*"' -->
```console
$ subedit-cli --version --format json | grep -o '"name":"[^"]*"'
"name":"ICU"
"name":"Enchant"
"name":"CLI11"
```

### `--list-encodings`

**As codificações que a ferramenta sabe ler e escrever, uma por linha, ordenadas por nome**, na saída padrão —
depois a ferramenta para, com o código `0`, como `--version` faz: é a resposta, não uma etapa. Um
subcomando indicado junto não é executado.

<!-- example: subedit-cli --list-encodings | grep '^UTF' | head -4 -->
```console
$ subedit-cli --list-encodings | grep '^UTF' | head -4
UTF-16BE
UTF-16LE
UTF-32BE
UTF-32LE
```

- **São os nomes que `--encoding` aceita**, os canônicos; seus apelidos (`cp1252`) também são aceitos,
  sem serem listados.
- **A contagem depende do ICU instalado**, e nenhum nome é prometido além dos que todo ICU tem
  (`UTF-8`, `UTF-16LE`, `windows-1252`…). O manual, portanto, não dá nenhuma contagem.
- **`UTF-16` e `UTF-32` não estão nela**: o ICU os conhece, mas seu conversor escreve uma marca própria,
  e o modelo os recusa — veja abaixo. Uma lista para escolher não oferece o que o campo seguinte
  recusaria.
- **Com `--format json`**, **um objeto por codificação**:
  `{"schema":1,"command":"list-encodings","name":"UTF-8"}`. Não é o relatório de um arquivo: esses
  objetos **não trazem nem `file` nem `ok`**, a única exceção ao envelope de
  [a forma](#a-forma). Ainda assim são um objeto por linha, um por entrada.
- **O nível de narração não muda nada**: uma lista é um resultado, e `-q` não a suprime.

### `--encoding`, e o que ela não pode forçar

**Ela se aplica a todos os subcomandos**, porque todos leem: um arquivo cuja
codificação foi adivinhada errado continua errado seja o que for que se faça com ele, e uma opção
que somente `inspect` tivesse deixaria um desacordo sem remédio.

**O nome é o do ICU, e todos os seus apelidos são aceitos**: `cp1252`,
`windows-1252` e `WINDOWS 1252` designam a mesma codificação. O conjunto não está escrito
em lugar nenhum: é o que o ICU da máquina sabe converter, e
[`--list-encodings`](#--list-encodings) o lista. Um nome que ele não sabe converter é
recusado antes que um único arquivo seja lido:

```console
$ subedit-cli --encoding klingon-1 inspect film.srt
no encoding is named "klingon-1"
```

**Uma exceção, e somente uma: a codificação precisa nomear sua ordem de bytes.**
`UTF-16` e `UTF-32` são nomes que o ICU conhece e cujo conversor escreve **uma marca
própria**. Tal codificação não pode viver neste modelo — a marca deixa
de ser uma escolha, `--no-bom` seria aceita e desobedecida — e por isso as duas são
recusadas, para leitura como para escrita:

```console
$ subedit-cli --encoding UTF-16 inspect film.srt
"UTF-16" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do
```

`UTF-16LE` e `UTF-16BE` leem e escrevem os mesmos bytes, marca incluída; o que
o nome preciso acrescenta é que se sabe qual.

**Uma marca de ordem de bytes vence a opção.** É a única coisa que um arquivo de legendas
declara sobre si mesmo, e lê-lo de outra maneira que não a que ele declara
seria obedecer ao chamador contra o arquivo. O desacordo é **dito**, como um
diagnóstico:

```console
$ subedit-cli -vvv --encoding cp1252 inspect avec-bom.srt
avec-bom.srt: a byte order mark that contradicts the encoding asked for ("UTF-16LE"), settled by the reader
```

É o que o Gaupol faz — ele recomeça a leitura com a codificação da
marca — mas sem dizê-lo; aqui a diferença entre o que foi pedido e o que
foi feito não é um silêncio.

## Subcomandos

| Subcomando | O que faz | Grava? |
| :------------ | :-------------- | :----------- |
| [`inspect`](inspect.md) | relata do que um arquivo é feito, sem alterar nada | não |
| [`convert`](convert.md) | grava um arquivo em outro formato, ou em outra forma | sim |
| [`shift`](shift.md) | desloca todas as posições na mesma quantidade | sim |
| [`transform`](transform.md) | corrige todas as posições a partir de dois pontos sabidamente corretos | sim |
| [`framerate`](framerate.md) | ressincroniza um arquivo de uma taxa de quadros para outra | sim |
| [`snap`](snap.md) | move cada marca de tempo para o quadro mais próximo | sim |
| [`hearing-impaired`](hearing-impaired.md) | remove as menções para deficientes auditivos | sim |
| [`adjust`](adjust.md) | ajusta as durações a uma velocidade de leitura e a limites | sim |
| [`replace`](replace.md) | substitui um texto nas legendas, sem quebrar uma marcação | sim |
| [`case`](case.md) | põe os textos em iniciais maiúsculas, primeira letra maiúscula, maiúsculas ou minúsculas | sim |
| [`italics`](italics.md) | põe os textos em itálico, ou tira o itálico deles | sim |
| [`dialogue-dashes`](dialogue-dashes.md) | põe travessões de diálogo, ou os tira | sim |
| [`sort`](sort.md) | recoloca as legendas na ordem de seu início | sim |
| [`join-repeats`](join-repeats.md) | faz uma única legenda dos pedaços de um mesmo texto que se seguem | sim |
| [`disjoint`](disjoint.md) | corta as legendas que se sobrepõem para que nenhum intervalo se sobreponha mais a outro | sim |
| [`repair`](repair.md) | encadeia `join-repeats`, `sort` e `disjoint` | sim |
| [`correct`](correct.md) | corrige os textos com os padrões do Gaupol: menções, união e divisão de palavras, erros comuns, uso de maiúsculas, quebra de linha | sim |
| [`append`](append.md) | coloca arquivos depois do primeiro, em **um único** arquivo | sim |
| [`split-file`](split-file.md) | corta um arquivo em **dois**: uma cabeça e uma cauda | sim |
| [`pair`](pair.md) | grava uma tradução nas posições de seu arquivo principal, e diz como suas linhas se alinharam | sim |
| [`config`](config.md) | lista, mostra e altera as configurações de operação do arquivo de configurações | somente o arquivo de configurações |

Todos os vinte e um estão ali; a ajuda da ferramenta os lista na mesma ordem.

**Apenas um não toca em nada**, e é a coluna da direita: `inspect` lê e
relata. Os outros dezenove gravam, e o que se segue vale para dezessete deles:
[`append`](append.md) e [`split-file`](split-file.md) têm cada um uma aridade própria — N arquivos para
**um** resultado, um arquivo para **dois** — e portanto seu próprio destino, que descrevem em sua página.
[`config`](config.md) não grava nenhum arquivo de legendas: ele lê e altera o arquivo de configurações, e
nada do que se segue se aplica a ele.

## O destino

Os dezessete subcomandos que gravam — [`convert`](convert.md),
[`shift`](shift.md), [`transform`](transform.md), [`framerate`](framerate.md),
[`snap`](snap.md), [`hearing-impaired`](hearing-impaired.md), [`adjust`](adjust.md),
[`replace`](replace.md), [`case`](case.md), [`italics`](italics.md),
[`dialogue-dashes`](dialogue-dashes.md), [`sort`](sort.md), [`join-repeats`](join-repeats.md),
[`disjoint`](disjoint.md), [`repair`](repair.md), [`correct`](correct.md), [`pair`](pair.md) — recebem seu destino da mesma maneira. ([`append`](append.md) e
[`split-file`](split-file.md) têm o seu, em sua página.) [`inspect`](inspect.md) não grava nenhum arquivo e
não aceita nenhuma destas opções.

**Um destino que já existe é sobrescrito**, sem pergunta e sem opção para
evitá-lo: a gravação é atômica — o arquivo nunca fica pela metade, e uma
gravação interrompida deixa o antigo intacto —, mas o conteúdo anterior é
perdido. Não é um defeito mantido por descuido: é a regra, e executar um comando
de novo sobre o mesmo diretório de saída o executa de novo sobre si mesmo.

**Nada é gravado sem um destino explícito.** As três maneiras de
indicá-lo se excluem mutuamente, e uma é necessária: sem ela, nada é gravado e o código de
saída é `1` — exceto com [`--dry-run`](#ver-antes-de-escrever), que não grava nada
e portanto não exige nenhuma.

| Opção | Para onde vai o resultado |
| :----- | :---------------- |
| `--output FILE` | para este arquivo, com exatamente este nome; uma única entrada |
| `--output-dir DIR` | para este diretório, com o nome base da entrada |
| `--in-place` | sobre a entrada, por gravação atômica |

Uma ferramenta que sobrescreve sua entrada porque não lhe disseram nada é uma ferramenta que se
deixa de usar na segunda vez. A recusa custa uma linha e salva o
arquivo.

`--output` com várias entradas é recusado: o último arquivo seria gravado sobre os
anteriores. Com um lote, `--output-dir` é a única das três que faz
sentido, junto com `--in-place`.

**O diretório de saída é criado** se estiver ausente, pais incluídos — para
`--output-dir`, e para o diretório em que `--output` grava. Isso é feito depois que a
linha de comando é validada e **antes do primeiro arquivo**. Se não puder ser criado,
isso é dito uma vez, nenhum arquivo é lido, e o código de saída é
`2`.

**O lote é julgado como um todo antes de gravar.** O destino de cada entrada
é calculado primeiro, e duas recusas decorrem disso — erros de uso, código
`1`, **nenhum arquivo gravado** e nenhum diretório criado:

- **duas entradas que acabariam no mesmo arquivo** — `a/film.srt` e
  `b/film.srt` com `--output-dir`, ou `a/film.srt` e `b/film.vtt` ambos
  convertidos para WebVTT, cujo destino final mudou de extensão. A
  mensagem nomeia o destino e as duas entradas; renomear a segunda ou
  deixar a última vencer perderia um arquivo sem dizê-lo, e recusar
  custa uma linha;
- **um destino que é uma das entradas**, sem `--in-place` — inclusive
  por meio de `--output-dir .`, de `--output` ou de um link simbólico. Gravar sobre
  a própria entrada é um gesto que se nomeia.

Dois caminhos designam o mesmo arquivo quando o sistema o diz, não quando
são escritos de maneira parecida: `in/../in/film.srt` e `in/film.srt` são um único arquivo.

**A extensão segue o formato gravado.** Ela só muda para
[`convert`](convert.md), o único subcomando que muda de formato; os
outros mantêm o formato do arquivo lido, e portanto sua extensão.

| O que o provoca | Mensagem |
| :------------------ | :------ |
| nenhum destino, sem `--dry-run` | `no destination given: use --output, --output-dir or --in-place` |
| dois destinos | `--output, --output-dir and --in-place exclude one another` |
| `--output` em um lote | `--output names one file but several were given: use --output-dir instead` |
| duas entradas, um destino | `<destination>: would be written by both <input> and <input>` |
| uma entrada seria sobrescrita | `<destination>: would be written over the input <input>` — a mesma frase para [`append`](append.md) e [`split-file`](split-file.md) |
| diretório de saída que não pode ser criado | `<directory>: cannot be created: permission denied`, ou `: cannot be created` |
| destino que não pode ser gravado | `<path>: <destination>: cannot be written: permission denied`, ou `: cannot be written` |
| caractere ausente da codificação gravada | `<path>: holds a character the chosen encoding cannot write` |

**O último merece uma frase.** Um arquivo é gravado de volta na **codificação em que foi
lido**, como o é com seus finais de linha e sua marca. Se uma
alteração introduziu nele um caractere que essa codificação não conhece — um
`ł` em Latin-1 —, a gravação falha e **nada é gravado**. Substituí-lo
por um `?` seria perder texto diante dos olhos de quem acabou de escrevê-lo.

## Ver antes de escrever

**`--dry-run`** é aceito por cada um dos dezesseis subcomandos que gravam, e significa a mesma
coisa em todos: **ler, calcular, relatar, não gravar arquivo**. Acrescenta-se à
linha que se ia executar; aplicar é executar a mesma linha de novo sem ele. O cálculo é
determinista: o que um `--dry-run` mostrou é o que a execução seguinte gravará, desde que o
arquivo não tenha mudado.

- **Ele não grava arquivo nem cria diretório**, `--in-place` incluído: o arquivo de
  entrada continua como estava.
- **Ele não exige destino.** Pode-se indicar um, e **ele é então verificado como sem
  `--dry-run`**: duas entradas para o mesmo destino, uma entrada que seria sobrescrita,
  `--output` em um lote, duas maneiras de dizer para onde — uma linha de comando recusada sem `--dry-run`
  é recusada com ele, pela mesma frase e pelo mesmo código `1`.
- **O código de saída é o de uma execução real**: `0` quando tudo foi calculado, `2` ou `3`
  quando arquivos não puderam ser lidos ou processados, `1` para uso. Um caractere que
  a codificação do arquivo não saberia gravar faz uma simulação falhar como faz uma execução real falhar.
  **Que existam alterações não muda o código**: os quatro códigos dizem se a ferramenta
  teve êxito, não se o arquivo está conforme.
- **A narração o diz.** Onde uma execução real escreve `-> <destination>`, uma simulação
  escreve `(dry run, nothing written)`.

<!-- example: printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by 1 --dry-run a.srt; ls -- *.srt -->
```console
$ printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by 1 --dry-run a.srt; ls -- *.srt
a.srt: 1 subtitle shifted by 1.000 s (dry run, nothing written)
a.srt
```

**Em um subcomando que altera textos** —
[`hearing-impaired`](hearing-impaired.md), [`replace`](replace.md), [`case`](case.md),
[`italics`](italics.md), [`dialogue-dashes`](dialogue-dashes.md) e [`correct`](correct.md) —, **a saída padrão traz as
alterações propostas**. Um bloco por legenda alterada, na ordem do arquivo:

```
<path>: subtitle <number>
- <the text before, line by line>
+ <the text after, line by line>
```

- o **número** é o que a legenda tem no arquivo lido, contado a partir de um;
- cada linha do texto anterior traz `- ` na frente, cada linha do texto posterior `+ `: um texto
  de várias linhas é lido como é, e uma linha que ela mesma começa por um travessão de
  diálogo dá `- - `;
- **uma legenda que a operação removeria** diz isso em sua primeira linha,
  `<path>: subtitle <number> (removed)`, e não tem linhas `+`;
- uma legenda da tradução, com `--document translation`, anuncia-se `(translation)` no mesmo
  lugar — veja [Uma tradução](#uma-tradução);
- a primeira linha de cada bloco começa pelo caminho, de modo que `grep '^film.srt: '`
  encontra os blocos de um arquivo em um lote;
- **nenhum bloco** quando nada mudaria.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\n- [Grincement]\n- Qui est là ?\n\n' > mentions.srt; subedit-cli hearing-impaired --dry-run mentions.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\n- [Grincement]\n- Qui est là ?\n\n' > mentions.srt; subedit-cli hearing-impaired --dry-run mentions.srt
mentions.srt: 2 subtitles cleaned, 1 removed (dry run, nothing written)
mentions.srt: subtitle 1 (removed)
- [Bruit de pas]
mentions.srt: subtitle 2
- Attends [il tousse] Marie.
+ Attends Marie.
mentions.srt: subtitle 3
- - [Grincement]
- - Qui est là ?
+ Qui est là ?
```

**A lista é a mesma em JSON**: o objeto do arquivo traz uma matriz `changes`, veja
[a forma](#a-forma), com `after` definido como `null` para uma legenda removida. **Ela só é
calculada quando `--dry-run` ou `--format json` a pede**: um lote de milhares de
legendas não a constrói à toa.

Nos **outros subcomandos** — [`convert`](convert.md), [`shift`](shift.md),
[`transform`](transform.md), [`framerate`](framerate.md), [`snap`](snap.md),
[`adjust`](adjust.md), [`sort`](sort.md), [`append`](append.md), [`split-file`](split-file.md), [`pair`](pair.md) —, a saída
padrão traz o que a de uma execução real traria, isto é, nada em texto; em
JSON, o objeto de cada arquivo com `"dry_run":true`, `"destination":null` e as mesmas
`counts`.

## Duas saídas, dois papéis

| Saída | O que traz |
| :----- | :--------------- |
| padrão | **o resultado, e somente o resultado** — o relatório de `inspect`, as alterações propostas de um texto [`--dry-run`](#ver-antes-de-escrever); com `--format json`, um objeto por arquivo — um por execução para [`append`](append.md), [`split-file`](split-file.md) e [`pair`](pair.md) |
| erro | **todo o resto** — a narração, os avisos, os erros |

É essa divisão que permite redirecionar o resultado sem levar junto a história:

```console
$ subedit-cli inspect *.srt > rapport.txt
```

O relatório vai para o arquivo, a narração continua na tela.

## Uma saída que um script pode ler

**`--format json`** substitui o texto da saída padrão por **um objeto JSON por
arquivo de entrada**, no formato JSON Lines: um objeto compacto por linha, em UTF-8 sem
marca, e o final de linha é `\n`. `text` é o padrão e não muda. É uma
opção global, e portanto é escrita antes do subcomando; um valor diferente de
`text` e `json` é um erro de uso, código `1`.

<!-- example: printf '1\n00:00:10,000 --> 00:00:12,000\nUn.\n\n' > a.srt; subedit-cli --format json shift --by 1 --output-dir sortie a.srt absent.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:10,000 --> 00:00:12,000\nUn.\n\n' > a.srt; subedit-cli --format json shift --by 1 --output-dir sortie a.srt absent.srt; echo "code=$?"
a.srt: 1 subtitle shifted by 1.000 s -> sortie/a.srt
{"schema":1,"command":"shift","file":"a.srt","ok":true,"dry_run":false,"destination":"sortie/a.srt","counts":{"subtitles":1,"shifted_by_ms":1000},"warnings":[]}
absent.srt: does not exist
{"schema":1,"command":"shift","file":"absent.srt","ok":false,"error":{"kind":"not-found","message":"absent.srt: does not exist"}}
1 of 2 files shifted, 1 failed
code=3
```

**Um objeto por entrada, nunca zero, nunca dois**, quer a entrada tenha tido êxito ou não, na
ordem em que as entradas foram dadas — ou encontradas, com [`--recursive`](lots.md). Conta-se
as linhas, encontra-se os arquivos. **Três subcomandos são a exceção, porque sua
aridade não é a de um lote: [`append`](append.md) (N entradas, uma saída), [`split-file`](split-file.md)
(uma entrada, duas saídas) e [`pair`](pair.md) (dois arquivos, uma saída) escrevem **um objeto para toda a
execução**, que nomeia o arquivo que sua página indica.** **Os subcomandos que reescrevem, portanto, também escrevem
na saída padrão**, o que seu contrato não fazia em texto: o
arquivo continua sendo seu resultado, e o objeto é o relatório dele.

**A narração não muda.** `-q`, `-v`, `-vv` e `-vvv` agem sobre a saída de
erro, em texto, como sem `--format`; **a saída padrão não varia com eles**.
Os diagnósticos de leitura, que o texto mantém no nível 3, são dados e estão
**sempre** em `warnings`. Um erro de uso não escreve nenhum objeto: ele é detectado
antes de qualquer arquivo ser tocado, de modo que não há a que anexá-lo; ele vai para a saída
de erro com o código `1`, e a saída padrão continua vazia.

### A forma

| Chave | Conteúdo | Presente |
| :-- | :------ | :------- |
| `schema` | inteiro, a versão desta forma — `1` | sempre |
| `command` | o nome do subcomando | sempre |
| `file` | o caminho **como dado**, ou como a varredura de um diretório o compôs | sempre |
| `ok` | booleano | sempre |
| `error` | `{"kind", "message"}` | se `ok` é falso |
| `dry_run` | booleano: verdadeiro para um [`--dry-run`](#ver-antes-de-escrever) | se `ok` é verdadeiro, em um subcomando que grava |
| `destination` | o caminho gravado, ou `null` quando `dry_run` é verdadeiro: nada foi gravado | idem |
| `counts` | um objeto de **inteiros**, específico do subcomando | idem |
| `changes` | uma matriz de `{"subtitle", "document", "before", "after"}`: os textos alterados, veja abaixo | em um subcomando que altera textos |
| `warnings` | uma matriz de `{"kind", "line"?, "detail"?, "settled"?}` | se `ok` é verdadeiro |
| `alignment` | como uma tradução foi colocada, veja [Uma tradução](#uma-tradução) | com `-t`, e para `pair` |
| `constraints` | as configurações usadas | `adjust` |
| `settings` | o arquivo de configurações que foi lido, ou `null` quando nenhum foi — veja [O arquivo de configurações](#o-arquivo-de-configurações) | `adjust`, `correct`, se `ok` é verdadeiro |
| `inputs`, `tail` | um elemento por arquivo anexado; o caminho da cauda | `append`; `split-file` |

`line` está ausente de um diagnóstico sobre o arquivo inteiro; `detail` está quando não há
nada a acrescentar; `settled` é `true` quando a leitura **resolveu** algo — uma
numeração regenerada —, e está ausente quando ela deixou as coisas como estão.

**`changes`** lista, na ordem do arquivo, cada legenda cujo texto muda:
`subtitle` é seu número no arquivo lido, contado a partir de um; `document` é `"main"`
ou `"translation"`; `before` é o texto anterior; `after` o texto posterior, ou **`null` quando
a legenda é removida**. As quebras de linha de um texto são `\n` nele. Ela é
escrita para uma execução real como para um [`--dry-run`](#ver-antes-de-escrever), e **vazia
(`[]`) quando nada muda**: sua presença diz que o subcomando lista suas alterações,
sua ausência que ele não tem nenhuma.

**Nenhum número de ponto flutuante, em lugar algum.** Uma posição ou uma duração é um inteiro de
**milissegundos** (`…_ms`); uma contagem é um inteiro; uma taxa de quadros é uma **cadeia de caracteres**
(`"25"`, `"24000/1001"`) porque é exata, e um ponto flutuante não pode
representar um racional; uma concentração de grade é um inteiro em **milésimos** — `1000` é uma
grade perfeita. Duas saídas então se comparam byte a byte, sem arredondamento.

**Um caminho que não é UTF-8** — o Linux o permite — não pode ser representado em JSON: ele
é escrito com U+FFFD no lugar dos bytes inválidos, e o objeto traz o aviso
`path-not-utf8`. A fidelidade de tal caminho não é prometida, e isso é dito.

### O que cada subcomando coloca em `counts`

| Subcomando | Chaves de `counts` |
| :------------ | :--------------- |
| [`shift`](shift.md) | `subtitles`, `shifted_by_ms` (com sinal) |
| [`transform`](transform.md) | `subtitles` |
| [`framerate`](framerate.md) | `subtitles` |
| [`snap`](snap.md) | `subtitles`, `moved` (posições movidas), `furthest_ms` (o maior deslocamento) |
| [`hearing-impaired`](hearing-impaired.md) | `cleaned` (textos reescritos), `removed` (legendas removidas) |
| [`replace`](replace.md) | `replaced` (correspondências substituídas nas legendas alteradas), `matched` (correspondências encontradas) |
| [`case`](case.md), [`italics`](italics.md), [`dialogue-dashes`](dialogue-dashes.md) | `changed` (legendas cujo texto mudou) |
| [`correct`](correct.md) | `corrected` (textos alterados), `removed` (legendas removidas) — nunca correspondências |
| [`sort`](sort.md) | `subtitles`, `moved` (os lugares em que uma legenda mudou) |
| [`adjust`](adjust.md) | `subtitles` (as legendas visadas), `adjusted` (aquelas cujo fim se moveu), depois o grupo `sacrificed`: `speed`, `minimum`, `gap`; e, fora de `counts`, `constraints` — as configurações usadas |
| [`convert`](convert.md) | `subtitles`, depois o que a conversão perdeu: `lost_ends` e `lost_header` (0 ou 1), `joined_lines`, `lost_tags`, `lost_fields`, `furthest_ms` |
| [`append`](append.md) | `files`, `subtitles`, `appended`, depois `lost_ends`, `lost_header`, `joined_lines`, `lost_tags`, `lost_fields`, `furthest_ms` somados; e, fora de `counts`, `inputs` — um elemento por arquivo anexado |
| [`split-file`](split-file.md) | `subtitles`, `head`, `tail`; e, fora de `counts`, `tail` — o caminho da cauda (`destination` é a cabeça) |
| [`pair`](pair.md) | `subtitles` (as do arquivo gravado); e, fora de `counts`, `alignment` |
| [`inspect`](inspect.md) | nenhuma `counts`: a descrição do arquivo, veja sua página |

### Identificadores de erro

Um `error.kind` é um destes; **um leitor trata um identificador desconhecido como
"outro"**.

| `kind` | O que é |
| :----- | :----------- |
| `not-found`, `permission-denied`, `io` | o sistema recusou ler — ou gravar — o arquivo |
| `unknown-format`, `no-subtitle-found`, `undecodable` | o arquivo é lido e não se deixa ler |
| `unencodable` | um caractere que a codificação escolhida não sabe gravar |
| `no-byte-order-mark`, `no-frame-rate` | o que `convert` não sabe gravar sem que lhe digam |
| `before-the-origin`, `beyond-the-end`, `no-transform`, `no-grid` | uma operação que não pode se aplicar a este arquivo |
| `range-out-of-bounds` | um `--range` que este arquivo não contém |
| `no-style` | um formato que não escreve estilo (TMPlayer, LRC), para `italics` |
| `refused` | qualquer outra recusa de uma operação |

### O que é prometido, e o que não é

**Prometido enquanto `schema` for `1`**: os nomes das chaves, seu tipo e sua unidade; o significado
de `ok`, `file` e `destination`; **um objeto por entrada, na ordem das entradas** — um por execução para os três subcomandos de aridade própria;
os identificadores `kind` que existem; o determinismo — mesmas entradas, mesmos argumentos,
mesmos bytes.

**Não prometido**: o **texto das mensagens** (`message`, `detail`), inglês para um humano,
corrigido quando preciso — um script que o compara está quebrado de antemão; a **ordem das
chaves** em um objeto, fixa mas não garantida pelo JSON; a presença de **chaves ainda não
acrescentadas**.

**Um leitor, portanto, ignora uma chave desconhecida**, e trata um `kind` desconhecido como "outro":
acrescentar uma chave ou um identificador não muda `schema`. Remover ou renomear uma chave,
mudar um tipo ou uma unidade, remover um identificador o incrementa — isso é uma ruptura,
anunciada no CHANGELOG e aqui.

## Níveis de narração

Cada nível **contém o anterior**: subir um degrau acrescenta, nunca
substitui.

| Nível | Como | O que a saída de erro traz |
| :----- | :------ | :------------------------------ |
| 0 | `--quiet` | nada, **exceto os erros** |
| 1 | por padrão, ou `-v` | uma linha por arquivo processado, e um resumo assim que há vários |
| 2 | `-vv` | e o que foi reconhecido: formato, codificação, BOM, finais de linha |
| 3 | `-vvv` | e o rastro de depuração: bytes lidos e gravados, e **cada diagnóstico de leitura** |

**Os erros nunca são silenciados, `--quiet` incluído.** Um comando que falha
em silêncio não deixaria nada além de seu código de saída.

O resumo só aparece a partir de dois arquivos: em uma única entrada, ele
repetiria a linha que o precede.

## Diagnósticos de leitura

Os formatos de legendas são lidos **o melhor possível**: diante de uma anomalia, o
leitor não para — ele decide, ou deixa as coisas como estão, e diz isso. O que
encontrou é emitido no **nível 3**, uma linha por diagnóstico, na saída de
erro:

```
a.srt: 2 diagnostics while reading
a.srt: line 6: SubRip numbers that do not follow ("7"), settled by the reader
a.srt: line 9: a line that fits nowhere, left as it stands
```

Todos os dezessete subcomandos os relatam, não apenas [`inspect`](inspect.md):
um arquivo lido o melhor possível e depois gravado de volta passou pelas mesmas decisões, e mantê-las em silêncio
faria acreditar que nada aconteceu.

**Um diagnóstico nunca é uma falha.** O arquivo foi lido, o comando
teve êxito, o código de saída é `0`. É por isso que eles vivem no nível mais
detalhado: mais abaixo, enterrariam a linha que diz o que foi realmente
feito.

### O que cada linha traz

| Parte | O que diz |
| :----- | :------------- |
| `line N` | onde, contado a partir de 1, como um editor mostra; **ausente** para o que diz respeito ao arquivo inteiro |
| a frase | o que foi encontrado, entre as doze categorias abaixo |
| `("…")` | o texto defeituoso do arquivo, quando a categoria não basta; truncado em 80 bytes |
| o final | o que foi feito a respeito: `settled by the reader`, ou `left as it stands` |

**Duas categorias não têm linha**, e são as duas sobre a
codificação: os bytes foram avaliados, ou sua marca lida, antes que uma única
linha do arquivo existisse. A linha delas, portanto, começa pela frase, sem `line N`
na frente — não há lugar a nomear.

**O final da linha é o que mais importa.** `settled by the reader` significa
que o leitor decidiu e que o arquivo gravado traz sua decisão — uma
numeração ausente é regenerada. `left as it stands` significa que ele não tocou em
nada porque **somente você pode decidir**: uma legenda que termina antes de
começar continua como está.

### O número de linha de um bloco

Uma anomalia sobre **um bloco inteiro** é ancorada em sua linha de
tempo, e não na linha exata que a dispara. Uma numeração inconsistente
escrita na linha 5 é, portanto, relatada na linha 6, a da linha de tempo que
a segue — o número defeituoso está no `("…")`. Somente as anomalias sobre
**uma linha** — uma linha de tempo ilegível, texto antes da primeira — são
relatadas nelas mesmas.

### As doze categorias

| Frase | O que a provoca |
| :----- | :------------------ |
| `a line that fits nowhere` | uma linha que não entra em nenhum bloco |
| `a timing line that could not be read` | uma linha de tempo ilegível |
| `a SubRip block without its number` | numeração ausente, regenerada na gravação |
| `SubRip numbers that do not follow` | numeração que salta |
| `text before the first timing line` | texto antes da primeira linha de tempo |
| `a WebVTT block of an unknown kind` | um bloco WebVTT não reconhecido |
| `declares an event column this tool cannot fill` | uma coluna de evento do Sub Station Alpha que nada aqui preenche |
| `counts in frames and states no rate; it was read at` | um arquivo em quadros, que não indica taxa de quadros — a usada é o detalhe |
| `carries no end times; each one was taken from the next start` | um formato sem fins, TMPlayer ou LRC: **nenhum fim da tabela vem do arquivo** |
| `more than one kind of line ending` | finais de linha misturados |
| `an encoding nothing declared` | a codificação foi proposta avaliando os bytes, e não é UTF-8 |
| `a byte order mark that contradicts the encoding asked for` | o arquivo traz uma marca, e `--encoding` nomeou outra — a marca vence |

**As três do meio são as da fase 9**, e as duas últimas dessas
três não falam de uma linha mas do arquivo inteiro: dizem que um
valor exibido não foi lido. Uma coluna `End` preenchida em um LRC, uma
posição em um MicroDVD — o arquivo não trazia nada disso.

**Uma operação pode acrescentar as suas**, que não são diagnósticos de leitura:
[`correct`](correct.md#um-padrão-que-não-pode-ser-aplicado) diz, com a legenda, o padrão
que desistiu. Isso é dito no nível 1 e traz, no objeto do arquivo, o identificador
`pattern-failed` — nunca uma falha do arquivo.

## Uma tradução

Um arquivo de tradução é processado **como qualquer arquivo de legendas**:
`subedit-cli case --to sentence film.fr.srt` o altera sem nada de novo. O que o emparelhamento acrescenta, e o que
três opções dizem, é restrito: **saber como as linhas de uma tradução caem sobre as
legendas do arquivo principal** — é `inspect -t` —, e **alterar a tradução mantendo-a
colocada sobre seu arquivo principal**, com as mesmas regras da janela.

| Opção | Onde | Valor | Padrão |
| :----- | :---- | :----- | :----- |
| `-t`, `--translation-file` | `inspect`, `hearing-impaired`, `replace`, `case`, `italics`, `dialogue-dashes`, `correct`, `pair` | o arquivo de tradução | — |
| `--document` | os seis subcomandos de texto, não `inspect` nem `pair` | `main` ou `translation` | `main` |
| `--align-method` | como `-t` | `position` ou `number` | `position` |

- **`position`** compara o meio de cada linha com os limites das legendas: uma linha ausente não
  desloca as que a seguem. **`number`** envia a linha *n* para a legenda *n*, sem olhar as
  posições. A posição é o padrão, pela razão que a janela tem: uma linha ausente no
  meio desloca todo o resto de uma tradução emparelhada por número.
- **`--document translation` exige `-t`, e `-t` exige `--document translation`** em um subcomando de
  texto — um sem o outro é um erro de uso, código `1`, porque um arquivo lido que nenhum gesto
  usa é um esquecimento e não uma preferência. `-t` em `inspect` não tem `--document`: ele
  relata. **`--align-method` sem `-t` também é um erro de uso.**
- **`-t` nomeia um arquivo, e portanto só faz sentido para uma única entrada**: com várias entradas, ou
  com um diretório tomado por `--recursive`, ele é recusado como `--output` é, **antes que um arquivo seja
  lido**. Para um lote, execute uma invocação por par.
- **Um arquivo de tradução idêntico ao arquivo principal é recusado** — seja qual for a maneira como seu
  caminho é escrito: `<translation>: the file is already open as the main document`, código `2`.
- **Somente o documento visado é gravado**, em **seu próprio caminho e em seu próprio formato**: o formato,
  a codificação e os finais de linha do arquivo de tradução. `--output` nomeia o arquivo da
  tradução, `--output-dir` coloca ali o seu nome base, `--in-place` o reescreve; **o arquivo principal
  nunca é tocado**. Os caminhos do relatório são os da tradução.
- **As legendas que o emparelhamento traz à vida** — uma linha que nada alcançou — são gravadas na
  tradução com **suas próprias posições**, seja qual for o método; **uma legenda que ninguém
  traduziu é gravada como um bloco sem texto**, que é lido sem nenhuma anomalia.
- **`--encoding` e `--frame-rate` se aplicam aos dois arquivos.**
- Os subcomandos de **posição** — `shift`, `transform`, `framerate`, `snap`, `adjust` — não têm
  essas opções: dê-lhes os dois arquivos, como para um lote,
  `subedit-cli shift --by 2 film.srt film.fr.srt --output-dir out/`.
- **Gravar a tradução ressincronizada sobre o arquivo principal** — a única coisa que o emparelhamento faz e que um arquivo sozinho
  não faz — é trabalho de [`pair`](pair.md), que não tem nem `--document` nem `--recursive`.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\n[Un oiseau] Bonjour.\n\n2\n00:00:03,000 --> 00:00:04,000\nAu revoir.\n\n' > film.fr.srt; subedit-cli hearing-impaired film.srt -t film.fr.srt --document translation --output propre.fr.srt; cat propre.fr.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\n[Un oiseau] Bonjour.\n\n2\n00:00:03,000 --> 00:00:04,000\nAu revoir.\n\n' > film.fr.srt; subedit-cli hearing-impaired film.srt -t film.fr.srt --document translation --output propre.fr.srt; cat propre.fr.srt
film.fr.srt: 1 subtitle cleaned, 0 removed -> propre.fr.srt
1
00:00:01,000 --> 00:00:02,000
Bonjour.

2
00:00:03,000 --> 00:00:04,000
Au revoir.
```

Com `--format json`, o objeto de cada arquivo traz também a chave `alignment`, que diz como a
tradução foi colocada — `{"file", "method", "attached", "born", "untranslated", "out_of_order"}`, como
a chave `translation` de [`inspect`](inspect.md#em-json) — e, **com `--dry-run`**, `changes` lista
as legendas da tradução, cada uma com `"document":"translation"`.

## Vários arquivos

Todo subcomando aceita vários caminhos, exceto [`append`](append.md), [`split-file`](split-file.md) e
[`pair`](pair.md), cuja aridade é a de sua página. Cada um é processado
de forma independente: a falha de um não interrompe os outros, e as falhas
são relatadas nomeando o arquivo e o motivo.

**Um diretório representa os arquivos que contém, com `--recursive` (`-r`)**:
sem essa opção, indicar um é um erro de uso, código `1`, e nada é
processado — recusá-lo é melhor do que não processar nada e retornar `0`:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?" -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?"
films: is a directory: use --recursive
code=1
```

A varredura, a árvore mantida sob `--output-dir` e o que é deixado
de lado são descritos em [Processar uma árvore de diretórios](lots.md).

## As páginas de manual

Uma instalação coloca **uma página de manual por subcomando** ao lado de `subedit-cli(1)`:
`subedit-cli-shift(1)`, `subedit-cli-convert(1)` e as outras, uma para cada linha da
[tabela de subcomandos](#subcomandos). `subedit-cli(1)` contém este capítulo — as opções globais,
as saídas, os códigos de saída — e cada outra página contém o capítulo de seu subcomando, com seus
exemplos incluídos. A página da janela, `subedit-gui(1)`, está ao lado delas.

```console
$ man subedit-cli
$ man subedit-cli-shift
```

**As páginas não são escritas separadamente deste manual**: são geradas a partir dele quando o
programa é compilado, de modo que dizem o que ele diz, e não podem divergir dele. A seção ARQUIVOS de cada
página nomeia o diretório em que o manual está instalado.

| Páginas | Onde são instaladas |
| :---- | :----------------------- |
| inglês | `share/man/man1` |
| cada idioma instalado — catalão, tcheco, alemão, espanhol, finlandês, francês, galego, hebraico, húngaro, interlingue, islandês, italiano, holandês, polonês, português, português do Brasil, russo, sérvio, turco, chinês simplificado | `share/man/<language>/man1` — `share/man/fr/man1` |

**`man` escolhe o idioma pela localidade**, como o programa: sob uma localidade francesa,
`man subedit-cli` mostra a página em francês, e um idioma cujas páginas não estão instaladas recebe as
páginas em inglês. `-L` pede um idioma seja qual for a localidade:

```console
$ man -L fr subedit-cli-shift
```

## Códigos de saída

| Código | Significado |
| :--- | :------------ |
| `0` | tudo teve êxito |
| `1` | erro de uso — opção desconhecida, valor inválido, combinação proibida |
| `2` | nenhum arquivo pôde ser processado |
| `3` | alguns arquivos foram processados, outros não |

`2` e `3` são distinguidos para que um script saiba como agir sem reler a saída:
"nada funcionou" e "um está faltando" não pedem a mesma reação.

Um erro de uso é detectado **antes de qualquer processamento**: nunca deixa um
lote processado pela metade.

## Erros

| O que o provoca | O que é escrito, na saída de erro |
| :------------------ | :--------------------------------------- |
| opção ou subcomando desconhecido | o nome defeituoso, e uma indicação para `--help` |
| `--quiet` com `-v` | `--quiet and -v ask for opposite things; give one or the other` |
| `--config` com `--no-config` | `--config excludes --no-config` |
| `--config` com um nome vazio | `--config needs a file name` |
| arquivo ausente | `<path>: does not exist` |
| arquivo ilegível | `<path>: cannot be opened: permission denied` |
| codificação que escreve sua própria marca | `"<name>" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do` |
| bytes que não decodificam | `<path>: cannot be decoded in the chosen encoding` |
| formato não reconhecido | `<path>: is in no format this tool knows` |
| nada que se pareça com uma legenda | `<path>: holds nothing recognisable as a subtitle` |
| `--document translation` sem `-t` | `--document translation needs the translation file: use -t` |
| `-t` sem `--document translation` | `-t names a translation to change: use --document translation` |
| `--align-method` sem `-t` | `--align-method needs a translation file: use -t` |
| `-t` com várias entradas, ou um diretório | `-t names one file but several inputs were given: use one invocation per pair` |
| tradução idêntica ao arquivo principal | `<translation>: the file is already open as the main document` |
