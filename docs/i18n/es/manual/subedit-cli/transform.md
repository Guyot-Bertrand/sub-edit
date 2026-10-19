# `transform`

<!-- machine-translated -->
> Esta página es una traducción automática y todavía no la ha revisado ningún lector del idioma.

```
subedit-cli transform --first <index>=<time> --last <index>=<time>
                      (--output FILE | --output-dir DIR | --in-place)
                      [--dry-run]
                      [--recursive]
                      <file>...
```

Corrige **todas** las posiciones del archivo a partir de dos puntos cuyo valor correcto
se conoce. Es la operación que pide una subtitulación que deriva
—correcta al principio, errónea al final—, ante la que [`shift`](shift.md), que lo mueve todo
la misma cantidad, no puede hacer nada.

El formato del archivo leído se **conserva**: cambiar de formato es tarea de
[`convert`](convert.md).

<!-- example: subedit-cli transform --help -->
```console
$ subedit-cli transform --help
Correct every position from two points known to be right
Usage: subedit-cli transform [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to transform

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --first TEXT REQUIRED       Earlier reference, as <index>=<time>: 1=00:00:01.000
  --last TEXT REQUIRED        Later reference, as <index>=<time>: 3=00:00:10.000
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
| `--first` | **sí** | una referencia, `<index>=<time>` | — |
| `--last` | **sí** | una referencia, `<index>=<time>` | — |
| `--output` / `--output-dir` / `--in-place` | **una de las tres**, salvo con `--dry-run` | véase [Invocación](invocation.md#el-destino) | — |
| `--dry-run` | no | un indicador: calcula e informa, no escribe nada — véase [Ver antes de escribir](invocation.md#ver-antes-de-escribir) | desactivado |

No hay forma corta: estas dos opciones solo se escriben
completas.

## Cómo se escribe una referencia

Una referencia nombra un subtítulo e indica dónde debe caer ahora su **inicio**:

```
<index>=<time>
```

**El índice se cuenta desde 1**, tal como se muestra y como lo numera un archivo
SubRip. `0` se rechaza, y también un índice más allá del último
subtítulo.

**El tiempo se escribe como el de [`shift`](shift.md)**: segundos o
marca de tiempo, con el punto como separador decimal y el milisegundo como resolución:

| Forma | Ejemplos |
| :---- | :------- |
| segundos | `10`, `2.999` |
| marca de tiempo | `00:00:10.000`, `01:39:37.040` |

`--first 3=10` y `--first 3=00:00:10.000` dicen lo mismo.

`--first` y `--last` nombran dos puntos, no un intervalo. Sus nombres indican
el uso esperado —una referencia al principio del archivo y la otra al final— y no una
restricción: intercambiarlas da el mismo resultado, y **todo lo que queda fuera
de las dos referencias sigue igual que lo que está entre ellas**.

## Lo que pasa con las demás posiciones

Las dos referencias fijan un factor de escala, y todas las posiciones lo siguen:

```
t′ = y₁ + (t − x₁) × r      with r = (y₂ − y₁) / (x₂ − x₁)
```

`x₁`, `x₂` son los inicios actuales de los dos subtítulos nombrados, `y₁`, `y₂` los
tiempos pedidos. El factor se guarda como un racional exacto y **el redondeo
se hace una sola vez**, al milisegundo; eso es lo que hace que las dos
referencias caigan **exactamente** en los tiempos pedidos y no con una milésima
de diferencia (véase [ADR 0013](../../../../adr/0013-mise-a-l-echelle-exacte-des-positions.md)).

Cada subtítulo permanece en pantalla **en proporción** al tiempo que permanecía:
el fin sigue la misma escala que el inicio. Una transformación no es un
desplazamiento, y no tiene su propiedad.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,001 --> 00:00:07,000\nDeux.\n\n3\n00:00:09,000 --> 00:00:11,000\nTrois.\n\n' > a.srt; subedit-cli transform --first 1=00:00:01.000 --last 3=00:00:10.000 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,001 --> 00:00:07,000\nDeux.\n\n3\n00:00:09,000 --> 00:00:11,000\nTrois.\n\n' > a.srt; subedit-cli transform --first 1=00:00:01.000 --last 3=00:00:10.000 --output b.srt a.srt; cat b.srt
a.srt: 3 subtitles transformed onto 1=00:00:01.000 and 3=00:00:10.000 -> b.srt
1
00:00:01,000 --> 00:00:03,250
Un.

2
00:00:05,501 --> 00:00:07,750
Deux.

3
00:00:10,000 --> 00:00:12,250
Trois.
```

