# `framerate`

<!-- machine-translated -->
> 本页为机器翻译，尚未经过熟悉该语言的读者审校。

> **这不是 [`snap`](snap.md)**。本命令**缩放整个文件**：用于定时有误、到影片结尾时偏差达数秒的情况。`snap` 把时间戳放回最近的网格上，不重新定时，用于定时本身已经正确的情况。二者的形式相同，都是一个帧率或两个帧率，把它们弄混不会产生任何错误提示。

```
subedit-cli framerate --from <fps> --to <fps>
                      (--output FILE | --output-dir DIR | --in-place)
                      [--dry-run]
                      [--recursive]
                      <file>...
```

把按某一帧率定时的文件重新定时为另一帧率。当字幕文件取自的版本与手头影片的版本不一致时——例如 PAL DVD 对 NTSC 转制版——就需要这一操作。

所读取文件的格式**予以保留**：更改格式是 [`convert`](convert.md) 的工作。

<!-- example: subedit-cli framerate --help -->
```console
$ subedit-cli framerate --help
Re-time a file mastered at one frame rate for another
Usage: subedit-cli framerate [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to re-time

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --from TEXT REQUIRED        Frame rate the file is timed at: 25, 23.976
  --to TEXT REQUIRED          Frame rate to time it for: 24, 29.97
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
| `--from` | **是** | 帧率，见下文 | — |
| `--to` | **是** | 帧率，见下文 | — |
| `--output` / `--output-dir` / `--in-place` | **三选一**，使用 `--dry-run` 时除外 | 参见[调用](invocation.md#输出位置) | — |
| `--dry-run` | 否 | 开关：只计算并说明，不写入——参见[先预览，再写入](invocation.md#先预览再写入) | 关闭 |

`--from` 是文件**当前**所用的定时帧率，`--to` 是希望得到的帧率。把二者对调会使文件朝相反方向偏移——在一部长片上，25 与 23.976 之间约相差七秒——而且在观看影片之前，没有任何提示。

## 帧率的写法

以每秒帧数表示，可带小数，小数点用作分隔符：

```
25      24      23.976      29.97      60
```

**零或负数的帧率会被拒绝**：它不是帧率。

### 三种 NTSC 帧率无法被精确书写

`23.976`、`29.97` 和 `59.94` 是**标签**。它们所指的帧率是 `24000/1001`、`30000/1001` 和 `60000/1001`，这些数都无法写成有限小数。本工具会把它们识别出来：

| 所写的内容 | 所指的帧率 |
| :------------- | :------------------ |
| `23.976` | `24000/1001`——转制为 NTSC 视频的电影 |
| `24` | `24`——电影 |
| `25` | `25`——PAL 和 SECAM 视频 |
| `29.97` | `30000/1001`——NTSC 视频 |
| `30`, `50`, `60` | 各自本身 |
| `59.94` | `60000/1001` |

识别依据的是**数值而不是字面**：`23.9760` 与 `23.976` 指的是同一个帧率。

其他任何小数都按字面理解——`23.9` 不是任何标准，其中没有什么可解读的含义。这也正是上表必不可少的原因：没有它，`23.976` 就会变成 `23976/1000`，命令行对同样的数字就会给出与窗口不同的结果。

## 位置会变成什么

```
t′ = round( t × (from / to) )
```

按 23.976 制作而以每秒 25 帧播放的影片播放得更快，因此它的字幕出现得更早；反过来，字幕则出现得更晚。

**系数是精确的有理数，舍入只进行一次**。看似等价的做法——先按输入帧率把位置转换为帧号，再按输出帧率转换回毫秒——并不等价：它把每个位置量化到输入帧网格上，误差最多可达半帧，实测为 21 ms。参见 [ADR 0013](../../../../adr/0013-mise-a-l-echelle-exacte-des-positions.md)。

实际上，从 25 到 23.976，系数是 `1001/960`：

| 读取的位置 | 写入的位置 | 经由帧得到的结果 |
| -----------: | -----: | -----------------------------------------: |
| `1 010 ms` | `1 053 ms` | `1 043 ms` |
| `3 600 017 ms` | `3 753 768 ms` | `3 753 750 ms` |

<!-- example: printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 25 --to 23.976 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 25 --to 23.976 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle retimed from 25 to 24000/1001 fps -> b.srt
1
00:00:01,053 --> 00:00:02,106
Un.
```

## 输出

**标准输出**——无内容：结果就是写入的文件。使用 `--dry-run` 时同样没有输出：没有文件，说明行会指出这一点——参见[先预览，再写入](invocation.md#先预览再写入)。

| 级别 | 在标准错误输出中追加的内容 |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles retimed from 25 to 24000/1001 fps -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept`——所读取文件的格式、编码、标记和行尾符，按原样写回 |
| 3 | `<path>: N bytes read, M written`，随后是**每一条读取诊断信息**——参见[调用](invocation.md#读取诊断信息) |

**说明行写出的是精确的帧率，而不是键入的标签**。在这一行中写 `23.976`，会报告一次并未发生的转换；只有在这一行中，才能看出实际用的是二者中的哪一个。能用小数精确写出的帧率在其中显示为小数，其余的显示为分数。

### 以 JSON 输出

使用 [`--format json`](invocation.md#脚本可读取的输出) 时，每个文件的对象包含 `counts.subtitles`（文件中的全部字幕数）。完整的表格见[调用](invocation.md#各子命令在-counts-中放入的内容)。

## 退出码

与该工具的退出码相同：所有文件都已写入时为 `0`，一个都没有写入时为 `2`，只写入了一部分时为 `3`，用法错误时为 `1`。

没有任何拒绝取决于文件的内容：两个帧率总能确定一个系数。在所有会写入文件的子命令中，只有它的全部拒绝都发生在第一次读取之前。

<!-- example: printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 0 --to 25 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 0 --to 25 --output b.srt a.srt; echo "code=$?"
"0" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed
code=1
```

## 错误

| 触发条件 | 消息 |
| :------------------ | :------ |
| 帧率为零或负数 | `"…" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed` |
| 使用了小数逗号 | `"…" is not a frame rate: use a decimal point and not a comma, in every language` |
| 文本不是数字 | `"…" is not a frame rate: expected frames per second, like 25 or 23.976` |
| 帧率大得离谱 | `"…" is not a frame rate: no video runs at that many frames per second` |

**如果没有三个输出位置选项之一，就不会写入任何内容**，这些选项的错误是所有写入文件的子命令共有的：参见[调用](invocation.md#输出位置)。
