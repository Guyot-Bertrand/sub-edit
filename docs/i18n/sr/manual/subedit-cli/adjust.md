# `adjust`

<!-- machine-translated -->
> Ова страница је машински превод који још није прегледао читалац језика.

```
subedit-cli adjust [--speed CPS|off] [--shorten | --no-shorten] [--lengthen | --no-lengthen]
                   [--minimum TIME|off] [--maximum TIME|off] [--gap TIME|off]
                   [--range N-M|N-]
                   (--output FILE | --output-dir DIR | --in-place)
                   [--dry-run]
                   [--recursive]
                   <file>...
```

Подешава **трајање** сваког титла према брзини читања и граничним вредностима:
титл прекратак да би се прочитао се продужава, титл који предуго остаје на
екрану може да се скрати, а два титла се не преклапају. **Помера се само
крај**: померање почетка би померило титл, а то је посао подкоманде
[`shift`](shift.md). То је операција `Adjust Durations…` у
[прозору](../subedit-gui/operations.md#adjust-durations), са истим подразумеваним вредностима.

## Четири ограничења и њихов редослед

Примењују се **овим редом**, свако поставља крај, а последње побеђује:

1. **брзина читања** — трајање које тексту треба при датом броју знакова у
   секунди (ознаке се не рачунају, прелом реда се рачуна);
2. **најкраће трајање** — трајање испод њега подиже се на најкраће;
3. **најдуже трајање** — трајање изнад њега спушта се на најдуже;
4. **размак** — време остављено до следећег титла. Долази последњи и
   **он побеђује**: два титла приказана заједно су грешка коју гледалац види,
   а титл који је мало прекратак није.

Последњи титл у датотеци нема следећег, па нема ни размака који треба чувати. Следећи
титл је онај који је у датотеци следећи, **без обзира на то да ли га је опција `--range` изабрала**.

| Ограничење | Опција | Подразумевано | Искључивање |
| :--------- | :----- | :----- | :------- |
| брзина | `--speed CPS` | **15** знакова у секунди | `--speed off` |
| смер брзине | `--shorten`, `--no-shorten`, `--lengthen`, `--no-lengthen` | само **продужава** | — |
| најкраће трајање | `--minimum TIME` | **1,5 s** | `--minimum off` |
| најдуже трајање | `--maximum TIME` | **нема** | `--maximum off` |
| размак | `--gap TIME` | **0** | `--gap off` |

То су поставке програма Gaupol и првог отварања прозора. **`off` искључује
ограничење: оно се више не проверава.** То није постављање на нулу — најкраће трајање `0`
је најкраће трајање нула, размак `0` забрањује свако преклапање.

## Шта даје датотека поставки

**Подразумеване вредности изнад су уграђене.** Подкоманда `adjust` подразумевано чита поставке операције из
датотеке поставки — вредности које `Adjust Durations…` памти у прозору, или које је
записала подкоманда [`config set`](config.md) — и оне замењују уграђене подразумеване вредности. **Опција
дата у линији увек побеђује**, затим датотека, затим уграђена подразумевана вредност. Погледајте
[Датотека поставки](invocation.md#датотека-поставки) да видите где се датотека налази, за `--config` и
`--no-config`.

| Кључ | Шта даје | Опција која га надјачава |
| :-- | :--------------- | :----------------------- |
| `duration-adjust.speed` | брзину читања, у знаковима у секунди | `--speed CPS` |
| `duration-adjust.lengthen`, `duration-adjust.shorten` | у ком смеру брзина може да помера крај; ако су оба `false`, брзина је искључена | `--lengthen`, `--no-lengthen`, `--shorten`, `--no-shorten`, `--speed off`; задавање `--speed CPS` или смера поново укључује брзину коју је датотека искључила, уз продужавање |
| `duration-adjust.minimum-enabled`, `duration-adjust.minimum-ms` | најкраће трајање, или ниједно | `--minimum TIME`, `--minimum off` |
| `duration-adjust.maximum-enabled`, `duration-adjust.maximum-ms` | најдуже трајање, или ниједно | `--maximum TIME`, `--maximum off` |
| `duration-adjust.gap-enabled`, `duration-adjust.gap-ms` | најмањи размак, или ниједан | `--gap TIME`, `--gap off` |

**За сваки кључ постоји опција која га поништава.** За сваки смер постоји пар опција који га поставља
шта год датотека каже: `--shorten` и `--no-shorten` за скраћивање, `--lengthen` и
`--no-lengthen` за продужавање; опција која није дата оставља смер из датотеке какав јесте.
`--maximum off` одбацује најдуже трајање које датотека укључује. Уз `--format json`, `constraints` каже
поставке које су заиста коришћене, ма откуда долазиле, а `settings` датотеку која је прочитана:

<!-- example: printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine/settings.conf config set duration-adjust.speed 12 2>/dev/null; subedit-cli --config mine/settings.conf --format json adjust --dry-run a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine/settings.conf config set duration-adjust.speed 12 2>/dev/null; subedit-cli --config mine/settings.conf --format json adjust --dry-run a.srt
settings: read mine/settings.conf
a.srt: adjusted the durations of 1 subtitle (dry run, nothing written)
{"schema":1,"command":"adjust","file":"a.srt","ok":true,"dry_run":true,"destination":null,"counts":{"subtitles":1,"adjusted":1,"sacrificed":{"speed":0,"minimum":0,"gap":0}},"constraints":{"speed":{"cps":"12","lengthen":true,"shorten":false},"minimum_ms":1500,"maximum_ms":null,"gap_ms":0},"settings":"mine/settings.conf","warnings":[]}
```

Остали примери на овој страници извршени су са `--no-config`, да би показали шта раде уграђене
подразумеване вредности, без обзира на поставке машине која их извршава.

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

## Аргументи и опције

| Опција | Обавезно | Вредности | Подразумевано |
| :----- | :----- | :------ | :----- |
| `<file>...` | да | једна или више путања | — |
| `--recursive`, `-r` | не | опција без вредности | искључено — види [Обрада стабла директоријума](lots.md) |
| `--speed` | не | број знакова у секунди, **већи од нуле** (`15`, `12.5`), или `off` | `15` |
| `--shorten` | не | опција без вредности: брзина може и да **помери** крај **раније** | искључено |
| `--no-shorten` | не | опција без вредности: брзина више не **помера** крај **раније**, шта год датотека поставки каже | искључено |
| `--lengthen` | не | опција без вредности: брзина **помера** крај **касније**, шта год датотека поставки каже | искључено |
| `--no-lengthen` | не | опција без вредности: брзина више не **помера** крај **касније** | искључено |
| `--minimum` | не | трајање од нула или више, или `off` | `1.5` |
| `--maximum` | не | трајање **веће од нуле**, или `off` | нема |
| `--gap` | не | трајање од нула или више, или `off` | `0` |
| `--range` | не | `N-M`, или `N-` до краја — види испод | цела датотека |
| `--output` / `--output-dir` / `--in-place` | **једна од три**, осим уз `--dry-run` | види [Покретање](invocation.md#одредиште) | — |
| `--dry-run` | не | опција без вредности: израчуна и каже, не пише ништа — види [Прво видети, па писати](invocation.md#прво-видети-па-писати) | искључено |

Трајање се пише као код подкоманде [`shift`](shift.md#писање-трајања): `2.5`, `00:00:02.500`.

**Без иједног активног ограничења, `adjust` се одбија** (код `1`, ништа се не пише): не би
било чему да се подеси, а команда која ништа не ради а каже да је успела је најгори
одговор. То је случај за `--speed off --minimum off --gap off` без најдужег трајања, и за
само `--no-lengthen` са искљученим најкраћим трајањем и размаком: брзина која не помера ниједан
крај не ограничава ништа.

`--shorten`, `--no-shorten`, `--lengthen` и `--no-lengthen` кажу у ком смеру брзина помера
крај; имају смисла само уз брзину, а **`--speed off` их одбија**. Опција дата са својом
супротном — `--shorten --no-shorten`, `--lengthen --no-lengthen` — такође се одбија.

### `--range`

`--range 3-7` ограничава подешавање на титлове од 3 до 7, **бројане од 1, границе
укључене**; `--range 3-` иде до последњег. Један титл се пише `4-4`.

- Интервал **који се завршава пре него што почне**, `0`, знак или усамљен број
  одбијају се као грешка у употреби (код `1`).
- **Важи за сваку датотеку серије**, што има смисла само ако су датотеке
  сличне. Процењује се **према свакој датотеци**: интервал који прелази ту датотеку
  одбија се **пре него што се дотакне**, уз навођење границе —
  `range 3-9 ends after the last subtitle: the file has 4` —, та датотека пада (код `2` или `3`, идентификатор
  `range-out-of-bounds`), а остале се настављају.
- Бројеви у извештају односе се само на изабране титлове.

## Шта се жртвује

Два ограничења могу да се сукобе на истом титлу: најкраће трајање од 1,5 s не стаје
у 0,8 s које следећи титл оставља. **Редослед одлучује, а извештај каже
шта је попустило** — што Gaupol не ради: он их крши тихо.

Свако ограничење се наводи само ако га **бар један титл није добио**, пошто
су остала примењена. Најдуже трајање нема број: примењује се пре размака, који може
само да скрати, па увек важи. Најкраће трајање веће од најдужег попушта пред најдужим и
рачуна се као најкраће трајање.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --output ajuste.srt courts.srt; grep -e '-->' ajuste.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --output ajuste.srt courts.srt; grep -e '-->' ajuste.srt
courts.srt: adjusted the durations of 3 subtitles; could not satisfy the minimum duration in 1 subtitle -> ajuste.srt
00:00:00,000 --> 00:00:02,000
00:00:10,000 --> 00:00:14,000
00:00:14,200 --> 00:00:15,000
00:00:15,000 --> 00:00:16,500
```

Трећи титл би желео да се заврши у 15,7 s да би трајао 1,5 s, али четврти почиње у
15,0 s: **размак побеђује**, завршава се у 15,0 s, а извештај наводи најкраће трајање.

Искључивањем размака најкраће трајање побеђује — и два титла се преклапају:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --gap off --output libre.srt courts.srt; grep -e '-->' libre.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --gap off --output libre.srt courts.srt; grep -e '-->' libre.srt
courts.srt: adjusted the durations of 3 subtitles -> libre.srt
00:00:00,000 --> 00:00:02,000
00:00:10,000 --> 00:00:14,000
00:00:14,200 --> 00:00:15,700
00:00:15,000 --> 00:00:16,500
```

А ништа се не жртвује нити подешава када датотека већ стаје у своје границе:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --speed off --minimum off --output intact.srt courts.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --speed off --minimum off --output intact.srt courts.srt
courts.srt: no duration to adjust -> intact.srt
```

Датотека се записује каква јесте: **задато одредиште је записано одредиште**.

## Излаз

**Стандардни излаз** — ништа: резултат је записана датотека.

| Ниво | Шта се додаје на излаз за грешке |
| :----- | :------------------------------------- |
| 1 | `<path>: adjusted the durations of N subtitles; could not satisfy …  -> <destination>`, или `<path>: no duration to adjust -> <destination>` — и, прво, прочитана датотека поставки — `settings: read <path>` — види [Датотека поставки](invocation.md#датотека-поставки) |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — формат, кодирање, ознака и завршеци редова прочитане датотеке, враћени какви су били |
| 3 | `<path>: N bytes read, M written`, затим **свака дијагностика читања** — види [Покретање](invocation.md#читање-дијагностике) |

Реченица је реченица прозора: `N` је број титлова чији се крај померио, а
остатак, одсутан када ништа није попустило, набраја шта није задржано — `the reading speed`,
`the minimum duration`, `the gap` — са бројем титлова на које се односи.

### У JSON-у

Уз [`--format json`](invocation.md#излаз-који-скрипта-може-да-чита), објекат сваке датотеке
носи, поред омотача:

| Кључ | Садржај |
| :-- | :------ |
| `counts.subtitles` | титлови на које се **циља**: цела датотека, или интервал |
| `counts.adjusted` | они чији се крај померио |
| `counts.sacrificed.speed`, `.minimum`, `.gap` | титлови који и даље не задовољавају ово ограничење, пошто су остала примењена, **укључујући оне чији се крај није померио** |
| `constraints.speed` | `{"cps", "lengthen", "shorten"}` — знакови у секунди **као ниска** (`"15"`, `"12.5"`) — или `null` ако је искључено |
| `constraints.minimum_ms`, `.maximum_ms`, `.gap_ms` | коришћена трајања, у **целим милисекундама**, или `null` ако је искључено |
| `settings` | прочитана датотека поставки, или `null` када ниједна није прочитана |

`constraints` каже **поставке које су заиста коришћене**, укључујући подразумеване: два рачунања
над истим титловима тада се пореде без читања командне линије.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config --format json adjust --dry-run --maximum 1.8 courts.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config --format json adjust --dry-run --maximum 1.8 courts.srt
courts.srt: adjusted the durations of 4 subtitles; could not satisfy the reading speed in 1 subtitle, the minimum duration in 1 subtitle (dry run, nothing written)
{"schema":1,"command":"adjust","file":"courts.srt","ok":true,"dry_run":true,"destination":null,"counts":{"subtitles":4,"adjusted":4,"sacrificed":{"speed":1,"minimum":1,"gap":0}},"constraints":{"speed":{"cps":"15","lengthen":true,"shorten":false},"minimum_ms":1500,"maximum_ms":1800,"gap_ms":0},"settings":null,"warnings":[]}
```

## Излазни кодови

Они самог алата: `0` ако је свака датотека обрађена, `2` ако ниједна, `3` ако само
неке, `1` код грешке у употреби. Датотека која већ стаје у своје границе се обрађује.

## Грешке

| Шта је изазива | Порука |
| :------------------ | :------ |
| датотека коју наводи `--config` а не постоји — корисникова сопствена датотека, одсутна, даје подразумеване вредности | `settings file not found: <FILE>` |
| нема активног ограничења | `adjust has nothing to adjust to: every constraint is off, or the reading speed is told to move no end` |
| брзина која то није | `--speed: "…" is not a reading speed: expected a number of characters per second above zero, like 15 or 12.5, or off` |
| опција смера уз `--speed off` | `--lengthen, --no-lengthen, --shorten and --no-shorten say which way the reading speed may move an end, and --speed off switches the reading speed off` |
| опција смера са својом супротном | `--shorten and --no-shorten say opposite things; give one of them` (исто тако `--lengthen` и `--no-lengthen`) |
| трајање које то није | `--minimum: "…" is not a time: …` (исто тако `--maximum`, `--gap`) |
| негативно трајање | `--minimum: "-1" is not a duration of zero or more` (исто тако `--gap`) |
| нула или негативно најдуже трајање | `--maximum: "0" is not a duration above zero` |
| лоше написан интервал | `--range: "…" is not a range: write it N-M, or N- to go to the end, counted from 1 and inclusive` |
| интервал који се завршава пре почетка | `--range: "5-3" is not a range: it ends before it starts` |
| интервал изван датотеке | `<path>: range N-M starts after the last subtitle: the file has K`, или `… ends after …` |

Грешке одредишта заједничке су подкомандама које пишу:
види [Покретање](invocation.md#одредиште).
