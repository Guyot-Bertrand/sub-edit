# `correct`

<!-- machine-translated -->
> Эта страница переведена машинным способом и ещё не проверена носителем языка.

```
subedit-cli correct --tasks LIST --code CODE
                    [--classes human|ocr|human,ocr]
                    [--enable NAME]... [--disable NAME]...
                    [--language CODE]
                    [--max-length N] [--max-lines N]
                    [--skip-length N|off] [--skip-lines N|off]
                    [--keep-blank-subtitles | --remove-blank-subtitles]
                    [--range N-M|N-]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    <file>...
```

Исправляет тексты по **шаблонам Gaupol**, как мастер
[`Correct Texts…`](../subedit-gui/correct-texts.md) в окне: те же шаблоны,
тот же движок, те же счётчики. Здесь есть шесть задач: **пометки для
слабослышащих**, **объединение** и **разделение слов**, **типичные ошибки**,
**заглавные буквы** и **разбиение на строки**.

<!-- example: subedit-cli correct --help -->
```console
$ subedit-cli correct --help
Correct the texts with the patterns of the correction assistant
Usage: subedit-cli correct [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to correct

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --tasks LIST                The tasks to run, in a list: none is chosen beforehand
  --code CODE                 The cascade of patterns the tasks read: Zyyy, Latn, Latn-en, Latn-en-US
  --classes human|ocr|human,ocr
                              The classes of common errors to apply: both by default
  --enable NAME               Switch a pattern on, by its English name or type:name; repeatable
  --disable NAME              Switch a pattern off, by its English name or type:name; repeatable
  --language CODE             The dictionary join-words and split-words read: fr_FR, en_US, en...
  --max-length N              Longest line of line-break, in characters; no default, Gaupol's is in ems
  --max-lines N               Most lines of line-break: 3, as Gaupol's
  --skip-length N|off         Leave alone a subtitle whose longest line is within this, or off: --max-length
  --skip-lines N|off          Leave alone a subtitle whose line count is within this, or off: --max-lines
  --keep-blank-subtitles      Leave the subtitles the correction empties, empty, instead of removing them
  --remove-blank-subtitles    Remove the subtitles the correction empties, whatever the settings file says
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

## Что выбирается, а что нет

**Заранее ничего не отмечено.** Окно открывается с полями в том виде, в каком их оставил пользователь; командная строка
берёт задачи, код, длину строки и язык из собственной командной строки,
а не из файла настроек: ни задачи, когда-то отмеченные в окне, ни поля шаблонов,
переключённые там (`correction.activations`), ни список замен проверки орфографии
не читаются, и с этой стороны ничего не записывается.

**Четыре настройки берутся из файла настроек по умолчанию** — значения, которые запоминает `Correct Texts…`
или которые записала подкоманда [`config set`](config.md). **Параметр в командной строке всегда побеждает**, затем
файл, затем встроенное значение по умолчанию; о том, где лежит файл, а также о `--config` и `--no-config` см.
[Файл настроек](invocation.md#файл-настроек).

| Ключ | Что он задаёт | Параметр, который его переопределяет |
| :-- | :--------------- | :----------------------- |
| `correction.human`, `correction.ocr` | применяемые классы типичных ошибок | `--classes` |
| `correction.remove-blank` | удаляются ли субтитры, опустошённые исправлением | `--keep-blank-subtitles`, `--remove-blank-subtitles` |
| `correction.line-break.max-lines` | наибольшее число строк `line-break` | `--max-lines` |

- **У каждого из четырёх есть параметр, который его отменяет**, что бы ни говорил файл: `--classes human,ocr`
  применяет оба класса, `--remove-blank-subtitles` удаляет опустошённые субтитры, `--max-lines 3`
  возвращает строки Gaupol. `--keep-blank-subtitles` и `--remove-blank-subtitles` вместе — это
  ошибка использования.
- **Оба класса, отключённые в файле, игнорируются, и об этом сообщается** на уровне 1 —
  `settings: correction.human and correction.ocr are both false, ignored` —: `--classes` не смог бы этого записать, и
  common-errors было бы нечего делать. Вместо этого применяются оба класса.
- **Число строк вне диапазона от 1 до 1000 из файла не читается вовсе**: читатель файла
  его отклоняет — `settings: correction.line-break.max-lines: cannot read 'N', default kept`, на
  уровне 1, — и используется встроенное значение 3.

Примеры на этой странице выполнены с `--no-config`, чтобы они показывали, что делают встроенные
значения по умолчанию, независимо от настроек машины, на которой они запускаются.

| Параметр | Обязательный | Значения | По умолчанию |
| :----- | :----- | :------ | :----- |
| `<file>...` | да | один или несколько путей | — |
| `--tasks` | **да** | список через запятую: `mentions`, `join-words`, `split-words`, `common-errors`, `capitalization`, `line-break` | ни одной задачи |
| `--code` | **да** при задаче с шаблонами и **неприменим** в остальных случаях | `Script[-language[-COUNTRY]]`: `Zyyy`, `Latn`, `Latn-en`, `Latn-en-US` | — |
| `--language` | **да** при `join-words` или `split-words` и **неприменим** в остальных случаях | код языка: `fr`, `fr_FR`, `en_US` | — |
| `--classes` | нет | `human`, `ocr` или `human,ocr` | оба — или то, что говорит файл настроек |
| `--enable`, `--disable` | нет, повторяемые | английское имя шаблона или `type:name` | поставляемые файлы `.conf` |
| `--max-length` | **да** при `line-break` | число **символов** больше нуля | **нет** — см. ниже |
| `--max-lines` | нет | целое число строк больше нуля | 3 — или то, что говорит файл настроек |
| `--skip-length`, `--skip-lines` | нет | число или `off` | те же границы, что у `--max-length` и `--max-lines` |
| `--keep-blank-subtitles` | нет | флаг: опустошённые субтитры остаются пустыми | опустошённые субтитры удаляются — если файл настроек не говорит иного |
| `--remove-blank-subtitles` | нет | флаг: опустошённые субтитры удаляются, что бы ни говорил файл настроек; отклоняется вместе с `--keep-blank-subtitles` | выкл. |
| `--range` | нет | `N-M` или `N-` до конца — см. [`adjust`](adjust.md#--range) | весь файл |
| `--document` | нет | `main` или `translation` — см. [Перевод](invocation.md#перевод) | `main` |
| `-t`, `--translation-file` | с `--document translation` | файл перевода, для **одного** входного файла | — |
| `--align-method` | нет, и только с `-t` | `position` или `number` | `position` |
| `--recursive`, `-r` | нет | флаг | выкл. — см. [Обработка дерева каталогов](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **один из трёх**, кроме случая с `--dry-run` | см. [Запуск](invocation.md#место-назначения) | — |
| `--dry-run` | нет | флаг: вычисляет и сообщает, ничего не записывает — см. [Просмотр перед записью](invocation.md#просмотр-перед-записью) | выкл. |

### Задачи

**Порядок в списке не имеет значения**: задачи применяются в
порядке Gaupol — пометки, затем объединение и разделение слов, затем типичные ошибки, затем заглавные буквы, затем разбиение на строки, — и каждая работает с
текстом, который оставила предыдущая. Субтитр, опустошённый пометками, не
участвует в последующем.

| Задача | Что она делает |
| :---- | :-------------- |
| `mentions` | удаляет звуки в квадратных скобках, а при включённом `Sound in parentheses` — и в круглых (обход, который выполняет [`hearing-impaired`](hearing-impaired.md)), затем шаблоны текстов песен и говорящих |
| `join-words` | склеивает обратно слово, разрезанное надвое, по словарю языка `--language` |
| `split-words` | разделяет два слитно написанных слова по тому же словарю |
| `common-errors` | исправляет типичные ошибки: пробелы, пунктуацию, лигатуры, ошибки распознавания |
| `capitalization` | ставит заглавную букву там, где её требует язык |
| `line-break` | заново разрезает строки субтитра так, чтобы ни одна не превышала `--max-length`, взвешивая шаблоны разрыва из кода |

### Код шаблонов

**`--code` называет каскад**: `Zyyy` для любой письменности, затем письменность, язык
и страна — `Latn-en-US` читает шаблоны `Zyyy`, `Latn`, `Latn-en` и
`Latn-en-US`. **Он обязателен**, потому что у командной строки нет окружающего языка:
универсальное значение по умолчанию применяло бы *меньше*, чем полагают, и не сообщало бы об этом. Он действует для
каждой задачи одного и того же запуска; задача, которой нужен собственный код, — это второй
запуск. Неверно записанный код — это ошибка использования:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code francais --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code francais --output b.srt a.srt; echo "code=$?"
--code: "francais" is not a pattern code: expected Script[-language[-COUNTRY]], like Zyyy, Latn or Latn-en-US
code=1
```

