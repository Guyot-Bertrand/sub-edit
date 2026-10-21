# Invocation

<!-- machine-translated -->
> Ca págine es un traduction machinal e it ancor ne ha esset revidet de un lector del lingue.

```
subedit-cli [global options] <subcommand> [options] <file>...
```

Sin subcomande, li instrument scri su auxilie e se ferma.

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

## Lingue

Li linea de comande parla li lingue de su ambient: **su auxilie, su narration e su
errores** seque li lingue demandat. Li exemples de ca manual es executet in anglesi — li
blocs es generat per `make manual`, nequande copiat a mane.

**Qualmen li lingue es electet.** Li prim de `LC_ALL`, `LC_MESSAGES` e `LANG` quel es definit e
ne vacui es li locale; `LANGUAGE`, si definit, lista tande li linguas desirat, separat per
duplic punctos (`LANGUAGE=pt_BR:fr`), e es leet primmen. To es li órdine quel gettext seque, leet per
li programma self: li locale ne besona esser generat sur li machine.

| Regul | Efecte |
| :--- | :----- |
| codification e modificator | ignorat: `fr_FR.UTF-8@euro` cerca `fr_FR`, pois `fr` |
| `LC_ALL=C`, locale `C` o `POSIX`, o null locale | anglesi, e `LANGUAGE` ne es leet |
| un entrata quel nomina anglesi (`en`, `en_GB`) | anglesi: quo seque it in `LANGUAGE` ne es leet |
| un lingue de quel catalog ne es installat | li sequent demandat, e anglesi pos li ultim, **sin un parol** |
| un missage quel li catalog ne tradue | monstrat in anglesi |

**Li nómine de un modelle de correction** quel `correct` di que it ne posse aplicar seque anc li
lingue, quande li catalog save it — li modelles livrat con li programma; un modelle propri es
nominat quam su file lo scri. Li files de modelles self es nequande rescrit.

Li catalogs es installat sub `<prefix>/share/subedit/locale/<language>/LC_MESSAGES/subedit.mo`
e trovat ex li directoria del executabile self. Anglesi es integrat. Ca version installa `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR`, `ru`, `sr`, `tr` e `zh_CN` — catalan,
tchec, german, hispan, finnic, francesi, galician, hebraic, hungaric, interlingue, islandesi, italian, hollandesi, polonesi, portugalesi, portugalesi brasilian, russ, serb, turc e chinesi simplificat — traductet in plen. Francesi ha esset revidet
de un lector del lingue; catalan, tchec, german, hispan, finnic, galician, hebraic, hungaric, interlingue, islandesi, italian, hollandesi, polonesi, portugalesi, portugalesi brasilian, russ, serb, turc e chinesi simplificat es
traductet machinalmen e ancor ne revidet de un lector del lingue.
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
Sub `LANG=fr_FR.UTF-8`, `subedit-cli --help` comensa per «Lire,
inspecter et recaler des fichiers de sous-titres.» Un script quel leer li paroles del
output textu fixa `LANGUAGE=en` (o `LC_ALL=C`) — o leer `--format json`, quel nequande changea.

**Quo nequande seque li lingue** — quo un script leer:

