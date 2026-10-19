# `repair`

<!-- machine-translated -->
> Esta página es una traducción automática y todavía no la ha revisado ningún lector del idioma.

```
subedit-cli repair (--output FILE | --output-dir DIR | --in-place)
                   [--minimum TIME] [--max-lines N] [--dashes]
                   [--dry-run] [--recursive]
                   <file>...
```

Hace, de una vez, lo que tres comandos hacen uno tras otro: [`join-repeats`](join-repeats.md), después
[`sort`](sort.md), después [`disjoint`](disjoint.md). **En ese orden, que es el único que funciona**: el recorte lee
el orden de los inicios, y los trozos repetidos de un mismo texto conviene eliminarlos antes que cualquiera de los dos. Ante un archivo
mal segmentado, desordenado y con solapamientos, es el comando que hay que ejecutar.

La configuración es la de `disjoint`, que cierra la cadena; `--sort` no existe aquí, puesto que la ordenación siempre se hace.

<!-- example: subedit-cli repair --help -->
```console
$ subedit-cli repair --help
Join the repeated subtitles, put them in order, then make the ranges disjoint
Usage: subedit-cli repair [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to repair

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --minimum TIME              Shortest interval kept; a shorter one is taken over by a neighbour (default 0.08)
  --max-lines N               Most lines of a merged subtitle; more are shown in windows, 0 for no limit (default 2)
  --dashes                    Put a dash before the text of each merged subtitle
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Argumentos y opciones

| Opción | Obligatoria | Valores | Por defecto |
| :----- | :----- | :------ | :----- |
| `<file>...` | sí | una o varias rutas | — |
| `--minimum` | no | una duración: `0.08`, `1.5`, `00:00:00,080`. Cero conserva todos los intervalos | `0.08` (unos dos fotogramas) |
| `--max-lines` | no | un número entero; `0` suprime el límite | `2` |
| `--dashes` | no | un indicador: una raya delante del texto que cada subtítulo aporta a un subtítulo fusionado | desactivado |
| `--recursive`, `-r` | no | un indicador | desactivado — véase [Procesar un árbol](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **una de las tres**, salvo con `--dry-run` | véase [Invocación](invocation.md#el-destino) | — |
| `--dry-run` | no | un indicador: calcula e informa, no escribe nada — véase [Ver antes de escribir](invocation.md#ver-antes-de-escribir) | desactivado |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:01,000 --> 00:00:04,000\nBeta\n\n' > segments.srt; subedit-cli repair --output repare.srt segments.srt; cat repare.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:01,000 --> 00:00:04,000\nBeta\n\n' > segments.srt; subedit-cli repair --output repare.srt segments.srt; cat repare.srt
segments.srt: 1 repeated subtitle joined; already in order; 0 subtitles split, 1 merged -> repare.srt
1
00:00:01,000 --> 00:00:04,000
Alpha
Beta
```

`Alpha`, cortado en dos trozos que se suceden, vuelve a ser un solo subtítulo; ocupa entonces el mismo intervalo que `Beta`,
y los dos ya solo forman uno, de dos líneas, el más temprano primero.

## Salida

**Salida estándar** — nada: el resultado es el archivo escrito. En la salida de error, desde el primer nivel, **una frase
que encadena los tres recuentos**, separados por punto y coma: lo que dicen `join-repeats`, `sort` y `disjoint`, cada uno
con sus propias palabras — `N repeated subtitles joined; M subtitles moved; K subtitles split, L merged…`.

### En JSON

Con [`--format json`](invocation.md#una-salida-que-un-script-puede-leer), los `counts` de cada archivo: `subtitles`, `joined`,
`moved`, y después los de [`disjoint`](disjoint.md#en-json) (`split`, `merged`, `windowed`, `absorbed`, `too_short`,
`ending_before_start`, `extras_dropped`).

## Códigos de salida y errores

Los de la herramienta. Ningún error es propio de `repair` aparte de la configuración, que se juzga como la de `disjoint`:
un `--minimum` que no es una duración o que es negativo, un `--max-lines` que no es un número entero, código `1`.
