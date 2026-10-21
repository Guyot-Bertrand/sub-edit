# `italics`

<!-- machine-translated -->
> 本页为机器翻译，尚未经过熟悉该语言的读者审校。

```
subedit-cli italics (--on | --off) [--range N-M|N-]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    <file>...
```

使用文件格式自身的标签，把文本设为**斜体**，或**取消斜体**。它就是[窗口](../subedit-gui/operations.md)中的 `Italic`。

**两个选项，而不是一个开关**：窗口中的按钮会向核心询问要朝哪个方向操作；这里则由用户明确指定。`--on` 和 `--off` 互相排斥，且必须给出其中之一——省略，或二者都给出，都是用法错误（退出码 `1`）。

- **`--on`** 先**删除文本中原有的**斜体标签，再给每段文本包上唯一的一对标签：已经是斜体的文本不会被包两次，被切成两段的斜体会重新合成一段。文本有多行时，一对标签覆盖整段文本。
- **`--off`** 删除斜体标签，**标签的其余部分予以保留**（`{\b1\i1}` 变成 `{\b1}`）。
- 文本**按文件格式书写斜体的方式**写入：SubRip、WebVTT 和 SubViewer 2 用 `<i>…</i>`，两种 Sub Station Alpha 用 `{\i1}…{\i0}`，MicroDVD 用 `{Y:i}`。

<!-- example: subedit-cli italics --help -->
```console
$ subedit-cli italics --help
Put the texts in italics, or take their italics out
Usage: subedit-cli italics [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --on                        Put the texts in italics
  --off                       Take the italics out
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
| `--on` / `--off` | **二选一** | 开关 | — |
| `--range` | 否 | `N-M`，或 `N-`（直到末尾）——参见 [`adjust`](adjust.md#--range) | 整个文件 |
| `--document` | 否 | `main` 或 `translation`——参见[一份翻译](invocation.md#一份翻译) | `main` |
| `-t`, `--translation-file` | 使用 `--document translation` 时必需 | 翻译文件，仅限**单个**输入 | — |
| `--align-method` | 否，且仅可与 `-t` 一起使用 | `position` 或 `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **三选一**，使用 `--dry-run` 时除外 | 参见[调用](invocation.md#输出位置) | — |
| `--dry-run` | 否 | 开关：只计算并说明，不写入——参见[先预览，再写入](invocation.md#先预览再写入) | 关闭 |

**默认处理主文本；使用 `--document translation -t FILE` 则处理翻译**。此时被更改并写入的是翻译，且只有翻译——参见[一份翻译](invocation.md#一份翻译)。

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour\nMarie\n\n' > a.srt; subedit-cli italics --on --output i.srt a.srt; cat i.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour\nMarie\n\n' > a.srt; subedit-cli italics --on --output i.srt a.srt; cat i.srt
a.srt: 2 subtitles put in italics -> i.srt
1
00:00:01,000 --> 00:00:02,000
<i>bonjour marie</i>

2
00:00:03,000 --> 00:00:04,000
<i>Bonjour
Marie</i>
```

## 没有样式的格式会被拒绝

**TMPlayer 和 LRC 不写入任何样式**：既没有可以设置的斜体，也没有可以取消的斜体。窗口会把该菜单项置灰而不是隐藏；命令行则**逐个文件地拒绝，并给出原因**，批处理中的其他文件继续处理。起决定作用的是格式能够承载什么，而不是它的书写方式：SubRip 和 WebVTT 书写斜体的方式相同，但在颜色上并不相同。

<!-- example: printf '00:00:01:Hello there\n00:00:03:Bye\n' > a.txt; subedit-cli italics --on --output-dir sortie a.txt; echo "code=$?" -->
```console
$ printf '00:00:01:Hello there\n00:00:03:Bye\n' > a.txt; subedit-cli italics --on --output-dir sortie a.txt; echo "code=$?"
a.txt: TMPlayer writes no style: there are no italics to put on
code=2
```

在 JSON 中，该失败带有标识符 **`no-style`**。

## 输出

**标准输出**——无内容：结果就是写入的文件。使用 `--dry-run` 时，输出更改列表，参见[先预览，再写入](invocation.md#先预览再写入)。

| 级别 | 在标准错误输出中追加的内容 |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles put in italics -> <destination>`，或 `N subtitles taken out of italics`，或 `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept`——所读取文件的格式、编码、标记和行尾符，按原样写回 |
| 3 | `<path>: N bytes read, M written`，随后是**每一条读取诊断信息**——参见[调用](invocation.md#读取诊断信息) |

`N` 是**文本发生了变化**的字幕数，由操作本身读出。

### 以 JSON 输出

使用 [`--format json`](invocation.md#脚本可读取的输出) 时，每个文件的对象包含 `counts.changed`（即上句中的 `N`）和 `changes`（发生变化的字幕列表，格式为 `{"subtitle", "document", "before", "after"}`，没有变化时为**空**）。

## 退出码

与该工具的退出码相同：所有文件都已处理（没有任何变化的文件也算已处理）时为 `0`，一个都没有处理时为 `2`，只处理了一部分时为 `3`，用法错误时为 `1`。

## 错误

| 触发条件 | 消息 |
| :------------------ | :------ |
| 既没有 `--on` 也没有 `--off` | `italics needs --on to put the texts in italics, or --off` |
| 二者都有 | `--on and --off say opposite things; give one of them` |
| 格式不支持样式 | `<path>: TMPlayer writes no style: there are no italics to put on`（或 `take out`） |
| 范围写法错误或超出文件 | 参见 [`adjust`](adjust.md#--range) |

输出位置的错误是所有写入文件的子命令共有的：参见[调用](invocation.md#输出位置)。
