# `correct`

<!-- machine-translated -->
> Esta página es una traducción automática y todavía no la ha revisado ningún lector del idioma.

```
subedit-cli correct --tasks LIST --code CODE
                    [--classes human|ocr|human,ocr]
                    [--enable NAME]... [--disable NAME]...
                    [--language CODE]
                    [--max-length N] [--max-lines N]
                    [--skip-length N|off] [--skip-lines N|off]
                    [--keep-blank-subtitles | --remove-blank-subtitles]
                    [--range N-M|N-]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    <file>...
```

Corrige los textos con los **patrones de Gaupol**, como el asistente
[`Correct Texts…`](../subedit-gui/correct-texts.md) de la ventana: mismos patrones,
mismo motor, mismos recuentos. Aquí existen seis tareas: las **anotaciones para
sordos**, **unir** y **separar palabras**, los **errores comunes**, la
**capitalización** y el **salto de línea**.

<!-- example: subedit-cli correct --help -->
```console
$ subedit-cli correct --help
Correct the texts with the patterns of the correction assistant
Usage: subedit-cli correct [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to correct

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --tasks LIST                The tasks to run, in a list: none is chosen beforehand
  --code CODE                 The cascade of patterns the tasks read: Zyyy, Latn, Latn-en, Latn-en-US
  --classes human|ocr|human,ocr
                              The classes of common errors to apply: both by default
  --enable NAME               Switch a pattern on, by its English name or type:name; repeatable
  --disable NAME              Switch a pattern off, by its English name or type:name; repeatable
  --language CODE             The dictionary join-words and split-words read: fr_FR, en_US, en...
  --max-length N              Longest line of line-break, in characters; no default, Gaupol's is in ems
  --max-lines N               Most lines of line-break: 3, as Gaupol's
  --skip-length N|off         Leave alone a subtitle whose longest line is within this, or off: --max-length
  --skip-lines N|off          Leave alone a subtitle whose line count is within this, or off: --max-lines
  --keep-blank-subtitles      Leave the subtitles the correction empties, empty, instead of removing them
  --remove-blank-subtitles    Remove the subtitles the correction empties, whatever the settings file says
  --range N-M|N-              Act only on subtitles N to M, or N to the end
  --document main|translation The document to change: the main one, or the translation given by -t
  -t,--translation-file FILE  Translation file to lay over the subtitle file, for a single input
  --align-method position|number
                              How the lines of the translation find their subtitles
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Qué se elige y qué no

**No hay nada marcado de antemano.** La ventana abre sus casillas tal como las dejó el usuario;
una línea de comandos toma sus tareas, su código, su longitud de línea y su idioma de su propia
línea, nunca del archivo de configuración: no se leen ni las tareas marcadas un día en la
ventana, ni las casillas de patrones activadas allí (`correction.activations`), ni la lista de
reemplazos del corrector ortográfico, y no se escribe nada de ese lado.

**Cuatro ajustes proceden del archivo de configuración, por defecto**: los valores que
`Correct Texts…` recuerda, o que escribió [`config set`](config.md). **Una opción en la línea
siempre gana**, después el archivo y después el valor por defecto integrado; véase
[El archivo de configuración](invocation.md#el-archivo-de-configuración) para saber dónde está,
`--config` y `--no-config`.

| Clave | Lo que aporta | Opción que la anula |
| :-- | :--------------- | :----------------------- |
| `correction.human`, `correction.ocr` | las clases de errores comunes aplicadas | `--classes` |
| `correction.remove-blank` | si se eliminan los subtítulos que la corrección deja vacíos | `--keep-blank-subtitles`, `--remove-blank-subtitles` |
| `correction.line-break.max-lines` | el número máximo de líneas de `line-break` | `--max-lines` |

- **Cada uno de los cuatro tiene una opción que lo deshace**, diga lo que diga el archivo:
  `--classes human,ocr` aplica las dos clases, `--remove-blank-subtitles` elimina los subtítulos
  vaciados, `--max-lines 3` devuelve las líneas de Gaupol. `--keep-blank-subtitles` y
  `--remove-blank-subtitles` juntas son un error de uso.
- **Las dos clases desactivadas en el archivo se ignoran, y se dice** en el nivel 1
  (`settings: correction.human and correction.ocr are both false, ignored`): `--classes` no
  podría escribirlo, y dejaría a common-errors sin nada que hacer. En su lugar se aplican las
  dos clases.
- **Un número de líneas fuera del intervalo de 1 a 1000 no se lee del archivo en absoluto**: el
  lector del archivo lo rechaza (`settings: correction.line-break.max-lines: cannot read 'N', default kept`,
  en el nivel 1) y se usa el 3 integrado.

Los ejemplos de esta página se ejecutan con `--no-config`, para que digan lo que hacen los
valores por defecto integrados, sea cual sea la configuración de la máquina que los ejecuta.

| Opción | Obligatoria | Valores | Por defecto |
| :----- | :----- | :------ | :----- |
| `<file>...` | sí | una o varias rutas | — |
| `--tasks` | **sí** | una lista separada por comas: `mentions`, `join-words`, `split-words`, `common-errors`, `capitalization`, `line-break` | ninguna tarea |
| `--code` | **sí** con una tarea de patrones, y **no aplicable** en otro caso | `Script[-language[-COUNTRY]]`: `Zyyy`, `Latn`, `Latn-en`, `Latn-en-US` | — |
| `--language` | **sí** con `join-words` o `split-words`, y **no aplicable** en otro caso | un código de idioma: `fr`, `fr_FR`, `en_US` | — |
| `--classes` | no | `human`, `ocr` o `human,ocr` | las dos, o lo que diga el archivo de configuración |
| `--enable`, `--disable` | no, repetible | el nombre inglés de un patrón, o `type:name` | los archivos `.conf` distribuidos |
| `--max-length` | **sí** con `line-break` | un número de **caracteres** mayor que cero | **ninguno**: véase más abajo |
| `--max-lines` | no | un número entero de líneas, mayor que cero | 3, o lo que diga el archivo de configuración |
| `--skip-length`, `--skip-lines` | no | un número, u `off` | los mismos límites que `--max-length` y `--max-lines` |
| `--keep-blank-subtitles` | no | un indicador: los subtítulos vaciados se quedan, vacíos | los subtítulos vaciados se eliminan, salvo que el archivo de configuración diga otra cosa |
| `--remove-blank-subtitles` | no | un indicador: los subtítulos vaciados se eliminan, diga lo que diga el archivo de configuración; rechazado con `--keep-blank-subtitles` | desactivado |
| `--range` | no | `N-M`, o `N-` hasta el final: véase [`adjust`](adjust.md#--range) | todo el archivo |
| `--document` | no | `main` o `translation`: véase [Una traducción](invocation.md#una-traducción) | `main` |
| `-t`, `--translation-file` | con `--document translation` | un archivo de traducción, para **una sola** entrada | — |
| `--align-method` | no, y solo con `-t` | `position` o `number` | `position` |
| `--recursive`, `-r` | no | un indicador | desactivado: véase [Procesar un árbol](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **una de las tres**, salvo con `--dry-run` | véase [Invocación](invocation.md#el-destino) | — |
| `--dry-run` | no | un indicador: calcula y dice, no escribe nada; véase [Ver antes de escribir](invocation.md#ver-antes-de-escribir) | desactivado |

### Las tareas

**El orden de la lista no cambia nada**: las tareas se aplican en el
orden de Gaupol (las anotaciones, después unir y separar palabras, después los errores comunes, después la capitalización, después el salto de línea), cada una sobre el
texto que dejó la anterior. Un subtítulo que las anotaciones dejaron vacío no
participa en lo que sigue.

| Tarea | Qué hace |
| :---- | :-------------- |
| `mentions` | elimina los sonidos entre corchetes, y entre paréntesis cuando `Sound in parentheses` está activado (el barrido de [`hearing-impaired`](hearing-impaired.md)), y después los patrones de letras de canciones y de interlocutores |
| `join-words` | vuelve a pegar una palabra cortada en dos, según el diccionario de `--language` |
| `split-words` | separa dos palabras pegadas, según el mismo diccionario |
| `common-errors` | corrige los errores comunes: espacios, puntuación, ligaduras, errores de OCR |
| `capitalization` | pone la mayúscula donde la quiere el idioma |
| `line-break` | vuelve a cortar las líneas de un subtítulo para que ninguna supere `--max-length`, sopesando los patrones de corte del código |

### El código de patrones

**`--code` nombra la cascada**: `Zyyy` para cualquier escritura, después la escritura, el idioma
y el país; `Latn-en-US` lee los patrones de `Zyyy`, de `Latn`, de `Latn-en` y de
`Latn-en-US`. **Es obligatorio**, porque una línea de comandos no tiene idioma ambiente:
un valor por defecto universal aplicaría *menos* de lo que se cree, sin decirlo. Vale para
todas las tareas de una misma ejecución; una tarea que quiere un código propio es una segunda
ejecución. Un código mal escrito es un error de uso:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code francais --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code francais --output b.srt a.srt; echo "code=$?"
--code: "francais" is not a pattern code: expected Script[-language[-COUNTRY]], like Zyyy, Latn or Latn-en-US
code=1
```

