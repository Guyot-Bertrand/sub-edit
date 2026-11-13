# `sync`

<!-- machine-translated -->
> Esta página es una traducción automática y todavía no la ha revisado ningún lector del idioma.

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
**Reajusta los tiempos de un archivo de subtítulos según el audio de una película.** El caso para el que existe es el
`.srt` descargado de internet, desplazado al principio y al final, que un único desplazamiento no devuelve a su sitio.
Dos métodos lo hacen, y `--method` (o el ajuste `alignment.method`) elige entre ellos:

- **`speech`, el predeterminado, coloca el archivo sobre los momentos en que alguien habla.** Solo pregunta al sonido
  dónde hay habla, y encuentra el desplazamiento que hace coincidir los subtítulos con ella, tramo de película a tramo:
  un desfase constante, una deriva, una segunda mitad que va tarde y un archivo temporizado para otra velocidad de
  fotogramas (25 por 23.976, y similares) quedan corregidos. **No depende del idioma** — los subtítulos pueden estar
  en francés y la película en inglés —, no necesita transcripción y tarda segundos. Necesita `ffmpeg`, el programa
  `whisper-vad-speech-segments` que viene con whisper.cpp y un modelo de actividad de voz (`--vad-model`, el ajuste
  `transcription.vad-model`).
- **`words` empareja las palabras de cada subtítulo con las palabras que ha oído un motor.** Escucha con un motor de
  transcripción — el mismo que [`transcribe`](transcribe.md) — y cada subtítulo toma el lugar donde se dicen sus
  palabras. No se traduce nada: los subtítulos deben estar en el idioma que se habla. Puede colocar un único subtítulo
  fuera de sitio en un archivo que por lo demás está bien, y tarda lo que tarda una transcripción.

**Lo que `speech` no hace se dice cada vez que se ejecuta**, en la salida de error y en el registro: corrige el archivo
**en conjunto**, nunca un subtítulo por separado. Un subtítulo que está fuera de lugar por sí solo en un archivo que
por lo demás está bien, o un título mostrado sobre un silencio (una tarjeta de capítulo, un rótulo que traduce algo en
pantalla), no se busca y aún puede ser incorrecto, y los primeros minutos pueden seguir desajustados cuando la
película empieza con música o créditos. Revise el resultado, o use `--method words` cuando el texto esté en el idioma
que se habla.

**Nada de lo que necesita se entrega con el programa**: los programas, el modelo y `ffmpeg` son los de
[Instalar las herramientas de transcripción](install-transcription.md), y su ausencia no rompe nada — el comando dice
lo que falta, con la referencia a esa página, no escribe nada y termina con un código distinto de cero.

- **Solo cambian los inicios y los finales.** Nunca el texto, el estilo, el número de subtítulos ni su orden.
- **Un subtítulo que el audio no da conserva sus tiempos** — unos créditos que nadie dice, una canción, una línea en
  otro idioma. **Nunca se inventa una posición, ni se adivina a partir de los vecinos**: la línea se deja donde el
  archivo la tenía, y el comando lo dice. *No* es un fallo, y el código de salida es `0`.
- **Con `words`, un subtítulo se encuentra cuando se oyen al menos la mitad de sus palabras** (`--min-match`,
  `alignment.min-match`), y al menos una. Las mayúsculas, los acentos, la puntuación y el marcado no importan; una
  palabra que difiere en una letra de una palabra larga cuenta como oída. `speech` coloca todos los subtítulos, y no
  admite `--language`, `--model`, `--engine` ni `--min-match`.
- **El final está donde termina la última palabra.** El motor sitúa las palabras de forma aproximada, unas centésimas
  de segundo antes del final propio del subtítulo, de modo que un subtítulo puede salir un poco más corto que antes.
- **Un subtítulo que invadiría el siguiente se recorta hasta donde empieza el siguiente.** Uno que sigue solapándose
  con el siguiente, lo que recortar no puede curar, se deja y se dice.
- **Un archivo que no encaja con el sonido se rechaza y no se escribe**: con `words`, no se encontró ninguno de los
  subtítulos, lo que casi siempre es el audio equivocado o el idioma equivocado — `speech` no depende del idioma; con
  `speech`, los subtítulos y el habla tienen demasiado poco en común, lo que casi siempre es la película equivocada.