La primera referencia no se mueve, la tercera pasa de 9 s a 10 s: el factor es
`9/8`, y el segundo subtítulo —que nadie ha nombrado— pasa de
`5,001` a `5,501`, es decir, `1000 + 4001 × 9/8` redondeado una sola vez.

## Dos referencias que no lo son

| Situación | Cuándo se detecta | Código |
| :-------- | :------------- | :--- |
| `--first` y `--last` nombran el mismo índice | antes de cualquier lectura | `1` |
| un índice está más allá del último subtítulo | al abrir el archivo | `2` o `3` |
| los dos subtítulos nombrados empiezan en el mismo instante | al abrir el archivo | `2` o `3` |

Los tres se rechazan por la misma razón: `x₂ − x₁` es cero, y dos
puntos que coinciden no definen ninguna recta. Devolver una división por cero disfrazada de
resultado sería peor que rechazar.

El primer caso es un **error de uso**: las dos opciones fallan sean cuales sean
los archivos, así que corresponde un solo mensaje, no uno por archivo. Los
otros dos dependen del archivo abierto y cuentan, por tanto, como un fallo
de procesamiento.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n' > a.srt; subedit-cli transform --first 2=1.000 --last 2=4.000 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n' > a.srt; subedit-cli transform --first 2=1.000 --last 2=4.000 --output b.srt a.srt; echo "code=$?"
--first and --last both name subtitle 2, and two references on one subtitle define no transform
code=1
```

Un índice fuera de los límites se rechaza **nombrando el límite**, lo que indica a la
vez lo que podría haberse escrito:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,000 --> 00:00:07,000\nDeux.\n\n' > a.srt; subedit-cli transform --first 1=1.000 --last 9=10.000 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,000 --> 00:00:07,000\nDeux.\n\n' > a.srt; subedit-cli transform --first 1=1.000 --last 9=10.000 --output b.srt a.srt; echo "code=$?"
a.srt: subtitle 9 is past the end: the file holds 2 subtitles
code=2
```

## Se rechaza una posición anterior al origen

Como en [`shift`](shift.md), y por la misma razón: una posición negativa
se escribe `-00:00:01,000`, una forma que ninguno de los nueve formatos define y que ningún
lector acepta. El caso se da aquí cuando un subtítulo está **antes de
la primera referencia** y la escala lo empuja más allá de cero. No se escribe
nada, el código es `2`, y el mensaje nombra el subtítulo culpable.

## Salida

**Salida estándar**: nada; el resultado es el archivo escrito. Con `--dry-run`, tampoco nada: no hay archivo, y la línea de narración lo dice — véase [Ver antes de escribir](invocation.md#ver-antes-de-escribir).

| Nivel | Lo que se añade en la salida de error |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles transformed onto 1=00:00:01.000 and 3=00:00:10.000 -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — el formato, la codificación, la marca y los finales de línea del archivo leído, restituidos tal como estaban |
| 3 | `<path>: N bytes read, M written`, y después **cada diagnóstico de lectura** — véase [Invocación](invocation.md#diagnósticos-de-lectura) |

Las referencias se vuelven a escribir con el punto decimal, se hayan tecleado como se hayan tecleado.

### En JSON

Con [`--format json`](invocation.md#una-salida-que-un-script-puede-leer), el objeto de cada archivo lleva `counts.subtitles` (todos los subtítulos del archivo). La tabla completa es la de [Invocación](invocation.md#lo-que-cada-subcomando-pone-en-counts).

## Códigos de salida

Los de la herramienta: `0` si se escribieron todos los archivos, `2` si ninguno, `3` si
solo algunos, `1` en caso de error de uso.

## Errores

| Qué lo provoca | Mensaje |
| :------------------ | :------ |
| referencia sin `=` | `"…" is not a reference: write it <index>=<time>, as in 3=00:00:10.000` |
| referencia a la que le falta una mitad | `"…" is missing one of its two halves: write it <index>=<time>, as in 3=00:00:10.000` |
| índice `0` | `"0" is not a subtitle number: subtitles are counted from 1, as the file shows them` |
| índice que no es un número entero | `"…" is not a subtitle number: expected a whole number, counted from 1 as the file shows them` |
| tiempo no válido | los de [`shift`](shift.md) |
| `--first` y `--last` en el mismo índice | `--first and --last both name subtitle N, and two references on one subtitle define no transform` |
| índice fuera de los límites | `<path>: subtitle N is past the end: the file holds M subtitles` |
| dos subtítulos que empiezan a la vez | `<path>: subtitles N and M start at the same moment, so they define no transform` |
| posición anterior al origen | `<path>: subtitle N would land before the origin, which no subtitle file can hold` |

**No se escribe nada sin una de las tres opciones de destino**, y sus
errores son comunes a los subcomandos que escriben:
véase [Invocación](invocation.md#el-destino).
