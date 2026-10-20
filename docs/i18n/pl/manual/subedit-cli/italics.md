# `italics`

<!-- machine-translated -->
> Ta strona jest tłumaczeniem maszynowym i nie została jeszcze sprawdzona przez osobę władającą tym językiem.

```
subedit-cli italics (--on | --off) [--range N-M|N-]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    <file>...
```

Ustawia teksty **w kursywie** lub **zdejmuje z nich kursywę**, w znacznikach formatu pliku.
To odpowiednik pozycji `Italic` z [okna](../subedit-gui/operations.md).

**Dwa słowa, a nie przełącznik**: przycisk w oknie pyta rdzeń, w którą stronę
ma działać; tutaj mówi się to wprost. Opcje `--on` i `--off` wzajemnie się wykluczają i jedna jest potrzebna — pominięcie obu lub podanie obu
to błąd użycia (kod `1`).

- **`--on`** otacza każdy tekst jedną parą znaczników, **po usunięciu tych, które
  już w nim były**: tekst, który już jest w kursywie, nie jest otaczany dwa razy, a kursywa przecięta na dwie
  części jest odtwarzana w jednym kawałku. Przy kilku liniach jedna para obejmuje całość.
- **`--off`** usuwa znaczniki kursywy; **reszta znacznika jest zachowana** (`{\b1\i1}` daje
  `{\b1}`).
- Tekst jest zapisywany **tak, jak format pliku zapisuje kursywę**: `<i>…</i>` dla SubRip, WebVTT i
  SubViewer 2, `{\i1}…{\i0}` dla obu formatów Sub Station Alpha, `{Y:i}` dla MicroDVD.

<!-- example: subedit-cli italics --help -->
```console
$ subedit-cli italics --help
Put the texts in italics, or take their italics out
Usage: subedit-cli italics [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --on                        Put the texts in italics
  --off                       Take the italics out
  --range N-M|N-              Act only on subtitles N to M, or N to the end
  --document main|translation The document to change: the main one, or the translation given by -t
  -t,--translation-file FILE  Translation file to lay over the subtitle file, for a single input
  --align-method position|number
                              How the lines of the translation find their subtitles
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
| `--on` / `--off` | **jedna z dwóch** | flagi | — |
| `--range` | nie | `N-M` lub `N-` do końca — zobacz [`adjust`](adjust.md#--range) | cały plik |
| `--document` | nie | `main` lub `translation` — zobacz [Tłumaczenie](invocation.md#tłumaczenie) | `main` |
| `-t`, `--translation-file` | z `--document translation` | plik tłumaczenia, dla **jednego** pliku wejściowego | — |
| `--align-method` | nie, i tylko z `-t` | `position` lub `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **jedna z trzech**, z wyjątkiem `--dry-run` | zobacz [Uruchamianie](invocation.md#miejsce-docelowe) | — |
| `--dry-run` | nie | flaga: oblicza i informuje, nic nie zapisuje — zobacz [Podgląd przed zapisem](invocation.md#podgląd-przed-zapisem) | wyłączona |

**Domyślnie tekst główny; tłumaczenie z opcjami `--document translation -t FILE`.** Wtedy
zmieniane i zapisywane jest tłumaczenie, i tylko ono — zobacz [Tłumaczenie](invocation.md#tłumaczenie).

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour\nMarie\n\n' > a.srt; subedit-cli italics --on --output i.srt a.srt; cat i.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour\nMarie\n\n' > a.srt; subedit-cli italics --on --output i.srt a.srt; cat i.srt
a.srt: 2 subtitles put in italics -> i.srt
1
00:00:01,000 --> 00:00:02,000
<i>bonjour marie</i>

2
00:00:03,000 --> 00:00:04,000
<i>Bonjour
Marie</i>
```

## Format bez stylu odmawia

**TMPlayer i LRC nie zapisują stylu**: nie ma kursywy do ustawienia ani do zdjęcia. Okno
wyszarza pozycję zamiast ją ukrywać; wiersz poleceń **odmawia, plik po
pliku, podając powód**, a pozostałe pliki partii są przetwarzane dalej. Liczy się to, co
format może przenosić, a nie sposób, w jaki to zapisuje: SubRip i WebVTT zapisują kursywę tak
samo, a o kolorze nie mówią tego samego.

<!-- example: printf '00:00:01:Hello there\n00:00:03:Bye\n' > a.txt; subedit-cli italics --on --output-dir sortie a.txt; echo "code=$?" -->
```console
$ printf '00:00:01:Hello there\n00:00:03:Bye\n' > a.txt; subedit-cli italics --on --output-dir sortie a.txt; echo "code=$?"
a.txt: TMPlayer writes no style: there are no italics to put on
code=2
```

W formacie JSON niepowodzenie niesie identyfikator **`no-style`**.

## Wynik

**Standardowe wyjście** — nic: wynikiem jest zapisany plik. Z opcją `--dry-run` lista
zmian, opisana w rozdziale [Podgląd przed zapisem](invocation.md#podgląd-przed-zapisem).

| Poziom | Co jest dodawane na wyjściu błędów |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles put in italics -> <destination>`, albo `N subtitles taken out of italics`, albo `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — format, kodowanie, znacznik i końce linii odczytanego pliku, przywrócone takie, jakie były |
| 3 | `<path>: N bytes read, M written`, a następnie **każdy komunikat diagnostyczny odczytu** — zobacz [Uruchamianie](invocation.md#odczyt-komunikatów-diagnostycznych) |

`N` to liczba napisów, **których tekst się zmienił**, odczytana z samej operacji.

### W formacie JSON

Z opcją [`--format json`](invocation.md#wynik-czytelny-dla-skryptu) obiekt każdego pliku zawiera
`counts.changed` — liczbę `N` z komunikatu — oraz `changes`, listę zmienionych napisów
(`{"subtitle", "document", "before", "after"}`, **pustą**, gdy nic się nie zmieniło).

## Kody wyjścia

Takie jak w całym narzędziu: `0`, jeśli każdy plik został przetworzony — plik, w którym nic się nie zmienia, również —,
`2`, jeśli żaden, `3`, jeśli tylko niektóre, `1` przy błędzie użycia.

## Błędy

| Co to wywołuje | Komunikat |
| :------------------ | :------ |
| ani `--on`, ani `--off` | `italics needs --on to put the texts in italics, or --off` |
| obie | `--on and --off say opposite things; give one of them` |
| format bez stylu | `<path>: TMPlayer writes no style: there are no italics to put on` (lub `take out`) |
| zakres źle zapisany lub spoza pliku | zobacz [`adjust`](adjust.md#--range) |

Błędy miejsca docelowego są wspólne dla podpoleceń, które zapisują:
zobacz [Uruchamianie](invocation.md#miejsce-docelowe).
