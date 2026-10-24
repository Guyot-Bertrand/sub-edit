# Uruchamianie

<!-- machine-translated -->
> Ta strona jest tłumaczeniem maszynowym i nie została jeszcze sprawdzona przez osobę władającą tym językiem.

```
subedit-cli [global options] <subcommand> [options] <file>...
```

Bez podpolecenia narzędzie wypisuje swoją pomoc i kończy działanie.

<!-- example: subedit-cli -->
```console
$ subedit-cli
Read, inspect and retime subtitle files.
Usage: subedit-cli [OPTIONS] [SUBCOMMAND]

Options:
  -h,--help                   Print this help message and exit
  --version                   Display program version information and exit
  -v                          Say more: -v is the default, -vv details, -vvv debugs
  -q,--quiet                  Say nothing but errors
  --encoding NAME             Encoding to read the files in; detected by default
  --format text|json          Form of the result on standard output: text or json
  --config FILE               Read the operation settings from FILE instead of the user's settings file
  --no-config Excludes: --config
                              Ignore any settings file: use the built-in defaults
  --list-encodings            Write the encodings that can be read and written, one a line, and stop

Subcommands:
  inspect                     Report what a subtitle file is made of
  convert                     Write a subtitle file out in another format or shape
  shift                       Move every position of a file by a fixed amount
  transform                   Correct every position from two points known to be right
  framerate                   Re-time a file mastered at one frame rate for another
  snap                        Move every position onto the nearest frame of a frame rate (see framerate)
  hearing-impaired            Remove the sounds described between square brackets
  adjust                      Bring the duration of every subtitle within a reading speed and limits
  replace                     Replace a text in the subtitles, without breaking a tag
  case                        Put the texts in title, sentence, upper or lower case, tags intact
  italics                     Put the texts in italics, or take their italics out
  dialogue-dashes             Put dialogue dashes on the lines, or take them off
  sort                        Put the subtitles in the order of their start, keeping ties as they are
  join-repeats                Join the subtitles that repeat each other, one after the other, into one
  disjoint                    Cut overlapping subtitles so that no range overlaps another, merging what overlaps
  repair                      Join the repeated subtitles, put them in order, then make the ranges disjoint
  correct                     Correct the texts with the patterns of the correction assistant
  append                      Put subtitle files one after another into a single file
  split-file                  Cut a subtitle file in two, a head and a tail
  pair                        Write a translation at the positions of its main file, and say how it lined up
  transcribe                  Write the speech of a video or an audio file as subtitles
  config                      List, show and change the operation settings
```

## Język

Wiersz poleceń mówi językiem swojego środowiska: **jego pomoc, komentarz i
błędy** podążają za żądanym językiem. Przykłady z tego podręcznika są uruchamiane po angielsku —
bloki są generowane przez `make manual`, nigdy nie kopiowane ręcznie.

**Jak wybierany jest język.** Pierwsza z zmiennych `LC_ALL`, `LC_MESSAGES` i `LANG`, która jest ustawiona i
niepusta, wyznacza ustawienia regionalne; `LANGUAGE`, jeśli jest ustawiona, wymienia następnie żądane języki oddzielone
dwukropkami (`LANGUAGE=pt_BR:fr`) i jest czytana jako pierwsza. Taką kolejność stosuje gettext, czytana
przez sam program: ustawienia regionalne nie muszą być wygenerowane na maszynie.

| Reguła | Skutek |
| :--- | :----- |
| kodowanie i modyfikator | ignorowane: `fr_FR.UTF-8@euro` szuka `fr_FR`, potem `fr` |
| `LC_ALL=C`, ustawienia regionalne `C` lub `POSIX` albo ich brak | angielski, a `LANGUAGE` nie jest czytana |
| wpis nazywający angielski (`en`, `en_GB`) | angielski: to, co po nim następuje w `LANGUAGE`, nie jest czytane |
| język, którego katalog nie jest zainstalowany | następny z żądanych, a po ostatnim angielski, **bez słowa** |
| komunikat, którego katalog nie tłumaczy | pokazany po angielsku |

**Nazwa wzorca poprawek**, którego zastosowania `correct` nie udało się wykonać, również podąża za
językiem, gdy katalog ją zna — dotyczy to wzorców dostarczanych z programem; wzorzec własny jest
nazwany tak, jak zapisuje go jego plik. Same pliki wzorców nigdy nie są przepisywane.

Katalogi są instalowane w `<prefix>/share/subedit/locale/<language>/LC_MESSAGES/subedit.mo`
i znajdowane z własnego katalogu pliku wykonywalnego. Angielski jest wbudowany. Ta wersja instaluje `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR`, `ru`, `sr`, `tr` i `zh_CN` — kataloński,
czeski, niemiecki, hiszpański, fiński, francuski, galicyjski, hebrajski, węgierski, interlingue, islandzki, włoski, niderlandzki, polski, portugalski, brazylijski portugalski, rosyjski, serbski, turecki i chiński uproszczony — w pełni przetłumaczone. Francuski został sprawdzony
przez osobę władającą językiem; kataloński, czeski, niemiecki, hiszpański, fiński, galicyjski, hebrajski, węgierski, interlingue, islandzki, włoski, niderlandzki, polski, portugalski, brazylijski portugalski, rosyjski, serbski, turecki i chiński uproszczony są
przetłumaczone maszynowo i nie zostały jeszcze sprawdzone przez osobę władającą językiem.
<!-- machine-translated: ca -->
<!-- machine-translated: cs -->
<!-- machine-translated: de -->
<!-- machine-translated: es -->
<!-- machine-translated: fi -->
<!-- machine-translated: gl -->
<!-- machine-translated: he -->
<!-- machine-translated: hu -->
<!-- machine-translated: ie -->
<!-- machine-translated: is -->
<!-- machine-translated: it_IT -->
<!-- machine-translated: nl -->
<!-- machine-translated: pl -->
<!-- machine-translated: pt -->
<!-- machine-translated: pt_BR -->
<!-- machine-translated: ru -->
<!-- machine-translated: sr -->
<!-- machine-translated: tr -->
<!-- machine-translated: zh_CN -->
Przy `LANG=fr_FR.UTF-8` polecenie `subedit-cli --help` zaczyna się od „Lire,
inspecter et recaler des fichiers de sous-titres.” Skrypt, który czyta słowa wyniku tekstowego,
ustawia `LANGUAGE=en` (lub `LC_ALL=C`) — albo czyta `--format json`, które nigdy się nie zmienia.

**Co nigdy nie podąża za językiem** — to, co czyta skrypt:

