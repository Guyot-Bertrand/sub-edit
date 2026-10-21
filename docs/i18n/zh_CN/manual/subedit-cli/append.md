# `append`

<!-- machine-translated -->
> 本页为机器翻译，尚未经过熟悉该语言的读者审校。

```
subedit-cli append --output FILE
                   [--dry-run] [--sort] [--disjoint]
                   <base> <file>...
```

把文件接在**第一个文件之后**，合成**一个**文件：把分成两部分的影片，变成一套字幕。它对应窗口中的 `Append File…`——[同一个引擎](../subedit-gui/operations.md#append-file)，同样的计数——有多少个文件要追加，就应用多少次。

- **第一个文件是基准文件。** 结果的**格式、编码、字节序标记和行尾符**都取自它。
- **两个文件之间不会插入任何内容**：每个文件都按**它前面内容中最后一条字幕的结束时间**移动，其中各个位置保持原有的顺序和间隔。第三个文件接在第二个之后，而不是接在基准文件之后：各文件按给出的顺序依次相接。
- **每个被追加的文件都要经过转换**，转为基准文件的格式：它的标签会被转换，该格式无法写出的内容，则逐个文件、用窗口的措辞说明。
- **被追加的文件没有翻译**：单独打开的文件没有翻译可提供。
- **同一个文件可以给出多次**，也可以追加到它自己后面：这并不被禁止。

**`append` 不是批量处理。** N 个输入，**一个**输出：其他子命令的循环——每个文件一个结果，某个文件失败时其余文件继续——在这里没有意义。由此引出三点：

- **输出位置只能是 `--output`，别无其他。** `--output-dir` 和 `--in-place` 在 `append` 中并不存在，会像任何未知选项一样被拒绝，退出码为 `1`；`--recursive` 也是如此：文件的顺序就是给出的顺序，而遍历得到的目录树没有顺序；
- **要么全部成功，要么什么也不做**：第一个无法读取的文件会使运行停止，并且**不写入任何内容**。一部分成三部分的影片缺了其中一部分，就不是一部分成两部分的影片；
- **每次运行的结果只有一行**，在 JSON 中则**只有一个对象**。

<!-- example: subedit-cli append --help -->
```console
$ subedit-cli append --help
Put subtitle files one after another into a single file
Usage: subedit-cli append [OPTIONS] files...

Positionals:
  files BASE FILE...          The base file, then the files to put after it, in order

Options:
  -h,--help                   Print this help message and exit
  --output FILE               File to write
  --dry-run                   Work out and say what would be written, and write nothing
  --sort                      Put the subtitles in order of their start before writing
  --disjoint                  Cut overlapping subtitles so that no range overlaps another, before writing
```

## 参数和选项

| 选项 | 必需 | 取值 | 默认值 |
| :----- | :----- | :------ | :----- |
| `<base> <file>...` | 是，**至少两个** | 路径：先是基准文件，然后是按顺序接在其后的文件 | — |
| `--output` | **是**，使用 `--dry-run` 时除外 | 一个文件路径；其目录不存在时会创建 | — |
| `--dry-run` | 否 | 开关：只计算并说明，不写入任何内容——参见[先预览，再写入](invocation.md#先预览再写入) | 关闭 |
| `--sort` | 否 | 开关：**全部追加完毕之后**，把整体按开始时间排好顺序，参见[对结果排序](#对结果排序) | 关闭 |
| `--disjoint` | 否 | 开关：**全部追加并排序完毕之后**，截短相互重叠的字幕，参见[使时间范围互不重叠](#使时间范围互不重叠) | 关闭 |

**输出位置不能是任何一个输入**，基准文件也不例外：如果要写入的正是正在读取的文件，则在读取任何内容之前就会被拒绝，退出码为 `1`。与各个输入都不同的既有 `--output` 会被覆盖，这与别处一样。

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n' > partie1.srt; printf '1\n00:00:00,500 --> 00:00:01,500\ntrois\n\n' > partie2.srt; subedit-cli append --output film.srt partie1.srt partie2.srt; cat film.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n' > partie1.srt; printf '1\n00:00:00,500 --> 00:00:01,500\ntrois\n\n' > partie2.srt; subedit-cli append --output film.srt partie1.srt partie2.srt; cat film.srt
partie2.srt: appended 1 subtitle
partie1.srt: 3 subtitles from 2 files -> film.srt
1
00:00:01,000 --> 00:00:02,000
un

2
00:00:03,000 --> 00:00:04,000
deux

3
00:00:04,500 --> 00:00:05,500
trois
```

`partie1.srt` 的最后一条字幕在 4 s 时结束；`partie2.srt` 的那条字幕原本在 0.5 s 开始，现在在 4.5 s 开始。

## 对结果排序

每个文件都**从它前面内容的结束处**开始移动：因此可能出现顺序问题的，是文件本身的顺序。`--sort` 会在**最后一次追加之后一次性**把结果按开始时间排好顺序，效果与 [`sort`](sort.md) 相同——稳定排序，且不改变任何内容。

默认不排序。使用该选项时，基准文件会在第一级输出中说明，写为 `<base>: N subtitles moved` 或 `<base>: already in order`，而 `--format json` 的记录中带有 `moved` 计数，**不使用该选项时没有这一项**。

## 使时间范围互不重叠

默认不执行。该选项的作用与 [`disjoint`](disjoint.md) 相同，使用的是**它的默认设置**，在全部追加完毕之后，对整体执行。它要求文件已排好顺序：否则会被拒绝，而排在它前面执行的 `--sort` 会事先把文件排好顺序。文件会在第一级输出中说明这一点（`<path>: N subtitles split, M merged`），而 `--format json` 的记录中带有 `split` 和 `merged` 计数，**不使用该选项时没有这两项**。

## 格式不同时

结果采用**基准文件的格式**，不论其他文件是什么格式。跨格式所付出的代价，会逐个文件、用窗口的措辞说明——与 `Append File…` 以及[从其他格式粘贴](../subedit-gui/clipboard.md#其他格式的标签会被转换)时的句子相同：

```text
d.ass: appended 1 subtitle; Advanced SSA into SubRip: 1 tag dropped, the header was dropped
```

损失的内容只取决于**每个**被追加文件的格式：以 Advanced SSA 为基准文件时，可以接收 SubRip 文件而不损失任何内容，反过来则不行。要选择结果的格式，可以把已经是该格式的文件放在最前面，或者事后用 [`convert`](convert.md) 转换。

## 输出

**标准输出**——为空：结果就是写出的文件。

| 级别 | 在错误输出中增加的内容 |
| :----- | :------------------------------------- |
| 1 | 每个被追加的文件一行——`<path>: appended N subtitles`，格式带来损失时，后面接 `; <source format> into <base format>: <what is lost>`——然后是 `<base>: N subtitles from M files -> <destination>`（使用 `--dry-run` 时，用 `(dry run, nothing written)` 代替 `-> <destination>`） |
| 2 | `<base>: SubRip, UTF-8, no BOM, LF line endings kept`——基准文件的格式、编码、标记和行尾符，并按原样写回 |
| 3 | `<base>: N bytes read, M written`（所有文件合计），然后是**每一条读取诊断信息**——参见[调用](invocation.md#读取诊断信息) |

### 以 JSON 输出

使用 [`--format json`](invocation.md#脚本可读取的输出) 时，整个运行**只有一个对象**，其 `file` 是**基准文件**。它的 `counts` 如下：

| 计数 | 含义 |
| :----- | :----------- |
| `files` | 本次运行的文件，包括基准文件 |
| `subtitles` | 结果中的字幕 |
| `appended` | 来自被追加文件的字幕 |
| `lost_ends`, `lost_header` | `0` 或 `1`：被追加的文件中有一个丢失了它 |
| `joined_lines`, `lost_tags`, `lost_fields` | 各被追加文件所丢失内容的总和 |
| `furthest_ms` | 位置的最大移动量，所有文件合计 |

**`inputs` 数组**与 `counts` 并列，不包含在其中：每个被追加的文件一个元素，按顺序排列，为 `{"file", "counts"}`——其中 `counts` 含有 `appended`，以及同样的各项损失，**仅针对该文件**。在有十个文件的运行中，正是靠它才能知道哪个文件损失了什么。所有文件的读取警告都汇集在 `warnings` 中。

失败同样是一个对象，而且**只有一个**：即第一个无法读取的文件的对象。

## 退出码

| 退出码 | 何时 |
| :--- | :---- |
| `0` | 结果已写入——使用 `--dry-run` 时则为已计算出 |
| `1` | 用法错误：少于两个文件、没有 `--output`、使用了 `append` 没有的选项、输出位置是某个输入 |
| `2` | **有文件无法读取**，或输出位置无法写入：不写入任何内容。没有退出码 `3`：只写出一个文件的运行，要么成功，要么失败 |

## 错误

| 触发条件 | 消息 |
| :------------------- | :------ |
| 只给出了一个文件 | `append needs a base file and at least one file to put after it` |
| 既没有 `--output` 也没有 `--dry-run` | `no destination given: append writes one file, use --output` |
| `--output` 指定了某个输入 | `<destination>: would be written over the input <input>` |
| 文件不存在、无法读取或格式未知 | `<path>: does not exist`、`<path>: is in no format this tool knows`……——与别处相同 |

**空文件不会被追加**：没有任何一种格式能在没有至少一条字幕的情况下读取，因此这样的文件会像任何无法读取的文件一样被拒绝——`<path>: holds nothing recognisable as a subtitle`——这会使运行停止，且不写入任何内容。