Los patrones que el usuario pone en `$XDG_DATA_HOME/subedit/patterns` (o
`~/.local/share/subedit/patterns`) **se añaden a los patrones distribuidos**, como en la
ventana, y un patrón distribuido que no se puede leer se indica en el nivel 1: `patterns: <file>, line N (<reason>)`.

### Unir y separar palabras, y el diccionario

**Estas dos tareas no leen ningún patrón**: consultan el **diccionario del sistema, a través de
Enchant**, una dependencia de los paquetes (`libenchant-2-2` en Debian y Ubuntu, `enchant2` en
Fedora) que `subedit-cli` comparte con la ventana. Por tanto no tienen ni código ni nombre que ajustar:
`--code` no tiene sentido sin una tarea de patrones, ni `--language` sin una de estas dos;
dar uno sin la otra es un error de uso, código `1`.

- **`join-words`** pega una palabra que el diccionario no conoce a la que la precede o la
  sigue, **cuando una sola dirección da una palabra conocida**: `hel lo` se convierte en `hello`; dos
  direcciones posibles, o ninguna, no deciden nada.
- **`split-words`** separa una palabra que el diccionario no conoce **cuando exactamente una de sus
  sugerencias es esa palabra con un espacio**: `hellothere` se convierte en `hello there`. Una palabra con
  inicial mayúscula seguida de minúsculas se deja como está: casi siempre es un nombre.

