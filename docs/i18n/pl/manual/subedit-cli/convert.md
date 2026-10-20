# `convert`

<!-- machine-translated -->
> Ta strona jest tłumaczeniem maszynowym i nie została jeszcze sprawdzona przez osobę władającą tym językiem.

```
subedit-cli convert --to <format>
                    [--line-endings unix|windows|mac] [--to-encoding NAME]
                    [--bom | --no-bom] [--sort] [--disjoint]
                    [--join-repeats] [--repair]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    <file>...
```

Zapisuje każdy plik w żądanym formacie i w żądanej postaci. **Nigdy nie zmienia
pliku wejściowego**, chyba że wyraźnie poprosi o to `--in-place`.

<!-- example: subedit-cli convert --help -->
```console
$ subedit-cli convert --help
Write a subtitle file out in another format or shape
Usage: subedit-cli convert [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to convert

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --to TEXT:{srt,vtt,subviewer2,ssa,ass,microdvd,mpl2,tmplayer,lrc} REQUIRED
                              Format to write
  --line-endings TEXT:{unix,windows,mac}
                              Line endings to write; the source's by default
  --to-encoding NAME          Encoding to write; the source's by default
  --frame-rate RATE           Frame rate of a file counted in frames: 25, 23.976
  --bom                       Write a byte order mark
  --no-bom                    Write no byte order mark
  --sort                      Put the subtitles in order of their start before writing
  --disjoint                  Cut overlapping subtitles so that no range overlaps another, before writing
  --join-repeats              Join the subtitles that repeat each other into one, before writing
  --repair                    Join the repeats, sort, then make the ranges disjoint, before writing
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
| `--to` | **tak** | nazwa z tabeli poniżej i nic innego | — |
| `--line-endings` | nie | `unix`, `windows` lub `mac` | końce linii odczytanego pliku |
| `--to-encoding` | nie | dowolne kodowanie, które potrafi zapisać ICU, z wyjątkiem tych, które zapisują własny znacznik | kodowanie odczytanego pliku |
| `--bom` / `--no-bom` | nie | flagi wzajemnie się wykluczające | to, co niósł odczytany plik |
| `--sort` | nie | flaga: ustawia napisy w kolejności ich początków **przed zapisem**, zob. [Sortowanie przy zapisie](#sortowanie-przy-zapisie) | wyłączone |
| `--join-repeats` | nie | flaga: robi jeden napis z kolejnych kawałków tego samego tekstu, **przed wszystkim innym**, zob. [Usuwanie nakładania się zakresów](#usuwanie-nakładania-się-zakresów) | wyłączone |
| `--repair` | nie | flaga: to samo co `--join-repeats --sort --disjoint`, w tej kolejności | wyłączone |
| `--disjoint` | nie | flaga: obcina nakładające się napisy **przed zapisem**, zob. [Usuwanie nakładania się zakresów](#usuwanie-nakładania-się-zakresów) | wyłączone |
| `--output` / `--output-dir` / `--in-place` | **jedna z trzech**, z wyjątkiem `--dry-run` | zob. [Uruchamianie](invocation.md#miejsce-docelowe) | — |
| `--dry-run` | nie | flaga: oblicza i mówi, nic nie zapisuje — zob. [Podgląd przed zapisem](invocation.md#podgląd-przed-zapisem) | wyłączone |

`mac` oznacza sam powrót karetki (`\r`), czyli koniec linii klasycznego Mac OS.

## Formaty akceptowane przez `--to`

| Nazwa | Format | Zapisywane rozszerzenie |
| :-- | :----- | :--------------- |
| `srt` | SubRip | `.srt` |
| `vtt` | WebVTT | `.vtt` |
| `subviewer2` | SubViewer 2.0 | `.sub` |
| `ssa` | Sub Station Alpha | `.ssa` |
| `ass` | Advanced SSA | `.ass` |
| `mpl2` | MPL2 | `.txt` |
| `microdvd` | MicroDVD | `.sub` |
| `tmplayer` | TMPlayer | `.txt` |
| `lrc` | LRC | `.lrc` |

**Nazwa, a nie rozszerzenie.** Dwa rozszerzenia nazywają po dwa formaty
— `.sub` należy także do MicroDVD, a `.txt` do MPL2 i do
TMPlayer — więc opcja przyjmująca rozszerzenia nie wiedziałaby, o który
chodzi. Powyższe nazwy są jednoznaczne i pokrywają się z rozszerzeniem
wszędzie tam, gdzie i rozszerzenie jest jednoznaczne.

**Lista jest zamknięta.** To lista Gaupola i wszystkie dziewięć formatów na niej jest.

## Dwa formaty, które nie przenoszą końca

**TMPlayer i LRC zapisują tylko jedną pozycję na linię**, jej początek. Widać to
po obu stronach.

**Przy odczycie** końce są wywnioskowane: koniec linii to początek
następnej, a ostatnia dostaje pięć sekund. Tak robi Gaupol.
Mówimy o tym także, przez komunikat diagnostyczny, który `--verbose` pokazuje w trzech
podpoleceniach:

```
file.lrc: carries no end times; each one was taken from the next start, settled by the reader
```

**Przy zapisie** końce nie są zapisywane, bo nie ma dla nich miejsca.
Konwersja do jednego z tych dwóch formatów traci je więc, a podróż w obie strony nie
odnajduje ich z powrotem — wnioskuje je od nowa.

**LRC traci jeszcze jedno**: nie potrafi przenosić podziału linii.
Linia dialogu w dwóch liniach jest zapisywana w jednej, obie połączone
spacją. Tej straty również nie da się naprawić.

Żaden z nich nie ma słownictwa dla kursywy.

## `--frame-rate` i jedyny format liczący w klatkach

**MicroDVD nie liczy w sekundach**: jego pozycje *są* numerami
klatek, a żaden plik MicroDVD nie deklaruje liczby klatek na sekundę, przy której je
policzono. Podaje ją `--frame-rate` i działa po obu stronach: to
liczba klatek, przy której odczytuje się plik `.sub` w klatkach, i ta, przy której
konwersja do MicroDVD liczy klatki.

| Opcja | Wymagane | Wartości | Domyślnie |
| :----- | :----- | :------ | :----- |
| `--frame-rate` | nie | liczba klatek na sekundę: `25`, `23.976`, `29.97` | zob. niżej |

**Przy odczycie, bez `--frame-rate`, plik jest odczytywany przy 23,976** — wartości
Gaupola — a poziom `-vvv` o tym mówi:
`counts in frames and states no rate; it was read at`. To jedyne miejsce w narzędziu, w którym każda pokazana pozycja
opiera się na założeniu, którego plik nie może potwierdzić.

**Przy zapisie, bez `--frame-rate`, trzy przypadki i trzy odpowiedzi:**

| Plik wyjściowy | Co się dzieje |
| :------------------- | :-------------- |
| jest już MicroDVD | liczba klatek, która go odczytała, zapisuje go z powrotem, a klatki wracają identyczne |
| jest oparty na czasie, a jego pozycje wypadają na siatce | brana jest siatka, a `-vv` mówi, która |
| jest oparty na czasie, bez siatki | **odmowa**, kod 2: `writing frames needs a frame rate, and the positions fall on no grid to take one from — give --frame-rate` |

Odmowa jest zamierzona. Jedynym pozostałym gestem byłoby wymyślenie wartości, a
każda linia pliku przesunęłaby się o tyle, o ile ta wartość byłaby błędna.

## Co traci konwersja

**Konwersja do innego formatu nigdy nie jest bezstratna**, a to, co nie przetrwa,
zależy od pary formatów:

| Co nie przechodzi | Dlaczego |
| :--------------------- | :------- |
| nagłówek | ma sens tylko we własnym formacie: `[INFORMATION]` nie jest nagłówkiem WebVTT |
| dane właściwe formatowi | współrzędne SubRip oraz identyfikator i ustawienia cue WebVTT nie mają odpowiednika gdzie indziej |
| dokładność | SubViewer 2, oba Sub Station Alpha i LRC zapisują z dokładnością do setnych, MPL2 do dziesiętnych, TMPlayer do sekundy, MicroDVD do klatki; `01:00:00,017` wraca jako `01:00:00,020` |
| koniec linii | TMPlayer i LRC nie mają na niego pola; następny odczyt wnioskuje go od nowa |
| podziały linii | tylko LRC: jego linie mieszczą się w jednej linii, a obie są sklejane spacją |
| znaczniki, których format docelowy nie potrafi zapisać | WebVTT nie ma `<font color>`, TMPlayer i LRC nie mają nic |
| układ | `{\pos(x,y)}`, `{\an8}`, `<v Marie>`, `<ruby>`: przetrwają podróż w obie strony we własnym formacie, a przy konwersji znikają |

**Zapis pliku z powrotem w jego własnym formacie niczego nie traci** — to gwarancja,
na której opiera się cała reszta, a `--to srt` dla pliku `.srt` oddaje te same bajty. Nic
nie jest nawet dekodowane: tekst pozostaje surowym łańcuchem, który odczytano.

## Znaczniki są tłumaczone, a nie kopiowane

**Każdy format zapisuje kursywę po swojemu**, a konwersja bez tłumaczenia
zostawiłaby `{\i1}` w pliku `.srt`: tekst, który widzi użytkownik, w
pliku, który nigdy nie będzie wiedział, jak go zinterpretować. Przechodzi **sześć
rzeczy** — pogrubienie, kursywa, podkreślenie, kolor, czcionka i rozmiar:

| Format | Pogrubienie | Kursywa | Podkreślenie | Kolor | Czcionka | Rozmiar |
| :----- | :--: | :------: | :------: | :-----: | :----: | :----: |
| SubRip | `<b>` | `<i>` | `<u>` | `<font color="#RRGGBB">` | — | — |
| WebVTT | `<b>` | `<i>` | `<u>` | — | — | — |
| SubViewer 2 | `<b>` | `<i>` | `<u>` | `<font color="#RRGGBB">` | — | — |
| Sub Station Alpha | `{\b1}` | `{\i1}` | — | `{\c&HBBGGRR&}` | `{\fnNAME}` | `{\fsN}` |
| Advanced SSA | `{\b1}` | `{\i1}` | `{\u1}` | `{\c&HBBGGRR&}` | `{\fnNAME}` | `{\fsN}` |
| MicroDVD | `{Y:b}` | `{Y:i}` | `{Y:u}` | `{C:$BBGGRR}` | `{F:NAME}` | `{S:N}` |
| MPL2 | `\` | `/` | `_` | `{C:$BBGGRR}` | `{F:NAME}` | `{S:N}` |
| TMPlayer | — | — | — | — | — | — |
| LRC | — | — | — | — | — | — |

**Kreska to zapowiedziana strata.** Konwersja pliku `.srt` z kolorem do WebVTT usuwa
kolor i mówi o tym; powstały plik to WebVTT, a nie SubRip w przebraniu.

**MicroDVD i MPL2 nie potrafią stylizować części linii.** Ich znaczniki
nie mają końca: `{y:i}` pochyla wszystko, co następuje, do końca linii, a
`{Y:i}` do końca napisu. Linia, w której tylko połowa była
pogrubiona, jest więc zapisywana bez żadnego pogrubienia, a nie z pogrubieniem, które się rozlewa.

## Co konwersja zgłasza jako utracone

**Mówi o tym, gdy coś traci, i milczy, gdy nic nie traci.** Raport,
który pojawia się przy każdym wywołaniu, to raport, którego nikt nie czyta.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,500\n<i>sur deux</i>\nlignes\n\n' > a.srt; subedit-cli convert --to lrc --output b.lrc a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,500\n<i>sur deux</i>\nlignes\n\n' > a.srt; subedit-cli convert --to lrc --output b.lrc a.srt
a.srt: 1 subtitle written as LRC -> b.lrc
a.srt: ends are not carried by LRC, line breaks were joined in 1 subtitle, 1 tag dropped
```

