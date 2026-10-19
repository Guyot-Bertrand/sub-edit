# `config`

<!-- machine-translated -->
> Esta página es una traducción automática y todavía no la ha revisado ningún lector del idioma.

```
subedit-cli [--config FILE | --no-config] [--format text|json] config list
subedit-cli [--config FILE | --no-config] [--format text|json] config show
subedit-cli [--config FILE] config set KEY VALUE
subedit-cli [--config FILE] config unset KEY
```

Enumera, muestra y cambia la **configuración de las operaciones**: la parte del archivo de configuración que
comparten la ventana y la línea de comandos. `Adjust Durations…` y `Correct Texts…` recuerdan allí sus
valores, y [`adjust`](adjust.md) y [`correct`](correct.md) los leen por defecto; véase
[El archivo de configuración](invocation.md#el-archivo-de-configuración).

**Lo que no hace:** nunca lee ni escribe una **preferencia de visualización** (la geometría de la
ventana, las columnas, el tema, las opciones de búsqueda, la ortografía, el vídeo…). Esas siguen siendo
asunto de la ventana, se ajustan en la ventana y se describen en [su manual](../subedit-gui/preferences.md#el-archivo).
Tampoco toca nunca un archivo de subtítulos.

<!-- example: subedit-cli config --help -->
```console
$ subedit-cli config --help
List, show and change the operation settings
Usage: subedit-cli config [OPTIONS] SUBCOMMAND

Options:
  -h,--help                   Print this help message and exit

Subcommands:
  list                        Write every operation key, what it is for, what it accepts and its default
  show                        Write the settings file used, then every operation key with its value
  set                         Write KEY = VALUE into the settings file, keeping every other setting
  unset                       Put KEY back to its default in the settings file, keeping every other setting
```

## Subcomandos

| Subcomando | Qué hace | Archivo |
| :---------- | :----------- | :--- |
| `config list` | escribe cada clave de operación, con para qué sirve, qué acepta, su valor por defecto y quién la usa; **ningún valor** | no lee ninguno |
| `config show` | escribe el archivo usado y si existe, y después cada clave de operación con su valor, de dónde viene el valor y su valor por defecto | lo lee |
| `config set KEY VALUE` | comprueba `VALUE` y después reescribe el archivo con `KEY = VALUE`, conservando todos los demás ajustes | lo lee y lo escribe |
| `config unset KEY` | devuelve `KEY` a su valor por defecto, escrito comentado como el archivo escribe siempre un valor por defecto | lo lee y lo escribe |

| Opción | Obligatoria | Valores | Por defecto |
| :----- | :------- | :----- | :------ |
| `KEY` | sí, para `set` y `unset` | una clave de operación, tal como la nombra `config list` | — |
| `VALUE` | sí, para `set` | lo que `config list` dice que acepta la clave | — |

`config` no tiene opciones propias. Las opciones globales escritas **antes** de `config` eligen el
archivo y la forma de la salida:

| Opción global | Efecto sobre `config` |
| :------------ | :----------------- |
| `--config FILE` | `show`, `set` y `unset` actúan sobre `FILE` en lugar del archivo del usuario, incluso cuando aún no existe; `list` lo ignora |
| `--no-config` | `list` funciona como siempre; `show` muestra los valores por defecto, con `File: none (--no-config)`; `set` y `unset` se rechazan: no hay archivo en el que escribir |
| `--format json` | `list` y `show` escriben un objeto JSON por línea; `set` y `unset` no escriben nada en la salida estándar |
| `-q`, `--quiet` | `set` y `unset` dejan de confirmar lo que escribieron |

## Qué archivo

Sin `--config`, el archivo del usuario:

```
$XDG_CONFIG_HOME/subedit/settings.conf
```

o `~/.config/subedit/settings.conf` cuando `XDG_CONFIG_HOME` no está definida, está vacía o no es una ruta
absoluta. Es el archivo que la ventana lee y escribe; `config show` escribe su ruta en su primera línea:

<!-- example: XDG_CONFIG_HOME=/home/vous/.config subedit-cli config show | head -1 -->
```console
$ XDG_CONFIG_HOME=/home/vous/.config subedit-cli config show | head -1
File: /home/vous/.config/subedit/settings.conf (not found)
```

Cuando ni `XDG_CONFIG_HOME` ni `HOME` dan un lugar, no hay archivo del usuario: `show`, `set`
y `unset` se rechazan, y `--config FILE` nombra uno.

## `config list`

**El catálogo de las claves de operación**, en el orden del archivo: cada clave, y después cuatro líneas
sangradas: para qué sirve, `Accepts:` (el conjunto cerrado, o la forma y los límites), `Default:` y
`Used by:` (los comandos de `subedit-cli` y las acciones de la ventana que la leen). No lee ningún archivo ni
necesita ninguno, y funciona con `--no-config`.

<!-- example: subedit-cli config list | head -10 -->
```console
$ subedit-cli config list | head -10
duration-adjust.speed
    Reading speed the durations are brought to.
    Accepts: a number of characters per second, above 0, decimals allowed
    Default: 15
    Used by: subedit-cli adjust; Adjust Durations…
duration-adjust.lengthen
    Let the reading speed move an end later.
    Accepts: true or false
    Default: true
    Used by: subedit-cli adjust; Adjust Durations…
```

Enumera **33 claves**:

<!-- example: subedit-cli config list | grep -c '^[a-z]' -->
```console
$ subedit-cli config list | grep -c '^[a-z]'
33
```

**La línea de comandos lee trece de ellas**; las otras veinte solo las lee la ventana,
y su línea `Used by:` lo dice, con la opción de `correct` que ocupa su lugar:

| Clave | Leída por |
| :-- | :------ |
| `duration-adjust.speed`, `duration-adjust.lengthen`, `duration-adjust.shorten` | [`adjust`](adjust.md) y `Adjust Durations…` |
| `duration-adjust.minimum-enabled`, `duration-adjust.minimum-ms` | `adjust` y `Adjust Durations…` |
| `duration-adjust.maximum-enabled`, `duration-adjust.maximum-ms` | `adjust` y `Adjust Durations…` |
| `duration-adjust.gap-enabled`, `duration-adjust.gap-ms` | `adjust` y `Adjust Durations…` |
| `correction.human`, `correction.ocr` | [`correct`](correct.md) y `Correct Texts…` |
| `correction.remove-blank` | `correct` y `Correct Texts…` |
| `correction.line-break.max-lines` | `correct` y `Correct Texts…` |
| cualquier otra clave `correction.*` | solo `Correct Texts…`; `correct` toma `--tasks`, `--code`, `--max-length`, `--skip-length`, `--skip-lines`, `--language`, `--enable` y `--disable` de su propia línea |

Los números de líneas, `correction.line-break.max-lines` y `correction.line-break.skip-max-lines`,
son números enteros de 1 a 1000 (los límites de `correct --max-lines`), y las longitudes de línea,
`correction.line-break.max-length` y `correction.line-break.skip-max-length`, son números mayores
que 0: `set` rechaza el resto, y un archivo editado a mano que contenga uno conserva el valor por defecto.

`correct` toma las tareas, el código y la longitud de línea de su propia línea a propósito: en una línea de
comandos no hay nada marcado de antemano, y la ventana mide las líneas en ems mientras que la línea de comandos
cuenta caracteres. `correction.activations`, la lista de casillas de patrones que gestiona la ventana, no es
una clave del catálogo: `correct` tiene `--enable` y `--disable`.

Con `--format json`, un objeto por clave: `key`, `description`, `accepts`, `default`, `used_by`
y `surface`, `"both"` o `"window-only"`:

<!-- example: subedit-cli --format json config list | head -1 -->
```console
$ subedit-cli --format json config list | head -1
{"schema":1,"command":"config","key":"duration-adjust.speed","description":"Reading speed the durations are brought to.","accepts":"a number of characters per second, above 0, decimals allowed","default":"15","used_by":"subedit-cli adjust; Adjust Durations…","surface":"both"}
```

## `config show`

**El archivo usado, y después cada clave de operación con su valor**: `File: <path> (found)`,
`(not found)` o `(unreadable: <reason>)`, y después una línea por clave:
`[file]` y el valor por defecto cuando el archivo la fija, `[default]` cuando no. Una clave que el archivo
contiene con un valor que no puede leer muestra su valor por defecto. Las preferencias de visualización no se enumeran.

<!-- example: subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --config mine/settings.conf config show | head -4 -->
```console
$ subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --config mine/settings.conf config show | head -4
duration-adjust.speed = 12
File: mine/settings.conf (found)
duration-adjust.speed = 12   [file]   (default 15)
duration-adjust.lengthen = true   [default]
duration-adjust.shorten = false   [default]
```

Un archivo inexistente no es un error: `show` escribe los valores por defecto, y el código es `0`, tanto para el
archivo del usuario como para un `FILE` nombrado por `--config`, que [`adjust`](adjust.md) y [`correct`](correct.md),
en cambio, rechazan (véase [El archivo de configuración](invocation.md#el-archivo-de-configuración)). Con
`--no-config`:

<!-- example: subedit-cli --no-config config show | head -2 -->
```console
$ subedit-cli --no-config config show | head -2
File: none (--no-config)
duration-adjust.speed = 15   [default]
```

Con `--format json`, un primer objeto `{"file", "exists"}` (`file` es `null` con `--no-config`)
y después un objeto por clave: `key`, `value`, `source` (`"file"` o `"default"`), `default`,
`used_by`, `surface`.

<!-- example: subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --format json --config mine/settings.conf config show | head -2 -->
```console
$ subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --format json --config mine/settings.conf config show | head -2
duration-adjust.speed = 12
{"schema":1,"command":"config","file":"mine/settings.conf","exists":true}
{"schema":1,"command":"config","key":"duration-adjust.speed","value":"12","source":"file","default":"15","used_by":"subedit-cli adjust; Adjust Durations…","surface":"both"}
```

## `config set` y `config unset`

**`set` comprueba el valor con el propio lector del archivo** (el lector que usa la ventana) y lo rechaza
cuando no puede leerse, antes de escribir nada. **`unset`** vuelve a escribir el valor por defecto,
comentado: la clave sigue entonces el valor por defecto aunque una versión posterior lo cambie. Restablecer una
clave que ya está en su valor por defecto tiene éxito y no cambia nada.

Los dos **reescriben el archivo entero y de forma atómica**: se conserva toda preferencia de visualización que ya contenga,
el bloque de operaciones va primero y el bloque de visualización después, cada uno bajo su encabezado. **Un valor
que el archivo no pudo leer, y una clave desconocida, no se conservan**: la reescritura es la de la ventana, que
también los descarta. **Cada valor descartado se indica**, en la salida de error estándar en el nivel por defecto, preferencias
de visualización incluidas: `<key>: unreadable value '<value>' replaced by its default`. El archivo y
su directorio se crean si faltan.

La confirmación va a la salida de error estándar, en el nivel por defecto: `KEY = VALUE` para `set`,
`KEY reset to its default (DEFAULT)` para `unset`.

<!-- example: subedit-cli --config mine/settings.conf config set duration-adjust.gap-ms 100; grep -e '^[a-z]' -e '^# [A-Z][a-z]* [a-z]*:' mine/settings.conf; subedit-cli --config mine/settings.conf config unset duration-adjust.gap-ms; grep gap-ms mine/settings.conf -->
```console
$ subedit-cli --config mine/settings.conf config set duration-adjust.gap-ms 100; grep -e '^[a-z]' -e '^# [A-Z][a-z]* [a-z]*:' mine/settings.conf; subedit-cli --config mine/settings.conf config unset duration-adjust.gap-ms; grep gap-ms mine/settings.conf
duration-adjust.gap-ms = 100
# Operation settings: shared by the window and the command line (subedit-cli reads these).
duration-adjust.gap-ms = 100
# Display preferences: the window's own, the command line never reads them.
duration-adjust.gap-ms reset to its default (0)
#duration-adjust.gap-ms = 0
```

## Códigos de salida

| Código | Significado |
| :--- | :------ |
| `0` | hecho, incluidos `show` sobre un archivo inexistente y `unset` sobre una clave en su valor por defecto |
| `1` | error de uso: ningún subcomando, un argumento que falta, una clave o un valor rechazados, `--no-config` con `set` o `unset`, `--config` con un nombre vacío, ningún archivo del usuario y ningún `--config`; no se escribe nada |
| `2` | el archivo no se puede leer o escribir; no se escribe nada |

## Errores

| Qué lo provoca | Qué se escribe, en la salida de error estándar |
| :--------------- | :------------------------------------- |
| una preferencia de visualización | `<key> is a display preference; set it in the window` |
| una clave desconocida | `unknown setting <key>; subedit-cli config list names them` |
| un valor que la clave no acepta | `cannot read '<value>' for <key>; subedit-cli config list says what it accepts` |
| `correction.activations` | `correction.activations is managed by the window; use --enable/--disable on correct` |
| `set` o `unset` con `--no-config` | `--no-config has nothing to write to` |
| no se puede localizar ningún archivo del usuario, y no hay `--config` | `no settings file could be located; give one with --config FILE` |
| `--config` con `--no-config` | `--config excludes --no-config` |
| `--config` con un nombre vacío | `--config needs a file name` |
| un archivo que existe y no se puede leer, para `set` o `unset` | `<path>: cannot be read; nothing written` |
| un archivo que no se puede escribir | `<path>: cannot be written`, o `<path>: cannot be written: permission denied` |

<!-- example: subedit-cli --config mine/settings.conf config set general.theme dark; echo "code=$?"; subedit-cli --config mine/settings.conf config set duration-adjust.speed fast; echo "code=$?"; subedit-cli --no-config config unset duration-adjust.speed; echo "code=$?" -->
```console
$ subedit-cli --config mine/settings.conf config set general.theme dark; echo "code=$?"; subedit-cli --config mine/settings.conf config set duration-adjust.speed fast; echo "code=$?"; subedit-cli --no-config config unset duration-adjust.speed; echo "code=$?"
general.theme is a display preference; set it in the window
code=1
cannot read 'fast' for duration-adjust.speed; subedit-cli config list says what it accepts
code=1
--no-config has nothing to write to
code=1
```
