# `sync`

<!-- machine-translated -->
> Ta strona jest tłumaczeniem maszynowym i nie została jeszcze sprawdzona przez osobę władającą tym językiem.

```
subedit-cli sync (--video FILE | --audio FILE)
                 [--audio-track N]
                 [--language CODE]
                 [--start TIME] [--end TIME]
                 [--model FILE]
                 [--vad-model FILE]
                 [--engine NAME]
                 [--keep-audio FILE]
                 [--method METHOD]
                 [--min-match PERCENT]
                 [--doubt-gap MS]
                 [--report FILE]
                 [-r]
                 [--output FILE | --output-dir DIR | --in-place] [--dry-run]
                 <files>...
```

**Przesuwa napisy z pliku tak, by pasowały do dźwięku filmu.** Służy do przypadku, gdy `.srt` pobrany z internetu jest
przesunięty na początku i na końcu, czego nie naprawi jedno przesunięcie. Robią to dwie metody, a `--method` (lub
ustawienie `alignment.method`) wybiera między nimi:

- **`speech`, domyślna, nakłada plik na miejsca, w których ktoś mówi.** Pyta dźwięk tylko o to, gdzie jest mowa, i
  znajduje przesunięcie, które zgadza napisy z nią, fragment filmu po fragmencie: stałe przesunięcie, dryf, druga
  połowa, która się spóźnia, oraz plik przygotowany dla innej liczby klatek na sekundę (25 zamiast 23,976 i tym
  podobne) — wszystko zostaje naprawione. **Nie zależy od języka** — napisy mogą być po francusku, a film po
  angielsku —, nie wymaga transkrypcji i trwa kilka sekund. Potrzebuje `ffmpeg`, programu
  `whisper-vad-speech-segments`, który jest dołączany do whisper.cpp, oraz modelu wykrywania mowy (`--vad-model`,
  ustawienie `transcription.vad-model`).
- **`words` dopasowuje słowa każdego napisu do słów, które usłyszał silnik.** Słucha za pomocą silnika transkrypcji —
  tego samego co [`transcribe`](transcribe.md) — a każdy napis zajmuje miejsce, w którym padają jego słowa. Nic nie
  jest tłumaczone: napisy muszą być w języku, w którym się mówi. Potrafi umieścić pojedynczy napis, który jest nie na
  swoim miejscu, w pliku, który poza tym jest dobry, i trwa tyle, ile transkrypcja.

**Czego `speech` nie robi, jest mówione za każdym razem**, na wyjściu błędów i w zapisie: poprawia plik **jako
całość**, nigdy pojedynczy napis. Napis, który sam jest nie na swoim miejscu w pliku, który poza tym jest dobry, albo
tytuł pokazany nad ciszą (plansza rozdziału, podpis tłumaczący coś na ekranie), nie jest szukany i może nadal być
błędny, a pierwsze minuty mogą zostać przesunięte, gdy film zaczyna się muzyką lub napisami. Sprawdź wynik albo użyj
`--method words`, gdy tekst jest w języku, w którym się mówi.

**Nic z tego, czego potrzebuje, nie jest dostarczane z programem**: programy, model i `ffmpeg` to te z
[Instalowanie narzędzi do transkrypcji](install-transcription.md), a ich brak niczego nie psuje — polecenie mówi, czego
brakuje, z odesłaniem do tej strony, niczego nie zapisuje i kończy się niezerowym kodem.

- **Zmieniają się tylko początki i końce.** Nigdy tekst, styl, liczba napisów ani ich kolejność.
- **Napis, którego dźwięk nie potwierdza, zachowuje swoje czasy** — napisy końcowe, których nikt nie wypowiada, piosenka,
  wiersz w innym języku. **Żadna pozycja nie jest nigdy zmyślana ani zgadywana na podstawie sąsiadów**: wiersz zostaje
  tam, gdzie miał go plik, i polecenie o tym mówi. To *nie* jest porażka, a kod wyjścia to `0`.
