# subedit-cli — 手册

<!-- machine-translated -->
> 本页为机器翻译，尚未经过熟悉该语言的读者审校。

subedit 的命令行工具：无需图形界面，即可批量处理字幕文件，或在脚本中处理。关于另一个程序，以及从哪里开始，参见[手册](../index.md)。

> **当前状态。** `subedit-cli` 的十七个子命令均已实现：检查、转换、移动、变换、重新调整帧率下的时间、对齐到帧网格、删除面向听障人士的提示、调整持续时间、替换文本、更改大小写、斜体和对话破折号、把字幕重新排好顺序、用 Gaupol 的模式更正文本、把文件依次接在一起、把一个文件切成两个，以及把翻译写到其主文件的位置上。本手册只描述已有的功能，绝不描述计划中的功能；接下来会有什么，见[路线图](../../../../feuille-de-route.md)。

## 章节

| 章节 | 内容 |
| :------ | :------ |
| [安装](installation.md) | 构建并安装工具 |
| [调用](invocation.md) | 全局选项、输出、退出码 |
| [设置文件](invocation.md#设置文件) | `adjust` 和 `correct` 从窗口的设置文件中读取的内容，`--config`，`--no-config` |
| [处理目录树](lots.md) | 多个文件，整个目录，`--recursive` |
| [`inspect`](inspect.md) | 报告文件由什么构成 |
| [`convert`](convert.md) | 以另一种格式或另一种形式写出文件 |
| [`shift`](shift.md) | 把每个位置移动相同的量 |
| [`transform`](transform.md) | 根据两个已知正确的点，更正每个位置 |
| [`framerate`](framerate.md) | 把文件的时间从一种帧率重新调整为另一种 |
| [`snap`](snap.md) | 把时间戳重新放回某种帧率的帧上 |
| [`hearing-impaired`](hearing-impaired.md) | 删除面向听障人士的提示 |
| [`adjust`](adjust.md) | 按阅读速度和界限调整持续时间 |
| [`replace`](replace.md) | 替换文本而不破坏标签 |
| [`case`](case.md) | 把文本改为单词首字母大写、句首字母大写、全部大写或全部小写 |
| [`italics`](italics.md) | 把文本设为斜体，或去掉斜体 |
| [`dialogue-dashes`](dialogue-dashes.md) | 加上对话破折号，或去掉 |
| [`sort`](sort.md) | 把字幕按开始时间重新排好顺序 |
| [`join-repeats`](join-repeats.md) | 把同一段文本中依次出现的几个片段合成一条字幕 |
| [`disjoint`](disjoint.md) | 截短相互重叠的字幕，使任何时间范围都不再与另一个重叠 |
| [`repair`](repair.md) | 对分段不当、顺序混乱且相互重叠的文件，依次执行 `join-repeats`、`sort` 和 `disjoint` |
| [`correct`](correct.md) | 用 Gaupol 的模式更正文本：提示、单词的连接与拆分、常见错误、大写规则、换行 |
| [`append`](append.md) | 把其他文件接在第一个文件之后，合成一个文件 |
| [`split-file`](split-file.md) | 把一个文件切成两个，后半部分移回起点 |
| [`pair`](pair.md) | 把翻译写到其主文件的位置上，并说明其各行是如何对应的 |
| [`transcribe`](transcribe.md) | 把视频或音频文件中的语音写成字幕 |
| [安装转录工具](install-transcription.md) | 引擎、模型和 `ffmpeg`，程序不随附提供 |
| [`config`](config.md) | 列出、显示和更改与窗口共用的操作设置 |
