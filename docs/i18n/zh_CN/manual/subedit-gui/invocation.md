# 调用

<!-- machine-translated -->
> 本页为机器翻译，尚未经过熟悉该语言的读者审校。

```
subedit-gui [--version] [<file>]
```

| 参数 | 必需 | 值 | 默认值 |
| :------- | :------- | :---- | :------ |
| `<file>` | 否 | 要打开的字幕文件的路径 | 无——窗口打开时为空 |
| `--version` | 否 | — | — |

**只处理一个文件**。其后的参数会被忽略：窗口一次只容纳一个项目。

## `--version`

在标准输出上写出命令行的 `--version` 所写的内容，并立即返回，不打开窗口。退出码为 `0`。

**第一行只有 `subedit <version>`**，随后每个窗口所依赖的库各占一行，写明运行时找到的版本，括号中是编译时所用的版本；最后是已安装的界面语言，以代码表示。在装有 Qt 6.4.2 的机器上：

```console
$ subedit-gui --version
subedit <version>
  ICU      74.2 (built with 74.2)
  Enchant  2.3.3 (built with 2.3.3)
  Qt       6.4.2 (built with 6.4.2)
  libmpv   2.2 (built with 2.2)
  Languages: en, fr
```

这里没有照抄数字：它们属于具体的机器，而且本手册没有可让程序自身的版本信息保持同步的生成块。所列的库中有命令行使用的 ICU 和 Enchant，也有只有窗口才链接的两个——Qt，以及播放器 libmpv，其版本取自客户端 API 的版本，即 `mpv.pc` 所声明的版本。其余内容见[命令行手册](../subedit-cli/invocation.md#--version)。`built with` 和 `Languages` 这两处文字随窗口的语言而变，其他内容则不会。

`帮助(H) ▸ 关于 subedit(A)` 以表格显示相同的库，来源是同一份列表——名称、已加载的版本、编译时所用的版本，每个库一行——表格下方是语言；其标题随窗口的语言而变。

## 打开文件

```console
$ subedit-gui film.srt
```

读取的格式是打开过滤器所列出的**这九种**——见[打开与保存](files.md#打开)。格式根据内容而不是扩展名识别，**编码也是如此**：文件带有 BOM 时按 BOM 判断，否则采用其字节所显示的编码。

## 打开失败时

文件不存在、无法读取、其字节在任何编码下都无法解码，或者其内容不属于任何格式：**窗口仍会打开，内容为空**，并把失败原因写到错误流。

**消息会指出原因**，用词与命令行相同：

| 消息 | 发生了什么 |
| :------ | :------------ |
| `does not exist` | 该路径不指向任何文件 |
| `cannot be opened: permission denied` | 系统拒绝打开 |
| `cannot be read` | 系统出于其他原因拒绝 |
| `cannot be decoded in the chosen encoding` | 字节无法按所选的编码读取 |
| `is in no format this tool knows` | 没有任何格式能识别其内容 |
| `holds nothing recognisable as a subtitle` | 格式已识别，但其中没有任何字幕 |

```console
$ subedit-gui notes.txt
subedit-gui: notes.txt: is in no format this tool knows
```

宁可空着打开也不拒绝启动：这个工具仍然有用，参数出错不是不启动的理由。

**格式有问题但可读的文件会被打开**，读取过程中补救的内容可以在状态栏的一个按钮中查看——见[读取的诊断信息](files.md#读取的诊断信息)。

## 退出码

| 退出码 | 何时 |
| :--- | :--- |
| `0` | 窗口已打开并正常关闭，或者 `--version` 已作出应答 |
| `2` | 打开之前发生意外错误——缺少 Qt、显示不可用 |

打开文件失败**不会改变退出码**：窗口已经打开，退出码报告的就是这一点。

## 手册页

安装会在 `share/man/man1` 下放入 `subedit-gui(1)`：

```console
$ man subedit-gui
```

它给出调用形式、`--version`、打开失败的消息、退出码、两个环境变量以及设置文件。**它不描述窗口**——这由本手册负责，手册页会指向这里。

## Qt 平台

`QT_QPA_PLATFORM` 决定窗口在什么平台上运行。**如果设置了该变量，`subedit-gui` 会予以尊重，无需任何参数。**

如果没有设置，则交由 Qt 选择：内置播放器在窗口内部自行绘制画面，对平台没有任何要求——见[播放器](player.md)。

`QT_QPA_PLATFORM=offscreen` 可以在没有图形服务器的环境中运行窗口——持续集成和测试就是这样做的。

## 语言

窗口使用其环境的语言，**只在启动时选定一次**：更改正在运行的窗口的环境变量，要到下次启动才会生效。变量和规则与命令行相同——`LANGUAGE`、`LC_ALL`、`LC_MESSAGES`、`LANG`，`LC_ALL=C` 表示英文，所请求的语言未安装时则**不加任何说明地**使用英文——详见[命令行手册](../subedit-cli/invocation.md#语言)。语言目录是同一批文件，安装在 `<prefix>/share/subedit/locale` 下。

**在 `编辑(E) ▸ 首选项…(E)` 中选定的语言优先**，只有当它为 `System default` 时才读取环境，在选定语言之前它一直是这个值：在此选定的已安装语言会压过所有变量，包括 `LC_ALL=C`。这一选择同样在下次启动时生效。见[界面语言](preferences.md#界面语言)。

| 对象 | 是否随语言变化 |
| :--- | :------------------- |
| 菜单及其条目（含带下划线的字母）、工具栏、工具提示 | 是 |
| 表格的列和 `视图(V) ▸ 列(C)` 的各列、标签页、状态行、播放栏 | 是 |
| Qt 自己绘制的标准按钮（`OK`、`Cancel`） | 是，前提是已安装该语言的 Qt 自带语言目录（`qtbase_<language>.qm`）；否则为英文 |
| 对话框、`Correct Texts…` 的各页、文件对话框的标题和过滤器、消息框及其内容 | 是 |
| 窗口写到错误流的行，`subedit-gui: …` | 是，程序名除外 |
| `Correct Texts…` 中更正模式的名称，以及工具提示中的说明 | 是，仅限程序自带的模式；你自己的模式按其文件所写的原样显示 |
| 编码、行尾和拼写语言的名称（`UTF-8`、`CRLF`、`en_US`） | 从不：它们是名称，不是词语 |
| 键盘快捷键 | 从不：`Ctrl+S` 在每种语言中都是 `Ctrl+S` |
| 语言目录中没有翻译的消息 | 以英文显示 |

此版本安装了 `ca`、`cs`、`de`、`es`、`fi`、`fr`、`gl`、`he`、`hu`、`ie`、`is`、`it_IT`、`nl`、`pl`、`pt`、`pt_BR`、`ru`、`sr`、`tr` 和 `zh_CN`——加泰罗尼亚语、捷克语、德语、西班牙语、芬兰语、法语、加利西亚语、希伯来语、匈牙利语、西方国际语、冰岛语、意大利语、荷兰语、波兰语、葡萄牙语、巴西葡萄牙语、俄语、塞尔维亚语、土耳其语和简体中文——均已完整翻译。法语已由熟悉该语言的读者审校；加泰罗尼亚语、捷克语、德语、西班牙语、芬兰语、加利西亚语、希伯来语、匈牙利语、西方国际语、冰岛语、意大利语、荷兰语、波兰语、葡萄牙语、巴西葡萄牙语、俄语、塞尔维亚语、土耳其语和简体中文为机器翻译，尚未经过熟悉该语言的读者审校。
<!-- machine-translated: ca -->
<!-- machine-translated: cs -->
<!-- machine-translated: de -->
<!-- machine-translated: es -->
<!-- machine-translated: fi -->
<!-- machine-translated: gl -->
<!-- machine-translated: he -->
<!-- machine-translated: hu -->
<!-- machine-translated: ie -->
<!-- machine-translated: is -->
<!-- machine-translated: it_IT -->
<!-- machine-translated: nl -->
<!-- machine-translated: pl -->
<!-- machine-translated: pt -->
<!-- machine-translated: pt_BR -->
<!-- machine-translated: ru -->
<!-- machine-translated: sr -->
<!-- machine-translated: tr -->
<!-- machine-translated: zh_CN -->
在法语桌面上，菜单栏显示为 `&Fichier`、`É&dition`、`&Affichage`、`&Vidéo`、`&Outils`、`&Projets`、`Aid&e`。本手册的屏幕截图是在简体中文环境下截取的。
