# `correct`

<!-- machine-translated -->
> Ca págine es un traduction machinal e it ancor ne ha esset revidet de un lector del lingue.

```
subedit-cli correct --tasks LIST --code CODE
                    [--classes human|ocr|human,ocr]
                    [--enable NAME]... [--disable NAME]...
                    [--language CODE]
                    [--max-length N] [--max-lines N]
                    [--skip-length N|off] [--skip-lines N|off]
                    [--keep-blank-subtitles | --remove-blank-subtitles]
                    [--range N-M|N-]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    <file>...
```

Corecte li textus con **li modelles de Gaupol**, quam li assistente
[`Correct Texts…`](../subedit-gui/correct-texts.md) del fenestre — li sam modelles,
li sam motor, li sam cuntages. Sis taches existe ci: li **mentiones por
surdes**, **unir** e **separar paroles**, **errores comun**,
**majusculisation** e **ruptura de linea**.

<!-- example: subedit-cli correct --help -->
```console
$ subedit-cli correct --help
Correct the texts with the patterns of the correction assistant
Usage: subedit-cli correct [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to correct

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --tasks LIST                The tasks to run, in a list: none is chosen beforehand
  --code CODE                 The cascade of patterns the tasks read: Zyyy, Latn, Latn-en, Latn-en-US
  --classes human|ocr|human,ocr
                              The classes of common errors to apply: both by default
  --enable NAME               Switch a pattern on, by its English name or type:name; repeatable
  --disable NAME              Switch a pattern off, by its English name or type:name; repeatable
  --language CODE             The dictionary join-words and split-words read: fr_FR, en_US, en...
  --max-length N              Longest line of line-break, in characters; no default, Gaupol's is in ems
  --max-lines N               Most lines of line-break: 3, as Gaupol's
  --skip-length N|off         Leave alone a subtitle whose longest line is within this, or off: --max-length
  --skip-lines N|off          Leave alone a subtitle whose line count is within this, or off: --max-lines
  --keep-blank-subtitles      Leave the subtitles the correction empties, empty, instead of removing them
  --remove-blank-subtitles    Remove the subtitles the correction empties, whatever the settings file says
  --range N-M|N-              Act only on subtitles N to M, or N to the end
  --document main|translation The document to change: the main one, or the translation given by -t
  -t,--translation-file FILE  Translation file to lay over the subtitle file, for a single input
  --align-method position|number
                              How the lines of the translation find their subtitles
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Quo es selectet e quo ne es

**Nullcos es marcat ante.** Li fenestre aperte su casses quam li usator les lassat; un
linea de comande prende su taches, su code, su long de linea e su lingue de su propri linea,
nequande del file de configuration: ni li taches marcat un die in li fenestre, ni li casses de
modelles activat ibi (`correction.activations`), ni li liste de substitution del controlator de ortografie es
leet, e nullcos es scrit de ti latere.

**Quar parametres veni del file de configuration, per predefinit** — li valores quel `Correct Texts…`
memora, o quel [`config set`](config.md) ha scrit. **Un option sur li linea sempre prevale**, poy
li file, poy li valore predefinit integrat; vide [Li file de configuration](invocation.md#li-file-de-configuration)
por saver u it es, `--config` e `--no-config`.

| Clave | Quo it provide | Option quel it supplanta |
| :-- | :--------------- | :----------------------- |
| `correction.human`, `correction.ocr` | li classes de errores comun aplicat | `--classes` |
| `correction.remove-blank` | si li subtitules quel li correction vacua es removet | `--keep-blank-subtitles`, `--remove-blank-subtitles` |
| `correction.line-break.max-lines` | li max de lineas de `line-break` | `--max-lines` |

- **Chascun del quar have un option quel l'anulla**, quel que di li file: `--classes human,ocr`
  aplica ambi classes, `--remove-blank-subtitles` remove li subtitules vacuat, `--max-lines 3`
  retrodona li lineas de Gaupol. `--keep-blank-subtitles` e `--remove-blank-subtitles` together es un
  error de usage.
- **Ambi classes desactivat in li file es ignorat, e dit** al nivelle 1 —
  `settings: correction.human and correction.ocr are both false, ignored` —: `--classes` ne posse scrir it, e it
  lassarea a common-errors nullcos a far. Ambi classes es aplicat vice it.
- **Un númere de lineas foris 1 til 1000 ne es leet del file del tot**: li leetor del file
  refusa it — `settings: correction.line-break.max-lines: cannot read 'N', default kept`, al
  nivelle 1 — e li 3 integrat es usat.

Li exemples de ca págine es executet con `--no-config`, por que ili di quo li valores
predefinit integrat fa, independentmen del parametres del machine quel les executa.

| Option | Obligatori | Valores | Predefinit |
| :----- | :----- | :------ | :----- |
| `<file>...` | yes | un o plu vias | — |
| `--tasks` | **yes** | un liste separat per commas: `mentions`, `join-words`, `split-words`, `common-errors`, `capitalization`, `line-break` | null tache |
| `--code` | **yes** con un tache de modelles, e **ne aplicabil** altrimen | `Script[-language[-COUNTRY]]`: `Zyyy`, `Latn`, `Latn-en`, `Latn-en-US` | — |
| `--language` | **yes** con `join-words` o `split-words`, e **ne aplicabil** altrimen | un code de lingue: `fr`, `fr_FR`, `en_US` | — |
| `--classes` | no | `human`, `ocr` o `human,ocr` | ambi — o quo li file de configuration di |
| `--enable`, `--disable` | no, repetibil | li nómine anglesi de un modelle, o `type:name` | li files `.conf` livrat |
| `--max-length` | **yes** con `line-break` | un númere de **characteres** supra zero | **null** — vide infra |
| `--max-lines` | no | un númere integri de lineas, supra zero | 3 — o quo li file de configuration di |
| `--skip-length`, `--skip-lines` | no | un númere, o `off` | li sam limites quam `--max-length` e `--max-lines` |
| `--keep-blank-subtitles` | no | un flag: li subtitules vacuat resta, vacui | li subtitules vacuat es removet — a minus que li file de configuration di altrimen |
| `--remove-blank-subtitles` | no | un flag: li subtitules vacuat es removet, quel que di li file de configuration; refusat con `--keep-blank-subtitles` | off |
| `--range` | no | `N-M`, o `N-` til li fin — vide [`adjust`](adjust.md#--range) | li tot file |
| `--document` | no | `main` o `translation` — vide [Un traduction](invocation.md#un-traduction) | `main` |
| `-t`, `--translation-file` | con `--document translation` | un file de traduction, por **un sol** input | — |
| `--align-method` | no, e solmen con `-t` | `position` o `number` | `position` |
| `--recursive`, `-r` | no | un flag | off — vide [Tractar un arbore](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **un del tri**, except con `--dry-run` | vide [Invocation](invocation.md#li-destination) | — |
| `--dry-run` | no | un flag: calcula e di, scri nullcos — vide [Vider ante scrir](invocation.md#vider-ante-scrir) | off |

### Li taches

**Li órdine del liste fa null diferentie**: li taches aplica se in li
órdine de Gaupol — li mentiones, poy unir e separar paroles, poy li errores comun, poy li majusculisation, poy li ruptura de linea —, chascun sur li
textu quel li precedent ha lassat. Un subtitul quel li mentiones ha vacuat ne have
null rol in quo seque.

| Tache | Quo it fa |
| :---- | :-------------- |
| `mentions` | remove li sones inter parenteses quadrat, e inter parenteses quande `Sound in parentheses` es activat (li passage de [`hearing-impaired`](hearing-impaired.md)), poy li modelles de cantas e de parlatores |
| `join-words` | recolla un parol decupat in du, secun li dictionarium de `--language` |
| `split-words` | separa du paroles junt, secun li sam dictionarium |
| `common-errors` | corecte li errores comun: spacies, punctuation, ligaturas, errores de OCR |
| `capitalization` | mette li majuscul u li lingue lo vole |
| `line-break` | decupe denov li lineas de un subtitul por que null excede `--max-length`, ponderante li modelles de ruptura del code |

### Li code de modelles

**`--code` nómina li cascada**: `Zyyy` por omni scrition, poy li scrition, li lingue
e li pais — `Latn-en-US` lee li modelles de `Zyyy`, de `Latn`, de `Latn-en` e de
`Latn-en-US`. **It es obligatori**, pro que un linea de comande have null lingue ambiental:
un valore predefinit universal aplicarea *minu* quam on crede, sin dir it. It vale por
chascun tache del sam execution; un tache queles vole un code propri es un duesim
execution. Un code mal scrit es un error de usage:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code francais --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code francais --output b.srt a.srt; echo "code=$?"
--code: "francais" is not a pattern code: expected Script[-language[-COUNTRY]], like Zyyy, Latn or Latn-en-US
code=1
```

