# `adjust`

<!-- machine-translated -->
> Эта страница переведена машинным способом и ещё не проверена носителем языка.

```
subedit-cli adjust [--speed CPS|off] [--shorten | --no-shorten] [--lengthen | --no-lengthen]
                   [--minimum TIME|off] [--maximum TIME|off] [--gap TIME|off]
                   [--range N-M|N-]
                   (--output FILE | --output-dir DIR | --in-place)
                   [--dry-run]
                   [--recursive]
                   <file>...
```

Подгоняет **длительность** каждого субтитра под скорость чтения и ограничения: слишком
короткий для чтения субтитр удлиняется, субтитр, который уходит с экрана
слишком поздно, можно сократить, а два субтитра не должны перекрываться. **Сдвигается только
конец**: сдвиг начала означал бы перемещение самого субтитра, а это работа подкоманды
[`shift`](shift.md). Это операция `Adjust Durations…` в
[окне](../subedit-gui/operations.md#adjust-durations), с теми же значениями по умолчанию.

## Четыре ограничения и их порядок

Они применяются **в таком порядке**, и каждое задаёт конец заново, а побеждает последнее:

1. **скорость чтения** — длительность, которая нужна тексту при заданном числе символов в
   секунду (теги не считаются, переводы строки считаются);
2. **минимум** — длительность ниже минимума доводится до минимума;
3. **максимум** — длительность выше максимума снижается до максимума;
4. **промежуток** — время, оставляемое до следующего субтитра. Он применяется последним, и
   **он побеждает**: два субтитра, показанные одновременно, — это недостаток, который видит зритель,
   а немного короткий субтитр — нет.

У последнего субтитра файла нет следующего, а значит, нет и промежутка, который нужно соблюдать. Следующим
для субтитра считается следующий в файле, **выбран он параметром `--range` или нет**.

| Ограничение | Параметр | По умолчанию | Как отключить |
| :--------- | :----- | :----- | :------- |
| скорость | `--speed CPS` | **15** символов в секунду | `--speed off` |
| направление скорости | `--shorten`, `--no-shorten`, `--lengthen`, `--no-lengthen` | только **удлиняет** | — |
| минимум | `--minimum TIME` | **1,5 с** | `--minimum off` |
| максимум | `--maximum TIME` | **нет** | `--maximum off` |
| промежуток | `--gap TIME` | **0** | `--gap off` |

Это настройки Gaupol и первого запуска окна. **`off` отключает
ограничение: оно больше не проверяется.** Это не то же самое, что задать нулевое значение: минимум `0`
— это минимум, равный нулю, а промежуток `0` запрещает любое перекрытие.

## Что задаёт файл настроек

**Значения по умолчанию, приведённые выше, — встроенные.** По умолчанию `adjust` читает настройки операции из
файла настроек — значения, которые запоминает `Adjust Durations…` в окне или которые
записала подкоманда [`config set`](config.md), — и они заменяют встроенные значения. **Параметр,
указанный в командной строке, всегда побеждает**, затем идёт файл, затем встроенное значение. О том,
где лежит файл, а также о `--config` и
`--no-config` см. [Файл настроек](invocation.md#файл-настроек).

| Ключ | Что он задаёт | Параметр, который его переопределяет |
| :-- | :--------------- | :----------------------- |
| `duration-adjust.speed` | скорость чтения в символах в секунду | `--speed CPS` |
| `duration-adjust.lengthen`, `duration-adjust.shorten` | в какую сторону скорость может сдвигать конец; если оба равны `false`, скорость отключена | `--lengthen`, `--no-lengthen`, `--shorten`, `--no-shorten`, `--speed off`; указание `--speed CPS` или направления снова включает скорость, отключённую в файле, с удлинением |
| `duration-adjust.minimum-enabled`, `duration-adjust.minimum-ms` | наименьшая длительность или её отсутствие | `--minimum TIME`, `--minimum off` |
| `duration-adjust.maximum-enabled`, `duration-adjust.maximum-ms` | наибольшая длительность или её отсутствие | `--maximum TIME`, `--maximum off` |
| `duration-adjust.gap-enabled`, `duration-adjust.gap-ms` | наименьший промежуток или его отсутствие | `--gap TIME`, `--gap off` |

**У каждого ключа есть параметр, который его отменяет.** У каждого направления есть пара флагов, которая
задаёт его независимо от файла: `--shorten` и `--no-shorten` для сокращения, `--lengthen` и
`--no-lengthen` для удлинения; не указанный флаг оставляет направление из файла без изменений.
`--maximum off` отбрасывает максимум, включённый в файле. С `--format json` поле `constraints` сообщает
настройки, которые действительно использовались, откуда бы они ни пришли, а `settings` — прочитанный файл:

<!-- example: printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine/settings.conf config set duration-adjust.speed 12 2>/dev/null; subedit-cli --config mine/settings.conf --format json adjust --dry-run a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine/settings.conf config set duration-adjust.speed 12 2>/dev/null; subedit-cli --config mine/settings.conf --format json adjust --dry-run a.srt
settings: read mine/settings.conf
a.srt: adjusted the durations of 1 subtitle (dry run, nothing written)
{"schema":1,"command":"adjust","file":"a.srt","ok":true,"dry_run":true,"destination":null,"counts":{"subtitles":1,"adjusted":1,"sacrificed":{"speed":0,"minimum":0,"gap":0}},"constraints":{"speed":{"cps":"12","lengthen":true,"shorten":false},"minimum_ms":1500,"maximum_ms":null,"gap_ms":0},"settings":"mine/settings.conf","warnings":[]}
```

Остальные примеры на этой странице выполнены с `--no-config`, чтобы они показывали, что делают встроенные
значения по умолчанию, независимо от настроек машины, на которой они запускаются.

<!-- example: subedit-cli adjust --help -->
```console
$ subedit-cli adjust --help
Bring the duration of every subtitle within a reading speed and limits
Usage: subedit-cli adjust [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to adjust

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --speed CPS|off             Reading speed in characters per second, or off (default 15)
  --shorten                   Let the reading speed bring an end earlier
  --no-shorten                Do not let the reading speed bring an end earlier
  --lengthen                  Let the reading speed move an end later
  --no-lengthen               Do not let the reading speed move an end later
  --minimum TIME|off          Shortest duration, or off (default 1.5)
  --maximum TIME|off          Longest duration, or off (default: none)
  --gap TIME|off              Least time left before the next subtitle, or off (default 0)
  --range N-M|N-              Act only on subtitles N to M, or N to the end
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
| `--speed` | нет | число символов в секунду, **больше нуля** (`15`, `12.5`), или `off` | `15` |
| `--shorten` | нет | флаг: скорость может также **приближать** конец, делая его **более ранним** | выкл. |
| `--no-shorten` | нет | флаг: скорость больше не **приближает** конец, делая его **более ранним**, что бы ни говорил файл настроек | выкл. |
| `--lengthen` | нет | флаг: скорость **сдвигает** конец **позже**, что бы ни говорил файл настроек | выкл. |
| `--no-lengthen` | нет | флаг: скорость больше не **сдвигает** конец **позже** | выкл. |
| `--minimum` | нет | длительность, равная нулю или больше, либо `off` | `1.5` |
| `--maximum` | нет | длительность **больше нуля** либо `off` | нет |
| `--gap` | нет | длительность, равная нулю или больше, либо `off` | `0` |
| `--range` | нет | `N-M` или `N-` до конца — см. ниже | весь файл |
| `--output` / `--output-dir` / `--in-place` | **один из трёх**, кроме случая с `--dry-run` | см. [Запуск](invocation.md#место-назначения) | — |
| `--dry-run` | нет | флаг: вычисляет и сообщает, ничего не записывает — см. [Просмотр перед записью](invocation.md#просмотр-перед-записью) | выкл. |

Длительность записывается так же, как для [`shift`](shift.md#запись-длительности): `2.5`, `00:00:02.500`.

**Без единого действующего ограничения `adjust` отклоняется** (код `1`, ничего не записывается): подгонять
было бы не под что, а команда, которая ничего не делает, но сообщает об успехе, — худший
из ответов. Так бывает при `--speed off --minimum off --gap off` без максимума и при
одном `--no-lengthen` с отключёнными минимумом и промежутком: скорость, которая не сдвигает
ни один конец, ничего не ограничивает.

`--shorten`, `--no-shorten`, `--lengthen` и `--no-lengthen` задают, в какую сторону скорость сдвигает
конец; они имеют смысл только при заданной скорости, а **`--speed off` их отклоняет**. Флаг, указанный вместе со своей
противоположностью, — `--shorten --no-shorten`, `--lengthen --no-lengthen`, — тоже отклоняется.

### `--range`

`--range 3-7` ограничивает подгонку субтитрами с 3 по 7, **нумерация с 1, границы
включаются**; `--range 3-` идёт до последнего. Один субтитр записывается как `4-4`.

- Диапазон, **который заканчивается раньше, чем начинается**, `0`, знак или одиночное число
  отклоняются как ошибка использования (код `1`).
- **Он действует для каждого файла пакета**, что имеет смысл, только если файлы
  похожи друг на друга. Он проверяется **по каждому файлу**: диапазон, выходящий за пределы этого файла,
  отклоняется **до того, как файл тронут**, с указанием границы —
  `range 3-9 ends after the last subtitle: the file has 4` —, этот файл завершается неудачей (код `2` или `3`, идентификатор
  `range-out-of-bounds`), а остальные продолжают обрабатываться.
- Числа в отчёте относятся только к выбранным субтитрам.

## Чем жертвуют

Два ограничения могут противоречить друг другу на одном субтитре: минимум 1,5 с не помещается
в 0,8 с, которые оставляет следующий субтитр. **Порядок решает, а отчёт сообщает,
что уступило**, — чего Gaupol не делает: он нарушает ограничения молча.

Ограничение называется, только если **хотя бы один субтитр его не получил**, после
применения остальных. У максимума нет счётчика: он применяется раньше промежутка, который может
только сократить, поэтому максимум соблюдается всегда. Минимум выше максимума уступает максимуму и
считается как минимум.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --output ajuste.srt courts.srt; grep -e '-->' ajuste.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --output ajuste.srt courts.srt; grep -e '-->' ajuste.srt
courts.srt: adjusted the durations of 3 subtitles; could not satisfy the minimum duration in 1 subtitle -> ajuste.srt
00:00:00,000 --> 00:00:02,000
00:00:10,000 --> 00:00:14,000
00:00:14,200 --> 00:00:15,000
00:00:15,000 --> 00:00:16,500
```

Третий субтитр хотел бы закончиться на 15,7 с, чтобы длиться 1,5 с, но четвёртый начинается
на 15,0 с: **побеждает промежуток**, субтитр заканчивается на 15,0 с, а отчёт называет минимум.

Если отключить промежуток, побеждает минимум — и два субтитра перекрываются:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --gap off --output libre.srt courts.srt; grep -e '-->' libre.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --gap off --output libre.srt courts.srt; grep -e '-->' libre.srt
courts.srt: adjusted the durations of 3 subtitles -> libre.srt
00:00:00,000 --> 00:00:02,000
00:00:10,000 --> 00:00:14,000
00:00:14,200 --> 00:00:15,700
00:00:15,000 --> 00:00:16,500
```

И ничем не жертвуют и ничего не подгоняют, когда файл уже укладывается в свои ограничения:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --speed off --minimum off --output intact.srt courts.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --speed off --minimum off --output intact.srt courts.srt
courts.srt: no duration to adjust -> intact.srt
```

Файл записывается как есть: **заданное место назначения — это записанное место назначения**.

## Вывод

**Стандартный вывод** — пусто: результат — записанный файл.

| Уровень | Что добавляется в поток ошибок |
| :----- | :------------------------------------- |
| 1 | `<path>: adjusted the durations of N subtitles; could not satisfy …  -> <destination>` или `<path>: no duration to adjust -> <destination>` — а в самом начале файл настроек, который был прочитан, — `settings: read <path>` — см. [Файл настроек](invocation.md#файл-настроек) |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — формат, кодировка, метка и символы конца строки прочитанного файла, возвращённые как были |
| 3 | `<path>: N bytes read, M written`, затем **все диагностические сообщения чтения** — см. [Запуск](invocation.md#чтение-диагностических-сообщений) |

Фраза — та же, что в окне: `N` — число субтитров, конец которых сдвинулся, а
остальное, отсутствующее, когда ничто не уступило, перечисляет то, что не было соблюдено, — `the reading speed`,
`the minimum duration`, `the gap` — с числом затронутых субтитров.

### В формате JSON

С [`--format json`](invocation.md#вывод-пригодный-для-чтения-скриптом) объект каждого файла
содержит, помимо оболочки:

| Ключ | Содержимое |
| :-- | :------ |
| `counts.subtitles` | субтитры, **на которые направлена** подгонка: весь файл или диапазон |
| `counts.adjusted` | те, у которых сдвинулся конец |
| `counts.sacrificed.speed`, `.minimum`, `.gap` | субтитры, которые по-прежнему не соблюдают это ограничение после применения остальных, **включая те, у которых конец не сдвинулся** |
| `constraints.speed` | `{"cps", "lengthen", "shorten"}` — число символов в секунду **в виде строки** (`"15"`, `"12.5"`) — или `null`, если отключено |
| `constraints.minimum_ms`, `.maximum_ms`, `.gap_ms` | использованные длительности в **целых миллисекундах** или `null`, если отключено |
| `settings` | прочитанный файл настроек или `null`, если файла не было |

`constraints` сообщает **настройки, которые действительно использовались**, включая значения по умолчанию:
два вычисления над одними и теми же субтитрами тогда можно сравнивать, не читая командную строку.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config --format json adjust --dry-run --maximum 1.8 courts.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config --format json adjust --dry-run --maximum 1.8 courts.srt
courts.srt: adjusted the durations of 4 subtitles; could not satisfy the reading speed in 1 subtitle, the minimum duration in 1 subtitle (dry run, nothing written)
{"schema":1,"command":"adjust","file":"courts.srt","ok":true,"dry_run":true,"destination":null,"counts":{"subtitles":4,"adjusted":4,"sacrificed":{"speed":1,"minimum":1,"gap":0}},"constraints":{"speed":{"cps":"15","lengthen":true,"shorten":false},"minimum_ms":1500,"maximum_ms":1800,"gap_ms":0},"settings":null,"warnings":[]}
```

## Коды завершения

Те же, что у программы: `0`, если обработан каждый файл, `2`, если не обработан ни один, `3`, если обработаны только
некоторые, `1` при ошибке использования. Файл, который уже укладывается в свои ограничения, считается обработанным.

## Ошибки

| Что их вызывает | Сообщение |
| :------------------ | :------ |
| файл, названный в `--config`, не существует — собственный файл пользователя, будучи отсутствующим, даёт значения по умолчанию | `settings file not found: <FILE>` |
| нет ни одного действующего ограничения | `adjust has nothing to adjust to: every constraint is off, or the reading speed is told to move no end` |
| значение, не являющееся скоростью | `--speed: "…" is not a reading speed: expected a number of characters per second above zero, like 15 or 12.5, or off` |
| флаг направления с `--speed off` | `--lengthen, --no-lengthen, --shorten and --no-shorten say which way the reading speed may move an end, and --speed off switches the reading speed off` |
| флаг направления вместе со своей противоположностью | `--shorten and --no-shorten say opposite things; give one of them` (так же для `--lengthen` и `--no-lengthen`) |
| значение, не являющееся длительностью | `--minimum: "…" is not a time: …` (так же для `--maximum`, `--gap`) |
| отрицательная длительность | `--minimum: "-1" is not a duration of zero or more` (так же для `--gap`) |
| нулевой или отрицательный максимум | `--maximum: "0" is not a duration above zero` |
| неверно записанный диапазон | `--range: "…" is not a range: write it N-M, or N- to go to the end, counted from 1 and inclusive` |
| диапазон, заканчивающийся раньше, чем начинается | `--range: "5-3" is not a range: it ends before it starts` |
| диапазон за пределами файла | `<path>: range N-M starts after the last subtitle: the file has K` или `… ends after …` |

Ошибки места назначения общие для подкоманд, которые записывают:
см. [Запуск](invocation.md#место-назначения).
