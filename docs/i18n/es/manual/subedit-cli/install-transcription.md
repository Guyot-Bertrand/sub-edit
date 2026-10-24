# Instalar las herramientas de transcripción

<!-- machine-translated -->
> Esta página es una traducción automática y todavía no la ha revisado ningún lector del idioma.

Transcribir audio en subtítulos — [`transcribe`](transcribe.md) en la línea de comandos,
[`Transcribe Audio…`](../subedit-gui/video.md#transcribir-el-audio) en la ventana — necesita tres
cosas que **subedit no entrega ni descarga**: un motor, un modelo para ese motor y
`ffmpeg`. Usted las instala, y le dice a subedit dónde está el modelo.

**Sin ellas no se rompe nada.** El resto del programa funciona como antes; solo la transcripción
se desactiva, con un mensaje que remite aquí.

## El motor: whisper.cpp

[whisper.cpp](https://github.com/ggml-org/whisper.cpp) es el motor que ejecuta subedit. Es un programa,
`whisper-cli`, que funciona en un procesador corriente — una tarjeta gráfica solo lo hace más rápido. Compílelo
como dice su propia documentación (bastan un compilador de C++ y CMake):

```bash
git clone https://github.com/ggml-org/whisper.cpp
cd whisper.cpp
cmake -B build
cmake --build build -j --config Release
```

El programa es entonces `build/bin/whisper-cli`. Póngalo donde lo encuentre su `PATH`, o **indique su
ruta** a subedit con el ajuste `transcription.executable`. Una compilación para una tarjeta gráfica se hace
con las opciones que da su documentación; subedit ejecuta lo que usted haya instalado y no elige nada
por usted.

## Un modelo

El motor lee un **modelo**, un archivo de varios cientos de megabytes a algunos gigabytes que
descarga una sola vez. Los modelos oficiales están enumerados en
[huggingface.co/ggerganov/whisper.cpp](https://huggingface.co/ggerganov/whisper.cpp), y el
repositorio del motor tiene un script que obtiene uno:

```bash
sh ./models/download-ggml-model.sh small
```

Luego indique su ruta con `--model` en la línea de comandos, o con el ajuste `transcription.model`,
o en el cuadro de diálogo de la ventana, que la recuerda.

### Qué modelo para qué idioma

Whisper no cambia de modelo una vez que ha encontrado el idioma: **el modelo que usted indica decide lo que
puede hacer.**

| Modelo | Idiomas | Detecta el idioma | Tamaño |
| :---- | :-------- | :------------------- | :--- |
| `tiny`, `base`, `small`, `medium`, `large-v3`, `large-v3-turbo` | un centenar | sí | de unos 75 MB a unos 3 GB |
| `tiny.en`, `base.en`, `small.en`, `medium.en` | solo inglés | no | como el mismo tamaño de arriba |

- **Para cualquier cosa que no sea inglés, tome un modelo multilingüe** — uno cuyo nombre no tenga `.en`. Un modelo solo
  de inglés rechaza otro idioma: el comando lo dice antes de hacer ningún trabajo.
- **Los modelos más pequeños son notablemente peores fuera del inglés.** Para el francés, el alemán, el español y
  similares, `small` es un mínimo, y `medium` o `large-v3-turbo` dan resultados mucho mejores a costa
  de más memoria y más tiempo.
- **La detección del idioma vale lo que vale el modelo.** Un modelo pequeño puede equivocarse con un comienzo hecho de
  música o de silencio; indique el idioma con `--language` cuando lo conozca.

## ffmpeg

[`ffmpeg`](https://ffmpeg.org/download.html) extrae la pista de audio de un vídeo, y `ffprobe`, que viene
con él, enumera las pistas. Instálelo con el gestor de paquetes de su distribución. Sin él, un archivo
de audio que el motor lee tal cual (`wav`, `mp3`, `flac`, `ogg`) se puede transcribir igualmente, pero un vídeo
no.

## Comprobar que funciona

```bash
subedit-cli transcribe --model /path/to/ggml-small.bin --output film.srt film.mkv
```

Si falta algo, el mensaje dice qué, y remite a esta página.
