# `transcribe`

<!-- machine-translated -->
> Esta página es una traducción automática y todavía no la ha revisado ningún lector del idioma.

```
subedit-cli transcribe --output FILE
                       [--to FORMAT]
                       [--audio-track N]
                       [--language CODE]
                       [--start TIME] [--end TIME]
                       [--suppress-notes | --keep-notes]
                       [--model FILE]
                       [--vad-model FILE]
                       [--engine NAME]
                       [--keep-audio FILE]
                       [--raw]
                       <input>

subedit-cli transcribe --list-languages [--model FILE] [--engine NAME]
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
- **Un idioma se escribe con dos letras — `fr`, `en`, `de` —, o con el código de tres letras que declara un archivo, `fra`,
  `eng`, que se lee como el de dos letras.** Todo lo demás se rechaza antes de empezar cualquier trabajo. **La detección
  solo escucha los primeros treinta segundos**: una película que empieza con música o silencio puede tomarse por
  otro idioma, y el motor escribe entonces el habla *traducida* a ese idioma — una película francesa tomada
  por inglesa sale en inglés, o como `[SPEAKING FRENCH]`. Cuando conozca el idioma, indíquelo.
- **Se puede transcribir una parte de la película sola**, con `--start` y `--end` — véase
  [Transcribir una parte](#transcribir-una-parte). Así se prueba un modelo en dos minutos antes de ejecutarlo en
  dos horas.
- **Los idiomas que ofrece un motor se pueden enumerar**, con `--list-languages` — véase
  [Enumerar los idiomas](#enumerar-los-idiomas).
- **Los sonidos no se escriben como notas, por defecto**: al motor se le ordena no hacerlo (`--suppress-notes`; su propia opción es
  `--suppress-nst`), de modo que `(Sonnerie de téléphone)` o `[MUSIC PLAYING]` no salen en los subtítulos, ni entre paréntesis ni entre corchetes.
  `--keep-notes` los escribe, para subtítulos para personas sordas o con discapacidad auditiva. Prohibidas las notas, el motor escribe `...` para un
  pasaje sin habla, y **el posprocesamiento descarta los fragmentos que no tienen ninguna letra ni ninguna cifra** — `...`, `…`, `♪` —
  antes de fusionar y cortar; `--raw` los conserva, tal como llegaron los fragmentos del motor.
- **Los segmentos se fusionan y se cortan** según los límites de los ajustes (`transcription.merge-gap-ms`,
  `transcription.max-duration-ms`, `transcription.max-characters`): dos fragmentos de una misma frase que están
  próximos forman un solo subtítulo, y un fragmento demasiado largo o demasiado ancho se corta — primero en un final de frase, luego en una coma, un punto y coma o dos puntos, y en medio de una frase solo cuando no hay puntuación donde cortar —, evitando un corte que deje un subtítulo demasiado pequeño para leerse. Los tres pasos se hacen en orden, cada uno sobre toda la transcripción: descartar, fusionar, cortar. Un subtítulo que empieza un fragmento conserva su inicio, y el que lo termina conserva su final. **`--raw`
  escribe los segmentos del motor tal como llegaron**, uno por subtítulo, que es también la forma de ver lo que
  hace la fusión.

<!-- example: subedit-cli transcribe --help -->
```console
$ subedit-cli transcribe --help
Write the speech of a video or an audio file as subtitles
Usage: subedit-cli transcribe [OPTIONS] [input]

Positionals:
  input TEXT                  Video or audio file to transcribe

Options:
  -h,--help                   Print this help message and exit
  --output FILE               Subtitle file to write
  --to TEXT:{srt,vtt,subviewer2,ssa,ass,microdvd,mpl2,tmplayer,lrc}
                              Format to write; the one the extension of --output says by default
  --audio-track N             Audio track of a video, counted from 0; the first by default
  --start TIME                Where to begin, as 2:00, 1:02:03 or seconds; the beginning by default
  --end TIME                  Where to stop, as 2:00, 1:02:03 or seconds; the end by default
  --language CODE             Language of the speech, two letters: fr, en, de; detected from the first 30 seconds by default
  --suppress-notes            Do not write sounds as notes, such as (phone ringing); the transcription.suppress-notes setting by default
  --keep-notes                Write sounds as notes, against the setting
  --model FILE                Model file; the transcription.model setting by default
  --vad-model FILE            Voice-activity model that skips silences; the transcription.vad-model setting by default
  --engine NAME               Engine; the transcription.engine setting by default
  --keep-audio FILE           Keep the extracted audio in this file
  --list-languages            List the languages the engine offers for the model, and transcribe nothing
  --raw                       Write the segments as the engine cut them
