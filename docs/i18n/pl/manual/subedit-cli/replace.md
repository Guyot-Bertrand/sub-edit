# `replace`

<!-- machine-translated -->
> Ta strona jest tłumaczeniem maszynowym i nie została jeszcze sprawdzona przez osobę władającą tym językiem.

```
subedit-cli replace [--regex] [--case-sensitive] [--range N-M|N-]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    <pattern> <replacement> <file>...
```

Zastępuje tekst w napisach **bez psucia znaczników**. To operacja
`Replace All` z [wyszukiwania](../subedit-gui/find-and-replace.md) w oknie, z jej zasadami:

- **wyszukiwanie dotyczy tekstu widocznego** — tego, co czyta widz. Znaczniki nie wchodzą
  w jego skład: szukanie `<i>` nic nie znajduje, a szukanie `Bonjour` znajduje „Bonjour”, którego połowa
  jest pochylona;
- **zastąpienie dotyczy tekstu źródłowego**, pliku, z zachowaniem znaczników tam, gdzie
  były. Znacznik, który przecina szukane słowo, obejmuje całe zastąpienie; styl, który
  dotyka dopasowania, obejmuje całe zastąpienie; to, czego dopasowanie nie dotyka,
  nie przesuwa się. Zasady, przypadek po przypadku, są takie jak w wyszukiwaniu okna.