Linia pojawia się na domyślnym poziomie szczegółowości i na każdym wyższym.
Pozycje, w tej kolejności:

| Pozycja | Co mówi |
| :---- | :----------- |
| `ends are not carried by <format>` | format docelowy nie zapisuje końca |
| `line breaks were joined in N subtitles` | tylko LRC: N napisów było w kilku liniach |
| `N tags dropped` | N znaczników nie dało się zapisać, łącznie z układem |
| `the header was dropped` | nagłówek nie przechodzi przez granicę formatu |
| `the <format> fields of N subtitles were dropped` | N napisów niosło dane właściwe ich formatowi |
| `positions moved by up to N ms` | największa odległość, o którą przesunęła się pozycja |

## Zmiana kodowania

**`--to-encoding` wybiera zapisywane kodowanie; `--encoding` mówi, w jakim
kodowaniu plik jest odczytywany.** Razem przekodowują plik:

```console
$ subedit-cli convert --to srt --encoding cp1252 --to-encoding utf-8 \
      --output-dir sortie film.srt
```

Bez `--to-encoding` plik jest zapisywany w **kodowaniu, w jakim go odczytano**,
tak jak jego końce linii i znacznik: plik przekonwertowany bez
wskazówki o kodowaniu oddaje te same znaki, w tych samych bajtach.