Li modelles quel li usator mette in `$XDG_DATA_HOME/subedit/patterns` (o
`~/.local/share/subedit/patterns`) **es adjuntet al modelles livrat**, quam in li
fenestre, e un modelle livrat quel ne posse esser leet es dit al nivelle 1: `patterns: <file>, line N (<reason>)`.

### Unir e separar paroles, e li dictionarium

**Ti du taches lee null modelle**: les interroga li **dictionarium del sistema, per
Enchant** — un dependentie del paccages (`libenchant-2-2` sur Debian e Ubuntu, `enchant2` sur
Fedora), quel `subedit-cli` partisse con li fenestre. Les have dunc ni code ni nómine a fixar:
`--code` ha null sense sin un tache de modelles, e `--language` null sin un de ti du
— un dat sin li altri es un error de usage, code `1`.

- **`join-words`** recolla un parol quel li dictionarium ne conosce a celi quel precede it o
  seque it, **quande un sol direction dona un parol conosset**: `hel lo` deveni `hello`; du
  directiones possibil, o null, decide nullcos.
- **`split-words`** separa un parol quel li dictionarium ne conosce **quande exactmen un de su
  suggestiones es ti parol con un spacie**: `hellothere` deveni `hello there`. Un parol con un majuscul
  initial sequet de minusculas es lassat in pace: it es sovente un nómine.

