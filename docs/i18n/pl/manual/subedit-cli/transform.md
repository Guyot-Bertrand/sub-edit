# `transform`

<!-- machine-translated -->
> Ta strona jest tłumaczeniem maszynowym i nie została jeszcze sprawdzona przez osobę władającą tym językiem.

```
subedit-cli transform --first <index>=<time> --last <index>=<time>
                      (--output FILE | --output-dir DIR | --in-place)
                      [--dry-run]
                      [--recursive]
                      <file>...
```

Koryguje **każdą** pozycję pliku na podstawie dwóch punktów, których poprawna wartość
jest znana. To operacja, której wymaga rozjeżdżająca się synchronizacja napisów
— dobra na początku, zła na końcu — gdzie [`shift`](shift.md), który przesuwa wszystko o
tę samą wartość, nic nie może zrobić.

Format odczytanego pliku jest **zachowywany**: zmiana formatu to zadanie
[`convert`](convert.md).

<!-- example: subedit-cli transform --help -->
```console
$ subedit-cli transform --help
Correct every position from two points known to be right
Usage: subedit-cli transform [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to transform

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --first TEXT REQUIRED       Earlier reference, as <index>=<time>: 1=00:00:01.000
  --last TEXT REQUIRED        Later reference, as <index>=<time>: 3=00:00:10.000
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
| `--first` | **tak** | odniesienie, `<index>=<time>` | — |
| `--last` | **tak** | odniesienie, `<index>=<time>` | — |
| `--output` / `--output-dir` / `--in-place` | **jedna z trzech**, z wyjątkiem `--dry-run` | zobacz [Uruchamianie](invocation.md#miejsce-docelowe) | — |
| `--dry-run` | nie | flaga: oblicza i mówi, nie zapisuje — zobacz [Podgląd przed zapisem](invocation.md#podgląd-przed-zapisem) | wyłączona |

Nie ma formy skróconej: te dwie opcje zapisuje się tylko w
pełnej postaci.

## Zapis odniesienia

Odniesienie wskazuje napis i mówi, gdzie ma teraz wypaść jego **początek**:

```
<index>=<time>
```

**Indeks liczy się od 1**, tak jak jest wyświetlany i tak jak numeruje go plik
SubRip. `0` jest odrzucane, tak samo jak indeks za ostatnim
napisem.

**Czas zapisuje się tak jak w [`shift`](shift.md)** — sekundy lub
kod czasowy, separatorem dziesiętnym jest kropka, rozdzielczością milisekunda:

| Postać | Przykłady |
| :---- | :------- |
| sekundy | `10`, `2.999` |
| kod czasowy | `00:00:10.000`, `01:39:37.040` |

`--first 3=10` i `--first 3=00:00:10.000` mówią to samo.

`--first` i `--last` wskazują dwa punkty, a nie przedział. Ich nazwy mówią
o oczekiwanym użyciu — jedno odniesienie na początku pliku, drugie na końcu — a nie o
ograniczeniu: zamiana ich miejscami daje ten sam wynik, a **wszystko poza
dwoma odniesieniami podąża tak samo jak to, co jest między nimi**.

## Czym stają się pozostałe pozycje

Dwa odniesienia ustalają współczynnik skali, za którym podąża każda pozycja:

```
t′ = y₁ + (t − x₁) × r      with r = (y₂ − y₁) / (x₂ − x₁)
```

`x₁`, `x₂` to bieżące początki dwóch wskazanych napisów, `y₁`, `y₂` — żądane
czasy. Współczynnik jest przechowywany jako dokładna liczba wymierna, a **zaokrąglenie
następuje tylko raz**, do milisekundy; dzięki temu oba
odniesienia wypadają **dokładnie** na żądanych czasach, a nie z dokładnością do tysięcznej
(zobacz [ADR 0013](../../../../adr/0013-mise-a-l-echelle-exacte-des-positions.md)).

Każdy napis pozostaje na ekranie **proporcjonalnie** do tego, jak długo tam był:
koniec podlega tej samej skali co początek. Przekształcenie nie jest
przesunięciem i nie ma jego właściwości.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,001 --> 00:00:07,000\nDeux.\n\n3\n00:00:09,000 --> 00:00:11,000\nTrois.\n\n' > a.srt; subedit-cli transform --first 1=00:00:01.000 --last 3=00:00:10.000 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,001 --> 00:00:07,000\nDeux.\n\n3\n00:00:09,000 --> 00:00:11,000\nTrois.\n\n' > a.srt; subedit-cli transform --first 1=00:00:01.000 --last 3=00:00:10.000 --output b.srt a.srt; cat b.srt
a.srt: 3 subtitles transformed onto 1=00:00:01.000 and 3=00:00:10.000 -> b.srt
1
00:00:01,000 --> 00:00:03,250
Un.

2
00:00:05,501 --> 00:00:07,750
Deux.

3
00:00:10,000 --> 00:00:12,250
Trois.
```

Pierwsze odniesienie się nie przesuwa, trzecie przechodzi z 9 s na 10 s: współczynnik to
`9/8`, a drugi napis — którego nikt nie wskazał — przechodzi z
`5,001` na `5,501`, czyli `1000 + 4001 × 9/8` zaokrąglone raz.