Шаблоны, которые пользователь кладёт в `$XDG_DATA_HOME/subedit/patterns` (или
`~/.local/share/subedit/patterns`), **добавляются к поставляемым шаблонам**, как в
окне, а поставляемый шаблон, который не удаётся прочитать, упоминается на уровне 1: `patterns: <file>, line N (<reason>)`.

### Объединение и разделение слов и словарь

**Эти две задачи не читают шаблонов**: они обращаются к **словарю системы через
Enchant** — зависимости пакетов (`libenchant-2-2` в Debian и Ubuntu, `enchant2` в
Fedora), общей для `subedit-cli` и окна. Поэтому у них нет ни кода, ни имени, которые нужно задавать:
`--code` не имеет смысла без задачи с шаблонами, а `--language` — без одной из этих двух,
— указание одного без другого является ошибкой использования, код `1`.

- **`join-words`** приклеивает слово, которого нет в словаре, к предыдущему или
  следующему, **если лишь одно направление даёт известное слово**: `hel lo` становится `hello`; два возможных
  направления или ни одного ничего не решают.
- **`split-words`** разделяет слово, которого нет в словаре, **если ровно одно из его
  предложений — это слово с пробелом**: `hellothere` становится `hello there`. Слово с начальной
  заглавной буквой и строчными следом остаётся нетронутым: чаще всего это имя.

