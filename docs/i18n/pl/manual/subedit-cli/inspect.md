# `inspect`

<!-- machine-translated -->
> Ta strona jest tłumaczeniem maszynowym i nie została jeszcze sprawdzona przez osobę władającą tym językiem.

```
subedit-cli inspect [--frame-rate RATE] [--recursive] [--detail]
                   [-t FILE] [--align-method position|number] <file>...
```

Zgłasza, z czego składa się każdy plik. **Niczego nie zmienia i nie zapisuje żadnego
pliku.**

<!-- example: subedit-cli inspect --help -->
```console
$ subedit-cli inspect --help
Report what a subtitle file is made of
Usage: subedit-cli inspect [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to report on

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --detail                    List every anomaly, subtitle by subtitle, instead of counting them
  --frame-rate RATE           Frame rate of a file counted in frames: 25, 23.976
  -t,--translation-file FILE  Translation file to lay over the subtitle file, for a single input
  --align-method position|number
                              How the lines of the translation find their subtitles
```

## Argumenty

| Argument | Wymagany | Wartość | Domyślnie |
| :------- | :----- | :----- | :----- |
| `<file>...` | tak | jedna lub więcej ścieżek plików napisów | — |
| `--recursive`, `-r` | nie | flaga | wyłączona — zobacz [Przetwarzanie drzewa katalogów](lots.md) |
| `--detail` | nie | flaga: nazywa każdą anomalię zamiast je zliczać — zobacz [Co zgłasza `anomalies`](#co-zgłasza-anomalies) | wyłączona |

Brak ścieżki to błąd użycia, a więc kod `1`.

## Opcje

| Opcja | Wymagana | Wartość | Domyślnie |
| :----- | :------ | :----- | :----- |
| `--frame-rate` | nie | liczba klatek na sekundę: `25`, `23.976`, `29.97` | 23,976 |

**Służy tylko plikom liczonym w klatkach**, czyli MicroDVD — jedynemu
z dziewięciu. Plik MicroDVD niesie wyłącznie numery klatek; liczba
klatek na sekundę, przy której są odczytywane, decyduje o **każdej pozycji raportu**,
począwszy od `span`. Podana dla innych formatów, niczego nie zmienia i nie jest
błędem.

Wartość, która niczego nie nazywa, to błąd użycia, a więc kod `1`:

<!-- example: subedit-cli inspect --frame-rate douze film.sub; echo $? -->
```console
$ subedit-cli inspect --frame-rate douze film.sub; echo $?
--frame-rate: "douze" is not a frame rate: expected frames per second, like 25 or 23.976
1
```

To ta sama opcja co w podpoleceniu [`convert`](convert.md), słowo w słowo, ponieważ
to ta sama liczba klatek na sekundę. W `convert` służy ona także do **zapisu**.

## Tłumaczenie: `-t`

| Opcja | Wymagana | Wartość | Domyślnie |
| :----- | :------ | :----- | :----- |
| `-t`, `--translation-file` | nie | plik tłumaczenia, dla **jednego** pliku wejściowego | — |
| `--align-method` | nie, i tylko z `-t` | `position` lub `number` | `position` |

Z opcją `-t` tłumaczenie jest nakładane na plik **tak, jak robi to okno przy jego otwieraniu** — zobacz
[Tłumaczenie](invocation.md#tłumaczenie) — a raport mówi, **jak jego linie znalazły swój
napis**. Skrypt wie więc, czy tłumaczenie jest dopasowane, zanim je poprawi.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour.\n\n' > film.fr.srt; subedit-cli --quiet inspect -t film.fr.srt film.srt | tail -3 -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour.\n\n' > film.fr.srt; subedit-cli --quiet inspect -t film.fr.srt film.srt | tail -3
  anomalies: none
  translation file: film.fr.srt, matched by position
  translation: 1 line attached; 1 subtitle left without a translation
```

Pierwszy dodany wiersz nazywa plik i metodę; drugi to **zdanie, które okno
wypowiada**, gdy otwiera tłumaczenie, każda jego część tylko wtedy, gdy nie jest zerem:

| Część | Co zlicza |
| :----- | :---------------- |
| `N lines attached` | linie tłumaczenia, które znalazły napis pliku głównego |
| `N subtitles born of a line` | linie, które nie znalazły żadnego i dały początek własnym napisom, w swoich własnych pozycjach |
| `N subtitles left without a translation` | napisy pliku głównego, do których nie dotarła żadna linia |
| `N lines out of order` | linie, które wystąpiły po późniejszej linii w pliku |

Tłumaczenie, które nie zawiera żadnej linii, mówi `translation: the file holds no line`. **Raport samego
pliku pozostaje raportem samego pliku**: napis zrodzony z linii nie jest zliczany w
`subtitles`.

Błędy opcji `-t` to błędy odczytu, podawane ze ścieżką tłumaczenia, oraz ten:
`<translation>: the file is already open as the main document`, gdy podany plik jest plikiem głównym.
Opcja `--align-method` bez `-t` to błąd użycia, podobnie jak `-t` z kilkoma plikami wejściowymi — zobacz
[Tłumaczenie](invocation.md#tłumaczenie).

## Wynik

Raport trafia na **standardowe wyjście** — jest wynikiem polecenia i
jest zapisywany na każdym poziomie, także z `--quiet`. Jeden blok na plik, poprzedzony
ścieżką:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,500\nLe canot dérive.\n\n2\n00:00:04,000 --> 00:00:06,200\nPersonne n a vu la côte.\n' > exemple.srt; subedit-cli --quiet inspect exemple.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,500\nLe canot dérive.\n\n2\n00:00:04,000 --> 00:00:06,200\nPersonne n a vu la côte.\n' > exemple.srt; subedit-cli --quiet inspect exemple.srt
exemple.srt
  format: SubRip
  encoding: UTF-8, detected
  byte order mark: absent
  line endings: LF
  subtitles: 2
  span: 00:00:01.000 -> 00:00:06.200
  frame rate grid: none (too few subtitles to tell)
  anomalies: none
```

| Pole | Co mówi |
| :---- | :----------- |
| `format` | jeden z formatów przyjmowanych przez `--to`, zapisany w całości: `SubRip`, `WebVTT`, `SubViewer 2`, `Sub Station Alpha`, `Advanced SSA`, `MicroDVD`, `MPL2`, `TMPlayer`, `LRC` |
| `encoding` | odczytane kodowanie, a po nim **skąd pochodzi odpowiedź**: `, from its byte order mark`, jeśli plik je deklaruje, `, as asked for`, jeśli narzuciła je opcja `--encoding`, `, detected`, jeśli zostało ocenione |
| `byte order mark` | `present` lub `absent` |
| `line endings` | `LF`, `CRLF` lub `CR`, a po nich `, mixed from line N`, jeśli plik je miesza |
| `subtitles` | liczba odczytanych napisów |
| `span` | od najwcześniejszego początku do najpóźniejszego końca, `HH:MM:SS.mmm` |
| `frame rate grid` | liczba klatek na sekundę, względem której obliczono pozycje, **wywnioskowana** — zobacz niżej |
| `frame rate` | dla pliku liczonego w klatkach liczba klatek na sekundę, przy której go odczytano, i **skąd pochodzi** — zastępuje poprzedni wiersz, zobacz niżej |
| `anomalies` | `none` w tym samym wierszu albo to, co jest nie tak: **jeden rodzaj w wierszu z jego liczbą**, z wcięciem pod `anomalies:`; z opcją `--detail` — **jedna anomalia w wierszu**, napis po napisie |

**`span` to nie „od pierwszego do ostatniego”**, lecz od najwcześniejszego do najpóźniejszego:
w pliku o zaburzonej kolejności oba się różnią i tylko drugie mówi
prawdę o tym, co plik obejmuje.

**`anomalies` z opcją `--detail` liczy napisy od 1**, tak jak numeruje je plik
— a nie linie pliku. Numer napisu przetrwa
zmianę; numer linii nie. Zobacz niżej.

## Co zgłasza `frame rate grid`

**Plik napisów nie deklaruje swojej liczby klatek na sekundę.** SubRip nie ma
nagłówka, nagłówek WebVTT to wolny tekst. Ten wiersz niczego więc nie odczytuje:
on **wnioskuje**, mierząc, czy pozycje wypadają na siatce klatek.

Mierzone są **początki**. Wszędzie tam, gdzie siatka istnieje, leżą one na niej
bez wyjątku, natomiast końce często od niej odbiegają — koniec jest
zwykle obliczany według reguły szybkości czytania, a nie ustawiany na
klatce.

Pierwszy wiersz zawsze mówi to samo: wybraną liczbę klatek na sekundę, werdykt
i koncentrację, która go podpiera.

| Werdykt | Co oznacza |
| :------ | :----------------- |
| `clean` | pozycje leżą na siatce |
| `partial` | część z nich leży, reszta nie |
| `none` | żaden kandydat niczego nie wyjaśnia |

`none` ma **dwie przyczyny i nie jest to ta sama odpowiedź**: albo żadna
siatka nie pasuje — wiersz podaje wtedy najlepszego kandydata i jego wynik,
aby było widać, o ile zawodzi — albo plik liczy zbyt mało
napisów, by cokolwiek o nim powiedzieć. Dwa początki zawsze wyglądają na
idealnie wyrównane; to niczego nie dowodzi.

**Regularny plik o niestandardowej liczbie klatek na sekundę daje `none` i tak ma być.**
Zbiór kandydatów jest zamknięty — osiem wartości, które
oferuje konwersja — a zgłaszanie najmniej błędnej z nich oznaczałoby podanie
złej odpowiedzi tam, gdzie „nie wiem” jest właściwą.

### Wiersze, które następują, gdy mają coś do powiedzenia

| Wiersz | Kiedy się pojawia |
| :---- | :------------------ |
| `grid offset` | pozycje leżą na siatce **z dokładnością do stałej**: plik został przesunięty, a oto o ile |
| `also fits` | inny kandydat pasuje równie dobrze, ponieważ jest całkowitą wielokrotnością wybranego |
| `too short a span to separate` | zakres pliku nie pozwala odróżnić wybranej liczby klatek na sekundę od wymienionych |
| `off the grid` | ile początków odbiega od siatki i w ilu **ciągach** |

**`also fits` nie jest wahaniem.** Siatka przy 25 klatkach na sekundę jest
*zawarta* w siatce przy 50: plik zsynchronizowany na 25 pasuje więc do obu.
Odwrotnie to nieprawda — prawdziwe 50 zapada się na 25 — dlatego wybierana jest
niższa, a druga jest nazywana zamiast przemilczana. Dotyczy to wyłącznie
par 25 i 50, 29.97 i 59.94, 30 i 60.

**`too short a span to separate` chroni przed zbyt pewną odpowiedzią.** 23.976
i 24 rozchodzą się o jedną milisekundę na sekundę filmu: na filmie pełnometrażowym
rozdzielają się ponad wszelką wątpliwość, na dziesięciu sekundach nie da się ich odróżnić. Wiersz mówi,
która pozostaje możliwa.

**`off the grid` odróżnia dwie historie, które sam procent myli.**
Wiele ciągów pojedynczego początku to pozycje poprawiane ręcznie,
jedna po drugiej. Kilka długich ciągów to sekcja zsynchronizowana od nowa lub plik
złożony z dwóch innych. Liczba ciągów je rozróżnia.

### Przykład

Dwanaście linii ustawionych na siatce przy 25 klatkach na sekundę — a więc w
pozycjach będących wielokrotnościami 40 milisekund — rozłożonych na prawie dziewięć minut:

<!-- example: k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > grille.srt; subedit-cli --quiet inspect grille.srt | tail -3 -->
```console
$ k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > grille.srt; subedit-cli --quiet inspect grille.srt | tail -3
  frame rate grid: 25 fps, clean (100.0%)
  also fits: 50 fps, of which this rate is a whole divisor
  anomalies: none
```

Przykład tworzy własny plik zamiast czytać plik z repozytorium i jest to
zamierzone: można go odtworzyć bez zmian, a pętla pokazuje, czym jest siatka,
lepiej niż zdanie — pozycje będące wielokrotnościami czasu trwania klatki.

## Co zgłasza `frame rate` i dlaczego to nie jest siatka

**Tylko jeden z dziewięciu formatów liczy w klatkach**: MicroDVD, którego każda linia
niesie dwa numery klatek i nic więcej. W takim pliku `inspect`
nie zapisuje `frame rate grid`, lecz `frame rate`, a oba wiersze nigdy
nie występują razem.

**Powód mieści się w jednym zdaniu.** Pozycje pliku MicroDVD zostały
obliczone *z* jego klatek, przy danej liczbie klatek na sekundę; wnioskowanie ich z
siatki dałoby z powrotem właśnie tę liczbę, skoro została przed chwilą podana. Byłaby to
podana wartość przebrana za pomiar.

Zamiast tego zapisywana jest **liczba klatek na sekundę użyta do odczytu i skąd pochodzi** —
w tej samej postaci co wiersz `encoding`:

| Co mówi wiersz | Kiedy |
| :------------------ | :---- |
| `frame rate: 25 fps, as asked for` | narzuciła ją opcja `--frame-rate` |
| `frame rate: 24000/1001 fps, assumed` | nikt jej nie podał; to wartość domyślna |

**Dwie odpowiedzi, a nie trzy**, jak przy `encoding`: plik MicroDVD
niczego nie deklaruje. W tym cały problem — liczba klatek na sekundę z konieczności pochodzi
z zewnątrz, a raport mówi, z którego z dwóch zewnętrz.

**Liczba klatek na sekundę decyduje o całej reszcie raportu.** Przechodzi przez nią `span`, a dwa
odczyty tego samego pliku przy dwóch wartościach dają dwa zakresy:

<!-- example: printf '{25}{75}Première.\n{100}{150}Deuxième.\n{200}{260}Troisième.\n' > images.sub; subedit-cli --quiet inspect images.sub | tail -3; subedit-cli --quiet inspect --frame-rate 25 images.sub | tail -3 -->
```console
$ printf '{25}{75}Première.\n{100}{150}Deuxième.\n{200}{260}Troisième.\n' > images.sub; subedit-cli --quiet inspect images.sub | tail -3; subedit-cli --quiet inspect --frame-rate 25 images.sub | tail -3
  span: 00:00:01.043 -> 00:00:10.844
  frame rate: 24000/1001 fps, assumed
  anomalies: none
  span: 00:00:01.000 -> 00:00:10.400
  frame rate: 25 fps, as asked for
  anomalies: none
```

## Co zgłasza `anomalies`

W dokumencie mogą być nie tak trzy rzeczy i każda naprawia się inaczej.
**Domyślnie `inspect` je zlicza**, jeden wiersz na rodzaj, w kolejności tabeli:
źle zrobiony plik niesie ich setki, a wiedza, ile jest każdego rodzaju,
mówi, co uruchomić. **Opcja `--detail` nazywa je jedna po drugiej.**

| Co jest zapisane | Z opcją `--detail` | Co to znaczy |
| :--------------- | :-------------- | :------------------ |
| `subtitle ends before it starts: N` | `subtitle N ends before it starts` | koniec wypada przed początkiem |
| `subtitle starts before the previous one ends: N` | `subtitle N starts before the previous one ends` | nakłada się na poprzedni |
| `subtitle starts before the previous one starts: N` | `subtitle N starts before the previous one starts` | łamie kolejność pliku — [`sort`](sort.md) przywraca ją |

Rodzaj, którego nie ma, nie jest zapisywany. W lewej kolumnie `N` to liczba
napisów; w następnej — numer napisu.

**Ten sam napis może pojawić się dwa razy** i jest to zamierzone: napis, który
zaczyna się przed początkiem poprzedniego, zaczyna się także przed jego
końcem. Pierwszą kwestię rozstrzyga się, przenosząc napis, drugą —
zmieniając czas trwania.

**Nazwany jest ten napis, który stoi w złym miejscu**, a nie ten, względem którego
stoi źle. Przy początkach `0`, `4 s`, `2 s`, `3 s`: trzeci łamie kolejność, czwarty
podąża za trzecim i dlatego nie jest nazwany — nie ma przy nim nic do
zrobienia.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect desordre.srt | tail -3 -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect desordre.srt | tail -3
  anomalies:
    subtitle starts before the previous one ends: 1
    subtitle starts before the previous one starts: 1
```

A z opcją `--detail` nazwany jest napis:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect --detail desordre.srt | tail -3 -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect --detail desordre.srt | tail -3
  anomalies:
    subtitle 3 starts before the previous one ends
    subtitle 3 starts before the previous one starts
```

**Jeden wiersz na anomalię z opcją `--detail`**, ponieważ setki ustaleń
sklejonych w jednym wierszu nie da się ani przeczytać, ani przeszukać. Dokument bez wad zachowuje
swój pojedynczy wiersz, `anomalies: none`, którego szuka skrypt. **Rekord
`--format json` zawsze wylicza każdą anomalię**: skrypt czyta szczegóły, a
`--detail` zmienia tylko tekst.

**To nie jest komunikat diagnostyczny odczytu.** Komunikaty diagnostyczne mówią, na co
natrafił odczyt, i wskazują **linię pliku**; anomalie mówią, czym
dokument *jest*, i wskazują **napis**. Plik o zaburzonej kolejności
nie jest przez to uszkodzony: odczyt nie ma o tym nic do powiedzenia.

## W formacie JSON

Z opcją [`--format json`](invocation.md#wynik-czytelny-dla-skryptu) `inspect` zapisuje **jeden
obiekt na plik** zamiast powyższych bloków; komentarz pozostaje na wyjściu błędów.
Klucze opisują to, co mówi tekst, a **żadnego nie należy odczytywać ze zdania**:

| Klucz | Zawartość |
| :-- | :------ |
| `format` | identyfikator formatu, ten przyjmowany przez `convert --to`: `srt`, `vtt`… |
| `encoding` | `{"charset", "origin", "byte_order_mark"}` — `origin` to `byte-order-mark`, `asked` lub `detected` |
| `line_endings` | `{"kind", "mixed_from_line"}` — `lf`, `crlf` lub `cr`; linia, w której pojawia się drugi rodzaj, lub `null` |
| `subtitles` | liczba napisów |
| `span_ms` | `{"start", "end"}` — pierwszy początek i ostatni koniec, w milisekundach |
| `frame_rate` | dla pliku liczonego w klatkach: `{"rate", "origin"}` (`asked` lub `assumed`); w przeciwnym razie `null` |
| `grid` | dla pozostałych: wywnioskowana siatka lub `null` dla pliku liczonego w klatkach |
| `anomalies` | tablica `{"subtitle", "kind"}` — `end-before-start`, `overlapping-subtitles`, `out-of-order` |
| `translation` | **tylko z opcją `-t`**: `{"file", "method", "attached", "born", "untranslated", "out_of_order"}` — `method` to `position` lub `number`, a cztery liczniki to te ze zdania powyżej |
| `warnings` | to, na co natrafił odczyt, na każdym poziomie |

`grid` niesie `verdict` (`clean`, `partial` lub `silent`), `enough_starts`, `rate` (łańcuch znaków
lub `null`, gdy nic nie jest nazwane), `concentration_permille` (tysięczne, `null` bez dostatecznej liczby
początków), `offset_ms`, `also_fits` (liczba klatek na sekundę, której ta jest całkowitym dzielnikiem, lub
`null`), `not_separated` (wartości, których zbyt krótki zakres nie rozdziela), `strays`
(początki poza siatką) i `starts`.

<!-- example: printf '{25}{50}Un.\n{75}{100}Deux.\n' > f.sub; subedit-cli --format json inspect --frame-rate 25 f.sub 2>/dev/null -->
```console
$ printf '{25}{50}Un.\n{75}{100}Deux.\n' > f.sub; subedit-cli --format json inspect --frame-rate 25 f.sub 2>/dev/null
{"schema":1,"command":"inspect","file":"f.sub","ok":true,"format":"microdvd","encoding":{"charset":"UTF-8","origin":"detected","byte_order_mark":false},"line_endings":{"kind":"lf","mixed_from_line":null},"subtitles":2,"span_ms":{"start":1000,"end":4000},"frame_rate":{"rate":"25","origin":"asked"},"grid":null,"anomalies":[],"warnings":[]}
```

## Komentarz

Na wyjściu błędów, zależnie od żądanego poziomu:

| Poziom | Co jest dodawane |
| :----- | :-------------- |
| 1 | `<path>: N subtitles` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings` |
| 3 | `<path>: N bytes read`, `<path>: N diagnostic(s) while reading`, a następnie **każdy z nich** — zobacz [Uruchamianie](invocation.md#odczyt-komunikatów-diagnostycznych) |

## Kody wyjścia

Takie jak w całym narzędziu: `0`, jeśli każdy plik został odczytany, `2`, jeśli żaden, `3`, jeśli
tylko niektóre, `1` przy błędzie użycia.

## Błędy

Błędy odczytu — pliku, a przy podanym `-t` także tłumaczenia — oraz
błędy opcji `-t`: zobacz wyżej. `inspect` nie zapisuje niczego, co mogłoby zostać odrzucone.

| Co to wywołuje | Komunikat |
| :------------------ | :------ |
| nie podano ścieżki | `files is required`, a po nim odsyłacz do `--help` |
| brakujący plik | `<path>: does not exist` |
| nieczytelny plik | `<path>: cannot be opened: permission denied` |
| bajty, których nie da się zdekodować | `<path>: cannot be decoded in the chosen encoding` |
| format nierozpoznany | `<path>: is in no format this tool knows` |
| nic, co przypominałoby napis | `<path>: holds nothing recognisable as a subtitle` |

To te same komunikaty dla siedemnastu podpoleceń: przepis na otwarcie
jest zapisany raz, w rdzeniu.

## Przykład zakończony niepowodzeniem

Pusty plik nie pasuje do żadnego formatu. Nie jest zgłaszany jako pusty:
jest odrzucany i nazwany.

<!-- example: : > vide.srt; subedit-cli inspect vide.srt; echo "code=$?" -->
```console
$ : > vide.srt; subedit-cli inspect vide.srt; echo "code=$?"
vide.srt: is in no format this tool knows
code=2
```
