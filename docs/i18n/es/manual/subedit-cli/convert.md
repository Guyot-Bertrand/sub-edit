# `convert`

<!-- machine-translated -->
> Esta página es una traducción automática y todavía no la ha revisado ningún lector del idioma.

```
subedit-cli convert --to <format>
                    [--line-endings unix|windows|mac] [--to-encoding NAME]
                    [--bom | --no-bom] [--sort] [--disjoint]
                    [--join-repeats] [--repair]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    <file>...
```

Escribe cada archivo en el formato y la forma pedidos. **Nunca cambia
la entrada**, salvo que `--in-place` lo pida explícitamente.

<!-- example: subedit-cli convert --help -->
```console
$ subedit-cli convert --help
Write a subtitle file out in another format or shape
Usage: subedit-cli convert [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to convert

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --to TEXT:{srt,vtt,subviewer2,ssa,ass,microdvd,mpl2,tmplayer,lrc} REQUIRED
                              Format to write
  --line-endings TEXT:{unix,windows,mac}
                              Line endings to write; the source's by default
  --to-encoding NAME          Encoding to write; the source's by default
  --frame-rate RATE           Frame rate of a file counted in frames: 25, 23.976
  --bom                       Write a byte order mark
  --no-bom                    Write no byte order mark
  --sort                      Put the subtitles in order of their start before writing
  --disjoint                  Cut overlapping subtitles so that no range overlaps another, before writing
  --join-repeats              Join the subtitles that repeat each other into one, before writing
  --repair                    Join the repeats, sort, then make the ranges disjoint, before writing
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Argumentos y opciones

| Opción | Obligatoria | Valores | Por defecto |
| :----- | :----- | :------ | :----- |
| `<file>...` | sí | una o varias rutas | — |
| `--recursive`, `-r` | no | un indicador | desactivado: véase [Procesar un árbol](lots.md) |
| `--to` | **sí** | un nombre de la tabla siguiente, y nada más | — |
| `--line-endings` | no | `unix`, `windows` o `mac` | los del archivo leído |
| `--to-encoding` | no | cualquier codificación que ICU sepa escribir, salvo las que escriben su propia marca | la del archivo leído |
| `--bom` / `--no-bom` | no | indicadores, excluyentes entre sí | lo que llevaba el archivo leído |
| `--sort` | no | un indicador: pone los subtítulos en el orden de su inicio **antes de escribir**, véase [Ordenar al escribir](#ordenar-al-escribir) | desactivado |
| `--join-repeats` | no | un indicador: hace un solo subtítulo de los trozos de un mismo texto que se suceden, **antes que nada**, véase [Hacer disjuntos los intervalos](#hacer-disjuntos-los-intervalos) | desactivado |
| `--repair` | no | un indicador: lo mismo que `--join-repeats --sort --disjoint`, en ese orden | desactivado |
| `--disjoint` | no | un indicador: corta los subtítulos solapados **antes de escribir**, véase [Hacer disjuntos los intervalos](#hacer-disjuntos-los-intervalos) | desactivado |
| `--output` / `--output-dir` / `--in-place` | **una de las tres**, salvo con `--dry-run` | véase [Invocación](invocation.md#el-destino) | — |
| `--dry-run` | no | un indicador: calcula y dice, no escribe nada; véase [Ver antes de escribir](invocation.md#ver-antes-de-escribir) | desactivado |

`mac` designa el retorno de carro solo (`\r`), el final de línea del Mac OS clásico.

## Los formatos que acepta `--to`

| Nombre | Formato | Extensión escrita |
| :-- | :----- | :--------------- |
| `srt` | SubRip | `.srt` |
| `vtt` | WebVTT | `.vtt` |
| `subviewer2` | SubViewer 2.0 | `.sub` |
| `ssa` | Sub Station Alpha | `.ssa` |
| `ass` | Advanced SSA | `.ass` |
| `mpl2` | MPL2 | `.txt` |
| `microdvd` | MicroDVD | `.sub` |
| `tmplayer` | TMPlayer | `.txt` |
| `lrc` | LRC | `.lrc` |

**Un nombre, y no una extensión.** Dos extensiones designan dos formatos
cada una (`.sub` es también la de MicroDVD, `.txt` la de MPL2 y la de
TMPlayer), así que una opción que tomara extensiones no sabría cuál
se pide. Los nombres anteriores no son ambiguos, y coinciden con la
extensión allí donde la extensión tampoco lo es.

**La lista es cerrada.** Es la de Gaupol, y los nueve están en ella.

## Los dos formatos que no llevan fin

**TMPlayer y LRC solo escriben una posición por línea**, su inicio. Esto
se nota en los dos sentidos.

**En la lectura**, los finales se deducen: el de una línea es el inicio de
la siguiente, y la última recibe cinco segundos. Es lo que hace Gaupol.
Además lo decimos, mediante un diagnóstico que `--verbose` muestra en los tres
subcomandos:

```
file.lrc: carries no end times; each one was taken from the next start, settled by the reader
```

**En la escritura**, los finales no se escriben, por falta de un sitio donde ponerlos.
Convertir a uno de estos dos formatos los pierde, por tanto, y una ida y vuelta no
los recupera: los vuelve a deducir.

**LRC pierde una cosa más**: no tiene forma de llevar un salto de línea.
Una réplica en dos líneas se escribe en una, con las dos líneas unidas por
un espacio. Esa pérdida tampoco se puede reparar.

Ninguno de los dos tiene vocabulario para la cursiva.

## `--frame-rate`, y el único formato que cuenta en fotogramas

**MicroDVD no cuenta en segundos**: sus posiciones *son* números de
fotograma, y ningún archivo MicroDVD declara la velocidad a la que se
contaron. `--frame-rate` la da, y sirve en los dos sentidos: es la
velocidad a la que se lee un `.sub` en fotogramas, y aquella a la que una
conversión a MicroDVD cuenta los fotogramas.

| Opción | Obligatoria | Valores | Por defecto |
| :----- | :----- | :------ | :----- |
| `--frame-rate` | no | una velocidad en fotogramas por segundo: `25`, `23.976`, `29.97` | véase más abajo |

**En la lectura, sin `--frame-rate`, el archivo se lee a 23,976** (el valor
de Gaupol), y el nivel `-vvv` lo dice:
`counts in frames and states no rate; it was read at`. Es el único lugar de la herramienta en el que todas las posiciones mostradas
descansan sobre una suposición que el archivo no puede confirmar.

**En la escritura, sin `--frame-rate`, tres casos y tres respuestas:**

| El archivo de partida | Lo que ocurre |
| :------------------- | :-------------- |
| ya es MicroDVD | la velocidad que lo leyó lo vuelve a escribir, y los fotogramas vuelven idénticos |
| está basado en tiempos, y sus posiciones caen sobre una rejilla | se toma la rejilla, y `-vv` dice cuál |
| está basado en tiempos, sin rejilla | **rechazo**, código 2: `writing frames needs a frame rate, and the positions fall on no grid to take one from — give --frame-rate` |

El rechazo es deliberado. El único gesto que quedaría sería inventar una cifra, y
cada línea del archivo se movería tanto como esa cifra estuviera equivocada.

## Lo que pierde una conversión

**Convertir a otro formato nunca es sin pérdidas**, y lo que no sobrevive
depende del par:

| Lo que no pasa | Por qué |
| :--------------------- | :------- |
| la cabecera | solo tiene sentido en su propio formato: `[INFORMATION]` no es una cabecera WebVTT |
| los datos propios del formato | las coordenadas de SubRip, el identificador y los ajustes de una indicación WebVTT no tienen equivalente en otro sitio |
| la precisión | SubViewer 2, los dos Sub Station Alpha y LRC escriben a la centésima, MPL2 a la décima, TMPlayer al segundo, MicroDVD al fotograma; `01:00:00,017` vuelve como `01:00:00,020` |
| el fin de una línea | TMPlayer y LRC no tienen campo para él; la lectura siguiente lo vuelve a deducir |
| los saltos de línea | solo LRC: sus réplicas caben en una línea, y las dos se vuelven a pegar con un espacio |
| las etiquetas que el formato de destino no sabe escribir | WebVTT no tiene `<font color>`, TMPlayer y LRC no tienen nada en absoluto |
| la disposición | `{\pos(x,y)}`, `{\an8}`, `<v Marie>`, `<ruby>`: sobreviven a una ida y vuelta en su propio formato, y desaparecen en la conversión |

**Volver a escribir un archivo en su propio formato no pierde nada**: es la garantía
que sostiene todo lo demás, y `--to srt` sobre un `.srt` devuelve los mismos bytes. Ni
siquiera se decodifica nada: el texto sigue siendo la cadena en bruto que se leyó.

## Las etiquetas se traducen, no se copian

**Cada formato escribe la cursiva a su manera**, y convertir sin traducir
dejaría un `{\i1}` en un `.srt`: texto que el usuario ve, en un
archivo que nunca sabrá interpretarlo. Lo que pasa son **seis
cosas**: negrita, cursiva, subrayado, color, fuente y tamaño:

| Formato | Negrita | Cursiva | Subrayado | Color | Fuente | Tamaño |
| :----- | :--: | :------: | :------: | :-----: | :----: | :----: |
| SubRip | `<b>` | `<i>` | `<u>` | `<font color="#RRGGBB">` | — | — |
| WebVTT | `<b>` | `<i>` | `<u>` | — | — | — |
| SubViewer 2 | `<b>` | `<i>` | `<u>` | `<font color="#RRGGBB">` | — | — |
| Sub Station Alpha | `{\b1}` | `{\i1}` | — | `{\c&HBBGGRR&}` | `{\fnNAME}` | `{\fsN}` |
| Advanced SSA | `{\b1}` | `{\i1}` | `{\u1}` | `{\c&HBBGGRR&}` | `{\fnNAME}` | `{\fsN}` |
| MicroDVD | `{Y:b}` | `{Y:i}` | `{Y:u}` | `{C:$BBGGRR}` | `{F:NAME}` | `{S:N}` |
| MPL2 | `\` | `/` | `_` | `{C:$BBGGRR}` | `{F:NAME}` | `{S:N}` |
| TMPlayer | — | — | — | — | — | — |
| LRC | — | — | — | — | — | — |

**Un guion es una pérdida anunciada.** Convertir un `.srt` en color a WebVTT elimina
el color y lo dice; el archivo producido es WebVTT, no SubRip disfrazado.

**MicroDVD y MPL2 no pueden dar estilo a una parte de línea.** Sus etiquetas
no tienen fin: `{y:i}` inclina todo lo que sigue hasta el final de la línea, y
`{Y:i}` hasta el final del subtítulo. Una línea de la que solo la mitad estaba
en negrita se escribe, por tanto, sin ninguna negrita, en lugar de con una negrita que se desborda.

## Lo que la conversión dice haber perdido

**Lo dice cuando pierde, y se calla cuando no pierde nada.** Un informe
que aparece en cada llamada es un informe que nadie lee.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,500\n<i>sur deux</i>\nlignes\n\n' > a.srt; subedit-cli convert --to lrc --output b.lrc a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,500\n<i>sur deux</i>\nlignes\n\n' > a.srt; subedit-cli convert --to lrc --output b.lrc a.srt
a.srt: 1 subtitle written as LRC -> b.lrc
a.srt: ends are not carried by LRC, line breaks were joined in 1 subtitle, 1 tag dropped
```

