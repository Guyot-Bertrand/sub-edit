# De transcriptiehulpmiddelen installeren

<!-- machine-translated -->
> Deze pagina is een machinevertaling en is nog niet nagelezen door een lezer van de taal.

Audio transcriberen naar ondertitels — [`transcribe`](transcribe.md) op de opdrachtregel,
[`Transcribe Audio…`](../subedit-gui/video.md#de-audio-transcriberen) in het venster — vereist drie
dingen die **subedit niet meelevert en niet downloadt**: een engine, een model voor die engine en
`ffmpeg`. U installeert ze, en u zegt aan subedit waar het model staat.

**Zonder deze dingen gaat er niets stuk.** De rest van het programma werkt zoals voorheen; alleen de transcriptie
schakelt zichzelf uit, met een melding die hiernaar verwijst.

## De engine: whisper.cpp

[whisper.cpp](https://github.com/ggml-org/whisper.cpp) is de engine die subedit uitvoert. Het is een programma,
`whisper-cli`, dat op een gewone processor draait — een grafische kaart maakt het alleen sneller. Bouw het
zoals de eigen documentatie zegt (een C++-compiler en CMake volstaan):

```bash
git clone https://github.com/ggml-org/whisper.cpp
cd whisper.cpp
cmake -B build -DBUILD_SHARED_LIBS=OFF
cmake --build build -j --config Release
```

Het programma is dan `build/bin/whisper-cli`. Gebouwd met `-DBUILD_SHARED_LIBS=OFF`, zoals hierboven, is het één enkel bestand dat geen bibliotheek naast zich nodig heeft, zodat u het overal naartoe kunt kopiëren. Zet het waar uw `PATH` het vindt, of **geef het pad ervan** aan subedit met de instelling `transcription.executable` — in het venster hebben de [instellingen](../subedit-gui/preferences.md#transcriptie) er een veld voor. Een build voor een grafische kaart voegt één optie toe aan de eerste `cmake`-regel: `-DGGML_CUDA=1` voor een NVIDIA-kaart, `-DGGML_VULKAN=1` voor de meeste andere; subedit voert uit wat u hebt geïnstalleerd en kiest niets voor u.

## Een model

De engine leest een **model**, een bestand van enkele honderden megabytes tot enkele gigabytes dat u
één keer downloadt. De officiële modellen staan vermeld op
[huggingface.co/ggerganov/whisper.cpp](https://huggingface.co/ggerganov/whisper.cpp), en de
repository van de engine heeft een script dat er een ophaalt:

```bash
sh ./models/download-ggml-model.sh small
```

Geef vervolgens het pad ervan op met `--model` op de opdrachtregel, of met de instelling `transcription.model`,
of in de [instellingen](../subedit-gui/preferences.md#transcriptie) van het venster, die het onthouden.

### Welk model voor welke taal

Whisper wisselt niet van model zodra het de taal heeft gevonden: **het model dat u opgeeft bepaalt wat het
kan.**

| Model | Talen | Detecteert de taal | Grootte |
| :---- | :-------- | :------------------- | :--- |
| `tiny`, `base`, `small`, `medium`, `large-v3`, `large-v3-turbo` | ongeveer honderd | ja | van ongeveer 75 MB tot ongeveer 3 GB |
| `tiny.en`, `base.en`, `small.en`, `medium.en` | alleen Engels | nee | als dezelfde grootte hierboven |

- **Neem voor alles behalve Engels een meertalig model** — een model waarvan de naam geen `.en` heeft. Een
  model dat alleen Engels kent weigert een andere taal: de opdracht zegt dat voordat er enig werk is gedaan.
- **De kleinste modellen zijn merkbaar slechter buiten het Engels.** Voor Frans, Duits, Spaans
  en dergelijke is `small` een minimum, en `medium` of `large-v3-turbo` geven veel betere resultaten ten koste
  van meer geheugen en meer tijd.
- **Taaldetectie is maar zo goed als het model.** Een klein model kan het mis hebben bij een begin van
  muziek of stilte; geef de taal op met `--language` wanneer u haar kent.

## ffmpeg

[`ffmpeg`](https://ffmpeg.org/download.html) haalt het audiospoor van een video eruit, en `ffprobe`, dat er
bij hoort, somt de sporen op. Installeer het met de pakketbeheerder van uw distributie. Zonder dit kan een
audiobestand dat de engine rechtstreeks leest (`wav`, `mp3`, `flac`, `ogg`) nog steeds worden getranscribeerd, maar een video
niet.

## Controleren of het werkt

```bash
subedit-cli transcribe --model /path/to/ggml-small.bin --output film.srt film.mkv
```

Als er iets ontbreekt, zegt de melding wat, en verwijst naar deze pagina.
