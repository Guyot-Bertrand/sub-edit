# `replace`

<!-- machine-translated -->
> 本页为机器翻译，尚未经过熟悉该语言的读者审校。

```
subedit-cli replace [--regex] [--case-sensitive] [--range N-M|N-]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    <pattern> <replacement> <file>...
```

替换字幕中的文本，**同时不破坏任何标签**。它相当于窗口[搜索](../subedit-gui/find-and-replace.md)中的 `Replace All` 操作，遵循同样的规则：

- **搜索针对可见文本**——即观看者读到的内容。标签不在其中：查找 `<i>` 什么也找不到，而查找 `Bonjour` 能找到其中一半为斜体的“Bonjour”；
- **替换发生在源文本中**，即文件中的文本，标签留在原来的位置。如果一个标签把要查找的词切开，它会包住整个替换文本；如果一种样式与匹配项相邻，它会覆盖整个替换文本；匹配项没有触及的内容不会移动。逐种情形的规则与窗口的搜索相同。

**没有 `find` 子命令**：若要在不做任何更改的情况下查看某个模式出现在哪里，可以用 [`--dry-run`](invocation.md#先预览再写入)，它会列出每条相关的字幕。

<!-- example: subedit-cli replace --help -->
```console
$ subedit-cli replace --help
Replace a text in the subtitles, without breaking a tag
Usage: subedit-cli replace [OPTIONS] pattern replacement files...

Positionals:
  pattern TEXT REQUIRED       Text to look for in what is shown
  replacement TEXT REQUIRED   What replaces it, tags allowed
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --regex                     Read the text to look for as a regular expression
  --case-sensitive            Tell capitals from small letters
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

## 参数与选项

| 选项 | 必需 | 取值 | 默认值 |
| :----- | :----- | :------ | :----- |
| `<pattern>` | 是 | 要在显示的内容中查找的文本；**不能为空** | — |
| `<replacement>` | 是 | 用来替换的内容，可以含标签；可以为空 | — |
| `<file>...` | 是 | 一个或多个路径 | — |
| `--recursive`, `-r` | 否 | 开关 | 关闭——参见[处理目录树](lots.md) |
| `--regex` | 否 | 开关：模式是 ICU 正则表达式 | 纯文本 |
| `--case-sensitive` | 否 | 开关：区分大写字母和小写字母 | 忽略大小写 |
| `--range` | 否 | `N-M`，或 `N-` 表示直到末尾——参见 [`adjust`](adjust.md#--range) | 整个文件 |
| `--document` | 否 | `main` 或 `translation`——参见[一份翻译](invocation.md#一份翻译) | `main` |
| `-t`, `--translation-file` | 使用 `--document translation` 时 | 翻译文件，只适用于**单个**输入 | — |
| `--align-method` | 否，且只能与 `-t` 一起使用 | `position` 或 `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **三者选一**，使用 `--dry-run` 时除外 | 参见[调用](invocation.md#输出位置) | — |
| `--dry-run` | 否 | 开关：只计算和说明，不写入——参见[先预览，再写入](invocation.md#先预览再写入) | 关闭 |

**以短横线开头的模式**要写在 `--` 之后：`subedit-cli replace -- -- "—" film.srt`。

**默认处理主文本；使用 `--document translation -t FILE` 则处理翻译**。此时被更改并写出的是翻译，而且只有翻译——参见[一份翻译](invocation.md#一份翻译)。

### 纯文本或表达式

默认情况下，模式**按字面读取**：`a.b` 查找的是一个 `a`、一个点和一个 `b`。忽略大小写的精细程度与窗口相同——`É` 和 `é` 算同一个字母。

使用 `--regex` 时，模式是 **ICU** 正则表达式，读取方式与 Gaupol 相同：`.` 可以跨越换行，`^` 和 `$` 对字幕的每一行都有效。在替换文本中，并且仅在替换文本中，可以使用：

| 写法 | 含义 |
| :----- | :-------- |
| `$1` 至 `$9` | 匹配项的各个分组 |
| `$0` | 整个匹配项 |
| `\n` | 换行 |
| `\$`, `\\` | 一个 `$`、一个反斜杠 |

注意 shell 的引号：要写成 `'$1 EUR'`，而不是 `"$1 EUR"`，后者会让 shell 去解读 `$1`。

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nPrix: 12 euros, et 7 euros.\n\n' > prix.srt; subedit-cli replace --regex '([0-9]+) euros' '$1 EUR' --output euros.srt prix.srt; cat euros.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nPrix: 12 euros, et 7 euros.\n\n' > prix.srt; subedit-cli replace --regex '([0-9]+) euros' '$1 EUR' --output euros.srt prix.srt; cat euros.srt
prix.srt: replaced 2 matches -> euros.srt
1
00:00:01,000 --> 00:00:02,000
Prix: 12 EUR, et 7 EUR.
```

## 标签

标签把要查找的词切开的文本：

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace Bonjour Salut --output salut.srt balises.srt; cat salut.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace Bonjour Salut --output salut.srt balises.srt; cat salut.srt
balises.srt: replaced 2 matches -> salut.srt
1
00:00:01,000 --> 00:00:02,000
<i>Salut</i> tout le monde

2
00:00:03,000 --> 00:00:04,000
Salut <i>Marie</i>, ca va
```

在第一条字幕中，标签把“Bonjour”切开了：现在它包住的是“Salut”。在第二条中，标签没有被触及，留在原处。**敞开的标签仍保持敞开**：修复有问题的标记属于另一项操作。

替换文本按**文件格式的标签**来读取：SubRip 用 `<i>…</i>`，Advanced SSA 用 `{\i1}…`。这是为所写内容设置格式的唯一方式。

## 没找到，没有可更改的

有两种结果不算失败——退出码为 `0`，并且**文件照样会写出**，因为给出了输出位置就要写到输出位置：

| 情形 | 语句 |
| :-------- | :----- |
| 模式哪里都没有 | `"zzz" not found` |
| 模式存在，但每个匹配项都被替换成它自身 | `nothing to change` |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli replace zzz x --output b.srt a.srt; subedit-cli replace Bonjour Bonjour --output c.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli replace zzz x --output b.srt a.srt; subedit-cli replace Bonjour Bonjour --output c.srt a.srt
a.srt: "zzz" not found -> b.srt
a.srt: nothing to change -> c.srt
```

## `--dry-run`

标准输出会列出**每一条文本将会改变的字幕**，给出改变前后的内容，与文件中的样子一致，标签也包括在内：

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace --dry-run Bonjour Salut balises.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace --dry-run Bonjour Salut balises.srt
balises.srt: replaced 2 matches (dry run, nothing written)
balises.srt: subtitle 1
- <i>Bon</i>jour tout le monde
+ <i>Salut</i> tout le monde
balises.srt: subtitle 2
- Bonjour <i>Marie</i>, ca va
+ Salut <i>Marie</i>, ca va
```

列表的形式在[先预览，再写入](invocation.md#先预览再写入)中说明；在 JSON 中则是 `changes`。

## 输出

**标准输出**——无内容：结果就是写出的文件。使用 `--dry-run` 时，则是上面的更改列表。

| 级别 | 在标准错误输出上追加的内容 |
| :----- | :------------------------------------- |
| 1 | `<path>: replaced N matches -> <destination>`，或 `"<pattern>" not found`，或 `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept`——所读文件的格式、编码、标记和行尾符，原样保留 |
| 3 | `<path>: N bytes read, M written`，然后是**每一条读取诊断信息**——参见[调用](invocation.md#读取诊断信息) |

`N` 统计的是**文本发生了改变的字幕中被替换的匹配项**——窗口报告的也是这个数。

### 以 JSON 输出

使用 [`--format json`](invocation.md#脚本可读取的输出) 时，每个文件的对象包含：

| 键 | 内容 |
| :-- | :------ |
| `counts.replaced` | 在文本发生了改变的字幕中被替换的匹配项：即说明语句中的 `N` |
| `counts.matched` | 找到的匹配项，**无论替换后文本是否改变**——据此可以区分“没找到”（`0`）与“没有可更改的”（`replaced` 为 `0`，`matched` 为正数） |
| `changes` | 发生了更改的字幕：`{"subtitle", "document", "before", "after"}`，没有任何更改时为**空** |

## 退出码

与整个工具相同：所有文件都已处理（没有找到任何内容的文件也算已处理）则为 `0`；一个都没有处理则为 `2`；只处理了其中一部分则为 `3`；用法错误则为 `1`。

## 错误

| 触发情形 | 消息 |
| :------------------ | :------ |
| 模式为空 | `pattern: nothing to look for` |
| 表达式无法解读 | `pattern: not a regular expression (<ICU's reason>)` |
| 范围写法有误或超出文件 | 参见 [`adjust`](adjust.md#--range) |

**模式在读取第一个文件之前就会被解读**：无法解读的表达式属于用法错误（退出码 `1`），不读取也不写入任何内容。

输出位置的错误是所有会写入的子命令共有的：参见[调用](invocation.md#输出位置)。
