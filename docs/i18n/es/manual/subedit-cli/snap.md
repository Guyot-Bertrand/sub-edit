# `snap`

<!-- machine-translated -->
> Esta página es una traducción automática y todavía no la ha revisado ningún lector del idioma.

```
subedit-cli snap --rate <rate>
                 (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 <file>...
```

Coloca **cada marca de tiempo** — inicio y fin — en el fotograma más cercano de la
velocidad de fotogramas indicada. No se reajusta ningún tiempo: cada posición se mueve medio fotograma
como máximo.

## `snap` no es `framerate`, y confundirlos no se nota

Los dos comandos tienen la misma forma — archivos, un destino, una velocidad de fotogramas — y hacen cosas opuestas.

| | [`framerate`](framerate.md) | `snap` |
| :--- | :-------------------------- | :----- |
| qué calcula | `t' = t x input rate / output rate` | el fotograma más cercano, en la rejilla de `--rate` |
| cuánto mueve | en proporción al tiempo — **varios segundos** en un largometraje | medio fotograma como máximo, **en todas partes** |
| para qué archivo | un archivo cuya **temporización es incorrecta** | un archivo cuya temporización es **correcta** y cuya rejilla es incorrecta |

**El caso de uso de `snap`**, explicado: una película a 25 fotogramas por
segundo, un archivo de subtítulos escrito sobre una rejilla a 24, líneas ya
más o menos en su sitio con unos pocos milisegundos de diferencia. No hay nada que reajustar
— solo devolver las marcas de tiempo a los fotogramas correctos.

**El caso de uso de `framerate`** es el otro: un archivo temporizado para un máster a
25 que se quiere reproducir en una copia a 23,976, y cuya temporización deriva
varios segundos hacia el final de la película.

Tomar uno por otro **no provoca ningún error**: los dos
aceptan el archivo y escriben un resultado. La línea de narración es lo que
los distingue — `snap` siempre dice cuánto ha movido las posiciones.

<!-- example: subedit-cli snap --help -->
```console
$ subedit-cli snap --help
Move every position onto the nearest frame of a frame rate (see framerate)
Usage: subedit-cli snap [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to align

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --rate TEXT REQUIRED        Frame rate to align on: 25, 23.976
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
| `--rate` | **sí** | una velocidad de fotogramas en fotogramas por segundo, escrita como para [`framerate`](framerate.md#cómo-se-escribe-una-velocidad-de-fotogramas) | — |
| `--output` / `--output-dir` / `--in-place` | **una de las tres**, salvo con `--dry-run` | véase [Invocación](invocation.md#el-destino) | — |
| `--dry-run` | no | un indicador: calcula e informa, no escribe nada — véase [Ver antes de escribir](invocation.md#ver-antes-de-escribir) | desactivado |

`--rate` acepta **cualquier velocidad de fotogramas válida**, y no solo las ocho
estándar: es elección del usuario, como para `framerate`. Una velocidad de fotogramas
nula o negativa se rechaza.

El formato del archivo leído **se conserva**: cambiar de formato es tarea de
[`convert`](convert.md).

## Salida

**Salida estándar** — nada: el resultado es el archivo escrito. Con `--dry-run`, tampoco nada: no hay archivo, y la línea de narración lo dice — véase [Ver antes de escribir](invocation.md#ver-antes-de-escribir).

| Nivel | Lo que se añade en la salida de error |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles aligned on 25 fps, M positions moved, by at most 20 ms -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — el formato, la codificación, la marca y los finales de línea del archivo leído, restituidos tal como estaban |
| 3 | `<path>: N bytes read, M written`, y después **cada diagnóstico de lectura** — véase [Invocación](invocation.md#diagnósticos-de-lectura) |

**Las dos cifras son la razón de ser de la línea.** Medio fotograma a 25 fotogramas por
segundo son veinte milisegundos, y allí no puede escribirse nada mayor. Quien
quería `framerate` y ha tecleado `snap` lee «by at most 20 ms» donde esperaba
segundos, y lo sabe enseguida.

## Un ejemplo

Una línea en `00:00:01,010`, alineada a 25 fotogramas por segundo — es decir, sobre un
múltiplo de cuarenta milisegundos:

<!-- example: printf '1\n00:00:01,010 --> 00:00:03,020\nUn.\n\n' > a.srt; subedit-cli snap --rate 25 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,010 --> 00:00:03,020\nUn.\n\n' > a.srt; subedit-cli snap --rate 25 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle aligned on 25 fps, 2 positions moved, by at most 20 ms -> b.srt
1
00:00:01,000 --> 00:00:03,040
Un.
```

## Dos propiedades

**Alinear dos veces no cambia nada la segunda vez.** Una posición que ya está en un
fotograma se queda en él: es la misma operación aplicada al mismo resultado.

**El orden de los subtítulos se conserva.** El redondeo al fotograma más cercano es
monótono, de modo que dos inicios separados al menos por un fotograma lo siguen estando. Más
próximos, pueden confundirse — lo que es una anomalía que
[`inspect`](inspect.md#lo-que-indica-anomalies) sabe nombrar, y no una
inversión.

### En JSON

Con [`--format json`](invocation.md#una-salida-que-un-script-puede-leer), el objeto de cada archivo lleva `counts.subtitles`, `counts.moved` (las posiciones movidas) y `counts.furthest_ms` (el mayor desplazamiento, en milisegundos). La tabla completa es la de [Invocación](invocation.md#lo-que-cada-subcomando-pone-en-counts).

## Códigos de salida

Los de la herramienta: `0` si se han escrito todos los archivos, `2` si ninguno, `3` si
solo algunos, `1` ante un error de uso.

## Errores

Los de una velocidad de fotogramas, palabra por palabra los de [`framerate`](framerate.md#errores)
— ambos leen el mismo valor por el mismo camino:

| Qué lo provoca | Mensaje |
| :------------------ | :------ |
| velocidad de fotogramas nula o negativa | `"…" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed` |
| coma decimal | `"…" is not a frame rate: use a decimal point and not a comma, in every language` |
| texto que no es un número | `"…" is not a frame rate: expected frames per second, like 25 or 23.976` |
| velocidad de fotogramas desproporcionada | `"…" is not a frame rate: no video runs at that many frames per second` |

**No se escribe nada sin una de las tres opciones de destino**, y sus errores
son comunes a los subcomandos que escriben:
véase [Invocación](invocation.md#el-destino).
