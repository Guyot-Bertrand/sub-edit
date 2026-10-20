# `correct`

<!-- machine-translated -->
> Ta strona jest tłumaczeniem maszynowym i nie została jeszcze sprawdzona przez osobę władającą tym językiem.

```
subedit-cli correct --tasks LIST --code CODE
                    [--classes human|ocr|human,ocr]
                    [--enable NAME]... [--disable NAME]...
                    [--language CODE]
                    [--max-length N] [--max-lines N]
                    [--skip-length N|off] [--skip-lines N|off]
                    [--keep-blank-subtitles | --remove-blank-subtitles]
                    [--range N-M|N-]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    <file>...
```

Poprawia teksty za pomocą **wzorców Gaupola**, tak jak kreator
[`Correct Texts…`](../subedit-gui/correct-texts.md) w oknie — te same wzorce,
ten sam silnik, te same liczniki. Istnieje tu sześć zadań: **opisy dla
niesłyszących**, **łączenie** i **dzielenie wyrazów**, **typowe błędy**,
**wielkie litery** i **łamanie linii**.

<!-- example: subedit-cli correct --help -->
```console
$ subedit-cli correct --help
Correct the texts with the patterns of the correction assistant
Usage: subedit-cli correct [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to correct

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --tasks LIST                The tasks to run, in a list: none is chosen beforehand
  --code CODE                 The cascade of patterns the tasks read: Zyyy, Latn, Latn-en, Latn-en-US
  --classes human|ocr|human,ocr
                              The classes of common errors to apply: both by default
  --enable NAME               Switch a pattern on, by its English name or type:name; repeatable
  --disable NAME              Switch a pattern off, by its English name or type:name; repeatable
  --language CODE             The dictionary join-words and split-words read: fr_FR, en_US, en...
  --max-length N              Longest line of line-break, in characters; no default, Gaupol's is in ems
  --max-lines N               Most lines of line-break: 3, as Gaupol's
  --skip-length N|off         Leave alone a subtitle whose longest line is within this, or off: --max-length
  --skip-lines N|off          Leave alone a subtitle whose line count is within this, or off: --max-lines
  --keep-blank-subtitles      Leave the subtitles the correction empties, empty, instead of removing them
  --remove-blank-subtitles    Remove the subtitles the correction empties, whatever the settings file says
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

## Co jest wybierane, a co nie

**Nic nie jest zaznaczone z góry.** Okno otwiera swoje pola tak, jak zostawił je użytkownik; wiersz
poleceń pobiera zadania, kod, długość linii i język z własnego wiersza,
nigdy z pliku ustawień: ani zadania zaznaczone kiedyś w oknie, ani pola
wzorców tam przełączone (`correction.activations`), ani lista zamian sprawdzania pisowni nie są
odczytywane, a po tej stronie nic nie jest zapisywane.

**Cztery ustawienia pochodzą domyślnie z pliku ustawień** — wartości, które `Correct Texts…`
zapamiętuje albo które zapisało [`config set`](config.md). **Opcja w wierszu zawsze wygrywa**, potem
plik, potem wbudowana wartość domyślna; zob. [Plik ustawień](invocation.md#plik-ustawień),
gdzie jest plik, oraz `--config` i `--no-config`.

| Klucz | Co dostarcza | Opcja, która go przesłania |
| :-- | :--------------- | :----------------------- |
| `correction.human`, `correction.ocr` | stosowane klasy typowych błędów | `--classes` |
| `correction.remove-blank` | czy napisy opróżnione przez poprawkę są usuwane | `--keep-blank-subtitles`, `--remove-blank-subtitles` |
| `correction.line-break.max-lines` | największa liczba linii w `line-break` | `--max-lines` |

- **Każde z czterech ma opcję, która je cofa**, niezależnie od tego, co mówi plik: `--classes human,ocr`
  stosuje obie klasy, `--remove-blank-subtitles` usuwa opróżnione napisy, `--max-lines 3`
  przywraca linie Gaupola. `--keep-blank-subtitles` i `--remove-blank-subtitles` razem to
  błąd użycia.
- **Obie klasy wyłączone w pliku są ignorowane, i mówi się o tym** na poziomie 1 —
  `settings: correction.human and correction.ocr are both false, ignored` —: `--classes` nie mogłoby tego zapisać, a
  common-errors nie miałoby nic do roboty. Stosowane są zamiast tego obie klasy.
- **Liczba linii spoza przedziału od 1 do 1000 w ogóle nie jest odczytywana z pliku**: czytnik pliku
  ją odrzuca — `settings: correction.line-break.max-lines: cannot read 'N', default kept`, na
  poziomie 1 — i używane jest wbudowane 3.

Przykłady na tej stronie uruchomiono z `--no-config`, aby pokazywały, co robią wbudowane
wartości domyślne, niezależnie od ustawień maszyny, która je uruchamia.

| Opcja | Wymagane | Wartości | Domyślnie |
| :----- | :----- | :------ | :----- |
| `<file>...` | tak | jedna lub więcej ścieżek | — |
| `--tasks` | **tak** | lista rozdzielona przecinkami: `mentions`, `join-words`, `split-words`, `common-errors`, `capitalization`, `line-break` | żadne zadanie |
| `--code` | **tak** przy zadaniu opartym na wzorcach, a w pozostałych przypadkach **nie dotyczy** | `Script[-language[-COUNTRY]]`: `Zyyy`, `Latn`, `Latn-en`, `Latn-en-US` | — |
| `--language` | **tak** przy `join-words` lub `split-words`, a w pozostałych przypadkach **nie dotyczy** | kod języka: `fr`, `fr_FR`, `en_US` | — |
| `--classes` | nie | `human`, `ocr` lub `human,ocr` | obie — albo to, co mówi plik ustawień |
| `--enable`, `--disable` | nie, powtarzalne | angielska nazwa wzorca lub `type:name` | dostarczone pliki `.conf` |
| `--max-length` | **tak** przy `line-break` | liczba **znaków** większa od zera | **brak** — zob. niżej |
| `--max-lines` | nie | liczba całkowita linii, większa od zera | 3 — albo to, co mówi plik ustawień |
| `--skip-length`, `--skip-lines` | nie | liczba lub `off` | te same granice co `--max-length` i `--max-lines` |
| `--keep-blank-subtitles` | nie | flaga: opróżnione napisy zostają, puste | opróżnione napisy są usuwane — chyba że plik ustawień mówi inaczej |
| `--remove-blank-subtitles` | nie | flaga: opróżnione napisy są usuwane, niezależnie od tego, co mówi plik ustawień; odrzucana z `--keep-blank-subtitles` | wyłączone |
| `--range` | nie | `N-M` lub `N-` do końca — zob. [`adjust`](adjust.md#--range) | cały plik |
| `--document` | nie | `main` lub `translation` — zob. [Tłumaczenie](invocation.md#tłumaczenie) | `main` |
| `-t`, `--translation-file` | z `--document translation` | plik tłumaczenia, dla **jednego** pliku wejściowego | — |
| `--align-method` | nie, i tylko z `-t` | `position` lub `number` | `position` |
| `--recursive`, `-r` | nie | flaga | wyłączone — zob. [Przetwarzanie drzewa katalogów](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **jedna z trzech**, z wyjątkiem `--dry-run` | zob. [Uruchamianie](invocation.md#miejsce-docelowe) | — |
| `--dry-run` | nie | flaga: oblicza i mówi, nic nie zapisuje — zob. [Podgląd przed zapisem](invocation.md#podgląd-przed-zapisem) | wyłączone |

### Zadania

**Kolejność na liście nie ma znaczenia**: zadania są stosowane w
kolejności Gaupola — opisy, potem łączenie i dzielenie wyrazów, potem typowe błędy, potem wielkie litery, potem łamanie linii —, każde na
tekście, który zostawiło poprzednie. Napis opróżniony przez opisy nie bierze
udziału w dalszych krokach.

| Zadanie | Co robi |
| :---- | :-------------- |
| `mentions` | usuwa dźwięki w nawiasach kwadratowych oraz w nawiasach okrągłych, gdy włączone jest `Sound in parentheses` (przeszukiwanie jak w [`hearing-impaired`](hearing-impaired.md)), a potem wzorce tekstów piosenek i mówców |
| `join-words` | skleja z powrotem wyraz przecięty na dwa, według słownika z `--language` |
| `split-words` | dzieli dwa sklejone wyrazy, według tego samego słownika |
| `common-errors` | poprawia typowe błędy: spacje, interpunkcję, ligatury, błędy OCR |
| `capitalization` | stawia wielką literę tam, gdzie chce jej język |
| `line-break` | dzieli linie napisu od nowa tak, aby żadna nie przekraczała `--max-length`, z uwzględnieniem wzorców podziału z kodu |

### Kod wzorców

**`--code` nazywa kaskadę**: `Zyyy` dla dowolnego pisma, potem pismo, język
i kraj — `Latn-en-US` odczytuje wzorce z `Zyyy`, z `Latn`, z `Latn-en` i z
`Latn-en-US`. **Jest wymagany**, ponieważ wiersz poleceń nie ma języka otoczenia:
uniwersalna wartość domyślna stosowałaby *mniej*, niż się sądzi, nie mówiąc o tym. Obowiązuje dla
każdego zadania tego samego przebiegu; zadanie, które chce mieć własny kod, to drugi
przebieg. Źle zapisany kod to błąd użycia:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code francais --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code francais --output b.srt a.srt; echo "code=$?"
--code: "francais" is not a pattern code: expected Script[-language[-COUNTRY]], like Zyyy, Latn or Latn-en-US
code=1
```

