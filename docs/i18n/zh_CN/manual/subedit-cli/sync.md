# `sync`

<!-- machine-translated -->
> 本页为机器翻译，尚未经过熟悉该语言的读者审校。

```
subedit-cli sync (--video FILE | --audio FILE)
                 [--audio-track N]
                 [--language CODE]
                 [--start TIME] [--end TIME]
                 [--model FILE]
                 [--vad-model FILE]
                 [--engine NAME]
                 [--keep-audio FILE]
                 [--method METHOD]
                 [--min-match PERCENT]
                 [--doubt-gap MS]
                 [--report FILE]
                 [-r]
                 [--output FILE | --output-dir DIR | --in-place] [--dry-run]
                 <files>...
```

**按影片的音频重新校准字幕文件的时间。** 它为这样的情形而生：在网上下载的 `.srt`，开头和结尾都有偏移，单靠一次平移无法纠正。有两种方法，`--method`（或设置 `alignment.method`）用来选择：

- **`speech`（默认）依据何时有人说话来对齐文件。** 它只向声音询问哪里有语音，并逐段影片找出使字幕与之吻合的偏移：恒定的偏移、漂移、偏晚的后半部分，以及按另一种帧率制作的文件（25 对 23.976 之类）都能得到纠正。**它与语言无关**——字幕可以是法语，影片可以是英语——，不需要转录，只需几秒钟。它需要 `ffmpeg`、随 whisper.cpp 提供的程序 `whisper-vad-speech-segments`，以及一个语音活动模型（`--vad-model`，设置 `transcription.vad-model`）。
- **`words` 把每条字幕的词与引擎听到的词相匹配。** 它用转录引擎聆听——与 [`transcribe`](transcribe.md) 使用的是同一个——每条字幕落在它的词被说出的位置。不做任何翻译：字幕必须与所说的语言一致。它能够在一个本来正确的文件中放置单独错位的字幕，所需时间与一次转录相当。

**`speech` 不做什么，每次运行时都会说明**，在标准错误输出和记录中：它**把文件作为一个整体**来纠正，绝不逐条字幕处理。一个本来正确的文件中单独错位的字幕，或在静音处显示的标题（章节卡、翻译屏幕上某物的说明），不会被查找，仍可能有误；当影片以音乐或片头开场时，最初几分钟也可能仍然不对。请检查结果，或在文本使用所说的语言时使用 `--method words`。

**所需的一切都不随程序提供**：程序、模型和 `ffmpeg` 就是[安装转录工具](install-transcription.md)中所说的那些，缺少它们不会破坏任何东西——命令会说明缺了什么，并指向该页，不写入任何内容，并以非零退出码退出。

