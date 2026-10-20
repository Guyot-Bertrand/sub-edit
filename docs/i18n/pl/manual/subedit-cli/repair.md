# `repair`

<!-- machine-translated -->
> Ta strona jest tłumaczeniem maszynowym i nie została jeszcze sprawdzona przez osobę władającą tym językiem.

```
subedit-cli repair (--output FILE | --output-dir DIR | --in-place)
                   [--minimum TIME] [--max-lines N] [--dashes]
                   [--dry-run] [--recursive]
                   <file>...
```

Wykonuje za jednym razem to, co trzy polecenia jedno po drugim: [`join-repeats`](join-repeats.md), potem
[`sort`](sort.md), potem [`disjoint`](disjoint.md). **W tej kolejności, która jest jedyną działającą**: cięcie odczytuje
kolejność początków, a powtórzone kawałki jednego tekstu lepiej usunąć przed jednym i drugim. Dla pliku
źle podzielonego, nieuporządkowanego i z nakładającymi się napisami to właściwe polecenie.

Ustawienia są takie jak w `disjoint`, które kończy łańcuch; opcji `--sort` tu nie ma, ponieważ sortowanie jest zawsze wykonywane.

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

## Argumenty i opcje

| Opcja | Wymagane | Wartości | Domyślnie |
| :----- | :----- | :------ | :----- |
| `<file>...` | tak | jedna lub więcej ścieżek | — |
| `--minimum` | nie | czas trwania: `0.08`, `1.5`, `00:00:00,080`. Zero zachowuje każdy przedział | `0.08` (około dwóch klatek) |
| `--max-lines` | nie | liczba całkowita; `0` znosi limit | `2` |
| `--dashes` | nie | flaga: myślnik przed tekstem, który każdy napis wnosi do napisu scalonego | wyłączona |
| `--recursive`, `-r` | nie | flaga | wyłączona — zobacz [Przetwarzanie drzewa katalogów](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **jedna z trzech**, z wyjątkiem `--dry-run` | zobacz [Uruchamianie](invocation.md#miejsce-docelowe) | — |
| `--dry-run` | nie | flaga: oblicza i mówi, nie zapisuje — zobacz [Podgląd przed zapisem](invocation.md#podgląd-przed-zapisem) | wyłączona |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:01,000 --> 00:00:04,000\nBeta\n\n' > segments.srt; subedit-cli repair --output repare.srt segments.srt; cat repare.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:01,000 --> 00:00:04,000\nBeta\n\n' > segments.srt; subedit-cli repair --output repare.srt segments.srt; cat repare.srt
segments.srt: 1 repeated subtitle joined; already in order; 0 subtitles split, 1 merged -> repare.srt
1
00:00:01,000 --> 00:00:04,000
Alpha
Beta
```

`Alpha`, pocięte na dwa kawałki następujące po sobie, znów staje się jednym napisem; zajmuje wtedy ten sam zakres co `Beta`,
a oba tworzą tylko jeden napis o dwóch liniach, najwcześniejszy jako pierwszy.

## Wynik

**Standardowe wyjście** — nic: wynikiem jest zapisany plik. Na wyjściu błędów, na pierwszym poziomie, **zdanie
łączące trzy liczniki**, oddzielone średnikami: to, co mówią `join-repeats`, `sort` i `disjoint`,
każde własnymi słowami — `N repeated subtitles joined; M subtitles moved; K subtitles split, L merged…`.

### W formacie JSON

Z opcją [`--format json`](invocation.md#wynik-czytelny-dla-skryptu) `counts` każdego pliku: `subtitles`, `joined`,
`moved`, potem te z [`disjoint`](disjoint.md#w-formacie-json) (`split`, `merged`, `windowed`, `absorbed`, `too_short`,
`ending_before_start`, `extras_dropped`).

## Kody wyjścia i błędy

Takie jak w całym narzędziu. Żaden błąd nie jest właściwy tylko dla `repair` poza ustawieniami, które są oceniane jak te z `disjoint`:
`--minimum`, które nie jest czasem trwania lub jest ujemne, `--max-lines`, które nie jest liczbą całkowitą, kod `1`.
