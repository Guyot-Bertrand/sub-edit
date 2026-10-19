# `append`

<!-- machine-translated -->
> Esta página es una traducción automática y todavía no la ha revisado ningún lector del idioma.

```
subedit-cli append --output FILE
                   [--dry-run] [--sort] [--disjoint]
                   <base> <file>...
```

Pone archivos **después del primero**, en **un solo** archivo: la película en dos partes, convertida en un
único conjunto de subtítulos. Es `Append File…` de la ventana —[el mismo motor](../subedit-gui/operations.md#append-file),
los mismos recuentos—, aplicado tantas veces como archivos haya que añadir.

- **El primer archivo es la base.** Da al resultado **su formato, su codificación, su marca de orden
  de bytes y sus finales de línea**.
- **No se inserta nada entre dos archivos**: cada uno se desplaza hasta **el final del último subtítulo de lo
  que lo precede**, y sus posiciones conservan su orden y su espaciado. El tercero sigue al segundo, no a la
  base: los archivos se suceden en el orden en que se dan.
- **Cada archivo añadido pasa por la conversión** al formato de la base: sus etiquetas se traducen,
  y lo que ese formato no puede escribir se indica, archivo por archivo, con las palabras de la ventana.
- **Los archivos añadidos no tienen traducción**: un archivo abierto solo no tiene ninguna que dar.
- **Un archivo puede darse varias veces**, o añadirse a sí mismo: nada lo impide.

**`append` no es un lote.** N entradas, **una** salida: el bucle de los demás subcomandos —un resultado
por archivo, y un fallo que deja seguir a los demás— no tiene sentido aquí. De ello se derivan tres cosas:

- **el destino es `--output`, y nada más.** `--output-dir` y `--in-place` no existen para
  `append` y se rechazan como cualquier opción desconocida, código `1`; `--recursive` también: el orden de los
  archivos es el indicado, y un árbol recorrido no tiene ninguno;
- **todo o nada**: el primer archivo que no puede leerse detiene la ejecución, y **no se escribe nada**. Una
  película en tres partes de la que falta una no es una película en dos;
- **una sola línea por ejecución** para el resultado y, en JSON, **un solo objeto**.

<!-- example: subedit-cli append --help -->
```console
$ subedit-cli append --help
Put subtitle files one after another into a single file
Usage: subedit-cli append [OPTIONS] files...

Positionals:
  files BASE FILE...          The base file, then the files to put after it, in order

Options:
  -h,--help                   Print this help message and exit
  --output FILE               File to write
  --dry-run                   Work out and say what would be written, and write nothing
  --sort                      Put the subtitles in order of their start before writing
  --disjoint                  Cut overlapping subtitles so that no range overlaps another, before writing
```

## Argumentos y opciones

| Opción | Obligatoria | Valores | Por defecto |
| :----- | :----- | :------ | :----- |
| `<base> <file>...` | sí, **al menos dos** | rutas: la base y después lo que se pone tras ella, en orden | — |
| `--output` | **sí**, salvo con `--dry-run` | una ruta de archivo; su directorio se crea si falta | — |
| `--dry-run` | no | un indicador: calcula e informa, no escribe nada — véase [Ver antes de escribir](invocation.md#ver-antes-de-escribir) | desactivado |
| `--sort` | no | un indicador: pone el conjunto en el orden de sus inicios **una vez añadido todo**, véase [Ordenar el resultado](#ordenar-el-resultado) | desactivado |
| `--disjoint` | no | un indicador: corta los subtítulos solapados **una vez añadido y ordenado todo**, véase [Hacer disjuntos los intervalos](#hacer-disjuntos-los-intervalos) | desactivado |

**El destino no puede ser ninguna de las entradas**, base incluida: escribir sobre un archivo que se está
leyendo se rechaza antes de leer nada, código `1`. Un `--output` existente distinto de las entradas se
sobrescribe, como en todas partes.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n' > partie1.srt; printf '1\n00:00:00,500 --> 00:00:01,500\ntrois\n\n' > partie2.srt; subedit-cli append --output film.srt partie1.srt partie2.srt; cat film.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n' > partie1.srt; printf '1\n00:00:00,500 --> 00:00:01,500\ntrois\n\n' > partie2.srt; subedit-cli append --output film.srt partie1.srt partie2.srt; cat film.srt
partie2.srt: appended 1 subtitle
partie1.srt: 3 subtitles from 2 files -> film.srt
1
00:00:01,000 --> 00:00:02,000
un

2
00:00:03,000 --> 00:00:04,000
deux

3
00:00:04,500 --> 00:00:05,500
trois
```

El último subtítulo de `partie1.srt` termina a los 4 s; el de `partie2.srt`, que empezaba a los 0,5 s, empieza a los
4,5 s.

## Ordenar el resultado

Cada archivo se desplaza **desde el final de lo que lo precede**: el orden que puede romperse es, por tanto, el de los propios archivos. `--sort`
pone el resultado en el orden de los inicios, **de una sola vez, tras la última adición**, como lo haría [`sort`](sort.md): de forma estable, y sin cambiar nada
del contenido.

Nunca por defecto. Con la opción, la base lo indica en el primer nivel de narración, `<base>: N subtitles moved` o `<base>: already in order`, y
el registro `--format json` lleva un recuento `moved`, **ausente sin la opción**.

## Hacer disjuntos los intervalos

Nunca por defecto. La opción hace lo que hace [`disjoint`](disjoint.md), **con su configuración por defecto**, una vez añadido todo, sobre el conjunto. Necesita un archivo en orden: si no, se rechaza, y `--sort` —que va primero— lo pone en orden antes. El archivo lo indica en el primer nivel de narración (`<path>: N subtitles split, M merged`), y el registro `--format json` lleva los recuentos `split` y `merged`, **ausentes sin la opción**.

## Cuando los formatos difieren

El resultado está en **el formato de la base**, sea cual sea el de los demás. Lo que cuesta el cruce se
indica archivo por archivo, con las palabras de la ventana: la misma frase que `Append File…` y que
[pegar desde otro formato](../subedit-gui/clipboard.md#las-etiquetas-de-otro-formato-se-traducen):

```text
d.ass: appended 1 subtitle; Advanced SSA into SubRip: 1 tag dropped, the header was dropped
```

Lo que se pierde depende solo del formato de **cada** archivo añadido: una base en Advanced SSA recibe un
archivo SubRip sin perder nada, pero no a la inversa. Para elegir el formato del resultado, se pone primero un archivo
que ya esté en ese formato, o se convierte después con [`convert`](convert.md).

## Salida

**Salida estándar**: nada; el resultado es el archivo escrito.

| Nivel | Lo que se añade en la salida de error |
| :----- | :------------------------------------- |
| 1 | una línea por archivo añadido —`<path>: appended N subtitles`, seguida, cuando el formato ha costado algo, de `; <source format> into <base format>: <what is lost>`— y después `<base>: N subtitles from M files -> <destination>` (con `--dry-run`, `(dry run, nothing written)` en lugar de `-> <destination>`) |
| 2 | `<base>: SubRip, UTF-8, no BOM, LF line endings kept` — el formato, la codificación, la marca y los finales de línea de la base, restituidos tal como estaban |
| 3 | `<base>: N bytes read, M written` (todos los archivos juntos), y después **cada diagnóstico de lectura** — véase [Invocación](invocation.md#diagnósticos-de-lectura) |

### En JSON

Con [`--format json`](invocation.md#una-salida-que-un-script-puede-leer), **un solo objeto** para toda la ejecución,
cuyo `file` es **la base**. Sus `counts`:

| Recuento | Lo que indica |
| :----- | :----------- |
| `files` | los archivos de la ejecución, base incluida |
| `subtitles` | los subtítulos del resultado |
| `appended` | los que proceden de los archivos añadidos |
| `lost_ends`, `lost_header` | `0` o `1`: uno de los archivos añadidos lo perdió |
| `joined_lines`, `lost_tags`, `lost_fields` | la suma de lo que perdieron los archivos añadidos |
| `furthest_ms` | el mayor desplazamiento de una posición, todos los archivos juntos |

**Un array `inputs`** acompaña a los `counts`, fuera de ellos: un elemento por archivo añadido, en orden,
`{"file", "counts"}`, con `counts` llevando `appended` y después las mismas pérdidas, **solo para ese archivo**. Es lo que
indica, en una ejecución de diez archivos, cuál ha costado qué. Los avisos de lectura de todos los archivos
se reúnen en `warnings`.

Un fallo también es un objeto, **uno solo**: el del primer archivo que no pudo leerse.

## Códigos de salida

| Código | Cuándo |
| :--- | :---- |
| `0` | el resultado está escrito, o calculado con `--dry-run` |
| `1` | error de uso: menos de dos archivos, falta `--output`, una opción que `append` no tiene, un destino que es una de las entradas |
| `2` | **un archivo no pudo leerse**, o el destino no pudo escribirse: no se escribe nada. No hay código `3`: una ejecución que solo escribe un archivo tiene éxito o no lo tiene |

## Errores

| Qué los provoca | Mensaje |
| :------------------- | :------ |
| un solo archivo indicado | `append needs a base file and at least one file to put after it` |
| ni `--output` ni `--dry-run` | `no destination given: append writes one file, use --output` |
| `--output` nombra una entrada | `<destination>: would be written over the input <input>` |
| un archivo ausente, ilegible o de formato desconocido | `<path>: does not exist`, `<path>: is in no format this tool knows`… — lo mismo que en todas partes |

**Un archivo vacío no se añade**: ningún formato lee sin al menos un subtítulo, y el archivo se
rechaza como cualquier archivo que no puede leerse —`<path>: holds nothing recognisable as a subtitle`—, lo que
detiene la ejecución y no escribe nada.