Wzorce, które użytkownik umieszcza w `$XDG_DATA_HOME/subedit/patterns` (lub
`~/.local/share/subedit/patterns`), **są dodawane do wzorców dostarczonych**, tak jak w
oknie, a dostarczony wzorzec, którego nie da się odczytać, jest zgłaszany na poziomie 1: `patterns: <file>, line N (<reason>)`.

### Łączenie i dzielenie wyrazów oraz słownik

**Te dwa zadania nie odczytują żadnego wzorca**: odpytują **słownik systemowy przez
Enchant** — zależność pakietów (`libenchant-2-2` w Debianie i Ubuntu, `enchant2` w
Fedorze), którą `subedit-cli` dzieli z oknem. Nie mają więc ani kodu, ani nazwy do ustawienia:
`--code` nie ma sensu bez zadania opartego na wzorcach, a `--language` nie ma sensu bez jednego z tych dwóch
— podanie jednego bez drugiego to błąd użycia, kod `1`.

- **`join-words`** skleja wyraz, którego słownik nie zna, z poprzedzającym go lub
  następującym po nim, **gdy tylko jeden kierunek daje znany wyraz**: `hel lo` zmienia się w `hello`; dwa możliwe
  kierunki lub żaden nie rozstrzygają niczego.
- **`split-words`** dzieli wyraz, którego słownik nie zna, **gdy dokładnie jedna z jego
  sugestii to ten wyraz ze spacją**: `hellothere` zmienia się w `hello there`. Wyraz z wielką literą
  na początku, po której następują małe litery, jest pomijany: najczęściej jest to nazwa własna.

