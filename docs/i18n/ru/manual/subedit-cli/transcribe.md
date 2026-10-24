# `transcribe`

<!-- machine-translated -->
> Эта страница переведена машинным способом и ещё не проверена носителем языка.

```
subedit-cli transcribe --output FILE
                       [--to FORMAT]
                       [--audio-track N]
                       [--language CODE]
                       [--model FILE]
                       [--engine NAME]
                       [--keep-audio FILE]
                       [--raw]
                       <input>
```

Записывает **речь видео или аудиофайла в виде файла субтитров**. Звук подготавливается и передаётся
движку, который определяет, что и когда сказано, а нарезанные им части объединяются в субтитры,
соблюдающие заданные длительность и длину.

**Ничего из необходимого не поставляется вместе с программой**: движок (whisper.cpp), модель и `ffmpeg`
устанавливает пользователь, который сам указывает, где лежит модель. [Установка инструментов
расшифровки](install-transcription.md) подсказывает, где найти каждый из них и какую модель выбрать. **Их отсутствие ничего
не ломает**: команда сообщает, чего не хватает, со ссылкой на эту страницу, ничего не записывает
и завершается с ненулевым кодом.

- **Входной файл — видео или аудиофайл.** Из видео `ffmpeg` извлекает одну звуковую дорожку; без
  `ffmpeg` аудиофайл, который движок читает как есть (`wav`, `mp3`, `flac`, `ogg`), всё же
  расшифровывается, а видео — нет, как и любой файл, если запрошена дорожка.
- **Только один файл за раз**, а место назначения — `--output`: ни `--output-dir`, ни
  `--in-place` нет, поскольку результат — новый файл, а не переписывание.
- **Язык определяется автоматически**, если его не задаёт `--language`, и использованный язык сообщается. Модель,
  созданная только для английского, ничего не определяет и ничего другого не расшифровывает: язык, который она не
  может принять, отклоняется *до* начала любой работы с указанием предела модели.
- **Сегменты объединяются и режутся** по пределам настроек (`transcription.merge-gap-ms`,
  `transcription.max-duration-ms`, `transcription.max-characters`): две близкие части одного предложения
  становятся одним субтитром, а слишком длинная или слишком широкая часть режется по слову. **`--raw`
  записывает сегменты движка как есть**, по одному на субтитр; так же можно увидеть, что делает
  объединение.

<!-- example: subedit-cli transcribe --help -->
```console
$ subedit-cli transcribe --help
Write the speech of a video or an audio file as subtitles
Usage: subedit-cli transcribe [OPTIONS] input

Positionals:
  input TEXT REQUIRED         Video or audio file to transcribe

Options:
  -h,--help                   Print this help message and exit
  --output FILE               Subtitle file to write
  --to TEXT:{srt,vtt,subviewer2,ssa,ass,microdvd,mpl2,tmplayer,lrc}
                              Format to write; the one the extension of --output says by default
  --audio-track N             Audio track of a video, counted from 0; the first by default
  --language CODE             Language of the speech: fr, en; detected by default
  --model FILE                Model file; the transcription.model setting by default
  --engine NAME               Engine; the transcription.engine setting by default
  --keep-audio FILE           Keep the extracted audio in this file
  --raw                       Write the segments as the engine cut them
```

## Аргументы и параметры

| Параметр | Обязательный | Значения | По умолчанию |
| :----- | :----- | :------ | :----- |
| `<input>` | да | видео или аудиофайл | — |
| `--output` | да | файл субтитров для записи; его каталог создаётся, если отсутствует | — |
| `--to` | нет | краткое название формата: `srt`, `vtt`, `subviewer2`, `ssa`, `ass`, `microdvd`, `mpl2`, `tmplayer`, `lrc` | формат, который указывает расширение `--output`, иначе `srt` |
| `--audio-track` | нет | целое число, начиная с 0: место среди звуковых дорожек видео | `0`, первая |
| `--language` | нет | две строчные буквы, код ISO 639-1: `en`, `fr` | настройка `transcription.language`; пусто: язык определяется автоматически |
| `--model` | нет | файл модели | настройка `transcription.model` |
| `--engine` | нет | название движка: `whisper.cpp` | настройка `transcription.engine` |
| `--keep-audio` | нет | файл, в котором сохраняется извлечённый звук, как 16-битный моно PCM с частотой 16 кГц в файле WAV; его каталог создаётся, если отсутствует | звук нигде не сохраняется |
| `--raw` | нет | флаг: записать сегменты движка без объединения и без нарезки | выключен |

