# Invocación

<!-- machine-translated -->
> Esta páxina é unha tradución automática e aínda non a revisou ningún lector da lingua.

```
subedit-cli [global options] <subcommand> [options] <file>...
```

Sen suborde, a ferramenta escribe a súa axuda e detense.

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
  sync                        Re-time the subtitles of a file on the audio of a film
  config                      List, show and change the operation settings
```

## Lingua

A liña de ordes fala a lingua do seu contorno: **a súa axuda, a súa narración e os seus
erros** seguen a lingua solicitada. Os exemplos deste manual execútanse en inglés — os
bloques xéraos `make manual`, nunca se copian a man.

**Como se escolle a lingua.** A primeira de `LC_ALL`, `LC_MESSAGES` e `LANG` que estea definida e
non baleira é a configuración rexional; `LANGUAGE`, cando está definida, enumera despois as linguas desexadas, separadas por
dous puntos (`LANGUAGE=pt_BR:fr`), e lese primeiro. Esta é a orde que segue gettext, lida polo
propio programa: non é preciso xerar a configuración rexional na máquina.

| Regra | Efecto |
| :--- | :----- |
| codificación e modificador | ignorados: `fr_FR.UTF-8@euro` busca `fr_FR`, despois `fr` |
| `LC_ALL=C`, configuración rexional `C` ou `POSIX`, ou ningunha configuración rexional | inglés, e `LANGUAGE` non se le |
| unha entrada que nomea o inglés (`en`, `en_GB`) | inglés: o que a segue en `LANGUAGE` non se le |
| unha lingua cuxo catálogo non está instalado | a seguinte solicitada, e o inglés despois da última, **sen dicir nada** |
| unha mensaxe que o catálogo non traduce | móstrase en inglés |

**O nome dun padrón de corrección** que `correct` di que non puido aplicar tamén segue a
lingua, cando o catálogo o coñece — os padróns distribuídos co programa; un padrón propio
nómease tal como o escribe o seu ficheiro. Os ficheiros de padróns en si nunca se reescriben.

Os catálogos instálanse en `<prefix>/share/subedit/locale/<language>/LC_MESSAGES/subedit.mo`
e atópanse a partir do propio directorio do executable. O inglés vai integrado. Esta versión instala `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR`, `ru`, `sr`, `tr` e `zh_CN` — catalán,
checo, alemán, español, finés, francés, galego, hebreo, húngaro, interlingue, islandés, italiano, neerlandés, polaco, portugués, portugués do Brasil, ruso, serbio, turco e chinés simplificado — traducidos por completo. O francés foi revisado
por un lector da lingua; o catalán, o checo, o alemán, o español, o finés, o galego, o hebreo, o húngaro, o interlingue, o islandés, o italiano, o neerlandés, o polaco, o portugués, o portugués do Brasil, o ruso, o serbio, o turco e o chinés simplificado son
traducións automáticas aínda non revisadas por ningún lector da lingua.
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
Con `LANG=fr_FR.UTF-8`, `subedit-cli --help` comeza por «Lire,
inspecter et recaler des fichiers de sous-titres.» Un script que le as palabras da saída
de texto define `LANGUAGE=en` (ou `LC_ALL=C`) — ou le `--format json`, que nunca cambia.

**O que nunca segue a lingua** — o que le un script:

| Nunca se traduce | Por que |
| :--------------- | :-- |
| a saída de `--format json`: claves, valores, e o `message` e o `detail` dos fallos e dos avisos | os mesmos bytes en todas as linguas; as palabras están en inglés |
| os códigos de saída | os catro de [Códigos de saída](#códigos-de-saída), en todas as linguas |
| os nomes que se escriben: subordes, opcións, os seus valores (`off`, `srt`), metavariables (`FILE`, `TEXT`, `OPTIONS`) | unha liña de ordes escrita unha vez funciona en todas as linguas |
| os nomes dos padróns de corrección que admiten `correct --enable` e `--disable` | un padrón chámase polo nome inglés que contén o seu ficheiro |
| os nomes dos formatos e das codificacións | un formato chámase polo seu nome |
| os diagnósticos propios de CLI11 — un argumento obrigatorio que falta, unha opción descoñecida, un valor que unha opción non acepta — e o seu `Run with --help for more information.` | redactados dentro do analizador sobre o que está construída a liña de ordes; quedan en inglés |

Pedir o inglés, sexa cal sexa o contorno:

<!-- example: LC_ALL=C LANGUAGE=fr subedit-cli shift --output-dir out a.srt; echo "code=$?" -->
```console
$ LC_ALL=C LANGUAGE=fr subedit-cli shift --output-dir out a.srt; echo "code=$?"
shift needs --by, or --to-grid to work the amount out from the positions
code=1
```

## Opcións globais

**Escríbense antes do nome da suborde, e só aí.**
`subedit-cli --encoding cp1252 inspect film.srt` é a forma;
`subedit-cli inspect --encoding cp1252 film.srt` rexéitase, con
`The following argument was not expected: --encoding`. O mesmo vale para
`--version`, `-v` e `--quiet`.

**`--help` é a excepción, e non o é de verdade**:
`subedit-cli inspect --help` acéptase porque cada suborde leva a súa propia, e o que escribe é a axuda da
suborde, non a da ferramenta.

| Opción | Efecto |
| :----- | :---- |
| `-h`, `--help` | escribe a axuda e detense; a da suborde se se nomea unha |
| `--version` | escribe `subedit <version>`, as bibliotecas coas que funciona a ferramenta e as linguas instaladas, e detense — véxase [`--version`](#--version) |
| `-q`, `--quiet` | nivel 0 — ningunha narración |
| `-v`, `-vv`, `-vvv` | niveis 1 a 3; o nivel 1 é o predeterminado |
| `--encoding NAME` | le os ficheiros nesta codificación, en vez de adiviñala |
| `--list-encodings` | escribe as codificacións que a ferramenta pode ler e escribir, unha por liña, e **detense** (código `0`) — véxase [`--list-encodings`](#--list-encodings) |
| `--format text\|json` | a forma do resultado na saída estándar: o texto, ou un obxecto JSON por ficheiro — véxase [Saída que pode ler un script](#saída-que-pode-ler-un-script) |
| `--config FILE` | le a configuración das operacións de `FILE` en vez do ficheiro de configuración do usuario — véxase [O ficheiro de configuración](#o-ficheiro-de-configuración) |
| `--no-config` | non le ningún ficheiro de configuración: cada axuste de operación que non se dea na liña toma o seu valor predeterminado integrado — véxase [O ficheiro de configuración](#o-ficheiro-de-configuración) |

`--quiet` e `-v` na mesma invocación rexéitanse: dúas intencións opostas
non se resolven a favor da última escrita.

`--config` e `--no-config` na mesma invocación tamén se rexeitan, con
`--config excludes --no-config`, e tamén `--config` cun nome baleiro, con
`--config needs a file name`: un nome baleiro non nomea ningún ficheiro, e o ficheiro propio do usuario non se le no
seu lugar.

### O ficheiro de configuración

**[`adjust`](adjust.md) e [`correct`](correct.md) len de forma predeterminada a configuración das operacións do
ficheiro de configuración** — os valores que `Adjust Durations…` e `Correct Texts…` lembran na
xanela, ou que escribiu [`config set`](config.md). Ningunha outra suborde o le. O ficheiro é:

```
$XDG_CONFIG_HOME/subedit/settings.conf
```

ou `~/.config/subedit/settings.conf` cando `XDG_CONFIG_HOME` non está definida, está baleira ou non é unha ruta
absoluta — o ficheiro da xanela, descrito no [seu manual](../subedit-gui/preferences.md#o-ficheiro).

**Prioridade, da máis forte á máis débil: a opción na liña de ordes, despois o ficheiro, despois o valor
predeterminado integrado.** Unha opción gaña sempre: `--speed off` desactiva unha velocidade que fixa o ficheiro. Só se le o
**bloque das operacións** do ficheiro; as preferencias de visualización (xeometría, columnas, tema…)
nunca se len. [`config list`](config.md#config-list) nomea as claves, e cales delas le a liña
de ordes: as nove claves `duration-adjust.*`, `correction.human`, `correction.ocr`,
`correction.remove-blank` e `correction.line-break.max-lines`. **Cada unha delas ten unha opción
que a substitúe durante unha invocación**, sexa cal sexa o valor que lle dea o ficheiro — as táboas de
[`adjust`](adjust.md#o-que-achega-o-ficheiro-de-configuración) e de
[`correct`](correct.md#o-que-se-escolle-e-o-que-non) nómeas.

**Ler o ficheiro do usuario nunca fai fallar unha orde.** Un ficheiro que falta dá os valores predeterminados; un ficheiro
que non se pode ler dá os valores predeterminados e dio; un valor que non se pode ler conserva o seu valor predeterminado e nómease; unha
clave descoñecida ignórase. O ficheiro nunca o escriben `adjust` nin `correct`.

**Dise o que se leu**, no nivel predeterminado 1, no erro estándar, antes do primeiro ficheiro:

| Caso | Liña |
| :--- | :--- |
| leuse o ficheiro | `settings: read <path>` |
| non se puido ler un valor do ficheiro | `settings: <key>: cannot read '<value>', default kept` |
| o ficheiro do usuario non existe | `settings: <path> not found, defaults used` |
| o ficheiro non se pode ler | `settings: <path> unreadable, defaults used` |
| `--no-config` | nada |

Con [`--format json`](#saída-que-pode-ler-un-script), o obxecto de cada ficheiro de `adjust` e de
`correct` leva `settings`: a ruta que se leu, ou `null` cando non se leu ningunha.


**`--no-config` fai que unha execución sexa reproducible**: a mesma liña dá o mesmo resultado en calquera
máquina, defina o que defina o seu usuario na xanela. Úseo en scripts, probas e informes de erros — ou
`--config FILE` cun ficheiro que os acompañe.

**`--config FILE` nomea outro ficheiro**, lido da mesma forma — **agás que debe existir**. Un
nome dado na liña de ordes non é unha petición dos valores predeterminados: cando `FILE` non existe,
`adjust` e `correct` detéñense antes de ler ningunha entrada, escriben `settings file not found: <FILE>` no
erro estándar, e saen co código de uso `1`. Un `FILE` que existe e non se pode ler
dá os valores predeterminados e dio, como fai o ficheiro do usuario. [`config show`](config.md#config-show)
segue mostrando un `FILE` que falta como `(not found)`, e [`config set`](config.md#config-set-e-config-unset) segue
creándoo.

<!-- example: printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine.conf adjust --dry-run a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine.conf adjust --dry-run a.srt; echo "code=$?"
settings file not found: mine.conf
code=1
```

