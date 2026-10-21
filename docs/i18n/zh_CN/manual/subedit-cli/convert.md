# `convert`

<!-- machine-translated -->
> 本页为机器翻译，尚未经过熟悉该语言的读者审校。

```
subedit-cli convert --to <format>
                    [--line-endings unix|windows|mac] [--to-encoding NAME]
                    [--bom | --no-bom] [--sort] [--disjoint]
                    [--join-repeats] [--repair]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    <file>...
```

把每个文件按要求的格式和形式写出。**从不更改输入**，除非 `--in-place` 明确要求。

<!-- example: subedit-cli convert --help -->
```console
$ subedit-cli convert --help
Write a subtitle file out in another format or shape
Usage: subedit-cli convert [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to convert

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --to TEXT:{srt,vtt,subviewer2,ssa,ass,microdvd,mpl2,tmplayer,lrc} REQUIRED
                              Format to write
  --line-endings TEXT:{unix,windows,mac}
                              Line endings to write; the source's by default
  --to-encoding NAME          Encoding to write; the source's by default
  --frame-rate RATE           Frame rate of a file counted in frames: 25, 23.976
  --bom                       Write a byte order mark
  --no-bom                    Write no byte order mark
  --sort                      Put the subtitles in order of their start before writing
  --disjoint                  Cut overlapping subtitles so that no range overlaps another, before writing
  --join-repeats              Join the subtitles that repeat each other into one, before writing
  --repair                    Join the repeats, sort, then make the ranges disjoint, before writing
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## 参数和选项

| 选项 | 必需 | 取值 | 默认值 |
| :----- | :----- | :------ | :----- |
| `<file>...` | 是 | 一个或多个路径 | — |
| `--recursive`, `-r` | 否 | 开关 | 关闭——参见[处理目录树](lots.md) |
| `--to` | **是** | 下表中的某个名称，不能是别的 | — |
| `--line-endings` | 否 | `unix`、`windows` 或 `mac` | 所读文件的行尾符 |
| `--to-encoding` | 否 | ICU 能写出的任何编码，但不包括自带标记的编码 | 所读文件的编码 |
| `--bom` / `--no-bom` | 否 | 开关，二者互斥 | 所读文件带有的标记 |
| `--sort` | 否 | 开关：**写入之前**把字幕按开始时间排好顺序，参见[写入时排序](#写入时排序) | 关闭 |
| `--join-repeats` | 否 | 开关：**最先**把同一段文本中依次出现的几个片段合成一条字幕，参见[使时间范围互不重叠](#使时间范围互不重叠) | 关闭 |
| `--repair` | 否 | 开关：等同于依次执行 `--join-repeats --sort --disjoint` | 关闭 |
| `--disjoint` | 否 | 开关：**写入之前**截短相互重叠的字幕，参见[使时间范围互不重叠](#使时间范围互不重叠) | 关闭 |
| `--output` / `--output-dir` / `--in-place` | **三选一**，使用 `--dry-run` 时除外 | 参见[调用](invocation.md#输出位置) | — |
| `--dry-run` | 否 | 开关：只计算并说明，不写入任何内容——参见[先预览，再写入](invocation.md#先预览再写入) | 关闭 |

`mac` 指单独的回车符（`\r`），即经典 Mac OS 的行尾符。

## `--to` 接受的格式

| 名称 | 格式 | 写出的扩展名 |
| :-- | :----- | :--------------- |
| `srt` | SubRip | `.srt` |
| `vtt` | WebVTT | `.vtt` |
| `subviewer2` | SubViewer 2.0 | `.sub` |
| `ssa` | Sub Station Alpha | `.ssa` |
| `ass` | Advanced SSA | `.ass` |
| `mpl2` | MPL2 | `.txt` |
| `microdvd` | MicroDVD | `.sub` |
| `tmplayer` | TMPlayer | `.txt` |
| `lrc` | LRC | `.lrc` |

**用的是名称，而不是扩展名。** 有两个扩展名各自对应两种格式——`.sub` 也是 MicroDVD 的扩展名，`.txt` 既是 MPL2 的，也是 TMPlayer 的——所以，如果选项接受扩展名，就无法知道要的是哪一种。上面的名称没有歧义，并且在扩展名本身没有歧义的地方，名称与扩展名一致。

**这个列表是封闭的。** 它取自 Gaupol，九种格式全部在内。

## 不带结束时间的两种格式

**TMPlayer 和 LRC 每行只写一个位置**，即开始时间。这在两个方面都有所体现。

**读取时**，结束时间是推断出来的：一行的结束时间就是下一行的开始时间，最后一行则取五秒。Gaupol 就是这样做的。我们也会说明这一点，方式是一条诊断信息，三个子命令中使用 `--verbose` 即可看到：

```
file.lrc: carries no end times; each one was taken from the next start, settled by the reader
```

**写入时**，不会写出结束时间，因为没有地方可放。因此，转换为这两种格式会丢失结束时间，往返转换也无法找回它们——只会重新推断。

**LRC 还会多丢失一样东西**：它无法容纳换行。分成两行的一句对白，会被写成一行，两行之间以一个空格连接。这一损失同样无法修复。

两者都没有表示斜体的词汇。

## `--frame-rate`，以及唯一按帧计数的格式

**MicroDVD 不按秒计数**：它的位置*就是*帧编号，而且没有任何 MicroDVD 文件会声明计数时采用的帧率。`--frame-rate` 提供这一信息，两个方向都用得上：它既是读取以帧计数的 `.sub` 时采用的帧率，也是转换为 MicroDVD 时按帧计数采用的帧率。

| 选项 | 必需 | 取值 | 默认值 |
| :----- | :----- | :------ | :----- |
| `--frame-rate` | 否 | 以每秒帧数表示的帧率：`25`、`23.976`、`29.97` | 见下文 |

**读取时，没有 `--frame-rate` 则按 23.976 读取文件**——这是 Gaupol 的取值——级别 `-vvv` 会说明这一点：`counts in frames and states no rate; it was read at`。这是工具中唯一一处，所显示的每个位置都建立在文件无法确认的假设之上。

**写入时，没有 `--frame-rate` 有三种情况，对应三种结果：**

| 起始文件 | 结果 |
| :------------------- | :-------------- |
| 本来就是 MicroDVD | 用读取时的帧率写回，帧与原来完全一致 |
| 基于时间，且位置落在某个网格上 | 采用该网格，`-vv` 会说明是哪一个 |
| 基于时间，没有网格 | **拒绝**，退出码 2：`writing frames needs a frame rate, and the positions fall on no grid to take one from — give --frame-rate` |

拒绝是有意为之。剩下的唯一办法就是编造一个数字，而文件的每一行都会随这个数字错多少而移动多少。

## 转换会丢失什么

**转换为另一种格式绝不是无损的**，哪些内容无法保留，取决于具体的格式组合：

| 无法保留的内容 | 原因 |
| :--------------------- | :------- |
| 头部 | 它只在自己的格式中才有意义：`[INFORMATION]` 不是 WebVTT 的头部 |
| 格式特有的数据 | SubRip 的坐标，以及 WebVTT 提示的标识符和设置，在别处都没有对应的内容 |
| 精度 | SubViewer 2、两种 Sub Station Alpha 和 LRC 精确到百分之一秒，MPL2 精确到十分之一秒，TMPlayer 精确到秒，MicroDVD 精确到帧；`01:00:00,017` 会变回 `01:00:00,020` |
| 一行的结束时间 | TMPlayer 和 LRC 没有相应的字段；下一次读取时会重新推断 |
| 换行 | 仅 LRC：它的每一行只能容纳一行文本，两行以一个空格接在一起 |
| 目标格式无法写出的标签 | WebVTT 没有 `<font color>`，TMPlayer 和 LRC 则完全没有标签 |
| 版式 | `{\pos(x,y)}`、`{\an8}`、`<v Marie>`、`<ruby>`：它们在自身格式的往返转换中得以保留，在转换为其他格式时则会消失 |

**以文件自己的格式写回，不会丢失任何内容**——这是其余一切的根本保证，对 `.srt` 文件使用 `--to srt` 会得到完全相同的字节。甚至不会进行任何解码：文本始终是读取到的原始字符串。

## 标签是转换的，不是复制的

**每种格式表示斜体的方式各不相同**，如果转换时不做转换，`.srt` 中就会留下一个 `{\i1}`：这是用户看得见的文本，而所在的文件永远不知道如何解释它。能够保留的有**六样东西**——粗体、斜体、下划线、颜色、字体和字号：

| 格式 | 粗体 | 斜体 | 下划线 | 颜色 | 字体 | 字号 |
| :----- | :--: | :------: | :------: | :-----: | :----: | :----: |
| SubRip | `<b>` | `<i>` | `<u>` | `<font color="#RRGGBB">` | — | — |
| WebVTT | `<b>` | `<i>` | `<u>` | — | — | — |
| SubViewer 2 | `<b>` | `<i>` | `<u>` | `<font color="#RRGGBB">` | — | — |
| Sub Station Alpha | `{\b1}` | `{\i1}` | — | `{\c&HBBGGRR&}` | `{\fnNAME}` | `{\fsN}` |
| Advanced SSA | `{\b1}` | `{\i1}` | `{\u1}` | `{\c&HBBGGRR&}` | `{\fnNAME}` | `{\fsN}` |
| MicroDVD | `{Y:b}` | `{Y:i}` | `{Y:u}` | `{C:$BBGGRR}` | `{F:NAME}` | `{S:N}` |
| MPL2 | `\` | `/` | `_` | `{C:$BBGGRR}` | `{F:NAME}` | `{S:N}` |
| TMPlayer | — | — | — | — | — | — |
| LRC | — | — | — | — | — | — |

**破折号表示已声明的损失。** 把带颜色的 `.srt` 转换为 WebVTT 会去掉颜色并说明这一点；生成的文件是 WebVTT，而不是伪装的 SubRip。

**MicroDVD 和 MPL2 无法为一行中的一部分设置样式。** 它们的标签没有结束标记：`{y:i}` 会让此后直到行尾的所有内容倾斜，`{Y:i}` 则直到字幕结尾。因此，一行中只有一半是粗体时，写出的结果是完全没有粗体，而不是让粗体溢出。

## 转换声明所丢失的内容

**有丢失时会说明，没有丢失时保持安静。** 每次调用都出现的报告，没有人会去读。

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,500\n<i>sur deux</i>\nlignes\n\n' > a.srt; subedit-cli convert --to lrc --output b.lrc a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,500\n<i>sur deux</i>\nlignes\n\n' > a.srt; subedit-cli convert --to lrc --output b.lrc a.srt
a.srt: 1 subtitle written as LRC -> b.lrc
a.srt: ends are not carried by LRC, line breaks were joined in 1 subtitle, 1 tag dropped
```