**Без словаря для языка запуск отклоняется**, код `1`, **до чтения какого-либо файла** и
словами окна, которое делает функцию недоступной и сообщает об этом, — здесь командная строка
отказывает и сообщает, а не молчит:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nhel lo\n\n' > a.srt; subedit-cli --no-config correct --tasks join-words --language qq_QQ --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nhel lo\n\n' > a.srt; subedit-cli --no-config correct --tasks join-words --language qq_QQ --output b.srt a.srt; echo "code=$?"
no dictionary for qq_QQ
code=1
```

**Список замен окна не читается и не записывается.** Он *пополняется* выбором
пользователя при проверке орфографии; у командной строки такого
выбора нет, а чтение списка поставило бы скрипт в зависимость от того, что когда-то в него положил его автор.

### Разбиение на строки

**Оно измеряется в символах, и этим отличается от окна.** Окно измеряет в *em* —
ширине, откалиброванной по шрифту, по 0,55 em на букву, — и его значение по умолчанию 24 — это значение
в em. У командной строки нет шрифта: она считает символы текста.
**Перенос 24 как числа символов обрезал бы почти всё без
предупреждения**; поэтому **длины по умолчанию нет**, а `--max-length` обязателен при
`line-break`:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn texte\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn texte\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --output b.srt a.srt; echo "code=$?"
--max-length is required by the task line-break: Gaupol's 24 is a width in ems, and has no value in characters
code=1
```