- **Z `words` napis jest znaleziony, gdy usłyszano co najmniej połowę jego słów** (`--min-match`,
  `alignment.min-match`), i co najmniej jedno. Wielkość liter, akcenty, interpunkcja i znaczniki nie mają znaczenia;
  słowo różniące się o jedną literę od długiego słowa liczy się jako usłyszane. `speech` umieszcza każdy napis i nie
  przyjmuje żadnej z opcji `--language`, `--model`, `--engine` ani `--min-match`.
- **Koniec jest tam, gdzie kończy się ostatnie słowo.** Silnik mierzy czas słów w przybliżeniu, o kilka setnych sekundy
  przed własnym końcem napisu, więc napis może wyjść nieco krótszy niż był.
- **Napis, który zachodziłby na następny, jest skracany do miejsca, w którym zaczyna się następny.** Napis, który nadal
  zachodzi na następny, czego skracanie nie wyleczy, jest zostawiany i zgłaszany.
- **Plik, który nie pasuje do dźwięku, jest odrzucany i nie jest zapisywany**: z `words` nie znaleziono żadnego
  napisu, co niemal zawsze oznacza zły dźwięk lub zły język — `speech` nie zależy od języka —; z `speech` napisy i
  mowa mają zbyt mało wspólnego, co niemal zawsze oznacza zły film.
