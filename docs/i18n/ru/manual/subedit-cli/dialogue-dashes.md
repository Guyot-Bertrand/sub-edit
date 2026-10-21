# `dialogue-dashes`

<!-- machine-translated -->
> Эта страница переведена машинным способом и ещё не проверена носителем языка.

```
subedit-cli dialogue-dashes (--add | --remove) [--range N-M|N-]
                            (--output FILE | --output-dir DIR | --in-place)
                            [--dry-run]
                            [--recursive]
                            [--document main|translation] [-t FILE] [--align-method position|number]
                            <file>...
```

Ставит **тире в диалоге** в начале строк или убирает его. Это `Dialogue Dashes` в
[окне](../subedit-gui/operations.md).

**Два слова, как у [`italics`](italics.md)**: `--add` и `--remove` исключают друг друга, и одно из них обязательно.

- **`--add`** ставит тире перед каждой строкой, где его нет: `Bonjour` даёт `- Bonjour`, а
  `Bonjour\nMarie` даёт `- Bonjour\n- Marie`. **Уже стоящее тире не удваивается.**
- **Распознаются три тире** — дефис с клавиатуры и два длинных тире из типографики —
  **а записывается дефис**: `— Bonjour` становится `- Bonjour`. Лишние пробелы
  после тире исчезают.
- Тире ставится **перед текстом, а не перед открывающим тегом**: `<i>Bonjour</i>` даёт
  `<i>- Bonjour</i>`. Ни один тег не перемещается.
- **`--remove`** убирает начальные тире во всех трёх видах.

<!-- example: subedit-cli dialogue-dashes --help -->
```console
$ subedit-cli dialogue-dashes --help
Put dialogue dashes on the lines, or take them off
Usage: subedit-cli dialogue-dashes [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --add                       Put a dialogue dash at the head of the lines
  --remove                    Take the dialogue dashes off
  --range N-M|N-              Act only on subtitles N to M, or N to the end
  --document main|translation The document to change: the main one, or the translation given by -t
  -t,--translation-file FILE  Translation file to lay over the subtitle file, for a single input
  --align-method position|number
                              How the lines of the translation find their subtitles
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Аргументы и параметры

| Параметр | Обязательный | Значения | По умолчанию |
| :----- | :----- | :------ | :----- |
| `<file>...` | да | один или несколько путей | — |
| `--recursive`, `-r` | нет | флаг | выкл. — см. [Обработка дерева каталогов](lots.md) |
| `--add` / `--remove` | **один из двух** | флаги | — |
| `--range` | нет | `N-M` или `N-` до конца — см. [`adjust`](adjust.md#--range) | весь файл |
| `--document` | нет | `main` или `translation` — см. [Перевод](invocation.md#перевод) | `main` |
| `-t`, `--translation-file` | с `--document translation` | файл перевода, для **одного** входного файла | — |
| `--align-method` | нет, и только с `-t` | `position` или `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **один из трёх**, кроме случая с `--dry-run` | см. [Запуск](invocation.md#место-назначения) | — |
| `--dry-run` | нет | флаг: вычисляет и сообщает, ничего не записывает — см. [Просмотр перед записью](invocation.md#просмотр-перед-записью) | выкл. |

**По умолчанию основной текст; перевод — с `--document translation -t FILE`.** Тогда изменяется и записывается
именно перевод и только он — см. [Перевод](invocation.md#перевод).

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\nMarie\n\n2\n00:00:03,000 --> 00:00:04,000\n<i>Salut</i>\n\n' > a.srt; subedit-cli dialogue-dashes --add --output t.srt a.srt; cat t.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\nMarie\n\n2\n00:00:03,000 --> 00:00:04,000\n<i>Salut</i>\n\n' > a.srt; subedit-cli dialogue-dashes --add --output t.srt a.srt; cat t.srt
a.srt: 2 subtitles dashed -> t.srt
1
00:00:01,000 --> 00:00:02,000
- Bonjour
- Marie

2
00:00:03,000 --> 00:00:04,000
<i>- Salut</i>
```

## Вывод

**Стандартный вывод** — пусто: результат — записанный файл. С `--dry-run` — список
изменений, описанный в разделе [Просмотр перед записью](invocation.md#просмотр-перед-записью).

| Уровень | Что добавляется в поток ошибок |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles dashed -> <destination>` или `N subtitles undashed`, или `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — формат, кодировка, метка и символы конца строки прочитанного файла, возвращённые как были |
| 3 | `<path>: N bytes read, M written`, затем **все диагностические сообщения чтения** — см. [Запуск](invocation.md#чтение-диагностических-сообщений) |

`N` — число субтитров, **текст которых изменился**, оно берётся из самой операции.

### В формате JSON

С [`--format json`](invocation.md#вывод-пригодный-для-чтения-скриптом) объект каждого файла содержит
`counts.changed` — то самое `N` из фразы — и `changes`, список изменённых субтитров
(`{"subtitle", "document", "before", "after"}`, **пустой**, если ничего не изменилось).

## Коды завершения

Те же, что у программы: `0`, если обработан каждый файл — файл, в котором ничего не меняется, считается обработанным, —
`2`, если не обработан ни один, `3`, если обработаны только некоторые, `1` при ошибке использования.

## Ошибки

| Что их вызывает | Сообщение |
| :------------------ | :------ |
| нет ни `--add`, ни `--remove` | `dialogue-dashes needs --add to put dashes on, or --remove` |
| указаны оба | `--add and --remove say opposite things; give one of them` |
| неверно записанный диапазон или диапазон за пределами файла | см. [`adjust`](adjust.md#--range) |

Ошибки места назначения общие для подкоманд, которые записывают:
см. [Запуск](invocation.md#место-назначения).
