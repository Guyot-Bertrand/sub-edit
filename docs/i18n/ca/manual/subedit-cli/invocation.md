# Invocació

<!-- machine-translated -->
> Aquesta pàgina és una traducció automàtica i encara no l'ha revisada cap lector de la llengua.

```
subedit-cli [global options] <subcommand> [options] <file>...
```

Sense cap subordre, l'eina escriu la seva ajuda i s'atura.

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


## Llengua

La línia d'ordres parla la llengua del seu entorn: **la seva ajuda, la seva narració i els seus
errors** segueixen la llengua demanada. Els exemples d'aquest manual s'executen en anglès — els
blocs els genera `make manual`, mai no es copien a mà.

**Com es tria la llengua.** La primera de `LC_ALL`, `LC_MESSAGES` i `LANG` que està definida i
no és buida és la configuració regional; `LANGUAGE`, quan està definida, llista després les llengües
volgudes, separades per dos punts (`LANGUAGE=pt_BR:fr`), i es llegeix primer. És l'ordre que segueix
gettext, llegit pel mateix programa: no cal que la configuració regional estigui generada a la màquina.

| Regla | Efecte |
| :--- | :----- |
| codificació i modificador | s'ignoren: `fr_FR.UTF-8@euro` cerca `fr_FR`, i després `fr` |
| `LC_ALL=C`, configuració regional `C` o `POSIX`, o cap configuració regional | anglès, i `LANGUAGE` no es llegeix |
| una entrada que anomena l'anglès (`en`, `en_GB`) | anglès: el que la segueix a `LANGUAGE` no es llegeix |
| una llengua el catàleg de la qual no està instal·lat | la següent que s'ha demanat, i l'anglès després de l'última, **sense dir res** |
| un missatge que el catàleg no tradueix | es mostra en anglès |

**El nom d'un patró de correcció** que `correct` diu que no ha pogut aplicar també segueix la
llengua, quan el catàleg el coneix — els patrons que es distribueixen amb el programa; un patró propi
s'anomena tal com l'escriu el seu fitxer. Els fitxers de patrons mateixos no es reescriuen mai.

Els catàlegs s'instal·len a `<prefix>/share/subedit/locale/<language>/LC_MESSAGES/subedit.mo`
i es troben a partir del directori del mateix executable. L'anglès està integrat al programa. Aquesta versió instal·la `ca`, `cs`, `de` i `fr` — català, txec, alemany
i francès — traduïts íntegrament. El francès l'ha revisat un lector de la llengua; el català,
el txec i l'alemany són traduccions automàtiques que encara no ha revisat cap lector de la llengua.
<!-- machine-translated: ca -->
<!-- machine-translated: cs -->
<!-- machine-translated: de -->
Amb `LANG=fr_FR.UTF-8`, `subedit-cli --help` comença amb «Lire,
inspecter et recaler des fichiers de sous-titres.» Un script que llegeix les paraules de la sortida
de text defineix `LANGUAGE=en` (o `LC_ALL=C`) — o llegeix `--format json`, que no canvia mai.

**El que no segueix mai la llengua** — el que llegeix un script:

