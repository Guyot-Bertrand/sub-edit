# `shift`

<!-- machine-translated -->
> 本页为机器翻译，尚未经过熟悉该语言的读者审校。

```
subedit-cli shift (--by <time> | --to-grid)
                  (--output FILE | --output-dir DIR | --in-place)
                  [--dry-run]
                  [--recursive]
                  <file>...
```

把文件中的**每一个**位置都向前或向后移动同样的量。没有哪条字幕在屏幕上停留的时间变长或变短：这正是“移动”之所以为移动的原因。

<!-- example: subedit-cli shift --help -->
```console
$ subedit-cli shift --help
Move every position of a file by a fixed amount
Usage: subedit-cli shift [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to shift

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --by TEXT                   Amount to move by: 2.999, -7.001, or 00:00:07.001
  --to-grid                   Move by the amount that puts the positions back on their frame grid
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
| `--by` | **二者选一** | 带符号的持续时间，见下文 | — |
| `--to-grid` | **二者选一** | 开关：移动量由测量得出，见下文 | — |
| `--output` / `--output-dir` / `--in-place` | **三者选一**，使用 `--dry-run` 时除外 | 参见[调用](invocation.md#输出位置) | — |
| `--dry-run` | 否 | 开关：只计算和说明，不写入——参见[先预览，再写入](invocation.md#先预览再写入) | 关闭 |

所读文件的格式会**保持不变**：更改格式是 [`convert`](convert.md) 的工作。

## `--to-grid`：测量得出的移动量，而非给定的移动量

如果文件中的位置落在帧网格上，**只相差一个常量**，就说明它被移动过，而这个常量可以根据位置本身测量出来。`--to-grid` 会测量并应用这个常量，无需任何人事先知道它。

移动量**因文件而异**：从同一个网格上偏移了不同量的两个文件，各自按自己的量复原。该量连同找到的帧率会写在说明语句中。

**取最短的路径**。超过半帧时，最近的网格线是下一条：修正会向前进行，而不是向后移动将近整整一帧。

<!-- example: k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40 + 7)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > decale.srt; subedit-cli shift --to-grid --output recale.srt decale.srt -->
```console
$ k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40 + 7)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > decale.srt; subedit-cli shift --to-grid --output recale.srt decale.srt
decale.srt: 12 subtitles shifted by -0.007 s onto their 25 fps grid -> recale.srt
```

**不在任何已知网格上的文件会被拒绝，并说明原因**。没有什么可以复原，而且根据噪声测得的相位会让文件随机移动。若想在尝试这项操作之前了解某个文件的情况，参见 [`inspect`](inspect.md#frame-rate-grid-报告的内容)。

`--by` 和 `--to-grid` 都表示移动多少：同时给出二者是用法错误，二者都不给出也是用法错误。

**`--to-grid` 不是 [`snap`](snap.md)**。它把整个文件移动同样的量，并**精确地**保持相对定时；`snap` 则逐个独立地移动每个位置，其间会把手动修正过的位置也一并吸收。对于干净的文件，二者结果相同；对于只有一部分在网格上的文件，则不相同。

## 持续时间的写法

有两种形式，也只有两种：

| 形式 | 示例 |
| :---- | :------- |
| 秒 | `30`, `2.999`, `-7.001`, `+1.5` |
| 时间码 | `00:00:07.001`, `01:39:37.040`, `-00:00:07.001` |

二者含义相同：`--by 7.001` 和 `--by 00:00:07.001` 产生同样的文件。

**小数点是句点**，因为命令行使用英语。逗号会被拒绝而不会被解释：世界上有一半的地方把七千零一写作 `7,001`，将它读作七秒只是一种猜测——而对那一半来说，这个猜测是错的。

**精度为毫秒**。更细的精度会被拒绝，而不是悄悄取整：默默取整得到的结果，与所要求的完全不相符。

<!-- example: printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle shifted by -7.001 s -> b.srt
1
00:00:02,999 --> 00:00:04,999
Bonjour.
```

## 早于起点的移动会被拒绝

向前移动到零点之前会得到退出码 `2`，并且**不写入任何内容**。错误消息会指出无法承受这次移动的那条字幕，这同时也说明了最多能移动多远。

原因并非出于原则：早于起点的位置会被写成 `-00:00:01,000`，这是九种格式中没有一种定义过的形式，也没有任何读取器接受。核心确实允许这样的位置——拒绝它们会使编辑操作成为特例——但文件终究不同于内存中的模型。

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; echo "code=$?"
a.srt: subtitle 1 would start before the origin, which no subtitle file can hold
code=2
```

## 输出

**标准输出**——无内容：结果就是写出的文件。使用 `--dry-run` 时同样无内容：没有文件，说明行也会指出这一点——参见[先预览，再写入](invocation.md#先预览再写入)。

| 级别 | 在标准错误输出上追加的内容 |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles shifted by 2.999 s -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept`——所读文件的格式、编码、标记和行尾符，原样保留 |
| 3 | `<path>: N bytes read, M written`，然后是**每一条读取诊断信息**——参见[调用](invocation.md#读取诊断信息) |

### 以 JSON 输出

使用 [`--format json`](invocation.md#脚本可读取的输出) 时，每个文件的对象包含 `counts.subtitles`（文件中的全部字幕）和 `counts.shifted_by_ms`（移动量，以整毫秒计，**带符号**）。完整的表格见[调用](invocation.md#各子命令在-counts-中放入的内容)。

## 退出码

与整个工具相同：所有文件都已写入则为 `0`；一个都没有写入则为 `2`；只写入了其中一部分则为 `3`；用法错误则为 `1`。

## 错误

| 触发情形 | 消息 |
| :------------------ | :------ |
| 小数点使用了逗号 | `"…" is not a time: use a decimal point and not a comma, in every language` |
| 精度细于毫秒 | `"…" is not a time: positions are held to the millisecond, no finer` |
| 文本不是持续时间 | `"…" is not a time: expected seconds like 2.999, or a timestamp like 00:00:07.001` |
| 移动到起点之前 | `<path>: subtitle N would start before the origin, which no subtitle file can hold` |

**不指定三个输出选项之一，就不会写入任何内容**，这些选项的错误是所有会写入的子命令共有的：参见[调用](invocation.md#输出位置)。
