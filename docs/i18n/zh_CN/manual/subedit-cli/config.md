# `config`

<!-- machine-translated -->
> 本页为机器翻译，尚未经过熟悉该语言的读者审校。

```
subedit-cli [--config FILE | --no-config] [--format text|json] config list
subedit-cli [--config FILE | --no-config] [--format text|json] config show
subedit-cli [--config FILE] config set KEY VALUE
subedit-cli [--config FILE] config unset KEY
```

列出、显示和更改**操作设置**：即设置文件中由窗口和命令行共用的部分。`Adjust Durations…` 和 `Correct Texts…` 会把各自的取值记在这里，而 [`adjust`](adjust.md) 和 [`correct`](correct.md) 默认读取这些取值——参见[设置文件](invocation.md#设置文件)。

**它不做的事：** 它从不读取也从不写入**显示首选项**（窗口的位置和大小、列、主题、搜索选项、拼写检查、视频等）。这些始终由窗口负责，在窗口中设置，并在[其手册](../subedit-gui/preferences.md#文件)中说明。它也从不触动字幕文件。

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

## 子命令

| 子命令 | 作用 | 文件 |
| :---------- | :----------- | :--- |
| `config list` | 写出每个操作键，以及它的用途、可接受的取值、默认值和使用者——**不含取值** | 不读取 |
| `config show` | 写出所用的文件及其是否存在，然后写出每个操作键及其取值、取值的来源和默认值 | 读取 |
| `config set KEY VALUE` | 检查 `VALUE`，然后以 `KEY = VALUE` 重写文件，其余设置保持不变 | 读取并写入 |
| `config unset KEY` | 让 `KEY` 恢复为默认值，写成注释掉的形式，与文件写入默认值时的做法一致 | 读取并写入 |

| 选项 | 必需 | 取值 | 默认值 |
| :----- | :------- | :----- | :------ |
| `KEY` | `set` 和 `unset` 必需 | 一个操作键，写法与 `config list` 所列一致 | — |
| `VALUE` | `set` 必需 | `config list` 中说明该键可接受的内容 | — |

`config` 本身没有选项。写在 `config` **之前**的全局选项决定所用的文件和输出的形式：

| 全局选项 | 对 `config` 的影响 |
| :------------ | :----------------- |
| `--config FILE` | `show`、`set` 和 `unset` 作用于 `FILE`，而不是用户的文件，即使该文件尚不存在；`list` 忽略它 |
| `--no-config` | `list` 照常工作；`show` 显示默认值，并给出 `File: none (--no-config)`；`set` 和 `unset` 会被拒绝：没有可写入的文件 |
| `--format json` | `list` 和 `show` 每行写出一个 JSON 对象；`set` 和 `unset` 在标准输出中不写任何内容 |
| `-q`, `--quiet` | `set` 和 `unset` 不再确认写入了什么 |

## 使用哪个文件

不带 `--config` 时，使用用户的文件：

```
$XDG_CONFIG_HOME/subedit/settings.conf
```

如果 `XDG_CONFIG_HOME` 未设置、为空或不是绝对路径，则使用 `~/.config/subedit/settings.conf`。这就是窗口读写的文件；`config show` 在第一行写出它的路径：

<!-- example: XDG_CONFIG_HOME=/home/vous/.config subedit-cli config show | head -1 -->
```console
$ XDG_CONFIG_HOME=/home/vous/.config subedit-cli config show | head -1
File: /home/vous/.config/subedit/settings.conf (not found)
```

如果 `XDG_CONFIG_HOME` 和 `HOME` 都给不出位置，就没有用户的文件：`show`、`set` 和 `unset` 会被拒绝，此时应使用 `--config FILE` 指定一个文件。

## `config list`

**操作键的目录**，顺序与文件中一致：先是每个键，然后是四行缩进的内容——用途、`Accepts:`（封闭集合，或形式和界限）、`Default:` 和 `Used by:`（读取该键的 `subedit-cli` 命令和窗口操作）。它不读取也不需要任何文件，使用 `--no-config` 时同样可以工作。

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

它列出 **33 个键**：

<!-- example: subedit-cli config list | grep -c '^[a-z]' -->
```console
$ subedit-cli config list | grep -c '^[a-z]'
33
```

**其中十三个由命令行读取**；其余二十个只由窗口读取，它们的 `Used by:` 行会这样说明，并给出取代它们的 `correct` 选项：

| 键 | 读取者 |
| :-- | :------ |
| `duration-adjust.speed`, `duration-adjust.lengthen`, `duration-adjust.shorten` | [`adjust`](adjust.md) 和 `Adjust Durations…` |
| `duration-adjust.minimum-enabled`, `duration-adjust.minimum-ms` | `adjust` 和 `Adjust Durations…` |
| `duration-adjust.maximum-enabled`, `duration-adjust.maximum-ms` | `adjust` 和 `Adjust Durations…` |
| `duration-adjust.gap-enabled`, `duration-adjust.gap-ms` | `adjust` 和 `Adjust Durations…` |
| `correction.human`, `correction.ocr` | [`correct`](correct.md) 和 `Correct Texts…` |
| `correction.remove-blank` | `correct` 和 `Correct Texts…` |
| `correction.line-break.max-lines` | `correct` 和 `Correct Texts…` |
| 其余所有 `correction.*` 键 | 只有 `Correct Texts…`——`correct` 从自己的命令行取任务（`--tasks`）、代码（`--code`）、最大长度（`--max-length`）、跳过的长度（`--skip-length`）、跳过的行数（`--skip-lines`）、语言（`--language`）以及 `--enable` 和 `--disable` |

行数 `correction.line-break.max-lines` 和 `correction.line-break.skip-max-lines` 是 1 到 1000 的整数——也就是 `correct --max-lines` 的界限；行长度 `correction.line-break.max-length` 和 `correction.line-break.skip-max-length` 是大于 0 的数：`set` 拒绝其他取值，而手工编辑后含有这类取值的文件，则保留默认值。

`correct` 特意从自己的命令行取任务、代码和行长度：命令行上不会事先勾选任何内容，而且窗口以 em 为单位测量行，命令行则按字符计数。`correction.activations` 是窗口管理的模式复选框列表，它不是目录中的键：`correct` 有 `--enable` 和 `--disable`。

使用 `--format json` 时，每个键对应一个对象——`key`、`description`、`accepts`、`default`、`used_by`，以及 `surface`（`"both"` 或 `"window-only"`）：

<!-- example: subedit-cli --format json config list | head -1 -->
```console
$ subedit-cli --format json config list | head -1
{"schema":1,"command":"config","key":"duration-adjust.speed","description":"Reading speed the durations are brought to.","accepts":"a number of characters per second, above 0, decimals allowed","default":"15","used_by":"subedit-cli adjust; Adjust Durations…","surface":"both"}
```

## `config show`

**所用的文件，然后是每个操作键及其取值**：`File: <path> (found)`、`(not found)` 或 `(unreadable: <reason>)`，然后每个键一行——文件设置了该键时，给出 `[file]` 和默认值；没有设置时，给出 `[default]`。文件中某个键的取值无法读取时，显示其默认值。显示首选项不会列出。

<!-- example: subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --config mine/settings.conf config show | head -4 -->
```console
$ subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --config mine/settings.conf config show | head -4
duration-adjust.speed = 12
File: mine/settings.conf (found)
duration-adjust.speed = 12   [file]   (default 15)
duration-adjust.lengthen = true   [default]
duration-adjust.shorten = false   [default]
```

文件不存在不是错误：`show` 写出默认值，退出码为 `0`——用户的文件也可以作为 `--config` 所指定的 `FILE`，而 [`adjust`](adjust.md) 和 [`correct`](correct.md) 则相反，会拒绝这样的文件（参见[设置文件](invocation.md#设置文件)）。使用 `--no-config` 时：

<!-- example: subedit-cli --no-config config show | head -2 -->
```console
$ subedit-cli --no-config config show | head -2
File: none (--no-config)
duration-adjust.speed = 15   [default]
```

使用 `--format json` 时，先输出第一个对象 `{"file", "exists"}`——使用 `--no-config` 时 `file` 为 `null`——然后每个键一个对象：`key`、`value`、`source`（`"file"` 或 `"default"`）、`default`、`used_by`、`surface`。

<!-- example: subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --format json --config mine/settings.conf config show | head -2 -->
```console
$ subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --format json --config mine/settings.conf config show | head -2
duration-adjust.speed = 12
{"schema":1,"command":"config","file":"mine/settings.conf","exists":true}
{"schema":1,"command":"config","key":"duration-adjust.speed","value":"12","source":"file","default":"15","used_by":"subedit-cli adjust; Adjust Durations…","surface":"both"}
```

## `config set` 与 `config unset`

**`set` 用文件自己的读取器检查取值**——即窗口使用的读取器——如果取值无法读取，就会在写入任何内容之前拒绝它。**`unset`** 把默认值以注释掉的形式写回：这样，即使以后的版本更改了默认值，该键也会跟随默认值。对已经是默认值的键执行 unset 会成功，且不作任何更改。

两者都会**整体且原子地重写文件**：文件中已有的所有显示首选项都会保留，先写操作区块，再写显示区块，各有自己的标题注释。**文件无法读取的取值，以及未知的键，不会保留**——重写由窗口负责，窗口同样会丢弃它们。**每一个被丢弃的取值都会说明**，在默认级别写到标准错误，显示首选项也包括在内：`<key>: unreadable value '<value>' replaced by its default`。文件及其目录不存在时会创建。

确认信息在默认级别写到标准错误：`set` 为 `KEY = VALUE`，`unset` 为 `KEY reset to its default (DEFAULT)`。

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

## 退出码

| 退出码 | 含义 |
| :--- | :------ |
| `0` | 完成——包括对不存在的文件执行 `show`，以及对已是默认值的键执行 `unset` |
| `1` | 用法错误：没有子命令、缺少参数、键或取值被拒绝、`--no-config` 与 `set` 或 `unset` 同用、`--config` 的名称为空、既没有用户的文件也没有 `--config`；不写入任何内容 |
| `2` | 文件无法读取或写入；不写入任何内容 |

## 错误

| 触发条件 | 在标准错误中写出的内容 |
| :--------------- | :------------------------------------- |
| 显示首选项 | `<key> is a display preference; set it in the window` |
| 未知的键 | `unknown setting <key>; subedit-cli config list names them` |
| 该键不接受的取值 | `cannot read '<value>' for <key>; subedit-cli config list says what it accepts` |
| `correction.activations` | `correction.activations is managed by the window; use --enable/--disable on correct` |
| `set` 或 `unset` 与 `--no-config` 同用 | `--no-config has nothing to write to` |
| 找不到用户的文件，且没有 `--config` | `no settings file could be located; give one with --config FILE` |
| `--config` 与 `--no-config` 同用 | `--config excludes --no-config` |
| `--config` 的名称为空 | `--config needs a file name` |
| 对 `set` 或 `unset` 而言，文件存在但无法读取 | `<path>: cannot be read; nothing written` |
| 文件无法写入 | `<path>: cannot be written`，或 `<path>: cannot be written: permission denied` |

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