| Mai traduït | Per què |
| :--------------- | :-- |
| la sortida de `--format json`: claus, valors, i el `message` i el `detail` de les fallades i els avisos | els mateixos bytes en totes les llengües; les paraules són en anglès |
| els codis de sortida | els quatre de [Codis de sortida](#codis-de-sortida), en totes les llengües |
| els noms que s'escriuen: subordres, opcions, els seus valors (`off`, `srt`), metavariables (`FILE`, `TEXT`, `OPTIONS`) | una línia d'ordres escrita una vegada funciona en totes les llengües |
| els noms dels patrons de correcció que accepten `correct --enable` i `--disable` | un patró es crida pel nom anglès que conté el seu fitxer |
| els noms dels formats i de les codificacions | un format es crida pel seu nom |
| els diagnòstics propis de CLI11 — un argument obligatori que falta, una opció desconeguda, un valor que una opció no accepta — i el seu `Run with --help for more information.` | es redacten dins de l'analitzador sobre el qual està construïda la línia d'ordres; es queden en anglès |

Demanar l'anglès, sigui quin sigui l'entorn:

<!-- example: LC_ALL=C LANGUAGE=fr subedit-cli shift --output-dir out a.srt; echo "code=$?" -->
```console
$ LC_ALL=C LANGUAGE=fr subedit-cli shift --output-dir out a.srt; echo "code=$?"
shift needs --by, or --to-grid to work the amount out from the positions
code=1
```

## Opcions globals

**S'escriuen abans del nom de la subordre, i només allà.**
`subedit-cli --encoding cp1252 inspect film.srt` és la forma;
`subedit-cli inspect --encoding cp1252 film.srt` es refusa, amb
`The following argument was not expected: --encoding`. El mateix val per a
`--version`, `-v` i `--quiet`.

**`--help` és l'excepció, i en realitat no ho és**:
`subedit-cli inspect --help` s'accepta perquè cada subordre porta la seva, i el que escriu és l'ajuda
de la subordre, no la de l'eina.

| Opció | Efecte |
| :----- | :---- |
| `-h`, `--help` | escriu l'ajuda i s'atura; la de la subordre si se n'anomena una |
| `--version` | escriu `subedit <version>`, les biblioteques amb què s'executa l'eina i les llengües instal·lades, i s'atura — vegeu [`--version`](#--version) |
| `-q`, `--quiet` | nivell 0 — cap narració |
| `-v`, `-vv`, `-vvv` | nivells 1 a 3; el nivell 1 és el per defecte |
| `--encoding NAME` | llegeix els fitxers en aquesta codificació, en lloc d'endevinar-la |
| `--list-encodings` | escriu les codificacions que l'eina pot llegir i escriure, una per línia, i **s'atura** (codi `0`) — vegeu [`--list-encodings`](#--list-encodings) |
| `--format text\|json` | la forma del resultat a la sortida estàndard: el text, o un objecte JSON per fitxer — vegeu [Sortida llegible per un script](#sortida-llegible-per-un-script) |
| `--config FILE` | llegeix la configuració de les operacions de `FILE` en lloc del fitxer de configuració de l'usuari — vegeu [El fitxer de configuració](#el-fitxer-de-configuració) |
| `--no-config` | no llegeix cap fitxer de configuració: cada paràmetre d'operació que no s'indica a la línia pren el seu valor per defecte integrat — vegeu [El fitxer de configuració](#el-fitxer-de-configuració) |

`--quiet` i `-v` a la mateixa invocació es refusen: dues intencions oposades
no es resolen a favor de l'última que s'ha escrit.

`--config` i `--no-config` a la mateixa invocació també es refusen, amb
`--config excludes --no-config`, i també `--config` amb un nom buit, amb
`--config needs a file name`: un nom buit no anomena cap fitxer, i el fitxer de l'usuari no es llegeix
en el seu lloc.

### El fitxer de configuració

**[`adjust`](adjust.md) i [`correct`](correct.md) llegeixen per defecte la configuració de les operacions del
fitxer de configuració** — els valors que `Adjust Durations…` i `Correct Texts…` recorden a
la finestra, o que ha escrit [`config set`](config.md). Cap altra subordre no el llegeix. El fitxer és:

```
$XDG_CONFIG_HOME/subedit/settings.conf
```

o `~/.config/subedit/settings.conf` quan `XDG_CONFIG_HOME` no està definida, és buida o no és un camí
absolut — el fitxer de la finestra, descrit al [seu manual](../subedit-gui/preferences.md#el-fitxer).

**Prioritat, de la més forta a la més feble: l'opció de la línia d'ordres, després el fitxer, després el valor
per defecte integrat.** Una opció sempre guanya: `--speed off` desactiva una velocitat que fixa el fitxer. Només
es llegeix el **bloc d'operacions** del fitxer; les preferències de visualització (geometria, columnes, tema…)
no es llegeixen mai. [`config list`](config.md#config-list) anomena les claus, i quines llegeix la línia
d'ordres: les nou claus `duration-adjust.*`, `correction.human`, `correction.ocr`,
`correction.remove-blank` i `correction.line-break.max-lines`. **Cadascuna té una opció
que la substitueix durant una invocació**, sigui com sigui que la fixi el fitxer — les taules
d'[`adjust`](adjust.md#què-aporta-el-fitxer-de-configuració) i
de [`correct`](correct.md#què-es-tria-i-què-no) les anomenen.

**Llegir el fitxer de l'usuari no fa fallar mai una ordre.** Un fitxer que falta dona els valors per defecte; un fitxer
que no es pot llegir dona els valors per defecte i ho diu; un valor que no es pot llegir conserva el seu valor per defecte i s'anomena; una
clau desconeguda s'ignora. `adjust` i `correct` no escriuen mai el fitxer.

**Es diu què s'ha llegit**, al nivell 1 per defecte, a la sortida d'error estàndard, abans del primer fitxer:

| Cas | Línia |
| :--- | :--- |
| s'ha llegit el fitxer | `settings: read <path>` |
| un valor del fitxer no s'ha pogut llegir | `settings: <key>: cannot read '<value>', default kept` |
| el fitxer de l'usuari no existeix | `settings: <path> not found, defaults used` |
| el fitxer no es pot llegir | `settings: <path> unreadable, defaults used` |
| `--no-config` | res |

Amb [`--format json`](#sortida-llegible-per-un-script), l'objecte de cada fitxer d'`adjust` i
de `correct` porta `settings`: el camí que s'ha llegit, o `null` quan no se n'ha llegit cap.


**`--no-config` fa que una execució sigui reproduïble**: la mateixa línia dona el mateix resultat a cada
màquina, sigui el que sigui el que l'usuari hagi fixat a la finestra. Useu-la en scripts, proves i informes d'errors — o
`--config FILE` amb un fitxer que els acompanyi.

**`--config FILE` anomena un altre fitxer**, que es llegeix de la mateixa manera — **excepte que ha d'existir**. Un
nom indicat a la línia d'ordres no és una petició dels valors per defecte: quan `FILE` no existeix,
`adjust` i `correct` s'aturen abans de llegir cap entrada, escriuen `settings file not found: <FILE>` a
la sortida d'error estàndard, i surten amb el codi d'ús `1`. Un `FILE` que existeix i no es pot llegir
dona els valors per defecte i ho diu, com ho fa el fitxer de l'usuari. [`config show`](config.md#config-show)
continua mostrant un `FILE` que falta com a `(not found)`, i [`config set`](config.md#config-set-i-config-unset) continua
creant-lo.

<!-- example: printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine.conf adjust --dry-run a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine.conf adjust --dry-run a.srt; echo "code=$?"
settings file not found: mine.conf
code=1
```

### `--version`

**La versió del programa, sola a la primera línia** — `subedit <version>`, el que era la sortida fins a la
0.15.38 i el que llegeix un script. Després **una línia per cada biblioteca amb què s'executa l'eina**, i després les llengües
de la interfície instal·lades, a la sortida estàndard; l'eina s'atura, amb el codi `0`. Una subordre indicada alhora
no s'executa, i `-q` i `-v` no hi canvien res.

<!-- example: subedit-cli --version | head -n 1 -->
```console
$ subedit-cli --version | head -n 1
subedit 0.15.42
```

La sortida sencera, en una màquina amb ICU 74.2 i Enchant 2.3.3 — els números són els de la màquina:

```console
$ subedit-cli --version
subedit <version>
  ICU      74.2 (built with 74.2)
  Enchant  2.3.3 (built with 2.3.3)
  CLI11    2.4.1 (built with 2.4.1)
  Languages: en, fr
```

| Biblioteca | Per a què la fa servir l'eina |
| :------ | :------------------------ |
| `ICU` | llegir i escriure les codificacions, detectar-les, canviar entre majúscules i minúscules |
| `Enchant` | el corrector ortogràfic de `correct` |
| `CLI11` | llegir aquesta línia d'ordres |

- **Cada línia dona dues versions**: la que es troba en temps d'execució, primer, i aquella contra la qual es va
  compilar el programa, entre parèntesis. Difereixen quan la biblioteca de la màquina s'ha actualitzat després de
  compilar el programa — el primer que cal indicar en un informe d'error.
- **Les capçaleres d'Enchant no porten cap versió**: la que hi ha entre parèntesis és la que va informar `pkg-config`
  quan es va configurar la compilació.
- **CLI11 es compila dins del programa**, ja que les seves capçaleres són tot el que en té: les seves dues versions són
  sempre les mateixes.
- **Ni Qt ni libmpv**: la línia d'ordres no enllaça cap de les dues, i només llista allò amb què s'executa.
  La finestra les llista — vegeu el [seu manual](../subedit-gui/invocation.md#--version).
- **`Languages`** llista `en`, la llengua d'origen, que no necessita cap catàleg, i després cada catàleg
  instal·lat a `<prefix>/share/subedit/locale` que es pot llegir, pel seu codi — les llengües
  que ofereix `Edita ▸ Preferències…`.
- **Què segueix la llengua**: `built with` i `Languages`. La primera línia, els noms, els
  números i els codis de llengua no la segueixen mai.
- **Amb `--format json`**, abans o després de `--version`, **un objecte en una línia**: `schema`
  (`1`), `command` (`"version"`), `version`, `libraries` — una matriu de
  `{"name":"ICU","loaded":"74.2","built":"74.2"}`, en l'ordre del text — i `languages`, una
  matriu de codis. No s'hi tradueix res.

<!-- example: subedit-cli --version --format json | grep -o '"name":"[^"]*"' -->
```console
$ subedit-cli --version --format json | grep -o '"name":"[^"]*"'
"name":"ICU"
"name":"Enchant"
"name":"CLI11"
```

### `--list-encodings`

**Les codificacions que l'eina pot llegir i escriure, una per línia, ordenades pel nom**, a la sortida estàndard —
després l'eina s'atura, amb el codi `0`, com fa `--version`: és la resposta, no un pas. Una
subordre indicada alhora no s'executa.

<!-- example: subedit-cli --list-encodings | grep '^UTF' | head -4 -->
```console
$ subedit-cli --list-encodings | grep '^UTF' | head -4
UTF-16BE
UTF-16LE
UTF-32BE
UTF-32LE
```

- **Són els noms que accepta `--encoding`**, els canònics; els seus àlies (`cp1252`) també s'accepten,
  sense que es llistin.
- **El nombre depèn de la ICU instal·lada**, i no es promet cap nom més enllà dels que té qualsevol ICU
  (`UTF-8`, `UTF-16LE`, `windows-1252`…). Per tant, el manual no en dona el nombre.
- **`UTF-16` i `UTF-32` no hi són**: ICU els coneix, però el seu convertidor escriu una marca pròpia,
  i el model els refusa — vegeu més avall. Una llista on triar no ofereix el que el camp següent
  refusaria.
- **Amb `--format json`**, **un objecte per codificació**:
  `{"schema":1,"command":"list-encodings","name":"UTF-8"}`. No és l'informe d'un fitxer: aquests
  objectes **no porten ni `file` ni `ok`**, l'única excepció a l'embolcall de
  [la forma](#la-forma). Continuen sent un objecte per línia, un per entrada.
- **El nivell de narració no hi canvia res**: una llista és un resultat, i `-q` no la treu.

### `--encoding`, i el que no pot forçar

**S'aplica a totes les subordres**, perquè totes llegeixen: un fitxer la
codificació del qual s'endevina malament continua malament es faci el que es faci, i una opció
que només portés `inspect` deixaria una discrepància sense remei.

**El nom és el d'ICU, i s'accepten tots els seus àlies**: `cp1252`,
`windows-1252` i `WINDOWS 1252` anomenen la mateixa codificació. El conjunt no està escrit
enlloc: és el que pot convertir la ICU de la màquina, i
[`--list-encodings`](#--list-encodings) el llista. Un nom que no pot convertir es
refusa abans de llegir cap fitxer:

```console
$ subedit-cli --encoding klingon-1 inspect film.srt
no encoding is named "klingon-1"
```

**Una excepció, i només una: la codificació ha d'anomenar el seu ordre de bytes.**
`UTF-16` i `UTF-32` són noms que ICU coneix i el convertidor dels quals escriu **una marca
pròpia**. Una codificació així no pot viure en aquest model — la marca deixa
de ser una elecció, `--no-bom` s'acceptaria i es desobeiria —, de manera que totes dues es
refusen, tant per llegir com per escriure:

```console
$ subedit-cli --encoding UTF-16 inspect film.srt
"UTF-16" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do
```

`UTF-16LE` i `UTF-16BE` llegeixen i escriuen els mateixos bytes, marca inclosa; el que
afegeix el nom precís és que se sap quin és.

**Una marca d'ordre de bytes guanya a l'opció.** És l'única cosa que un fitxer de
subtítols declara sobre si mateix, i llegir-lo d'una altra manera que com es declara
seria obeir qui crida en contra del fitxer. La discrepància **es diu**, com a
diagnòstic:

```console
$ subedit-cli -vvv --encoding cp1252 inspect avec-bom.srt
avec-bom.srt: a byte order mark that contradicts the encoding asked for ("UTF-16LE"), settled by the reader
```

És el que fa Gaupol — torna a començar la lectura amb la codificació de
la marca —, però sense dir-ho; aquí la diferència entre el que s'ha demanat i
el que s'ha fet no és un silenci.

## Subordres

| Subordre | Què fa | Escriu? |
| :------------ | :-------------- | :----------- |
| [`inspect`](inspect.md) | informa de què està fet un fitxer, sense canviar res | no |
| [`convert`](convert.md) | escriu un fitxer en un altre format, o en una altra forma | sí |
| [`shift`](shift.md) | desplaça cada posició la mateixa quantitat | sí |
| [`transform`](transform.md) | corregeix cada posició a partir de dos punts que se sap que són correctes | sí |
| [`framerate`](framerate.md) | resincronitza un fitxer d'una freqüència de fotogrames a una altra | sí |
| [`snap`](snap.md) | mou cada marca de temps al fotograma més proper | sí |
| [`hearing-impaired`](hearing-impaired.md) | suprimeix les mencions per a persones sordes | sí |
| [`adjust`](adjust.md) | ajusta les durades a una velocitat de lectura i a uns límits | sí |
| [`replace`](replace.md) | reemplaça un text als subtítols, sense trencar cap etiqueta | sí |
| [`case`](case.md) | posa els textos en majúscula inicial a cada paraula, en majúscula de frase, en majúscules o en minúscules | sí |
| [`italics`](italics.md) | posa els textos en cursiva, o els treu la cursiva | sí |
| [`dialogue-dashes`](dialogue-dashes.md) | posa guions de diàleg, o els treu | sí |
| [`sort`](sort.md) | torna a posar els subtítols en l'ordre del seu inici | sí |
| [`join-repeats`](join-repeats.md) | fa un sol subtítol dels trossos d'un mateix text que se segueixen | sí |
| [`disjoint`](disjoint.md) | talla els subtítols que s'encavalquen perquè cap franja no s'encavalqui més amb una altra | sí |
| [`repair`](repair.md) | encadena `join-repeats`, `sort` i `disjoint` | sí |
| [`correct`](correct.md) | corregeix els textos amb els patrons de Gaupol: mencions, unió i separació de paraules, errors comuns, majúscules, divisió de línies | sí |
| [`append`](append.md) | posa fitxers després del primer, en **un sol** fitxer | sí |
| [`split-file`](split-file.md) | talla un fitxer en **dos**: un cap i una cua | sí |
| [`pair`](pair.md) | escriu una traducció a les posicions del seu fitxer principal, i diu com s'han alineat les seves línies | sí |
| [`config`](config.md) | llista, mostra i canvia la configuració de les operacions del fitxer de configuració | només el fitxer de configuració |

Hi són totes vint-i-una; l'ajuda de l'eina les llista en el mateix ordre.

**Només una no toca res**, i és la columna de la dreta: `inspect` llegeix i
informa. Les altres dinou escriuen, i el que segueix val per a disset d'elles:
[`append`](append.md) i [`split-file`](split-file.md) tenen cadascuna una aritat pròpia — N fitxers per a
**un** resultat, un fitxer per a **dos** — i per tant la seva pròpia destinació, que descriuen a la seva pàgina.
[`config`](config.md) no escriu cap fitxer de subtítols: llegeix i canvia el fitxer de configuració, i
no se li aplica res del que segueix.

## La destinació

Les disset subordres que escriuen — [`convert`](convert.md),
[`shift`](shift.md), [`transform`](transform.md), [`framerate`](framerate.md),
[`snap`](snap.md), [`hearing-impaired`](hearing-impaired.md), [`adjust`](adjust.md),
[`replace`](replace.md), [`case`](case.md), [`italics`](italics.md),
[`dialogue-dashes`](dialogue-dashes.md), [`sort`](sort.md), [`join-repeats`](join-repeats.md),
[`disjoint`](disjoint.md), [`repair`](repair.md), [`correct`](correct.md), [`pair`](pair.md) — prenen la seva destinació de la mateixa manera. ([`append`](append.md) i
[`split-file`](split-file.md) tenen la seva, a la seva pàgina.) [`inspect`](inspect.md) no escriu cap fitxer i
no accepta cap d'aquestes opcions.

**Una destinació que ja existeix se sobreescriu**, sense cap pregunta i sense cap opció per
evitar-ho: l'escriptura és atòmica — el fitxer no queda mai mig escrit, i una
escriptura interrompuda deixa intacte l'antic —, però el contingut anterior
es perd. No és un defecte que es manté: és la regla, i tornar a executar una ordre
sobre el mateix directori de sortida la torna a executar sobre si mateixa.

**No s'escriu mai res sense una destinació explícita.** Les tres maneres
d'indicar-la s'exclouen mútuament, i cal una d'elles: sense, no s'escriu res i el codi
de sortida és `1` — excepte amb [`--dry-run`](#veure-abans-descriure), que no escriu res
i per tant no en requereix cap.

| Opció | On va el resultat |
| :----- | :---------------- |
| `--output FILE` | a aquest fitxer, amb aquest nom exacte; una sola entrada |
| `--output-dir DIR` | a aquest directori, amb el nom base de l'entrada |
| `--in-place` | sobre l'entrada, amb escriptura atòmica |

Una eina que sobreescriu la seva entrada perquè no se li ha dit res és una eina que
es deixa d'usar la segona vegada. El refús costa una línia i salva el
fitxer.

`--output` amb diverses entrades es refusa: l'últim fitxer s'escriuria sobre
els anteriors. Amb un lot, `--output-dir` és l'única de les tres que té
sentit, juntament amb `--in-place`.

**El directori de sortida es crea** si falta, pares inclosos — per a
`--output-dir`, i per al directori on escriu `--output`. Es fa un cop
validada la línia d'ordres i **abans del primer fitxer**. Si no es pot crear,
es diu una vegada, no es llegeix cap fitxer, i el codi de sortida és
`2`.

**El lot es jutja sencer abans d'escriure.** Primer es calcula la destinació de cada entrada,
i se'n deriven dos refusos — errors d'ús, codi
`1`, **cap fitxer escrit** i cap directori creat:

- **dues entrades que anirien a parar al mateix fitxer** — `a/film.srt` i
  `b/film.srt` amb `--output-dir`, o `a/film.srt` i `b/film.vtt` convertits
  tots dos a WebVTT, la destinació final dels quals ha canviat d'extensió. El
  missatge anomena la destinació i totes dues entrades; canviar el nom de la segona o
  deixar guanyar l'última faria perdre un fitxer sense dir-ho, i refusar
  costa una línia;
- **una destinació que és una de les entrades**, sense `--in-place` — fins i tot
  a través de `--output-dir .`, a través de `--output` o a través d'un enllaç simbòlic. Escriure sobre
  la pròpia entrada és un gest que s'anomena.

Dos camins anomenen el mateix fitxer quan ho diu el sistema, no quan
s'escriuen igual: `in/../in/film.srt` i `in/film.srt` són un sol fitxer.

**L'extensió segueix el format escrit.** Només canvia per a
[`convert`](convert.md), l'única subordre que canvia de format; les
altres conserven el format del fitxer llegit, i per tant la seva extensió.

| Què el provoca | Missatge |
| :------------------ | :------ |
| cap destinació, sense `--dry-run` | `no destination given: use --output, --output-dir or --in-place` |
| dues destinacions | `--output, --output-dir and --in-place exclude one another` |
| `--output` en un lot | `--output names one file but several were given: use --output-dir instead` |
| dues entrades, una destinació | `<destination>: would be written by both <input> and <input>` |
| una entrada se sobreescriuria | `<destination>: would be written over the input <input>` — la mateixa frase per a [`append`](append.md) i [`split-file`](split-file.md) |
| directori de sortida que no es pot crear | `<directory>: cannot be created: permission denied`, o `: cannot be created` |
| destinació que no es pot escriure | `<path>: <destination>: cannot be written: permission denied`, o `: cannot be written` |
| caràcter que falta a la codificació escrita | `<path>: holds a character the chosen encoding cannot write` |

**L'últim mereix una frase.** Un fitxer es torna a escriure en **la codificació en què
es va llegir**, com passa amb els seus finals de línia i la seva marca. Si un
canvi hi ha introduït un caràcter que aquesta codificació no coneix — una
`ł` en Latin-1 —, l'escriptura falla i **no s'escriu res**. Reemplaçar-lo
per un `?` seria perdre text davant els ulls de qui l'acaba d'escriure.

## Veure abans d'escriure

**`--dry-run`** l'accepta cadascuna de les setze subordres que escriuen, i vol dir el mateix
a tot arreu: **llegir, calcular, informar, no escriure cap fitxer**. S'afegeix a la
línia que s'anava a executar; aplicar és tornar a executar la mateixa línia sense ella. El càlcul és
determinista: el que ha mostrat un `--dry-run` és el que escriurà la propera execució, mentre el
fitxer no hagi canviat.

- **No escriu cap fitxer ni crea cap directori**, `--in-place` inclòs: el fitxer
  d'entrada queda com estava.
- **No requereix cap destinació.** Se'n pot indicar una, i **aleshores es comprova com sense
  `--dry-run`**: dues entrades per a la mateixa destinació, una entrada que se sobreescriuria,
  `--output` en un lot, dues maneres de dir on — una línia d'ordres refusada sense `--dry-run`
  es refusa amb ella, amb la mateixa frase i el mateix codi `1`.
- **El codi de sortida és el d'una execució real**: `0` quan s'ha calculat tot, `2` o `3`
  quan hi ha fitxers que no s'han pogut llegir o processar, `1` per a l'ús. Un caràcter que
  la codificació del fitxer no podia escriure fa fallar una simulació igual que una de real.
  **Que hi hagi canvis no canvia el codi**: els quatre codis diuen si l'eina
  ha tingut èxit, no si el fitxer és conforme.
- **La narració ho diu.** On una execució real escriu `-> <destination>`, una
  simulació escriu `(dry run, nothing written)`.

<!-- example: printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by 1 --dry-run a.srt; ls -- *.srt -->
```console
$ printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by 1 --dry-run a.srt; ls -- *.srt
a.srt: 1 subtitle shifted by 1.000 s (dry run, nothing written)
a.srt
```

**En una subordre que canvia textos** —
[`hearing-impaired`](hearing-impaired.md), [`replace`](replace.md), [`case`](case.md),
[`italics`](italics.md), [`dialogue-dashes`](dialogue-dashes.md) i [`correct`](correct.md) —, **la sortida estàndard porta els
canvis proposats**. Un bloc per subtítol canviat, en l'ordre del fitxer:

```
<path>: subtitle <number>
- <the text before, line by line>
+ <the text after, line by line>
```

- el **número** és el que té el subtítol al fitxer llegit, comptant des d'u;
- cada línia del text d'abans porta `- ` al davant, cada línia del text de després `+ `: un text
  de diverses línies es llegeix tal com és, i una línia que ja comença amb un guió de
  diàleg dona `- - `;
- **un subtítol que l'operació suprimiria** ho diu a la seva primera línia,
  `<path>: subtitle <number> (removed)`, i no té línies `+`;
- un subtítol de la traducció, amb `--document translation`, s'anuncia com a `(translation)` al mateix
  lloc — vegeu [Una traducció](#una-traducció);
- la primera línia de cada bloc comença amb el camí, de manera que `grep '^film.srt: '`
  troba els blocs d'un fitxer dins d'un lot;
- **cap bloc** quan no canviaria res.

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

**La llista és la mateixa en JSON**: l'objecte del fitxer porta una matriu `changes`, vegeu
[la forma](#la-forma), amb `after` a `null` per a un subtítol suprimit. **Només es
calcula quan `--dry-run` o `--format json` la demanen**: un lot de milers de
subtítols no la construeix per res.

A **les altres subordres** — [`convert`](convert.md), [`shift`](shift.md),
[`transform`](transform.md), [`framerate`](framerate.md), [`snap`](snap.md),
[`adjust`](adjust.md), [`sort`](sort.md), [`append`](append.md), [`split-file`](split-file.md), [`pair`](pair.md) —, la sortida
estàndard porta el que portaria la d'una execució real, és a dir res en text; en
JSON, l'objecte de cada fitxer amb `"dry_run":true`, `"destination":null` i els mateixos
`counts`.

## Dues sortides, dos papers

| Sortida | Què porta |
| :----- | :--------------- |
| estàndard | **el resultat, i només el resultat** — l'informe d'`inspect`, els canvis proposats d'un [`--dry-run`](#veure-abans-descriure) de text; amb `--format json`, un objecte per fitxer — un per execució per a [`append`](append.md), [`split-file`](split-file.md) i [`pair`](pair.md) |
| d'error | **tota la resta** — la narració, els avisos, els errors |

És aquesta separació la que permet redirigir el resultat sense que hi entri el relat:

```console
$ subedit-cli inspect *.srt > rapport.txt
```

L'informe va al fitxer, la narració es queda a la pantalla.

## Sortida llegible per un script

**`--format json`** reemplaça el text de la sortida estàndard per **un objecte JSON per
fitxer d'entrada**, en el format JSON Lines: un objecte compacte per línia, en UTF-8 sense
marca, i el final de línia és `\n`. `text` és el valor per defecte i no canvia. És una
opció global, i per tant s'escriu abans de la subordre; un valor diferent de
`text` i `json` és un error d'ús, codi `1`.

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

**Un objecte per entrada, mai zero, mai dos**, tant si l'entrada ha tingut èxit com si no, en
l'ordre en què s'han indicat les entrades — o en què s'han trobat, amb [`--recursive`](lots.md). Es
compten les línies, es troben els fitxers. **Tres subordres són l'excepció, perquè la seva
aritat no és la d'un lot: [`append`](append.md) (N entrades, una sortida), [`split-file`](split-file.md)
(una entrada, dues sortides) i [`pair`](pair.md) (dos fitxers, una sortida) escriuen **un objecte per a tota
l'execució**, que anomena el fitxer que diu la seva pàgina.** **Les subordres que reescriuen, per tant, també escriuen
a la sortida estàndard**, cosa que el seu contracte no feia en text: el seu
fitxer continua sent el seu resultat, i l'objecte n'és l'informe.

**La narració no canvia.** `-q`, `-v`, `-vv` i `-vvv` actuen sobre la sortida
d'error, en text, com sense `--format`; **la sortida estàndard no varia amb ells**.
Els diagnòstics de lectura, que el text guarda per al nivell 3, són dades i són
**sempre** a `warnings`. Un error d'ús no escriu cap objecte: es detecta
abans de tocar cap fitxer, de manera que no hi ha res a què vincular-lo; va a la sortida
d'error amb el codi `1`, i la sortida estàndard queda buida.

### La forma

| Clau | Contingut | Present |
| :-- | :------ | :------- |
| `schema` | enter, la versió d'aquesta forma — `1` | sempre |
| `command` | el nom de la subordre | sempre |
| `file` | el camí **tal com s'ha indicat**, o tal com l'ha compost el recorregut d'un directori | sempre |
| `ok` | booleà | sempre |
| `error` | `{"kind", "message"}` | si `ok` és fals |
| `dry_run` | booleà: cert per a un [`--dry-run`](#veure-abans-descriure) | si `ok` és cert, en una subordre que escriu |
| `destination` | el camí escrit, o `null` quan `dry_run` és cert: no s'ha escrit res | igualment |
| `counts` | un objecte d'**enters**, propi de la subordre | igualment |
| `changes` | una matriu de `{"subtitle", "document", "before", "after"}`: els textos canviats, vegeu més avall | en una subordre que canvia textos |
| `warnings` | una matriu de `{"kind", "line"?, "detail"?, "settled"?}` | si `ok` és cert |
| `alignment` | com s'ha col·locat una traducció, vegeu [Una traducció](#una-traducció) | amb `-t`, i per a `pair` |
| `constraints` | els paràmetres utilitzats | `adjust` |
| `settings` | el fitxer de configuració que s'ha llegit, o `null` quan no se n'ha llegit cap — vegeu [El fitxer de configuració](#el-fitxer-de-configuració) | `adjust`, `correct`, si `ok` és cert |
| `inputs`, `tail` | un element per fitxer afegit; el camí de la cua | `append`; `split-file` |

`line` és absent d'un diagnòstic sobre el fitxer sencer; `detail` ho és quan no hi ha
res a afegir; `settled` és `true` quan la lectura **ha resolt** alguna cosa — una
numeració regenerada —, i és absent quan ha deixat les coses tal com estan.

**`changes`** llista, en l'ordre del fitxer, cada subtítol el text del qual canvia:
`subtitle` és el seu número al fitxer llegit, comptant des d'u; `document` és `"main"`
o `"translation"`; `before` és el text d'abans; `after` el text de després, o **`null` quan
el subtítol se suprimeix**. Els salts de línia d'un text hi són `\n`. S'escriu
tant per a una execució real com per a un [`--dry-run`](#veure-abans-descriure), i és **buida
(`[]`) quan no canvia res**: la seva presència diu que la subordre llista els seus canvis,
la seva absència que no en té.

**Cap nombre de coma flotant, enlloc.** Una posició o una durada és un enter de
**mil·lisegons** (`…_ms`); un recompte és un enter; una freqüència de fotogrames és una **cadena**
(`"25"`, `"24000/1001"`) perquè és exacta, cosa que una coma flotant racional no pot
contenir; una concentració de graella és un enter en **mil·lèsimes** — `1000` és una
graella perfecta. Aleshores dues sortides es comparen byte a byte, sense cap arrodoniment.

**Un camí que no és UTF-8** — Linux ho permet — no es pot representar en JSON: s'hi
escriu amb U+FFFD en lloc dels bytes no vàlids, i l'objecte porta l'avís
`path-not-utf8`. La fidelitat d'un camí així no es promet, i es diu.

### Què posa cada subordre a `counts`

| Subordre | Claus de `counts` |
| :------------ | :--------------- |
| [`shift`](shift.md) | `subtitles`, `shifted_by_ms` (amb signe) |
| [`transform`](transform.md) | `subtitles` |
| [`framerate`](framerate.md) | `subtitles` |
| [`snap`](snap.md) | `subtitles`, `moved` (posicions mogudes), `furthest_ms` (el moviment més gran) |
| [`hearing-impaired`](hearing-impaired.md) | `cleaned` (textos reescrits), `removed` (subtítols suprimits) |
| [`replace`](replace.md) | `replaced` (coincidències reemplaçades als subtítols canviats), `matched` (coincidències trobades) |
| [`case`](case.md), [`italics`](italics.md), [`dialogue-dashes`](dialogue-dashes.md) | `changed` (subtítols el text dels quals ha canviat) |
| [`correct`](correct.md) | `corrected` (textos canviats), `removed` (subtítols suprimits) — mai coincidències |
| [`sort`](sort.md) | `subtitles`, `moved` (els llocs que han canviat de subtítol) |
| [`adjust`](adjust.md) | `subtitles` (els subtítols afectats), `adjusted` (aquells el final dels quals s'ha mogut), després el grup `sacrificed`: `speed`, `minimum`, `gap`; i, fora de `counts`, `constraints` — els paràmetres utilitzats |
| [`convert`](convert.md) | `subtitles`, després el que ha perdut la conversió: `lost_ends` i `lost_header` (0 o 1), `joined_lines`, `lost_tags`, `lost_fields`, `furthest_ms` |
| [`append`](append.md) | `files`, `subtitles`, `appended`, després `lost_ends`, `lost_header`, `joined_lines`, `lost_tags`, `lost_fields`, `furthest_ms` sumats; i, fora de `counts`, `inputs` — un element per fitxer afegit |
| [`split-file`](split-file.md) | `subtitles`, `head`, `tail`; i, fora de `counts`, `tail` — el camí de la cua (`destination` és el cap) |
| [`pair`](pair.md) | `subtitles` (els del fitxer escrit); i, fora de `counts`, `alignment` |
| [`inspect`](inspect.md) | cap `counts`: la descripció del fitxer, vegeu la seva pàgina |

### Identificadors d'error

Un `error.kind` és un d'aquests; **un lector tracta un identificador desconegut com a
«altre»**.

| `kind` | Què és |
| :----- | :----------- |
| `not-found`, `permission-denied`, `io` | el sistema s'ha negat a llegir — o a escriure — el fitxer |
| `unknown-format`, `no-subtitle-found`, `undecodable` | el fitxer es llegeix i no es deixa llegir |
| `unencodable` | un caràcter que la codificació triada no pot escriure |
| `no-byte-order-mark`, `no-frame-rate` | el que `convert` no pot escriure sense que se li digui |
| `before-the-origin`, `beyond-the-end`, `no-transform`, `no-grid` | una operació que no es pot aplicar a aquest fitxer |
| `range-out-of-bounds` | un `--range` que aquest fitxer no conté |
| `no-style` | un format que no escriu cap estil (TMPlayer, LRC), per a `italics` |
| `refused` | qualsevol altre refús d'una operació |

### Què es promet, i què no

**Es promet mentre `schema` sigui `1`**: els noms de les claus, el seu tipus i la seva unitat; el significat
d'`ok`, `file` i `destination`; **un objecte per entrada, en l'ordre de les entrades** — un per execució per a les tres subordres amb una aritat pròpia;
els identificadors `kind` que existeixen; el determinisme — mateixes entrades, mateixos arguments,
mateixos bytes.

**No es promet**: el **text dels missatges** (`message`, `detail`), en anglès per a un humà,
que es corregeix quan cal — un script que el compara està trencat per endavant; l'**ordre de
les claus** dins d'un objecte, fix però no garantit per JSON; la presència de **claus que encara
no s'han afegit**.

**Per tant, un lector ignora una clau desconeguda**, i tracta un `kind` desconegut com a «altre»:
afegir una clau o un identificador no canvia `schema`. Suprimir o canviar el nom d'una clau,
canviar un tipus o una unitat, suprimir un identificador l'incrementa — és una ruptura,
anunciada al CHANGELOG i aquí.

## Nivells de narració

Cada nivell **conté l'anterior**: pujar un graó afegeix, mai no
reemplaça.

| Nivell | Com | Què porta la sortida d'error |
| :----- | :------ | :------------------------------ |
| 0 | `--quiet` | res, **excepte els errors** |
| 1 | per defecte, o `-v` | una línia per fitxer processat, i un resum tan bon punt n'hi ha diversos |
| 2 | `-vv` | i el que s'ha reconegut: format, codificació, BOM, finals de línia |
| 3 | `-vvv` | i la traça de depuració: bytes llegits i escrits, i **cada diagnòstic de lectura** |

**Els errors no se silencien mai, `--quiet` inclòs.** Una ordre que falla
en silenci no deixaria res més que el seu codi de sortida.

El resum només apareix a partir de dos fitxers: amb una sola entrada,
repetiria la línia anterior.

## Diagnòstics de lectura

Els formats de subtítols es llegeixen **tan bé com es pot**: davant d'una anomalia, el
lector no s'atura — decideix, o deixa les coses tal com estan, i ho diu. El que
ha trobat s'emet al **nivell 3**, una línia per diagnòstic, a la sortida
d'error:

```
a.srt: 2 diagnostics while reading
a.srt: line 6: SubRip numbers that do not follow ("7"), settled by the reader
a.srt: line 9: a line that fits nowhere, left as it stands
```

Les disset subordres els informen, no només [`inspect`](inspect.md):
un fitxer llegit tan bé com es pot i després tornat a escriure ha passat per les mateixes decisions, i callar-les
faria creure que no ha passat res.

**Un diagnòstic no és mai una fallada.** El fitxer s'ha llegit, l'ordre
ha tingut èxit, el codi de sortida és `0`. Per això viuen al nivell més
detallat: més avall, enterrarien la línia que diu el que realment s'ha
fet.

### Què porta cada línia

| Part | Què diu |
| :----- | :------------- |
| `line N` | on, comptant des d'1, com ho mostra un editor; **absent** per al que afecta el fitxer sencer |
| la frase | què s'ha trobat, entre les dotze categories de més avall |
| `("…")` | el text defectuós del fitxer, quan la categoria no n'hi ha prou; truncat a 80 bytes |
| el final | què s'ha fet al respecte: `settled by the reader`, o `left as it stands` |

**Dues categories no tenen línia**, i són les dues sobre la
codificació: els bytes s'han sospesat, o se n'ha llegit la marca, abans que existís cap
línia del fitxer. La seva línia comença, doncs, amb la frase, sense cap `line N`
al davant — no hi ha cap lloc per anomenar.

**El final de la línia és el que més compta.** `settled by the reader` vol dir
que el lector ha decidit i que el fitxer escrit porta la seva decisió — una
numeració que falta es regenera. `left as it stands` vol dir que no ha tocat
res perquè **només vosaltres podeu decidir**: un subtítol que acaba abans de
començar es queda tal com és.

### El número de línia d'un bloc

Una anomalia sobre **un bloc sencer** s'ancora a la seva línia
de temps, i no a la línia exacta que la provoca. Una numeració inconsistent
escrita a la línia 5 s'informa, doncs, a la línia 6, la de la línia de temps que
segueix — el número defectuós és a `("…")`. Només les anomalies sobre
**una línia** — una línia de temps il·legible, text abans de la primera — s'informen
sobre si mateixes.

### Les dotze categories

| Frase | Què la provoca |
| :----- | :------------------ |
| `a line that fits nowhere` | una línia que no entra en cap bloc |
| `a timing line that could not be read` | una línia de temps il·legible |
| `a SubRip block without its number` | numeració que falta, regenerada en escriure |
| `SubRip numbers that do not follow` | numeració que salta |
| `text before the first timing line` | text abans de la primera línia de temps |
| `a WebVTT block of an unknown kind` | un bloc WebVTT no reconegut |
| `declares an event column this tool cannot fill` | una columna d'esdeveniments de Sub Station Alpha que res d'aquí no omple |
| `counts in frames and states no rate; it was read at` | un fitxer en fotogrames, que no indica cap freqüència de fotogrames — la que s'ha usat és el detall |
| `carries no end times; each one was taken from the next start` | un format sense finals, TMPlayer o LRC: **cap final de la taula no ve del fitxer** |
| `more than one kind of line ending` | finals de línia barrejats |
| `an encoding nothing declared` | la codificació s'ha proposat sospesant els bytes, i no és UTF-8 |
| `a byte order mark that contradicts the encoding asked for` | el fitxer porta una marca, i `--encoding` n'ha anomenat una altra — la marca guanya |

**Les tres del mig són les de la fase 9**, i les dues últimes d'aquestes
tres no parlen d'una línia sinó del fitxer sencer: diuen que un
valor mostrat no s'ha llegit. Una columna `End` omplerta en un LRC, una
posició en un MicroDVD — el fitxer no en portava cap.

**Una operació pot afegir els seus propis**, que no són diagnòstics de lectura:
[`correct`](correct.md#un-patró-que-no-es-pot-aplicar) diu, amb el subtítol, el patró
que ha desistit. Es diu al nivell 1 i porta, a l'objecte del fitxer, l'identificador
`pattern-failed` — mai una fallada del fitxer.

## Una traducció

Un fitxer de traducció es processa **com qualsevol fitxer de subtítols**:
`subedit-cli case --to sentence film.fr.srt` el canvia sense res de nou. El que afegeix l'aparellament, i el que
diuen tres opcions, és limitat: **saber com cauen les línies d'una traducció sobre els
subtítols del fitxer principal** — això és `inspect -t` —, i **canviar la traducció mentre es manté
col·locada sobre el seu fitxer principal**, amb les mateixes regles que la finestra.

| Opció | On | Valor | Per defecte |
| :----- | :---- | :----- | :----- |
| `-t`, `--translation-file` | `inspect`, `hearing-impaired`, `replace`, `case`, `italics`, `dialogue-dashes`, `correct`, `pair` | el fitxer de traducció | — |
| `--document` | les sis subordres de text, ni `inspect` ni `pair` | `main` o `translation` | `main` |
| `--align-method` | com `-t` | `position` o `number` | `position` |

- **`position`** compara el mig de cada línia amb els límits dels subtítols: una línia que falta no
  desplaça les que la segueixen. **`number`** envia la línia *n* al subtítol *n*, sense mirar les
  posicions. La posició és el valor per defecte, pel motiu que té la finestra: una línia que falta al
  mig desplaça tota la resta d'una traducció aparellada per número.
- **`--document translation` requereix `-t`, i `-t` requereix `--document translation`** en una subordre
  de text — l'un sense l'altre és un error d'ús, codi `1`, perquè un fitxer llegit que cap gest
  no usa és una omissió i no una preferència. `-t` a `inspect` no té `--document`:
  informa. **`--align-method` sense `-t` també és un error d'ús.**
- **`-t` anomena un fitxer, i per tant només té sentit per a una sola entrada**: amb diverses entrades, o
  amb un directori pres per `--recursive`, es refusa com es refusa `--output`, **abans de llegir cap
  fitxer**. Per a un lot, executeu una invocació per parell.
- **Un fitxer de traducció idèntic al fitxer principal es refusa** — s'escrigui com s'escrigui el seu
  camí: `<translation>: the file is already open as the main document`, codi `2`.
- **Només s'escriu el document afectat**, al **seu propi camí i en el seu propi format**: el format,
  la codificació i els finals de línia del fitxer de traducció. `--output` anomena el fitxer de la
  traducció, `--output-dir` hi posa el seu nom base, `--in-place` el reescriu; **el fitxer principal
  no es toca mai**. Els camins de l'informe són els de la traducció.
- **Els subtítols que l'aparellament fa néixer** — una línia que res no ha assolit — s'escriuen a
  la traducció amb **les seves pròpies posicions**, sigui quin sigui el mètode; **un subtítol que ningú
  no ha traduït s'escriu com un bloc sense text**, que es llegeix sense cap anomalia.
- **`--encoding` i `--frame-rate` s'apliquen a tots dos fitxers.**
- Les subordres de **posició** — `shift`, `transform`, `framerate`, `snap`, `adjust` — no tenen
  aquestes opcions: doneu-los tots dos fitxers, com per a un lot,
  `subedit-cli shift --by 2 film.srt film.fr.srt --output-dir out/`.
- **Escriure la traducció resincronitzada sobre el fitxer principal** — l'única cosa que fa l'aparellament i que un fitxer sol
  no fa — és la feina de [`pair`](pair.md), que no té ni `--document` ni `--recursive`.

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

Amb `--format json`, l'objecte de cada fitxer porta també la clau `alignment`, que diu com s'ha
col·locat la traducció — `{"file", "method", "attached", "born", "untranslated", "out_of_order"}`, com
la clau `translation` d'[`inspect`](inspect.md#en-json) — i, **amb `--dry-run`**, `changes` llista
els subtítols de la traducció, cadascun amb `"document":"translation"`.

## Diversos fitxers

Totes les subordres accepten diversos camins, excepte [`append`](append.md), [`split-file`](split-file.md) i
[`pair`](pair.md), l'aritat dels quals és la que diu la seva pàgina. Cadascun es processa
de manera independent: la fallada d'un no interromp els altres, i les fallades
s'informen anomenant el fitxer i el motiu.

**Un directori representa els fitxers que conté, amb `--recursive` (`-r`)**:
sense aquesta opció, indicar-ne un és un error d'ús, codi `1`, i no es
processa res — refusar-lo és millor que no processar res i tornar `0`:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?" -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?"
films: is a directory: use --recursive
code=1
```

El recorregut, l'arbre conservat sota `--output-dir` i el que es deixa
de banda es descriuen a [Processar un arbre](lots.md).

## Les pàgines de manual

Una instal·lació posa **una pàgina de manual per subordre** al costat de `subedit-cli(1)`:
`subedit-cli-shift(1)`, `subedit-cli-convert(1)` i les altres, una per cada fila de
[la taula de subordres](#subordres). `subedit-cli(1)` conté aquest capítol — les opcions globals,
les sortides, els codis de sortida — i cada una de les altres pàgines conté el capítol de la seva subordre, amb els
exemples inclosos. La pàgina de la finestra, `subedit-gui(1)`, és al costat.

```console
$ man subedit-cli
$ man subedit-cli-shift
```

**Les pàgines no s'escriuen a part d'aquest manual**: es generen a partir d'ell quan es
compila el programa, de manera que diuen el que diu ell, i no se'n poden apartar. La secció FITXERS de cada
pàgina anomena el directori on està instal·lat el manual.

| Pàgines | On s'instal·len |
| :---- | :----------------------- |
| anglès | `share/man/man1` |
| cada llengua instal·lada — català, txec, alemany, francès | `share/man/<language>/man1` — `share/man/fr/man1` |

**`man` tria la llengua a partir de la configuració regional**, com fa el programa: amb una configuració regional francesa,
`man subedit-cli` mostra la pàgina francesa, i una llengua les pàgines de la qual no estan instal·lades rep les
angleses. `-L` demana una llengua sigui quina sigui la configuració regional:

```console
$ man -L fr subedit-cli-shift
```

## Codis de sortida

| Codi | Significat |
| :--- | :------------ |
| `0` | tot ha anat bé |
| `1` | error d'ús — opció desconeguda, valor no vàlid, combinació prohibida |
| `2` | no s'ha pogut processar cap fitxer |
| `3` | alguns fitxers s'han processat, d'altres no |

`2` i `3` es distingeixen perquè un script sàpiga com actuar sense tornar a llegir la sortida:
«no ha funcionat res» i «en falta un» no demanen la mateixa reacció.

Un error d'ús es detecta **abans de cap processament**: no deixa mai un
lot mig processat.

## Errors

| Què el provoca | Què s'escriu, a la sortida d'error |
| :------------------ | :--------------------------------------- |
| opció o subordre desconeguda | el nom defectuós, i una indicació cap a `--help` |
| `--quiet` amb `-v` | `--quiet and -v ask for opposite things; give one or the other` |
| `--config` amb `--no-config` | `--config excludes --no-config` |
| `--config` amb un nom buit | `--config needs a file name` |
| fitxer que falta | `<path>: does not exist` |
| fitxer il·legible | `<path>: cannot be opened: permission denied` |
| codificació que escriu la seva pròpia marca | `"<name>" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do` |
| bytes que no es descodifiquen | `<path>: cannot be decoded in the chosen encoding` |
| format no reconegut | `<path>: is in no format this tool knows` |
| res que sembli un subtítol | `<path>: holds nothing recognisable as a subtitle` |
| `--document translation` sense `-t` | `--document translation needs the translation file: use -t` |
| `-t` sense `--document translation` | `-t names a translation to change: use --document translation` |
| `--align-method` sense `-t` | `--align-method needs a translation file: use -t` |
| `-t` amb diverses entrades, o un directori | `-t names one file but several inputs were given: use one invocation per pair` |
| traducció idèntica al fitxer principal | `<translation>: the file is already open as the main document` |