```

## Argumentos y opciones

| Opción | Obligatoria | Valores | Por defecto |
| :----- | :------- | :----- | :------ |
| `<input>` | sí, salvo con `--list-languages` | un vídeo o un archivo de audio | — |
| `--output` | sí, salvo con `--list-languages` | el archivo de subtítulos que se va a escribir; su directorio se crea si falta | — |
| `--to` | no | el nombre corto de un formato: `srt`, `vtt`, `subviewer2`, `ssa`, `ass`, `microdvd`, `mpl2`, `tmplayer`, `lrc` | el formato que indica la extensión de `--output`, si no `srt` |
| `--audio-track` | no | un número entero desde 0: la posición entre las pistas de audio del vídeo | `0`, la primera |
| `--language` | no | dos letras, un código ISO 639-1: `en`, `fr`; un código de tres letras como `fra` se lee como `fr` | el ajuste `transcription.language`; vacío: se detecta |
| `--suppress-notes` | no | un indicador: no escribir los sonidos como notas; rechazado para un motor al que no se le puede decir | el ajuste `transcription.suppress-notes` |
| `--keep-notes` | no | un indicador: escribirlos, contra el ajuste; lo contrario de `--suppress-notes`, y los dos juntos se rechazan | el ajuste `transcription.suppress-notes` |
| `--list-languages` | no | un indicador: enumerar los idiomas que el motor ofrece para el modelo, y no transcribir nada | desactivado |
| `--start` | no | un tiempo: `2:00`, `1:02:03`, `2:00.5`, o un número de segundos, `90` | el principio del audio |
| `--end` | no | un tiempo, escrito como para `--start`; debe ir después de `--start` | el final del audio |
| `--model` | no | el archivo de modelo | el ajuste `transcription.model` |
| `--vad-model` | no | un modelo de actividad de voz (un modelo Silero para whisper.cpp), que hace que el motor omita los silencios antes de decodificar: sus segmentos siguen entonces al habla, y el primero empieza donde empieza el habla en lugar de en 0. El archivo debe existir, y un motor que no pueda usarlo rechaza la opción por su nombre | el ajuste `transcription.vad-model` |
| `--engine` | no | el nombre de un motor: `whisper.cpp` | el ajuste `transcription.engine` |
| `--keep-audio` | no | un archivo donde conservar el audio extraído, como PCM mono de 16 bits en un archivo WAV, a la frecuencia de muestreo que pide el motor (16 kHz para whisper.cpp); su directorio se crea si falta, y el nombre puede llevar `{rate}`, sustituido por esa frecuencia en hercios (`film.{rate}.wav` se escribe `film.16000.wav`) | el audio no se conserva en ningún sitio |
| `--raw` | no | un indicador: escribe los segmentos del motor sin fusionar ni cortar | desactivado |

Los **ajustes** se leen del archivo de configuración como los de [`adjust`](adjust.md) — véase
[el archivo de configuración](invocation.md#el-archivo-de-configuración) — y cada uno tiene aquí su opción:

| Clave | Significado | Por defecto |
| :-- | :------ | :------ |
| `transcription.engine` | el motor | `whisper.cpp` |
| `transcription.executable` | el programa del motor, una ruta absoluta; vacío: se busca en el `PATH` | vacío |
| `transcription.model` | el archivo de modelo, una ruta absoluta | vacío |
| `transcription.vad-model` | el modelo de actividad de voz, una ruta absoluta; vacío: ninguno, y los silencios no se omiten. Un ajuste que un motor no puede usar se descarta sin decir nada | vacío |
| `transcription.language` | el idioma del audio, dos letras; vacío: se detecta | vacío |
| `transcription.suppress-notes` | no escribir los sonidos como notas | `true` |
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

## Transcribir una parte

`--start` y `--end` restringen el trabajo a la parte de la película que hay entre ambos. Se puede dar solo uno: un
`--start` solo llega hasta el final, un `--end` solo empieza por el principio.

```bash
subedit-cli transcribe --start 2:00 --end 3:30 --output test.srt --model large-v3.bin film.mkv
```

- **Un tiempo se escribe `M:SS`, `H:MM:SS`, ambos con una parte decimal tras un punto (`2:00.5`), o como un número de
  segundos (`90`, `90.5`)**. Una coma se rechaza, como en toda la herramienta.
- **Los subtítulos conservan el tiempo de la película**: la frase dicha en 2:10 de la película se escribe en `00:02:10`, no
  en `00:00:10`. El archivo se puede superponer tal cual a toda la película.
- **Los bordes son aproximados**: el motor trabaja con ventanas de treinta segundos, de modo que lo que devuelve se recorta
  después al intervalo; una frase a caballo de un borde se corta en él.
- **No empieza nada si el intervalo no se puede usar**: un límite que no es un tiempo, o un `--end` que no va después de
  `--start`, es un error de uso que nombra el problema, no escribe nada y no ejecuta ni `ffmpeg` ni el
  motor.

El audio de toda la película se extrae igualmente: lo que el intervalo ahorra es la transcripción, mucho más larga.

## Enumerar los idiomas

`--list-languages` escribe los idiomas que el motor ofrece **para el modelo**, uno por línea: el código que toma `--language`, un tabulador, y el nombre
del idioma en el idioma de la interfaz. No necesita ni entrada ni salida, y no transcribe nada.

```bash
subedit-cli transcribe --list-languages --model ggml-small.bin
```

```text
en	English
zh	Chinese
de	German
…
```

La lista depende del motor y del modelo: un centenar de idiomas para un modelo whisper.cpp multilingüe, solo `en` para un modelo solo para inglés. Un motor que no puede enumerar sus idiomas no enumera ninguno. Un modelo que no se da o que no está es un error, con la indicación de la guía; solo
la forma de texto, no se escribe nada con [`--format json`](invocation.md#una-salida-que-un-script-puede-leer).

## Salida

**Salida estándar** — nada: el resultado es el archivo escrito.

| Nivel | Lo que se añade en la salida de error |
| :---- | :-------------------------------- |
| 1 | `<input>: N subtitles transcribed, language: xx -> <output>` |
| 2 | el paso y el progreso, una línea por cada por ciento: `extracting the audio…`, luego `transcribing…`, luego `transcribing: 25%, 0:12:30 of 0:50:00, about 38 min left, elapsed: 0:05:12` — el por ciento, la posición alcanzada en el audio, el tiempo restante y el tiempo transcurrido; **la línea se repite cada diez segundos cuando no ha llegado nada nuevo**, para que una espera larga no se tome por un bloqueo |
| 3 | y cada texto nuevo que obtiene el motor, `last text: …` |

**El por ciento es la posición del motor en el audio, no un recuento de sus ventanas**: el motor escribe lo que ha
transcrito a medida que avanza, y el comando lee hasta dónde ha llegado, de modo que la cifra se mueve desde el primer
minuto. (El progreso propio del motor llega en pasos del 5 % del audio, una vez hecha una ventana de treinta segundos:
por sí solo, puede quedarse en 0 % durante un cuarto de hora en una película larga con un modelo grande en un procesador.) **El
tiempo restante calla al principio**: se da cuando se ha hecho trabajo suficiente para extrapolar, y sigue la velocidad del
último minuto, no el promedio de toda la ejecución.

### En JSON

Con [`--format json`](invocation.md#una-salida-que-un-script-puede-leer) (la opción global, escrita antes del
subcomando), el objeto del archivo lleva el idioma empleado en `language`, `counts.subtitles` (los
subtítulos escritos) y `counts.segments` (los fragmentos del motor a partir de los cuales se hicieron), y `audio` cuando
`--keep-audio` conservó el audio.

## Avisos

Un aviso se dice en el primer nivel, antes del trabajo, porque cambia lo que cabe esperar:

- **Con un modelo solo para inglés**: `This model is for English only: whatever language the film
  speaks, the result will be English, or notes in brackets. Choose a multilingual model in the preferences.` Un modelo
  cuyo nombre de archivo termina en `.en` lo es.

## Códigos de salida

Los de la herramienta: `0` si se escribió el archivo, `2` si no se pudo — una herramienta o un modelo que falta, un
idioma que el modelo no puede aceptar, una extracción o una transcripción fallida —, `1` en un error de uso, como
un motor que nadie ha registrado o un intervalo que no se puede usar.

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
| falta `<input>` o `--output` | `input is required` o `--output is required` |
| `--suppress-notes` con `--keep-notes` | `--suppress-notes and --keep-notes say opposite things; give one of them` |
| `--suppress-notes` para un motor al que no se le puede decir | `the engine NAME cannot be told not to write sounds as notes` |
| un idioma que no es un código | `"TEXT" is not a language code: write two letters, such as fr, en, de or es` |
| un límite del intervalo que no es un tiempo | `"TEXT" is not a time: write it as 2:00, 1:02:03 or 2:00.5, or in seconds` |
| `--end` no posterior a `--start` | `the end of the range must come after its start` |
| cualquier otro fallo | `the transcription failed`, con lo que dijo el motor |

La referencia a la guía es `See the manual page "Installing the transcription tools"
(install-transcription).`