**Nie ma podpolecenia `find`**: aby zobaczyć, gdzie jest wzorzec, niczego nie zmieniając,
służy [`--dry-run`](invocation.md#podgląd-przed-zapisem), który wylicza każdy napis, którego sprawa dotyczy.

<!-- example: subedit-cli replace --help -->
```console
$ subedit-cli replace --help
Replace a text in the subtitles, without breaking a tag
Usage: subedit-cli replace [OPTIONS] pattern replacement files...

Positionals:
  pattern TEXT REQUIRED       Text to look for in what is shown
  replacement TEXT REQUIRED   What replaces it, tags allowed
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --regex                     Read the text to look for as a regular expression
  --case-sensitive            Tell capitals from small letters
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

## Argumenty i opcje

| Opcja | Wymagane | Wartości | Domyślnie |
| :----- | :----- | :------ | :----- |
| `<pattern>` | tak | tekst do wyszukania w tym, co jest pokazywane; **niepusty** | — |
| `<replacement>` | tak | to, co go zastępuje, znaczniki dozwolone; może być puste | — |
| `<file>...` | tak | jedna lub więcej ścieżek | — |
| `--recursive`, `-r` | nie | flaga | wyłączona — zobacz [Przetwarzanie drzewa katalogów](lots.md) |
| `--regex` | nie | flaga: wzorzec jest wyrażeniem regularnym ICU | zwykły tekst |
| `--case-sensitive` | nie | flaga: odróżnia wielkie litery od małych | są ignorowane |
| `--range` | nie | `N-M` lub `N-` do końca — zobacz [`adjust`](adjust.md#--range) | cały plik |
| `--document` | nie | `main` lub `translation` — zobacz [Tłumaczenie](invocation.md#tłumaczenie) | `main` |
| `-t`, `--translation-file` | z `--document translation` | plik tłumaczenia, dla **jednego** pliku wejściowego | — |
| `--align-method` | nie, i tylko z `-t` | `position` lub `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **jedna z trzech**, z wyjątkiem `--dry-run` | zobacz [Uruchamianie](invocation.md#miejsce-docelowe) | — |
| `--dry-run` | nie | flaga: oblicza i mówi, nie zapisuje — zobacz [Podgląd przed zapisem](invocation.md#podgląd-przed-zapisem) | wyłączona |

**Wzorzec zaczynający się od myślnika** zapisuje się po `--`: `subedit-cli replace -- -- "—" film.srt`.

**Domyślnie tekst główny; tłumaczenie z `--document translation -t FILE`.** Wtedy
zmieniane i zapisywane jest tłumaczenie, i tylko ono — zobacz [Tłumaczenie](invocation.md#tłumaczenie).

### Zwykły tekst lub wyrażenie

Domyślnie wzorzec jest **odczytywany dosłownie**: `a.b` szuka litery `a`, kropki i litery `b`. Wielkość
liter jest ignorowana równie dokładnie jak w oknie — `É` i `é` to jedna litera.

Z opcją `--regex` wzorzec jest wyrażeniem regularnym **ICU**, odczytywanym tak, jak czyta je Gaupol: `.` przekracza
koniec linii, `^` i `$` obowiązują w każdej linii napisu. W zastąpieniu, i
tylko tam:

| Zapisane | Do powiedzenia |
| :----- | :-------- |
| `$1` do `$9` | grupy dopasowania |
| `$0` | całe dopasowanie |
| `\n` | podział linii |
| `\$`, `\\` | znak `$`, ukośnik wsteczny |

Uważaj na cudzysłowy powłoki: `'$1 EUR'`, a nie `"$1 EUR"`, które pozwoliłoby powłoce odczytać `$1`.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nPrix: 12 euros, et 7 euros.\n\n' > prix.srt; subedit-cli replace --regex '([0-9]+) euros' '$1 EUR' --output euros.srt prix.srt; cat euros.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nPrix: 12 euros, et 7 euros.\n\n' > prix.srt; subedit-cli replace --regex '([0-9]+) euros' '$1 EUR' --output euros.srt prix.srt; cat euros.srt
prix.srt: replaced 2 matches -> euros.srt
1
00:00:01,000 --> 00:00:02,000
Prix: 12 EUR, et 7 EUR.
```

## Znaczniki

Tekst, w którym znacznik przecina szukane słowo:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace Bonjour Salut --output salut.srt balises.srt; cat salut.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace Bonjour Salut --output salut.srt balises.srt; cat salut.srt
balises.srt: replaced 2 matches -> salut.srt
1
00:00:01,000 --> 00:00:02,000
<i>Salut</i> tout le monde

2
00:00:03,000 --> 00:00:04,000
Salut <i>Marie</i>, ca va
```

W pierwszym napisie znacznik przeciął „Bonjour”: teraz obejmuje „Salut”.
W drugim nie jest dotknięty i zostaje tam, gdzie był. **Znacznik pozostawiony otwarty
pozostaje otwarty**: naprawianie chwiejnych znaczników byłoby inną operacją.

Zastąpienie jest odczytywane w **znacznikach formatu pliku**: `<i>…</i>` dla SubRip,
`{\i1}…` dla Advanced SSA. To jedyny sposób formatowania tego, co się wpisuje.

## Nic nie znaleziono, nic do zmiany

Dwie odpowiedzi, które nie są porażką — kod wyjścia to `0`, a **plik jest mimo to zapisywany**,
bo podane miejsce docelowe to miejsce docelowe zapisane:

| Sytuacja | Zdanie |
| :-------- | :----- |
| wzorca nie ma nigdzie | `"zzz" not found` |
| jest, ale każde dopasowanie jest zastępowane samym sobą | `nothing to change` |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli replace zzz x --output b.srt a.srt; subedit-cli replace Bonjour Bonjour --output c.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli replace zzz x --output b.srt a.srt; subedit-cli replace Bonjour Bonjour --output c.srt a.srt
a.srt: "zzz" not found -> b.srt
a.srt: nothing to change -> c.srt
```

## `--dry-run`

Standardowe wyjście wylicza **każdy napis, którego tekst by się zmienił**, przed i po, tak
jak jest w pliku — łącznie ze znacznikami:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace --dry-run Bonjour Salut balises.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace --dry-run Bonjour Salut balises.srt
balises.srt: replaced 2 matches (dry run, nothing written)
balises.srt: subtitle 1
- <i>Bon</i>jour tout le monde
+ <i>Salut</i> tout le monde
balises.srt: subtitle 2
- Bonjour <i>Marie</i>, ca va
+ Salut <i>Marie</i>, ca va
```

Postać listy opisuje [Podgląd przed zapisem](invocation.md#podgląd-przed-zapisem);
w formacie JSON jest to `changes`.

## Wynik

**Standardowe wyjście** — nic: wynikiem jest zapisany plik. Z `--dry-run` lista
zmian powyżej.

| Poziom | Co jest dodawane na wyjściu błędów |
| :----- | :------------------------------------- |
| 1 | `<path>: replaced N matches -> <destination>`, lub `"<pattern>" not found`, lub `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — format, kodowanie, znacznik i końce linii odczytanego pliku, przywrócone takie, jakie były |
| 3 | `<path>: N bytes read, M written`, potem **każdy komunikat diagnostyczny odczytu** — zobacz [Uruchamianie](invocation.md#odczyt-komunikatów-diagnostycznych) |

`N` liczy **dopasowania zastąpione w napisach, których tekst się zmienił** — jest to
liczba, którą podaje także okno.

### W formacie JSON

Z opcją [`--format json`](invocation.md#wynik-czytelny-dla-skryptu) obiekt każdego pliku zawiera:

| Klucz | Zawartość |
| :-- | :------ |
| `counts.replaced` | zastąpione dopasowania w napisach, których tekst się zmienił: `N` ze zdania |
| `counts.matched` | znalezione dopasowania, **niezależnie od tego, czy ich zastąpienie zmieniło tekst** — to odróżnia „nic nie znaleziono” (`0`) od „nic do zmiany” (`replaced` równe `0`, `matched` dodatnie) |
| `changes` | zmienione napisy: `{"subtitle", "document", "before", "after"}`, **puste**, gdy nic się nie zmieniło |

## Kody wyjścia

Takie jak w całym narzędziu: `0`, jeśli każdy plik został przetworzony — plik, w którym nic nie znaleziono, też —,
`2`, jeśli żaden, `3`, jeśli tylko niektóre, `1` przy błędzie użycia.

## Błędy

| Co to wywołuje | Komunikat |
| :------------------ | :------ |
| pusty wzorzec | `pattern: nothing to look for` |
| nieczytelne wyrażenie | `pattern: not a regular expression (<ICU's reason>)` |
| zakres źle zapisany lub poza plikiem | zobacz [`adjust`](adjust.md#--range) |

**Wzorzec jest odczytywany przed pierwszym plikiem**: nieczytelne wyrażenie to błąd użycia
(kod `1`), nic nie jest odczytywane ani zapisywane.

Błędy miejsca docelowego są wspólne dla podpoleceń, które zapisują:
zobacz [Uruchamianie](invocation.md#miejsce-docelowe).
