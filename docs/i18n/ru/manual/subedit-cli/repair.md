# `repair`

<!-- machine-translated -->
> Эта страница переведена машинным способом и ещё не проверена носителем языка.

```
subedit-cli repair (--output FILE | --output-dir DIR | --in-place)
                   [--minimum TIME] [--max-lines N] [--dashes]
                   [--dry-run] [--recursive]
                   <file>...
```

Делает за один проход то, что три команды делают одна за другой: [`join-repeats`](join-repeats.md), затем
[`sort`](sort.md), затем [`disjoint`](disjoint.md). **В этом порядке, который единственно работает**: разрезание читает
порядок начал, а повторяющиеся фрагменты одного текста лучше убрать до любого из двух. Для файла
с плохой сегментацией, неправильным порядком и перекрытиями это та команда, которую нужно запускать.

Настройки — те же, что у `disjoint`, которым цепочка заканчивается; параметра `--sort` здесь нет, поскольку сортировка выполняется всегда.

<!-- example: subedit-cli repair --help -->
```console
$ subedit-cli repair --help
Join the repeated subtitles, put them in order, then make the ranges disjoint
Usage: subedit-cli repair [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to repair

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --minimum TIME              Shortest interval kept; a shorter one is taken over by a neighbour (default 0.08)
  --max-lines N               Most lines of a merged subtitle; more are shown in windows, 0 for no limit (default 2)
  --dashes                    Put a dash before the text of each merged subtitle
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Аргументы и параметры

| Параметр | Обязательный | Значения | По умолчанию |
| :----- | :----- | :------ | :----- |
| `<file>...` | да | один или несколько путей | — |
| `--minimum` | нет | длительность: `0.08`, `1.5`, `00:00:00,080`. Нуль сохраняет все интервалы | `0.08` (около двух кадров) |
| `--max-lines` | нет | целое число; `0` снимает ограничение | `2` |
| `--dashes` | нет | флаг: тире перед текстом, который каждый субтитр вносит в объединённый субтитр | выключен |
| `--recursive`, `-r` | нет | флаг | выключен; см. [Обработка дерева каталогов](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **один из трёх**, кроме случая `--dry-run` | см. [Запуск](invocation.md#место-назначения) | — |
| `--dry-run` | нет | флаг: вычисляет и сообщает, ничего не записывает; см. [Просмотр перед записью](invocation.md#просмотр-перед-записью) | выключен |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:01,000 --> 00:00:04,000\nBeta\n\n' > segments.srt; subedit-cli repair --output repare.srt segments.srt; cat repare.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:01,000 --> 00:00:04,000\nBeta\n\n' > segments.srt; subedit-cli repair --output repare.srt segments.srt; cat repare.srt
segments.srt: 1 repeated subtitle joined; already in order; 0 subtitles split, 1 merged -> repare.srt
1
00:00:01,000 --> 00:00:04,000
Alpha
Beta
```

`Alpha`, разрезанный на два следующих друг за другом фрагмента, снова становится одним субтитром; затем он занимает тот же диапазон, что и `Beta`,
и оба сливаются в один — из двух строк, самая ранняя первой.

## Вывод

**Стандартный вывод** — пусто: результат — это записанный файл. В потоке ошибок на первом уровне выводится **фраза,
соединяющая три счётчика** через точки с запятой: то, что сообщают `join-repeats`, `sort` и `disjoint`,
каждая своими словами — `N repeated subtitles joined; M subtitles moved; K subtitles split, L merged…`.

### В формате JSON

С параметром [`--format json`](invocation.md#вывод-пригодный-для-чтения-скриптом) — `counts` каждого файла: `subtitles`, `joined`,
`moved`, затем счётчики [`disjoint`](disjoint.md#в-формате-json) (`split`, `merged`, `windowed`, `absorbed`, `too_short`,
`ending_before_start`, `extras_dropped`).

## Коды завершения и ошибки

Те же, что у всего инструмента. Собственных ошибок у `repair` нет, кроме связанных с настройками, которые оцениваются так же, как у `disjoint`:
`--minimum`, не являющийся длительностью или отрицательный, и `--max-lines`, не являющийся целым числом, — код `1`.