**Sin dictionarium por li lingue, li execution es refusat**, code `1`, **ante que un file es
leet** e in li paroles del fenestre, quel desactiva li function e di it — ci li linea de comande
refusa e di it, plu tosc quam tacer:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nhel lo\n\n' > a.srt; subedit-cli --no-config correct --tasks join-words --language qq_QQ --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nhel lo\n\n' > a.srt; subedit-cli --no-config correct --tasks join-words --language qq_QQ --output b.srt a.srt; echo "code=$?"
no dictionary for qq_QQ
code=1
```

**Li liste de substitution del fenestre ne es leet ni scrit.** It *se plena* per li
selectiones del usator in li controle de ortografie; un linea de comande have null tal
selection, e leer li liste va far depender un script de quo su autor ha un vice mettet in it.

### Ruptura de linea

**It es mesurat in characteres, e to es li diferentie con li fenestre.** Li fenestre mesura in *ems* —
un largore calibrat sur li font, a 0,55 em per littere —, e su 24 predefinit es un valore
in ems. Un linea de comande have null font: it cunta characteres, celes del
textu. **Copiar li 24 quam characteres decuperea quasi omnicos, sin
dir it**; ne ha dunc **null long predefinit**, e `--max-length` es obligatori con
`line-break`:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn texte\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn texte\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --output b.srt a.srt; echo "code=$?"
--max-length is required by the task line-break: Gaupol's 24 is a width in ems, and has no value in characters
code=1
```

