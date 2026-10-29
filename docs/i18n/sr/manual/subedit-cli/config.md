# `config`

<!-- machine-translated -->
> Ова страница је машински превод који још није прегледао читалац језика.

```
subedit-cli [--config FILE | --no-config] [--format text|json] config list
subedit-cli [--config FILE | --no-config] [--format text|json] config show
subedit-cli [--config FILE] config set KEY VALUE
subedit-cli [--config FILE] config unset KEY
```

Исписује, приказује и мења **поставке операција**: део датотеке поставки који
прозор и командна линија деле. `Adjust Durations…` и `Correct Texts…` памте своје
вредности ту, а подкоманде [`adjust`](adjust.md) и [`correct`](correct.md) их подразумевано читају — види
[Датотека поставки](invocation.md#датотека-поставки).

**Шта не ради:** никада не чита нити пише **поставку приказа** (геометрију прозора,
ступце, тему, опције претраге, проверу писања, филм…). Оне остају посао прозора,
постављају се у прозору и описане су у [његовом приручнику](../subedit-gui/preferences.md#датотека).
Не дира ни датотеку титлова.

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

## Подкоманде

| Подкоманда | Шта ради | Датотека |
| :---------- | :----------- | :--- |
| `config list` | исписује сваки кључ операције, чему служи, шта прихвата, његову подразумевану вредност и ко га користи — **без вредности** | не чита ниједну |
| `config show` | исписује коришћену датотеку и да ли постоји, затим сваки кључ операције са његовом вредношћу, одакле вредност долази и његовом подразумеваном вредношћу | чита је |
| `config set KEY VALUE` | проверава `VALUE`, затим поново записује датотеку са `KEY = VALUE`, задржавајући све остале поставке | чита је и пише је |
| `config unset KEY` | враћа `KEY` на подразумевану вредност, записану као коментар, јер датотека увек тако записује подразумевану вредност | чита је и пише је |

| Опција | Обавезно | Вредности | Подразумевано |
| :----- | :------- | :----- | :------ |
| `KEY` | да, за `set` и `unset` | кључ операције, онако како га `config list` именује | — |
| `VALUE` | да, за `set` | оно што `config list` каже да кључ прихвата | — |

`config` нема своје опције. Опште опције писане **пре** `config` бирају
датотеку и облик излаза:

| Општа опција | Дејство на `config` |
| :------------ | :----------------- |
| `--config FILE` | `show`, `set` и `unset` раде над датотеком `FILE` уместо корисникове, чак и када још не постоји; `list` је занемарује |
| `--no-config` | `list` ради као и обично; `show` приказује подразумеване вредности, са `File: none (--no-config)`; `set` и `unset` се одбијају: нема датотеке у коју би писали |
| `--format json` | `list` и `show` пишу по један JSON објекат у реду; `set` и `unset` не пишу ништа на стандардни излаз |
| `-q`, `--quiet` | `set` и `unset` више не потврђују шта су записали |

## Која датотека

Без `--config`, корисникова датотека:

```
$XDG_CONFIG_HOME/subedit/settings.conf
```

или `~/.config/subedit/settings.conf` када `XDG_CONFIG_HOME` није постављен, празан је или није апсолутна
путања. То је датотека коју прозор чита и пише; `config show` исписује њену путању у првом реду:

<!-- example: XDG_CONFIG_HOME=/home/vous/.config subedit-cli config show | head -1 -->
```console
$ XDG_CONFIG_HOME=/home/vous/.config subedit-cli config show | head -1
File: /home/vous/.config/subedit/settings.conf (not found)
```

Када ни `XDG_CONFIG_HOME` ни `HOME` не дају место, нема корисникове датотеке: `show`, `set`
и `unset` се одбијају, а `--config FILE` именује једну.

## `config list`

**Каталог кључева операција**, редом којим су у датотеци: сваки кључ, па четири увучена
реда — чему служи, `Accepts:` (затворен скуп, или облик и границе), `Default:` и
`Used by:` (команде `subedit-cli` и радње прозора које га читају). Не чита датотеку и
не тражи је, а ради и уз `--no-config`.

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

Исписује **45 кључева**:

<!-- example: subedit-cli config list | grep -c '^[a-z]' -->
```console
$ subedit-cli config list | grep -c '^[a-z]'
45
```

**Двадесет и пет од њих чита командна линија**; осталих двадесет чита само прозор,
и њихов ред `Used by:` то каже, уз опцију подкоманде `correct` која их замењује:

| Кључ | Ко чита |
| :-- | :------ |
| `duration-adjust.speed`, `duration-adjust.lengthen`, `duration-adjust.shorten` | [`adjust`](adjust.md) и `Adjust Durations…` |
| `duration-adjust.minimum-enabled`, `duration-adjust.minimum-ms` | `adjust` и `Adjust Durations…` |
| `duration-adjust.maximum-enabled`, `duration-adjust.maximum-ms` | `adjust` и `Adjust Durations…` |
| `duration-adjust.gap-enabled`, `duration-adjust.gap-ms` | `adjust` и `Adjust Durations…` |
| `correction.human`, `correction.ocr` | [`correct`](correct.md) и `Correct Texts…` |
| `correction.remove-blank` | `correct` и `Correct Texts…` |
| `correction.line-break.max-lines` | `correct` и `Correct Texts…` |
| `transcription.engine`, `transcription.executable`, `transcription.model`, `transcription.vad-model`, `transcription.language`, `transcription.suppress-notes`, `transcription.merge-gap-ms`, `transcription.max-duration-ms`, `transcription.max-characters` | [`transcribe`](transcribe.md) и `Transcribe Audio…` |
| `alignment.engine`, `alignment.min-match`, `alignment.doubt-gap` | [`sync`](sync.md) |
| сваки други кључ `correction.*` | само `Correct Texts…` — `correct` узима `--tasks`, `--code`, `--max-length`, `--skip-length`, `--skip-lines`, `--language`, `--enable` и `--disable` из своје линије |

Бројеви редова, `correction.line-break.max-lines` и `correction.line-break.skip-max-lines`,
су цели бројеви од 1 до 1000 — границе опције `correct --max-lines` —, а дужине редова,
`correction.line-break.max-length` и `correction.line-break.skip-max-length`, су бројеви већи од
0: `set` одбија све остало, а датотека уређена ручно која садржи такву вредност задржава подразумевану.

Подкоманда `correct` задатке, код и дужину реда узима из своје линије намерно: на командној линији
ништа није унапред означено, а прозор мери редове у емовима тамо где командна линија
броји знакове. `correction.activations`, списак поља образаца којим управља прозор, није
кључ каталога: `correct` има `--enable` и `--disable`.

Уз `--format json`, један објекат по кључу — `key`, `description`, `accepts`, `default`, `used_by`
и `surface`, `"both"` или `"window-only"`:

<!-- example: subedit-cli --format json config list | head -1 -->
```console
$ subedit-cli --format json config list | head -1
{"schema":1,"command":"config","key":"duration-adjust.speed","description":"Reading speed the durations are brought to.","accepts":"a number of characters per second, above 0, decimals allowed","default":"15","used_by":"subedit-cli adjust; Adjust Durations…","surface":"both"}
```

## `config show`

**Коришћена датотека, затим сваки кључ операције са својом вредношћу**: `File: <path> (found)`,
`(not found)` или `(unreadable: <reason>)`, затим један ред по кључу —
`[file]` и подразумевана вредност када је датотека поставља, `[default]` када не поставља. Кључ који
датотека садржи са вредношћу коју не може да прочита приказује своју подразумевану вредност. Поставке приказа се не наводе.

<!-- example: subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --config mine/settings.conf config show | head -4 -->
```console
$ subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --config mine/settings.conf config show | head -4
duration-adjust.speed = 12
File: mine/settings.conf (found)
duration-adjust.speed = 12   [file]   (default 15)
duration-adjust.lengthen = true   [default]
duration-adjust.shorten = false   [default]
```

Датотека која недостаје није грешка: `show` исписује подразумеване вредности, а код је `0` — корисникова
датотека као `FILE` који именује `--config`, што [`adjust`](adjust.md) и [`correct`](correct.md), напротив, одбијају (види [Датотека поставки](invocation.md#датотека-поставки)). Уз
`--no-config`:

<!-- example: subedit-cli --no-config config show | head -2 -->
```console
$ subedit-cli --no-config config show | head -2
File: none (--no-config)
duration-adjust.speed = 15   [default]
```

Уз `--format json`, први објекат `{"file", "exists"}` — `file` је `null` уз `--no-config` —
затим један објекат по кључу: `key`, `value`, `source` (`"file"` или `"default"`), `default`,
`used_by`, `surface`.

<!-- example: subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --format json --config mine/settings.conf config show | head -2 -->
```console
$ subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --format json --config mine/settings.conf config show | head -2
duration-adjust.speed = 12
{"schema":1,"command":"config","file":"mine/settings.conf","exists":true}
{"schema":1,"command":"config","key":"duration-adjust.speed","value":"12","source":"file","default":"15","used_by":"subedit-cli adjust; Adjust Durations…","surface":"both"}
```

## `config set` и `config unset`

**`set` проверава вредност читачем саме датотеке** — оним који користи прозор — и одбија
је кад не може да се прочита, пре него што се било шта запише. **`unset`** враћа подразумевану вредност,
као коментар: кључ тада прати подразумевану вредност чак и ако је каснија верзија промени. Уклањање вредности
кључа који је већ на подразумеваној вредности успева и ништа не мења.

Обе **поново записују целу датотеку и то атомски**: свака поставка приказа која је већ у њој се задржава,
блок операција долази први, а блок приказа други, сваки под својим натписом. **Вредност
коју датотека није могла да прочита и непознат кључ се не задржавају** — поновно записивање је оно из прозора, које
их такође одбацује. **Свака одбачена вредност се јавља**, на стандардној грешци на подразумеваном нивоу, укључујући
поставке приказа: `<key>: unreadable value '<value>' replaced by its default`. Датотека и
њен директоријум се стварају кад недостају.

Потврда иде на стандардну грешку, на подразумеваном нивоу: `KEY = VALUE` за `set`,
`KEY reset to its default (DEFAULT)` за `unset`.

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

## Излазни кодови

| Код | Значење |
| :--- | :------ |
| `0` | урађено — укључујући `show` над датотеком која недостаје и `unset` над кључем на подразумеваној вредности |
| `1` | грешка у употреби: нема подкоманде, недостаје аргумент, одбијен кључ или вредност, `--no-config` уз `set` или `unset`, `--config` са празним именом, нема корисникове датотеке и нема `--config`; ништа се не пише |
| `2` | датотека не може да се прочита или запише; ништа се не пише |

## Грешке

| Шта је изазива | Шта се пише на стандардну грешку |
| :--------------- | :------------------------------------- |
| поставка приказа | `<key> is a display preference; set it in the window` |
| непознат кључ | `unknown setting <key>; subedit-cli config list names them` |
| вредност коју кључ не прихвата | `cannot read '<value>' for <key>; subedit-cli config list says what it accepts` |
| `correction.activations` | `correction.activations is managed by the window; use --enable/--disable on correct` |
| `set` или `unset` уз `--no-config` | `--no-config has nothing to write to` |
| не може да се пронађе корисникова датотека, а нема `--config` | `no settings file could be located; give one with --config FILE` |
| `--config` уз `--no-config` | `--config excludes --no-config` |
| `--config` са празним именом | `--config needs a file name` |
| датотека која постоји а не може да се прочита, за `set` или `unset` | `<path>: cannot be read; nothing written` |
| датотека која не може да се запише | `<path>: cannot be written`, или `<path>: cannot be written: permission denied` |

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