这一行出现在默认的详细级别，以及更高的每个级别。各项按如下顺序排列：

| 项 | 含义 |
| :---- | :----------- |
| `ends are not carried by <format>` | 目标格式不写结束时间 |
| `line breaks were joined in N subtitles` | 仅 LRC：有 N 条字幕原本是多行 |
| `N tags dropped` | 有 N 个标签无法写出，版式也包括在内 |
| `the header was dropped` | 头部无法跨越格式的边界 |
| `the <format> fields of N subtitles were dropped` | 有 N 条字幕带有其格式特有的数据 |
| `positions moved by up to N ms` | 位置移动的最大距离 |

## 更改编码

**`--to-encoding` 选择写出的编码；`--encoding` 说明读取时采用的编码。** 两者一起使用，就能为文件重新编码：

```console
$ subedit-cli convert --to srt --encoding cp1252 --to-encoding utf-8 \
      --output-dir sortie film.srt
```

没有 `--to-encoding` 时，文件以**读取时的编码**写出，行尾符和标记也是如此：转换时不指定编码，得到的是相同的字符，相同的字节。

**目标编码无法写出的字符会使该文件中止**，且不写入任何内容。`ł` 在 Latin-1 中没有位置；如果把它替换成 `?`，就会在读取和写入之间悄悄丢失文本，而无人察觉。

