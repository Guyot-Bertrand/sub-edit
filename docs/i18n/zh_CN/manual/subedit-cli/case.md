# `case`

<!-- machine-translated -->
> 本页为机器翻译，尚未经过熟悉该语言的读者审校。

```
subedit-cli case --to title|sentence|upper|lower [--range N-M|N-]
                 (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 [--document main|translation] [-t FILE] [--align-method position|number]
                 <file>...
```

把文本改为**单词首字母大写、句首字母大写、全部大写或全部小写**，且不触动任何标签。它对应[窗口](../subedit-gui/operations.md)中的 `工具(O) ▸ 大小写(S)`，规则也相同。

| `--to` | 效果 | `bonjour marie` 变为 |
| :----- | :---- | :---------------------- |
| `title` | 每个单词的首字母大写，其余小写 | `Bonjour Marie` |
| `sentence` | 只有文本的第一个字母大写，其余小写 | `Bonjour marie` |
| `upper` | 全部大写 | `BONJOUR MARIE` |
| `lower` | 全部小写 | `bonjour marie` |

- **第一个字母或第一个数字之前的内容不受影响**：对话破折号、开引号或括号都会保留（`- bonjour` 变为 `- Bonjour`）。
- **标签不是文本**：标签不会改变大小写——`<I>` 仍是 `<I>`——也不会移动位置：把一个单词截断的标签，处理后仍然把它截断。
- **由 ICU 决定单词从哪里开始**：`l'été sera chaud` 变为 `L'été Sera Chaud`，重音符号得以保留（`éléonore` 变为 `Éléonore`）。

<!-- example: subedit-cli case --help -->
```console
$ subedit-cli case --help
Put the texts in title, sentence, upper or lower case, tags intact
Usage: subedit-cli case [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --to title|sentence|upper|lower
                              The case to put the texts in
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
| `--to` | **是** | `title`、`sentence`、`upper`、`lower` | — |
| `--range` | 否 | `N-M`，或 `N-` 直到结尾——参见 [`adjust`](adjust.md#--range) | 整个文件 |
| `--document` | 否 | `main` 或 `translation`——参见[一份翻译](invocation.md#一份翻译) | `main` |
| `-t`, `--translation-file` | 使用 `--document translation` 时必需 | 一个翻译文件，且只能有**一个**输入 | — |
| `--align-method` | 否，且只能与 `-t` 同用 | `position` 或 `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **三选一**，使用 `--dry-run` 时除外 | 参见[调用](invocation.md#输出位置) | — |
| `--dry-run` | 否 | 开关：只计算并说明，不写入任何内容——参见[先预览，再写入](invocation.md#先预览再写入) | 关闭 |

**默认处理主文本；使用 `--document translation -t FILE` 则处理翻译。** 此时更改并写出的是翻译，而且只有翻译——参见[一份翻译](invocation.md#一份翻译)。

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\n- BONJOUR MARIE\n\n' > a.srt; subedit-cli case --to title --output titre.srt a.srt; cat titre.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\n- BONJOUR MARIE\n\n' > a.srt; subedit-cli case --to title --output titre.srt a.srt; cat titre.srt
a.srt: 2 subtitles recased -> titre.srt
1
00:00:01,000 --> 00:00:02,000
<i>Bon</i>jour Marie

2
00:00:03,000 --> 00:00:04,000
- Bonjour Marie
```

如果文件里**没有任何文本发生更改**——所有内容本来就是所要求的大小写——文件会原样写出，并给出说明：

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBONJOUR\n\n' > a.srt; subedit-cli case --to upper --output b.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBONJOUR\n\n' > a.srt; subedit-cli case --to upper --output b.srt a.srt
a.srt: nothing to change -> b.srt
```

## 输出

**标准输出**——为空：结果就是写出的文件。使用 `--dry-run` 时，输出更改列表，参见[先预览，再写入](invocation.md#先预览再写入)中的说明。

| 级别 | 在错误输出中增加的内容 |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles recased -> <destination>`，或 `nothing to change` |
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
| 缺少 `--to` | `--to is required` |
| 值不是这四个之一 | `--to: … not in {title,sentence,upper,lower}` |
| 范围写法有误，或超出文件 | 参见 [`adjust`](adjust.md#--range) |

输出位置的错误是所有会写入文件的子命令共有的：参见[调用](invocation.md#输出位置)。
