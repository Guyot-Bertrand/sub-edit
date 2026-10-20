# `convert`

<!-- machine-translated -->
> Ca págine es un traduction machinal e it ancor ne ha esset revidet de un lector del lingue.

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

Scri chascun file in li formate e li forme demandat. **Nequande it changea
li input**, a minus que `--in-place` lo demanda explicitmen.

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

## Argumentes e optiones

| Option | Obligatori | Valores | Predefinit |
| :----- | :----- | :------ | :----- |
| `<file>...` | yes | un o plu vias | — |
| `--recursive`, `-r` | no | un flag | off — vide [Tractar un arbore](lots.md) |
| `--to` | **yes** | un nómine del tabelle ci-infra, e nullcos altri | — |
| `--line-endings` | no | `unix`, `windows` o `mac` | celes del file leet |
| `--to-encoding` | no | omni codification quel ICU posse scrir, except celes quel scri lor propri marca | celi del file leet |
| `--bom` / `--no-bom` | no | flags, exclusiv un del altri | quo li file leet portava |
| `--sort` | no | un flag: mette li subtitules in li órdine de lor comense **ante scrir**, vide [Ordinar durant li scrition](#ordinar-durant-li-scrition) | off |
| `--join-repeats` | no | un flag: crea un sol subtitul ex li peces de un textu queles seque se, **ante omnicos altri**, vide [Far disjuncte li intervalles](#far-disjuncte-li-intervalles) | off |
| `--repair` | no | un flag: li sam quam `--join-repeats --sort --disjoint`, in ti órdine | off |
| `--disjoint` | no | un flag: decupe li subtitules superponent **ante scrir**, vide [Far disjuncte li intervalles](#far-disjuncte-li-intervalles) | off |
| `--output` / `--output-dir` / `--in-place` | **un del tri**, except con `--dry-run` | vide [Invocation](invocation.md#li-destination) | — |
| `--dry-run` | no | un flag: calcula e di, scri nullcos — vide [Vider ante scrir](invocation.md#vider-ante-scrir) | off |

`mac` nómina li retorn de carro sol (`\r`), li fine de linea del Mac OS classic.

## Li formates quel `--to` accepta

| Nómine | Formate | Extension scrit |
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

**Un nómine, e ne un extension.** Du extensiones nómina du formates
chascun — `.sub` es anc celi de MicroDVD, `.txt` celi de MPL2 e de
TMPlayer — dunc un option quel prenderea extensiones ne saverea qual
es demandat. Li nómines ci-supra es inambigu, e corresponde al
extension u li extension es anc inambigu.

**Li liste es clausi.** It es celi de Gaupol, e omni li nov es in it.

## Li du formates sin fin

**TMPlayer e LRC scri solmen un position per linea**, su comense. To
se monstra de ambi lateres.

**Durant li lectura**, li fines es deducet: celi de un linea es li comense del
sequent, e li ultim recive cinc secondes. To es quo Gaupol fa.
On lo di anc, per un diagnose quel `--verbose` monstra sur li tri
subcomandes:

```
file.lrc: carries no end times; each one was taken from the next start, settled by the reader
```

**Durant li scrition**, li fines ne es scrit, mancante un loc u mettre les.
Converter a un de ci du formates dunc les perde, e un ear e reveni ne
les retrova — it les deduce denov.

**LRC perde un cose plu**: it have null manie de portar un ruptura de linea.
Un linea de dialog sur du lineas es scrit sur un, li du lineas junt per
un spacie. Ti perde anc ne posse esser reparat.

Null del du have un vocabulario por li italic.

## `--frame-rate`, e li sol formate quel cunta in fotogrammas

**MicroDVD ne cunta in secondes**: su positiones *es* númeres de
fotogramma, e null file MicroDVD declara li frequentie secun quel ili esset
contat. `--frame-rate` dona it, e servi de ambi lateres: it es li
frequentie secun quel un `.sub` in fotogrammas es leet, e celi secun quel un
conversion a MicroDVD cunta li fotogrammas.

| Option | Obligatori | Valores | Predefinit |
| :----- | :----- | :------ | :----- |
| `--frame-rate` | no | un frequentie in fotogrammas per second: `25`, `23.976`, `29.97` | vide infra |

**Durant li lectura, sin `--frame-rate`, li file es leet a 23,976** — li valore de
Gaupol — e li nivelle `-vvv` di it:
`counts in frames and states no rate; it was read at`. It es li sol loc in li instrument u chascun position monstrat
reposa sur un suposition quel li file ne posse confirmar.

**Durant li scrition, sin `--frame-rate`, tri casus e tri responses:**

| Li file de comense | Quo evenit |
| :------------------- | :-------------- |
| es ja MicroDVD | li frequentie quel leet it scri it denov, e li fotogrammas reveni identic |
| es basat sur tempore, e su positiones cade sur un grille | li grille es prendet, e `-vv` di qual |
| es basat sur tempore, sin grille | **refusa**, code 2: `writing frames needs a frame rate, and the positions fall on no grid to take one from — give --frame-rate` |

Li refusa es deliberat. Li sol gesto restant serea inventar un cifre, e
chascun linea del file moverea de tam quam it esset erronat.

## Quo un conversion perde

**Converter a un altri formate nequande es sin perde**, e quo ne survive
depende del par:

| Quo ne passa | Pro quo |
| :--------------------- | :------- |
| li capite | it have sense solmen in su propri formate: `[INFORMATION]` ne es un capite WebVTT |
| li data specific del formate | li coordinates de SubRip, li identificator e li parametres de un cue WebVTT have null equivalent altrubi |
| li precision | SubViewer 2, li du Sub Station Alpha e LRC scri al centesim, MPL2 al decim, TMPlayer al second, MicroDVD al fotogramma; `01:00:00,017` reveni quam `01:00:00,020` |
| li fin de un linea | TMPlayer e LRC have null campe por it; li lectura sequent deduce it denov |
| li ruptures de linea | LRC solmen: su lineas intra in un sol linea, e li du es collat per un spacie |
| li etiquettes quel li formate cible ne posse scrir | WebVTT have null `<font color>`, TMPlayer e LRC have nullcos |
| li disposition | `{\pos(x,y)}`, `{\an8}`, `<v Marie>`, `<ruby>`: ili survive un ear e reveni in lor propri formate, e desapari in conversion |

**Scrir un file denov in su propri formate nequande perde nullcos** — to es li garantie
quel sustene omni li resta, e `--to srt` sur un `.srt` retrodona li sam bytes. Nullcos
es mem decodat: li textu resta li catene bru quel esset leet.

## Li etiquettes es traductet, ne copiat

**Chascun formate scri li italic a su manie**, e converter sin traducter
lassarea un `{\i1}` in un `.srt`: textu quel li usator vide, in un
file quel nequande saverea interpretar it. Quo passa es **sis
cosas** — grasse, italic, suslineat, color, font e grandore:

| Formate | Grasse | Italic | Suslineat | Color | Font | Grandore |
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

**Un strec es un perde anunciat.** Converter un `.srt` colorat a WebVTT remove
li color e di it; li file produit es WebVTT, ne SubRip camuflat.

**MicroDVD e MPL2 ne posse stilar un parte de un linea.** Lor etiquettes
have null fin: `{y:i}` inclina omnicos quel seque til li fin del linea, e
`{Y:i}` til li fin del subtitul. Un linea de quel solmen un mediage esset
in grasse es dunc scrit sin grasse del tot, plu tosc quam con un grasse quel excede.

## Quo li conversion declara haver perdit

**It di it quande it perde, e tace quande it perde nullcos.** Un raporte
quel apari a chascun appelle es un raporte quel nequi lee.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,500\n<i>sur deux</i>\nlignes\n\n' > a.srt; subedit-cli convert --to lrc --output b.lrc a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,500\n<i>sur deux</i>\nlignes\n\n' > a.srt; subedit-cli convert --to lrc --output b.lrc a.srt
a.srt: 1 subtitle written as LRC -> b.lrc
a.srt: ends are not carried by LRC, line breaks were joined in 1 subtitle, 1 tag dropped
```

Li linea apari al nivelle de verbosité predefinit, e a chascun nivelle supra.
Li elementes, in ci órdine:

| Element | Quo it di |
| :---- | :----------- |
| `ends are not carried by <format>` | li formate cible scri null fin |
| `line breaks were joined in N subtitles` | LRC solmen: N subtitules esset sur plu lineas |
| `N tags dropped` | N etiquettes ne posse esser scrit, disposition includet |
| `the header was dropped` | li capite ne passa un límite de formate |
| `the <format> fields of N subtitles were dropped` | N subtitules portava data specific de lor formate |
| `positions moved by up to N ms` | li max grand distantie per quel un position ha movet |

## Changear li codification

**`--to-encoding` selecte li codification scrit; `--encoding` di celi quel es
leet.** Li du together recodifica un file:

```console
$ subedit-cli convert --to srt --encoding cp1252 --to-encoding utf-8 \
      --output-dir sortie film.srt
```

Sin `--to-encoding`, li file es scrit in **li codification in quel it esset leet**,
quam con su fines de linea e su marca: un file convertet sin
instruction pri li codification retrodona li sam characteres, con li sam bytes.

**Un character quel li codification cible ne posse scrir halta li file**, e
nullcos es scrit. Un `ł` have null loc in Latin-1; substituer it per un `?`
perderea textu inter lectura e scrition, sin que nequi vide it.

**Un marca de órdine de bytes existe solmen por li codificationes Unicode.**
`--bom` con un `--to-encoding` quel porta null es refusat plu tosc quam ignorat
— vide li tabelle de errores ci-infra.

**E li codification deve nominar su órdine de bytes.** `UTF-16` e `UTF-32` es
nómines quel ICU conosce, queles convertitor scri **un marca propri**, quel que
on demanda: `--no-bom` serea alor acceptat e disobedit. Ambi es
dunc refusat, e li missage nómina li output:

```console
$ subedit-cli convert --to srt --to-encoding UTF-16 --output copie.srt film.srt
"UTF-16" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do
```

`UTF-16LE`, `UTF-16BE`, `UTF-32LE` e `UTF-32BE` scri exactmen li sam
bytes, marca includet — ma sub li control de `--bom` e `--no-bom`. Li
refusa dunc costa solmen un nómine plu precis, e it vale por li lectura
quam por li scrition: `--encoding UTF-16` recive li sam response.

## Li destination

Li tri optiones e lor regules es comun al subcomandes quel
scri: vide [Invocation](invocation.md#li-destination). Nullcos es scrit
sin un de ili.

Quo es specific a `convert`: **`--output-dir` changea li extension secun li
formate.** `convert --to vtt` sur `a.srt` scri `a.vtt`, nequande un `a.srt` contenent
WebVTT — un file queles nómine mente fa trebuchar chascun altri instrument.

## Converter sur li input self es refusat

`--in-place` con un `--to` quel changea li formate dona code `1` e scri
nullcos: sur li input self, ne ha un duesim nómine por portar li nov formate, e
li file resterea sub un extension quel su contenete ne justifica plu.

`--in-place` posse totvez esser usat por changear **li forme sin li formate** —
scrir un `.srt` denov quam `.srt` con altri fines de linea, p.ex.

Li refusa es decidet sur **li extension sol**, ante omni lectura: un error de
usage deve nequande lassar un lot scrit a mediage.

## Fines de linea e marca de órdine de bytes

Per predefinit, **li file scrit reprende quo li file leet portava**. Li model
conserva ambi durant li lectura; imposir les per predefinit perderea, a chascun
conversion, un information conservat expressimen.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli convert --to vtt --output b.vtt a.srt; cat b.vtt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli convert --to vtt --output b.vtt a.srt; cat b.vtt
a.srt: 1 subtitle written as WebVTT -> b.vtt
WEBVTT

00:01.000 --> 00:03.000
Bonjour.
```

## Ordinar durant li scrition

`--sort` scri li file **in li órdine del comenses**, quel que sia li formate cible. Li ordination es fat **sur un copie**: li file leet ne es
tuchat, e quo es scrit es quo [`sort`](sort.md) haverea fat — stabil, du subtitules quel comensa together conserva lor órdine.

**Nequande per predefinit**: li órdine de un file es celi de su autor tam long quam nullcos altri es demandat. Con li option, chascun file di it, al
unesim nivelle de narration, `<path>: N subtitles moved` o `<path>: already in order`, e li registre `--format json` porta un cuntage
`moved` — **absent sin li option**, por ne changear li forme de quo li scripts existent lee.

## Far disjuncte li intervalles

`--disjoint` decupe li subtitules superponent por que null intervalle superpone plu un altri, **sur li sam copie quam li ordination**: li file leet ne es tuchat.

Nequande per predefinit. Li option fa quo [`disjoint`](disjoint.md) fa, **con su parametres predefinit**, ante scrir, pos li ordination. It besona un file in órdine: si no, it es refusat, e `--sort` — quel veni unesim — mette it in órdine ante. Li file di it al unesim nivelle de narration (`<path>: N subtitles split, M merged`), e li registre `--format json` porta li cuntages `split` e `merged`, **absent sin li option**.

**`--join-repeats` veni ante li ordination e li decuperation**: un textu decupat in peces queles seque se es unesim reconstructet (vide
[`join-repeats`](join-repeats.md)), poy li subtitules es ordinat, poy li intervalles fat disjuncte. **`--repair` es omni tri in un vice**,
quam [`repair`](repair.md), con li parametres predefinit del decuperation. Con `--join-repeats`, chascun file di
`<path>: N repeated subtitles joined` al unesim nivelle de narration, e li registre `--format json` porta `counts.joined` — **absent sin li option**.

## Output

**Output standard** — nullcos: li resultate es li file scrit. Con `--dry-run`, nullcos anc: ne ha file, e li linea de narration di it — vide [Vider ante scrir](invocation.md#vider-ante-scrir).

Sur li output de error, secun li nivelle:

| Nivelle | Quo es adjuntet |
| :----- | :-------------- |
| 1 | `<path>: N subtitles written as WebVTT -> <destination>` |
| 2 | `<path>: SubRip -> WebVTT, UTF-8, no BOM, LF line endings` — li codification nominat es celi scrit |
| 3 | `<path>: N bytes read, M written`, poy **chascun diagnose de lectura** — vide [Invocation](invocation.md#diagnoses-de-lectura) |

### In JSON

Con [`--format json`](invocation.md#un-output-quel-un-script-posse-leer), li objecte de chascun file porta `counts.subtitles`, poy quo li conversion ha perdit: `lost_ends` e `lost_header` (`0` o `1`), `joined_lines`, `lost_tags`, `lost_fields` e `furthest_ms` — li elementes de [li frase de perde](#quo-li-conversion-declara-haver-perdit). Li tabelle complet es celi de [Invocation](invocation.md#quo-chascun-subcomande-pone-in-counts).

## Codes de exeunte

Celes del instrument: `0` si chascun file esset scrit, `2` si null, `3` si
solmen alcunes, `1` in cas de error de usage.

## Errores

| Quo it causa | Missage |
| :------------------ | :------ |
| `--bom` con `--no-bom` | `--bom and --no-bom ask for opposite things; give one or the other` |
| `--in-place` quel changea li formate | `--in-place cannot change the format: the file would keep a name its content no longer matches` |
| `--bom` sur un codification quel have null | `<path>: <encoding> has no byte order mark to write` |
| `--to-encoding` nominante un codification inconosset | `no encoding is named "<name>"` |
| `--to-encoding` nominante un codification quel scri su propri marca | `"<name>" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do` |
| character mancant in li codification scrit | `<path>: holds a character the chosen encoding cannot write` |

**Refusar `--bom` sur un codification quel porta null ne es
pedantie.** Un marca de órdine de bytes existe por li codificationes Unicode e
por null altri: demandar un sur un file in Windows-1252 es demandar
alquo quel ne existe. Li file serea scrit sin it, e nequi saverea
que li demande ne esset onorat.

Celes del destination es comun al subcomandes quel scri:
vide [Invocation](invocation.md#li-destination).

Li errores de lectura es celes de [`inspect`](inspect.md): ili concerne
li file de input e have li sam missages.
