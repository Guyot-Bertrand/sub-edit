# `repair`

<!-- machine-translated -->
> Ca págine es un traduction machinal e it ancor ne ha esset revidet de un lector del lingue.

```
subedit-cli repair (--output FILE | --output-dir DIR | --in-place)
                   [--minimum TIME] [--max-lines N] [--dashes]
                   [--dry-run] [--recursive]
                   <file>...
```

Fa in un vice quo tri comandes fa un pos li altri: [`join-repeats`](join-repeats.md), poy
[`sort`](sort.md), poy [`disjoint`](disjoint.md). **In ca órdine, li sol quel functiona**: li decupation lee
li órdine de comenses, e li pezzes repetit de un textu es melior ja disparit ante li du altres. Por un file
mal segmentat, foris de órdine e con superpositiones, it es li comande a usar.

Li parametres es les de `disjoint`, quel fine li catene; `--sort` ne existe ci, pro que li ordination es sempre fat.

<!-- example: subedit-cli repair --help -->
```console
$ subedit-cli repair --help
Join the repeated subtitles, put them in order, then make the ranges disjoint
Usage: subedit-cli repair [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to repair

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --minimum TIME              Shortest interval kept; a shorter one is taken over by a neighbour (default 0.08)
  --max-lines N               Most lines of a merged subtitle; more are shown in windows, 0 for no limit (default 2)
  --dashes                    Put a dash before the text of each merged subtitle
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Arguments e optiones

| Option | Obligatori | Valores | Predefinit |
| :----- | :----- | :------ | :----- |
| `<file>...` | yes | un o plu chemins | — |
| `--minimum` | no | un duration: `0.08`, `1.5`, `00:00:00,080`. Zero conserva chascun intervalle | `0.08` (cirque du fotogrammas) |
| `--max-lines` | no | un númere intere; `0` leva li limite | `2` |
| `--dashes` | no | un flag: un strec ante li textu quel chascun subtitul apporta a un subtitul coalescet | off |
| `--recursive`, `-r` | no | un flag | off — vide [Tractar un arbore](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **un del tri**, except con `--dry-run` | vide [Invocation](invocation.md#li-destination) | — |
| `--dry-run` | no | un flag: calcula e dis, scri null — vide [Vider ante scrir](invocation.md#vider-ante-scrir) | off |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:01,000 --> 00:00:04,000\nBeta\n\n' > segments.srt; subedit-cli repair --output repare.srt segments.srt; cat repare.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:01,000 --> 00:00:04,000\nBeta\n\n' > segments.srt; subedit-cli repair --output repare.srt segments.srt; cat repare.srt
segments.srt: 1 repeated subtitle joined; already in order; 0 subtitles split, 1 merged -> repare.srt
1
00:00:01,000 --> 00:00:04,000
Alpha
Beta
```

`Alpha`, decupat in du pezzes quel seque se, deveni denov un sol subtitul; it occupa poy li sam intervalle quam `Beta`,
e li du fa solmen un, de du lineas, li plu precoce in prim.

## Output

**Exeunte standard** — null: li resultate es li file scrit. Sur li exeunte de error, al prim nivelle, **un frase
quel liga li tri cuntages**, separat per punctocommas: quo `join-repeats`, `sort` e `disjoint` dis chascun,
per su propri paroles — `N repeated subtitles joined; M subtitles moved; K subtitles split, L merged…`.

### In JSON

Con [`--format json`](invocation.md#un-output-quel-un-script-posse-leer), li `counts` de chascun file: `subtitles`, `joined`,
`moved`, poy les de [`disjoint`](disjoint.md#in-json) (`split`, `merged`, `windowed`, `absorbed`, `too_short`,
`ending_before_start`, `extras_dropped`).

## Codes de exeunte e errores

Les del instrument. Null error es specific a `repair` ultra li parametres, queles es judicat quam les de `disjoint`:
`--minimum` quel ne es un duration o es negativ, `--max-lines` quel ne es un númere intere, code `1`.