| Nequande traductet | Pro quo |
| :--------------- | :-- |
| li output de `--format json`: claves, valores, e li `message` e `detail` de fallimentes e avises | li sam bytes in chascun lingue; li paroles es anglesi |
| li codes de exeunte | li quar de [Codes de exeunte](#codes-de-exeunte), in chascun lingue |
| li nómines tipat: subcomandes, optiones, lor valores (`off`, `srt`), metavariabiles (`FILE`, `TEXT`, `OPTIONS`) | un linea de comande scrit un vez functiona sub chascun lingue |
| li nómines de modelles de correction quel `correct --enable` e `--disable` prende | un modelle es nominat per li nómine anglesi quel su file contene |
| li nómines de formates e de codifications | un formate es nominat per su nómine |
| li diagnoses propri de CLI11 — un argument obligatori mancant, un option inconosset, un valore quel un option ne accepta — e su `Run with --help for more information.` | formulat in li analisator sur quel li linea de comande es construet; les resta in anglesi |

Demandar anglesi, quel que sia li ambient:

<!-- example: LC_ALL=C LANGUAGE=fr subedit-cli shift --output-dir out a.srt; echo "code=$?" -->
```console
$ LC_ALL=C LANGUAGE=fr subedit-cli shift --output-dir out a.srt; echo "code=$?"
shift needs --by, or --to-grid to work the amount out from the positions
code=1
```

## Optiones global

**Les es scrit ante li nómine del subcomande, e solmen ci.**
`subedit-cli --encoding cp1252 inspect film.srt` es li forme;
`subedit-cli inspect --encoding cp1252 film.srt` es refusat, con
`The following argument was not expected: --encoding`. Li sam vale por
`--version`, `-v` e `--quiet`.

**`--help` es li exception, e it ne es vermen un**:
`subedit-cli inspect --help` es acceptat pro que chascun subcomande porta su propri, e it es li
auxilie del subcomande quel it scri, ne ti del instrument.

| Option | Efecte |
| :----- | :---- |
| `-h`, `--help` | scri li auxilie e se ferma; ti del subcomande si un es nominat |
| `--version` | scri `subedit <version>`, li bibliotecas con queles li instrument functiona e li linguas installat, e se ferma — vide [`--version`](#--version) |
| `-q`, `--quiet` | nivelle 0 — null narration |
| `-v`, `-vv`, `-vvv` | nivelles 1 til 3; nivelle 1 es li predefinit |
| `--encoding NAME` | leer li files in ti codification, vice divinar it |
| `--list-encodings` | scri li codifications quel li instrument posse leer e scrir, un per linea, e **se ferma** (code `0`) — vide [`--list-encodings`](#--list-encodings) |
| `--format text\|json` | li forme del resultate sur li exeunte standard: li textu, o un objecte JSON per file — vide [Un output quel un script posse leer](#un-output-quel-un-script-posse-leer) |
| `--config FILE` | leer li parametres de operation de `FILE` vice li file de parametres del usator — vide [Li file de configuration](#li-file-de-configuration) |
| `--no-config` | leer null file de parametres: chascun parametre de operation ne donat sur li linea prende su valore predefinit integrat — vide [Li file de configuration](#li-file-de-configuration) |

`--quiet` e `-v` in li sam invocation es refusat: du intentiones contrari
ne es decidet in favor del ultim scrit.

`--config` e `--no-config` in li sam invocation es anc refusat, con
`--config excludes --no-config`, e anc `--config` con un nómine vacui, con
`--config needs a file name`: un nómine vacui nomina null file, e li file propri del usator ne es leet
vice it.

### Li file de configuration

**[`adjust`](adjust.md) e [`correct`](correct.md) leer predefinit li parametres de operation del
file de parametres** — li valores quel `Adjust Durations…` e `Correct Texts…` memora in
li fenestre, o quel [`config set`](config.md) ha scrit. Null altri subcomande leer it. Li file es:

```
$XDG_CONFIG_HOME/subedit/settings.conf
```

o `~/.config/subedit/settings.conf` quande `XDG_CONFIG_HOME` es ne definit, vacui o ne un camine
absolut — li file del fenestre, descrit in [su manual](../subedit-gui/preferences.md#li-file).

**Prioritá, li max fort primmen: li option sur li linea de comande, pois li file, pois li valore
predefinit integrat.** Un option vinci sempre: `--speed off` desactiva un rapidesse quel li file fixa. Solmen li
**bloc de operation** del file es leet; li preferenties de monstration (geometrie, columnes, tema…)
nequande. [`config list`](config.md#config-list) nomina li claves, e qual de les li linea de comande
leer: li nin claves `duration-adjust.*`, `correction.human`, `correction.ocr`,
`correction.remove-blank` e `correction.line-break.max-lines`. **Chascun de les have un option
quel supersede it por un invocation**, quam que li file lo fixa — li tabelles de
[`adjust`](adjust.md#quo-li-file-de-configuration-provide) e
[`correct`](correct.md#quo-es-selectet-e-quo-ne-es) les nomina.

**Leer li file del usator nequande fa falir un comande.** Un file mancant dona li predefinitis; un file
quel ne posse esser leet dona li predefinitis e di it; un valore quel ne posse esser leet conserva su predefinit e es nominat; un
clave inconosset es ignorat. Li file es nequande scrit per `adjust` o `correct`.

**Quo esset leet es dit**, al nivelle predefinit 1, sur li exeunte de error, ante li prim file:

| Caso | Linea |
| :--- | :--- |
| li file esset leet | `settings: read <path>` |
| un valore in it ne posse esser leet | `settings: <key>: cannot read '<value>', default kept` |
| li file del usator ne existe | `settings: <path> not found, defaults used` |
| li file ne posse esser leet | `settings: <path> unreadable, defaults used` |
| `--no-config` | nequo |

Con [`--format json`](#un-output-quel-un-script-posse-leer), li objecte de chascun file de `adjust` e
`correct` porta `settings`: li camine quel esset leet, o `null` quande null.


**`--no-config` fa un execution reproducibil**: li sam linea dona li sam resultate sur chascun
machine, quel que li usator ha fixat in li fenestre. Usa it in scripts, tests e raportes de erra — o
`--config FILE` con un file quel les accompania.

**`--config FILE` nomina un altri file**, leet in li sam manere — **except que it deve exister**. Un
nómine donat sur li linea de comande ne es un demande por li predefinitis: quande `FILE` ne existe,
`adjust` e `correct` se ferma ante que quelcunc input es leet, scri `settings file not found: <FILE>` sur
li exeunte de error, e exi con li code de usage `1`. Un `FILE` quel existe e ne posse esser leet
dona li predefinitis e di it, quam fa li file del usator. [`config show`](config.md#config-show)
monstra ancor un `FILE` mancant quam `(not found)`, e [`config set`](config.md#config-set-e-config-unset) ancor
crea it.

<!-- example: printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine.conf adjust --dry-run a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine.conf adjust --dry-run a.srt; echo "code=$?"
settings file not found: mine.conf
code=1
```

### `--version`

**Li version del programma, sol sur li prim linea** — `subedit <version>`, quo li output esset til
0.15.38 e quo un script leer. Pois **un linea per biblioteca con quel li instrument functiona**, pois li
linguas de interfacie installat, sur li exeunte standard; li instrument se ferma, con code `0`. Un subcomande donat anc
ne es executet, e `-q` e `-v` changea nequo.

<!-- example: subedit-cli --version | head -n 1 -->
```console
$ subedit-cli --version | head -n 1
subedit 0.16.0
```

Li tot, sur un machine con ICU 74.2 e Enchant 2.3.3 — li númeres es ti del machine:

```console
$ subedit-cli --version
subedit <version>
  ICU      74.2 (built with 74.2)
  Enchant  2.3.3 (built with 2.3.3)
  CLI11    2.4.1 (built with 2.4.1)
  Languages: en, fr
```

| Biblioteca | Por quo li instrument usa it |
| :------ | :------------------------ |
| `ICU` | leer e scrir li codifications, detecter les, changear li casu de littres |
| `Enchant` | li controlator de ortografie de `correct` |
| `CLI11` | leer ca linea de comande |

- **Chascun linea dona du versiones**: ti trovat durant li execution, primmen, e ti con quel li programma esset
  compilat, inter parentheses. Les diffre quande li biblioteca del machine esset actualisat pos que li
  programma esset construet — li prim cose a donar in un raporte de erra.
- **Li capites de Enchant porta null version**: ti inter parentheses es ti quel `pkg-config` ha reportat
  quande li construction esset configurat.
- **CLI11 es compilat in li programma**, su capites esset omni quo existe de it: su du versiones es
  sempre li sam.
- **Ni Qt ni libmpv**: li linea de comande ne les linka, e lista solmen quo it functiona con.
  Li fenestre les lista — vide [su manual](../subedit-gui/invocation.md#--version).
- **`Languages`** lista `en`, li lingue fonte, quel besona null catalog, pois chascun catalog
  installat sub `<prefix>/share/subedit/locale` quel es leibil, per code — li linguas quel
  `Redacter ▸ Preferenties…` oferte.
- **Quo seque li lingue**: `built with` e `Languages`. Li prim linea, li nómines, li
  númeres e li codes de lingue nequande.
- **Con `--format json`**, ante o pos `--version`, **un objecte sur un linea**: `schema`
  (`1`), `command` (`"version"`), `version`, `libraries` — un array de
  `{"name":"ICU","loaded":"74.2","built":"74.2"}`, in li órdine del textu — e `languages`, un
  array de codes. Nequo in it es traductet.

<!-- example: subedit-cli --version --format json | grep -o '"name":"[^"]*"' -->
```console
$ subedit-cli --version --format json | grep -o '"name":"[^"]*"'
"name":"ICU"
"name":"Enchant"
"name":"CLI11"
```

### `--list-encodings`

**Li codifications quel li instrument posse leer e scrir, un per linea, ordinat per nómine**, sur li exeunte standard —
pois li instrument se ferma, con code `0`, quam `--version` fa: it es li responde, ne un passu. Un
subcomande donat anc ne es executet.

<!-- example: subedit-cli --list-encodings | grep '^UTF' | head -4 -->
```console
$ subedit-cli --list-encodings | grep '^UTF' | head -4
UTF-16BE
UTF-16LE
UTF-32BE
UTF-32LE
```

- **Les es li nómines quel `--encoding` accepta**, li canonic; lor aliases (`cp1252`) es anc acceptat,
  sin esser listat.
- **Li cuntage depende del ICU installat**, e null nómine es promisset ultra ti quel chascun ICU have
  (`UTF-8`, `UTF-16LE`, `windows-1252`…). Li manual dona dunc null cuntage.
- **`UTF-16` e `UTF-32` ne es in it**: ICU les conosce, ma lor convertor scri un marca propri,
  e li modelle les refusa — vide infra. Un liste ex quel eliger ne oferte quo li campe sequent
  refusarea.
- **Con `--format json`**, **un objecte per codification**:
  `{"schema":1,"command":"list-encodings","name":"UTF-8"}`. It ne es li raporte de un file: ti
  objectes **porta ni `file` ni `ok`**, li sol exception al inviluppe de
  [li forme](#li-forme). Les es tamen un objecte per linea, un per entrata.
- **Li nivelle de narration changea nequo**: un liste es un resultate, e `-q` ne lo remove.

### `--encoding`, e quo it ne posse constrinzer

**It aplica a chascun subcomande**, pro que omni leer: un file de quel
codification es mal divinat resta mal quo que on fa con it, e un option
quel solmen `inspect` portat lassarea un desaccord sin remedie.

**Li nómine es ti de ICU, e omni su aliases es acceptat**: `cp1252`,
`windows-1252` e `WINDOWS 1252` nomina li sam codification. Li ensemble ne es scrit
in null loc: it es ti quel li ICU del machine posse converter, e
[`--list-encodings`](#--list-encodings) lo lista. Un nómine quel it ne posse converter es
refusat ante que un sol file es leet:

```console
$ subedit-cli --encoding klingon-1 inspect film.srt
no encoding is named "klingon-1"
```

**Un exception, e solmen un: li codification deve nominar su órdine de bytes.**
`UTF-16` e `UTF-32` es nómines quel ICU conosce e de queles li convertor scri **un marca
propri**. Tal codification ne posse viver in ca modelle — li marca cessa
esser un electe, `--no-bom` esset acceptat e disobedit — dunc ambi es
refusat, por lectura quam por scrition:

```console
$ subedit-cli --encoding UTF-16 inspect film.srt
"UTF-16" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do
```

`UTF-16LE` e `UTF-16BE` leer e scri li sam bytes, marca includet; quo
li nómine precis adjunte es que on save qual.

**Un marca de órdine de bytes vinci li option.** It es li sol cose quel un file de
subtitules declara pri se self, e leer it altrimen quam it declara se
esset obedir al apelator contra li file. Li desaccord es **dit**, quam un
diagnose:

```console
$ subedit-cli -vvv --encoding cp1252 inspect avec-bom.srt
avec-bom.srt: a byte order mark that contradicts the encoding asked for ("UTF-16LE"), settled by the reader
```

To es quo Gaupol fa — it recomensa li lectura con li codification de
li marca — ma sin dir it; ci li diferentie inter quo esset demandat e
quo esset fat ne es un silentie.

## Subcomandes

| Subcomande | Quo it fa | Es it scri? |
| :------------ | :-------------- | :----------- |
| [`inspect`](inspect.md) | reporta de quo un file es fat, sin changear nequo | no |
| [`convert`](convert.md) | scri un file in un altri formate, o un altri forme | yes |
| [`shift`](shift.md) | move chascun position per li sam quantitá | yes |
| [`transform`](transform.md) | corecte chascun position ex du punctus conosset quam just | yes |
| [`framerate`](framerate.md) | retemporisa un file de un frequentie de fotogrammas a un altri | yes |
| [`snap`](snap.md) | move chascun marca de tempore sur li fotogramma li plu proxim | yes |
| [`hearing-impaired`](hearing-impaired.md) | remove li mentiones por surdes | yes |
| [`adjust`](adjust.md) | ajusta li durationes a un rapidesse de lectura e a limites | yes |
| [`replace`](replace.md) | substitue un textu in li subtitules, sin rupter un etiquette | yes |
| [`case`](case.md) | mette li textus in majusculas de titul, de frase, MAJUSCULES o minuscules | yes |
| [`italics`](italics.md) | mette li textus in italic, o remove lor italic | yes |
| [`dialogue-dashes`](dialogue-dashes.md) | mette strecs de dialog, o les remove | yes |
| [`sort`](sort.md) | remette li subtitules in li órdine de lor comense | yes |
| [`join-repeats`](join-repeats.md) | fa un sol subtitul ex li pezzes de un textu queles sequa se | yes |
| [`disjoint`](disjoint.md) | decupa li subtitules superponent se por que null intervallo se superpone plu a un altri | yes |
| [`repair`](repair.md) | encatena `join-repeats`, `sort` e `disjoint` | yes |
| [`correct`](correct.md) | corecte li textus con li modelles de Gaupol: mentiones, unir e separar paroles, errores comun, majusculisation, ruptura de linea | yes |
| [`append`](append.md) | mette files pos li prim, in **un sol** file | yes |
| [`split-file`](split-file.md) | separa un file in **du**: un cap e un cauda | yes |
| [`pair`](pair.md) | scri un traduction a-presso li positiones de su file principal, e di qualmen su lineas se alliniat | yes |
| [`config`](config.md) | lista, monstra e changea li parametres de operation del file de parametres | solmen li file de parametres |

Omni vint-un es ci; li auxilie del instrument les lista in li sam órdine.

**Solmen un tange nequo**, e to es li columne a dextre: `inspect` leer e
reporta. Li altri dece-nin scri, e quo seque vale por dece-sett de les:
[`append`](append.md) e [`split-file`](split-file.md) have chascun un aritá propri — N files por
**un** resultate, un file por **du** — e dunc lor propri destination, quel ili descrie sur lor págine.
[`config`](config.md) scri null file de subtitules: it leer e changea li file de parametres, e
nequo de quo seque aplica a it.

## Li destination

Li dece-sett subcomandes quel scri — [`convert`](convert.md),
[`shift`](shift.md), [`transform`](transform.md), [`framerate`](framerate.md),
[`snap`](snap.md), [`hearing-impaired`](hearing-impaired.md), [`adjust`](adjust.md),
[`replace`](replace.md), [`case`](case.md), [`italics`](italics.md),
[`dialogue-dashes`](dialogue-dashes.md), [`sort`](sort.md), [`join-repeats`](join-repeats.md),
[`disjoint`](disjoint.md), [`repair`](repair.md), [`correct`](correct.md), [`pair`](pair.md) — prende lor destination in li sam manere. ([`append`](append.md) e
[`split-file`](split-file.md) have li lor propri, sur lor págine.) [`inspect`](inspect.md) scri null file e
accepta null de ca optiones.

**Un destination quel ja existe es scrit sur**, sin question e sin option por
evitar it: li scrition es atomic — li file es nequande scrit a mitá, e un
scrition interrupet lassa li old intact —, ma li contenete precedent es
perdit. To ne es un defecte conservat: it es li regul, e executar un comande
denov sur li sam directoria de output executa it denov sur se self.

**Nequo es nequande scrit sin un destination explicit.** Li tri manieres de
donar it se exclude, e un es besonat: sin it, nequo es scrit e li code de
exeunte es `1` — except con [`--dry-run`](#vider-ante-scrir), quel scri nequo
e dunc besona null.

| Option | U li resultate vade |
| :----- | :---------------- |
| `--output FILE` | in ti file, sub ti nómine exact; un sol input |
| `--output-dir DIR` | in ti directoria, sub li nómine de base del input |
| `--in-place` | sur li input, per scrition atomic |

Un instrument quel scri sur su input pro que nequo li ha dit es un instrument quel on
cessa usar li duesim vez. Li refusa costa un linea e salva li
file.

`--output` con plu inputs es refusat: li ultim file va esser scrit sur
li precedent. Con un lot, `--output-dir` es li sol del tri quel have
sens, con `--in-place`.

**Li directoria de output es creat** si it manca, parentes includet — por
`--output-dir`, e por li directoria in quel `--output` scri. To es fat quande li
linea de comande es validat e **ante li prim file**. Si it ne posse esser creat,
it es dit un vez, null file es leet, e li code de exeunte es
`2`.

**Li lot es judicat in total ante scrir.** Li destination de chascun input
es calculat primmen, e du refusas sequa: errores de usage, code
`1`, **null file scrit** e null directoria creat:

- **du inputs quel finiria in li sam file** — `a/film.srt` e
  `b/film.srt` con `--output-dir`, o `a/film.srt` e `b/film.vtt` ambi
  convertet a WebVTT, de quel destination final changea extension. Li
  missage nomina li destination e ambi inputs; renominar li duesim o
  lassar li ultim vincer perderea un file sin dir it, e refusar
  costa un linea;
- **un destination quel es un del inputs**, sin `--in-place` — includent
  per `--output-dir .`, per `--output` o per un ligament simbolic. Scrir sur
  su input es un gesto quel on nomina.

Du camines nomina li sam file quande li sistema lo di, ne quande les es
scrit egalmen: `in/../in/film.srt` e `in/film.srt` es un sol file.

**Li extension seque li formate scrit.** It changea solmen por
[`convert`](convert.md), li sol subcomande quel changea formate; li
altres conserva li formate del file leet, e dunc su extension.

| Quo it causa | Missage |
| :------------------ | :------ |
| null destination, sin `--dry-run` | `no destination given: use --output, --output-dir or --in-place` |
| du destinationes | `--output, --output-dir and --in-place exclude one another` |
| `--output` sur un lot | `--output names one file but several were given: use --output-dir instead` |
| du inputs, un destination | `<destination>: would be written by both <input> and <input>` |
| un input esset scrit sur | `<destination>: would be written over the input <input>` — li sam frase por [`append`](append.md) e [`split-file`](split-file.md) |
| directoria de output quel ne posse esser creat | `<directory>: cannot be created: permission denied`, o `: cannot be created` |
| destination quel ne posse esser scrit | `<path>: <destination>: cannot be written: permission denied`, o `: cannot be written` |
| character mancant in li codification scrit | `<path>: holds a character the chosen encoding cannot write` |

**Li ultim merita un frase.** Un file es scrit retro in **li codification in quel it
esset leet**, quam it es con su fines de linea e su marca. Si un
change ha portat in it un character quel ti codification ne conosce — un
`ł` in Latin-1 —, li scrition falie e **nequo es scrit**. Substituer it
per un `?` esset perder textu davant li ocules de qui just lo ha scrit.

## Vider ante scrir

**`--dry-run`** es acceptat per chascun del dece-six subcomandes quel scri, e significa li sam
cose omni-u: **leer, calcular, reportar, scrir null file**. On lo adjunte al
linea quel on esset pro executar; aplicar es executar li sam linea denov sin it. Li calcul es
determinist: quo un `--dry-run` ha monstrat es quo li sequent execution va scrir, til que li
file ha changeat.

- **It scri null file e crea null directoria**, `--in-place` includet: li file
  de input resta quam it esset.
- **It besona null destination.** Un posse esser donat, e **it es tande controlat quam sin
  `--dry-run`**: du inputs por li sam destination, un input quel esset scrit sur,
  `--output` sur un lot, du manieres de dir u — un linea de comande refusat sin `--dry-run`
  es refusat con it, per li sam frase e li sam code `1`.
- **Li code de exeunte es ti de un execution real**: `0` quande omni esset calculat, `2` o `3`
  quande files ne posse esser leet o tractat, `1` por usage. Un character quel
  li codification del file ne posse scrir fa falir un dry run quam it fa falir un real.
  **Que changes existe ne changea li code**: li quar codes di si li instrument
  ha succedet, ne si li file es conform.
- **Li narration di it.** U un execution real scri `-> <destination>`, un dry
  run scri `(dry run, nothing written)`.

<!-- example: printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by 1 --dry-run a.srt; ls -- *.srt -->
```console
$ printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by 1 --dry-run a.srt; ls -- *.srt
a.srt: 1 subtitle shifted by 1.000 s (dry run, nothing written)
a.srt
```

**Sur un subcomande quel changea textus** —
[`hearing-impaired`](hearing-impaired.md), [`replace`](replace.md), [`case`](case.md),
[`italics`](italics.md), [`dialogue-dashes`](dialogue-dashes.md) e [`correct`](correct.md) —, **li exeunte standard porta li
changes proposit**. Un bloc per subtitul changeat, in li órdine del file:

```
<path>: subtitle <number>
- <the text before, line by line>
+ <the text after, line by line>
```

- li **númere** es ti quel li subtitul have in li file leet, contat de un;
- chascun linea del textu ante porta `- ` ante it, chascun linea del textu pos `+ `: un textu
  de plu lineas leer quam it es, e un linea quel self comensa per un strec de
  dialog dona `- - `;
- **un subtitul quel li operation va remover** di it sur su prim linea,
  `<path>: subtitle <number> (removed)`, e have null lineas `+`;
- un subtitul del traduction, con `--document translation`, se anuncia `(translation)` al sam
  loc — vide [Un traduction](#un-traduction);
- li prim linea de chascun bloc comensa per li camine, por que `grep '^film.srt: '`
  trova li blocs de un file in un lot;
- **null bloc** quande nequo va changear.

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

**Li liste es li sam in JSON**: li objecte del file porta un array `changes`, vide
[li forme](#li-forme), con `after` fixat a `null` por un subtitul removet. **It es solmen
calculat quande `--dry-run` o `--format json` lo demanda**: un lot de millies de
subtitules ne lo constructe por nequo.

Sur **li altri subcomandes** — [`convert`](convert.md), [`shift`](shift.md),
[`transform`](transform.md), [`framerate`](framerate.md), [`snap`](snap.md),
[`adjust`](adjust.md), [`sort`](sort.md), [`append`](append.md), [`split-file`](split-file.md), [`pair`](pair.md) —, li exeunte
standard porta quo ti de un execution real portarea, to es nequo in textu; in
JSON, li objecte de chascun file con `"dry_run":true`, `"destination":null` e li sam
`counts`.

## Du outputs, du roles

| Output | Quo it porta |
| :----- | :--------------- |
| standard | **li resultate, e solmen li resultate** — li raporte de `inspect`, li changes proposit de un textu [`--dry-run`](#vider-ante-scrir); con `--format json`, un objecte per file — un per execution por [`append`](append.md), [`split-file`](split-file.md) e [`pair`](pair.md) |
| error | **omni li resto** — li narration, li avises, li errores |

It es ca division quel permisse redirecter li resultate sin obtener li historie in it:

```console
$ subedit-cli inspect *.srt > rapport.txt
```

Li raporte vade al file, li narration resta sur li ecran.

## Un output quel un script posse leer

**`--format json`** remplacia li textu del exeunte standard per **un objecte JSON per
file de input**, in li formate JSON Lines: un objecte compact per linea, in UTF-8 sin
marca, li fine de linea es `\n`. `text` es li predefinit e ne changea. It es un
option global, e es dunc scrit ante li subcomande; un valore altri quam
`text` e `json` es un error de usage, code `1`.

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

**Un objecte per input, nequande null, nequande du**, que li input succedet o ne, in
li órdine in quel li inputs esset donat — o trovat, con [`--recursive`](lots.md). On
cunta li lineas, on trova su files. **Tri subcomandes es li exception, pro que lor
aritá ne es ti de un lot: [`append`](append.md) (N inputs, un output), [`split-file`](split-file.md)
(un input, du outputs) e [`pair`](pair.md) (du files, un output) scri **un objecte por li tot
execution**, quel nomina li file quel lor págine di.** **Li subcomandes quel rescri dunc anc scri
sur li exeunte standard**, quo lor contract ne fat in textu: lor
file resta lor resultate, e li objecte es li raporte de it.

**Li narration ne changea.** `-q`, `-v`, `-vv` e `-vvv` actua sur li exeunte de
error, in textu, quam sin `--format`; **li exeunte standard ne varia con les**.
Li diagnoses de lectura, queles li textu conserva al nivelle 3, es data e es
**sempre** in `warnings`. Un error de usage scri null objecte: it es detectet
ante que quelcunc file es tangit, dunc ne hay a quo atachar it; it vade al exeunte de
error con code `1`, e li exeunte standard resta vacui.

### Li forme

| Clave | Contenete | Presente |
| :-- | :------ | :------- |
| `schema` | integre, li version de ca forme — `1` | sempre |
| `command` | li nómine del subcomande | sempre |
| `file` | li camine **quam donat**, o quam li promenada de un directoria lo ha composit | sempre |
| `ok` | boolean | sempre |
| `error` | `{"kind", "message"}` | si `ok` es false |
| `dry_run` | boolean: true por un [`--dry-run`](#vider-ante-scrir) | si `ok` es true, sur un subcomande quel scri |
| `destination` | li camine scrit, o `null` quande `dry_run` es true: nequo esset scrit | sam |
| `counts` | un objecte de **integres**, specific al subcomande | sam |
| `changes` | un array de `{"subtitle", "document", "before", "after"}`: li textus changeat, vide infra | sur un subcomande quel changea textus |
| `warnings` | un array de `{"kind", "line"?, "detail"?, "settled"?}` | si `ok` es true |
| `alignment` | qualmen un traduction esset posit, vide [Un traduction](#un-traduction) | con `-t`, e por `pair` |
| `constraints` | li parametres usat | `adjust` |
| `settings` | li file de parametres quel esset leet, o `null` quande null — vide [Li file de configuration](#li-file-de-configuration) | `adjust`, `correct`, si `ok` es true |
| `inputs`, `tail` | un element per file apendet; li camine del cauda | `append`; `split-file` |

`line` es absent de un diagnose pri li tot file; `detail` es absent quande ne hay
nequo a adjunter; `settled` es `true` quande li lectura **ha decidet** alquo — un
numeration regenerat —, e es absent quande it ha lassat li cosas quam les es.

**`changes`** lista, in li órdine del file, chascun subtitul de quel textu changea:
`subtitle` es su númere in li file leet, contat de un; `document` es `"main"`
o `"translation"`; `before` es li textu ante; `after` li textu pos, o **`null` quande
li subtitul es removet**. Li ruptures de linea de un textu es `\n` in it. It es
scrit por un execution real quam por un [`--dry-run`](#vider-ante-scrir), e **vacui
(`[]`) quande nequo changea**: su presentie di que li subcomande lista su changes,
su absentie que it have null.

**Null númere a virgul flotant, in null loc.** Un position o un duration es un integre de
**millisecundes** (`…_ms`); un cuntage es un integre; un frequentie de fotogrammas es un **catene**
(`"25"`, `"24000/1001"`) pro que it es exact, quo un virgul flotant rational ne posse
tener; un concentration de grille es un integre in **millesimes** — `1000` es un
grille perfect. Du outputs se compara tande byte per byte, sin arrotundation.

**Un camine quel ne es UTF-8** — Linux lo permisse — ne posse esser representat in JSON: it
es scrit con U+FFFD vice li bytes invalid, e li objecte porta li avise
`path-not-utf8`. Li fidelitá de tal camine ne es promisset, e to es dit.

### Quo chascun subcomande pone in `counts`

| Subcomande | Claves de `counts` |
| :------------ | :--------------- |
| [`shift`](shift.md) | `subtitles`, `shifted_by_ms` (con signe) |
| [`transform`](transform.md) | `subtitles` |
| [`framerate`](framerate.md) | `subtitles` |
| [`snap`](snap.md) | `subtitles`, `moved` (positiones movet), `furthest_ms` (li max grand movement) |
| [`hearing-impaired`](hearing-impaired.md) | `cleaned` (textus rescrit), `removed` (subtitules removet) |
| [`replace`](replace.md) | `replaced` (concordanties substituet in li subtitules changeat), `matched` (concordanties trovat) |
| [`case`](case.md), [`italics`](italics.md), [`dialogue-dashes`](dialogue-dashes.md) | `changed` (subtitules de quel textu changeat) |
| [`correct`](correct.md) | `corrected` (textus changeat), `removed` (subtitules removet) — nequande concordanties |
| [`sort`](sort.md) | `subtitles`, `moved` (li locs quel changeat de subtitul) |
| [`adjust`](adjust.md) | `subtitles` (li subtitules cielat), `adjusted` (ti de quel fin movet), pois li gruppe `sacrificed`: `speed`, `minimum`, `gap`; e, foris `counts`, `constraints` — li parametres usat |
| [`convert`](convert.md) | `subtitles`, pois quo li conversion ha perdit: `lost_ends` e `lost_header` (0 o 1), `joined_lines`, `lost_tags`, `lost_fields`, `furthest_ms` |
| [`append`](append.md) | `files`, `subtitles`, `appended`, pois `lost_ends`, `lost_header`, `joined_lines`, `lost_tags`, `lost_fields`, `furthest_ms` summat; e, foris `counts`, `inputs` — un element per file apendet |
| [`split-file`](split-file.md) | `subtitles`, `head`, `tail`; e, foris `counts`, `tail` — li camine del cauda (`destination` es li cap) |
| [`pair`](pair.md) | `subtitles` (ti del file scrit); e, foris `counts`, `alignment` |
| [`inspect`](inspect.md) | null `counts`: li descrition del file, vide su págine |

### Identificatores de error

Un `error.kind` es un de ti-ci; **un lector tracta un identificator inconosset quam
«other»**.

| `kind` | Quo it es |
| :----- | :----------- |
| `not-found`, `permission-denied`, `io` | li sistema ha refusat leer — o scrir — li file |
| `unknown-format`, `no-subtitle-found`, `undecodable` | li file es leet e ne se lassa leer |
| `unencodable` | un character quel li codification electet ne posse scrir |
| `no-byte-order-mark`, `no-frame-rate` | quo `convert` ne posse scrir sin que on lo di |
| `before-the-origin`, `beyond-the-end`, `no-transform`, `no-grid` | un operation quel ne posse aplicar a ca file |
| `range-out-of-bounds` | un `--range` quel ca file ne contene |
| `no-style` | un formate quel scri null stil (TMPlayer, LRC), por `italics` |
| `refused` | quelcunc altri refusa de un operation |

### Quo es promisset, e quo ne es

**Promisset til que `schema` es `1`**: li nómines del claves, lor tip e lor unité; li signification
de `ok`, `file` e `destination`; **un objecte per input, in li órdine del inputs** — un per execution por li tri subcomandes con un aritá propri;
li identificatores `kind` quel existe; determinism — li sam inputs, li sam arguments,
li sam bytes.

**Ne promisset**: li **textu del missages** (`message`, `detail`), anglesi por un human,
corecet quande it deve esser — un script quel compara it es rupt ante tempore; li **órdine del
claves** in un objecte, fixat ma ne garantit per JSON; li presentie de **claves ancor ne
adjuntet**.

**Un lector dunc ignora un clave inconosset**, e tracta un `kind` inconosset quam «other»:
adjunter un clave o un identificator ne changea `schema`. Remover o renominar un clave,
changear un tip o un unité, remover un identificator lo incrementa — to es un ruptura,
anunciat in li CHANGELOG e ci.

## Nivelles de narration

Chascun nivelle **contene li precedent**: munter un gradu adjunte, nequande
remplacia.

| Nivelle | Qualmen | Quo li exeunte de error porta |
| :----- | :------ | :------------------------------ |
| 0 | `--quiet` | nequo, **except li errores** |
| 1 | predefinit, o `-v` | un linea per file tractat, e un summarie tam tost que hay plu files |
| 2 | `-vv` | e quo esset recognosset: formate, codification, BOM, fines de linea |
| 3 | `-vvv` | e li traca de debuggage: bytes leet e scrit, e **chascun diagnose de lectura** |

**Li errores es nequande tacet, `--quiet` includet.** Un comande quel falie
in silentie ne lassarea ma su code de exeunte.

Li summarie apare solmen de du files: sur un sol input, it
repetirea li linea ante it.

## Diagnoses de lectura

Li formates de subtitules es leet **tam bon quam possibil**: davant un anomalie, li
lector ne se ferma — it decide, o it lassa li cosas quam les es, e it di it. Quo
it ha incontrat es exeunte al **nivelle 3**, un linea per diagnose, sur li exeunte de
error:

```
a.srt: 2 diagnostics while reading
a.srt: line 6: SubRip numbers that do not follow ("7"), settled by the reader
a.srt: line 9: a line that fits nowhere, left as it stands
```

Chascun del dece-sett subcomandes les reporta, ne solmen [`inspect`](inspect.md):
un file leet tam bon quam possibil e pois scrit retro ha passat tra li sam decisiones, e tacer les
lassarea creder que nequo evenit.

**Un diagnose es nequande un fallimente.** Li file esset leet, li comande
succedet, li code de exeunte es `0`. To es pro quo les vive al nivelle li plu
detaliat: plu bass, les sepulterea li linea quel di quo esset vermen
fat.

### Quo chascun linea porta

| Parte | Quo it di |
| :----- | :------------- |
| `line N` | u, contat de 1, quam un redactor lo monstra; **absent** por quo concerne li tot file |
| li frase | quo esset incontrat, inter li dudece categories infra |
| `("…")` | li textu defectiv del file, quande li categorie ne basta; truncat a 80 bytes |
| li fine | quo esset fat pri it: `settled by the reader`, o `left as it stands` |

**Du categories have null linea**, e les es li du pri li
codification: li bytes esset pesat, o lor marca leet, ante que un sol
linea del file existet. Lor linea dunc comensa per li frase, sin `line N`
ante it — ne hay loc a nominar.

**Li fine del linea es quo importa max.** `settled by the reader` significa
que li lector ha decidet e que li file scrit porta su decision — un
numeration mancant es regenerat. `left as it stands` significa que it ha tangit
nequo pro que **solmen vu posse decider**: un subtitul quel fini ante que it
comensa resta quam it es.

### Li numere de linea de un bloc

Un anomalie pri **un bloc integri** es ancorat sur su linea de
temporisation, e ne sur li linea exact quel lo causa. Un numeration inconsistent
scrit sur linea 5 es dunc reportat sur linea 6, ti del linea de temporisation quel
seque — li númere erronic es in li `("…")`. Solmen li anomalies pri
**un linea** — un linea de temporisation illegibil, textu ante li prim — es
reportat sur se self.

### Li dudece categories

| Frase | Quo it causa |
| :----- | :------------------ |
| `a line that fits nowhere` | un linea quel intra in null bloc |
| `a timing line that could not be read` | un linea de temporisation illegibil |
| `a SubRip block without its number` | numeration mancant, regenerat durant li scrition |
| `SubRip numbers that do not follow` | numeration quel saltea |
| `text before the first timing line` | textu ante li prim linea de temporisation |
| `a WebVTT block of an unknown kind` | un bloc WebVTT ne recognosset |
| `declares an event column this tool cannot fill` | un columne de eventu de Sub Station Alpha quel nequo ci plenia |
| `counts in frames and states no rate; it was read at` | un file in fotogrammas, quel declara null frequentie de fotogrammas — ti usat es li detail |
| `carries no end times; each one was taken from the next start` | un formate sin fines, TMPlayer o LRC: **null fin in li tabelle veni del file** |
| `more than one kind of line ending` | fines de linea mixtat |
| `an encoding nothing declared` | li codification esset proposit per pesar li bytes, e it ne es UTF-8 |
| `a byte order mark that contradicts the encoding asked for` | li file porta un marca, e `--encoding` ha nominat un altri — li marca vinci |

**Li tri in li medie es ti del fase 9**, e li ultim du de ti tri
ne parla de un linea ma del tot file: ili di que un
valore monstrat ne esset leet. Un columne `End` plenit sur un LRC, un
position sur un MicroDVD — li file portat nequo de to.

**Un operation posse adjunter su propri**, quel ne es un diagnose de lectura:
[`correct`](correct.md#un-modelle-quel-ne-posse-esser-aplicat) di, con li subtitul, li modelle
quel ha cedet. It es dit al nivelle 1 e porta, in li objecte del file, li identificator
`pattern-failed` — nequande un fallimente del file.

## Un traduction

Un file de traduction es tractat **quam quelcunc file de subtitules**:
`subedit-cli case --to sentence film.fr.srt` lo changea sin nequo de nov. Quo li parigation adjunte, e quo
tri optiones di, es streit: **saver qualmen li lineas de un traduction cade sur li
subtitules del file principal** — to es `inspect -t` —, e **changear li traduction durant que on lo tene
posit sur su file principal**, con li sam regules quam li fenestre.

| Option | U | Valore | Predefinit |
| :----- | :---- | :----- | :----- |
| `-t`, `--translation-file` | `inspect`, `hearing-impaired`, `replace`, `case`, `italics`, `dialogue-dashes`, `correct`, `pair` | li file de traduction | — |
| `--document` | li sis subcomandes de textu, ne `inspect` ni `pair` | `main` o `translation` | `main` |
| `--align-method` | quam `-t` | `position` o `number` | `position` |

- **`position`** compara li medie de chascun linea con li limites del subtitules: un linea mancant ne
  displacia ti quel seque. **`number`** invia li linea *n* al subtitul *n*, sin regardar li
  positiones. Position es li predefinit, por li rason quel li fenestre have: un linea mancant in li
  medie displacia tot li resto de un traduction parigat per númere.
- **`--document translation` besona `-t`, e `-t` besona `--document translation`** sur un subcomande
  de textu — un sin li altri es un error de usage, code `1`, pro que un file leet quel null gesto
  usa es un omission e ne un preferentie. `-t` sur `inspect` have null `--document`: it
  reporta. **`--align-method` sin `-t` es anc un error de usage.**
- **`-t` nomina un file, e dunc have sens solmen por un sol input**: con plu inputs, o
  con un directoria prendet per `--recursive`, it es refusat quam `--output` es, **ante que un file es
  leet**. Por un lot, executa un invocation per par.
- **Un file de traduction identic al file principal es refusat** — quam que su
  camine es scrit: `<translation>: the file is already open as the main document`, code `2`.
- **Solmen li document cielat es scrit**, a **su propri camine e in su propri formate**: li formate,
  li codification e li fines de linea del file de traduction. `--output` nomina li file del
  traduction, `--output-dir` mette ibi su nómine de base, `--in-place` lo rescri; **li file
  principal es nequande tangit**. Li camines del raporte es ti del traduction.
- **Li subtitules quel li parigation fa naster** — un linea quel nequo atinget — es scrit in
  li traduction con **lor propri positiones**, quel que li metode; **un subtitul quel nequi
  ha traductet es scrit quam un bloc sin textu**, quel leer sin anomalie.
- **`--encoding` e `--frame-rate` aplica a ambi files.**
- Li subcomandes de **position** — `shift`, `transform`, `framerate`, `snap`, `adjust` — ne have
  ca optiones: dona les ambi files, quam por un lot,
  `subedit-cli shift --by 2 film.srt film.fr.srt --output-dir out/`.
- **Scrir li traduction retemporisat sur li file principal** — li sol cose quel li parigation fa e quel un file
  sol ne fa — es li labor de [`pair`](pair.md), quel have ni `--document` ni `--recursive`.

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

Con `--format json`, li objecte de chascun file porta anc li clave `alignment`, quel di qualmen li
traduction esset posit — `{"file", "method", "attached", "born", "untranslated", "out_of_order"}`, quam
li clave `translation` de [`inspect`](inspect.md#in-json) — e, **con `--dry-run`**, `changes` lista
li subtitules del traduction, chascun con `"document":"translation"`.

## Plu files

Chascun subcomande accepta plu camines, except [`append`](append.md), [`split-file`](split-file.md) e
[`pair`](pair.md), de queles li aritá es ti de lor págine. Chascun es tractat
independentmen: li fallimente de un ne interrupte li altres, e li fallimentes
es reportat con li nómine del file e li rason.

**Un directoria representa li files quel it contene, con `--recursive` (`-r`)**:
sin ca option, donar un es un error de usage, code `1`, e nequo es
tractat — refusar it vale mey quam tractar nequo e retornar `0`:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?" -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?"
films: is a directory: use --recursive
code=1
```

Li promenada, li arbore conservat sub `--output-dir` e quo es lassat
a-parte es descrit in [Tractar un arbore](lots.md).

## Li págines de manual

Un installation mette **un págine de manual per subcomande** apu `subedit-cli(1)`:
`subedit-cli-shift(1)`, `subedit-cli-convert(1)` e li altres, un por chascun rangea del
[tabelle de subcomandes](#subcomandes). `subedit-cli(1)` contene ca capitul — li optiones global,
li outputs, li codes de exeunte — e chascun altri págine contene li capitul de su subcomande, su
exemples includet. Li págine del fenestre, `subedit-gui(1)`, es apu les.

```console
$ man subedit-cli
$ man subedit-cli-shift
```

**Li págines ne es scrit separatmen de ca manual**: les es generat ex it quande li
programma es construet, dunc les di quo it di, e ne posse divergir de it. Li section FILES de chascun
págine nomina li directoria in quel li manual es installat.

| Págines | U les es installat |
| :---- | :----------------------- |
| anglesi | `share/man/man1` |
| chascun lingue installat — catalan, tchec, german, hispan, finnic, francesi, galician, hebraic, hungaric, interlingue, islandesi, italian, hollandesi, polonesi, portugalesi, portugalesi brasilian, russ, serb, turc, chinesi simplificat | `share/man/<language>/man1` — `share/man/fr/man1` |

**`man` elige li lingue ex li locale**, quam li programma fa: sub un locale francesi,
`man subedit-cli` monstra li págine francesi, e un lingue de quel págines ne es installat obtene
les anglesi. `-L` demanda un lingue quel que li locale:

```console
$ man -L fr subedit-cli-shift
```

## Codes de exeunte

| Code | Signification |
| :--- | :------------ |
| `0` | omni ha succedet |
| `1` | error de usage — option inconosset, valore invalid, combination interdit |
| `2` | null file ha posset esser tractat |
| `3` | alcun files esset tractat, altres no |

`2` e `3` es distinet por que un script save qualmen actuar sin releer li output:
«nequo functionat» e «un manca» ne vocat li sam reaction.

Un error de usage es detectet **ante quelcunc tractation**: it nequande lassa un
lot tractat a mitá.

## Errores

| Quo it causa | Quo es scrit, sur li exeunte de error |
| :------------------ | :--------------------------------------- |
| option o subcomande inconosset | li nómine erronic, e un indication a `--help` |
| `--quiet` con `-v` | `--quiet and -v ask for opposite things; give one or the other` |
| `--config` con `--no-config` | `--config excludes --no-config` |
| `--config` con un nómine vacui | `--config needs a file name` |
| file mancant | `<path>: does not exist` |
| file illegibil | `<path>: cannot be opened: permission denied` |
| codification quel scri su propri marca | `"<name>" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do` |
| bytes quel ne decodar se | `<path>: cannot be decoded in the chosen encoding` |
| formate ne recognoscet | `<path>: is in no format this tool knows` |
| nequo quel sembla un subtitul | `<path>: holds nothing recognisable as a subtitle` |
| `--document translation` sin `-t` | `--document translation needs the translation file: use -t` |
| `-t` sin `--document translation` | `-t names a translation to change: use --document translation` |
| `--align-method` sin `-t` | `--align-method needs a translation file: use -t` |
| `-t` con plu inputs, o un directoria | `-t names one file but several inputs were given: use one invocation per pair` |
| traduction identic al file principal | `<translation>: the file is already open as the main document` |
