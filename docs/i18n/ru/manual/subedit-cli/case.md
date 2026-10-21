# `case`

<!-- machine-translated -->
> Эта страница переведена машинным способом и ещё не проверена носителем языка.

```
subedit-cli case --to title|sentence|upper|lower [--range N-M|N-]
                 (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 [--document main|translation] [-t FILE] [--align-method position|number]
                 <file>...
```

Приводит тексты к виду **«каждое слово с заглавной», «как в предложении», заглавными или строчными буквами**, не затрагивая ни
одного тега. Это `Сервис ▸ Регистр` в [окне](../subedit-gui/operations.md) с теми же правилами.

| `--to` | Действие | `bonjour marie` превращается в |
| :----- | :---- | :---------------------- |
| `title` | каждое слово получает заглавную букву, остальные буквы становятся строчными | `Bonjour Marie` |
| `sentence` | только первая буква текста, и больше никакая; остальные становятся строчными | `Bonjour marie` |
| `upper` | всё заглавными | `BONJOUR MARIE` |
| `lower` | всё строчными | `bonjour marie` |

- **То, что стоит перед первой буквой или первой цифрой, не затрагивается**: тире
  в диалоге, открывающая кавычка или скобка сохраняются (`- bonjour` даёт `- Bonjour`).
- **Теги — не текст**: они не меняют регистр — `<I>` остаётся `<I>` — и не
  меняют место: тег, который разрезает слово, продолжает его разрезать и после этого.
- **Где начинается слово, решает ICU**: `l'été sera chaud` даёт `L'été Sera Chaud`, а диакритические
  знаки сохраняются (`éléonore` даёт `Éléonore`).

<!-- example: subedit-cli case --help -->
```console
$ subedit-cli case --help
Put the texts in title, sentence, upper or lower case, tags intact
Usage: subedit-cli case [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --to title|sentence|upper|lower
                              The case to put the texts in
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
| `--to` | **да** | `title`, `sentence`, `upper`, `lower` | — |
| `--range` | нет | `N-M` или `N-` до конца — см. [`adjust`](adjust.md#--range) | весь файл |
| `--document` | нет | `main` или `translation` — см. [Перевод](invocation.md#перевод) | `main` |
| `-t`, `--translation-file` | с `--document translation` | файл перевода, для **одного** входного файла | — |
| `--align-method` | нет, и только с `-t` | `position` или `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **один из трёх**, кроме случая с `--dry-run` | см. [Запуск](invocation.md#место-назначения) | — |
| `--dry-run` | нет | флаг: вычисляет и сообщает, ничего не записывает — см. [Просмотр перед записью](invocation.md#просмотр-перед-записью) | выкл. |

**По умолчанию основной текст; перевод — с `--document translation -t FILE`.** Тогда изменяется и записывается
именно перевод и только он — см. [Перевод](invocation.md#перевод).

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\n- BONJOUR MARIE\n\n' > a.srt; subedit-cli case --to title --output titre.srt a.srt; cat titre.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\n- BONJOUR MARIE\n\n' > a.srt; subedit-cli case --to title --output titre.srt a.srt; cat titre.srt
a.srt: 2 subtitles recased -> titre.srt
1
00:00:01,000 --> 00:00:02,000
<i>Bon</i>jour Marie

2
00:00:03,000 --> 00:00:04,000
- Bonjour Marie
```

Файл, где **ни один текст не изменился** — всё уже в нужном регистре, — записывается как есть и
сообщает об этом:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBONJOUR\n\n' > a.srt; subedit-cli case --to upper --output b.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBONJOUR\n\n' > a.srt; subedit-cli case --to upper --output b.srt a.srt
a.srt: nothing to change -> b.srt
```

## Вывод

**Стандартный вывод** — пусто: результат — записанный файл. С `--dry-run` — список
изменений, описанный в разделе [Просмотр перед записью](invocation.md#просмотр-перед-записью).

| Уровень | Что добавляется в поток ошибок |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles recased -> <destination>` или `nothing to change` |
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
| нет `--to` | `--to is required` |
| значение, не входящее в эти четыре | `--to: … not in {title,sentence,upper,lower}` |
| неверно записанный диапазон или диапазон за пределами файла | см. [`adjust`](adjust.md#--range) |

Ошибки места назначения общие для подкоманд, которые записывают:
см. [Запуск](invocation.md#место-назначения).
