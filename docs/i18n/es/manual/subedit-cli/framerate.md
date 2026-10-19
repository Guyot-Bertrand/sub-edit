# `framerate`

<!-- machine-translated -->
> Esta página es una traducción automática y todavía no la ha revisado ningún lector del idioma.

> **Esto no es [`snap`](snap.md).** Este comando **cambia la escala
> del archivo**: sirve cuando la temporización es incorrecta y deriva varios
> segundos hacia el final de la película. `snap` devuelve las marcas de tiempo a la rejilla
> más cercana sin reajustar ningún tiempo, y sirve cuando la temporización ya es
> correcta. Los dos tienen la misma forma, una velocidad de fotogramas o dos, y confundirlos no provoca
> ningún error.

```
subedit-cli framerate --from <fps> --to <fps>
                      (--output FILE | --output-dir DIR | --in-place)
                      [--dry-run]
                      [--recursive]
                      <file>...
```

Reajusta los tiempos de un archivo temporizado para una velocidad de fotogramas a otra. Es
la operación que pide un archivo de subtítulos cuando se obtuvo para una copia que
no es la de la película que se tiene: un DVD PAL frente a una transferencia NTSC, por
ejemplo.

El formato del archivo leído se **conserva**: cambiar de formato es tarea de
[`convert`](convert.md).

<!-- example: subedit-cli framerate --help -->
```console
$ subedit-cli framerate --help
Re-time a file mastered at one frame rate for another
Usage: subedit-cli framerate [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to re-time

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --from TEXT REQUIRED        Frame rate the file is timed at: 25, 23.976
  --to TEXT REQUIRED          Frame rate to time it for: 24, 29.97
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Argumentos y opciones

| Opción | Obligatoria | Valores | Por defecto |
| :----- | :----- | :------ | :----- |
| `<file>...` | sí | una o varias rutas | — |
| `--recursive`, `-r` | no | un indicador | desactivado — véase [Procesar un árbol](lots.md) |
| `--from` | **sí** | una velocidad de fotogramas, véase más abajo | — |
| `--to` | **sí** | una velocidad de fotogramas, véase más abajo | — |
| `--output` / `--output-dir` / `--in-place` | **una de las tres**, salvo con `--dry-run` | véase [Invocación](invocation.md#el-destino) | — |
| `--dry-run` | no | un indicador: calcula e informa, no escribe nada — véase [Ver antes de escribir](invocation.md#ver-antes-de-escribir) | desactivado |

`--from` es la velocidad de fotogramas para la que el archivo **está** temporizado, `--to` aquella
para la que se quiere. Intercambiarlas desplaza el archivo en el
otro sentido —unos siete segundos en un largometraje entre 25 y
23,976— y nada lo señala antes de ver la película.

## Cómo se escribe una velocidad de fotogramas

En fotogramas por segundo, decimales incluidos, con el punto como separador:

```
25      24      23.976      29.97      60
```

**Una velocidad de fotogramas nula o negativa se rechaza**: no es una velocidad de fotogramas.

### Las tres velocidades de fotogramas NTSC no pueden escribirse exactamente

`23.976`, `29.97` y `59.94` son **etiquetas**. Las velocidades de fotogramas que
nombran son `24000/1001`, `30000/1001` y `60000/1001`, ninguna de las cuales tiene
una escritura decimal finita. La herramienta las reconoce como tales:

| Lo que se escribe | Lo que nombra |
| :------------- | :------------------ |
| `23.976` | `24000/1001` — cine transferido a vídeo NTSC |
| `24` | `24` — cine |
| `25` | `25` — vídeo PAL y SECAM |
| `29.97` | `30000/1001` — vídeo NTSC |
| `30`, `50`, `60` | ellas mismas |
| `59.94` | `60000/1001` |

El reconocimiento se basa en **el valor y no en los caracteres**: `23.9760`
nombra la misma velocidad de fotogramas que `23.976`.

Cualquier otro decimal se toma al pie de la letra: `23.9` no es el estándar
de nadie, no hay nada que interpretar en él. Eso es también lo que hace necesaria la tabla
de arriba: sin ella, `23.976` sería `23976/1000`, y la línea de comandos
diría algo distinto de la ventana con las mismas palabras.

## Lo que pasa con las posiciones

```
t′ = round( t × (from / to) )
```

Una película masterizada a 23,976 y reproducida a 25 fotogramas por segundo va más rápido,
así que sus subtítulos llegan antes; en el otro sentido, llegan
más tarde.

**El factor es un racional exacto, y el redondeo se hace una sola vez.** El
camino que parece equivalente —convertir la posición en un número de fotograma a la
velocidad de entrada y luego volver a milisegundos a la velocidad de salida— no
lo es: cuantiza cada posición sobre la rejilla de entrada y se equivoca hasta en
medio fotograma, 21 ms medidos. Véase
[ADR 0013](../../../../adr/0013-mise-a-l-echelle-exacte-des-positions.md).

En la práctica, de 25 a 23,976, el factor es `1001/960`:

| Posición leída | Escrita | Lo que daría pasar por los fotogramas |
| -----------: | -----: | -----------------------------------------: |
| `1 010 ms` | `1 053 ms` | `1 043 ms` |
| `3 600 017 ms` | `3 753 768 ms` | `3 753 750 ms` |

<!-- example: printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 25 --to 23.976 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 25 --to 23.976 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle retimed from 25 to 24000/1001 fps -> b.srt
1
00:00:01,053 --> 00:00:02,106
Un.
```

