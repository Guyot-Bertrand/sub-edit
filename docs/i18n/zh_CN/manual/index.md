# subedit — 手册

<!-- machine-translated -->
> 本页为机器翻译，尚未经过熟悉该语言的读者审校。

subedit 可以读取、检查字幕文件，并重新调整其时间。它由两个程序组成，二者共用同一个核心，因此规则相同，用词也相同。

| 程序 | 用途 | 其手册 |
| :-------- | :-------------- | :--------- |
| `subedit-gui` | 打开文件，在表格中查看和编辑，并对照影片观看 | [窗口手册](subedit-gui/index.md) |
| `subedit-cli` | 在脚本中批量处理文件，无需界面 | [命令行手册](subedit-cli/index.md) |

两者都能读写**这九种格式**——SubRip、WebVTT、SubViewer 2、
Sub Station Alpha、Advanced SSA、MicroDVD、MPL2、TMPlayer、LRC——并且根据内容而不是扩展名识别格式。**编码同样根据内容识别**：Latin-1、CP1252 或 UTF-16 编码的文件
可以正常打开，程序会说明读取时采用的编码，保存时再以文件原有的编码写回——文件打开后不作更改直接保存，一个字节也不会变。

## 从哪里开始

| 想做的事 | 去哪里看 |
| :----------------- | :------- |
| 安装工具 | [安装](subedit-cli/installation.md)——对两个程序都适用 |
| 打开文件，查看其中的内容 | [表格](subedit-gui/table.md) |
| 为偏移了固定量的字幕重新调整时间 | [操作](subedit-gui/operations.md#shift-positions)，或 [`shift`](subedit-cli/shift.md) |
| 为按另一种帧率编写的文件重新调整时间 | [`framerate`](subedit-cli/framerate.md)；如果定时本身已经正确，则用 [`snap`](subedit-cli/snap.md) |
| 了解文件是按哪种帧率编写的 | [帧网格](subedit-gui/frame-grid.md)，或 [`inspect`](subedit-cli/inspect.md) |
| 删除面向听障人士的提示 | [操作](subedit-gui/operations.md#remove-hearing-impaired-mentions)，或 [`hearing-impaired`](subedit-cli/hearing-impaired.md) |
| 一次性更正提示、常见错误、大小写和换行 | [`Correct Texts…`](subedit-gui/correct-texts.md)，或 [`correct`](subedit-cli/correct.md) |
| 使用系统词典逐词检查拼写 | [`Check Spelling…`](subedit-gui/check-spelling.md) |
| 延长过短而无法读完的字幕 | [`Adjust Durations…`](subedit-gui/operations.md#adjust-durations)，或 [`adjust`](subedit-cli/adjust.md) |
| 重新排好顺序混乱的文件 | [`sort`](subedit-cli/sort.md) |
| 把同一段文本中依次出现的几个片段合成一条字幕 | [`Join Repeated Subtitles`](subedit-gui/lines.md#连接重复的字幕)，或 [`join-repeats`](subedit-cli/join-repeats.md) |
| 一次性整理分段不当、顺序混乱且相互重叠的文件 | [`repair`](subedit-cli/repair.md) |
| 消除字幕之间的重叠 | [`Make Ranges Disjoint…`](subedit-gui/lines.md#使时间范围互不重叠)，或 [`disjoint`](subedit-cli/disjoint.md) |
| 把一部影片的两个部分首尾相接 | [`Append File…`](subedit-gui/operations.md#append-file)，或 [`append`](subedit-cli/append.md) |
| 把一部影片切成两个部分 | [`Split Project…`](subedit-gui/operations.md#split-project)，或 [`split-file`](subedit-cli/split-file.md) |
| 更改整个文件的大小写、斜体或对话破折号 | [操作](subedit-gui/operations.md#italic)，或 [`case`](subedit-cli/case.md)、[`italics`](subedit-cli/italics.md)、[`dialogue-dashes`](subedit-cli/dialogue-dashes.md) |
| 查找并替换某个词，同时不破坏标签 | [查找与替换](subedit-gui/find-and-replace.md)，或 [`replace`](subedit-cli/replace.md) |
| 对照原文翻译 | [打开翻译](subedit-gui/files.md#打开翻译)；若要按主文件重新调整翻译的时间，则用 [`pair`](subedit-cli/pair.md) |
| 一次处理上百个文件 | [处理目录树](subedit-cli/lots.md)，然后是[调用](subedit-cli/invocation.md) |

**`framerate` 与 `snap` 之间的混淆不会有任何提示。**
如果拿不准，可以先看 [`snap`](subedit-cli/snap.md#snap-不是-framerate而且二者混淆后不会有任何提示)，它开头就对二者作了比较。

## 本手册的承诺

**手册只描述已有的功能，绝不描述计划中的功能。** 今后会有什么、按什么顺序推出，见[路线图](../../../feuille-de-route.md)。

**本手册以英文撰写，并有各语言译本。** 译本与手册逐页对应，位于仓库的 `docs/i18n/` 目录下，每种语言一棵目录树。程序使用所在环境的语言；程序显示的名称——子命令、选项、菜单标题——在这里仍按程序的英文写法引用。

**示例和图片是生成的，正文不是。** `console` 代码块是实际运行命令得到的；屏幕截图则是构建真实的窗口后拍摄的。因此，二者都不可能描述一个已经不存在的工具。没有保障的只有它们周围的文字，其正确性依靠人工校对。

**屏幕截图展示两种配色**，即 `Light` 和 `Dark`——[首选项](subedit-gui/preferences.md#主题)中提供的正是这两种。

## 两个界面用词相同

阅读其中一份手册对使用另一个程序也有帮助，原因是：错误消息、格式名称、网格判定结果和撤销标签只在核心中写**一次**，两个程序都引用它们。被拒绝的文件，在两边得到的是同一句话。

两个界面的区别在于各自能做什么：

| | `subedit-gui` | `subedit-cli` |
| :--- | :------------ | :------------ |
| 同时处理多个文件 | 是——每个项目一个标签页 | 是 |
| 在主文件旁放置翻译 | 是 | 是——文本类子命令使用 `-t`，并用 [`pair`](subedit-cli/pair.md) 重新调整其时间 |
| 追加文件、拆分项目 | 是 | 是——[`append`](subedit-cli/append.md)、[`split-file`](subedit-cli/split-file.md) |
| 撤销 | 是，最多一千条记录 | 不适用——未经要求，不会原地更改任何内容 |
| 手动编辑文本或位置 | 是 | 否 |
| 插入、删除、合并和拆分行 | 是 | 否 |
| 查找并替换 | 是 | 是——[`replace`](subedit-cli/replace.md) |
| 剪切、复制和粘贴文本 | 是 | 否 |
| 调整持续时间、斜体、大小写、破折号 | 是 | 是——[`adjust`](subedit-cli/adjust.md)、[`italics`](subedit-cli/italics.md)、[`case`](subedit-cli/case.md)、[`dialogue-dashes`](subedit-cli/dialogue-dashes.md) |
| 按模式更正文本 | 是——`Correct Texts…` | 是——[`correct`](subedit-cli/correct.md) |
| 逐词检查拼写 | 是——`Check Spelling…` | 否 |
| 定时的同时观看影片 | 是 | 否 |
| 覆盖写入输入文件 | `Save` | `--in-place` |
