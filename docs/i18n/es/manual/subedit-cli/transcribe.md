# `transcribe`

<!-- machine-translated -->
> Esta página es una traducción automática y todavía no la ha revisado ningún lector del idioma.

```
subedit-cli transcribe --output FILE
                       [--to FORMAT]
                       [--audio-track N]
                       [--language CODE]
                       [--model FILE]
                       [--engine NAME]
                       [--keep-audio FILE]
                       [--raw]
                       <input>
```

Escribe **el habla de un vídeo, o de un archivo de audio, como archivo de subtítulos**. El audio se prepara,
se entrega a un motor que encuentra lo que se dice y cuándo, y los fragmentos que corta se fusionan en
subtítulos que respetan una duración y una longitud.

**Nada de lo que necesita se entrega con el programa**: el motor (whisper.cpp), un modelo y `ffmpeg`
los instala el usuario, que indica dónde está el modelo. [Instalar las herramientas de
transcripción](install-transcription.md) dice dónde encontrar cada una y qué modelo elegir. **Su ausencia no rompe
nada**: el comando dice lo que falta, con la referencia a esa página, no escribe nada y
termina con un código distinto de cero.

- **La entrada es un vídeo o un archivo de audio.** De un vídeo, `ffmpeg` extrae una pista de audio; sin
  `ffmpeg`, un archivo de audio que el motor lee tal cual (`wav`, `mp3`, `flac`, `ogg`) se transcribe igualmente,
  pero un vídeo no, ni tampoco ningún archivo cuando se pide una pista.
- **Un solo archivo a la vez**, y el destino es `--output`: no hay ni `--output-dir` ni
  `--in-place`, ya que el resultado es un archivo nuevo y no una reescritura.
- **El idioma se detecta** a menos que `--language` lo fuerce, y se dice el idioma empleado. Un
  modelo hecho solo para inglés no detecta nada y no transcribe nada más: un idioma que no puede
  aceptar se rechaza *antes* de empezar cualquier trabajo, indicando el límite del modelo.
- **Los segmentos se fusionan y se cortan** según los límites de los ajustes (`transcription.merge-gap-ms`,
  `transcription.max-duration-ms`, `transcription.max-characters`): dos fragmentos de una misma frase que están
  próximos forman un solo subtítulo, y un fragmento demasiado largo o demasiado ancho se corta en una palabra. **`--raw`
  escribe los segmentos del motor tal como llegaron**, uno por subtítulo, que es también la forma de ver lo que
  hace la fusión.

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
  --language CODE             Language of the speech: fr, en; detected by default
  --model FILE                Model file; the transcription.model setting by default
  --engine NAME               Engine; the transcription.engine setting by default
  --keep-audio FILE           Keep the extracted audio in this file
  --raw                       Write the segments as the engine cut them
