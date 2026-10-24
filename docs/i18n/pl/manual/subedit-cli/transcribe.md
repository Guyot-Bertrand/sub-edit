# `transcribe`

<!-- machine-translated -->
> Ta strona jest tłumaczeniem maszynowym i nie została jeszcze sprawdzona przez osobę władającą tym językiem.

```
subedit-cli transcribe --output FILE
                       [--to FORMAT]
                       [--audio-track N]
                       [--language CODE]
                       [--start TIME] [--end TIME]
                       [--model FILE]
                       [--engine NAME]
                       [--keep-audio FILE]
                       [--raw]
                       <input>
```

Zapisuje **mowę z filmu lub z pliku dźwiękowego jako plik napisów**. Dźwięk jest przygotowywany,
przekazywany silnikowi, który znajduje, co jest mówione i kiedy, a fragmenty, które silnik wycina, są łączone w
napisy mieszczące się w określonym czasie trwania i długości.

**Nic z tego, czego potrzebuje, nie jest dostarczane z programem**: silnik (whisper.cpp), model i `ffmpeg`
instaluje użytkownik, który wskazuje, gdzie jest model. [Instalowanie narzędzi do
transkrypcji](install-transcription.md) mówi, gdzie znaleźć każde z nich i jaki model wybrać. **Ich brak niczego
nie psuje**: polecenie mówi, czego brakuje, z odesłaniem do tej strony, niczego nie zapisuje i
kończy się niezerowym kodem.

- **Wejściem jest film lub plik dźwiękowy.** Z filmu `ffmpeg` wyodrębnia jedną ścieżkę dźwiękową; bez
  `ffmpeg` plik dźwiękowy, który silnik czyta bezpośrednio (`wav`, `mp3`, `flac`, `ogg`), jest mimo to
  transkrybowany, ale film nie, ani żaden plik, gdy zażądano ścieżki.
- **Tylko jeden plik naraz**, a miejscem docelowym jest `--output`: nie ma `--output-dir` ani
  `--in-place`, ponieważ wynikiem jest nowy plik, a nie przepisanie istniejącego.
- **Język jest wykrywany**, chyba że `--language` go narzuci, a użyty język jest podawany. Model
  przeznaczony tylko do angielskiego niczego nie wykrywa i nie transkrybuje nic innego: język, którego nie może
  przyjąć, jest odrzucany *przed* rozpoczęciem pracy, z podaniem ograniczenia modelu.
