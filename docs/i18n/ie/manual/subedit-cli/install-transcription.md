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
cmake -B build -DBUILD_SHARED_LIBS=OFF
cmake --build build -j --config Release
```

Li programma es tande `build/bin/whisper-cli`. Compilat con `-DBUILD_SHARED_LIBS=OFF`, quam ci-supra, it es un singul file quel besona null biblioteca apu it, dunc vu posse copiar it u vu vole. Pone it u vor `PATH` lo trova, o **dona su chemin** a subedit con li parametre `transcription.executable` — in li fenestre, li [preferenties](../subedit-gui/preferences.md#transcription) have un camp por to. Un compilation por un carte graphic adde un optione a li unesim linea de `cmake`: `-DGGML_CUDA=1` por un carte NVIDIA, `-DGGML_VULKAN=1` por li plu de them altri; subedit lance li un quel vu ha instalat e selecte nequo por vu.

Por conservar solmen li programma, **installa** it e deleta li clon:

```bash
cmake --install build --prefix ~/.local
```

`--prefix` dit u. `~/.local` besona null jure de administrator, e su fólder `bin` deve esser in vor `PATH`; sin li optione, li predefinition es `/usr/local`, quel demanda `sudo`. `whisper-cli` arrive in li fólder `bin` de ti prefix, apu utensiles quel subedit ne usa; quande it es ta, li clon e su fólder `build` posse partir.

## Un model

Li motor leer un **model**, un file de plu centenes de megabytes til un poc gigabytes quel vu
discarga un vice. Li models official es listat in
[huggingface.co/ggerganov/whisper.cpp](https://huggingface.co/ggerganov/whisper.cpp), e li
repositoria del motor have un script quel discarga un:

```bash
sh ./models/download-ggml-model.sh small
```

Poy dona su chemin con `--model` in li linea de comande, o con li parametre `transcription.model`,
o in li [preferenties](../subedit-gui/preferences.md#transcription) del fenestre, quel memora les.

### Un model de activitá de voce, por comensar u comensa li parol

Optionalmen, un **model de activitá de voce** (Silero) lassa li machine saltar li silentias ante decodar. Sin it, whisper comensa su unesim subtitul a 0, quo ci que veni prim in li film, musica includet, e arrotunda su tempores a tot fenestres; con it, li unesim subtitul comensa u comensa li parol e li altris sequa it. Li deposito del machine have un script quel obtene un:

```bash
sh ./models/download-vad-model.sh silero-v6.2.0
```

Dona su chemin con `--vad-model`, con li parametre `transcription.vad-model`, o in li [preferenties](../subedit-gui/preferences.md#transcription) del fenestre.

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
