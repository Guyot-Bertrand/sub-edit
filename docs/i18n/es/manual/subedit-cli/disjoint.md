# `disjoint`

<!-- machine-translated -->
> Esta página es una traducción automática y todavía no la ha revisado ningún lector del idioma.

```
subedit-cli disjoint (--output FILE | --output-dir DIR | --in-place)
                     [--minimum TIME] [--max-lines N] [--dashes] [--sort]
                     [--dry-run] [--recursive]
                     <file>...
```

Corta los subtítulos solapados para que **ningún intervalo `[start, end]` se solape
ya con otro**. Es `Make Ranges Disjoint…` de la ventana: [el mismo motor](../subedit-gui/lines.md#hacer-disjuntos-los-intervalos).

**No se acorta ni se alarga nada**: en cada instante, el archivo muestra los mismos textos que antes. Lo que
cambia es la forma de cortar la línea de tiempo:

- se corta en **cada inicio y cada fin** de los subtítulos de un grupo solapado;
- un intervalo cubierto por **un solo** subtítulo sigue siendo ese subtítulo;
- un intervalo cubierto por **varios** se convierte en **un** subtítulo cuyas líneas son las suyas, **en
  el orden de sus inicios**: el más temprano en la línea 1, el siguiente en la línea 2;
- un subtítulo que no se solapa con nada no se toca, y el orden del resto se conserva.

<!-- example: subedit-cli disjoint --help -->
```console
$ subedit-cli disjoint --help
Cut overlapping subtitles so that no range overlaps another, merging what overlaps
Usage: subedit-cli disjoint [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to make disjoint

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --minimum TIME              Shortest interval kept; a shorter one is taken over by a neighbour (default 0.08)
  --max-lines N               Most lines of a merged subtitle; more are shown in windows, 0 for no limit (default 2)
  --dashes                    Put a dash before the text of each merged subtitle
  --sort                      Put the subtitles in order of their start first; without it, a file out of order is refused
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
| `--max-lines` | no | un número entero: el máximo de líneas de un subtítulo fusionado; `0` elimina el límite | `2` |
| `--dashes` | no | un indicador: un guion delante del texto que cada subtítulo aporta a un subtítulo fusionado | desactivado |
| `--sort` | no | un indicador: pone **primero** los subtítulos en el orden de su inicio — véase [Hace falta un archivo en orden](#hace-falta-un-archivo-en-orden) | desactivado |
| `--recursive`, `-r` | no | un indicador | desactivado — véase [Procesar un árbol](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **una de las tres**, salvo con `--dry-run` | véase [Invocación](invocation.md#el-destino) | — |
| `--dry-run` | no | un indicador: calcula e informa, no escribe nada — véase [Ver antes de escribir](invocation.md#ver-antes-de-escribir) | desactivado |

<!-- example: printf '1\n00:00:00,000 --> 00:00:05,000\nA\n\n2\n00:00:03,000 --> 00:00:08,000\nB\n\n' > chevauche.srt; subedit-cli disjoint --output disjoint.srt chevauche.srt; cat disjoint.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:05,000\nA\n\n2\n00:00:03,000 --> 00:00:08,000\nB\n\n' > chevauche.srt; subedit-cli disjoint --output disjoint.srt chevauche.srt; cat disjoint.srt
chevauche.srt: 2 subtitles split, 1 merged -> disjoint.srt
1
00:00:00,000 --> 00:00:03,000
A

2
00:00:03,000 --> 00:00:05,000
A
B

3
00:00:05,000 --> 00:00:08,000
B
```

`A [0, 5]` y `B [3, 8]` dan tres subtítulos disjuntos: `A` solo, `A` y `B` juntos, `B` solo. No
se pierde ni se añade ningún instante.

## Lo que decide la operación

**Una línea nunca se duplica.** Dos subtítulos con el mismo texto dan una sola línea donde se
solapan, y dos trozos vecinos que muestran lo mismo forman un solo subtítulo. Por lo demás, el texto de un
subtítulo cortado en varios trozos se **repite en cada uno**: es lo que hay en pantalla en ese
momento.

**Un intervalo muy corto lo absorbe un vecino.** Por debajo de `--minimum`, un intervalo desaparece: el vecino
**extiende su intervalo para cubrirlo**, de modo que ningún instante queda vacío y nada se mueve más que ese
umbral. El vecino elegido es aquel cuyas líneas **ya contienen** las del intervalo; si no, el más
largo. El texto del intervalo solo se pierde si ningún vecino lo lleva, y entonces durante menos que el umbral.
**Un subtítulo entero más corto que el umbral nunca se absorbe**: se deja, y se cuenta.

**Más líneas que el límite: ventanas.** Un subtítulo fusionado que tendría más de `--max-lines` líneas
se muestra **en ventanas sucesivas** de ese tamaño, en partes iguales de la duración. Tres líneas en tres
segundos:

<!-- example: printf '1\n00:00:00,000 --> 00:00:03,000\nA\n\n2\n00:00:00,000 --> 00:00:03,000\nB\n\n3\n00:00:00,000 --> 00:00:03,000\nC\n\n' > trois.srt; subedit-cli disjoint --output fenetres.srt trois.srt; cat fenetres.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:03,000\nA\n\n2\n00:00:00,000 --> 00:00:03,000\nB\n\n3\n00:00:00,000 --> 00:00:03,000\nC\n\n' > trois.srt; subedit-cli disjoint --output fenetres.srt trois.srt; cat fenetres.srt
trois.srt: 0 subtitles split, 1 merged; 1 shown in windows for holding too many lines -> fenetres.srt
1
00:00:00,000 --> 00:00:01,500
A
B

2
00:00:01,500 --> 00:00:03,000
B
C
```

Cuatro líneas dan tres ventanas, y así sucesivamente: la ventana se desliza de línea en línea. Nada se
trunca ni se vuelve a cortar.

**La traducción sigue los mismos cortes**, ya que un subtítulo lleva los dos textos. **Lo que el formato lleva
además** —un estilo, una posición— es lo del primer subtítulo que cubre; la pérdida se cuenta cuando
los subtítulos fusionados difieren.

**Un subtítulo cuyo fin llega antes que su inicio** no se adivina: se deja donde está, y se cuenta.

## Hace falta un archivo en orden

Los grupos son series de vecinos: fuera de orden, los vecinos no son los que se solapan. Un archivo
cuyos inicios no están en orden se **rechaza**, por tanto, y el mensaje lo dice:

<!-- example: printf '1\n00:00:03,000 --> 00:00:08,000\nB\n\n2\n00:00:00,000 --> 00:00:05,000\nA\n\n' > desordre.srt; subedit-cli disjoint --output x.srt desordre.srt; echo "code $?" -->
```console
$ printf '1\n00:00:03,000 --> 00:00:08,000\nB\n\n2\n00:00:00,000 --> 00:00:05,000\nA\n\n' > desordre.srt; subedit-cli disjoint --output x.srt desordre.srt; echo "code $?"
desordre.srt: The subtitles are not in the order of their start: sort them first. Use --sort to do that here.
code 2
```

`--sort` ordena primero —igual que [`sort`](sort.md)—, y lo dice. El núcleo nunca ordena por su cuenta.

Una segunda pasada no cambia nada: una vez disjuntos los intervalos, no queda nada que cortar, y la
frase dice `already disjoint`.

## Salida

**Salida estándar**: nada; el resultado es el archivo escrito.

| Nivel | Lo que se añade en la salida de error |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles split, M merged -> <destination>`, seguido, tras un `;`, de lo que no pudo hacerse por completo: `K shown in windows for holding too many lines`, `K very short taken over by a neighbour`, `K subtitles shorter than the limit left whole`, `K merged with styles or positions that differ, the first kept`, `K subtitles ending before the start left alone` — o `<path>: already disjoint -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, y después **cada diagnóstico de lectura** |

`N subtitles split` cuenta los subtítulos originales cortados en varios trozos, `M merged` los trozos que
muestran varios subtítulos juntos.

### En JSON

Con [`--format json`](invocation.md#una-salida-que-un-script-puede-leer), los `counts` de cada archivo:

| Recuento | Lo que indica |
| :----- | :----------- |
| `subtitles` | los subtítulos del archivo escrito |
| `split`, `merged` | como en la frase |
| `windowed` | los trozos mostrados en ventanas |
| `absorbed` | los intervalos absorbidos por un vecino |
| `too_short` | los subtítulos más cortos que el umbral, dejados enteros |
| `ending_before_start` | los subtítulos cuyo fin llega antes que su inicio, dejados tal cual |
| `extras_dropped` | los trozos fusionados cuyo estilo o posición difieren |
| `moved` | solo con `--sort`: los lugares que cambiaron |

## Códigos de salida

Los de la herramienta: `0` si se procesaron todos los archivos —y un archivo sin solapamientos lo está—, `2` si
ninguno, `3` si solo algunos, `1` en caso de error de uso.

## Errores

| Qué los provoca | Mensaje |
| :------------------- | :------ |
| un archivo fuera de orden sin `--sort` | `<path>: The subtitles are not in the order of their start: sort them first. Use --sort to do that here.` |
| `--minimum` no es una duración, o es negativa | `--minimum: …` |
| `--max-lines` no es un número entero | `--max-lines: "…" is not a number of lines: expected a whole number, 0 for no limit` |

Un archivo ilegible falla como en cualquier subcomando; los errores del destino son comunes a los
subcomandos que escriben: véase [Invocación](invocation.md#el-destino).

## En los demás comandos

[`convert`](convert.md#hacer-disjuntos-los-intervalos), [`append`](append.md#hacer-disjuntos-los-intervalos) y
[`split-file`](split-file.md#hacer-disjuntos-los-intervalos) aceptan `--disjoint`, que hace lo mismo con
la configuración por defecto, antes de escribir.

En un archivo mal segmentado —un texto cortado en trozos que se suceden, dados fuera de orden—, ordenar y cortar
sin más da líneas que saltan de un trozo al siguiente: primero [`join-repeats`](join-repeats.md), o
[`repair`](repair.md), que encadena los tres.
