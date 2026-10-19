# `append`

<!-- machine-translated -->
> Ca págine es un traduction machinal e it ancor ne ha esset revidet de un lector del lingue.

```
subedit-cli append --output FILE
                   [--dry-run] [--sort] [--disjoint]
                   <base> <file>...
```

Mette files **pos li unesim** in **un sol** file: li film in du partes, transformat in un
sol aggregat de subtitules. It es `Append File…` del fenestre — [li sam motor](../subedit-gui/operations.md#append-file),
li sam cuntages — aplicat tam ofte quam ha files a apender.

- **Li unesim file es li basa.** It dona al resultate **su formate, su codification, su marca de
  órdine de bytes e su fines de linea**.
- **Nullcos es inserit inter du files**: chascun es displaciat per **li fin del ultim subtitul de quo
  precede it**, su positiones conservante lor órdine e lor interval. Li tertie seque li duesim, ne li
  basa: li files seque se in li órdine in queles ili es dat.
- **Chascun file apendet passa per li conversion** al formate del basa: su etiquettes es traductet,
  e quo ti formate ne posse scrir es dit, file per file, in li paroles del fenestre.
- **Li files apendet have null traduction**: un file aperit sol ne have alquo a dar.
- **Un file posse esser dat plu vices**, o apendet a se self: nullcos li interdi.

**`append` ne es un lot.** N inputs, **un** output: li bucle del altri subcomandes — un resultate per
file, e un fallimente quel lassa li altres continuar — ha null sense ci. Tri consequenties seque:

- **li destination es `--output`, e nullcos altri.** `--output-dir` e `--in-place` ne existe por
  `append` e es refusat quam omni option inconosset, code `1`; `--recursive` anc: li órdine del
  files es celi dat, e un arbore percurset have null;
- **omnicos o nullcos**: li unesim file quel ne posse esser leet halta li execution, e **nullcos es scrit**. Un
  film in tri partes de quel un manca ne es un film in du;
- **un sol linea per execution** por li resultate, e, in JSON, **un sol objecte**.

<!-- example: subedit-cli append --help -->
```console
$ subedit-cli append --help
Put subtitle files one after another into a single file
Usage: subedit-cli append [OPTIONS] files...

Positionals:
  files BASE FILE...          The base file, then the files to put after it, in order

Options:
  -h,--help                   Print this help message and exit
  --output FILE               File to write
  --dry-run                   Work out and say what would be written, and write nothing
  --sort                      Put the subtitles in order of their start before writing
  --disjoint                  Cut overlapping subtitles so that no range overlaps another, before writing
```

## Argumentes e optiones

| Option | Obligatori | Valores | Predefinit |
| :----- | :----- | :------ | :----- |
| `<base> <file>...` | yes, **al minus du** | vias: li basa, poy quo es mettet pos it, in órdine | — |
| `--output` | **yes**, except con `--dry-run` | un via de file; su directoria es creat si it manca | — |
| `--dry-run` | no | un flag: calcula e di, scri nullcos — vide [Vider ante scrir](invocation.md#vider-ante-scrir) | off |
| `--sort` | no | un flag: mette li total in li órdine de su comenses **pos que omnicos es apendet**, vide [Ordinar li resultate](#ordinar-li-resultate) | off |
| `--disjoint` | no | un flag: decupe li subtitules superponent **pos que omnicos es apendet e ordinat**, vide [Far disjuncte li intervalles](#far-disjuncte-li-intervalles) | off |

**Li destination ne posse esser alcun del inputs**, li basa includet: scrir sur un file quel on
lee es refusat ante que alquo es leet, code `1`. Un `--output` existent e distinct del inputs es
scrit sur, quam in omni loc.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n' > partie1.srt; printf '1\n00:00:00,500 --> 00:00:01,500\ntrois\n\n' > partie2.srt; subedit-cli append --output film.srt partie1.srt partie2.srt; cat film.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n' > partie1.srt; printf '1\n00:00:00,500 --> 00:00:01,500\ntrois\n\n' > partie2.srt; subedit-cli append --output film.srt partie1.srt partie2.srt; cat film.srt
partie2.srt: appended 1 subtitle
partie1.srt: 3 subtitles from 2 files -> film.srt
1
00:00:01,000 --> 00:00:02,000
un

2
00:00:03,000 --> 00:00:04,000
deux

3
00:00:04,500 --> 00:00:05,500
trois
```

Li ultim subtitul de `partie1.srt` fini a 4 s; celi de `partie2.srt`, quel comensat a 0,5 s, comensa a
4,5 s.

## Ordinar li resultate

Chascun file es displaciat **ex li fin de quo precede it**: li órdine quel posse rupter se es dunc celi del files self. `--sort`
mette li resultate in li órdine del comenses, **in un vice, pos li ultim apension**, quam [`sort`](sort.md) fa — stabil, e sin changear nullcos
in li contenete.

Nequande per predefinit. Con li option, li basa di it al unesim nivelle de narration, `<base>: N subtitles moved` o `<base>: already in order`, e
li registre `--format json` porta un cuntage `moved`, **absent sin li option**.

## Far disjuncte li intervalles

Nequande per predefinit. Li option fa quo [`disjoint`](disjoint.md) fa, **con su parametres predefinit**, pos que omnicos es apendet, sur li total. It besona un file in órdine: si no, it es refusat, e `--sort` — quel veni unesim — mette it in órdine ante. Li file di it al unesim nivelle de narration (`<path>: N subtitles split, M merged`), e li registre `--format json` porta li cuntages `split` e `merged`, **absent sin li option**.

## Quande li formates diffère

Li resultate es in **li formate del basa**, quel que sia celi del altres. Quo li transit costa es
dit file per file, in li paroles del fenestre — li sam frase quam por `Append File…` e por
[collar ex un altri formate](../subedit-gui/clipboard.md#li-etiquettes-de-un-altri-formate-es-traductet):

```text
d.ass: appended 1 subtitle; Advanced SSA into SubRip: 1 tag dropped, the header was dropped
```

Quo es perdit depende solmen del formate de **chascun** file apendet: un basa in Advanced SSA recive un
file SubRip sin perder nullcos, ne li contrari. Por selecter li formate del resultate, mette unesim un file
ja in ti formate, o converte pos con [`convert`](convert.md).

## Output

**Output standard** — nullcos: li resultate es li file scrit.

| Nivelle | Quo es adjuntet al output de error |
| :----- | :------------------------------------- |
| 1 | un linea per file apendet — `<path>: appended N subtitles`, sequet, quande li formate ha costat alquo, per `; <source format> into <base format>: <what is lost>` — poy `<base>: N subtitles from M files -> <destination>` (con `--dry-run`, `(dry run, nothing written)` vice `-> <destination>`) |
| 2 | `<base>: SubRip, UTF-8, no BOM, LF line endings kept` — li formate, li codification, li marca e li fines de linea del basa, remisset quam ili esset |
| 3 | `<base>: N bytes read, M written` (omni files together), poy **chascun diagnose de lectura** — vide [Invocation](invocation.md#diagnoses-de-lectura) |

### In JSON

Con [`--format json`](invocation.md#un-output-quel-un-script-posse-leer), **un sol objecte** por li tot execution,
quel `file` es **li basa**. Su `counts`:

| Cuntage | Quo it di |
| :----- | :----------- |
| `files` | li files del execution, li basa includet |
| `subtitles` | li subtitules del resultate |
| `appended` | celes queles veni del files apendet |
| `lost_ends`, `lost_header` | `0` o `1`: un del files apendet l'ha perdit |
| `joined_lines`, `lost_tags`, `lost_fields` | li summa de quo li files apendet ha perdit |
| `furthest_ms` | li max grand displaciament de un position, omni files together |

**Un array `inputs`** accompania li `counts`, foris les: un element per file apendet, in órdine,
`{"file", "counts"}` — `counts` portante `appended`, poy li sam perdes, **por ti file sol**. It es to quel
di, in un execution de dece files, qual ha costat quo. Li advertimentes de lectura de omni files
es reunit in `warnings`.

Un fallimente es anc un objecte, **un sol**: celi del unesim file quel ne posse esser leet.

## Codes de exeunte

| Code | Quande |
| :--- | :---- |
| `0` | li resultate es scrit — o calculat, con `--dry-run` |
| `1` | error de usage: minu de du files, null `--output`, un option quel `append` ne have, un destination quel es un del inputs |
| `2` | **un file ne posse esser leet**, o li destination ne posse esser scrit: nullcos es scrit. Null code `3` existe: un execution quel scri solmen un file successa o ne |

## Errores

| Quo les causa | Missage |
| :------------------- | :------ |
| un sol file dat | `append needs a base file and at least one file to put after it` |
| ni `--output` ni `--dry-run` | `no destination given: append writes one file, use --output` |
| `--output` nómina un input | `<destination>: would be written over the input <input>` |
| un file mancant, illegibil o de un formate inconosset | `<path>: does not exist`, `<path>: is in no format this tool knows`… — li sam quam in omni loc |

**Un file vacui ne es apendet**: null formate lee sin al minus un subtitul, e li file es
refusat quam omni file quel ne posse esser leet — `<path>: holds nothing recognisable as a subtitle` —, quo
halta li execution e scri nullcos.
