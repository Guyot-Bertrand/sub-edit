# `join-repeats`

<!-- machine-translated -->
> 本页为机器翻译，尚未经过熟悉该语言的读者审校。

```
subedit-cli join-repeats (--output FILE | --output-dir DIR | --in-place)
                         [--dry-run]
                         [--recursive]
                         <file>...
```

把前后相接、显示相同内容的若干字幕变成**一条字幕**：一段文本被切成了前后相继的若干片段，某些分段工具就会留下这样的结果。它就是窗口中的 `Join Repeated Subtitles`——[同一个引擎](../subedit-gui/lines.md#连接重复的字幕)。

**重复字幕**是指与另一条字幕**文本相同、翻译相同、格式设置也相同**（样式、位置、提示设置）的字幕，并且**最迟在那条字幕结束时开始**：二者相接或重叠。一串重复字幕会变成**最先开始的那一条**，其结束时间延后到这一串中最晚的结束时间；其余的字幕则消失。

- **文件中的顺序无关紧要，也不会被改变**：保留的是最先开始的那一条字幕，其余字幕在哪里都一样。已排好序的文件仍然有序。
- **不会误连任何字幕**：文本不同、翻译不同、设置不同、两个片段之间有间隔、字幕没有文本或结束时间早于开始时间——这些情况下，字幕都保持分开。
- **应先于其余操作进行**：排序以及[消除重叠](disjoint.md)在处理干净的字幕串时效果更好。[`repair`](repair.md) 会依次执行这三项，[`convert`](convert.md#使时间范围互不重叠)也把它作为选项接受（`--join-repeats`、`--repair`）。

<!-- example: subedit-cli join-repeats --help -->
```console
$ subedit-cli join-repeats --help
Join the subtitles that repeat each other, one after the other, into one
Usage: subedit-cli join-repeats [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to join the repeats of

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
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
| `--output` / `--output-dir` / `--in-place` | **三选一**，使用 `--dry-run` 时除外 | 参见[调用](invocation.md#输出位置) | — |
| `--dry-run` | 否 | 开关：只计算并说明，不写入——参见[先预览，再写入](invocation.md#先预览再写入) | 关闭 |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:04,000 --> 00:00:06,000\nAlpha\n\n4\n00:00:08,000 --> 00:00:09,000\nBeta\n\n' > morceaux.srt; subedit-cli join-repeats --output joint.srt morceaux.srt; cat joint.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:04,000 --> 00:00:06,000\nAlpha\n\n4\n00:00:08,000 --> 00:00:09,000\nBeta\n\n' > morceaux.srt; subedit-cli join-repeats --output joint.srt morceaux.srt; cat joint.srt
morceaux.srt: 2 repeated subtitles joined -> joint.srt
1
00:00:01,000 --> 00:00:06,000
Alpha

2
00:00:08,000 --> 00:00:09,000
Beta
```

从 1 s 到 6 s 前后相接的三段 `Alpha` 现在只剩一条。中间隔着间隔的 `Beta` 则保持不变。再运行一遍不会改变任何内容，并会说明：`no repeated subtitles`。

## 输出

**标准输出**——无内容：结果就是写入的文件。

| 级别 | 在标准错误输出中追加的内容 |
| :----- | :------------------------------------- |
| 1 | `<path>: N repeated subtitles joined -> <destination>`，或 `<path>: no repeated subtitles -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`，随后是**每一条读取诊断信息** |

### 以 JSON 输出

使用 [`--format json`](invocation.md#脚本可读取的输出) 时，每个文件的 `counts` 包含：`subtitles`（写入文件中的字幕数）、`joined`（因被另一条字幕覆盖而删除的字幕数）和 `chains`（被连接的字幕串数）。

## 退出码

与该工具的退出码相同：所有文件都已处理（没有重复字幕的文件也算已处理）时为 `0`，一个都没有处理时为 `2`，只处理了一部分时为 `3`，用法错误时为 `1`。

## 错误

`join-repeats` 没有专属的错误：无法读取的文件与任何子命令一样会失败，输出位置的错误则是所有写入文件的子命令共有的：参见[调用](invocation.md#输出位置)。
