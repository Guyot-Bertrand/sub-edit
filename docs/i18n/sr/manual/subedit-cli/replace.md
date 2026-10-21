# `replace`

<!-- machine-translated -->
> Ова страница је машински превод који још није прегледао читалац језика.

```
subedit-cli replace [--regex] [--case-sensitive] [--range N-M|N-]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    <pattern> <replacement> <file>...
```

Замењује текст у титловима **не ломећи ниједну ознаку**. То је операција
`Replace All` из [претраге](../subedit-gui/find-and-replace.md) прозора, са њеним правилима:

- **претрага је у видљивом тексту** — ономе што гледалац чита. Ознаке нису његов
  део: тражење `<i>` не налази ништа, а тражење `Bonjour` налази „Bonjour“ чија је половина
  искошена;
- **замена је у изворном тексту**, оном из датотеке, а ознаке остају тамо где су
  биле. Ознака која пресеца тражену реч обухвата целу замену; стил који
  додирује поклапање покрива целу замену; оно чега се поклапање не тиче
  не помера се. Правила, случај по случај, су она претраге прозора.

**Не постоји подкоманда `find`**: да би се видело где је неки образац а да се ништа не измени,
[`--dry-run`](invocation.md#прво-видети-па-писати) исписује сваки титл који се тиче.

<!-- example: subedit-cli replace --help -->
```console
$ subedit-cli replace --help
Replace a text in the subtitles, without breaking a tag
Usage: subedit-cli replace [OPTIONS] pattern replacement files...

Positionals:
  pattern TEXT REQUIRED       Text to look for in what is shown
  replacement TEXT REQUIRED   What replaces it, tags allowed
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --regex                     Read the text to look for as a regular expression
  --case-sensitive            Tell capitals from small letters
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

## Аргументи и опције

| Опција | Обавезно | Вредности | Подразумевано |
| :----- | :----- | :------ | :----- |
| `<pattern>` | да | текст који се тражи у ономе што је приказано; **није празан** | — |
| `<replacement>` | да | оно што га замењује, ознаке дозвољене; може бити празно | — |
| `<file>...` | да | једна или више путања | — |
| `--recursive`, `-r` | не | опција без вредности | искључено — види [Обрада стабла директоријума](lots.md) |
| `--regex` | не | опција без вредности: образац је регуларни израз ICU | обичан текст |
| `--case-sensitive` | не | опција без вредности: разликује велика и мала слова | занемарују се |
| `--range` | не | `N-M`, или `N-` до краја — види [`adjust`](adjust.md#--range) | цела датотека |
| `--document` | не | `main` или `translation` — види [Превод](invocation.md#превод) | `main` |
| `-t`, `--translation-file` | уз `--document translation` | датотека превода, за **један** улаз | — |
| `--align-method` | не, и само уз `-t` | `position` или `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **једна од три**, осим уз `--dry-run` | види [Покретање](invocation.md#одредиште) | — |
| `--dry-run` | не | опција без вредности: израчуна и јави, ништа не пише — види [Прво видети, па писати](invocation.md#прво-видети-па-писати) | искључено |

**Образац који почиње цртицом** пише се после `--`: `subedit-cli replace -- -- "—" film.srt`.

**По подразумеваној вредности главни текст; превод уз `--document translation -t FILE`.** Тада се
мења и пише превод, и само он — види [Превод](invocation.md#превод).

### Обичан текст или израз

По подразумеваној вредности образац се **чита какав јесте**: `a.b` тражи `a`, тачку, `b`. Величина слова
занемарује се са истом префињеношћу као у прозору — `É` и `é` су једно слово.

Уз `--regex`, образац је **ICU** регуларни израз, читан онако како га чита Gaupol: `.` прелази
прелом реда, `^` и `$` важе на сваком реду титла. У замени, и
само ту:

| Пише се | Значи |
| :----- | :-------- |
| `$1` до `$9` | групе поклапања |
| `$0` | цело поклапање |
| `\n` | прелом реда |
| `\$`, `\\` | `$`, обрнута коса црта |

Пази на наводнике љуске: `'$1 EUR'`, а не `"$1 EUR"`, јер би љуска прочитала `$1`.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nPrix: 12 euros, et 7 euros.\n\n' > prix.srt; subedit-cli replace --regex '([0-9]+) euros' '$1 EUR' --output euros.srt prix.srt; cat euros.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nPrix: 12 euros, et 7 euros.\n\n' > prix.srt; subedit-cli replace --regex '([0-9]+) euros' '$1 EUR' --output euros.srt prix.srt; cat euros.srt
prix.srt: replaced 2 matches -> euros.srt
1
00:00:01,000 --> 00:00:02,000
Prix: 12 EUR, et 7 EUR.
```

## Ознаке

Текст у коме ознака пресеца тражену реч:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace Bonjour Salut --output salut.srt balises.srt; cat salut.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace Bonjour Salut --output salut.srt balises.srt; cat salut.srt
balises.srt: replaced 2 matches -> salut.srt
1
00:00:01,000 --> 00:00:02,000
<i>Salut</i> tout le monde

2
00:00:03,000 --> 00:00:04,000
Salut <i>Marie</i>, ca va
```

У првом титлу ознака је пресекла „Bonjour“: сада обухвата „Salut“.
У другом није дотакнута и остаје где је. **Ознака остављена отвореном
остаје отворена**: поправљање климавог означавања била би нека друга операција.

Замена се чита у **ознакама формата датотеке**: `<i>…</i>` за SubRip,
`{\i1}…` за Advanced SSA. То је једини начин да се обликује оно што се пише.

## Ништа нађено, ништа за измену

Два одговора који нису неуспеси — излазни код је `0`, а **датотека се ипак
пише**, јер је задато одредиште написано одредиште:

| Ситуација | Реченица |
| :-------- | :----- |
| обрасца нема нигде | `"zzz" not found` |
| ту је, али се свако поклапање замењује самим собом | `nothing to change` |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli replace zzz x --output b.srt a.srt; subedit-cli replace Bonjour Bonjour --output c.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli replace zzz x --output b.srt a.srt; subedit-cli replace Bonjour Bonjour --output c.srt a.srt
a.srt: "zzz" not found -> b.srt
a.srt: nothing to change -> c.srt
```

## `--dry-run`

Стандардни излаз исписује **сваки титл чији би се текст изменио**, пре и после, какви
су у датотеци — укључујући ознаке:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace --dry-run Bonjour Salut balises.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace --dry-run Bonjour Salut balises.srt
balises.srt: replaced 2 matches (dry run, nothing written)
balises.srt: subtitle 1
- <i>Bon</i>jour tout le monde
+ <i>Salut</i> tout le monde
balises.srt: subtitle 2
- Bonjour <i>Marie</i>, ca va
+ Salut <i>Marie</i>, ca va
```

Облик списка описан је у одељку [Прво видети, па писати](invocation.md#прво-видети-па-писати);
у JSON-у то је `changes`.

## Излаз

**Стандардни излаз** — ништа: резултат је написана датотека. Уз `--dry-run`, горњи списак
измена.

| Ниво | Шта се додаје на стандардној грешци |
| :----- | :------------------------------------- |
| 1 | `<path>: replaced N matches -> <destination>`, или `"<pattern>" not found`, или `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — формат, кодирање, ознака и завршеци редова прочитане датотеке, враћени какви су били |
| 3 | `<path>: N bytes read, M written`, затим **свака дијагностика читања** — види [Покретање](invocation.md#читање-дијагностике) |

`N` броји **замењена поклапања у титловима чији се текст изменио** — то је
број који каже и прозор.

### У JSON-у

Уз [`--format json`](invocation.md#излаз-који-скрипта-може-да-чита), објекат сваке датотеке садржи:

| Кључ | Садржај |
| :-- | :------ |
| `counts.replaced` | замењена поклапања, у титловима чији се текст изменио: `N` из реченице |
| `counts.matched` | пронађена поклапања, **без обзира на то да ли је њихова замена изменила текст** — то разликује „ништа нађено“ (`0`) од „ништа за измену“ (`replaced` на `0`, `matched` позитиван) |
| `changes` | измењени титлови: `{"subtitle", "document", "before", "after"}`, **празно** када се ништа није изменило |

## Излазни кодови

Они из програма: `0` ако је свака датотека обрађена — датотека у којој ништа није нађено јесте
—, `2` ако ниједна, `3` ако само неке, `1` при грешци у употреби.

## Грешке

| Шта је изазива | Порука |
| :------------------ | :------ |
| празан образац | `pattern: nothing to look for` |
| нечитљив израз | `pattern: not a regular expression (<ICU's reason>)` |
| интервал лоше написан или ван датотеке | види [`adjust`](adjust.md#--range) |

**Образац се чита пре прве датотеке**: нечитљив израз је грешка у употреби
(код `1`), ништа се не чита нити пише.

Грешке одредишта заједничке су подкомандама које пишу:
види [Покретање](invocation.md#одредиште).
