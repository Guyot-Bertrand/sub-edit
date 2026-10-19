# `join-repeats`

<!-- machine-translated -->
> Esta página es una traducción automática y todavía no la ha revisado ningún lector del idioma.

```
subedit-cli join-repeats (--output FILE | --output-dir DIR | --in-place)
                         [--dry-run]
                         [--recursive]
                         <file>...
```

Hace **un solo subtítulo** de los subtítulos que muestran lo mismo uno tras otro: un texto cortado en trozos
que se suceden, tal como lo dejan algunas herramientas de segmentación. Es el `Join Repeated Subtitles` de la ventana —
[el mismo motor](../subedit-gui/lines.md#unir-los-subtítulos-repetidos).

**Una repetición** es un subtítulo que tiene **el mismo texto, la misma traducción y los mismos ajustes de formato** (un
estilo, una posición, ajustes de indicación) que otro, **y que empieza como muy tarde cuando ese termina**: se
tocan o se solapan. Una serie de repeticiones se convierte en **la que empieza primero**, cuyo fin se
lleva al fin más tardío de la serie; las demás desaparecen.

- **El orden del archivo no cambia nada, y no se modifica**: el subtítulo que se conserva es el que empieza primero,
  estén donde estén los demás. Un archivo en orden sigue en orden.
- **Nada se une por error**: otro texto, otra traducción, otros ajustes, una separación entre dos
  trozos, un subtítulo sin texto o cuyo fin llega antes que su inicio — en todos estos casos se quedan separados.
- **Hay que hacerlo antes que lo demás**: la ordenación y [recortar los solapamientos](disjoint.md) leen mejor una serie limpia.
  [`repair`](repair.md) encadena las tres cosas, y [`convert`](convert.md#hacer-disjuntos-los-intervalos) lo acepta como opción (`--join-repeats`, `--repair`).

<!-- example: subedit-cli join-repeats --help -->
```console
$ subedit-cli join-repeats --help
Join the subtitles that repeat each other, one after the other, into one
Usage: subedit-cli join-repeats [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to join the repeats of

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
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
| `--output` / `--output-dir` / `--in-place` | **una de las tres**, salvo con `--dry-run` | véase [Invocación](invocation.md#el-destino) | — |
| `--dry-run` | no | un indicador: calcula e informa, no escribe nada — véase [Ver antes de escribir](invocation.md#ver-antes-de-escribir) | desactivado |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:04,000 --> 00:00:06,000\nAlpha\n\n4\n00:00:08,000 --> 00:00:09,000\nBeta\n\n' > morceaux.srt; subedit-cli join-repeats --output joint.srt morceaux.srt; cat joint.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:04,000 --> 00:00:06,000\nAlpha\n\n4\n00:00:08,000 --> 00:00:09,000\nBeta\n\n' > morceaux.srt; subedit-cli join-repeats --output joint.srt morceaux.srt; cat joint.srt
morceaux.srt: 2 repeated subtitles joined -> joint.srt
1
00:00:01,000 --> 00:00:06,000
Alpha

2
00:00:08,000 --> 00:00:09,000
Beta
```

Los tres trozos de `Alpha`, que se suceden de 1 s a 6 s, ya solo forman uno. `Beta`, al que precede una separación,
se ha quedado. Una segunda pasada no cambia nada y lo dice: `no repeated subtitles`.

## Salida

**Salida estándar** — nada: el resultado es el archivo escrito.

| Nivel | Lo que se añade en la salida de error |
| :----- | :------------------------------------- |
| 1 | `<path>: N repeated subtitles joined -> <destination>`, o `<path>: no repeated subtitles -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, y después **cada diagnóstico de lectura** |

### En JSON

Con [`--format json`](invocation.md#una-salida-que-un-script-puede-leer), los `counts` de cada archivo: `subtitles` (los del archivo
escrito), `joined` (los subtítulos eliminados porque otro los cubre) y `chains` (las series unidas).

## Códigos de salida

Los de la herramienta: `0` si se han procesado todos los archivos — un archivo sin repeticiones lo está —, `2` si ninguno, `3` si
solo algunos, `1` ante un error de uso.

## Errores

Ninguno propio de `join-repeats`: un archivo ilegible falla como en cualquier subcomando, y los errores del
destino son comunes a los subcomandos que escriben: véase [Invocación](invocation.md#el-destino).
