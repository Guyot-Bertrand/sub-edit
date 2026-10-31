# `sync`

<!-- machine-translated -->
> Эта страница переведена машинным способом и ещё не проверена носителем языка.

```
subedit-cli sync (--video FILE | --audio FILE)
                 [--audio-track N]
                 [--language CODE]
                 [--start TIME] [--end TIME]
                 [--model FILE]
                 [--vad-model FILE]
                 [--engine NAME]
                 [--keep-audio FILE]
                 [--min-match PERCENT]
                 [--doubt-gap MS]
                 [--report FILE]
                 [-r]
                 [--output FILE | --output-dir DIR | --in-place] [--dry-run]
                 <files>...
```

**Пересчитывает время файла субтитров по звуку фильма.** Он нужен для случая с `.srt`, скачанным из интернета и
сдвинутым в начале и в конце, который одним сдвигом не вернуть: каждый субтитр ищется в том, что сказано, и
встаёт туда, где это сказано.

Звук слушает движок расшифровки — тот же, что у [`transcribe`](transcribe.md), — и **слова каждого субтитра
сопоставляются со словами, которые он услышал**. Ничего не переводится: субтитры должны быть на том языке,
на котором говорят.

**Ничего из необходимого не поставляется вместе с программой**: движок, модель и `ffmpeg` — те же, что в
[Установке инструментов расшифровки](install-transcription.md), и их отсутствие ничего не ломает: команда
сообщает, чего не хватает, со ссылкой на эту страницу, ничего не записывает и завершается с ненулевым кодом.

- **Меняются только начала и концы.** Ни текст, ни стиль, ни число субтитров, ни их порядок.
- **Субтитр, которого нет в звуке, сохраняет своё время** — титры, которые никто не произносит, песня, реплика на другом языке.
  **Никакая позиция не выдумывается и не угадывается по соседним**: строка остаётся там, где была в файле,
  и команда об этом сообщает. Это *не* сбой, и код завершения — `0`.
- **Субтитр считается найденным, когда услышана хотя бы половина его слов** (`--min-match`, `alignment.min-match`) и
  хотя бы одно. Регистр, диакритика, пунктуация и разметка не важны; слово, отличающееся одной буквой от длинного слова, считается услышанным.
- **Конец — там, где кончается последнее слово.** Движок указывает время слов приблизительно, на несколько сотых секунды раньше
  собственного конца субтитра, поэтому субтитр может стать чуть короче, чем был.
- **Субтитр, который налез бы на следующий, обрезается до начала следующего.** Тот, который всё равно пересекается со
  следующим и которого обрезка не вылечит, остаётся как есть, и об этом сообщается.
- **Файл, в котором не найден ни один субтитр, отклоняется и не записывается**: почти наверняка это не тот звук или
  не тот язык.
