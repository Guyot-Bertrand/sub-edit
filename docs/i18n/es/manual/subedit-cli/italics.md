# `italics`

<!-- machine-translated -->
> Esta página es una traducción automática y todavía no la ha revisado ningún lector del idioma.

```
subedit-cli italics (--on | --off) [--range N-M|N-]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    <file>...
```

Pone los textos **en cursiva**, o **les quita la cursiva**, con las etiquetas del formato del archivo.
Es el `Italic` de la [ventana](../subedit-gui/operations.md).

**Dos palabras y no un conmutador**: el botón de la ventana pregunta al núcleo en qué sentido
actúa; aquí se dice. `--on` y `--off` se excluyen mutuamente, y hace falta una de las dos — omitirla, o dar ambas,
es un error de uso (código `1`).

- **`--on`** envuelve cada texto en un único par de etiquetas, **después de quitar las que
  ya contenía**: un texto ya en cursiva no se envuelve dos veces, y una cursiva cortada en dos
  se rehace de una sola pieza. Sobre varias líneas, un solo par cubre el conjunto.
- **`--off`** quita las etiquetas de cursiva; **el resto de una etiqueta se conserva** (`{\b1\i1}` da
  `{\b1}`).
- El texto se escribe **como el formato del archivo escribe la cursiva**: `<i>…</i>` para SubRip, WebVTT y
  SubViewer 2, `{\i1}…{\i0}` para los dos Sub Station Alpha, `{Y:i}` para MicroDVD.

<!-- example: subedit-cli italics --help -->
```console
$ subedit-cli italics --help
Put the texts in italics, or take their italics out
Usage: subedit-cli italics [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --on                        Put the texts in italics
  --off                       Take the italics out
  --range N-M|N-              Act only on subtitles N to M, or N to the end
  --document main|translation The document to change: the main one, or the translation given by -t
  -t,--translation-file FILE  Translation file to lay over the subtitle file, for a single input
  --align-method position|number
                              How the lines of the translation find their subtitles
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
| `--on` / `--off` | **una de las dos** | indicadores | — |
| `--range` | no | `N-M`, o `N-` hasta el final — véase [`adjust`](adjust.md#--range) | todo el archivo |
| `--document` | no | `main` o `translation` — véase [Una traducción](invocation.md#una-traducción) | `main` |
| `-t`, `--translation-file` | con `--document translation` | un archivo de traducción, para **una sola** entrada | — |
| `--align-method` | no, y solo con `-t` | `position` o `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **una de las tres**, salvo con `--dry-run` | véase [Invocación](invocation.md#el-destino) | — |
| `--dry-run` | no | un indicador: calcula e informa, no escribe nada — véase [Ver antes de escribir](invocation.md#ver-antes-de-escribir) | desactivado |

**El texto principal por defecto; la traducción con `--document translation -t FILE`.** Entonces es
la traducción la que se modifica y se escribe, y solo ella — véase [Una traducción](invocation.md#una-traducción).

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour\nMarie\n\n' > a.srt; subedit-cli italics --on --output i.srt a.srt; cat i.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour\nMarie\n\n' > a.srt; subedit-cli italics --on --output i.srt a.srt; cat i.srt
a.srt: 2 subtitles put in italics -> i.srt
1
00:00:01,000 --> 00:00:02,000
<i>bonjour marie</i>

2
00:00:03,000 --> 00:00:04,000
<i>Bonjour
Marie</i>
```

## Un formato sin estilo se niega

**TMPlayer y LRC no escriben ningún estilo**: no hay cursiva que poner ni cursiva que quitar. La
ventana atenúa la entrada en lugar de ocultarla; la línea de comandos **se niega, archivo por
archivo, dando el motivo**, y los demás archivos del lote siguen adelante. Lo que cuenta es lo que el
formato puede conservar, no la manera en que lo escribe: SubRip y WebVTT escriben la cursiva de la misma
manera, y no dicen lo mismo sobre el color.

<!-- example: printf '00:00:01:Hello there\n00:00:03:Bye\n' > a.txt; subedit-cli italics --on --output-dir sortie a.txt; echo "code=$?" -->
```console
$ printf '00:00:01:Hello there\n00:00:03:Bye\n' > a.txt; subedit-cli italics --on --output-dir sortie a.txt; echo "code=$?"
a.txt: TMPlayer writes no style: there are no italics to put on
code=2
```

En JSON, el fallo lleva el identificador **`no-style`**.

## Salida

**Salida estándar** — nada: el resultado es el archivo escrito. Con `--dry-run`, la lista de
cambios, descrita en [Ver antes de escribir](invocation.md#ver-antes-de-escribir).

| Nivel | Lo que se añade en la salida de error |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles put in italics -> <destination>`, o `N subtitles taken out of italics`, o `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — el formato, la codificación, la marca y los finales de línea del archivo leído, restituidos tal como estaban |
| 3 | `<path>: N bytes read, M written`, y después **cada diagnóstico de lectura** — véase [Invocación](invocation.md#diagnósticos-de-lectura) |

`N` es el número de subtítulos **cuyo texto ha cambiado**, leído de la propia operación.

### En JSON

Con [`--format json`](invocation.md#una-salida-que-un-script-puede-leer), el objeto de cada archivo lleva
`counts.changed` — el `N` de la frase — y `changes`, la lista de los subtítulos modificados
(`{"subtitle", "document", "before", "after"}`, **vacía** cuando nada ha cambiado).

## Códigos de salida

Los de la herramienta: `0` si se han procesado todos los archivos — un archivo en el que nada cambia lo está —,
`2` si ninguno, `3` si solo algunos, `1` ante un error de uso.

## Errores

| Qué lo provoca | Mensaje |
| :------------------ | :------ |
| ni `--on` ni `--off` | `italics needs --on to put the texts in italics, or --off` |
| ambas | `--on and --off say opposite things; give one of them` |
| un formato sin estilo | `<path>: TMPlayer writes no style: there are no italics to put on` (o `take out`) |
| intervalo mal escrito o fuera del archivo | véase [`adjust`](adjust.md#--range) |

Los errores del destino son comunes a los subcomandos que escriben:
véase [Invocación](invocation.md#el-destino).
