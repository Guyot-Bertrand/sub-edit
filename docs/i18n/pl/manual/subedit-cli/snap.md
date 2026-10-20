# `snap`

<!-- machine-translated -->
> Ta strona jest tłumaczeniem maszynowym i nie została jeszcze sprawdzona przez osobę władającą tym językiem.

```
subedit-cli snap --rate <rate>
                 (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 <file>...
```

Ustawia **każdy znacznik czasu** — początek i koniec — na najbliższej klatce
podanej liczby klatek na sekundę. Nic nie jest synchronizowane od nowa: każda pozycja przesuwa się co najwyżej
o pół klatki.

## `snap` to nie `framerate`, a pomylenie ich nie rzuca się w oczy

Oba polecenia mają tę samą postać — pliki, miejsce docelowe, liczba klatek na sekundę — i robią rzeczy przeciwne.

| | [`framerate`](framerate.md) | `snap` |
| :--- | :-------------------------- | :----- |
| co oblicza | `t' = t x input rate / output rate` | najbliższą klatkę, na siatce `--rate` |
| o ile przesuwa | proporcjonalnie do czasu — **kilka sekund** w filmie fabularnym | co najwyżej pół klatki, **wszędzie** |
| dla jakiego pliku | pliku, którego **synchronizacja jest zła** | pliku, którego synchronizacja jest **dobra**, a siatka zła |

**Przypadek użycia `snap`** w szczegółach: film z 25 klatkami na
sekundę, plik napisów zapisany na siatce 24, linie już
mniej więcej na swoich miejscach z dokładnością do kilku milisekund. Nie ma czego synchronizować od nowa
— trzeba tylko ustawić znaczniki czasu z powrotem na właściwych klatkach.

**Przypadek użycia `framerate`** jest odwrotny: plik zsynchronizowany dla kopii wzorcowej
25 klatek, którą chce się odtwarzać na kopii 23.976, a której synchronizacja rozjeżdża się o
kilka sekund pod koniec filmu.

Wzięcie jednego za drugie **nie powoduje żadnego błędu**: oba
przyjmują plik i zapisują wynik. Odróżnia je
wiersz komentarza — `snap` zawsze mówi, o ile przesunął pozycje.

<!-- example: subedit-cli snap --help -->
```console
$ subedit-cli snap --help
Move every position onto the nearest frame of a frame rate (see framerate)
Usage: subedit-cli snap [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to align

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --rate TEXT REQUIRED        Frame rate to align on: 25, 23.976
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Argumenty i opcje

| Opcja | Wymagane | Wartości | Domyślnie |
| :----- | :----- | :------ | :----- |
| `<file>...` | tak | jedna lub więcej ścieżek | — |
| `--recursive`, `-r` | nie | flaga | wyłączona — zobacz [Przetwarzanie drzewa katalogów](lots.md) |
| `--rate` | **tak** | liczba klatek na sekundę, zapisana jak w [`framerate`](framerate.md#zapis-liczby-klatek-na-sekundę) | — |
| `--output` / `--output-dir` / `--in-place` | **jedna z trzech**, z wyjątkiem `--dry-run` | zobacz [Uruchamianie](invocation.md#miejsce-docelowe) | — |
| `--dry-run` | nie | flaga: oblicza i mówi, nie zapisuje — zobacz [Podgląd przed zapisem](invocation.md#podgląd-przed-zapisem) | wyłączona |

`--rate` przyjmuje **każdą poprawną liczbę klatek na sekundę**, a nie tylko osiem
standardowych: wybór należy do użytkownika, tak jak w `framerate`. Zerowa lub ujemna
liczba klatek na sekundę jest odrzucana.

Format odczytanego pliku jest **zachowywany**: zmiana formatu to zadanie
[`convert`](convert.md).

## Wynik

**Standardowe wyjście** — nic: wynikiem jest zapisany plik. Z `--dry-run` też nic: nie ma pliku, a wiersz komentarza to mówi — zobacz [Podgląd przed zapisem](invocation.md#podgląd-przed-zapisem).

| Poziom | Co jest dodawane na wyjściu błędów |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles aligned on 25 fps, M positions moved, by at most 20 ms -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — format, kodowanie, znacznik i końce linii odczytanego pliku, przywrócone takie, jakie były |
| 3 | `<path>: N bytes read, M written`, potem **każdy komunikat diagnostyczny odczytu** — zobacz [Uruchamianie](invocation.md#odczyt-komunikatów-diagnostycznych) |

**Te dwie liczby są sednem wiersza.** Pół klatki przy 25 klatkach na
sekundę to dwadzieścia milisekund i nic większego nie może tam zostać zapisane. Kto
chciał `framerate`, a wpisał `snap`, czyta „by at most 20 ms” tam, gdzie spodziewał się
sekund, i wie to od razu.

## Przykład

Linia przy `00:00:01,010`, dopasowana do 25 klatek na sekundę — czyli do
wielokrotności czterdziestu milisekund:

<!-- example: printf '1\n00:00:01,010 --> 00:00:03,020\nUn.\n\n' > a.srt; subedit-cli snap --rate 25 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,010 --> 00:00:03,020\nUn.\n\n' > a.srt; subedit-cli snap --rate 25 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle aligned on 25 fps, 2 positions moved, by at most 20 ms -> b.srt
1
00:00:01,000 --> 00:00:03,040
Un.
```

## Dwie właściwości

**Dwukrotne dopasowanie nie zmienia już niczego za drugim razem.** Pozycja, która już leży na
klatce, tam pozostaje: to ta sama operacja zastosowana do tego samego wyniku.

**Kolejność napisów jest zachowywana.** Zaokrąglanie do najbliższej klatki jest
monotoniczne, więc dwa początki oddalone o co najmniej jedną klatkę takie pozostają. Bliżej
siebie mogą się zlać — to anomalia, którą
[`inspect`](inspect.md#co-zgłasza-anomalies) umie nazwać, a nie
odwrócenie kolejności.

### W formacie JSON

Z opcją [`--format json`](invocation.md#wynik-czytelny-dla-skryptu) obiekt każdego pliku zawiera `counts.subtitles`, `counts.moved` (przesunięte pozycje) i `counts.furthest_ms` (największe przesunięcie w milisekundach). Pełna tabela znajduje się w [Uruchamianie](invocation.md#co-każde-podpolecenie-umieszcza-w-counts).

## Kody wyjścia

Takie jak w całym narzędziu: `0`, jeśli każdy plik został zapisany, `2`, jeśli żaden, `3`, jeśli
tylko niektóre, `1` przy błędzie użycia.

## Błędy

Takie jak przy liczbie klatek na sekundę, słowo w słowo jak w [`framerate`](framerate.md#błędy)
— oba odczytują tę samą wartość tą samą drogą:

| Co to wywołuje | Komunikat |
| :------------------ | :------ |
| zerowa lub ujemna liczba klatek na sekundę | `"…" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed` |
| przecinek dziesiętny | `"…" is not a frame rate: use a decimal point and not a comma, in every language` |
| tekst, który nie jest liczbą | `"…" is not a frame rate: expected frames per second, like 25 or 23.976` |
| liczba klatek na sekundę zupełnie nieproporcjonalna | `"…" is not a frame rate: no video runs at that many frames per second` |

**Nic nie jest zapisywane bez jednej z trzech opcji miejsca docelowego**, a ich błędy
są wspólne dla podpoleceń, które zapisują:
zobacz [Uruchamianie](invocation.md#miejsce-docelowe).
