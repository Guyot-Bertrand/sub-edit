# `inspect`

<!-- machine-translated -->
> Ова страница је машински превод који још није прегледао читалац језика.

```
subedit-cli inspect [--frame-rate RATE] [--recursive] [--detail]
                   [-t FILE] [--align-method position|number] <file>...
```

Јавља од чега се свака датотека састоји. **Не мења ништа и не пише
ниједну датотеку.**

<!-- example: subedit-cli inspect --help -->
```console
$ subedit-cli inspect --help
Report what a subtitle file is made of
Usage: subedit-cli inspect [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to report on

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --detail                    List every anomaly, subtitle by subtitle, instead of counting them
  --frame-rate RATE           Frame rate of a file counted in frames: 25, 23.976
  -t,--translation-file FILE  Translation file to lay over the subtitle file, for a single input
  --align-method position|number
                              How the lines of the translation find their subtitles
```

## Аргументи

| Аргумент | Обавезно | Вредност | Подразумевано |
| :------- | :----- | :----- | :----- |
| `<file>...` | да | једна или више путања датотека титлова | — |
| `--recursive`, `-r` | не | опција без вредности | искључено — види [Обрада стабла директоријума](lots.md) |
| `--detail` | не | опција без вредности: именује сваку аномалију уместо да их броји — види [Шта јавља `anomalies`](#шта-јавља-anomalies) | искључено |

Изостанак путање је грешка у употреби, дакле код `1`.

## Опције

| Опција | Обавезно | Вредност | Подразумевано |
| :----- | :------ | :----- | :----- |
| `--frame-rate` | не | брзина у кадровима у секунди: `25`, `23.976`, `29.97` | 23.976 |

**Служи само за датотеке које се броје у кадровима**, то јест MicroDVD, једини
од девет. Датотека MicroDVD носи само бројеве кадрова; брзина
којом се читају одређује **сваки положај у извештају**,
почев од `span`. Задата другде, не мења ништа и није
грешка.

Вредност која ништа не именује је грешка у употреби, дакле код `1`:

<!-- example: subedit-cli inspect --frame-rate douze film.sub; echo $? -->
```console
$ subedit-cli inspect --frame-rate douze film.sub; echo $?
--frame-rate: "douze" is not a frame rate: expected frames per second, like 25 or 23.976
1
```

То је иста опција као код подкоманде [`convert`](convert.md), реч по реч, јер
је то иста брзина. `convert` је користи и да **пише**.

## Превод: `-t`

| Опција | Обавезно | Вредност | Подразумевано |
| :----- | :------ | :----- | :----- |
| `-t`, `--translation-file` | не | датотека превода, за **један једини** улаз | — |
| `--align-method` | не, и само са `-t` | `position` или `number` | `position` |

Са `-t`, превод се поставља на датотеку **као што то ради прозор при отварању** — види
[Превод](invocation.md#превод) — а извештај каже **како су његови редови нашли свој
титл**. Скрипта тако зна да ли је превод поравнат пре него што га исправља.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour.\n\n' > film.fr.srt; subedit-cli --quiet inspect -t film.fr.srt film.srt | tail -3 -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour.\n\n' > film.fr.srt; subedit-cli --quiet inspect -t film.fr.srt film.srt | tail -3
  anomalies: none
  translation file: film.fr.srt, matched by position
  translation: 1 line attached; 1 subtitle left without a translation
```

Први додати ред именује датотеку и метод; други је **реченица коју прозор
изговара** када отвара превод, сваки део само када није нула:

| Део | Шта броји |
| :----- | :---------------- |
| `N lines attached` | редови превода који су нашли титл главне датотеке |
| `N subtitles born of a line` | редови који нису нашли ниједан и родили су свој титл, на својим положајима |
| `N subtitles left without a translation` | титлови главне датотеке до којих није стигао ниједан ред |
| `N lines out of order` | редови који су у датотеци дошли после каснијег |

Превод који не носи ниједан ред каже `translation: the file holds no line`. **Извештај о самој
датотеци остаје извештај о самој датотеци**: титл рођен из реда не броји се у
`subtitles`.

Грешке опције `-t` су грешке једног читања, изречене са путањом превода, и ова:
`<translation>: the file is already open as the main document`, када је задата датотека главна.
Опција `--align-method` без `-t` је грешка у употреби, као и `-t` са више улаза — види
[Превод](invocation.md#превод).

## Излаз

Извештај иде на **стандардни излаз** — он је резултат команде и
пише се на сваком нивоу, укључујући `--quiet`. Један блок по датотеци, којем претходи
путања:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,500\nLe canot dérive.\n\n2\n00:00:04,000 --> 00:00:06,200\nPersonne n a vu la côte.\n' > exemple.srt; subedit-cli --quiet inspect exemple.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,500\nLe canot dérive.\n\n2\n00:00:04,000 --> 00:00:06,200\nPersonne n a vu la côte.\n' > exemple.srt; subedit-cli --quiet inspect exemple.srt
exemple.srt
  format: SubRip
  encoding: UTF-8, detected
  byte order mark: absent
  line endings: LF
  subtitles: 2
  span: 00:00:01.000 -> 00:00:06.200
  frame rate grid: none (too few subtitles to tell)
  anomalies: none
```

| Поље | Шта каже |
| :---- | :----------- |
| `format` | један од формата које прихвата `--to`, исписан у целини: `SubRip`, `WebVTT`, `SubViewer 2`, `Sub Station Alpha`, `Advanced SSA`, `MicroDVD`, `MPL2`, `TMPlayer`, `LRC` |
| `encoding` | прочитано кодирање, а за њим **одакле одговор долази**: `, from its byte order mark` ако га датотека декларише, `, as asked for` ако га је наметнуло `--encoding`, `, detected` ако је процењено |
| `byte order mark` | `present` или `absent` |
| `line endings` | `LF`, `CRLF` или `CR`, а за њим `, mixed from line N` ако их датотека меша |
| `subtitles` | број прочитаних титлова |
| `span` | од најранијег почетка до најкаснијег краја, `HH:MM:SS.mmm` |
| `frame rate grid` | брзина кадрова на којој су положаји израчунати, **закључена** — види испод |
| `frame rate` | за датотеку која се броји у кадровима, брзина којом је прочитана и **одакле долази** — замењује претходни ред, види испод |
| `anomalies` | `none` у истом реду, или шта није у реду: **једна врста у реду са својим бројем**, увучена испод `anomalies:`; са `--detail`, **једна аномалија у реду**, титл по титл |

**`span` није „од првог до последњег“**, већ од најранијег до најкаснијег:
у датотеци са поремећеним редоследом та два се разликују, а само друго казује
истину о ономе што датотека покрива.

**`anomalies`, са `--detail`, броји титлове од 1**, како их датотека
нумерише — а не редове датотеке. Број титла преживи
измену; број реда не. Види испод.

## Шта јавља `frame rate grid`

**Датотека титлова не декларише своју брзину кадрова.** SubRip нема
заглавље, а заглавље формата WebVTT је слободан текст. Овај ред зато ништа не чита:
он **закључује**, мерећи да ли положаји падају на мрежу кадрова.

Мере се **почеци**. Где год мрежа постоји, они су на њој
без изузетка, док се крајеви често удаљавају од ње — крај се
често израчунава правилом брзине читања, а не поставља на
кадар.

Први ред увек каже исто: изабрану брзину, пресуду
и концентрацију која је подржава.

| Пресуда | Шта значи |
| :------ | :----------------- |
| `clean` | положаји су на мрежи |
| `partial` | део њих јесте, остали нису |
| `none` | ниједан кандидат ништа не објашњава |

`none` има **два узрока, и то нису исти одговор**: или ниједна
мрежа не одговара — ред тада даје најбољег кандидата и његов резултат,
да се види за колико не успева — или датотека броји премало
титлова да би се о њој нешто рекло. Два почетка увек изгледају
савршено поравнато; то ништа не доказује.

**Правилна датотека на нестандардној брзини даје `none`, и то је
намера.** Скуп кандидата је затворен — осам брзина које
претварање нуди — а јављање најмање погрешне међу њима значило би давање
погрешног одговора тамо где је „не знам“ прави.

### Редови који следе, када имају шта да кажу

| Ред | Када се појављује |
| :---- | :------------------ |
| `grid offset` | положаји су на мрежи **до на константу**: датотека је померена, а овде је за колико |
| `also fits` | други кандидат одговара подједнако добро, јер је цео садржалац изабраног |
| `too short a span to separate` | распон датотеке не допушта да се изабрана брзина разликује од именованих |
| `off the grid` | колико се почетака удаљава од мреже, и у колико **низова** |

**`also fits` није оклевање.** Мрежа на 25 кадрова у секунди је
*укључена* у мрежу на 50: датотека усклађена на 25 зато одговара обема.
Обрнуто не важи — права 50 се сурва на 25 — па се бира нижа,
а друга се именује уместо да се прећути. Једини парови о којима је реч су
25 и 50, 29.97 и 59.94, 30 и 60.

**`too short a span to separate` чува од сувише самоувереног одговора.** 23.976
и 24 се разилазе за једну милисекунду по секунди филма: на дугометражном филму
се несумњиво раздвајају, на десет секунди се не могу разликовати. Ред каже
која остаје могућа.

**`off the grid` разликује две приче које сам проценат меша.**
Много низова од једног почетка су положаји исправљени ручно,
један по један. Мало дугих низова је поново усклађен одељак или датотека
састављена од две друге. Број низова је оно што их раздваја.

### Пример

Дванаест редова постављених на мрежу од 25 кадрова у секунди — дакле на
положаје који су умношци 40 милисекунди — распоређених на скоро девет минута:

<!-- example: k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > grille.srt; subedit-cli --quiet inspect grille.srt | tail -3 -->
```console
$ k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > grille.srt; subedit-cli --quiet inspect grille.srt | tail -3
  frame rate grid: 25 fps, clean (100.0%)
  also fits: 50 fps, of which this rate is a whole divisor
  anomalies: none
```

Пример прави сопствену датотеку уместо да чита једну из репозиторијума, и то је
намерно: може се поновити какав јесте, а петља показује шта је мрежа
боље него једна реченица — положаји који су умношци трајања једног кадра.

## Шта јавља `frame rate`, и зашто то није мрежа

**Само један од девет формата броји у кадровима**: MicroDVD, чији сваки ред
носи два броја кадрова и ништа друго. На таквој датотеци `inspect`
не пише `frame rate grid` него `frame rate`, а та два реда се
никада не виде заједно.

**Разлог стаје у једну реченицу.** Положаји датотеке MicroDVD
израчунати су *из* њених кадрова, на датој брзини; њихово закључивање из
мреже вратило би управо ту брзину, јер је тек задата. То би била
задата цифра, обучена у мерење.

Уместо тога пише се **брзина читања и одакле долази** —
у истом облику као ред `encoding`:

| Шта ред каже | Када |
| :------------------ | :---- |
| `frame rate: 25 fps, as asked for` | наметнула га је опција `--frame-rate` |
| `frame rate: 24000/1001 fps, assumed` | нико је није рекао; то је подразумевана |

**Два одговора, а не три**, где `encoding` има три: датотека MicroDVD
не декларише ништа. У томе је цео проблем — брзина нужно долази
споља, а извештај каже која је од две спољашњости.

**Брзина одређује сав остатак извештаја.** `span` пролази кроз њу, а два
читања исте датотеке на две брзине дају два распона:

<!-- example: printf '{25}{75}Première.\n{100}{150}Deuxième.\n{200}{260}Troisième.\n' > images.sub; subedit-cli --quiet inspect images.sub | tail -3; subedit-cli --quiet inspect --frame-rate 25 images.sub | tail -3 -->
```console
$ printf '{25}{75}Première.\n{100}{150}Deuxième.\n{200}{260}Troisième.\n' > images.sub; subedit-cli --quiet inspect images.sub | tail -3; subedit-cli --quiet inspect --frame-rate 25 images.sub | tail -3
  span: 00:00:01.043 -> 00:00:10.844
  frame rate: 24000/1001 fps, assumed
  anomalies: none
  span: 00:00:01.000 -> 00:00:10.400
  frame rate: 25 fps, as asked for
  anomalies: none
```

## Шта јавља `anomalies`

У документу три ствари могу бити погрешне, и свака се поправља другачије.
**Подразумевано `inspect` их броји**, један ред по врсти, редом из табеле:
лоше направљена датотека носи стотине њих, а знање колико их има од сваке врсте
говори шта треба покренути. **`--detail` их именује једну по једну.**

| Шта је исписано | Са `--detail` | Шта значи |
| :--------------- | :-------------- | :------------------ |
| `subtitle ends before it starts: N` | `subtitle N ends before it starts` | крај долази пре почетка |
| `subtitle starts before the previous one ends: N` | `subtitle N starts before the previous one ends` | преклапа се са претходним |
| `subtitle starts before the previous one starts: N` | `subtitle N starts before the previous one starts` | квари редослед датотеке — [`sort`](sort.md) га враћа на место |

Врста која изостаје не пише се. У левој колони `N` је број
титлова; у следећој, број једног титла.

**Исти титл може да се појави двапут**, и то је намера: онај који
почиње пре него што је претходни почео почиње и пре него што је
завршио. Прва тачка се решава померањем титла, друга
променом трајања.

**Именује се титл који је на погрешном месту**, а не онај у односу на који
је на погрешном месту. На почецима `0`, `4 s`, `2 s`, `3 s`: трећи квари редослед, а
четврти следи трећи и зато се не именује — ту се
нема шта учинити.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect desordre.srt | tail -3 -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect desordre.srt | tail -3
  anomalies:
    subtitle starts before the previous one ends: 1
    subtitle starts before the previous one starts: 1
```

А са `--detail`, именовани титл:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect --detail desordre.srt | tail -3 -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect --detail desordre.srt | tail -3
  anomalies:
    subtitle 3 starts before the previous one ends
    subtitle 3 starts before the previous one starts
```

**Један ред по аномалији са `--detail`**, јер сто налаза
слепљених у један ред не може да се ни прочита ни претражи. Документ без мане задржава
свој једини ред, `anomalies: none`, који скрипта тражи. **Запис
`--format json` увек набраја сваку аномалију**: скрипта чита појединости, а
`--detail` мења само текст.

**То није дијагностика читања.** Дијагностике казују на шта је
читање наишло и показују на **ред датотеке**; аномалије казују
шта документ *јесте* и показују на **титл**. Датотека са поремећеним
редоследом није зато неисправна: читање о томе нема шта да каже.

## У JSON-у

Са [`--format json`](invocation.md#излаз-који-скрипта-може-да-чита), `inspect` пише **један
објекат по датотеци** уместо горњих блокова; приповедање остаје на излазу грешака.
Кључеви описују шта текст каже, а **ниједан се не чита назад из реченице**:

| Кључ | Садржај |
| :-- | :------ |
| `format` | идентификатор формата, онај који прихвата `convert --to`: `srt`, `vtt`… |
| `encoding` | `{"charset", "origin", "byte_order_mark"}` — `origin` је `byte-order-mark`, `asked` или `detected` |
| `line_endings` | `{"kind", "mixed_from_line"}` — `lf`, `crlf` или `cr`; ред где се појављује друга врста, или `null` |
| `subtitles` | број титлова |
| `span_ms` | `{"start", "end"}` — први почетак и последњи крај, у милисекундама |
| `frame_rate` | за датотеку која се броји у кадровима: `{"rate", "origin"}` (`asked` или `assumed`); иначе `null` |
| `grid` | за остале: закључена мрежа, или `null` за датотеку која се броји у кадровима |
| `anomalies` | низ `{"subtitle", "kind"}` — `end-before-start`, `overlapping-subtitles`, `out-of-order` |
| `translation` | **само са `-t`**: `{"file", "method", "attached", "born", "untranslated", "out_of_order"}` — `method` је `position` или `number`, а четири броја су она из горње реченице |
| `warnings` | на шта је читање наишло, на сваком нивоу |

`grid` носи `verdict` (`clean`, `partial` или `silent`), `enough_starts`, `rate` (ниска,
или `null` када се ништа не именује), `concentration_permille` (хиљадити делови, `null` без довољно
почетака), `offset_ms`, `also_fits` (брзина чији је ова цео делилац, или
`null`), `not_separated` (брзине које сувише кратак распон не раздваја), `strays`
(почеци ван мреже) и `starts`.

<!-- example: printf '{25}{50}Un.\n{75}{100}Deux.\n' > f.sub; subedit-cli --format json inspect --frame-rate 25 f.sub 2>/dev/null -->
```console
$ printf '{25}{50}Un.\n{75}{100}Deux.\n' > f.sub; subedit-cli --format json inspect --frame-rate 25 f.sub 2>/dev/null
{"schema":1,"command":"inspect","file":"f.sub","ok":true,"format":"microdvd","encoding":{"charset":"UTF-8","origin":"detected","byte_order_mark":false},"line_endings":{"kind":"lf","mixed_from_line":null},"subtitles":2,"span_ms":{"start":1000,"end":4000},"frame_rate":{"rate":"25","origin":"asked"},"grid":null,"anomalies":[],"warnings":[]}
```

## Приповедање

На излазу грешака, према траженом нивоу:

| Ниво | Шта се додаје |
| :----- | :-------------- |
| 1 | `<path>: N subtitles` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings` |
| 3 | `<path>: N bytes read`, `<path>: N diagnostic(s) while reading`, затим **свака од њих** — види [Покретање](invocation.md#читање-дијагностике) |

## Излазни кодови

Они самог алата: `0` ако је свака датотека прочитана, `2` ако ниједна није, `3` ако су
само неке, `1` при грешци у употреби.

## Грешке

Оне једног читања — датотеке, и превода када је задато `-t` —, и
оне опције `-t`: види изнад. `inspect` не пише ништа што би могло бити одбијено.

| Шта је изазива | Порука |
| :------------------ | :------ |
| није задата путања | `files is required`, а за њим упућивање на `--help` |
| датотека недостаје | `<path>: does not exist` |
| нечитљива датотека | `<path>: cannot be opened: permission denied` |
| бајтови који се не декодирају | `<path>: cannot be decoded in the chosen encoding` |
| формат није препознат | `<path>: is in no format this tool knows` |
| ништа што личи на титл | `<path>: holds nothing recognisable as a subtitle` |

То су исте поруке за свих седамнаест подкоманди: рецепт за отварање
написан је једном, у језгру.

## Пример који пада

Празна датотека не одговара ниједном формату. Не јавља се као празна:
одбија се и именује.

<!-- example: : > vide.srt; subedit-cli inspect vide.srt; echo "code=$?" -->
```console
$ : > vide.srt; subedit-cli inspect vide.srt; echo "code=$?"
vide.srt: is in no format this tool knows
code=2
```
