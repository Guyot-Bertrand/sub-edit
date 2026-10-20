# `split-file`

<!-- machine-translated -->
> Ta strona jest tłumaczeniem maszynowym i nie została jeszcze sprawdzona przez osobę władającą tym językiem.

```
subedit-cli split-file --at N --head FILE --tail FILE
                       [--dry-run] [--sort] [--disjoint]
                       <file>
```

Tnie plik **na dwie części**: początek i koniec. To `Split Project…` okna — [ten sam silnik](../subedit-gui/operations.md#split-project),
te same liczniki, ta sama odmowa — i **dokładne odwrócenie [`append`](append.md)**: dołączenie końca do
początku daje z powrotem plik wyjściowy.

- **`--at N` to numer pierwszego napisu końca**, numerowanego tak, jak numeruje go tabela okna:
  od 1. `--at 3` umieszcza napisy 1 i 2 w początku, a trzeci i resztę w
  końcu. Wartość mieści się **od drugiego do ostatniego**: cięcie wymaga napisu po każdej stronie.
- **Koniec jest przywracany do punktu zerowego**: jest przesuwany o minus koniec ostatniego napisu pozostawionego w początku,
  a jego pozycje zachowują kolejność i odstępy. To sprawia, że `split-file` jest odwrotnością `append`.
- **Obie połowy zachowują format, kodowanie, znacznik kolejności bajtów i końce linii odczytanego pliku** i
  są numerowane od nowa od 1 przy zapisie, jak każdy plik zapisywany z powrotem.
- **Dwa wyjścia, więc własne miejsce docelowe dla każdego**: `--head` i `--tail`, **oba wymagane**. `--output`,
  `--output-dir`, `--in-place` i `--recursive` nie istnieją dla `split-file` i są odrzucane jak każda nieznana
  opcja, kod `1`: nie ma ani katalogu, w którym można by umieścić dwa nazwane pliki, ani pliku wejściowego do nadpisania.

<!-- example: subedit-cli split-file --help -->
```console
$ subedit-cli split-file --help
Cut a subtitle file in two, a head and a tail
Usage: subedit-cli split-file [OPTIONS] file

Positionals:
  file TEXT REQUIRED          Subtitle file to cut

Options:
  -h,--help                   Print this help message and exit
  --at N                      Number of the first subtitle of the tail, as the table numbers them
  --head FILE                 File to write the subtitles before the cut to
  --tail FILE                 File to write the subtitles from the cut on to
  --dry-run                   Work out and say what would be written, and write nothing
  --sort                      Put the subtitles in order of their start before cutting; --at counts in that order
  --disjoint                  Cut overlapping subtitles so that no range overlaps another, before cutting
```

## Argumenty i opcje

| Opcja | Wymagane | Wartości | Domyślnie |
| :----- | :----- | :------ | :----- |
| `<file>` | tak | **jedna** ścieżka | — |
| `--at` | tak | liczba całkowita, od 2 do liczby napisów pliku | — |
| `--head`, `--tail` | **tak, obie**, z wyjątkiem `--dry-run` | ścieżki plików; ich katalog jest tworzony, jeśli go brakuje | — |
| `--dry-run` | nie | flaga: oblicza i mówi, nie zapisuje — zobacz [Podgląd przed zapisem](invocation.md#podgląd-przed-zapisem) | wyłączona |
| `--sort` | nie | flaga: ustawia napisy w kolejności ich początków **przed cięciem**; `--at` liczy w tej kolejności | wyłączona |
| `--disjoint` | nie | flaga: tnie nakładające się napisy **przed cięciem**; `--at` liczy w tym, co zostaje, zobacz [Usuwanie nakładania się zakresów](#usuwanie-nakładania-się-zakresów) | wyłączona |

**Z `--dry-run` żadne miejsce docelowe nie jest wymagane**: można nie podać żadnego, ale nie tylko jednego z dwóch —
`--head` i `--tail` idą razem. Te, które podano, są oceniane tak jak bez `--dry-run`.

**Miejscem docelowym nie może być ani plik wejściowy, ani ten sam plik dwa razy**: zapis nad tym, co się
czyta, albo zapis końca nad początkiem jest odrzucany, zanim cokolwiek zostanie odczytane, kod `1`.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n3\n00:00:06,000 --> 00:00:07,000\ntrois\n\n4\n00:00:08,000 --> 00:00:09,000\nquatre\n\n' > film.srt; subedit-cli split-file --at 3 --head tete.srt --tail queue.srt film.srt; cat queue.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n3\n00:00:06,000 --> 00:00:07,000\ntrois\n\n4\n00:00:08,000 --> 00:00:09,000\nquatre\n\n' > film.srt; subedit-cli split-file --at 3 --head tete.srt --tail queue.srt film.srt; cat queue.srt
film.srt: split at subtitle 3: 2 subtitles in the head, 2 subtitles in the tail -> tete.srt, queue.srt
1
00:00:02,000 --> 00:00:03,000
trois

2
00:00:04,000 --> 00:00:05,000
quatre
```

Drugi napis, pozostawiony w początku, kończy się w 4 s: koniec cofa się o 4 s, a trzeci napis,
który zaczynał się w 6 s, zaczyna się w 2 s.

## Cięcie w kolejności czasowej

`--at` numeruje napisy **tak, jak podaje je plik**. W pliku o zaburzonej kolejności początek nie jest więc połową najwcześniejszą
w czasie. `--sort` ustawia napisy w kolejności ich początków **przed oceną cięcia** — stabilnie, jak robi to [`sort`](sort.md) —, a
`--at` liczy wtedy w tej kolejności.

Nigdy domyślnie. Z tą opcją plik mówi na pierwszym poziomie komentarza, ile napisów się przesunęło (`<path>: N subtitles moved`
lub `<path>: already in order`), a rekord `--format json` zawiera licznik `moved`, **nieobecny bez tej opcji**.

## Usuwanie nakładania się zakresów

Nigdy domyślnie. Opcja robi to, co [`disjoint`](disjoint.md), **z jego ustawieniami domyślnymi**, przed oceną cięcia, po sortowaniu: `--at` liczy w tym, co z tego wyjdzie. Wymaga pliku we właściwej kolejności: w przeciwnym razie jest odrzucana, a `--sort` — który działa pierwszy — ustawia go wcześniej we właściwej kolejności. Plik mówi o tym na pierwszym poziomie komentarza (`<path>: N subtitles split, M merged`), a rekord `--format json` zawiera liczniki `split` i `merged`, **nieobecne bez tej opcji**.

## Gdy cięcie jest odrzucane

**Dwie połowy nakładające się w miejscu cięcia nie są cięte.** Gdyby napis końca zaczynał się przed
końcem ostatniego napisu pozostawionego z tyłu, przesunięcie wyniosłoby go przed początek wideo — pozycja ujemna,
której nie zapisuje żaden format. `split-file` odmawia, **wskazując napis**, słowami okna:

```text
o.srt: Cannot split at subtitle 3: subtitle 3 would fall before the start of the video. Cut somewhere else.
```

Kod to `2` i **nic nie jest zapisywane**. Tnij gdzie indziej. Trafienie dokładnie w początek jest dozwolone: zero to
pozycja.

**`--at` poza plikiem jest odrzucane tak samo**, po odczytaniu pliku: `--at 1` — pierwszy napis nie może
zaczynać końca, nic by nie zostało — oraz `--at` za ostatnim napisem, co zostawiłoby pusty
koniec. Pliku o jednym napisie nie można ciąć.

## Wynik

**Standardowe wyjście** — nic: wynikiem są dwa zapisane pliki.

| Poziom | Co jest dodawane na wyjściu błędów |
| :----- | :------------------------------------- |
| 1 | `<path>: split at subtitle N: H subtitles in the head, T subtitles in the tail -> <head>, <tail>`; z `--dry-run` zamiast `-> …` pojawia się `(dry run, nothing written)` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — format, kodowanie, znacznik i końce linii odczytanego pliku, przywrócone takie, jakie były |
| 3 | `<path>: N bytes read, H and T written`, potem **każdy komunikat diagnostyczny odczytu** — zobacz [Uruchamianie](invocation.md#odczyt-komunikatów-diagnostycznych) |

### W formacie JSON

Z opcją [`--format json`](invocation.md#wynik-czytelny-dla-skryptu) **jeden obiekt**, w którym `file` to cięty plik.
**`destination` to początek** — plik, którym staje się wejście — a **`tail` to koniec**, ścieżka lub `null` z
`--dry-run`, tak jak `destination`. Liczniki `counts`: `subtitles` (napisy odczytanego pliku), `head` i `tail` (napisy każdej
połowy).

## Kody wyjścia

| Kod | Kiedy |
| :--- | :---- |
| `0` | oba pliki zostały zapisane — lub obliczone, z `--dry-run` |
| `1` | błąd użycia: brak `--head` i `--tail`, opcja, której `split-file` nie ma, miejsce docelowe będące plikiem wejściowym lub drugim miejscem docelowym, `--at`, które nie jest dodatnią liczbą całkowitą |
| `2` | nie udało się odczytać pliku, **cięcie jest odrzucone** (poza plikiem lub nakładanie się) albo nie udało się zapisać miejsca docelowego. Kodu `3` nie ma: przetwarzany jest jeden plik |

**Początek jest zapisywany pierwszy**: dysk, który odmawia zapisu końca, zostawia za sobą początek, a kod to `2`.

## Błędy

| Co to wywołuje | Komunikat |
| :------------------- | :------ |
| ani `--head`, ani `--tail` | `no destination given: split-file writes two files, use --head and --tail` |
| tylko jedno z dwóch, z `--dry-run` | `--head and --tail go together: give both, or neither with --dry-run` |
| oba miejsca docelowe to ten sam plik | `<path>: named as both the head and the tail` |
| miejsce docelowe jest plikiem wejściowym | `<path>: would be written over the input <input>` |
| `--at` poza plikiem | `<file>: --at 9: the file holds 3 subtitles, and the tail can start from 2 to 3` |
| plik o jednym napisie | `<file>: a file of 1 subtitle cannot be split: a cut needs a subtitle on each side` |
| nakładanie się | `<file>: Cannot split at subtitle N: subtitle M would fall before the start of the video. Cut somewhere else.` |
| brak pliku, nieczytelny lub o nieznanym formacie | `<path>: does not exist`… — tak samo jak wszędzie |
