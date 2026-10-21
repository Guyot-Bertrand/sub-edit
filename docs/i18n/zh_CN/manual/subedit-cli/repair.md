# `repair`

<!-- machine-translated -->
> 本页为机器翻译，尚未经过熟悉该语言的读者审校。

```
subedit-cli repair (--output FILE | --output-dir DIR | --in-place)
                   [--minimum TIME] [--max-lines N] [--dashes]
                   [--dry-run] [--recursive]
                   <file>...
```

一次完成三条命令依次所做的事：先 [`join-repeats`](join-repeats.md)，再 [`sort`](sort.md)，最后 [`disjoint`](disjoint.md)。**必须按这个顺序，只有这个顺序行得通**：切分依据的是开始时间的顺序，而同一段文本的重复片段最好在这两步之前就先消除。对于分段不当、顺序混乱且相互重叠的文件，应当运行这条命令。

各项设置与 `disjoint` 相同，因为它位于链条的末端；这里没有 `--sort`，因为排序总是会执行。

<!-- example: subedit-cli repair --help -->
```console
$ subedit-cli repair --help
Join the repeated subtitles, put them in order, then make the ranges disjoint
Usage: subedit-cli repair [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to repair

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --minimum TIME              Shortest interval kept; a shorter one is taken over by a neighbour (default 0.08)
  --max-lines N               Most lines of a merged subtitle; more are shown in windows, 0 for no limit (default 2)
  --dashes                    Put a dash before the text of each merged subtitle
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## 参数与选项

| 选项 | 必需 | 取值 | 默认值 |
| :----- | :----- | :------ | :----- |
| `<file>...` | 是 | 一个或多个路径 | — |
| `--minimum` | 否 | 持续时间：`0.08`、`1.5`、`00:00:00,080`。零表示保留所有时间间隔 | `0.08`（约两帧） |
| `--max-lines` | 否 | 整数；`0` 表示取消限制 | `2` |
| `--dashes` | 否 | 开关：在合并后的字幕中，为每条字幕带来的文本前加上破折号 | 关闭 |
| `--recursive`, `-r` | 否 | 开关 | 关闭——参见[处理目录树](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **三者选一**，使用 `--dry-run` 时除外 | 参见[调用](invocation.md#输出位置) | — |
| `--dry-run` | 否 | 开关：只计算和说明，不写入——参见[先预览，再写入](invocation.md#先预览再写入) | 关闭 |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:01,000 --> 00:00:04,000\nBeta\n\n' > segments.srt; subedit-cli repair --output repare.srt segments.srt; cat repare.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:01,000 --> 00:00:04,000\nBeta\n\n' > segments.srt; subedit-cli repair --output repare.srt segments.srt; cat repare.srt
segments.srt: 1 repeated subtitle joined; already in order; 0 subtitles split, 1 merged -> repare.srt
1
00:00:01,000 --> 00:00:04,000
Alpha
Beta
```

`Alpha` 被切成前后相接的两段，现在又变回一条字幕；随后它与 `Beta` 占据相同的时间范围，二者合成一条，共两行，先开始的在前。

## 输出

**标准输出**——无内容：结果就是写出的文件。在标准错误输出上，第一级会输出**一句串联三项计数的说明**，以分号隔开：`join-repeats`、`sort` 和 `disjoint` 各自以自己的措辞给出的说明——`N repeated subtitles joined; M subtitles moved; K subtitles split, L merged…`。

### 以 JSON 输出

使用 [`--format json`](invocation.md#脚本可读取的输出) 时，每个文件的 `counts` 包含 `subtitles`、`joined`、`moved`，然后是 [`disjoint`](disjoint.md#以-json-输出) 的各项（`split`、`merged`、`windowed`、`absorbed`、`too_short`、`ending_before_start`、`extras_dropped`）。

## 退出码与错误

与整个工具相同。除各项设置之外，`repair` 没有专属的错误，设置按 `disjoint` 的方式判断：`--minimum` 不是持续时间或为负数，`--max-lines` 不是整数，均为退出码 `1`。
