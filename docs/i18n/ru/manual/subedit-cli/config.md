# `config`

<!-- machine-translated -->
> Эта страница переведена машинным способом и ещё не проверена носителем языка.

```
subedit-cli [--config FILE | --no-config] [--format text|json] config list
subedit-cli [--config FILE | --no-config] [--format text|json] config show
subedit-cli [--config FILE] config set KEY VALUE
subedit-cli [--config FILE] config unset KEY
```

Перечисляет, показывает и изменяет **настройки операций**: ту часть файла настроек, которую
окно и командная строка используют совместно. `Adjust Durations…` и `Correct Texts…` запоминают в нём свои
значения, а [`adjust`](adjust.md) и [`correct`](correct.md) читают их по умолчанию — см.
[Файл настроек](invocation.md#файл-настроек).

**Чего она не делает:** она никогда не читает и не записывает **параметр отображения** (геометрию
окна, столбцы, тему, параметры поиска, проверку орфографии, видео…). Эти параметры остаются
делом окна, задаются в окне и описаны в [его руководстве](../subedit-gui/preferences.md#файл).
Файлов субтитров она тоже не трогает.

<!-- example: subedit-cli config --help -->
```console
$ subedit-cli config --help
List, show and change the operation settings
Usage: subedit-cli config [OPTIONS] SUBCOMMAND

Options:
  -h,--help                   Print this help message and exit

Subcommands:
  list                        Write every operation key, what it is for, what it accepts and its default
  show                        Write the settings file used, then every operation key with its value
  set                         Write KEY = VALUE into the settings file, keeping every other setting
  unset                       Put KEY back to its default in the settings file, keeping every other setting
```

## Подкоманды

| Подкоманда | Что делает | Файл |
| :---------- | :----------- | :--- |
| `config list` | выводит все ключи операций с их назначением, допустимыми значениями, значением по умолчанию и теми, кто их использует, — **без значений** | не читает |
| `config show` | выводит используемый файл и сообщает, существует ли он, затем все ключи операций с их значением, источником значения и значением по умолчанию | читает его |
| `config set KEY VALUE` | проверяет `VALUE`, затем переписывает файл с `KEY = VALUE`, сохраняя все остальные настройки | читает и записывает его |
| `config unset KEY` | возвращает `KEY` к значению по умолчанию, записанному в виде комментария, как файл всегда записывает значение по умолчанию | читает и записывает его |

| Параметр | Обязательный | Значения | По умолчанию |
| :----- | :------- | :----- | :------ |
| `KEY` | да, для `set` и `unset` | ключ операции в том виде, как его называет `config list` | — |
| `VALUE` | да, для `set` | то, что допускает ключ по сообщению `config list` | — |

У `config` нет собственных параметров. Глобальные параметры, указанные **перед** `config`, выбирают
файл и вид вывода:

| Глобальный параметр | Действие на `config` |
| :------------ | :----------------- |
| `--config FILE` | `show`, `set` и `unset` работают с `FILE` вместо файла пользователя, даже если он ещё не существует; `list` его игнорирует |
| `--no-config` | `list` работает как обычно; `show` показывает значения по умолчанию, с `File: none (--no-config)`; `set` и `unset` отклоняются: записывать некуда |
| `--format json` | `list` и `show` выводят по одному объекту JSON в строке; `set` и `unset` ничего не выводят в стандартный вывод |
| `-q`, `--quiet` | `set` и `unset` больше не подтверждают, что они записали |

## Какой файл

Без `--config` — файл пользователя:

```
$XDG_CONFIG_HOME/subedit/settings.conf
```

или `~/.config/subedit/settings.conf`, если `XDG_CONFIG_HOME` не задана, пуста или не является абсолютным
путём. Это файл, который окно читает и записывает; `config show` выводит его путь в первой строке:

<!-- example: XDG_CONFIG_HOME=/home/vous/.config subedit-cli config show | head -1 -->
```console
$ XDG_CONFIG_HOME=/home/vous/.config subedit-cli config show | head -1
File: /home/vous/.config/subedit/settings.conf (not found)
```

Если ни `XDG_CONFIG_HOME`, ни `HOME` не указывают места, файла пользователя нет: `show`, `set`
и `unset` отклоняются, а `--config FILE` задаёт файл.

## `config list`

**Каталог ключей операций** в порядке файла: каждый ключ, затем четыре строки с отступом —
его назначение, `Accepts:` (закрытое множество или форма и границы), `Default:` и
`Used by:` (команды `subedit-cli` и действия окна, которые его читают). Она не читает
никакого файла, он ей не нужен, и она работает также с `--no-config`.

<!-- example: subedit-cli config list | head -10 -->
```console
$ subedit-cli config list | head -10
duration-adjust.speed
    Reading speed the durations are brought to.
    Accepts: a number of characters per second, above 0, decimals allowed
    Default: 15
    Used by: subedit-cli adjust; Adjust Durations…
duration-adjust.lengthen
    Let the reading speed move an end later.
    Accepts: true or false
    Default: true
    Used by: subedit-cli adjust; Adjust Durations…
```

Она перечисляет **45 ключей**:

<!-- example: subedit-cli config list | grep -c '^[a-z]' -->
```console
$ subedit-cli config list | grep -c '^[a-z]'
45
```

**Двадцать пять из них читаются командной строкой**; остальные двадцать читаются только окном,
о чём говорит их строка `Used by:` с указанием параметра `correct`, который занимает их место:

| Ключ | Кто читает |
| :-- | :------ |
| `duration-adjust.speed`, `duration-adjust.lengthen`, `duration-adjust.shorten` | [`adjust`](adjust.md) и `Adjust Durations…` |
| `duration-adjust.minimum-enabled`, `duration-adjust.minimum-ms` | `adjust` и `Adjust Durations…` |
| `duration-adjust.maximum-enabled`, `duration-adjust.maximum-ms` | `adjust` и `Adjust Durations…` |
| `duration-adjust.gap-enabled`, `duration-adjust.gap-ms` | `adjust` и `Adjust Durations…` |
| `correction.human`, `correction.ocr` | [`correct`](correct.md) и `Correct Texts…` |
| `correction.remove-blank` | `correct` и `Correct Texts…` |
| `correction.line-break.max-lines` | `correct` и `Correct Texts…` |
| `transcription.engine`, `transcription.executable`, `transcription.model`, `transcription.vad-model`, `transcription.language`, `transcription.suppress-notes`, `transcription.merge-gap-ms`, `transcription.max-duration-ms`, `transcription.max-characters` | [`transcribe`](transcribe.md) и `Transcribe Audio…` |
| `alignment.engine`, `alignment.min-match`, `alignment.doubt-gap` | [`sync`](sync.md) и `Synchronize with Audio…` |
| все остальные ключи `correction.*` | только `Correct Texts…` — `correct` берёт `--tasks`, `--code`, `--max-length`, `--skip-length`, `--skip-lines`, `--language`, `--enable` и `--disable` из собственной командной строки |

Числа строк, `correction.line-break.max-lines` и `correction.line-break.skip-max-lines`,
— целые числа от 1 до 1000, границы `correct --max-lines`, — а длины строк,
`correction.line-break.max-length` и `correction.line-break.skip-max-length`, — числа больше
0: `set` отклоняет остальное, а файл, отредактированный вручную и содержащий такое значение, сохраняет значение по умолчанию.

`correct` берёт задачи, код и длину строки из собственной командной строки намеренно: в командной строке
заранее ничего не отмечено, а окно измеряет строки в em, тогда как командная строка
считает символы. `correction.activations`, список полей шаблонов, которым управляет окно, не
является ключом каталога: у `correct` есть `--enable` и `--disable`.

С `--format json` выводится по одному объекту на ключ — `key`, `description`, `accepts`, `default`, `used_by`
и `surface`, `"both"` или `"window-only"`:

<!-- example: subedit-cli --format json config list | head -1 -->
```console
$ subedit-cli --format json config list | head -1
{"schema":1,"command":"config","key":"duration-adjust.speed","description":"Reading speed the durations are brought to.","accepts":"a number of characters per second, above 0, decimals allowed","default":"15","used_by":"subedit-cli adjust; Adjust Durations…","surface":"both"}
```

## `config show`

**Используемый файл, затем все ключи операций с их значением**: `File: <path> (found)`,
`(not found)` или `(unreadable: <reason>)`, затем по строке на ключ —
`[file]` и значение по умолчанию, если его задаёт файл, `[default]`, если не задаёт. Ключ,
который есть в файле с нечитаемым значением, показывается со значением по умолчанию. Параметры отображения не перечисляются.

<!-- example: subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --config mine/settings.conf config show | head -4 -->
```console
$ subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --config mine/settings.conf config show | head -4
duration-adjust.speed = 12
File: mine/settings.conf (found)
duration-adjust.speed = 12   [file]   (default 15)
duration-adjust.lengthen = true   [default]
duration-adjust.shorten = false   [default]
```

Отсутствующий файл — не ошибка: `show` выводит значения по умолчанию, а код равен `0`. Это относится к
файлу пользователя, но не к `FILE`, названному в `--config`, который [`adjust`](adjust.md) и [`correct`](correct.md), напротив,
отклоняют (см. [Файл настроек](invocation.md#файл-настроек)). С
`--no-config`:

<!-- example: subedit-cli --no-config config show | head -2 -->
```console
$ subedit-cli --no-config config show | head -2
File: none (--no-config)
duration-adjust.speed = 15   [default]
```

С `--format json` первым идёт объект `{"file", "exists"}` — `file` равен `null` с `--no-config` —,
затем по объекту на ключ: `key`, `value`, `source` (`"file"` или `"default"`), `default`,
`used_by`, `surface`.

<!-- example: subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --format json --config mine/settings.conf config show | head -2 -->
```console
$ subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --format json --config mine/settings.conf config show | head -2
duration-adjust.speed = 12
{"schema":1,"command":"config","file":"mine/settings.conf","exists":true}
{"schema":1,"command":"config","key":"duration-adjust.speed","value":"12","source":"file","default":"15","used_by":"subedit-cli adjust; Adjust Durations…","surface":"both"}
```

## `config set` и `config unset`

**`set` проверяет значение собственным читателем файла** — тем же, которым пользуется окно, — и отклоняет
его, если оно нечитаемо, до того, как что-либо записано. **`unset`** записывает значение по умолчанию обратно
в виде комментария: ключ после этого следует значению по умолчанию, даже если более поздняя версия его изменит. Сброс
ключа, который уже имеет значение по умолчанию, завершается успешно и ничего не меняет.

Обе подкоманды **переписывают файл целиком и атомарно**: все параметры отображения, уже лежащие в нём, сохраняются,
блок операций идёт первым, блок отображения вторым, каждый под своим заголовком. **Значение,
которое файл не смог прочитать, и неизвестный ключ не сохраняются** — перезапись выполняется так же, как в окне, которое
их тоже отбрасывает. **Каждое отброшенное значение сообщается** в потоке ошибок на уровне по умолчанию, включая параметры
отображения: `<key>: unreadable value '<value>' replaced by its default`. Файл и
его каталог создаются, если отсутствуют.

Подтверждение идёт в поток ошибок на уровне по умолчанию: `KEY = VALUE` для `set`,
`KEY reset to its default (DEFAULT)` для `unset`.

<!-- example: subedit-cli --config mine/settings.conf config set duration-adjust.gap-ms 100; grep -e '^[a-z]' -e '^# [A-Z][a-z]* [a-z]*:' mine/settings.conf; subedit-cli --config mine/settings.conf config unset duration-adjust.gap-ms; grep gap-ms mine/settings.conf -->
```console
$ subedit-cli --config mine/settings.conf config set duration-adjust.gap-ms 100; grep -e '^[a-z]' -e '^# [A-Z][a-z]* [a-z]*:' mine/settings.conf; subedit-cli --config mine/settings.conf config unset duration-adjust.gap-ms; grep gap-ms mine/settings.conf
duration-adjust.gap-ms = 100
# Operation settings: shared by the window and the command line (subedit-cli reads these).
duration-adjust.gap-ms = 100
# Display preferences: the window's own, the command line never reads them.
duration-adjust.gap-ms reset to its default (0)
#duration-adjust.gap-ms = 0
#transcription.merge-gap-ms = 400
```

## Коды завершения

| Код | Значение |
| :--- | :------ |
| `0` | выполнено — включая `show` для отсутствующего файла и `unset` для ключа со значением по умолчанию |
| `1` | ошибка использования: нет подкоманды, пропущен аргумент, отклонён ключ или значение, `--no-config` с `set` или `unset`, `--config` с пустым именем, нет файла пользователя и нет `--config`; ничего не записывается |
| `2` | файл не удаётся прочитать или записать; ничего не записывается |

## Ошибки

| Что их вызывает | Что выводится в поток ошибок |
| :--------------- | :------------------------------------- |
| параметр отображения | `<key> is a display preference; set it in the window` |
| неизвестный ключ | `unknown setting <key>; subedit-cli config list names them` |
| значение, которое ключ не допускает | `cannot read '<value>' for <key>; subedit-cli config list says what it accepts` |
| `correction.activations` | `correction.activations is managed by the window; use --enable/--disable on correct` |
| `set` или `unset` с `--no-config` | `--no-config has nothing to write to` |
| не удаётся определить файл пользователя и нет `--config` | `no settings file could be located; give one with --config FILE` |
| `--config` с `--no-config` | `--config excludes --no-config` |
| `--config` с пустым именем | `--config needs a file name` |
| файл существует, но его не удаётся прочитать, для `set` или `unset` | `<path>: cannot be read; nothing written` |
| файл, который не удаётся записать | `<path>: cannot be written` или `<path>: cannot be written: permission denied` |

<!-- example: subedit-cli --config mine/settings.conf config set general.theme dark; echo "code=$?"; subedit-cli --config mine/settings.conf config set duration-adjust.speed fast; echo "code=$?"; subedit-cli --no-config config unset duration-adjust.speed; echo "code=$?" -->
```console
$ subedit-cli --config mine/settings.conf config set general.theme dark; echo "code=$?"; subedit-cli --config mine/settings.conf config set duration-adjust.speed fast; echo "code=$?"; subedit-cli --no-config config unset duration-adjust.speed; echo "code=$?"
general.theme is a display preference; set it in the window
code=1
cannot read 'fast' for duration-adjust.speed; subedit-cli config list says what it accepts
code=1
--no-config has nothing to write to
code=1
```
