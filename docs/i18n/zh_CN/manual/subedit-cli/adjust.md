# `adjust`

<!-- machine-translated -->
> 本页为机器翻译，尚未经过熟悉该语言的读者审校。

```
subedit-cli adjust [--speed CPS|off] [--shorten | --no-shorten] [--lengthen | --no-lengthen]
                   [--minimum TIME|off] [--maximum TIME|off] [--gap TIME|off]
                   [--range N-M|N-]
                   (--output FILE | --output-dir DIR | --in-place)
                   [--dry-run]
                   [--recursive]
                   <file>...
```

按阅读速度和各项界限调整每条字幕的**持续时间**：过短而无法读完的字幕会被延长，过晚才离开屏幕的字幕可以被缩短，而且两条字幕不会重叠。**只有结束时间会移动**：移动开始时间就等于移动整条字幕，那是 [`shift`](shift.md) 的工作。它对应[窗口](../subedit-gui/operations.md#adjust-durations)中的 `Adjust Durations…` 操作，默认值也相同。

## 四个约束及其顺序

它们**按此顺序**应用，每个约束都会设定结束时间，最后一个约束胜出：

1. **阅读速度**——文本所需的持续时间，按每秒若干字符计算（标签不计入，换行计入）；
2. **最短持续时间**——低于它的持续时间会被提高到最短持续时间；
3. **最长持续时间**——高于它的持续时间会被降低到最长持续时间；
4. **间隔**——距离下一条字幕之前要留出的时间。它排在最后，而且**它胜出**：两条字幕同时显示是观众看得见的错误，而一条字幕稍短则不是。

文件的最后一条字幕没有下一条，因此也没有需要保留的间隔。一条字幕的下一条，指的是文件中的下一条，**不论它是否被** `--range` **选中**。

| 约束 | 选项 | 默认值 | 关闭方式 |
| :--------- | :----- | :----- | :------- |
| 速度 | `--speed CPS` | **15** 字符每秒 | `--speed off` |
| 速度的方向 | `--shorten`, `--no-shorten`, `--lengthen`, `--no-lengthen` | 只**延长** | — |
| 最短持续时间 | `--minimum TIME` | **1.5 s** | `--minimum off` |
| 最长持续时间 | `--maximum TIME` | **无** | `--maximum off` |
| 间隔 | `--gap TIME` | **0** | `--gap off` |

这些是 Gaupol 以及窗口首次打开时的设置。**`off` 表示关闭某个约束：不再检查它。** 这与把它设为零不同——最短持续时间设为 `0` 就是最短持续时间为零，间隔设为 `0` 则禁止任何重叠。

## 设置文件提供的内容

**上面的默认值是内置的默认值。** `adjust` 默认会读取设置文件中的操作设置——即 `Adjust Durations…` 在窗口中记住的取值，或 [`config set`](config.md) 写入的取值——它们会取代内置的默认值。**命令行上给出的选项始终优先**，其次是文件，最后才是内置的默认值。文件在哪里、`--config` 和 `--no-config`，参见[设置文件](invocation.md#设置文件)。

| 键 | 提供的内容 | 可以覆盖它的选项 |
| :-- | :--------------- | :----------------------- |
| `duration-adjust.speed` | 阅读速度，单位为字符每秒 | `--speed CPS` |
| `duration-adjust.lengthen`, `duration-adjust.shorten` | 速度可以向哪个方向移动结束时间；两者都为 `false` 则关闭速度约束 | `--lengthen`, `--no-lengthen`, `--shorten`, `--no-shorten`, `--speed off`；给出 `--speed CPS` 或某个方向，会把文件关闭的速度约束重新打开，且为延长 |
| `duration-adjust.minimum-enabled`, `duration-adjust.minimum-ms` | 最短持续时间，或无 | `--minimum TIME`, `--minimum off` |
| `duration-adjust.maximum-enabled`, `duration-adjust.maximum-ms` | 最长持续时间，或无 | `--maximum TIME`, `--maximum off` |
| `duration-adjust.gap-enabled`, `duration-adjust.gap-ms` | 最小间隔，或无 | `--gap TIME`, `--gap off` |

**每个键都有一个能撤销它的选项。** 每个方向都有一对开关，可以不顾文件怎么说而设定它：缩短用 `--shorten` 和 `--no-shorten`，延长用 `--lengthen` 和 `--no-lengthen`；没有给出的开关，则保持文件中的方向不变。`--maximum off` 会去掉文件中启用的最长持续时间。使用 `--format json` 时，`constraints` 给出实际使用的设置（不论来自何处），`settings` 给出读取的文件：

<!-- example: printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine/settings.conf config set duration-adjust.speed 12 2>/dev/null; subedit-cli --config mine/settings.conf --format json adjust --dry-run a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine/settings.conf config set duration-adjust.speed 12 2>/dev/null; subedit-cli --config mine/settings.conf --format json adjust --dry-run a.srt
settings: read mine/settings.conf
a.srt: adjusted the durations of 1 subtitle (dry run, nothing written)
{"schema":1,"command":"adjust","file":"a.srt","ok":true,"dry_run":true,"destination":null,"counts":{"subtitles":1,"adjusted":1,"sacrificed":{"speed":0,"minimum":0,"gap":0}},"constraints":{"speed":{"cps":"12","lengthen":true,"shorten":false},"minimum_ms":1500,"maximum_ms":null,"gap_ms":0},"settings":"mine/settings.conf","warnings":[]}
```

本页的其他示例都带 `--no-config` 运行，这样它们体现的就是内置默认值的效果，而与运行它们的机器上的设置无关。

<!-- example: subedit-cli adjust --help -->
```console
$ subedit-cli adjust --help
Bring the duration of every subtitle within a reading speed and limits
Usage: subedit-cli adjust [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to adjust

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --speed CPS|off             Reading speed in characters per second, or off (default 15)
  --shorten                   Let the reading speed bring an end earlier
  --no-shorten                Do not let the reading speed bring an end earlier
  --lengthen                  Let the reading speed move an end later
  --no-lengthen               Do not let the reading speed move an end later
  --minimum TIME|off          Shortest duration, or off (default 1.5)
  --maximum TIME|off          Longest duration, or off (default: none)
  --gap TIME|off              Least time left before the next subtitle, or off (default 0)
  --range N-M|N-              Act only on subtitles N to M, or N to the end
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
| `--speed` | 否 | 每秒字符数，**大于零**（`15`、`12.5`），或 `off` | `15` |
| `--shorten` | 否 | 开关：速度也可以把结束时间**提前** | 关闭 |
| `--no-shorten` | 否 | 开关：不论设置文件怎么说，速度都不再把结束时间**提前** | 关闭 |
| `--lengthen` | 否 | 开关：不论设置文件怎么说，速度都会把结束时间**推后** | 关闭 |
| `--no-lengthen` | 否 | 开关：速度不再把结束时间**推后** | 关闭 |
| `--minimum` | 否 | 大于或等于零的持续时间，或 `off` | `1.5` |
| `--maximum` | 否 | **大于零**的持续时间，或 `off` | 无 |
| `--gap` | 否 | 大于或等于零的持续时间，或 `off` | `0` |
| `--range` | 否 | `N-M`，或 `N-` 直到结尾——参见下文 | 整个文件 |
| `--output` / `--output-dir` / `--in-place` | **三选一**，使用 `--dry-run` 时除外 | 参见[调用](invocation.md#输出位置) | — |
| `--dry-run` | 否 | 开关：只计算并说明，不写入任何内容——参见[先预览，再写入](invocation.md#先预览再写入) | 关闭 |

持续时间的写法与 [`shift`](shift.md#持续时间的写法) 相同：`2.5`、`00:00:02.500`。

**如果没有任何有效的约束，`adjust` 会被拒绝**（退出码 `1`，不写入任何内容）：没有可以调整到的目标，而一个什么也不做却声称成功的命令，是最糟糕的回答。`--speed off --minimum off --gap off` 且没有最长持续时间就属于这种情况；只有 `--no-lengthen`、同时关闭最短持续时间和间隔，也属于这种情况：一个不移动任何结束时间的速度，什么也约束不了。

`--shorten`、`--no-shorten`、`--lengthen` 和 `--no-lengthen` 说明速度向哪个方向移动结束时间；它们只在有速度时才有意义，**`--speed off` 会拒绝它们**。把一个开关与它的反义开关一同给出——`--shorten --no-shorten`、`--lengthen --no-lengthen`——同样会被拒绝。

### `--range`

`--range 3-7` 把调整限制在第 3 到第 7 条字幕，**从 1 开始计数，包含两端**；`--range 3-` 一直到最后一条。单独一条字幕写作 `4-4`。

- **结束位置在开始位置之前**的范围、`0`、带符号的数或孤零零的一个数，都会被当作用法错误拒绝（退出码 `1`）。
- **它对批量处理中的每个文件都有效**，这只有在这些文件相似时才有意义。它会**针对每个文件**单独判断：超出该文件的范围，会在**动手之前**就被拒绝，并指出是哪个界限——`range 3-9 ends after the last subtitle: the file has 4`——，该文件失败（退出码 `2` 或 `3`，标识符为 `range-out-of-bounds`），其余文件继续处理。
- 报告中的计数只涉及被选中的字幕。

## 被放弃的约束

两个约束可能在同一条字幕上相互矛盾：1.5 s 的最短持续时间，放不进下一条字幕留出的 0.8 s。**由顺序决定，而报告会说明哪个约束让了步**——这是 Gaupol 没有做到的：它悄悄地破坏它们。

只有**至少有一条字幕没有满足**某个约束时，才会指出它，且是在应用了其他约束之后。最长持续时间没有计数：它在间隔之前应用，而间隔只能缩短，所以它总是成立。最短持续时间如果大于最长持续时间，就要让位于最长持续时间，并计入最短持续时间。

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --output ajuste.srt courts.srt; grep -e '-->' ajuste.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --output ajuste.srt courts.srt; grep -e '-->' ajuste.srt
courts.srt: adjusted the durations of 3 subtitles; could not satisfy the minimum duration in 1 subtitle -> ajuste.srt
00:00:00,000 --> 00:00:02,000
00:00:10,000 --> 00:00:14,000
00:00:14,200 --> 00:00:15,000
00:00:15,000 --> 00:00:16,500
```

第三条字幕本想在 15.7 s 结束，以持续 1.5 s，但第四条在 15.0 s 开始：**间隔胜出**，它在 15.0 s 结束，报告则指出最短持续时间。

关闭间隔就会让最短持续时间胜出——于是两条字幕重叠：

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --gap off --output libre.srt courts.srt; grep -e '-->' libre.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --gap off --output libre.srt courts.srt; grep -e '-->' libre.srt
courts.srt: adjusted the durations of 3 subtitles -> libre.srt
00:00:00,000 --> 00:00:02,000
00:00:10,000 --> 00:00:14,000
00:00:14,200 --> 00:00:15,700
00:00:15,000 --> 00:00:16,500
```

而当文件本来就在各项界限之内时，既不会放弃任何约束，也不会调整任何内容：

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --speed off --minimum off --output intact.srt courts.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --speed off --minimum off --output intact.srt courts.srt
courts.srt: no duration to adjust -> intact.srt
```

文件原样写出：**给出了输出位置，就会写入输出位置**。

## 输出

**标准输出**——为空：结果就是写出的文件。

| 级别 | 在错误输出中增加的内容 |
| :----- | :------------------------------------- |
| 1 | `<path>: adjusted the durations of N subtitles; could not satisfy …  -> <destination>`，或 `<path>: no duration to adjust -> <destination>`——并且最先写出所读取的设置文件——`settings: read <path>`——参见[设置文件](invocation.md#设置文件) |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept`——所读文件的格式、编码、标记和行尾符，并按原样写回 |
| 3 | `<path>: N bytes read, M written`，然后是**每一条读取诊断信息**——参见[调用](invocation.md#读取诊断信息) |

这句话与窗口的一致：`N` 是结束时间发生移动的字幕数，其余部分在没有任何约束让步时不会出现，列出未能满足的约束——`the reading speed`、`the minimum duration`、`the gap`——以及涉及的字幕数。

### 以 JSON 输出

使用 [`--format json`](invocation.md#脚本可读取的输出) 时，每个文件的对象除信封之外，还带有：

| 键 | 内容 |
| :-- | :------ |
| `counts.subtitles` | **目标**字幕：整个文件，或所选范围 |
| `counts.adjusted` | 结束时间发生移动的字幕 |
| `counts.sacrificed.speed`, `.minimum`, `.gap` | 在应用其他约束之后，仍然不满足该约束的字幕，**包括结束时间没有移动的字幕** |
| `constraints.speed` | `{"cps", "lengthen", "shorten"}`——每秒字符数，**为字符串**（`"15"`、`"12.5"`）——如果已关闭则为 `null` |
| `constraints.minimum_ms`, `.maximum_ms`, `.gap_ms` | 所用的持续时间，**以整毫秒计**，如果已关闭则为 `null` |
| `settings` | 所读取的设置文件，没有读取则为 `null` |

`constraints` 给出**实际使用的设置**，默认值也包括在内：这样，针对同样几条字幕的两次计算，无需阅读命令行就能比较。

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config --format json adjust --dry-run --maximum 1.8 courts.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config --format json adjust --dry-run --maximum 1.8 courts.srt
courts.srt: adjusted the durations of 4 subtitles; could not satisfy the reading speed in 1 subtitle, the minimum duration in 1 subtitle (dry run, nothing written)
{"schema":1,"command":"adjust","file":"courts.srt","ok":true,"dry_run":true,"destination":null,"counts":{"subtitles":4,"adjusted":4,"sacrificed":{"speed":1,"minimum":1,"gap":0}},"constraints":{"speed":{"cps":"15","lengthen":true,"shorten":false},"minimum_ms":1500,"maximum_ms":1800,"gap_ms":0},"settings":null,"warnings":[]}
```

## 退出码

与整个工具相同：每个文件都已处理则为 `0`，一个都没有处理则为 `2`，只处理了部分文件则为 `3`，用法错误则为 `1`。已经在各项界限之内的文件也算已处理。

## 错误

| 触发条件 | 消息 |
| :------------------ | :------ |
| `--config` 指定的文件不存在——用户自己的文件不存在时，使用默认值 | `settings file not found: <FILE>` |
| 没有任何有效的约束 | `adjust has nothing to adjust to: every constraint is off, or the reading speed is told to move no end` |
| 不是速度的值 | `--speed: "…" is not a reading speed: expected a number of characters per second above zero, like 15 or 12.5, or off` |
| 方向开关与 `--speed off` 同用 | `--lengthen, --no-lengthen, --shorten and --no-shorten say which way the reading speed may move an end, and --speed off switches the reading speed off` |
| 方向开关与其反义开关同用 | `--shorten and --no-shorten say opposite things; give one of them`（`--lengthen` 与 `--no-lengthen` 同理） |
| 不是时间的值 | `--minimum: "…" is not a time: …`（`--maximum`、`--gap` 同理） |
| 持续时间为负 | `--minimum: "-1" is not a duration of zero or more`（`--gap` 同理） |
| 最长持续时间为零或为负 | `--maximum: "0" is not a duration above zero` |
| 范围写法有误 | `--range: "…" is not a range: write it N-M, or N- to go to the end, counted from 1 and inclusive` |
| 范围的结束位置在开始位置之前 | `--range: "5-3" is not a range: it ends before it starts` |
| 范围超出文件 | `<path>: range N-M starts after the last subtitle: the file has K`，或 `… ends after …` |

输出位置的错误是所有会写入文件的子命令共有的：参见[调用](invocation.md#输出位置)。