La línea aparece en el nivel de detalle por defecto, y en todos los superiores.
Los elementos, en este orden:

| Elemento | Lo que dice |
| :---- | :----------- |
| `ends are not carried by <format>` | el formato de destino no escribe ningún fin |
| `line breaks were joined in N subtitles` | solo LRC: N subtítulos estaban en varias líneas |
| `N tags dropped` | N etiquetas no pudieron escribirse, disposición incluida |
| `the header was dropped` | la cabecera no cruza la frontera de un formato |
| `the <format> fields of N subtitles were dropped` | N subtítulos llevaban datos propios de su formato |
| `positions moved by up to N ms` | la mayor distancia que se movió una posición |

## Cambiar la codificación

**`--to-encoding` elige la codificación escrita; `--encoding` dice la que se
lee.** Las dos juntas recodifican un archivo:

```console
$ subedit-cli convert --to srt --encoding cp1252 --to-encoding utf-8 \
      --output-dir sortie film.srt
```

Sin `--to-encoding`, el archivo se escribe en **la codificación en la que se leyó**,
como ocurre con sus finales de línea y su marca: un archivo convertido sin
instrucción sobre la codificación devuelve los mismos caracteres, en los mismos bytes.

**Un carácter que la codificación de destino no sabe escribir detiene el archivo**, y
no se escribe nada. Una `ł` no tiene sitio en Latin-1; sustituirla por un `?`
perdería texto entre la lectura y la escritura, sin que nadie lo viera.

