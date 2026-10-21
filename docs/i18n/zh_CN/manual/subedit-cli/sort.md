# `sort`

<!-- machine-translated -->
> 本页为机器翻译，尚未经过熟悉该语言的读者审校。

```
subedit-cli sort (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 <file>...
```

把字幕恢复为**按开始时间排列的顺序**。其他内容都不变：文本、位置和文件格式均保持原样。写入时，字幕会从 1 起**重新编号**，与任何写回的文件相同。

**核心从不自行排序**：读取文件时保持其原有顺序，而 [`inspect`](inspect.md#anomalies-报告的内容) 会以 `subtitle N starts before the previous one starts` 报告顺序被打乱。`sort` 是**有意为之的操作**，针对顺序已被打乱的文件或批量文件——而且在窗口中可以像任何操作一样撤销。

- **排序是稳定的**：同一时刻开始的两条字幕**保持文件原有的先后顺序**。二者没有先后之分；调换它们就是一个无人要求的决定。
- **`sort` 既不接受时间范围，也不接受文档**：没有什么可选择的，顺序就是开始时间的顺序。

<!-- example: subedit-cli sort --help -->
```console
$ subedit-cli sort --help
Put the subtitles in the order of their start, keeping ties as they are
Usage: subedit-cli sort [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to sort

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## 参数与选项

| 选项 | 必需 | 取值 | 默认值 |
| :----- | :----- | :------ | :----- |
| `<file>...` | 是 | 一个或多个路径 | — |
| `--recursive`, `-r` | 否 | 开关 | 关闭——参见[处理目录树](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **三者选一**，使用 `--dry-run` 时除外 | 参见[调用](invocation.md#输出位置) | — |
| `--dry-run` | 否 | 开关：只计算和说明，不写入——参见[先预览，再写入](invocation.md#先预览再写入) | 关闭 |

<!-- example: printf '1\n00:00:05,000 --> 00:00:06,000\ncinq\n\n2\n00:00:01,000 --> 00:00:02,000\nun\n\n3\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > desordre.srt; subedit-cli sort --output ordre.srt desordre.srt; cat ordre.srt -->
```console
$ printf '1\n00:00:05,000 --> 00:00:06,000\ncinq\n\n2\n00:00:01,000 --> 00:00:02,000\nun\n\n3\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > desordre.srt; subedit-cli sort --output ordre.srt desordre.srt; cat ordre.srt
desordre.srt: 3 subtitles moved -> ordre.srt
1
00:00:01,000 --> 00:00:02,000
un

2
00:00:03,000 --> 00:00:04,000
trois

3
00:00:05,000 --> 00:00:06,000
cinq
```

## 计数的内容

**计数的是更换了字幕的位置个数**，与核心报告的一致——也就是窗口的历史记录所说的“已移动”。一条错位的字幕会牵动好几个位置：在上面的示例中，三个位置上的字幕都不是原来的那条，说明语句写的是 `3 subtitles moved`。

**已经有序的文件同样会被写出**，并会说明这一点：给出了输出位置就要写到输出位置，如果让 `sort` 成为例外，脚本就必须知道哪些子命令有时产生文件、有时什么也不产生。

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > ordre.srt; subedit-cli sort --output copie.srt ordre.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > ordre.srt; subedit-cli sort --output copie.srt ordre.srt
ordre.srt: already in order -> copie.srt
```

## 输出

**标准输出**——无内容：结果就是写出的文件。使用 `--dry-run` 时同样无内容：没有文件，说明行也会指出这一点——参见[先预览，再写入](invocation.md#先预览再写入)。

| 级别 | 在标准错误输出上追加的内容 |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles moved -> <destination>`，或 `<path>: already in order -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept`——所读文件的格式、编码、标记和行尾符，原样保留 |
| 3 | `<path>: N bytes read, M written`，然后是**每一条读取诊断信息**——参见[调用](invocation.md#读取诊断信息) |

### 以 JSON 输出

使用 [`--format json`](invocation.md#脚本可读取的输出) 时，每个文件的对象包含 `counts.subtitles`（文件中的全部字幕）和 `counts.moved`（更换了字幕的位置个数；已经有序的文件为 `0`）。

## 退出码

与整个工具相同：所有文件都已处理（已经有序的文件也算已处理）则为 `0`；一个都没有处理则为 `2`；只处理了其中一部分则为 `3`；用法错误则为 `1`。

## 错误

`sort` 没有专属的错误：不可读的文件与任何子命令一样失败，输出位置的错误则是所有会写入的子命令共有的：参见[调用](invocation.md#输出位置)。
