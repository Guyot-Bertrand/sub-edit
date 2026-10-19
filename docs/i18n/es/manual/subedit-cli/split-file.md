# `split-file`

<!-- machine-translated -->
> Esta página es una traducción automática y todavía no la ha revisado ningún lector del idioma.

```
subedit-cli split-file --at N --head FILE --tail FILE
                       [--dry-run] [--sort] [--disjoint]
                       <file>
```

Corta un archivo **en dos**: una cabeza y una cola. Es `Split Project…` de la ventana —[el mismo motor](../subedit-gui/operations.md#split-project),
los mismos recuentos, el mismo rechazo— y **el inverso exacto de [`append`](append.md)**: añadir la cola a la
cabeza devuelve el archivo de partida.

- **`--at N` es el número del primer subtítulo de la cola**, numerado como lo numera la tabla de
  la ventana: desde 1. `--at 3` pone los subtítulos 1 y 2 en la cabeza, y el tercero y los demás en la
  cola. Va **del segundo al último**: un corte necesita un subtítulo a cada lado.
- **La cola se lleva de vuelta al origen**: se desplaza en menos el final del último subtítulo que queda en la cabeza,
  y sus posiciones conservan su orden y su espaciado. Es lo que hace de `split-file` el inverso de `append`.
- **Las dos mitades conservan el formato, la codificación, la marca de orden de bytes y los finales de línea del archivo leído**, y
  la escritura las renumera desde 1, como cualquier archivo que se vuelve a escribir.
- **Dos salidas, y por tanto un destino propio**: `--head` y `--tail`, **ambas obligatorias**. `--output`,
  `--output-dir`, `--in-place` y `--recursive` no existen para `split-file` y se rechazan como cualquier opción
  desconocida, código `1`: no hay ni un directorio en el que repartir dos archivos con nombre, ni una entrada que sobrescribir.

<!-- example: subedit-cli split-file --help -->
```console
$ subedit-cli split-file --help
Cut a subtitle file in two, a head and a tail
Usage: subedit-cli split-file [OPTIONS] file

Positionals:
  file TEXT REQUIRED          Subtitle file to cut

Options:
  -h,--help                   Print this help message and exit
  --at N                      Number of the first subtitle of the tail, as the table numbers them
  --head FILE                 File to write the subtitles before the cut to
  --tail FILE                 File to write the subtitles from the cut on to
  --dry-run                   Work out and say what would be written, and write nothing
  --sort                      Put the subtitles in order of their start before cutting; --at counts in that order
  --disjoint                  Cut overlapping subtitles so that no range overlaps another, before cutting
```

## Argumentos y opciones

| Opción | Obligatoria | Valores | Por defecto |
| :----- | :----- | :------ | :----- |
| `<file>` | sí | **una sola** ruta | — |
| `--at` | sí | un número entero, de 2 al número de subtítulos del archivo | — |
| `--head`, `--tail` | **sí, las dos**, salvo con `--dry-run` | rutas de archivo; su directorio se crea si falta | — |
| `--dry-run` | no | un indicador: calcula e informa, no escribe nada — véase [Ver antes de escribir](invocation.md#ver-antes-de-escribir) | desactivado |
| `--sort` | no | un indicador: pone los subtítulos en el orden de su inicio **antes de cortar**; `--at` cuenta en ese orden | desactivado |
| `--disjoint` | no | un indicador: corta los subtítulos solapados **antes de cortar**; `--at` cuenta en lo que queda, véase [Hacer disjuntos los intervalos](#hacer-disjuntos-los-intervalos) | desactivado |

**Con `--dry-run`, no se exige ningún destino**: puede no darse ninguno, pero no solo uno de los dos;
`--head` y `--tail` van juntas. Los que se dan se juzgan igual que sin `--dry-run`.

**Los dos destinos no pueden ser ni la entrada ni el mismo archivo**: escribir sobre lo que se está
leyendo, o escribir la cola sobre la cabeza, se rechaza antes de leer nada, código `1`.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n3\n00:00:06,000 --> 00:00:07,000\ntrois\n\n4\n00:00:08,000 --> 00:00:09,000\nquatre\n\n' > film.srt; subedit-cli split-file --at 3 --head tete.srt --tail queue.srt film.srt; cat queue.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n3\n00:00:06,000 --> 00:00:07,000\ntrois\n\n4\n00:00:08,000 --> 00:00:09,000\nquatre\n\n' > film.srt; subedit-cli split-file --at 3 --head tete.srt --tail queue.srt film.srt; cat queue.srt
film.srt: split at subtitle 3: 2 subtitles in the head, 2 subtitles in the tail -> tete.srt, queue.srt
1
00:00:02,000 --> 00:00:03,000
trois

2
00:00:04,000 --> 00:00:05,000
quatre
```

El segundo subtítulo, que queda en la cabeza, termina a los 4 s: la cola retrocede 4 s, y el tercer subtítulo,
que empezaba a los 6 s, empieza a los 2 s.

## Cortar en orden temporal

`--at` numera los subtítulos **tal como los da el archivo**. En un archivo cuyo orden está roto, la cabeza no es, por tanto, la mitad más temprana
en el tiempo. `--sort` pone los subtítulos en el orden de sus inicios **antes de juzgar el corte** —de forma estable, como hace [`sort`](sort.md)—, y
`--at` cuenta entonces en ese orden.

Nunca por defecto. Con la opción, el archivo indica en el primer nivel de narración cuántos subtítulos se han movido (`<path>: N subtitles moved`
o `<path>: already in order`), y el registro `--format json` lleva un recuento `moved`, **ausente sin la opción**.

## Hacer disjuntos los intervalos

Nunca por defecto. La opción hace lo que hace [`disjoint`](disjoint.md), **con su configuración por defecto**, antes de juzgar el corte y después de ordenar: `--at` cuenta en lo que eso da. Necesita un archivo en orden: si no, se rechaza, y `--sort` —que va primero— lo pone en orden antes. El archivo lo indica en el primer nivel de narración (`<path>: N subtitles split, M merged`), y el registro `--format json` lleva los recuentos `split` y `merged`, **ausentes sin la opción**.

## Cuando se rechaza el corte

**Dos mitades que se solapan en el corte no se cortan.** Si un subtítulo de la cola empezara antes
del final del último subtítulo que queda atrás, el desplazamiento lo llevaría antes del inicio del vídeo —una posición negativa,
que ningún formato escribe—. `split-file` lo rechaza **nombrando el subtítulo**, con las palabras de la ventana:

```text
o.srt: Cannot split at subtitle 3: subtitle 3 would fall before the start of the video. Cut somewhere else.
```

El código es `2` y **no se escribe nada**. Hay que cortar en otro sitio. Caer exactamente en el inicio está permitido: cero es una
posición.

**Un `--at` fuera del archivo se rechaza del mismo modo**, una vez leído el archivo: `--at 1` —el primer subtítulo no puede
empezar una cola, no quedaría nada— y un `--at` más allá del último subtítulo, que dejaría una cola
vacía. Un archivo de un solo subtítulo no puede cortarse.

## Salida

**Salida estándar**: nada; el resultado son los dos archivos escritos.

| Nivel | Lo que se añade en la salida de error |
| :----- | :------------------------------------- |
| 1 | `<path>: split at subtitle N: H subtitles in the head, T subtitles in the tail -> <head>, <tail>`; con `--dry-run`, `(dry run, nothing written)` en lugar de `-> …` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — el formato, la codificación, la marca y los finales de línea del archivo leído, restituidos tal como estaban |
| 3 | `<path>: N bytes read, H and T written`, y después **cada diagnóstico de lectura** — véase [Invocación](invocation.md#diagnósticos-de-lectura) |

### En JSON

Con [`--format json`](invocation.md#una-salida-que-un-script-puede-leer), **un solo objeto**, cuyo `file` es el archivo cortado.
**`destination` es la cabeza** —el archivo en que se convierte la entrada— y **`tail` la cola**, una ruta, o `null` con
`--dry-run`, igual que `destination`. Los `counts`: `subtitles` (los del archivo leído), `head` y `tail` (los de cada
mitad).

## Códigos de salida

| Código | Cuándo |
| :--- | :---- |
| `0` | los dos archivos están escritos, o calculados con `--dry-run` |
| `1` | error de uso: faltan `--head` y `--tail`, una opción que `split-file` no tiene, un destino que es la entrada o el otro destino, un `--at` que no es un número entero positivo |
| `2` | el archivo no pudo leerse, **se rechaza el corte** (fuera del archivo, o solapamiento), o un destino no pudo escribirse. No hay código `3`: se procesa un solo archivo |

**La cabeza se escribe primero**: un disco que rechaza la cola deja la cabeza escrita, y el código es `2`.

## Errores

| Qué los provoca | Mensaje |
| :------------------- | :------ |
| ni `--head` ni `--tail` | `no destination given: split-file writes two files, use --head and --tail` |
| solo una de las dos, con `--dry-run` | `--head and --tail go together: give both, or neither with --dry-run` |
| los dos destinos son el mismo archivo | `<path>: named as both the head and the tail` |
| un destino es la entrada | `<path>: would be written over the input <input>` |
| `--at` fuera del archivo | `<file>: --at 9: the file holds 3 subtitles, and the tail can start from 2 to 3` |
| un archivo de un subtítulo | `<file>: a file of 1 subtitle cannot be split: a cut needs a subtitle on each side` |
| solapamiento | `<file>: Cannot split at subtitle N: subtitle M would fall before the start of the video. Cut somewhere else.` |
| un archivo ausente, ilegible o de formato desconocido | `<path>: does not exist`… — lo mismo que en todas partes |
