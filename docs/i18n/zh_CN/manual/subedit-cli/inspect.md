# `inspect`

<!-- machine-translated -->
> 本页为机器翻译，尚未经过熟悉该语言的读者审校。

```
subedit-cli inspect [--frame-rate RATE] [--recursive] [--detail]
                   [-t FILE] [--align-method position|number] <file>...
```

报告每个文件由什么构成。**不会更改任何内容，也不写入任何文件。**

<!-- example: subedit-cli inspect --help -->
```console
$ subedit-cli inspect --help
Report what a subtitle file is made of
Usage: subedit-cli inspect [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to report on

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --detail                    List every anomaly, subtitle by subtitle, instead of counting them
  --frame-rate RATE           Frame rate of a file counted in frames: 25, 23.976
  -t,--translation-file FILE  Translation file to lay over the subtitle file, for a single input
  --align-method position|number
                              How the lines of the translation find their subtitles
```

## 参数

| 参数 | 必需 | 值 | 默认值 |
| :------- | :----- | :----- | :----- |
| `<file>...` | 是 | 一个或多个字幕文件的路径 | — |
| `--recursive`, `-r` | 否 | 开关 | 关闭——参见[处理目录树](lots.md) |
| `--detail` | 否 | 开关：逐条指出每个异常，而不是只计数——参见 [`anomalies` 报告的内容](#anomalies-报告的内容) | 关闭 |

没有给出路径属于用法错误，因此退出码为 `1`。

## 选项

| 选项 | 必需 | 值 | 默认值 |
| :----- | :------ | :----- | :----- |
| `--frame-rate` | 否 | 以每秒帧数表示的帧率：`25`、`23.976`、`29.97` | 23.976 |

**它只对以帧计数的文件有用**，也就是 MicroDVD，九种格式中只有它是这样。MicroDVD 文件除了帧号之外什么也不带；读取时采用的帧率决定了**报告中的每一个位置**，首先就是 `span`。在其他文件上给出它不会改变任何内容，也不属于错误。

无法表示任何帧率的值属于用法错误，因此退出码为 `1`：

<!-- example: subedit-cli inspect --frame-rate douze film.sub; echo $? -->
```console
$ subedit-cli inspect --frame-rate douze film.sub; echo $?
--frame-rate: "douze" is not a frame rate: expected frames per second, like 25 or 23.976
1
```

它与 [`convert`](convert.md) 上的选项是同一个选项，一字不差，因为它们指的是同一个帧率。`convert` 还用它来**写入**。

## 翻译：`-t`

| 选项 | 必需 | 值 | 默认值 |
| :----- | :------ | :----- | :----- |
| `-t`, `--translation-file` | 否 | 翻译文件，仅限**单个**输入 | — |
| `--align-method` | 否，且仅可与 `-t` 一起使用 | `position` 或 `number` | `position` |

使用 `-t` 时，翻译会**像窗口打开它时那样**叠加到文件上——参见[一份翻译](invocation.md#一份翻译)——报告还会说明**翻译的各行是如何找到各自字幕的**。这样，脚本在更正翻译之前就能知道翻译是否已对齐。

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour.\n\n' > film.fr.srt; subedit-cli --quiet inspect -t film.fr.srt film.srt | tail -3 -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour.\n\n' > film.fr.srt; subedit-cli --quiet inspect -t film.fr.srt film.srt | tail -3
  anomalies: none
  translation file: film.fr.srt, matched by position
  translation: 1 line attached; 1 subtitle left without a translation
```

新增的第一行给出文件名和对齐方法；第二行是窗口打开翻译时**所显示的那句话**，其中每个部分仅在不为零时才出现：

| 部分 | 统计的内容 |
| :----- | :---------------- |
| `N lines attached` | 找到了主文件中某条字幕的翻译行数 |
| `N subtitles born of a line` | 没有找到任何字幕、因而在各自位置上产生了自己的字幕的行数 |
| `N subtitles left without a translation` | 主文件中没有任何翻译行对应到的字幕数 |
| `N lines out of order` | 排在文件中更靠后的某一行之后才出现的行数 |

不带任何行的翻译会显示 `translation: the file holds no line`。**文件自身的报告仍然只是该文件本身的报告**：由某一行产生的字幕不会计入 `subtitles`。

`-t` 的错误就是读取时的错误，并附上翻译文件的路径，外加这一条：`<translation>: the file is already open as the main document`，当所给文件就是主文件时出现。不带 `-t` 使用 `--align-method` 属于用法错误，带有多个输入时使用 `-t` 同样如此——参见[一份翻译](invocation.md#一份翻译)。

## 输出

报告输出到**标准输出**——它是命令的结果，在任何级别都会写出，包括 `--quiet`。每个文件一个区块，前面是路径：

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,500\nLe canot dérive.\n\n2\n00:00:04,000 --> 00:00:06,200\nPersonne n a vu la côte.\n' > exemple.srt; subedit-cli --quiet inspect exemple.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,500\nLe canot dérive.\n\n2\n00:00:04,000 --> 00:00:06,200\nPersonne n a vu la côte.\n' > exemple.srt; subedit-cli --quiet inspect exemple.srt
exemple.srt
  format: SubRip
  encoding: UTF-8, detected
  byte order mark: absent
  line endings: LF
  subtitles: 2
  span: 00:00:01.000 -> 00:00:06.200
  frame rate grid: none (too few subtitles to tell)
  anomalies: none
```

| 字段 | 含义 |
| :---- | :----------- |
| `format` | `--to` 接受的格式之一，写出全称：`SubRip`、`WebVTT`、`SubViewer 2`、`Sub Station Alpha`、`Advanced SSA`、`MicroDVD`、`MPL2`、`TMPlayer`、`LRC` |
| `encoding` | 读取时采用的编码，其后注明**答案的来源**：文件声明了编码时为 `, from its byte order mark`，由 `--encoding` 强制指定时为 `, as asked for`，经过权衡得出时为 `, detected` |
| `byte order mark` | `present` 或 `absent` |
| `line endings` | `LF`、`CRLF` 或 `CR`；如果文件混用了几种，其后是 `, mixed from line N` |
| `subtitles` | 读取到的字幕数 |
| `span` | 从最早的开始时间到最晚的结束时间，格式为 `HH:MM:SS.mmm` |
| `frame rate grid` | 用来计算各位置的帧率，是**推断**出来的——见下文 |
| `frame rate` | 对以帧计数的文件，给出读取时采用的帧率及**其来源**——取代上一行，见下文 |
| `anomalies` | 同一行上的 `none`，或者出现的问题：**每种问题一行，附带其计数**，缩进在 `anomalies:` 之下；使用 `--detail` 时，**每个异常一行**，逐条字幕列出 |

**`span` 并不是“从第一条到最后一条”**，而是从最早到最晚：在顺序混乱的文件上，二者并不相同，只有后者才能如实说明文件覆盖的范围。

**使用 `--detail` 时，`anomalies` 从 1 开始为字幕编号**，与文件自身的编号一致——而不是文件的行号。字幕编号在更改之后依然有效，行号则不然。见下文。

## `frame rate grid` 报告的内容

**字幕文件不会声明自己的帧率**。SubRip 没有文件头，WebVTT 的文件头是自由文本。因此这一行什么也没有读取：它是**推断**出来的，方法是测量各个位置是否落在帧网格上。

测量的对象是**开始时间**。凡是存在网格的地方，开始时间无一例外都落在网格上，而结束时间却常常偏离——结束时间往往由阅读速度规则算出，而不是对齐到某一帧。

第一行总是说同样的内容：选定的帧率、结论，以及支持该结论的集中度。

| 结论 | 含义 |
| :------ | :----------------- |
| `clean` | 各位置都在网格上 |
| `partial` | 一部分在网格上，其余不在 |
| `none` | 没有任何候选项能解释这些位置 |

`none` 有**两种原因，它们给出的并不是同一个答案**：要么没有任何网格吻合——这时该行会给出最接近的候选项及其得分，以便看出相差多少——要么文件的字幕太少，无从判断。两个开始时间看起来总是完美对齐；这什么也证明不了。

**对于帧率不标准的规整文件，结果为 `none`，这是有意为之**。候选项的集合是封闭的——转换所提供的八种帧率——把其中最不离谱的一个报告出来，等于在“不知道”才是正确答案的地方给出了错误答案。

### 随后出现的行，在有内容可说时才会出现

| 行 | 出现的时机 |
| :---- | :------------------ |
| `grid offset` | 各位置在网格上，**只差一个常量**：文件被移动过，这里给出移动了多少 |
| `also fits` | 另一个候选项同样吻合，因为它是所选帧率的整数倍 |
| `too short a span to separate` | 文件的跨度不足以把所选帧率与所列出的其他帧率区分开 |
| `off the grid` | 有多少个开始时间偏离了网格，分布在多少个**连续段**中 |

**`also fits` 并不是犹豫不决**。每秒 25 帧的网格*包含于*每秒 50 帧的网格之中：按 25 定时的文件因此与两者都吻合。反过来则不成立——真正的 50 无法落到 25 上——所以选的是较低的那个，另一个则被列出，而不是闭口不提。涉及的组合只有 25 与 50、29.97 与 59.94、30 与 60。

**`too short a span to separate` 用来防止过于自信的答案**。23.976 与 24 每秒影片相差一毫秒：在一部长片上，二者可以毫无疑问地区分开，在十秒之内则无法区分。这一行会说明还剩下哪一个可能。

**`off the grid` 区分了单看百分比会混淆的两种情形**。许多只含一个开始时间的连续段，是被逐个手工修正过的位置。少数几个很长的连续段，则是被重新定时的一部分，或是由另外两个文件拼成的文件。连续段的数目就是区分它们的依据。

### 示例

十二行文本被放在每秒 25 帧的网格上——因此位于 40 毫秒的整数倍处——分布在将近九分钟之内：

<!-- example: k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > grille.srt; subedit-cli --quiet inspect grille.srt | tail -3 -->
```console
$ k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > grille.srt; subedit-cli --quiet inspect grille.srt | tail -3
  frame rate grid: 25 fps, clean (100.0%)
  also fits: 50 fps, of which this rate is a whole divisor
  anomalies: none
```

这个示例自己生成文件，而不是读取仓库中的某个文件，这是有意为之：它可以原样重放，而且这段循环比一句话更能说明什么是网格——位置都是帧时长的整数倍。

## `frame rate` 报告的内容，以及它为什么不是网格

**九种格式中只有一种以帧计数**：MicroDVD，它的每一行只带两个帧号，别无其他。对这样的文件，`inspect` 写出的不是 `frame rate grid` 而是 `frame rate`，这两行绝不会同时出现。

**原因一句话就能说清**。MicroDVD 文件的各个位置，是按某一帧率*根据*帧数算出的；从网格中把它们推断出来，得到的恰恰是刚刚提供的那个帧率。那只会是一个被包装成测量结果的给定数字。

这里写出的是**读取时采用的帧率及其来源**——与 `encoding` 一行的形式相同：

| 该行的内容 | 出现的时机 |
| :------------------ | :---- |
| `frame rate: 25 fps, as asked for` | 由 `--frame-rate` 指定 |
| `frame rate: 24000/1001 fps, assumed` | 没有人指定；这是默认值 |

**只有两种答案，而不是三种**，而 `encoding` 有三种：MicroDVD 文件什么也不声明。问题的全部就在于此——帧率必然来自外部，而报告会说明它来自两种外部来源中的哪一种。

**帧率决定了报告的其余全部内容。**`span` 要经过它，同一个文件按两种帧率读取就会得到两个跨度：

<!-- example: printf '{25}{75}Première.\n{100}{150}Deuxième.\n{200}{260}Troisième.\n' > images.sub; subedit-cli --quiet inspect images.sub | tail -3; subedit-cli --quiet inspect --frame-rate 25 images.sub | tail -3 -->
```console
$ printf '{25}{75}Première.\n{100}{150}Deuxième.\n{200}{260}Troisième.\n' > images.sub; subedit-cli --quiet inspect images.sub | tail -3; subedit-cli --quiet inspect --frame-rate 25 images.sub | tail -3
  span: 00:00:01.043 -> 00:00:10.844
  frame rate: 24000/1001 fps, assumed
  anomalies: none
  span: 00:00:01.000 -> 00:00:10.400
  frame rate: 25 fps, as asked for
  anomalies: none
```

## `anomalies` 报告的内容

文档中可能出现三种问题，每一种的修复方式各不相同。**默认情况下，`inspect` 只给它们计数**，每种一行，按下表的顺序排列：制作粗糙的文件会带有数百个问题，知道每种问题有多少个，才能决定该运行什么命令。**`--detail` 则逐个列出。**

| 写出的内容 | 使用 `--detail` 时 | 含义 |
| :--------------- | :-------------- | :------------------ |
| `subtitle ends before it starts: N` | `subtitle N ends before it starts` | 结束时间早于开始时间 |
| `subtitle starts before the previous one ends: N` | `subtitle N starts before the previous one ends` | 与前一条字幕重叠 |
| `subtitle starts before the previous one starts: N` | `subtitle N starts before the previous one starts` | 它破坏了文件的顺序——[`sort`](sort.md) 可以把它放回原位 |

不存在的种类不会写出。在左侧一列中，`N` 是字幕的数量；在下一列中，则是某条字幕的编号。

**同一条字幕可能出现两次**，这是有意为之：比前一条字幕开始得更早的那条字幕，同样也在前一条结束之前就开始了。第一点通过移动字幕来解决，第二点则通过更改持续时间来解决。

**被指出的是位置不对的那条字幕**，而不是它所对照的那一条。以 `0`、`4 s`、`2 s`、`3 s` 这几个开始时间为例：第三条破坏了顺序，第四条确实跟在第三条之后，因此不会被指出——对它无需做任何处理。

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect desordre.srt | tail -3 -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect desordre.srt | tail -3
  anomalies:
    subtitle starts before the previous one ends: 1
    subtitle starts before the previous one starts: 1
```

使用 `--detail` 时，则会指出具体的字幕：

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect --detail desordre.srt | tail -3 -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect --detail desordre.srt | tail -3
  anomalies:
    subtitle 3 starts before the previous one ends
    subtitle 3 starts before the previous one starts
```

**使用 `--detail` 时每个异常占一行**，因为上百条发现挤在一行里，既无法阅读也无法搜索。没有任何问题的文档仍然只有那一行 `anomalies: none`，脚本就是查找这一行。**`--format json` 的记录始终会列出每个异常**：脚本读取的是详细内容，`--detail` 只改变文本输出。

**它不是读取诊断信息**。诊断信息说明读取过程中遇到了什么，指向的是**文件的某一行**；异常说明文档*是什么样的*，指向的是**某条字幕**。顺序混乱的文件并不因此就是格式错误：读取过程对此没有什么可说的。

## 以 JSON 输出

使用 [`--format json`](invocation.md#脚本可读取的输出) 时，`inspect` 为**每个文件写出一个对象**，取代上面的区块；说明行仍输出到标准错误。各个键描述的是文本所说的内容，**任何一项都不应从一句话中反向读取**：

| 键 | 内容 |
| :-- | :------ |
| `format` | 格式的标识符，即 `convert --to` 接受的那个：`srt`、`vtt`…… |
| `encoding` | `{"charset", "origin", "byte_order_mark"}`——`origin` 为 `byte-order-mark`、`asked` 或 `detected` |
| `line_endings` | `{"kind", "mixed_from_line"}`——`lf`、`crlf` 或 `cr`；出现第二种行尾符的行号，或 `null` |
| `subtitles` | 字幕数 |
| `span_ms` | `{"start", "end"}`——最早的开始时间和最晚的结束时间，以毫秒表示 |
| `frame_rate` | 对以帧计数的文件：`{"rate", "origin"}`（`asked` 或 `assumed`）；其他情况为 `null` |
| `grid` | 对其他文件：推断出的网格；对以帧计数的文件为 `null` |
| `anomalies` | 由 `{"subtitle", "kind"}` 组成的数组——`end-before-start`、`overlapping-subtitles`、`out-of-order` |
| `translation` | **仅在使用 `-t` 时出现**：`{"file", "method", "attached", "born", "untranslated", "out_of_order"}`——`method` 为 `position` 或 `number`，四个计数就是上文那句话中的计数 |
| `warnings` | 读取过程中遇到的内容，在任何级别都会给出 |

`grid` 包含 `verdict`（`clean`、`partial` 或 `silent`）、`enough_starts`、`rate`（字符串；没有指出任何帧率时为 `null`）、`concentration_permille`（以千分比表示；开始时间不足时为 `null`）、`offset_ms`、`also_fits`（以本帧率为整数约数的那个帧率，或 `null`）、`not_separated`（因跨度过短而无法区分的帧率）、`strays`（偏离网格的开始时间）和 `starts`。

<!-- example: printf '{25}{50}Un.\n{75}{100}Deux.\n' > f.sub; subedit-cli --format json inspect --frame-rate 25 f.sub 2>/dev/null -->
```console
$ printf '{25}{50}Un.\n{75}{100}Deux.\n' > f.sub; subedit-cli --format json inspect --frame-rate 25 f.sub 2>/dev/null
{"schema":1,"command":"inspect","file":"f.sub","ok":true,"format":"microdvd","encoding":{"charset":"UTF-8","origin":"detected","byte_order_mark":false},"line_endings":{"kind":"lf","mixed_from_line":null},"subtitles":2,"span_ms":{"start":1000,"end":4000},"frame_rate":{"rate":"25","origin":"asked"},"grid":null,"anomalies":[],"warnings":[]}
```

## 说明行

在标准错误输出中，按所要求的级别：

| 级别 | 追加的内容 |
| :----- | :-------------- |
| 1 | `<path>: N subtitles` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings` |
| 3 | `<path>: N bytes read`、`<path>: N diagnostic(s) while reading`，随后是**其中的每一条**——参见[调用](invocation.md#读取诊断信息) |

## 退出码

与该工具的退出码相同：所有文件都已读取时为 `0`，一个都没有读取时为 `2`，只读取了一部分时为 `3`，用法错误时为 `1`。

## 错误

即读取时的错误——针对文件，给出 `-t` 时还针对翻译——以及 `-t` 的错误：见上文。`inspect` 不会写入任何可能被拒绝的内容。

| 触发条件 | 消息 |
| :------------------ | :------ |
| 没有给出路径 | `files is required`，其后是指向 `--help` 的提示 |
| 文件不存在 | `<path>: does not exist` |
| 文件无法读取 | `<path>: cannot be opened: permission denied` |
| 字节无法解码 | `<path>: cannot be decoded in the chosen encoding` |
| 格式无法识别 | `<path>: is in no format this tool knows` |
| 没有任何像字幕的内容 | `<path>: holds nothing recognisable as a subtitle` |

十七个子命令的消息都相同：打开文件的流程只在核心中写了一次。

## 失败的示例

空文件不符合任何格式。它不会被报告为空文件：而是被拒绝，并指明文件名。

<!-- example: : > vide.srt; subedit-cli inspect vide.srt; echo "code=$?" -->
```console
$ : > vide.srt; subedit-cli inspect vide.srt; echo "code=$?"
vide.srt: is in no format this tool knows
code=2
```