**字节序标记只存在于 Unicode 编码中。** 对不带标记的 `--to-encoding` 使用 `--bom` 会被拒绝，而不是被忽略——参见下面的错误表。

**而且编码必须指明字节顺序。** `UTF-16` 和 `UTF-32` 是 ICU 认识的名称，其转换器无论被要求什么，都会写出**自己的标记**：这样一来，`--no-bom` 会被接受却不被遵守。因此二者都会被拒绝，消息会指出所指的输出：

```console
$ subedit-cli convert --to srt --to-encoding UTF-16 --output copie.srt film.srt
"UTF-16" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do
```

`UTF-16LE`、`UTF-16BE`、`UTF-32LE` 和 `UTF-32BE` 写出的字节完全相同，标记也包括在内——但受 `--bom` 和 `--no-bom` 控制。因此，拒绝的代价只是换一个更精确的名称，而且读取和写入都是如此：`--encoding UTF-16` 会得到同样的回答。

## 输出位置

这三个选项及其规则是所有会写入文件的子命令共有的：参见[调用](invocation.md#输出位置)。没有其中之一，就不会写入任何内容。

`convert` 特有的是：**`--output-dir` 会随格式改变扩展名。** 对 `a.srt` 执行 `convert --to vtt` 会写出 `a.vtt`，绝不会写出一个内容是 WebVTT 的 `a.srt`——名称撒谎的文件，会让所有其他工具出错。

## 禁止原地转换

`--in-place` 与会改变格式的 `--to` 同用，退出码为 `1`，且不写入任何内容：原地写入时，没有第二个名称来承载新格式，文件会留在一个内容已不再相符的扩展名下。

`--in-place` 仍然可以用来更改**形式而不改变格式**——例如，把 `.srt` 以另一种行尾符写回 `.srt`。

这一拒绝**只根据扩展名**决定，并且在任何读取之前：用法错误绝不能使一批文件只写了一半。

## 行尾符与字节序标记

默认情况下，**写出的文件沿用所读文件带有的内容**。模型在读取时保留了这两者；如果默认强加它们，每次转换都会丢掉有意保留的一条信息。

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli convert --to vtt --output b.vtt a.srt; cat b.vtt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli convert --to vtt --output b.vtt a.srt; cat b.vtt
a.srt: 1 subtitle written as WebVTT -> b.vtt
WEBVTT

00:01.000 --> 00:03.000
Bonjour.
```

## 写入时排序

`--sort` 会把文件**按开始时间的顺序**写出，不论目标格式是什么。排序是**在副本上**进行的：所读文件不会被触动，写出的内容就是 [`sort`](sort.md) 会得到的结果——稳定排序，同时开始的两条字幕保持原有顺序。

**默认不排序**：只要没有别的要求，文件的顺序就是作者的顺序。使用该选项时，每个文件会在第一级输出中说明，写为 `<path>: N subtitles moved` 或 `<path>: already in order`，而 `--format json` 的记录中带有 `moved` 计数——**不使用该选项时没有这一项**，以免改变现有脚本所读取内容的形状。

## 使时间范围互不重叠

`--disjoint` 会截短相互重叠的字幕，使任何时间范围都不再与另一个重叠，**使用的副本与排序相同**：所读文件不会被触动。

默认不执行。该选项的作用与 [`disjoint`](disjoint.md) 相同，使用的是**它的默认设置**，在写入之前、排序之后进行。它要求文件已排好顺序：否则会被拒绝，而排在它前面执行的 `--sort` 会事先把文件排好顺序。文件会在第一级输出中说明这一点（`<path>: N subtitles split, M merged`），而 `--format json` 的记录中带有 `split` 和 `merged` 计数，**不使用该选项时没有这两项**。

**`--join-repeats` 排在排序和截短之前**：先重建被切成依次出现的若干片段的文本（参见 [`join-repeats`](join-repeats.md)），然后对字幕排序，再使时间范围互不重叠。**`--repair` 一次完成这三步**，与 [`repair`](repair.md) 相同，截短使用默认设置。使用 `--join-repeats` 时，每个文件会在第一级输出中说明 `<path>: N repeated subtitles joined`，而 `--format json` 的记录中带有 `counts.joined`——**不使用该选项时没有这一项**。

## 输出

**标准输出**——为空：结果就是写出的文件。使用 `--dry-run` 时同样为空：没有文件，说明由叙述行给出——参见[先预览，再写入](invocation.md#先预览再写入)。

在错误输出中，视级别而定：

| 级别 | 增加的内容 |
| :----- | :-------------- |
| 1 | `<path>: N subtitles written as WebVTT -> <destination>` |
| 2 | `<path>: SubRip -> WebVTT, UTF-8, no BOM, LF line endings`——所指的编码就是写出的编码 |
| 3 | `<path>: N bytes read, M written`，然后是**每一条读取诊断信息**——参见[调用](invocation.md#读取诊断信息) |

### 以 JSON 输出

使用 [`--format json`](invocation.md#脚本可读取的输出) 时，每个文件的对象带有 `counts.subtitles`，然后是转换所丢失的内容：`lost_ends` 和 `lost_header`（`0` 或 `1`）、`joined_lines`、`lost_tags`、`lost_fields` 和 `furthest_ms`——即[丢失声明语句](#转换声明所丢失的内容)中的各项。完整的表格见[调用](invocation.md#各子命令在-counts-中放入的内容)。

## 退出码

与整个工具相同：每个文件都已写出则为 `0`，一个都没有写出则为 `2`，只写出了部分文件则为 `3`，用法错误则为 `1`。

## 错误

| 触发条件 | 消息 |
| :------------------ | :------ |
| `--bom` 与 `--no-bom` 同用 | `--bom and --no-bom ask for opposite things; give one or the other` |
| `--in-place` 会改变格式 | `--in-place cannot change the format: the file would keep a name its content no longer matches` |
| 对没有标记的编码使用 `--bom` | `<path>: <encoding> has no byte order mark to write` |
| `--to-encoding` 指定了未知编码 | `no encoding is named "<name>"` |
| `--to-encoding` 指定了自带标记的编码 | `"<name>" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do` |
| 所写编码中没有该字符 | `<path>: holds a character the chosen encoding cannot write` |

**拒绝对不带标记的编码使用 `--bom`，并不是吹毛求疵。** 字节序标记只存在于 Unicode 编码中，其他编码都没有：要求在 Windows-1252 文件中加上标记，就是要求一样并不存在的东西。文件会在没有标记的情况下写出，而没有人会知道这一请求并未得到满足。

输出位置的错误是所有会写入文件的子命令共有的：参见[调用](invocation.md#输出位置)。

读取错误与 [`inspect`](inspect.md) 的相同：它们涉及输入文件，消息也相同。
