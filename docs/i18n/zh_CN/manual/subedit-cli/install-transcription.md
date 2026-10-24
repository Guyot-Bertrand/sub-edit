# 安装转录工具

<!-- machine-translated -->
> 本页为机器翻译，尚未经过熟悉该语言的读者审校。

把音频转录为字幕——命令行中用 [`transcribe`](transcribe.md)，窗口中用 [`Transcribe Audio…`](../subedit-gui/video.md#转录音频)——需要三样**subedit 既不随程序提供也不下载**的东西：一个引擎、一个供该引擎使用的模型，以及 `ffmpeg`。由您自己安装它们，并告诉 subedit 模型在哪里。

**没有它们，什么也不会坏。** 程序的其余部分照常工作；只有转录功能会自行停用，并给出指向这里的消息。

## 引擎：whisper.cpp

[whisper.cpp](https://github.com/ggml-org/whisper.cpp) 是 subedit 运行的引擎。它是一个名为 `whisper-cli` 的程序，在普通处理器上即可运行——显卡只会让它更快。按它自己的文档构建它（有 C++ 编译器和 CMake 就够了）：

```bash
git clone https://github.com/ggml-org/whisper.cpp
cd whisper.cpp
cmake -B build -DBUILD_SHARED_LIBS=OFF
cmake --build build -j --config Release
```

之后程序位于 `build/bin/whisper-cli`。如上用 `-DBUILD_SHARED_LIBS=OFF` 构建时，它是一个单独的文件，旁边不需要任何库，因此可以复制到任何地方。把它放到 `PATH` 能找到的地方，或者用设置 `transcription.executable` 把**它的路径告诉** subedit——在窗口中，[首选项](../subedit-gui/preferences.md#转录)里有一个字段用于此。针对显卡的构建会在第一条 `cmake` 命令中增加一个选项：NVIDIA 显卡用 `-DGGML_CUDA=1`，大多数其他显卡用 `-DGGML_VULKAN=1`；subedit 运行您安装的那个，不会替您选择任何东西。

若只想保留程序，请**安装**它，然后删除克隆：

```bash
cmake --install build --prefix ~/.local
```

`--prefix` 指定安装位置。`~/.local` 不需要管理员权限，其 `bin` 目录必须在您的 `PATH` 中；不加该选项时默认是 `/usr/local`，需要 `sudo`。`whisper-cli` 会落在该前缀的 `bin` 目录中，与 subedit 不使用的工具放在一起；它到位之后，克隆及其 `build` 目录即可删除。

## 模型

引擎读取一个**模型**，那是一个几百兆字节到几吉字节的文件，只需下载一次。官方模型列在 [huggingface.co/ggerganov/whisper.cpp](https://huggingface.co/ggerganov/whisper.cpp)，引擎的仓库里有一个脚本可以获取其中一个：

```bash
sh ./models/download-ggml-model.sh small
```

然后在命令行上用 `--model` 给出它的路径，或者用设置 `transcription.model`，或者在窗口的[首选项](../subedit-gui/preferences.md#转录)中给出，首选项会记住它。

### 哪种语言用哪个模型

Whisper 找到语言之后不会更换模型：**您给出的模型决定了它能做什么。**

| 模型 | 语言 | 检测语言 | 大小 |
| :---- | :-------- | :------------------- | :--- |
| `tiny`, `base`, `small`, `medium`, `large-v3`, `large-v3-turbo` | 约一百种 | 是 | 从约 75 MB 到约 3 GB |
| `tiny.en`, `base.en`, `small.en`, `medium.en` | 仅英语 | 否 | 与上面相同大小的一样 |

- **除英语以外的任何语言，请选用多语言模型**——名称中没有 `.en` 的模型。仅英语的模型会拒绝其他语言：命令会在做任何工作之前这样说明。
- **最小的模型在英语之外明显更差。** 对于法语、德语、西班牙语等，`small` 是最低要求，而 `medium` 或 `large-v3-turbo` 的效果好得多，代价是更多的内存和更多的时间。
- **语言检测的好坏取决于模型。** 小模型可能会在以音乐或静音开头的音频上判断错误；知道语言时，请用 `--language` 说明。

## ffmpeg

[`ffmpeg`](https://ffmpeg.org/download.html) 提取视频的音轨，随它附带的 `ffprobe` 则列出各音轨。请用您所用发行版的包管理器安装它。没有它时，引擎可直接读取的音频文件（`wav`、`mp3`、`flac`、`ogg`）仍然可以转录，但视频不行。

## 检查是否可用

```bash
subedit-cli transcribe --model /path/to/ggml-small.bin --output film.srt film.mkv
```

如果缺少什么，消息会说明缺的是什么，并指向本页。