**`--max-lines` равен 3**, значению Gaupol, которое единица измерения не меняет; разбиение на строки берёт
ещё одну, если трёх не хватает. **Пропуск** — субтитр, уже укладывающийся в пределы,
остаётся как есть — по умолчанию использует **те же границы** (`--skip-length` = `--max-length`,
`--skip-lines` = `--max-lines`), как у Gaupol; `--skip-length off` и `--skip-lines off`
отключают его, и тогда текст читается снова как один и режется заново:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nWhen the rain stopped, we walked home under the dark sky of the old town.\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --max-length 24 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nWhen the rain stopped, we walked home under the dark sky of the old town.\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --max-length 24 --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:03,000
When the rain
stopped, we walked
home under the dark
sky of the old town.
```

Шаблоны разрыва — из кода: `Latn` не разрывает между числом и его единицей измерения и
после тире в диалоге, `Latn-en` не разрывает после артикля, предлога, звания
или притяжательного определения. **Под `Zyyy` шаблонов разрыва нет**: строки
только выравниваются. **Эта задача никогда не отклоняется из-за отсутствия действующего шаблона**: её шаблоны
определяют, где разорвать, а разрыв происходит и без них. Эти четыре параметра имеют смысл только с
`line-break`; указанные без него, они являются ошибкой использования.

### Классы и переключение по имени

**`--classes` окончательно фильтрует типичные ошибки**: снятие `ocr` выводит его
шаблоны из применения — лигатура `ﬁ` тогда остаётся `ﬁ`. Остальные три типа шаблонов
не имеют класса.

**`--enable` и `--disable` переключают шаблон по его английскому имени**, а ключом служит
имя (ADR 0037); одно имя может обозначать несколько записей, которые тогда делят одно
поле. Это необходимо и это отменяет значения по умолчанию окна: **поставляемые
файлы `.conf` отключают шаблоны пометок**, так что одна задача `--tasks mentions`
ничего бы не применила. Два шаблона «Sound in brackets» и «Sound in parentheses»
управляют каждый своим ограничителем — квадратными скобками, круглыми скобками — и переключаются теми же
параметрами: включение одного не удаляет другой.

Три случая — **ошибки использования**, код `1`, о которых сообщается до чтения любого файла:

| Что их вызывает | Сообщение |
| :------------------ | :------ |
| имя, которое не обозначает ни одного шаблона заданных задач при заданном коде | `--enable: "<name>" names no pattern of the tasks given, under the code <code>` |
| имя, обозначающее шаблоны двух типов, — тогда оно записывается как `type:name`, где тип — `common-error`, `capitalization`, `hearing-impaired` или `line-break` | `--enable: "<name>" names patterns of several types: write type:name, …` |
| имя, указанное и в `--enable`, и в `--disable` | `"<name>" is given to --enable and to --disable: choose one` |

**Опечатка заметна сразу**: окно игнорирует устаревшую настройку, потому что
настройка может пережить исчезнувший шаблон; у вызова программы переживших нет.

**Задача без единого действующего шаблона отклоняется**, код `1` — «делать нечего» — это
ответ, который озвучивается, а не молчаливый успех:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --output b.srt a.srt; echo "code=$?"
mentions: no pattern is active under the code Latn; switch one on with --enable
code=1
```

Тот же файл с включённым обходом квадратных скобок:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --enable "Sound in brackets" --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --enable "Sound in brackets" --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:02,000
Bonjour
```

## Что записывается и что сообщается

Формат, кодировка и символы конца строки прочитанного файла **сохраняются**. **Субтитры,
опустошённые исправлением, удаляются**, как в поле окна по умолчанию;
`--keep-blank-subtitles` оставляет их пустыми, а `--remove-blank-subtitles` удаляет их,
даже когда файл настроек их сохраняет. **Опустошённый перевод остаётся опустошённым**
и сохраняет свой субтитр.

**Счётчик — как в окне** — «Edited N and removed M subtitles»: изменённые
тексты и удалённые субтитры, **но не совпадения**. Файл, в котором ничего
не меняется, всё равно записывается: заданное место назначения — это записанное место назначения.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\neﬀet  de ﬁn\n\n2\n00:00:03,000 --> 00:00:04,000\nPropre.\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code Latn --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\neﬀet  de ﬁn\n\n2\n00:00:03,000 --> 00:00:04,000\nPropre.\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code Latn --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:02,000
effet de fin

2
00:00:03,000 --> 00:00:04,000
Propre.
```

### Шаблон, который нельзя применить

**Шаблон, который не удаётся прочитать, нельзя преобразовать или который не завершается, называется вместе с
субтитром, на котором он сдался**, и здесь командная строка сообщает больше, чем
окно: страница подтверждения называет шаблон лишь один раз и без текста.

| Причина | Что это значит |
| :----- | :------------------- |
| `cannot be translated` | в его выражении есть то, что отвергает преобразование в ICU |
| `will not compile` | движок отверг его выражение |
| `has an invalid replacement` | его замена не читается так, как её читает Python |
| `timed out` | движок отказался от этого текста |
| `never settled` | `Repeat` продолжал менять текст и после ста проходов |
| `grew the text too long` | текст вышел за пределы того, что может вместить субтитр |

