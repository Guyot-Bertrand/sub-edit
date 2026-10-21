# `hearing-impaired`

<!-- machine-translated -->
> 本页为机器翻译，尚未经过熟悉该语言的读者审校。

```
subedit-cli hearing-impaired
                  (--output FILE | --output-dir DIR | --in-place)
                  [--dry-run]
                  [--recursive] [--parentheses]
                  [--document main|translation] [-t FILE] [--align-method position|number]
                  <file>...
```

从整个文件中删除**听障提示**——即写在方括号之间、描述声音的内容。**默认保留圆括号**：圆括号既常用于低声说出的台词，也常用于描述声音，删除它们会抹掉对白。对于用圆括号描述声音的文件，`--parentheses` 也会把它们一并删除。

本命令不做的事：井号之间的歌词、冒号前的说话者姓名、重新大写、更正 OCR 错误。这些模式属于窗口中的[`Correct Texts…`](../subedit-gui/correct-texts.md)助手，命令行以 [`correct`](correct.md) 的名义提供它；而这里的任何内容都不可设置：变换是既定的，不可配置。

<!-- example: subedit-cli hearing-impaired --help -->
```console
$ subedit-cli hearing-impaired --help
Remove the sounds described between square brackets
Usage: subedit-cli hearing-impaired [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to clean

Options:
  -h,--help                   Print this help message and exit
  --parentheses               Also remove the sounds between parentheses, often whispered lines
  -r,--recursive              Take directories as inputs, and every subtitle file in them
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
| `--parentheses` | 否 | 开关：同时删除圆括号之间的提示 | 关闭 |
| `--recursive`, `-r` | 否 | 开关 | 关闭——参见[处理目录树](lots.md) |
| `--document` | 否 | `main` 或 `translation`——参见[一份翻译](invocation.md#一份翻译) | `main` |
| `-t`, `--translation-file` | 使用 `--document translation` 时必需 | 翻译文件，仅限**单个**输入 | — |
| `--align-method` | 否，且仅可与 `-t` 一起使用 | `position` 或 `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **三选一**，使用 `--dry-run` 时除外 | 参见[调用](invocation.md#输出位置) | — |
| `--dry-run` | 否 | 开关：只计算并说明，不写入——参见[先预览，再写入](invocation.md#先预览再写入) | 关闭 |

所读取文件的格式**予以保留**：更改格式是 [`convert`](convert.md) 的工作。

**默认处理主文本；使用 `--document translation -t FILE` 则处理翻译**。此时被更改并写入的是翻译，且只有翻译——参见[一份翻译](invocation.md#一份翻译)。

**如果某条提示被删除后会使翻译变空，翻译会以空文本写入，并保留该字幕**：字幕仍留在文件中，只是没有文本。而主文本则会失去完全被清空的字幕，详见下文。

## 删除什么，保留什么

提示是一对方括号——使用 `--parentheses` 时也可以是一对圆括号——及其所包围的内容，**包括其中的换行**：在真实文件中，提示常常被行尾截断。

| 输入 | 输出 |
| :----- | :----- |
| `Bonjour [il tousse] Marie` | `Bonjour Marie` |
| `Bonjour[il tousse]Marie` | `Bonjour Marie` |
| `[Bruit de pas] Bonjour` | `Bonjour` |
| `A [un] [deux] B` | `A B` |

删除之后，被删内容两侧的文字之间**恰好留下一个空格，行首行尾则不留任何空白**。这并不是对文件中空格的清理：文本原本带有的两个连续空格，仍然是两个空格。

**当提示跨越换行时，保留下来的是换行**——两行仍然是两行。这种情况并非特例：在十五份真实的字幕文件中，每一个看似孤立的方括号，都是以这种方式被截断的提示。

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nReculez ! [Il\nhurle] Tout de suite !\n\n' > a.srt; subedit-cli --quiet hearing-impaired --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nReculez ! [Il\nhurle] Tout de suite !\n\n' > a.srt; subedit-cli --quiet hearing-impaired --output b.srt a.srt; cat b.srt
1
00:00:01,000 --> 00:00:03,000
Reculez !
Tout de suite !
```

**不属于提示、不会被改动的内容：**

| 情形 | 结果 |
| :-- | :-------------- |
| 纯数字内容——`[1]`、`(12)`、`[ 156478 ]` | 原样保留，包括分隔符：引用编号从来不是对声音的命名 |
| 圆括号及其所包围的内容，未使用 `--parentheses` | 原样保留：`Venez (chut) ici` 不会改变 |
| 没有对应闭合的方括号或圆括号 | 保持原样 |
| 格式标签——`<i>`、`</i>` | 绝不删除 |

数字、字母或符号**之间**有空白——`[1 2]`、`[1a]`、`[-1]`——就会被视为提示。内容为空的 `[]` 或 `( )` 也是如此：它没有数字，因此不是引用编号；没有任何内容，因此也没什么可保留的。

## 哪些内容会彻底消失

**被删除操作清空的行会消失**，**被完全清空的字幕会从文件中删除**。其后的字幕会重新编号。

如果一行中只剩下空白、格式标签或孤立的对话破折号，就视为空行——`<i>[Musique]</i>` 不会在屏幕上留下一个 `<i></i>`。

**标签取决于文件的格式**。Advanced SSA 中的 `{\i1}[Musique]{\i0}`、MicroDVD 中的 `{Y:i}[Musique]` 和 MPL2 中的 `/[Musique]` 同样不会留下任何内容。反过来，在不写入标签的格式——TMPlayer、LRC——中，`<i>` 是观众会读到的文本，包含它的行并不是空行。

**只剩一个说话者的对话不再是对话**，剩下的那个破折号也会去掉。只要还有两个说话者，破折号就会保留。

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\nVoir [1] la note.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\nVoir [1] la note.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle cleaned, 1 removed -> b.srt
1
00:00:04,000 --> 00:00:06,000
Attends Marie.

2
00:00:07,000 --> 00:00:09,000
Voir [1] la note.
```

## 没有任何提示的文件同样会被写入

文件会被原样复制，退出码为 `0`，报告会说明没有可删除的内容。

这是写入文件的各子命令的规则：**给定了输出位置，就会写入该位置**。若在这里破例，脚本就必须知道哪些子命令有时生成文件、有时什么也不生成。

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nRien à signaler.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nRien à signaler.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt
a.srt: no mention to remove -> b.srt
```

## 输出

**标准输出**——无内容：结果就是写入的文件。**使用 `--dry-run` 时，输出的是建议的更改**：对每条文本会发生变化的字幕，给出它的编号、更改前的文本和更改后的文本——参见[先预览，再写入](invocation.md#先预览再写入)。

| 级别 | 在标准错误输出中追加的内容 |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles cleaned, M removed -> <destination>`，或 `<path>: no mention to remove -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept`——所读取文件的格式、编码、标记和行尾符，按原样写回 |
| 3 | `<path>: N bytes read, M written`，随后是**每一条读取诊断信息**——参见[调用](invocation.md#读取诊断信息) |

`N` 统计文本发生变化的字幕数，`M` 统计被删除操作完全清空、因而离开了文件的字幕数。

### 以 JSON 输出

使用 [`--format json`](invocation.md#脚本可读取的输出) 时，每个文件的对象包含 `counts.cleaned`（被重写的文本数）和 `counts.removed`（被删除的字幕数）；**使用 `--dry-run` 时**，`changes` 键还会列出发生变化的字幕，与文本形式的标准输出一致。完整的表格见[调用](invocation.md#各子命令在-counts-中放入的内容)。

## 退出码

与该工具的退出码相同：所有文件都已写入时为 `0`，一个都没有写入时为 `2`，只写入了一部分时为 `3`，用法错误时为 `1`。

## 错误

这个子命令没有专属的错误：它没有需要解析的值，也没有任何文本会使它拒绝某个文件。可能失败的只有读取或写入，与其他子命令相同。

**如果没有三个输出位置选项之一，就不会写入任何内容**，这些选项的错误是所有写入文件的子命令共有的：参见[调用](invocation.md#输出位置)。