**Bez słownika dla danego języka przebieg jest odrzucany**, kod `1`, **zanim jakikolwiek plik zostanie
odczytany**, i słowami okna, które wyszarza tę funkcję i o tym mówi — tu wiersz poleceń
odmawia i mówi o tym, zamiast milczeć:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nhel lo\n\n' > a.srt; subedit-cli --no-config correct --tasks join-words --language qq_QQ --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nhel lo\n\n' > a.srt; subedit-cli --no-config correct --tasks join-words --language qq_QQ --output b.srt a.srt; echo "code=$?"
no dictionary for qq_QQ
code=1
```

**Lista zamian okna nie jest ani odczytywana, ani zapisywana.** *Zapełnia się* przez
wybory użytkownika w sprawdzaniu pisowni; wiersz poleceń nie ma takiego
wyboru, a odczytywanie listy uzależniłoby skrypt od tego, co jego autor kiedyś do niej włożył.

### Łamanie linii

**Mierzy się je w znakach, i na tym polega różnica względem okna.** Okno mierzy w *em* —
szerokości skalibrowanej według czcionki, przy 0,55 em na literę —, a jego domyślne 24 to wartość
w em. Wiersz poleceń nie ma czcionki: liczy znaki, te z
tekstu. **Przepisanie 24 jako znaków pocięłoby prawie wszystko, nie mówiąc
o tym**; dlatego **nie ma domyślnej długości**, a `--max-length` jest wymagane przy
`line-break`:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn texte\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn texte\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --output b.srt a.srt; echo "code=$?"
--max-length is required by the task line-break: Gaupol's 24 is a width in ems, and has no value in characters
code=1
```