### `--version`

**A versión do programa, soa na primeira liña** — `subedit <version>`, o que era a saída ata
a 0.15.38 e o que le un script. Despois **unha liña por biblioteca coa que funciona a ferramenta**, despois as linguas
da interface instaladas, na saída estándar; a ferramenta detense, co código `0`. Unha suborde dada ademais
non se executa, e `-q` e `-v` non cambian nada.

<!-- example: subedit-cli --version | head -n 1 -->
```console
$ subedit-cli --version | head -n 1
subedit 1.1.5
```

Completa, nunha máquina con ICU 74.2 e Enchant 2.3.3 — os números son os da máquina:

```console
$ subedit-cli --version
subedit <version>
  ICU      74.2 (built with 74.2)
  Enchant  2.3.3 (built with 2.3.3)
  CLI11    2.4.1 (built with 2.4.1)
  Languages: en, fr
```

| Biblioteca | Para que a usa a ferramenta |
| :------ | :------------------------ |
| `ICU` | ler e escribir as codificacións, detectalas, cambiar as maiúsculas e minúsculas das letras |
| `Enchant` | o corrector ortográfico de `correct` |
| `CLI11` | ler esta liña de ordes |

- **Cada liña dá dúas versións**: a atopada no momento da execución, primeiro, e aquela coa que se
  compilou o programa, entre parénteses. Difiren cando a biblioteca da máquina se actualizou despois de
  compilar o programa — o primeiro que hai que indicar nun informe de erro.
- **As cabeceiras de Enchant non levan versión**: a que vai entre parénteses é a que indicou `pkg-config`
  cando se configurou a compilación.
- **CLI11 está compilada dentro do programa**, xa que as súas cabeceiras son todo o que ten: as súas dúas versións son
  sempre iguais.