- **Se puede escuchar una parte de la película sola**, con `--start` y `--end`, como en
  [`transcribe`](transcribe.md#transcribir-una-parte): solo los subtítulos que **empiezan** en el intervalo se buscan
  y se tocan; los demás se dejan como están y no se cuentan.
- **El audio se escucha una vez por cada archivo indicado.** Para reajustar varios archivos sobre la misma película,
  hay que esperar por cada uno.

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
## Argumentos y opciones

| Opción | Obligatoria | Valores | Por defecto |
| :----- | :------- | :----- | :------ |
| `<files>` | sí | los archivos de subtítulos que reajustar; con `-r`, directorios | — |
| `--recursive`, `-r` | no | un indicador | desactivado — véase [Procesar un árbol](lots.md) |
| `--video` | una de `--video` y `--audio` | la película que escuchar | — |
| `--audio` | una de `--video` y `--audio` | un archivo de audio que escuchar, en lugar de una película | — |
| `--audio-track` | no | un número entero desde 0: la posición entre las pistas de audio de la película | `0`, la primera |
| `--language` | no | dos letras, un código ISO 639-1: `en`, `fr` | el ajuste `transcription.language`; vacío: se detecta |
| `--start`, `--end` | no | un tiempo: `2:00`, `1:02:03`, `2:00.5`, o un número de segundos; `--end` debe ir después de `--start` | todo el audio |
| `--model` | no | el archivo de modelo | el ajuste `transcription.model` |
| `--vad-model` | no | un modelo de actividad de voz, que hace que el motor omita los silencios; con él las palabras siguen más de cerca al habla | el ajuste `transcription.vad-model` |
| `--engine` | no | el nombre del motor que escucha: `whisper.cpp` | el ajuste `transcription.engine` |
| `--keep-audio` | no | un archivo donde conservar el audio extraído; su directorio se crea si falta | el audio no se conserva en ningún sitio |
| `--method` | no | `speech`: coloca el archivo según cuándo hay habla, en cualquier idioma; `words`: empareja las palabras con lo que se oyó, en el idioma que se habla | el ajuste `alignment.method`, `speech` |
| `--min-match` | no | un número entero de 1 a 100: la proporción de las palabras de un subtítulo que deben oírse, en porcentaje | el ajuste `alignment.min-match`, `50` |
| `--doubt-gap` | no | un número entero de milisegundos, 0 o más: cuánto puede diferir el desplazamiento de un subtítulo del de sus vecinos antes de señalarse | el ajuste `alignment.doubt-gap`, `2000` |
| `--report` | no | un archivo donde escribir los subtítulos que hay que revisar, uno por línea: el número del subtítulo, un tabulador y el motivo; un archivo de subtítulos a la vez | no se escribe nada |
| `--output` / `--output-dir` / `--in-place` | **una de las tres**, salvo con `--dry-run` | véase [Invocación](invocation.md#el-destino) | — |
| `--dry-run` | no | un indicador: calcula e informa, no escribe nada — véase [Ver antes de escribir](invocation.md#ver-antes-de-escribir) | desactivado |

El destino — `--output`, `--output-dir`, `--in-place`, `--dry-run` — y `-r` son los de los demás comandos que
reescriben un archivo: véase [el destino](invocation.md#el-destino). El archivo conserva su formato, su codificación y
sus finales de línea.

Los **ajustes** se leen del archivo de configuración — véase
[el archivo de configuración](invocation.md#el-archivo-de-configuración) — y cada uno tiene aquí su opción. El motor,
el modelo y el modelo de actividad de voz son los de `transcribe`.

| Clave | Significado | Por defecto |
| :-- | :------ | :------ |
| `alignment.engine` | el motor de alineación del método `words`; hay uno, `transcript-matching`, que escucha y empareja | `transcript-matching` |
| `alignment.method` | cómo se alinean los subtítulos: `speech` o `words` | `speech` |
| `alignment.min-match` | la proporción de las palabras de un subtítulo que deben oírse para que se encuentre, en porcentaje | `50` |
| `alignment.doubt-gap` | cuánto, en milisegundos, puede alejarse el desplazamiento de un subtítulo del de sus vecinos antes de señalarse | `2000` |

## Lo que se señala

El comando dice qué subtítulos hay que revisar, y por qué. Un subtítulo se señala por el **primero** de estos motivos
que se cumpla:

| Motivo | Significado |
| :-- | :------ |
| not found in the audio, left as it was | se oyeron menos de `--min-match` de sus palabras: conserva sus tiempos |
| only partly heard | encontrado, pero se oyeron menos del 80 % de sus palabras |
| moved differently from its neighbours | encontrado, pero su desplazamiento se aleja del desplazamiento mediano de los dos subtítulos de cada lado en más de `--doubt-gap` |

Un subtítulo también puede **recortarse hasta el inicio del subtítulo siguiente**, o **seguir solapándose con el
subtítulo siguiente**.

Con `speech` se coloca todos los subtítulos, y solo puede aplicarse el último de estos motivos: allí donde el archivo
cambia su desplazamiento en más de `--doubt-gap` de un subtítulo al siguiente (una segunda mitad que va tarde), se
señalan los subtítulos a cada lado del escalón.

Dos cifras cuentan la historia del archivo en conjunto. **El desplazamiento mediano** es el valor central de cuánto se
han movido los subtítulos: un archivo que simplemente iba tarde se lee aquí, y en ningún otro sitio. **La deriva** es el
desplazamiento mediano de la última décima parte de los subtítulos menos el de la primera décima: es casi cero para un
desfase constante, y grande para un archivo que iba cada vez más tarde a lo largo de la película.

## Salida

**Salida estándar** — nada: el resultado es el archivo escrito.

| Nivel | Lo que se dice en la salida de error |
| :---- | :------------------------------- |
| 1 | `source: <file>`, `media: <audio>`, `result: N of M subtitles placed on the audio, K to check; median shift X s, drift Y s`, `dest: <destination>` — y, cuando se corrigió una velocidad de fotogramas, la proporción; con `speech`, **la advertencia de lo que no hace**, en una línea aparte |
| 2 | el paso y el progreso, como en [`transcribe`](transcribe.md#salida) |

### En JSON

Con [`--format json`](invocation.md#una-salida-que-un-script-puede-leer), el objeto del archivo lleva `counts`:
`subtitles` (buscados), `placed` (a los que se dio un lugar), `moved` (cuyos tiempos cambiaron), `partial`,
`out_of_line`, `not_found`, `median_shift_ms` y `drift_ms`; y `flagged`, una lista de `{"subtitle": N, "kind": "…"}`
donde el número es el lugar del subtítulo en el archivo y `kind` es uno de `not-found`, `partial`, `out-of-line`,
`cut-back` y `overlaps`. Con `speech`, `warnings` lleva una entrada de `kind` `speech-only`. Los identificadores están prometidos; las frases no.

## Ejemplo

```bash
subedit-cli sync --video film.mkv --vad-model silero.bin \
    --report to-check.txt --output film.synced.srt film.srt
```

Para colocar cada subtítulo según sus palabras, con un texto en el idioma que se habla:

```bash
subedit-cli sync --method words --video film.mkv --model small.bin --vad-model silero.bin \
    --output film.synced.srt film.srt
```
## Códigos de salida

Los de la herramienta: `0` si se escribió el archivo, haya o no subtítulos señalados, `2` si no se pudo — una
herramienta o un modelo que falta, una extracción o una escucha fallida, un archivo que no encaja con el sonido (ninguno de sus subtítulos encontrado, o demasiado poco en común con el habla) —, `1` en
un error de uso, como ni `--video` ni `--audio` (o ambos), una opción fuera de su intervalo, o un intervalo que no se
puede usar.

## Errores

| Causa | Mensaje |
| :---- | :------ |
| ni `--video` ni `--audio`, o ambos | `sync needs the audio to listen to: give --video or --audio, and not both` |
| no se encuentra ningún programa de motor, o el configurado no existe | `the transcription engine was not found`, y luego la referencia a la guía |
| no se indicó ningún modelo, o no existe | `no model was given, or the model file is not there`, y luego la referencia a la guía |
| `ffmpeg` no encontrado para una película | `ffmpeg was not found, so the audio of a video cannot be extracted`, y luego la referencia |
| una pista que la película no tiene | `the video has no audio track N`, y luego las pistas que tiene |
| un idioma que el modelo no puede transcribir | `the model cannot transcribe the language xx` |
| un motor que nadie ha registrado | `no transcription engine is called NAME`, o `there is no alignment engine called NAME` |
| ninguno de los subtítulos encontrado (`words`) | `none of the N subtitles was found in the audio: is it the right audio, and the right language?`, y luego `The method based on speech does not depend on the language.` |
| no se indicó el modelo de actividad de voz, o no se encuentra el programa que halla el habla (`speech`) | `Finding where there is speech needs a voice-activity model: name one in the transcription.vad-model setting.` o `Finding where there is speech needs the program whisper-vad-speech-segments, which comes with whisper.cpp, and it was not found.`, y luego la referencia a la guía |
| los subtítulos y el habla tienen demasiado poco en común (`speech`) | `The sound and the subtitles have too little in common to be put in line: is it the right film?` |
| `--min-match` fuera de 1 a 100, `--doubt-gap` negativo | el error de uso de la opción |
| `--report` con más de un archivo de subtítulos | `--report names one file: give one subtitle file when you use it` |
