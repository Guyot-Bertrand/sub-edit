# `repair`

<!-- machine-translated -->
> Deze pagina is een machinevertaling en is nog niet nagelezen door een lezer van de taal.

```
subedit-cli repair (--output FILE | --output-dir DIR | --in-place)
                   [--minimum TIME] [--max-lines N] [--dashes]
                   [--dry-run] [--recursive]
                   <file>...
```

Doet in één keer wat drie commando's na elkaar doen: [`join-repeats`](join-repeats.md), dan
[`sort`](sort.md), dan [`disjoint`](disjoint.md). **In die volgorde, de enige die werkt**: het knippen leest
de volgorde van de begintijden, en de herhaalde stukken van één tekst zijn beter weg vóór beide. Op een bestand
dat slecht is gesegmenteerd, niet in volgorde staat en overlapt, is dit het commando om uit te voeren.

De instellingen zijn die van `disjoint`, dat de keten afsluit; `--sort` bestaat hier niet, aangezien er altijd wordt gesorteerd.

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

## Argumenten en opties

| Optie | Verplicht | Waarden | Standaard |
| :----- | :----- | :------ | :----- |
| `<file>...` | ja | een of meer paden | — |
| `--minimum` | nee | een duur: `0.08`, `1.5`, `00:00:00,080`. Nul behoudt elk interval | `0.08` (ongeveer twee frames) |
| `--max-lines` | nee | een geheel getal; `0` heft de limiet op | `2` |
| `--dashes` | nee | een vlag: een streepje voor de tekst die elke ondertitel aan een samengevoegde ondertitel bijdraagt | uit |
| `--recursive`, `-r` | nee | een vlag | uit — zie [Een mappenstructuur verwerken](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **een van de drie**, behalve met `--dry-run` | zie [Aanroep](invocation.md#de-bestemming) | — |
| `--dry-run` | nee | een vlag: berekent en meldt, schrijft niets — zie [Kijken voor het schrijven](invocation.md#kijken-voor-het-schrijven) | uit |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:01,000 --> 00:00:04,000\nBeta\n\n' > segments.srt; subedit-cli repair --output repare.srt segments.srt; cat repare.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:01,000 --> 00:00:04,000\nBeta\n\n' > segments.srt; subedit-cli repair --output repare.srt segments.srt; cat repare.srt
segments.srt: 1 repeated subtitle joined; already in order; 0 subtitles split, 1 merged -> repare.srt
1
00:00:01,000 --> 00:00:04,000
Alpha
Beta
```

`Alpha`, in twee op elkaar volgende stukken geknipt, wordt weer één ondertitel; die beslaat dan hetzelfde bereik als `Beta`,
en de twee vormen er nog maar één, van twee regels, de vroegste eerst.

## Uitvoer

**Standaarduitvoer** — niets: het resultaat is het geschreven bestand. Op de foutuitvoer, op het eerste niveau, **een zin
die de drie tellingen aaneenschakelt**, gescheiden door puntkomma's: wat `join-repeats`, `sort` en `disjoint` elk zeggen,
in hun eigen woorden — `N repeated subtitles joined; M subtitles moved; K subtitles split, L merged…`.

### In JSON

Met [`--format json`](invocation.md#uitvoer-die-een-script-kan-lezen) de `counts` van elk bestand: `subtitles`, `joined`,
`moved`, daarna die van [`disjoint`](disjoint.md#in-json) (`split`, `merged`, `windowed`, `absorbed`, `too_short`,
`ending_before_start`, `extras_dropped`).

## Afsluitcodes en fouten

Die van het hulpprogramma. Geen enkele fout is specifiek voor `repair` buiten de instellingen, die worden beoordeeld zoals die van `disjoint`:
`--minimum` dat geen duur is of negatief is, `--max-lines` dat geen geheel getal is, code `1`.
