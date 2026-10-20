# Invocación

<!-- machine-translated -->
> Esta página es una traducción automática y todavía no la ha revisado ningún lector del idioma.

```
subedit-cli [global options] <subcommand> [options] <file>...
```

Sin subcomando, la herramienta escribe su ayuda y se detiene.

<!-- example: subedit-cli -->
```console
$ subedit-cli
Read, inspect and retime subtitle files.
Usage: subedit-cli [OPTIONS] [SUBCOMMAND]

Options:
  -h,--help                   Print this help message and exit
  --version                   Display program version information and exit
  -v                          Say more: -v is the default, -vv details, -vvv debugs
  -q,--quiet                  Say nothing but errors
  --encoding NAME             Encoding to read the files in; detected by default
  --format text|json          Form of the result on standard output: text or json
  --config FILE               Read the operation settings from FILE instead of the user's settings file
  --no-config Excludes: --config
                              Ignore any settings file: use the built-in defaults
  --list-encodings            Write the encodings that can be read and written, one a line, and stop

Subcommands:
  inspect                     Report what a subtitle file is made of
  convert                     Write a subtitle file out in another format or shape
  shift                       Move every position of a file by a fixed amount
  transform                   Correct every position from two points known to be right
  framerate                   Re-time a file mastered at one frame rate for another
  snap                        Move every position onto the nearest frame of a frame rate (see framerate)
  hearing-impaired            Remove the sounds described between square brackets
  adjust                      Bring the duration of every subtitle within a reading speed and limits
  replace                     Replace a text in the subtitles, without breaking a tag
  case                        Put the texts in title, sentence, upper or lower case, tags intact
  italics                     Put the texts in italics, or take their italics out
  dialogue-dashes             Put dialogue dashes on the lines, or take them off
  sort                        Put the subtitles in the order of their start, keeping ties as they are
  join-repeats                Join the subtitles that repeat each other, one after the other, into one
  disjoint                    Cut overlapping subtitles so that no range overlaps another, merging what overlaps
  repair                      Join the repeated subtitles, put them in order, then make the ranges disjoint
  correct                     Correct the texts with the patterns of the correction assistant
  append                      Put subtitle files one after another into a single file
  split-file                  Cut a subtitle file in two, a head and a tail
  pair                        Write a translation at the positions of its main file, and say how it lined up
  config                      List, show and change the operation settings
```

## Idioma

La línea de comandos habla el idioma de su entorno: **su ayuda, su narración y sus
errores** siguen el idioma pedido. Los ejemplos de este manual se ejecutan en inglés — los
bloques los genera `make manual`, nunca se copian a mano.

**Cómo se elige el idioma.** La primera de `LC_ALL`, `LC_MESSAGES` y `LANG` que está definida y
no vacía es la configuración regional; `LANGUAGE`, si está definida, enumera entonces los idiomas deseados, separados por
dos puntos (`LANGUAGE=pt_BR:fr`), y se lee primero. Es el orden que sigue gettext, leído por el
propio programa: no hace falta que la configuración regional esté generada en la máquina.

| Regla | Efecto |
| :--- | :----- |
| codificación y modificador | ignorados: `fr_FR.UTF-8@euro` busca `fr_FR` y después `fr` |
| `LC_ALL=C`, configuración regional `C` o `POSIX`, o ninguna configuración regional | inglés, y `LANGUAGE` no se lee |
| una entrada que nombra el inglés (`en`, `en_GB`) | inglés: lo que la sigue en `LANGUAGE` no se lee |
| un idioma cuyo catálogo no está instalado | el siguiente pedido, y el inglés después del último, **sin decir nada** |
| un mensaje que el catálogo no traduce | se muestra en inglés |

**El nombre de un patrón de corrección** que `correct` dice no haber podido aplicar también sigue el
idioma, cuando el catálogo lo conoce — los patrones que se distribuyen con el programa; un patrón propio
se nombra tal como lo escribe su archivo. Los archivos de patrones en sí nunca se reescriben.

Los catálogos se instalan bajo `<prefix>/share/subedit/locale/<language>/LC_MESSAGES/subedit.mo`
y se encuentran a partir del propio directorio del ejecutable. El inglés va integrado. Esta versión instala `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt` y `pt_BR` — catalán,
checo, alemán, español, finés, francés, gallego, hebreo, húngaro, interlingue, islandés, italiano, neerlandés, polaco, portugués y portugués brasileño — traducidos por completo. El francés lo ha revisado
un lector del idioma; el catalán, el checo, el alemán, el español, el finés, el gallego, el hebreo, el húngaro, el interlingue, el islandés, el italiano, el neerlandés, el polaco, el portugués y el portugués brasileño están
traducidos automáticamente y todavía no los ha revisado ningún lector del idioma.
<!-- machine-translated: ca -->
<!-- machine-translated: cs -->
<!-- machine-translated: de -->
<!-- machine-translated: es -->
<!-- machine-translated: fi -->
<!-- machine-translated: gl -->
<!-- machine-translated: he -->
<!-- machine-translated: hu -->
<!-- machine-translated: ie -->
<!-- machine-translated: is -->
<!-- machine-translated: it_IT -->
<!-- machine-translated: nl -->
<!-- machine-translated: pl -->
<!-- machine-translated: pt -->
<!-- machine-translated: pt_BR -->
Con `LANG=fr_FR.UTF-8`, `subedit-cli --help` empieza por «Lire,
inspecter et recaler des fichiers de sous-titres.» Un script que lee las palabras de la salida
de texto define `LANGUAGE=en` (o `LC_ALL=C`) — o lee `--format json`, que nunca cambia.

**Lo que nunca sigue el idioma** — lo que lee un script:

