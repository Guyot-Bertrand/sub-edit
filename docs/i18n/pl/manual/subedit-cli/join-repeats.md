# `join-repeats`

<!-- machine-translated -->
> Ta strona jest tłumaczeniem maszynowym i nie została jeszcze sprawdzona przez osobę władającą tym językiem.

```
subedit-cli join-repeats (--output FILE | --output-dir DIR | --in-place)
                         [--dry-run]
                         [--recursive]
                         <file>...
```

Robi **jeden napis** z napisów, które pokazują to samo jeden po drugim: tekst pocięty na kolejne fragmenty,
jak zostawiają go niektóre narzędzia do segmentacji. To odpowiednik pozycji `Join Repeated Subtitles` z okna —
[ten sam silnik](../subedit-gui/lines.md#łączenie-powtórzonych-napisów).

**Powtórzenie** to napis, który ma **ten sam tekst, to samo tłumaczenie i te same ustawienia formatu** (styl,
pozycję, ustawienia napisu) co inny napis **i który zaczyna się najpóźniej wtedy, gdy tamten się kończy**: stykają się
lub nakładają. Ciąg powtórzeń staje się **napisem, który zaczyna się najwcześniej**, a jego koniec
zostaje przesunięty na najpóźniejszy koniec ciągu; pozostałe znikają.

- **Kolejność w pliku nie ma znaczenia i nie jest zmieniana**: zachowany zostaje napis, który zaczyna się najwcześniej,
  niezależnie od tego, gdzie są pozostałe. Plik we właściwej kolejności pozostaje w kolejności.
- **Nic nie jest łączone przez pomyłkę**: inny tekst, inne tłumaczenie, inne ustawienia, przerwa między dwoma
  fragmentami, napis bez tekstu lub taki, którego koniec wypada przed początkiem — we wszystkich tych przypadkach pozostają osobno.
- **Do wykonania przed resztą**: sortowanie i [obcinanie nakładających się części](disjoint.md) lepiej odczytują czysty ciąg.
  [`repair`](repair.md) łączy wszystkie trzy, a [`convert`](convert.md#usuwanie-nakładania-się-zakresów) przyjmuje je jako opcję (`--join-repeats`, `--repair`).

<!-- example: subedit-cli join-repeats --help -->
```console
$ subedit-cli join-repeats --help
Join the subtitles that repeat each other, one after the other, into one
Usage: subedit-cli join-repeats [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to join the repeats of

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Argumenty i opcje

| Opcja | Wymagana | Wartości | Domyślnie |
| :----- | :----- | :------ | :----- |
| `<file>...` | tak | jedna lub więcej ścieżek | — |
| `--recursive`, `-r` | nie | flaga | wyłączona — zobacz [Przetwarzanie drzewa katalogów](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **jedna z trzech**, z wyjątkiem `--dry-run` | zobacz [Uruchamianie](invocation.md#miejsce-docelowe) | — |
| `--dry-run` | nie | flaga: oblicza i informuje, nic nie zapisuje — zobacz [Podgląd przed zapisem](invocation.md#podgląd-przed-zapisem) | wyłączona |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:04,000 --> 00:00:06,000\nAlpha\n\n4\n00:00:08,000 --> 00:00:09,000\nBeta\n\n' > morceaux.srt; subedit-cli join-repeats --output joint.srt morceaux.srt; cat joint.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:04,000 --> 00:00:06,000\nAlpha\n\n4\n00:00:08,000 --> 00:00:09,000\nBeta\n\n' > morceaux.srt; subedit-cli join-repeats --output joint.srt morceaux.srt; cat joint.srt
morceaux.srt: 2 repeated subtitles joined -> joint.srt
1
00:00:01,000 --> 00:00:06,000
Alpha

2
00:00:08,000 --> 00:00:09,000
Beta
```

Trzy fragmenty `Alpha`, następujące po sobie od 1 s do 6 s, tworzą teraz tylko jeden. `Beta`, oddzielone przerwą,
pozostało. Drugi przebieg niczego nie zmienia i mówi o tym: `no repeated subtitles`.

## Wynik

**Standardowe wyjście** — nic: wynikiem jest zapisany plik.

| Poziom | Co jest dodawane na wyjściu błędów |
| :----- | :------------------------------------- |
| 1 | `<path>: N repeated subtitles joined -> <destination>`, albo `<path>: no repeated subtitles -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, a następnie **każdy komunikat diagnostyczny odczytu** |

### W formacie JSON

Z opcją [`--format json`](invocation.md#wynik-czytelny-dla-skryptu) w `counts` każdego pliku: `subtitles` (napisy zapisanego
pliku), `joined` (napisy usunięte, ponieważ pokrywa je inny) oraz `chains` (połączone ciągi).

## Kody wyjścia

Takie jak w całym narzędziu: `0`, jeśli każdy plik został przetworzony — plik bez powtórzeń również —, `2`, jeśli żaden, `3`, jeśli
tylko niektóre, `1` przy błędzie użycia.

## Błędy

Żadnych specyficznych dla `join-repeats`: nieczytelny plik kończy się błędem jak w każdym podpoleceniu, a błędy
miejsca docelowego są wspólne dla podpoleceń, które zapisują: zobacz [Uruchamianie](invocation.md#miejsce-docelowe).
