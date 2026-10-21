# `correct`

<!-- machine-translated -->
> 本页为机器翻译，尚未经过熟悉该语言的读者审校。

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

用 **Gaupol 的模式**更正文本，与窗口的 [`Correct Texts…`](../subedit-gui/correct-texts.md) 助手一样——模式相同，引擎相同，计数也相同。这里有六项任务：**面向听障人士的提示**、单词的**连接**与**拆分**、**常见错误**、**大写规则**和**换行**。

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

## 选中了什么，没选中什么

**不会事先勾选任何内容。** 窗口打开时，各复选框保持用户上次离开时的状态；命令行则从自己的命令行取任务、代码、行长度和语言，从不取自设置文件：某天在窗口中勾选的任务、窗口中开关的模式复选框（`correction.activations`）、拼写检查器的替换列表，都不会被读取，在这一侧也不会写入任何内容。

**有四项设置默认来自设置文件**——即 `Correct Texts…` 记住的取值，或 [`config set`](config.md) 写入的取值。**命令行上的选项始终优先**，其次是文件，最后才是内置的默认值；文件在哪里、`--config` 和 `--no-config`，参见[设置文件](invocation.md#设置文件)。

| 键 | 提供的内容 | 可以覆盖它的选项 |
| :-- | :--------------- | :----------------------- |
| `correction.human`, `correction.ocr` | 所应用的常见错误的类别 | `--classes` |
| `correction.remove-blank` | 是否删除被更正清空的字幕 | `--keep-blank-subtitles`, `--remove-blank-subtitles` |
| `correction.line-break.max-lines` | `line-break` 的最大行数 | `--max-lines` |

- **这四项都有一个不论文件怎么说都能撤销它的选项**：`--classes human,ocr` 同时应用两个类别，`--remove-blank-subtitles` 删除被清空的字幕，`--max-lines 3` 恢复 Gaupol 的行数。`--keep-blank-subtitles` 与 `--remove-blank-subtitles` 同用属于用法错误。
- **文件中两个类别都关闭，会被忽略，并会说明**，在级别 1 时——`settings: correction.human and correction.ocr are both false, ignored`——：`--classes` 写不出这种情况，而且这会使 common-errors 无事可做。此时会改为同时应用两个类别。
- **不在 1 到 1000 范围内的行数，根本不会从文件中读取**：文件的读取器会拒绝它——在级别 1 时为 `settings: correction.line-break.max-lines: cannot read 'N', default kept`——并使用内置的 3。

本页的示例都带 `--no-config` 运行，这样它们体现的就是内置默认值的效果，而与运行它们的机器上的设置无关。

| 选项 | 必需 | 取值 | 默认值 |
| :----- | :----- | :------ | :----- |
| `<file>...` | 是 | 一个或多个路径 | — |
| `--tasks` | **是** | 以逗号分隔的列表：`mentions`, `join-words`, `split-words`, `common-errors`, `capitalization`, `line-break` | 无任务 |
| `--code` | 有模式任务时**必需**，否则**不适用** | `Script[-language[-COUNTRY]]`：`Zyyy`、`Latn`、`Latn-en`、`Latn-en-US` | — |
| `--language` | 有 `join-words` 或 `split-words` 时**必需**，否则**不适用** | 语言代码：`fr`、`fr_FR`、`en_US` | — |
| `--classes` | 否 | `human`、`ocr` 或 `human,ocr` | 两者都有——或设置文件所说的 |
| `--enable`, `--disable` | 否，可重复 | 模式的英文名称，或 `type:name` | 随附的 `.conf` 文件 |
| `--max-length` | 有 `line-break` 时**必需** | **字符**数，大于零 | **无**——参见下文 |
| `--max-lines` | 否 | 行数，为大于零的整数 | 3——或设置文件所说的 |
| `--skip-length`, `--skip-lines` | 否 | 一个数，或 `off` | 与 `--max-length` 和 `--max-lines` 相同的界限 |
| `--keep-blank-subtitles` | 否 | 开关：被清空的字幕保留，内容为空 | 被清空的字幕会被删除——除非设置文件另有说明 |
| `--remove-blank-subtitles` | 否 | 开关：不论设置文件怎么说，被清空的字幕都会被删除；与 `--keep-blank-subtitles` 同用会被拒绝 | 关闭 |
| `--range` | 否 | `N-M`，或 `N-` 直到结尾——参见 [`adjust`](adjust.md#--range) | 整个文件 |
| `--document` | 否 | `main` 或 `translation`——参见[一份翻译](invocation.md#一份翻译) | `main` |
| `-t`, `--translation-file` | 使用 `--document translation` 时必需 | 一个翻译文件，且只能有**一个**输入 | — |
| `--align-method` | 否，且只能与 `-t` 同用 | `position` 或 `number` | `position` |
| `--recursive`, `-r` | 否 | 开关 | 关闭——参见[处理目录树](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **三选一**，使用 `--dry-run` 时除外 | 参见[调用](invocation.md#输出位置) | — |
| `--dry-run` | 否 | 开关：只计算并说明，不写入任何内容——参见[先预览，再写入](invocation.md#先预览再写入) | 关闭 |

### 任务

**列表中的顺序没有影响**：各任务按 Gaupol 的顺序应用——先是提示，然后是单词的连接与拆分，再是常见错误，然后是大写规则，最后是换行——，每个任务都作用于前一个任务留下的文本。被提示清空的字幕，不参与后面的任何处理。

| 任务 | 作用 |
| :---- | :-------------- |
| `mentions` | 删除方括号中的声音提示，以及在 `Sound in parentheses` 开启时删除圆括号中的声音提示（即 [`hearing-impaired`](hearing-impaired.md) 的清理），然后处理歌词和说话者模式 |
| `join-words` | 根据 `--language` 的词典，把被切成两半的单词重新连起来 |
| `split-words` | 根据同一个词典，拆分粘在一起的两个单词 |
| `common-errors` | 更正常见错误：空格、标点、连字、OCR 错误 |
| `capitalization` | 在该语言要求的地方使用大写 |
| `line-break` | 重新切分一条字幕的各行，使任何一行都不超过 `--max-length`，并权衡该代码的断行模式 |

### 模式代码

**`--code` 指定级联**：`Zyyy` 适用于任何文字，然后依次是文字、语言和国家——`Latn-en-US` 会读取 `Zyyy`、`Latn`、`Latn-en` 和 `Latn-en-US` 的模式。**它是必需的**，因为命令行没有环境语言：通用的默认值适用的范围会比人们以为的*小*，却不说明。它对同一次运行的每个任务都有效；需要自己代码的任务，只能另外运行一次。代码写法有误属于用法错误：

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code francais --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code francais --output b.srt a.srt; echo "code=$?"
--code: "francais" is not a pattern code: expected Script[-language[-COUNTRY]], like Zyyy, Latn or Latn-en-US
code=1
```

用户放在 `$XDG_DATA_HOME/subedit/patterns`（或 `~/.local/share/subedit/patterns`）中的模式，会像在窗口中一样**添加到随附的模式之上**，而无法读取的随附模式会在级别 1 时说明：`patterns: <file>, line N (<reason>)`。

### 单词的连接与拆分，以及词典

**这两项任务不读取任何模式**：它们通过 Enchant 查询**系统的词典**——这是软件包的一项依赖（Debian 和 Ubuntu 上是 `libenchant-2-2`，Fedora 上是 `enchant2`），`subedit-cli` 与窗口共用它。因此它们既没有代码，也没有名称可设置：没有模式任务时，`--code` 没有意义；这两项任务都没有时，`--language` 没有意义——只给出其中一个而不给出另一个，属于用法错误，退出码为 `1`。

- **`join-words`** 会把词典不认识的单词，与它前面或后面的单词连接起来，**前提是只有一个方向能得到已知单词**：`hel lo` 变为 `hello`；有两个可能的方向，或一个都没有，则不作决定。
- **`split-words`** 会拆分词典不认识的单词，**前提是它的建议中恰好有一条是在该单词中加一个空格**：`hellothere` 变为 `hello there`。首字母大写、后面是小写字母的单词不处理：它多半是人名。

**如果没有该语言的词典，运行会被拒绝**，退出码为 `1`，**在读取任何文件之前**，措辞与窗口一致——窗口会把这个功能置灰并说明原因——命令行则是拒绝并说明，而不是保持沉默：

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nhel lo\n\n' > a.srt; subedit-cli --no-config correct --tasks join-words --language qq_QQ --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nhel lo\n\n' > a.srt; subedit-cli --no-config correct --tasks join-words --language qq_QQ --output b.srt a.srt; echo "code=$?"
no dictionary for qq_QQ
code=1
```

**窗口的替换列表既不读取也不写入。** 它是通过用户在拼写检查中的选择*逐渐积累*起来的；命令行没有这样的选择，而读取这份列表，会让脚本依赖其作者当初放进去的东西。

### 换行

**换行按字符测量，这是它与窗口的区别。** 窗口以 *em* 为单位测量——这是按字体校准的宽度，每个字母 0.55 em——它默认的 24 是以 em 为单位的值。命令行没有字体：它数的是文本的字符。**把 24 直接照搬为字符数，几乎会切断所有内容，却不说明**；因此**没有默认的长度**，使用 `line-break` 时 `--max-length` 是必需的：

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn texte\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn texte\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --output b.srt a.srt; echo "code=$?"
--max-length is required by the task line-break: Gaupol's 24 is a width in ems, and has no value in characters
code=1
```

**`--max-lines` 为 3**，这是 Gaupol 的取值，不受单位影响；三行不够时，换行会多取一行。**跳过**——已经在限制之内的字幕保持原样——默认采用**相同的界限**（`--skip-length` = `--max-length`，`--skip-lines` = `--max-lines`），Gaupol 也是如此；`--skip-length off` 和 `--skip-lines off` 会将其关闭，此时文本会被视为一整段重新读取并重新切分：

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

断行模式取决于代码：`Latn` 不会在数字与其单位之间断行，也不会在对话破折号之后断行；`Latn-en` 不会在冠词、介词、称谓或所有格限定词之后断行。**在 `Zyyy` 之下，不存在断行模式**：各行只是被均衡分配。**这项任务绝不会因为没有有效的模式而被拒绝**：它的模式用来权衡在哪里断行，没有它们，断行照样进行。这四个选项只在有 `line-break` 时才有意义；不带它而给出这些选项，属于用法错误。

### 类别，以及按名称开关

**`--classes` 会永久过滤常见错误**：取消勾选 `ocr`，就会把它的模式从应用中去掉——连字 `ﬁ` 此后仍是 `ﬁ`。其他三种类型的模式没有类别。

**`--enable` 和 `--disable` 按模式的英文名称设置模式**，名称就是键（ADR 0037）；一个名称可以指向多条记录，它们共用一个复选框。这是必要的，也是它与窗口默认值的区别所在：**随附的 `.conf` 文件会关闭提示模式**，因此单独使用 `--tasks mentions` 什么也不会应用。“Sound in brackets”和“Sound in parentheses”这两个模式各自控制一种分隔符——方括号、圆括号——并由相同的选项设置：只开启其中一个，不会删除另一个。

有三种情况属于**用法错误**，退出码为 `1`，在读取任何文件之前说明：

| 触发条件 | 消息 |
| :------------------ | :------ |
| 某个名称在给定的代码下，不对应给定任务的任何模式 | `--enable: "<name>" names no pattern of the tasks given, under the code <code>` |
| 某个名称对应两种类型的模式——此时要写成 `type:name`，类型为 `common-error`、`capitalization`、`hearing-impaired` 或 `line-break` | `--enable: "<name>" names patterns of several types: write type:name, …` |
| 同一个名称既给了 `--enable` 又给了 `--disable` | `"<name>" is given to --enable and to --disable: choose one` |

**输入错误会明确报出**：窗口会忽略过期的设置，因为设置可能比已经消失的模式存留得更久；而一次调用没有这样的遗留。

**没有任何有效模式的任务会被拒绝**，退出码为 `1`——“无事可做”是一个要说明的回答，而不是悄无声息的成功：

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --output b.srt a.srt; echo "code=$?"
mentions: no pattern is active under the code Latn; switch one on with --enable
code=1
```

同一个文件，开启方括号清理：

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --enable "Sound in brackets" --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --enable "Sound in brackets" --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:02,000
Bonjour
```

## 写入什么，说明什么

所读文件的格式、编码和行尾符都会**保留**。**被更正清空的字幕会被删除**，与窗口的默认复选框一致；`--keep-blank-subtitles` 让它们保持为空，而 `--remove-blank-subtitles` 即使在设置文件保留它们时也会删除它们。**被清空的翻译保持为空**，并保留其字幕。

**计数与窗口一致**——“Edited N and removed M subtitles”：更改的文本数和删除的字幕数，**绝不是匹配数**。没有任何更改的文件也照样写出：给出了输出位置，就会写入输出位置。

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

### 无法应用的模式

**无法读取、无法转换或无法结束的模式会被点名，并给出它放弃时所在的字幕**，命令行在这一点上比窗口说得更多：确认页面只点一次模式的名称，不给出文本。

| 原因 | 含义 |
| :----- | :------------------- |
| `cannot be translated` | 它的表达式用到了转换为 ICU 时不支持的内容 |
| `will not compile` | 引擎拒绝了它的表达式 |
| `has an invalid replacement` | 它的替换文本按 Python 的读法无法读取 |
| `timed out` | 引擎在该文本上放弃了 |
| `never settled` | `Repeat` 在一百遍之后仍在更改文本 |
| `grew the text too long` | 文本超出了一条字幕所能容纳的范围 |

在级别 1，写到错误输出：`<path>: pattern "<name>" (<reason>) was not applied to subtitle N`——如果该模式对所有文本都被搁置，则没有末尾部分。**其中的编号是文件中的编号**，不论之前有哪些任务删除了字幕。在 JSON 中，它是文件的一条 `pattern-failed` 警告，其 `detail` 含有这句话。**放弃的模式不算文件失败**：文件会写出，退出码为 `0`。

## 输出

**标准输出**——为空，结果就是写出的文件。**使用 `--dry-run` 时，输出的是拟作的更改**，每条字幕一个块：它的编号、更改前的文本、更改后的文本——对于更正会删除的字幕，写 `(removed)`，没有 `+` 行；使用 `--keep-blank-subtitles` 时，它会保留，内容为空。参见[先预览，再写入](invocation.md#先预览再写入)。

| 级别 | 在错误输出中增加的内容 |
| :----- | :------------------------------------- |
| 1 | `<path>: Edited N and removed M subtitles -> <destination>`，无法应用的模式，无法读取的模式文件——并且最先写出所读取的设置文件——`settings: read <path>`——以及其中被忽略的部分 |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`，然后是每一条读取诊断信息 |

在 JSON 中，`counts` 含有 `corrected`（更改的文本数）和 `removed`（删除的字幕数），`changes` 则是被触及的字幕。使用 `-t` 时，`alignment` 键说明翻译是如何对应的。`settings` 给出所读取的设置文件，没有读取则为 `null`。

## 退出码

与整个工具相同：每个文件都已写出则为 `0`，一个都没有写出则为 `2`，只写出了部分文件则为 `3`，用法错误则为 `1`。

## 错误

| 触发条件 | 在错误输出中写出的内容 |
| :------------------ | :--------------------------------------- |
| `--config` 指定的文件不存在——用户自己的文件不存在时，使用默认值 | `settings file not found: <FILE>` |
| 缺少 `--tasks` | `--tasks is required`，后面是指向 `--help` 的提示 |
| 未知的任务 | `--tasks: "<name>" is not a task: expected mentions, join-words, split-words, common-errors, capitalization or line-break` |
| 缺少 `--code` | `--code is required by the tasks that read patterns: <tasks>` |
| 没有模式任务却给出 `--code` | `--code is for the tasks that read patterns, and none was asked for` |
| 有 `join-words` 或 `split-words` 却缺少 `--language` | `--language is required by the tasks that check words: <tasks>` |
| `--language` 不是语言代码 | `--language: "<value>" is not a language: expected a locale code, like fr, en_US or sr@Latn` |
| 没有这两项任务却给出 `--language` | `--language is for the tasks join-words and split-words, which were not asked for` |
| 没有词典的语言 | `no dictionary for <language>` |
| 代码写法有误 | `--code: "<code>" is not a pattern code: expected Script[-language[-COUNTRY]], like Zyyy, Latn or Latn-en-US` |
| 有 `line-break` 却缺少 `--max-length` | `--max-length is required by the task line-break: Gaupol's 24 is a width in ems, and has no value in characters` |
| `--max-length` 不是大于零的数 | `--max-length: "<value>" is not a length: expected a number of characters greater than zero` |
| `--max-lines`、`--skip-lines` 不是整数行数 | `--max-lines: "<value>" is not a number of lines: expected a whole number greater than zero` |
| `--skip-length` 或 `--skip-lines` 无法读取 | `--skip-length: "<value>" is not a bound: expected a number greater than zero, or off` |
| 没有 `line-break` 却给出这四个选项之一 | `--max-length is for the task line-break, which was not asked for` |
| 未知的类别 | `--classes: "<name>" is not a class: expected human, ocr or human,ocr` |
| `--keep-blank-subtitles` 与 `--remove-blank-subtitles` 同用 | `--keep-blank-subtitles and --remove-blank-subtitles say opposite things; give one of them` |
| 模式名称有误 | 参见上表 |
| 任务没有任何有效模式 | `<task>: no pattern is active under the code <code>; switch one on with --enable` |
| `--range`、`-t`、`--document`、输出位置 | 参见[调用](invocation.md#错误)和[一份翻译](invocation.md#一份翻译) |
