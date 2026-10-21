# `dialogue-dashes`

<!-- machine-translated -->
> 本页为机器翻译，尚未经过熟悉该语言的读者审校。

```
subedit-cli dialogue-dashes (--add | --remove) [--range N-M|N-]
                            (--output FILE | --output-dir DIR | --in-place)
                            [--dry-run]
                            [--recursive]
                            [--document main|translation] [-t FILE] [--align-method position|number]
                            <file>...
```

在各行开头加上**对话破折号**，或去掉。它对应[窗口](../subedit-gui/operations.md)中的 `Dialogue Dashes`。

**与 [`italics`](italics.md) 一样，有两个开关**：`--add` 和 `--remove` 互相排斥，且必须给出其中一个。

- **`--add`** 会在每个没有破折号的行前加上破折号：`Bonjour` 变为 `- Bonjour`，`Bonjour\nMarie` 变为 `- Bonjour\n- Marie`。**已有的破折号不会重复添加。**
- **三种破折号都能识别**——键盘上的连字符和排版用的两种长破折号——**写入的则是连字符**：`— Bonjour` 变为 `- Bonjour`。破折号后多余的空格会消失。
- 破折号加在**文本之前，而不是起始标签之前**：`<i>Bonjour</i>` 变为 `<i>- Bonjour</i>`。任何标签都不会移动。
- **`--remove`** 会去掉行首的破折号，三种形式均可。

<!-- example: subedit-cli dialogue-dashes --help -->
```console
$ subedit-cli dialogue-dashes --help
Put dialogue dashes on the lines, or take them off
Usage: subedit-cli dialogue-dashes [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --add                       Put a dialogue dash at the head of the lines
  --remove                    Take the dialogue dashes off
  --range N-M|N-              Act only on subtitles N to M, or N to the end
  --document main|translation The document to change: the main one, or the translation given by -t
  -t,--translation-file FILE  Translation file to lay over the subtitle file, for a single input
  --align-method position|number
                              How the lines of the translation find their subtitles
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
| `--add` / `--remove` | **二选一** | 开关 | — |
| `--range` | 否 | `N-M`，或 `N-` 直到结尾——参见 [`adjust`](adjust.md#--range) | 整个文件 |
| `--document` | 否 | `main` 或 `translation`——参见[一份翻译](invocation.md#一份翻译) | `main` |
| `-t`, `--translation-file` | 使用 `--document translation` 时必需 | 一个翻译文件，且只能有**一个**输入 | — |
| `--align-method` | 否，且只能与 `-t` 同用 | `position` 或 `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **三选一**，使用 `--dry-run` 时除外 | 参见[调用](invocation.md#输出位置) | — |
| `--dry-run` | 否 | 开关：只计算并说明，不写入任何内容——参见[先预览，再写入](invocation.md#先预览再写入) | 关闭 |

**默认处理主文本；使用 `--document translation -t FILE` 则处理翻译。** 此时更改并写出的是翻译，而且只有翻译——参见[一份翻译](invocation.md#一份翻译)。

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\nMarie\n\n2\n00:00:03,000 --> 00:00:04,000\n<i>Salut</i>\n\n' > a.srt; subedit-cli dialogue-dashes --add --output t.srt a.srt; cat t.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\nMarie\n\n2\n00:00:03,000 --> 00:00:04,000\n<i>Salut</i>\n\n' > a.srt; subedit-cli dialogue-dashes --add --output t.srt a.srt; cat t.srt
a.srt: 2 subtitles dashed -> t.srt
1
00:00:01,000 --> 00:00:02,000
- Bonjour
- Marie

2
00:00:03,000 --> 00:00:04,000
<i>- Salut</i>
```

## 输出

**标准输出**——为空：结果就是写出的文件。使用 `--dry-run` 时，输出更改列表，参见[先预览，再写入](invocation.md#先预览再写入)中的说明。

| 级别 | 在错误输出中增加的内容 |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles dashed -> <destination>`，或 `N subtitles undashed`，或 `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept`——所读文件的格式、编码、标记和行尾符，并按原样写回 |
| 3 | `<path>: N bytes read, M written`，然后是**每一条读取诊断信息**——参见[调用](invocation.md#读取诊断信息) |

`N` 是**文本发生了更改**的字幕数，由操作本身统计得出。

### 以 JSON 输出

使用 [`--format json`](invocation.md#脚本可读取的输出) 时，每个文件的对象带有 `counts.changed`（即上面那句话中的 `N`）和 `changes`，后者是发生更改的字幕列表（`{"subtitle", "document", "before", "after"}`，没有更改时为**空**）。

## 退出码

与整个工具相同：每个文件都已处理则为 `0`（没有任何更改的文件也算已处理），一个都没有处理则为 `2`，只处理了部分文件则为 `3`，用法错误则为 `1`。

## 错误

| 触发条件 | 消息 |
| :------------------ | :------ |
| `--add` 和 `--remove` 都没有给出 | `dialogue-dashes needs --add to put dashes on, or --remove` |
| 两者都给出 | `--add and --remove say opposite things; give one of them` |
| 范围写法有误，或超出文件 | 参见 [`adjust`](adjust.md#--range) |

输出位置的错误是所有会写入文件的子命令共有的：参见[调用](invocation.md#输出位置)。
