# `hearing-impaired`

<!-- machine-translated -->
> Ова страница је машински превод који још није прегледао читалац језика.

```
subedit-cli hearing-impaired
                  (--output FILE | --output-dir DIR | --in-place)
                  [--dry-run]
                  [--recursive] [--parentheses]
                  [--document main|translation] [-t FILE] [--align-method position|number]
                  <file>...
```

Уклања **напомене за наглуве** — описане звукове између угластих заграда — из целе
датотеке. **Округле заграде по подразумеваној вредности остају**: користе се подједнако често за шапутан
ред колико и за описан звук, а њихово уклањање брисало би разговор. Опција `--parentheses` уклања и њих,
за датотеку која у њима описује своје звукове.

Оно што не ради: стихове песама између тарабица, име говорника
испред двотачке, поновно писање великим словом, исправљање грешака OCR-а. Ти
обрасци су они из прозорског чаробњака
[`Correct Texts…`](../subedit-gui/correct-texts.md), који командна
линија нуди под именом [`correct`](correct.md); а овде се ништа не може подесити:
преобликовање је одлучено, а не подесиво.

<!-- example: subedit-cli hearing-impaired --help -->
```console
$ subedit-cli hearing-impaired --help
Remove the sounds described between square brackets
Usage: subedit-cli hearing-impaired [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to clean

Options:
  -h,--help                   Print this help message and exit
  --parentheses               Also remove the sounds between parentheses, often whispered lines
  -r,--recursive              Take directories as inputs, and every subtitle file in them
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
| `<file>...` | да | једна или више путања | — |
| `--parentheses` | не | опција без вредности: уклања и напомене између округлих заграда | искључено |
| `--recursive`, `-r` | не | опција без вредности | искључено — види [Обрада стабла директоријума](lots.md) |
| `--document` | не | `main` или `translation` — види [Превод](invocation.md#превод) | `main` |
| `-t`, `--translation-file` | са `--document translation` | датотека превода, за **један једини** улаз | — |
| `--align-method` | не, и само са `-t` | `position` или `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **једна од три**, осим са `--dry-run` | види [Покретање](invocation.md#одредиште) | — |
| `--dry-run` | не | опција без вредности: рачуна и јавља, ништа не пише — види [Прво видети, па писати](invocation.md#прво-видети-па-писати) | искључено |

Формат прочитане датотеке се **чува**: промена формата је посао подкоманде
[`convert`](convert.md).

**Подразумевано главни текст; превод са `--document translation -t FILE`.** Тада се
мења и пише превод, и само он — види [Превод](invocation.md#превод).

**Напомена која би испразнила превод пише га празног и чува титл**: титл
остаје у датотеци, без текста. Главни текст, пак, губи титл потпуно испражњен, као
у наставку.

## Шта се уклања, а шта остаје

Напомена је угласта заграда — или округла, са `--parentheses` — и оно што обухвата, **укључујући
прелом реда**: у стварним датотекама напомену често прекида
крај реда.

| Улаз | Излаз |
| :----- | :----- |
| `Bonjour [il tousse] Marie` | `Bonjour Marie` |
| `Bonjour[il tousse]Marie` | `Bonjour Marie` |
| `[Bruit de pas] Bonjour` | `Bonjour` |
| `A [un] [deux] B` | `A B` |

Уклањање оставља **тачно један размак између онога што га је окруживало, и ништа на
ивици реда**. То није чишћење размака у датотеци: два размака
која је текст већ имао остају два размака.

**Када напомена прелази преко прелома реда, остаје прелом** —
два реда остају два реда. Тај случај није куриозитет: у петнаест
стварних датотека титлова, свака угласта заграда која је деловала усамљено била је
напомена прекинута на тај начин.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nReculez ! [Il\nhurle] Tout de suite !\n\n' > a.srt; subedit-cli --quiet hearing-impaired --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nReculez ! [Il\nhurle] Tout de suite !\n\n' > a.srt; subedit-cli --quiet hearing-impaired --output b.srt a.srt; cat b.srt
1
00:00:01,000 --> 00:00:03,000
Reculez !
Tout de suite !
```

**Оно што није напомена и не мења се:**

| Случај | Шта се дешава |
| :-- | :-------------- |
| чисто бројчан садржај — `[1]`, `(12)`, `[ 156478 ]` | остаје какав јесте, заједно са граничницима: упутница никада није именовала звук |
| округла заграда и оно што обухвата, без `--parentheses` | остаје каква јесте: `Venez (chut) ici` се не мења |
| угласта или округла заграда коју ништа не затвара | оставља се каква јесте |
| ознака формата — `<i>`, `</i>` | никада се не уклања |

Празнина **између** цифара, слово или знак — `[1 2]`, `[1a]`,
`[-1]` — чини је напоменом. Као и празан садржај, `[]` или `( )`: нема
цифре, дакле није упутница; нема шта да се каже, дакле нема шта да се чува.

## Шта нестаје у потпуности

**Ред који је уклањање испразнило нестаје**, а **титл потпуно испражњен
уклања се из датотеке**. Титлови који следе добијају нове бројеве.

Ред се сматра празним када у њему остану само размаци, ознаке формата
или усамљена цртица разговора — `<i>[Musique]</i>` не оставља
`<i></i>` на екрану.

**Ознаке су оне формата датотеке.** `{\i1}[Musique]{\i0}` у
формату Advanced SSA, `{Y:i}[Musique]` у MicroDVD и `/[Musique]` у MPL2 такође не
остављају ништа. Супротно томе, `<i>` у формату који не пише ниједну
ознаку — TMPlayer, LRC — јесте текст који гледалац чита, а ред који
га носи није празан.

**Разговор сведен на један глас више то није**, а цртица која остане
нестаје. Све док остану два гласа, цртице остају.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\nVoir [1] la note.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\nVoir [1] la note.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle cleaned, 1 removed -> b.srt
1
00:00:04,000 --> 00:00:06,000
Attends Marie.

2
00:00:07,000 --> 00:00:09,000
Voir [1] la note.
```

## Датотека без иједне напомене пише се ипак

Копира се идентично, излазни код је `0`, а извештај каже да није
било шта да се уклони.

То је правило подкоманди које пишу: **задато одредиште је
записано одредиште**. Изузетак овде натерао би скрипту да зна
која од подкоманди некад даје датотеку а некад ништа.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nRien à signaler.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nRien à signaler.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt
a.srt: no mention to remove -> b.srt
```

## Излаз

**Стандардни излаз** — ништа: резултат је записана датотека. **Са `--dry-run` то су предложене измене**: за сваки титл чији би се текст променио, његов број, текст пре и текст после — види [Прво видети, па писати](invocation.md#прво-видети-па-писати).

| Ниво | Шта се додаје на излаз грешака |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles cleaned, M removed -> <destination>`, или `<path>: no mention to remove -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — формат, кодирање, ознака и завршеци редова прочитане датотеке, враћени какви су били |
| 3 | `<path>: N bytes read, M written`, затим **свака дијагностика читања** — види [Покретање](invocation.md#читање-дијагностике) |

`N` броји титлове чији се текст променио, а `M` оне које је уклањање
потпуно испразнило и који су зато напустили датотеку.

### У JSON-у

Са [`--format json`](invocation.md#излаз-који-скрипта-може-да-чита), објекат сваке датотеке носи `counts.cleaned` (преписане текстове) и `counts.removed` (уклоњене титлове); **са `--dry-run`**, кључ `changes` такође набраја измењене титлове, као стандардни излаз текстуалног облика. Пуна табела је у одељку [Покретање](invocation.md#шта-свака-подкоманда-уписује-у-counts).

## Излазни кодови

Они самог алата: `0` ако је свака датотека записана, `2` ако ниједна није, `3` ако су
само неке, `1` при грешци у употреби.

## Грешке

Ова подкоманда нема сопствену грешку: не прима
вредност коју треба рашчланити, и никакав текст не може је натерати да одбије датотеку. Оно што
може да падне јесте читање или писање, као код осталих.

**Ништа се не пише без једне од три опције одредишта**, а њихове грешке
заједничке су подкомандама које пишу:
види [Покретање](invocation.md#одредиште).
