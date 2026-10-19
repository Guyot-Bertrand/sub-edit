# `replace`

<!-- machine-translated -->
> Esta página es una traducción automática y todavía no la ha revisado ningún lector del idioma.

```
subedit-cli replace [--regex] [--case-sensitive] [--range N-M|N-]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    <pattern> <replacement> <file>...
```

Reemplaza un texto en los subtítulos **sin romper ninguna etiqueta**. Es la
operación `Replace All` de la [búsqueda](../subedit-gui/find-and-replace.md) de la ventana, con sus reglas:

- **la búsqueda se hace en el texto visible**, lo que lee el espectador. Las etiquetas no forman
  parte de él: buscar `<i>` no encuentra nada, y buscar `Bonjour` encuentra un «Bonjour» que está
  a medias en cursiva;
- **el reemplazo se hace en el texto fuente**, el del archivo, conservando las etiquetas donde
  estaban. Una etiqueta que corta la palabra buscada encierra todo el reemplazo; un estilo que
  toca la coincidencia cubre todo el reemplazo; lo que la coincidencia no toca
  no se mueve. Las reglas, caso por caso, son las de la búsqueda de la ventana.

**No existe un subcomando `find`**: para ver dónde está un patrón sin cambiar nada,
es [`--dry-run`](invocation.md#ver-antes-de-escribir) el que enumera cada subtítulo afectado.

<!-- example: subedit-cli replace --help -->
```console
$ subedit-cli replace --help
Replace a text in the subtitles, without breaking a tag
Usage: subedit-cli replace [OPTIONS] pattern replacement files...

Positionals:
  pattern TEXT REQUIRED       Text to look for in what is shown
  replacement TEXT REQUIRED   What replaces it, tags allowed
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --regex                     Read the text to look for as a regular expression
  --case-sensitive            Tell capitals from small letters
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
| `<pattern>` | sí | el texto que buscar en lo que se muestra; **no vacío** | — |
| `<replacement>` | sí | lo que lo reemplaza, se admiten etiquetas; puede estar vacío | — |
| `<file>...` | sí | una o varias rutas | — |
| `--recursive`, `-r` | no | un indicador | desactivado — véase [Procesar un árbol](lots.md) |
| `--regex` | no | un indicador: el patrón es una expresión regular ICU | texto literal |
| `--case-sensitive` | no | un indicador: distingue mayúsculas de minúsculas | se ignoran |
| `--range` | no | `N-M`, o `N-` hasta el final — véase [`adjust`](adjust.md#--range) | todo el archivo |
| `--document` | no | `main` o `translation` — véase [Una traducción](invocation.md#una-traducción) | `main` |
| `-t`, `--translation-file` | con `--document translation` | un archivo de traducción, para **una sola** entrada | — |
| `--align-method` | no, y solo con `-t` | `position` o `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **una de las tres**, salvo con `--dry-run` | véase [Invocación](invocation.md#el-destino) | — |
| `--dry-run` | no | un indicador: calcula e informa, no escribe nada — véase [Ver antes de escribir](invocation.md#ver-antes-de-escribir) | desactivado |

**Un patrón que empieza por un guion** se escribe después de `--`: `subedit-cli replace -- -- "—" film.srt`.

**El texto principal por defecto; la traducción con `--document translation -t FILE`.** En ese caso
es la traducción la que se modifica y se escribe, y solo ella — véase [Una traducción](invocation.md#una-traducción).

### Texto literal o expresión

Por defecto, el patrón se **lee tal cual**: `a.b` busca una `a`, un punto y una `b`. Las mayúsculas y minúsculas
se ignoran con la misma finura que en la ventana: `É` y `é` son una sola letra.

Con `--regex`, el patrón es una expresión regular **ICU**, leída como la lee Gaupol: `.` atraviesa
un salto de línea, `^` y `$` valen en cada línea de un subtítulo. En el reemplazo, y
solo en él:

| Se escribe | Para indicar |
| :----- | :-------- |
| `$1` a `$9` | los grupos de la coincidencia |
| `$0` | la coincidencia entera |
| `\n` | un salto de línea |
| `\$`, `\\` | un `$`, una barra invertida |

Cuidado con las comillas del shell: `'$1 EUR'`, y no `"$1 EUR"`, que dejaría que el shell leyera `$1`.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nPrix: 12 euros, et 7 euros.\n\n' > prix.srt; subedit-cli replace --regex '([0-9]+) euros' '$1 EUR' --output euros.srt prix.srt; cat euros.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nPrix: 12 euros, et 7 euros.\n\n' > prix.srt; subedit-cli replace --regex '([0-9]+) euros' '$1 EUR' --output euros.srt prix.srt; cat euros.srt
prix.srt: replaced 2 matches -> euros.srt
1
00:00:01,000 --> 00:00:02,000
Prix: 12 EUR, et 7 EUR.
```

## Etiquetas

Un texto en el que una etiqueta corta la palabra buscada:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace Bonjour Salut --output salut.srt balises.srt; cat salut.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace Bonjour Salut --output salut.srt balises.srt; cat salut.srt
balises.srt: replaced 2 matches -> salut.srt
1
00:00:01,000 --> 00:00:02,000
<i>Salut</i> tout le monde

2
00:00:03,000 --> 00:00:04,000
Salut <i>Marie</i>, ca va
```

En el primer subtítulo, la etiqueta cortaba «Bonjour»: ahora encierra «Salut».
En el segundo, no se toca y se queda donde está. **Una etiqueta que quedó abierta
sigue abierta**: reparar un marcado inestable sería otra operación.

El reemplazo se lee en **las etiquetas del formato del archivo**: `<i>…</i>` para SubRip,
`{\i1}…` para Advanced SSA. Es la única manera de dar formato a lo que se escribe.

## Nada encontrado, nada que cambiar

Dos respuestas que no son fallos: el código de salida es `0` y **el archivo se escribe
de todos modos**, porque un destino indicado es un destino escrito:

| Situación | Frase |
| :-------- | :----- |
| el patrón no está en ninguna parte | `"zzz" not found` |
| está, pero cada coincidencia se reemplaza por sí misma | `nothing to change` |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli replace zzz x --output b.srt a.srt; subedit-cli replace Bonjour Bonjour --output c.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli replace zzz x --output b.srt a.srt; subedit-cli replace Bonjour Bonjour --output c.srt a.srt
a.srt: "zzz" not found -> b.srt
a.srt: nothing to change -> c.srt
```

## `--dry-run`

La salida estándar enumera **cada subtítulo cuyo texto cambiaría**, antes y después, tal como
están en el archivo, etiquetas incluidas:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace --dry-run Bonjour Salut balises.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace --dry-run Bonjour Salut balises.srt
balises.srt: replaced 2 matches (dry run, nothing written)
balises.srt: subtitle 1
- <i>Bon</i>jour tout le monde
+ <i>Salut</i> tout le monde
balises.srt: subtitle 2
- Bonjour <i>Marie</i>, ca va
+ Salut <i>Marie</i>, ca va
```

La forma de la lista se describe en [Ver antes de escribir](invocation.md#ver-antes-de-escribir);
en JSON, es `changes`.

## Salida

**Salida estándar**: nada; el resultado es el archivo escrito. Con `--dry-run`, la lista de
cambios de arriba.

| Nivel | Lo que se añade en la salida de error |
| :----- | :------------------------------------- |
| 1 | `<path>: replaced N matches -> <destination>`, o `"<pattern>" not found`, o `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — el formato, la codificación, la marca y los finales de línea del archivo leído, restituidos tal como estaban |
| 3 | `<path>: N bytes read, M written`, y después **cada diagnóstico de lectura** — véase [Invocación](invocation.md#diagnósticos-de-lectura) |

`N` cuenta **las coincidencias reemplazadas en los subtítulos cuyo texto cambió**: es también
el número que indica la ventana.

### En JSON

Con [`--format json`](invocation.md#una-salida-que-un-script-puede-leer), el objeto de cada archivo lleva:

| Clave | Contenido |
| :-- | :------ |
| `counts.replaced` | las coincidencias reemplazadas, en los subtítulos cuyo texto cambió: el `N` de la frase |
| `counts.matched` | las coincidencias encontradas, **haya cambiado o no el texto su reemplazo**: es lo que distingue «nada encontrado» (`0`) de «nada que cambiar» (`replaced` a `0`, `matched` positivo) |
| `changes` | los subtítulos modificados: `{"subtitle", "document", "before", "after"}`, **vacío** cuando nada ha cambiado |

## Códigos de salida

Los de la herramienta: `0` si se procesaron todos los archivos —y un archivo en el que no se encontró nada
lo está—, `2` si ninguno, `3` si solo algunos, `1` en caso de error de uso.

## Errores

| Qué lo provoca | Mensaje |
| :------------------ | :------ |
| patrón vacío | `pattern: nothing to look for` |
| expresión ilegible | `pattern: not a regular expression (<ICU's reason>)` |
| intervalo mal escrito o fuera del archivo | véase [`adjust`](adjust.md#--range) |

**El patrón se lee antes del primer archivo**: una expresión ilegible es un error de uso
(código `1`), y no se lee ni se escribe nada.

Los errores del destino son comunes a los subcomandos que escriben:
véase [Invocación](invocation.md#el-destino).
