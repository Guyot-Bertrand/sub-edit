# Обработка дерева каталогов

<!-- machine-translated -->
> Эта страница переведена машинным способом и ещё не проверена носителем языка.

```
subedit-cli <subcommand> --recursive [options] <directory>...
```

Каждая подкоманда, читающая пакет файлов (включая `inspect`), принимает **каталог**
вместо файла вместе с параметром `--recursive` (`-r`). Исключение составляют
[`append`](append.md), [`split-file`](split-file.md) и [`pair`](pair.md), чья арность — это
пара файлов или фильм.
На этой странице описано, что берёт обход, в каком порядке и куда он записывает результат.

**Ничто из описанного ниже не меняет того, что делается с файлом**: каждый файл читается,
обрабатывается и записывается независимо, сбой одного не останавливает остальные, а
четыре кода завершения — те же, что и в [запуске](invocation.md#коды-завершения).
Обход создаёт список путей, который остальная часть пакета использует так, как если бы
их ввели вручную.

## Без `--recursive` каталог отклоняется

Ошибка использования, код `1`; ни один файл не обрабатывается и не записывается:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?" -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?"
films: is a directory: use --recursive
code=1
```

## Что берёт обход

Обход **рекурсивный, детерминированный и без неожиданностей**:

- **порядок** — записи каждого каталога берутся в порядке их
  имён, **байт за байтом** (заглавные буквы перед строчными, независимо от языка
  машины), а подкаталог обходится на своём месте, вглубь.
  Два запуска дают одни и те же байты на любой
  машине;
- **символические ссылки не разворачиваются** — ни на файл, ни на
  каталог: у дерева, содержащего ссылку на один из своих родительских каталогов, не было бы
  конца;
- **скрытые файлы и каталоги игнорируются** — те, чьё имя начинается с
  точки;
- **сохраняются только расширения известных форматов**: `.srt`, `.vtt`,
  `.ssa`, `.ass`, `.lrc`, `.sub` без учёта регистра. **`.txt` исключено**: оно
  обозначает два формата, но прежде всего каждый `LISEZMOI.txt` на свете, а пакет, который
  завершался бы сбоем на каждом из них, был бы непригоден.

**Файл, названный в командной строке, никогда не фильтруется.** Назвать его —
значит хотеть его обработать: `subedit-cli shift --by 1 --in-place notes.txt` обработает его независимо от
расширения и пожалуется, только если его содержимое не соответствует ни одному формату.

**Всё, что оставлено в стороне, подсчитывается** и сообщается на уровне `-vv`:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli -vv inspect --recursive films 2>&1 | head -1 -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli -vv inspect --recursive films 2>&1 | head -1
films: 1 file left aside, with no extension of a known format
```

Каталог, в котором нечего обходить, сообщает об этом на уровне по умолчанию
(`<directory>: holds no file in a format this tool walks for`) и ошибкой не считается.

## Куда обход записывает результат

**Дерево сохраняется в каталоге `--output-dir`**: путь файла
**относительно заданного каталога** воспроизводится внутри выходного каталога. Имя
заданного каталога в него не входит — такова семантика `rsync` с
завершающей косой чертой, а не `cp -r`: вы указываете место назначения и сами добавляете к нему
нужный уровень (`--output-dir sortie/films`).

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

Выходной каталог и каталоги дерева **создаются**, если их нет. С параметром
`--in-place` каждый сохранённый файл записывается обратно на своё место. Расширение, которое
меняет `convert`, меняется и в дереве: `films/a/x.srt`, преобразованный
в WebVTT, записывается как `sortie/a/x.vtt`.

**Выходной каталог, если он находится внутри обходимого дерева, исключается из обхода**:
повторный запуск не читает заново то, что записал первый.

## Два каталога и совпадения путей

Это решает правило [места назначения](invocation.md#место-назначения): два файла,
которые оказались бы по одному и тому же пути, отклоняются **до начала любой записи**, код `1`; в сообщении
названы оба входных файла и место назначения.

<!-- example: mkdir -p un/a deux/a; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > un/a/x.srt; cp un/a/x.srt deux/a/x.srt; subedit-cli shift --by 1 -r --output-dir sortie un deux; echo "code=$?" -->
```console
$ mkdir -p un/a deux/a; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > un/a/x.srt; cp un/a/x.srt deux/a/x.srt; subedit-cli shift --by 1 -r --output-dir sortie un deux; echo "code=$?"
sortie/a/x.srt: would be written by both un/a/x.srt and deux/a/x.srt
code=1
```

## Во что это обходится

Пакет остаётся последовательным: его длительность равна сумме длительностей его файлов.
Эталонный тест `make bench` измеряет обход и ту же работу над одним файлом; цифры
приведены в [журнале производительности](../../../../mesures/performances.md).