- **Nin Qt nin libmpv**: a liña de ordes non enlaza ningunha das dúas, e só enumera aquilo co que funciona.
  A xanela enuméraas — véxase o [seu manual](../subedit-gui/invocation.md#--version).
- **`Languages`** enumera `en`, a lingua de orixe, que non precisa catálogo, e despois cada catálogo
  instalado en `<prefix>/share/subedit/locale` que se pode ler, polo seu código — as linguas
  que ofrece `Editar ▸ Preferencias…`.
- **O que segue a lingua**: `built with` e `Languages`. A primeira liña, os nomes, os
  números e os códigos de lingua nunca a seguen.
- **Con `--format json`**, antes ou despois de `--version`, **un obxecto nunha liña**: `schema`
  (`1`), `command` (`"version"`), `version`, `libraries` — unha matriz de
  `{"name":"ICU","loaded":"74.2","built":"74.2"}`, na orde do texto — e `languages`, unha
  matriz de códigos. Nada nel se traduce.

<!-- example: subedit-cli --version --format json | grep -o '"name":"[^"]*"' -->
```console
$ subedit-cli --version --format json | grep -o '"name":"[^"]*"'
"name":"ICU"
"name":"Enchant"
"name":"CLI11"
```

### `--list-encodings`

**As codificacións que a ferramenta pode ler e escribir, unha por liña, ordenadas polo nome**, na saída estándar —
despois a ferramenta detense, co código `0`, como fai `--version`: é a resposta, non un paso. Unha
suborde dada ademais non se executa.

<!-- example: subedit-cli --list-encodings | grep '^UTF' | head -4 -->
```console
$ subedit-cli --list-encodings | grep '^UTF' | head -4
UTF-16BE
UTF-16LE
UTF-32BE
UTF-32LE
```

- **Son os nomes que acepta `--encoding`**, os canónicos; os seus alias (`cp1252`) tamén se aceptan,
  sen figurar na lista.
- **O número depende da ICU instalada**, e non se promete ningún nome alén dos que ten calquera ICU
  (`UTF-8`, `UTF-16LE`, `windows-1252`…). Por iso o manual non dá ningún número.
- **`UTF-16` e `UTF-32` non figuran nela**: ICU coñéceas, pero o seu conversor escribe unha marca propia,
  e o modelo rexéitaas — véxase máis abaixo. Unha lista na que escoller non ofrece o que o campo seguinte
  rexeitaría.
- **Con `--format json`**, **un obxecto por codificación**:
  `{"schema":1,"command":"list-encodings","name":"UTF-8"}`. Non é o informe dun ficheiro: estes
  obxectos **non levan nin `file` nin `ok`**, a única excepción ao envoltorio de
  [a forma](#a-forma). Seguen sendo un obxecto por liña, un por entrada.
- **O nivel de narración non cambia nada**: unha lista é un resultado, e `-q` non a suprime.

### `--encoding`, e o que non pode forzar

**Aplícase a todas as subordes**, porque todas len: un ficheiro cuxa
codificación se adiviña mal segue mal se faga o que se faga con el, e unha opción
que só levase `inspect` deixaría un desaxuste sen remedio.

**O nome é o de ICU, e acéptanse todos os seus alias**: `cp1252`,
`windows-1252` e `WINDOWS 1252` nomean a mesma codificación. O conxunto non está escrito
en ningures: é o que pode converter a ICU da máquina, e
[`--list-encodings`](#--list-encodings) enumérao. Un nome que non pode converter
rexéitase antes de ler un só ficheiro:

```console
$ subedit-cli --encoding klingon-1 inspect film.srt
no encoding is named "klingon-1"
```

**Unha excepción, e só unha: a codificación ten que nomear a súa orde de bytes.**
`UTF-16` e `UTF-32` son nomes que ICU coñece e cuxo conversor escribe **unha marca
propia**. Unha codificación así non pode vivir neste modelo — a marca deixaría
de ser unha escolla, `--no-bom` aceptaríase e desobedeceríase —, así que as dúas
rexéitanse, tanto para ler como para escribir:

```console
$ subedit-cli --encoding UTF-16 inspect film.srt
"UTF-16" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do
```

`UTF-16LE` e `UTF-16BE` len e escriben os mesmos bytes, marca incluída; o que
achega o nome preciso é que se sabe cal é.

**Unha marca de orde de bytes gaña sobre a opción.** É o único que un ficheiro de
subtítulos declara sobre si mesmo, e lelo doutra forma distinta da que el declara
sería obedecer a quen chama en contra do ficheiro. O desaxuste **dise**, como
diagnóstico:

```console
$ subedit-cli -vvv --encoding cp1252 inspect avec-bom.srt
avec-bom.srt: a byte order mark that contradicts the encoding asked for ("UTF-16LE"), settled by the reader
```

Iso é o que fai Gaupol — comeza de novo a lectura coa codificación da
marca — pero sen dicilo; aquí a distancia entre o que se pediu e
o que se fixo non é un silencio.

## Subordes

| Suborde | O que fai | Escribe? |
| :------------ | :-------------- | :----------- |
| [`inspect`](inspect.md) | indica de que está feito un ficheiro, sen cambiar nada | non |
| [`convert`](convert.md) | escribe un ficheiro noutro formato, ou noutra forma | si |
| [`shift`](shift.md) | despraza todas as posicións na mesma cantidade | si |
| [`transform`](transform.md) | corrixe todas as posicións a partir de dous puntos que se sabe que son correctos | si |
| [`framerate`](framerate.md) | recalcula os tempos dun ficheiro dunha taxa de fotogramas a outra | si |
| [`snap`](snap.md) | move cada marca de tempo ao fotograma máis próximo | si |
| [`hearing-impaired`](hearing-impaired.md) | elimina as mencións para persoas xordas | si |
| [`adjust`](adjust.md) | axusta as duracións a unha velocidade de lectura e a uns límites | si |
| [`replace`](replace.md) | substitúe un texto nos subtítulos, sen romper ningunha etiqueta | si |
| [`case`](case.md) | pon os textos en maiúsculas de título, de frase, en maiúsculas ou en minúsculas | si |
| [`italics`](italics.md) | pon os textos en cursiva, ou quítalles a cursiva | si |
| [`dialogue-dashes`](dialogue-dashes.md) | pon os trazos de diálogo, ou quítaos | si |
| [`sort`](sort.md) | volve poñer os subtítulos na orde do seu inicio | si |
| [`join-repeats`](join-repeats.md) | fai un só subtítulo cos anacos dun mesmo texto que se suceden | si |
| [`disjoint`](disjoint.md) | corta os subtítulos que se solapan para que ningunha franxa se solape xa con outra | si |
| [`repair`](repair.md) | encadea `join-repeats`, `sort` e `disjoint` | si |
| [`correct`](correct.md) | corrixe os textos cos padróns de Gaupol: mencións, unión e división de palabras, erros comúns, maiúsculas, salto de liña | si |
| [`append`](append.md) | pon ficheiros despois do primeiro, nun **único** ficheiro | si |
| [`split-file`](split-file.md) | corta un ficheiro en **dous**: unha cabeza e unha cola | si |
| [`pair`](pair.md) | escribe unha tradución nas posicións do seu ficheiro principal, e di como se aliñaron as súas liñas | si |
| [`transcribe`](transcribe.md) | escribe a fala dun vídeo ou dun ficheiro de son como subtítulos | si — o seu propio destino, `--output` |
| [`sync`](sync.md) | recalcula os tempos dos subtítulos dun ficheiro segundo o son dunha película | si |
| [`config`](config.md) | enumera, mostra e cambia a configuración das operacións do ficheiro de configuración | só o ficheiro de configuración |

Están as vinte e tres; a axuda da ferramenta enuméraas na mesma orde.

**Só unha non toca nada**, e iso é a columna da dereita: `inspect` le e
informa. As outras vinte e unha escriben, e o que segue vale para dezaoito delas:
[`append`](append.md) e [`split-file`](split-file.md) teñen cada unha unha aridade propia — N ficheiros para
**un** resultado, un ficheiro para **dous** — e polo tanto o seu propio destino, que describen na súa páxina;
[`transcribe`](transcribe.md) fai un ficheiro novo a partir dun vídeo ou dun ficheiro de son, e toma só `--output`.
[`config`](config.md) non escribe ningún ficheiro de subtítulos: le e cambia o ficheiro de configuración, e
nada do que segue se lle aplica.

## O destino

As dezaoito subordes que escriben — [`convert`](convert.md),
[`shift`](shift.md), [`transform`](transform.md), [`framerate`](framerate.md),
[`snap`](snap.md), [`hearing-impaired`](hearing-impaired.md), [`adjust`](adjust.md),
[`replace`](replace.md), [`case`](case.md), [`italics`](italics.md),
[`dialogue-dashes`](dialogue-dashes.md), [`sort`](sort.md), [`join-repeats`](join-repeats.md),
[`disjoint`](disjoint.md), [`repair`](repair.md), [`correct`](correct.md), [`pair`](pair.md), [`sync`](sync.md) — reciben o seu destino da mesma forma. ([`append`](append.md) e
[`split-file`](split-file.md) teñen o seu propio, na súa páxina.) [`inspect`](inspect.md) non escribe ningún ficheiro e
non acepta ningunha destas opcións.

**Un destino que xa existe sobrescríbese**, sen ningunha pregunta nin ningunha opción para
evitalo: a escritura é atómica — o ficheiro nunca queda escrito a medias, e unha
escritura interrompida deixa intacto o antigo —, pero o contido anterior
pérdese. Non é un defecto que se conserva: é a regra, e executar de novo unha orde
sobre o mesmo directorio de saída execútaa de novo sobre si mesma.

**Nunca se escribe nada sen un destino explícito.** As tres formas de
indicalo exclúense entre si, e cómpre unha: sen ela, non se escribe nada e o código
de saída é `1` — agás con [`--dry-run`](#ver-antes-de-escribir), que non escribe nada
e polo tanto non precisa ningún.

| Opción | Onde vai o resultado |
| :----- | :---------------- |
| `--output FILE` | a este ficheiro, con este nome exacto; unha soa entrada |
| `--output-dir DIR` | a este directorio, co nome base da entrada |
| `--in-place` | por riba da entrada, mediante escritura atómica |

Unha ferramenta que sobrescribe a súa entrada porque non se lle dixo nada é unha ferramenta que se
deixa de usar a segunda vez. O rexeitamento custa unha liña e salva o
ficheiro.

`--output` con varias entradas rexéitase: o último ficheiro escribiríase por riba dos
anteriores. Cun lote, `--output-dir` é a única das tres que ten
sentido, xunto con `--in-place`.

**O directorio de saída créase** se falta, incluídos os pais — para
`--output-dir`, e para o directorio no que escribe `--output`. Faise unha vez
validada a liña de ordes e **antes do primeiro ficheiro**. Se non se pode crear,
dise unha vez, non se le ningún ficheiro, e o código de saída é
`2`.

**O lote xúlgase no seu conxunto antes de escribir.** Primeiro calcúlase o destino de cada
entrada, e diso derivan dous rexeitamentos — erros de uso, código
`1`, **ningún ficheiro escrito** e ningún directorio creado:

- **dúas entradas que acabarían no mesmo ficheiro** — `a/film.srt` e
  `b/film.srt` con `--output-dir`, ou `a/film.srt` e `b/film.vtt` convertidos os dous
  a WebVTT, cuxo destino final cambiou de extensión. A
  mensaxe nomea o destino e as dúas entradas; renomear a segunda ou
  deixar gañar a última perdería un ficheiro sen dicilo, e rexeitar
  custa unha liña;
- **un destino que é unha das entradas**, sen `--in-place` — incluso
  a través de `--output-dir .`, de `--output` ou dunha ligazón simbólica. Escribir por riba
  da propia entrada é un xesto que se nomea.

Dúas rutas nomean o mesmo ficheiro cando o di o sistema, non cando se
escriben igual: `in/../in/film.srt` e `in/film.srt` son un só ficheiro.

**A extensión segue o formato escrito.** Só cambia para
[`convert`](convert.md), a única suborde que cambia de formato; as
outras conservan o formato do ficheiro lido, e polo tanto a súa extensión.

| O que o provoca | Mensaxe |
| :------------------ | :------ |
| ningún destino, sen `--dry-run` | `no destination given: use --output, --output-dir or --in-place` |
| dous destinos | `--output, --output-dir and --in-place exclude one another` |
| `--output` nun lote | `--output names one file but several were given: use --output-dir instead` |
| dúas entradas, un destino | `<destination>: would be written by both <input> and <input>` |
| sobrescribiríase unha entrada | `<destination>: would be written over the input <input>` — a mesma frase para [`append`](append.md) e [`split-file`](split-file.md) |
| directorio de saída que non se pode crear | `<directory>: cannot be created: permission denied`, ou `: cannot be created` |
| destino que non se pode escribir | `<path>: <destination>: cannot be written: permission denied`, ou `: cannot be written` |
| carácter que falta na codificación escrita | `<path>: holds a character the chosen encoding cannot write` |

**A última merece unha frase.** Un ficheiro escríbese de novo na **codificación na que
se leu**, igual que cos seus finais de liña e a súa marca. Se un
cambio introduciu nel un carácter que esta codificación non coñece — un
`ł` en Latin-1 —, a escritura falla e **non se escribe nada**. Substituílo
por un `?` sería perder texto diante dos ollos de quen acaba de escribilo.

## Ver antes de escribir

**`--dry-run`** acéptao cada unha das dezaseis subordes que escriben, e significa o mesmo
en todas partes: **ler, calcular, informar, non escribir ningún ficheiro**. Engádese á
liña que se ía executar; aplicar é executar de novo a mesma liña sen ela. O cálculo é
determinista: o que mostrou un `--dry-run` é o que escribirá a execución seguinte, mentres o
ficheiro non cambie.

- **Non escribe ningún ficheiro nin crea ningún directorio**, `--in-place` incluído: o ficheiro
  de entrada queda como estaba.
- **Non precisa ningún destino.** Pódese dar un, e **compróbase entón como sen
  `--dry-run`**: dúas entradas para o mesmo destino, unha entrada que se sobrescribiría,
  `--output` nun lote, dúas formas de dicir onde — unha liña de ordes rexeitada sen `--dry-run`
  rexéitase con el, coa mesma frase e o mesmo código `1`.
- **O código de saída é o dunha execución real**: `0` cando se calculou todo, `2` ou `3`
  cando non se puideron ler ou procesar ficheiros, `1` para o uso. Un carácter que
  a codificación do ficheiro non puido escribir fai fallar unha simulación como fai fallar unha execución real.
  **Que haxa cambios non cambia o código**: os catro códigos din se a ferramenta
  tivo éxito, non se o ficheiro é conforme.
- **A narración dio.** Onde unha execución real escribe `-> <destination>`, unha
  simulación escribe `(dry run, nothing written)`.

<!-- example: printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by 1 --dry-run a.srt; ls -- *.srt -->
```console
$ printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by 1 --dry-run a.srt; ls -- *.srt
a.srt: 1 subtitle shifted by 1.000 s (dry run, nothing written)
a.srt
```

**Nunha suborde que cambia textos** —
[`hearing-impaired`](hearing-impaired.md), [`replace`](replace.md), [`case`](case.md),
[`italics`](italics.md), [`dialogue-dashes`](dialogue-dashes.md) e [`correct`](correct.md) —, **a saída estándar leva os cambios
propostos**. Un bloque por subtítulo cambiado, na orde do ficheiro:

```
<path>: subtitle <number>
- <the text before, line by line>
+ <the text after, line by line>
```

- o **número** é o que ten o subtítulo no ficheiro lido, contando desde un;
- cada liña do texto anterior leva `- ` diante, cada liña do texto posterior `+ `: un texto
  de varias liñas lese tal como é, e unha liña que xa comeza cun trazo
  de diálogo dá `- - `;
- **un subtítulo que a operación eliminaría** dio na súa primeira liña,
  `<path>: subtitle <number> (removed)`, e non ten liñas `+`;
- un subtítulo da tradución, con `--document translation`, anúnciase `(translation)` no mesmo
  lugar — véxase [Unha tradución](#unha-tradución);
- a primeira liña de cada bloque comeza pola ruta, para que `grep '^film.srt: '`
  atope os bloques dun ficheiro nun lote;
- **ningún bloque** cando nada cambiaría.

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

**A lista é a mesma en JSON**: o obxecto do ficheiro leva unha matriz `changes`, véxase
[a forma](#a-forma), con `after` en `null` para un subtítulo eliminado. **Só se
calcula cando o pide `--dry-run` ou `--format json`**: un lote de miles de
subtítulos non a constrúe para nada.

Nas **outras subordes** — [`convert`](convert.md), [`shift`](shift.md),
[`transform`](transform.md), [`framerate`](framerate.md), [`snap`](snap.md),
[`adjust`](adjust.md), [`sort`](sort.md), [`append`](append.md), [`split-file`](split-file.md), [`pair`](pair.md) —, a saída
estándar leva o que levaría a dunha execución real, é dicir nada en texto; en
JSON, o obxecto de cada ficheiro con `"dry_run":true`, `"destination":null` e os mesmos
`counts`.

## Dúas saídas, dous papeis

| Saída | O que leva |
| :----- | :--------------- |
| estándar | **o resultado, e só o resultado** — o informe de `inspect`, os cambios propostos dun [`--dry-run`](#ver-antes-de-escribir) de texto; con `--format json`, un obxecto por ficheiro — un por execución para [`append`](append.md), [`split-file`](split-file.md) e [`pair`](pair.md) |
| erro | **todo o demais** — a narración, os avisos, os erros |

É esta separación a que permite redirixir o resultado sen meter nel o relato:

```console
$ subedit-cli inspect *.srt > rapport.txt
```

O informe vai ao ficheiro, a narración queda na pantalla.

## Saída que pode ler un script

**`--format json`** substitúe o texto da saída estándar por **un obxecto JSON por
ficheiro de entrada**, no formato JSON Lines: un obxecto compacto por liña, en UTF-8 sen
marca, e o final de liña é `\n`. `text` é o predeterminado e non cambia. É unha
opción global, e polo tanto escríbese antes da suborde; un valor distinto de
`text` e `json` é un erro de uso, código `1`.

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

**Un obxecto por entrada, nunca cero, nunca dous**, tanto se a entrada tivo éxito como se non, na
orde en que se deron as entradas — ou en que se atoparon, con [`--recursive`](lots.md). Cóntanse
as liñas, atópanse os ficheiros propios. **Tres subordes son a excepción, porque a súa
aridade non é a dun lote: [`append`](append.md) (N entradas, unha saída), [`split-file`](split-file.md)
(unha entrada, dúas saídas) e [`pair`](pair.md) (dous ficheiros, unha saída) escriben **un obxecto para toda a
execución**, que nomea o ficheiro que di a súa páxina.** **As subordes que reescriben tamén escriben polo tanto
na saída estándar**, o que o seu contrato non facía en texto: o seu
ficheiro segue sendo o seu resultado, e o obxecto é o informe del.

**A narración non cambia.** `-q`, `-v`, `-vv` e `-vvv` actúan sobre a saída
de erro, en texto, igual que sen `--format`; **a saída estándar non varía con eles**.
Os diagnósticos de lectura, que o texto reserva para o nivel 3, son datos e están
**sempre** en `warnings`. Un erro de uso non escribe ningún obxecto: detéctase
antes de tocar ningún ficheiro, así que non hai nada ao que asocialo; vai á saída de
erro co código `1`, e a saída estándar queda baleira.

### A forma

| Clave | Contido | Presente |
| :-- | :------ | :------- |
| `schema` | enteiro, a versión desta forma — `1` | sempre |
| `command` | o nome da suborde | sempre |
| `file` | a ruta **tal como se deu**, ou tal como a compuxo o percorrido dun directorio | sempre |
| `ok` | booleano | sempre |
| `error` | `{"kind", "message"}` | se `ok` é falso |
| `dry_run` | booleano: verdadeiro para un [`--dry-run`](#ver-antes-de-escribir) | se `ok` é verdadeiro, nunha suborde que escribe |
| `destination` | a ruta escrita, ou `null` cando `dry_run` é verdadeiro: non se escribiu nada | igualmente |
| `counts` | un obxecto de **enteiros**, propio da suborde | igualmente |
| `changes` | unha matriz de `{"subtitle", "document", "before", "after"}`: os textos cambiados, véxase máis abaixo | nunha suborde que cambia textos |
| `warnings` | unha matriz de `{"kind", "line"?, "detail"?, "settled"?}` | se `ok` é verdadeiro |
| `alignment` | como se dispuxo unha tradución, véxase [Unha tradución](#unha-tradución) | con `-t`, e para `pair` |
| `constraints` | os axustes empregados | `adjust` |
| `settings` | o ficheiro de configuración que se leu, ou `null` cando non se leu ningún — véxase [O ficheiro de configuración](#o-ficheiro-de-configuración) | `adjust`, `correct`, se `ok` é verdadeiro |
| `inputs`, `tail` | un elemento por ficheiro engadido; a ruta da cola | `append`; `split-file` |

`line` falta nun diagnóstico sobre todo o ficheiro; `detail` falta cando non hai
nada que engadir; `settled` é `true` cando a lectura **resolveu** algo — unha
numeración rexenerada —, e falta cando deixou as cousas como estaban.

**`changes`** enumera, na orde do ficheiro, cada subtítulo cuxo texto cambia:
`subtitle` é o seu número no ficheiro lido, contando desde un; `document` é `"main"`
ou `"translation"`; `before` é o texto anterior; `after` o texto posterior, ou **`null` cando
o subtítulo se elimina**. Os saltos de liña dun texto son `\n` nel. Escríbese
tanto para unha execución real como para un [`--dry-run`](#ver-antes-de-escribir), e está **baleira
(`[]`) cando nada cambia**: a súa presenza di que a suborde enumera os seus cambios,
a súa ausencia que non os ten.

**Ningún número de coma flotante, en ningures.** Unha posición ou unha duración é un enteiro de
**milisegundos** (`…_ms`); un reconto é un enteiro; unha taxa de fotogramas é unha **cadea**
(`"25"`, `"24000/1001"`) porque é exacta, cousa que un racional en coma flotante non pode
conter; unha concentración da grade é un enteiro en **milésimas** — `1000` é unha
grade perfecta. Dúas saídas compáranse entón byte a byte, sen ningún redondeo.

**Unha ruta que non está en UTF-8** — Linux permíteo — non se pode representar en JSON:
escríbese con U+FFFD no lugar dos bytes non válidos, e o obxecto leva o aviso
`path-not-utf8`. Non se promete a fidelidade dunha ruta así, e dise.

### O que pon cada suborde en `counts`

| Suborde | Claves de `counts` |
| :------------ | :--------------- |
| [`shift`](shift.md) | `subtitles`, `shifted_by_ms` (con signo) |
| [`transform`](transform.md) | `subtitles` |
| [`framerate`](framerate.md) | `subtitles` |
| [`snap`](snap.md) | `subtitles`, `moved` (posicións movidas), `furthest_ms` (o maior movemento) |
| [`hearing-impaired`](hearing-impaired.md) | `cleaned` (textos reescritos), `removed` (subtítulos eliminados) |
| [`replace`](replace.md) | `replaced` (coincidencias substituídas nos subtítulos cambiados), `matched` (coincidencias atopadas) |
| [`case`](case.md), [`italics`](italics.md), [`dialogue-dashes`](dialogue-dashes.md) | `changed` (subtítulos cuxo texto cambiou) |
| [`correct`](correct.md) | `corrected` (textos cambiados), `removed` (subtítulos eliminados) — nunca coincidencias |
| [`sort`](sort.md) | `subtitles`, `moved` (os lugares que cambiaron de subtítulo) |
| [`adjust`](adjust.md) | `subtitles` (os subtítulos obxectivo), `adjusted` (aqueles cuxo final se moveu), despois o grupo `sacrificed`: `speed`, `minimum`, `gap`; e, fóra de `counts`, `constraints` — os axustes empregados |
| [`convert`](convert.md) | `subtitles`, despois o que perdeu a conversión: `lost_ends` e `lost_header` (0 ou 1), `joined_lines`, `lost_tags`, `lost_fields`, `furthest_ms` |
| [`append`](append.md) | `files`, `subtitles`, `appended`, despois `lost_ends`, `lost_header`, `joined_lines`, `lost_tags`, `lost_fields`, `furthest_ms` sumados; e, fóra de `counts`, `inputs` — un elemento por ficheiro engadido |
| [`split-file`](split-file.md) | `subtitles`, `head`, `tail`; e, fóra de `counts`, `tail` — a ruta da cola (`destination` é a cabeza) |
| [`pair`](pair.md) | `subtitles` (os do ficheiro escrito); e, fóra de `counts`, `alignment` |
| [`inspect`](inspect.md) | ningún `counts`: a descrición do ficheiro, véxase a súa páxina |

### Identificadores de erro

Un `error.kind` é un destes; **un lector trata un identificador descoñecido como
«outro»**.

| `kind` | O que é |
| :----- | :----------- |
| `not-found`, `permission-denied`, `io` | o sistema negouse a ler — ou a escribir — o ficheiro |
| `unknown-format`, `no-subtitle-found`, `undecodable` | o ficheiro lese e non se deixa ler |
| `unencodable` | un carácter que a codificación escollida non pode escribir |
| `no-byte-order-mark`, `no-frame-rate` | o que `convert` non pode escribir sen que llo digan |
| `before-the-origin`, `beyond-the-end`, `no-transform`, `no-grid` | unha operación que non se pode aplicar a este ficheiro |
| `range-out-of-bounds` | unha franxa de `--range` que este ficheiro non contén |
| `no-style` | un formato que non escribe ningún estilo (TMPlayer, LRC), para `italics` |
| `refused` | calquera outro rexeitamento dunha operación |

### O que se promete, e o que non

**Prometido mentres `schema` sexa `1`**: os nomes das claves, o seu tipo e a súa unidade; o significado
de `ok`, `file` e `destination`; **un obxecto por entrada, na orde das entradas** — un por execución para as tres subordes cunha aridade propia;
os identificadores `kind` que existen; o determinismo — mesmas entradas, mesmos argumentos,
mesmos bytes.

**Non prometido**: o **texto das mensaxes** (`message`, `detail`), inglés para un humano
que se corrixe cando cómpre — un script que o compara está roto de antemán; a **orde das
claves** nun obxecto, fixa pero non garantida por JSON; a presenza de **claves aínda non
engadidas**.

**Un lector ignora polo tanto unha clave descoñecida**, e trata un `kind` descoñecido como «outro»:
engadir unha clave ou un identificador non cambia `schema`. Eliminar ou renomear unha clave,
cambiar un tipo ou unha unidade, eliminar un identificador increméntao — iso é unha ruptura,
anunciada no CHANGELOG e aquí.

## Niveis de narración

Cada nivel **contén o anterior**: subir un chanzo engade, nunca
substitúe.

| Nivel | Como | O que leva a saída de erro |
| :----- | :------ | :------------------------------ |
| 0 | `--quiet` | nada, **agás os erros** |
| 1 | de forma predeterminada, ou `-v` | unha liña por ficheiro procesado, e un resumo en canto hai varios |
| 2 | `-vv` | e o que se recoñeceu: formato, codificación, BOM, finais de liña |
| 3 | `-vvv` | e o rastro de depuración: bytes lidos e escritos, e **cada diagnóstico de lectura** |

**Os erros nunca se calan, `--quiet` incluído.** Unha orde que falla
en silencio non deixaría máis que o seu código de saída.

O resumo só aparece a partir de dous ficheiros: cunha soa entrada,
repetiría a liña anterior.

## Diagnósticos de lectura

Os formatos de subtítulos lense **o mellor que se pode**: ante unha anomalía, o
lector non se detén — decide, ou deixa as cousas como estaban, e dio. O que
atopou emítese no **nivel 3**, unha liña por diagnóstico, na saída
de erro:

```
a.srt: 2 diagnostics while reading
a.srt: line 6: SubRip numbers that do not follow ("7"), settled by the reader
a.srt: line 9: a line that fits nowhere, left as it stands
```

As dezaoito subordes infórmanos, non só [`inspect`](inspect.md):
un ficheiro lido o mellor que se pode e despois escrito de novo pasou polas mesmas decisións, e calalas
faría crer que non pasou nada.

**Un diagnóstico nunca é un fallo.** O ficheiro leuse, a orde
tivo éxito, o código de saída é `0`. Por iso viven no nivel máis
detallado: máis abaixo, enterrarían a liña que di o que realmente se
fixo.

### O que leva cada liña

| Parte | O que di |
| :----- | :------------- |
| `line N` | onde, contando desde 1, como o mostra un editor; **falta** para o que afecta a todo o ficheiro |
| a frase | o que se atopou, entre as doce categorías de abaixo |
| `("…")` | o texto defectuoso do ficheiro, cando a categoría non abonda; truncado a 80 bytes |
| o final | o que se fixo con iso: `settled by the reader`, ou `left as it stands` |

**Dúas categorías non teñen liña**, e son as dúas que tratan da
codificación: os bytes pesáronse, ou leuse a súa marca, antes de que existise unha soa
liña do ficheiro. A súa liña comeza polo tanto pola frase, sen `line N`
diante — non hai ningún lugar que nomear.

**O final da liña é o que máis importa.** `settled by the reader` significa
que o lector decidiu e que o ficheiro escrito leva a súa decisión — unha
numeración que falta rexenérase. `left as it stands` significa que non tocou
nada porque **só vostede pode decidir**: un subtítulo que remata antes de
comezar queda como está.

### O número de liña dun bloque

Unha anomalía sobre **un bloque enteiro** ancórase na súa liña de
temporización, e non na liña exacta que a provoca. Unha numeración incoherente
escrita na liña 5 indícase polo tanto na liña 6, a da liña de temporización que
segue — o número defectuoso está no `("…")`. Só as anomalías sobre
**unha liña** — unha liña de temporización ilexible, texto antes da primeira — se
indican sobre si mesmas.

### As doce categorías

| Frase | O que a provoca |
| :----- | :------------------ |
| `a line that fits nowhere` | unha liña que non entra en ningún bloque |
| `a timing line that could not be read` | unha liña de temporización ilexible |
| `a SubRip block without its number` | numeración que falta, rexenerada ao escribir |
| `SubRip numbers that do not follow` | numeración que salta |
| `text before the first timing line` | texto antes da primeira liña de temporización |
| `a WebVTT block of an unknown kind` | un bloque WebVTT non recoñecido |
| `declares an event column this tool cannot fill` | unha columna de eventos de Sub Station Alpha que aquí nada enche |
| `counts in frames and states no rate; it was read at` | un ficheiro en fotogramas, que non indica ningunha taxa de fotogramas — a empregada é o detalle |
| `carries no end times; each one was taken from the next start` | un formato sen finais, TMPlayer ou LRC: **ningún final da táboa vén do ficheiro** |
| `more than one kind of line ending` | finais de liña mesturados |
| `an encoding nothing declared` | a codificación propúxose pesando os bytes, e non é UTF-8 |
| `a byte order mark that contradicts the encoding asked for` | o ficheiro leva unha marca, e `--encoding` nomeou outra — gaña a marca |

**As tres do medio son as da fase 9**, e as dúas últimas desas
tres non falan dunha liña senón de todo o ficheiro: din que un
valor mostrado non se leu. Unha columna `End` enchida nun LRC, unha
posición nun MicroDVD — o ficheiro non levaba nada diso.

**Unha operación pode engadir os seus propios**, que non son diagnósticos de lectura:
[`correct`](correct.md#un-padrón-que-non-se-pode-aplicar) di, co subtítulo, o padrón
que desistiu. Dise no nivel 1 e leva, no obxecto do ficheiro, o identificador
`pattern-failed` — nunca un fallo do ficheiro.

## Unha tradución

Un ficheiro de tradución procésase **como calquera ficheiro de subtítulos**:
`subedit-cli case --to sentence film.fr.srt` cámbiao sen nada novo. O que engade o emparellamento, e o que
din tres opcións, é limitado: **saber como caen as liñas dunha tradución sobre os
subtítulos do ficheiro principal** — iso é `inspect -t` —, e **cambiar a tradución mantendo
a súa disposición sobre o seu ficheiro principal**, coas mesmas regras que a xanela.

| Opción | Onde | Valor | Predeterminado |
| :----- | :---- | :----- | :----- |
| `-t`, `--translation-file` | `inspect`, `hearing-impaired`, `replace`, `case`, `italics`, `dialogue-dashes`, `correct`, `pair` | o ficheiro de tradución | — |
| `--document` | as seis subordes de texto, nin `inspect` nin `pair` | `main` ou `translation` | `main` |
| `--align-method` | como `-t` | `position` ou `number` | `position` |

- **`position`** compara o medio de cada liña cos límites dos subtítulos: unha liña que falta non
  despraza as que seguen. **`number`** envía a liña *n* ao subtítulo *n*, sen mirar as
  posicións. A posición é o predeterminado, polo mesmo motivo que na xanela: unha liña que falta no
  medio despraza todo o resto dunha tradución emparellada por número.
- **`--document translation` require `-t`, e `-t` require `--document translation`** nunha suborde
  de texto — un sen o outro é un erro de uso, código `1`, porque un ficheiro lido que ningún xesto
  usa é un descoido e non unha preferencia. `-t` en `inspect` non ten `--document`:
  informa. **`--align-method` sen `-t` tamén é un erro de uso.**
- **`-t` nomea un ficheiro, e polo tanto só ten sentido para unha soa entrada**: con varias entradas, ou
  cun directorio tomado por `--recursive`, rexéitase igual que `--output`, **antes de ler ningún
  ficheiro**. Para un lote, execute unha invocación por par.
- **Un ficheiro de tradución idéntico ao ficheiro principal rexéitase** — se escriba como se escriba a súa
  ruta: `<translation>: the file is already open as the main document`, código `2`.
- **Só se escribe o documento obxectivo**, na **súa propia ruta e no seu propio formato**: o formato,
  a codificación e os finais de liña do ficheiro de tradución. `--output` nomea o ficheiro da
  tradución, `--output-dir` pon alí o seu nome base, `--in-place` reescríbeo; **o ficheiro principal
  nunca se toca**. As rutas do informe son as da tradución.
- **Os subtítulos que fai nacer o emparellamento** — unha liña á que non chegou nada — escríbense na
  tradución coas **súas propias posicións**, sexa cal sexa o método; **un subtítulo que ninguén
  traduciu escríbese como un bloque sen texto**, que se le sen ningunha anomalía.
- **`--encoding` e `--frame-rate` aplícanse aos dous ficheiros.**
- As subordes de **posición** — `shift`, `transform`, `framerate`, `snap`, `adjust` — non teñen
  estas opcións: déalles os dous ficheiros, como para un lote,
  `subedit-cli shift --by 2 film.srt film.fr.srt --output-dir out/`.
- **Escribir a tradución cos tempos recalculados sobre o ficheiro principal** — o único que fai o emparellamento e que un ficheiro só
  non fai — é o traballo de [`pair`](pair.md), que non ten nin `--document` nin `--recursive`.

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

Con `--format json`, o obxecto de cada ficheiro leva tamén a clave `alignment`, que di como se
dispuxo a tradución — `{"file", "method", "attached", "born", "untranslated", "out_of_order"}`, como
a clave `translation` de [`inspect`](inspect.md#en-json) — e, **con `--dry-run`**, `changes` enumera
os subtítulos da tradución, cada un con `"document":"translation"`.

## Varios ficheiros

Todas as subordes aceptan varias rutas, agás [`append`](append.md), [`split-file`](split-file.md) e
[`pair`](pair.md), cuxa aridade é a da súa páxina. Cada unha procésase
de forma independente: o fallo dunha non interrompe as outras, e os fallos
infórmanse nomeando o ficheiro e o motivo.

**Un directorio representa os ficheiros que contén, con `--recursive` (`-r`)**:
sen esta opción, dar un é un erro de uso, código `1`, e non se
procesa nada — rexeitalo é mellor que non procesar nada e devolver `0`:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?" -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?"
films: is a directory: use --recursive
code=1
```

O percorrido, a árbore conservada baixo `--output-dir` e o que se deixa
de lado descríbense en [Procesar unha árbore](lots.md).

## As páxinas de manual

Unha instalación pon **unha páxina de manual por suborde** xunto a `subedit-cli(1)`:
`subedit-cli-shift(1)`, `subedit-cli-convert(1)` e as demais, unha por cada fila da
[táboa das subordes](#subordes). `subedit-cli(1)` contén este capítulo — as opcións globais,
as saídas, os códigos de saída — e cada unha das outras páxinas contén o capítulo da súa suborde, cos seus
exemplos incluídos. A páxina da xanela, `subedit-gui(1)`, está ao seu lado.

```console
$ man subedit-cli
$ man subedit-cli-shift
```

**As páxinas non se escriben á parte deste manual**: xéranse a partir del cando se
compila o programa, así que din o que el di, e non poden afastarse del. A sección FICHEIROS de cada
páxina nomea o directorio no que está instalado o manual.

| Páxinas | Onde se instalan |
| :---- | :----------------------- |
| inglés | `share/man/man1` |
| cada lingua instalada — catalán, checo, alemán, español, finés, francés, galego, hebreo, húngaro, interlingue, islandés, italiano, neerlandés, polaco, portugués, portugués do Brasil, ruso, serbio, turco, chinés simplificado | `share/man/<language>/man1` — `share/man/fr/man1` |

**`man` escolle a lingua a partir da configuración rexional**, como fai o programa: cunha configuración rexional francesa,
`man subedit-cli` mostra a páxina francesa, e unha lingua cuxas páxinas non están instaladas recibe as
inglesas. `-L` pide unha lingua sexa cal sexa a configuración rexional:

```console
$ man -L fr subedit-cli-shift
```

## Códigos de saída

| Código | Significado |
| :--- | :------------ |
| `0` | todo tivo éxito |
| `1` | erro de uso — opción descoñecida, valor non válido, combinación prohibida |
| `2` | non se puido procesar ningún ficheiro |
| `3` | procesáronse algúns ficheiros, outros non |

`2` e `3` distínguense para que un script saiba como actuar sen ler de novo a saída:
«nada funcionou» e «falta un» non piden a mesma reacción.

Un erro de uso detéctase **antes de calquera procesamento**: nunca deixa un
lote procesado a medias.

## Erros

| O que o provoca | O que se escribe, na saída de erro |
| :------------------ | :--------------------------------------- |
| opción ou suborde descoñecida | o nome defectuoso, e unha indicación cara a `--help` |
| `--quiet` con `-v` | `--quiet and -v ask for opposite things; give one or the other` |
| `--config` con `--no-config` | `--config excludes --no-config` |
| `--config` cun nome baleiro | `--config needs a file name` |
| ficheiro que falta | `<path>: does not exist` |
| ficheiro ilexible | `<path>: cannot be opened: permission denied` |
| codificación que escribe a súa propia marca | `"<name>" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do` |
| bytes que non se descodifican | `<path>: cannot be decoded in the chosen encoding` |
| formato non recoñecido | `<path>: is in no format this tool knows` |
| nada que pareza un subtítulo | `<path>: holds nothing recognisable as a subtitle` |
| `--document translation` sen `-t` | `--document translation needs the translation file: use -t` |
| `-t` sen `--document translation` | `-t names a translation to change: use --document translation` |
| `--align-method` sen `-t` | `--align-method needs a translation file: use -t` |
| `-t` con varias entradas, ou un directorio | `-t names one file but several inputs were given: use one invocation per pair` |
| tradución idéntica ao ficheiro principal | `<translation>: the file is already open as the main document` |
