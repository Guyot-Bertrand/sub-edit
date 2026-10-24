# `transcribe`

<!-- machine-translated -->
> Ова страница је машински превод који још није прегледао читалац језика.

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

Пише **говор филма или звучне датотеке као датотеку титлова**. Звук се припреми и преда
мотору који налази шта је речено и када, а делови које он исече спајају се у титлове
који поштују задато трајање и дужину.

**Ништа од потребног не испоручује се са програмом**: мотор (whisper.cpp), модел и `ffmpeg`
инсталира корисник, који каже где се налази модел. [Инсталирање алата за
транскрипцију](install-transcription.md) каже где се налази сваки од њих и који модел изабрати. **Њихово одсуство
ништа не квари**: команда каже шта недостаје, са упутом на ту страницу, не пише ништа
и завршава се ненултим кодом.

- **Улаз је филм или звучна датотека.** Из филма `ffmpeg` издваја једну звучну нумеру; без
  `ffmpeg`-а звучна датотека коју мотор чита каква јесте (`wav`, `mp3`, `flac`, `ogg`) ипак се
  транскрибује, али филм не, као ни било која датотека када се тражи нумера.
- **Само једна датотека одједном**, а одредиште је `--output`: нема `--output-dir` ни
  `--in-place`, јер је резултат нова датотека, а не преписивање.
- **Језик се открива** осим ако га `--language` не наметне, и употребљени језик се јавља. Модел
  направљен само за енглески не открива ништа и не транскрибује ништа друго: језик који не
  може да прими одбија се *пре* почетка било каквог рада, уз навођење границе модела.
- **Сегменти се спајају и секу** према границама поставки (`transcription.merge-gap-ms`,
  `transcription.max-duration-ms`, `transcription.max-characters`): два блиска дела једне
  реченице постају један титл, а део који је предугачак или преширок сече се на речи. **`--raw`
  пише сегменте мотора какви су дошли**, по један у сваком титлу, што је уједно начин да се види шта
  спајање ради.

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

## Аргументи и опције

| Опција | Обавезно | Вредности | Подразумевано |
| :----- | :----- | :------ | :----- |
| `<input>` | да | филм или звучна датотека | — |
| `--output` | да | датотека титлова за писање; њен директоријум се ствара ако не постоји | — |
| `--to` | не | кратки назив формата: `srt`, `vtt`, `subviewer2`, `ssa`, `ass`, `microdvd`, `mpl2`, `tmplayer`, `lrc` | формат који каже наставак датотеке `--output`, иначе `srt` |
| `--audio-track` | не | цео број од 0: положај међу звучним нумерама филма | `0`, прва |
| `--language` | не | два мала слова, ISO 639-1 код: `en`, `fr` | поставка `transcription.language`; празно: језик се открива |
| `--model` | не | датотека модела | поставка `transcription.model` |
| `--engine` | не | назив мотора: `whisper.cpp` | поставка `transcription.engine` |
| `--keep-audio` | не | датотека у којој се задржава издвојени звук, као 16-битни моно PCM од 16 kHz у WAV датотеци; њен директоријум се ствара ако не постоји | звук се нигде не задржава |
| `--raw` | не | заставица: пише сегменте мотора без спајања и без сечења | искључено |

**Поставке** се читају из датотеке поставки као оне команде [`adjust`](adjust.md) — види
[Датотека поставки](invocation.md#датотека-поставки) — и свака има своју опцију овде:

| Кључ | Значење | Подразумевано |
| :-- | :------ | :------ |
| `transcription.engine` | мотор | `whisper.cpp` |
| `transcription.executable` | програм мотора, апсолутна путања; празно: тражи се у `PATH` | празно |
| `transcription.model` | датотека модела, апсолутна путања | празно |
| `transcription.language` | језик звука, два слова; празно: открива се | празно |
| `transcription.merge-gap-ms` | делови једне реченице ближи од толико милисекунди спајају се | `400` |
| `transcription.max-duration-ms` | најдуже време које титл остаје на екрану, у милисекундама | `7000` |
| `transcription.max-characters` | највише знакова које титл садржи, без бројања прелома редова | `84` |

## Избор звука

Филм може имати више звучних нумера — оригинал, коментар, други језик. `--audio-track`
узима **положај међу звучним нумерама** датотеке, од 0, редом којим их датотека набраја.
Нумера коју филм нема одбија се, **уз нумере које има**, да би следећи позив био
прави:

```bash
subedit-cli transcribe --audio-track 4 --output film.srt --model small.bin film.mkv
```

```text
film.mkv: the video has no audio track 4
audio track 0: fra, Original
audio track 1: eng, Commentary
```

## Излаз

**Стандардни излаз** — ништа: резултат је записана датотека.

| Ниво | Шта се додаје на стандардној грешци |
| :----- | :------------------------------------- |
| 1 | `<input>: N subtitles transcribed, language: xx -> <output>` |
| 2 | напредак мотора, `transcribing: 25%`, како га он јавља — у корацима од око тридесет секунди звука |

### У JSON-у

Са [`--format json`](invocation.md#излаз-који-скрипта-може-да-чита) (глобална опција, пише се пре
подкоманде) објекат датотеке носи употребљени језик у `language`, `counts.subtitles` (записани
титлови) и `counts.segments` (делови мотора од којих су направљени), као и `audio` када је
`--keep-audio` задржао звук.

## Излазни кодови

Они читавог алата: `0` ако је датотека записана, `2` ако није могла да буде — недостаје алат или модел,
језик који модел не може да прими, неуспело издвајање или транскрипција —, `1` код грешке у употреби, на пример
мотор који нико није регистровао.

## Грешке

| Узрок | Порука |
| :---- | :------ |
| програм мотора није пронађен, или подешеног нема | `the transcription engine was not found`, па упутство на водич |
| модел није наведен, или га нема | `no model was given, or the model file is not there`, па упутство на водич |
| `ffmpeg` није пронађен за филм | `ffmpeg was not found, so the audio of a video cannot be extracted`, па упутство |
| нумера коју филм нема | `the video has no audio track N`, па нумере које има |
| језик који модел не може да транскрибује | `the model cannot transcribe the language xx` |
| нема језика, а модел не уме да га открије | `the model cannot detect the language: give one` |
| мотор који нико није регистровао | `no transcription engine is called NAME` |
| било који други квар | `the transcription failed`, са оним што је мотор рекао |

Упут на водич је `See the manual page "Installing the transcription tools"
(install-transcription).`
