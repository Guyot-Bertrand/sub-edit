# 调用

<!-- machine-translated -->
> 本页为机器翻译，尚未经过熟悉该语言的读者审校。

```
subedit-cli [global options] <subcommand> [options] <file>...
```

不带子命令时，本工具会输出帮助并停止。

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

## 语言

命令行使用其所在环境的语言：**帮助、说明信息和错误消息**都遵循所要求的语言。本手册中的示例是用英语运行的——这些区块由 `make manual` 生成，绝不手工复制。

**语言如何选定。**`LC_ALL`、`LC_MESSAGES` 和 `LANG` 中第一个已设置且不为空的变量就是区域设置；如果设置了 `LANGUAGE`，它会以冒号分隔列出所需的语言（`LANGUAGE=pt_BR:fr`），并且最先被读取。这是 gettext 遵循的顺序，由程序自己读取：不需要在机器上生成该区域设置。

| 规则 | 效果 |
| :--- | :----- |
| 编码和修饰符 | 忽略：`fr_FR.UTF-8@euro` 先查找 `fr_FR`，再查找 `fr` |
| `LC_ALL=C`、`C` 或 `POSIX` 区域设置，或完全没有区域设置 | 英语，并且不读取 `LANGUAGE` |
| 指明英语的条目（`en`、`en_GB`） | 英语：`LANGUAGE` 中排在它之后的内容不再读取 |
| 未安装翻译目录的语言 | 改用所要求的下一种语言，在最后一种之后改用英语，**不作任何提示** |
| 翻译目录中没有翻译的消息 | 以英语显示 |

当翻译目录收录了纠正模式的名称时，`correct` 声明无法应用的**纠正模式名称**同样遵循语言——这指的是随程序提供的模式；用户自己的模式则按其文件中的写法命名。模式文件本身从不被改写。

翻译目录安装在 `<prefix>/share/subedit/locale/<language>/LC_MESSAGES/subedit.mo` 之下，并从可执行文件自身所在的目录中查找。英语是内置的。此版本安装了 `ca`、`cs`、`de`、`es`、`fi`、`fr`、`gl`、`he`、`hu`、`ie`、`is`、`it_IT`、`nl`、`pl`、`pt`、`pt_BR`、`ru`、`sr`、`tr` 和 `zh_CN`——即加泰罗尼亚语、捷克语、德语、西班牙语、芬兰语、法语、加利西亚语、希伯来语、匈牙利语、西方国际语、冰岛语、意大利语、荷兰语、波兰语、葡萄牙语、巴西葡萄牙语、俄语、塞尔维亚语、土耳其语和简体中文——已全部翻译。法语已经由熟悉该语言的读者审校；加泰罗尼亚语、捷克语、德语、西班牙语、芬兰语、加利西亚语、希伯来语、匈牙利语、西方国际语、冰岛语、意大利语、荷兰语、波兰语、葡萄牙语、巴西葡萄牙语、俄语、塞尔维亚语、土耳其语和简体中文为机器翻译，尚未经过熟悉该语言的读者审校。
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
<!-- machine-translated: ru -->
<!-- machine-translated: sr -->
<!-- machine-translated: tr -->
<!-- machine-translated: zh_CN -->
在 `LANG=fr_FR.UTF-8` 下，`subedit-cli --help` 以“Lire, inspecter et recaler des fichiers de sous-titres.”开头。需要读取文本输出中文字的脚本应设置 `LANGUAGE=en`（或 `LC_ALL=C`）——或者读取始终不变的 `--format json`。

**始终不随语言变化的内容**——脚本所读取的部分：

