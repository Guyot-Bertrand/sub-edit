# `config`

<!-- machine-translated -->
> Esta páxina é unha tradución automática e aínda non a revisou ningún lector da lingua.

```
subedit-cli [--config FILE | --no-config] [--format text|json] config list
subedit-cli [--config FILE | --no-config] [--format text|json] config show
subedit-cli [--config FILE] config set KEY VALUE
subedit-cli [--config FILE] config unset KEY
```

Lista, mostra e modifica a **configuración das operacións**: a parte do ficheiro de configuración
que comparten a xanela e a liña de ordes. `Adjust Durations…` e `Correct Texts…` lembran alí os
seus valores, e [`adjust`](adjust.md) e [`correct`](correct.md) lénos de maneira predeterminada —
véxase [O ficheiro de configuración](invocation.md#o-ficheiro-de-configuración).

**O que non fai:** nunca le nin escribe unha **preferencia de visualización** (a xeometría da
xanela, as columnas, o tema, as opcións de busca, a corrección ortográfica, o vídeo…). Esas seguen
sendo asunto da xanela, fíxanse na xanela e descríbense no [seu manual](../subedit-gui/preferences.md#o-ficheiro).
Tampouco toca nunca un ficheiro de subtítulos.

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

## Subordes

| Suborde | O que fai | Ficheiro |
| :---------- | :----------- | :--- |
| `config list` | escribe cada clave das operacións, co seu propósito, o que acepta, o seu valor predeterminado e quen a usa — **ningún valor** | non le ningún |
| `config show` | escribe o ficheiro usado e se existe, despois cada clave das operacións co seu valor, a orixe do valor e o seu valor predeterminado | léo |
| `config set KEY VALUE` | verifica `VALUE` e despois reescribe o ficheiro con `KEY = VALUE`, conservando todos os demais axustes | léo e escríbeo |
| `config unset KEY` | devolve `KEY` ao seu valor predeterminado, escrito como comentario, tal como o ficheiro escribe sempre un valor predeterminado | léo e escríbeo |

| Opción | Obrigatorio | Valores | Predeterminado |
| :----- | :------- | :----- | :------ |
| `KEY` | si, para `set` e `unset` | unha clave das operacións, tal como a nomea `config list` | — |
| `VALUE` | si, para `set` | o que `config list` di que acepta a clave | — |

`config` non ten ningunha opción propia. As opcións globais escritas **antes** de `config` escollen
o ficheiro e a forma da saída:

| Opción global | Efecto sobre `config` |
| :------------ | :----------------- |
| `--config FILE` | `show`, `set` e `unset` actúan sobre `FILE` en lugar do ficheiro do usuario, mesmo cando aínda non existe; `list` ignóraa |
| `--no-config` | `list` funciona como de costume; `show` mostra os valores predeterminados, con `File: none (--no-config)`; `set` e `unset` rexéitanse: non hai ningún ficheiro no que escribir |
| `--format json` | `list` e `show` escriben un obxecto JSON por liña; `set` e `unset` non escriben nada na saída estándar |
| `-q`, `--quiet` | `set` e `unset` deixan de confirmar o que escribiron |

## Que ficheiro

Sen `--config`, o ficheiro do usuario:

```
$XDG_CONFIG_HOME/subedit/settings.conf
```

ou `~/.config/subedit/settings.conf` cando `XDG_CONFIG_HOME` non está definida, está baleira ou non
é un camiño absoluto. É o ficheiro que le e escribe a xanela; `config show` escribe o seu camiño na
primeira liña:

<!-- example: XDG_CONFIG_HOME=/home/vous/.config subedit-cli config show | head -1 -->
```console
$ XDG_CONFIG_HOME=/home/vous/.config subedit-cli config show | head -1
File: /home/vous/.config/subedit/settings.conf (not found)
```

Cando nin `XDG_CONFIG_HOME` nin `HOME` dan un lugar, non hai ficheiro do usuario: `show`, `set` e
`unset` rexéitanse, e `--config FILE` indica un.

## `config list`

**O catálogo das claves das operacións**, na orde do ficheiro: cada clave e despois catro liñas
sangradas — o seu propósito, `Accepts:` (o conxunto pechado, ou a forma e os límites), `Default:` e
`Used by:` (as ordes de `subedit-cli` e as accións da xanela que a len). Non le ningún ficheiro nin
necesita ningún, e funciona con `--no-config`.

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

Lista **40 claves**:

<!-- example: subedit-cli config list | grep -c '^[a-z]' -->
```console
$ subedit-cli config list | grep -c '^[a-z]'
40
```

**Vinte delas lense desde a liña de ordes**; as outras vinte só as le a xanela, e a súa liña
`Used by:` dino, coa opción de `correct` que ocupa o seu lugar:

| Clave | Lido por |
| :-- | :------ |
| `duration-adjust.speed`, `duration-adjust.lengthen`, `duration-adjust.shorten` | [`adjust`](adjust.md) e `Adjust Durations…` |
| `duration-adjust.minimum-enabled`, `duration-adjust.minimum-ms` | `adjust` e `Adjust Durations…` |
| `duration-adjust.maximum-enabled`, `duration-adjust.maximum-ms` | `adjust` e `Adjust Durations…` |
| `duration-adjust.gap-enabled`, `duration-adjust.gap-ms` | `adjust` e `Adjust Durations…` |
| `correction.human`, `correction.ocr` | [`correct`](correct.md) e `Correct Texts…` |
| `correction.remove-blank` | `correct` e `Correct Texts…` |
| `correction.line-break.max-lines` | `correct` e `Correct Texts…` |
| `transcription.engine`, `transcription.executable`, `transcription.model`, `transcription.language`, `transcription.merge-gap-ms`, `transcription.max-duration-ms`, `transcription.max-characters` | [`transcribe`](transcribe.md) e `Transcribe Audio…` |
| calquera outra clave `correction.*` | só `Correct Texts…` — `correct` toma `--tasks`, `--code`, `--max-length`, `--skip-length`, `--skip-lines`, `--language`, `--enable` e `--disable` da súa propia liña |

Os números de liñas, `correction.line-break.max-lines` e `correction.line-break.skip-max-lines`,
son números enteiros de 1 a 1000 — os límites de `correct --max-lines` —, e as lonxitudes das
liñas, `correction.line-break.max-length` e `correction.line-break.skip-max-length`, son números
maiores que 0: `set` rexeita o resto, e un ficheiro editado a man que conteña un conserva o valor
predeterminado.

`correct` toma as tarefas, o código e a lonxitude da liña da súa propia liña adrede: nunha liña de
ordes non hai nada marcado de antemán, e a xanela mide as liñas en ems alí onde a liña de ordes
conta caracteres. `correction.activations`, a lista de caixas de padróns que xestiona a xanela,
non é unha clave do catálogo: `correct` ten `--enable` e `--disable`.

Con `--format json`, un obxecto por clave — `key`, `description`, `accepts`, `default`, `used_by`
e `surface`, `"both"` ou `"window-only"`:

<!-- example: subedit-cli --format json config list | head -1 -->
```console
$ subedit-cli --format json config list | head -1
{"schema":1,"command":"config","key":"duration-adjust.speed","description":"Reading speed the durations are brought to.","accepts":"a number of characters per second, above 0, decimals allowed","default":"15","used_by":"subedit-cli adjust; Adjust Durations…","surface":"both"}
```

## `config show`

**O ficheiro usado e despois cada clave das operacións co seu valor**: `File: <path> (found)`,
`(not found)` ou `(unreadable: <reason>)`, e despois unha liña por clave —
`[file]` e o valor predeterminado cando o ficheiro a fixa, `[default]` cando non. Unha clave que o
ficheiro contén cun valor que non pode ler mostra o seu valor predeterminado. As preferencias de
visualización non se listan.

<!-- example: subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --config mine/settings.conf config show | head -4 -->
```console
$ subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --config mine/settings.conf config show | head -4
duration-adjust.speed = 12
File: mine/settings.conf (found)
duration-adjust.speed = 12   [file]   (default 15)
duration-adjust.lengthen = true   [default]
duration-adjust.shorten = false   [default]
```

Un ficheiro ausente non é un erro: `show` escribe os valores predeterminados, e o código é `0` —
tanto para o ficheiro do usuario como para un `FILE` indicado con `--config`, que
[`adjust`](adjust.md) e [`correct`](correct.md), pola contra, rexeitan (véxase
[O ficheiro de configuración](invocation.md#o-ficheiro-de-configuración)). Con
`--no-config`:

<!-- example: subedit-cli --no-config config show | head -2 -->
```console
$ subedit-cli --no-config config show | head -2
File: none (--no-config)
duration-adjust.speed = 15   [default]
```

Con `--format json`, un primeiro obxecto `{"file", "exists"}` — `file` é `null` con `--no-config` —
e despois un obxecto por clave: `key`, `value`, `source` (`"file"` ou `"default"`), `default`,
`used_by`, `surface`.

<!-- example: subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --format json --config mine/settings.conf config show | head -2 -->
```console
$ subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --format json --config mine/settings.conf config show | head -2
duration-adjust.speed = 12
{"schema":1,"command":"config","file":"mine/settings.conf","exists":true}
{"schema":1,"command":"config","key":"duration-adjust.speed","value":"12","source":"file","default":"15","used_by":"subedit-cli adjust; Adjust Durations…","surface":"both"}
```

## `config set` e `config unset`

**`set` verifica o valor co propio lector do ficheiro** — o lector que usa a xanela — e rexéitao
cando non se pode ler, antes de escribir nada. **`unset`** volve escribir o valor predeterminado,
como comentario: a clave segue entón o valor predeterminado aínda que unha versión posterior o
cambie. Desfixar unha clave que xa está no seu valor predeterminado ten éxito e non cambia nada.

As dúas **reescriben o ficheiro enteiro e de forma atómica**: consérvanse todas as preferencias de
visualización que xa contén, o bloque das operacións vai primeiro e o bloque de visualización
despois, cada un baixo a súa cabeceira. **Un valor que o ficheiro non puido ler e unha clave
descoñecida non se conservan** — a reescritura é a da xanela, que tamén os descarta. **Cada valor
descartado indícase**, no erro estándar no nivel predeterminado, incluídas as preferencias de
visualización: `<key>: unreadable value '<value>' replaced by its default`. O ficheiro e o seu
directorio créanse cando faltan.

A confirmación vai ao erro estándar, no nivel predeterminado: `KEY = VALUE` para `set`,
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
#transcription.merge-gap-ms = 400
```

## Códigos de saída

| Código | Significado |
| :--- | :------ |
| `0` | feito — incluídos `show` sobre un ficheiro ausente e `unset` sobre unha clave no seu valor predeterminado |
| `1` | erro de uso: ningunha suborde, un argumento ausente, unha clave ou un valor rexeitados, `--no-config` con `set` ou `unset`, `--config` cun nome baleiro, ningún ficheiro do usuario e ningún `--config`; non se escribe nada |
| `2` | o ficheiro non se pode ler nin escribir; non se escribe nada |

## Erros

| O que o provoca | O que se escribe, no erro estándar |
| :--------------- | :------------------------------------- |
| unha preferencia de visualización | `<key> is a display preference; set it in the window` |
| unha clave descoñecida | `unknown setting <key>; subedit-cli config list names them` |
| un valor que a clave non acepta | `cannot read '<value>' for <key>; subedit-cli config list says what it accepts` |
| `correction.activations` | `correction.activations is managed by the window; use --enable/--disable on correct` |
| `set` ou `unset` con `--no-config` | `--no-config has nothing to write to` |
| non se pode localizar ningún ficheiro do usuario, e ningún `--config` | `no settings file could be located; give one with --config FILE` |
| `--config` con `--no-config` | `--config excludes --no-config` |
| `--config` cun nome baleiro | `--config needs a file name` |
| un ficheiro que existe e non se pode ler, para `set` ou `unset` | `<path>: cannot be read; nothing written` |
| un ficheiro que non se pode escribir | `<path>: cannot be written`, ou `<path>: cannot be written: permission denied` |

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
