# `split-file`

<!-- machine-translated -->
> 本页为机器翻译，尚未经过熟悉该语言的读者审校。

```
subedit-cli split-file --at N --head FILE --tail FILE
                       [--dry-run] [--sort] [--disjoint]
                       <file>
```

把一个文件**一分为二**：前半部分和后半部分。它相当于窗口中的 `Split Project…`——[同一个引擎](../subedit-gui/operations.md#split-project)，同样的计数，同样的拒绝——并且是 [`append`](append.md) 的**精确逆操作**：把后半部分追加到前半部分，就得回原来的文件。

- **`--at N` 是后半部分第一条字幕的编号**，编号方式与窗口表格相同，从 1 开始。`--at 3` 会把第 1、2 条字幕放入前半部分，把第 3 条及其后的字幕放入后半部分。取值范围**从第二条到最后一条**：切分处两侧各需要一条字幕。
- **后半部分会被移回起点**：它整体移动的量等于前半部分中最后一条字幕的结束时间的相反数，各位置的顺序和间距保持不变。这就是 `split-file` 成为 `append` 的逆操作的原因。
- **两部分都保留所读文件的格式、编码、字节序标记（BOM）和行尾符**，并且写入时从 1 起重新编号，与任何写回的文件相同。
- **有两个输出，所以有各自的输出位置**：`--head` 和 `--tail`，**两者都是必需的**。`split-file` 没有 `--output`、`--output-dir`、`--in-place` 和 `--recursive`，使用它们会像任何未知选项一样被拒绝，退出码为 `1`：既没有可供两个指定文件共用的目录，也没有可供覆盖的输入。

<!-- example: subedit-cli split-file --help -->
```console
$ subedit-cli split-file --help
Cut a subtitle file in two, a head and a tail
Usage: subedit-cli split-file [OPTIONS] file

Positionals:
  file TEXT REQUIRED          Subtitle file to cut

Options:
  -h,--help                   Print this help message and exit
  --at N                      Number of the first subtitle of the tail, as the table numbers them
  --head FILE                 File to write the subtitles before the cut to
  --tail FILE                 File to write the subtitles from the cut on to
  --dry-run                   Work out and say what would be written, and write nothing
  --sort                      Put the subtitles in order of their start before cutting; --at counts in that order
  --disjoint                  Cut overlapping subtitles so that no range overlaps another, before cutting
```

## 参数与选项

| 选项 | 必需 | 取值 | 默认值 |
| :----- | :----- | :------ | :----- |
| `<file>` | 是 | **唯一的**路径 | — |
| `--at` | 是 | 整数，从 2 到文件的字幕条数 | — |
| `--head`, `--tail` | **是，两者都需要**，使用 `--dry-run` 时除外 | 文件路径；其所在目录不存在时会被创建 | — |
| `--dry-run` | 否 | 开关：只计算和说明，不写入——参见[先预览，再写入](invocation.md#先预览再写入) | 关闭 |
| `--sort` | 否 | 开关：**在切分之前**先把字幕按开始时间排序；`--at` 按这个顺序计数 | 关闭 |
| `--disjoint` | 否 | 开关：**在切分之前**先切开相互重叠的字幕；`--at` 按处理后的结果计数，参见[使时间范围互不重叠](#使时间范围互不重叠) | 关闭 |

**使用 `--dry-run` 时不要求指定输出位置**：可以一个都不给，但不能只给两者之一——`--head` 和 `--tail` 必须同时出现。给出的输出位置仍然按没有 `--dry-run` 时的规则判断。

**两个输出位置既不能是输入，也不能是同一个文件**：覆盖正在读取的文件，或者把后半部分写到前半部分之上，都会在读取任何内容之前被拒绝，退出码为 `1`。

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n3\n00:00:06,000 --> 00:00:07,000\ntrois\n\n4\n00:00:08,000 --> 00:00:09,000\nquatre\n\n' > film.srt; subedit-cli split-file --at 3 --head tete.srt --tail queue.srt film.srt; cat queue.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n3\n00:00:06,000 --> 00:00:07,000\ntrois\n\n4\n00:00:08,000 --> 00:00:09,000\nquatre\n\n' > film.srt; subedit-cli split-file --at 3 --head tete.srt --tail queue.srt film.srt; cat queue.srt
film.srt: split at subtitle 3: 2 subtitles in the head, 2 subtitles in the tail -> tete.srt, queue.srt
1
00:00:02,000 --> 00:00:03,000
trois

2
00:00:04,000 --> 00:00:05,000
quatre
```

留在前半部分的第二条字幕在 4 s 结束：后半部分向前移动 4 s，原本在 6 s 开始的第三条字幕于是在 2 s 开始。

## 按时间顺序切分

`--at` 按**文件给出的顺序**给字幕编号。因此在顺序被打乱的文件上，前半部分未必是时间上较早的那一半。`--sort` 会**在判断切分之前**先按开始时间给字幕排序——与 [`sort`](sort.md) 一样是稳定的——`--at` 随后按这个顺序计数。

默认从不这样做。使用该选项时，文件会在第一级说明中指出有多少条字幕被移动（`<path>: N subtitles moved` 或 `<path>: already in order`），而 `--format json` 的记录会带有 `moved` 计数，**不使用该选项时没有这一项**。

## 使时间范围互不重叠

默认从不这样做。该选项会在判断切分之前、排序之后，按**默认设置**执行 [`disjoint`](disjoint.md) 所做的事：`--at` 按其结果计数。它要求文件已经有序：否则会被拒绝，而排在它前面的 `--sort` 可以事先把文件排好序。文件会在第一级说明中指出这一点（`<path>: N subtitles split, M merged`），而 `--format json` 的记录会带有 `split` 和 `merged` 计数，**不使用该选项时没有这些项**。

## 拒绝切分的情形

**在切分处重叠的两半不会被切开**。如果后半部分有一条字幕在前半部分最后一条字幕结束之前就已开始，移动会把它带到视频开头之前——得到负的位置，没有任何格式能写出。`split-file` 会拒绝并**指明那条字幕**，用的是窗口的措辞：

```text
o.srt: Cannot split at subtitle 3: subtitle 3 would fall before the start of the video. Cut somewhere else.
```

退出码为 `2`，并且**不写入任何内容**。请在别处切分。恰好落在起点上是允许的：零是一个合法的位置。

**`--at` 超出文件范围同样会被拒绝**，在读取文件之后判断：`--at 1`——第一条字幕不能作为后半部分的开头，否则前半部分什么也没有——以及 `--at` 超过最后一条字幕，那样后半部分会是空的。只有一条字幕的文件无法切分。

## 输出

**标准输出**——无内容：结果就是写出的两个文件。

| 级别 | 在标准错误输出上追加的内容 |
| :----- | :------------------------------------- |
| 1 | `<path>: split at subtitle N: H subtitles in the head, T subtitles in the tail -> <head>, <tail>`；使用 `--dry-run` 时，以 `(dry run, nothing written)` 取代 `-> …` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept`——所读文件的格式、编码、标记和行尾符，原样保留 |
| 3 | `<path>: N bytes read, H and T written`，然后是**每一条读取诊断信息**——参见[调用](invocation.md#读取诊断信息) |

### 以 JSON 输出

使用 [`--format json`](invocation.md#脚本可读取的输出) 时输出**单个对象**，其中 `file` 是被切分的文件。**`destination` 是前半部分**——即输入变成的那个文件——**`tail` 是后半部分**，为一个路径；使用 `--dry-run` 时与 `destination` 一样为 `null`。`counts` 包含：`subtitles`（所读文件的字幕条数）、`head` 和 `tail`（两部分各自的字幕条数）。

## 退出码

| 退出码 | 何时 |
| :--- | :---- |
| `0` | 两个文件都已写入——使用 `--dry-run` 时为已计算 |
| `1` | 用法错误：缺少 `--head` 和 `--tail`、使用了 `split-file` 没有的选项、输出位置就是输入或另一个输出位置、`--at` 不是正整数 |
| `2` | 文件无法读取，**切分被拒绝**（超出文件范围或存在重叠），或某个输出位置无法写入。没有退出码 `3`：只处理单个文件 |

**前半部分先写入**：如果磁盘拒绝写入后半部分，前半部分仍会保留下来，退出码为 `2`。

## 错误

| 触发情形 | 消息 |
| :------------------- | :------ |
| `--head` 和 `--tail` 都没有 | `no destination given: split-file writes two files, use --head and --tail` |
| 使用 `--dry-run` 时只给了两者之一 | `--head and --tail go together: give both, or neither with --dry-run` |
| 两个输出位置是同一个文件 | `<path>: named as both the head and the tail` |
| 某个输出位置就是输入 | `<path>: would be written over the input <input>` |
| `--at` 超出文件范围 | `<file>: --at 9: the file holds 3 subtitles, and the tail can start from 2 to 3` |
| 只有一条字幕的文件 | `<file>: a file of 1 subtitle cannot be split: a cut needs a subtitle on each side` |
| 重叠 | `<file>: Cannot split at subtitle N: subtitle M would fall before the start of the video. Cut somewhere else.` |
| 文件缺失、不可读或格式未知 | `<path>: does not exist`……——与其他地方相同 |