**Sin diccionario para el idioma, la ejecución se rechaza**, código `1`, **antes de leer un
archivo** y con las palabras de la ventana, que desactiva la función y lo dice; aquí la línea de
comandos rechaza y lo dice, en lugar de callarse:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nhel lo\n\n' > a.srt; subedit-cli --no-config correct --tasks join-words --language qq_QQ --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nhel lo\n\n' > a.srt; subedit-cli --no-config correct --tasks join-words --language qq_QQ --output b.srt a.srt; echo "code=$?"
no dictionary for qq_QQ
code=1
```

**La lista de reemplazos de la ventana no se lee ni se escribe.** Se *llena* con las
elecciones del usuario en la comprobación ortográfica; una línea de comandos no tiene esa
elección, y leer la lista haría depender un script de lo que su autor puso en ella un día.

### Salto de línea

**Se mide en caracteres, y esa es la diferencia con la ventana.** La ventana mide en *ems*
(una anchura calibrada sobre la fuente, a 0,55 em por letra), y su 24 por defecto es un valor
en ems. Una línea de comandos no tiene fuente: cuenta caracteres, los del
texto. **Copiar el 24 como caracteres cortaría casi todo, sin
decirlo**; por tanto **no hay longitud por defecto**, y `--max-length` es obligatoria con
`line-break`:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn texte\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn texte\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --output b.srt a.srt; echo "code=$?"
--max-length is required by the task line-break: Gaupol's 24 is a width in ems, and has no value in characters
code=1
```

