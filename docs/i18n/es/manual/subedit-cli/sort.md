# `sort`

<!-- machine-translated -->
> Esta página es una traducción automática y todavía no la ha revisado ningún lector del idioma.

```
subedit-cli sort (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 <file>...
```

Vuelve a poner los subtítulos **en el orden de su inicio**. Nada más cambia: ni los textos, ni las
posiciones, ni el formato del archivo. La escritura **renumera** los subtítulos a partir de 1,
como en cualquier archivo que se vuelve a escribir.

**El núcleo nunca ordena por iniciativa propia**: leer un archivo deja su orden como estaba, e
[`inspect`](inspect.md#lo-que-indica-anomalies) señala una ruptura del orden con
`subtitle N starts before the previous one starts`. `sort` es **el gesto hecho a propósito**, para un archivo
o un lote cuyo orden está roto — y puede deshacerse, en la ventana, como cualquier operación.

- **La ordenación es estable**: dos subtítulos que empiezan en el mismo instante **conservan el orden que
  el archivo les daba**. Ninguno va antes que el otro; moverlos sería una decisión que nadie
  ha pedido.
- **`sort` no acepta ni intervalo ni documento**: no hay nada que elegir, el orden es el de los inicios.

<!-- example: subedit-cli sort --help -->
```console
$ subedit-cli sort --help
Put the subtitles in the order of their start, keeping ties as they are
Usage: subedit-cli sort [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to sort

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

<!-- example: printf '1\n00:00:05,000 --> 00:00:06,000\ncinq\n\n2\n00:00:01,000 --> 00:00:02,000\nun\n\n3\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > desordre.srt; subedit-cli sort --output ordre.srt desordre.srt; cat ordre.srt -->
```console
$ printf '1\n00:00:05,000 --> 00:00:06,000\ncinq\n\n2\n00:00:01,000 --> 00:00:02,000\nun\n\n3\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > desordre.srt; subedit-cli sort --output ordre.srt desordre.srt; cat ordre.srt
desordre.srt: 3 subtitles moved -> ordre.srt
1
00:00:01,000 --> 00:00:02,000
un

2
00:00:03,000 --> 00:00:04,000
trois

3
00:00:05,000 --> 00:00:06,000
cinq
```

## Lo que se cuenta

**El recuento es el de los lugares que han cambiado de subtítulo**, tal como lo indica el núcleo — lo que
el historial de la ventana llamaría movido. Un solo subtítulo mal colocado desplaza varios: en
el ejemplo anterior, ninguno de los tres lugares conserva su subtítulo, y la frase dice `3 subtitles moved`.

**Un archivo ya en orden se escribe igualmente**, y lo dice: un destino indicado es un
destino escrito, y hacer de `sort` la excepción obligaría a un script a saber qué
subcomandos producen unas veces un archivo y otras veces nada.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > ordre.srt; subedit-cli sort --output copie.srt ordre.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > ordre.srt; subedit-cli sort --output copie.srt ordre.srt
ordre.srt: already in order -> copie.srt
```

## Salida

**Salida estándar** — nada: el resultado es el archivo escrito. Con `--dry-run`, tampoco nada: no hay
archivo, y la línea de narración lo dice — véase [Ver antes de escribir](invocation.md#ver-antes-de-escribir).

| Nivel | Lo que se añade en la salida de error |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles moved -> <destination>`, o `<path>: already in order -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — el formato, la codificación, la marca y los finales de línea del archivo leído, restituidos tal como estaban |
| 3 | `<path>: N bytes read, M written`, y después **cada diagnóstico de lectura** — véase [Invocación](invocation.md#diagnósticos-de-lectura) |

### En JSON

Con [`--format json`](invocation.md#una-salida-que-un-script-puede-leer), el objeto de cada archivo lleva
`counts.subtitles` (todos los subtítulos del archivo) y `counts.moved` (los lugares que han cambiado;
`0` para un archivo ya en orden).

## Códigos de salida

Los de la herramienta: `0` si se han procesado todos los archivos — un archivo ya en orden lo está —, `2` si
ninguno, `3` si solo algunos, `1` ante un error de uso.

## Errores

Ninguno propio de `sort`: un archivo ilegible falla como en cualquier subcomando, y los
errores del destino son comunes a los subcomandos que escriben:
véase [Invocación](invocation.md#el-destino).
