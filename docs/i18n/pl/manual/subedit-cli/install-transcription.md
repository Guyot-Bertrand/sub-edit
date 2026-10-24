# Instalowanie narzędzi do transkrypcji

<!-- machine-translated -->
> Ta strona jest tłumaczeniem maszynowym i nie została jeszcze sprawdzona przez osobę władającą tym językiem.

Transkrypcja dźwięku na napisy — [`transcribe`](transcribe.md) w wierszu poleceń,
[`Transcribe Audio…`](../subedit-gui/video.md#transkrypcja-dźwięku) w oknie — wymaga trzech
rzeczy, których **subedit nie dostarcza i nie pobiera**: silnika, modelu dla tego silnika oraz
`ffmpeg`. Instalujesz je sam i wskazujesz programowi subedit, gdzie jest model.

**Bez nich nic się nie psuje.** Reszta programu działa jak dotąd; tylko transkrypcja
sama się wyłącza, z komunikatem odsyłającym tutaj.

## Silnik: whisper.cpp

[whisper.cpp](https://github.com/ggml-org/whisper.cpp) to silnik, który uruchamia subedit. Jest to program,
`whisper-cli`, działający na zwykłym procesorze — karta graficzna tylko go przyspiesza. Zbuduj go
tak, jak mówi jego własna dokumentacja (wystarczą kompilator C++ i CMake):

```bash
git clone https://github.com/ggml-org/whisper.cpp
cd whisper.cpp
cmake -B build
cmake --build build -j --config Release
```

Programem jest wtedy `build/bin/whisper-cli`. Umieść go tam, gdzie znajdzie go `PATH`, albo **podaj jego
ścieżkę** programowi subedit w ustawieniu `transcription.executable`. Wersję dla karty graficznej buduje się
z opcjami podanymi w jego dokumentacji; subedit uruchamia to, co zainstalowałeś, i niczego nie wybiera
za ciebie.

## Model

Silnik czyta **model**, plik o rozmiarze od kilkuset megabajtów do kilku gigabajtów, który
pobierasz raz. Oficjalne modele są wymienione pod adresem
[huggingface.co/ggerganov/whisper.cpp](https://huggingface.co/ggerganov/whisper.cpp), a
repozytorium silnika ma skrypt, który pobiera jeden z nich:

```bash
sh ./models/download-ggml-model.sh small
```

Następnie podaj jego ścieżkę w `--model` w wierszu poleceń albo w ustawieniu `transcription.model`,
albo w oknie dialogowym, które ją zapamiętuje.

### Który model dla którego języka

Whisper nie zmienia modelu po znalezieniu języka: **model, który podasz, decyduje o tym, co
potrafi.**

| Model | Języki | Wykrywa język | Rozmiar |
| :---- | :-------- | :------------------- | :--- |
| `tiny`, `base`, `small`, `medium`, `large-v3`, `large-v3-turbo` | około stu | tak | od około 75 MB do około 3 GB |
| `tiny.en`, `base.en`, `small.en`, `medium.en` | tylko angielski | nie | taki sam jak powyżej |

- **Do wszystkiego poza angielskim weź model wielojęzyczny** — taki, którego nazwa nie ma `.en`. Model
  tylko angielski odrzuca inny język: polecenie mówi o tym przed rozpoczęciem jakiejkolwiek pracy.
- **Najmniejsze modele są wyraźnie gorsze poza angielskim.** Dla francuskiego, niemieckiego, hiszpańskiego
  i podobnych `small` to minimum, a `medium` lub `large-v3-turbo` dają znacznie lepsze wyniki za cenę
  większej pamięci i większej ilości czasu.
- **Wykrywanie języka jest tak dobre, jak model.** Mały model może się pomylić na początku złożonym z
  muzyki lub ciszy; podaj język w `--language`, gdy go znasz.

## ffmpeg

[`ffmpeg`](https://ffmpeg.org/download.html) wyodrębnia ścieżkę dźwiękową filmu, a `ffprobe`, który
jest w pakiecie, wymienia ścieżki. Zainstaluj go menedżerem pakietów swojej dystrybucji. Bez niego plik
dźwiękowy, który silnik czyta bezpośrednio (`wav`, `mp3`, `flac`, `ogg`), nadal można transkrybować, ale filmu
nie.

## Sprawdzenie, czy działa

```bash
subedit-cli transcribe --model /path/to/ggml-small.bin --output film.srt film.mkv
```

Jeśli czegoś brakuje, komunikat mówi czego i odsyła do tej strony.
