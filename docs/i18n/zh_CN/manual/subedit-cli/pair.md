# `pair`

<!-- machine-translated -->
> 本页为机器翻译，尚未经过熟悉该语言的读者审校。

```
subedit-cli pair (--output FILE | --output-dir DIR | --in-place)
                 -t FILE [--align-method position|number]
                 [--dry-run]
                 <main>
```

写出**按主文件的位置重新定时的翻译**，并说明翻译的各行是如何找到各自所属字幕的。它相当于窗口中的 `Open Translation…`——[同样的配对](../subedit-gui/files.md#打开翻译)，同样的说明语句——再加上写入结果，窗口里这一步由 `Save Translation` 完成。

**这是配对独有的功能，单独处理一个文件时没有**。其他接受 `-t` 的子命令会修改它们读取的翻译——[`case`](case.md)、[`replace`](replace.md)……；`pair` 不改动文本的任何内容：它把翻译**铺**到主文件上，并写出铺设的结果。主文件只用来接收这些行，**从不写入**。

- **写出的是翻译**，写到**它自己的路径，采用它自己的格式**：即翻译文件的格式、编码和行尾符。其中的字幕**位于主文件的位置**：偏离标记的行会取回它所翻译的那条字幕的起止时间。
- **`-t` 是必需的**。没有翻译的 `pair` 无物可铺：这是用法错误，退出码为 `1`。
- **只有一个主文件**，也没有 `--recursive`：一对就是两个文件，而目录树无法表明哪份翻译对应哪个主文件。批量处理时，每一对调用一次。
- **`--output` 指定写出的翻译文件，`--output-dir` 把它的基本名放到该目录中，`--in-place` 则改写它**，与任何接受 `-t` 的子命令相同——参见[一份翻译](invocation.md#一份翻译)。
- **不存在 `--document`**：被针对的文档只有一个，就是翻译。

<!-- example: subedit-cli pair --help -->
```console
$ subedit-cli pair --help
Write a translation at the positions of its main file, and say how it lined up
Usage: subedit-cli pair [OPTIONS] main

Positionals:
  main TEXT REQUIRED          The main subtitle file

Options:
  -h,--help                   Print this help message and exit
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
| `<main>` | 是 | **唯一的**路径 | — |
| `-t`, `--translation-file` | **是** | 翻译文件 | — |
| `--align-method` | 否 | `position` 或 `number`——参见[一份翻译](invocation.md#一份翻译) | `position` |
| `--output` / `--output-dir` / `--in-place` | **三者选一**，使用 `--dry-run` 时除外 | 参见[调用](invocation.md#输出位置) | — |
| `--dry-run` | 否 | 开关：只计算和说明，不写入——参见[先预览，再写入](invocation.md#先预览再写入) | 关闭 |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,300 --> 00:00:02,300\nBonjour.\n\n2\n00:00:03,300 --> 00:00:04,300\nAu revoir.\n\n' > film.fr.srt; subedit-cli pair film.srt -t film.fr.srt --output recale.fr.srt; cat recale.fr.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,300 --> 00:00:02,300\nBonjour.\n\n2\n00:00:03,300 --> 00:00:04,300\nAu revoir.\n\n' > film.fr.srt; subedit-cli pair film.srt -t film.fr.srt --output recale.fr.srt; cat recale.fr.srt
film.fr.srt: translation: 2 lines attached -> recale.fr.srt
1
00:00:01,000 --> 00:00:02,000
Bonjour.

2
00:00:03,000 --> 00:00:04,000
Au revoir.
```

这份翻译晚了 0.3 s：每一行都找到了与其中点重叠的字幕，写出的文件带有主文件的位置。

## 对齐说明的内容

这句说明语句与窗口打开翻译时说的**是同一句**，其中每一部分仅在不为零时才出现——各部分的列表参见 [`inspect -t`](inspect.md#翻译-t)。**它从级别 1 起就会写出**，而不像其他子命令那样在级别 2 才写出：在这里，对齐不是处理过程中的细节，而是处理的结果。

**对齐对写出的文件有什么影响**：

- **没有人翻译的字幕写成不含文本的块**，读取时不会产生任何异常；
- **由某一行产生的字幕**——即没有任何字幕接收的行——写成**它自己的位置**，与所用方法无关：因此写出的文件包含的字幕可能比主文件更多；
- **文档中原有的翻译内容在铺设之前会被清除**，这一点不适用于命令行：命令行只单独读取主文件。

**两种方法**在各行都落在各自字幕上时得到相同的文件，而在缺少某一行或翻译整体错位时则不同——这就是方法需要选择的原因，也是 position 作为默认值的原因。`src/test/data/paires/` 中的八种情形列出了这些差别，并给出每种方法所预期的文件。

## 输出

**标准输出**——无内容：结果就是写出的文件。

| 级别 | 在标准错误输出上追加的内容 |
| :----- | :------------------------------------- |
| 1 | `<translation>: translation: N lines attached; … -> <destination>`——**路径是翻译的路径**，即写出的文件；使用 `--dry-run` 时，以 `(dry run, nothing written)` 取代 `-> <destination>` |
| 2 | `<translation>: SubRip, UTF-8, no BOM, LF line endings kept`——**翻译的**格式、编码、标记和行尾符，原样保留 |
| 3 | `<translation>: N bytes read, M written`，然后是主文件和翻译的**每一条读取诊断信息**——参见[调用](invocation.md#读取诊断信息) |

### 以 JSON 输出

使用 [`--format json`](invocation.md#脚本可读取的输出) 时输出**单个对象**，其中 `file` 是翻译，`destination` 是写出的文件。`counts.subtitles` 统计的是**写出文件中的字幕**，由某一行产生的字幕也计算在内。`alignment` 键——`{"file", "method", "attached", "born", "untranslated", "out_of_order"}`——所含的计数与说明语句相同，就像 [`inspect`](inspect.md#以-json-输出) 的 `translation` 键一样。

## 退出码

与整个工具相同：翻译已写入（使用 `--dry-run` 时为已计算）则为 `0`；两个文件之一无法读取或输出位置无法写入则为 `2`；用法错误则为 `1`。**对齐不佳的翻译不算失败**：它会被写出，说明语句会指出哪些内容没有找到位置。

## 错误

| 触发情形 | 消息 |
| :------------------- | :------ |
| 没有 `-t` | `pair needs the translation file to lay over the main one: use -t` |
| `--document` | `The following argument was not expected: --document`——`pair` 中不存在该选项 |
| 主文件多于一个 | `The following argument was not expected: <path>` |
| 翻译就是主文件 | `<translation>: the file is already open as the main document`，退出码 `2` |
| 其中一个文件缺失、不可读或格式未知 | 与其他地方相同，并附上出错文件的路径 |