- **Można wysłuchać tylko część filmu**, za pomocą `--start` i `--end`, jak dla
  [`transcribe`](transcribe.md#transkrypcja-fragmentu): szukane i zmieniane są tylko napisy, które **zaczynają się** w
  tym zakresie, pozostałe zostają, jak są, i nie są liczone.
- **Dźwięk jest słuchany raz dla każdego podanego pliku.** Aby przesunąć kilka plików na tym samym filmie, trzeba
  poczekać na każdy z nich.

<!-- example: subedit-cli sync --help -->
```console
$ subedit-cli sync --help
Re-time the subtitles of a file on the audio of a film
Usage: subedit-cli sync [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to re-time

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --video FILE                Film to listen to
  --audio FILE                Audio file to listen to, instead of a film
  --audio-track N             Audio track of a film, counted from 0; the first by default
  --start TIME                Where to begin, as 2:00, 1:02:03 or seconds; the beginning by default
  --end TIME                  Where to stop, as 2:00, 1:02:03 or seconds; the end by default
  --language CODE             Language of the speech, two letters: fr, en, de; detected from the first 30 seconds by default
  --model FILE                Model file; the transcription.model setting by default
  --vad-model FILE            Voice-activity model that skips silences; the transcription.vad-model setting by default
  --engine NAME               Engine that listens; the transcription.engine setting by default
  --keep-audio FILE           Keep the extracted audio in this file
  --method METHOD             How the subtitles are put in line: speech (from when there is speech, whatever the language) or words (from the words heard); the alignment.method setting by default
  --min-match PERCENT         Share of a subtitle's words that must be heard, in percent; the alignment.min-match setting by default
  --doubt-gap MS              How far a subtitle may move from its neighbours' shift before it is flagged, in milliseconds; the alignment.doubt-gap setting by default
  --report FILE               Write the subtitles to check to this file
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Argumenty i opcje

| Opcja | Wymagane | Wartości | Domyślnie |
| :----- | :------- | :----- | :------ |
| `<files>` | tak | pliki napisów do przesunięcia; z `-r` katalogi | — |
| `--recursive`, `-r` | nie | flaga | wyłączona — zobacz [Przetwarzanie drzewa katalogów](lots.md) |
| `--video` | jedno z `--video` i `--audio` | film do wysłuchania | — |
| `--audio` | jedno z `--video` i `--audio` | plik dźwiękowy do wysłuchania zamiast filmu | — |
| `--audio-track` | nie | liczba całkowita od 0: pozycja wśród ścieżek dźwiękowych filmu | `0`, pierwsza |
| `--language` | nie | dwie litery, kod ISO 639-1: `en`, `fr` | ustawienie `transcription.language`; puste: wykrywany |
| `--start`, `--end` | nie | czas: `2:00`, `1:02:03`, `2:00.5` lub liczba sekund; `--end` musi następować po `--start` | cały dźwięk |
| `--model` | nie | plik modelu | ustawienie `transcription.model` |
| `--vad-model` | nie | model wykrywania mowy, który sprawia, że silnik pomija ciszę; z nim słowa dokładniej podążają za mową | ustawienie `transcription.vad-model` |
| `--engine` | nie | nazwa silnika, który słucha: `whisper.cpp` | ustawienie `transcription.engine` |
| `--keep-audio` | nie | plik, w którym zachować wyodrębniony dźwięk; jego katalog jest tworzony, gdy go brak | dźwięk nie jest nigdzie zachowywany |
| `--method` | nie | `speech`: nałożyć plik na miejsca, w których ktoś mówi, w dowolnym języku; `words`: dopasować słowa do usłyszanych, w języku, w którym się mówi | ustawienie `alignment.method`, `speech` |
| `--min-match` | nie | liczba całkowita od 1 do 100: udział słów napisu, które muszą zostać usłyszane, w procentach | ustawienie `alignment.min-match`, `50` |
| `--doubt-gap` | nie | liczba całkowita milisekund, 0 lub więcej: o ile przesunięcie napisu może odbiegać od przesunięcia sąsiadów, zanim zostanie oznaczony | ustawienie `alignment.doubt-gap`, `2000` |
| `--report` | nie | plik, w którym zapisać napisy do sprawdzenia, po jednym w wierszu: numer napisu, tabulator i powód; naraz jeden plik napisów | nic nie jest zapisywane |
| `--output` / `--output-dir` / `--in-place` | **jedna z trzech**, z wyjątkiem `--dry-run` | zobacz [Uruchamianie](invocation.md#miejsce-docelowe) | — |
| `--dry-run` | nie | flaga: oblicza i mówi, nie zapisuje — zobacz [Podgląd przed zapisem](invocation.md#podgląd-przed-zapisem) | wyłączona |

Miejsce docelowe — `--output`, `--output-dir`, `--in-place`, `--dry-run` — oraz `-r` są takie jak w innych poleceniach,
które przepisują plik: zobacz [miejsce docelowe](invocation.md#miejsce-docelowe). Plik zachowuje swój format, kodowanie
i zakończenia wierszy.

**Ustawienia** są odczytywane z pliku ustawień — zobacz [plik ustawień](invocation.md#plik-ustawień) — i każde ma tu
swoją opcję. Silnik, model i model wykrywania mowy są te same co w `transcribe`.

| Klucz | Znaczenie | Domyślnie |
| :-- | :------ | :------ |
| `alignment.engine` | silnik dopasowania metody `words`; jest jeden, `transcript-matching`, który słucha i dopasowuje | `transcript-matching` |
| `alignment.method` | jak napisy są dopasowywane: `speech` lub `words` | `speech` |
| `alignment.min-match` | udział słów napisu, które muszą zostać usłyszane, by został znaleziony, w procentach | `50` |
| `alignment.doubt-gap` | o ile, w milisekundach, przesunięcie napisu może odbiegać od przesunięcia sąsiadów, zanim zostanie oznaczony | `2000` |

## Co jest oznaczane

Polecenie mówi, które napisy należy sprawdzić i dlaczego. Napis jest oznaczany z **pierwszego** z tych powodów, który
zachodzi:

| Dlaczego | Znaczenie |
| :-- | :------ |
| not found in the audio, left as it was | usłyszano mniej niż `--min-match` jego słów: zachowuje swoje czasy |
| only partly heard | znaleziony, ale usłyszano mniej niż 80 % jego słów |
| moved differently from its neighbours | znaleziony, ale jego przesunięcie odbiega od środkowego przesunięcia dwóch napisów z każdej strony o więcej niż `--doubt-gap` |

Z `speech` każdy napis jest umieszczany i może zajść tylko ostatni z tych powodów: tam, gdzie plik zmienia swoje
przesunięcie o więcej niż `--doubt-gap` między jednym napisem a następnym (druga połowa, która się spóźnia), napisy po
obu stronach skoku są oznaczane.

Napis może też zostać **skrócony do początku następnego napisu** albo **nadal zachodzić na następny napis**.

Dwie liczby opowiadają historię całego pliku. **Mediana przesunięcia** to środek tego, o ile przesunęły się napisy:
plik, który po prostu się spóźniał, widać tu i nigdzie indziej. **Dryf** to mediana przesunięcia ostatniej dziesiątej
części napisów minus mediana pierwszej dziesiątej: jest bliski zera przy stałym przesunięciu i duży dla pliku, który
spóźniał się coraz bardziej w trakcie filmu.

## Wynik

**Standardowe wyjście** — nic: wynikiem jest zapisany plik.

| Poziom | Co jest dodawane na wyjściu błędów |
| :---- | :------------------------------- |
| 1 | `<file>: N of M subtitles placed on the audio, K to check; median shift X s, drift Y s -> <destination>` — a gdy poprawiono liczbę klatek na sekundę, także stosunek; z `speech` **ostrzeżenie o tym, czego nie robi**, w osobnym wierszu |
| 2 | etap i postęp, jak dla [`transcribe`](transcribe.md#wynik) |

### W formacie JSON

Z [`--format json`](invocation.md#wynik-czytelny-dla-skryptu) obiekt pliku zawiera `counts`: `subtitles`
(szukane), `placed` (którym nadano miejsce), `moved` (których czasy się zmieniły), `partial`, `out_of_line`, `not_found`,
`median_shift_ms` i `drift_ms`; oraz `flagged`, listę `{"subtitle": N, "kind": "…"}`, gdzie numer to miejsce napisu w
pliku, a `kind` to jedno z `not-found`, `partial`, `out-of-line`, `cut-back` i `overlaps`. Z `speech` `warnings` zawiera jeden wpis o `kind` równym `speech-only`. Identyfikatory są
gwarantowane; zdania nie.

## Przykład

```bash
subedit-cli sync --video film.mkv --vad-model silero.bin \
    --report to-check.txt --output film.synced.srt film.srt
```

Aby umieścić każdy napis według jego słów, dla tekstu w języku, w którym się mówi:

```bash
subedit-cli sync --method words --video film.mkv --model small.bin --vad-model silero.bin \
    --output film.synced.srt film.srt
```

## Kody wyjścia

Takie jak w narzędziu: `0`, jeśli plik został zapisany, z oznaczonymi napisami lub bez, `2`, jeśli nie mógł być —
brakujące narzędzie lub model, nieudane wyodrębnienie lub słuchanie, plik, który nie pasuje do dźwięku (nie znaleziono
żadnego jego napisu albo zbyt mało wspólnego z mową) —, `1` przy błędzie użycia, takim jak brak zarówno `--video`, jak i `--audio` (lub podanie obu), opcja poza zakresem lub zakres, którego nie
można użyć.

## Błędy

| Przyczyna | Komunikat |
| :---- | :------ |
| ani `--video`, ani `--audio`, albo oba | `sync needs the audio to listen to: give --video or --audio, and not both` |
| nie znaleziono programu silnika albo skonfigurowanego nie ma | `the transcription engine was not found`, potem odesłanie do przewodnika |
| nie podano modelu albo go nie ma | `no model was given, or the model file is not there`, potem odesłanie do przewodnika |
| nie znaleziono `ffmpeg` dla filmu | `ffmpeg was not found, so the audio of a video cannot be extracted`, potem odesłanie |
| ścieżka, której film nie ma | `the video has no audio track N`, potem ścieżki, które ma |
| język, którego model nie może transkrybować | `the model cannot transcribe the language xx` |
| silnik, którego nikt nie zarejestrował | `no transcription engine is called NAME` albo `there is no alignment engine called NAME` |
| nie znaleziono żadnego napisu (`words`) | `none of the N subtitles was found in the audio: is it the right audio, and the right language?`, potem `The method based on speech does not depend on the language.` |
| nie podano modelu wykrywania mowy albo nie znaleziono programu, który znajduje mowę (`speech`) | `Finding where there is speech needs a voice-activity model: name one in the transcription.vad-model setting.` albo `Finding where there is speech needs the program whisper-vad-speech-segments, which comes with whisper.cpp, and it was not found.`, potem odesłanie do przewodnika |
| napisy i mowa mają zbyt mało wspólnego (`speech`) | `The sound and the subtitles have too little in common to be put in line: is it the right film?` |
| `--min-match` poza zakresem 1 do 100, `--doubt-gap` ujemne | błąd użycia opcji |
| `--report` z więcej niż jednym plikiem napisów | `--report names one file: give one subtitle file when you use it` |
