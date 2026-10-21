# 安装

<!-- machine-translated -->
> 本页为机器翻译，尚未经过熟悉该语言的读者审校。

**本页适用于两个程序**——`subedit-cli` 和 `subedit-gui`。它们一起构建，一起安装。

有两种途径：**原生软件包**（`.deb` 或 `.rpm`），或者**从源代码构建**，后者可以安装到任意位置。

## 软件包

两种软件包都由 CPack 根据**同一套安装规则**生成：它们把相同的文件放在相同的位置，区别只在于所依赖的软件包名称——同样的库在 Debian 和 Fedora 上叫法不同。

```bash
sudo apt install ./subedit_<version>_amd64.deb      # Debian, Ubuntu
sudo dnf install ./subedit-<version>.x86_64.rpm     # Fedora, and relatives
```

| 软件包安装的内容 | 位置 |
| :---------------------- | :-- |
| `subedit-cli`, `subedit-gui` | `/usr/bin` |
| 菜单项、图标、软件中心元数据 | `/usr/share/applications`, `/usr/share/icons`, `/usr/share/metainfo` |
| 本手册（Markdown 格式） | `/usr/share/subedit/manual` |
| 手册在每种已安装语言中的翻译——加泰罗尼亚语、捷克语、德语、西班牙语、芬兰语、法语、加利西亚语、希伯来语、匈牙利语、西方国际语、冰岛语、意大利语、荷兰语、波兰语、葡萄牙语、巴西葡萄牙语、俄语、塞尔维亚语、土耳其语、简体中文 | `/usr/share/subedit/manual/<language>`——`ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR`, `ru`, `sr`, `tr`, `zh_CN` |
| Gaupol 的更正模式 | `/usr/share/subedit/patterns` |
| `subedit-cli` 的 man 手册页（每个子命令一页）以及 `subedit-gui` 的手册页——参见 [man 手册页](invocation.md#man-手册页) | `/usr/share/man/man1` |
| `subedit-cli` 的页面在每种已安装语言中的版本——加泰罗尼亚语、捷克语、德语、西班牙语、芬兰语、法语、加利西亚语、希伯来语、匈牙利语、西方国际语、冰岛语、意大利语、荷兰语、波兰语、葡萄牙语、巴西葡萄牙语、俄语、塞尔维亚语、土耳其语、简体中文 | `/usr/share/man/<language>/man1`——`ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR`, `ru`, `sr`, `tr`, `zh_CN` |

两种软件包对 `ffmpeg` 都是**推荐而非必需**：没有它，窗口只是不再提供影片所声明的帧率——参见 [`ffmpeg` 带来的变化](../subedit-gui/video.md#不需要-ffmpeg)。

### 软件包对发行版的要求

**请使用 `apt` 或 `dnf` 安装，绝不要使用 `dpkg -i` 或 `rpm -i`**。这两条命令只是把软件包放下，并不解决其依赖关系：它们仅仅列出缺少的依赖，然后就停止。下面是在一个 `dnf` 本可以取得所需依赖的 Fedora 上，`rpm -i` 的回答：

```console
$ rpm -i subedit-0.13.0-1.fc43.x86_64.rpm
error: Failed dependencies:
        libicui18n.so.77()(64bit) is needed by subedit-0.13.0-1.fc43.x86_64
        libicuuc.so.77()(64bit) is needed by subedit-0.13.0-1.fc43.x86_64
```

`sudo dnf install ./subedit-<version>-1.fc43.x86_64.rpm` 会取得这些库（只要它们存在）。

**应选用与所用 Fedora 对应的那个 `.rpm`**：文件名中带有版本，即 `fc42`、`fc43`、`fc44`。每一个都在对应的 Fedora 上、针对它的 ICU 构建——`cat /etc/fedora-release` 会说明该选哪一个。来自其他 Fedora 的 `.rpm` 会要求当前系统没有的 ICU。

| 依赖项 | `.deb` | `.rpm` | 约束 |
| :--------- | :----- | :----- | :--------- |
| Qt 6：`Widgets`、`Gui`、`Core` | `libqt6widgets6`, `libqt6gui6`, `libqt6core6` | `qt6-qtbase-gui` | 窗口需要 **≥ 6.4** |
| libmpv | `libmpv2` 或 `libmpv1` | `mpv-libs` | 二进制文件链接 `libmpv.so.2` |
| Enchant 2 | `libenchant-2-2` | `enchant2` | 拼写检查器，以及 [`correct`](correct.md) 的 `join-words` 和 `split-words` 任务 |
| **ICU** | **`libicu74`** | `libicu`，即**构建它的 Fedora 所带的** `libicuuc.so.N` 和 `libicui18n.so.N` | **必须与 `.rpm` 所对应的 Fedora 的版本完全一致**——见下文 |
| glibc 和 libstdc++ | — | — | **足够新**：`.deb` 要求 Ubuntu 24.04 的版本，`.rpm` 要求其对应的 Fedora 的版本 |

**ICU 是关键的约束，而且无法通过修改软件包中的一行来放宽**。ICU 的符号带有其主版本号（`u_strToUTF8_74`）：针对 ICU 74 构建的二进制文件无法链接到 ICU 76 或 77。把依赖写得更宽，只会让软件包能够安装，然后在启动时失败——声明为精确版本，才是诚实的做法。**这就是 `.rpm` 不是只构建一次，而是在每个已发布的 Fedora 上各构建一次的原因**，每次都针对它的 ICU；`.deb` 则在 Ubuntu 24.04 上构建。

| 发行版 | 结果 |
| :----------- | :---------------- |
| Ubuntu 24.04 | `.deb` 可以安装：那里的 ICU 版本为 74 |
| Fedora 42、43、44 | 对应其版本的 `.rpm`（`…fc42…`、`…fc43…`、`…fc44…`）**通过 `dnf`** 安装，针对该 Fedora 的 ICU 构建 |
| Fedora rawhide（下一个版本）以及其他 `.rpm` 发行版 | **不行**：没有为它们构建 `.rpm`——请从源代码构建 |
| ICU 版本不是 74 的任何 Debian 或 Ubuntu 发行版 | **不行**：`.deb` 要求 `libicu74`——请从源代码构建 |

**从源代码构建是备用方案，而且没有这一约束**：它链接机器上的 ICU，不论其版本如何。如果发行版的 glibc 比软件包所要求的更旧，也需要这样做。

### 各自测试了什么，没测试什么

**两者检查的范围并不相同，说明这一点，好过让人相信存在并不存在的对等**。开发在 Ubuntu 上进行；在那里构建的 `.rpm` 无法在那里安装。它在别处安装：每周一次，在容器中的 Fedora 上。

| | `.deb` | `.rpm` |
| :--- | :----- | :----- |
| 文件列表 | 已检查 | 已检查，并且**与 `.deb` 的列表作了比较** |
| 声明的依赖项 | 已检查是否存在 | 已检查是否存在 |
| 依赖项的名称在发行版中是否存在 | 是，它们是 Ubuntu 的 | **是**，由 `dnf` 解析 |
| 软件包能否安装 | 否——这需要管理员权限 | **是**，在容器中的 Fedora 上 |
| 已安装的二进制文件能否启动 | 是，从临时前缀启动 | **是**，从已安装的软件包启动 |

**Fedora 检查测试的是在 Ubuntu 上构建的 `.rpm`，而不是已发布的那些**。该镜像本身没有 ICU 74：软件包之所以能在那里安装，是因为 Fedora 保留了一个兼容性软件包 `libicu74`，只要这个软件包存在，检查就会是绿色。已发布的 `.rpm` 文件在各自的 Fedora 上构建，不再依赖那个兼容性软件包。参见[软件包对发行版的要求](#软件包对发行版的要求)。

**比较两份列表是最重要的检查**。这两种软件包出自同一次安装：二者之间的差异，只能说明安装规则有缺陷，而不是格式有缺陷。

**`.deb` 仍然无法证明的是它能否安装**，原因没有变：`dpkg -i` 需要管理员权限，而质量门禁并不具备，也不应该去索取。而 `.rpm` 则在一个不属于任何人的容器中安装。参见 [ADR 0023](../../../../adr/0023-deb-et-rpm-pour-la-premiere-livraison.md)。

**不提供 Flatpak 和 AppImage**，这是一个选择，而不是疏忽。同一份 ADR 对此作了说明，而且这是**最终**决定：既不属于 Debian 系也不属于 Fedora 系的发行版，请从源代码构建，下一节会完整描述这一过程。

## 从源代码构建

### 前提条件

| 需要什么 | 用途 | Debian、Ubuntu |
| :------------ | :------- | :------------- |
| CMake ≥ 3.28 | 构建 | `cmake` |
| C++23 编译器——GCC 13 即可 | — | `g++` |
| `make`、`pkg-config` | 构建 | `make`, `pkg-config` |
| CLI11 | 读取命令行 | `libcli11-dev` |
| Qt 6 ≥ 6.4，`Widgets` 模块 | 窗口 | `qt6-base-dev` |
| `libmpv` | 内置播放器 | `libmpv-dev` |
| ICU | 读取编码 | `libicu-dev` |
| Enchant 2 | 拼写检查器的库 | `libenchant-2-dev` |

**即使只构建 `subedit-cli`，最后四项也是必需的**：CMake 配置会为整个项目查找它们，缺少时就会停止。ICU 和 Enchant 的必需性是双重的——可执行文件会加载它们，一个用于转换编码，另一个用于拼写检查器——它们是可执行文件在运行时加载的仅有的第三方库。

**第一次运行 `cmake` 时需要网络连接**：Catch2 测试库在配置时从 GitHub 获取。此后不会再次下载。若要在没有网络的情况下只构建命令行工具，参见下文。

#### 不需要的内容

| 可选的内容 | 没有它会失去什么 |
| :-------------------- | :--------------------- |
| `ffmpeg`，主要是其中的 `ffprobe` | 窗口不再提供影片所声明的帧率——[详情](../subedit-gui/video.md#不需要-ffmpeg) |
| 显示服务器 | 对 `subedit-cli` 没有任何影响；`subedit-gui` 需要它才能显示自己 |

### 构建

```bash
git clone git@github.com:Guyot-Bertrand/sub-edit.git subedit
cd subedit
make build
```

二进制文件生成在 `build/dev/bin/` 中——即 `subedit-cli` 和 `subedit-gui`。

要构建优化版本：

```bash
make release
```

两个二进制文件随后位于 `build/release/bin/` 中——即 `subedit-cli` 和 `subedit-gui`。该目标不构建其他任何内容：基准测试和端到端测试框架都不构建，它们有各自的目标。

**请使用这个目标，而不是与之等价的两条 `cmake` 命令**，后者无从得知链接时优化允许启动多少个进程：每次链接时，它都会启动与核心数相同的进程。`make release` 会把 `JOBS` 传给它，就像对构建的其余部分那样。

### 执行安装

```bash
make release
cmake --install build/release --prefix ~/.local
```

所安装的内容，位于 `GNUInstallDirs` 为所选前缀设定的路径下：

| 文件 | 位置 |
| :------ | :-- |
| `subedit-cli`, `subedit-gui` | `<prefix>/bin` |
| 桌面菜单项 | `<prefix>/share/applications` |
| 图标 | `<prefix>/share/icons/hicolor/scalable/apps` |
| 软件中心元数据 | `<prefix>/share/metainfo` |
| 本手册（Markdown 格式） | `<prefix>/share/subedit/manual` |
| 手册在每种已安装语言中的翻译——加泰罗尼亚语、捷克语、德语、西班牙语、芬兰语、法语、加利西亚语、希伯来语、匈牙利语、西方国际语、冰岛语、意大利语、荷兰语、波兰语、葡萄牙语、巴西葡萄牙语、俄语、塞尔维亚语、土耳其语、简体中文 | `<prefix>/share/subedit/manual/<language>`——`ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR`, `ru`, `sr`, `tr`, `zh_CN` |
| Gaupol 的更正模式 | `<prefix>/share/subedit/patterns` |
| 两个二进制文件的 man 手册页，`subedit-cli` 每个子命令一页 | `<prefix>/share/man/man1` |
| `subedit-cli` 的页面在每种已安装语言中的版本——加泰罗尼亚语、捷克语、德语、西班牙语、芬兰语、法语、加利西亚语、希伯来语、匈牙利语、西方国际语、冰岛语、意大利语、荷兰语、波兰语、葡萄牙语、巴西葡萄牙语、俄语、塞尔维亚语、土耳其语、简体中文 | `<prefix>/share/man/<language>/man1`——`ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR`, `ru`, `sr`, `tr`, `zh_CN` |

**它们与软件包安装的是同一批文件**：软件包就是由它们生成的，而不是另行描述一遍。

使用 `--prefix ~/.local` 时，两个二进制文件都会落在 `~/.local/bin` 中，大多数发行版的 `PATH` 里都有这个目录。系统前缀——例如 `/usr/local`——则需要相应的权限。

**放在 `~/.local` 下的菜单项并不总是立刻出现**：桌面环境按自己的节奏重新读取缓存。在此期间，`subedit-gui` 可以像任何其他命令一样从终端启动。

支持 `DESTDIR`，这是打包者所期望的：暂存安装不会触碰暂存目录之外的任何内容。

```bash
DESTDIR=/tmp/scene cmake --install build/release --prefix /usr
```

### 只构建命令行工具

测试默认会被构建，而它们正是需要网络的原因——`make release` 不构建它们，但会配置项目，而获取 Catch2 的正是配置这一步。只需禁用一次：该设置会保留在 CMake 缓存中，后续构建沿用它。

```bash
cmake --preset release -DSUBEDIT_BUILD_TESTS=OFF
make release
```

这并不能免除贡献者的任何义务：质量门禁确实要求这些测试。

### 链接时构建失败

对有效的代码出现大量“undefined reference to `std::cout`”，通常意味着系统的 `c++` 替代项指向 `gcc` 而不是 `g++`：C++ 能够编译，但标准库没有被链接。

```bash
ls -l /etc/alternatives/c++                                       # check
sudo update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++ 100   # fix
CXX=g++ make build                                                # work around
```

CMake 配置会检测到这种情况，并以这条消息停止，而不是让链接失败。

### 安装软件包时提示 `libicu` 的问题

```console
error: Failed dependencies:
        libicuuc.so.74()(64bit) is needed by subedit-<version>.x86_64
```

有三种原因，可以根据键入的命令和文件名区分：

- **`rpm -i`**：它不解决任何依赖。请改用 `sudo dnf install ./subedit-<version>.fcNN.x86_64.rpm` 重试；
- **文件名中没有 `fcNN` 的 `.rpm`**，或来自其他 Fedora 的软件包：它要求构建它的那台机器的 ICU。请选用 `fcNN` 与 `cat /etc/fedora-release` 的输出一致的那个；
- **没有适合当前版本的 `.rpm`**：Fedora rawhide 或其他发行版。本仓库的任何软件包都无法在那里安装；请[从源代码构建](#从源代码构建)。
