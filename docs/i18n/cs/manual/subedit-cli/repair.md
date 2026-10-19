# `repair`

<!-- machine-translated -->
> Tato stránka je strojovým překladem a dosud ji nezkontroloval žádný čtenář tohoto jazyka.

```
subedit-cli repair (--output FILE | --output-dir DIR | --in-place)
                   [--minimum TIME] [--max-lines N] [--dashes]
                   [--dry-run] [--recursive]
                   <file>...
```

Provede najednou to, co po sobě dělají tři příkazy: [`join-repeats`](join-repeats.md), pak
[`sort`](sort.md), pak [`disjoint`](disjoint.md). **V tomto pořadí, které je jediné funkční**: dělení čte
pořadí začátků a opakované kusy jednoho textu je lepší odstranit před obojím. U souboru,
který je špatně segmentovaný, neseřazený a překrývající se, je to příkaz, který je třeba spustit.

Nastavení jsou nastaveními podpříkazu `disjoint`, který řetězec uzavírá; `--sort` tu neexistuje, protože se řadí vždy.

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

## Argumenty a volby

| Volba | Povinná | Hodnoty | Výchozí |
| :----- | :----- | :------ | :----- |
| `<file>...` | ano | jedna nebo více cest | — |
| `--minimum` | ne | doba trvání: `0.08`, `1.5`, `00:00:00,080`. Nula ponechá všechny intervaly | `0.08` (asi dva snímky) |
| `--max-lines` | ne | celé číslo; `0` omezení zruší | `2` |
| `--dashes` | ne | přepínač: pomlčka před textem, který každý titulek přináší do sloučeného titulku | vypnuto |
| `--recursive`, `-r` | ne | přepínač | vypnuto — viz [Zpracování stromu](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **jedna ze tří**, kromě `--dry-run` | viz [Spuštění](invocation.md#cíl-zápisu) | — |
| `--dry-run` | ne | přepínač: spočítá a vypíše, nic nezapíše — viz [Náhled před zápisem](invocation.md#náhled-před-zápisem) | vypnuto |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:01,000 --> 00:00:04,000\nBeta\n\n' > segments.srt; subedit-cli repair --output repare.srt segments.srt; cat repare.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:01,000 --> 00:00:04,000\nBeta\n\n' > segments.srt; subedit-cli repair --output repare.srt segments.srt; cat repare.srt
segments.srt: 1 repeated subtitle joined; already in order; 0 subtitles split, 1 merged -> repare.srt
1
00:00:01,000 --> 00:00:04,000
Alpha
Beta
```

`Alpha`, rozdělená na dva kusy, které po sobě následují, se znovu stane jediným titulkem; pak zabírá stejný rozsah jako `Beta`
a oba tvoří jen jeden titulek o dvou řádcích, nejdřívější první.

## Výstup

**Standardní výstup** — nic: výsledkem je zapsaný soubor. Na chybovém výstupu, na první úrovni, **věta,
která řetězí tři počty**, oddělené středníky: co každý z podpříkazů `join-repeats`, `sort` a `disjoint` říká,
jeho vlastními slovy — `N repeated subtitles joined; M subtitles moved; K subtitles split, L merged…`.

### Ve formátu JSON

S [`--format json`](invocation.md#výstup-čitelný-pro-skript) jsou `counts` každého souboru: `subtitles`, `joined`,
`moved`, pak počty podpříkazu [`disjoint`](disjoint.md#ve-formátu-json) (`split`, `merged`, `windowed`, `absorbed`, `too_short`,
`ending_before_start`, `extras_dropped`).

## Návratové kódy a chyby

Kódy a chyby nástroje. Žádná chyba není vlastní podpříkazu `repair` kromě nastavení, která se posuzují jako u podpříkazu `disjoint`:
`--minimum`, které není doba trvání nebo je záporné, `--max-lines`, které není celé číslo, kód `1`.
