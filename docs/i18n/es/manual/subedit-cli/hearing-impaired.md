# `hearing-impaired`

<!-- machine-translated -->
> Esta página es una traducción automática y todavía no la ha revisado ningún lector del idioma.

```
subedit-cli hearing-impaired
                  (--output FILE | --output-dir DIR | --in-place)
                  [--dry-run]
                  [--recursive] [--parentheses]
                  [--document main|translation] [-t FILE] [--align-method position|number]
                  <file>...
```

Elimina de todo el archivo las **anotaciones para sordos**, los sonidos descritos entre corchetes.
**Los paréntesis se conservan por defecto**: se usan tanto para una réplica
susurrada como para un sonido descrito, y eliminarlos borraría diálogo. `--parentheses` los elimina
también, para un archivo que describe en ellos sus sonidos.

Lo que no hace: las letras de canciones entre almohadillas, el nombre del hablante
delante de dos puntos, volver a poner mayúsculas, corregir los errores de OCR. Esos
patrones son los del asistente
[`Correct Texts…`](../subedit-gui/correct-texts.md) de la ventana, que la línea
de comandos ofrece con el nombre de [`correct`](correct.md); y aquí no se puede ajustar nada: la
transformación está decidida, no es configurable.

<!-- example: subedit-cli hearing-impaired --help -->
```console
$ subedit-cli hearing-impaired --help
Remove the sounds described between square brackets
Usage: subedit-cli hearing-impaired [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to clean

Options:
  -h,--help                   Print this help message and exit
  --parentheses               Also remove the sounds between parentheses, often whispered lines
  -r,--recursive              Take directories as inputs, and every subtitle file in them
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
| `<file>...` | sí | una o varias rutas | — |
| `--parentheses` | no | un indicador: elimina también las anotaciones entre paréntesis | desactivado |
| `--recursive`, `-r` | no | un indicador | desactivado — véase [Procesar un árbol](lots.md) |
| `--document` | no | `main` o `translation` — véase [Una traducción](invocation.md#una-traducción) | `main` |
| `-t`, `--translation-file` | con `--document translation` | un archivo de traducción, para **una sola** entrada | — |
| `--align-method` | no, y solo con `-t` | `position` o `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **una de las tres**, salvo con `--dry-run` | véase [Invocación](invocation.md#el-destino) | — |
| `--dry-run` | no | un indicador: calcula e informa, no escribe nada — véase [Ver antes de escribir](invocation.md#ver-antes-de-escribir) | desactivado |

El formato del archivo leído se **conserva**: cambiar de formato es tarea de
[`convert`](convert.md).

**El texto principal por defecto; la traducción con `--document translation -t FILE`.** En ese caso
es la traducción la que se modifica y se escribe, y solo ella — véase [Una traducción](invocation.md#una-traducción).

**Una anotación que vaciaría una traducción la escribe vacía, y conserva el subtítulo**: el subtítulo
se queda en el archivo, sin texto. El texto principal, por su parte, pierde un subtítulo vaciado por completo, como
se ve más abajo.

## Lo que se elimina y lo que se queda

Una anotación es un corchete —o un paréntesis, con `--parentheses`— y lo que encierra, **salto
de línea incluido**: en los archivos reales, una anotación suele estar cortada por el
final de una línea.

| Entrada | Salida |
| :----- | :----- |
| `Bonjour [il tousse] Marie` | `Bonjour Marie` |
| `Bonjour[il tousse]Marie` | `Bonjour Marie` |
| `[Bruit de pas] Bonjour` | `Bonjour` |
| `A [un] [deux] B` | `A B` |

La eliminación deja **exactamente un espacio entre lo que la rodeaba, y nada en
el borde de una línea**. No es una limpieza de los espacios del archivo: dos espacios
que el texto ya tenía siguen siendo dos espacios.

**Cuando la anotación se extiende a ambos lados del salto de línea, es el salto lo que queda**: las
dos líneas siguen siendo dos líneas. Este caso no es una curiosidad: en quince
subtitulaciones reales, todos los corchetes que parecían huérfanos eran una
anotación cortada así.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nReculez ! [Il\nhurle] Tout de suite !\n\n' > a.srt; subedit-cli --quiet hearing-impaired --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nReculez ! [Il\nhurle] Tout de suite !\n\n' > a.srt; subedit-cli --quiet hearing-impaired --output b.srt a.srt; cat b.srt
1
00:00:01,000 --> 00:00:03,000
Reculez !
Tout de suite !
```

**Lo que no es una anotación y no se mueve:**

| Caso | Lo que ocurre |
| :-- | :-------------- |
| contenido puramente numérico —`[1]`, `(12)`, `[ 156478 ]`— | se conserva tal cual, delimitadores incluidos: una referencia nunca ha nombrado un sonido |
| un paréntesis y lo que encierra, sin `--parentheses` | se conserva tal cual: `Venez (chut) ici` no cambia |
| un corchete o un paréntesis que nada cierra | se deja tal cual |
| una etiqueta de formato —`<i>`, `</i>`— | nunca se elimina |

Un espacio **entre** las cifras, una letra o un signo —`[1 2]`, `[1a]`,
`[-1]`— lo convierten en anotación. Lo mismo ocurre con un contenido vacío, `[]` o `( )`: ninguna
cifra, luego no es una referencia; nada que decir, luego nada que conservar.

## Lo que desaparece por completo

**Una línea que la eliminación ha vaciado desaparece**, y **un subtítulo vaciado
por completo se elimina del archivo**. Los subtítulos siguientes se renumeran.

Una línea cuenta como vacía cuando lo único que queda en ella son espacios, etiquetas
de formato o una raya de diálogo aislada: `<i>[Musique]</i>` no deja un
`<i></i>` en pantalla.

**Las etiquetas son las del formato del archivo.** `{\i1}[Musique]{\i0}` en un
Advanced SSA, `{Y:i}[Musique]` en un MicroDVD y `/[Musique]` en un MPL2 tampoco dejan
nada. A la inversa, un `<i>` en un formato que no escribe ninguna
etiqueta —TMPlayer, LRC— es texto que el espectador lee, y la línea que
lo lleva no está vacía.

**Un diálogo reducido a una sola voz deja de serlo**, y la raya que queda
desaparece. Mientras queden dos voces, las rayas se quedan.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\nVoir [1] la note.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\nVoir [1] la note.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle cleaned, 1 removed -> b.srt
1
00:00:04,000 --> 00:00:06,000
Attends Marie.

2
00:00:07,000 --> 00:00:09,000
Voir [1] la note.
```

## Un archivo sin ninguna anotación se escribe de todos modos

Se copia idéntico, el código de salida es `0`, y el informe dice que no
había nada que eliminar.

Es la regla de los subcomandos que escriben: **un destino indicado es
un destino escrito**. Hacer una excepción aquí obligaría a un script a saber
cuál de los subcomandos produce unas veces un archivo y otras nada.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nRien à signaler.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nRien à signaler.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt
a.srt: no mention to remove -> b.srt
```

## Salida

**Salida estándar**: nada; el resultado es el archivo escrito. **Con `--dry-run`, son los cambios propuestos**: para cada subtítulo cuyo texto cambiaría, su número, el texto antes y el texto después — véase [Ver antes de escribir](invocation.md#ver-antes-de-escribir).

| Nivel | Lo que se añade en la salida de error |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles cleaned, M removed -> <destination>`, o `<path>: no mention to remove -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — el formato, la codificación, la marca y los finales de línea del archivo leído, restituidos tal como estaban |
| 3 | `<path>: N bytes read, M written`, y después **cada diagnóstico de lectura** — véase [Invocación](invocation.md#diagnósticos-de-lectura) |

`N` cuenta los subtítulos cuyo texto cambió, `M` los que la eliminación
vació por completo y que, por tanto, salieron del archivo.

### En JSON

Con [`--format json`](invocation.md#una-salida-que-un-script-puede-leer), el objeto de cada archivo lleva `counts.cleaned` (los textos reescritos) y `counts.removed` (los subtítulos eliminados); **con `--dry-run`**, la clave `changes` enumera además los subtítulos modificados, como la salida estándar de la forma de texto. La tabla completa es la de [Invocación](invocation.md#lo-que-cada-subcomando-pone-en-counts).

## Códigos de salida

Los de la herramienta: `0` si se escribieron todos los archivos, `2` si ninguno, `3` si
solo algunos, `1` en caso de error de uso.

## Errores

Este subcomando no tiene ningún error propio: no recibe ningún
valor que analizar, y ningún texto puede hacer que rechace un archivo. Lo que
puede fallar es la lectura o la escritura, como en los demás.

**No se escribe nada sin una de las tres opciones de destino**, y sus errores
son comunes a los subcomandos que escriben:
véase [Invocación](invocation.md#el-destino).