**`--max-lines` vale 3**, el valor de Gaupol, que la unidad no cambia; el salto de línea toma
una más cuando tres no bastan. **La omisión** (un subtítulo que ya está dentro de los límites se
deja como está) toma por defecto **los mismos límites** (`--skip-length` = `--max-length`,
`--skip-lines` = `--max-lines`), como hace Gaupol; `--skip-length off` y `--skip-lines off`
la desactivan, y el texto se vuelve a leer entonces como un todo y se vuelve a cortar:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nWhen the rain stopped, we walked home under the dark sky of the old town.\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --max-length 24 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nWhen the rain stopped, we walked home under the dark sky of the old town.\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --max-length 24 --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:03,000
When the rain
stopped, we walked
home under the dark
sky of the old town.
```

Los patrones de corte son los del código: `Latn` no corta entre un número y su unidad ni
después de una raya de diálogo, `Latn-en` no corta después de un artículo, una preposición, un
tratamiento o un determinante posesivo. **Con `Zyyy` no existe ningún patrón de corte**: las líneas
solo se equilibran. **Esta tarea nunca se rechaza por falta de un patrón activo**: sus patrones
sopesan dónde cortar, el corte se produce sin ellos. Las cuatro opciones solo tienen sentido con
`line-break`; dadas sin ella, son un error de uso.

### Las clases, y la activación por nombre

**`--classes` filtra los errores comunes de verdad**: desmarcar `ocr` saca sus
patrones de la aplicación, y la ligadura `ﬁ` se queda entonces como `ﬁ`. Los otros tres tipos de patrones
no llevan clase.

**`--enable` y `--disable` ajustan un patrón por su nombre inglés**, que es la clave
(ADR 0037); un nombre puede designar varios registros, que comparten entonces una sola
casilla. Es necesario, y es lo que deja de lado los valores por defecto de la ventana: **los
archivos `.conf` distribuidos desactivan los patrones de anotaciones**, de modo que `--tasks mentions` sola
no aplicaría nada. Los dos patrones «Sound in brackets» y «Sound in parentheses»
gobiernan cada uno su delimitador (los corchetes, los paréntesis) y se ajustan con las mismas
opciones: activar uno solo no elimina el otro.

Tres cosas son **errores de uso**, código `1`, indicados antes de leer un archivo:

| Qué lo provoca | Mensaje |
| :------------------ | :------ |
| un nombre que no designa ningún patrón de las tareas dadas, con el código dado | `--enable: "<name>" names no pattern of the tasks given, under the code <code>` |
| un nombre que designa patrones de dos tipos: se escribe entonces `type:name`, siendo el tipo `common-error`, `capitalization`, `hearing-impaired` o `line-break` | `--enable: "<name>" names patterns of several types: write type:name, …` |
| un nombre dado a `--enable` y a `--disable` | `"<name>" is given to --enable and to --disable: choose one` |

**Un error de escritura hace ruido**: la ventana ignora un ajuste obsoleto porque un
ajuste puede sobrevivir a un patrón que ha desaparecido; una invocación no tiene superviviente.

**Una tarea sin ningún patrón activo se rechaza**, código `1`: «nada que hacer» es una
respuesta que se dice, no un éxito silencioso:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --output b.srt a.srt; echo "code=$?"
mentions: no pattern is active under the code Latn; switch one on with --enable
code=1
```

El mismo archivo, con el barrido de corchetes activado:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --enable "Sound in brackets" --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --enable "Sound in brackets" --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:02,000
Bonjour
```

## Qué se escribe y qué se dice

El formato, la codificación y los finales de línea del archivo leído **se conservan**. **Los
subtítulos que la corrección deja vacíos se eliminan**, como la casilla por defecto de la
ventana; `--keep-blank-subtitles` los deja vacíos, y `--remove-blank-subtitles` los elimina
incluso cuando el archivo de configuración los conserva. **Una traducción vaciada se queda vacía**,
y conserva su subtítulo.

**El recuento es el de la ventana** («Edited N and removed M subtitles»): textos
cambiados y subtítulos eliminados, **nunca coincidencias**. Un archivo en el que nada
cambia se escribe de todos modos: un destino dado es un destino escrito.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\neﬀet  de ﬁn\n\n2\n00:00:03,000 --> 00:00:04,000\nPropre.\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code Latn --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\neﬀet  de ﬁn\n\n2\n00:00:03,000 --> 00:00:04,000\nPropre.\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code Latn --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:02,000
effet de fin

2
00:00:03,000 --> 00:00:04,000
Propre.
```

### Un patrón que no puede aplicarse

**Un patrón que no se puede leer, no se puede traducir o no termina se nombra, con el
subtítulo en el que se rindió**, y aquí la línea de comandos dice más que la
ventana: la página de confirmación nombra el patrón una sola vez, sin el texto.

| Motivo | Qué significa |
| :----- | :------------------- |
| `cannot be translated` | su expresión usa algo que la traducción a ICU rechaza |
| `will not compile` | el motor rechazó su expresión |
| `has an invalid replacement` | su reemplazo no se lee como lo lee Python |
| `timed out` | el motor se rindió con ese texto |
| `never settled` | `Repeat` seguía cambiando el texto después de cien pasadas |
| `grew the text too long` | el texto superó lo que un subtítulo puede contener |

