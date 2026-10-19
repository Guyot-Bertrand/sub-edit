# `snap`

<!-- machine-translated -->
> Ca págine es un traduction machinal e it ancor ne ha esset revidet de un lector del lingue.

```
subedit-cli snap --rate <rate>
                 (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 <file>...
```

Pone **chascun marca de tempore** — comense e fin — sur li plu proxim fotogramma del
frequentie de fotogrammas indicat. Nequo es retemporisat: chascun position move se de un demi fotogramma
max.

## `snap` ne es `framerate`, e confunder les ne es visibil

Li du comandes have li sam forme — files, un destination, un frequentie de fotogrammas — e fa choses oppositi.

| | [`framerate`](framerate.md) | `snap` |
| :--- | :-------------------------- | :----- |
| quo it calcula | `t' = t x input rate / output rate` | li plu proxim fotogramma, sur li grille de `--rate` |
| de quant it move | proportional al tempore — **pluri secundas** sur un long metrage | un demi fotogramma max, **omniloc** |
| por qual file | un file queles **temporisation es fals** | un file queles temporisation es **just** e queles grille es fals |

**Li cas de usage de `snap`**, explicat: un film a 25 fotogrammas per
second, un file de subtitules scrit sur un grille a 24, lineas ja
plu o minus a lor loc ad alcun millisecundas. Ne es nequo a retemporisar
— solmen a remetter li marcas de tempore sur li just fotogrammas.

**Li cas de usage de `framerate`** es li altri: un file temporisat por un master a
25 quel on vole reproducter sur un copie a 23.976, e queles temporisation derive de
pluri secundas al fine del film.

Prender un por li altri **causa null error**: ambes
accepta li file e scri un resultate. Li linea de narration es quo
les distinde — `snap` dis sempre de quant it ha movet li positiones.

<!-- example: subedit-cli snap --help -->
```console
$ subedit-cli snap --help
Move every position onto the nearest frame of a frame rate (see framerate)
Usage: subedit-cli snap [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to align

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --rate TEXT REQUIRED        Frame rate to align on: 25, 23.976
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Arguments e optiones

| Option | Obligatori | Valores | Predefinit |
| :----- | :----- | :------ | :----- |
| `<file>...` | yes | un o plu chemins | — |
| `--recursive`, `-r` | no | un flag | off — vide [Tractar un arbore](lots.md) |
| `--rate` | **yes** | un frequentie de fotogrammas in fotogrammas per second, scrit quam por [`framerate`](framerate.md#qualmen-scrir-un-frequentie-de-fotogrammas) | — |
| `--output` / `--output-dir` / `--in-place` | **un del tri**, except con `--dry-run` | vide [Invocation](invocation.md#li-destination) | — |
| `--dry-run` | no | un flag: calcula e dis, scri null — vide [Vider ante scrir](invocation.md#vider-ante-scrir) | off |

`--rate` accepta **omni frequentie de fotogrammas valid**, e ne solmen li ot
standard: it es li electe del usator, quam por `framerate`. Un frequentie de fotogrammas
zero o negativ es refusat.

Li formate del file leet es **conservat**: changear li formate es li labor de
[`convert`](convert.md).

## Output

**Exeunte standard** — null: li resultate es li file scrit. Con `--dry-run`, anc null: ne es file, e li linea de narration dis it — vide [Vider ante scrir](invocation.md#vider-ante-scrir).

| Nivelle | Quo es adjuntet al exeunte de error |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles aligned on 25 fps, M positions moved, by at most 20 ms -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — li formate, li codification, li marca e li fines de linea del file leet, remetet quam les esset |
| 3 | `<path>: N bytes read, M written`, poy **omni diagnose de lectura** — vide [Invocation](invocation.md#diagnoses-de-lectura) |

**Li du númeres es li punctu del linea.** Un demi fotogramma a 25 fotogrammas per
second es vint millisecundas, e nequo plu grand posse esser scrit ibi. Qui
voleva `framerate` e ha tipat `snap` lee «by at most 20 ms» u it expectat
secundas, e save it strax.

## Un exemple

Un linea a `00:00:01,010`, alliniat a 25 fotogrammas per second — dunc sur un
multiple de quaranta millisecundas:

<!-- example: printf '1\n00:00:01,010 --> 00:00:03,020\nUn.\n\n' > a.srt; subedit-cli snap --rate 25 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,010 --> 00:00:03,020\nUn.\n\n' > a.srt; subedit-cli snap --rate 25 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle aligned on 25 fps, 2 positions moved, by at most 20 ms -> b.srt
1
00:00:01,000 --> 00:00:03,040
Un.
```

## Du propriétés

**Alliniar du vices ne changea nequo li duesim vice.** Un position ja sur un
fotogramma resta ibi: it es li sam operation aplicat al sam resultate.

**Li órdine del subtitules es conservat.** Arondar al plu proxim fotogramma es
monoton, dunc du comenses distant de minimum un fotogramma resta tal. Plu
proxim, les posse coalescer — quo es un anomalie quel
[`inspect`](inspect.md#quo-anomalies-reporta) save nominar, e ne un
inversion.

### In JSON

Con [`--format json`](invocation.md#un-output-quel-un-script-posse-leer), li objecte de chascun file porta `counts.subtitles`, `counts.moved` (li positiones movet) e `counts.furthest_ms` (li max movement, in millisecundas). Li tabelle complet es li de [Invocation](invocation.md#quo-chascun-subcomande-pone-in-counts).

## Codes de exeunte

Les del instrument: `0` si omni file esset scrit, `2` si null, `3` si
solmen alcunes, `1` in cas de un error de usage.

## Errores

Les de un frequentie de fotogrammas, parol por parol les de [`framerate`](framerate.md#errores)
— ambes lee li sam valore per li sam via:

| Quo it causa | Missage |
| :------------------ | :------ |
| frequentie de fotogrammas zero o negativ | `"…" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed` |
| comma decimal | `"…" is not a frame rate: use a decimal point and not a comma, in every language` |
| textu quel ne es un númere | `"…" is not a frame rate: expected frames per second, like 25 or 23.976` |
| frequentie de fotogrammas foris omni proportion | `"…" is not a frame rate: no video runs at that many frames per second` |

**Nequo es scrit sin un del tri optiones de destination**, e lor errores
es comun a su li subcomandes quel scri:
vide [Invocation](invocation.md#li-destination).
