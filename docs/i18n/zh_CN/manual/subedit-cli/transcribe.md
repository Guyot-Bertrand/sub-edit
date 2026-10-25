# `transcribe`

<!-- machine-translated -->
> 本页为机器翻译，尚未经过熟悉该语言的读者审校。

```
subedit-cli transcribe --output FILE
                       [--to FORMAT]
                       [--audio-track N]
                       [--language CODE]
                       [--start TIME] [--end TIME]
                       [--suppress-notes | --keep-notes]
                       [--model FILE]
                       [--engine NAME]
                       [--keep-audio FILE]
                       [--raw]
                       <input>

subedit-cli transcribe --list-languages [--model FILE] [--engine NAME]
```

把**视频或音频文件中的语音写成字幕文件**。音频经过准备后交给引擎，由引擎找出说了什么以及何时说的，引擎切出的片段再被合并成符合时长和长度要求的字幕。

**所需的一切都不随程序提供**：引擎（whisper.cpp）、模型和 `ffmpeg` 由用户自行安装，并由用户说明模型所在的位置。[安装转录
工具](install-transcription.md)说明在哪里可以找到每一样，以及该选哪个模型。**缺少它们不会破坏任何东西**：命令会说明缺了什么，并指向该页，不写入任何内容，并以非零退出码退出。