**`--max-lines` wynosi 3**, jak u Gaupola, czego jednostka nie zmienia; łamanie linii bierze
jedną linię więcej, gdy trzy nie wystarczają. **Pomijanie** — napis mieszczący się już w limitach jest
zostawiany bez zmian — bierze domyślnie **te same granice** (`--skip-length` = `--max-length`,
`--skip-lines` = `--max-lines`), jak u Gaupola; `--skip-length off` i `--skip-lines off`
je wyłączają, a tekst jest wtedy odczytywany od nowa jako jedna całość i dzielony od nowa:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nWhen the rain stopped, we walked home under the dark sky of the old town.\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --max-length 24 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nWhen the rain stopped, we walked home under the dark sky of the old town.\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --max-length 24 --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:03,000
When the rain
stopped, we walked
home under the dark
sky of the old town.
```

Wzorce podziału pochodzą z kodu: `Latn` nie łamie między liczbą a jej jednostką ani
po myślniku dialogowym, `Latn-en` nie łamie po rodzajniku, przyimku, tytule
ani zaimku dzierżawczym. **Pod `Zyyy` nie istnieje żaden wzorzec podziału**: linie są
tylko wyrównywane. **To zadanie nigdy nie jest odrzucane z braku aktywnego wzorca**: jego wzorce
ważą, gdzie łamać, a łamanie następuje i bez nich. Cztery opcje mają sens tylko z
`line-break`; podane bez niego są błędem użycia.

### Klasy i włączanie według nazwy

**`--classes` filtruje typowe błędy na dobre**: odznaczenie `ocr` usuwa jego
wzorce ze stosowania — ligatura `ﬁ` pozostaje wtedy `ﬁ`. Pozostałe trzy typy wzorców
nie niosą klasy.

**`--enable` i `--disable` ustawiają wzorzec według jego angielskiej nazwy**, przy czym nazwa jest
kluczem (ADR 0037); nazwa może wskazywać kilka rekordów, które dzielą wtedy jedno
pole. Jest to konieczne i to ono odsuwa wartości domyślne okna: **dostarczone
pliki `.conf` wyłączają wzorce opisów**, więc samo `--tasks mentions`
niczego by nie zastosowało. Dwa wzorce „Sound in brackets” i „Sound in parentheses”
sterują każdy swoim ogranicznikiem — nawiasami kwadratowymi, nawiasami okrągłymi — i są ustawiane tymi samymi
opcjami: włączenie tylko jednego nie usuwa drugiego.

Trzy rzeczy są **błędami użycia**, kod `1`, zgłaszanymi, zanim jakikolwiek plik zostanie odczytany:

| Co to wywołuje | Komunikat |
| :------------------ | :------ |
| nazwa, która nie wskazuje żadnego wzorca podanych zadań, przy podanym kodzie | `--enable: "<name>" names no pattern of the tasks given, under the code <code>` |
| nazwa, która wskazuje wzorce dwóch typów — zapisuje się ją wtedy `type:name`, gdzie typem jest `common-error`, `capitalization`, `hearing-impaired` lub `line-break` | `--enable: "<name>" names patterns of several types: write type:name, …` |
| nazwa podana zarówno w `--enable`, jak i w `--disable` | `"<name>" is given to --enable and to --disable: choose one` |

**Literówka jest głośna**: okno ignoruje przestarzałe ustawienie, ponieważ
ustawienie może przeżyć wzorzec, który zniknął; wywołanie nie ma ocalałych.

**Zadanie bez żadnego aktywnego wzorca jest odrzucane**, kod `1` — „nie ma nic do zrobienia” to
odpowiedź, która jest wypowiadana, a nie ciche powodzenie:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --output b.srt a.srt; echo "code=$?"
mentions: no pattern is active under the code Latn; switch one on with --enable
code=1
```