## Dwa odniesienia, które nimi nie są

| Sytuacja | Kiedy to widać | Kod |
| :-------- | :------------- | :--- |
| `--first` i `--last` wskazują ten sam indeks | przed jakimkolwiek odczytem | `1` |
| indeks jest za ostatnim napisem | przy otwarciu pliku | `2` lub `3` |
| dwa wskazane napisy zaczynają się w tej samej chwili | przy otwarciu pliku | `2` lub `3` |

Wszystkie trzy są odrzucane z tego samego powodu: `x₂ − x₁` wynosi zero, a dwa
pokrywające się punkty nie wyznaczają prostej. Zwracanie dzielenia przez zero przebranego za
wynik byłoby gorsze niż odmowa.

Pierwszy przypadek to **błąd użycia** — winne są obie opcje, niezależnie od
plików, więc należy się jeden komunikat, a nie jeden na plik. Pozostałe
dwa zależą od otwartego pliku, a więc liczą się jako
porażka przetwarzania.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n' > a.srt; subedit-cli transform --first 2=1.000 --last 2=4.000 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n' > a.srt; subedit-cli transform --first 2=1.000 --last 2=4.000 --output b.srt a.srt; echo "code=$?"
--first and --last both name subtitle 2, and two references on one subtitle define no transform
code=1
```

Indeks poza zakresem jest odrzucany **ze wskazaniem granicy**, co mówi jednocześnie,
co można było wpisać:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,000 --> 00:00:07,000\nDeux.\n\n' > a.srt; subedit-cli transform --first 1=1.000 --last 9=10.000 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,000 --> 00:00:07,000\nDeux.\n\n' > a.srt; subedit-cli transform --first 1=1.000 --last 9=10.000 --output b.srt a.srt; echo "code=$?"
a.srt: subtitle 9 is past the end: the file holds 2 subtitles
code=2
```

## Pozycja przed początkiem osi czasu jest odrzucana

Tak jak w [`shift`](shift.md) i z tego samego powodu: pozycja ujemna
zapisałaby się jako `-00:00:01,000`, postać, której nie definiuje żaden z dziewięciu formatów i której nie
przyjmuje żaden czytnik. Przypadek pojawia się tutaj, gdy napis leży **przed
pierwszym odniesieniem**, a skala spycha go poniżej zera. Nic nie jest
zapisywane, kod to `2`, a komunikat wskazuje wadliwy napis.

## Wynik

**Standardowe wyjście** — nic: wynikiem jest zapisany plik. Z `--dry-run` też nic: nie ma pliku, a wiersz komentarza to mówi — zobacz [Podgląd przed zapisem](invocation.md#podgląd-przed-zapisem).

| Poziom | Co jest dodawane na wyjściu błędów |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles transformed onto 1=00:00:01.000 and 3=00:00:10.000 -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — format, kodowanie, znacznik i końce linii odczytanego pliku, przywrócone takie, jakie były |
| 3 | `<path>: N bytes read, M written`, potem **każdy komunikat diagnostyczny odczytu** — zobacz [Uruchamianie](invocation.md#odczyt-komunikatów-diagnostycznych) |

Odniesienia są zapisywane z powrotem z kropką dziesiętną, niezależnie od tego, jak je wpisano.

### W formacie JSON

Z opcją [`--format json`](invocation.md#wynik-czytelny-dla-skryptu) obiekt każdego pliku zawiera `counts.subtitles` (każdy napis pliku). Pełna tabela znajduje się w [Uruchamianie](invocation.md#co-każde-podpolecenie-umieszcza-w-counts).

## Kody wyjścia

Takie jak w całym narzędziu: `0`, jeśli każdy plik został zapisany, `2`, jeśli żaden, `3`, jeśli
tylko niektóre, `1` przy błędzie użycia.

## Błędy

| Co to wywołuje | Komunikat |
| :------------------ | :------ |
| odniesienie bez `=` | `"…" is not a reference: write it <index>=<time>, as in 3=00:00:10.000` |
| odniesieniu brakuje jednej połowy | `"…" is missing one of its two halves: write it <index>=<time>, as in 3=00:00:10.000` |
| indeks `0` | `"0" is not a subtitle number: subtitles are counted from 1, as the file shows them` |
| indeks, który nie jest liczbą całkowitą | `"…" is not a subtitle number: expected a whole number, counted from 1 as the file shows them` |
| nieprawidłowy czas | takie jak w [`shift`](shift.md) |
| `--first` i `--last` z tym samym indeksem | `--first and --last both name subtitle N, and two references on one subtitle define no transform` |
| indeks poza zakresem | `<path>: subtitle N is past the end: the file holds M subtitles` |
| dwa napisy zaczynające się razem | `<path>: subtitles N and M start at the same moment, so they define no transform` |
| pozycja przed początkiem osi czasu | `<path>: subtitle N would land before the origin, which no subtitle file can hold` |

**Nic nie jest zapisywane bez jednej z trzech opcji miejsca docelowego**, a ich
błędy są wspólne dla podpoleceń, które zapisują:
zobacz [Uruchamianie](invocation.md#miejsce-docelowe).
