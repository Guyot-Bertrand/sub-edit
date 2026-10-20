# `hearing-impaired`

<!-- machine-translated -->
> Ta strona jest tłumaczeniem maszynowym i nie została jeszcze sprawdzona przez osobę władającą tym językiem.

```
subedit-cli hearing-impaired
                  (--output FILE | --output-dir DIR | --in-place)
                  [--dry-run]
                  [--recursive] [--parentheses]
                  [--document main|translation] [-t FILE] [--align-method position|number]
                  <file>...
```

Usuwa z całego pliku **opisy dla niesłyszących** — dźwięki opisane w nawiasach kwadratowych. **Nawiasy okrągłe domyślnie zostają**: używa się ich równie często
dla szeptanej kwestii, jak dla opisanego dźwięku, a ich usunięcie wymazałoby dialog. Opcja `--parentheses` usuwa je
również, w pliku, który opisuje dźwięki właśnie w nich.

Czego polecenie nie robi: nie usuwa tekstów piosenek między znakami liczb, imienia mówiącego
przed dwukropkiem, nie przywraca wielkich liter i nie poprawia błędów OCR. Te
wzorce należą do kreatora
[`Correct Texts…`](../subedit-gui/correct-texts.md) z okna, który wiersz
poleceń oferuje pod nazwą [`correct`](correct.md); a tutaj nic nie da się ustawić: przekształcenie
jest ustalone, nie konfigurowalne.