- **Можно прослушать только часть фильма** с помощью `--start` и `--end`, как для
  [`transcribe`](transcribe.md#расшифровка-части-фильма): ищутся и меняются только субтитры, которые **начинаются** в этом диапазоне,
  остальные остаются как есть и не считаются.
- **Звук слушается один раз для каждого указанного файла.** Чтобы пересчитать несколько файлов по одному фильму, приготовьтесь ждать
  каждый раз.

<!-- example: subedit-cli sync --help -->
```console
$ subedit-cli sync --help
Re-time the subtitles of a file on the audio of a film
Usage: subedit-cli sync [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to re-time

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --video FILE                Film to listen to
  --audio FILE                Audio file to listen to, instead of a film
  --audio-track N             Audio track of a film, counted from 0; the first by default
  --start TIME                Where to begin, as 2:00, 1:02:03 or seconds; the beginning by default
  --end TIME                  Where to stop, as 2:00, 1:02:03 or seconds; the end by default
  --language CODE             Language of the speech, two letters: fr, en, de; detected from the first 30 seconds by default
  --model FILE                Model file; the transcription.model setting by default
  --vad-model FILE            Voice-activity model that skips silences; the transcription.vad-model setting by default
  --engine NAME               Engine that listens; the transcription.engine setting by default
  --keep-audio FILE           Keep the extracted audio in this file
  --min-match PERCENT         Share of a subtitle's words that must be heard, in percent; the alignment.min-match setting by default
  --doubt-gap MS              How far a subtitle may move from its neighbours' shift before it is flagged, in milliseconds; the alignment.doubt-gap setting by default
  --report FILE               Write the subtitles to check to this file
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Аргументы и параметры

| Параметр | Обязательный | Значения | По умолчанию |
| :----- | :------- | :----- | :------ |
| `<files>` | да | файлы субтитров для пересчёта; с `-r` — каталоги | — |
| `--recursive`, `-r` | нет | флаг | выключен; см. [Обработка дерева каталогов](lots.md) |
| `--video` | один из `--video` и `--audio` | фильм для прослушивания | — |
| `--audio` | один из `--video` и `--audio` | аудиофайл для прослушивания вместо фильма | — |
| `--audio-track` | нет | целое число, начиная с 0: место среди звуковых дорожек фильма | `0`, первая |
| `--language` | нет | две буквы, код ISO 639-1: `en`, `fr` | настройка `transcription.language`; пусто: определяется автоматически |
| `--start`, `--end` | нет | время: `2:00`, `1:02:03`, `2:00.5` или число секунд; `--end` должен идти после `--start` | весь звук |
| `--model` | нет | файл модели | настройка `transcription.model` |
| `--vad-model` | нет | модель обнаружения речи, которая заставляет движок пропускать паузы; с ней слова точнее следуют за речью | настройка `transcription.vad-model` |
| `--engine` | нет | название движка, который слушает: `whisper.cpp` | настройка `transcription.engine` |
| `--keep-audio` | нет | файл, в котором сохраняется извлечённый звук; его каталог создаётся, если отсутствует | звук нигде не сохраняется |
| `--min-match` | нет | целое число от 1 до 100: доля слов субтитра, которые должны быть услышаны, в процентах | настройка `alignment.min-match`, `50` |
| `--doubt-gap` | нет | целое число миллисекунд, 0 или больше: насколько сдвиг субтитра может отличаться от сдвига соседних, прежде чем он будет отмечен | настройка `alignment.doubt-gap`, `2000` |
| `--report` | нет | файл, в который записываются субтитры, требующие проверки, по одному в строке: номер субтитра, табуляция и причина; только один файл субтитров за раз | ничего не записывается |
| `--output` / `--output-dir` / `--in-place` | **один из трёх**, кроме случая `--dry-run` | см. [Запуск](invocation.md#место-назначения) | — |
| `--dry-run` | нет | флаг: вычисляет и сообщает, ничего не записывает; см. [Просмотр перед записью](invocation.md#просмотр-перед-записью) | выключен |

Место назначения — `--output`, `--output-dir`, `--in-place`, `--dry-run` — и `-r` те же, что у других команд,
которые переписывают файл: см. [Место назначения](invocation.md#место-назначения). Файл сохраняет свой формат, кодировку и
окончания строк.

**Настройки** читаются из файла настроек — см. [Файл настроек](invocation.md#файл-настроек), — и у каждой
есть свой параметр. Движок, модель и модель обнаружения речи — те же, что у `transcribe`.

| Ключ | Значение | По умолчанию |
| :-- | :------ | :------ |
| `alignment.engine` | движок выравнивания; он один, `transcript-matching`, который слушает и сопоставляет | `transcript-matching` |
| `alignment.min-match` | доля слов субтитра, которые должны быть услышаны, чтобы он считался найденным, в процентах | `50` |
| `alignment.doubt-gap` | насколько, в миллисекундах, сдвиг субтитра может отличаться от сдвига соседних, прежде чем он будет отмечен | `2000` |

## Что отмечается

Команда сообщает, какие субтитры проверить и почему. Субтитр отмечается по **первой** из этих причин, которая верна:

| Причина | Значение |
| :-- | :------ |
| not found in the audio, left as it was | услышано меньше `--min-match` его слов: он сохраняет своё время |
| only partly heard | найден, но услышано меньше 80 % его слов |
| moved differently from its neighbours | найден, но его сдвиг отличается от среднего сдвига двух субтитров с каждой стороны больше чем на `--doubt-gap` |

Субтитр также может быть **обрезан до начала следующего** (cut back to the start of the next subtitle) или **по-прежнему пересекаться со следующим** (still overlap the next subtitle).

Две цифры рассказывают о файле в целом. **Медианный сдвиг** — это середина того, насколько сдвинулись субтитры: файл,
который просто опаздывал, виден здесь и больше нигде. **Дрейф** — это медианный сдвиг последней десятой части
субтитров минус медианный сдвиг первой десятой: он близок к нулю при постоянном смещении и велик для файла, который
опаздывал всё больше и больше по ходу фильма.

## Вывод

**Стандартный вывод** — пусто: результат — это записанный файл.

| Уровень | Что выводится в поток ошибок |
| :---- | :------------------------------- |
| 1 | `<file>: N of M subtitles placed on the audio, K to check; median shift X s, drift Y s -> <destination>` |
| 2 | этап и ход работы, как для [`transcribe`](transcribe.md#вывод) |

### В формате JSON

С параметром [`--format json`](invocation.md#вывод-пригодный-для-чтения-скриптом) объект файла содержит `counts`: `subtitles`
(искали), `placed` (получили место), `moved` (время изменилось), `partial`, `out_of_line`, `not_found`,
`median_shift_ms` и `drift_ms`; а также `flagged` — список `{"subtitle": N, "kind": "…"}`, где число — это
место субтитра в файле, а `kind` — одно из `not-found`, `partial`, `out-of-line`, `cut-back` и `overlaps`.
Идентификаторы гарантируются; формулировки — нет.

## Пример

```bash
subedit-cli sync --video film.mkv --model small.bin --vad-model silero.bin \
    --report to-check.txt --output film.synced.srt film.srt
```

## Коды завершения

Те же, что у всего инструмента: `0`, если файл записан, независимо от того, есть ли отмеченные субтитры, `2`, если записать его не удалось — не хватает инструмента или модели,
не удалось извлечение или прослушивание, не найден ни один субтитр —, `1` при ошибке использования, например
если не указан ни `--video`, ни `--audio` (или указаны оба), параметр вне допустимых границ или непригодный диапазон.

## Ошибки

| Причина | Сообщение |
| :---- | :------ |
| не указан ни `--video`, ни `--audio`, или указаны оба | `sync needs the audio to listen to: give --video or --audio, and not both` |
| программа движка не найдена, или настроенной нет на месте | `the transcription engine was not found`, затем указание на руководство |
| модель не указана, или её нет на месте | `no model was given, or the model file is not there`, затем указание на руководство |
| `ffmpeg` не найден для фильма | `ffmpeg was not found, so the audio of a video cannot be extracted`, затем указание |
| дорожка, которой в фильме нет | `the video has no audio track N`, затем имеющиеся дорожки |
| язык, который модель не может расшифровать | `the model cannot transcribe the language xx` |
| движок, который никто не зарегистрировал | `no transcription engine is called NAME` или `there is no alignment engine called NAME` |
| не найден ни один субтитр | `none of the N subtitles was found in the audio: is it the right audio, and the right language?` |
| `--min-match` вне диапазона от 1 до 100, отрицательный `--doubt-gap` | ошибка использования параметра |
| `--report` с несколькими файлами субтитров | `--report names one file: give one subtitle file when you use it` |
