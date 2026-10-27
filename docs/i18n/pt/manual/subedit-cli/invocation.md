# Invocação

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revista por nenhum leitor do idioma.

```
subedit-cli [global options] <subcommand> [options] <file>...
```

Sem subcomando, a ferramenta escreve a sua ajuda e para.

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
  transcribe                  Write the speech of a video or an audio file as subtitles
  config                      List, show and change the operation settings
```

## Idioma

A linha de comandos fala o idioma do seu ambiente: **a sua ajuda, a sua narração e os seus
erros** seguem o idioma pedido. Os exemplos deste manual são executados em inglês — os
blocos são gerados por `make manual`, nunca copiados à mão.

**Como se escolhe o idioma.** A primeira de `LC_ALL`, `LC_MESSAGES` e `LANG` que estiver definida e
não vazia é a localização; `LANGUAGE`, quando definida, enumera então os idiomas pretendidos, separados por
dois pontos (`LANGUAGE=pt_BR:fr`), e é lida em primeiro lugar. É a ordem que o gettext segue, lida pelo
próprio programa: a localização não precisa de estar gerada na máquina.

| Regra | Efeito |
| :--- | :----- |
| codificação e modificador | ignorados: `fr_FR.UTF-8@euro` procura `fr_FR` e depois `fr` |
| `LC_ALL=C`, localização `C` ou `POSIX`, ou nenhuma localização | inglês, e `LANGUAGE` não é lida |
| uma entrada que nomeia o inglês (`en`, `en_GB`) | inglês: o que a segue em `LANGUAGE` não é lido |
| um idioma cujo catálogo não está instalado | o seguinte pedido, e o inglês depois do último, **sem uma palavra** |
| uma mensagem que o catálogo não traduz | mostrada em inglês |

**O nome de um padrão de correção** que o `correct` diz não ter conseguido aplicar segue também o
idioma, quando o catálogo o conhece — os padrões distribuídos com o programa; um padrão próprio
é nomeado tal como o seu ficheiro o escreve. Os ficheiros de padrões em si nunca são reescritos.

Os catálogos são instalados em `<prefix>/share/subedit/locale/<language>/LC_MESSAGES/subedit.mo`
e encontrados a partir do próprio diretório do executável. O inglês está integrado. Esta versão instala `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR`, `ru`, `sr`, `tr` e `zh_CN` — catalão,
checo, alemão, espanhol, finlandês, francês, galego, hebraico, húngaro, interlingue, islandês, italiano, neerlandês, polaco, português, português do Brasil, russo, sérvio, turco e chinês simplificado — traduzidos na totalidade. O francês foi revisto
por um leitor do idioma; o catalão, o checo, o alemão, o espanhol, o finlandês, o galego, o hebraico, o húngaro, o interlingue, o islandês, o italiano, o neerlandês, o polaco, o português, o português do Brasil, o russo, o sérvio, o turco e o chinês simplificado são
traduções automáticas e ainda não foram revistos por nenhum leitor do idioma.
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
Com `LANG=fr_FR.UTF-8`, `subedit-cli --help` começa por “Lire,
inspecter et recaler des fichiers de sous-titres.” Um script que lê as palavras da saída
de texto define `LANGUAGE=en` (ou `LC_ALL=C`) — ou lê `--format json`, que nunca muda.

**O que nunca segue o idioma** — o que um script lê:

| Nunca traduzido | Porquê |
| :--------------- | :-- |
| a saída de `--format json`: chaves, valores, e a `message` e o `detail` das falhas e dos avisos | os mesmos bytes em todos os idiomas; as palavras são em inglês |
| os códigos de saída | os quatro de [Códigos de saída](#códigos-de-saída), em todos os idiomas |
| os nomes escritos: subcomandos, opções, os seus valores (`off`, `srt`), metavariáveis (`FILE`, `TEXT`, `OPTIONS`) | uma linha de comandos escrita uma vez funciona em todos os idiomas |
| os nomes dos padrões de correção que `correct --enable` e `--disable` recebem | um padrão é chamado pelo nome inglês que o seu ficheiro contém |
| os nomes dos formatos e das codificações | um formato é chamado pelo seu nome |
| os diagnósticos do próprio CLI11 — um argumento obrigatório em falta, uma opção desconhecida, um valor que uma opção não aceita — e o seu `Run with --help for more information.` | redigidos dentro do analisador sobre o qual a linha de comandos é construída; permanecem em inglês |

Pedir o inglês, seja qual for o ambiente:

<!-- example: LC_ALL=C LANGUAGE=fr subedit-cli shift --output-dir out a.srt; echo "code=$?" -->
```console
$ LC_ALL=C LANGUAGE=fr subedit-cli shift --output-dir out a.srt; echo "code=$?"
shift needs --by, or --to-grid to work the amount out from the positions
code=1
```

## Opções globais

**Escrevem-se antes do nome do subcomando, e só aí.**
`subedit-cli --encoding cp1252 inspect film.srt` é a forma correta;
`subedit-cli inspect --encoding cp1252 film.srt` é recusado, com
`The following argument was not expected: --encoding`. O mesmo vale para
`--version`, `-v` e `--quiet`.

**`--help` é a exceção, e não é verdadeiramente uma**:
`subedit-cli inspect --help` é aceite porque cada subcomando traz a sua própria, e é a ajuda
do subcomando que escreve, não a da ferramenta.

| Opção | Efeito |
| :----- | :---- |
| `-h`, `--help` | escreve a ajuda e para; a do subcomando, se um for nomeado |
| `--version` | escreve `subedit <version>`, as bibliotecas com que a ferramenta funciona e os idiomas instalados, e para — ver [`--version`](#--version) |
| `-q`, `--quiet` | nível 0 — nenhuma narração |
| `-v`, `-vv`, `-vvv` | níveis 1 a 3; o nível 1 é o predefinido |
| `--encoding NAME` | lê os ficheiros nesta codificação, em vez de a adivinhar |
| `--list-encodings` | escreve as codificações que a ferramenta sabe ler e escrever, uma por linha, e **para** (código `0`) — ver [`--list-encodings`](#--list-encodings) |
| `--format text\|json` | a forma do resultado na saída padrão: o texto, ou um objeto JSON por ficheiro — ver [Uma saída que um script pode ler](#uma-saída-que-um-script-pode-ler) |
| `--config FILE` | lê as definições de operação de `FILE` em vez do ficheiro de definições do utilizador — ver [O ficheiro de definições](#o-ficheiro-de-definições) |
| `--no-config` | não lê nenhum ficheiro de definições: cada definição de operação não indicada na linha toma o seu valor predefinido de origem — ver [O ficheiro de definições](#o-ficheiro-de-definições) |

`--quiet` e `-v` na mesma invocação são recusados: duas intenções opostas
não se resolvem a favor da última escrita.

`--config` e `--no-config` na mesma invocação também são recusados, com
`--config excludes --no-config`, tal como `--config` com um nome vazio, com
`--config needs a file name`: um nome vazio não nomeia nenhum ficheiro, e o ficheiro do próprio utilizador não é lido
em seu lugar.

### O ficheiro de definições

**[`adjust`](adjust.md) e [`correct`](correct.md) leem por predefinição as definições de operação do
ficheiro de definições** — os valores que `Adjust Durations…` e `Correct Texts…` memorizam na
janela, ou que [`config set`](config.md) escreveu. Nenhum outro subcomando o lê. O ficheiro é:

```
$XDG_CONFIG_HOME/subedit/settings.conf
```

ou `~/.config/subedit/settings.conf` quando `XDG_CONFIG_HOME` não está definida, está vazia ou não é um caminho
absoluto — o ficheiro da janela, descrito no [seu manual](../subedit-gui/preferences.md#o-ficheiro).

**Prioridade, da mais forte para a mais fraca: a opção na linha de comandos, depois o ficheiro, depois o valor
predefinido de origem.** Uma opção ganha sempre: `--speed off` desativa uma velocidade que o ficheiro define. Só o
**bloco de operação** do ficheiro é lido; as preferências de apresentação (geometria, colunas, tema…)
nunca o são. [`config list`](config.md#config-list) nomeia as chaves, e quais delas a linha
de comandos lê: as nove chaves `duration-adjust.*`, `correction.human`, `correction.ocr`,
`correction.remove-blank` e `correction.line-break.max-lines`. **Cada uma delas tem uma opção
que a sobrepõe numa invocação**, seja qual for o modo como o ficheiro a define — as tabelas de
[`adjust`](adjust.md#o-que-o-ficheiro-de-definições-fornece) e
[`correct`](correct.md#o-que-é-escolhido-e-o-que-não-é) nomeiam-nas.

**Ler o ficheiro do utilizador nunca faz falhar um comando.** Um ficheiro em falta dá os valores predefinidos; um ficheiro
que não pode ser lido dá os valores predefinidos e di-lo; um valor que não pode ser lido mantém o seu valor predefinido e é nomeado; uma
chave desconhecida é ignorada. O ficheiro nunca é escrito por `adjust` ou `correct`.

**O que foi lido é dito**, no nível 1 predefinido, na saída de erros, antes do primeiro ficheiro:

| Caso | Linha |
| :--- | :--- |
| o ficheiro foi lido | `settings: read <path>` |
| um valor nele não pôde ser lido | `settings: <key>: cannot read '<value>', default kept` |
| o ficheiro do utilizador não existe | `settings: <path> not found, defaults used` |
| o ficheiro não pode ser lido | `settings: <path> unreadable, defaults used` |
| `--no-config` | nada |

Com [`--format json`](#uma-saída-que-um-script-pode-ler), o objeto de cada ficheiro de `adjust` e
`correct` traz `settings`: o caminho que foi lido, ou `null` quando nenhum o foi.


**`--no-config` torna uma execução reprodutível**: a mesma linha dá o mesmo resultado em todas as
máquinas, seja o que for que o utilizador tenha definido na janela. Use-o em scripts, testes e relatórios de erros — ou
`--config FILE` com um ficheiro que os acompanhe.

**`--config FILE` nomeia outro ficheiro**, lido da mesma maneira — **exceto que tem de existir**. Um
nome dado na linha de comandos não é um pedido dos valores predefinidos: quando `FILE` não existe,
`adjust` e `correct` param antes de ler qualquer entrada, escrevem `settings file not found: <FILE>` na
saída de erros, e terminam com o código de utilização `1`. Um `FILE` que existe e não pode ser lido
dá os valores predefinidos e di-lo, tal como o ficheiro do utilizador. [`config show`](config.md#config-show)
continua a mostrar um `FILE` em falta como `(not found)`, e [`config set`](config.md#config-set-e-config-unset) continua a
criá-lo.

<!-- example: printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine.conf adjust --dry-run a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine.conf adjust --dry-run a.srt; echo "code=$?"
settings file not found: mine.conf
code=1
```

