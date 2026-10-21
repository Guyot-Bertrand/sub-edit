# `transform`

<!-- machine-translated -->
> 本页为机器翻译，尚未经过熟悉该语言的读者审校。

```
subedit-cli transform --first <index>=<time> --last <index>=<time>
                      (--output FILE | --output-dir DIR | --in-place)
                      [--dry-run]
                      [--recursive]
                      <file>...
```

根据两个已知正确时间的点，修正文件中的**每一个**位置。字幕定时逐渐漂移——开头准确，结尾偏差——时就需要这项操作，而把所有内容移动同样量的 [`shift`](shift.md) 对此无能为力。

所读文件的格式会**保持不变**：更改格式是 [`convert`](convert.md) 的工作。

<!-- example: subedit-cli transform --help -->
```console
$ subedit-cli transform --help
Correct every position from two points known to be right
Usage: subedit-cli transform [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to transform

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --first TEXT REQUIRED       Earlier reference, as <index>=<time>: 1=00:00:01.000
  --last TEXT REQUIRED        Later reference, as <index>=<time>: 3=00:00:10.000
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
| `--first` | **是** | 一个参照点，`<index>=<time>` | — |
| `--last` | **是** | 一个参照点，`<index>=<time>` | — |
| `--output` / `--output-dir` / `--in-place` | **三者选一**，使用 `--dry-run` 时除外 | 参见[调用](invocation.md#输出位置) | — |
| `--dry-run` | 否 | 开关：只计算和说明，不写入——参见[先预览，再写入](invocation.md#先预览再写入) | 关闭 |

这两个选项没有短形式，只能写全称。

## 参照点的写法

参照点指明一条字幕，并说明它的**开始时间**现在应当落在哪里：

```
<index>=<time>
```

**序号从 1 开始计数**，与显示的序号以及 SubRip 文件的编号一致。`0` 会被拒绝，超过最后一条字幕的序号也会被拒绝。

**时间的写法与 [`shift`](shift.md) 相同**——秒或时间码，小数点是句点，精度为毫秒：

| 形式 | 示例 |
| :---- | :------- |
| 秒 | `10`, `2.999` |
| 时间码 | `00:00:10.000`, `01:39:37.040` |

`--first 3=10` 和 `--first 3=00:00:10.000` 含义相同。

`--first` 和 `--last` 指明的是两个点，而不是一个区间。它们的名称体现的是预期的用法——一个参照点在文件开头，另一个在文件结尾——而不是一种约束：把二者对调，结果相同，而且**两个参照点之外的内容与二者之间的内容一样按同一比例变化**。

## 其他位置会变成什么

两个参照点确定一个缩放系数，每个位置都遵循它：

```
t′ = y₁ + (t − x₁) × r      with r = (y₂ − y₁) / (x₂ − x₁)
```

`x₁`、`x₂` 是所指明的两条字幕当前的开始时间，`y₁`、`y₂` 是要求的时间。系数以精确的有理数保存，**取整只发生一次**，精确到毫秒；因此两个参照点会**精确地**落在要求的时间上，而不是只精确到千分之一以内（参见 [ADR 0013](../../../../adr/0013-mise-a-l-echelle-exacte-des-positions.md)）。

每条字幕在屏幕上停留的时间**按比例**变化：结束时间与开始时间遵循同一缩放。变换不是移动，也不具备移动的性质。

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,001 --> 00:00:07,000\nDeux.\n\n3\n00:00:09,000 --> 00:00:11,000\nTrois.\n\n' > a.srt; subedit-cli transform --first 1=00:00:01.000 --last 3=00:00:10.000 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,001 --> 00:00:07,000\nDeux.\n\n3\n00:00:09,000 --> 00:00:11,000\nTrois.\n\n' > a.srt; subedit-cli transform --first 1=00:00:01.000 --last 3=00:00:10.000 --output b.srt a.srt; cat b.srt
a.srt: 3 subtitles transformed onto 1=00:00:01.000 and 3=00:00:10.000 -> b.srt
1
00:00:01,000 --> 00:00:03,250
Un.

2
00:00:05,501 --> 00:00:07,750
Deux.

3
00:00:10,000 --> 00:00:12,250
Trois.
```

第一个参照点没有移动，第三个从 9 s 变为 10 s：系数是 `9/8`，而无人指明的第二条字幕从 `5,001` 变为 `5,501`，即 `1000 + 4001 × 9/8` 取整一次的结果。

