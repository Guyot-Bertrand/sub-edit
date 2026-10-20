# `dialogue-dashes`

<!-- machine-translated -->
> Ta strona jest tłumaczeniem maszynowym i nie została jeszcze sprawdzona przez osobę władającą tym językiem.

```
subedit-cli dialogue-dashes (--add | --remove) [--range N-M|N-]
                            (--output FILE | --output-dir DIR | --in-place)
                            [--dry-run]
                            [--recursive]
                            [--document main|translation] [-t FILE] [--align-method position|number]
                            <file>...
```

Wstawia **myślnik dialogowy** na początku linii albo go usuwa. Odpowiada to `Dialogue Dashes` w
[oknie](../subedit-gui/operations.md).

**Dwa słowa, jak w [`italics`](italics.md)**: `--add` i `--remove` wzajemnie się wykluczają i jedno z nich jest wymagane.

- **`--add`** wstawia myślnik przed każdą linią, która go nie ma: `Bonjour` daje `- Bonjour`, a
  `Bonjour\nMarie` daje `- Bonjour\n- Marie`. **Myślnik, który już jest, nie jest podwajany.**
- **Rozpoznawane są trzy myślniki** — dywiz z klawiatury i dwie długie kreski typograficzne —
  **a zapisywany jest dywiz**: `— Bonjour` zmienia się w `- Bonjour`. Dodatkowe spacje
  po myślniku znikają.
- Myślnik trafia **przed tekst, a nie przed otwierający znacznik**: `<i>Bonjour</i>` daje
  `<i>- Bonjour</i>`. Żaden znacznik się nie przesuwa.
- **`--remove`** usuwa myślniki na początku linii we wszystkich trzech postaciach.

<!-- example: subedit-cli dialogue-dashes --help -->
```console
$ subedit-cli dialogue-dashes --help
Put dialogue dashes on the lines, or take them off
Usage: subedit-cli dialogue-dashes [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --add                       Put a dialogue dash at the head of the lines
  --remove                    Take the dialogue dashes off
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
| `--add` / `--remove` | **jedna z dwóch** | flagi | — |
| `--range` | nie | `N-M` lub `N-` do końca — zob. [`adjust`](adjust.md#--range) | cały plik |
| `--document` | nie | `main` lub `translation` — zob. [Tłumaczenie](invocation.md#tłumaczenie) | `main` |
| `-t`, `--translation-file` | z `--document translation` | plik tłumaczenia, dla **jednego** pliku wejściowego | — |
| `--align-method` | nie, i tylko z `-t` | `position` lub `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **jedna z trzech**, z wyjątkiem `--dry-run` | zob. [Uruchamianie](invocation.md#miejsce-docelowe) | — |
| `--dry-run` | nie | flaga: oblicza i mówi, nic nie zapisuje — zob. [Podgląd przed zapisem](invocation.md#podgląd-przed-zapisem) | wyłączone |

**Domyślnie tekst główny; tłumaczenie z `--document translation -t FILE`.** Wtedy
zmieniane i zapisywane jest tłumaczenie, i tylko ono — zob. [Tłumaczenie](invocation.md#tłumaczenie).

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\nMarie\n\n2\n00:00:03,000 --> 00:00:04,000\n<i>Salut</i>\n\n' > a.srt; subedit-cli dialogue-dashes --add --output t.srt a.srt; cat t.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\nMarie\n\n2\n00:00:03,000 --> 00:00:04,000\n<i>Salut</i>\n\n' > a.srt; subedit-cli dialogue-dashes --add --output t.srt a.srt; cat t.srt
a.srt: 2 subtitles dashed -> t.srt
1
00:00:01,000 --> 00:00:02,000
- Bonjour
- Marie

2
00:00:03,000 --> 00:00:04,000
<i>- Salut</i>
```

## Wynik

**Standardowe wyjście** — nic: wynikiem jest zapisany plik. Z `--dry-run` lista
zmian, opisana w [Podgląd przed zapisem](invocation.md#podgląd-przed-zapisem).

| Poziom | Co jest dodawane na wyjściu błędów |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles dashed -> <destination>`, `N subtitles undashed` lub `nothing to change` |
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
| ani `--add`, ani `--remove` | `dialogue-dashes needs --add to put dashes on, or --remove` |
| oba naraz | `--add and --remove say opposite things; give one of them` |
| zakres źle zapisany lub spoza pliku | zob. [`adjust`](adjust.md#--range) |

Błędy miejsca docelowego są wspólne dla podpoleceń, które zapisują:
zob. [Uruchamianie](invocation.md#miejsce-docelowe).
