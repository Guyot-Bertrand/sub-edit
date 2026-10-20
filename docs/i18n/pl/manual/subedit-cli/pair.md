# `pair`

<!-- machine-translated -->
> Ta strona jest tłumaczeniem maszynowym i nie została jeszcze sprawdzona przez osobę władającą tym językiem.

```
subedit-cli pair (--output FILE | --output-dir DIR | --in-place)
                 -t FILE [--align-method position|number]
                 [--dry-run]
                 <main>
```

Zapisuje **tłumaczenie z synchronizacją dopasowaną do pozycji pliku głównego** i mówi, jak jego linie znalazły
swój napis. To `Open Translation…` okna — [to samo dopasowanie](../subedit-gui/files.md#otwieranie-tłumaczenia),
to samo zdanie — po którym następuje zapis wyniku, który okno wykonuje przez `Save Translation`.

**To jedyna czynność dopasowania, której nie wykonuje plik przetwarzany samodzielnie.** Pozostałe podpolecenia,
które przyjmują `-t`, zmieniają odczytane tłumaczenie — [`case`](case.md), [`replace`](replace.md)… —; `pair` nie zmienia
niczego w tekście: **nakłada** tłumaczenie na plik główny i zapisuje to, co z nakładania wynikło. Plik główny jest
odczytywany, aby przyjąć linie, **nigdy nie jest zapisywany**.

- **Zapisywane jest tłumaczenie**, pod **własną ścieżką i we własnym formacie**: formacie, kodowaniu i
  końcach linii pliku tłumaczenia. Jego napisy znajdują się **na pozycjach pliku głównego**: linia, która
  wypadła obok znacznika, odzyskuje granice napisu, który tłumaczy.
- **`-t` jest wymagane.** `pair` bez tłumaczenia nie ma czego nakładać: to błąd użycia, kod `1`.
- **Jeden plik główny** i bez `--recursive`: para to dwa pliki, a drzewo nie ma
  jak powiedzieć, które tłumaczenie należy do którego pliku głównego. Dla partii — jedno wywołanie na parę.
- **`--output` wskazuje plik zapisywanego tłumaczenia, `--output-dir` umieszcza tam jego nazwę podstawową, `--in-place`
  nadpisuje go**, jak w każdym podpoleceniu przyjmującym `-t` — zobacz [Tłumaczenie](invocation.md#tłumaczenie).
- **`--document` nie istnieje**: wskazywany jest tylko jeden dokument, tłumaczenie.

<!-- example: subedit-cli pair --help -->
```console
$ subedit-cli pair --help
Write a translation at the positions of its main file, and say how it lined up
Usage: subedit-cli pair [OPTIONS] main

Positionals:
  main TEXT REQUIRED          The main subtitle file

Options:
  -h,--help                   Print this help message and exit
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
| `<main>` | tak | **jedna** ścieżka | — |
| `-t`, `--translation-file` | **tak** | plik tłumaczenia | — |
| `--align-method` | nie | `position` lub `number` — zobacz [Tłumaczenie](invocation.md#tłumaczenie) | `position` |
| `--output` / `--output-dir` / `--in-place` | **jedna z trzech**, z wyjątkiem `--dry-run` | zobacz [Uruchamianie](invocation.md#miejsce-docelowe) | — |
| `--dry-run` | nie | flaga: oblicza i mówi, nie zapisuje — zobacz [Podgląd przed zapisem](invocation.md#podgląd-przed-zapisem) | wyłączona |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,300 --> 00:00:02,300\nBonjour.\n\n2\n00:00:03,300 --> 00:00:04,300\nAu revoir.\n\n' > film.fr.srt; subedit-cli pair film.srt -t film.fr.srt --output recale.fr.srt; cat recale.fr.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,300 --> 00:00:02,300\nBonjour.\n\n2\n00:00:03,300 --> 00:00:04,300\nAu revoir.\n\n' > film.fr.srt; subedit-cli pair film.srt -t film.fr.srt --output recale.fr.srt; cat recale.fr.srt
film.fr.srt: translation: 2 lines attached -> recale.fr.srt
1
00:00:01,000 --> 00:00:02,000
Bonjour.

2
00:00:03,000 --> 00:00:04,000
Au revoir.
```

Tłumaczenie spóźniało się o 0,3 s: każda linia znalazła napis, na który wypada jej środek, a
zapisany plik niesie pozycje pliku głównego.

## Co mówi dopasowanie

Zdanie jest **tym samym, które mówi okno** przy otwieraniu tłumaczenia, każda jego część tylko wtedy, gdy nie jest
zerem — tabela jego części jest w [`inspect -t`](inspect.md#tłumaczenie--t). **Jest zapisywane
od poziomu 1**, a nie od poziomu 2 jak w innych podpoleceniach: tutaj dopasowanie nie jest
szczegółem przetwarzania, jest wynikiem.

**Co dopasowanie robi z zapisywanym plikiem**:

- **napis, którego nikt nie przetłumaczył, jest zapisywany jako blok bez tekstu**, który odczytuje się bez anomalii;
- **napis zrodzony z linii** — linii, do której nic nie dotarło — jest zapisywany z **własnymi pozycjami**, niezależnie od
  metody: zapisany plik może więc zawierać więcej napisów niż plik główny;
- **wszystko, co dokument niósł jako tłumaczenie, jest czyszczone** przed nałożeniem, co nie dotyczy wiersza poleceń:
  plik główny jest odczytywany osobno.

**Dwie metody** dają ten sam plik, gdy każda linia trafia na swój napis, a nie
dają go, gdy brakuje linii lub tłumaczenie jest przesunięte — dlatego metoda jest wyborem i dlatego
domyślna jest pozycja. Osiem przypadków z `src/test/data/paires/` wylicza je wraz z plikiem oczekiwanym dla
każdej metody.

## Wynik

**Standardowe wyjście** — nic: wynikiem jest zapisany plik.

| Poziom | Co jest dodawane na wyjściu błędów |
| :----- | :------------------------------------- |
| 1 | `<translation>: translation: N lines attached; … -> <destination>` — **ścieżka jest ścieżką tłumaczenia**, czyli zapisywanego pliku; z `--dry-run` zamiast `-> <destination>` pojawia się `(dry run, nothing written)` |
| 2 | `<translation>: SubRip, UTF-8, no BOM, LF line endings kept` — format, kodowanie, znacznik i końce linii **tłumaczenia**, przywrócone takie, jakie były |
| 3 | `<translation>: N bytes read, M written`, potem **każdy komunikat diagnostyczny odczytu** pliku głównego i tłumaczenia — zobacz [Uruchamianie](invocation.md#odczyt-komunikatów-diagnostycznych) |

### W formacie JSON

Z opcją [`--format json`](invocation.md#wynik-czytelny-dla-skryptu) **jeden obiekt**, w którym `file` to tłumaczenie, a
`destination` to zapisany plik. `counts.subtitles` liczy **napisy zapisanego pliku**, łącznie z tymi, które
linia powołała do życia. Klucz `alignment` — `{"file", "method", "attached", "born", "untranslated", "out_of_order"}` —
niesie te same liczby co zdanie, tak jak klucz `translation` w [`inspect`](inspect.md#w-formacie-json).

## Kody wyjścia

Takie jak w całym narzędziu: `0`, jeśli tłumaczenie zostało zapisane — lub obliczone, z `--dry-run` —, `2`, jeśli któregoś z dwóch plików nie udało się
odczytać albo nie udało się zapisać miejsca docelowego, `1` przy błędzie użycia. **Źle dopasowane tłumaczenie nie jest
porażką**: jest zapisywane, a zdanie mówi, co nie znalazło swojego miejsca.

## Błędy

| Co to wywołuje | Komunikat |
| :------------------- | :------ |
| brak `-t` | `pair needs the translation file to lay over the main one: use -t` |
| `--document` | `The following argument was not expected: --document` — nie istnieje dla `pair` |
| więcej niż jeden plik główny | `The following argument was not expected: <path>` |
| tłumaczenie jest plikiem głównym | `<translation>: the file is already open as the main document`, kod `2` |
| brak któregoś z plików, nie da się go odczytać lub ma nieznany format | tak samo jak wszędzie, ze ścieżką wadliwego pliku |