**Una marca de orden de bytes solo existe para las codificaciones Unicode.**
`--bom` con una `--to-encoding` que no la lleva se rechaza en lugar de ignorarse;
véase la tabla de errores más abajo.

**Y la codificación tiene que nombrar su orden de bytes.** `UTF-16` y `UTF-32` son
nombres que ICU conoce, cuyo convertidor escribe **una marca propia**, se le pida
lo que se le pida: `--no-bom` se aceptaría entonces y se desobedecería. Por tanto
se rechazan los dos, y el mensaje nombra la salida:

```console
$ subedit-cli convert --to srt --to-encoding UTF-16 --output copie.srt film.srt
"UTF-16" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do
```

`UTF-16LE`, `UTF-16BE`, `UTF-32LE` y `UTF-32BE` escriben exactamente los mismos
bytes, marca incluida, pero bajo el control de `--bom` y `--no-bom`. El
rechazo no cuesta, por tanto, más que un nombre más preciso, y vale tanto para
la lectura como para la escritura: `--encoding UTF-16` recibe la misma respuesta.

## El destino

Las tres opciones y sus reglas son comunes a los subcomandos que
escriben: véase [Invocación](invocation.md#el-destino). No se escribe nada
sin una de ellas.

Lo propio de `convert`: **`--output-dir` cambia la extensión con el
formato.** `convert --to vtt` sobre `a.srt` escribe `a.vtt`, nunca un `a.srt` que contenga
WebVTT: un archivo cuyo nombre miente hace tropezar a todas las demás herramientas.

## Convertir sobre la propia entrada se rechaza

`--in-place` con un `--to` que cambia el formato da el código `1` y no escribe
nada: sobre la propia entrada no hay un segundo nombre que lleve el nuevo formato, y
el archivo se quedaría con una extensión que su contenido ya no justifica.

`--in-place` sigue pudiendo usarse para cambiar **la forma sin el formato**:
volver a escribir un `.srt` como un `.srt` con otros finales de línea, por ejemplo.

El rechazo se decide **solo por la extensión**, antes de cualquier lectura: un error de uso
nunca debe dejar un lote escrito a medias.

## Finales de línea y marca de orden de bytes

Por defecto, **el archivo escrito retoma lo que llevaba el archivo leído**. El modelo
conserva los dos en la lectura; imponerlos por defecto perdería, en cada
conversión, una información conservada a propósito.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli convert --to vtt --output b.vtt a.srt; cat b.vtt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli convert --to vtt --output b.vtt a.srt; cat b.vtt
a.srt: 1 subtitle written as WebVTT -> b.vtt
WEBVTT

00:01.000 --> 00:03.000
Bonjour.
```

## Ordenar al escribir

`--sort` escribe el archivo **en el orden de los inicios**, sea cual sea el formato de destino. La ordenación se hace **sobre una copia**: el archivo leído
no se toca, y lo que se escribe es lo que [`sort`](sort.md) habría hecho con él: estable, dos subtítulos que empiezan a la vez conservan su orden.

**Nunca por defecto**: el orden de un archivo es el de su autor mientras no se pida otra cosa. Con la opción, cada archivo lo dice, en el
primer nivel de narración, `<path>: N subtitles moved` o `<path>: already in order`, y el registro de `--format json` lleva un
recuento `moved`, **ausente sin la opción**, para no cambiar la forma de lo que leen los scripts existentes.

## Hacer disjuntos los intervalos

`--disjoint` corta los subtítulos solapados para que ningún intervalo se solape ya con otro, **sobre la misma copia que la ordenación**: el archivo leído no se toca.

Nunca por defecto. La opción hace lo que hace [`disjoint`](disjoint.md), **con su configuración por defecto**, antes de escribir, después de la ordenación. Necesita un archivo en orden: si no, se rechaza, y `--sort` (que va primero) lo pone en orden antes. El archivo lo dice en el primer nivel de narración (`<path>: N subtitles split, M merged`), y el registro de `--format json` lleva los recuentos `split` y `merged`, **ausentes sin la opción**.

**`--join-repeats` va antes de la ordenación y del corte**: un texto cortado en trozos que se suceden se reconstruye primero (véase
[`join-repeats`](join-repeats.md)), después se ordenan los subtítulos y después se hacen disjuntos los intervalos. **`--repair` son las tres a la vez**,
como [`repair`](repair.md), con la configuración por defecto del corte. Con `--join-repeats`, cada archivo dice
`<path>: N repeated subtitles joined` en el primer nivel de narración, y el registro de `--format json` lleva `counts.joined`, **ausente sin la opción**.

## Salida

**Salida estándar**: nada, el resultado es el archivo escrito. Con `--dry-run`, tampoco nada: no hay archivo, y la línea de narración lo dice; véase [Ver antes de escribir](invocation.md#ver-antes-de-escribir).

En la salida de error, según el nivel:

| Nivel | Lo que se añade |
| :----- | :-------------- |
| 1 | `<path>: N subtitles written as WebVTT -> <destination>` |
| 2 | `<path>: SubRip -> WebVTT, UTF-8, no BOM, LF line endings`; la codificación nombrada es la escrita |
| 3 | `<path>: N bytes read, M written`, y después **cada diagnóstico de lectura**; véase [Invocación](invocation.md#diagnósticos-de-lectura) |

### En JSON

Con [`--format json`](invocation.md#una-salida-que-un-script-puede-leer), el objeto de cada archivo lleva `counts.subtitles` y después lo que perdió la conversión: `lost_ends` y `lost_header` (`0` o `1`), `joined_lines`, `lost_tags`, `lost_fields` y `furthest_ms`, los elementos de [la frase de pérdida](#lo-que-la-conversión-dice-haber-perdido). La tabla completa es la de [Invocación](invocation.md#lo-que-cada-subcomando-pone-en-counts).

## Códigos de salida

Los de la herramienta: `0` si se escribieron todos los archivos, `2` si ninguno, `3` si
solo algunos, `1` en un error de uso.

## Errores

| Qué lo provoca | Mensaje |
| :------------------ | :------ |
| `--bom` con `--no-bom` | `--bom and --no-bom ask for opposite things; give one or the other` |
| `--in-place` que cambia el formato | `--in-place cannot change the format: the file would keep a name its content no longer matches` |
| `--bom` sobre una codificación que no la tiene | `<path>: <encoding> has no byte order mark to write` |
| `--to-encoding` que nombra una codificación desconocida | `no encoding is named "<name>"` |
| `--to-encoding` que nombra una codificación que escribe su propia marca | `"<name>" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do` |
| carácter ausente de la codificación escrita | `<path>: holds a character the chosen encoding cannot write` |

**Rechazar `--bom` sobre una codificación que no la lleva no es
pedantería.** Una marca de orden de bytes existe para las codificaciones Unicode y
para ninguna otra: pedirla en un archivo en Windows-1252 es pedir
algo que no existe. El archivo se escribiría sin ella, y nadie sabría
que la petición no se atendió.

Los del destino son comunes a los subcomandos que escriben:
véase [Invocación](invocation.md#el-destino).

Los errores de lectura son los de [`inspect`](inspect.md): afectan
al archivo de entrada y tienen los mismos mensajes.