### `--version`

**A versão do programa, sozinha na primeira linha** — `subedit <version>`, o que a saída era até
0.15.38 e o que um script lê. Depois, **uma linha por biblioteca com que a ferramenta funciona**, depois os idiomas
da interface instalados, na saída padrão; a ferramenta para, com o código `0`. Um subcomando indicado também
não é executado, e `-q` e `-v` não mudam nada.

<!-- example: subedit-cli --version | head -n 1 -->
```console
$ subedit-cli --version | head -n 1
subedit 1.1.2
```

O conjunto completo, numa máquina com ICU 74.2 e Enchant 2.3.3 — os números são os da máquina:

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
| `ICU` | ler e escrever as codificações, detetá-las, mudar a caixa das letras |
| `Enchant` | o corretor ortográfico de `correct` |
| `CLI11` | ler esta linha de comandos |

- **Cada linha dá duas versões**: a encontrada em tempo de execução, primeiro, e aquela contra a qual o programa foi
  compilado, entre parênteses. Diferem quando a biblioteca da máquina foi atualizada depois de o
  programa ter sido compilado — a primeira coisa a indicar num relatório de erros.
- **Os cabeçalhos do Enchant não trazem versão**: a que está entre parênteses é a que o `pkg-config` comunicou
  quando a compilação foi configurada.
- **O CLI11 está compilado dentro do programa**, sendo os seus cabeçalhos tudo o que dele existe: as suas duas versões são
  sempre iguais.