| Nunca traducido | Por qué |
| :--------------- | :-- |
| la salida de `--format json`: claves, valores, y el `message` y el `detail` de los fallos y las advertencias | los mismos bytes en todos los idiomas; las palabras son inglesas |
| los códigos de salida | los cuatro de [Códigos de salida](#códigos-de-salida), en todos los idiomas |
| los nombres que se teclean: subcomandos, opciones, sus valores (`off`, `srt`), metavariables (`FILE`, `TEXT`, `OPTIONS`) | una línea de comandos escrita una vez funciona en cualquier idioma |
| los nombres de los patrones de corrección que aceptan `correct --enable` y `--disable` | un patrón se llama por el nombre inglés que contiene su archivo |
| los nombres de los formatos y de las codificaciones | un formato se llama por su nombre |
| los diagnósticos propios de CLI11 — un argumento obligatorio que falta, una opción desconocida, un valor que una opción no acepta — y su `Run with --help for more information.` | redactados dentro del analizador sobre el que se construye la línea de comandos; se quedan en inglés |

Pedir el inglés, sea cual sea el entorno:

<!-- example: LC_ALL=C LANGUAGE=fr subedit-cli shift --output-dir out a.srt; echo "code=$?" -->
```console
$ LC_ALL=C LANGUAGE=fr subedit-cli shift --output-dir out a.srt; echo "code=$?"
shift needs --by, or --to-grid to work the amount out from the positions
code=1
```

## Opciones globales

**Se escriben antes del nombre del subcomando, y solo ahí.**
`subedit-cli --encoding cp1252 inspect film.srt` es la forma;
`subedit-cli inspect --encoding cp1252 film.srt` se rechaza, con
`The following argument was not expected: --encoding`. Lo mismo vale para
`--version`, `-v` y `--quiet`.

**`--help` es la excepción, y no lo es realmente**:
`subedit-cli inspect --help` se acepta porque cada subcomando lleva la suya, y lo que escribe es la ayuda
del subcomando, no la de la herramienta.

| Opción | Efecto |
| :----- | :---- |
| `-h`, `--help` | escribe la ayuda y se detiene; la del subcomando si se nombra uno |
| `--version` | escribe `subedit <version>`, las bibliotecas con las que funciona la herramienta y los idiomas instalados, y se detiene — véase [`--version`](#--version) |
| `-q`, `--quiet` | nivel 0 — ninguna narración |
| `-v`, `-vv`, `-vvv` | niveles 1 a 3; el nivel 1 es el valor por defecto |
| `--encoding NAME` | lee los archivos en esta codificación, en lugar de adivinarla |
| `--list-encodings` | escribe las codificaciones que la herramienta sabe leer y escribir, una por línea, y **se detiene** (código `0`) — véase [`--list-encodings`](#--list-encodings) |
| `--format text\|json` | la forma del resultado en la salida estándar: el texto, o un objeto JSON por archivo — véase [Una salida que un script puede leer](#una-salida-que-un-script-puede-leer) |
| `--config FILE` | lee la configuración de las operaciones de `FILE` en lugar del archivo de configuración del usuario — véase [El archivo de configuración](#el-archivo-de-configuración) |
| `--no-config` | no lee ningún archivo de configuración: cada ajuste de operación no dado en la línea toma su valor por defecto integrado — véase [El archivo de configuración](#el-archivo-de-configuración) |

`--quiet` y `-v` en la misma invocación se rechazan: dos intenciones opuestas
no se resuelven a favor de la última escrita.

`--config` y `--no-config` en la misma invocación también se rechazan, con
`--config excludes --no-config`, y lo mismo `--config` con un nombre vacío, con
`--config needs a file name`: un nombre vacío no nombra ningún archivo, y el archivo propio del usuario no se lee en
su lugar.

### El archivo de configuración

**[`adjust`](adjust.md) y [`correct`](correct.md) leen por defecto la configuración de las operaciones del
archivo de configuración** — los valores que `Adjust Durations…` y `Correct Texts…` recuerdan en
la ventana, o que escribió [`config set`](config.md). Ningún otro subcomando lo lee. El archivo es:

```
$XDG_CONFIG_HOME/subedit/settings.conf
```

o `~/.config/subedit/settings.conf` cuando `XDG_CONFIG_HOME` no está definida, está vacía o no es una ruta
absoluta — el archivo de la ventana, descrito en [su manual](../subedit-gui/preferences.md#el-archivo).

**Prioridad, de la más fuerte a la más débil: la opción en la línea de comandos, luego el archivo, luego el valor por
defecto integrado.** Una opción siempre gana: `--speed off` desactiva una velocidad que fija el archivo. Solo se lee el
**bloque de las operaciones** del archivo; las preferencias de visualización (geometría, columnas, tema…)
nunca. [`config list`](config.md#config-list) nombra las claves, y cuáles de ellas lee la línea
de comandos: las nueve claves `duration-adjust.*`, `correction.human`, `correction.ocr`,
`correction.remove-blank` y `correction.line-break.max-lines`. **Cada una tiene una opción
que la anula durante una invocación**, sea cual sea el valor que le da el archivo — las tablas de
[`adjust`](adjust.md#lo-que-aporta-el-archivo-de-configuración) y
[`correct`](correct.md#qué-se-elige-y-qué-no) las nombran.

**Leer el archivo del usuario nunca hace fallar un comando.** Un archivo que falta da los valores por defecto; un archivo
que no se puede leer da los valores por defecto y lo dice; un valor que no se puede leer conserva su valor por defecto y se nombra; una
clave desconocida se ignora. `adjust` y `correct` nunca escriben el archivo.

**Lo que se ha leído se indica**, en el nivel por defecto 1, en la salida de error estándar, antes del primer archivo:

| Caso | Línea |
| :--- | :--- |
| el archivo se ha leído | `settings: read <path>` |
| un valor del archivo no se ha podido leer | `settings: <key>: cannot read '<value>', default kept` |
| el archivo del usuario no existe | `settings: <path> not found, defaults used` |
| el archivo no se puede leer | `settings: <path> unreadable, defaults used` |
| `--no-config` | nada |

Con [`--format json`](#una-salida-que-un-script-puede-leer), el objeto de cada archivo de `adjust` y
`correct` lleva `settings`: la ruta que se ha leído, o `null` cuando no se ha leído ninguna.


**`--no-config` hace reproducible una ejecución**: la misma línea da el mismo resultado en todas las
máquinas, sea lo que sea lo que su usuario haya fijado en la ventana. Conviene usarla en scripts, pruebas e informes de errores — o
`--config FILE` con un archivo que los acompañe.

**`--config FILE` nombra otro archivo**, leído de la misma manera — **salvo que debe existir**. Un
nombre dado en la línea de comandos no es una petición de los valores por defecto: cuando `FILE` no existe,
`adjust` y `correct` se detienen antes de leer ninguna entrada, escriben `settings file not found: <FILE>` en
la salida de error estándar, y terminan con el código de uso `1`. Un `FILE` que existe y no se puede leer
da los valores por defecto y lo dice, como el archivo del usuario. [`config show`](config.md#config-show)
sigue mostrando un `FILE` que falta como `(not found)`, y [`config set`](config.md#config-set-y-config-unset) sigue
creándolo.

<!-- example: printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine.conf adjust --dry-run a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine.conf adjust --dry-run a.srt; echo "code=$?"
settings file not found: mine.conf
code=1
```

### `--version`

**La versión del programa, sola en la primera línea** — `subedit <version>`, lo que era la salida hasta
la 0.15.38 y lo que lee un script. Después, **una línea por cada biblioteca con la que funciona la herramienta**, y luego los idiomas
de la interfaz instalados, en la salida estándar; la herramienta se detiene, con el código `0`. Un subcomando dado además
no se ejecuta, y `-q` y `-v` no cambian nada.

<!-- example: subedit-cli --version | head -n 1 -->
```console
$ subedit-cli --version | head -n 1
subedit 0.15.45
```

Completa, en una máquina con ICU 74.2 y Enchant 2.3.3 — los números son los de la máquina:

```console
$ subedit-cli --version
subedit <version>
  ICU      74.2 (built with 74.2)
  Enchant  2.3.3 (built with 2.3.3)
  CLI11    2.4.1 (built with 2.4.1)
  Languages: en, fr
```

| Biblioteca | Para qué la usa la herramienta |
| :------ | :------------------------ |
| `ICU` | leer y escribir las codificaciones, detectarlas, cambiar la capitalización de las letras |
| `Enchant` | el corrector ortográfico de `correct` |
| `CLI11` | leer esta línea de comandos |

- **Cada línea da dos versiones**: la encontrada en tiempo de ejecución, primero, y aquella contra la que se
  compiló el programa, entre paréntesis. Difieren cuando la biblioteca de la máquina se actualizó después de
  compilar el programa — lo primero que hay que dar en un informe de error.
- **Las cabeceras de Enchant no llevan versión**: la que va entre paréntesis es la que indicó `pkg-config`
  cuando se configuró la compilación.
- **CLI11 se compila dentro del programa**, ya que consiste solo en sus cabeceras: sus dos versiones son
  siempre la misma.
- **Ni Qt ni libmpv**: la línea de comandos no enlaza ninguna de las dos, y solo enumera aquello con lo que funciona.
  La ventana sí las enumera — véase [su manual](../subedit-gui/invocation.md#--version).
- **`Languages`** enumera `en`, el idioma de origen, que no necesita catálogo, y después cada catálogo
  instalado bajo `<prefix>/share/subedit/locale` que se puede leer, por código — los idiomas que
  ofrece `Editar ▸ Preferencias…`.
- **Lo que sigue el idioma**: `built with` y `Languages`. La primera línea, los nombres, los
  números y los códigos de idioma nunca lo siguen.
- **Con `--format json`**, antes o después de `--version`, **un objeto en una línea**: `schema`
  (`1`), `command` (`"version"`), `version`, `libraries` — una matriz de
  `{"name":"ICU","loaded":"74.2","built":"74.2"}`, en el orden del texto — y `languages`, una
  matriz de códigos. Nada en él se traduce.

<!-- example: subedit-cli --version --format json | grep -o '"name":"[^"]*"' -->
```console
$ subedit-cli --version --format json | grep -o '"name":"[^"]*"'
"name":"ICU"
"name":"Enchant"
"name":"CLI11"
```

### `--list-encodings`

**Las codificaciones que la herramienta sabe leer y escribir, una por línea, ordenadas por nombre**, en la salida estándar —
después la herramienta se detiene, con el código `0`, como hace `--version`: es la respuesta, no una etapa. Un
subcomando dado además no se ejecuta.

<!-- example: subedit-cli --list-encodings | grep '^UTF' | head -4 -->
```console
$ subedit-cli --list-encodings | grep '^UTF' | head -4
UTF-16BE
UTF-16LE
UTF-32BE
UTF-32LE
```

- **Son los nombres que acepta `--encoding`**, los canónicos; sus alias (`cp1252`) también se aceptan,
  sin figurar en la lista.
- **El número depende de la ICU instalada**, y no se promete ningún nombre más allá de los que tiene toda ICU
  (`UTF-8`, `UTF-16LE`, `windows-1252`…). Por eso el manual no da ningún número.
- **`UTF-16` y `UTF-32` no figuran**: ICU los conoce, pero su convertidor escribe una marca propia,
  y el modelo los rechaza — véase más abajo. Una lista entre la que elegir no ofrece lo que el campo siguiente
  rechazaría.
- **Con `--format json`**, **un objeto por codificación**:
  `{"schema":1,"command":"list-encodings","name":"UTF-8"}`. No es el informe de un archivo: estos
  objetos **no llevan ni `file` ni `ok`**, la única excepción al envoltorio de
  [la forma](#la-forma). Siguen siendo un objeto por línea, uno por entrada.
- **El nivel de narración no cambia nada**: una lista es un resultado, y `-q` no la suprime.

### `--encoding`, y lo que no puede forzar

**Se aplica a todos los subcomandos**, porque todos leen: un archivo cuya
codificación se adivina mal sigue estando mal se le haga lo que se le haga, y una opción
que solo llevara `inspect` dejaría un desajuste sin remedio.

**El nombre es el de ICU, y se aceptan todos sus alias**: `cp1252`,
`windows-1252` y `WINDOWS 1252` nombran la misma codificación. El conjunto no está escrito
en ninguna parte: es el que la ICU de la máquina sabe convertir, y
[`--list-encodings`](#--list-encodings) lo enumera. Un nombre que no sabe convertir se
rechaza antes de leer un solo archivo:

```console
$ subedit-cli --encoding klingon-1 inspect film.srt
no encoding is named "klingon-1"
```

**Una excepción, y solo una: la codificación tiene que nombrar su orden de bytes.**
`UTF-16` y `UTF-32` son nombres que ICU conoce y cuyo convertidor escribe **una marca
propia**. Una codificación así no puede vivir en este modelo — la marca deja
de ser una elección, `--no-bom` se aceptaría y se desobedecería — así que ambos se
rechazan, tanto para leer como para escribir:

```console
$ subedit-cli --encoding UTF-16 inspect film.srt
"UTF-16" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do
```

`UTF-16LE` y `UTF-16BE` leen y escriben los mismos bytes, marca incluida; lo que
añade el nombre preciso es que se sabe cuál.

**Una marca de orden de bytes gana a la opción.** Es lo único que un archivo de subtítulos
declara sobre sí mismo, y leerlo de otra manera que como se declara
sería obedecer a quien llama en contra del archivo. El desajuste **se indica**, como
diagnóstico:

```console
$ subedit-cli -vvv --encoding cp1252 inspect avec-bom.srt
avec-bom.srt: a byte order mark that contradicts the encoding asked for ("UTF-16LE"), settled by the reader
```

Es lo que hace Gaupol — vuelve a empezar la lectura con la codificación de
la marca — pero sin decirlo; aquí la diferencia entre lo que se pidió y
lo que se hizo no es un silencio.

## Subcomandos

| Subcomando | Qué hace | ¿Escribe? |
| :------------ | :-------------- | :----------- |
| [`inspect`](inspect.md) | informa de qué está hecho un archivo, sin cambiar nada | no |
| [`convert`](convert.md) | escribe un archivo en otro formato, o con otra forma | sí |
| [`shift`](shift.md) | mueve todas las posiciones la misma cantidad | sí |
| [`transform`](transform.md) | corrige todas las posiciones a partir de dos puntos que se saben correctos | sí |
| [`framerate`](framerate.md) | reajusta los tiempos de un archivo de una velocidad de fotogramas a otra | sí |
| [`snap`](snap.md) | mueve cada marca de tiempo al fotograma más cercano | sí |
| [`hearing-impaired`](hearing-impaired.md) | elimina las anotaciones para sordos | sí |
| [`adjust`](adjust.md) | ajusta las duraciones a una velocidad de lectura y a unos límites | sí |
| [`replace`](replace.md) | reemplaza un texto en los subtítulos, sin romper una etiqueta | sí |
| [`case`](case.md) | pone los textos en tipo título, tipo oración, mayúsculas o minúsculas | sí |
| [`italics`](italics.md) | pone los textos en cursiva, o les quita la cursiva | sí |
| [`dialogue-dashes`](dialogue-dashes.md) | pone las rayas de diálogo, o las quita | sí |
| [`sort`](sort.md) | vuelve a poner los subtítulos en el orden de su inicio | sí |
| [`join-repeats`](join-repeats.md) | hace un solo subtítulo con los trozos de un mismo texto que se suceden | sí |
| [`disjoint`](disjoint.md) | recorta los subtítulos solapados para que ningún intervalo se solape ya con otro | sí |
| [`repair`](repair.md) | encadena `join-repeats`, `sort` y `disjoint` | sí |
| [`correct`](correct.md) | corrige los textos con los patrones de Gaupol: anotaciones, unir y separar palabras, errores comunes, capitalización, salto de línea | sí |
| [`append`](append.md) | pone archivos tras el primero, en **un único** archivo | sí |
| [`split-file`](split-file.md) | corta un archivo en **dos**: una cabeza y una cola | sí |
| [`pair`](pair.md) | escribe una traducción en las posiciones de su archivo principal, e indica cómo se emparejaron sus líneas | sí |
| [`config`](config.md) | enumera, muestra y cambia la configuración de las operaciones del archivo de configuración | solo el archivo de configuración |

Están los veintiuno; la ayuda de la herramienta los enumera en el mismo orden.

**Solo uno no toca nada**, y es la columna de la derecha la que lo dice: `inspect` lee e
informa. Los otros diecinueve escriben, y lo que sigue vale para diecisiete de ellos:
[`append`](append.md) y [`split-file`](split-file.md) tienen cada uno una aridad propia — N archivos para
**un** resultado, un archivo para **dos** — y por tanto su propio destino, que describen en su página.
[`config`](config.md) no escribe ningún archivo de subtítulos: lee y cambia el archivo de configuración, y
nada de lo que sigue se le aplica.

## El destino

Los diecisiete subcomandos que escriben — [`convert`](convert.md),
[`shift`](shift.md), [`transform`](transform.md), [`framerate`](framerate.md),
[`snap`](snap.md), [`hearing-impaired`](hearing-impaired.md), [`adjust`](adjust.md),
[`replace`](replace.md), [`case`](case.md), [`italics`](italics.md),
[`dialogue-dashes`](dialogue-dashes.md), [`sort`](sort.md), [`join-repeats`](join-repeats.md),
[`disjoint`](disjoint.md), [`repair`](repair.md), [`correct`](correct.md), [`pair`](pair.md) — toman su destino de la misma manera. ([`append`](append.md) y
[`split-file`](split-file.md) tienen el suyo, en su página.) [`inspect`](inspect.md) no escribe ningún archivo y
no acepta ninguna de estas opciones.

**Un destino que ya existe se sobrescribe**, sin pregunta y sin opción para
evitarlo: la escritura es atómica — el archivo nunca queda escrito a medias, y una
escritura interrumpida deja intacto el antiguo —, pero el contenido anterior se
pierde. No es un defecto que se conserva: es la regla, y volver a ejecutar un comando
sobre el mismo directorio de salida lo vuelve a ejecutar sobre sí mismo.

**Nunca se escribe nada sin un destino explícito.** Las tres maneras de
darlo se excluyen entre sí, y hace falta una: sin ella, no se escribe nada y el código
de salida es `1` — salvo con [`--dry-run`](#ver-antes-de-escribir), que no escribe nada
y por tanto no exige ninguno.

| Opción | Adónde va el resultado |
| :----- | :---------------- |
| `--output FILE` | a este archivo, con este nombre exacto; una sola entrada |
| `--output-dir DIR` | a este directorio, con el nombre base de la entrada |
| `--in-place` | sobre la entrada, por escritura atómica |

Una herramienta que sobrescribe su entrada porque no se le dijo nada es una herramienta que se
deja de usar la segunda vez. La negativa cuesta una línea y salva el
archivo.

`--output` con varias entradas se rechaza: el último archivo se escribiría sobre
los anteriores. Con un lote, `--output-dir` es la única de las tres que tiene
sentido, junto con `--in-place`.

**El directorio de salida se crea** si falta, padres incluidos — para
`--output-dir`, y para el directorio en el que escribe `--output`. Se hace una vez
validada la línea de comandos y **antes del primer archivo**. Si no se puede,
se dice una sola vez, no se lee ningún archivo, y el código de salida es
`2`.

**El lote se juzga como un todo antes de escribir.** Primero se calcula el destino de cada entrada,
y de ahí se siguen dos negativas — errores de uso, código
`1`, **ningún archivo escrito** y ningún directorio creado:

- **dos entradas que acabarían en el mismo archivo** — `a/film.srt` y
  `b/film.srt` con `--output-dir`, o `a/film.srt` y `b/film.vtt` convertidos ambos
  a WebVTT, cuyo destino final cambió de extensión. El
  mensaje nombra el destino y las dos entradas; renombrar la segunda o
  dejar que gane la última perdería un archivo sin decirlo, y negarse
  cuesta una línea;
- **un destino que es una de las entradas**, sin `--in-place` — también
  mediante `--output-dir .`, mediante `--output` o mediante un enlace simbólico. Escribir sobre
  la propia entrada es un gesto que se nombra.

Dos rutas nombran el mismo archivo cuando el sistema lo dice, no cuando se
escriben igual: `in/../in/film.srt` e `in/film.srt` son un único archivo.

**La extensión sigue al formato escrito.** Solo cambia con
[`convert`](convert.md), el único subcomando que cambia de formato; los
demás conservan el formato del archivo leído, y por tanto su extensión.

| Lo que lo provoca | Mensaje |
| :------------------ | :------ |
| ningún destino, sin `--dry-run` | `no destination given: use --output, --output-dir or --in-place` |
| dos destinos | `--output, --output-dir and --in-place exclude one another` |
| `--output` sobre un lote | `--output names one file but several were given: use --output-dir instead` |
| dos entradas, un destino | `<destination>: would be written by both <input> and <input>` |
| una entrada se sobrescribiría | `<destination>: would be written over the input <input>` — la misma frase para [`append`](append.md) y [`split-file`](split-file.md) |
| directorio de salida que no se puede crear | `<directory>: cannot be created: permission denied`, o `: cannot be created` |
| destino que no se puede escribir | `<path>: <destination>: cannot be written: permission denied`, o `: cannot be written` |
| carácter que falta en la codificación escrita | `<path>: holds a character the chosen encoding cannot write` |

**El último merece una frase.** Un archivo se vuelve a escribir en **la codificación en la que
se leyó**, igual que con sus finales de línea y su marca. Si un
cambio le introdujo un carácter que esa codificación no conoce — una
`ł` en Latin-1 —, la escritura falla y **no se escribe nada**. Reemplazarlo
por un `?` sería perder texto ante los ojos de quien acaba de escribirlo.

## Ver antes de escribir

**`--dry-run`** lo acepta cada uno de los dieciséis subcomandos que escriben, y significa lo mismo
en todas partes: **leer, calcular, informar, no escribir ningún archivo**. Se añade a la
línea que se iba a ejecutar; aplicar es volver a ejecutar la misma línea sin ella. El cálculo es
determinista: lo que mostró un `--dry-run` es lo que escribirá la ejecución siguiente, mientras el
archivo no haya cambiado.

- **No escribe ningún archivo ni crea ningún directorio**, `--in-place` incluido: el archivo
  de entrada se queda como estaba.
- **No exige ningún destino.** Se puede dar uno, y **entonces se comprueba como sin
  `--dry-run`**: dos entradas para el mismo destino, una entrada que se sobrescribiría,
  `--output` sobre un lote, dos maneras de decir dónde — una línea de comandos rechazada sin `--dry-run`
  se rechaza con ella, con la misma frase y el mismo código `1`.
- **El código de salida es el de una ejecución real**: `0` cuando se ha calculado todo, `2` o `3`
  cuando hay archivos que no se han podido leer o procesar, `1` para el uso. Un carácter que
  la codificación del archivo no podía escribir hace fallar una simulación igual que una ejecución real.
  **Que haya cambios no cambia el código**: los cuatro códigos dicen si la herramienta
  tuvo éxito, no si el archivo es conforme.
- **La narración lo dice.** Donde una ejecución real escribe `-> <destination>`, una
  simulación escribe `(dry run, nothing written)`.

<!-- example: printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by 1 --dry-run a.srt; ls -- *.srt -->
```console
$ printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by 1 --dry-run a.srt; ls -- *.srt
a.srt: 1 subtitle shifted by 1.000 s (dry run, nothing written)
a.srt
```

**En un subcomando que cambia textos** —
[`hearing-impaired`](hearing-impaired.md), [`replace`](replace.md), [`case`](case.md),
[`italics`](italics.md), [`dialogue-dashes`](dialogue-dashes.md) y [`correct`](correct.md) —, **la salida estándar lleva los cambios
propuestos**. Un bloque por subtítulo cambiado, en el orden del archivo:

```
<path>: subtitle <number>
- <the text before, line by line>
+ <the text after, line by line>
```

- el **número** es el que tiene el subtítulo en el archivo leído, contando desde uno;
- cada línea del texto anterior lleva `- ` delante, cada línea del texto posterior `+ `: un texto
  de varias líneas se lee tal cual, y una línea que empieza ella misma por una raya de
  diálogo da `- - `;
- **un subtítulo que la operación eliminaría** lo dice en su primera línea,
  `<path>: subtitle <number> (removed)`, y no tiene líneas `+`;
- un subtítulo de la traducción, con `--document translation`, se anuncia `(translation)` en el mismo
  lugar — véase [Una traducción](#una-traducción);
- la primera línea de cada bloque empieza por la ruta, de modo que `grep '^film.srt: '`
  encuentra los bloques de un archivo dentro de un lote;
- **ningún bloque** cuando nada cambiaría.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\n- [Grincement]\n- Qui est là ?\n\n' > mentions.srt; subedit-cli hearing-impaired --dry-run mentions.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\n- [Grincement]\n- Qui est là ?\n\n' > mentions.srt; subedit-cli hearing-impaired --dry-run mentions.srt
mentions.srt: 2 subtitles cleaned, 1 removed (dry run, nothing written)
mentions.srt: subtitle 1 (removed)
- [Bruit de pas]
mentions.srt: subtitle 2
- Attends [il tousse] Marie.
+ Attends Marie.
mentions.srt: subtitle 3
- - [Grincement]
- - Qui est là ?
+ Qui est là ?
```

**La lista es la misma en JSON**: el objeto del archivo lleva una matriz `changes`, véase
[la forma](#la-forma), con `after` a `null` para un subtítulo eliminado. **Solo se
calcula cuando `--dry-run` o `--format json` la piden**: un lote de miles de
subtítulos no la construye para nada.

En **los demás subcomandos** — [`convert`](convert.md), [`shift`](shift.md),
[`transform`](transform.md), [`framerate`](framerate.md), [`snap`](snap.md),
[`adjust`](adjust.md), [`sort`](sort.md), [`append`](append.md), [`split-file`](split-file.md), [`pair`](pair.md) —, la salida
estándar lleva lo que llevaría la de una ejecución real, es decir, nada en texto; en
JSON, el objeto de cada archivo con `"dry_run":true`, `"destination":null` y los mismos
`counts`.

## Dos salidas, dos papeles

| Salida | Lo que lleva |
| :----- | :--------------- |
| estándar | **el resultado, y solo el resultado** — el informe de `inspect`, los cambios propuestos de un [`--dry-run`](#ver-antes-de-escribir) de texto; con `--format json`, un objeto por archivo — uno por ejecución para [`append`](append.md), [`split-file`](split-file.md) y [`pair`](pair.md) |
| error | **todo lo demás** — la narración, las advertencias, los errores |

Es esta separación la que permite redirigir el resultado sin que se mezcle con el relato:

```console
$ subedit-cli inspect *.srt > rapport.txt
```

El informe va al archivo, la narración se queda en la pantalla.

## Una salida que un script puede leer

**`--format json`** reemplaza el texto de la salida estándar por **un objeto JSON por
archivo de entrada**, en el formato JSON Lines: un objeto compacto por línea, en UTF-8 sin
marca, y el final de línea es `\n`. `text` es el valor por defecto y no cambia. Es una
opción global, y por tanto se escribe antes del subcomando; un valor distinto de
`text` y `json` es un error de uso, código `1`.

<!-- example: printf '1\n00:00:10,000 --> 00:00:12,000\nUn.\n\n' > a.srt; subedit-cli --format json shift --by 1 --output-dir sortie a.srt absent.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:10,000 --> 00:00:12,000\nUn.\n\n' > a.srt; subedit-cli --format json shift --by 1 --output-dir sortie a.srt absent.srt; echo "code=$?"
a.srt: 1 subtitle shifted by 1.000 s -> sortie/a.srt
{"schema":1,"command":"shift","file":"a.srt","ok":true,"dry_run":false,"destination":"sortie/a.srt","counts":{"subtitles":1,"shifted_by_ms":1000},"warnings":[]}
absent.srt: does not exist
{"schema":1,"command":"shift","file":"absent.srt","ok":false,"error":{"kind":"not-found","message":"absent.srt: does not exist"}}
1 of 2 files shifted, 1 failed
code=3
```

**Un objeto por entrada, nunca cero, nunca dos**, haya tenido éxito la entrada o no, en
el orden en que se dieron las entradas — o se encontraron, con [`--recursive`](lots.md). Se
cuentan las líneas, se encuentran los propios archivos. **Tres subcomandos son la excepción, porque su
aridad no es la de un lote: [`append`](append.md) (N entradas, una salida), [`split-file`](split-file.md)
(una entrada, dos salidas) y [`pair`](pair.md) (dos archivos, una salida) escriben **un objeto para toda la
ejecución**, que nombra el archivo que indica su página.** **Los subcomandos que reescriben escriben por tanto también
en la salida estándar**, lo que su contrato no hacía en texto: su
archivo sigue siendo su resultado, y el objeto es el informe sobre él.

**La narración no cambia.** `-q`, `-v`, `-vv` y `-vvv` actúan sobre la salida de
error, en texto, como sin `--format`; **la salida estándar no varía con ellos**.
Los diagnósticos de lectura, que el texto reserva al nivel 3, son datos y están
**siempre** en `warnings`. Un error de uso no escribe ningún objeto: se detecta
antes de tocar ningún archivo, así que no hay nada a lo que asociarlo; va a la salida
de error con el código `1`, y la salida estándar se queda vacía.

### La forma

| Clave | Contenido | Presente |
| :-- | :------ | :------- |
| `schema` | entero, la versión de esta forma — `1` | siempre |
| `command` | el nombre del subcomando | siempre |
| `file` | la ruta **tal como se dio**, o tal como la compuso el recorrido de un directorio | siempre |
| `ok` | booleano | siempre |
| `error` | `{"kind", "message"}` | si `ok` es falso |
| `dry_run` | booleano: verdadero para un [`--dry-run`](#ver-antes-de-escribir) | si `ok` es verdadero, en un subcomando que escribe |
| `destination` | la ruta escrita, o `null` cuando `dry_run` es verdadero: no se ha escrito nada | igual |
| `counts` | un objeto de **enteros**, propio del subcomando | igual |
| `changes` | una matriz de `{"subtitle", "document", "before", "after"}`: los textos cambiados, véase más abajo | en un subcomando que cambia textos |
| `warnings` | una matriz de `{"kind", "line"?, "detail"?, "settled"?}` | si `ok` es verdadero |
| `alignment` | cómo se colocó una traducción, véase [Una traducción](#una-traducción) | con `-t`, y para `pair` |
| `constraints` | los ajustes utilizados | `adjust` |
| `settings` | el archivo de configuración que se leyó, o `null` cuando no se leyó ninguno — véase [El archivo de configuración](#el-archivo-de-configuración) | `adjust`, `correct`, si `ok` es verdadero |
| `inputs`, `tail` | un elemento por archivo añadido; la ruta de la cola | `append`; `split-file` |

`line` falta en un diagnóstico sobre el archivo entero; `detail` falta cuando no hay
nada que añadir; `settled` es `true` cuando la lectura **resolvió** algo — una
numeración regenerada —, y falta cuando dejó las cosas como estaban.

**`changes`** enumera, en el orden del archivo, cada subtítulo cuyo texto cambia:
`subtitle` es su número en el archivo leído, contando desde uno; `document` es `"main"`
o `"translation"`; `before` es el texto anterior; `after` el texto posterior, o **`null` cuando
el subtítulo se elimina**. Los saltos de línea de un texto son `\n` dentro de él. Se
escribe tanto en una ejecución real como en un [`--dry-run`](#ver-antes-de-escribir), y está **vacía
(`[]`) cuando nada cambia**: su presencia dice que el subcomando enumera sus cambios,
su ausencia, que no tiene.

**Ningún número de coma flotante, en ninguna parte.** Una posición o una duración es un entero de
**milisegundos** (`…_ms`); un recuento es un entero; una velocidad de fotogramas es una **cadena**
(`"25"`, `"24000/1001"`) porque es exacta, algo que una coma flotante racional no puede
contener; una concentración de rejilla es un entero en **milésimas** — `1000` es una
rejilla perfecta. Dos salidas se comparan entonces byte a byte, sin redondeo.

**Una ruta que no es UTF-8** — Linux lo permite — no se puede representar en JSON:
se escribe con U+FFFD en lugar de los bytes no válidos, y el objeto lleva la advertencia
`path-not-utf8`. No se promete la fidelidad de una ruta así, y se dice.

### Lo que cada subcomando pone en `counts`

| Subcomando | Claves de `counts` |
| :------------ | :--------------- |
| [`shift`](shift.md) | `subtitles`, `shifted_by_ms` (con signo) |
| [`transform`](transform.md) | `subtitles` |
| [`framerate`](framerate.md) | `subtitles` |
| [`snap`](snap.md) | `subtitles`, `moved` (posiciones movidas), `furthest_ms` (el mayor movimiento) |
| [`hearing-impaired`](hearing-impaired.md) | `cleaned` (textos reescritos), `removed` (subtítulos eliminados) |
| [`replace`](replace.md) | `replaced` (coincidencias reemplazadas en los subtítulos cambiados), `matched` (coincidencias encontradas) |
| [`case`](case.md), [`italics`](italics.md), [`dialogue-dashes`](dialogue-dashes.md) | `changed` (subtítulos cuyo texto cambió) |
| [`correct`](correct.md) | `corrected` (textos cambiados), `removed` (subtítulos eliminados) — nunca coincidencias |
| [`sort`](sort.md) | `subtitles`, `moved` (los lugares que cambiaron de subtítulo) |
| [`adjust`](adjust.md) | `subtitles` (los subtítulos a los que se apunta), `adjusted` (aquellos cuyo fin se movió), después el grupo `sacrificed`: `speed`, `minimum`, `gap`; y, fuera de `counts`, `constraints` — los ajustes utilizados |
| [`convert`](convert.md) | `subtitles`, después lo que perdió la conversión: `lost_ends` y `lost_header` (0 o 1), `joined_lines`, `lost_tags`, `lost_fields`, `furthest_ms` |
| [`append`](append.md) | `files`, `subtitles`, `appended`, después `lost_ends`, `lost_header`, `joined_lines`, `lost_tags`, `lost_fields`, `furthest_ms` sumados; y, fuera de `counts`, `inputs` — un elemento por archivo añadido |
| [`split-file`](split-file.md) | `subtitles`, `head`, `tail`; y, fuera de `counts`, `tail` — la ruta de la cola (`destination` es la cabeza) |
| [`pair`](pair.md) | `subtitles` (los del archivo escrito); y, fuera de `counts`, `alignment` |
| [`inspect`](inspect.md) | ningún `counts`: la descripción del archivo, véase su página |

### Identificadores de error

Un `error.kind` es uno de estos; **un lector trata un identificador desconocido como
«otro»**.

| `kind` | Qué es |
| :----- | :----------- |
| `not-found`, `permission-denied`, `io` | el sistema se negó a leer — o a escribir — el archivo |
| `unknown-format`, `no-subtitle-found`, `undecodable` | el archivo se lee y no se deja leer |
| `unencodable` | un carácter que la codificación elegida no puede escribir |
| `no-byte-order-mark`, `no-frame-rate` | lo que `convert` no puede escribir sin que se le diga |
| `before-the-origin`, `beyond-the-end`, `no-transform`, `no-grid` | una operación que no puede aplicarse a este archivo |
| `range-out-of-bounds` | un `--range` que este archivo no contiene |
| `no-style` | un formato que no escribe ningún estilo (TMPlayer, LRC), para `italics` |
| `refused` | cualquier otra negativa de una operación |

### Lo que se promete, y lo que no

**Prometido mientras `schema` sea `1`**: los nombres de las claves, su tipo y su unidad; el significado
de `ok`, `file` y `destination`; **un objeto por entrada, en el orden de las entradas** — uno por ejecución para los tres subcomandos con aridad propia;
los identificadores `kind` que existen; el determinismo — mismas entradas, mismos argumentos,
mismos bytes.

**No prometido**: el **texto de los mensajes** (`message`, `detail`), inglés para un humano
que se corrige cuando hace falta — un script que lo compara está roto de antemano; el **orden de
las claves** en un objeto, fijo pero no garantizado por JSON; la presencia de **claves aún no
añadidas**.

**Un lector ignora por tanto una clave desconocida**, y trata un `kind` desconocido como «otro»:
añadir una clave o un identificador no cambia `schema`. Eliminar o renombrar una clave,
cambiar un tipo o una unidad, eliminar un identificador lo incrementa — eso es una ruptura,
anunciada en el CHANGELOG y aquí.

## Niveles de narración

Cada nivel **contiene el anterior**: subir un escalón añade, nunca
reemplaza.

| Nivel | Cómo | Lo que lleva la salida de error |
| :----- | :------ | :------------------------------ |
| 0 | `--quiet` | nada, **salvo los errores** |
| 1 | por defecto, o `-v` | una línea por archivo procesado, y un resumen en cuanto hay varios |
| 2 | `-vv` | y lo que se ha reconocido: formato, codificación, BOM, finales de línea |
| 3 | `-vvv` | y la traza de depuración: bytes leídos y escritos, y **cada diagnóstico de lectura** |

**Los errores nunca se silencian, `--quiet` incluido.** Un comando que fallara
en silencio no dejaría más que su código de salida.

El resumen solo aparece a partir de dos archivos: con una sola entrada,
repetiría la línea anterior.

## Diagnósticos de lectura

Los formatos de subtítulos se leen **lo mejor posible**: ante una anomalía, el
lector no se detiene — decide, o deja las cosas como están, y lo dice. Lo que
se ha encontrado se emite en el **nivel 3**, una línea por diagnóstico, en la salida
de error:

```
a.srt: 2 diagnostics while reading
a.srt: line 6: SubRip numbers that do not follow ("7"), settled by the reader
a.srt: line 9: a line that fits nowhere, left as it stands
```

Los diecisiete subcomandos los indican, no solo [`inspect`](inspect.md):
un archivo leído lo mejor posible y luego vuelto a escribir ha pasado por las mismas decisiones, y callarlas
haría creer que no ha pasado nada.

**Un diagnóstico nunca es un fallo.** El archivo se ha leído, el comando
ha tenido éxito, el código de salida es `0`. Por eso viven en el nivel más
detallado: más abajo, enterrarían la línea que dice lo que realmente se
ha hecho.

### Lo que lleva cada línea

| Parte | Lo que dice |
| :----- | :------------- |
| `line N` | dónde, contando desde 1, como lo muestra un editor; **falta** para lo que concierne al archivo entero |
| la frase | lo que se ha encontrado, entre las doce categorías de más abajo |
| `("…")` | el texto defectuoso del archivo, cuando la categoría no basta; truncado a 80 bytes |
| el final | lo que se ha hecho al respecto: `settled by the reader`, o `left as it stands` |

**Dos categorías no tienen línea**, y son las dos que tratan de la
codificación: los bytes se sopesaron, o se leyó su marca, antes de que existiera una sola
línea del archivo. Su línea empieza por tanto por la frase, sin `line N`
delante — no hay ningún lugar que nombrar.

**El final de la línea es lo que más importa.** `settled by the reader` significa
que el lector decidió y que el archivo escrito lleva su decisión — una
numeración que falta se regenera. `left as it stands` significa que no tocó
nada porque **solo el usuario puede decidir**: un subtítulo que termina antes de
empezar se queda como está.

### El número de línea de un bloque

Una anomalía que afecta a **un bloque entero** se ancla en su línea
de tiempos, y no en la línea exacta que la provoca. Una numeración incoherente
escrita en la línea 5 se indica por tanto en la línea 6, la de la línea de tiempos que
sigue — el número defectuoso está en el `("…")`. Solo las anomalías que afectan a
**una línea** — una línea de tiempos ilegible, texto antes de la primera — se
indican sobre sí mismas.

### Las doce categorías

| Frase | Lo que la provoca |
| :----- | :------------------ |
| `a line that fits nowhere` | una línea que no entra en ningún bloque |
| `a timing line that could not be read` | una línea de tiempos ilegible |
| `a SubRip block without its number` | numeración que falta, regenerada al escribir |
| `SubRip numbers that do not follow` | numeración que salta |
| `text before the first timing line` | texto antes de la primera línea de tiempos |
| `a WebVTT block of an unknown kind` | un bloque WebVTT no reconocido |
| `declares an event column this tool cannot fill` | una columna de eventos de Sub Station Alpha que nada rellena aquí |
| `counts in frames and states no rate; it was read at` | un archivo en fotogramas que no indica ninguna velocidad de fotogramas — la utilizada es el detalle |
| `carries no end times; each one was taken from the next start` | un formato sin fines, TMPlayer o LRC: **ningún fin de la tabla procede del archivo** |
| `more than one kind of line ending` | finales de línea mezclados |
| `an encoding nothing declared` | la codificación se propuso sopesando los bytes, y no es UTF-8 |
| `a byte order mark that contradicts the encoding asked for` | el archivo lleva una marca, y `--encoding` nombró otra — gana la marca |

**Las tres del medio son las de la fase 9**, y las dos últimas de esas
tres no hablan de una línea sino del archivo entero: dicen que un
valor mostrado no se ha leído. Una columna `End` rellena en un LRC, una
posición en un MicroDVD — el archivo no llevaba nada de eso.

**Una operación puede añadir los suyos**, que no son diagnósticos de lectura:
[`correct`](correct.md#un-patrón-que-no-puede-aplicarse) indica, con el subtítulo, el patrón
que ha desistido. Se dice en el nivel 1 y lleva, en el objeto del archivo, el identificador
`pattern-failed` — nunca un fallo del archivo.

## Una traducción

Un archivo de traducción se procesa **como cualquier archivo de subtítulos**:
`subedit-cli case --to sentence film.fr.srt` lo cambia sin nada nuevo. Lo que añade el emparejamiento, y lo que
dicen tres opciones, es limitado: **saber cómo caen las líneas de una traducción sobre los
subtítulos del archivo principal** — es `inspect -t` —, y **cambiar la traducción manteniéndola
colocada sobre su archivo principal**, con las mismas reglas que la ventana.

| Opción | Dónde | Valor | Por defecto |
| :----- | :---- | :----- | :----- |
| `-t`, `--translation-file` | `inspect`, `hearing-impaired`, `replace`, `case`, `italics`, `dialogue-dashes`, `correct`, `pair` | el archivo de traducción | — |
| `--document` | los seis subcomandos de texto, ni `inspect` ni `pair` | `main` o `translation` | `main` |
| `--align-method` | como `-t` | `position` o `number` | `position` |

- **`position`** compara el centro de cada línea con los límites de los subtítulos: una línea que falta no
  desplaza las siguientes. **`number`** envía la línea *n* al subtítulo *n*, sin mirar las
  posiciones. La posición es el valor por defecto, por la misma razón que en la ventana: una línea que falta en
  medio desplaza todo el resto de una traducción emparejada por número.
- **`--document translation` exige `-t`, y `-t` exige `--document translation`** en un subcomando
  de texto — uno sin el otro es un error de uso, código `1`, porque un archivo leído que ningún gesto
  utiliza es un olvido y no una preferencia. `-t` en `inspect` no tiene `--document`:
  informa. **`--align-method` sin `-t` también es un error de uso.**
- **`-t` nombra un archivo, y por tanto solo tiene sentido con una sola entrada**: con varias entradas, o
  con un directorio tomado por `--recursive`, se rechaza como `--output`, **antes de leer
  un archivo**. Para un lote, hay que lanzar una invocación por par.
- **Un archivo de traducción idéntico al archivo principal se rechaza** — se escriba como se escriba
  su ruta: `<translation>: the file is already open as the main document`, código `2`.
- **Solo se escribe el documento al que se apunta**, en **su propia ruta y en su propio formato**: el formato,
  la codificación y los finales de línea del archivo de traducción. `--output` nombra el archivo de la
  traducción, `--output-dir` pone ahí su nombre base, `--in-place` lo reescribe; **el archivo principal
  nunca se toca**. Las rutas del informe son las de la traducción.
- **Los subtítulos que crea el emparejamiento** — una línea a la que nada llegó — se escriben en
  la traducción con **sus propias posiciones**, sea cual sea el método; **un subtítulo que nadie
  tradujo se escribe como un bloque sin texto**, que se lee sin anomalía.
- **`--encoding` y `--frame-rate` se aplican a los dos archivos.**
- Los subcomandos de **posición** — `shift`, `transform`, `framerate`, `snap`, `adjust` — no tienen
  estas opciones: se les dan los dos archivos, como para un lote,
  `subedit-cli shift --by 2 film.srt film.fr.srt --output-dir out/`.
- **Escribir la traducción con los tiempos reajustados sobre el archivo principal** — lo único que hace el emparejamiento y que un archivo solo
  no hace — es el trabajo de [`pair`](pair.md), que no tiene ni `--document` ni `--recursive`.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\n[Un oiseau] Bonjour.\n\n2\n00:00:03,000 --> 00:00:04,000\nAu revoir.\n\n' > film.fr.srt; subedit-cli hearing-impaired film.srt -t film.fr.srt --document translation --output propre.fr.srt; cat propre.fr.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\n[Un oiseau] Bonjour.\n\n2\n00:00:03,000 --> 00:00:04,000\nAu revoir.\n\n' > film.fr.srt; subedit-cli hearing-impaired film.srt -t film.fr.srt --document translation --output propre.fr.srt; cat propre.fr.srt
film.fr.srt: 1 subtitle cleaned, 0 removed -> propre.fr.srt
1
00:00:01,000 --> 00:00:02,000
Bonjour.

2
00:00:03,000 --> 00:00:04,000
Au revoir.
```

Con `--format json`, el objeto de cada archivo lleva también la clave `alignment`, que dice cómo se
colocó la traducción — `{"file", "method", "attached", "born", "untranslated", "out_of_order"}`, como
la clave `translation` de [`inspect`](inspect.md#en-json) — y, **con `--dry-run`**, `changes` enumera
los subtítulos de la traducción, cada uno con `"document":"translation"`.

## Varios archivos

Todo subcomando acepta varias rutas, salvo [`append`](append.md), [`split-file`](split-file.md) y
[`pair`](pair.md), cuya aridad es la que indica su página. Cada una se procesa
de forma independiente: el fallo de una no interrumpe las demás, y los fallos
se indican nombrando el archivo y la razón.

**Un directorio vale por los archivos que contiene, con `--recursive` (`-r`)**:
sin esta opción, darlo es un error de uso, código `1`, y no se procesa
nada — rechazarlo es mejor que no procesar nada y devolver `0`:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?" -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?"
films: is a directory: use --recursive
code=1
```

El recorrido, el árbol conservado bajo `--output-dir` y lo que se deja
de lado se describen en [Procesar un árbol](lots.md).

## Las páginas de manual

Una instalación coloca **una página de manual por subcomando** junto a `subedit-cli(1)`:
`subedit-cli-shift(1)`, `subedit-cli-convert(1)` y las demás, una por cada fila de
[la tabla de subcomandos](#subcomandos). `subedit-cli(1)` contiene este capítulo — las opciones globales,
las salidas, los códigos de salida — y cada una de las demás páginas contiene el capítulo de su subcomando, con sus
ejemplos. La página de la ventana, `subedit-gui(1)`, está a su lado.

```console
$ man subedit-cli
$ man subedit-cli-shift
```

**Las páginas no se escriben aparte de este manual**: se generan a partir de él cuando se
compila el programa, de modo que dicen lo que él dice, y no pueden alejarse de él. La sección ARCHIVOS de cada
página nombra el directorio en el que está instalado el manual.

| Páginas | Dónde se instalan |
| :---- | :----------------------- |
| inglés | `share/man/man1` |
| cada idioma instalado — catalán, checo, alemán, español, finés, francés, gallego, hebreo, húngaro, interlingue, islandés, italiano, neerlandés, polaco, portugués, portugués brasileño | `share/man/<language>/man1` — `share/man/fr/man1` |

**`man` elige el idioma según la configuración regional**, como hace el programa: con una configuración regional francesa,
`man subedit-cli` muestra la página francesa, y un idioma cuyas páginas no están instaladas recibe las
inglesas. `-L` pide un idioma sea cual sea la configuración regional:

```console
$ man -L fr subedit-cli-shift
```

## Códigos de salida

| Código | Significado |
| :--- | :------------ |
| `0` | todo ha tenido éxito |
| `1` | error de uso — opción desconocida, valor no válido, combinación prohibida |
| `2` | no se ha podido procesar ningún archivo |
| `3` | algunos archivos se han procesado y otros no |

`2` y `3` se distinguen para que un script sepa cómo actuar sin volver a leer la salida:
«nada ha funcionado» y «falta uno» no piden la misma reacción.

Un error de uso se detecta **antes de cualquier procesamiento**: nunca deja un
lote procesado a medias.

## Errores

| Lo que lo provoca | Lo que se escribe, en la salida de error |
| :------------------ | :--------------------------------------- |
| opción o subcomando desconocidos | el nombre defectuoso, y una indicación hacia `--help` |
| `--quiet` con `-v` | `--quiet and -v ask for opposite things; give one or the other` |
| `--config` con `--no-config` | `--config excludes --no-config` |
| `--config` con un nombre vacío | `--config needs a file name` |
| archivo que falta | `<path>: does not exist` |
| archivo ilegible | `<path>: cannot be opened: permission denied` |
| codificación que escribe su propia marca | `"<name>" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do` |
| bytes que no se decodifican | `<path>: cannot be decoded in the chosen encoding` |
| formato no reconocido | `<path>: is in no format this tool knows` |
| nada que parezca un subtítulo | `<path>: holds nothing recognisable as a subtitle` |
| `--document translation` sin `-t` | `--document translation needs the translation file: use -t` |
| `-t` sin `--document translation` | `-t names a translation to change: use --document translation` |
| `--align-method` sin `-t` | `--align-method needs a translation file: use -t` |
| `-t` con varias entradas, o un directorio | `-t names one file but several inputs were given: use one invocation per pair` |
| traducción idéntica al archivo principal | `<translation>: the file is already open as the main document` |