```

## Argumentos y opciones

| Opción | Obligatoria | Valores | Por defecto |
| :----- | :------- | :----- | :------ |
| `<input>` | sí | un vídeo o un archivo de audio | — |
| `--output` | sí | el archivo de subtítulos que se va a escribir; su directorio se crea si falta | — |
| `--to` | no | el nombre corto de un formato: `srt`, `vtt`, `subviewer2`, `ssa`, `ass`, `microdvd`, `mpl2`, `tmplayer`, `lrc` | el formato que indica la extensión de `--output`, si no `srt` |
| `--audio-track` | no | un número entero desde 0: la posición entre las pistas de audio del vídeo | `0`, la primera |
| `--language` | no | dos letras minúsculas, un código ISO 639-1: `en`, `fr` | el ajuste `transcription.language`; vacío: se detecta |
| `--model` | no | el archivo de modelo | el ajuste `transcription.model` |
| `--engine` | no | el nombre de un motor: `whisper.cpp` | el ajuste `transcription.engine` |
| `--keep-audio` | no | un archivo donde conservar el audio extraído, como PCM mono de 16 kHz y 16 bits en un archivo WAV; su directorio se crea si falta | el audio no se conserva en ningún sitio |
| `--raw` | no | un indicador: escribe los segmentos del motor sin fusionar ni cortar | desactivado |

Los **ajustes** se leen del archivo de configuración como los de [`adjust`](adjust.md) — véase
[el archivo de configuración](invocation.md#el-archivo-de-configuración) — y cada uno tiene aquí su opción:

| Clave | Significado | Por defecto |
| :-- | :------ | :------ |
| `transcription.engine` | el motor | `whisper.cpp` |
| `transcription.executable` | el programa del motor, una ruta absoluta; vacío: se busca en el `PATH` | vacío |
| `transcription.model` | el archivo de modelo, una ruta absoluta | vacío |
| `transcription.language` | el idioma del audio, dos letras; vacío: se detecta | vacío |
| `transcription.merge-gap-ms` | los fragmentos de una misma frase más próximos que estos milisegundos se unen | `400` |
| `transcription.max-duration-ms` | la duración máxima de un subtítulo en pantalla, en milisegundos | `7000` |
| `transcription.max-characters` | el máximo de caracteres de un subtítulo, sin contar los saltos de línea | `84` |

## Elegir el audio

Un vídeo puede llevar varias pistas de audio — el original, un comentario, otro idioma. `--audio-track`
toma la **posición entre las pistas de audio** del archivo, desde 0, en el orden en que el archivo las enumera. Una
pista que el vídeo no tiene se rechaza, **con las pistas que sí tiene**, para que la siguiente llamada
sea la correcta:

```bash
subedit-cli transcribe --audio-track 4 --output film.srt --model small.bin film.mkv
```

```text
film.mkv: the video has no audio track 4
audio track 0: fra, Original
audio track 1: eng, Commentary
```

## Salida

**Salida estándar** — nada: el resultado es el archivo escrito.

| Nivel | Lo que se añade en la salida de error |
| :---- | :-------------------------------- |
| 1 | `<input>: N subtitles transcribed, language: xx -> <output>` |
| 2 | el progreso del motor, `transcribing: 25%`, tal como lo informa — a intervalos de unos treinta segundos de audio |

### En JSON

Con [`--format json`](invocation.md#una-salida-que-un-script-puede-leer) (la opción global, escrita antes del
subcomando), el objeto del archivo lleva el idioma empleado en `language`, `counts.subtitles` (los
subtítulos escritos) y `counts.segments` (los fragmentos del motor a partir de los cuales se hicieron), y `audio` cuando
`--keep-audio` conservó el audio.

## Códigos de salida

Los de la herramienta: `0` si se escribió el archivo, `2` si no se pudo — una herramienta o un modelo que falta, un
idioma que el modelo no puede aceptar, una extracción o una transcripción fallida —, `1` en un error de uso, como
un motor que nadie ha registrado.

## Errores

| Causa | Mensaje |
| :---- | :------ |
| no se encuentra ningún programa de motor, o el configurado no existe | `the transcription engine was not found`, y luego la referencia a la guía |
| no se indicó ningún modelo, o no existe | `no model was given, or the model file is not there`, y luego la referencia a la guía |
| `ffmpeg` no encontrado para un vídeo | `ffmpeg was not found, so the audio of a video cannot be extracted`, y luego la referencia |
| una pista que el vídeo no tiene | `the video has no audio track N`, y luego las pistas que tiene |
| un idioma que el modelo no puede transcribir | `the model cannot transcribe the language xx` |
| ningún idioma, y un modelo que no puede detectar ninguno | `the model cannot detect the language: give one` |
| un motor que nadie ha registrado | `no transcription engine is called NAME` |
| cualquier otro fallo | `the transcription failed`, con lo que dijo el motor |

La referencia a la guía es `See the manual page "Installing the transcription tools"
(install-transcription).`