**`--max-lines` es 3**, li valore de Gaupol, quel li unité ne changea; li ruptura de linea prende
un plu quande tri ne basta. **Li omission** — un subtitul ja in li limites es
lassat quam it es — prende per predefinit **li sam limites** (`--skip-length` = `--max-length`,
`--skip-lines` = `--max-lines`), quam Gaupol fa; `--skip-length off` e `--skip-lines off`
li desactiva, e li textu es alor leet denov quam un e decupat denov:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nWhen the rain stopped, we walked home under the dark sky of the old town.\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --max-length 24 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nWhen the rain stopped, we walked home under the dark sky of the old town.\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --max-length 24 --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:03,000
When the rain
stopped, we walked
home under the dark
sky of the old town.
```

Li modelles de ruptura es celes del code: `Latn` ne rupte inter un númere e su unité ni
pos un strec de dialog, `Latn-en` ne rupte pos un articul, un preposition, un titul
o un determinant possessiv. **Sub `Zyyy`, null modelle de ruptura existe**: li lineas es
solmen equilibrat. **Ti tache nequande es refusat pro li manca de un modelle activ**: su modelles
pondera u rupter, li ruptura evenit sin les. Li quar optiones have sense solmen con
`line-break`; dat sin it, ili es un error de usage.

### Li classes, e activar per nómine

**`--classes` filtra li errores comun definitivmen**: desmarcar `ocr` leva su
modelles ex li application — li ligatura `ﬁ` resta alor `ﬁ`. Li altri tri gene de modelles
porta null classe.

**`--enable` e `--disable` fixa un modelle per su nómine anglesi**, li nómine esient li
clave (ADR 0037); un nómine posse designar plu registres, queles partisse un sol
casse. It es necessi, e it es quo mette a un latere li valores predefinit del fenestre: **li
files `.conf` livrat desactiva li modelles de mention**, por que `--tasks mentions` sol
aplicarea nullcos. Li du modelles «Sound in brackets» e «Sound in parentheses»
comanda chascun su delimitator — li parenteses quadrat, li parenteses — e es fixat per li sam
optiones: activar un sol ne remove li altri.

Tri cosas es **errores de usage**, code `1`, dit ante que un file es leet:

| Quo it causa | Missage |
| :------------------ | :------ |
| un nómine quel designa null modelle del taches dat, sub li code dat | `--enable: "<name>" names no pattern of the tasks given, under the code <code>` |
| un nómine quel designa modelles de du gene — it es alor scrit `type:name`, li gene esient `common-error`, `capitalization`, `hearing-impaired` o `line-break` | `--enable: "<name>" names patterns of several types: write type:name, …` |
| un nómine dat a `--enable` e a `--disable` | `"<name>" is given to --enable and to --disable: choose one` |

**Un erra de tipation es sonori**: li fenestre ignora un parametre obsolet pro que un
parametre posse survivir a un modelle desaparit; un invocation have null superviventes.

**Un tache sin null modelle activ es refusat**, code `1` — «null cose a far» es un
response quel es dit, ne un successe silentiosi:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --output b.srt a.srt; echo "code=$?"
mentions: no pattern is active under the code Latn; switch one on with --enable
code=1
```

Li sam file, con li passage de parentese quadrat activat:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --enable "Sound in brackets" --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --enable "Sound in brackets" --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:02,000
Bonjour
```

## Quo es scrit, e quo es dit

Li formate, li codification e li fines de linea del file leet es **conservat**. **Li
subtitules quel li correction vacua es removet**, quam li casse predefinit del
fenestre; `--keep-blank-subtitles` les lassa vacui, e `--remove-blank-subtitles` les remove
mem quande li file de configuration les conserva. **Un traduction vacuat resta vacuat**,
e conserva su subtitul.

**Li cuntage es celi del fenestre** — «Edited N and removed M subtitles»: textus
changeat e subtitules removet, **nequande concordanties**. Un file u nullcos
changea es scrit tamen: un destination dat es un destination scrit.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\neﬀet  de ﬁn\n\n2\n00:00:03,000 --> 00:00:04,000\nPropre.\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code Latn --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\neﬀet  de ﬁn\n\n2\n00:00:03,000 --> 00:00:04,000\nPropre.\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code Latn --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:02,000
effet de fin

2
00:00:03,000 --> 00:00:04,000
Propre.
```

### Un modelle quel ne posse esser aplicat

**Un modelle quel ne posse esser leet, ne posse esser traductet o ne fini ne es nominat, con li
subtitul u it ha abandonat**, e it es ci que li linea de comande di plu quam li
fenestre: li págine de confirmation nómina li modelle un sol vice, sin li textu.

| Rason | Quo it significa |
| :----- | :------------------- |
| `cannot be translated` | su expression usa quo li traduction a ICU refusa |
| `will not compile` | li motor ha refusat su expression |
| `has an invalid replacement` | su substitution ne se lee quam Python la lee |
| `timed out` | li motor ha abandonat sur ti textu |
| `never settled` | `Repeat` ancor changeat li textu pos cent passages |
| `grew the text too long` | li textu ha passat quo un subtitul posse conter |