| 从不翻译的内容 | 原因 |
| :--------------- | :-- |
| `--format json` 的输出：键、值，以及失败和警告的 `message` 与 `detail` | 各语言下字节完全相同；文字为英语 |
| 退出码 | 即[退出码](#退出码)中的那四个，在任何语言下都不变 |
| 键入的名称：子命令、选项及其取值（`off`、`srt`）、元变量（`FILE`、`TEXT`、`OPTIONS`） | 一条命令行写一次，在任何语言下都能运行 |
| `correct --enable` 和 `--disable` 所接受的纠正模式名称 | 模式以其文件中的英文名称调用 |
| 格式和编码的名称 | 格式以其名称调用 |
| CLI11 自身的诊断信息——缺少必需参数、未知选项、选项不接受的值——以及它的 `Run with --help for more information.` | 措辞写在命令行所基于的解析器内部；它们保持英语 |

无论环境如何，都要求使用英语：

<!-- example: LC_ALL=C LANGUAGE=fr subedit-cli shift --output-dir out a.srt; echo "code=$?" -->
```console
$ LC_ALL=C LANGUAGE=fr subedit-cli shift --output-dir out a.srt; echo "code=$?"
shift needs --by, or --to-grid to work the amount out from the positions
code=1
```

## 全局选项

**它们写在子命令名称之前，而且只能写在那里。**`subedit-cli --encoding cp1252 inspect film.srt` 是正确的形式；`subedit-cli inspect --encoding cp1252 film.srt` 会被拒绝，并提示 `The following argument was not expected: --encoding`。`--version`、`-v` 和 `--quiet` 也是如此。

**`--help` 是例外，但又不算真正的例外**：`subedit-cli inspect --help` 之所以被接受，是因为每个子命令都有自己的帮助选项，它输出的是该子命令的帮助，而不是本工具的帮助。

| 选项 | 效果 |
| :----- | :---- |
| `-h`, `--help` | 输出帮助并停止；如果指定了子命令，则输出该子命令的帮助 |
| `--version` | 输出 `subedit <version>`、本工具所运行依赖的各个库以及已安装的语言，然后停止——参见 [`--version`](#--version) |
| `-q`, `--quiet` | 级别 0——完全没有说明信息 |
| `-v`, `-vv`, `-vvv` | 级别 1 至 3；级别 1 为默认 |
| `--encoding NAME` | 按此编码读取文件，而不是猜测编码 |
| `--list-encodings` | 输出本工具可读写的编码，每行一个，然后**停止**（退出码 `0`）——参见 [`--list-encodings`](#--list-encodings) |
| `--format text\|json` | 标准输出中结果的形式：文本，或每个文件一个 JSON 对象——参见[脚本可读取的输出](#脚本可读取的输出) |
| `--config FILE` | 从 `FILE` 而不是用户的设置文件读取操作设置——参见[设置文件](#设置文件) |
| `--no-config` | 不读取任何设置文件：命令行上未给出的每个操作设置都采用内置默认值——参见[设置文件](#设置文件) |

同一次调用中同时使用 `--quiet` 和 `-v` 会被拒绝：两个相反的意图，不会以最后写出的那个为准来解决。

同一次调用中同时使用 `--config` 和 `--no-config` 同样会被拒绝，并提示 `--config excludes --no-config`；`--config` 带有空名称也会被拒绝，并提示 `--config needs a file name`：空名称不指向任何文件，也不会改去读取用户自己的文件。

### 设置文件

**[`adjust`](adjust.md) 和 [`correct`](correct.md) 默认读取设置文件中的操作设置**——即窗口中的 `Adjust Durations…` 和 `Correct Texts…` 所记住的值，或者 [`config set`](config.md) 写入的值。其他子命令都不读取它。该文件为：

```
$XDG_CONFIG_HOME/subedit/settings.conf
```

如果 `XDG_CONFIG_HOME` 未设置、为空或不是绝对路径，则为 `~/.config/subedit/settings.conf`——也就是窗口所用的那个文件，详见[其手册](../subedit-gui/preferences.md#文件)。

**优先级由高到低：命令行上的选项，然后是文件，然后是内置默认值**。选项始终优先：`--speed off` 会关闭文件中设置的速度。只读取文件中的**操作区块**；显示首选项（窗口位置和大小、列、主题……）从不读取。[`config list`](config.md#config-list) 列出各个键，并指明命令行会读取其中哪些：九个 `duration-adjust.*` 键，以及 `correction.human`、`correction.ocr`、`correction.remove-blank` 和 `correction.line-break.max-lines`。**不论文件如何设置，其中每个键都有一个选项可以在单次调用中覆盖它**——[`adjust`](adjust.md#设置文件提供的内容)和 [`correct`](correct.md#选中了什么没选中什么)的表格列出了这些选项。

**读取用户的文件绝不会使命令失败**。文件不存在则使用默认值；无法读取的文件使用默认值并作出说明；无法读取的值保留其默认值并被指出；未知的键被忽略。`adjust` 和 `correct` 从不写入该文件。

**读取了什么会被说明**，在默认的级别 1 下，在第一个文件之前输出到标准错误：

| 情形 | 行 |
| :--- | :--- |
| 文件已读取 | `settings: read <path>` |
| 其中某个值无法读取 | `settings: <key>: cannot read '<value>', default kept` |
| 用户的文件不存在 | `settings: <path> not found, defaults used` |
| 文件无法读取 | `settings: <path> unreadable, defaults used` |
| `--no-config` | 无输出 |

使用 [`--format json`](#脚本可读取的输出) 时，`adjust` 和 `correct` 的每个文件对象都带有 `settings`：所读取的路径，没有读取任何文件时为 `null`。

**`--no-config` 使一次运行可以重现**：无论用户在窗口中设置了什么，同一条命令行在每台机器上都给出相同的结果。在脚本、测试和错误报告中使用它——或者使用 `--config FILE` 并附上随之提供的文件。

**`--config FILE` 指定另一个文件**，以同样方式读取——**但该文件必须存在**。命令行上给出的名称并不是对默认值的请求：`FILE` 不存在时，`adjust` 和 `correct` 会在读取任何输入之前停止，向标准错误写入 `settings file not found: <FILE>`，并以用法错误码 `1` 退出。存在但无法读取的 `FILE` 会使用默认值并作出说明，与用户的文件相同。[`config show`](config.md#config-show) 仍会把不存在的 `FILE` 显示为 `(not found)`，[`config set`](config.md#config-set-与-config-unset)仍会创建它。

<!-- example: printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine.conf adjust --dry-run a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine.conf adjust --dry-run a.srt; echo "code=$?"
settings file not found: mine.conf
code=1
```

### `--version`

**程序的版本号单独占第一行**——`subedit <version>`，这是 0.15.38 之前的输出，也是脚本所读取的内容。然后是**本工具所运行依赖的每个库各占一行**，再是已安装的界面语言，输出到标准输出；本工具随即停止，退出码为 `0`。同时给出的子命令不会运行，`-q` 和 `-v` 也不起任何作用。

<!-- example: subedit-cli --version | head -n 1 -->
```console
$ subedit-cli --version | head -n 1
subedit 0.15.46
```

完整输出如下，机器上装有 ICU 74.2 和 Enchant 2.3.3——数字取决于具体机器：

```console
$ subedit-cli --version
subedit <version>
  ICU      74.2 (built with 74.2)
  Enchant  2.3.3 (built with 2.3.3)
  CLI11    2.4.1 (built with 2.4.1)
  Languages: en, fr
```

| 库 | 本工具用它做什么 |
| :------ | :------------------------ |
| `ICU` | 读写编码、检测编码、更改字母的大小写 |
| `Enchant` | `correct` 的拼写检查器 |
| `CLI11` | 读取此命令行 |

- **每行给出两个版本**：先是运行时找到的版本，然后是括号中程序编译时所针对的版本。如果机器上的库在程序构建之后升级过，两者就会不同——这是错误报告中首先要给出的信息。
- **Enchant 的头文件不带版本号**：括号中的版本是配置构建时 `pkg-config` 报告的版本。
- **CLI11 被编译进了程序**，它只有头文件：因此它的两个版本始终相同。
- **既没有 Qt，也没有 libmpv**：命令行不链接它们，只列出自己所依赖的库。窗口会列出它们——参见[其手册](../subedit-gui/invocation.md#--version)。
- **`Languages`** 先列出源语言 `en`，它不需要翻译目录，然后按代码列出 `<prefix>/share/subedit/locale` 下所有可读取的已安装翻译目录——也就是 `Edit ▸ Preferences…` 所提供的语言。
- **随语言变化的内容**：`built with` 和 `Languages`。第一行、名称、数字和语言代码从不变化。
- **使用 `--format json` 时**，无论它在 `--version` 之前还是之后，都输出**一行一个对象**：`schema`（`1`）、`command`（`"version"`）、`version`、`libraries`——一个由 `{"name":"ICU","loaded":"74.2","built":"74.2"}` 组成的数组，顺序与文本相同——以及 `languages`，一个代码数组。其中没有任何内容会被翻译。

<!-- example: subedit-cli --version --format json | grep -o '"name":"[^"]*"' -->
```console
$ subedit-cli --version --format json | grep -o '"name":"[^"]*"'
"name":"ICU"
"name":"Enchant"
"name":"CLI11"
```

### `--list-encodings`

**本工具可读写的编码，每行一个，按名称排序**，输出到标准输出——然后本工具停止，退出码为 `0`，与 `--version` 相同：它本身就是答案，而不是中间步骤。同时给出的子命令不会运行。

<!-- example: subedit-cli --list-encodings | grep '^UTF' | head -4 -->
```console
$ subedit-cli --list-encodings | grep '^UTF' | head -4
UTF-16BE
UTF-16LE
UTF-32BE
UTF-32LE
```

- **它们是 `--encoding` 接受的名称**，即规范名称；它们的别名（`cp1252`）同样被接受，只是不在列表中。
- **数量取决于所安装的 ICU**，除了每个 ICU 都有的名称（`UTF-8`、`UTF-16LE`、`windows-1252`……）之外，不承诺任何名称。因此本手册不给出数量。
- **其中没有 `UTF-16` 和 `UTF-32`**：ICU 认识它们，但它们的转换器会写入自己的标记，而模型会拒绝它们——见下文。供选择的列表不会提供下一个字段就会拒绝的内容。
- **使用 `--format json` 时**，**每个编码一个对象**：`{"schema":1,"command":"list-encodings","name":"UTF-8"}`。它不是某个文件的报告：这些对象**既不带 `file` 也不带 `ok`**，这是[输出形式](#输出形式)这一信封格式的唯一例外。它们仍然是一行一个对象，每个条目一个。
- **说明级别不起作用**：列表是结果，`-q` 不会把它去掉。

### `--encoding`，以及它无法强制的内容

**它适用于所有子命令**，因为它们都要读取文件：编码被猜错的文件，无论对它做什么都仍然是错的，而只有 `inspect` 带有的选项，会让不匹配的情况无从补救。

**名称是 ICU 的名称，并且接受它的所有别名**：`cp1252`、`windows-1252` 和 `WINDOWS 1252` 指的是同一种编码。这个集合没有写在任何地方：它就是本机 ICU 能够转换的那一组，[`--list-encodings`](#--list-encodings) 会把它列出。无法转换的名称会在读取任何文件之前被拒绝：

```console
$ subedit-cli --encoding klingon-1 inspect film.srt
no encoding is named "klingon-1"
```

**只有一个例外：编码必须指明字节序。**`UTF-16` 和 `UTF-32` 是 ICU 认识的名称，它们的转换器会写入**自己的标记**。这样的编码无法存在于此模型中——标记不再是一个选择，`--no-bom` 会被接受却不被遵守——因此两者无论读取还是写入都被拒绝：

```console
$ subedit-cli --encoding UTF-16 inspect film.srt
"UTF-16" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do
```

`UTF-16LE` 和 `UTF-16BE` 读写的字节相同，包括标记；精确的名称带来的好处是让人知道是哪一种。

**字节序标记优先于选项**。这是字幕文件对自身唯一作出的声明，若按与其声明不同的方式读取，就是服从调用者而违背文件。这种不一致会作为诊断信息被**说明**：

```console
$ subedit-cli -vvv --encoding cp1252 inspect avec-bom.srt
avec-bom.srt: a byte order mark that contradicts the encoding asked for ("UTF-16LE"), settled by the reader
```

Gaupol 也是这样做的——它用标记所指明的编码重新开始读取——但不会作任何说明；这里，所要求的与实际所做的之间的差距，不会被悄悄略过。

## 子命令

| 子命令 | 作用 | 是否写入 |
| :------------ | :-------------- | :----------- |
| [`inspect`](inspect.md) | 报告文件由什么构成，不作任何更改 | 否 |
| [`convert`](convert.md) | 以另一种格式或另一种形式写出文件 | 是 |
| [`shift`](shift.md) | 把每个位置移动相同的量 | 是 |
| [`transform`](transform.md) | 根据两个已知正确的点修正每个位置 | 是 |
| [`framerate`](framerate.md) | 把文件从一种帧率重新定时为另一种帧率 | 是 |
| [`snap`](snap.md) | 把每个时间戳移到最近的一帧上 | 是 |
| [`hearing-impaired`](hearing-impaired.md) | 删除听障提示 | 是 |
| [`adjust`](adjust.md) | 把持续时间调整到符合阅读速度和各项限制 | 是 |
| [`replace`](replace.md) | 替换字幕中的文本，且不破坏标签 | 是 |
| [`case`](case.md) | 把文本改为单词首字母大写、句首字母大写、全部大写或全部小写 | 是 |
| [`italics`](italics.md) | 把文本设为斜体，或取消斜体 | 是 |
| [`dialogue-dashes`](dialogue-dashes.md) | 添加或删除对话破折号 | 是 |
| [`sort`](sort.md) | 把字幕按开始时间的先后放回顺序 | 是 |
| [`join-repeats`](join-repeats.md) | 把前后相接的同一段文本的各个片段变成一条字幕 | 是 |
| [`disjoint`](disjoint.md) | 切分相互重叠的字幕，使任何时间范围都不再与另一个范围重叠 | 是 |
| [`repair`](repair.md) | 依次执行 `join-repeats`、`sort` 和 `disjoint` | 是 |
| [`correct`](correct.md) | 用 Gaupol 的模式更正文本：提示、单词的连接与拆分、常见错误、大写规则、换行 | 是 |
| [`append`](append.md) | 把其余文件接在第一个文件之后，写入**同一个**文件 | 是 |
| [`split-file`](split-file.md) | 把一个文件切成**两个**：前半部分和后半部分 | 是 |
| [`pair`](pair.md) | 把翻译写到其主文件的位置上，并说明各行是如何对应的 | 是 |
| [`config`](config.md) | 列出、显示和更改设置文件中的操作设置 | 仅设置文件 |

共二十一个，全部列在这里；本工具的帮助以同样的顺序列出它们。

**只有一个什么都不碰**，也就是右侧一列所示的情形：`inspect` 只读取并报告。其余十九个都会写入，而下文所说的内容适用于其中十七个：[`append`](append.md) 和 [`split-file`](split-file.md) 各有自己的参数个数——N 个文件生成**一个**结果，一个文件生成**两个**结果——因此各有自己的输出位置，在各自的页面中说明。[`config`](config.md) 不写入任何字幕文件：它读取并更改设置文件，下文所说的内容都不适用于它。

## 输出位置

写入文件的十七个子命令——[`convert`](convert.md)、[`shift`](shift.md)、[`transform`](transform.md)、[`framerate`](framerate.md)、[`snap`](snap.md)、[`hearing-impaired`](hearing-impaired.md)、[`adjust`](adjust.md)、[`replace`](replace.md)、[`case`](case.md)、[`italics`](italics.md)、[`dialogue-dashes`](dialogue-dashes.md)、[`sort`](sort.md)、[`join-repeats`](join-repeats.md)、[`disjoint`](disjoint.md)、[`repair`](repair.md)、[`correct`](correct.md)、[`pair`](pair.md)——以同样的方式指定输出位置。（[`append`](append.md) 和 [`split-file`](split-file.md) 在各自的页面中有自己的输出位置。）[`inspect`](inspect.md) 不写入任何文件，也不接受这些选项。

**已经存在的输出位置会被覆盖**，不会询问，也没有选项可以避免：写入是原子的——文件绝不会只写一半，被中断的写入会让旧文件保持完好——但原有内容会丢失。这不是遗留下来的缺陷：这就是规则，在同一个输出目录上再次运行命令，就是让它在自己的结果之上再运行一遍。

**没有明确的输出位置，就绝不会写入任何内容**。三种指定方式互相排斥，且必须给出其中一种：否则不会写入任何内容，退出码为 `1`——使用 [`--dry-run`](#先预览再写入)时除外，它不写入任何内容，因此不需要输出位置。

| 选项 | 结果写到哪里 |
| :----- | :---------------- |
| `--output FILE` | 写入这个文件，文件名与所给的完全一致；仅限单个输入 |
| `--output-dir DIR` | 写入这个目录，文件名为输入文件的基本名称 |
| `--in-place` | 通过原子写入覆盖输入文件 |

一个因为没有被告知而覆盖输入文件的工具，用第二次就不会再用了。拒绝只花一行，却能保住文件。

对多个输入使用 `--output` 会被拒绝：最后一个文件会覆盖前面的文件。对于批处理，三者中只有 `--output-dir` 说得通，`--in-place` 也说得通。

**输出目录若不存在则会创建**，包括各级父目录——这对 `--output-dir` 适用，对 `--output` 所写入的目录同样适用。创建在命令行通过验证之后、**第一个文件之前**进行。如果无法创建，只会说明一次，不读取任何文件，退出码为 `2`。

**整批输入在写入之前会作为一个整体接受判断**。先算出每个输入的输出位置，由此得出两种拒绝——它们都是用法错误，退出码为 `1`，**不写入任何文件**，也不创建任何目录：

- **两个输入最终会落到同一个文件**——使用 `--output-dir` 时的 `a/film.srt` 和 `b/film.srt`，或者都转换为 WebVTT 的 `a/film.srt` 和 `b/film.vtt`，它们最终的输出位置更换了扩展名。消息会指出该输出位置和两个输入；改名第二个文件或者让最后一个文件胜出，都会在不作说明的情况下丢失一个文件，而拒绝只花一行；
- **输出位置就是某个输入文件**，且没有 `--in-place`——包括经由 `--output-dir .`、经由 `--output` 或经由符号链接的情形。覆盖自己的输入文件是一个需要明确表达的动作。

当系统认为两个路径指向同一个文件时，它们才是同一个文件，而不是它们写得相像：`in/../in/film.srt` 和 `in/film.srt` 是同一个文件。

**扩展名随所写入的格式而定**。只有 [`convert`](convert.md) 会改变它，因为只有它会改变格式；其余子命令保留所读取文件的格式，因此也保留其扩展名。

| 触发条件 | 消息 |
| :------------------ | :------ |
| 没有输出位置，且没有 `--dry-run` | `no destination given: use --output, --output-dir or --in-place` |
| 两个输出位置 | `--output, --output-dir and --in-place exclude one another` |
| 对批处理使用 `--output` | `--output names one file but several were given: use --output-dir instead` |
| 两个输入，一个输出位置 | `<destination>: would be written by both <input> and <input>` |
| 某个输入会被覆盖 | `<destination>: would be written over the input <input>`——[`append`](append.md) 和 [`split-file`](split-file.md) 的句子相同 |
| 无法创建输出目录 | `<directory>: cannot be created: permission denied`，或 `: cannot be created` |
| 无法写入输出位置 | `<path>: <destination>: cannot be written: permission denied`，或 `: cannot be written` |
| 所写入的编码中缺少某个字符 | `<path>: holds a character the chosen encoding cannot write` |

**最后一条值得多说一句**。文件会以**读取时所用的编码**写回，行尾符和标记也是如此。如果某项更改带入了该编码不认识的字符——比如 Latin-1 中的 `ł`——写入就会失败，**不会写入任何内容**。用 `?` 来代替它，等于当着刚刚写下它的人的面丢失文本。

## 先预览，再写入

**`--dry-run`** 被写入文件的十六个子命令中的每一个所接受，并且在所有地方含义相同：**读取、计算、报告，不写入任何文件**。把它加到本来要运行的命令行上即可；要应用，只需去掉它再运行一次同样的命令行。计算是确定的：只要文件没有变化，`--dry-run` 所显示的就是下一次运行将要写入的内容。

- **它不写入任何文件，也不创建任何目录**，包括 `--in-place`：输入文件保持原样。
- **它不需要输出位置**。也可以给出输出位置，此时**会像没有 `--dry-run` 时那样对它进行检查**：两个输入对应同一个输出位置、某个输入会被覆盖、对批处理使用 `--output`、用两种方式指定位置——没有 `--dry-run` 时被拒绝的命令行，有它时同样被拒绝，句子相同，退出码同为 `1`。
- **退出码与真实运行的相同**：全部计算完成时为 `0`，文件无法读取或处理时为 `2` 或 `3`，用法错误时为 `1`。文件的编码无法写入的字符，会使试运行与真实运行一样失败。**存在更改并不会改变退出码**：这四个退出码说明的是本工具是否成功，而不是文件是否合规。
- **说明信息会指明这一点**。真实运行写出 `-> <destination>` 的地方，试运行写出 `(dry run, nothing written)`。

<!-- example: printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by 1 --dry-run a.srt; ls -- *.srt -->
```console
$ printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by 1 --dry-run a.srt; ls -- *.srt
a.srt: 1 subtitle shifted by 1.000 s (dry run, nothing written)
a.srt
```

**对于会更改文本的子命令**——[`hearing-impaired`](hearing-impaired.md)、[`replace`](replace.md)、[`case`](case.md)、[`italics`](italics.md)、[`dialogue-dashes`](dialogue-dashes.md) 和 [`correct`](correct.md)——**标准输出会给出建议的更改**。每条发生更改的字幕一个区块，按文件中的顺序排列：

```
<path>: subtitle <number>
- <the text before, line by line>
+ <the text after, line by line>
```

- **编号**是该字幕在所读取文件中的编号，从 1 开始计数；
- 更改前文本的每一行前面带有 `- `，更改后文本的每一行前面带有 `+ `：多行文本按原样显示，本身以对话破折号开头的行则是 `- - `；
- **操作将会删除的字幕**会在第一行说明这一点，即 `<path>: subtitle <number> (removed)`，并且没有 `+` 行；
- 使用 `--document translation` 时，翻译中的字幕在同一位置标明 `(translation)`——参见[一份翻译](#一份翻译)；
- 每个区块的第一行以路径开头，因此 `grep '^film.srt: '` 可以在批处理中找出同一个文件的各个区块；
- 没有任何更改时，**没有任何区块**。

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

**JSON 中的列表与此相同**：文件对象带有 `changes` 数组，参见[输出形式](#输出形式)，被删除的字幕的 `after` 为 `null`。**只有在 `--dry-run` 或 `--format json` 要求时才会计算它**：包含数千条字幕的批处理不会白白构建它。

对于**其他子命令**——[`convert`](convert.md)、[`shift`](shift.md)、[`transform`](transform.md)、[`framerate`](framerate.md)、[`snap`](snap.md)、[`adjust`](adjust.md)、[`sort`](sort.md)、[`append`](append.md)、[`split-file`](split-file.md)、[`pair`](pair.md)——标准输出给出的内容与真实运行相同，也就是说文本形式下没有任何输出；JSON 形式下，则是每个文件的对象，带有 `"dry_run":true`、`"destination":null` 以及相同的 `counts`。

## 两种输出，两种角色

| 输出 | 承载的内容 |
| :----- | :--------------- |
| 标准输出 | **结果，而且只有结果**——`inspect` 的报告、文本类 [`--dry-run`](#先预览再写入)所建议的更改；使用 `--format json` 时，每个文件一个对象——[`append`](append.md)、[`split-file`](split-file.md) 和 [`pair`](pair.md) 每次运行一个 |
| 标准错误 | **其余的一切**——说明信息、警告、错误 |

正是这种划分，使得重定向结果时不会把过程信息一并带入：

```console
$ subedit-cli inspect *.srt > rapport.txt
```

报告写入文件，说明信息仍留在屏幕上。

## 脚本可读取的输出

**`--format json`** 把标准输出中的文本替换为**每个输入文件一个 JSON 对象**，采用 JSON Lines 格式：一行一个紧凑的对象，UTF-8 编码、不带标记，行尾符为 `\n`。默认为 `text`，且不变。它是全局选项，因此写在子命令之前；`text` 和 `json` 之外的值属于用法错误，退出码为 `1`。

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

**每个输入一个对象，不会没有，也不会有两个**，无论该输入成功与否，都按给出输入的顺序排列——使用 [`--recursive`](lots.md) 时则按找到的顺序。逐行计数，就能找到各自的文件。**有三个子命令是例外，因为它们的参数个数不是批处理的那种：[`append`](append.md)（N 个输入，一个输出）、[`split-file`](split-file.md)（一个输入，两个输出）和 [`pair`](pair.md)（两个文件，一个输出）整次运行只写出一个对象，该对象指明各自页面所说的那个文件。** **因此，会改写文件的子命令现在也会向标准输出写入内容**，这是它们在文本形式下所没有的：它们的文件仍然是它们的结果，而对象就是这个结果的报告。

**说明信息不变。**`-q`、`-v`、`-vv` 和 `-vvv` 作用于标准错误输出中的文本，与没有 `--format` 时相同；**标准输出不随它们变化**。读取诊断信息在文本形式下保留在级别 3，而它们是数据，**始终**位于 `warnings` 中。用法错误不写出对象：它在触碰任何文件之前就被检测到，因此没有可以附加的对象；它输出到标准错误，退出码为 `1`，标准输出保持为空。

### 输出形式

| 键 | 内容 | 出现条件 |
| :-- | :------ | :------- |
| `schema` | 整数，此形式的版本——`1` | 始终 |
| `command` | 子命令的名称 | 始终 |
| `file` | **按所给形式**的路径，或者按目录遍历所组成的路径 | 始终 |
| `ok` | 布尔值 | 始终 |
| `error` | `{"kind", "message"}` | `ok` 为 false 时 |
| `dry_run` | 布尔值：[`--dry-run`](#先预览再写入)时为 true | `ok` 为 true 时，对于会写入文件的子命令 |
| `destination` | 所写入的路径；`dry_run` 为 true 时为 `null`：没有写入任何内容 | 同上 |
| `counts` | 由**整数**组成的对象，因子命令而异 | 同上 |
| `changes` | 由 `{"subtitle", "document", "before", "after"}` 组成的数组：发生更改的文本，见下文 | 对于会更改文本的子命令 |
| `warnings` | 由 `{"kind", "line"?, "detail"?, "settled"?}` 组成的数组 | `ok` 为 true 时 |
| `alignment` | 翻译是如何叠加的，参见[一份翻译](#一份翻译) | 使用 `-t` 时，以及 `pair` |
| `constraints` | 所用的设置 | `adjust` |
| `settings` | 所读取的设置文件，没有读取任何文件时为 `null`——参见[设置文件](#设置文件) | `ok` 为 true 时的 `adjust`、`correct` |
| `inputs`, `tail` | 每个被追加的文件一个元素；后半部分的路径 | `append`；`split-file` |

关于整个文件的诊断信息不带 `line`；没有可补充的内容时不带 `detail`；当读取过程**处理了**某些问题——例如重新生成的编号——时，`settled` 为 `true`，而当读取过程保持原样时，则不出现。

**`changes`** 按文件中的顺序列出文本发生更改的每一条字幕：`subtitle` 是它在所读取文件中的编号，从 1 开始计数；`document` 是 `"main"` 或 `"translation"`；`before` 是更改前的文本；`after` 是更改后的文本，**字幕被删除时为 `null`**。文本中的换行是其中的 `\n`。真实运行与 [`--dry-run`](#先预览再写入)都会写出它，**没有任何更改时为空（`[]`）**：它存在，说明该子命令会列出自己的更改；它不存在，说明该子命令没有更改。

**任何地方都没有浮点数**。位置或持续时间是以**毫秒**为单位的整数（`…_ms`）；计数是整数；帧率是**字符串**（`"25"`、`"24000/1001"`），因为它是精确的，有理数无法用浮点数表示；网格集中度是以**千分比**表示的整数——`1000` 是完美的网格。这样，两份输出就能逐字节比较，无需考虑舍入。

**不是 UTF-8 的路径**——Linux 允许这样的路径——无法在 JSON 中表示：写出时无效字节会被替换为 U+FFFD，并且对象带有警告 `path-not-utf8`。对于这类路径的保真度不作承诺，并且这一点已经说明。

### 各子命令在 `counts` 中放入的内容

| 子命令 | `counts` 的键 |
| :------------ | :--------------- |
| [`shift`](shift.md) | `subtitles`、`shifted_by_ms`（带符号） |
| [`transform`](transform.md) | `subtitles` |
| [`framerate`](framerate.md) | `subtitles` |
| [`snap`](snap.md) | `subtitles`、`moved`（移动的位置数）、`furthest_ms`（最大的移动量） |
| [`hearing-impaired`](hearing-impaired.md) | `cleaned`（被重写的文本数）、`removed`（被删除的字幕数） |
| [`replace`](replace.md) | `replaced`（在发生更改的字幕中被替换的匹配数）、`matched`（找到的匹配数） |
| [`case`](case.md)、[`italics`](italics.md)、[`dialogue-dashes`](dialogue-dashes.md) | `changed`（文本发生更改的字幕数） |
| [`correct`](correct.md) | `corrected`（被更改的文本数）、`removed`（被删除的字幕数）——从不统计匹配 |
| [`sort`](sort.md) | `subtitles`、`moved`（字幕换了位置的个数） |
| [`adjust`](adjust.md) | `subtitles`（目标字幕数）、`adjusted`（结束时间发生移动的字幕数），然后是 `sacrificed` 组：`speed`、`minimum`、`gap`；此外在 `counts` 之外还有 `constraints`——所用的设置 |
| [`convert`](convert.md) | `subtitles`，然后是转换所丢失的内容：`lost_ends` 和 `lost_header`（0 或 1）、`joined_lines`、`lost_tags`、`lost_fields`、`furthest_ms` |
| [`append`](append.md) | `files`、`subtitles`、`appended`，然后是累加的 `lost_ends`、`lost_header`、`joined_lines`、`lost_tags`、`lost_fields`、`furthest_ms`；此外在 `counts` 之外还有 `inputs`——每个被追加的文件一个元素 |
| [`split-file`](split-file.md) | `subtitles`、`head`、`tail`；此外在 `counts` 之外还有 `tail`——后半部分的路径（`destination` 是前半部分） |
| [`pair`](pair.md) | `subtitles`（所写入文件的字幕数）；此外在 `counts` 之外还有 `alignment` |
| [`inspect`](inspect.md) | 没有 `counts`：文件的描述，见其页面 |

### 错误标识符

`error.kind` 是下列之一；**读取方应把未知的标识符视为“其他”**。

| `kind` | 含义 |
| :----- | :----------- |
| `not-found`, `permission-denied`, `io` | 系统拒绝读取——或写入——该文件 |
| `unknown-format`, `no-subtitle-found`, `undecodable` | 文件已读取，但无法被解析 |
| `unencodable` | 所选编码无法写入的字符 |
| `no-byte-order-mark`, `no-frame-rate` | `convert` 在没有被告知的情况下无法写入的内容 |
| `before-the-origin`, `beyond-the-end`, `no-transform`, `no-grid` | 无法应用于该文件的操作 |
| `range-out-of-bounds` | 此文件不包含的 `--range` |
| `no-style` | 不写入样式的格式（TMPlayer、LRC），用于 `italics` |
| `refused` | 操作的任何其他拒绝 |

### 承诺什么，不承诺什么

**只要 `schema` 为 `1`，就承诺**：键的名称、类型和单位；`ok`、`file` 和 `destination` 的含义；**每个输入一个对象，按输入的顺序排列**——对于有自己的参数个数的三个子命令，则每次运行一个；现有的 `kind` 标识符；确定性——相同的输入、相同的参数，得到相同的字节。

**不承诺**：**消息的文字**（`message`、`detail`），它们是面向人的英语，必要时会被更正——比较它们的脚本事先就是坏的；对象中**键的顺序**，它是固定的，但 JSON 并不保证；**尚未添加的键**是否存在。

**因此读取方应忽略未知的键**，并把未知的 `kind` 视为“其他”：添加键或标识符不会改变 `schema`。删除或重命名键、更改类型或单位、删除标识符，都会使它递增——这是一次破坏性变更，会在 CHANGELOG 和此处予以公告。

## 说明级别

每个级别**包含前一个级别**：每升一级只增加，不替换。

| 级别 | 方式 | 标准错误输出的内容 |
| :----- | :------ | :------------------------------ |
| 0 | `--quiet` | 无，**错误除外** |
| 1 | 默认，或 `-v` | 每处理一个文件一行；有多个文件时，再加一行汇总 |
| 2 | `-vv` | 再加上所识别的内容：格式、编码、BOM、行尾符 |
| 3 | `-vvv` | 再加上调试跟踪：读取和写入的字节数，以及**每一条读取诊断信息** |

**错误绝不会被屏蔽，`--quiet` 也不例外**。一个悄悄失败的命令，除了退出码之外什么也不会留下。

汇总从两个文件起才会出现：只有一个输入时，它会重复前面那一行。

## 读取诊断信息

字幕格式会被**尽可能好地**读取：遇到异常时，读取器不会停止——它会作出决定，或者保持原样，并说明这一点。它所遇到的内容在**级别 3** 输出，每条诊断信息一行，输出到标准错误：

```
a.srt: 2 diagnostics while reading
a.srt: line 6: SubRip numbers that do not follow ("7"), settled by the reader
a.srt: line 9: a line that fits nowhere, left as it stands
```

全部十七个子命令都会报告它们，而不只是 [`inspect`](inspect.md)：一个被尽可能好地读取、然后又被写回的文件，经历了同样的决定，对此保持沉默会让人误以为什么也没有发生。

**诊断信息绝不是失败**。文件已读取，命令已成功，退出码为 `0`。这就是它们放在最详细的级别的原因：放在更低的级别，它们会淹没那条说明真正做了什么的行。

### 每一行包含什么

| 部分 | 含义 |
| :----- | :------------- |
| `line N` | 位置，从 1 开始计数，与编辑器显示的一致；涉及整个文件的内容则**没有此部分** |
| 句子 | 遇到了什么，属于下面十二个类别之一 |
| `("…")` | 文件中有问题的文本，在类别不足以说明时给出；截断为 80 字节 |
| 结尾 | 对此做了什么：`settled by the reader`，或 `left as it stands` |

**有两个类别没有行号**，它们就是关于编码的那两个：字节是经过权衡的，或者其标记是在文件的任何一行出现之前就被读取的。因此它们的行以句子开头，前面没有 `line N`——没有可以指出的位置。

**行的结尾最为重要。**`settled by the reader` 表示读取器作出了决定，写出的文件带有它的决定——缺失的编号会被重新生成。`left as it stands` 表示它什么也没有动，因为**只有你能决定**：结束时间早于开始时间的字幕保持原样。

### 区块的行号

关于**整个区块**的异常，以它的定时行为锚点，而不是触发它的那一行。因此，写在第 5 行的不一致编号，会报告在第 6 行，也就是随后的定时行——有问题的编号在 `("…")` 中。只有关于**某一行**的异常——无法读取的定时行、第一个定时行之前的文本——才在该行自身上报告。

### 十二个类别

| 句子 | 触发条件 |
| :----- | :------------------ |
| `a line that fits nowhere` | 不属于任何区块的行 |
| `a timing line that could not be read` | 无法读取的定时行 |
| `a SubRip block without its number` | 缺少编号，写入时会重新生成 |
| `SubRip numbers that do not follow` | 编号出现跳跃 |
| `text before the first timing line` | 第一个定时行之前的文本 |
| `a WebVTT block of an unknown kind` | 无法识别的 WebVTT 区块 |
| `declares an event column this tool cannot fill` | 此处无法填充的 Sub Station Alpha 事件列 |
| `counts in frames and states no rate; it was read at` | 以帧计数而没有声明帧率的文件——所采用的帧率是详细内容 |
| `carries no end times; each one was taken from the next start` | 没有结束时间的格式，TMPlayer 或 LRC：**表中没有任何结束时间来自文件** |
| `more than one kind of line ending` | 混用了行尾符 |
| `an encoding nothing declared` | 编码是通过权衡字节提出的，且不是 UTF-8 |
| `a byte order mark that contradicts the encoding asked for` | 文件带有标记，而 `--encoding` 指定了另一种编码——标记优先 |

**中间的三个来自第 9 阶段**，这三个中的后两个说的不是某一行，而是整个文件：它们说明所显示的某个值并不是读取得来的。LRC 上填充的 `End` 列、MicroDVD 上的某个位置——文件中并没有带任何这些内容。

**操作可以添加自己的诊断信息**，它不是读取诊断信息：[`correct`](correct.md#无法应用的模式)会连同字幕一起，说明哪个模式放弃了。它在级别 1 说明，并在文件的对象中带有标识符 `pattern-failed`——从不是文件的失败。

## 一份翻译

翻译文件与**任何字幕文件**一样处理：`subedit-cli case --to sentence film.fr.srt` 无需任何新东西就能更改它。配对增加的内容，以及三个选项所说明的内容，范围很窄：**了解翻译的各行如何落在主文件的字幕上**——这就是 `inspect -t`——以及**在让翻译保持叠加于其主文件的情况下更改翻译**，规则与窗口相同。

| 选项 | 适用于 | 值 | 默认值 |
| :----- | :---- | :----- | :----- |
| `-t`, `--translation-file` | `inspect`、`hearing-impaired`、`replace`、`case`、`italics`、`dialogue-dashes`、`correct`、`pair` | 翻译文件 | — |
| `--document` | 六个文本子命令，不含 `inspect` 和 `pair` | `main` 或 `translation` | `main` |
| `--align-method` | 与 `-t` 相同 | `position` 或 `number` | `position` |

- **`position`** 把每一行的中点与字幕的边界作比较：缺少一行不会使其后的各行错位。**`number`** 把第 *n* 行送到第 *n* 条字幕，不看位置。默认采用位置，原因与窗口相同：按编号配对的翻译，中间缺少一行就会使其余所有行错位。
- 在文本子命令上，**`--document translation` 需要 `-t`，`-t` 也需要 `--document translation`**——缺少任何一个都属于用法错误，退出码为 `1`，因为读取了却没有被任何动作用到的文件，是疏漏而不是偏好。`inspect` 上的 `-t` 没有 `--document`：它只做报告。**不带 `-t` 使用 `--align-method` 同样是用法错误。**
- **`-t` 指定的是一个文件，因此只对单个输入有意义**：有多个输入时，或者通过 `--recursive` 取到目录时，它会像 `--output` 一样被拒绝，**在读取任何文件之前**。对于批处理，请对每一对文件各运行一次。
- **与主文件相同的翻译文件会被拒绝**——不论其路径怎样写：`<translation>: the file is already open as the main document`，退出码为 `2`。
- **只写入所针对的文档**，写入**它自己的路径，采用它自己的格式**：即翻译文件的格式、编码和行尾符。`--output` 指定翻译的文件，`--output-dir` 把它的基本名称放到那里，`--in-place` 改写它；**主文件绝不会被触碰**。报告中的路径是翻译的路径。
- **配对所产生的字幕**——没有任何字幕对应到的行——会以**它们自己的位置**写入翻译，不论采用哪种方法；**没有任何人翻译的字幕会写成没有文本的区块**，读取时不会产生任何异常。
- **`--encoding` 和 `--frame-rate` 对两个文件都适用。**
- **位置**类子命令——`shift`、`transform`、`framerate`、`snap`、`adjust`——没有这些选项：像处理批处理那样把两个文件都交给它们，`subedit-cli shift --by 2 film.srt film.fr.srt --output-dir out/`。
- **写出在主文件上重新定时的翻译**——这是配对才能做到、单个文件做不到的事——由 [`pair`](pair.md) 完成，它既没有 `--document`，也没有 `--recursive`。

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

使用 `--format json` 时，每个文件的对象还带有 `alignment` 键，它说明翻译是如何叠加的——`{"file", "method", "attached", "born", "untranslated", "out_of_order"}`，与 [`inspect`](inspect.md#以-json-输出) 的 `translation` 键相同——并且**使用 `--dry-run` 时**，`changes` 会列出翻译中的字幕，每条都带有 `"document":"translation"`。

## 多个文件

每个子命令都接受多个路径，[`append`](append.md)、[`split-file`](split-file.md) 和 [`pair`](pair.md) 除外，它们的参数个数以各自页面所述为准。每个文件都独立处理：一个文件失败不会中断其他文件，失败会连同文件名和原因一并报告。

**目录代表其中所包含的文件，需要使用 `--recursive`（`-r`）**：没有这个选项时，给出目录属于用法错误，退出码为 `1`，且不处理任何内容——拒绝它，比什么也不处理却返回 `0` 要好：

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?" -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?"
films: is a directory: use --recursive
code=1
```

遍历的方式、`--output-dir` 之下保持的目录树，以及被搁置不处理的内容，在[处理目录树](lots.md)中说明。

## man 手册页

安装时，会在 `subedit-cli(1)` 旁边为**每个子命令放置一个手册页**：`subedit-cli-shift(1)`、`subedit-cli-convert(1)` 以及其余各页，[子命令表](#子命令)中的每一行对应一页。`subedit-cli(1)` 包含本章——全局选项、输出、退出码——其他每个页面包含其子命令的章节，其中的示例也包括在内。窗口的页面 `subedit-gui(1)` 与它们放在一起。

```console
$ man subedit-cli
$ man subedit-cli-shift
```

**这些页面不是脱离本手册另行编写的**：它们在构建程序时由本手册生成，因此内容与手册一致，不会与之脱节。每个页面的 FILES 章节会指明手册所安装的目录。

| 页面 | 安装位置 |
| :---- | :----------------------- |
| 英语 | `share/man/man1` |
| 每种已安装的语言——加泰罗尼亚语、捷克语、德语、西班牙语、芬兰语、法语、加利西亚语、希伯来语、匈牙利语、西方国际语、冰岛语、意大利语、荷兰语、波兰语、葡萄牙语、巴西葡萄牙语、俄语、塞尔维亚语、土耳其语、简体中文 | `share/man/<language>/man1`——`share/man/fr/man1` |

**`man` 与程序一样，根据区域设置选择语言**：在法语区域设置下，`man subedit-cli` 显示法语页面，而未安装其页面的语言则得到英语页面。`-L` 可以不论区域设置而指定语言：

```console
$ man -L fr subedit-cli-shift
```

## 退出码

| 退出码 | 含义 |
| :--- | :------------ |
| `0` | 一切成功 |
| `1` | 用法错误——未知选项、无效的值、被禁止的组合 |
| `2` | 没有任何文件能被处理 |
| `3` | 一部分文件已处理，另一部分没有 |

区分 `2` 和 `3`，是为了让脚本不必再次读取输出就知道该怎么做：“全都没成功”和“缺了一个”所需的反应并不相同。

用法错误会在**任何处理之前**检测到：它绝不会让一批文件处理到一半。

## 错误

| 触发条件 | 标准错误输出中写出的内容 |
| :------------------ | :--------------------------------------- |
| 未知的选项或子命令 | 出错的名称，以及指向 `--help` 的提示 |
| `--quiet` 与 `-v` 同时使用 | `--quiet and -v ask for opposite things; give one or the other` |
| `--config` 与 `--no-config` 同时使用 | `--config excludes --no-config` |
| `--config` 带有空名称 | `--config needs a file name` |
| 文件不存在 | `<path>: does not exist` |
| 文件无法读取 | `<path>: cannot be opened: permission denied` |
| 会写入自己的标记的编码 | `"<name>" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do` |
| 字节无法解码 | `<path>: cannot be decoded in the chosen encoding` |
| 格式无法识别 | `<path>: is in no format this tool knows` |
| 没有任何像字幕的内容 | `<path>: holds nothing recognisable as a subtitle` |
| 使用 `--document translation` 却没有 `-t` | `--document translation needs the translation file: use -t` |
| 使用 `-t` 却没有 `--document translation` | `-t names a translation to change: use --document translation` |
| 使用 `--align-method` 却没有 `-t` | `--align-method needs a translation file: use -t` |
| 对多个输入或目录使用 `-t` | `-t names one file but several inputs were given: use one invocation per pair` |
| 翻译与主文件相同 | `<translation>: the file is already open as the main document` |
