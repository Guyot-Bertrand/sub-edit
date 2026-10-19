# `dialogue-dashes`

<!-- machine-translated -->
> Esta página es una traducción automática y todavía no la ha revisado ningún lector del idioma.

```
subedit-cli dialogue-dashes (--add | --remove) [--range N-M|N-]
                            (--output FILE | --output-dir DIR | --in-place)
                            [--dry-run]
                            [--recursive]
                            [--document main|translation] [-t FILE] [--align-method position|number]
                            <file>...
```

Pone una **raya de diálogo** al principio de las líneas, o la quita. Es el `Dialogue Dashes` de la
[ventana](../subedit-gui/operations.md).

**Dos palabras, como para [`italics`](italics.md)**: `--add` y `--remove` se excluyen mutuamente, y hace falta una de las dos.

- **`--add`** pone una raya delante de cada línea que no la tiene: `Bonjour` da `- Bonjour`, y
  `Bonjour\nMarie` da `- Bonjour\n- Marie`. **Una raya que ya está no se duplica.**
- **Se reconocen tres rayas** — el guion del teclado y las dos rayas largas de la tipografía —
  **y lo que se escribe es el guion**: `— Bonjour` se convierte en `- Bonjour`. Los espacios sobrantes
  tras la raya desaparecen.
- La raya va **delante del texto, no delante de una etiqueta de apertura**: `<i>Bonjour</i>` da
  `<i>- Bonjour</i>`. Ninguna etiqueta se mueve.
- **`--remove`** quita las rayas iniciales, en sus tres formas.

<!-- example: subedit-cli dialogue-dashes --help -->
```console
$ subedit-cli dialogue-dashes --help
Put dialogue dashes on the lines, or take them off
Usage: subedit-cli dialogue-dashes [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --add                       Put a dialogue dash at the head of the lines
  --remove                    Take the dialogue dashes off
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
| `--add` / `--remove` | **una de las dos** | indicadores | — |
| `--range` | no | `N-M`, o `N-` hasta el final — véase [`adjust`](adjust.md#--range) | todo el archivo |
| `--document` | no | `main` o `translation` — véase [Una traducción](invocation.md#una-traducción) | `main` |
| `-t`, `--translation-file` | con `--document translation` | un archivo de traducción, para **una sola** entrada | — |
| `--align-method` | no, y solo con `-t` | `position` o `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **una de las tres**, salvo con `--dry-run` | véase [Invocación](invocation.md#el-destino) | — |
| `--dry-run` | no | un indicador: calcula e informa, no escribe nada — véase [Ver antes de escribir](invocation.md#ver-antes-de-escribir) | desactivado |

**El texto principal por defecto; la traducción con `--document translation -t FILE`.** Entonces es
la traducción la que se modifica y se escribe, y solo ella — véase [Una traducción](invocation.md#una-traducción).

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\nMarie\n\n2\n00:00:03,000 --> 00:00:04,000\n<i>Salut</i>\n\n' > a.srt; subedit-cli dialogue-dashes --add --output t.srt a.srt; cat t.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\nMarie\n\n2\n00:00:03,000 --> 00:00:04,000\n<i>Salut</i>\n\n' > a.srt; subedit-cli dialogue-dashes --add --output t.srt a.srt; cat t.srt
a.srt: 2 subtitles dashed -> t.srt
1
00:00:01,000 --> 00:00:02,000
- Bonjour
- Marie

2
00:00:03,000 --> 00:00:04,000
<i>- Salut</i>
```

## Salida

**Salida estándar** — nada: el resultado es el archivo escrito. Con `--dry-run`, la lista de
cambios, descrita en [Ver antes de escribir](invocation.md#ver-antes-de-escribir).

| Nivel | Lo que se añade en la salida de error |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles dashed -> <destination>`, o `N subtitles undashed`, o `nothing to change` |
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
| ni `--add` ni `--remove` | `dialogue-dashes needs --add to put dashes on, or --remove` |
| ambas | `--add and --remove say opposite things; give one of them` |
| intervalo mal escrito o fuera del archivo | véase [`adjust`](adjust.md#--range) |

Los errores del destino son comunes a los subcomandos que escriben:
véase [Invocación](invocation.md#el-destino).