**Znak, którego kodowanie docelowe nie potrafi zapisać, zatrzymuje plik**, a
nic nie jest zapisywane. Litera `ł` nie ma miejsca w Latin-1; zastąpienie jej znakiem `?`
zgubiłoby tekst między odczytem a zapisem, tak że nikt by tego nie zauważył.

**Znacznik kolejności bajtów istnieje tylko dla kodowań Unicode.**
`--bom` z `--to-encoding`, które go nie niesie, jest odrzucane, a nie ignorowane
— zob. tabelę błędów poniżej.

**I kodowanie musi nazywać kolejność bajtów.** `UTF-16` i `UTF-32` to
nazwy, które zna ICU, a ich konwerter zapisuje **własny znacznik**, niezależnie od
prośby: `--no-bom` byłoby wtedy przyjęte i nieposłuchane. Obie nazwy są
więc odrzucane, a komunikat nazywa wyjście:

```console
$ subedit-cli convert --to srt --to-encoding UTF-16 --output copie.srt film.srt
"UTF-16" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do
```

`UTF-16LE`, `UTF-16BE`, `UTF-32LE` i `UTF-32BE` zapisują dokładnie te same
bajty, łącznie ze znacznikiem — ale pod kontrolą `--bom` i `--no-bom`. Odmowa
kosztuje więc tylko dokładniejszą nazwę i obowiązuje zarówno przy odczycie, jak
i przy zapisie: `--encoding UTF-16` dostaje tę samą odpowiedź.