Ten sam plik, z włączonym przeszukiwaniem nawiasów kwadratowych:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --enable "Sound in brackets" --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --enable "Sound in brackets" --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:02,000
Bonjour
```

## Co jest zapisywane, a co mówione

Format, kodowanie i końce linii odczytanego pliku są **zachowywane**. **Napisy
opróżnione przez poprawkę są usuwane**, jak w domyślnie zaznaczonym polu
okna; `--keep-blank-subtitles` zostawia je puste, a `--remove-blank-subtitles` usuwa je
nawet wtedy, gdy plik ustawień je zachowuje. **Opróżnione tłumaczenie pozostaje opróżnione**
i zachowuje swój napis.

**Licznik jest licznikiem okna** — „Edited N and removed M subtitles”: teksty
zmienione i napisy usunięte, **nigdy dopasowania**. Plik, w którym nic się
nie zmienia, jest mimo to zapisywany: podane miejsce docelowe jest miejscem zapisanym.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\neﬀet  de ﬁn\n\n2\n00:00:03,000 --> 00:00:04,000\nPropre.\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code Latn --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\neﬀet  de ﬁn\n\n2\n00:00:03,000 --> 00:00:04,000\nPropre.\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code Latn --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:02,000
effet de fin

2
00:00:03,000 --> 00:00:04,000
Propre.
```

### Wzorzec, którego nie można zastosować

**Wzorzec, którego nie da się odczytać, przetłumaczyć lub który się nie kończy, jest nazywany, razem z
napisem, na którym się poddał**, i tu wiersz poleceń mówi więcej niż
okno: strona potwierdzenia nazywa wzorzec tylko raz, bez tekstu.

| Powód | Co to oznacza |
| :----- | :------------------- |
| `cannot be translated` | jego wyrażenie używa tego, czego tłumaczenie na ICU nie przyjmuje |
| `will not compile` | silnik odrzucił jego wyrażenie |
| `has an invalid replacement` | jego zamiennik nie daje się odczytać tak, jak czyta go Python |
| `timed out` | silnik zrezygnował z tego tekstu |
| `never settled` | `Repeat` wciąż zmieniał tekst po stu przebiegach |
| `grew the text too long` | tekst przekroczył to, co może pomieścić napis |

Na poziomie 1, na wyjściu błędów:
`<path>: pattern "<name>" (<reason>) was not applied to subtitle N` — bez końca, gdy wzorzec jest odsunięty dla każdego tekstu. **Numer jest numerem
w pliku**, niezależnie od tego, jakie zadania usunęły wcześniej napisy. W JSON jest to
ostrzeżenie `pattern-failed` pliku, którego `detail` zawiera to zdanie. **Wzorzec, który
się poddał, nie jest plikiem zakończonym błędem**: plik jest zapisywany, a kod wyjścia to `0`.

## Wynik

