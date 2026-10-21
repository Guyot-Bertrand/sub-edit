# `repair`

<!-- machine-translated -->
> Ова страница је машински превод који још није прегледао читалац језика.

```
subedit-cli repair (--output FILE | --output-dir DIR | --in-place)
                   [--minimum TIME] [--max-lines N] [--dashes]
                   [--dry-run] [--recursive]
                   <file>...
```

Одједном ради оно што три команде раде једна за другом: [`join-repeats`](join-repeats.md), затим
[`sort`](sort.md), затим [`disjoint`](disjoint.md). **Тим редоследом, јединим који ради**: сечење
чита редослед почетака, а поновљене делове једног текста боље је уклонити пре једног и другог. За датотеку
која је лоше сегментирана, неуређена и са преклапањима, ово је команда коју треба покренути.

Поставке су оне подкоманде `disjoint`, која завршава ланац; опција `--sort` овде не постоји, јер се ређање увек обавља.

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

## Аргументи и опције

| Опција | Обавезно | Вредности | Подразумевано |
| :----- | :----- | :------ | :----- |
| `<file>...` | да | једна или више путања | — |
| `--minimum` | не | трајање: `0.08`, `1.5`, `00:00:00,080`. Нула задржава сваки интервал | `0.08` (око два кадра) |
| `--max-lines` | не | цео број; `0` скида ограничење | `2` |
| `--dashes` | не | опција без вредности: цртица испред текста који сваки титл уноси у спојени титл | искључено |
| `--recursive`, `-r` | не | опција без вредности | искључено — види [Обрада стабла директоријума](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **једна од три**, осим уз `--dry-run` | види [Покретање](invocation.md#одредиште) | — |
| `--dry-run` | не | опција без вредности: израчуна и јави, ништа не пише — види [Прво видети, па писати](invocation.md#прво-видети-па-писати) | искључено |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:01,000 --> 00:00:04,000\nBeta\n\n' > segments.srt; subedit-cli repair --output repare.srt segments.srt; cat repare.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:01,000 --> 00:00:04,000\nBeta\n\n' > segments.srt; subedit-cli repair --output repare.srt segments.srt; cat repare.srt
segments.srt: 1 repeated subtitle joined; already in order; 0 subtitles split, 1 merged -> repare.srt
1
00:00:01,000 --> 00:00:04,000
Alpha
Beta
```

`Alpha`, исечен на два дела који следе један за другим, поново постаје један титл; он тада заузима исти интервал као `Beta`,
па њих двоје чине само један, од два реда, најранији први.

## Излаз

**Стандардни излаз** — ништа: резултат је написана датотека. На стандардној грешци, на првом нивоу, **једна реченица
која надовезује три броја**, одвојена тачкама са зарезом: оно што `join-repeats`, `sort` и `disjoint` свака за себе кажу,
својим речима — `N repeated subtitles joined; M subtitles moved; K subtitles split, L merged…`.

### У JSON-у

Уз [`--format json`](invocation.md#излаз-који-скрипта-може-да-чита), `counts` сваке датотеке: `subtitles`, `joined`,
`moved`, па они из подкоманде [`disjoint`](disjoint.md#у-json-у) (`split`, `merged`, `windowed`, `absorbed`, `too_short`,
`ending_before_start`, `extras_dropped`).

## Излазни кодови и грешке

Они из програма. Ниједна грешка није својствена подкоманди `repair` изван поставки, које се оцењују као оне подкоманде `disjoint`:
`--minimum` који није трајање или је негативан, `--max-lines` који није цео број, код `1`.
