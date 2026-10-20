# `adjust`

<!-- machine-translated -->
> Ta strona jest tłumaczeniem maszynowym i nie została jeszcze sprawdzona przez osobę władającą tym językiem.

```
subedit-cli adjust [--speed CPS|off] [--shorten | --no-shorten] [--lengthen | --no-lengthen]
                   [--minimum TIME|off] [--maximum TIME|off] [--gap TIME|off]
                   [--range N-M|N-]
                   (--output FILE | --output-dir DIR | --in-place)
                   [--dry-run]
                   [--recursive]
                   <file>...
```

Dostosowuje **czas trwania** każdego napisu do szybkości czytania i do limitów: napis
zbyt krótki, by dało się go przeczytać, jest wydłużany, napis, który znika z ekranu
zbyt późno, może być skrócony, a dwa napisy nie nakładają się na siebie. **Przesuwa się tylko
koniec**: przesunięcie początku przesunęłoby napis, a to zadanie dla
[`shift`](shift.md). To operacja `Adjust Durations…` w
[oknie](../subedit-gui/operations.md#adjust-durations), z tymi samymi wartościami domyślnymi.

## Cztery ograniczenia i ich kolejność

Stosuje się je **w tej kolejności**, każde ustawia koniec, a ostatnie wygrywa:

1. **szybkość czytania** — czas trwania, którego potrzebuje tekst, przy zadanej liczbie znaków na
   sekundę (znaczniki się nie liczą, podziały linii tak);
2. **minimum** — czas trwania poniżej niego jest podnoszony do minimum;
3. **maksimum** — czas trwania powyżej niego jest obniżany do maksimum;
4. **przerwa** — czas pozostawiony przed następnym napisem. Działa na końcu i
   **wygrywa**: dwa napisy pokazane jednocześnie to błąd, który widz zauważa,
   napis nieco za krótki nim nie jest.

Ostatni napis pliku nie ma następnego, a więc nie ma przerwy do zachowania. Następnym
napisem jest ten, który jest w pliku, **niezależnie od tego, czy wybrała go** opcja `--range`.

| Ograniczenie | Opcja | Domyślnie | Wyłączenie |
| :--------- | :----- | :----- | :------- |
| szybkość | `--speed CPS` | **15** znaków na sekundę | `--speed off` |
| kierunek szybkości | `--shorten`, `--no-shorten`, `--lengthen`, `--no-lengthen` | tylko **wydłuża** | — |
| minimum | `--minimum TIME` | **1,5 s** | `--minimum off` |
| maksimum | `--maximum TIME` | **brak** | `--maximum off` |
| przerwa | `--gap TIME` | **0** | `--gap off` |

Są to ustawienia Gaupola i pierwszego uruchomienia okna. **`off` wyłącza
ograniczenie: nie jest już sprawdzane.** To nie to samo co ustawienie go na zero — minimum równe `0`
to minimum równe zero, przerwa równa `0` zabrania jakiegokolwiek nakładania się.

## Co dostarcza plik ustawień

**Powyższe wartości domyślne są wbudowane.** Domyślnie `adjust` odczytuje ustawienia operacji z
pliku ustawień — wartości, które `Adjust Durations…` zapamiętuje w oknie albo które zapisało
[`config set`](config.md) — i zajmują one miejsce wbudowanych wartości domyślnych. **Opcja
podana w wierszu zawsze wygrywa**, potem plik, potem wbudowana wartość domyślna. Zob.
[Plik ustawień](invocation.md#plik-ustawień), gdzie jest plik, oraz `--config` i
`--no-config`.

| Klucz | Co dostarcza | Opcja, która go przesłania |
| :-- | :--------------- | :----------------------- |
| `duration-adjust.speed` | szybkość czytania, w znakach na sekundę | `--speed CPS` |
| `duration-adjust.lengthen`, `duration-adjust.shorten` | w którą stronę szybkość może przesuwać koniec; oba `false` wyłączają szybkość | `--lengthen`, `--no-lengthen`, `--shorten`, `--no-shorten`, `--speed off`; podanie `--speed CPS` lub kierunku włącza z powrotem szybkość wyłączoną w pliku, z wydłużaniem |
| `duration-adjust.minimum-enabled`, `duration-adjust.minimum-ms` | najkrótszy czas trwania albo żaden | `--minimum TIME`, `--minimum off` |
| `duration-adjust.maximum-enabled`, `duration-adjust.maximum-ms` | najdłuższy czas trwania albo żaden | `--maximum TIME`, `--maximum off` |
| `duration-adjust.gap-enabled`, `duration-adjust.gap-ms` | najmniejsza przerwa albo żadna | `--gap TIME`, `--gap off` |

**Każdy klucz ma opcję, która go cofa.** Każdy kierunek ma parę flag, które go ustawiają
niezależnie od tego, co mówi plik: `--shorten` i `--no-shorten` dla skracania, `--lengthen` i
`--no-lengthen` dla wydłużania; flaga niepodana zostawia kierunek z pliku bez zmian.
`--maximum off` usuwa maksimum, które włącza plik. Z `--format json` pole `constraints` podaje
ustawienia faktycznie użyte, skądkolwiek pochodzą, a `settings` — odczytany plik:

<!-- example: printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine/settings.conf config set duration-adjust.speed 12 2>/dev/null; subedit-cli --config mine/settings.conf --format json adjust --dry-run a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine/settings.conf config set duration-adjust.speed 12 2>/dev/null; subedit-cli --config mine/settings.conf --format json adjust --dry-run a.srt
settings: read mine/settings.conf
a.srt: adjusted the durations of 1 subtitle (dry run, nothing written)
{"schema":1,"command":"adjust","file":"a.srt","ok":true,"dry_run":true,"destination":null,"counts":{"subtitles":1,"adjusted":1,"sacrificed":{"speed":0,"minimum":0,"gap":0}},"constraints":{"speed":{"cps":"12","lengthen":true,"shorten":false},"minimum_ms":1500,"maximum_ms":null,"gap_ms":0},"settings":"mine/settings.conf","warnings":[]}
```

Pozostałe przykłady na tej stronie uruchomiono z `--no-config`, aby pokazywały, co robią wbudowane
wartości domyślne, niezależnie od ustawień maszyny, która je uruchamia.

<!-- example: subedit-cli adjust --help -->
```console
$ subedit-cli adjust --help
Bring the duration of every subtitle within a reading speed and limits
Usage: subedit-cli adjust [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to adjust

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --speed CPS|off             Reading speed in characters per second, or off (default 15)
  --shorten                   Let the reading speed bring an end earlier
  --no-shorten                Do not let the reading speed bring an end earlier
  --lengthen                  Let the reading speed move an end later
  --no-lengthen               Do not let the reading speed move an end later
  --minimum TIME|off          Shortest duration, or off (default 1.5)
  --maximum TIME|off          Longest duration, or off (default: none)
  --gap TIME|off              Least time left before the next subtitle, or off (default 0)
  --range N-M|N-              Act only on subtitles N to M, or N to the end
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
| `--speed` | nie | liczba znaków na sekundę, **większa od zera** (`15`, `12.5`), lub `off` | `15` |
| `--shorten` | nie | flaga: szybkość może także **przesuwać** koniec **wcześniej** | wyłączone |
| `--no-shorten` | nie | flaga: szybkość nie **przesuwa** już końca **wcześniej**, niezależnie od tego, co mówi plik ustawień | wyłączone |
| `--lengthen` | nie | flaga: szybkość **przesuwa** koniec **później**, niezależnie od tego, co mówi plik ustawień | wyłączone |
| `--no-lengthen` | nie | flaga: szybkość nie **przesuwa** już końca **później** | wyłączone |
| `--minimum` | nie | czas trwania równy zero lub większy, albo `off` | `1.5` |
| `--maximum` | nie | czas trwania **większy od zera**, albo `off` | brak |
| `--gap` | nie | czas trwania równy zero lub większy, albo `off` | `0` |
| `--range` | nie | `N-M` lub `N-` do końca — zob. niżej | cały plik |
| `--output` / `--output-dir` / `--in-place` | **jedna z trzech**, z wyjątkiem `--dry-run` | zob. [Uruchamianie](invocation.md#miejsce-docelowe) | — |
| `--dry-run` | nie | flaga: oblicza i mówi, nic nie zapisuje — zob. [Podgląd przed zapisem](invocation.md#podgląd-przed-zapisem) | wyłączone |

Czas trwania zapisuje się tak jak w [`shift`](shift.md#zapis-czasu-trwania): `2.5`, `00:00:02.500`.

**Bez żadnego aktywnego ograniczenia `adjust` jest odrzucane** (kod `1`, nic nie jest zapisywane): nie
byłoby do czego dostosowywać, a polecenie, które nic nie robi, mówiąc, że się powiodło, to najgorsza
odpowiedź. Tak jest w przypadku `--speed off --minimum off --gap off` bez maksimum oraz samego
`--no-lengthen` z wyłączonym minimum i przerwą: szybkość, która nie przesuwa żadnego
końca, niczego nie ogranicza.

`--shorten`, `--no-shorten`, `--lengthen` i `--no-lengthen` mówią, w którą stronę szybkość przesuwa
koniec; mają sens tylko z szybkością, a **`--speed off` je odrzuca**. Flaga podana razem ze swoją
przeciwnością — `--shorten --no-shorten`, `--lengthen --no-lengthen` — też jest odrzucana.

### `--range`

`--range 3-7` ogranicza dostosowanie do napisów od 3 do 7, **liczonych od 1, łącznie z
granicami**; `--range 3-` sięga do ostatniego. Pojedynczy napis zapisuje się `4-4`.

- Zakres, **który kończy się przed początkiem**, `0`, znak lub pojedyncza liczba są
  odrzucane jako błąd użycia (kod `1`).
- **Obowiązuje dla każdego pliku partii**, co ma sens tylko wtedy, gdy pliki
  są do siebie podobne. Jest oceniany **względem każdego pliku**: zakres wykraczający poza ten plik jest
  odrzucany **zanim cokolwiek zostanie ruszone**, z nazwaniem granicy —
  `range 3-9 ends after the last subtitle: the file has 4` —, ten plik kończy się błędem (kod `2` lub `3`, identyfikator
  `range-out-of-bounds`), a pozostałe są przetwarzane dalej.
- Liczniki raportu dotyczą tylko wybranych napisów.

## Co zostaje poświęcone

Dwa ograniczenia mogą być ze sobą sprzeczne dla tego samego napisu: minimum 1,5 s nie mieści się
w 0,8 s, które pozostawia następny napis. **Decyduje kolejność, a raport mówi,
co ustąpiło** — czego Gaupol nie robi: łamie je po cichu.

Każde ograniczenie jest wymieniane tylko wtedy, gdy **co najmniej jeden napis go nie otrzymał**, po
zastosowaniu pozostałych. Maksimum nie ma licznika: jest stosowane przed przerwą, która może
tylko skracać, więc zawsze obowiązuje. Minimum większe od maksimum ustępuje maksimum i jest
liczone jako minimum.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --output ajuste.srt courts.srt; grep -e '-->' ajuste.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --output ajuste.srt courts.srt; grep -e '-->' ajuste.srt
courts.srt: adjusted the durations of 3 subtitles; could not satisfy the minimum duration in 1 subtitle -> ajuste.srt
00:00:00,000 --> 00:00:02,000
00:00:10,000 --> 00:00:14,000
00:00:14,200 --> 00:00:15,000
00:00:15,000 --> 00:00:16,500
```

Trzeci napis chciałby kończyć się w 15,7 s, aby trwać 1,5 s, ale czwarty zaczyna się w
15,0 s: **przerwa wygrywa**, kończy się w 15,0 s, a raport wymienia minimum.

Wyłączenie przerwy pozwala wygrać minimum — i oba napisy nakładają się na siebie:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --gap off --output libre.srt courts.srt; grep -e '-->' libre.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --gap off --output libre.srt courts.srt; grep -e '-->' libre.srt
courts.srt: adjusted the durations of 3 subtitles -> libre.srt
00:00:00,000 --> 00:00:02,000
00:00:10,000 --> 00:00:14,000
00:00:14,200 --> 00:00:15,700
00:00:15,000 --> 00:00:16,500
```

I nic nie jest poświęcane ani dostosowywane, gdy plik mieści się już w swoich limitach:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --speed off --minimum off --output intact.srt courts.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --speed off --minimum off --output intact.srt courts.srt
courts.srt: no duration to adjust -> intact.srt
```

Plik jest zapisywany bez zmian: **podane miejsce docelowe jest miejscem zapisanym**.

## Wynik

**Standardowe wyjście** — nic: wynikiem jest zapisany plik.

| Poziom | Co jest dodawane na wyjściu błędów |
| :----- | :------------------------------------- |
| 1 | `<path>: adjusted the durations of N subtitles; could not satisfy …  -> <destination>` lub `<path>: no duration to adjust -> <destination>` — a najpierw odczytany plik ustawień — `settings: read <path>` — zob. [Plik ustawień](invocation.md#plik-ustawień) |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — format, kodowanie, znacznik i końce linii odczytanego pliku, przywrócone takie, jakie były |
| 3 | `<path>: N bytes read, M written`, a następnie **każdy komunikat diagnostyczny odczytu** — zob. [Uruchamianie](invocation.md#odczyt-komunikatów-diagnostycznych) |

Zdanie jest zdaniem okna: `N` to liczba napisów, których koniec się przesunął, a
reszta, nieobecna, gdy nic nie ustąpiło, wymienia to, czego nie zachowano — `the reading speed`,
`the minimum duration`, `the gap` — wraz z liczbą napisów, których to dotyczy.

### W formacie JSON

Z [`--format json`](invocation.md#wynik-czytelny-dla-skryptu) obiekt każdego pliku
zawiera, poza kopertą:

| Klucz | Zawartość |
| :-- | :------ |
| `counts.subtitles` | napisy **objęte operacją**: cały plik albo zakres |
| `counts.adjusted` | te, których koniec się przesunął |
| `counts.sacrificed.speed`, `.minimum`, `.gap` | napisy, które nadal nie spełniają tego ograniczenia po zastosowaniu pozostałych, **łącznie z tymi, których koniec się nie przesunął** |
| `constraints.speed` | `{"cps", "lengthen", "shorten"}` — liczba znaków na sekundę **jako łańcuch** (`"15"`, `"12.5"`) — lub `null`, jeśli wyłączona |
| `constraints.minimum_ms`, `.maximum_ms`, `.gap_ms` | użyte czasy trwania, w **całych milisekundach**, lub `null`, jeśli wyłączone |
| `settings` | odczytany plik ustawień albo `null`, gdy żadnego nie odczytano |

`constraints` podaje **ustawienia faktycznie użyte**, łącznie z domyślnymi: dwa obliczenia
dotyczące tych samych napisów można wtedy porównać bez czytania wiersza poleceń.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config --format json adjust --dry-run --maximum 1.8 courts.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config --format json adjust --dry-run --maximum 1.8 courts.srt
courts.srt: adjusted the durations of 4 subtitles; could not satisfy the reading speed in 1 subtitle, the minimum duration in 1 subtitle (dry run, nothing written)
{"schema":1,"command":"adjust","file":"courts.srt","ok":true,"dry_run":true,"destination":null,"counts":{"subtitles":4,"adjusted":4,"sacrificed":{"speed":1,"minimum":1,"gap":0}},"constraints":{"speed":{"cps":"15","lengthen":true,"shorten":false},"minimum_ms":1500,"maximum_ms":1800,"gap_ms":0},"settings":null,"warnings":[]}
```

## Kody wyjścia

Takie jak w całym narzędziu: `0`, jeśli przetworzono każdy plik, `2`, jeśli żaden, `3`, jeśli tylko
niektóre, `1` przy błędzie użycia. Plik, który mieści się już w swoich limitach, jest przetworzony.

## Błędy

| Co to wywołuje | Komunikat |
| :------------------ | :------ |
| plik wskazany w `--config`, który nie istnieje — brakujący własny plik użytkownika daje wartości domyślne | `settings file not found: <FILE>` |
| brak aktywnego ograniczenia | `adjust has nothing to adjust to: every constraint is off, or the reading speed is told to move no end` |
| wartość, która nie jest szybkością | `--speed: "…" is not a reading speed: expected a number of characters per second above zero, like 15 or 12.5, or off` |
| flaga kierunku z `--speed off` | `--lengthen, --no-lengthen, --shorten and --no-shorten say which way the reading speed may move an end, and --speed off switches the reading speed off` |
| flaga kierunku ze swoją przeciwnością | `--shorten and --no-shorten say opposite things; give one of them` (tak samo `--lengthen` i `--no-lengthen`) |
| wartość, która nie jest czasem trwania | `--minimum: "…" is not a time: …` (tak samo `--maximum`, `--gap`) |
| ujemny czas trwania | `--minimum: "-1" is not a duration of zero or more` (tak samo `--gap`) |
| maksimum równe zero lub ujemne | `--maximum: "0" is not a duration above zero` |
| źle zapisany zakres | `--range: "…" is not a range: write it N-M, or N- to go to the end, counted from 1 and inclusive` |
| zakres, który kończy się przed początkiem | `--range: "5-3" is not a range: it ends before it starts` |
| zakres poza plikiem | `<path>: range N-M starts after the last subtitle: the file has K` lub `… ends after …` |

Błędy miejsca docelowego są wspólne dla podpoleceń, które zapisują:
zob. [Uruchamianie](invocation.md#miejsce-docelowe).
