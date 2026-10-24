# Instal·lació de les eines de transcripció

<!-- machine-translated -->
> Aquesta pàgina és una traducció automàtica i encara no l'ha revisada cap lector de la llengua.

Transcriure l'àudio en subtítols — [`transcribe`](transcribe.md) a la línia d'ordres,
[`Transcribe Audio…`](../subedit-gui/video.md#transcriure-làudio) a la finestra — necessita tres
coses que **subedit no lliura ni baixa**: un motor, un model per a aquest motor, i
`ffmpeg`. Les instal·leu vosaltres, i dieu a subedit on és el model.

**Res no es trenca sense elles.** La resta del programa funciona com abans; només la transcripció
es desactiva, amb un missatge que remet aquí.

## El motor: whisper.cpp

[whisper.cpp](https://github.com/ggml-org/whisper.cpp) és el motor que executa subedit. És un programa,
`whisper-cli`, que funciona en un processador corrent — una targeta gràfica només el fa més ràpid. Compileu-lo
com diu la seva pròpia documentació (n'hi ha prou amb un compilador de C++ i CMake):

```bash
git clone https://github.com/ggml-org/whisper.cpp
cd whisper.cpp
cmake -B build -DBUILD_SHARED_LIBS=OFF
cmake --build build -j --config Release
```

El programa és llavors `build/bin/whisper-cli`. Compilat amb `-DBUILD_SHARED_LIBS=OFF`, com més amunt, és un sol fitxer que no necessita cap biblioteca al costat, de manera que el podeu copiar on vulgueu. Poseu-lo allà on el trobi el vostre `PATH`, o **doneu-ne el camí** a subedit amb el paràmetre `transcription.executable` — a la finestra, les [preferències](../subedit-gui/preferences.md#transcripció) en tenen un camp. Una compilació per a una targeta gràfica afegeix una opció a la primera línia de `cmake`: `-DGGML_CUDA=1` per a una targeta NVIDIA, `-DGGML_VULKAN=1` per a la majoria de les altres; subedit executa el que hàgiu instal·lat i no tria res per vosaltres.

## Un model

El motor llegeix un **model**, un fitxer d'unes quantes centenes de megabytes a alguns gigabytes que
baixeu una sola vegada. Els models oficials estan llistats a
[huggingface.co/ggerganov/whisper.cpp](https://huggingface.co/ggerganov/whisper.cpp), i el
repositori del motor té un script que n'obté un:

```bash
sh ./models/download-ggml-model.sh small
```

Després doneu-ne el camí amb `--model` a la línia d'ordres, o amb el paràmetre `transcription.model`,
o a les [preferències](../subedit-gui/preferences.md#transcripció) de la finestra, que se'n recorden.

### Quin model per a quina llengua

Whisper no canvia de model un cop ha trobat la llengua: **el model que doneu decideix què
pot fer.**

| Model | Llengües | Detecta la llengua | Mida |
| :---- | :-------- | :------------------- | :--- |
| `tiny`, `base`, `small`, `medium`, `large-v3`, `large-v3-turbo` | un centenar | sí | d'uns 75 MB a uns 3 GB |
| `tiny.en`, `base.en`, `small.en`, `medium.en` | només anglès | no | com la mateixa mida de dalt |

- **Per a qualsevol cosa que no sigui anglès, preneu un model multilingüe** — un el nom del qual no tingui `.en`. Un
  model només d'anglès refusa una altra llengua: l'ordre ho diu abans de fer cap feina.
- **Els models més petits són notablement pitjors fora de l'anglès.** Per al francès, l'alemany, l'espanyol i
  similars, `small` és un mínim, i `medium` o `large-v3-turbo` donen resultats molt millors a canvi
  de més memòria i més temps.
- **La detecció de la llengua val el que val el model.** Un model petit es pot equivocar amb un inici fet de
  música o de silenci; digueu la llengua amb `--language` quan la sapigueu.

## ffmpeg

[`ffmpeg`](https://ffmpeg.org/download.html) extreu la pista d'àudio d'un vídeo, i `ffprobe`, que ve
amb ell, llista les pistes. Instal·leu-lo amb el gestor de paquets de la vostra distribució. Sense ell, un fitxer
d'àudio que el motor llegeix tal qual (`wav`, `mp3`, `flac`, `ogg`) es pot transcriure igualment, però un vídeo
no.

## Comprovar que funciona

```bash
subedit-cli transcribe --model /path/to/ggml-small.bin --output film.srt film.mkv
```

Si falta alguna cosa, el missatge diu què, i remet a aquesta pàgina.
