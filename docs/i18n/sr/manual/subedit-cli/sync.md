# `sync`

<!-- machine-translated -->
> Ова страница је машински превод који још није прегледао читалац језика.

```
subedit-cli sync (--video FILE | --audio FILE)
                 [--audio-track N]
                 [--language CODE]
                 [--start TIME] [--end TIME]
                 [--model FILE]
                 [--vad-model FILE]
                 [--engine NAME]
                 [--keep-audio FILE]
                 [--method METHOD]
                 [--min-match PERCENT]
                 [--doubt-gap MS]
                 [--report FILE]
                 [-r]
                 [--output FILE | --output-dir DIR | --in-place] [--dry-run]
                 <files>...
```

**Усклађује време титлова једне датотеке са звуком филма.** Случај за који постоји је `.srt` преузет са интернета, померен
на почетку и на крају, што једно померање не поправља. То раде два метода, а између њих бира `--method` (или поставка
`alignment.method`):

- **`speech`, подразумевани, полаже датотеку на звук према томе када неко говори.** Од звука тражи само где има говора и
  налази померање које усклађује титлове с њим, део по део филма: стално померање, одступање, друга половина која касни и
  датотека темпирана за другу брзину сличица (25 уместо 23.976 и слично) — све се исправља.
  **Не зависи од језика** — титлови могу бити на француском, а филм на енглеском —, не тражи никакву
  транскрипцију и траје секунде. Потребни су му `ffmpeg`, програм `whisper-vad-speech-segments` који долази уз
  whisper.cpp и модел откривања говора (`--vad-model`, поставка `transcription.vad-model`).
- **`words` упарује речи сваког титла са речима које је мотор чуо.** Слуша помоћу мотора за транскрипцију —
  истог као код [`transcribe`](transcribe.md) — и сваки титл заузима место на којем се изговарају његове речи. Ништа
  се не преводи: титлови морају бити на језику којим се говори. Може да постави један титл који је ван места у датотеци
  која је иначе исправна, а траје колико и транскрипција.

**Оно што `speech` не ради каже се сваки пут када се покрене**, на излазу за грешке и у запису: исправља датотеку
**као целину**, никада један титл по један. Титл који је сам ван места у датотеци која је иначе исправна,
или натпис приказан преко тишине (картица поглавља, натпис који преводи нешто на екрану), не тражи се
и може остати погрешан, а прве минуте могу остати нетачне када филм почиње музиком или шпицом. Проверите резултат,
или користите `--method words` када је текст на језику којим се говори.

**Ништа од потребног не испоручује се са програмом**: програми, модел и `ffmpeg` су они из
[Инсталирања алата за транскрипцију](install-transcription.md), а њихово одсуство ништа не квари — команда каже
шта недостаје, са упутом на ту страницу, не пише ништа и завршава се ненултим кодом.

- **Мењају се само почеци и крајеви.** Никада текст, стил, број титлова или њихов ред.
- **Титл који звук не даје задржава своје време** — шпица коју нико не изговара, песма, ред на другом језику.
  **Ниједан положај се никада не измишља, нити погађа по суседима**: ред остаје тамо где га је датотека имала,
  а команда то каже. То *није* неуспех, а излазни кôд је `0`.
- **Са `words`, титл је пронађен када је чуто најмање пола његових речи** (`--min-match`,
  `alignment.min-match`), и најмање једна. Величина слова, акценти, интерпункција и ознаке не рачунају се; реч која се за једно слово разликује од дуге речи рачуна се као чута. `speech` поставља сваки титл и не узима ниједну од опција `--language`, `--model`, `--engine` или `--min-match`.
- **Крај је тамо где се последња реч завршава.** Мотор приближно мери време речи, неколико стотинки секунде пре
  краја самог титла, па титл може испасти нешто краћи него што је био.
- **Титл који би залазио у следећи скраћује се до места где следећи почиње.** Онај који се и даље преклапа са
  следећим, што скраћивање не лечи, оставља се и јавља.
- **Датотека која не одговара звуку одбија се и не пише се**: са `words` ниједан титл није пронађен, што је готово
  увек погрешан звук или погрешан језик — `speech` не зависи од језика; са `speech` титлови и говор имају премало заједничког,
  што је готово увек погрешан филм.