- **只有开始时间和结束时间会改变。** 文本、样式、字幕数量及其顺序都不会变。
- **音频没有给出位置的字幕保留原来的时间**——没有人念的片尾字幕、一首歌、另一种语言的台词。**绝不凭空编造位置，也不根据相邻字幕去猜**：该行留在文件原来的位置，命令会说明这一点。这*不是*失败，退出码为 `0`。
- **使用 `words` 时，一条字幕至少有一半的词被听到，就算找到了**（`--min-match`、`alignment.min-match`），并且至少一个词。大小写、重音、标点和标记都无关紧要；与长词相差一个字母的词也算听到。`speech` 会放置每一条字幕，并且不使用 `--language`、`--model`、`--engine` 或 `--min-match` 中的任何一个。
- **结束时间是最后一个词结束的地方。** 引擎对词的计时是近似的，比字幕本身的结束早几百分之一秒，因此字幕可能比原来略短。
- **会撞上下一条的字幕，被截短到下一条开始的地方。** 截短也无法解决、仍与下一条重叠的字幕，则保持原样并加以说明。
- **与声音不吻合的文件会被拒绝，不会写入**：使用 `words` 时，是一条字幕都没有找到，这几乎总是音频或语言不对——`speech` 与语言无关；使用 `speech` 时，是字幕与语音的共同之处太少，这几乎总是影片不对。
- **可以只聆听影片的一部分**，使用 `--start` 和 `--end`，与 [`transcribe`](transcribe.md#转录影片的一部分)一样：只有**开始于**该范围内的字幕才会被查找和改动，其余的保持原样，也不计入。
- **给出的每个文件，音频都会听一遍。** 要在同一部影片上重新校准多个文件，每个文件都要等待一次。

<!-- example: subedit-cli sync --help -->
```console
$ subedit-cli sync --help
Re-time the subtitles of a file on the audio of a film
Usage: subedit-cli sync [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to re-time

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --video FILE                Film to listen to
  --audio FILE                Audio file to listen to, instead of a film
  --audio-track N             Audio track of a film, counted from 0; the first by default
  --start TIME                Where to begin, as 2:00, 1:02:03 or seconds; the beginning by default
  --end TIME                  Where to stop, as 2:00, 1:02:03 or seconds; the end by default
  --language CODE             Language of the speech, two letters: fr, en, de; detected from the first 30 seconds by default
  --model FILE                Model file; the transcription.model setting by default
  --vad-model FILE            Voice-activity model that skips silences; the transcription.vad-model setting by default
  --engine NAME               Engine that listens; the transcription.engine setting by default
  --keep-audio FILE           Keep the extracted audio in this file
  --method METHOD             How the subtitles are put in line: speech (from when there is speech, whatever the language) or words (from the words heard); the alignment.method setting by default
  --min-match PERCENT         Share of a subtitle's words that must be heard, in percent; the alignment.min-match setting by default
  --doubt-gap MS              How far a subtitle may move from its neighbours' shift before it is flagged, in milliseconds; the alignment.doubt-gap setting by default
  --report FILE               Write the subtitles to check to this file
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## 参数与选项

| 选项 | 必需 | 取值 | 默认值 |
| :----- | :------- | :----- | :------ |
| `<files>` | 是 | 要重新校准的字幕文件；使用 `-r` 时为目录 | — |
| `--recursive`, `-r` | 否 | 开关 | 关闭——参见[处理目录树](lots.md) |
| `--video` | `--video` 和 `--audio` 二选一 | 要聆听的影片 | — |
| `--audio` | `--video` 和 `--audio` 二选一 | 要聆听的音频文件，用于代替影片 | — |
| `--audio-track` | 否 | 从 0 开始的整数：在影片各音轨中的位置 | `0`，即第一条 |
| `--language` | 否 | 两个字母，ISO 639-1 代码：`en`、`fr` | 设置 `transcription.language`；为空：自动检测 |
| `--start`, `--end` | 否 | 时间：`2:00`、`1:02:03`、`2:00.5`，或秒数；`--end` 必须在 `--start` 之后 | 整段音频 |
| `--model` | 否 | 模型文件 | 设置 `transcription.model` |
| `--vad-model` | 否 | 一个语音活动模型，让引擎跳过静音；有了它，词会更贴近语音 | 设置 `transcription.vad-model` |
| `--engine` | 否 | 负责聆听的引擎的名称：`whisper.cpp` | 设置 `transcription.engine` |
| `--keep-audio` | 否 | 用来保存所提取音频的文件；其目录不存在时会创建 | 音频不保存在任何地方 |
| `--method` | 否 | `speech`：依据何时有语音来对齐文件，任何语言均可；`words`：把词与听到的内容相匹配，须与所说的语言一致 | 设置 `alignment.method`，`speech` |
| `--min-match` | 否 | 1 到 100 之间的整数：一条字幕中必须被听到的词所占的比例，以百分比表示 | 设置 `alignment.min-match`，`50` |
| `--doubt-gap` | 否 | 0 或更大的毫秒整数：一条字幕的位移与相邻字幕的位移相差多远才会被标记 | 设置 `alignment.doubt-gap`，`2000` |
| `--report` | 否 | 用来写入需检查字幕的文件，每行一条：字幕的编号、一个制表符和原因；一次只能处理一个字幕文件 | 不写入任何内容 |
| `--output` / `--output-dir` / `--in-place` | **三者选一**，使用 `--dry-run` 时除外 | 参见[调用](invocation.md#输出位置) | — |
| `--dry-run` | 否 | 开关：只计算和说明，不写入——参见[先预览，再写入](invocation.md#先预览再写入) | 关闭 |

输出位置——`--output`、`--output-dir`、`--in-place`、`--dry-run`——以及 `-r`，与其他改写文件的命令相同：参见[输出位置](invocation.md#输出位置)。文件保持其格式、编码和换行符。

**设置**从设置文件中读取——参见[设置文件](invocation.md#设置文件)——并且每项设置在这里都有对应的选项。引擎、模型和语音活动模型与 `transcribe` 的相同。

| 键 | 含义 | 默认值 |
| :-- | :------ | :------ |
| `alignment.engine` | `words` 方法的对齐引擎；只有一个，`transcript-matching`，它负责聆听并匹配 | `transcript-matching` |
| `alignment.method` | 字幕对齐的方式：`speech` 或 `words` | `speech` |
| `alignment.min-match` | 一条字幕要被认为找到，其词中必须被听到的比例，以百分比表示 | `50` |
| `alignment.doubt-gap` | 一条字幕的位移与相邻字幕的位移最多可相差多少毫秒，超过则被标记 | `2000` |

## 被标记的内容

命令会说明哪些字幕需要检查，以及原因。一条字幕按以下情形中**第一个**成立的被标记：

| 原因 | 含义 |
| :-- | :------ |
| not found in the audio, left as it was | 被听到的词少于 `--min-match`：它保留原来的时间 |
| only partly heard | 已找到，但被听到的词不足 80 % |
| moved differently from its neighbours | 已找到，但它的位移与其前后各两条字幕的位移中位数相差超过 `--doubt-gap` |

使用 `speech` 时每条字幕都会被放置，以下情形中只有最后一个可能适用：当文件的偏移从一条字幕到下一条变化超过 `--doubt-gap`（偏晚的后半部分）时，台阶两侧的字幕都会被标记。

一条字幕也可能被**截短到下一条字幕的开始处**，或**仍与下一条字幕重叠**。

有两个数字讲述整个文件的情况。**位移中位数**是字幕移动幅度的中间值：单纯晚了一点的文件只在这里体现，别处看不到。**漂移**是最后十分之一字幕的位移中位数减去最前十分之一的：对恒定的偏移，它接近零；对沿着影片越来越晚的文件，它很大。

## 输出

**标准输出**——无内容：结果就是写出的文件。

| 级别 | 在标准错误输出上说明的内容 |
| :---- | :------------------------------- |
| 1 | `<file>: N of M subtitles placed on the audio, K to check; median shift X s, drift Y s -> <destination>` ——以及在纠正了帧率时，说明比例；使用 `speech` 时，还有**它不做什么的警告**，单独占一行 |
| 2 | 步骤和进度，与 [`transcribe`](transcribe.md#输出)相同 |

### 以 JSON 输出

使用 [`--format json`](invocation.md#脚本可读取的输出)时，文件的对象带有 `counts`：`subtitles`（被查找的）、`placed`（得到位置的）、`moved`（时间有变化的）、`partial`、`out_of_line`、`not_found`、`median_shift_ms` 和 `drift_ms`；以及 `flagged`，一个 `{"subtitle": N, "kind": "…"}` 的列表，其中数字是字幕在文件中的位置，`kind` 是 `not-found`、`partial`、`out-of-line`、`cut-back` 和 `overlaps` 之一。使用 `speech` 时，`warnings` 带有一个 `kind` 为 `speech-only` 的条目。标识符是有保证的，句子则没有。

## 示例

```bash
subedit-cli sync --video film.mkv --vad-model silero.bin \
    --report to-check.txt --output film.synced.srt film.srt
```

要依据词来放置每条字幕（文本使用所说的语言）：

```bash
subedit-cli sync --method words --video film.mkv --model small.bin --vad-model silero.bin \
    --output film.synced.srt film.srt
```

## 退出码

与整个工具相同：文件已写出则为 `0`，无论有没有被标记的字幕；无法写出则为 `2`——缺少工具或模型、提取或聆听失败、文件与声音不吻合（一条字幕都没找到，或与语音的共同之处太少）——；用法错误则为 `1`，例如既没有 `--video` 也没有 `--audio`（或两者都有）、选项超出范围，或无法使用的范围。

## 错误

| 原因 | 消息 |
| :---- | :------ |
| 既没有 `--video` 也没有 `--audio`，或两者都有 | `sync needs the audio to listen to: give --video or --audio, and not both` |
| 找不到引擎程序，或配置的程序不存在 | `the transcription engine was not found`，然后是指向指南的提示 |
| 未指定模型，或模型不存在 | `no model was given, or the model file is not there`，然后是指向指南的提示 |
| 影片找不到 `ffmpeg` | `ffmpeg was not found, so the audio of a video cannot be extracted`，然后是提示 |
| 影片没有的音轨 | `the video has no audio track N`，然后是它有的音轨 |
| 模型无法转录的语言 | `the model cannot transcribe the language xx` |
| 没有人注册过的引擎 | `no transcription engine is called NAME`，或 `there is no alignment engine called NAME` |
| 一条字幕都没找到（`words`） | `none of the N subtitles was found in the audio: is it the right audio, and the right language?`，然后是 `The method based on speech does not depend on the language.` |
| 未指定语音活动模型，或找不到查找语音的程序（`speech`） | `Finding where there is speech needs a voice-activity model: name one in the transcription.vad-model setting.` 或 `Finding where there is speech needs the program whisper-vad-speech-segments, which comes with whisper.cpp, and it was not found.`，然后是指向指南的提示 |
| 字幕与语音的共同之处太少（`speech`） | `The sound and the subtitles have too little in common to be put in line: is it the right film?` |
| `--min-match` 超出 1 到 100，`--doubt-gap` 为负 | 该选项的用法错误 |
| `--report` 与多个字幕文件一起使用 | `--report names one file: give one subtitle file when you use it` |