- **输入是视频或音频文件。** 对于视频，`ffmpeg` 会提取一条音轨；没有 `ffmpeg` 时，引擎可直接读取的音频文件（`wav`、`mp3`、`flac`、`ogg`）仍可转录，但视频不行，指定音轨时任何文件都不行。
- **一次只处理一个文件**，输出位置是 `--output`：没有 `--output-dir` 或 `--in-place`，因为结果是新文件，而不是改写。
- **语言会被检测**，除非用 `--language` 强制指定，并且会说明所用的语言。只为英语制作的模型不检测任何语言，也不转录其他语言：它无法接受的语言会在任何工作开始*之前*被拒绝，并指出该模型的限制。
- **语言用两个字母输入——`fr`、`en`、`de`——也可以用文件所声明的三字母代码，`fra`、`eng`，它会被读作对应的两字母代码。** 其他写法都会在任何工作开始之前被拒绝。**检测只听前三十秒**：以音乐或静音开头的影片可能被当成另一种语言，引擎随后会把语音*翻译*成该语言写出——被当成英语的法语影片，输出的是英语，或是 `[SPEAKING FRENCH]`。知道语言时，请直接给出。
- **可以只转录影片的一部分**，使用 `--start` 和 `--end`——参见[转录影片的一部分](#转录影片的一部分)。这样可以先在两分钟上试用模型，再放到两小时上运行。
- **可以列出引擎提供的语言**，使用 `--list-languages`——参见[列出语言](#列出语言)。
- **默认情况下声音不会被写成注释**：引擎被告知不要这样做（`--suppress-notes`；它自己的选项是 `--suppress-nst`），因此 `(Sonnerie de téléphone)` 或 `[MUSIC PLAYING]` 不会出现在字幕中，无论是圆括号还是方括号。`--keep-notes` 会写出它们，用于听障人士字幕。注释被禁止时，引擎会为没有语音的段落写出 `...`，而**后处理会丢弃既没有字母也没有数字的片段**——`...`、`…`、`♪`——然后才合并和切分；`--raw` 按引擎片段到来的原样保留它们。
- **片段会被合并和切分**，依据是各项设置的限制（`transcription.merge-gap-ms`、`transcription.max-duration-ms`、`transcription.max-characters`）：同一句话中相距很近的两个片段合为一条字幕，过长或过宽的片段则被切开——先在句末切，再在逗号、分号或冒号处切，只有在没有可切的标点时才在句子中间切——，并避开会留下小到无法阅读的字幕的切分。这三步按顺序进行，每一步都处理整个转录：丢弃、合并、切分。开启某个片段的字幕保留该片段的起点，结束该片段的字幕保留它的终点。**`--raw` 按引擎给出的原样写入片段**，每条字幕一个片段，这也是查看合并起了什么作用的办法。

<!-- example: subedit-cli transcribe --help -->
```console
$ subedit-cli transcribe --help
Write the speech of a video or an audio file as subtitles
Usage: subedit-cli transcribe [OPTIONS] [input]

Positionals:
  input TEXT                  Video or audio file to transcribe

Options:
  -h,--help                   Print this help message and exit
  --output FILE               Subtitle file to write
  --to TEXT:{srt,vtt,subviewer2,ssa,ass,microdvd,mpl2,tmplayer,lrc}
                              Format to write; the one the extension of --output says by default
  --audio-track N             Audio track of a video, counted from 0; the first by default
  --start TIME                Where to begin, as 2:00, 1:02:03 or seconds; the beginning by default
  --end TIME                  Where to stop, as 2:00, 1:02:03 or seconds; the end by default
  --language CODE             Language of the speech, two letters: fr, en, de; detected from the first 30 seconds by default
  --suppress-notes            Do not write sounds as notes, such as (phone ringing); the transcription.suppress-notes setting by default
  --keep-notes                Write sounds as notes, against the setting
  --model FILE                Model file; the transcription.model setting by default
  --engine NAME               Engine; the transcription.engine setting by default
  --keep-audio FILE           Keep the extracted audio in this file
  --list-languages            List the languages the engine offers for the model, and transcribe nothing
  --raw                       Write the segments as the engine cut them
```

## 参数与选项

| 选项 | 必需 | 取值 | 默认值 |
| :----- | :----- | :------ | :----- |
| `<input>` | 是，使用 `--list-languages` 时除外 | 视频或音频文件 | — |
| `--output` | 是，使用 `--list-languages` 时除外 | 要写入的字幕文件；其目录不存在时会创建 | — |
| `--to` | 否 | 格式的简称：`srt`、`vtt`、`subviewer2`、`ssa`、`ass`、`microdvd`、`mpl2`、`tmplayer`、`lrc` | `--output` 的扩展名所指的格式，否则为 `srt` |
| `--audio-track` | 否 | 从 0 开始的整数：在视频各音轨中的位置 | `0`，即第一条 |
| `--language` | 否 | 两个字母，ISO 639-1 代码：`en`、`fr`；像 `fra` 这样的三字母代码会被读作 `fr` | 设置 `transcription.language`；为空：自动检测 |
| `--suppress-notes` | 否 | 标志：不把声音写成注释；对无法被如此告知的引擎会被拒绝 | 设置 `transcription.suppress-notes` |
| `--keep-notes` | 否 | 标志：写出它们，与设置相反；与 `--suppress-notes` 相对，两者同时给出会被拒绝 | 设置 `transcription.suppress-notes` |
| `--list-languages` | 否 | 标志：列出引擎为该模型提供的语言，不转录任何内容 | 关闭 |
| `--start` | 否 | 时间：`2:00`、`1:02:03`、`2:00.5`，或秒数，`90` | 音频的开头 |
| `--end` | 否 | 时间，写法与 `--start` 相同；必须在 `--start` 之后 | 音频的结尾 |
| `--model` | 否 | 模型文件 | 设置 `transcription.model` |
| `--engine` | 否 | 引擎的名称：`whisper.cpp` | 设置 `transcription.engine` |
| `--keep-audio` | 否 | 保存提取出的音频的文件，格式为 WAV 文件中的单声道 16 位 PCM，采样率为引擎所要求的（whisper.cpp 为 16 kHz）；其目录不存在时会创建，文件名可以包含 `{rate}`，它会被替换为以赫兹为单位的该采样率（`film.{rate}.wav` 会写成 `film.16000.wav`） | 音频不保存在任何地方 |
| `--raw` | 否 | 开关：写入引擎的片段，不合并也不切分 | 关闭 |

**设置**与 [`adjust`](adjust.md) 的设置一样从设置文件读取——参见[设置文件](invocation.md#设置文件)——并且每一项在这里都有对应的选项：

| 键 | 含义 | 默认值 |
| :-- | :------ | :------ |
| `transcription.engine` | 引擎 | `whisper.cpp` |
| `transcription.executable` | 引擎的程序，绝对路径；为空：在 `PATH` 中查找 | 空 |
| `transcription.model` | 模型文件，绝对路径 | 空 |
| `transcription.language` | 音频的语言，两个字母；为空：自动检测 | 空 |
| `transcription.suppress-notes` | 不把声音写成注释 | `true` |
| `transcription.merge-gap-ms` | 同一句话中间隔小于这么多毫秒的片段会被合并 | `400` |
| `transcription.max-duration-ms` | 一条字幕在屏幕上停留的最长时间，单位为毫秒 | `7000` |
| `transcription.max-characters` | 一条字幕所含的最多字符数，不计换行 | `84` |

## 选择音频

一个视频可以带有多条音轨——原声、评论、另一种语言。`--audio-track` 取的是文件**各音轨中的位置**，从 0 开始，按文件列出的顺序。视频没有的音轨会被拒绝，**并列出它实际有的音轨**，以便下一次调用是正确的：

```bash
subedit-cli transcribe --audio-track 4 --output film.srt --model small.bin film.mkv
```

```text
film.mkv: the video has no audio track 4
audio track 0: fra, Original
audio track 1: eng, Commentary
```

## 转录影片的一部分

`--start` 和 `--end` 把工作限制在两者之间的那部分影片。可以只给其中一个：单独的 `--start` 一直到结尾，单独的 `--end` 从开头开始。

```bash
subedit-cli transcribe --start 2:00 --end 3:30 --output test.srt --model large-v3.bin film.mkv
```

- **时间写成 `M:SS` 或 `H:MM:SS`，可以在小数点后带小数部分（`2:00.5`），也可以写成秒数（`90`、`90.5`）**。逗号会被拒绝，工具中处处如此。
- **字幕保留影片的时间**：影片 2:10 处说的那句话写在 `00:02:10`，而不是 `00:00:10`。该文件可以直接叠加到整部影片上。
- **边缘是近似的**：引擎以三十秒为窗口工作，因此它返回的内容事后会按范围裁切；跨越边缘的句子会在那里被切开。
- **范围无法使用时什么都不会开始**：不是时间的边界，或不在 `--start` 之后的 `--end`，属于用法错误，会指出问题，不写入任何内容，也不运行 `ffmpeg` 或引擎。

整部影片的音频仍然会被提取：范围节省的是耗时长得多的转录。

## 列出语言

`--list-languages` 写出引擎**为该模型**提供的语言，每行一种：`--language` 所接受的代码、一个制表符，以及该语言用界面语言写的名称。它既不需要输入也不需要输出，也不转录任何内容。

```bash
subedit-cli transcribe --list-languages --model ggml-small.bin
```

```text
en	English
zh	Chinese
de	German
…
```

列表取决于引擎和模型：多语言的 whisper.cpp 模型约有一百种语言，仅限英语的模型则只有 `en`。无法列出自己语言的引擎不列出任何语言。未给出或不存在的模型是一个错误，并附有指向指南的提示；只有文本形式，使用 [`--format json`](invocation.md#脚本可读取的输出)时不写出任何内容。

## 输出

**标准输出**——无内容：结果就是写出的文件。

| 级别 | 在标准错误输出上追加的内容 |
| :----- | :------------------------------------- |
| 1 | `<input>: N subtitles transcribed, language: xx -> <output>` |
| 2 | 步骤和进度，每个百分点一行：`extracting the audio…`，然后是 `transcribing…`，再然后是 `transcribing: 25%, 0:12:30 of 0:50:00, about 38 min left, elapsed: 0:05:12`——百分比、在音频中到达的位置、剩余时间和已用时间；**没有新内容时，这一行每十秒重复一次**，以免长时间的等待被当成卡死 |
| 3 | 以及引擎每得到一段新文本，`last text: …` |

**百分比是引擎在音频中的位置，而不是它自己窗口的计数**：引擎边转录边写出已转录的内容，命令读取它进行到哪里，因此数字从第一分钟起就在变化。（引擎自己的进度要在完成一个三十秒窗口之后，以音频的 5 % 为一步到来：单独看，在处理器上用大模型处理长影片时，它可能在 0 % 停留一刻钟。）**剩余时间一开始不显示**：做完足够多的工作、可以外推时才给出，并且依据最后一分钟的速度，而不是整个运行过程的平均值。

### 以 JSON 输出

使用 [`--format json`](invocation.md#脚本可读取的输出)（写在子命令之前的全局选项）时，文件的对象在 `language` 中包含所用的语言，并包含 `counts.subtitles`（写出的字幕）和 `counts.segments`（生成这些字幕所用的引擎片段）；当 `--keep-audio` 保存了音频时，还包含 `audio`。

## 提示

有一条提示会在工作之前于第一级输出，因为它会改变您应有的预期：

- **使用只支持英语的模型时**：`This model is for English only: whatever language the film
  speaks, the result will be English, or notes in brackets. Choose a multilingual model in the preferences.`
  文件名以 `.en` 结尾的模型就是这样的模型。

## 退出码

与整个工具相同：文件已写出则为 `0`；无法写出则为 `2`——缺少工具或模型、模型无法接受的语言、提取或转录失败——；用法错误则为 `1`，例如没有人注册过的引擎或无法使用的范围。

## 错误

| 原因 | 消息 |
| :---- | :------ |
| 找不到引擎程序，或配置的程序不存在 | `the transcription engine was not found`，然后是指向指南的提示 |
| 未指定模型，或模型不存在 | `no model was given, or the model file is not there`，然后是指向指南的提示 |
| 视频找不到 `ffmpeg` | `ffmpeg was not found, so the audio of a video cannot be extracted`，然后是提示 |
| 视频没有的音轨 | `the video has no audio track N`，然后是它有的音轨 |
| 模型无法转录的语言 | `the model cannot transcribe the language xx` |
| 没有语言，且模型无法检测语言 | `the model cannot detect the language: give one` |
| 没有人注册过的引擎 | `no transcription engine is called NAME` |
| 缺少 `<input>` 或 `--output` | `input is required` 或 `--output is required` |
| `--suppress-notes` 与 `--keep-notes` 同时给出 | `--suppress-notes and --keep-notes say opposite things; give one of them` |
| 对无法被如此告知的引擎使用 `--suppress-notes` | `the engine NAME cannot be told not to write sounds as notes` |
| 不是代码的语言 | `"TEXT" is not a language code: write two letters, such as fr, en, de or es` |
| 范围的某个边界不是时间 | `"TEXT" is not a time: write it as 2:00, 1:02:03 or 2:00.5, or in seconds` |
| `--end` 不在 `--start` 之后 | `the end of the range must come after its start` |
| 其他任何失败 | `the transcription failed`，附上引擎所说的内容 |

指向指南的提示是 `See the manual page "Installing the transcription tools"
(install-transcription).`