- **Można transkrybować sam fragment filmu**, za pomocą `--start` i `--end` — zobacz
  [Transkrypcja fragmentu](#transkrypcja-fragmentu). Tak próbuje się model na dwóch minutach, zanim uruchomi się go na
  dwóch godzinach.
- **Segmenty są łączone i cięte** zgodnie z limitami z ustawień (`transcription.merge-gap-ms`,
  `transcription.max-duration-ms`, `transcription.max-characters`): dwa fragmenty jednego zdania, które leżą
  blisko siebie, tworzą jeden napis, a fragment zbyt długi lub zbyt szeroki jest cięty na słowie. **`--raw`
  zapisuje segmenty silnika tak, jak przyszły**, po jednym na napis, co jest też sposobem, by zobaczyć, co
  robi łączenie.

<!-- example: subedit-cli transcribe --help -->
```console
$ subedit-cli transcribe --help
Write the speech of a video or an audio file as subtitles
Usage: subedit-cli transcribe [OPTIONS] input

Positionals:
  input TEXT REQUIRED         Video or audio file to transcribe

Options:
  -h,--help                   Print this help message and exit
  --output FILE               Subtitle file to write
  --to TEXT:{srt,vtt,subviewer2,ssa,ass,microdvd,mpl2,tmplayer,lrc}
                              Format to write; the one the extension of --output says by default
  --audio-track N             Audio track of a video, counted from 0; the first by default
  --start TIME                Where to begin, as 2:00, 1:02:03 or seconds; the beginning by default
  --end TIME                  Where to stop, as 2:00, 1:02:03 or seconds; the end by default
  --language CODE             Language of the speech: fr, en; detected by default
  --model FILE                Model file; the transcription.model setting by default
  --engine NAME               Engine; the transcription.engine setting by default
  --keep-audio FILE           Keep the extracted audio in this file
  --raw                       Write the segments as the engine cut them
```

## Argumenty i opcje

| Opcja | Wymagane | Wartości | Domyślnie |
| :----- | :----- | :------ | :----- |
| `<input>` | tak | film lub plik dźwiękowy | — |
| `--output` | tak | plik napisów do zapisania; jego katalog jest tworzony, gdy go brak | — |
| `--to` | nie | krótka nazwa formatu: `srt`, `vtt`, `subviewer2`, `ssa`, `ass`, `microdvd`, `mpl2`, `tmplayer`, `lrc` | format wskazany przez rozszerzenie `--output`, w przeciwnym razie `srt` |
| `--audio-track` | nie | liczba całkowita od 0: pozycja wśród ścieżek dźwiękowych filmu | `0`, pierwsza |
| `--language` | nie | dwie małe litery, kod ISO 639-1: `en`, `fr` | ustawienie `transcription.language`; puste: wykrywany |
| `--start` | nie | czas: `2:00`, `1:02:03`, `2:00.5` lub liczba sekund, `90` | początek dźwięku |
| `--end` | nie | czas, zapisany jak dla `--start`; musi następować po `--start` | koniec dźwięku |
| `--model` | nie | plik modelu | ustawienie `transcription.model` |
| `--engine` | nie | nazwa silnika: `whisper.cpp` | ustawienie `transcription.engine` |
| `--keep-audio` | nie | plik, w którym zachować wyodrębniony dźwięk, jako 16-bitowy mono PCM 16 kHz w pliku WAV; jego katalog jest tworzony, gdy go brak | dźwięk nie jest nigdzie zachowywany |
| `--raw` | nie | flaga: zapisuje segmenty silnika bez łączenia i bez cięcia | wyłączona |

**Ustawienia** są odczytywane z pliku ustawień tak jak ustawienia [`adjust`](adjust.md) — zobacz
[plik ustawień](invocation.md#plik-ustawień) — i każde ma tu swoją opcję:

| Klucz | Znaczenie | Domyślnie |
| :-- | :------ | :------ |
| `transcription.engine` | silnik | `whisper.cpp` |
| `transcription.executable` | program silnika, ścieżka bezwzględna; puste: szukany w `PATH` | puste |
| `transcription.model` | plik modelu, ścieżka bezwzględna | puste |
| `transcription.language` | język dźwięku, dwie litery; puste: wykrywany | puste |
| `transcription.merge-gap-ms` | fragmenty jednego zdania położone bliżej niż ta liczba milisekund są łączone | `400` |
| `transcription.max-duration-ms` | najdłuższy czas, przez jaki napis pozostaje na ekranie, w milisekundach | `7000` |
| `transcription.max-characters` | największa liczba znaków napisu, bez liczenia podziałów wierszy | `84` |

## Wybór dźwięku

Film może mieć kilka ścieżek dźwiękowych — oryginał, komentarz, inny język. `--audio-track`
przyjmuje **pozycję wśród ścieżek dźwiękowych** pliku, od 0, w kolejności, w jakiej plik je wymienia.
Ścieżka, której film nie ma, jest odrzucana, **wraz z ścieżkami, które ma**, tak aby następne wywołanie było
właściwe:

```bash
subedit-cli transcribe --audio-track 4 --output film.srt --model small.bin film.mkv
```

```text
film.mkv: the video has no audio track 4
audio track 0: fra, Original
audio track 1: eng, Commentary
```

## Transkrypcja fragmentu

`--start` i `--end` ograniczają pracę do fragmentu filmu między nimi. Można podać tylko jedno z nich: samo
`--start` sięga do końca, samo `--end` zaczyna od początku.

```bash
subedit-cli transcribe --start 2:00 --end 3:30 --output test.srt --model large-v3.bin film.mkv
```

- **Czas zapisuje się jako `M:SS` lub `H:MM:SS`, w obu przypadkach z częścią dziesiętną po kropce (`2:00.5`), albo jako liczbę
  sekund (`90`, `90.5`)**. Przecinek jest odrzucany, jak wszędzie w narzędziu.
- **Napisy zachowują czas filmu**: kwestia wypowiedziana w 2:10 filmu jest zapisana jako `00:02:10`, a nie jako
  `00:00:10`. Plik można nałożyć na cały film bez zmian.
- **Brzegi są przybliżone**: silnik pracuje na oknach po trzydzieści sekund, więc to, co zwraca, jest potem przycinane
  do zakresu; zdanie leżące na brzegu jest tam ucinane.
- **Nic się nie zaczyna, jeśli zakresu nie można użyć**: granica, która nie jest czasem, lub `--end`, które nie następuje po
  `--start`, to błąd użycia, który nazywa problem, niczego nie zapisuje i nie uruchamia ani `ffmpeg`, ani
  silnika.

Dźwięk całego filmu i tak jest wyodrębniany: oszczędza się transkrypcję, znacznie dłuższą.

## Wynik

**Standardowe wyjście** — nic: wynikiem jest zapisany plik.

| Poziom | Co jest dodawane na wyjściu błędów |
| :----- | :------------------------------------- |
| 1 | `<input>: N subtitles transcribed, language: xx -> <output>` |
| 2 | etap i postęp, jeden wiersz na każdy procent: `extracting the audio…`, potem `transcribing…`, potem `transcribing: 25%, 0:12:30 of 0:50:00, about 38 min left` — procent, osiągnięta pozycja w dźwięku i pozostały czas |
| 3 | oraz każdy nowy tekst, który silnik otrzymuje, `last text: …` |

**Procent to pozycja silnika w dźwięku, a nie liczba jego własnych okien**: silnik zapisuje to, co przetranskrybował, na bieżąco,
a polecenie odczytuje, dokąd to dotarło, tak że liczba rusza od pierwszej minuty. (Własny postęp silnika przychodzi co 5 % dźwięku,
po ukończeniu okna trzydziestosekundowego: sam może stać na 0 % przez kwadrans na długim filmie z dużym modelem na procesorze.)
**Pozostały czas milczy na początku**: jest podawany, gdy zrobiono dość pracy, by dało się ekstrapolować, i podąża za tempem
ostatniej minuty, a nie za średnią z całego przebiegu.

### W formacie JSON

Z [`--format json`](invocation.md#wynik-czytelny-dla-skryptu) (opcja globalna, zapisywana przed
podpoleceniem) obiekt pliku zawiera użyty język w `language`, `counts.subtitles` (zapisane
napisy) i `counts.segments` (fragmenty silnika, z których je zrobiono) oraz `audio`, gdy
`--keep-audio` zachowało dźwięk.

## Kody wyjścia

Takie jak w narzędziu: `0`, jeśli plik został zapisany, `2`, jeśli nie mógł być — brakujące narzędzie lub model, język,
którego model nie może przyjąć, nieudane wyodrębnienie lub transkrypcja —, `1` przy błędzie użycia, takim jak
silnik, którego nikt nie zarejestrował, lub zakres, którego nie można użyć.

## Błędy

| Przyczyna | Komunikat |
| :---- | :------ |
| nie znaleziono programu silnika albo skonfigurowanego nie ma | `the transcription engine was not found`, potem odesłanie do przewodnika |
| nie podano modelu albo go nie ma | `no model was given, or the model file is not there`, potem odesłanie do przewodnika |
| nie znaleziono `ffmpeg` dla filmu | `ffmpeg was not found, so the audio of a video cannot be extracted`, potem odesłanie |
| ścieżka, której film nie ma | `the video has no audio track N`, potem ścieżki, które ma |
| język, którego model nie może transkrybować | `the model cannot transcribe the language xx` |
| brak języka i model, który nie potrafi go wykryć | `the model cannot detect the language: give one` |
| silnik, którego nikt nie zarejestrował | `no transcription engine is called NAME` |
| granica zakresu, która nie jest czasem | `"TEXT" is not a time: write it as 2:00, 1:02:03 or 2:00.5, or in seconds` |
| `--end` nie po `--start` | `the end of the range must come after its start` |
| każda inna porażka | `the transcription failed`, z tym, co powiedział silnik |

Odesłaniem do przewodnika jest `See the manual page "Installing the transcription tools"
(install-transcription).`