<!-- example: subedit-cli hearing-impaired --help -->
```console
$ subedit-cli hearing-impaired --help
Remove the sounds described between square brackets
Usage: subedit-cli hearing-impaired [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to clean

Options:
  -h,--help                   Print this help message and exit
  --parentheses               Also remove the sounds between parentheses, often whispered lines
  -r,--recursive              Take directories as inputs, and every subtitle file in them
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

| Opcja | Wymagana | Wartości | Domyślnie |
| :----- | :----- | :------ | :----- |
| `<file>...` | tak | jedna lub więcej ścieżek | — |
| `--parentheses` | nie | flaga: usuwa także opisy w nawiasach okrągłych | wyłączona |
| `--recursive`, `-r` | nie | flaga | wyłączona — zobacz [Przetwarzanie drzewa katalogów](lots.md) |
| `--document` | nie | `main` lub `translation` — zobacz [Tłumaczenie](invocation.md#tłumaczenie) | `main` |
| `-t`, `--translation-file` | z `--document translation` | plik tłumaczenia, dla **jednego** pliku wejściowego | — |
| `--align-method` | nie, i tylko z `-t` | `position` lub `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **jedna z trzech**, z wyjątkiem `--dry-run` | zobacz [Uruchamianie](invocation.md#miejsce-docelowe) | — |
| `--dry-run` | nie | flaga: oblicza i informuje, nic nie zapisuje — zobacz [Podgląd przed zapisem](invocation.md#podgląd-przed-zapisem) | wyłączona |

Format odczytanego pliku jest **zachowywany**: zmiana formatu to zadanie
podpolecenia [`convert`](convert.md).

**Domyślnie tekst główny; tłumaczenie z opcjami `--document translation -t FILE`.** Wtedy
zmieniane i zapisywane jest tłumaczenie, i tylko ono — zobacz [Tłumaczenie](invocation.md#tłumaczenie).

**Opis, który opróżniłby tłumaczenie, zapisuje je puste i zachowuje napis**: napis
zostaje w pliku, bez tekstu. Tekst główny natomiast traci napis opróżniony w całości, jak
niżej.

## Co jest usuwane, a co zostaje

Opis to nawias kwadratowy — lub okrągły, z opcją `--parentheses` — i to, co obejmuje, **łącznie z
podziałem linii**: w prawdziwych plikach opis bywa przecięty
końcem linii.

| Wejście | Wyjście |
| :----- | :----- |
| `Bonjour [il tousse] Marie` | `Bonjour Marie` |
| `Bonjour[il tousse]Marie` | `Bonjour Marie` |
| `[Bruit de pas] Bonjour` | `Bonjour` |
| `A [un] [deux] B` | `A B` |

Po usunięciu zostaje **dokładnie jedna spacja między tym, co go otaczało, i żadna na
brzegu linii**. To nie jest czyszczenie spacji w pliku: dwie spacje,
które tekst już zawierał, pozostają dwiema spacjami.

**Gdy opis obejmuje podział linii, to podział zostaje** — dwie
linie pozostają dwiema liniami. Ten przypadek nie jest ciekawostką: w piętnastu
prawdziwych zestawach napisów każdy nawias kwadratowy, który wyglądał na osierocony, był
opisem przeciętym w ten sposób.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nReculez ! [Il\nhurle] Tout de suite !\n\n' > a.srt; subedit-cli --quiet hearing-impaired --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nReculez ! [Il\nhurle] Tout de suite !\n\n' > a.srt; subedit-cli --quiet hearing-impaired --output b.srt a.srt; cat b.srt
1
00:00:01,000 --> 00:00:03,000
Reculez !
Tout de suite !
```

**Co nie jest opisem i nie jest ruszane:**

| Przypadek | Co się dzieje |
| :-- | :-------------- |
| zawartość czysto liczbowa — `[1]`, `(12)`, `[ 156478 ]` | zostaje bez zmian, razem z ogranicznikami: odwołanie nigdy nie nazywało dźwięku |
| nawias okrągły i to, co obejmuje, bez `--parentheses` | zostaje bez zmian: `Venez (chut) ici` się nie zmienia |
| nawias kwadratowy lub okrągły, którego nic nie zamyka | zostaje bez zmian |
| znacznik formatu — `<i>`, `</i>` | nigdy nie jest usuwany |

Odstęp **między** cyframi, litera lub znak — `[1 2]`, `[1a]`,
`[-1]` — sprawia, że jest to opis. Tak samo pusta zawartość, `[]` lub `( )`: brak
cyfry, więc to nie odwołanie; nic do powiedzenia, więc nic do zachowania.

## Co znika całkowicie

**Linia opróżniona przez usunięcie znika**, a **napis opróżniony
w całości jest usuwany z pliku**. Następujące po nim napisy są numerowane od nowa.

Linia jest uznawana za pustą, gdy zostają w niej tylko odstępy, znaczniki
formatu lub samotny myślnik dialogowy — `<i>[Musique]</i>` nie zostawia
na ekranie `<i></i>`.

**Znaczniki są znacznikami formatu pliku.** `{\i1}[Musique]{\i0}` w
Advanced SSA, `{Y:i}[Musique]` w MicroDVD i `/[Musique]` w MPL2 również nie zostawiają
niczego. Natomiast `<i>` w formacie, który nie zapisuje żadnego
znacznika — TMPlayer, LRC — jest tekstem czytanym przez widza, a linia, która
go zawiera, nie jest pusta.

**Dialog sprowadzony do jednego głosu przestaje nim być**, a pozostały myślnik
znika. Dopóki zostają dwa głosy, myślniki pozostają.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\nVoir [1] la note.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\nVoir [1] la note.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle cleaned, 1 removed -> b.srt
1
00:00:04,000 --> 00:00:06,000
Attends Marie.

2
00:00:07,000 --> 00:00:09,000
Voir [1] la note.
```

## Plik bez żadnego opisu też jest zapisywany

Zostaje skopiowany bez zmian, kod wyjścia to `0`, a raport mówi, że nie było
nic do usunięcia.

Taka jest reguła podpoleceń, które zapisują: **podane miejsce docelowe to
zapisane miejsce docelowe**. Zrobienie tu wyjątku zmuszałoby skrypt do wiedzy, które
z podpoleceń czasem tworzy plik, a czasem nic.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nRien à signaler.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nRien à signaler.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt
a.srt: no mention to remove -> b.srt
```

## Wynik

**Standardowe wyjście** — nic: wynikiem jest zapisany plik. **Z opcją `--dry-run` są to proponowane zmiany**: dla każdego napisu, którego tekst by się zmienił, jego numer, tekst przed i tekst po — zobacz [Podgląd przed zapisem](invocation.md#podgląd-przed-zapisem).

| Poziom | Co jest dodawane na wyjściu błędów |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles cleaned, M removed -> <destination>`, albo `<path>: no mention to remove -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — format, kodowanie, znacznik i końce linii odczytanego pliku, przywrócone takie, jakie były |
| 3 | `<path>: N bytes read, M written`, a następnie **każdy komunikat diagnostyczny odczytu** — zobacz [Uruchamianie](invocation.md#odczyt-komunikatów-diagnostycznych) |

`N` zlicza napisy, których tekst się zmienił, `M` — te, które usunięcie
opróżniło w całości i które dlatego opuściły plik.

### W formacie JSON

Z opcją [`--format json`](invocation.md#wynik-czytelny-dla-skryptu) obiekt każdego pliku zawiera `counts.cleaned` (przepisane teksty) i `counts.removed` (usunięte napisy); **z opcją `--dry-run`** klucz `changes` wylicza ponadto zmienione napisy, jak standardowe wyjście w postaci tekstowej. Pełna tabela znajduje się w rozdziale [Uruchamianie](invocation.md#co-każde-podpolecenie-umieszcza-w-counts).

## Kody wyjścia

Takie jak w całym narzędziu: `0`, jeśli każdy plik został zapisany, `2`, jeśli żaden, `3`, jeśli
tylko niektóre, `1` przy błędzie użycia.

## Błędy

To podpolecenie nie ma własnego błędu: nie przyjmuje żadnej
wartości do przeanalizowania i żaden tekst nie może go skłonić do odrzucenia pliku. Zawieść
może odczyt lub zapis, jak w pozostałych.

**Nic nie jest zapisywane bez jednej z trzech opcji miejsca docelowego**, a ich błędy
są wspólne dla podpoleceń, które zapisują:
zobacz [Uruchamianie](invocation.md#miejsce-docelowe).
