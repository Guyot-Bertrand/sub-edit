# `correct`

<!-- machine-translated -->
> Ова страница је машински превод који још није прегледао читалац језика.

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

Исправља текстове по **Gaupol-овим обрасцима**, као асистент
[`Correct Texts…`](../subedit-gui/correct-texts.md) у прозору — исти обрасци,
исти мотор, исти бројеви. Овде постоји шест задатака: **напомене за
наглуве**, **спајање** и **дељење речи**, **уобичајене грешке**,
**писање великим словом** и **прелом редова**.

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

## Шта се бира, а шта не

**Ништа није унапред означено.** Прозор отвара своја поља онако како их је корисник оставио; командна
линија задатке, код, дужину реда и језик узима из своје линије,
никада из датотеке поставки: ни задаци означени једног дана у прозору, ни поља
образаца пребачена тамо (`correction.activations`), ни списак замена провере писања
се не читају, и ништа се не пише на тој страни.

**Четири поставке подразумевано долазе из датотеке поставки** — вредности које `Correct Texts…`
памти, или које је записала подкоманда [`config set`](config.md). **Опција у линији увек побеђује**, затим
датотека, затим уграђена подразумевана вредност; види [Датотека поставки](invocation.md#датотека-поставки)
да видите где се она налази, за `--config` и `--no-config`.

| Кључ | Шта даје | Опција која га надјачава |
| :-- | :--------------- | :----------------------- |
| `correction.human`, `correction.ocr` | класе примењених уобичајених грешака | `--classes` |
| `correction.remove-blank` | да ли се титлови које исправка испразни уклањају | `--keep-blank-subtitles`, `--remove-blank-subtitles` |
| `correction.line-break.max-lines` | највећи број редова задатка `line-break` | `--max-lines` |

- **За сваку од четири постоји опција која је поништава**, шта год датотека каже: `--classes human,ocr`
  примењује обе класе, `--remove-blank-subtitles` уклања испражњене титлове, `--max-lines 3`
  враћа Gaupol-ове редове. `--keep-blank-subtitles` и `--remove-blank-subtitles` заједно су
  грешка у употреби.
- **Обе класе искључене у датотеци се занемарују, и то се јавља** на нивоу 1 —
  `settings: correction.human and correction.ocr are both false, ignored` —: `--classes` то не би могла да запише, а
  задатку common-errors не би остало ништа да ради. Уместо тога се примењују обе класе.
- **Број редова ван распона од 1 до 1000 се из датотеке уопште не чита**: читач датотеке
  га одбија — `settings: correction.line-break.max-lines: cannot read 'N', default kept`, на
  нивоу 1 — и користи се уграђених 3.

Примери на овој страници извршени су са `--no-config`, да би показали шта раде уграђене
подразумеване вредности, без обзира на поставке машине која их извршава.

| Опција | Обавезно | Вредности | Подразумевано |
| :----- | :----- | :------ | :----- |
| `<file>...` | да | једна или више путања | — |
| `--tasks` | **да** | списак раздвојен зарезима: `mentions`, `join-words`, `split-words`, `common-errors`, `capitalization`, `line-break` | ниједан задатак |
| `--code` | **да** уз задатак са обрасцима, а иначе **није применљиво** | `Script[-language[-COUNTRY]]`: `Zyyy`, `Latn`, `Latn-en`, `Latn-en-US` | — |
| `--language` | **да** уз `join-words` или `split-words`, а иначе **није применљиво** | код језика: `fr`, `fr_FR`, `en_US` | — |
| `--classes` | не | `human`, `ocr` или `human,ocr` | обе — или оно што каже датотека поставки |
| `--enable`, `--disable` | не, понављају се | енглески назив образца, или `type:name` | испоручене датотеке `.conf` |
| `--max-length` | **да** уз `line-break` | број **знакова** већи од нуле | **нема** — види испод |
| `--max-lines` | не | цео број редова, већи од нуле | 3 — или оно што каже датотека поставки |
| `--skip-length`, `--skip-lines` | не | број, или `off` | исте границе као `--max-length` и `--max-lines` |
| `--keep-blank-subtitles` | не | опција без вредности: испражњени титлови остају, празни | испражњени титлови се уклањају — осим ако датотека поставки не каже другачије |
| `--remove-blank-subtitles` | не | опција без вредности: испражњени титлови се уклањају, шта год датотека поставки каже; одбија се уз `--keep-blank-subtitles` | искључено |
| `--range` | не | `N-M`, или `N-` до краја — види [`adjust`](adjust.md#--range) | цела датотека |
| `--document` | не | `main` или `translation` — види [Превод](invocation.md#превод) | `main` |
| `-t`, `--translation-file` | уз `--document translation` | датотека превода, за **један једини** улаз | — |
| `--align-method` | не, и само уз `-t` | `position` или `number` | `position` |
| `--recursive`, `-r` | не | опција без вредности | искључено — види [Обрада стабла директоријума](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **једна од три**, осим уз `--dry-run` | види [Покретање](invocation.md#одредиште) | — |
| `--dry-run` | не | опција без вредности: израчуна и каже, не пише ништа — види [Прво видети, па писати](invocation.md#прво-видети-па-писати) | искључено |

### Задаци

**Редослед на списку нема значаја**: задаци се примењују Gaupol-овим
редом — напомене, затим спајање и дељење речи, затим уобичајене грешке, затим писање великим словом, затим прелом редова —, сваки над
текстом који је претходни оставио. Титл који су напомене испразниле не учествује
у ономе што следи.

| Задатак | Шта ради |
| :---- | :-------------- |
| `mentions` | уклања звукове између угластих заграда, и између обичних заграда када је укључено `Sound in parentheses` (пролаз подкоманде [`hearing-impaired`](hearing-impaired.md)), затим обрасце за текстове песама и говорнике |
| `join-words` | поново спаја реч пресечену на два дела, према речнику језика `--language` |
| `split-words` | дели две слепљене речи, према истом речнику |
| `common-errors` | исправља уобичајене грешке: размаке, интерпункцију, лигатуре, грешке OCR-а |
| `capitalization` | ставља велико слово тамо где језик то тражи |
| `line-break` | поново сече редове титла тако да ниједан не прелази `--max-length`, одмеравајући обрасце прелома кода |

### Код образаца

**`--code` именује каскаду**: `Zyyy` за свако писмо, затим писмо, језик
и земља — `Latn-en-US` чита обрасце из `Zyyy`, из `Latn`, из `Latn-en` и из
`Latn-en-US`. **Обавезан је**, јер командна линија нема околни језик:
универзална подразумевана вредност примењивала би *мање* него што се верује, а да то не каже. Важи за
сваки задатак истог извршавања; задатак који жели свој код је друго
извршавање. Лоше написан код је грешка у употреби:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code francais --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code francais --output b.srt a.srt; echo "code=$?"
--code: "francais" is not a pattern code: expected Script[-language[-COUNTRY]], like Zyyy, Latn or Latn-en-US
code=1
```

Обрасци које корисник ставља у `$XDG_DATA_HOME/subedit/patterns` (или
`~/.local/share/subedit/patterns`) **додају се испорученим обрасцима**, као у
прозору, а испоручени образац који не може да се прочита јавља се на нивоу 1: `patterns: <file>, line N (<reason>)`.

### Спајање и дељење речи, и речник

**Ова два задатка не читају ниједан образац**: питају **системски речник, преко
Enchant-а** — зависности пакета (`libenchant-2-2` на Debian-у и Ubuntu-у, `enchant2` на
Fedora-и), коју `subedit-cli` дели са прозором. Зато немају ни код ни назив које би требало поставити:
`--code` нема смисла без задатка са обрасцима, а `--language` нема смисла без једног од ова два
— једно дато без другог је грешка у употреби, код `1`.

- **`join-words`** спаја реч коју речник не познаје са оном која јој претходи или
  следи, **када само један смер даје познату реч**: `hel lo` постаје `hello`; два могућа
  смера, или ниједан, не одлучују ништа.
- **`split-words`** дели реч коју речник не познаје **када је тачно један од њених
  предлога та реч са размаком**: `hellothere` постаје `hello there`. Реч са почетним
  великим словом иза ког следе мала слова се оставља: то је најчешће име.

**Без речника за језик, извршавање се одбија**, код `1`, **пре него што се датотека
прочита** и речима прозора, који функцију засивљује и то каже — овде је командна линија
одбија и то каже, уместо да ћути:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nhel lo\n\n' > a.srt; subedit-cli --no-config correct --tasks join-words --language qq_QQ --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nhel lo\n\n' > a.srt; subedit-cli --no-config correct --tasks join-words --language qq_QQ --output b.srt a.srt; echo "code=$?"
no dictionary for qq_QQ
code=1
```

**Списак замена прозора се не чита нити пише.** Он се *пуни* изборима
корисника у провери писања; командна линија нема такав
избор, а читање списка учинило би да скрипта зависи од онога што је њен аутор једном у њега ставио.

### Прелом редова

**Мери се у знаковима, и то је разлика у односу на прозор.** Прозор мери у *емовима* —
ширини калибрисаној према фонту, при 0,55 ем по слову —, а његових подразумеваних 24 је вредност
у емовима. Командна линија нема фонт: она броји знакове, оне из
текста. **Преписивање тих 24 као знакова пресекло би скоро све, а да то не
каже**; зато **не постоји подразумевана дужина**, а `--max-length` је обавезно уз
`line-break`:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn texte\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn texte\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --output b.srt a.srt; echo "code=$?"
--max-length is required by the task line-break: Gaupol's 24 is a width in ems, and has no value in characters
code=1
```

**`--max-lines` је 3**, вредност програма Gaupol, коју јединица не мења; прелом редова узима
један више када три нису довољна. **Прескакање** — титл који је већ у границама
оставља се какав јесте — подразумевано узима **исте границе** (`--skip-length` = `--max-length`,
`--skip-lines` = `--max-lines`), као Gaupol; `--skip-length off` и `--skip-lines off`
га искључују, а текст се тада поново чита као један и поново сече:

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

Обрасци прелома су они кода: `Latn` не прелама између броја и његове јединице нити
иза цртице разговора, `Latn-en` не прелама иза члана, предлога, титуле
или присвојне заменице. **Под `Zyyy` не постоји ниједан образац прелома**: редови се
само уравнотежују. **Овај задатак се никада не одбија због недостатка активног образца**: његови обрасци
одмеравају где ред треба прекинути, а прелом се дешава и без њих. Ове четири опције имају смисла само уз
`line-break`; дате без њега, оне су грешка у употреби.

### Класе и укључивање по називу

**`--classes` филтрира уобичајене грешке заувек**: искључивање `ocr` уклања његове
обрасце из примене — лигатура `ﬁ` тада остаје `ﬁ`. Остала три типа образаца
не носе класу.

**`--enable` и `--disable` постављају образац по његовом енглеском називу**, назив је
кључ (ADR 0037); један назив може означавати више записа, који тада деле једно
поље. То је потребно, и тиме се издвајају подразумеване вредности прозора: **испоручене
датотеке `.conf` искључују обрасце напомена**, тако да `--tasks mentions` само
не би применило ништа. Два обрасца „Sound in brackets“ и „Sound in parentheses“
сваки управља својим граничником — угластим заградама, обичним заградама — и постављају се истим
опцијама: укључивање једног не уклања други.

Три ствари су **грешке у употреби**, код `1`, јављене пре читања датотеке:

| Шта их изазива | Порука |
| :------------------ | :------ |
| назив који не означава ниједан образац задатака датих под датим кодом | `--enable: "<name>" names no pattern of the tasks given, under the code <code>` |
| назив који означава обрасце два типа — тада се пише `type:name`, тип је `common-error`, `capitalization`, `hearing-impaired` или `line-break` | `--enable: "<name>" names patterns of several types: write type:name, …` |
| назив дат и уз `--enable` и уз `--disable` | `"<name>" is given to --enable and to --disable: choose one` |

**Грешка у куцању је гласна**: прозор занемарује застарелу поставку јер
поставка може да надживи образац који је нестао; позив нема преживелих.

**Задатак без иједног активног образца се одбија**, код `1` — „нема шта да се ради“ је
одговор који се изговара, а не тихи успех:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --output b.srt a.srt; echo "code=$?"
mentions: no pattern is active under the code Latn; switch one on with --enable
code=1
```

Иста датотека, са укљученим пролазом кроз угласте заграде:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --enable "Sound in brackets" --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --enable "Sound in brackets" --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:02,000
Bonjour
```

## Шта се пише, а шта се каже

Формат, кодирање и завршеци редова прочитане датотеке се **задржавају**. **Титлови
које исправка испразни се уклањају**, као подразумевано поље прозора;
`--keep-blank-subtitles` их оставља празне, а `--remove-blank-subtitles` их уклања
чак и када их датотека поставки задржава. **Испражњен превод остаје испражњен**
и задржава свој титл.

**Бројање је оно из прозора** — „Edited N and removed M subtitles“: измењени
текстови и уклоњени титлови, **никада поклапања**. Датотека у којој се ништа не
мења ипак се записује: задато одредиште је записано одредиште.

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

### Образац који се не може применити

**Образац који не може да се прочита, не може да се преведе или се не завршава именује се, са
титлом на ком је одустао**, и ту командна линија каже више од
прозора: страница потврде именује образац само једном, без текста.

| Разлог | Шта значи |
| :----- | :------------------- |
| `cannot be translated` | његов израз користи оно што превођење у ICU одбија |
| `will not compile` | мотор је одбио његов израз |
| `has an invalid replacement` | његова замена се не чита онако како је чита Python |
| `timed out` | мотор је одустао од тог текста |
| `never settled` | `Repeat` је и после сто пролаза још мењао текст |
| `grew the text too long` | текст је прешао оно што титл може да садржи |

На нивоу 1, на излазу за грешке: 
`<path>: pattern "<name>" (<reason>) was not applied to subtitle N` — без краја када се образац издваја за сваки текст. **Број је
број у датотеци**, ма који задаци уклонили титлове пре њега. У JSON-у то је
упозорење `pattern-failed` датотеке, чији `detail` носи реченицу. **Образац од ког се
одустало није неуспела датотека**: датотека се записује, а излазни код је `0`.

## Излаз

**Стандардни излаз** — ништа, јер је резултат записана датотека. **Уз `--dry-run` то су
предложене измене**, један блок по титлу: његов број, текст пре, текст после —
и `(removed)` без реда `+` за титл који би исправка уклонила; уз
`--keep-blank-subtitles` он остаје, празан. Види [Прво видети, па писати](invocation.md#прво-видети-па-писати).

| Ниво | Шта се додаје на излаз за грешке |
| :----- | :------------------------------------- |
| 1 | `<path>: Edited N and removed M subtitles -> <destination>`, обрасци који нису могли да се примене, датотеке образаца које не могу да се прочитају — и, прво, прочитана датотека поставки — `settings: read <path>` — и оно од ње што је занемарено |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, затим свака дијагностика читања |

У JSON-у, `counts` носи `corrected` (измењене текстове) и `removed` (уклоњене
титлове), а `changes` дотакнуте титлове. Уз `-t`, кључ `alignment` каже како је
превод положен.
`settings` именује прочитану датотеку поставки, или је `null` када ниједна није прочитана.

## Излазни кодови

Они самог алата: `0` ако је свака датотека записана, `2` ако ниједна, `3` ако само
неке, `1` код грешке у употреби.

## Грешке

| Шта је изазива | Шта се пише на излаз за грешке |
| :------------------ | :--------------------------------------- |
| датотека коју наводи `--config` а не постоји — корисникова сопствена датотека, одсутна, даје подразумеване вредности | `settings file not found: <FILE>` |
| недостаје `--tasks` | `--tasks is required`, а за њим упућивање на `--help` |
| непознат задатак | `--tasks: "<name>" is not a task: expected mentions, join-words, split-words, common-errors, capitalization or line-break` |
| недостаје `--code` | `--code is required by the tasks that read patterns: <tasks>` |
| `--code` без задатка са обрасцима | `--code is for the tasks that read patterns, and none was asked for` |
| недостаје `--language` уз `join-words` или `split-words` | `--language is required by the tasks that check words: <tasks>` |
| `--language` који није код језика | `--language: "<value>" is not a language: expected a locale code, like fr, en_US or sr@Latn` |
| `--language` без једног од та два задатка | `--language is for the tasks join-words and split-words, which were not asked for` |
| језик без речника | `no dictionary for <language>` |
| лоше написан код | `--code: "<code>" is not a pattern code: expected Script[-language[-COUNTRY]], like Zyyy, Latn or Latn-en-US` |
| недостаје `--max-length` уз `line-break` | `--max-length is required by the task line-break: Gaupol's 24 is a width in ems, and has no value in characters` |
| `--max-length` који није број већи од нуле | `--max-length: "<value>" is not a length: expected a number of characters greater than zero` |
| `--max-lines`, `--skip-lines` који нису цео број редова | `--max-lines: "<value>" is not a number of lines: expected a whole number greater than zero` |
| нечитљив `--skip-length` или `--skip-lines` | `--skip-length: "<value>" is not a bound: expected a number greater than zero, or off` |
| једна од четири опције без `line-break` | `--max-length is for the task line-break, which was not asked for` |
| непозната класа | `--classes: "<name>" is not a class: expected human, ocr or human,ocr` |
| `--keep-blank-subtitles` уз `--remove-blank-subtitles` | `--keep-blank-subtitles and --remove-blank-subtitles say opposite things; give one of them` |
| погрешан назив образца | види табелу изнад |
| задатак без активног образца | `<task>: no pattern is active under the code <code>; switch one on with --enable` |
| `--range`, `-t`, `--document`, одредиште | они из одељка [Покретање](invocation.md#грешке) и [Превод](invocation.md#превод) |
