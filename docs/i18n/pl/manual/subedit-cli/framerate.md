# `framerate`

<!-- machine-translated -->
> Ta strona jest tłumaczeniem maszynowym i nie została jeszcze sprawdzona przez osobę władającą tym językiem.

> **To nie jest [`snap`](snap.md).** To polecenie **skaluje
> plik**: służy do sytuacji, gdy synchronizacja jest błędna i do końca
> filmu rozjeżdża się o kilka sekund. `snap` przesuwa znaczniki czasu z powrotem na najbliższą
> siatkę, niczego nie synchronizując od nowa, i służy do sytuacji, gdy synchronizacja jest już
> prawidłowa. Oba mają tę samą postać, jedną liczbę klatek na sekundę lub dwie, a pomylenie ich nie powoduje
> żadnego błędu.

```
subedit-cli framerate --from <fps> --to <fps>
                      (--output FILE | --output-dir DIR | --in-place)
                      [--dry-run]
                      [--recursive]
                      <file>...
```

Synchronizuje od nowa plik zsynchronizowany dla jednej liczby klatek na sekundę, dostosowując go do innej. To
operacja, której wymaga plik napisów wybrany dla kopii innej
niż ta, którą się ma — na przykład płyta DVD w systemie PAL, a materiał w systemie NTSC.

Format odczytanego pliku jest **zachowywany**: zmiana formatu to zadanie
podpolecenia [`convert`](convert.md).

<!-- example: subedit-cli framerate --help -->
```console
$ subedit-cli framerate --help
Re-time a file mastered at one frame rate for another
Usage: subedit-cli framerate [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to re-time

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --from TEXT REQUIRED        Frame rate the file is timed at: 25, 23.976
  --to TEXT REQUIRED          Frame rate to time it for: 24, 29.97
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
| `--from` | **tak** | liczba klatek na sekundę, zobacz niżej | — |
| `--to` | **tak** | liczba klatek na sekundę, zobacz niżej | — |
| `--output` / `--output-dir` / `--in-place` | **jedna z trzech**, z wyjątkiem `--dry-run` | zobacz [Uruchamianie](invocation.md#miejsce-docelowe) | — |
| `--dry-run` | nie | flaga: oblicza i informuje, nic nie zapisuje — zobacz [Podgląd przed zapisem](invocation.md#podgląd-przed-zapisem) | wyłączona |

Opcja `--from` to liczba klatek na sekundę, dla której plik **jest** zsynchronizowany, a `--to` — ta, dla której
jest potrzebny. Zamiana ich miejscami przesuwa plik w
drugą stronę — około siedmiu sekund w filmie pełnometrażowym między 25 a
23,976 — i nic o tym nie ostrzega, dopóki nie obejrzy się filmu.

## Zapis liczby klatek na sekundę

W klatkach na sekundę, z miejscami dziesiętnymi, z kropką jako separatorem:

```
25      24      23.976      29.97      60
```

**Zerowa lub ujemna liczba klatek na sekundę jest odrzucana**: to nie jest liczba klatek na sekundę.

### Trzech częstotliwości NTSC nie da się zapisać dokładnie

`23.976`, `29.97` i `59.94` to **etykiety**. Liczby klatek na sekundę, które
nazywają, to `24000/1001`, `30000/1001` i `60000/1001`, a żadna z nich nie ma
skończonego zapisu dziesiętnego. Narzędzie rozpoznaje je jako takie:

| Co jest zapisane | Co oznacza |
| :------------- | :------------------ |
| `23.976` | `24000/1001` — kino przeniesione na wideo NTSC |
| `24` | `24` — kino |
| `25` | `25` — wideo PAL i SECAM |
| `29.97` | `30000/1001` — wideo NTSC |
| `30`, `50`, `60` | one same |
| `59.94` | `60000/1001` |

Rozpoznanie dotyczy **wartości, a nie znaków**: `23.9760`
oznacza tę samą liczbę klatek na sekundę co `23.976`.

Każda inna liczba dziesiętna jest brana dosłownie — `23.9` nie jest niczyim
standardem i nie ma w niej nic do wyczytania. To także sprawia, że powyższa tabela
jest konieczna: bez niej `23.976` oznaczałoby `23976/1000`, a wiersz poleceń
mówiłby co innego niż okno o tych samych słowach.

## Czym stają się pozycje

```
t′ = round( t × (from / to) )
```

Film zsynchronizowany dla 23,976 i odtwarzany z prędkością 25 klatek na sekundę biegnie szybciej,
więc jego napisy pojawiają się wcześniej; w drugą stronę pojawiają się
później.

**Współczynnik jest dokładną liczbą wymierną, a zaokrąglenie następuje tylko raz.** Droga,
która wygląda na równoważną — zamiana pozycji na numer klatki przy
liczbie klatek wejściowej, a potem z powrotem na milisekundy przy liczbie wyjściowej — nią
nie jest: kwantyzuje każdą pozycję na siatce wejściowej i myli się nawet o
pół klatki, co zmierzono jako 21 ms. Zobacz
[ADR 0013](../../../../adr/0013-mise-a-l-echelle-exacte-des-positions.md).

W praktyce, przy zmianie z 25 na 23,976, współczynnik wynosi `1001/960`:

| Odczytana pozycja | Zapisana | Co dałoby przejście przez klatki |
| -----------: | -----: | -----------------------------------------: |
| `1 010 ms` | `1 053 ms` | `1 043 ms` |
| `3 600 017 ms` | `3 753 768 ms` | `3 753 750 ms` |

<!-- example: printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 25 --to 23.976 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 25 --to 23.976 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle retimed from 25 to 24000/1001 fps -> b.srt
1
00:00:01,053 --> 00:00:02,106
Un.
```

