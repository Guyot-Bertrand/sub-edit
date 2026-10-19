# `inspect`

<!-- machine-translated -->
> Esta página es una traducción automática y todavía no la ha revisado ningún lector del idioma.

```
subedit-cli inspect [--frame-rate RATE] [--recursive] [--detail]
                   [-t FILE] [--align-method position|number] <file>...
```

Informa de qué está hecho cada archivo. **No cambia nada y no escribe ningún
archivo.**

<!-- example: subedit-cli inspect --help -->
```console
$ subedit-cli inspect --help
Report what a subtitle file is made of
Usage: subedit-cli inspect [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to report on

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --detail                    List every anomaly, subtitle by subtitle, instead of counting them
  --frame-rate RATE           Frame rate of a file counted in frames: 25, 23.976
  -t,--translation-file FILE  Translation file to lay over the subtitle file, for a single input
  --align-method position|number
                              How the lines of the translation find their subtitles
```

## Argumentos

| Argumento | Obligatorio | Valor | Por defecto |
| :------- | :----- | :----- | :----- |
| `<file>...` | sí | una o varias rutas de archivos de subtítulos | — |
| `--recursive`, `-r` | no | un indicador | desactivado: véase [Procesar un árbol](lots.md) |
| `--detail` | no | un indicador: nombra cada anomalía, en lugar de contarlas; véase [Lo que indica `anomalies`](#lo-que-indica-anomalies) | desactivado |

No dar ninguna ruta es un error de uso, y por tanto código `1`.

## Opciones

| Opción | Obligatoria | Valor | Por defecto |
| :----- | :------ | :----- | :----- |
| `--frame-rate` | no | una velocidad en fotogramas por segundo: `25`, `23.976`, `29.97` | 23,976 |

**Solo sirve para los archivos contados en fotogramas**, es decir MicroDVD, el
único de los nueve. Un archivo MicroDVD no lleva más que números de fotograma; la
velocidad a la que se leen decide **todas las posiciones del informe**,
empezando por `span`. Dada en otro caso, no cambia nada y no es un
error.

Un valor que no nombra nada es un error de uso, y por tanto código `1`:

<!-- example: subedit-cli inspect --frame-rate douze film.sub; echo $? -->
```console
$ subedit-cli inspect --frame-rate douze film.sub; echo $?
--frame-rate: "douze" is not a frame rate: expected frames per second, like 25 or 23.976
1
```

Es la misma opción que en [`convert`](convert.md), palabra por palabra, porque
es la misma velocidad. `convert` la usa además para **escribir**.

## Una traducción: `-t`

| Opción | Obligatoria | Valor | Por defecto |
| :----- | :------ | :----- | :----- |
| `-t`, `--translation-file` | no | un archivo de traducción, para **una sola** entrada | — |
| `--align-method` | no, y solo con `-t` | `position` o `number` | `position` |

Con `-t`, la traducción se coloca sobre el archivo **como hace la ventana al abrirla** (véase
[Una traducción](invocation.md#una-traducción)) y el informe dice **cómo encontraron sus líneas
su subtítulo**. Así, un script sabe si una traducción está alineada antes de corregirla.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour.\n\n' > film.fr.srt; subedit-cli --quiet inspect -t film.fr.srt film.srt | tail -3 -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour.\n\n' > film.fr.srt; subedit-cli --quiet inspect -t film.fr.srt film.srt | tail -3
  anomalies: none
  translation file: film.fr.srt, matched by position
  translation: 1 line attached; 1 subtitle left without a translation
```

La primera línea añadida nombra el archivo y el método; la segunda es **la frase que dice la
ventana** cuando abre una traducción, cada parte solo cuando no es cero:

| Parte | Lo que cuenta |
| :----- | :---------------- |
| `N lines attached` | las líneas de la traducción que encontraron un subtítulo del archivo principal |
| `N subtitles born of a line` | las líneas que no encontraron ninguno y dieron origen al suyo propio, en sus propias posiciones |
| `N subtitles left without a translation` | los subtítulos del archivo principal a los que no llegó ninguna línea |
| `N lines out of order` | las líneas que venían después de otra posterior en el archivo |

Una traducción que no lleva ninguna línea dice `translation: the file holds no line`. **El informe del
archivo en sí sigue siendo el del archivo solo**: un subtítulo nacido de una línea no se cuenta en
`subtitles`.

Los errores de `-t` son los de una lectura, dichos con la ruta de la traducción, y este:
`<translation>: the file is already open as the main document`, cuando el archivo dado es el principal.
`--align-method` sin `-t` es un error de uso, y también lo es `-t` con varias entradas; véase
[Una traducción](invocation.md#una-traducción).

## Salida

El informe va a la **salida estándar**: es el resultado del comando, y
se escribe en todos los niveles, incluido `--quiet`. Un bloque por archivo, precedido
de la ruta:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,500\nLe canot dérive.\n\n2\n00:00:04,000 --> 00:00:06,200\nPersonne n a vu la côte.\n' > exemple.srt; subedit-cli --quiet inspect exemple.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,500\nLe canot dérive.\n\n2\n00:00:04,000 --> 00:00:06,200\nPersonne n a vu la côte.\n' > exemple.srt; subedit-cli --quiet inspect exemple.srt
exemple.srt
  format: SubRip
  encoding: UTF-8, detected
  byte order mark: absent
  line endings: LF
  subtitles: 2
  span: 00:00:01.000 -> 00:00:06.200
  frame rate grid: none (too few subtitles to tell)
  anomalies: none
```

| Campo | Lo que dice |
| :---- | :----------- |
| `format` | uno de los formatos que acepta `--to`, escrito completo: `SubRip`, `WebVTT`, `SubViewer 2`, `Sub Station Alpha`, `Advanced SSA`, `MicroDVD`, `MPL2`, `TMPlayer`, `LRC` |
| `encoding` | la codificación leída, seguida de **la procedencia de la respuesta**: `, from its byte order mark` si el archivo la declara, `, as asked for` si `--encoding` la impuso, `, detected` si se sopesó |
| `byte order mark` | `present` o `absent` |
| `line endings` | `LF`, `CRLF` o `CR`, seguido de `, mixed from line N` si el archivo los mezcla |
| `subtitles` | el número de subtítulos leídos |
| `span` | del inicio más temprano al fin más tardío, `HH:MM:SS.mmm` |
| `frame rate grid` | la velocidad de fotogramas sobre la que se calcularon las posiciones, **deducida**; véase más abajo |
| `frame rate` | para un archivo contado en fotogramas, la velocidad a la que se leyó y **de dónde procede**; sustituye a la línea anterior, véase más abajo |
| `anomalies` | `none` en la misma línea, o lo que está mal: **un tipo por línea con su recuento**, sangrado bajo `anomalies:`; con `--detail`, **una anomalía por línea**, subtítulo a subtítulo |

**`span` no es «del primero al último»**, sino del más temprano al más tardío:
en un archivo cuyo orden está roto, los dos difieren, y solo el segundo dice
la verdad sobre lo que abarca el archivo.

**`anomalies`, con `--detail`, cuenta los subtítulos desde 1**, como los numera
el archivo, y no las líneas del archivo. Un número de subtítulo sobrevive a un
cambio; un número de línea no. Véase más abajo.

## Lo que indica `frame rate grid`

**Un archivo de subtítulos no declara su velocidad de fotogramas.** SubRip no tiene
cabecera, la de WebVTT es texto libre. Esta línea, por tanto, no lee nada:
**deduce**, midiendo si las posiciones caen sobre una rejilla de fotogramas.

Lo que se mide son los **inicios**. Allí donde existe una rejilla están sobre ella
sin excepción, mientras que los finales a menudo se apartan de ella: un fin se
calcula con frecuencia mediante una regla de velocidad de lectura en lugar de ponerse sobre
un fotograma.

La primera línea dice siempre lo mismo: la velocidad elegida, el veredicto
y la concentración que lo respalda.

| Veredicto | Qué significa |
| :------ | :----------------- |
| `clean` | las posiciones están sobre una rejilla |
| `partial` | una parte lo está, el resto no |
| `none` | ningún candidato explica nada |

`none` tiene **dos causas, y no son la misma respuesta**: o ninguna
rejilla encaja (la línea da entonces el mejor candidato y su puntuación,
para que se vea por cuánto falla) o el archivo cuenta demasiado pocos
subtítulos para poder decir nada de él. Dos inicios parecen siempre
perfectamente alineados; eso no prueba nada.

**Un archivo regular sobre una velocidad no estándar da `none`, y es
intencionado.** El conjunto de candidatos es cerrado (las ocho velocidades que
ofrece la conversión), e indicar la menos errónea de ellas sería dar
una respuesta errónea donde «no lo sé» es la correcta.

### Las líneas que siguen, cuando tienen algo que decir

| Línea | Cuándo aparece |
| :---- | :------------------ |
| `grid offset` | las posiciones están sobre la rejilla **salvo una constante**: el archivo se desplazó, y aquí está cuánto |
| `also fits` | otro candidato encaja igual de bien, porque es un múltiplo entero del elegido |
| `too short a span to separate` | el lapso del archivo no permite distinguir la velocidad elegida de las nombradas |
| `off the grid` | cuántos inicios se apartan de la rejilla, y en cuántas **series** |

**`also fits` no es una vacilación.** Una rejilla a 25 fotogramas por segundo está
*incluida* en una rejilla a 50: un archivo temporizado a 25 encaja, por tanto, en las dos.
Lo contrario es falso (un verdadero 50 se desmorona sobre 25), así que se elige la
más baja, y la otra se nombra en lugar de callarla. Los únicos
pares afectados son 25 y 50, 29,97 y 59,94, 30 y 60.

**`too short a span to separate` protege contra una respuesta demasiado segura.** 23,976
y 24 se separan un milisegundo por segundo de película: en un largometraje
se distinguen sin duda, en diez segundos no se pueden distinguir. La línea dice
cuál sigue siendo posible.

**`off the grid` distingue dos historias que el porcentaje solo confunde.**
Muchas series de un solo inicio son posiciones corregidas a mano,
una a una. Unas pocas series largas son una sección retemporizada o un archivo
montado a partir de otros dos. El número de series es lo que las separa.

### Un ejemplo

Doce líneas puestas sobre una rejilla a 25 fotogramas por segundo (es decir, en
posiciones múltiplos de 40 milisegundos), repartidas en casi nueve minutos:

<!-- example: k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > grille.srt; subedit-cli --quiet inspect grille.srt | tail -3 -->
```console
$ k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > grille.srt; subedit-cli --quiet inspect grille.srt | tail -3
  frame rate grid: 25 fps, clean (100.0%)
  also fits: 50 fps, of which this rate is a whole divisor
  anomalies: none
```

El ejemplo fabrica su propio archivo en lugar de leer uno del repositorio, y es
deliberado: puede reproducirse tal cual, y el bucle muestra lo que es una rejilla
mejor que una frase: posiciones múltiplos de la duración de un fotograma.

## Lo que indica `frame rate`, y por qué no es una rejilla

**Solo uno de los nueve formatos cuenta en fotogramas**: MicroDVD, cada una de cuyas líneas
lleva dos números de fotograma y nada más. En un archivo así, `inspect`
no escribe `frame rate grid` sino `frame rate`, y las dos líneas nunca
se muestran juntas.

**La razón cabe en una frase.** Las posiciones de un archivo MicroDVD se
calcularon *a partir de* sus fotogramas, a una velocidad dada; deducirlas de una
rejilla devolvería esa misma velocidad, tal como se acaba de proporcionar. Sería
una cifra dada, disfrazada de medida.

Lo que se escribe en su lugar es **la velocidad de lectura y de dónde procede**,
con la misma forma que la línea `encoding`:

| Lo que dice la línea | Cuándo |
| :------------------ | :---- |
| `frame rate: 25 fps, as asked for` | `--frame-rate` la impuso |
| `frame rate: 24000/1001 fps, assumed` | nadie la dijo; es el valor por defecto |

**Dos respuestas y no tres**, donde `encoding` tiene tres: un archivo MicroDVD
no declara nada. Ese es todo el problema: la velocidad viene necesariamente
de fuera, y el informe dice de cuál de los dos «fuera».

**La velocidad decide todo el resto del informe.** `span` pasa por ella, y dos
lecturas del mismo archivo a dos velocidades dan dos lapsos:

<!-- example: printf '{25}{75}Première.\n{100}{150}Deuxième.\n{200}{260}Troisième.\n' > images.sub; subedit-cli --quiet inspect images.sub | tail -3; subedit-cli --quiet inspect --frame-rate 25 images.sub | tail -3 -->
```console
$ printf '{25}{75}Première.\n{100}{150}Deuxième.\n{200}{260}Troisième.\n' > images.sub; subedit-cli --quiet inspect images.sub | tail -3; subedit-cli --quiet inspect --frame-rate 25 images.sub | tail -3
  span: 00:00:01.043 -> 00:00:10.844
  frame rate: 24000/1001 fps, assumed
  anomalies: none
  span: 00:00:01.000 -> 00:00:10.400
  frame rate: 25 fps, as asked for
  anomalies: none
```

## Lo que indica `anomalies`

Tres cosas pueden estar mal en un documento, y cada una se repara de forma distinta.
**Por defecto, `inspect` las cuenta**, una línea por tipo, en el orden de la tabla:
un archivo mal hecho lleva cientos de ellas, y saber cuántas hay de cada tipo
es lo que dice qué ejecutar. **`--detail` las nombra una a una.**

| Lo que se escribe | Con `--detail` | Qué significa |
| :--------------- | :-------------- | :------------------ |
| `subtitle ends before it starts: N` | `subtitle N ends before it starts` | el fin llega antes del inicio |
| `subtitle starts before the previous one ends: N` | `subtitle N starts before the previous one ends` | se solapa con el anterior |
| `subtitle starts before the previous one starts: N` | `subtitle N starts before the previous one starts` | rompe el orden del archivo; [`sort`](sort.md) lo vuelve a colocar |

Un tipo ausente no se escribe. En la columna de la izquierda, `N` es el número
de subtítulos; en la siguiente, el número de un subtítulo.

**El mismo subtítulo puede aparecer dos veces**, y es intencionado: el que
empieza antes de que haya empezado el anterior también empieza antes de que este haya
terminado. Lo primero se resuelve moviendo el subtítulo, lo segundo
cambiando una duración.

**El subtítulo nombrado es el que está mal colocado**, no aquel respecto al cual
está mal colocado. Con inicios en `0`, `4 s`, `2 s`, `3 s`: el tercero rompe el orden, el
cuarto sí sigue al tercero y por tanto no se nombra: no hay nada que
hacer con él.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect desordre.srt | tail -3 -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect desordre.srt | tail -3
  anomalies:
    subtitle starts before the previous one ends: 1
    subtitle starts before the previous one starts: 1
```

Y con `--detail`, el subtítulo nombrado:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect --detail desordre.srt | tail -3 -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect --detail desordre.srt | tail -3
  anomalies:
    subtitle 3 starts before the previous one ends
    subtitle 3 starts before the previous one starts
```

**Una línea por anomalía con `--detail`**, porque cien hallazgos
pegados en una línea no se pueden ni leer ni buscar. Un documento sin defecto conserva
su única línea, `anomalies: none`, que es la que busca un script. **El
registro de `--format json` enumera siempre cada anomalía**: un script lee el detalle, y
`--detail` solo cambia el texto.

**No es un diagnóstico de lectura.** Los diagnósticos dicen con qué se encontró la
lectura y señalan una **línea del archivo**; las anomalías dicen
lo que el documento *es* y señalan un **subtítulo**. Un archivo cuyo orden
está roto no está mal formado por ello: la lectura no tiene nada que decir al respecto.

## En JSON

Con [`--format json`](invocation.md#una-salida-que-un-script-puede-leer), `inspect` escribe **un
objeto por archivo** en lugar de los bloques anteriores; la narración se queda en la salida de error.
Las claves describen lo que dice el texto, y **ninguna tiene que extraerse de una frase**:

| Clave | Contenido |
| :-- | :------ |
| `format` | el identificador del formato, el que acepta `convert --to`: `srt`, `vtt`… |
| `encoding` | `{"charset", "origin", "byte_order_mark"}`; `origin` es `byte-order-mark`, `asked` o `detected` |
| `line_endings` | `{"kind", "mixed_from_line"}`; `lf`, `crlf` o `cr`; la línea donde aparece un segundo tipo, o `null` |
| `subtitles` | el número de subtítulos |
| `span_ms` | `{"start", "end"}`: el primer inicio y el último fin, en milisegundos |
| `frame_rate` | para un archivo contado en fotogramas: `{"rate", "origin"}` (`asked` o `assumed`); en otro caso, `null` |
| `grid` | para los demás: la rejilla deducida, o `null` para un archivo contado en fotogramas |
| `anomalies` | una matriz de `{"subtitle", "kind"}`: `end-before-start`, `overlapping-subtitles`, `out-of-order` |
| `translation` | **solo con `-t`**: `{"file", "method", "attached", "born", "untranslated", "out_of_order"}`; `method` es `position` o `number`, y los cuatro recuentos son los de la frase anterior |
| `warnings` | aquello con lo que se encontró la lectura, en todos los niveles |

`grid` lleva `verdict` (`clean`, `partial` o `silent`), `enough_starts`, `rate` (una cadena,
o `null` cuando no se nombra nada), `concentration_permille` (milésimas, `null` sin suficientes
inicios), `offset_ms`, `also_fits` (la velocidad de la que esta es un divisor entero, o
`null`), `not_separated` (las velocidades que un lapso demasiado corto no separa), `strays`
(los inicios fuera de la rejilla) y `starts`.

<!-- example: printf '{25}{50}Un.\n{75}{100}Deux.\n' > f.sub; subedit-cli --format json inspect --frame-rate 25 f.sub 2>/dev/null -->
```console
$ printf '{25}{50}Un.\n{75}{100}Deux.\n' > f.sub; subedit-cli --format json inspect --frame-rate 25 f.sub 2>/dev/null
{"schema":1,"command":"inspect","file":"f.sub","ok":true,"format":"microdvd","encoding":{"charset":"UTF-8","origin":"detected","byte_order_mark":false},"line_endings":{"kind":"lf","mixed_from_line":null},"subtitles":2,"span_ms":{"start":1000,"end":4000},"frame_rate":{"rate":"25","origin":"asked"},"grid":null,"anomalies":[],"warnings":[]}
```

## Narración

En la salida de error, según el nivel pedido:

| Nivel | Lo que se añade |
| :----- | :-------------- |
| 1 | `<path>: N subtitles` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings` |
| 3 | `<path>: N bytes read`, `<path>: N diagnostic(s) while reading`, y después **cada uno de ellos**; véase [Invocación](invocation.md#diagnósticos-de-lectura) |

## Códigos de salida

Los de la herramienta: `0` si se leyeron todos los archivos, `2` si ninguno, `3` si
solo algunos, `1` en un error de uso.

## Errores

Los de una lectura (del archivo, y de la traducción cuando se da `-t`) y
los de `-t`: véase más arriba. `inspect` no escribe nada que pudiera rechazarse.

| Qué lo provoca | Mensaje |
| :------------------ | :------ |
| no se da ninguna ruta | `files is required`, seguido de una indicación hacia `--help` |
| archivo inexistente | `<path>: does not exist` |
| archivo ilegible | `<path>: cannot be opened: permission denied` |
| bytes que no se decodifican | `<path>: cannot be decoded in the chosen encoding` |
| formato no reconocido | `<path>: is in no format this tool knows` |
| nada que parezca un subtítulo | `<path>: holds nothing recognisable as a subtitle` |

Son los mismos mensajes para los diecisiete subcomandos: la receta de apertura
se escribe una sola vez, en el núcleo.

## Un ejemplo que falla

Un archivo vacío no corresponde a ningún formato. No se indica como vacío:
se rechaza, y se nombra.

<!-- example: : > vide.srt; subedit-cli inspect vide.srt; echo "code=$?" -->
```console
$ : > vide.srt; subedit-cli inspect vide.srt; echo "code=$?"
vide.srt: is in no format this tool knows
code=2
```
