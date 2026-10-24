# `transcribe`

<!-- machine-translated -->
> 本页为机器翻译，尚未经过熟悉该语言的读者审校。

```
subedit-cli transcribe --output FILE
                       [--to FORMAT]
                       [--audio-track N]
                       [--language CODE]
                       [--model FILE]
                       [--engine NAME]
                       [--keep-audio FILE]
                       [--raw]
                       <input>
```

把**视频或音频文件中的语音写成字幕文件**。音频经过准备后交给引擎，由引擎找出说了什么以及何时说的，引擎切出的片段再被合并成符合时长和长度要求的字幕。

**所需的一切都不随程序提供**：引擎（whisper.cpp）、模型和 `ffmpeg` 由用户自行安装，并由用户说明模型所在的位置。[安装转录
工具](install-transcription.md)说明在哪里可以找到每一样，以及该选哪个模型。**缺少它们不会破坏任何东西**：命令会说明缺了什么，并指向该页，不写入任何内容，并以非零退出码退出。

- **输入是视频或音频文件。** 对于视频，`ffmpeg` 会提取一条音轨；没有 `ffmpeg` 时，引擎可直接读取的音频文件（`wav`、`mp3`、`flac`、`ogg`）仍可转录，但视频不行，指定音轨时任何文件都不行。
- **一次只处理一个文件**，输出位置是 `--output`：没有 `--output-dir` 或 `--in-place`，因为结果是新文件，而不是改写。
- **语言会被检测**，除非用 `--language` 强制指定，并且会说明所用的语言。只为英语制作的模型不检测任何语言，也不转录其他语言：它无法接受的语言会在任何工作开始*之前*被拒绝，并指出该模型的限制。
- **片段会被合并和切分**，依据是各项设置的限制（`transcription.merge-gap-ms`、`transcription.max-duration-ms`、`transcription.max-characters`）：同一句话中相距很近的两个片段合为一条字幕，过长或过宽的片段则在词处切开。**`--raw` 按引擎给出的原样写入片段**，每条字幕一个片段，这也是查看合并起了什么作用的办法。

<!-- example: subedit-cli transcribe --help -->
```console
$ subedit-cli transcribe --help
Write the speech of a video or an audio file as subtitles
Usage: subedit-cli transcribe [OPTIONS] input

Positionals:
  input TEXT REQUIRED         Video or audio file to transcribe

Options:
  -h,--help                   Print this help message and exit
  --output FILE               Subtitle file to write
  --to TEXT:{srt,vtt,subviewer2,ssa,ass,microdvd,mpl2,tmplayer,lrc}
                              Format to write; the one the extension of --output says by default
  --audio-track N             Audio track of a video, counted from 0; the first by default
  --language CODE             Language of the speech: fr, en; detected by default
  --model FILE                Model file; the transcription.model setting by default
  --engine NAME               Engine; the transcription.engine setting by default
  --keep-audio FILE           Keep the extracted audio in this file
  --raw                       Write the segments as the engine cut them
```

## 参数与选项

| 选项 | 必需 | 取值 | 默认值 |
| :----- | :----- | :------ | :----- |
| `<input>` | 是 | 视频或音频文件 | — |
| `--output` | 是 | 要写入的字幕文件；其目录不存在时会创建 | — |
| `--to` | 否 | 格式的简称：`srt`、`vtt`、`subviewer2`、`ssa`、`ass`、`microdvd`、`mpl2`、`tmplayer`、`lrc` | `--output` 的扩展名所指的格式，否则为 `srt` |
| `--audio-track` | 否 | 从 0 开始的整数：在视频各音轨中的位置 | `0`，即第一条 |
| `--language` | 否 | 两个小写字母，ISO 639-1 代码：`en`、`fr` | 设置 `transcription.language`；为空：自动检测 |
| `--model` | 否 | 模型文件 | 设置 `transcription.model` |
| `--engine` | 否 | 引擎的名称：`whisper.cpp` | 设置 `transcription.engine` |
| `--keep-audio` | 否 | 保存提取出的音频的文件，格式为 WAV 文件中的 16 kHz 单声道 16 位 PCM；其目录不存在时会创建 | 音频不保存在任何地方 |
| `--raw` | 否 | 开关：写入引擎的片段，不合并也不切分 | 关闭 |

**设置**与 [`adjust`](adjust.md) 的设置一样从设置文件读取——参见[设置文件](invocation.md#设置文件)——并且每一项在这里都有对应的选项：

| 键 | 含义 | 默认值 |
| :-- | :------ | :------ |
| `transcription.engine` | 引擎 | `whisper.cpp` |
| `transcription.executable` | 引擎的程序，绝对路径；为空：在 `PATH` 中查找 | 空 |
| `transcription.model` | 模型文件，绝对路径 | 空 |
| `transcription.language` | 音频的语言，两个字母；为空：自动检测 | 空 |
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

## 输出

**标准输出**——无内容：结果就是写出的文件。

| 级别 | 在标准错误输出上追加的内容 |
| :----- | :------------------------------------- |
| 1 | `<input>: N subtitles transcribed, language: xx -> <output>` |
| 2 | 引擎的进度，`transcribing: 25%`，按引擎的报告给出——每约三十秒音频一步 |

### 以 JSON 输出

使用 [`--format json`](invocation.md#脚本可读取的输出)（写在子命令之前的全局选项）时，文件的对象在 `language` 中包含所用的语言，并包含 `counts.subtitles`（写出的字幕）和 `counts.segments`（生成这些字幕所用的引擎片段）；当 `--keep-audio` 保存了音频时，还包含 `audio`。

## 退出码

与整个工具相同：文件已写出则为 `0`；无法写出则为 `2`——缺少工具或模型、模型无法接受的语言、提取或转录失败——；用法错误则为 `1`，例如没有人注册过的引擎。

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
| 其他任何失败 | `the transcription failed`，附上引擎所说的内容 |

指向指南的提示是 `See the manual page "Installing the transcription tools"
(install-transcription).`
