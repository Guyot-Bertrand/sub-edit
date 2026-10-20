# `repair`

<!-- machine-translated -->
> Ez az oldal gépi fordítás, amelyet a nyelv egyik olvasója még nem nézett át.

```
subedit-cli repair (--output FILE | --output-dir DIR | --in-place)
                   [--minimum TIME] [--max-lines N] [--dashes]
                   [--dry-run] [--recursive]
                   <file>...
```

Egyetlen menetben elvégzi azt, amit három parancs egymás után: a [`join-repeats`](join-repeats.md), majd a
[`sort`](sort.md), majd a [`disjoint`](disjoint.md). **Ebben a sorrendben, amely az egyetlen működő**: a vágás
a kezdetek sorrendjéből dolgozik, és egy szöveg ismétlődő darabjait érdemes mindkettő előtt eltüntetni. Egy
rosszul szegmentált, rendezetlen és átfedő fájlnál ez a futtatandó parancs.

A beállítások a `disjoint` beállításai, amely lezárja a láncot; a `--sort` kapcsoló itt nem létezik, mert a rendezés mindig megtörténik.

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

## Argumentumok és kapcsolók

| Kapcsoló | Kötelező | Értékek | Alapértelmezés |
| :----- | :----- | :------ | :----- |
| `<file>...` | igen | egy vagy több útvonal | — |
| `--minimum` | nem | időtartam: `0.08`, `1.5`, `00:00:00,080`. A nulla minden intervallumot megtart | `0.08` (nagyjából két képkocka) |
| `--max-lines` | nem | egész szám; a `0` megszünteti a korlátot | `2` |
| `--dashes` | nem | jelzőkapcsoló: kötőjel az egyesített feliratsorba kerülő egyes szövegek előtt | ki |
| `--recursive`, `-r` | nem | jelzőkapcsoló | ki — lásd [Könyvtárfa feldolgozása](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **a három közül egy**, kivéve a `--dry-run` esetén | lásd [Indítás](invocation.md#a-cél) | — |
| `--dry-run` | nem | jelzőkapcsoló: kiszámítja és közli, de nem ír semmit — lásd [Megnézés írás előtt](invocation.md#megnézés-írás-előtt) | ki |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:01,000 --> 00:00:04,000\nBeta\n\n' > segments.srt; subedit-cli repair --output repare.srt segments.srt; cat repare.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:01,000 --> 00:00:04,000\nBeta\n\n' > segments.srt; subedit-cli repair --output repare.srt segments.srt; cat repare.srt
segments.srt: 1 repeated subtitle joined; already in order; 0 subtitles split, 1 merged -> repare.srt
1
00:00:01,000 --> 00:00:04,000
Alpha
Beta
```

Az `Alpha`, amely két egymást követő darabra volt vágva, ismét egyetlen feliratsor lesz; ez ezután ugyanazt a tartományt foglalja el, mint a `Beta`,
és a kettőből csak egy lesz, két sorral, a korábbi elöl.

## Kimenet

**Szabványos kimenet** — semmi: az eredmény a megírt fájl. A hibakimeneten, az első szinten, **egy mondat
fűzi össze a három számot**, pontosvesszővel elválasztva: amit a `join-repeats`, a `sort` és a `disjoint` külön-külön
mond, a saját szavaival — `N repeated subtitles joined; M subtitles moved; K subtitles split, L merged…`.

### JSON-ban

A [`--format json`](invocation.md#szkript-által-olvasható-kimenet) esetén minden fájl `counts` mezője: `subtitles`, `joined`,
`moved`, majd a [`disjoint`](disjoint.md#json-ban) mezői (`split`, `merged`, `windowed`, `absorbed`, `too_short`,
`ending_before_start`, `extras_dropped`).

## Kilépési kódok és hibák

Az eszköz általános kódjai és hibái. A beállításokon túl egy hiba sem sajátja a `repair`-nek, a beállításokat pedig úgy ítéli meg, mint a `disjoint` esetében:
a nem időtartam vagy negatív `--minimum`, a nem egész szám `--max-lines`: `1` kód.