En el nivel 1, en la salida de error: 
`<path>: pattern "<name>" (<reason>) was not applied to subtitle N`, sin el final cuando el patrón se deja de lado para todos los textos. **El número es el del
archivo**, sean cuales sean las tareas que eliminaron subtítulos antes. En JSON, es
una advertencia `pattern-failed` del archivo, cuyo `detail` lleva la frase. **Un patrón
abandonado no es un archivo fallido**: el archivo se escribe, y el código de salida es `0`.

## Salida

**Salida estándar**: nada, ya que el resultado es el archivo escrito. **Con `--dry-run`, son los
cambios propuestos**, un bloque por subtítulo: su número, el texto antes, el texto después,
y `(removed)` sin línea `+` para un subtítulo que la corrección eliminaría; con
`--keep-blank-subtitles`, se queda, vacío. Véase [Ver antes de escribir](invocation.md#ver-antes-de-escribir).

| Nivel | Lo que se añade en la salida de error |
| :----- | :------------------------------------- |
| 1 | `<path>: Edited N and removed M subtitles -> <destination>`, los patrones que no pudieron aplicarse, los archivos de patrones que no se pueden leer y, en primer lugar, el archivo de configuración leído (`settings: read <path>`) y lo que de él se ignoró |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, y después cada diagnóstico de lectura |

En JSON, `counts` lleva `corrected` (los textos cambiados) y `removed` (los subtítulos
eliminados), y `changes` los subtítulos tocados. Con `-t`, la clave `alignment` dice cómo se
colocó la traducción.
`settings` nombra el archivo de configuración que se leyó, o es `null` cuando no se leyó ninguno.

## Códigos de salida

Los de la herramienta: `0` si se escribieron todos los archivos, `2` si ninguno, `3` si solo
algunos, `1` en un error de uso.

## Errores

| Qué lo provoca | Qué se escribe, en la salida de error |
| :------------------ | :--------------------------------------- |
| un archivo nombrado por `--config` que no existe (el archivo propio del usuario, si falta, da los valores por defecto) | `settings file not found: <FILE>` |
| falta `--tasks` | `--tasks is required`, seguido de una indicación hacia `--help` |
| una tarea desconocida | `--tasks: "<name>" is not a task: expected mentions, join-words, split-words, common-errors, capitalization or line-break` |
| falta `--code` | `--code is required by the tasks that read patterns: <tasks>` |
| `--code` sin tarea de patrones | `--code is for the tasks that read patterns, and none was asked for` |
| falta `--language` con `join-words` o `split-words` | `--language is required by the tasks that check words: <tasks>` |
| un `--language` que no es un código de idioma | `--language: "<value>" is not a language: expected a locale code, like fr, en_US or sr@Latn` |
| `--language` sin una de esas dos tareas | `--language is for the tasks join-words and split-words, which were not asked for` |
| un idioma sin diccionario | `no dictionary for <language>` |
| un código mal escrito | `--code: "<code>" is not a pattern code: expected Script[-language[-COUNTRY]], like Zyyy, Latn or Latn-en-US` |
| falta `--max-length` con `line-break` | `--max-length is required by the task line-break: Gaupol's 24 is a width in ems, and has no value in characters` |
| un `--max-length` que no es un número mayor que cero | `--max-length: "<value>" is not a length: expected a number of characters greater than zero` |
| `--max-lines`, `--skip-lines` que no son un número entero de líneas | `--max-lines: "<value>" is not a number of lines: expected a whole number greater than zero` |
| `--skip-length` o `--skip-lines` ilegibles | `--skip-length: "<value>" is not a bound: expected a number greater than zero, or off` |
| una de las cuatro opciones sin `line-break` | `--max-length is for the task line-break, which was not asked for` |
| una clase desconocida | `--classes: "<name>" is not a class: expected human, ocr or human,ocr` |
| `--keep-blank-subtitles` con `--remove-blank-subtitles` | `--keep-blank-subtitles and --remove-blank-subtitles say opposite things; give one of them` |
| un nombre de patrón erróneo | véase la tabla anterior |
| una tarea sin patrón activo | `<task>: no pattern is active under the code <code>; switch one on with --enable` |
| `--range`, `-t`, `--document`, el destino | los de [Invocación](invocation.md#errores) y de [Una traducción](invocation.md#una-traducción) |
