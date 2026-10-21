# `disjoint`

<!-- machine-translated -->
> 本页为机器翻译，尚未经过熟悉该语言的读者审校。

```
subedit-cli disjoint (--output FILE | --output-dir DIR | --in-place)
                     [--minimum TIME] [--max-lines N] [--dashes] [--sort]
                     [--dry-run] [--recursive]
                     <file>...
```

切分相互重叠的字幕，使**任何 `[start, end]` 时间范围都不再与另一个范围重叠**。它就是窗口中的 `Make Ranges Disjoint…`——[同一个引擎](../subedit-gui/lines.md#使时间范围互不重叠)。

**不会缩短，也不会拉长任何内容**：在任何时刻，文件显示的文本都与之前相同。改变的只是时间轴的切分方式：

- 在重叠字幕组中，**每一个开始时间和每一个结束时间**处都会切分；
- 只被**一条**字幕覆盖的时间段，仍然是那条字幕；
- 被**多条**字幕覆盖的时间段，变成**一条**字幕，其各行就是这些字幕的行，**按它们开始时间的先后排列**——最早的在第 1 行，其次的在第 2 行；
- 不与任何字幕重叠的字幕不会被改动，其余字幕的顺序保持不变。

<!-- example: subedit-cli disjoint --help -->
```console
$ subedit-cli disjoint --help
Cut overlapping subtitles so that no range overlaps another, merging what overlaps
Usage: subedit-cli disjoint [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to make disjoint

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --minimum TIME              Shortest interval kept; a shorter one is taken over by a neighbour (default 0.08)
  --max-lines N               Most lines of a merged subtitle; more are shown in windows, 0 for no limit (default 2)
  --dashes                    Put a dash before the text of each merged subtitle
  --sort                      Put the subtitles in order of their start first; without it, a file out of order is refused
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## 参数和选项

| 选项 | 必需 | 取值 | 默认值 |
| :----- | :----- | :------ | :----- |
| `<file>...` | 是 | 一个或多个路径 | — |
| `--minimum` | 否 | 持续时间：`0.08`、`1.5`、`00:00:00,080`。取零则保留所有时间段 | `0.08`（约两帧） |
| `--max-lines` | 否 | 整数：合并后字幕的最大行数；`0` 表示不设上限 | `2` |
| `--dashes` | 否 | 开关：在每条字幕带入合并字幕的文本前加一个破折号 | 关闭 |
| `--sort` | 否 | 开关：**先**按开始时间的先后对字幕排序——参见[需要先排好序的文件](#需要先排好序的文件) | 关闭 |
| `--recursive`, `-r` | 否 | 开关 | 关闭——参见[处理目录树](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **三选一**，使用 `--dry-run` 时除外 | 参见[调用](invocation.md#输出位置) | — |
| `--dry-run` | 否 | 开关：只计算并说明，不写入——参见[先预览，再写入](invocation.md#先预览再写入) | 关闭 |

<!-- example: printf '1\n00:00:00,000 --> 00:00:05,000\nA\n\n2\n00:00:03,000 --> 00:00:08,000\nB\n\n' > chevauche.srt; subedit-cli disjoint --output disjoint.srt chevauche.srt; cat disjoint.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:05,000\nA\n\n2\n00:00:03,000 --> 00:00:08,000\nB\n\n' > chevauche.srt; subedit-cli disjoint --output disjoint.srt chevauche.srt; cat disjoint.srt
chevauche.srt: 2 subtitles split, 1 merged -> disjoint.srt
1
00:00:00,000 --> 00:00:03,000
A

2
00:00:03,000 --> 00:00:05,000
A
B

3
00:00:05,000 --> 00:00:08,000
B
```

`A [0, 5]` 和 `B [3, 8]` 得到三条互不重叠的字幕：单独的 `A`、`A` 与 `B` 一起、单独的 `B`。没有任何时刻丢失，也没有任何时刻增加。

## 操作的决定规则

**同一行绝不会重复出现**。两条文本相同的字幕，在重叠处只给出一行；两个显示相同内容的相邻片段，也只构成一条字幕。除此之外，一条被切成多个片段的字幕，其文本会**在每个片段中重复**：因为那就是该时刻屏幕上显示的内容。

**极短的时间段由相邻字幕接管**。短于 `--minimum` 的时间段会消失：相邻字幕**延长自己的时间范围来覆盖它**，因此没有任何时刻是空的，任何时间点的移动也不会超过该阈值。选中的相邻字幕，是其各行**已经包含**该时间段各行的那一条，否则选最长的那一条。只有在没有任何相邻字幕带有该时间段的文本时，这段文本才会丢失，而且丢失的时长小于阈值。**整条短于阈值的字幕绝不会被接管**：它会被保留，并计入计数。

**行数超过上限：分窗口显示**。合并后的字幕如果行数超过 `--max-lines`，会**按该大小分成相继的窗口**显示，各窗口平分持续时间。三行文本、持续三秒的例子：

<!-- example: printf '1\n00:00:00,000 --> 00:00:03,000\nA\n\n2\n00:00:00,000 --> 00:00:03,000\nB\n\n3\n00:00:00,000 --> 00:00:03,000\nC\n\n' > trois.srt; subedit-cli disjoint --output fenetres.srt trois.srt; cat fenetres.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:03,000\nA\n\n2\n00:00:00,000 --> 00:00:03,000\nB\n\n3\n00:00:00,000 --> 00:00:03,000\nC\n\n' > trois.srt; subedit-cli disjoint --output fenetres.srt trois.srt; cat fenetres.srt
trois.srt: 0 subtitles split, 1 merged; 1 shown in windows for holding too many lines -> fenetres.srt
1
00:00:00,000 --> 00:00:01,500
A
B

2
00:00:01,500 --> 00:00:03,000
B
C
```

四行文本得到三个窗口，依此类推：窗口每次滑动一行。没有任何内容被截断，也不会被再次切分。

**翻译遵循同样的切分**，因为一条字幕同时带有两种文本。**格式额外承载的内容**——样式、位置——取自第一条覆盖该时间段的字幕；合并的字幕互不相同时，这一损失会计入计数。

**结束时间早于开始时间的字幕**不会被猜测处理：它保持原位，并计入计数。

## 需要先排好序的文件

字幕组是由相邻字幕构成的连续序列：顺序混乱时，相邻的字幕并不是相互重叠的那些。因此，开始时间没有按顺序排列的文件会被**拒绝**，并由消息说明：

<!-- example: printf '1\n00:00:03,000 --> 00:00:08,000\nB\n\n2\n00:00:00,000 --> 00:00:05,000\nA\n\n' > desordre.srt; subedit-cli disjoint --output x.srt desordre.srt; echo "code $?" -->
```console
$ printf '1\n00:00:03,000 --> 00:00:08,000\nB\n\n2\n00:00:00,000 --> 00:00:05,000\nA\n\n' > desordre.srt; subedit-cli disjoint --output x.srt desordre.srt; echo "code $?"
desordre.srt: The subtitles are not in the order of their start: sort them first. Use --sort to do that here.
code 2
```

`--sort` 会先执行排序——与 [`sort`](sort.md) 相同——并作出说明。核心绝不会自行排序。

再运行一遍不会改变任何内容：时间范围一旦互不重叠，就没有什么可切分的了，并且会提示 `already disjoint`。

## 输出

**标准输出**——无内容：结果就是写入的文件。

| 级别 | 在标准错误输出中追加的内容 |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles split, M merged -> <destination>`，其后用 `;` 接上未能完整完成的事项：`K shown in windows for holding too many lines`、`K very short taken over by a neighbour`、`K subtitles shorter than the limit left whole`、`K merged with styles or positions that differ, the first kept`、`K subtitles ending before the start left alone`——或者 `<path>: already disjoint -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`，随后是**每一条读取诊断信息** |

`N subtitles split` 统计的是被切成多个片段的原始字幕数，`M merged` 统计的是同时显示多条字幕的片段数。

### 以 JSON 输出

使用 [`--format json`](invocation.md#脚本可读取的输出) 时，每个文件的 `counts` 如下：

| 计数 | 含义 |
| :----- | :----------- |
| `subtitles` | 写入文件中的字幕数 |
| `split`, `merged` | 与上述语句中的含义相同 |
| `windowed` | 分窗口显示的片段数 |
| `absorbed` | 由相邻字幕接管的时间段数 |
| `too_short` | 短于阈值而被整条保留的字幕数 |
| `ending_before_start` | 结束时间早于开始时间而被保持不动的字幕数 |
| `extras_dropped` | 样式或位置不同的合并片段数 |
| `moved` | 仅在使用 `--sort` 时：位置发生变化的字幕数 |

## 退出码

与该工具的退出码相同：所有文件都已处理（没有重叠的文件也算已处理）时为 `0`，一个都没有处理时为 `2`，只处理了一部分时为 `3`，用法错误时为 `1`。

## 错误

| 触发条件 | 消息 |
| :------------------- | :------ |
| 文件顺序混乱且没有 `--sort` | `<path>: The subtitles are not in the order of their start: sort them first. Use --sort to do that here.` |
| `--minimum` 不是持续时间，或为负数 | `--minimum: …` |
| `--max-lines` 不是整数 | `--max-lines: "…" is not a number of lines: expected a whole number, 0 for no limit` |

无法读取的文件与任何子命令一样会失败；输出位置的错误是所有写入文件的子命令共有的：参见[调用](invocation.md#输出位置)。

## 在其他命令中

[`convert`](convert.md#使时间范围互不重叠)、[`append`](append.md#使时间范围互不重叠)和 [`split-file`](split-file.md#使时间范围互不重叠)都接受 `--disjoint`，它在写入之前以默认设置完成同样的工作。

对于分段不当的文件——文本被切成前后相继的片段，且顺序混乱——只靠排序和切分，得到的行会在片段之间来回跳动：应先使用 [`join-repeats`](join-repeats.md)，或者使用依次执行这三项的 [`repair`](repair.md)。