## Salida

**Salida estándar**: nada; el resultado es el archivo escrito. Con `--dry-run`, tampoco nada: no hay archivo, y la línea de narración lo dice — véase [Ver antes de escribir](invocation.md#ver-antes-de-escribir).

| Nivel | Lo que se añade en la salida de error |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles retimed from 25 to 24000/1001 fps -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — el formato, la codificación, la marca y los finales de línea del archivo leído, restituidos tal como estaban |
| 3 | `<path>: N bytes read, M written`, y después **cada diagnóstico de lectura** — véase [Invocación](invocation.md#diagnósticos-de-lectura) |

**La narración nombra la velocidad de fotogramas exacta, no la etiqueta tecleada.** Escribir
`23.976` en esa línea informaría de una conversión que no ha tenido lugar; es
el único lugar donde se ve cuál de las dos se ha usado. Una velocidad de fotogramas que un
decimal escribe exactamente se muestra ahí como decimal, y las demás como fracción.

### En JSON

Con [`--format json`](invocation.md#una-salida-que-un-script-puede-leer), el objeto de cada archivo lleva `counts.subtitles` (todos los subtítulos del archivo). La tabla completa es la de [Invocación](invocation.md#lo-que-cada-subcomando-pone-en-counts).

## Códigos de salida

Los de la herramienta: `0` si se escribieron todos los archivos, `2` si ninguno, `3` si
solo algunos, `1` en caso de error de uso.

Ningún rechazo depende del contenido de un archivo: dos velocidades de fotogramas siempre definen
un factor. Es el único de los subcomandos que escriben cuyos rechazos
llegan todos antes de la primera lectura.

<!-- example: printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 0 --to 25 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 0 --to 25 --output b.srt a.srt; echo "code=$?"
"0" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed
code=1
```

## Errores

| Qué lo provoca | Mensaje |
| :------------------ | :------ |
| velocidad de fotogramas nula o negativa | `"…" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed` |
| coma decimal | `"…" is not a frame rate: use a decimal point and not a comma, in every language` |
| texto que no es un número | `"…" is not a frame rate: expected frames per second, like 25 or 23.976` |
| velocidad de fotogramas desproporcionada | `"…" is not a frame rate: no video runs at that many frames per second` |

**No se escribe nada sin una de las tres opciones de destino**, y sus
errores son comunes a los subcomandos que escriben:
véase [Invocación](invocation.md#el-destino).
