# `config`

<!-- machine-translated -->
> Aquesta pàgina és una traducció automàtica i encara no l'ha revisada cap lector de la llengua.

```
subedit-cli [--config FILE | --no-config] [--format text|json] config list
subedit-cli [--config FILE | --no-config] [--format text|json] config show
subedit-cli [--config FILE] config set KEY VALUE
subedit-cli [--config FILE] config unset KEY
```

Llista, mostra i canvia la **configuració de les operacions**: la part del fitxer de configuració que
la finestra i la línia d'ordres comparteixen. `Adjust Durations…` i `Correct Texts…` hi recorden els seus
valors, i [`adjust`](adjust.md) i [`correct`](correct.md) els llegeixen per defecte — vegeu
[El fitxer de configuració](invocation.md#el-fitxer-de-configuració).

**El que no fa:** no llegeix ni escriu mai cap **preferència de visualització** (la geometria de la
finestra, les columnes, el tema, les opcions de cerca, la correcció ortogràfica, el vídeo…). Aquestes continuen sent
cosa de la finestra, es defineixen a la finestra i es descriuen al [seu manual](../subedit-gui/preferences.md#el-fitxer).
Tampoc no toca mai cap fitxer de subtítols.

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

## Subordres

| Subordre | Què fa | Fitxer |
| :---------- | :----------- | :--- |
| `config list` | escriu cada clau d'operació, amb per a què serveix, què accepta, el seu valor per defecte i qui la fa servir — **cap valor** | no en llegeix cap |
| `config show` | escriu el fitxer usat i si existeix, i després cada clau d'operació amb el seu valor, d'on ve el valor, i el seu valor per defecte | el llegeix |
| `config set KEY VALUE` | comprova `VALUE`, i després reescriu el fitxer amb `KEY = VALUE`, conservant tots els altres paràmetres | el llegeix i l'escriu |
| `config unset KEY` | torna `KEY` al seu valor per defecte, escrit en comentari tal com el fitxer escriu sempre un valor per defecte | el llegeix i l'escriu |

| Opció | Obligatori | Valors | Per defecte |
| :----- | :------- | :----- | :------ |
| `KEY` | sí, per a `set` i `unset` | una clau d'operació, tal com l'anomena `config list` | — |
| `VALUE` | sí, per a `set` | el que `config list` diu que accepta la clau | — |

`config` no té cap opció pròpia. Les opcions globals escrites **abans** de `config` trien el
fitxer i la forma de la sortida:

| Opció global | Efecte sobre `config` |
| :------------ | :----------------- |
| `--config FILE` | `show`, `set` i `unset` actuen sobre `FILE` en lloc del fitxer de l'usuari, fins i tot quan encara no existeix; `list` l'ignora |
| `--no-config` | `list` funciona com de costum; `show` mostra els valors per defecte, amb `File: none (--no-config)`; `set` i `unset` es refusen: no hi ha cap fitxer on escriure |
| `--format json` | `list` i `show` escriuen un objecte JSON per línia; `set` i `unset` no escriuen res a la sortida estàndard |
| `-q`, `--quiet` | `set` i `unset` ja no confirmen el que han escrit |

## Quin fitxer

Sense `--config`, el fitxer de l'usuari:

```
$XDG_CONFIG_HOME/subedit/settings.conf
```

o `~/.config/subedit/settings.conf` quan `XDG_CONFIG_HOME` no està definida, és buida o no és un camí
absolut. És el fitxer que la finestra llegeix i escriu; `config show` n'escriu el camí a la primera línia:

<!-- example: XDG_CONFIG_HOME=/home/vous/.config subedit-cli config show | head -1 -->
```console
$ XDG_CONFIG_HOME=/home/vous/.config subedit-cli config show | head -1
File: /home/vous/.config/subedit/settings.conf (not found)
```

Quan ni `XDG_CONFIG_HOME` ni `HOME` no donen cap lloc, no hi ha fitxer de l'usuari: `show`, `set`
i `unset` es refusen, i `--config FILE` n'anomena un.

## `config list`

**El catàleg de les claus d'operació**, en l'ordre del fitxer: cada clau, i després quatre línies
sagnades — per a què serveix, `Accepts:` (el conjunt tancat, o la forma i els límits), `Default:` i
`Used by:` (les ordres de `subedit-cli` i les accions de la finestra que la llegeixen). No llegeix cap fitxer ni
en necessita cap, i funciona amb `--no-config`.

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

Llista **41 claus**:

<!-- example: subedit-cli config list | grep -c '^[a-z]' -->
```console
$ subedit-cli config list | grep -c '^[a-z]'
41
```

**Vint-i-una les llegeix la línia d'ordres**; les vint restants només les llegeix la finestra,
i la seva línia `Used by:` ho diu, amb l'opció de `correct` que n'ocupa el lloc:

| Clau | Llegida per |
| :-- | :------ |
| `duration-adjust.speed`, `duration-adjust.lengthen`, `duration-adjust.shorten` | [`adjust`](adjust.md) i `Adjust Durations…` |
| `duration-adjust.minimum-enabled`, `duration-adjust.minimum-ms` | `adjust` i `Adjust Durations…` |
| `duration-adjust.maximum-enabled`, `duration-adjust.maximum-ms` | `adjust` i `Adjust Durations…` |
| `duration-adjust.gap-enabled`, `duration-adjust.gap-ms` | `adjust` i `Adjust Durations…` |
| `correction.human`, `correction.ocr` | [`correct`](correct.md) i `Correct Texts…` |
| `correction.remove-blank` | `correct` i `Correct Texts…` |
| `correction.line-break.max-lines` | `correct` i `Correct Texts…` |
| `transcription.engine`, `transcription.executable`, `transcription.model`, `transcription.language`, `transcription.suppress-notes`, `transcription.merge-gap-ms`, `transcription.max-duration-ms`, `transcription.max-characters` | [`transcribe`](transcribe.md) i `Transcribe Audio…` |
| qualsevol altra clau `correction.*` | només `Correct Texts…` — `correct` pren `--tasks`, `--code`, `--max-length`, `--skip-length`, `--skip-lines`, `--language`, `--enable` i `--disable` de la seva pròpia línia |

Els nombres de línies, `correction.line-break.max-lines` i `correction.line-break.skip-max-lines`,
són nombres enters d'1 a 1000 — els límits de `correct --max-lines` —, i les longituds de línia,
`correction.line-break.max-length` i `correction.line-break.skip-max-length`, són nombres més grans que
0: `set` refusa la resta, i un fitxer editat a mà que en contingui un conserva el valor per defecte.

`correct` pren les tasques, el codi i la longitud de línia de la seva pròpia línia expressament: en una
línia d'ordres no hi ha res marcat d'entrada, i la finestra mesura les línies en ems mentre que la línia d'ordres
compta caràcters. `correction.activations`, la llista de caselles de patrons que gestiona la finestra, no és
una clau del catàleg: `correct` té `--enable` i `--disable`.

Amb `--format json`, un objecte per clau — `key`, `description`, `accepts`, `default`, `used_by`,
i `surface`, `"both"` o `"window-only"`:

<!-- example: subedit-cli --format json config list | head -1 -->
```console
$ subedit-cli --format json config list | head -1
{"schema":1,"command":"config","key":"duration-adjust.speed","description":"Reading speed the durations are brought to.","accepts":"a number of characters per second, above 0, decimals allowed","default":"15","used_by":"subedit-cli adjust; Adjust Durations…","surface":"both"}
```

## `config show`

**El fitxer usat, i després cada clau d'operació amb el seu valor**: `File: <path> (found)`,
`(not found)` o `(unreadable: <reason>)`, i després una línia per clau —
`[file]` i el valor per defecte quan el fitxer la defineix, `[default]` quan no. Una clau que el fitxer
conté amb un valor que no pot llegir mostra el seu valor per defecte. Les preferències de visualització no es llisten.

<!-- example: subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --config mine/settings.conf config show | head -4 -->
```console
$ subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --config mine/settings.conf config show | head -4
duration-adjust.speed = 12
File: mine/settings.conf (found)
duration-adjust.speed = 12   [file]   (default 15)
duration-adjust.lengthen = true   [default]
duration-adjust.shorten = false   [default]
```

Un fitxer que falta no és un error: `show` escriu els valors per defecte, i el codi és `0` — tant el fitxer
de l'usuari com un `FILE` anomenat per `--config`, que [`adjust`](adjust.md) i [`correct`](correct.md), al
contrari, refusen (vegeu [El fitxer de configuració](invocation.md#el-fitxer-de-configuració)). Amb
`--no-config`:

<!-- example: subedit-cli --no-config config show | head -2 -->
```console
$ subedit-cli --no-config config show | head -2
File: none (--no-config)
duration-adjust.speed = 15   [default]
```

Amb `--format json`, un primer objecte `{"file", "exists"}` — `file` és `null` amb `--no-config` —
i després un objecte per clau: `key`, `value`, `source` (`"file"` o `"default"`), `default`,
`used_by`, `surface`.

<!-- example: subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --format json --config mine/settings.conf config show | head -2 -->
```console
$ subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --format json --config mine/settings.conf config show | head -2
duration-adjust.speed = 12
{"schema":1,"command":"config","file":"mine/settings.conf","exists":true}
{"schema":1,"command":"config","key":"duration-adjust.speed","value":"12","source":"file","default":"15","used_by":"subedit-cli adjust; Adjust Durations…","surface":"both"}
```

## `config set` i `config unset`

**`set` comprova el valor amb el lector propi del fitxer** — el lector que fa servir la finestra — i el
refusa quan no es pot llegir, abans d'escriure res. **`unset`** torna a escriure el valor per defecte,
en comentari: aleshores la clau segueix el valor per defecte encara que una versió posterior el canviï. Restablir una
clau que ja és al seu valor per defecte té èxit i no canvia res.

Totes dues **reescriuen el fitxer sencer i de manera atòmica**: es conserva cada preferència de visualització que ja hi era,
el bloc de les operacions va primer i el bloc de visualització en segon lloc, cadascun sota el seu encapçalament. **Un valor
que el fitxer no ha pogut llegir, i una clau desconeguda, no es conserven** — la reescriptura és la de la finestra, que
també els elimina. **Cada valor eliminat es diu**, a la sortida d'error al nivell per defecte, preferències
de visualització incloses: `<key>: unreadable value '<value>' replaced by its default`. El fitxer i
el seu directori es creen quan falten.

La confirmació va a la sortida d'error, al nivell per defecte: `KEY = VALUE` per a `set`,
`KEY reset to its default (DEFAULT)` per a `unset`.

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

## Codis de sortida

| Codi | Significat |
| :--- | :------ |
| `0` | fet — inclosos `show` sobre un fitxer que falta i `unset` sobre una clau al seu valor per defecte |
| `1` | error d'ús: cap subordre, un argument que falta, una clau o un valor refusats, `--no-config` amb `set` o `unset`, `--config` amb un nom buit, cap fitxer de l'usuari i cap `--config`; no s'escriu res |
| `2` | el fitxer no es pot llegir o escriure; no s'escriu res |

## Errors

| Què el provoca | Què s'escriu, a la sortida d'error |
| :--------------- | :------------------------------------- |
| una preferència de visualització | `<key> is a display preference; set it in the window` |
| una clau desconeguda | `unknown setting <key>; subedit-cli config list names them` |
| un valor que la clau no accepta | `cannot read '<value>' for <key>; subedit-cli config list says what it accepts` |
| `correction.activations` | `correction.activations is managed by the window; use --enable/--disable on correct` |
| `set` o `unset` amb `--no-config` | `--no-config has nothing to write to` |
| no es pot localitzar cap fitxer de l'usuari, i no hi ha `--config` | `no settings file could be located; give one with --config FILE` |
| `--config` amb `--no-config` | `--config excludes --no-config` |
| `--config` amb un nom buit | `--config needs a file name` |
| un fitxer que existeix i no es pot llegir, per a `set` o `unset` | `<path>: cannot be read; nothing written` |
| un fitxer que no es pot escriure | `<path>: cannot be written`, o `<path>: cannot be written: permission denied` |

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
