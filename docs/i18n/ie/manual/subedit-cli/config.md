# `config`

<!-- machine-translated -->
> Ca págine es un traduction machinal e it ancor ne ha esset revidet de un lector del lingue.

```
subedit-cli [--config FILE | --no-config] [--format text|json] config list
subedit-cli [--config FILE | --no-config] [--format text|json] config show
subedit-cli [--config FILE] config set KEY VALUE
subedit-cli [--config FILE] config unset KEY
```

Lista, monstra e changea li **parametres de operation**: li parte del file de configuration quel li
fenestre e li linea de comande partisse. `Adjust Durations…` e `Correct Texts…` memora lor
valores ibi, e [`adjust`](adjust.md) e [`correct`](correct.md) les lee per predefinit — vide
[Li file de configuration](invocation.md#li-file-de-configuration).

**Quo it ne fa:** it nequande lee ni scri un **preferentie de monstrada** (li
geometrie del fenestre, columnes, tema, optiones de sercha, ortografie, video…). Ti resta li
afere del fenestre, fixat in li fenestre e descrit in [su manual](../subedit-gui/preferences.md#li-file).
It nequande tucha anc un file de subtitules.

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

## Subcomandes

| Subcomande | Quo it fa | File |
| :---------- | :----------- | :--- |
| `config list` | scri chascun clave de operation, con su usage, quo it acceptas, su valore predefinit e qui l'usa — **null valore** | lee null |
| `config show` | scri li file usat e si it existe, poy chascun clave de operation con su valore, ex u li valore veni, e su valore predefinit | lee it |
| `config set KEY VALUE` | controla `VALUE`, poy scri denov li file con `KEY = VALUE`, conservante chascun altri parametre | lee e scri it |
| `config unset KEY` | remette `KEY` a su valore predefinit, scrit in comentario quam li file sempre scri un valore predefinit | lee e scri it |

| Option | Obligatori | Valores | Predefinit |
| :----- | :------- | :----- | :------ |
| `KEY` | yes, por `set` e `unset` | un clave de operation, quam `config list` nómina it | — |
| `VALUE` | yes, por `set` | quo `config list` di que li clave acceptas | — |

`config` have null option propri. Li optiones global scrit **ante** `config` selecte li
file e li forme del output:

| Option global | Efecte sur `config` |
| :------------ | :----------------- |
| `--config FILE` | `show`, `set` e `unset` actua sur `FILE` vice li file del usator, mem si it ne existe ancor; `list` ignora it |
| `--no-config` | `list` functiona quam sempre; `show` monstra li valores predefinit, con `File: none (--no-config)`; `set` e `unset` es refusat: ne ha file in quel scrir |
| `--format json` | `list` e `show` scri un objecte JSON per linea; `set` e `unset` scri nullcos sur li output standard |
| `-q`, `--quiet` | `set` e `unset` ne confirma plu quo ili ha scrit |

## Qual file

Sin `--config`, li file del usator:

```
$XDG_CONFIG_HOME/subedit/settings.conf
```

o `~/.config/subedit/settings.conf` quande `XDG_CONFIG_HOME` ne es definit, es vacui o ne es un via
absolut. It es li file quel li fenestre lee e scri; `config show` scri su via in su unesim linea:

<!-- example: XDG_CONFIG_HOME=/home/vous/.config subedit-cli config show | head -1 -->
```console
$ XDG_CONFIG_HOME=/home/vous/.config subedit-cli config show | head -1
File: /home/vous/.config/subedit/settings.conf (not found)
```

Quande ni `XDG_CONFIG_HOME` ni `HOME` dona un loc, null file del usator existe: `show`, `set`
e `unset` es refusat, e `--config FILE` nómina un.

## `config list`

**Li catalog del claves de operation**, in li órdine del file: chascun clave, poy quar lineas
indentat — su usage, `Accepts:` (li aggregat clausi, o li forme e li limites), `Default:` e
`Used by:` (li comandes `subedit-cli` e li actiones del fenestre queles lee it). It lee null file e
besona null, e functiona con `--no-config`.

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

It lista **40 claves**:

<!-- example: subedit-cli config list | grep -c '^[a-z]' -->
```console
$ subedit-cli config list | grep -c '^[a-z]'
40
```

**Vint de ili es leet per li linea de comande**; li vinte altres es leet solmen del fenestre,
e lor linea `Used by:` di it, con li option de `correct` quel prende lor loc:

| Clave | Leet per |
| :-- | :------ |
| `duration-adjust.speed`, `duration-adjust.lengthen`, `duration-adjust.shorten` | [`adjust`](adjust.md) e `Adjust Durations…` |
| `duration-adjust.minimum-enabled`, `duration-adjust.minimum-ms` | `adjust` e `Adjust Durations…` |
| `duration-adjust.maximum-enabled`, `duration-adjust.maximum-ms` | `adjust` e `Adjust Durations…` |
| `duration-adjust.gap-enabled`, `duration-adjust.gap-ms` | `adjust` e `Adjust Durations…` |
| `correction.human`, `correction.ocr` | [`correct`](correct.md) e `Correct Texts…` |
| `correction.remove-blank` | `correct` e `Correct Texts…` |
| `correction.line-break.max-lines` | `correct` e `Correct Texts…` |
| `transcription.engine`, `transcription.executable`, `transcription.model`, `transcription.language`, `transcription.merge-gap-ms`, `transcription.max-duration-ms`, `transcription.max-characters` | [`transcribe`](transcribe.md) e `Transcribe Audio…` |
| chascun altri clave `correction.*` | `Correct Texts…` solmen — `correct` prende `--tasks`, `--code`, `--max-length`, `--skip-length`, `--skip-lines`, `--language`, `--enable` e `--disable` de su propri linea |

Li númeres de lineas, `correction.line-break.max-lines` e `correction.line-break.skip-max-lines`,
es númeres integri de 1 til 1000 — li limites de `correct --max-lines` —, e li longes de linea,
`correction.line-break.max-length` e `correction.line-break.skip-max-length`, es númeres supra
0: `set` refusa li resta, e un file redactet per manu quel contene un conserva li valore predefinit.

`correct` prende li taches, li code e li long de linea de su propri linea intentionalmen: nullcos
es marcat ante sur un linea de comande, e li fenestre mesura lineas in ems u li linea de comande
cunta characteres. `correction.activations`, li liste de casses de modelles quel li fenestre gere, ne es
un clave del catalog: `correct` have `--enable` e `--disable`.

Con `--format json`, un objecte per clave — `key`, `description`, `accepts`, `default`, `used_by`,
e `surface`, `"both"` o `"window-only"`:

<!-- example: subedit-cli --format json config list | head -1 -->
```console
$ subedit-cli --format json config list | head -1
{"schema":1,"command":"config","key":"duration-adjust.speed","description":"Reading speed the durations are brought to.","accepts":"a number of characters per second, above 0, decimals allowed","default":"15","used_by":"subedit-cli adjust; Adjust Durations…","surface":"both"}
```

## `config show`

**Li file usat, poy chascun clave de operation con su valore**: `File: <path> (found)`,
`(not found)` o `(unreadable: <reason>)`, poy un linea per clave —
`[file]` e li valore predefinit quande li file fixa it, `[default]` quande it ne fixa it. Un clave quel li file
contene con un valore quel it ne posse leer monstra su valore predefinit. Li preferenties de monstrada ne es listat.

<!-- example: subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --config mine/settings.conf config show | head -4 -->
```console
$ subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --config mine/settings.conf config show | head -4
duration-adjust.speed = 12
File: mine/settings.conf (found)
duration-adjust.speed = 12   [file]   (default 15)
duration-adjust.lengthen = true   [default]
duration-adjust.shorten = false   [default]
```

Un file mancant ne es un error: `show` scri li valores predefinit, e li code es `0` — li file del
usator quam un `FILE` nominat per `--config`, quel [`adjust`](adjust.md) e [`correct`](correct.md), in contrari,
refusa (vide [Li file de configuration](invocation.md#li-file-de-configuration)). Con
`--no-config`:

<!-- example: subedit-cli --no-config config show | head -2 -->
```console
$ subedit-cli --no-config config show | head -2
File: none (--no-config)
duration-adjust.speed = 15   [default]
```

Con `--format json`, un unesim objecte `{"file", "exists"}` — `file` es `null` con `--no-config` —
poy un objecte per clave: `key`, `value`, `source` (`"file"` o `"default"`), `default`,
`used_by`, `surface`.

<!-- example: subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --format json --config mine/settings.conf config show | head -2 -->
```console
$ subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --format json --config mine/settings.conf config show | head -2
duration-adjust.speed = 12
{"schema":1,"command":"config","file":"mine/settings.conf","exists":true}
{"schema":1,"command":"config","key":"duration-adjust.speed","value":"12","source":"file","default":"15","used_by":"subedit-cli adjust; Adjust Durations…","surface":"both"}
```

## `config set` e `config unset`

**`set` controla li valore con li propri leetor del file** — li leetor quel li fenestre usa — e refusa
it quande it ne posse esser leet, ante que alquo es scrit. **`unset`** scri li valore predefinit denov,
in comentario: li clave seque alor li valore predefinit mem si un version posteriori changea it. Desfixar un
clave ja a su valore predefinit successa e changea nullcos.

Ambi **scri denov li file integrimen e atomicmen**: chascun preferentie de monstrada ja in it es conservat,
li bloc de operation veni unesim e li bloc de monstrada duesim, chascun sub su banere. **Un valore
quel li file ne posse leer, e un clave inconosset, ne es conservat** — li reescrition es celi del fenestre, quel
les omisse anc. **Chascun valore omisset es dit**, sur li error standard al nivelle predefinit, preferenties de monstrada
includet: `<key>: unreadable value '<value>' replaced by its default`. Li file e
su directoria es creat quande mancant.

Li confirmation va al error standard, al nivelle predefinit: `KEY = VALUE` por `set`,
`KEY reset to its default (DEFAULT)` por `unset`.

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

## Codes de exeunte

| Code | Signification |
| :--- | :------ |
| `0` | fat — includent `show` sur un file mancant e `unset` sur un clave a su valore predefinit |
| `1` | error de usage: null subcomande, un argument mancant, un clave o valore refusat, `--no-config` con `set` o `unset`, `--config` con un nómine vacui, null file del usator e null `--config`; nullcos es scrit |
| `2` | li file ne posse esser leet o scrit; nullcos es scrit |

## Errores

| Quo it causa | Quo es scrit, sur li error standard |
| :--------------- | :------------------------------------- |
| un preferentie de monstrada | `<key> is a display preference; set it in the window` |
| un clave inconosset | `unknown setting <key>; subedit-cli config list names them` |
| un valore quel li clave ne acceptas | `cannot read '<value>' for <key>; subedit-cli config list says what it accepts` |
| `correction.activations` | `correction.activations is managed by the window; use --enable/--disable on correct` |
| `set` o `unset` con `--no-config` | `--no-config has nothing to write to` |
| null file del usator posse esser localisat, e null `--config` | `no settings file could be located; give one with --config FILE` |
| `--config` con `--no-config` | `--config excludes --no-config` |
| `--config` con un nómine vacui | `--config needs a file name` |
| un file quel existe e ne posse esser leet, por `set` o `unset` | `<path>: cannot be read; nothing written` |
| un file quel ne posse esser scrit | `<path>: cannot be written`, o `<path>: cannot be written: permission denied` |

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