- **Може се слушати само део филма**, помоћу `--start` и `--end`, као код
  [`transcribe`](transcribe.md#транскрипција-дела-филма): траже се и дирају само титлови који **почињу** у опсегу,
  остали се остављају какви јесу и не рачунају се.
- **Звук се слуша једном за сваку наведену датотеку.** Да бисте ускладили више датотека на истом филму, рачунајте да ћете чекати
  сваку посебно.

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
  --method METHOD             How the subtitles are put in line: speech (from when there is speech, whatever the language) or words (from the words heard); the alignment.method setting by default
  --min-match PERCENT         Share of a subtitle's words that must be heard, in percent; the alignment.min-match setting by default
  --doubt-gap MS              How far a subtitle may move from its neighbours' shift before it is flagged, in milliseconds; the alignment.doubt-gap setting by default
  --report FILE               Write the subtitles to check to this file
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Аргументи и опције

| Опција | Обавезно | Вредности | Подразумевано |
| :----- | :------- | :----- | :------ |
| `<files>` | да | титл датотеке чије се време усклађује; са `-r`, директоријуми | — |
| `--recursive`, `-r` | не | опција без вредности | искључено — види [Обрада стабла директоријума](lots.md) |
| `--video` | једна од `--video` и `--audio` | филм који се слуша | — |
| `--audio` | једна од `--video` и `--audio` | звучна датотека која се слуша, уместо филма | — |
| `--audio-track` | не | цео број од 0: положај међу звучним нумерама филма | `0`, прва |
| `--language` | не | два слова, ISO 639-1 кôд: `en`, `fr` | поставка `transcription.language`; празно: језик се открива |
| `--start`, `--end` | не | време: `2:00`, `1:02:03`, `2:00.5` или број секунди; `--end` мора доћи после `--start` | цео звук |
| `--model` | не | датотека модела | поставка `transcription.model` |
| `--vad-model` | не | модел откривања говора, који тера мотор да прескочи тишине; са њим речи боље прате говор | поставка `transcription.vad-model` |
| `--engine` | не | назив мотора који слуша: `whisper.cpp` | поставка `transcription.engine` |
| `--keep-audio` | не | датотека у којој се задржава издвојени звук; њен директоријум се ствара ако не постоји | звук се нигде не задржава |
| `--method` | не | `speech`: полаже датотеку на звук према томе када има говора, на ком год језику; `words`: упарује речи са чутим, на језику којим се говори | поставка `alignment.method`, `speech` |
| `--min-match` | не | цео број од 1 до 100: удео речи титла које морају бити чуте, у процентима | поставка `alignment.min-match`, `50` |
| `--doubt-gap` | не | цео број милисекунди, 0 или више: за колико померање титла може одступати од померања његових суседа пре него што буде означен | поставка `alignment.doubt-gap`, `2000` |
| `--report` | не | датотека у коју се пишу титлови за проверу, један по реду: број титла, табулатор и разлог; једна титл датотека одједном | ништа се не пише |
| `--output` / `--output-dir` / `--in-place` | **једна од три**, осим уз `--dry-run` | види [Покретање](invocation.md#одредиште) | — |
| `--dry-run` | не | опција без вредности: израчуна и јави, ништа не пише — види [Прво видети, па писати](invocation.md#прво-видети-па-писати) | искључено |

Одредиште — `--output`, `--output-dir`, `--in-place`, `--dry-run` — и `-r` су они из осталих команди које
преписују датотеку: видите [одредиште](invocation.md#одредиште). Датотека задржава свој формат, своје кодирање и своје
крајеве редова.

**Поставке** се читају из датотеке поставки — видите [датотеку поставки](invocation.md#датотека-поставки) — и свака
има своју опцију овде. Мотор, модел и модел откривања говора су они од `transcribe`.

| Кључ | Значење | Подразумевано |
| :-- | :------ | :------ |
| `alignment.engine` | мотор усклађивања метода `words`; постоји један, `transcript-matching`, који слуша и упарује | `transcript-matching` |
| `alignment.method` | како се титлови усклађују: `speech` или `words` | `speech` |
| `alignment.min-match` | удео речи титла које морају бити чуте да би био пронађен, у процентима | `50` |
| `alignment.doubt-gap` | за колико, у милисекундама, померање титла може одступати од померања његових суседа пре него што буде означен | `2000` |

## Шта се означава

Команда каже које титлове треба проверити и зашто. Титл се означава због **првог** од ових разлога који важи:

| Зашто | Значење |
| :-- | :------ |
| није пронађен у звуку, остављен какав је био | чуто је мање од `--min-match` његових речи: задржава своје време |
| само делимично чут | пронађен, али је чуто мање од 80 % његових речи |
| померен другачије од суседа | пронађен, али његово померање одступа од средњег померања два титла са сваке стране за више од `--doubt-gap` |

Са `speech` сваки титл је постављен и може важити само последњи од ових разлога: где датотека промени своје померање за
више од `--doubt-gap` од једног титла до следећег (друга половина која касни), титлови са обе стране степенице
се означавају.

Титл такође може бити **скраћен до почетка следећег титла** или **се и даље преклапа са следећим титлом**.

Две бројке причају причу о датотеци у целини. **Медијана померања** је средина тога колико су се титлови померили:
датотека која је једноставно каснила чита се ту, и нигде другде. **Одступање** је медијана померања последње десетине
титлова умањена за ону прве десетине: близу нуле је за стално померање, а велико за датотеку која је каснила све
више како филм одмиче.

## Излаз

**Стандардни излаз** — ништа: резултат је записана датотека.

| Ниво | Шта се каже на излазу за грешке |
| :---- | :------------------------------- |
| 1 | `<file>: N of M subtitles placed on the audio, K to check; median shift X s, drift Y s -> <destination>` — и, када је брзина сличица исправљена, однос; са `speech`, **упозорење о ономе што не ради**, у посебном реду |
| 2 | корак и напредак, као код [`transcribe`](transcribe.md#излаз) |

### У JSON-у

Са [`--format json`](invocation.md#излаз-који-скрипта-може-да-чита) објекат датотеке носи `counts`: `subtitles`
(тражени), `placed` (који су добили место), `moved` (чије се време променило), `partial`, `out_of_line`, `not_found`,
`median_shift_ms` и `drift_ms`; и `flagged`, списак `{"subtitle": N, "kind": "…"}` где је број
место титла у датотеци, а `kind` је једно од `not-found`, `partial`, `out-of-line`, `cut-back` и `overlaps`.
Са `speech`, `warnings` носи један унос са `kind` `speech-only`. Идентификатори су загарантовани; реченице нису.

## Пример

```bash
subedit-cli sync --video film.mkv --vad-model silero.bin \
    --report to-check.txt --output film.synced.srt film.srt
```

Да бисте сваки титл поставили према његовим речима, за текст на језику којим се говори:

```bash
subedit-cli sync --method words --video film.mkv --model small.bin --vad-model silero.bin \
    --output film.synced.srt film.srt
```

## Излазни кодови

Они из програма: `0` ако је датотека записана, са означеним титловима или без њих, `2` ако није могла бити — алат или модел
недостаје, неуспело издвајање или слушање, датотека која не одговара звуку (ниједан њен титл није пронађен, или премало
заједничког са говором) —, `1` при грешци у употреби, као што је ни `--video`
ни `--audio` (или оба), опција ван свог опсега или опсег који се не може користити.

## Грешке

| Шта их изазива | Порука |
| :---- | :------ |
| ни `--video` ни `--audio`, или оба | `sync needs the audio to listen to: give --video or --audio, and not both` |
| програм мотора није пронађен, или конфигурисани не постоји | `the transcription engine was not found`, затим упута на водич |
| модел није дат, или не постоји | `no model was given, or the model file is not there`, затим упута на водич |
| `ffmpeg` није пронађен за филм | `ffmpeg was not found, so the audio of a video cannot be extracted`, затим упута |
| нумера коју филм нема | `the video has no audio track N`, затим нумере које има |
| језик који модел не може да транскрибује | `the model cannot transcribe the language xx` |
| мотор који нико није регистровао | `no transcription engine is called NAME`, или `there is no alignment engine called NAME` |
| ниједан титл није пронађен (`words`) | `none of the N subtitles was found in the audio: is it the right audio, and the right language?`, затим `The method based on speech does not depend on the language.` |
| модел откривања говора није наведен, или програм који налази говор није пронађен (`speech`) | `Finding where there is speech needs a voice-activity model: name one in the transcription.vad-model setting.` или `Finding where there is speech needs the program whisper-vad-speech-segments, which comes with whisper.cpp, and it was not found.`, затим упута на водич |
| титлови и говор имају премало заједничког (`speech`) | `The sound and the subtitles have too little in common to be put in line: is it the right film?` |
| `--min-match` ван 1 до 100, `--doubt-gap` негативан | грешка у употреби те опције |
| `--report` са више од једне титл датотеке | `--report names one file: give one subtitle file when you use it` |
