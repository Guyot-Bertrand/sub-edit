# Instalar li utensiles de transcription

<!-- machine-translated -->
> Ca págine es un traduction machinal e it ancor ne ha esset revidet de un lector del lingue.

Transcrir audio in subtitules — [`transcribe`](transcribe.md) in li linea de comande,
[`Transcribe Audio…`](../subedit-gui/video.md#transcrir-li-audio) in li fenestre — besona tri
cosas quel **subedit ne livra e ne discarga**: un motor, un model por ti motor, e
`ffmpeg`. Vu les installa, e vu dit a subedit u es li model.

**Nequo rumpe sin les.** Li resto del programma functiona quam ante; solmen li transcription
se desactiva, con un missage quel indica ci.

## Li motor: whisper.cpp

[whisper.cpp](https://github.com/ggml-org/whisper.cpp) es li motor quel subedit lance. It es un programma,
`whisper-cli`, quel functiona sur un processor ordinari — un carte graphic solmen fa it plu rapid. Compilar it
quam su propri documentation dit (un compilator de C++ e CMake sufficte):

```bash
git clone https://github.com/ggml-org/whisper.cpp
cd whisper.cpp
cmake -B build
cmake --build build -j --config Release
```

Li programma es tande `build/bin/whisper-cli`. Pone it u vor `PATH` lo trova, o **dona su
chemin** a subedit con li parametre `transcription.executable`. Un compilation por un carte graphic es fat
con li optiones quel su documentation dona; subedit lance li un quel vu ha instalat e selecte nequo
por vu.

## Un model

Li motor leer un **model**, un file de plu centenes de megabytes til un poc gigabytes quel vu
discarga un vice. Li models official es listat in
[huggingface.co/ggerganov/whisper.cpp](https://huggingface.co/ggerganov/whisper.cpp), e li
repositoria del motor have un script quel discarga un:

```bash
sh ./models/download-ggml-model.sh small
```

Poy dona su chemin con `--model` in li linea de comande, o con li parametre `transcription.model`,
o in li dialog del fenestre, quel memora it.

### Quel model por quel lingue

Whisper ne changea model unquande it ha trovat li lingue: **li model quel vu dona decide quo it
posse far.**

| Model | Linguas | Detecte li lingue | Grandore |
| :---- | :-------- | :------------------- | :--- |
| `tiny`, `base`, `small`, `medium`, `large-v3`, `large-v3-turbo` | circa cent | yes | de circa 75 MB til circa 3 GB |
| `tiny.en`, `base.en`, `small.en`, `medium.en` | solmen anglesi | no | quam li sam grandore ad-supra |

- **Por tot ma anglesi, prende un model multilingual** — un quel nómine ne have `.en`. Un model
  solmen anglesi refusa un altri lingue: li comande lo dit ante far qualcunc labor.
- **Li plu small models es clarmen pejor foris anglesi.** Por francesi, germanic, hispan e
  similar, `small` es un minimum, e `medium` o `large-v3-turbo` dona resultates mult plu bon al pretz
  de plu memorie e plu temps.
- **Li detection del lingue es solmen tam bon quam li model.** Un small model posse errar con un comense fat de
  musica o silentie; dit li lingue con `--language` quande vu save it.

## ffmpeg

[`ffmpeg`](https://ffmpeg.org/download.html) extrae li pista audio de un video, e `ffprobe`, quel vene
con it, lista li pistas. Instalar it con li gerente de paccages de vor distribution. Sin it, un file
audio quel li motor leer talmen (`wav`, `mp3`, `flac`, `ogg`) posse ancor esser transcrit, ma un video
ne.

## Verificar que it functiona

```bash
subedit-cli transcribe --model /path/to/ggml-small.bin --output film.srt film.mkv
```

Si alquo manca, li missage dit quo, e indica ci.