**Standardowe wyjście** — nic, wynikiem jest zapisany plik. **Z `--dry-run` są to
proponowane zmiany**, jeden blok na napis: jego numer, tekst przed, tekst po —
oraz `(removed)` bez linii `+` dla napisu, który poprawka by usunęła; z
`--keep-blank-subtitles` zostaje, pusty. Zob. [Podgląd przed zapisem](invocation.md#podgląd-przed-zapisem).

| Poziom | Co jest dodawane na wyjściu błędów |
| :----- | :------------------------------------- |
| 1 | `<path>: Edited N and removed M subtitles -> <destination>`, wzorce, których nie dało się zastosować, pliki wzorców, których nie da się odczytać — a najpierw odczytany plik ustawień — `settings: read <path>` — i to, co z niego zignorowano |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, a następnie każdy komunikat diagnostyczny odczytu |

W JSON `counts` zawiera `corrected` (zmienione teksty) i `removed` (usunięte
napisy), a `changes` — ruszone napisy. Z `-t` klucz `alignment` mówi, jak
ułożono tłumaczenie.
`settings` nazywa odczytany plik ustawień albo ma wartość `null`, gdy żadnego nie odczytano.

## Kody wyjścia

Takie jak w całym narzędziu: `0`, jeśli zapisano każdy plik, `2`, jeśli żaden, `3`, jeśli tylko
niektóre, `1` przy błędzie użycia.

## Błędy

| Co to wywołuje | Co jest zapisywane na wyjściu błędów |
| :------------------ | :--------------------------------------- |
| plik wskazany w `--config`, który nie istnieje — brakujący własny plik użytkownika daje wartości domyślne | `settings file not found: <FILE>` |
| brak `--tasks` | `--tasks is required`, a po nim wskazówka do `--help` |
| nieznane zadanie | `--tasks: "<name>" is not a task: expected mentions, join-words, split-words, common-errors, capitalization or line-break` |
| brak `--code` | `--code is required by the tasks that read patterns: <tasks>` |
| `--code` bez zadania opartego na wzorcach | `--code is for the tasks that read patterns, and none was asked for` |
| brak `--language` przy `join-words` lub `split-words` | `--language is required by the tasks that check words: <tasks>` |
| `--language`, które nie jest kodem języka | `--language: "<value>" is not a language: expected a locale code, like fr, en_US or sr@Latn` |
| `--language` bez jednego z tych dwóch zadań | `--language is for the tasks join-words and split-words, which were not asked for` |
| język bez słownika | `no dictionary for <language>` |
| źle zapisany kod | `--code: "<code>" is not a pattern code: expected Script[-language[-COUNTRY]], like Zyyy, Latn or Latn-en-US` |
| brak `--max-length` przy `line-break` | `--max-length is required by the task line-break: Gaupol's 24 is a width in ems, and has no value in characters` |
| `--max-length`, które nie jest liczbą większą od zera | `--max-length: "<value>" is not a length: expected a number of characters greater than zero` |
| `--max-lines`, `--skip-lines`, które nie są liczbą całkowitą linii | `--max-lines: "<value>" is not a number of lines: expected a whole number greater than zero` |
| nieczytelne `--skip-length` lub `--skip-lines` | `--skip-length: "<value>" is not a bound: expected a number greater than zero, or off` |
| jedna z czterech opcji bez `line-break` | `--max-length is for the task line-break, which was not asked for` |
| nieznana klasa | `--classes: "<name>" is not a class: expected human, ocr or human,ocr` |
| `--keep-blank-subtitles` z `--remove-blank-subtitles` | `--keep-blank-subtitles and --remove-blank-subtitles say opposite things; give one of them` |
| błędna nazwa wzorca | zob. tabelę powyżej |
| zadanie bez aktywnego wzorca | `<task>: no pattern is active under the code <code>; switch one on with --enable` |
| `--range`, `-t`, `--document`, miejsce docelowe | te z [Uruchamianie](invocation.md#błędy) i z [Tłumaczenie](invocation.md#tłumaczenie) |
