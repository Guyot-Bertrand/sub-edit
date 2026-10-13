# Processing a tree

```
subedit-cli <subcommand> --recursive [options] <directory>...
```

Every subcommand that reads a batch of files — `inspect` included, and not
[`append`](append.md), [`split-file`](split-file.md) nor [`pair`](pair.md), whose arity is that of a pair or
of a film — accepts a **directory** in place of a file, with `--recursive` (`-r`).
This page says what the walk takes, in what order, and where it writes.

**Nothing of what follows changes what is done to a file**: each one is read,
processed and written independently, the failure of one does not stop the others, and the
four exit codes are those of [the invocation](invocation.md#exit-codes).
The walk produces a list of paths, which the rest of the batch consumes as if they
had been typed.

## Without `--recursive`, a directory is refused

Usage error, code `1`, no file processed nor written:

<!-- exemple: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?" -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?"
films: is a directory: use --recursive
code=1
```

## What the walk takes

The walk is **recursive, deterministic and unsurprising**:

- **the order** — the entries of each directory are taken in the order of their
  names, **byte by byte** (capitals before lower case, regardless of the
  machine's language), and a subdirectory is walked in its place, depth
  first. Two runs give the same bytes, on every
  machine;
- **symbolic links are not followed** — neither to a file, nor to a
  directory: a tree that contains a link to one of its parents would have no
  end;
- **hidden files and directories are ignored** — those whose name begins with
  a dot;
- **only the extensions of the known formats are kept**: `.srt`, `.vtt`,
  `.ssa`, `.ass`, `.lrc`, `.sub`, regardless of case. **`.txt` is excluded**: it
  names two formats, but above all every `LISEZMOI.txt` in the world, and a batch that
  failed on each one would be unusable.

**A file named on the command line is never filtered.** Naming it is
wanting it: `subedit-cli shift --by 1 --in-place notes.txt` processes it, whatever
its extension, and only complains if its content is in no format.

**What was left aside is counted**, and said at level `-vv`:

<!-- exemple: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli -vv inspect --recursive films 2>&1 | head -1 -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli -vv inspect --recursive films 2>&1 | head -1
films: 1 file left aside, with no extension of a known format
```

A directory that contains nothing to walk says so at the default level
(`<directory>: holds no file in a format this tool walks for`) and is not an error.

## Where the walk writes

**The tree is kept under `--output-dir`**: the path of a file,
**relative to the directory given**, is reproduced under the output directory. The name
of the directory given is not part of it — these are the semantics of `rsync` with a
trailing slash, not those of `cp -r`: one names the destination, and adds the
level one wants to it (`--output-dir sortie/films`).

<!-- exemple: mkdir -p films/a/b; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/x.srt; cp films/x.srt films/a/x.srt; cp films/x.srt films/a/b/y.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli shift --by 1 --recursive --output-dir sortie films; find sortie -type f | sort -->
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

The output directory and those of the tree are **created** if they are missing. With
`--in-place`, each file kept is written back in its place. An extension that
`convert` changes changes in the tree too: `films/a/x.srt` converted
to WebVTT is written `sortie/a/x.vtt`.

**The output directory, if it is inside the tree walked, is excluded from the walk**:
a second run does not read again what the first one wrote.

## Two directories, and collisions

The rule of [the destination](invocation.md#the-destination) settles it: two files
that would end up at the same path are refused **before anything is written**, code `1`, the
two inputs and the destination named.

<!-- exemple: mkdir -p un/a deux/a; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > un/a/x.srt; cp un/a/x.srt deux/a/x.srt; subedit-cli shift --by 1 -r --output-dir sortie un deux; echo "code=$?" -->
```console
$ mkdir -p un/a deux/a; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > un/a/x.srt; cp un/a/x.srt deux/a/x.srt; subedit-cli shift --by 1 -r --output-dir sortie un deux; echo "code=$?"
sortie/a/x.srt: would be written by both un/a/x.srt and deux/a/x.srt
code=1
```

## What it costs

The batch stays sequential: its duration is the sum of those of its files. The
`make bench` benchmark measures the walk and the same work in a single file; the figures
are in the [performance log](../../mesures/performances.md).
