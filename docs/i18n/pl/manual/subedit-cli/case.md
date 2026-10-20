# `case`

<!-- machine-translated -->
> Ta strona jest tłumaczeniem maszynowym i nie została jeszcze sprawdzona przez osobę władającą tym językiem.

```
subedit-cli case --to title|sentence|upper|lower [--range N-M|N-]
                 (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 [--document main|translation] [-t FILE] [--align-method position|number]
                 <file>...
```

Zmienia teksty na **wielkie litery w wyrazach, wielką literę w zdaniu, same wielkie lub same małe litery**, nie dotykając
żadnego znacznika. Odpowiada to `Narzędzia ▸ Wielkość liter` w [oknie](../subedit-gui/operations.md), z jego regułami.

| `--to` | Efekt | `bonjour marie` zmienia się w |
| :----- | :---- | :---------------------- |
| `title` | każdy wyraz dostaje wielką literę, reszta jest zmniejszana | `Bonjour Marie` |
| `sentence` | pierwsza litera tekstu, i tylko ona; reszta jest zmniejszana | `Bonjour marie` |
| `upper` | wszystko wielkimi literami | `BONJOUR MARIE` |
| `lower` | wszystko małymi literami | `bonjour marie` |

- **To, co stoi przed pierwszą literą lub pierwszą cyfrą, pozostaje nietknięte**: myślnik
  dialogowy, otwierający cudzysłów lub nawias zostają (`- bonjour` daje `- Bonjour`).
- **Znaczniki nie są tekstem**: nie zmieniają wielkości liter — `<I>` pozostaje `<I>` — i nie
  zmieniają miejsca: znacznik, który przecina wyraz, przecina go także potem.
- **O tym, gdzie zaczyna się wyraz, decyduje ICU**: `l'été sera chaud` daje `L'été Sera Chaud`, a akcenty
  zostają (`éléonore` daje `Éléonore`).

<!-- example: subedit-cli case --help -->
```console
$ subedit-cli case --help
Put the texts in title, sentence, upper or lower case, tags intact
Usage: subedit-cli case [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --to title|sentence|upper|lower
                              The case to put the texts in
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

| Opcja | Wymagane | Wartości | Domyślnie |
| :----- | :----- | :------ | :----- |
| `<file>...` | tak | jedna lub więcej ścieżek | — |
| `--recursive`, `-r` | nie | flaga | wyłączone — zob. [Przetwarzanie drzewa katalogów](lots.md) |
| `--to` | **tak** | `title`, `sentence`, `upper`, `lower` | — |
| `--range` | nie | `N-M` lub `N-` do końca — zob. [`adjust`](adjust.md#--range) | cały plik |
| `--document` | nie | `main` lub `translation` — zob. [Tłumaczenie](invocation.md#tłumaczenie) | `main` |
| `-t`, `--translation-file` | z `--document translation` | plik tłumaczenia, dla **jednego** pliku wejściowego | — |
| `--align-method` | nie, i tylko z `-t` | `position` lub `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **jedna z trzech**, z wyjątkiem `--dry-run` | zob. [Uruchamianie](invocation.md#miejsce-docelowe) | — |
| `--dry-run` | nie | flaga: oblicza i mówi, nic nie zapisuje — zob. [Podgląd przed zapisem](invocation.md#podgląd-przed-zapisem) | wyłączone |

**Domyślnie tekst główny; tłumaczenie z `--document translation -t FILE`.** Wtedy
zmieniane i zapisywane jest tłumaczenie, i tylko ono — zob. [Tłumaczenie](invocation.md#tłumaczenie).

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\n- BONJOUR MARIE\n\n' > a.srt; subedit-cli case --to title --output titre.srt a.srt; cat titre.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\n- BONJOUR MARIE\n\n' > a.srt; subedit-cli case --to title --output titre.srt a.srt; cat titre.srt
a.srt: 2 subtitles recased -> titre.srt
1
00:00:01,000 --> 00:00:02,000
<i>Bon</i>jour Marie

2
00:00:03,000 --> 00:00:04,000
- Bonjour Marie
```

Plik, w którym **żaden tekst się nie zmienia** — wszystko ma już żądaną wielkość liter — jest zapisywany bez zmian i
mówi o tym:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBONJOUR\n\n' > a.srt; subedit-cli case --to upper --output b.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBONJOUR\n\n' > a.srt; subedit-cli case --to upper --output b.srt a.srt
a.srt: nothing to change -> b.srt
```

## Wynik

**Standardowe wyjście** — nic: wynikiem jest zapisany plik. Z `--dry-run` lista
zmian, opisana w [Podgląd przed zapisem](invocation.md#podgląd-przed-zapisem).

| Poziom | Co jest dodawane na wyjściu błędów |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles recased -> <destination>` lub `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — format, kodowanie, znacznik i końce linii odczytanego pliku, przywrócone takie, jakie były |
| 3 | `<path>: N bytes read, M written`, a następnie **każdy komunikat diagnostyczny odczytu** — zob. [Uruchamianie](invocation.md#odczyt-komunikatów-diagnostycznych) |

`N` to liczba napisów, **których tekst się zmienił**, odczytana z samej operacji.

### W formacie JSON

Z [`--format json`](invocation.md#wynik-czytelny-dla-skryptu) obiekt każdego pliku zawiera
`counts.changed` — czyli `N` z tego zdania — oraz `changes`, listę zmienionych napisów
(`{"subtitle", "document", "before", "after"}`, **pustą**, gdy nic się nie zmieniło).

## Kody wyjścia

Takie jak w całym narzędziu: `0`, jeśli przetworzono każdy plik — plik, w którym nic się nie zmienia, też jest przetworzony —,
`2`, jeśli żaden, `3`, jeśli tylko niektóre, `1` przy błędzie użycia.

## Błędy

| Co to wywołuje | Komunikat |
| :------------------ | :------ |
| brak `--to` | `--to is required` |
| wartość spoza tych czterech | `--to: … not in {title,sentence,upper,lower}` |
| zakres źle zapisany lub spoza pliku | zob. [`adjust`](adjust.md#--range) |

Błędy miejsca docelowego są wspólne dla podpoleceń, które zapisują:
zob. [Uruchamianie](invocation.md#miejsce-docelowe).