## 无法确定变换的两个参照点

| 情形 | 何时发现 | 退出码 |
| :-------- | :------------- | :--- |
| `--first` 和 `--last` 指明同一个序号 | 在读取任何内容之前 | `1` |
| 某个序号超过最后一条字幕 | 打开文件时 | `2` 或 `3` |
| 所指明的两条字幕在同一时刻开始 | 打开文件时 | `2` 或 `3` |

这三种情形被拒绝的原因相同：`x₂ − x₁` 为零，两个重合的点确定不了一条直线。如果返回一个伪装成结果的除以零，比拒绝更糟。

第一种情形是**用法错误**——无论是哪些文件，都是这两个选项本身有问题，所以只需给出一条消息，而不是每个文件一条。另外两种取决于所打开的文件，因此算作处理失败。

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n' > a.srt; subedit-cli transform --first 2=1.000 --last 2=4.000 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n' > a.srt; subedit-cli transform --first 2=1.000 --last 2=4.000 --output b.srt a.srt; echo "code=$?"
--first and --last both name subtitle 2, and two references on one subtitle define no transform
code=1
```

越界的序号会被拒绝，并**指明边界**，这同时也说明了本来可以写什么：

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,000 --> 00:00:07,000\nDeux.\n\n' > a.srt; subedit-cli transform --first 1=1.000 --last 9=10.000 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,000 --> 00:00:07,000\nDeux.\n\n' > a.srt; subedit-cli transform --first 1=1.000 --last 9=10.000 --output b.srt a.srt; echo "code=$?"
a.srt: subtitle 9 is past the end: the file holds 2 subtitles
code=2
```

## 早于起点的位置会被拒绝

与 [`shift`](shift.md) 相同，原因也相同：负的位置会被写成 `-00:00:01,000`，这是九种格式中没有一种定义过的形式，也没有任何读取器接受。当某条字幕位于**第一个参照点之前**，而缩放把它推到零点以前时，就会出现这种情形。不会写入任何内容，退出码为 `2`，错误消息会指出有问题的那条字幕。

## 输出

**标准输出**——无内容：结果就是写出的文件。使用 `--dry-run` 时同样无内容：没有文件，说明行也会指出这一点——参见[先预览，再写入](invocation.md#先预览再写入)。

| 级别 | 在标准错误输出上追加的内容 |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles transformed onto 1=00:00:01.000 and 3=00:00:10.000 -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept`——所读文件的格式、编码、标记和行尾符，原样保留 |
| 3 | `<path>: N bytes read, M written`，然后是**每一条读取诊断信息**——参见[调用](invocation.md#读取诊断信息) |

参照点写回时一律使用小数点，无论输入时是怎样写的。

### 以 JSON 输出

使用 [`--format json`](invocation.md#脚本可读取的输出) 时，每个文件的对象包含 `counts.subtitles`（文件中的全部字幕）。完整的表格见[调用](invocation.md#各子命令在-counts-中放入的内容)。

## 退出码

与整个工具相同：所有文件都已写入则为 `0`；一个都没有写入则为 `2`；只写入了其中一部分则为 `3`；用法错误则为 `1`。

## 错误

| 触发情形 | 消息 |
| :------------------ | :------ |
| 参照点中没有 `=` | `"…" is not a reference: write it <index>=<time>, as in 3=00:00:10.000` |
| 参照点缺少一半 | `"…" is missing one of its two halves: write it <index>=<time>, as in 3=00:00:10.000` |
| 序号为 `0` | `"0" is not a subtitle number: subtitles are counted from 1, as the file shows them` |
| 序号不是整数 | `"…" is not a subtitle number: expected a whole number, counted from 1 as the file shows them` |
| 时间无效 | 与 [`shift`](shift.md) 的相同 |
| `--first` 和 `--last` 指向同一序号 | `--first and --last both name subtitle N, and two references on one subtitle define no transform` |
| 序号越界 | `<path>: subtitle N is past the end: the file holds M subtitles` |
| 两条字幕同时开始 | `<path>: subtitles N and M start at the same moment, so they define no transform` |
| 位置早于起点 | `<path>: subtitle N would land before the origin, which no subtitle file can hold` |

**不指定三个输出选项之一，就不会写入任何内容**，这些选项的错误是所有会写入的子命令共有的：参见[调用](invocation.md#输出位置)。