На уровне 1, в потоке ошибок: 
`<path>: pattern "<name>" (<reason>) was not applied to subtitle N` — без конца, когда шаблон отложен для всех текстов. **Номер —
номер в файле**, какие бы задачи ни удалили субтитры раньше. В JSON это
предупреждение файла `pattern-failed`, в поле `detail` которого лежит фраза. **Шаблон,
от которого отказались, — это не неудача файла**: файл записывается, а код завершения равен `0`.

## Вывод

**Стандартный вывод** — пусто, поскольку результат — записанный файл. **С `--dry-run` это
предлагаемые изменения**, по блоку на субтитр: его номер, текст до, текст после —
и `(removed)` без строки `+` для субтитра, который исправление удалило бы; с
`--keep-blank-subtitles` он остаётся пустым. См. [Просмотр перед записью](invocation.md#просмотр-перед-записью).

| Уровень | Что добавляется в поток ошибок |
| :----- | :------------------------------------- |
| 1 | `<path>: Edited N and removed M subtitles -> <destination>`, шаблоны, которые не удалось применить, файлы шаблонов, которые не удаётся прочитать, — а в самом начале файл настроек, который был прочитан, — `settings: read <path>` — и то, что из него было проигнорировано |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, затем все диагностические сообщения чтения |

В JSON `counts` содержит `corrected` (изменённые тексты) и `removed` (удалённые
субтитры), а `changes` — затронутые субтитры. С `-t` ключ `alignment` сообщает, как был
наложен перевод.
`settings` называет прочитанный файл настроек или равен `null`, если файла не было.

## Коды завершения

Те же, что у программы: `0`, если записан каждый файл, `2`, если не записан ни один, `3`, если
записаны только некоторые, `1` при ошибке использования.

## Ошибки

| Что их вызывает | Что выводится в поток ошибок |
| :------------------ | :--------------------------------------- |
| файл, названный в `--config`, не существует — собственный файл пользователя, будучи отсутствующим, даёт значения по умолчанию | `settings file not found: <FILE>` |
| нет `--tasks` | `--tasks is required`, за которым следует указание на `--help` |
| неизвестная задача | `--tasks: "<name>" is not a task: expected mentions, join-words, split-words, common-errors, capitalization or line-break` |
| нет `--code` | `--code is required by the tasks that read patterns: <tasks>` |
| `--code` без задачи с шаблонами | `--code is for the tasks that read patterns, and none was asked for` |
| нет `--language` при `join-words` или `split-words` | `--language is required by the tasks that check words: <tasks>` |
| `--language`, не являющийся кодом языка | `--language: "<value>" is not a language: expected a locale code, like fr, en_US or sr@Latn` |
| `--language` без одной из этих двух задач | `--language is for the tasks join-words and split-words, which were not asked for` |
| язык без словаря | `no dictionary for <language>` |
| неверно записанный код | `--code: "<code>" is not a pattern code: expected Script[-language[-COUNTRY]], like Zyyy, Latn or Latn-en-US` |
| нет `--max-length` при `line-break` | `--max-length is required by the task line-break: Gaupol's 24 is a width in ems, and has no value in characters` |
| `--max-length`, не являющийся числом больше нуля | `--max-length: "<value>" is not a length: expected a number of characters greater than zero` |
| `--max-lines`, `--skip-lines`, не являющиеся целым числом строк | `--max-lines: "<value>" is not a number of lines: expected a whole number greater than zero` |
| нечитаемый `--skip-length` или `--skip-lines` | `--skip-length: "<value>" is not a bound: expected a number greater than zero, or off` |
| один из четырёх параметров без `line-break` | `--max-length is for the task line-break, which was not asked for` |
| неизвестный класс | `--classes: "<name>" is not a class: expected human, ocr or human,ocr` |
| `--keep-blank-subtitles` с `--remove-blank-subtitles` | `--keep-blank-subtitles and --remove-blank-subtitles say opposite things; give one of them` |
| неверное имя шаблона | см. таблицу выше |
| задача без действующего шаблона | `<task>: no pattern is active under the code <code>; switch one on with --enable` |
| `--range`, `-t`, `--document`, место назначения | те, что описаны в разделах [Запуск](invocation.md#ошибки) и [Перевод](invocation.md#перевод) |