## Wynik

**Standardowe wyjście** — nic: wynikiem jest zapisany plik. Z opcją `--dry-run` również nic: nie ma pliku, a wiersz komentarza mówi o tym — zobacz [Podgląd przed zapisem](invocation.md#podgląd-przed-zapisem).

| Poziom | Co jest dodawane na wyjściu błędów |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles retimed from 25 to 24000/1001 fps -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — format, kodowanie, znacznik i końce linii odczytanego pliku, przywrócone takie, jakie były |
| 3 | `<path>: N bytes read, M written`, a następnie **każdy komunikat diagnostyczny odczytu** — zobacz [Uruchamianie](invocation.md#odczyt-komunikatów-diagnostycznych) |

**Komentarz podaje dokładną liczbę klatek na sekundę, a nie wpisaną etykietę.** Zapis
`23.976` w tym wierszu zgłaszałby konwersję, która się nie odbyła; to
jedyne miejsce, w którym widać, która z dwóch została użyta. Liczba klatek na sekundę
zapisywalna dokładnie jako liczba dziesiętna jest tam pokazana dziesiętnie, pozostałe — jako ułamek.

### W formacie JSON

Z opcją [`--format json`](invocation.md#wynik-czytelny-dla-skryptu) obiekt każdego pliku zawiera `counts.subtitles` (wszystkie napisy pliku). Pełna tabela znajduje się w rozdziale [Uruchamianie](invocation.md#co-każde-podpolecenie-umieszcza-w-counts).

## Kody wyjścia

Takie jak w całym narzędziu: `0`, jeśli każdy plik został zapisany, `2`, jeśli żaden, `3`, jeśli
tylko niektóre, `1` przy błędzie użycia.

Żadna odmowa nie zależy od zawartości pliku: dwie liczby klatek na sekundę zawsze wyznaczają
współczynnik. To jedyne spośród podpoleceń zapisujących, którego wszystkie
odmowy następują przed pierwszym odczytem.

<!-- example: printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 0 --to 25 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 0 --to 25 --output b.srt a.srt; echo "code=$?"
"0" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed
code=1
```

## Błędy

| Co to wywołuje | Komunikat |
| :------------------ | :------ |
| zerowa lub ujemna liczba klatek na sekundę | `"…" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed` |
| przecinek dziesiętny | `"…" is not a frame rate: use a decimal point and not a comma, in every language` |
| tekst, który nie jest liczbą | `"…" is not a frame rate: expected frames per second, like 25 or 23.976` |
| liczba klatek na sekundę zupełnie nieproporcjonalna | `"…" is not a frame rate: no video runs at that many frames per second` |

**Nic nie jest zapisywane bez jednej z trzech opcji miejsca docelowego**, a ich
błędy są wspólne dla podpoleceń, które zapisują:
zobacz [Uruchamianie](invocation.md#miejsce-docelowe).