**Настройки** читаются из файла настроек так же, как у [`adjust`](adjust.md) — см.
[Файл настроек](invocation.md#файл-настроек), — и у каждой есть свой параметр:

| Ключ | Значение | По умолчанию |
| :-- | :------ | :------ |
| `transcription.engine` | движок | `whisper.cpp` |
| `transcription.executable` | программа движка, абсолютный путь; пусто: ищется в `PATH` | пусто |
| `transcription.model` | файл модели, абсолютный путь | пусто |
| `transcription.language` | язык звука, две буквы; пусто: определяется автоматически | пусто |
| `transcription.merge-gap-ms` | части одного предложения, расстояние между которыми меньше этого числа миллисекунд, объединяются | `400` |
| `transcription.max-duration-ms` | наибольшая длительность показа субтитра на экране, в миллисекундах | `7000` |
| `transcription.max-characters` | наибольшее число символов в субтитре, без учёта переводов строки | `84` |

## Выбор звука

В видео может быть несколько звуковых дорожек — оригинал, комментарий, другой язык. `--audio-track`
принимает **место среди звуковых дорожек** файла, начиная с 0, в том порядке, в каком их перечисляет файл.
Дорожка, которой в видео нет, отклоняется **с перечислением тех, что есть**, чтобы следующий вызов
был верным:

```bash
subedit-cli transcribe --audio-track 4 --output film.srt --model small.bin film.mkv
```

```text
film.mkv: the video has no audio track 4
audio track 0: fra, Original
audio track 1: eng, Commentary
```

## Вывод

**Стандартный вывод** — пусто: результат — это записанный файл.

| Уровень | Что добавляется в поток ошибок |
| :----- | :------------------------------------- |
| 1 | `<input>: N subtitles transcribed, language: xx -> <output>` |
| 2 | ход работы движка, `transcribing: 25%`, как он его сообщает, — шагами примерно в тридцать секунд звука |

### В формате JSON

С параметром [`--format json`](invocation.md#вывод-пригодный-для-чтения-скриптом) (глобальный параметр, пишется перед
подкомандой) объект файла содержит использованный язык в `language`, `counts.subtitles` (записанные
субтитры) и `counts.segments` (части движка, из которых они сделаны), а также `audio`, если
`--keep-audio` сохранил звук.

## Коды завершения

Те же, что у всего инструмента: `0`, если файл записан, `2`, если записать его не удалось — не хватает инструмента или модели,
модель не может принять язык, не удалось извлечение или расшифровка —, `1` при ошибке использования, например
если указан движок, который никто не зарегистрировал.

## Ошибки

| Причина | Сообщение |
| :---- | :------ |
| программа движка не найдена, или настроенной нет на месте | `the transcription engine was not found`, затем указание на руководство |
| модель не указана, или её нет на месте | `no model was given, or the model file is not there`, затем указание на руководство |
| `ffmpeg` не найден для видео | `ffmpeg was not found, so the audio of a video cannot be extracted`, затем указание |
| дорожка, которой в видео нет | `the video has no audio track N`, затем имеющиеся дорожки |
| язык, который модель не может расшифровать | `the model cannot transcribe the language xx` |
| язык не указан, а модель не умеет его определять | `the model cannot detect the language: give one` |
| движок, который никто не зарегистрировал | `no transcription engine is called NAME` |
| любой другой сбой | `the transcription failed`, с тем, что сказал движок |

Указание на руководство — `See the manual page "Installing the transcription tools"
(install-transcription).`
