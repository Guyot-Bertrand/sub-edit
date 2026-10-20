# `repair`

<!-- machine-translated -->
> Þessi síða er vélþýðing sem enginn lesandi tungumálsins hefur enn farið yfir.

```
subedit-cli repair (--output FILE | --output-dir DIR | --in-place)
                   [--minimum TIME] [--max-lines N] [--dashes]
                   [--dry-run] [--recursive]
                   <file>...
```

Gerir í einu lagi það sem þrjár skipanir gera hver á eftir annarri: [`join-repeats`](join-repeats.md), síðan
[`sort`](sort.md), síðan [`disjoint`](disjoint.md). **Í þessari röð, sem er sú eina sem gengur**: skiptingin les
röð upphafanna, og endurteknir hlutar sama texta eru betur farnir áður en hvor tveggja fer fram. Á skrá
sem er illa skipt, í ólagi og með skörun er þetta skipunin sem á að keyra.

Stillingarnar eru þær sömu og í `disjoint`, sem lokar keðjunni; `--sort` er ekki til hér, þar sem röðun fer alltaf fram.

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

## Röksemdir og valkostir

| Valkostur | Nauðsynlegt | Gildi | Sjálfgefið |
| :----- | :----- | :------ | :----- |
| `<file>...` | já | ein eða fleiri slóðir | — |
| `--minimum` | nei | tímalengd: `0.08`, `1.5`, `00:00:00,080`. Núll heldur öllum bilum | `0.08` (um tveir rammar) |
| `--max-lines` | nei | heiltala; `0` afnemur takmörkunina | `2` |
| `--dashes` | nei | rofi: þankastrik fyrir framan textann sem hver skjátexti leggur til sameinaðs skjátexta | slökkt |
| `--recursive`, `-r` | nei | rofi | slökkt — sjá [Vinnsla á möpputré](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **eitt af þremur**, nema með `--dry-run` | sjá [Ræsing](invocation.md#áfangastaðurinn) | — |
| `--dry-run` | nei | rofi: reiknar og segir frá, skrifar ekkert — sjá [Að sjá áður en skrifað er](invocation.md#að-sjá-áður-en-skrifað-er) | slökkt |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:01,000 --> 00:00:04,000\nBeta\n\n' > segments.srt; subedit-cli repair --output repare.srt segments.srt; cat repare.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:01,000 --> 00:00:04,000\nBeta\n\n' > segments.srt; subedit-cli repair --output repare.srt segments.srt; cat repare.srt
segments.srt: 1 repeated subtitle joined; already in order; 0 subtitles split, 1 merged -> repare.srt
1
00:00:01,000 --> 00:00:04,000
Alpha
Beta
```

`Alpha`, sem var skipt í tvo hluta sem fylgja hvor öðrum, verður aftur að einum skjátexta; hann tekur þá yfir sama tímabil og `Beta`,
og þeir tveir verða aðeins einn, með tveimur línum, þeirri fyrri fyrst.

## Úttak

**Staðalúttak** — ekkert: niðurstaðan er skráin sem er skrifuð. Á villuúttakinu, á fyrsta stigi, kemur **ein setning
sem tengir saman þrjár talningarnar**, aðskildar með semíkommum: það sem `join-repeats`, `sort` og `disjoint` segja hver um sig,
með eigin orðum — `N repeated subtitles joined; M subtitles moved; K subtitles split, L merged…`.

### Á JSON-sniði

Með [`--format json`](invocation.md#úttak-sem-skriftur-geta-lesið) eru `counts` hverrar skrár: `subtitles`, `joined`,
`moved`, síðan þær úr [`disjoint`](disjoint.md#á-json-sniði) (`split`, `merged`, `windowed`, `absorbed`, `too_short`,
`ending_before_start`, `extras_dropped`).

## Skilakóðar og villur

Þeir sömu og hjá tólinu. Engin villa er sérstök fyrir `repair` umfram stillingarnar, sem eru metnar eins og stillingar `disjoint`:
`--minimum` sem er ekki tímalengd eða er neikvætt, `--max-lines` sem er ekki heiltala, kóði `1`.
