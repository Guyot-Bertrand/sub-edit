# `case`

<!-- machine-translated -->
> Esta página es una traducción automática y todavía no la ha revisado ningún lector del idioma.

```
subedit-cli case --to title|sentence|upper|lower [--range N-M|N-]
                 (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 [--document main|translation] [-t FILE] [--align-method position|number]
                 <file>...
```

Pone los textos en **tipo título, tipo oración, mayúsculas o minúsculas**, sin tocar ninguna
etiqueta. Es el `Herramientas ▸ Capitalización` de la [ventana](../subedit-gui/operations.md), con sus reglas.

| `--to` | Efecto | `bonjour marie` se convierte en |
| :----- | :---- | :---------------------- |
| `title` | cada palabra lleva mayúscula inicial, el resto pasa a minúsculas | `Bonjour Marie` |
| `sentence` | la primera letra del texto, y solo ella; el resto pasa a minúsculas | `Bonjour marie` |
| `upper` | todo en mayúsculas | `BONJOUR MARIE` |
| `lower` | todo en minúsculas | `bonjour marie` |

- **Lo que precede a la primera letra o a la primera cifra no se toca**: una raya de
  diálogo, unas comillas de apertura o un paréntesis se mantienen (`- bonjour` da `- Bonjour`).
- **Las etiquetas no son texto**: no cambian de capitalización — `<I>` sigue siendo `<I>` — ni
  cambian de sitio: una etiqueta que corta una palabra la sigue cortando después.
- **ICU decide dónde empieza una palabra**: `l'été sera chaud` da `L'été Sera Chaud`, y los acentos
  se mantienen (`éléonore` da `Éléonore`).

<!-- example: subedit-cli case --help -->
```console
$ subedit-cli case --help
Put the texts in title, sentence, upper or lower case, tags intact
Usage: subedit-cli case [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --to title|sentence|upper|lower
                              The case to put the texts in
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
| `--to` | **sí** | `title`, `sentence`, `upper`, `lower` | — |
| `--range` | no | `N-M`, o `N-` hasta el final — véase [`adjust`](adjust.md#--range) | todo el archivo |
| `--document` | no | `main` o `translation` — véase [Una traducción](invocation.md#una-traducción) | `main` |
| `-t`, `--translation-file` | con `--document translation` | un archivo de traducción, para **una sola** entrada | — |
| `--align-method` | no, y solo con `-t` | `position` o `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **una de las tres**, salvo con `--dry-run` | véase [Invocación](invocation.md#el-destino) | — |
| `--dry-run` | no | un indicador: calcula e informa, no escribe nada — véase [Ver antes de escribir](invocation.md#ver-antes-de-escribir) | desactivado |

**El texto principal por defecto; la traducción con `--document translation -t FILE`.** Entonces es
la traducción la que se modifica y se escribe, y solo ella — véase [Una traducción](invocation.md#una-traducción).

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\n- BONJOUR MARIE\n\n' > a.srt; subedit-cli case --to title --output titre.srt a.srt; cat titre.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\n- BONJOUR MARIE\n\n' > a.srt; subedit-cli case --to title --output titre.srt a.srt; cat titre.srt
a.srt: 2 subtitles recased -> titre.srt
1
00:00:01,000 --> 00:00:02,000
<i>Bon</i>jour Marie

2
00:00:03,000 --> 00:00:04,000
- Bonjour Marie
```

Un archivo en el que **ningún texto cambia** — todo está ya en la capitalización pedida — se escribe tal cual y
lo dice:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBONJOUR\n\n' > a.srt; subedit-cli case --to upper --output b.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBONJOUR\n\n' > a.srt; subedit-cli case --to upper --output b.srt a.srt
a.srt: nothing to change -> b.srt
```

## Salida

**Salida estándar** — nada: el resultado es el archivo escrito. Con `--dry-run`, la lista de
cambios, descrita en [Ver antes de escribir](invocation.md#ver-antes-de-escribir).

| Nivel | Lo que se añade en la salida de error |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles recased -> <destination>`, o `nothing to change` |
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
| falta `--to` | `--to is required` |
| un valor que no es ninguno de los cuatro | `--to: … not in {title,sentence,upper,lower}` |
| intervalo mal escrito o fuera del archivo | véase [`adjust`](adjust.md#--range) |

Los errores del destino son comunes a los subcomandos que escriben:
véase [Invocación](invocation.md#el-destino).
