# 处理目录树

<!-- machine-translated -->
> 本页为机器翻译，尚未经过熟悉该语言的读者审校。

```
subedit-cli <subcommand> --recursive [options] <directory>...
```

凡是读取一批文件的子命令（包括 `inspect`，但不包括 [`append`](append.md)、[`split-file`](split-file.md) 和 [`pair`](pair.md)，它们的参数个数是一对文件或一部影片）都可以用**目录**代替文件，并加上 `--recursive`（`-r`）。本页说明遍历会选取哪些文件、按什么顺序、以及写到哪里。

**以下内容都不会改变对单个文件所做的处理**：每个文件各自独立地读取、处理和写入，一个文件失败不会使其他文件停止，四个退出码也与[调用](invocation.md#退出码)中的相同。遍历只产生一份路径列表，批处理的其余部分把它当作手工输入的路径来使用。

## 没有 `--recursive` 时，目录会被拒绝

这是用法错误，退出码为 `1`，不处理也不写入任何文件：

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?" -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?"
films: is a directory: use --recursive
code=1
```

## 遍历会选取什么

遍历是**递归的、确定的、不出意外的**：

- **顺序**——每个目录中的条目按名称**逐字节**排序（大写字母在小写字母之前，与机器的语言无关），子目录在其所在位置按深度优先遍历。两次运行得到完全相同的字节，在任何机器上都是如此；
- **不跟随符号链接**——无论指向文件还是目录：如果某棵树含有指向其上级目录的链接，遍历就永远不会结束；
- **忽略隐藏文件和目录**——即名称以点开头的文件和目录；
- **只保留已知格式的扩展名**：`.srt`、`.vtt`、`.ssa`、`.ass`、`.lrc`、`.sub`，不区分大小写。**`.txt` 被排除在外**：它对应两种格式，但更常见的是世界上无数的 `LISEZMOI.txt`，如果批处理在每一个这样的文件上都失败，就无法使用了。

**命令行上点名的文件从不过滤**。点名就是想要处理它：`subedit-cli shift --by 1 --in-place notes.txt` 会处理该文件，不论其扩展名是什么，只有当其内容不属于任何格式时才会报错。

**被搁置的文件会被计数**，并在 `-vv` 级别说明：

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli -vv inspect --recursive films 2>&1 | head -1 -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli -vv inspect --recursive films 2>&1 | head -1
films: 1 file left aside, with no extension of a known format
```

如果目录中没有任何可遍历的内容，会在默认级别说明这一点（`<directory>: holds no file in a format this tool walks for`），但这不算错误。

## 遍历写到哪里

**`--output-dir` 之下保留目录树的结构**：文件**相对于所给目录**的路径会在输出目录之下重现。所给目录自身的名称不在其中——这与带尾部斜杠的 `rsync` 语义相同，而不是 `cp -r` 的语义：由用户指定输出位置，再自行加上想要的层级（`--output-dir sortie/films`）。

<!-- example: mkdir -p films/a/b; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/x.srt; cp films/x.srt films/a/x.srt; cp films/x.srt films/a/b/y.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli shift --by 1 --recursive --output-dir sortie films; find sortie -type f | sort -->
```console
$ mkdir -p films/a/b; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/x.srt; cp films/x.srt films/a/x.srt; cp films/x.srt films/a/b/y.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli shift --by 1 --recursive --output-dir sortie films; find sortie -type f | sort
films/a/b/y.srt: 1 subtitle shifted by 1.000 s -> sortie/a/b/y.srt
films/a/x.srt: 1 subtitle shifted by 1.000 s -> sortie/a/x.srt
films/x.srt: 1 subtitle shifted by 1.000 s -> sortie/x.srt
3 of 3 files shifted
sortie/a/b/y.srt
sortie/a/x.srt
sortie/x.srt
```

输出目录及树中的各级目录如果不存在，都会被**创建**。使用 `--in-place` 时，保留下来的每个文件都写回原处。`convert` 更改的扩展名在树中同样会更改：`films/a/x.srt` 转换为 WebVTT 后写为 `sortie/a/x.vtt`。

**输出目录如果位于被遍历的树之内，则会被排除在遍历之外**：第二次运行不会再次读取第一次写入的内容。

## 两个目录与冲突

[输出位置](invocation.md#输出位置)的规则对此作了裁定：如果两个文件最终会落在同一路径，则**在写入任何内容之前**就被拒绝，退出码为 `1`，并指明这两个输入和该输出位置。

<!-- example: mkdir -p un/a deux/a; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > un/a/x.srt; cp un/a/x.srt deux/a/x.srt; subedit-cli shift --by 1 -r --output-dir sortie un deux; echo "code=$?" -->
```console
$ mkdir -p un/a deux/a; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > un/a/x.srt; cp un/a/x.srt deux/a/x.srt; subedit-cli shift --by 1 -r --output-dir sortie un deux; echo "code=$?"
sortie/a/x.srt: would be written by both un/a/x.srt and deux/a/x.srt
code=1
```

## 开销

批处理仍然是顺序执行的：其耗时是各文件耗时之和。基准测试 `make bench` 会测量遍历，以及同样的工作在单个文件中的耗时；数据见[性能记录](../../../../mesures/performances.md)。
