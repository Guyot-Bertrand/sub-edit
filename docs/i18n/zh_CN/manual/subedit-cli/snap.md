# `snap`

<!-- machine-translated -->
> 本页为机器翻译，尚未经过熟悉该语言的读者审校。

```
subedit-cli snap --rate <rate>
                 (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 <file>...
```

把**每一个时间码**——开始和结束——都对齐到所给帧率下最近的一帧。不会重新定时：每个位置最多移动半帧。

## `snap` 不是 `framerate`，而且二者混淆后不会有任何提示

这两条命令形式相同——文件、输出位置、帧率——做的事情却截然相反。

| | [`framerate`](framerate.md) | `snap` |
| :--- | :-------------------------- | :----- |
| 计算内容 | `t' = t x input rate / output rate` | 在 `--rate` 的网格上取最近的一帧 |
| 移动幅度 | 与时间成正比——在一部长片上可达**数秒** | 最多半帧，**处处如此** |
| 适用的文件 | **定时有误**的文件 | 定时**正确**而网格有误的文件 |

**`snap` 的使用场景**，具体来说：一部每秒 25 帧的影片，一份按 24 帧的网格写成的字幕文件，各行已经大致就位，误差在几毫秒之内。没有什么需要重新定时——只需把时间码放回正确的帧上。

**`framerate` 的使用场景**则相反：一份按 25 帧母版定时的文件，想用于 23.976 帧的拷贝播放，到影片结尾时定时会偏离数秒。

把其中一个当作另一个来用，**不会产生任何错误**：两者都会接受文件并写出结果。能区分它们的是说明行——`snap` 总会说明它把位置移动了多少。

<!-- example: subedit-cli snap --help -->
```console
$ subedit-cli snap --help
Move every position onto the nearest frame of a frame rate (see framerate)
Usage: subedit-cli snap [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to align

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --rate TEXT REQUIRED        Frame rate to align on: 25, 23.976
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
| `--rate` | **是** | 以每秒帧数表示的帧率，写法与 [`framerate`](framerate.md#帧率的写法) 相同 | — |
| `--output` / `--output-dir` / `--in-place` | **三者选一**，使用 `--dry-run` 时除外 | 参见[调用](invocation.md#输出位置) | — |
| `--dry-run` | 否 | 开关：只计算和说明，不写入——参见[先预览，再写入](invocation.md#先预览再写入) | 关闭 |

`--rate` 接受**任何有效的帧率**，而不仅限于八个标准帧率：这由用户自行选择，与 `framerate` 相同。零或负数的帧率会被拒绝。

所读文件的格式会**保持不变**：更改格式是 [`convert`](convert.md) 的工作。

## 输出

**标准输出**——无内容：结果就是写出的文件。使用 `--dry-run` 时同样无内容：没有文件，说明行也会指出这一点——参见[先预览，再写入](invocation.md#先预览再写入)。

| 级别 | 在标准错误输出上追加的内容 |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles aligned on 25 fps, M positions moved, by at most 20 ms -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept`——所读文件的格式、编码、标记和行尾符，原样保留 |
| 3 | `<path>: N bytes read, M written`，然后是**每一条读取诊断信息**——参见[调用](invocation.md#读取诊断信息) |

**这两个数字是这一行的重点。**在每秒 25 帧下，半帧是二十毫秒，这里不可能写出比它更大的数。想用 `framerate` 却输入了 `snap` 的人，会在本该看到秒数的地方读到“最多 20 ms”，立刻就能发觉。

## 示例

一行位于 `00:00:01,010`，对齐到每秒 25 帧——也就是对齐到四十毫秒的整数倍：

<!-- example: printf '1\n00:00:01,010 --> 00:00:03,020\nUn.\n\n' > a.srt; subedit-cli snap --rate 25 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,010 --> 00:00:03,020\nUn.\n\n' > a.srt; subedit-cli snap --rate 25 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle aligned on 25 fps, 2 positions moved, by at most 20 ms -> b.srt
1
00:00:01,000 --> 00:00:03,040
Un.
```

## 两个性质

**对齐两次，第二次不会产生任何改变。**已经落在某一帧上的位置会留在原处：这是对同一个结果再次执行同样的操作。

**字幕的顺序保持不变。**取整到最近一帧是单调的，因此相隔至少一帧的两个开始时间仍然如此。如果更接近，它们可能合并——这是 [`inspect`](inspect.md#anomalies-报告的内容) 能够指出的异常，而不是顺序颠倒。

### 以 JSON 输出

使用 [`--format json`](invocation.md#脚本可读取的输出) 时，每个文件的对象包含 `counts.subtitles`、`counts.moved`（被移动的位置个数）和 `counts.furthest_ms`（最大的移动量，以毫秒计）。完整的表格见[调用](invocation.md#各子命令在-counts-中放入的内容)。

## 退出码

与整个工具相同：所有文件都已写入则为 `0`；一个都没有写入则为 `2`；只写入了其中一部分则为 `3`；用法错误则为 `1`。

## 错误

这是帧率的错误，与 [`framerate`](framerate.md#错误) 的逐字相同——二者通过同一路径读取同一个值：

| 触发情形 | 消息 |
| :------------------ | :------ |
| 帧率为零或负数 | `"…" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed` |
| 小数点使用了逗号 | `"…" is not a frame rate: use a decimal point and not a comma, in every language` |
| 文本不是数字 | `"…" is not a frame rate: expected frames per second, like 25 or 23.976` |
| 帧率大得离谱 | `"…" is not a frame rate: no video runs at that many frames per second` |

**不指定三个输出选项之一，就不会写入任何内容**，这些选项的错误是所有会写入的子命令共有的：参见[调用](invocation.md#输出位置)。
