# `shift`

<!-- machine-translated -->
> Esta página es una traducción automática y todavía no la ha revisado ningún lector del idioma.

```
subedit-cli shift (--by <time> | --to-grid)
                  (--output FILE | --output-dir DIR | --in-place)
                  [--dry-run]
                  [--recursive]
                  <file>...
```

Desplaza **todas** las posiciones del archivo la misma cantidad, en un sentido
o en otro. Ningún subtítulo permanece en pantalla más ni menos tiempo:
eso es lo que hace de un desplazamiento un desplazamiento.

<!-- example: subedit-cli shift --help -->
```console
$ subedit-cli shift --help
Move every position of a file by a fixed amount
Usage: subedit-cli shift [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to shift

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --by TEXT                   Amount to move by: 2.999, -7.001, or 00:00:07.001
  --to-grid                   Move by the amount that puts the positions back on their frame grid
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
| `--by` | **una de las dos** | una duración con signo, véase más abajo | — |
| `--to-grid` | **una de las dos** | indicador: la cantidad se mide, véase más abajo | — |
| `--output` / `--output-dir` / `--in-place` | **una de las tres**, salvo con `--dry-run` | véase [Invocación](invocation.md#el-destino) | — |
| `--dry-run` | no | un indicador: calcula e informa, no escribe nada — véase [Ver antes de escribir](invocation.md#ver-antes-de-escribir) | desactivado |

El formato del archivo leído se **conserva**: cambiar de formato es tarea de
[`convert`](convert.md).

## `--to-grid`: la cantidad medida en lugar de indicada

Un archivo cuyas posiciones están sobre una rejilla de fotogramas **salvo una
constante** ha sido desplazado, y esa constante puede medirse en las propias posiciones.
`--to-grid` la mide y la aplica, sin que nadie tenga que conocerla.

La cantidad es **propia de cada archivo**: dos archivos desplazados fuera de la misma
rejilla en dos cantidades distintas vuelven cada uno según la suya. Se
indica en la narración, junto con la velocidad de fotogramas encontrada.

**El camino es el más corto.** Pasado medio fotograma, la línea de la rejilla más
cercana es la siguiente: la corrección avanza en lugar de retroceder
casi un fotograma entero.

<!-- example: k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40 + 7)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > decale.srt; subedit-cli shift --to-grid --output recale.srt decale.srt -->
```console
$ k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40 + 7)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > decale.srt; subedit-cli shift --to-grid --output recale.srt decale.srt
decale.srt: 12 subtitles shifted by -0.007 s onto their 25 fps grid -> recale.srt
```

**Un archivo que no está sobre ninguna rejilla conocida se rechaza, y se dice por qué.**
No hay nada a lo que volver, y una fase medida sobre ruido movería el archivo
al azar. Véase [`inspect`](inspect.md#lo-que-indica-frame-rate-grid) para
saber lo que da un archivo antes de intentar la operación.

`--by` y `--to-grid` indican ambas cuánto desplazar: darlas juntas
es un error de uso, y no dar ninguna también.

**`--to-grid` no es [`snap`](snap.md).** Desplaza todo el archivo
la misma cantidad y conserva la temporización relativa **exactamente**; `snap`
mueve cada posición por separado y absorbe de paso las que se habían
corregido a mano. Sobre un archivo limpio, las dos dan el mismo
resultado; sobre un archivo parcial, no.

## Cómo se escribe una duración

Dos formas, y solo dos:

| Forma | Ejemplos |
| :---- | :------- |
| segundos | `30`, `2.999`, `-7.001`, `+1.5` |
| marca de tiempo | `00:00:07.001`, `01:39:37.040`, `-00:00:07.001` |

Las dos dicen lo mismo: `--by 7.001` y `--by 00:00:07.001` producen
el mismo archivo.

**El separador decimal es el punto**, porque la línea de comandos habla inglés.
Una coma se rechaza en lugar de interpretarse: medio mundo escribe `7,001`
para siete mil uno, y leerlo como siete segundos sería una suposición,
equivocada para esa mitad.

**La resolución es el milisegundo.** Lo que es más fino se rechaza en lugar de redondearse
en silencio: un redondeo tácito daría un resultado que no corresponde a nada de lo
que se pidió.

<!-- example: printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle shifted by -7.001 s -> b.srt
1
00:00:02,999 --> 00:00:04,999
Bonjour.
```

## Se rechaza un desplazamiento anterior al origen

Retroceder más allá de cero da el código `2` y **no escribe nada**. El mensaje nombra
el subtítulo que no podía admitir el desplazamiento, lo que indica a la vez
hasta dónde se puede llegar.

La razón no es de principio: una posición anterior al origen se escribe
`-00:00:01,000`, una forma que ninguno de los nueve formatos define y que ningún lector
acepta. El núcleo sí admite esas posiciones —rechazarlas haría de una
operación de edición un caso particular—, pero un archivo es algo distinto de un
modelo en memoria.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; echo "code=$?"
a.srt: subtitle 1 would start before the origin, which no subtitle file can hold
code=2
```

## Salida

**Salida estándar**: nada; el resultado es el archivo escrito. Con `--dry-run`, tampoco nada: no hay archivo, y la línea de narración lo dice — véase [Ver antes de escribir](invocation.md#ver-antes-de-escribir).

| Nivel | Lo que se añade en la salida de error |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles shifted by 2.999 s -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — el formato, la codificación, la marca y los finales de línea del archivo leído, restituidos tal como estaban |
| 3 | `<path>: N bytes read, M written`, y después **cada diagnóstico de lectura** — véase [Invocación](invocation.md#diagnósticos-de-lectura) |

### En JSON

Con [`--format json`](invocation.md#una-salida-que-un-script-puede-leer), el objeto de cada archivo lleva `counts.subtitles` (todos los subtítulos del archivo) y `counts.shifted_by_ms` (el desplazamiento, en milisegundos enteros, **con signo**). La tabla completa es la de [Invocación](invocation.md#lo-que-cada-subcomando-pone-en-counts).

## Códigos de salida

Los de la herramienta: `0` si se escribieron todos los archivos, `2` si ninguno, `3` si
solo algunos, `1` en caso de error de uso.

## Errores

| Qué lo provoca | Mensaje |
| :------------------ | :------ |
| coma decimal | `"…" is not a time: use a decimal point and not a comma, in every language` |
| más fino que el milisegundo | `"…" is not a time: positions are held to the millisecond, no finer` |
| texto que no es una duración | `"…" is not a time: expected seconds like 2.999, or a timestamp like 00:00:07.001` |
| desplazamiento anterior al origen | `<path>: subtitle N would start before the origin, which no subtitle file can hold` |

**No se escribe nada sin una de las tres opciones de destino**, y sus
errores son comunes a los subcomandos que escriben:
véase [Invocación](invocation.md#el-destino).
