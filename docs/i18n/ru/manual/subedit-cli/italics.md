# `italics`

<!-- machine-translated -->
> Эта страница переведена машинным способом и ещё не проверена носителем языка.

```
subedit-cli italics (--on | --off) [--range N-M|N-]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    <file>...
```

Выделяет тексты **курсивом** или **убирает с них курсив** с помощью тегов формата файла.
Это `Italic` из [окна](../subedit-gui/operations.md).

**Два слова, а не переключатель**: кнопка в окне спрашивает у ядра, в какую сторону
менять; здесь это указывают явно. `--on` и `--off` исключают друг друга, и нужен один из них: если не указать ни одного или указать оба,
это ошибка использования (код `1`).

- **`--on`** заключает каждый текст в одну пару тегов, **предварительно удалив те, что
  в нём были**: текст, уже набранный курсивом, не оборачивается дважды, а курсив, разорванный на две
  части, собирается заново в одну. Для нескольких строк одна пара тегов охватывает весь текст.
- **`--off`** удаляет теги курсива; **остальное содержимое тега сохраняется** (`{\b1\i1}` превращается в
  `{\b1}`).
- Текст записывается **так, как формат файла записывает курсив**: `<i>…</i>` для SubRip, WebVTT и
  SubViewer 2, `{\i1}…{\i0}` для двух форматов Sub Station Alpha, `{Y:i}` для MicroDVD.

<!-- example: subedit-cli italics --help -->
```console
$ subedit-cli italics --help
Put the texts in italics, or take their italics out
Usage: subedit-cli italics [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --on                        Put the texts in italics
  --off                       Take the italics out
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
| `--recursive`, `-r` | нет | флаг | выключен — см. [Обработка дерева каталогов](lots.md) |
| `--on` / `--off` | **один из двух** | флаги | — |
| `--range` | нет | `N-M` или `N-` до конца — см. [`adjust`](adjust.md#--range) | весь файл |
| `--document` | нет | `main` или `translation` — см. [Перевод](invocation.md#перевод) | `main` |
| `-t`, `--translation-file` | при `--document translation` | файл перевода, для **одного** входного файла | — |
| `--align-method` | нет, и только вместе с `-t` | `position` или `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **один из трёх**, кроме случая `--dry-run` | см. [Запуск](invocation.md#место-назначения) | — |
| `--dry-run` | нет | флаг: вычисляет и сообщает, ничего не записывает — см. [Просмотр перед записью](invocation.md#просмотр-перед-записью) | выключен |

**По умолчанию изменяется основной текст; перевод — с `--document translation -t FILE`.** Тогда
изменяется и записывается перевод, и только он — см. [Перевод](invocation.md#перевод).

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour\nMarie\n\n' > a.srt; subedit-cli italics --on --output i.srt a.srt; cat i.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour\nMarie\n\n' > a.srt; subedit-cli italics --on --output i.srt a.srt; cat i.srt
a.srt: 2 subtitles put in italics -> i.srt
1
00:00:01,000 --> 00:00:02,000
<i>bonjour marie</i>

2
00:00:03,000 --> 00:00:04,000
<i>Bonjour
Marie</i>
```

## Формат без стиля вызывает отказ

**TMPlayer и LRC не записывают стиль**: курсив нельзя ни добавить, ни убрать. Окно
делает этот пункт серым, а не скрывает его; командная строка **отказывает по каждому
файлу отдельно, с указанием причины**, а остальные файлы пакета обрабатываются дальше. Важно то, что
формат способен сохранить, а не то, как он это записывает: SubRip и WebVTT записывают курсив одинаково,
но по-разному обходятся с цветом.

<!-- example: printf '00:00:01:Hello there\n00:00:03:Bye\n' > a.txt; subedit-cli italics --on --output-dir sortie a.txt; echo "code=$?" -->
```console
$ printf '00:00:01:Hello there\n00:00:03:Bye\n' > a.txt; subedit-cli italics --on --output-dir sortie a.txt; echo "code=$?"
a.txt: TMPlayer writes no style: there are no italics to put on
code=2
```

В формате JSON отказ несёт идентификатор **`no-style`**.

## Вывод

**Стандартный вывод** — пуст: результат — это записанный файл. С `--dry-run` выводится список
изменений, описанный в разделе [Просмотр перед записью](invocation.md#просмотр-перед-записью).

| Уровень | Что добавляется в поток ошибок |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles put in italics -> <destination>`, или `N subtitles taken out of italics`, или `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — формат, кодировка, метка и символы конца строки прочитанного файла, возвращённые в прежнем виде |
| 3 | `<path>: N bytes read, M written`, затем **каждое диагностическое сообщение чтения** — см. [Запуск](invocation.md#чтение-диагностических-сообщений) |

`N` — число субтитров, **текст которых изменился**; оно берётся из самой операции.

### В формате JSON

С [`--format json`](invocation.md#вывод-пригодный-для-чтения-скриптом) объект каждого файла содержит
`counts.changed` (то самое `N` из предложения) и `changes` — список изменённых субтитров
(`{"subtitle", "document", "before", "after"}`, **пустой**, если ничего не изменилось).

## Коды завершения

Коды самой программы: `0`, если обработаны все файлы (файл, в котором ничего не меняется, считается обработанным),
`2`, если не обработан ни один, `3`, если обработаны лишь некоторые, `1` при ошибке использования.

## Ошибки

| Что её вызывает | Сообщение |
| :------------------ | :------ |
| не указан ни `--on`, ни `--off` | `italics needs --on to put the texts in italics, or --off` |
| указаны оба | `--on and --off say opposite things; give one of them` |
| формат без стиля | `<path>: TMPlayer writes no style: there are no italics to put on` (или `take out`) |
| диапазон записан неверно или выходит за пределы файла | см. [`adjust`](adjust.md#--range) |

Ошибки места назначения общие для подкоманд, которые записывают:
см. [Запуск](invocation.md#место-назначения).
