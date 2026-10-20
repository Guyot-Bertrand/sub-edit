# `disjoint`

<!-- machine-translated -->
> Ta strona jest tłumaczeniem maszynowym i nie została jeszcze sprawdzona przez osobę władającą tym językiem.

```
subedit-cli disjoint (--output FILE | --output-dir DIR | --in-place)
                     [--minimum TIME] [--max-lines N] [--dashes] [--sort]
                     [--dry-run] [--recursive]
                     <file>...
```

Obcina nakładające się napisy tak, aby **żaden zakres `[start, end]` nie nakładał się już na inny**. To odpowiednik pozycji `Make Ranges Disjoint…` z okna — [ten sam silnik](../subedit-gui/lines.md#usuwanie-nakładania-się-zakresów).

**Nic nie jest skracane ani wydłużane**: w każdej chwili plik pokazuje te same teksty co wcześniej. Zmienia się tylko sposób, w jaki podzielona jest oś czasu:

- jest ona cięta w **każdym początku i każdym końcu** napisów grupy nakładających się napisów;
- przedział pokryty przez **jeden** napis pozostaje tym napisem;
- przedział pokryty przez **kilka** napisów staje się **jednym** napisem, którego linie są liniami tych napisów, **w
  kolejności ich początków** — najwcześniejszy w linii 1, następny w linii 2;
- napis, który na nic nie nachodzi, pozostaje nietknięty, a kolejność pozostałych jest zachowana.

<!-- example: subedit-cli disjoint --help -->
```console
$ subedit-cli disjoint --help
Cut overlapping subtitles so that no range overlaps another, merging what overlaps
Usage: subedit-cli disjoint [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to make disjoint

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --minimum TIME              Shortest interval kept; a shorter one is taken over by a neighbour (default 0.08)
  --max-lines N               Most lines of a merged subtitle; more are shown in windows, 0 for no limit (default 2)
  --dashes                    Put a dash before the text of each merged subtitle
  --sort                      Put the subtitles in order of their start first; without it, a file out of order is refused
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Argumenty i opcje

| Opcja | Wymagana | Wartości | Domyślnie |
| :----- | :----- | :------ | :----- |
| `<file>...` | tak | jedna lub więcej ścieżek | — |
| `--minimum` | nie | czas trwania: `0.08`, `1.5`, `00:00:00,080`. Zero zachowuje każdy przedział | `0.08` (około dwóch klatek) |
| `--max-lines` | nie | liczba całkowita: największa liczba linii połączonego napisu; `0` znosi limit | `2` |
| `--dashes` | nie | flaga: myślnik przed tekstem, który każdy napis wnosi do połączonego napisu | wyłączona |
| `--sort` | nie | flaga: układa napisy w kolejności początków **najpierw** — zobacz [Potrzebny jest plik we właściwej kolejności](#potrzebny-jest-plik-we-właściwej-kolejności) | wyłączona |
| `--recursive`, `-r` | nie | flaga | wyłączona — zobacz [Przetwarzanie drzewa katalogów](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **jedna z trzech**, z wyjątkiem `--dry-run` | zobacz [Uruchamianie](invocation.md#miejsce-docelowe) | — |
| `--dry-run` | nie | flaga: oblicza i informuje, nic nie zapisuje — zobacz [Podgląd przed zapisem](invocation.md#podgląd-przed-zapisem) | wyłączona |

<!-- example: printf '1\n00:00:00,000 --> 00:00:05,000\nA\n\n2\n00:00:03,000 --> 00:00:08,000\nB\n\n' > chevauche.srt; subedit-cli disjoint --output disjoint.srt chevauche.srt; cat disjoint.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:05,000\nA\n\n2\n00:00:03,000 --> 00:00:08,000\nB\n\n' > chevauche.srt; subedit-cli disjoint --output disjoint.srt chevauche.srt; cat disjoint.srt
chevauche.srt: 2 subtitles split, 1 merged -> disjoint.srt
1
00:00:00,000 --> 00:00:03,000
A

2
00:00:03,000 --> 00:00:05,000
A
B

3
00:00:05,000 --> 00:00:08,000
B
```

`A [0, 5]` i `B [3, 8]` dają trzy rozłączne napisy: samo `A`, `A` i `B` razem, samo `B`. Żadna
chwila nie jest ani utracona, ani dodana.

## Co rozstrzyga operacja

**Linia nigdy nie jest powielana.** Dwa napisy o tym samym tekście dają jedną linię tam, gdzie
się nakładają, a dwa sąsiednie fragmenty pokazujące to samo tworzą tylko jeden napis. Tekst
napisu podzielonego na kilka fragmentów jest poza tym **powtórzony w każdym z nich**: to jest to, co jest na ekranie
w danej chwili.

**Bardzo krótki przedział przejmuje sąsiad.** Poniżej wartości `--minimum` przedział znika: sąsiad
**wydłuża swój zakres, aby go objąć**, tak że żadna chwila nie zostaje pusta i nic nie przesuwa się o więcej niż ten
próg. Wybranym sąsiadem jest ten, którego linie **już zawierają** linie przedziału, a w przeciwnym razie
dłuższy. Tekst przedziału ginie tylko wtedy, gdy żaden sąsiad go nie niesie, i to na czas krótszy niż próg.
**Cały napis krótszy niż próg nigdy nie jest przejmowany**: zostaje i jest zliczany.

**Więcej linii niż limit: okna.** Połączony napis, który miałby więcej linii niż `--max-lines`,
jest pokazywany **w kolejnych oknach** o tym rozmiarze, w równych częściach czasu trwania. Trzy linie przez trzy
sekundy:

<!-- example: printf '1\n00:00:00,000 --> 00:00:03,000\nA\n\n2\n00:00:00,000 --> 00:00:03,000\nB\n\n3\n00:00:00,000 --> 00:00:03,000\nC\n\n' > trois.srt; subedit-cli disjoint --output fenetres.srt trois.srt; cat fenetres.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:03,000\nA\n\n2\n00:00:00,000 --> 00:00:03,000\nB\n\n3\n00:00:00,000 --> 00:00:03,000\nC\n\n' > trois.srt; subedit-cli disjoint --output fenetres.srt trois.srt; cat fenetres.srt
trois.srt: 0 subtitles split, 1 merged; 1 shown in windows for holding too many lines -> fenetres.srt
1
00:00:00,000 --> 00:00:01,500
A
B

2
00:00:01,500 --> 00:00:03,000
B
C
```

Cztery linie dają trzy okna i tak dalej: okno przesuwa się o jedną linię naraz. Nic nie jest
obcinane ani cięte ponownie.

**Tłumaczenie podlega tym samym cięciom**, ponieważ napis niesie oba teksty. **To, co format niesie
dodatkowo** — styl, pozycję — pochodzi z pierwszego napisu, który pokrywa przedział; utrata jest zliczana, gdy
połączone napisy się różnią.

**Napisu, którego koniec wypada przed początkiem**, nie próbuje się zgadywać: zostaje tam, gdzie jest, i jest zliczany.

## Potrzebny jest plik we właściwej kolejności

Grupy to ciągi sąsiadów: poza kolejnością sąsiedzi nie są tymi, które się nakładają. Plik,
którego początki nie są w kolejności, jest więc **odrzucany**, o czym mówi komunikat:

<!-- example: printf '1\n00:00:03,000 --> 00:00:08,000\nB\n\n2\n00:00:00,000 --> 00:00:05,000\nA\n\n' > desordre.srt; subedit-cli disjoint --output x.srt desordre.srt; echo "code $?" -->
```console
$ printf '1\n00:00:03,000 --> 00:00:08,000\nB\n\n2\n00:00:00,000 --> 00:00:05,000\nA\n\n' > desordre.srt; subedit-cli disjoint --output x.srt desordre.srt; echo "code $?"
desordre.srt: The subtitles are not in the order of their start: sort them first. Use --sort to do that here.
code 2
```

Opcja `--sort` najpierw sortuje — tak samo jak [`sort`](sort.md) — i mówi o tym. Rdzeń nigdy nie sortuje sam z siebie.

Drugi przebieg niczego nie zmienia: gdy zakresy są rozłączne, nie ma już nic do cięcia,
a komunikat mówi `already disjoint`.

## Wynik

**Standardowe wyjście** — nic: wynikiem jest zapisany plik.

| Poziom | Co jest dodawane na wyjściu błędów |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles split, M merged -> <destination>`, a po `;` to, czego nie udało się zrobić w pełni: `K shown in windows for holding too many lines`, `K very short taken over by a neighbour`, `K subtitles shorter than the limit left whole`, `K merged with styles or positions that differ, the first kept`, `K subtitles ending before the start left alone` — albo `<path>: already disjoint -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, a następnie **każdy komunikat diagnostyczny odczytu** |

`N subtitles split` zlicza napisy pierwotne podzielone na kilka fragmentów, `M merged` — fragmenty, które
pokazują razem kilka napisów.

### W formacie JSON

Z opcją [`--format json`](invocation.md#wynik-czytelny-dla-skryptu) w `counts` każdego pliku:

| Licznik | Co mówi |
| :----- | :----------- |
| `subtitles` | napisy zapisanego pliku |
| `split`, `merged` | jak w komunikacie |
| `windowed` | fragmenty pokazywane w oknach |
| `absorbed` | przedziały przejęte przez sąsiada |
| `too_short` | napisy krótsze niż próg, pozostawione w całości |
| `ending_before_start` | napisy, których koniec wypada przed początkiem, pozostawione bez zmian |
| `extras_dropped` | połączone fragmenty, których styl lub pozycja się różnią |
| `moved` | tylko z opcją `--sort`: miejsca, które się zmieniły |

## Kody wyjścia

Takie jak w całym narzędziu: `0`, jeśli każdy plik został przetworzony — plik bez nakładań również —, `2`, jeśli
żaden, `3`, jeśli tylko niektóre, `1` przy błędzie użycia.

## Błędy

| Co je wywołuje | Komunikat |
| :------------------- | :------ |
| plik poza kolejnością bez `--sort` | `<path>: The subtitles are not in the order of their start: sort them first. Use --sort to do that here.` |
| `--minimum` nie jest czasem trwania lub jest ujemne | `--minimum: …` |
| `--max-lines` nie jest liczbą całkowitą | `--max-lines: "…" is not a number of lines: expected a whole number, 0 for no limit` |

Nieczytelny plik kończy się błędem jak w każdym podpoleceniu; błędy miejsca docelowego są wspólne dla
podpoleceń, które zapisują: zobacz [Uruchamianie](invocation.md#miejsce-docelowe).

## W innych poleceniach

Podpolecenia [`convert`](convert.md#usuwanie-nakładania-się-zakresów), [`append`](append.md#usuwanie-nakładania-się-zakresów) i
[`split-file`](split-file.md#usuwanie-nakładania-się-zakresów) przyjmują opcję `--disjoint`, która przed zapisem robi to samo
z ustawieniami domyślnymi.

W źle podzielonym pliku — tekst pocięty na następujące po sobie fragmenty, podany bez kolejności — samo sortowanie i cięcie
dają linie, które przeskakują z jednego fragmentu na drugi: najpierw [`join-repeats`](join-repeats.md) albo
[`repair`](repair.md), który łączy wszystkie trzy.