Al nivelle 1, sur li output de error: 
`<path>: pattern "<name>" (<reason>) was not applied to subtitle N` — sin li fin quande li modelle es mettet a un latere por chascun textu. **Li númere es celi del
file**, quel que taches ha removet subtitules ante. In JSON, it es
un advertimente `pattern-failed` del file, quel `detail` porta li frase. **Un modelle
abandonat ne es un file falit**: li file es scrit, e li code de exeunte es `0`.

## Output

**Output standard** — nullcos, li resultate esient li file scrit. **Con `--dry-run`, it es li
changes proposit**, un bloc per subtitul: su númere, li textu ante, li textu pos —
e `(removed)` sin linea `+` por un subtitul quel li correction removerea; con
`--keep-blank-subtitles`, it resta, vacui. Vide [Vider ante scrir](invocation.md#vider-ante-scrir).

| Nivelle | Quo es adjuntet al output de error |
| :----- | :------------------------------------- |
| 1 | `<path>: Edited N and removed M subtitles -> <destination>`, li modelles quel ne posse esser aplicat, li files de modelles quel ne posse esser leet — e, unesim, li file de configuration leet — `settings: read <path>` — e quo de it esset ignorat |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, poy chascun diagnose de lectura |

In JSON, `counts` porta `corrected` (li textus changeat) e `removed` (li subtitules
removet), e `changes` li subtitules tuchat. Con `-t`, li clave `alignment` di qualmen li
traduction esset mettet.
`settings` nómina li file de configuration quel esset leet, o es `null` si null esset leet.

## Codes de exeunte

Celes del instrument: `0` si chascun file esset scrit, `2` si null, `3` si solmen
alcunes, `1` in cas de error de usage.

## Errores

| Quo it causa | Quo es scrit, sur li output de error |
| :------------------ | :--------------------------------------- |
| un file nominat per `--config` quel ne existe — li propri file del usator, mancant, dona li valores predefinit | `settings file not found: <FILE>` |
| `--tasks` mancant | `--tasks is required`, sequet de un indicator a `--help` |
| un tache inconosset | `--tasks: "<name>" is not a task: expected mentions, join-words, split-words, common-errors, capitalization or line-break` |
| `--code` mancant | `--code is required by the tasks that read patterns: <tasks>` |
| `--code` sin un tache de modelles | `--code is for the tasks that read patterns, and none was asked for` |
| `--language` mancant con `join-words` o `split-words` | `--language is required by the tasks that check words: <tasks>` |
| `--language` quel ne es un code de lingue | `--language: "<value>" is not a language: expected a locale code, like fr, en_US or sr@Latn` |
| `--language` sin un de ti du taches | `--language is for the tasks join-words and split-words, which were not asked for` |
| un lingue sin dictionarium | `no dictionary for <language>` |
| un code mal scrit | `--code: "<code>" is not a pattern code: expected Script[-language[-COUNTRY]], like Zyyy, Latn or Latn-en-US` |
| `--max-length` mancant con `line-break` | `--max-length is required by the task line-break: Gaupol's 24 is a width in ems, and has no value in characters` |
| `--max-length` quel ne es un númere supra zero | `--max-length: "<value>" is not a length: expected a number of characters greater than zero` |
| `--max-lines`, `--skip-lines` queles ne es un númere integri de lineas | `--max-lines: "<value>" is not a number of lines: expected a whole number greater than zero` |
| `--skip-length` o `--skip-lines` illegibil | `--skip-length: "<value>" is not a bound: expected a number greater than zero, or off` |
| un del quar optiones sin `line-break` | `--max-length is for the task line-break, which was not asked for` |
| un classe inconosset | `--classes: "<name>" is not a class: expected human, ocr or human,ocr` |
| `--keep-blank-subtitles` con `--remove-blank-subtitles` | `--keep-blank-subtitles and --remove-blank-subtitles say opposite things; give one of them` |
| un nómine de modelle erronat | vide li tabelle ci-supra |
| un tache sin modelle activ | `<task>: no pattern is active under the code <code>; switch one on with --enable` |
| `--range`, `-t`, `--document`, li destination | celes de [Invocation](invocation.md#errores) e de [Un traduction](invocation.md#un-traduction) |