| Nigdy nie tłumaczone | Dlaczego |
| :--------------- | :-- |
| wynik `--format json`: klucze, wartości oraz `message` i `detail` niepowodzeń i ostrzeżeń | te same bajty w każdym języku; słowa są angielskie |
| kody wyjścia | cztery z rozdziału [Kody wyjścia](#kody-wyjścia), w każdym języku |
| wpisywane nazwy: podpolecenia, opcje, ich wartości (`off`, `srt`), metazmienne (`FILE`, `TEXT`, `OPTIONS`) | wiersz poleceń napisany raz działa w każdym języku |
| nazwy wzorców poprawek, które przyjmują `correct --enable` i `--disable` | wzorzec jest wywoływany angielską nazwą, którą zawiera jego plik |
| nazwy formatów i kodowań | format nazywa się swoją nazwą |
| własne komunikaty diagnostyczne CLI11 — brak wymaganego argumentu, nieznana opcja, wartość, której opcja nie przyjmuje — oraz jego `Run with --help for more information.` | sformułowane wewnątrz parsera, na którym zbudowano wiersz poleceń; pozostają angielskie |

Żądanie angielskiego, bez względu na środowisko:

<!-- example: LC_ALL=C LANGUAGE=fr subedit-cli shift --output-dir out a.srt; echo "code=$?" -->
```console
$ LC_ALL=C LANGUAGE=fr subedit-cli shift --output-dir out a.srt; echo "code=$?"
shift needs --by, or --to-grid to work the amount out from the positions
code=1
```

## Opcje globalne

**Zapisuje się je przed nazwą podpolecenia i tylko tam.**
Poprawna postać to `subedit-cli --encoding cp1252 inspect film.srt`;
`subedit-cli inspect --encoding cp1252 film.srt` jest odrzucane z komunikatem
`The following argument was not expected: --encoding`. To samo dotyczy
`--version`, `-v` i `--quiet`.

**`--help` jest wyjątkiem, choć nie do końca**:
`subedit-cli inspect --help` jest przyjmowane, ponieważ każde podpolecenie ma własną opcję, i wypisuje pomoc
podpolecenia, a nie narzędzia.

| Opcja | Skutek |
| :----- | :---- |
| `-h`, `--help` | wypisuje pomoc i kończy działanie; pomoc podpolecenia, jeśli jedno jest nazwane |
| `--version` | wypisuje `subedit <version>`, biblioteki, z którymi narzędzie działa, i zainstalowane języki, po czym kończy działanie — zobacz [`--version`](#--version) |
| `-q`, `--quiet` | poziom 0 — żadnego komentarza |
| `-v`, `-vv`, `-vvv` | poziomy od 1 do 3; poziom 1 jest domyślny |
| `--encoding NAME` | czyta pliki w tym kodowaniu zamiast je zgadywać |
| `--list-encodings` | wypisuje kodowania, które narzędzie potrafi czytać i zapisywać, jedno w wierszu, i **kończy działanie** (kod `0`) — zobacz [`--list-encodings`](#--list-encodings) |
| `--format text\|json` | postać wyniku na standardowym wyjściu: tekst albo jeden obiekt JSON na plik — zobacz [Wynik czytelny dla skryptu](#wynik-czytelny-dla-skryptu) |
| `--config FILE` | czyta ustawienia operacji z `FILE` zamiast z pliku ustawień użytkownika — zobacz [Plik ustawień](#plik-ustawień) |
| `--no-config` | nie czyta żadnego pliku ustawień: każde ustawienie operacji niepodane w wierszu przyjmuje wbudowaną wartość domyślną — zobacz [Plik ustawień](#plik-ustawień) |

Opcje `--quiet` i `-v` w tym samym wywołaniu są odrzucane: dwa przeciwstawne zamiary
nie są rozstrzygane na korzyść ostatnio zapisanego.

Opcje `--config` i `--no-config` w tym samym wywołaniu również są odrzucane, z komunikatem
`--config excludes --no-config`, podobnie jak `--config` z pustą nazwą, z komunikatem
`--config needs a file name`: pusta nazwa nie wskazuje żadnego pliku, a plik użytkownika nie jest czytany
w jej zastępstwie.

### Plik ustawień

**Podpolecenia [`adjust`](adjust.md) i [`correct`](correct.md) domyślnie czytają ustawienia operacji z
pliku ustawień** — wartości, które `Adjust Durations…` i `Correct Texts…` zapamiętują w
oknie albo które zapisało [`config set`](config.md). Żadne inne podpolecenie go nie czyta. Plik to:

```
$XDG_CONFIG_HOME/subedit/settings.conf
```

lub `~/.config/subedit/settings.conf`, gdy `XDG_CONFIG_HOME` nie jest ustawiona, jest pusta albo nie jest ścieżką
bezwzględną — plik okna, opisany w [jego podręczniku](../subedit-gui/preferences.md#plik).

**Pierwszeństwo, od najsilniejszego: opcja w wierszu poleceń, potem plik, potem wbudowana
wartość domyślna.** Opcja zawsze wygrywa: `--speed off` wyłącza szybkość ustawioną w pliku. Czytany jest tylko
**blok operacji** pliku; preferencje wyświetlania (geometria, kolumny, motyw…)
nigdy. [`config list`](config.md#config-list) wymienia klucze i to, które z nich czyta wiersz
poleceń: dziewięć kluczy `duration-adjust.*`, `correction.human`, `correction.ocr`,
`correction.remove-blank` i `correction.line-break.max-lines`. **Każdy z nich ma opcję,
która go nadpisuje na czas jednego wywołania**, niezależnie od tego, jak ustawia go plik — nazywają je tabele
[`adjust`](adjust.md#co-dostarcza-plik-ustawień) i
[`correct`](correct.md#co-jest-wybierane-a-co-nie).

**Odczyt pliku użytkownika nigdy nie powoduje niepowodzenia polecenia.** Brakujący plik daje wartości domyślne; plik,
którego nie można odczytać, daje wartości domyślne i mówi o tym; wartość, której nie można odczytać, zachowuje domyślną i zostaje nazwana;
nieznany klucz jest ignorowany. Plik nigdy nie jest zapisywany przez `adjust` ani `correct`.

**To, co odczytano, jest zgłaszane** na domyślnym poziomie 1, na standardowym wyjściu błędów, przed pierwszym plikiem:

| Przypadek | Wiersz |
| :--- | :--- |
| plik został odczytany | `settings: read <path>` |
| nie udało się odczytać wartości w nim | `settings: <key>: cannot read '<value>', default kept` |
| plik użytkownika nie istnieje | `settings: <path> not found, defaults used` |
| pliku nie można odczytać | `settings: <path> unreadable, defaults used` |
| `--no-config` | nic |

Z opcją [`--format json`](#wynik-czytelny-dla-skryptu) obiekt każdego pliku z `adjust` i
`correct` niesie `settings`: odczytaną ścieżkę albo `null`, gdy żadnej nie odczytano.


**`--no-config` czyni uruchomienie powtarzalnym**: ten sam wiersz daje ten sam wynik na każdej
maszynie, cokolwiek użytkownik ustawił w oknie. Używaj go w skryptach, testach i zgłoszeniach błędów — albo
`--config FILE` z plikiem, który im towarzyszy.

**`--config FILE` wskazuje inny plik**, czytany tak samo — **z tą różnicą, że musi istnieć**.
Nazwa podana w wierszu poleceń nie jest prośbą o wartości domyślne: gdy `FILE` nie istnieje,
`adjust` i `correct` zatrzymują się, zanim jakiekolwiek wejście zostanie odczytane, zapisują `settings file not found: <FILE>` na
standardowym wyjściu błędów i kończą się kodem użycia `1`. `FILE`, który istnieje, a którego nie można odczytać,
daje wartości domyślne i mówi o tym, tak jak plik użytkownika. [`config show`](config.md#config-show)
nadal pokazuje brakujący `FILE` jako `(not found)`, a [`config set`](config.md#config-set-i-config-unset) nadal
go tworzy.

<!-- example: printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine.conf adjust --dry-run a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine.conf adjust --dry-run a.srt; echo "code=$?"
settings file not found: mine.conf
code=1
```

### `--version`

**Wersja programu, sama w pierwszym wierszu** — `subedit <version>`, czyli to, czym był wynik aż do
0.15.38 i co czyta skrypt. Potem **jeden wiersz na bibliotekę, z którą narzędzie działa**, potem zainstalowane
języki interfejsu, na standardowym wyjściu; narzędzie kończy działanie z kodem `0`. Podane dodatkowo podpolecenie
nie jest uruchamiane, a `-q` i `-v` niczego nie zmieniają.

<!-- example: subedit-cli --version | head -n 1 -->
```console
$ subedit-cli --version | head -n 1
subedit 1.0.11
```

Całość, na maszynie z ICU 74.2 i Enchant 2.3.3 — liczby są liczbami maszyny:

```console
$ subedit-cli --version
subedit <version>
  ICU      74.2 (built with 74.2)
  Enchant  2.3.3 (built with 2.3.3)
  CLI11    2.4.1 (built with 2.4.1)
  Languages: en, fr
```

| Biblioteka | Do czego narzędzie jej używa |
| :------ | :------------------------ |
| `ICU` | odczyt i zapis kodowań, ich wykrywanie, zmiana wielkości liter |
| `Enchant` | sprawdzanie pisowni w `correct` |
| `CLI11` | odczyt tego wiersza poleceń |

- **Każdy wiersz podaje dwie wersje**: tę znalezioną w czasie działania, najpierw, i tę, względem której program został
  skompilowany, w nawiasie. Różnią się, gdy biblioteka maszyny została uaktualniona po zbudowaniu
  programu — to pierwsza rzecz do podania w zgłoszeniu błędu.
- **Nagłówki Enchant nie niosą wersji**: ta w nawiasie to ta, którą zgłosił `pkg-config`, gdy
  skonfigurowano budowanie.
- **CLI11 jest skompilowane w program**, a jego nagłówki to wszystko, co z niego jest: jego dwie wersje są
  zawsze takie same.
- **Ani Qt, ani libmpv**: wiersz poleceń nie łączy się z żadną z nich i wymienia tylko to, z czym działa.
  Okno je wymienia — zobacz [jego podręcznik](../subedit-gui/invocation.md#--version).
- **`Languages`** wymienia `en`, język źródłowy, który nie potrzebuje katalogu, potem każdy katalog
  zainstalowany w `<prefix>/share/subedit/locale`, który daje się odczytać, według kodu — języki, które oferuje
  `Edit ▸ Preferences…`.
- **Co podąża za językiem**: `built with` i `Languages`. Pierwszy wiersz, nazwy,
  liczby i kody języków — nigdy.
- **Z `--format json`**, przed `--version` lub po nim, **jeden obiekt w jednym wierszu**: `schema`
  (`1`), `command` (`"version"`), `version`, `libraries` — tablica
  `{"name":"ICU","loaded":"74.2","built":"74.2"}`, w kolejności tekstu — oraz `languages`,
  tablica kodów. Nic w nim nie jest tłumaczone.

<!-- example: subedit-cli --version --format json | grep -o '"name":"[^"]*"' -->
```console
$ subedit-cli --version --format json | grep -o '"name":"[^"]*"'
"name":"ICU"
"name":"Enchant"
"name":"CLI11"
```

### `--list-encodings`

**Kodowania, które narzędzie potrafi czytać i zapisywać, jedno w wierszu, posortowane według nazwy**, na standardowym wyjściu —
potem narzędzie kończy działanie z kodem `0`, jak `--version`: to jest odpowiedź, a nie krok. Podane dodatkowo
podpolecenie nie jest uruchamiane.

<!-- example: subedit-cli --list-encodings | grep '^UTF' | head -4 -->
```console
$ subedit-cli --list-encodings | grep '^UTF' | head -4
UTF-16BE
UTF-16LE
UTF-32BE
UTF-32LE
```

- **To nazwy, które przyjmuje `--encoding`**, kanoniczne; ich aliasy (`cp1252`) są przyjmowane również,
  bez wymieniania.
- **Liczba zależy od zainstalowanego ICU**, a żadna nazwa nie jest obiecana poza tymi, które ma każde ICU
  (`UTF-8`, `UTF-16LE`, `windows-1252`…). Podręcznik nie podaje więc żadnej liczby.
- **`UTF-16` i `UTF-32` na niej nie ma**: ICU je zna, ale ich konwerter zapisuje własny znacznik,
  a model je odrzuca — zobacz niżej. Lista do wyboru nie oferuje tego, co następne pole
  by odrzuciło.
- **Z `--format json`** **jeden obiekt na kodowanie**:
  `{"schema":1,"command":"list-encodings","name":"UTF-8"}`. To nie jest raport o pliku: te
  obiekty **nie niosą ani `file`, ani `ok`**, to jedyny wyjątek od koperty opisanej w rozdziale
  [postać](#postać). Nadal jest to jeden obiekt w wierszu, po jednym na wpis.
- **Poziom komentarza niczego nie zmienia**: lista jest wynikiem, a `-q` jej nie odbiera.

### `--encoding` i czego nie może wymusić

**Dotyczy każdego podpolecenia**, ponieważ wszystkie czytają: plik, którego
kodowanie zostało błędnie odgadnięte, pozostaje błędny, cokolwiek się z nim zrobi, a opcja
dostępna tylko w `inspect` zostawiłaby niezgodność bez środka zaradczego.

**Nazwa jest nazwą ICU i wszystkie jej aliasy są przyjmowane**: `cp1252`,
`windows-1252` i `WINDOWS 1252` nazywają to samo kodowanie. Zbiór nie jest zapisany
nigdzie: to ten, który ICU maszyny potrafi konwertować, a
[`--list-encodings`](#--list-encodings) go wymienia. Nazwa, której nie potrafi skonwertować, jest
odrzucana, zanim zostanie odczytany choćby jeden plik:

```console
$ subedit-cli --encoding klingon-1 inspect film.srt
no encoding is named "klingon-1"
```

**Jeden wyjątek, i tylko jeden: kodowanie musi nazywać swoją kolejność bajtów.**
`UTF-16` i `UTF-32` to nazwy, które ICU zna i których konwerter zapisuje **własny
znacznik**. Takie kodowanie nie może żyć w tym modelu — znacznik przestaje
być wyborem, `--no-bom` byłoby przyjęte i nieposłuszne — więc oba są
odrzucane, zarówno przy odczycie, jak i przy zapisie:

```console
$ subedit-cli --encoding UTF-16 inspect film.srt
"UTF-16" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do
```

`UTF-16LE` i `UTF-16BE` czytają i zapisują te same bajty, znacznik łącznie; to, co
dodaje dokładna nazwa, to wiedza, który z nich.

**Znacznik kolejności bajtów wygrywa z opcją.** To jedyna rzecz, jaką plik napisów
deklaruje o sobie, a czytanie go inaczej, niż deklaruje, byłoby
posłuszeństwem wobec wywołującego wbrew plikowi. Niezgodność jest **zgłaszana** jako
komunikat diagnostyczny:

```console
$ subedit-cli -vvv --encoding cp1252 inspect avec-bom.srt
avec-bom.srt: a byte order mark that contradicts the encoding asked for ("UTF-16LE"), settled by the reader
```

Tak robi Gaupol — rozpoczyna odczyt od nowa z kodowaniem
znacznika — ale nie mówi o tym; tutaj różnica między tym, o co poproszono, a tym, co
zrobiono, nie jest milczeniem.

## Podpolecenia

| Podpolecenie | Co robi | Czy zapisuje? |
| :------------ | :-------------- | :----------- |
| [`inspect`](inspect.md) | zgłasza, z czego składa się plik, niczego nie zmieniając | nie |
| [`convert`](convert.md) | zapisuje plik w innym formacie lub innej postaci | tak |
| [`shift`](shift.md) | przesuwa każdą pozycję o tę samą wartość | tak |
| [`transform`](transform.md) | koryguje każdą pozycję na podstawie dwóch punktów, o których wiadomo, że są prawidłowe | tak |
| [`framerate`](framerate.md) | synchronizuje plik od nowa z jednej liczby klatek na sekundę na inną | tak |
| [`snap`](snap.md) | przesuwa każdy znacznik czasu na najbliższą klatkę | tak |
| [`hearing-impaired`](hearing-impaired.md) | usuwa opisy dla niesłyszących | tak |
| [`adjust`](adjust.md) | dostosowuje czasy trwania do szybkości czytania i do limitów | tak |
| [`replace`](replace.md) | zastępuje tekst w napisach, nie psując znacznika | tak |
| [`case`](case.md) | ustawia teksty wielkimi literami w wyrazach, wielką literą w zdaniu, wielkimi lub małymi literami | tak |
| [`italics`](italics.md) | ustawia teksty w kursywie lub zdejmuje z nich kursywę | tak |
| [`dialogue-dashes`](dialogue-dashes.md) | dodaje myślniki dialogowe lub je usuwa | tak |
| [`sort`](sort.md) | przywraca napisom kolejność ich początków | tak |
| [`join-repeats`](join-repeats.md) | robi jeden napis z fragmentów jednego tekstu, które następują po sobie | tak |
| [`disjoint`](disjoint.md) | obcina nakładające się napisy tak, aby żaden zakres nie nakładał się już na inny | tak |
| [`repair`](repair.md) | łączy w łańcuch `join-repeats`, `sort` i `disjoint` | tak |
| [`correct`](correct.md) | poprawia teksty wzorcami Gaupola: opisy, łączenie i dzielenie wyrazów, częste błędy, wielkie litery, łamanie linii | tak |
| [`append`](append.md) | dokłada pliki za pierwszym, do **jednego** pliku | tak |
| [`split-file`](split-file.md) | tnie plik na **dwa**: początek i koniec | tak |
| [`pair`](pair.md) | zapisuje tłumaczenie w pozycjach jego pliku głównego i mówi, jak jego linie się dopasowały | tak |
| [`transcribe`](transcribe.md) | zapisuje mowę z filmu lub z pliku dźwiękowego jako napisy | tak — własne miejsce docelowe, `--output` |
| [`config`](config.md) | wymienia, pokazuje i zmienia ustawienia operacji z pliku ustawień | tylko plik ustawień |

Jest ich tu wszystkich dwadzieścia dwa; pomoc narzędzia wymienia je w tej samej kolejności.

**Tylko jedno niczego nie dotyka**, co widać w prawej kolumnie: `inspect` czyta i
zgłasza. Pozostałych dwadzieścia zapisuje, a to, co następuje, dotyczy siedemnastu z nich:
[`append`](append.md) i [`split-file`](split-file.md) mają każde własną liczbę argumentów — N plików na
**jeden** wynik, jeden plik na **dwa** — a więc własne miejsce docelowe, które opisują na swojej stronie;
[`transcribe`](transcribe.md) tworzy nowy plik z filmu lub z pliku dźwiękowego i przyjmuje wyłącznie `--output`.
[`config`](config.md) nie zapisuje żadnego pliku napisów: czyta i zmienia plik ustawień, a
nic z tego, co następuje, go nie dotyczy.

## Miejsce docelowe

Siedemnaście podpoleceń, które zapisują — [`convert`](convert.md),
[`shift`](shift.md), [`transform`](transform.md), [`framerate`](framerate.md),
[`snap`](snap.md), [`hearing-impaired`](hearing-impaired.md), [`adjust`](adjust.md),
[`replace`](replace.md), [`case`](case.md), [`italics`](italics.md),
[`dialogue-dashes`](dialogue-dashes.md), [`sort`](sort.md), [`join-repeats`](join-repeats.md),
[`disjoint`](disjoint.md), [`repair`](repair.md), [`correct`](correct.md), [`pair`](pair.md) — przyjmuje swoje miejsce docelowe w ten sam sposób. ([`append`](append.md) i
[`split-file`](split-file.md) mają własne, opisane na ich stronie.) [`inspect`](inspect.md) nie zapisuje żadnego pliku i
nie przyjmuje żadnej z tych opcji.

**Miejsce docelowe, które już istnieje, jest nadpisywane**, bez pytania i bez opcji,
która by to wyłączyła: zapis jest atomowy — plik nigdy nie jest zapisany do połowy, a
przerwany zapis zostawia stary nienaruszony —, ale poprzednia zawartość
ginie. To nie jest zachowana wada: to reguła, a ponowne uruchomienie polecenia
na tym samym katalogu wyjściowym uruchamia je ponownie na samym sobie.

**Nic nigdy nie jest zapisywane bez jawnego miejsca docelowego.** Trzy sposoby
jego podania wykluczają się wzajemnie i jeden jest potrzebny: bez niego nic nie jest zapisywane, a kod
wyjścia to `1` — z wyjątkiem opcji [`--dry-run`](#podgląd-przed-zapisem), która nic nie zapisuje
i dlatego żadnego nie wymaga.

| Opcja | Dokąd trafia wynik |
| :----- | :---------------- |
| `--output FILE` | do tego pliku, pod dokładnie tą nazwą; jeden plik wejściowy |
| `--output-dir DIR` | do tego katalogu, pod nazwą podstawową pliku wejściowego |
| `--in-place` | na plik wejściowy, przez zapis atomowy |

Narzędzie, które nadpisuje swoje wejście, bo nic mu nie powiedziano, to narzędzie, z którego
przestaje się korzystać za drugim razem. Odmowa kosztuje jeden wiersz i ratuje
plik.

Opcja `--output` z kilkoma plikami wejściowymi jest odrzucana: ostatni plik zostałby zapisany na
poprzednie. Przy partii jedyną z trzech opcji, która ma
sens, jest `--output-dir`, obok `--in-place`.

**Katalog wyjściowy jest tworzony**, jeśli go brakuje, razem z katalogami nadrzędnymi — dla
`--output-dir` i dla katalogu, do którego zapisuje `--output`. Dzieje się to po zatwierdzeniu
poprawności wiersza poleceń i **przed pierwszym plikiem**. Jeśli nie można go utworzyć,
jest to zgłaszane raz, żaden plik nie jest odczytywany, a kod wyjścia to
`2`.

**Partia jest oceniana jako całość przed zapisem.** Miejsce docelowe każdego pliku wejściowego
jest obliczane najpierw i wynikają z tego dwie odmowy — błędy użycia, kod
`1`, **żaden plik nie zostaje zapisany** i żaden katalog nie zostaje utworzony:

- **dwa pliki wejściowe, które trafiłyby do tego samego pliku** — `a/film.srt` i
  `b/film.srt` z `--output-dir` albo `a/film.srt` i `b/film.vtt`, oba
  konwertowane do WebVTT, którego końcowe miejsce docelowe zmieniło rozszerzenie. Komunikat
  nazywa miejsce docelowe i oba pliki wejściowe; zmiana nazwy drugiego lub
  pozwolenie, by wygrał ostatni, zgubiłoby plik bez słowa, a odmowa
  kosztuje jeden wiersz;
- **miejsce docelowe będące jednym z plików wejściowych**, bez `--in-place` — także
  przez `--output-dir .`, przez `--output` lub przez dowiązanie symboliczne. Zapisanie na własne
  wejście to gest, który należy nazwać.

Dwie ścieżki nazywają ten sam plik, gdy tak twierdzi system, a nie gdy
są zapisane tak samo: `in/../in/film.srt` i `in/film.srt` to jeden plik.

**Rozszerzenie podąża za zapisywanym formatem.** Zmienia się tylko w
[`convert`](convert.md), jedynym podpoleceniu, które zmienia format;
pozostałe zachowują format odczytanego pliku, a więc jego rozszerzenie.

| Co to wywołuje | Komunikat |
| :------------------ | :------ |
| brak miejsca docelowego, bez `--dry-run` | `no destination given: use --output, --output-dir or --in-place` |
| dwa miejsca docelowe | `--output, --output-dir and --in-place exclude one another` |
| `--output` przy partii | `--output names one file but several were given: use --output-dir instead` |
| dwa pliki wejściowe, jedno miejsce docelowe | `<destination>: would be written by both <input> and <input>` |
| plik wejściowy zostałby nadpisany | `<destination>: would be written over the input <input>` — to samo zdanie dla [`append`](append.md) i [`split-file`](split-file.md) |
| nie można utworzyć katalogu wyjściowego | `<directory>: cannot be created: permission denied` lub `: cannot be created` |
| nie można zapisać miejsca docelowego | `<path>: <destination>: cannot be written: permission denied` lub `: cannot be written` |
| znak, którego brakuje w zapisywanym kodowaniu | `<path>: holds a character the chosen encoding cannot write` |

**Ostatni zasługuje na zdanie.** Plik jest zapisywany z powrotem w **kodowaniu, w którym
został odczytany**, tak samo jak z jego końcami linii i znacznikiem. Jeśli
zmiana wprowadziła do niego znak, którego to kodowanie nie zna — na przykład
`ł` w Latin-1 —, zapis kończy się niepowodzeniem i **nic nie jest zapisywane**. Zastąpienie go
znakiem `?` oznaczałoby utratę tekstu na oczach osoby, która dopiero co go wpisała.

## Podgląd przed zapisem

**`--dry-run`** jest przyjmowane przez każde z szesnastu podpoleceń, które zapisują, i znaczy to samo
wszędzie: **odczytać, obliczyć, zgłosić, nie zapisywać żadnego pliku**. Dodaje się je do
wiersza, który miało się uruchomić; zastosowanie to ponowne uruchomienie tego samego wiersza bez niego. Obliczenie jest
deterministyczne: to, co pokazało `--dry-run`, jest tym, co zapisze następne uruchomienie, o ile
plik się nie zmienił.

- **Nie zapisuje żadnego pliku i nie tworzy żadnego katalogu**, także z `--in-place`: plik
  wejściowy pozostaje taki, jaki był.
- **Nie wymaga miejsca docelowego.** Można je podać i **jest wtedy sprawdzane tak jak bez
  `--dry-run`**: dwa pliki wejściowe dla tego samego miejsca docelowego, plik wejściowy, który zostałby nadpisany,
  `--output` przy partii, dwa sposoby wskazania miejsca — wiersz poleceń odrzucony bez `--dry-run`
  jest odrzucany także z nim, tym samym zdaniem i tym samym kodem `1`.
- **Kod wyjścia jest taki jak przy prawdziwym uruchomieniu**: `0`, gdy wszystko zostało obliczone, `2` lub `3`,
  gdy nie udało się odczytać lub przetworzyć plików, `1` przy błędzie użycia. Znak, którego
  kodowanie pliku nie potrafiło zapisać, kończy próbne uruchomienie niepowodzeniem tak samo jak prawdziwe.
  **To, że zmiany istnieją, nie zmienia kodu**: cztery kody mówią, czy narzędzie
  się powiodło, a nie czy plik jest zgodny.
- **Komentarz o tym mówi.** Tam, gdzie prawdziwe uruchomienie zapisuje `-> <destination>`, próbne
  zapisuje `(dry run, nothing written)`.

<!-- example: printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by 1 --dry-run a.srt; ls -- *.srt -->
```console
$ printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by 1 --dry-run a.srt; ls -- *.srt
a.srt: 1 subtitle shifted by 1.000 s (dry run, nothing written)
a.srt
```

**W podpoleceniu, które zmienia teksty** —
[`hearing-impaired`](hearing-impaired.md), [`replace`](replace.md), [`case`](case.md),
[`italics`](italics.md), [`dialogue-dashes`](dialogue-dashes.md) i [`correct`](correct.md) —, **standardowe wyjście niesie proponowane
zmiany**. Jeden blok na zmieniony napis, w kolejności pliku:

```
<path>: subtitle <number>
- <the text before, line by line>
+ <the text after, line by line>
```

- **numer** to ten, który napis ma w odczytanym pliku, liczony od jedynki;
- każda linia tekstu przed zmianą ma z przodu `- `, każda linia tekstu po zmianie `+ `: tekst
  wielowierszowy czyta się tak, jak jest, a linia, która sama zaczyna się od myślnika
  dialogowego, daje `- - `;
- **napis, który operacja by usunęła**, mówi o tym w pierwszej linii,
  `<path>: subtitle <number> (removed)`, i nie ma linii `+`;
- napis tłumaczenia, z opcją `--document translation`, zapowiada się jako `(translation)` w tym samym
  miejscu — zobacz [Tłumaczenie](#tłumaczenie);
- pierwsza linia każdego bloku zaczyna się od ścieżki, tak aby `grep '^film.srt: '`
  znajdowało bloki jednego pliku w partii;
- **żadnego bloku**, gdy nic by się nie zmieniło.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\n- [Grincement]\n- Qui est là ?\n\n' > mentions.srt; subedit-cli hearing-impaired --dry-run mentions.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\n- [Grincement]\n- Qui est là ?\n\n' > mentions.srt; subedit-cli hearing-impaired --dry-run mentions.srt
mentions.srt: 2 subtitles cleaned, 1 removed (dry run, nothing written)
mentions.srt: subtitle 1 (removed)
- [Bruit de pas]
mentions.srt: subtitle 2
- Attends [il tousse] Marie.
+ Attends Marie.
mentions.srt: subtitle 3
- - [Grincement]
- - Qui est là ?
+ Qui est là ?
```

**Lista jest taka sama w JSON**: obiekt pliku niesie tablicę `changes`, zobacz
[postać](#postać), z `after` ustawionym na `null` dla usuniętego napisu. **Jest ona
obliczana tylko wtedy, gdy żąda tego `--dry-run` lub `--format json`**: partia tysięcy
napisów nie buduje jej na darmo.

W **pozostałych podpoleceniach** — [`convert`](convert.md), [`shift`](shift.md),
[`transform`](transform.md), [`framerate`](framerate.md), [`snap`](snap.md),
[`adjust`](adjust.md), [`sort`](sort.md), [`append`](append.md), [`split-file`](split-file.md), [`pair`](pair.md) — standardowe
wyjście niesie to, co niosłoby wyjście prawdziwego uruchomienia, czyli w tekście nic; w
JSON obiekt każdego pliku z `"dry_run":true`, `"destination":null` i tymi samymi
`counts`.

## Dwa wyjścia, dwie role

| Wyjście | Co niesie |
| :----- | :--------------- |
| standardowe | **wynik i tylko wynik** — raport z `inspect`, proponowane zmiany tekstu z [`--dry-run`](#podgląd-przed-zapisem); z `--format json` jeden obiekt na plik — jeden na uruchomienie dla [`append`](append.md), [`split-file`](split-file.md) i [`pair`](pair.md) |
| błędów | **cała reszta** — komentarz, ostrzeżenia, błędy |

To ten podział pozwala przekierować wynik bez dołączania do niego opowieści:

```console
$ subedit-cli inspect *.srt > rapport.txt
```

Raport trafia do pliku, komentarz zostaje na ekranie.

## Wynik czytelny dla skryptu

**`--format json`** zastępuje tekst standardowego wyjścia **jednym obiektem JSON na
plik wejściowy**, w formacie JSON Lines: jeden zwarty obiekt w wierszu, w UTF-8 bez
znacznika, koniec linii to `\n`. `text` jest domyślne i się nie zmienia. To
opcja globalna, więc zapisuje się ją przed podpoleceniem; wartość inna niż
`text` i `json` to błąd użycia, kod `1`.

<!-- example: printf '1\n00:00:10,000 --> 00:00:12,000\nUn.\n\n' > a.srt; subedit-cli --format json shift --by 1 --output-dir sortie a.srt absent.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:10,000 --> 00:00:12,000\nUn.\n\n' > a.srt; subedit-cli --format json shift --by 1 --output-dir sortie a.srt absent.srt; echo "code=$?"
a.srt: 1 subtitle shifted by 1.000 s -> sortie/a.srt
{"schema":1,"command":"shift","file":"a.srt","ok":true,"dry_run":false,"destination":"sortie/a.srt","counts":{"subtitles":1,"shifted_by_ms":1000},"warnings":[]}
absent.srt: does not exist
{"schema":1,"command":"shift","file":"absent.srt","ok":false,"error":{"kind":"not-found","message":"absent.srt: does not exist"}}
1 of 2 files shifted, 1 failed
code=3
```

**Jeden obiekt na plik wejściowy, nigdy zero, nigdy dwa**, niezależnie od tego, czy plik się powiódł, w
kolejności, w jakiej podano pliki — lub w jakiej je znaleziono, z opcją [`--recursive`](lots.md). Jedno
zlicza wiersze, drugie znajduje swoje pliki. **Trzy podpolecenia są wyjątkiem, bo ich
liczba argumentów nie jest liczbą partii: [`append`](append.md) (N plików wejściowych, jedno wyjście), [`split-file`](split-file.md)
(jeden plik wejściowy, dwa wyjścia) i [`pair`](pair.md) (dwa pliki, jedno wyjście) zapisują **jeden obiekt na całe
uruchomienie**, który nazywa plik wskazany na ich stronie.** **Podpolecenia, które przepisują, zapisują więc także
na standardowe wyjście**, czego ich umowa w tekście nie robiła: ich
plik pozostaje ich wynikiem, a obiekt jest jego raportem.

**Komentarz się nie zmienia.** Opcje `-q`, `-v`, `-vv` i `-vvv` działają na wyjście
błędów, w tekście, jak bez `--format`; **standardowe wyjście się z nimi nie zmienia**.
Komunikaty diagnostyczne odczytu, które tekst zachowuje na poziomie 3, są danymi i są
**zawsze** w `warnings`. Błąd użycia nie zapisuje obiektu: jest wykrywany
przed dotknięciem jakiegokolwiek pliku, więc nie ma do czego go dołączyć; trafia na wyjście
błędów z kodem `1`, a standardowe wyjście pozostaje puste.

### Postać

| Klucz | Zawartość | Obecny |
| :-- | :------ | :------- |
| `schema` | liczba całkowita, wersja tej postaci — `1` | zawsze |
| `command` | nazwa podpolecenia | zawsze |
| `file` | ścieżka **jak podana** lub jak ułożył ją przegląd katalogu | zawsze |
| `ok` | wartość logiczna | zawsze |
| `error` | `{"kind", "message"}` | jeśli `ok` jest fałszem |
| `dry_run` | wartość logiczna: prawda dla [`--dry-run`](#podgląd-przed-zapisem) | jeśli `ok` jest prawdą, w podpoleceniu, które zapisuje |
| `destination` | zapisana ścieżka lub `null`, gdy `dry_run` jest prawdą: nic nie zapisano | tak samo |
| `counts` | obiekt **liczb całkowitych**, właściwy dla podpolecenia | tak samo |
| `changes` | tablica `{"subtitle", "document", "before", "after"}`: zmienione teksty, zobacz niżej | w podpoleceniu, które zmienia teksty |
| `warnings` | tablica `{"kind", "line"?, "detail"?, "settled"?}` | jeśli `ok` jest prawdą |
| `alignment` | jak nałożono tłumaczenie, zobacz [Tłumaczenie](#tłumaczenie) | z `-t` oraz dla `pair` |
| `constraints` | użyte ustawienia | `adjust` |
| `settings` | odczytany plik ustawień lub `null`, gdy żadnego nie odczytano — zobacz [Plik ustawień](#plik-ustawień) | `adjust`, `correct`, jeśli `ok` jest prawdą |
| `inputs`, `tail` | jeden element na dołączony plik; ścieżka końca | `append`; `split-file` |

`line` nie występuje w komunikacie dotyczącym całego pliku; `detail` nie występuje, gdy
nie ma nic do dodania; `settled` ma wartość `true`, gdy odczyt coś **rozstrzygnął** — na przykład
odtworzył numerację — i nie występuje, gdy zostawił rzeczy tak, jak są.

**`changes`** wylicza, w kolejności pliku, każdy napis, którego tekst się zmienia:
`subtitle` to jego numer w odczytanym pliku, liczony od jedynki; `document` to `"main"`
lub `"translation"`; `before` to tekst przed zmianą; `after` tekst po zmianie, lub **`null`, gdy
napis jest usuwany**. Podziały linii tekstu są w nim zapisane jako `\n`.
Jest zapisywane przy prawdziwym uruchomieniu tak samo jak przy [`--dry-run`](#podgląd-przed-zapisem) i jest **puste
(`[]`), gdy nic się nie zmienia**: jego obecność mówi, że podpolecenie wylicza swoje zmiany,
jego brak, że ich nie ma.

**Żadnej liczby zmiennoprzecinkowej, nigdzie.** Pozycja lub czas trwania to liczba całkowita
**milisekund** (`…_ms`); licznik to liczba całkowita; liczba klatek na sekundę to **łańcuch znaków**
(`"25"`, `"24000/1001"`), bo jest dokładna, a wymierna liczba zmiennoprzecinkowa nie potrafi
jej zawrzeć; koncentracja siatki to liczba całkowita w **tysięcznych** — `1000` to
doskonała siatka. Dwa wyniki porównuje się wtedy bajt po bajcie, bez zaokrągleń.

**Ścieżka, która nie jest w UTF-8** — Linux na to pozwala — nie może być przedstawiona w JSON: jest
zapisywana z U+FFFD w miejscu nieprawidłowych bajtów, a obiekt niesie ostrzeżenie
`path-not-utf8`. Wierność takiej ścieżki nie jest obiecywana i jest to powiedziane.

### Co każde podpolecenie umieszcza w `counts`

| Podpolecenie | Klucze `counts` |
| :------------ | :--------------- |
| [`shift`](shift.md) | `subtitles`, `shifted_by_ms` (ze znakiem) |
| [`transform`](transform.md) | `subtitles` |
| [`framerate`](framerate.md) | `subtitles` |
| [`snap`](snap.md) | `subtitles`, `moved` (przesunięte pozycje), `furthest_ms` (największe przesunięcie) |
| [`hearing-impaired`](hearing-impaired.md) | `cleaned` (przepisane teksty), `removed` (usunięte napisy) |
| [`replace`](replace.md) | `replaced` (zastąpione dopasowania w zmienionych napisach), `matched` (znalezione dopasowania) |
| [`case`](case.md), [`italics`](italics.md), [`dialogue-dashes`](dialogue-dashes.md) | `changed` (napisy, których tekst się zmienił) |
| [`correct`](correct.md) | `corrected` (zmienione teksty), `removed` (usunięte napisy) — nigdy dopasowania |
| [`sort`](sort.md) | `subtitles`, `moved` (miejsca, w których zmienił się napis) |
| [`adjust`](adjust.md) | `subtitles` (napisy, których dotyczy operacja), `adjusted` (te, których koniec się przesunął), potem grupa `sacrificed`: `speed`, `minimum`, `gap`; a poza `counts` — `constraints`, czyli użyte ustawienia |
| [`convert`](convert.md) | `subtitles`, potem to, co konwersja utraciła: `lost_ends` i `lost_header` (0 lub 1), `joined_lines`, `lost_tags`, `lost_fields`, `furthest_ms` |
| [`append`](append.md) | `files`, `subtitles`, `appended`, potem `lost_ends`, `lost_header`, `joined_lines`, `lost_tags`, `lost_fields`, `furthest_ms` zsumowane; a poza `counts` — `inputs`, jeden element na dołączony plik |
| [`split-file`](split-file.md) | `subtitles`, `head`, `tail`; a poza `counts` — `tail`, czyli ścieżka końca (`destination` to początek) |
| [`pair`](pair.md) | `subtitles` (napisy zapisanego pliku); a poza `counts` — `alignment` |
| [`inspect`](inspect.md) | brak `counts`: opis pliku, zobacz jego stronę |

### Identyfikatory błędów

`error.kind` to jeden z tych; **czytelnik traktuje nieznany identyfikator jako
„inny”**.

| `kind` | Czym jest |
| :----- | :----------- |
| `not-found`, `permission-denied`, `io` | system odmówił odczytu — lub zapisu — pliku |
| `unknown-format`, `no-subtitle-found`, `undecodable` | plik został odczytany i nie daje się odczytać |
| `unencodable` | znak, którego wybrane kodowanie nie potrafi zapisać |
| `no-byte-order-mark`, `no-frame-rate` | to, czego `convert` nie może zapisać bez wskazówki |
| `before-the-origin`, `beyond-the-end`, `no-transform`, `no-grid` | operacja, której nie da się zastosować do tego pliku |
| `range-out-of-bounds` | `--range`, którego ten plik nie zawiera |
| `no-style` | format, który nie zapisuje stylu (TMPlayer, LRC), dla `italics` |
| `refused` | każda inna odmowa operacji |

### Co jest obiecane, a co nie

**Obiecane, dopóki `schema` wynosi `1`**: nazwy kluczy, ich typ i jednostka; znaczenie
`ok`, `file` i `destination`; **jeden obiekt na plik wejściowy, w kolejności plików wejściowych** — jeden na uruchomienie dla trzech podpoleceń z własną liczbą argumentów;
istniejące identyfikatory `kind`; determinizm — te same dane wejściowe, te same argumenty,
te same bajty.

**Nieobiecane**: **tekst komunikatów** (`message`, `detail`), angielski dla człowieka,
poprawiany, gdy trzeba — skrypt, który go porównuje, jest zepsuty z góry; **kolejność
kluczy** w obiekcie, ustalona, ale niegwarantowana przez JSON; obecność **kluczy jeszcze nie
dodanych**.

**Czytelnik ignoruje więc nieznany klucz** i traktuje nieznane `kind` jako „inne”:
dodanie klucza lub identyfikatora nie zmienia `schema`. Usunięcie lub zmiana nazwy klucza,
zmiana typu lub jednostki, usunięcie identyfikatora zwiększa ją — to zerwanie,
ogłaszane w CHANGELOG i tutaj.

## Poziomy komentarza

Każdy poziom **zawiera poprzedni**: wejście o stopień wyżej dodaje, nigdy
nie zastępuje.

| Poziom | Jak | Co niesie wyjście błędów |
| :----- | :------ | :------------------------------ |
| 0 | `--quiet` | nic, **z wyjątkiem błędów** |
| 1 | domyślnie lub `-v` | jeden wiersz na przetworzony plik i podsumowanie, gdy tylko jest ich kilka |
| 2 | `-vv` | oraz to, co rozpoznano: format, kodowanie, BOM, końce linii |
| 3 | `-vvv` | oraz ślad diagnostyczny: odczytane i zapisane bajty oraz **każdy komunikat diagnostyczny odczytu** |

**Błędy nigdy nie są wyciszane, także z `--quiet`.** Polecenie, które kończy się niepowodzeniem
po cichu, nie zostawiłoby niczego poza kodem wyjścia.

Podsumowanie pojawia się dopiero od dwóch plików: przy jednym pliku wejściowym
powtarzałoby poprzedzający je wiersz.

## Odczyt komunikatów diagnostycznych

Formaty napisów są odczytywane **najlepiej, jak się da**: wobec anomalii
czytnik się nie zatrzymuje — decyduje albo zostawia rzeczy tak, jak są, i mówi o tym. To,
na co natrafił, jest wypisywane na **poziomie 3**, jeden wiersz na komunikat, na wyjściu
błędów:

```
a.srt: 2 diagnostics while reading
a.srt: line 6: SubRip numbers that do not follow ("7"), settled by the reader
a.srt: line 9: a line that fits nowhere, left as it stands
```

Wszystkie siedemnaście podpoleceń je zgłasza, nie tylko [`inspect`](inspect.md):
plik odczytany najlepiej, jak się dało, a potem zapisany z powrotem, przeszedł przez te same decyzje, a przemilczenie ich
pozwoliłoby wierzyć, że nic się nie stało.

**Komunikat diagnostyczny nigdy nie jest niepowodzeniem.** Plik został odczytany, polecenie
się powiodło, kod wyjścia to `0`. Dlatego żyją na najbardziej
szczegółowym poziomie: niżej zagłuszałyby wiersz, który mówi, co naprawdę
zrobiono.

### Co niesie każdy wiersz

| Część | Co mówi |
| :----- | :------------- |
| `line N` | gdzie, liczone od 1, tak jak pokazuje edytor; **nieobecne** dla tego, co dotyczy całego pliku |
| zdanie | na co natrafiono, spośród dwunastu kategorii poniżej |
| `("…")` | wadliwy tekst pliku, gdy kategoria nie wystarcza; obcięty do 80 bajtów |
| zakończenie | co z tym zrobiono: `settled by the reader` lub `left as it stands` |

**Dwie kategorie nie mają linii** i są to dwie dotyczące
kodowania: bajty zostały zważone lub ich znacznik odczytany, zanim istniała choćby jedna
linia pliku. Ich wiersz zaczyna się więc od zdania, bez `line N`
z przodu — nie ma miejsca do nazwania.

**Zakończenie wiersza jest najważniejsze.** `settled by the reader` oznacza,
że czytnik zdecydował i że zapisany plik niesie jego decyzję — brakująca
numeracja zostaje odtworzona. `left as it stands` oznacza, że niczego nie ruszył, ponieważ
**decydować może tylko człowiek**: napis, który kończy się przed swoim
początkiem, pozostaje taki, jaki jest.

### Numer linii bloku

Anomalia dotycząca **całego bloku** jest zakotwiczona w jego linii
czasu, a nie w dokładnej linii, która ją wywołuje. Niespójna numeracja
zapisana w linii 5 jest więc zgłaszana w linii 6, czyli w linii czasu, która
następuje — wadliwy numer znajduje się w `("…")`. Tylko anomalie dotyczące
**linii** — nieczytelnej linii czasu, tekstu przed pierwszą z nich — są
zgłaszane w niej samej.

### Dwanaście kategorii

| Zdanie | Co to wywołuje |
| :----- | :------------------ |
| `a line that fits nowhere` | linia, która nie wchodzi do żadnego bloku |
| `a timing line that could not be read` | nieczytelna linia czasu |
| `a SubRip block without its number` | brakująca numeracja, odtwarzana przy zapisie |
| `SubRip numbers that do not follow` | numeracja, która przeskakuje |
| `text before the first timing line` | tekst przed pierwszą linią czasu |
| `a WebVTT block of an unknown kind` | nierozpoznany blok WebVTT |
| `declares an event column this tool cannot fill` | kolumna zdarzeń Sub Station Alpha, której nic tu nie wypełnia |
| `counts in frames and states no rate; it was read at` | plik w klatkach, który nie podaje liczby klatek na sekundę — użyta liczba jest szczegółem |
| `carries no end times; each one was taken from the next start` | format bez końców, TMPlayer lub LRC: **żaden koniec w tabeli nie pochodzi z pliku** |
| `more than one kind of line ending` | mieszane końce linii |
| `an encoding nothing declared` | kodowanie zostało zaproponowane przez zważenie bajtów i nie jest to UTF-8 |
| `a byte order mark that contradicts the encoding asked for` | plik niesie znacznik, a `--encoding` nazwało inny — wygrywa znacznik |

**Trzy środkowe pochodzą z fazy 9**, a dwa ostatnie z tych
trzech nie mówią o linii, lecz o całym pliku: mówią, że pokazana
wartość nie została odczytana. Wypełniona kolumna `End` w LRC, pozycja
w MicroDVD — plik nic z tego nie niósł.

**Operacja może dodać własne**, które nie jest komunikatem diagnostycznym odczytu:
[`correct`](correct.md#wzorzec-którego-nie-można-zastosować) mówi, wraz z napisem, który wzorzec
się poddał. Jest zgłaszane na poziomie 1 i niesie, w obiekcie pliku, identyfikator
`pattern-failed` — nigdy niepowodzenie pliku.

## Tłumaczenie

Plik tłumaczenia jest przetwarzany **jak każdy plik napisów**:
`subedit-cli case --to sentence film.fr.srt` zmienia go bez niczego nowego. To, co dodaje dopasowanie, a co
mówią trzy opcje, jest wąskie: **wiedza, jak linie tłumaczenia przypadają na
napisy pliku głównego** — to `inspect -t` — oraz **zmiana tłumaczenia przy zachowaniu go
nałożonego na plik główny**, z tymi samymi regułami co w oknie.

| Opcja | Gdzie | Wartość | Domyślnie |
| :----- | :---- | :----- | :----- |
| `-t`, `--translation-file` | `inspect`, `hearing-impaired`, `replace`, `case`, `italics`, `dialogue-dashes`, `correct`, `pair` | plik tłumaczenia | — |
| `--document` | sześć podpoleceń tekstowych, nie `inspect` ani `pair` | `main` lub `translation` | `main` |
| `--align-method` | jak `-t` | `position` lub `number` | `position` |

- **`position`** porównuje środek każdej linii z granicami napisów: brakująca linia nie przesuwa
  następnych. **`number`** kieruje linię *n* do napisu *n*, nie patrząc na
  pozycje. Domyślna jest pozycja, z powodu, który ma okno: brakująca linia w
  środku przesuwa całą resztę tłumaczenia dopasowanego według numeru.
- **`--document translation` wymaga `-t`, a `-t` wymaga `--document translation`** w podpoleceniu
  tekstowym — jedno bez drugiego to błąd użycia, kod `1`, ponieważ odczytany plik, którego żaden gest
  nie używa, to przeoczenie, a nie preferencja. `-t` w `inspect` nie ma `--document`:
  zgłasza. **`--align-method` bez `-t` to również błąd użycia.**
- **`-t` nazywa plik, więc ma sens tylko dla jednego pliku wejściowego**: z kilkoma plikami wejściowymi lub
  z katalogiem wziętym przez `--recursive` jest odrzucane tak jak `--output`, **zanim jakikolwiek plik zostanie
  odczytany**. Dla partii należy uruchomić jedno wywołanie na parę.
- **Plik tłumaczenia identyczny z plikiem głównym jest odrzucany** — jakkolwiek zapisana jest jego
  ścieżka: `<translation>: the file is already open as the main document`, kod `2`.
- **Zapisywany jest tylko dokument, którego dotyczy operacja**, w **jego własnej ścieżce i jego własnym formacie**: formacie,
  kodowaniu i końcach linii pliku tłumaczenia. `--output` nazywa plik
  tłumaczenia, `--output-dir` umieszcza w nim jego nazwę podstawową, `--in-place` go przepisuje; **plik główny
  nigdy nie jest dotykany**. Ścieżki raportu to ścieżki tłumaczenia.
- **Napisy, które dopasowanie wywołuje do życia** — linia, do której nic nie dotarło — są zapisywane w
  tłumaczeniu z **własnymi pozycjami**, niezależnie od metody; **napis, którego nikt
  nie przetłumaczył, jest zapisywany jako blok bez tekstu**, który odczytuje się bez anomalii.
- **`--encoding` i `--frame-rate` dotyczą obu plików.**
- Podpolecenia **pozycji** — `shift`, `transform`, `framerate`, `snap`, `adjust` — nie mają
  tych opcji: podaje się im oba pliki, jak w partii,
  `subedit-cli shift --by 2 film.srt film.fr.srt --output-dir out/`.
- **Zapisanie tłumaczenia zsynchronizowanego od nowa na pliku głównym** — jedyna rzecz, którą robi dopasowanie, a której nie robi plik
  sam — to zadanie podpolecenia [`pair`](pair.md), które nie ma ani `--document`, ani `--recursive`.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\n[Un oiseau] Bonjour.\n\n2\n00:00:03,000 --> 00:00:04,000\nAu revoir.\n\n' > film.fr.srt; subedit-cli hearing-impaired film.srt -t film.fr.srt --document translation --output propre.fr.srt; cat propre.fr.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\n[Un oiseau] Bonjour.\n\n2\n00:00:03,000 --> 00:00:04,000\nAu revoir.\n\n' > film.fr.srt; subedit-cli hearing-impaired film.srt -t film.fr.srt --document translation --output propre.fr.srt; cat propre.fr.srt
film.fr.srt: 1 subtitle cleaned, 0 removed -> propre.fr.srt
1
00:00:01,000 --> 00:00:02,000
Bonjour.

2
00:00:03,000 --> 00:00:04,000
Au revoir.
```

Z opcją `--format json` obiekt każdego pliku niesie także klucz `alignment`, który mówi, jak
tłumaczenie zostało nałożone — `{"file", "method", "attached", "born", "untranslated", "out_of_order"}`, jak
klucz `translation` w [`inspect`](inspect.md#w-formacie-json) — a **z `--dry-run`** `changes` wylicza
napisy tłumaczenia, każdy z `"document":"translation"`.

## Wiele plików

Każde podpolecenie przyjmuje kilka ścieżek, z wyjątkiem [`append`](append.md), [`split-file`](split-file.md) i
[`pair`](pair.md), których liczba argumentów jest opisana na ich stronie. Każda jest przetwarzana
niezależnie: niepowodzenie jednej nie przerywa pozostałych, a niepowodzenia
są zgłaszane z nazwą pliku i powodem.

**Katalog zastępuje zawarte w nim pliki, z opcją `--recursive` (`-r`)**:
bez tej opcji podanie go to błąd użycia, kod `1`, i nic nie jest
przetwarzane — odmowa jest lepsza niż nieprzetworzenie niczego i zwrócenie `0`:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?" -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?"
films: is a directory: use --recursive
code=1
```

Przegląd, drzewo zachowywane w `--output-dir` i to, co jest pomijane,
opisuje [Przetwarzanie drzewa katalogów](lots.md).

## Strony podręcznika man

Instalacja umieszcza **jedną stronę podręcznika na podpolecenie** obok `subedit-cli(1)`:
`subedit-cli-shift(1)`, `subedit-cli-convert(1)` i pozostałe, po jednej na każdy wiersz
[tabeli podpoleceń](#podpolecenia). `subedit-cli(1)` zawiera ten rozdział — opcje globalne,
wyjścia, kody wyjścia — a każda pozostała strona zawiera rozdział swojego podpolecenia, wraz z jego
przykładami. Strona okna, `subedit-gui(1)`, jest obok nich.

```console
$ man subedit-cli
$ man subedit-cli-shift
```

**Strony nie są pisane osobno od tego podręcznika**: są z niego generowane podczas
budowania programu, więc mówią to, co on, i nie mogą się od niego rozjechać. Sekcja FILES każdej
strony nazywa katalog, w którym zainstalowano podręcznik.

| Strony | Gdzie są zainstalowane |
| :---- | :----------------------- |
| angielskie | `share/man/man1` |
| każdy zainstalowany język — kataloński, czeski, niemiecki, hiszpański, fiński, francuski, galicyjski, hebrajski, węgierski, interlingue, islandzki, włoski, niderlandzki, polski, portugalski, brazylijski portugalski, rosyjski, serbski, turecki, chiński uproszczony | `share/man/<language>/man1` — `share/man/fr/man1` |

**`man` wybiera język na podstawie ustawień regionalnych**, tak jak program: przy francuskich ustawieniach
`man subedit-cli` pokazuje stronę francuską, a język, którego stron nie zainstalowano, dostaje
angielskie. Opcja `-L` żąda języka bez względu na ustawienia regionalne:

```console
$ man -L fr subedit-cli-shift
```

## Kody wyjścia

| Kod | Znaczenie |
| :--- | :------------ |
| `0` | wszystko się powiodło |
| `1` | błąd użycia — nieznana opcja, nieprawidłowa wartość, zabroniona kombinacja |
| `2` | żadnego pliku nie udało się przetworzyć |
| `3` | niektóre pliki zostały przetworzone, inne nie |

`2` i `3` są rozróżniane po to, by skrypt wiedział, jak postąpić, bez ponownego czytania wyniku:
„nic nie zadziałało” i „jednego brakuje” nie wymagają takiej samej reakcji.

Błąd użycia jest wykrywany **przed jakimkolwiek przetwarzaniem**: nigdy nie zostawia
partii przetworzonej do połowy.

## Błędy

| Co to wywołuje | Co jest zapisywane na wyjściu błędów |
| :------------------ | :--------------------------------------- |
| nieznana opcja lub podpolecenie | błędna nazwa i odsyłacz do `--help` |
| `--quiet` z `-v` | `--quiet and -v ask for opposite things; give one or the other` |
| `--config` z `--no-config` | `--config excludes --no-config` |
| `--config` z pustą nazwą | `--config needs a file name` |
| brakujący plik | `<path>: does not exist` |
| nieczytelny plik | `<path>: cannot be opened: permission denied` |
| kodowanie, które zapisuje własny znacznik | `"<name>" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do` |
| bajty, których nie da się zdekodować | `<path>: cannot be decoded in the chosen encoding` |
| format nierozpoznany | `<path>: is in no format this tool knows` |
| nic, co przypominałoby napis | `<path>: holds nothing recognisable as a subtitle` |
| `--document translation` bez `-t` | `--document translation needs the translation file: use -t` |
| `-t` bez `--document translation` | `-t names a translation to change: use --document translation` |
| `--align-method` bez `-t` | `--align-method needs a translation file: use -t` |
| `-t` z kilkoma plikami wejściowymi lub katalogiem | `-t names one file but several inputs were given: use one invocation per pair` |
| tłumaczenie identyczne z plikiem głównym | `<translation>: the file is already open as the main document` |
