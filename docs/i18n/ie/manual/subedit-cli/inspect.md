# `inspect`

<!-- machine-translated -->
> Ca págine es un traduction machinal e it ancor ne ha esset revidet de un lector del lingue.

```
subedit-cli inspect [--frame-rate RATE] [--recursive] [--detail]
                   [-t FILE] [--align-method position|number] <file>...
```

Reporta de quo chascun file es fat. **Changea nequo e scri null
file.**

<!-- example: subedit-cli inspect --help -->
```console
$ subedit-cli inspect --help
Report what a subtitle file is made of
Usage: subedit-cli inspect [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to report on

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --detail                    List every anomaly, subtitle by subtitle, instead of counting them
  --frame-rate RATE           Frame rate of a file counted in frames: 25, 23.976
  -t,--translation-file FILE  Translation file to lay over the subtitle file, for a single input
  --align-method position|number
                              How the lines of the translation find their subtitles
```

## Arguments

| Argument | Obligatori | Valore | Predefinit |
| :------- | :----- | :----- | :----- |
| `<file>...` | yes | un o plu camines de files de subtitules | — |
| `--recursive`, `-r` | no | un flag | off — vide [Tractar un arbore](lots.md) |
| `--detail` | no | un flag: nomina chascun anomalie, vice cuntar les — vide [Quo `anomalies` reporta](#quo-anomalies-reporta) | off |

Null camine es un error de usage, dunc code `1`.

## Optiones

| Option | Obligatori | Valore | Predefinit |
| :----- | :------ | :----- | :----- |
| `--frame-rate` | no | un frequentie in fotogrammas per second: `25`, `23.976`, `29.97` | 23,976 |

**It serve solmen files contat in fotogrammas**, to es MicroDVD, li
sol del nin. Un file MicroDVD porta nequo ma númeres de fotogramma; li
frequentie con quel les es leet decide **chascun position del raporte**,
comensant per `span`. Donat altrubi, it changea nequo e ne es un
error.

Un valore quel nomina nequo es un error de usage, dunc code `1`:

<!-- example: subedit-cli inspect --frame-rate douze film.sub; echo $? -->
```console
$ subedit-cli inspect --frame-rate douze film.sub; echo $?
--frame-rate: "douze" is not a frame rate: expected frames per second, like 25 or 23.976
1
```

It es li sam option quam in [`convert`](convert.md), parol por parol, pro que
it es li sam frequentie. `convert` anc usa it por **scrir**.

## Un traduction: `-t`

| Option | Obligatori | Valore | Predefinit |
| :----- | :------ | :----- | :----- |
| `-t`, `--translation-file` | no | un file de traduction, por **un sol** input | — |
| `--align-method` | no, e solmen con `-t` | `position` o `number` | `position` |

Con `-t`, li traduction es posit sur li file **quam li fenestre fa quande it aperte** — vide
[Un traduction](invocation.md#un-traduction) — e li raporte di **qualmen su lineas trovat lor
subtitul**. Un script save dunc si un traduction es alliniat ante corecter it.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour.\n\n' > film.fr.srt; subedit-cli --quiet inspect -t film.fr.srt film.srt | tail -3 -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour.\n\n' > film.fr.srt; subedit-cli --quiet inspect -t film.fr.srt film.srt | tail -3
  anomalies: none
  translation file: film.fr.srt, matched by position
  translation: 1 line attached; 1 subtitle left without a translation
```

Li prim linea adjuntet nomina li file e li metode; li duesim es **li frase quel li fenestre
di** quande it aperte un traduction, chascun parte solmen quande it ne es null:

| Parte | Quo it cunta |
| :----- | :---------------- |
| `N lines attached` | li lineas del traduction queles trovat un subtitul del file principal |
| `N subtitles born of a line` | li lineas queles trovat null e natat lor propri subtitul, a lor propri positiones |
| `N subtitles left without a translation` | li subtitules del file principal quel null linea atinget |
| `N lines out of order` | li lineas queles veni pos un plu tardi in li file |

Un traduction quel porta null linea di `translation: the file holds no line`. **Li raporte del
file self resta ti del file sol**: un subtitul natat de un linea ne es contat in
`subtitles`.

Li errores de `-t` es ti de un lectura, dit con li camine del traduction, e ti-ci:
`<translation>: the file is already open as the main document`, quande li file donat es li principal.
`--align-method` sin `-t` es un error de usage, e anc `-t` con plu inputs — vide
[Un traduction](invocation.md#un-traduction).

## Output

Li raporte vade al **exeunte standard** — it es li resultate del comande, e
it es scrit a chascun nivelle, `--quiet` includet. Un bloc por file, precedet
del camine:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,500\nLe canot dérive.\n\n2\n00:00:04,000 --> 00:00:06,200\nPersonne n a vu la côte.\n' > exemple.srt; subedit-cli --quiet inspect exemple.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,500\nLe canot dérive.\n\n2\n00:00:04,000 --> 00:00:06,200\nPersonne n a vu la côte.\n' > exemple.srt; subedit-cli --quiet inspect exemple.srt
exemple.srt
  format: SubRip
  encoding: UTF-8, detected
  byte order mark: absent
  line endings: LF
  subtitles: 2
  span: 00:00:01.000 -> 00:00:06.200
  frame rate grid: none (too few subtitles to tell)
  anomalies: none
```

| Campe | Quo it di |
| :---- | :----------- |
| `format` | un del formates quel `--to` accepta, scrit in plen: `SubRip`, `WebVTT`, `SubViewer 2`, `Sub Station Alpha`, `Advanced SSA`, `MicroDVD`, `MPL2`, `TMPlayer`, `LRC` |
| `encoding` | li codification leet, sequet de **u li responde veni**: `, from its byte order mark` si li file declara it, `, as asked for` si `--encoding` imposit it, `, detected` si it esset pesat |
| `byte order mark` | `present` o `absent` |
| `line endings` | `LF`, `CRLF` o `CR`, sequet de `, mixed from line N` si li file mixtura les |
| `subtitles` | li númere de subtitules leet |
| `span` | del comense li plu precoce al fin li plu tardi, `HH:MM:SS.mmm` |
| `frame rate grid` | li frequentie de fotogrammas sur quel li positiones esset calculat, **deducet** — vide infra |
| `frame rate` | por un file contat in fotogrammas, li frequentie con quel it esset leet e **u it veni** — remplacia li linea precedent, vide infra |
| `anomalies` | `none` sur li sam linea, o quo es mal: **un gene per linea con su cuntage**, indentat sub `anomalies:`; con `--detail`, **un anomalie per linea**, subtitul per subtitul |

**`span` ne es «del prim al ultim»** ma del plu precoce al plu tardi:
sur un file de quel órdine es rupt, li du diffre, e solmen li duesim di
li veritá pri quo li file covre.

**`anomalies`, con `--detail`, cunta li subtitules de 1**, quam li file les
numera — e ne li lineas del file. Un númere de subtitul subvive a un
change; un númere de linea ne. Vide infra.

## Quo `frame rate grid` reporta

**Un file de subtitules ne declara su frequentie de fotogrammas.** SubRip have null
capite, ti de WebVTT es textu libere. Ca linea dunc leer nequo:
it **deduce**, per mesurar si li positiones cade sur un grille de fotogrammas.

Quo es mesurat es li **comenses**. U un grille existe, les es sur it
sin exception, durant que li fines sovente erra de it — un fin es
frequentmen calculat per un regul de rapidesse de lectura plu tost quam metet sur
un fotogramma.

Li prim linea di sempre li sam cose: li frequentie electet, li verdicte,
e li concentration quel supporta it.

| Verdicte | Quo it significa |
| :------ | :----------------- |
| `clean` | li positiones es sur un grille |
| `partial` | un parte de les es, li resto ne |
| `none` | null candidate explica quocunc |

`none` have **du causas, e ili ne es li sam responde**: o null
grille convene — li linea tande dona li max bon candidate e su score,
por que on vide de quant it falie — o li file cunta tro poc
subtitules por que quocunc posse esser dit pri it. Du comenses sempre sembla
perfectmen alliniat; to proba nequo.

**Un file regulari sur un frequentie ne standard dona `none`, e to es
intentet.** Li ensemble de candidates es cluset — li ot frequenties quel li
conversion oferte — e reportar li min erronic de les va esser donar
un responde erronic u «yo ne save» es li responde just.

### Li lineas sequent, quande ili have quo dir

| Linea | Quande it apare |
| :---- | :------------------ |
| `grid offset` | li positiones es sur li grille **a un constant prés**: li file esset displaciat, e ci es de quant |
| `also fits` | un altri candidate convene tam bon, pro que it es un multiplo integri del electet |
| `too short a span to separate` | li extension del file ne permisse distinter li frequentie electet de ti nominat |
| `off the grid` | quant comenses erra foris li grille, e in quant **successiones** |

**`also fits` ne es un hesitation.** Un grille a 25 fotogrammas per second es
*includet* in un grille a 50: un file temporisat sur 25 convene dunc ad ambi.
Li contrari es fals — un 50 real colapsa sur 25 — dunc es li plu bass quel
es electet, e li altri es nominat vice esser tacet. Li sol
pares concernet es 25 e 50, 29,97 e 59,94, 30 e 60.

**`too short a span to separate` guarda contra un responde tro confidente.** 23,976
e 24 diverge de un millisecunde per secunde de film: sur un long film les
se separa sin dubita, sur dece secundes on ne posse distinter les. Li linea di
qual resta possibil.

**`off the grid` distinte du histories quel li percentage sol confunde.**
Mult successiones de un sol comense es positiones corecet a mane,
un per un. Alcun long successiones es un section retemporisat o un file
assemblat ex du altres. Li númere de successiones es quo les separa.

### Un exemple

Dudece lineas metet sur un grille a 25 fotogrammas per second — dunc a
positiones multiplo de 40 millisecundes — repartit sur quasi nin minutes:

<!-- example: k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > grille.srt; subedit-cli --quiet inspect grille.srt | tail -3 -->
```console
$ k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > grille.srt; subedit-cli --quiet inspect grille.srt | tail -3
  frame rate grid: 25 fps, clean (100.0%)
  also fits: 50 fps, of which this rate is a whole divisor
  anomalies: none
```

Li exemple crea su propri file vice leer un del repositoria, e to es
deliberat: it posse esser replayat quam it es, e li loop monstra quo es un grille
mey bon quam un frase — positiones multiplo de un duration de fotogramma.

## Quo `frame rate` reporta, e pro quo it ne es un grille

**Solmen un del nin formates cunta in fotogrammas**: MicroDVD, de quel chascun linea
porta du númeres de fotogramma e nequo plu. Sur tal file, `inspect`
ne scri `frame rate grid` ma `frame rate`, e li du lineas ne es
nequande monstrat together.

**Li rason ten in un frase.** Li positiones de un file MicroDVD esset
calculat *ex* su fotogrammas, a un frequentie donat; deducer les ex un
grille dat retro ti mem frequentie, quam it ha just esset fornit. It va esser
un cifre donat, vestit quam mesuration.

Quo es scrit vice it es **li frequentie de lectura e u it veni** —
li sam forme quam li linea `encoding`:

| Quo li linea di | Quande |
| :------------------ | :---- |
| `frame rate: 25 fps, as asked for` | `--frame-rate` imposit it |
| `frame rate: 24000/1001 fps, assumed` | nequi dit it; it es li predefinit |

**Du responses e ne tri**, u `encoding` have tri: un file MicroDVD
declara nequo. To es li tot problema — li frequentie veni necessarimen
de foris, e li raporte di qual del du foris.

**Li frequentie decide tot li resto del raporte.** `span` passa per it, e du
lectiones del sam file a du frequenties dona du extensiones:

<!-- example: printf '{25}{75}Première.\n{100}{150}Deuxième.\n{200}{260}Troisième.\n' > images.sub; subedit-cli --quiet inspect images.sub | tail -3; subedit-cli --quiet inspect --frame-rate 25 images.sub | tail -3 -->
```console
$ printf '{25}{75}Première.\n{100}{150}Deuxième.\n{200}{260}Troisième.\n' > images.sub; subedit-cli --quiet inspect images.sub | tail -3; subedit-cli --quiet inspect --frame-rate 25 images.sub | tail -3
  span: 00:00:01.043 -> 00:00:10.844
  frame rate: 24000/1001 fps, assumed
  anomalies: none
  span: 00:00:01.000 -> 00:00:10.400
  frame rate: 25 fps, as asked for
  anomalies: none
```

## Quo `anomalies` reporta

Tri cosas posse esser mal in un document, e chascun es reparat altrimen.
**Predefinit `inspect` les cunta**, un linea per gene, in li órdine del tabelle:
un file mal fat porta centes de les, e saver quant de chascun gene
es quo di quo executar. **`--detail` les nomina un per un.**

| Quo es scrit | Con `--detail` | Quo it significa |
| :--------------- | :-------------- | :------------------ |
| `subtitle ends before it starts: N` | `subtitle N ends before it starts` | li fin veni ante li comense |
| `subtitle starts before the previous one ends: N` | `subtitle N starts before the previous one ends` | it se superpone a ti ante it |
| `subtitle starts before the previous one starts: N` | `subtitle N starts before the previous one starts` | it rupte li órdine del file — [`sort`](sort.md) remette it |

Un gene absent ne es scrit. In li columne a sinistra, `N` es li númere
de subtitules; in li columne sequent, li númere de un subtitul.

**Li sam subtitul posse aparer du vezes**, e to es intentet: ti quel
comensa ante que li precedent ha comensat anc comensa ante que it ha
finit. Li prim punctu es resolvet per displaciar li subtitul, li duesim per
changear un duration.

**Li subtitul nominat es ti quel es mal plasset**, ne ti contra quel
it es mal plasset. Sur comenses a `0`, `4 s`, `2 s`, `3 s`: li triesim rupte li órdine, li
quartesim seque ver li triesim e dunc ne es nominat — nequo es a
far pri it.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect desordre.srt | tail -3 -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect desordre.srt | tail -3
  anomalies:
    subtitle starts before the previous one ends: 1
    subtitle starts before the previous one starts: 1
```

E con `--detail`, li subtitul nominat:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect --detail desordre.srt | tail -3 -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect --detail desordre.srt | tail -3
  anomalies:
    subtitle 3 starts before the previous one ends
    subtitle 3 starts before the previous one starts
```

**Un linea per anomalie con `--detail`**, pro que cent trovages
colat sur un linea ne posse esser leet ni serchat. Un document sin defecte conserva
su sol linea, `anomalies: none`, quel un script cerca. **Li
registre `--format json` lista sempre chascun anomalie**: un script leer li detail, e
`--detail` changea solmen li textu.

**It ne es un diagnose de lectura.** Li diagnoses di quo li
lectura ha incontrat e indica un **linea del file**; li anomalies di
quo li document *es* e indica un **subtitul**. Un file de quel órdine
es rupt ne es malformat pro to: li lectura have nequo a dir pri it.

## In JSON

Con [`--format json`](invocation.md#un-output-quel-un-script-posse-leer), `inspect` scri **un
objecte per file** vice li blocs supra; li narration resta sur li exeunte de error.
Li claves descrie quo li textu di, e **null es a releer ex un frase**:

| Clave | Contenete |
| :-- | :------ |
| `format` | li identificator del formate, ti quel `convert --to` accepta: `srt`, `vtt`… |
| `encoding` | `{"charset", "origin", "byte_order_mark"}` — `origin` es `byte-order-mark`, `asked` o `detected` |
| `line_endings` | `{"kind", "mixed_from_line"}` — `lf`, `crlf` o `cr`; li linea u un duesim gene apare, o `null` |
| `subtitles` | li númere de subtitules |
| `span_ms` | `{"start", "end"}` — li prim comense e li ultim fin, in millisecundes |
| `frame_rate` | por un file contat in fotogrammas: `{"rate", "origin"}` (`asked` o `assumed`); altrimen `null` |
| `grid` | por li altres: li grille deducet, o `null` por un file contat in fotogrammas |
| `anomalies` | un array de `{"subtitle", "kind"}` — `end-before-start`, `overlapping-subtitles`, `out-of-order` |
| `translation` | **solmen con `-t`**: `{"file", "method", "attached", "born", "untranslated", "out_of_order"}` — `method` es `position` o `number`, e li quar cuntages es ti del frase supra |
| `warnings` | quo li lectura ha incontrat, a chascun nivelle |

`grid` porta `verdict` (`clean`, `partial` o `silent`), `enough_starts`, `rate` (un catene,
o `null` quande nequo es nominat), `concentration_permille` (millesimes, `null` sin suficent
comenses), `offset_ms`, `also_fits` (li frequentie de quel ca un es un divisor integri, o
`null`), `not_separated` (li frequenties quel un extension tro curt ne separa), `strays`
(li comenses foris li grille) e `starts`.

<!-- example: printf '{25}{50}Un.\n{75}{100}Deux.\n' > f.sub; subedit-cli --format json inspect --frame-rate 25 f.sub 2>/dev/null -->
```console
$ printf '{25}{50}Un.\n{75}{100}Deux.\n' > f.sub; subedit-cli --format json inspect --frame-rate 25 f.sub 2>/dev/null
{"schema":1,"command":"inspect","file":"f.sub","ok":true,"format":"microdvd","encoding":{"charset":"UTF-8","origin":"detected","byte_order_mark":false},"line_endings":{"kind":"lf","mixed_from_line":null},"subtitles":2,"span_ms":{"start":1000,"end":4000},"frame_rate":{"rate":"25","origin":"asked"},"grid":null,"anomalies":[],"warnings":[]}
```

## Narration

Sur li exeunte de error, secun li nivelle demandat:

| Nivelle | Quo es adjuntet |
| :----- | :-------------- |
| 1 | `<path>: N subtitles` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings` |
| 3 | `<path>: N bytes read`, `<path>: N diagnostic(s) while reading`, pois **chascun de les** — vide [Invocation](invocation.md#diagnoses-de-lectura) |

## Codes de exeunte

Ti del instrument: `0` si chascun file esset leet, `2` si null, `3` si
solmen alcun, `1` in cas de error de usage.

## Errores

Ti de un lectura — del file, e del traduction quande `-t` es donat —, e
ti de `-t`: vide supra. `inspect` scri nequo quel posse esser refusat.

| Quo it causa | Missage |
| :------------------ | :------ |
| null camine donat | `files is required`, sequet de un indication a `--help` |
| file mancant | `<path>: does not exist` |
| file illegibil | `<path>: cannot be opened: permission denied` |
| bytes quel ne decodar se | `<path>: cannot be decoded in the chosen encoding` |
| formate ne recognoscet | `<path>: is in no format this tool knows` |
| nequo quel sembla un subtitul | `<path>: holds nothing recognisable as a subtitle` |

Ti es li sam missages por li dissette subcomandes: li recepte por aperter
es scrit un vez, in li core.

## Un exemple quel falie

Un file vacui corresponde a null formate. It ne es reportat quam vacui:
it es refusat, e nominat.

<!-- example: : > vide.srt; subedit-cli inspect vide.srt; echo "code=$?" -->
```console
$ : > vide.srt; subedit-cli inspect vide.srt; echo "code=$?"
vide.srt: is in no format this tool knows
code=2
```
