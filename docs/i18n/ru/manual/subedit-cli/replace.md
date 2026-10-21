# `replace`

<!-- machine-translated -->
> Эта страница переведена машинным способом и ещё не проверена носителем языка.

```
subedit-cli replace [--regex] [--case-sensitive] [--range N-M|N-]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    <pattern> <replacement> <file>...
```

Заменяет текст в субтитрах, **не ломая ни одного тега**. Это
операция `Replace All` из [поиска](../subedit-gui/find-and-replace.md) в окне, с теми же правилами:

- **поиск идёт по видимому тексту** — тому, что читает зритель. Теги в него не
  входят: поиск `<i>` ничего не находит, а поиск `Bonjour` находит «Bonjour», половина которого
  набрана курсивом;
- **замена выполняется в исходном тексте** файла, теги остаются там, где
  были. Тег, разрезающий искомое слово, охватывает всю замену; стиль,
  затрагивающий совпадение, покрывает всю замену; то, чего совпадение не касается,
  не сдвигается. Правила по случаям — те же, что у поиска в окне.

**Подкоманды `find` нет**: чтобы увидеть, где находится шаблон, ничего не меняя,
нужен [`--dry-run`](invocation.md#просмотр-перед-записью), который перечисляет каждый затронутый субтитр.

<!-- example: subedit-cli replace --help -->
```console
$ subedit-cli replace --help
Replace a text in the subtitles, without breaking a tag
Usage: subedit-cli replace [OPTIONS] pattern replacement files...

Positionals:
  pattern TEXT REQUIRED       Text to look for in what is shown
  replacement TEXT REQUIRED   What replaces it, tags allowed
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --regex                     Read the text to look for as a regular expression
  --case-sensitive            Tell capitals from small letters
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
| `<pattern>` | да | текст, который нужно найти в показываемом; **не пустой** | — |
| `<replacement>` | да | то, чем он заменяется, теги допускаются; может быть пустым | — |
| `<file>...` | да | один или несколько путей | — |
| `--recursive`, `-r` | нет | флаг | выключен; см. [Обработка дерева каталогов](lots.md) |
| `--regex` | нет | флаг: шаблон — регулярное выражение ICU | простой текст |
| `--case-sensitive` | нет | флаг: различает заглавные и строчные буквы | не различает |
| `--range` | нет | `N-M` или `N-` до конца; см. [`adjust`](adjust.md#--range) | весь файл |
| `--document` | нет | `main` или `translation`; см. [Перевод](invocation.md#перевод) | `main` |
| `-t`, `--translation-file` | с `--document translation` | файл перевода, для **одного** входного файла | — |
| `--align-method` | нет, и только с `-t` | `position` или `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **один из трёх**, кроме случая `--dry-run` | см. [Запуск](invocation.md#место-назначения) | — |
| `--dry-run` | нет | флаг: вычисляет и сообщает, ничего не записывает; см. [Просмотр перед записью](invocation.md#просмотр-перед-записью) | выключен |

**Шаблон, начинающийся с дефиса**, записывается после `--`: `subedit-cli replace -- -- "—" film.srt`.

**По умолчанию — основной текст; перевод — с параметрами `--document translation -t FILE`.** Тогда
изменяется и записывается перевод, и только он; см. [Перевод](invocation.md#перевод).

### Простой текст или выражение

По умолчанию шаблон **читается как есть**: `a.b` ищет `a`, точку, `b`. Регистр
игнорируется с той же тонкостью, что и в окне: `É` и `é` — это одна буква.

С параметром `--regex` шаблон — регулярное выражение **ICU**, читаемое так, как его читает Gaupol: `.` пересекает
перевод строки, а `^` и `$` действуют на каждой строке субтитра. В замене, и
только в ней:

| Запись | Означает |
| :----- | :-------- |
| `$1` … `$9` | группы совпадения |
| `$0` | всё совпадение |
| `\n` | перевод строки |
| `\$`, `\\` | знак `$`, обратная косая черта |

Следите за кавычками оболочки: `'$1 EUR'`, а не `"$1 EUR"`, иначе оболочка прочтёт `$1` сама.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nPrix: 12 euros, et 7 euros.\n\n' > prix.srt; subedit-cli replace --regex '([0-9]+) euros' '$1 EUR' --output euros.srt prix.srt; cat euros.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nPrix: 12 euros, et 7 euros.\n\n' > prix.srt; subedit-cli replace --regex '([0-9]+) euros' '$1 EUR' --output euros.srt prix.srt; cat euros.srt
prix.srt: replaced 2 matches -> euros.srt
1
00:00:01,000 --> 00:00:02,000
Prix: 12 EUR, et 7 EUR.
```

## Теги

Текст, в котором тег разрезает искомое слово:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace Bonjour Salut --output salut.srt balises.srt; cat salut.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace Bonjour Salut --output salut.srt balises.srt; cat salut.srt
balises.srt: replaced 2 matches -> salut.srt
1
00:00:01,000 --> 00:00:02,000
<i>Salut</i> tout le monde

2
00:00:03,000 --> 00:00:04,000
Salut <i>Marie</i>, ca va
```

В первом субтитре тег разрезал «Bonjour»: теперь он охватывает «Salut».
Во втором тег не затронут и остаётся на месте. **Оставленный открытым тег
остаётся открытым**: исправление неряшливой разметки было бы другой операцией.

Замена читается **в тегах формата файла**: `<i>…</i>` для SubRip,
`{\i1}…` для Advanced SSA. Это единственный способ отформатировать то, что вы пишете.

## Ничего не найдено, нечего менять

Два ответа, которые не являются сбоями: код завершения `0`, и **файл всё равно
записывается**, ведь заданное место назначения — это записанное место назначения:

| Ситуация | Сообщение |
| :-------- | :----- |
| шаблона нет нигде | `"zzz" not found` |
| он есть, но каждое совпадение заменяется само собой | `nothing to change` |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli replace zzz x --output b.srt a.srt; subedit-cli replace Bonjour Bonjour --output c.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli replace zzz x --output b.srt a.srt; subedit-cli replace Bonjour Bonjour --output c.srt a.srt
a.srt: "zzz" not found -> b.srt
a.srt: nothing to change -> c.srt
```

## `--dry-run`

Стандартный вывод перечисляет **каждый субтитр, текст которого изменился бы**, до и после, в том виде,
в каком они стоят в файле, вместе с тегами:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace --dry-run Bonjour Salut balises.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace --dry-run Bonjour Salut balises.srt
balises.srt: replaced 2 matches (dry run, nothing written)
balises.srt: subtitle 1
- <i>Bon</i>jour tout le monde
+ <i>Salut</i> tout le monde
balises.srt: subtitle 2
- Bonjour <i>Marie</i>, ca va
+ Salut <i>Marie</i>, ca va
```

Вид списка описан в разделе [Просмотр перед записью](invocation.md#просмотр-перед-записью);
в JSON это `changes`.

## Вывод

**Стандартный вывод** — пусто: результат — это записанный файл. С параметром `--dry-run` — приведённый выше список
изменений.

| Уровень | Что добавляется в поток ошибок |
| :----- | :------------------------------------- |
| 1 | `<path>: replaced N matches -> <destination>`, или `"<pattern>" not found`, или `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — формат, кодировка, метка и символы конца строки прочитанного файла, возвращённые как были |
| 3 | `<path>: N bytes read, M written`, затем **все диагностические сообщения чтения**; см. [Запуск](invocation.md#чтение-диагностических-сообщений) |

`N` считает **совпадения, заменённые в субтитрах, чей текст изменился**; это то же
число, что сообщает и окно.

### В формате JSON

С параметром [`--format json`](invocation.md#вывод-пригодный-для-чтения-скриптом) объект каждого файла содержит:

| Ключ | Содержимое |
| :-- | :------ |
| `counts.replaced` | совпадения, заменённые в субтитрах, чей текст изменился: `N` из фразы |
| `counts.matched` | найденные совпадения, **изменила ли их замена текст или нет**; именно это отличает «ничего не найдено» (`0`) от «нечего менять» (`replaced` равно `0`, `matched` положительно) |
| `changes` | изменённые субтитры: `{"subtitle", "document", "before", "after"}`, **пусто**, когда ничего не изменилось |

## Коды завершения

Те же, что у всего инструмента: `0`, если обработан каждый файл (файл, в котором ничего не найдено,
тоже считается обработанным), `2`, если не обработан ни один, `3`, если обработаны только некоторые, `1` при ошибке использования.

## Ошибки

| Что её вызывает | Сообщение |
| :------------------ | :------ |
| пустой шаблон | `pattern: nothing to look for` |
| нечитаемое выражение | `pattern: not a regular expression (<ICU's reason>)` |
| диапазон записан неверно или выходит за файл | см. [`adjust`](adjust.md#--range) |

**Шаблон читается до первого файла**: нечитаемое выражение — это ошибка использования
(код `1`), ничего не читается и не записывается.

Ошибки места назначения общие для подкоманд, которые записывают:
см. [Запуск](invocation.md#место-назначения).