- **Nem Qt nem libmpv**: a linha de comandos não liga a nenhum dos dois, e lista apenas aquilo com que funciona.
  A janela lista-os — ver o [seu manual](../subedit-gui/invocation.md#--version).
- **`Languages`** lista `en`, o idioma de origem, que não precisa de catálogo, e depois cada catálogo
  instalado em `<prefix>/share/subedit/locale` que se lê, por código — os idiomas que
  `Editar ▸ Preferências…` propõe.
- **O que segue o idioma**: `built with` e `Languages`. A primeira linha, os nomes, os
  números e os códigos de idioma nunca o fazem.
- **Com `--format json`**, antes ou depois de `--version`, **um objeto numa linha**: `schema`
  (`1`), `command` (`"version"`), `version`, `libraries` — uma matriz de
  `{"name":"ICU","loaded":"74.2","built":"74.2"}`, pela ordem do texto — e `languages`, uma
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
depois a ferramenta para, com o código `0`, como faz `--version`: é a resposta, não uma etapa. Um
subcomando indicado também não é executado.

<!-- example: subedit-cli --list-encodings | grep '^UTF' | head -4 -->
```console
$ subedit-cli --list-encodings | grep '^UTF' | head -4
UTF-16BE
UTF-16LE
UTF-32BE
UTF-32LE
```

- **São os nomes que `--encoding` aceita**, os canónicos; os seus aliases (`cp1252`) também são aceites,
  sem serem listados.
- **O número depende do ICU instalado**, e nenhum nome é prometido para além dos que todos os ICU têm
  (`UTF-8`, `UTF-16LE`, `windows-1252`…). O manual não dá, por isso, nenhum número.
- **`UTF-16` e `UTF-32` não estão lá**: o ICU conhece-os, mas o seu conversor escreve uma marca própria,
  e o modelo recusa-os — ver abaixo. Uma lista para escolher não propõe o que o campo seguinte
  recusaria.
- **Com `--format json`**, **um objeto por codificação**:
  `{"schema":1,"command":"list-encodings","name":"UTF-8"}`. Não é o relatório de um ficheiro: estes
  objetos **não trazem nem `file` nem `ok`**, a única exceção ao envelope de
  [a forma](#a-forma). Continuam a ser um objeto por linha, um por entrada.
- **O nível de narração não muda nada**: uma lista é um resultado, e `-q` não a retira.

### `--encoding` e o que não pode forçar

**Aplica-se a todos os subcomandos**, porque todos leem: um ficheiro cuja
codificação foi mal adivinhada continua errado seja o que for que se lhe faça, e uma opção
que só `inspect` trouxesse deixaria uma discordância sem remédio.

**O nome é o do ICU, e todos os seus aliases são aceites**: `cp1252`,
`windows-1252` e `WINDOWS 1252` nomeiam a mesma codificação. O conjunto não está escrito
em lado nenhum: é o que o ICU da máquina sabe converter, e
[`--list-encodings`](#--list-encodings) lista-o. Um nome que não sabe converter é
recusado antes de um único ficheiro ser lido:

```console
$ subedit-cli --encoding klingon-1 inspect film.srt
no encoding is named "klingon-1"
```

**Uma exceção, e apenas uma: a codificação tem de nomear a sua ordem de bytes.**
`UTF-16` e `UTF-32` são nomes que o ICU conhece e cujo conversor escreve **uma marca
própria**. Tal codificação não pode viver neste modelo — a marca deixa de
ser uma escolha, `--no-bom` seria aceite e desobedecido — pelo que ambas são
recusadas, para leitura como para escrita:

```console
$ subedit-cli --encoding UTF-16 inspect film.srt
"UTF-16" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do
```

`UTF-16LE` e `UTF-16BE` leem e escrevem os mesmos bytes, marca incluída; o que
o nome preciso acrescenta é que se sabe qual.

**Uma marca de ordem de bytes prevalece sobre a opção.** É a única coisa que um ficheiro de legendas
declara sobre si próprio, e lê-lo de outra forma que não a que ele declara
seria obedecer ao chamador contra o ficheiro. A discordância é **dita**, como um
diagnóstico:

```console
$ subedit-cli -vvv --encoding cp1252 inspect avec-bom.srt
avec-bom.srt: a byte order mark that contradicts the encoding asked for ("UTF-16LE"), settled by the reader
```

É o que faz o Gaupol — recomeça a leitura com a codificação da
marca — mas sem o dizer; aqui, a diferença entre o que foi pedido e
o que foi feito não é um silêncio.

## Subcomandos

| Subcomando | O que faz | Escreve? |
| :------------ | :-------------- | :----------- |
| [`inspect`](inspect.md) | comunica de que é feito um ficheiro, sem alterar nada | não |
| [`convert`](convert.md) | escreve um ficheiro noutro formato, ou noutra forma | sim |
| [`shift`](shift.md) | desloca todas as posições da mesma quantidade | sim |
| [`transform`](transform.md) | corrige todas as posições a partir de dois pontos conhecidos como certos | sim |
| [`framerate`](framerate.md) | ressincroniza um ficheiro de uma taxa de fotogramas para outra | sim |
| [`snap`](snap.md) | desloca cada instante para o fotograma mais próximo | sim |
| [`hearing-impaired`](hearing-impaired.md) | remove as menções para deficientes auditivos | sim |
| [`adjust`](adjust.md) | ajusta as durações a uma velocidade de leitura e a limites | sim |
| [`replace`](replace.md) | substitui um texto nas legendas, sem quebrar uma etiqueta | sim |
| [`case`](case.md) | põe os textos em iniciais maiúsculas, maiúscula na frase, maiúsculas ou minúsculas | sim |
| [`italics`](italics.md) | põe os textos em itálico, ou tira-lhes o itálico | sim |
| [`dialogue-dashes`](dialogue-dashes.md) | põe hífenes de diálogo, ou tira-os | sim |
| [`sort`](sort.md) | repõe as legendas pela ordem do respetivo início | sim |
| [`join-repeats`](join-repeats.md) | faz uma única legenda dos pedaços de um texto que se sucedem | sim |
| [`disjoint`](disjoint.md) | corta as legendas sobrepostas de modo que nenhum intervalo se sobreponha a outro | sim |
| [`repair`](repair.md) | encadeia `join-repeats`, `sort` e `disjoint` | sim |
| [`correct`](correct.md) | corrige os textos com os padrões do Gaupol: menções, junção e divisão de palavras, erros comuns, utilização de maiúsculas, quebra de linhas | sim |
| [`append`](append.md) | põe ficheiros a seguir ao primeiro, num **único** ficheiro | sim |
| [`split-file`](split-file.md) | corta um ficheiro em **dois**: uma cabeça e uma cauda | sim |
| [`pair`](pair.md) | escreve uma tradução nas posições do seu ficheiro principal, e diz como as suas linhas se emparelharam | sim |
| [`transcribe`](transcribe.md) | escreve a fala de um vídeo ou de um ficheiro de áudio como legendas | sim — o seu próprio destino, `--output` |
| [`config`](config.md) | lista, mostra e altera as definições de operação do ficheiro de definições | só o ficheiro de definições |

Estão lá os vinte e dois; a ajuda da ferramenta lista-os pela mesma ordem.

**Só um não toca em nada**, e é a coluna da direita: `inspect` lê e
comunica. Os outros vinte escrevem, e o que se segue vale para dezassete deles:
[`append`](append.md) e [`split-file`](split-file.md) têm cada um uma aridade própria — N ficheiros para
**um** resultado, um ficheiro para **dois** — e, portanto, o seu próprio destino, que descrevem na sua página;
[`transcribe`](transcribe.md) cria um ficheiro novo a partir de um vídeo ou de um ficheiro de áudio, e aceita apenas `--output`.
[`config`](config.md) não escreve nenhum ficheiro de legendas: lê e altera o ficheiro de definições, e
nada do que se segue se lhe aplica.

## O destino

Os dezassete subcomandos que escrevem — [`convert`](convert.md),
[`shift`](shift.md), [`transform`](transform.md), [`framerate`](framerate.md),
[`snap`](snap.md), [`hearing-impaired`](hearing-impaired.md), [`adjust`](adjust.md),
[`replace`](replace.md), [`case`](case.md), [`italics`](italics.md),
[`dialogue-dashes`](dialogue-dashes.md), [`sort`](sort.md), [`join-repeats`](join-repeats.md),
[`disjoint`](disjoint.md), [`repair`](repair.md), [`correct`](correct.md), [`pair`](pair.md) — recebem o seu destino da mesma maneira. ([`append`](append.md) e
[`split-file`](split-file.md) têm o seu próprio, na sua página.) [`inspect`](inspect.md) não escreve nenhum ficheiro e
não aceita nenhuma destas opções.

**Um destino que já existe é substituído**, sem pergunta e sem opção para
o evitar: a escrita é atómica — o ficheiro nunca fica meio escrito, e uma
escrita interrompida deixa o antigo intacto —, mas o conteúdo anterior é
perdido. Não é uma falha mantida: é a regra, e executar de novo um comando
sobre a mesma pasta de saída executa-o outra vez sobre si próprio.

**Nunca se escreve nada sem um destino explícito.** As três maneiras de
o indicar excluem-se mutuamente, e é preciso uma: sem ela, nada é escrito e o código de
saída é `1` — exceto com [`--dry-run`](#ver-antes-de-escrever), que não escreve nada
e, por isso, não exige nenhuma.

| Opção | Para onde vai o resultado |
| :----- | :---------------- |
| `--output FILE` | para este ficheiro, com este nome exato; uma única entrada |
| `--output-dir DIR` | para esta pasta, com o nome de base da entrada |
| `--in-place` | sobre a entrada, por escrita atómica |

Uma ferramenta que substitui a sua entrada porque nada lhe foi dito é uma ferramenta que se
deixa de usar à segunda vez. A recusa custa uma linha e salva o
ficheiro.

`--output` com várias entradas é recusado: o último ficheiro seria escrito sobre
os anteriores. Num lote, `--output-dir` é o único dos três que faz
sentido, juntamente com `--in-place`.

**A pasta de saída é criada** se não existir, incluindo as pastas superiores — para
`--output-dir`, e para a pasta em que `--output` escreve. Isso é feito depois de a
linha de comandos ser validada e **antes do primeiro ficheiro**. Se não puder ser,
di-lo uma só vez, nenhum ficheiro é lido, e o código de saída é
`2`.

**O lote é julgado no seu conjunto antes de escrever.** O destino de cada entrada
é calculado primeiro, e dele decorrem duas recusas — erros de utilização, código
`1`, **nenhum ficheiro escrito** e nenhuma pasta criada:

- **duas entradas que acabariam no mesmo ficheiro** — `a/film.srt` e
  `b/film.srt` com `--output-dir`, ou `a/film.srt` e `b/film.vtt` ambos
  convertidos para WebVTT, cujo destino final mudou de extensão. A
  mensagem nomeia o destino e as duas entradas; renomear a segunda ou
  deixar a última ganhar perderia um ficheiro sem o dizer, e recusar
  custa uma linha;
- **um destino que é uma das entradas**, sem `--in-place` — incluindo
  através de `--output-dir .`, de `--output` ou de uma ligação simbólica. Escrever sobre
  a própria entrada é um gesto que se nomeia.

Dois caminhos nomeiam o mesmo ficheiro quando o sistema o diz, não quando se
escrevem de forma semelhante: `in/../in/film.srt` e `in/film.srt` são um único ficheiro.

**A extensão acompanha o formato escrito.** Só muda para
[`convert`](convert.md), o único subcomando que muda de formato; os
outros mantêm o formato do ficheiro lido, e portanto a sua extensão.

| O que os desencadeia | Mensagem |
| :------------------ | :------ |
| nenhum destino, sem `--dry-run` | `no destination given: use --output, --output-dir or --in-place` |
| dois destinos | `--output, --output-dir and --in-place exclude one another` |
| `--output` num lote | `--output names one file but several were given: use --output-dir instead` |
| duas entradas, um destino | `<destination>: would be written by both <input> and <input>` |
| uma entrada seria substituída | `<destination>: would be written over the input <input>` — a mesma frase para [`append`](append.md) e [`split-file`](split-file.md) |
| pasta de saída que não pode ser criada | `<directory>: cannot be created: permission denied`, ou `: cannot be created` |
| destino que não pode ser escrito | `<path>: <destination>: cannot be written: permission denied`, ou `: cannot be written` |
| carater em falta na codificação escrita | `<path>: holds a character the chosen encoding cannot write` |

**A última merece uma frase.** Um ficheiro é escrito de volta na **codificação em que
foi lido**, tal como acontece com os seus fins de linha e a sua marca. Se uma
alteração lhe trouxe um carater que essa codificação não conhece — um
`ł` em Latin-1 —, a escrita falha e **nada é escrito**. Substituí-lo
por um `?` seria perder texto à vista de quem acabou de o escrever.

## Ver antes de escrever

**`--dry-run`** é aceite por cada um dos dezasseis subcomandos que escrevem, e significa a mesma
coisa em todos: **ler, calcular, comunicar, não escrever nenhum ficheiro**. Acrescenta-se à
linha que se ia executar; aplicar é executar a mesma linha de novo sem ele. O cálculo é
determinista: o que um `--dry-run` mostrou é o que a execução seguinte escreverá, enquanto o
ficheiro não tiver mudado.

- **Não escreve nenhum ficheiro e não cria nenhuma pasta**, `--in-place` incluído: o ficheiro
  de entrada fica como estava.
- **Não exige nenhum destino.** Pode ser indicado um, e **é então verificado como sem
  `--dry-run`**: duas entradas para o mesmo destino, uma entrada que seria substituída,
  `--output` num lote, duas maneiras de dizer onde — uma linha de comandos recusada sem `--dry-run`
  é recusada com ele, pela mesma frase e pelo mesmo código `1`.
- **O código de saída é o de uma execução real**: `0` quando tudo foi calculado, `2` ou `3`
  quando ficheiros não puderam ser lidos ou processados, `1` para a utilização. Um carater que
  a codificação do ficheiro não saberia escrever faz falhar uma simulação como faz falhar uma execução real.
  **A existência de alterações não muda o código**: os quatro códigos dizem se a ferramenta
  teve êxito, não se o ficheiro está conforme.
- **A narração di-lo.** Onde uma execução real escreve `-> <destination>`, uma simulação
  escreve `(dry run, nothing written)`.

<!-- example: printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by 1 --dry-run a.srt; ls -- *.srt -->
```console
$ printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by 1 --dry-run a.srt; ls -- *.srt
a.srt: 1 subtitle shifted by 1.000 s (dry run, nothing written)
a.srt
```

**Num subcomando que altera textos** —
[`hearing-impaired`](hearing-impaired.md), [`replace`](replace.md), [`case`](case.md),
[`italics`](italics.md), [`dialogue-dashes`](dialogue-dashes.md) e [`correct`](correct.md) —, **a saída padrão traz as alterações
propostas**. Um bloco por legenda alterada, pela ordem do ficheiro:

```
<path>: subtitle <number>
- <the text before, line by line>
+ <the text after, line by line>
```

- o **número** é o que a legenda tem no ficheiro lido, contado a partir de um;
- cada linha do texto antes leva `- ` à frente, cada linha do texto depois `+ `: um texto
  de várias linhas lê-se tal como é, e uma linha que ela própria começa com um hífen
  de diálogo dá `- - `;
- **uma legenda que a operação removeria** di-lo na sua primeira linha,
  `<path>: subtitle <number> (removed)`, e não tem linhas `+`;
- uma legenda da tradução, com `--document translation`, anuncia-se `(translation)` no mesmo
  sítio — ver [Uma tradução](#uma-tradução);
- a primeira linha de cada bloco começa pelo caminho, de modo que `grep '^film.srt: '`
  encontra os blocos de um ficheiro num lote;
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

**A lista é a mesma em JSON**: o objeto do ficheiro traz uma matriz `changes`, ver
[a forma](#a-forma), com `after` definido como `null` para uma legenda removida. **Só é
calculada quando `--dry-run` ou `--format json` a pedem**: um lote de milhares de
legendas não a constrói para nada.

Nos **outros subcomandos** — [`convert`](convert.md), [`shift`](shift.md),
[`transform`](transform.md), [`framerate`](framerate.md), [`snap`](snap.md),
[`adjust`](adjust.md), [`sort`](sort.md), [`append`](append.md), [`split-file`](split-file.md), [`pair`](pair.md) —, a saída
padrão traz o que traria a de uma execução real, isto é, nada em texto; em
JSON, o objeto de cada ficheiro com `"dry_run":true`, `"destination":null` e os mesmos
`counts`.

## Duas saídas, dois papéis

| Saída | O que traz |
| :----- | :--------------- |
| padrão | **o resultado, e só o resultado** — o relatório de `inspect`, as alterações propostas de um texto [`--dry-run`](#ver-antes-de-escrever); com `--format json`, um objeto por ficheiro — um por execução para [`append`](append.md), [`split-file`](split-file.md) e [`pair`](pair.md) |
| de erros | **tudo o resto** — a narração, os avisos, os erros |

É esta separação que permite redirecionar o resultado sem levar a história com ele:

```console
$ subedit-cli inspect *.srt > rapport.txt
```

O relatório vai para o ficheiro, a narração fica no ecrã.

## Uma saída que um script pode ler

**`--format json`** substitui o texto da saída padrão por **um objeto JSON por
ficheiro de entrada**, no formato JSON Lines: um objeto compacto por linha, em UTF-8 sem
marca, o fim de linha é `\n`. `text` é o valor predefinido e não muda. É uma
opção global, e escreve-se portanto antes do subcomando; um valor que não seja
`text` nem `json` é um erro de utilização, código `1`.

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

**Um objeto por entrada, nunca zero, nunca dois**, quer a entrada tenha tido êxito ou não, pela
ordem em que as entradas foram dadas — ou encontradas, com [`--recursive`](lots.md). Contam-se
as linhas, encontram-se os ficheiros. **Três subcomandos são a exceção, porque a sua
aridade não é a de um lote: [`append`](append.md) (N entradas, uma saída), [`split-file`](split-file.md)
(uma entrada, duas saídas) e [`pair`](pair.md) (dois ficheiros, uma saída) escrevem **um objeto para toda a
execução**, que nomeia o ficheiro que a sua página indica.** **Os subcomandos que reescrevem escrevem portanto também
na saída padrão**, o que o seu contrato não fazia em texto: o seu
ficheiro continua a ser o seu resultado, e o objeto é o relatório dele.

**A narração não muda.** `-q`, `-v`, `-vv` e `-vvv` atuam sobre a saída de
erros, em texto, como sem `--format`; **a saída padrão não varia com eles**.
Os diagnósticos de leitura, que o texto mantém no nível 3, são dados e estão
**sempre** em `warnings`. Um erro de utilização não escreve nenhum objeto: é detetado
antes de qualquer ficheiro ser tocado, pelo que não há nada a que o associar; vai para a saída de
erros com o código `1`, e a saída padrão fica vazia.

### A forma

| Chave | Conteúdo | Presente |
| :-- | :------ | :------- |
| `schema` | inteiro, a versão desta forma — `1` | sempre |
| `command` | o nome do subcomando | sempre |
| `file` | o caminho **tal como dado**, ou tal como o percurso de uma pasta o compôs | sempre |
| `ok` | booleano | sempre |
| `error` | `{"kind", "message"}` | se `ok` é falso |
| `dry_run` | booleano: verdadeiro para um [`--dry-run`](#ver-antes-de-escrever) | se `ok` é verdadeiro, num subcomando que escreve |
| `destination` | o caminho escrito, ou `null` quando `dry_run` é verdadeiro: nada foi escrito | idem |
| `counts` | um objeto de **inteiros**, específico do subcomando | idem |
| `changes` | uma matriz de `{"subtitle", "document", "before", "after"}`: os textos alterados, ver abaixo | num subcomando que altera textos |
| `warnings` | uma matriz de `{"kind", "line"?, "detail"?, "settled"?}` | se `ok` é verdadeiro |
| `alignment` | como uma tradução foi colocada, ver [Uma tradução](#uma-tradução) | com `-t`, e para `pair` |
| `constraints` | as definições usadas | `adjust` |
| `settings` | o ficheiro de definições que foi lido, ou `null` quando nenhum o foi — ver [O ficheiro de definições](#o-ficheiro-de-definições) | `adjust`, `correct`, se `ok` é verdadeiro |
| `inputs`, `tail` | um elemento por ficheiro anexado; o caminho da cauda | `append`; `split-file` |

`line` está ausente de um diagnóstico sobre o ficheiro inteiro; `detail` está quando não há
nada a acrescentar; `settled` é `true` quando a leitura **resolveu** alguma coisa — uma
numeração regenerada —, e está ausente quando deixou as coisas como estavam.

**`changes`** lista, pela ordem do ficheiro, cada legenda cujo texto muda:
`subtitle` é o seu número no ficheiro lido, contado a partir de um; `document` é `"main"`
ou `"translation"`; `before` é o texto antes; `after` o texto depois, ou **`null` quando
a legenda é removida**. As quebras de linha de um texto são `\n` nele. É
escrita para uma execução real como para um [`--dry-run`](#ver-antes-de-escrever), e **vazia
(`[]`) quando nada muda**: a sua presença diz que o subcomando lista as suas alterações,
a sua ausência que não tem nenhuma.

**Nenhum número de vírgula flutuante, em lado nenhum.** Uma posição ou uma duração é um inteiro de
**milissegundos** (`…_ms`); uma contagem é um inteiro; uma taxa de fotogramas é uma **cadeia**
(`"25"`, `"24000/1001"`) porque é exata, o que uma vírgula flutuante racional não pode
conter; uma concentração de grelha é um inteiro em **milésimos** — `1000` é uma
grelha perfeita. Duas saídas comparam-se então byte a byte, sem arredondamento.

**Um caminho que não é UTF-8** — o Linux permite-o — não pode ser representado em JSON:
é escrito com U+FFFD no lugar dos bytes inválidos, e o objeto traz o aviso
`path-not-utf8`. A fidelidade de tal caminho não é prometida, e isso fica dito.

### O que cada subcomando põe em `counts`

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
| [`sort`](sort.md) | `subtitles`, `moved` (os lugares que mudaram de legenda) |
| [`adjust`](adjust.md) | `subtitles` (as legendas visadas), `adjusted` (aquelas cujo fim se moveu), depois o grupo `sacrificed`: `speed`, `minimum`, `gap`; e, fora de `counts`, `constraints` — as definições usadas |
| [`convert`](convert.md) | `subtitles`, depois o que a conversão perdeu: `lost_ends` e `lost_header` (0 ou 1), `joined_lines`, `lost_tags`, `lost_fields`, `furthest_ms` |
| [`append`](append.md) | `files`, `subtitles`, `appended`, depois `lost_ends`, `lost_header`, `joined_lines`, `lost_tags`, `lost_fields`, `furthest_ms` somados; e, fora de `counts`, `inputs` — um elemento por ficheiro anexado |
| [`split-file`](split-file.md) | `subtitles`, `head`, `tail`; e, fora de `counts`, `tail` — o caminho da cauda (`destination` é a cabeça) |
| [`pair`](pair.md) | `subtitles` (as do ficheiro escrito); e, fora de `counts`, `alignment` |
| [`inspect`](inspect.md) | nenhum `counts`: a descrição do ficheiro, ver a sua página |

### Identificadores de erro

Um `error.kind` é um destes; **um leitor trata um identificador desconhecido como
“outro”**.

| `kind` | O que é |
| :----- | :----------- |
| `not-found`, `permission-denied`, `io` | o sistema recusou ler — ou escrever — o ficheiro |
| `unknown-format`, `no-subtitle-found`, `undecodable` | o ficheiro é lido e não se deixa ler |
| `unencodable` | um carater que a codificação escolhida não pode escrever |
| `no-byte-order-mark`, `no-frame-rate` | o que `convert` não pode escrever sem que lho digam |
| `before-the-origin`, `beyond-the-end`, `no-transform`, `no-grid` | uma operação que não se pode aplicar a este ficheiro |
| `range-out-of-bounds` | um `--range` que este ficheiro não contém |
| `no-style` | um formato que não escreve estilo (TMPlayer, LRC), para `italics` |
| `refused` | qualquer outra recusa de uma operação |

### O que é prometido e o que não é

**Prometido enquanto `schema` for `1`**: os nomes das chaves, o seu tipo e a sua unidade; o significado
de `ok`, `file` e `destination`; **um objeto por entrada, pela ordem das entradas** — um por execução para os três subcomandos com uma aridade própria;
os identificadores `kind` que existem; o determinismo — mesmas entradas, mesmos argumentos,
mesmos bytes.

**Não prometido**: o **texto das mensagens** (`message`, `detail`), inglês para um humano
e corrigido quando é preciso — um script que o compare está estragado de antemão; a **ordem das
chaves** num objeto, fixa mas não garantida pelo JSON; a presença de **chaves ainda não
acrescentadas**.

**Um leitor ignora, portanto, uma chave desconhecida**, e trata um `kind` desconhecido como “outro”:
acrescentar uma chave ou um identificador não altera `schema`. Remover ou renomear uma chave,
mudar um tipo ou uma unidade, remover um identificador incrementa-o — é uma rutura,
anunciada no CHANGELOG e aqui.

## Níveis de narração

Cada nível **contém o anterior**: subir um grau acrescenta, nunca
substitui.

| Nível | Como | O que a saída de erros traz |
| :----- | :------ | :------------------------------ |
| 0 | `--quiet` | nada, **exceto os erros** |
| 1 | por predefinição, ou `-v` | uma linha por ficheiro processado, e um resumo a partir do momento em que há vários |
| 2 | `-vv` | e o que foi reconhecido: formato, codificação, BOM, fins de linha |
| 3 | `-vvv` | e o rasto de depuração: bytes lidos e escritos, e **cada diagnóstico de leitura** |

**Os erros nunca são silenciados, `--quiet` incluído.** Um comando que falha
em silêncio não deixaria mais do que o seu código de saída.

O resumo só aparece a partir de dois ficheiros: numa única entrada, repetiria
a linha que o precede.

## Ler os diagnósticos

Os formatos de legendas são lidos **o melhor possível**: perante uma anomalia, o
leitor não para — decide, ou deixa as coisas como estão, e di-lo. O que
encontrou é emitido no **nível 3**, uma linha por diagnóstico, na saída
de erros:

```
a.srt: 2 diagnostics while reading
a.srt: line 6: SubRip numbers that do not follow ("7"), settled by the reader
a.srt: line 9: a line that fits nowhere, left as it stands
```

Todos os dezassete subcomandos os comunicam, e não apenas [`inspect`](inspect.md):
um ficheiro lido o melhor possível e depois escrito de volta passou pelas mesmas decisões, e calá-las
levaria a crer que nada aconteceu.

**Um diagnóstico nunca é uma falha.** O ficheiro foi lido, o comando
teve êxito, o código de saída é `0`. É por isso que vivem no nível mais
detalhado: mais abaixo, enterrariam a linha que diz o que foi realmente
feito.

### O que cada linha contém

| Parte | O que diz |
| :----- | :------------- |
| `line N` | onde, contado a partir de 1, como um editor o mostra; **ausente** para o que diz respeito ao ficheiro inteiro |
| a frase | o que foi encontrado, entre as doze categorias abaixo |
| `("…")` | o texto defeituoso do ficheiro, quando a categoria não basta; truncado a 80 bytes |
| o fim | o que foi feito a respeito: `settled by the reader`, ou `left as it stands` |

**Duas categorias não têm linha**, e são as duas sobre a
codificação: os bytes foram avaliados, ou a sua marca lida, antes de uma única
linha do ficheiro existir. A sua linha começa portanto pela frase, sem `line N`
à frente — não há lugar a nomear.

**O fim da linha é o que mais importa.** `settled by the reader` significa
que o leitor decidiu e que o ficheiro escrito traz a sua decisão — uma
numeração em falta é regenerada. `left as it stands` significa que não tocou em
nada porque **só o utilizador pode decidir**: uma legenda que termina antes de
começar fica como está.

### O número de linha de um bloco

Uma anomalia sobre **um bloco inteiro** é ancorada na sua linha de
sincronização, e não na linha exata que a desencadeia. Uma numeração incoerente
escrita na linha 5 é portanto comunicada na linha 6, a da linha de sincronização que
se segue — o número defeituoso está no `("…")`. Só as anomalias sobre
**uma linha** — uma linha de sincronização ilegível, texto antes da primeira — são
comunicadas nelas próprias.

### As doze categorias

| Frase | O que a desencadeia |
| :----- | :------------------ |
| `a line that fits nowhere` | uma linha que não entra em nenhum bloco |
| `a timing line that could not be read` | uma linha de sincronização ilegível |
| `a SubRip block without its number` | numeração em falta, regenerada ao escrever |
| `SubRip numbers that do not follow` | numeração que salta |
| `text before the first timing line` | texto antes da primeira linha de sincronização |
| `a WebVTT block of an unknown kind` | um bloco WebVTT não reconhecido |
| `declares an event column this tool cannot fill` | uma coluna de evento Sub Station Alpha que nada aqui preenche |
| `counts in frames and states no rate; it was read at` | um ficheiro em fotogramas, que não indica nenhuma taxa de fotogramas — a utilizada é o pormenor |
| `carries no end times; each one was taken from the next start` | um formato sem fins, TMPlayer ou LRC: **nenhum fim da tabela vem do ficheiro** |
| `more than one kind of line ending` | fins de linha misturados |
| `an encoding nothing declared` | a codificação foi proposta por avaliação dos bytes, e não é UTF-8 |
| `a byte order mark that contradicts the encoding asked for` | o ficheiro traz uma marca, e `--encoding` nomeou outra — a marca prevalece |

**As três do meio são as da fase 9**, e as duas últimas dessas
três não falam de uma linha mas do ficheiro inteiro: dizem que um
valor mostrado não foi lido. Uma coluna `End` preenchida num LRC, uma
posição num MicroDVD — o ficheiro não trazia nada disso.

**Uma operação pode acrescentar as suas próprias**, que não são um diagnóstico de leitura:
[`correct`](correct.md#um-padrão-que-não-se-pode-aplicar) diz, com a legenda, o padrão
que desistiu. É dito no nível 1 e traz, no objeto do ficheiro, o identificador
`pattern-failed` — nunca uma falha do ficheiro.

## Uma tradução

Um ficheiro de tradução é processado **como qualquer ficheiro de legendas**:
`subedit-cli case --to sentence film.fr.srt` altera-o sem nada de novo. O que o emparelhamento acrescenta, e o que
três opções dizem, é restrito: **saber como as linhas de uma tradução caem sobre as
legendas do ficheiro principal** — é o `inspect -t` —, e **alterar a tradução mantendo-a
colocada sobre o seu ficheiro principal**, com as mesmas regras que a janela.

| Opção | Onde | Valor | Predefinição |
| :----- | :---- | :----- | :----- |
| `-t`, `--translation-file` | `inspect`, `hearing-impaired`, `replace`, `case`, `italics`, `dialogue-dashes`, `correct`, `pair` | o ficheiro de tradução | — |
| `--document` | os seis subcomandos de texto, não `inspect` nem `pair` | `main` ou `translation` | `main` |
| `--align-method` | como `-t` | `position` ou `number` | `position` |

- **`position`** compara o meio de cada linha com os limites das legendas: uma linha em falta não
  desloca as que se seguem. **`number`** envia a linha *n* para a legenda *n*, sem olhar para as
  posições. A posição é a predefinida, pela razão que a janela tem: uma linha em falta a
  meio desloca todo o resto de uma tradução emparelhada por número.
- **`--document translation` exige `-t`, e `-t` exige `--document translation`** num subcomando
  de texto — um sem o outro é um erro de utilização, código `1`, porque um ficheiro lido que nenhum gesto
  usa é uma omissão e não uma preferência. `-t` em `inspect` não tem `--document`: comunica.
  **`--align-method` sem `-t` também é um erro de utilização.**
- **`-t` nomeia um ficheiro, e por isso só faz sentido para uma única entrada**: com várias entradas, ou
  com uma pasta tomada por `--recursive`, é recusado como `--output`, **antes de um ficheiro ser
  lido**. Para um lote, executa-se uma invocação por par.
- **Um ficheiro de tradução idêntico ao ficheiro principal é recusado** — seja qual for a forma como o seu
  caminho é escrito: `<translation>: the file is already open as the main document`, código `2`.
- **Só o documento visado é escrito**, **no seu próprio caminho e no seu próprio formato**: o formato,
  a codificação e os fins de linha do ficheiro de tradução. `--output` nomeia o ficheiro da
  tradução, `--output-dir` coloca lá o seu nome de base, `--in-place` reescreve-o; **o ficheiro principal
  nunca é tocado**. Os caminhos do relatório são os da tradução.
- **As legendas que o emparelhamento faz nascer** — uma linha que nada alcançou — são escritas na
  tradução com **as suas próprias posições**, seja qual for o método; **uma legenda que ninguém
  traduziu é escrita como um bloco sem texto**, que se lê sem nenhuma anomalia.
- **`--encoding` e `--frame-rate` aplicam-se aos dois ficheiros.**
- Os subcomandos de **posição** — `shift`, `transform`, `framerate`, `snap`, `adjust` — não têm
  estas opções: indicam-se-lhes os dois ficheiros, como num lote,
  `subedit-cli shift --by 2 film.srt film.fr.srt --output-dir out/`.
- **Escrever a tradução ressincronizada sobre o ficheiro principal** — a única coisa que o emparelhamento faz e que um ficheiro
  sozinho não faz — é trabalho de [`pair`](pair.md), que não tem nem `--document` nem `--recursive`.

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

Com `--format json`, o objeto de cada ficheiro traz também a chave `alignment`, que diz como a
tradução foi colocada — `{"file", "method", "attached", "born", "untranslated", "out_of_order"}`, como
a chave `translation` de [`inspect`](inspect.md#em-json) — e, **com `--dry-run`**, `changes` lista
as legendas da tradução, cada uma com `"document":"translation"`.

## Vários ficheiros

Todos os subcomandos aceitam vários caminhos, exceto [`append`](append.md), [`split-file`](split-file.md) e
[`pair`](pair.md), cuja aridade é a da sua página. Cada um é processado
de forma independente: a falha de um não interrompe os outros, e as falhas
são comunicadas com o nome do ficheiro e o motivo.

**Uma pasta representa os ficheiros que contém, com `--recursive` (`-r`)**:
sem esta opção, indicar uma é um erro de utilização, código `1`, e nada é
processado — recusá-la é melhor do que não processar nada e devolver `0`:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?" -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?"
films: is a directory: use --recursive
code=1
```

O percurso, a árvore mantida em `--output-dir` e o que é deixado
de lado estão descritos em [Processar uma árvore de pastas](lots.md).

## As páginas de manual

Uma instalação coloca **uma página de manual por subcomando** ao lado de `subedit-cli(1)`:
`subedit-cli-shift(1)`, `subedit-cli-convert(1)` e as outras, uma para cada linha da
[tabela de subcomandos](#subcomandos). `subedit-cli(1)` contém este capítulo — as opções globais,
as saídas, os códigos de saída — e cada outra página contém o capítulo do seu subcomando, com os seus
exemplos. A página da janela, `subedit-gui(1)`, está ao lado delas.

```console
$ man subedit-cli
$ man subedit-cli-shift
```

**As páginas não são escritas à parte deste manual**: são geradas a partir dele quando o
programa é compilado, pelo que dizem o que ele diz, e não podem divergir dele. A secção FILES de cada
página nomeia a pasta em que o manual está instalado.

| Páginas | Onde são instaladas |
| :---- | :----------------------- |
| Inglês | `share/man/man1` |
| cada idioma instalado — catalão, checo, alemão, espanhol, finlandês, francês, galego, hebraico, húngaro, interlingue, islandês, italiano, neerlandês, polaco, português, português do Brasil, russo, sérvio, turco, chinês simplificado | `share/man/<language>/man1` — `share/man/fr/man1` |

**O `man` escolhe o idioma a partir da localização**, como o programa: numa localização francesa,
`man subedit-cli` mostra a página em francês, e um idioma cujas páginas não estão instaladas recebe as
inglesas. `-L` pede um idioma seja qual for a localização:

```console
$ man -L fr subedit-cli-shift
```

## Códigos de saída

| Código | Significado |
| :--- | :------------ |
| `0` | tudo teve êxito |
| `1` | erro de utilização — opção desconhecida, valor inválido, combinação proibida |
| `2` | nenhum ficheiro pôde ser processado |
| `3` | alguns ficheiros foram processados, outros não |

`2` e `3` distinguem-se para que um script saiba como agir sem voltar a ler a saída:
“nada funcionou” e “falta um” não pedem a mesma reação.

Um erro de utilização é detetado **antes de qualquer processamento**: nunca deixa um
lote processado a meio.

## Erros

| O que os desencadeia | O que é escrito, na saída de erros |
| :------------------ | :--------------------------------------- |
| opção ou subcomando desconhecido | o nome defeituoso, e uma indicação de `--help` |
| `--quiet` com `-v` | `--quiet and -v ask for opposite things; give one or the other` |
| `--config` com `--no-config` | `--config excludes --no-config` |
| `--config` com um nome vazio | `--config needs a file name` |
| ficheiro em falta | `<path>: does not exist` |
| ficheiro ilegível | `<path>: cannot be opened: permission denied` |
| codificação que escreve a sua própria marca | `"<name>" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do` |
| bytes que não se descodificam | `<path>: cannot be decoded in the chosen encoding` |
| formato não reconhecido | `<path>: is in no format this tool knows` |
| nada que se pareça com uma legenda | `<path>: holds nothing recognisable as a subtitle` |
| `--document translation` sem `-t` | `--document translation needs the translation file: use -t` |
| `-t` sem `--document translation` | `-t names a translation to change: use --document translation` |
| `--align-method` sem `-t` | `--align-method needs a translation file: use -t` |
| `-t` com várias entradas, ou uma pasta | `-t names one file but several inputs were given: use one invocation per pair` |
| tradução idêntica ao ficheiro principal | `<translation>: the file is already open as the main document` |