## Miejsce docelowe

Trzy opcje i ich reguły są wspólne dla podpoleceń, które
zapisują: zob. [Uruchamianie](invocation.md#miejsce-docelowe). Nic nie jest zapisywane
bez jednej z nich.

To, co właściwe dla `convert`: **`--output-dir` zmienia rozszerzenie wraz z
formatem.** `convert --to vtt` dla `a.srt` zapisuje `a.vtt`, nigdy `a.srt` zawierający
WebVTT — plik, którego nazwa kłamie, myli każde inne narzędzie.

## Konwersja w miejscu jest odrzucana

`--in-place` z `--to`, które zmienia format, daje kod `1` i niczego
nie zapisuje: w miejscu nie ma drugiej nazwy, która niosłaby nowy format, a
plik pozostałby pod rozszerzeniem, którego jego zawartość już nie uzasadnia.

`--in-place` można nadal używać do zmiany **postaci bez zmiany formatu** —
na przykład do zapisania pliku `.srt` z powrotem jako `.srt` z innymi końcami linii.

Odmowa jest rozstrzygana **wyłącznie na podstawie rozszerzenia**, przed jakimkolwiek odczytem: błąd
użycia nigdy nie może zostawić partii zapisanej do połowy.

## Końce linii i znacznik kolejności bajtów

Domyślnie **zapisany plik przejmuje to, co niósł plik odczytany**. Model
zachowuje jedno i drugie przy odczycie; narzucanie ich domyślnie traciłoby, przy każdej
konwersji, informację zachowaną celowo.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli convert --to vtt --output b.vtt a.srt; cat b.vtt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli convert --to vtt --output b.vtt a.srt; cat b.vtt
a.srt: 1 subtitle written as WebVTT -> b.vtt
WEBVTT

00:01.000 --> 00:03.000
Bonjour.
```

## Sortowanie przy zapisie

`--sort` zapisuje plik **w kolejności początków**, niezależnie od formatu docelowego. Sortowanie odbywa się **na kopii**: odczytany plik
nie jest ruszany, a zapisywane jest to, co zrobiłoby z niego [`sort`](sort.md) — stabilnie, dwa napisy, które zaczynają się razem, zachowują swoją kolejność.

**Nigdy domyślnie**: kolejność pliku jest kolejnością jego autora, dopóki nic innego nie zostanie zażądane. Z tą opcją każdy plik mówi o tym na
pierwszym poziomie komentarza: `<path>: N subtitles moved` lub `<path>: already in order`, a rekord `--format json` zawiera licznik
`moved` — **nieobecny bez tej opcji**, aby nie zmieniać postaci tego, co czytają istniejące skrypty.

## Usuwanie nakładania się zakresów

`--disjoint` obcina nakładające się napisy tak, aby żaden zakres nie nakładał się już na inny, **na tej samej kopii co sortowanie**: odczytany plik nie jest ruszany.

Nigdy domyślnie. Opcja robi to, co robi [`disjoint`](disjoint.md), **z jego ustawieniami domyślnymi**, przed zapisem, po sortowaniu. Wymaga pliku we właściwej kolejności: w przeciwnym razie jest odrzucana, a `--sort` — które działa wcześniej — wcześniej tę kolejność ustawia. Plik mówi o tym na pierwszym poziomie komentarza (`<path>: N subtitles split, M merged`), a rekord `--format json` zawiera liczniki `split` i `merged`, **nieobecne bez tej opcji**.

**`--join-repeats` działa przed sortowaniem i obcinaniem**: tekst pocięty na następujące po sobie kawałki jest najpierw odbudowywany (zob.
[`join-repeats`](join-repeats.md)), potem napisy są sortowane, a potem zakresy rozłączane. **`--repair` to wszystkie trzy naraz**,
jak [`repair`](repair.md), z domyślnymi ustawieniami obcinania. Z `--join-repeats` każdy plik mówi
`<path>: N repeated subtitles joined` na pierwszym poziomie komentarza, a rekord `--format json` zawiera `counts.joined` — **nieobecne bez tej opcji**.

## Wynik

**Standardowe wyjście** — nic: wynikiem jest zapisany plik. Z `--dry-run` również nic: nie ma pliku, a linia komentarza o tym mówi — zob. [Podgląd przed zapisem](invocation.md#podgląd-przed-zapisem).

Na wyjściu błędów, zależnie od poziomu:

| Poziom | Co jest dodawane |
| :----- | :-------------- |
| 1 | `<path>: N subtitles written as WebVTT -> <destination>` |
| 2 | `<path>: SubRip -> WebVTT, UTF-8, no BOM, LF line endings` — podane kodowanie to kodowanie zapisywane |
| 3 | `<path>: N bytes read, M written`, a następnie **każdy komunikat diagnostyczny odczytu** — zob. [Uruchamianie](invocation.md#odczyt-komunikatów-diagnostycznych) |

### W formacie JSON

Z [`--format json`](invocation.md#wynik-czytelny-dla-skryptu) obiekt każdego pliku zawiera `counts.subtitles`, a potem to, co konwersja utraciła: `lost_ends` i `lost_header` (`0` lub `1`), `joined_lines`, `lost_tags`, `lost_fields` i `furthest_ms` — pozycje [zdania o stracie](#co-konwersja-zgłasza-jako-utracone). Pełna tabela znajduje się w [Uruchamianie](invocation.md#co-każde-podpolecenie-umieszcza-w-counts).

## Kody wyjścia

Takie jak w całym narzędziu: `0`, jeśli zapisano każdy plik, `2`, jeśli żaden, `3`, jeśli
tylko niektóre, `1` przy błędzie użycia.

## Błędy

| Co to wywołuje | Komunikat |
| :------------------ | :------ |
| `--bom` z `--no-bom` | `--bom and --no-bom ask for opposite things; give one or the other` |
| `--in-place` zmieniające format | `--in-place cannot change the format: the file would keep a name its content no longer matches` |
| `--bom` dla kodowania, które go nie ma | `<path>: <encoding> has no byte order mark to write` |
| `--to-encoding` podające nieznane kodowanie | `no encoding is named "<name>"` |
| `--to-encoding` podające kodowanie, które zapisuje własny znacznik | `"<name>" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do` |
| znak, którego brak w zapisywanym kodowaniu | `<path>: holds a character the chosen encoding cannot write` |

**Odrzucanie `--bom` dla kodowania, które go nie niesie, nie jest
pedanterią.** Znacznik kolejności bajtów istnieje dla kodowań Unicode i
dla żadnych innych: prosić o niego dla pliku w Windows-1252 to prosić o
coś, co nie istnieje. Plik zostałby zapisany bez niego i nikt nie
wiedziałby, że prośba nie została spełniona.

Błędy miejsca docelowego są wspólne dla podpoleceń, które zapisują:
zob. [Uruchamianie](invocation.md#miejsce-docelowe).

Błędy odczytu są takie same jak w [`inspect`](inspect.md): dotyczą
pliku wejściowego i mają te same komunikaty.
