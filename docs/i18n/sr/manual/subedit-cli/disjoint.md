# `disjoint`

<!-- machine-translated -->
> Ова страница је машински превод који још није прегледао читалац језика.

```
subedit-cli disjoint (--output FILE | --output-dir DIR | --in-place)
                     [--minimum TIME] [--max-lines N] [--dashes] [--sort]
                     [--dry-run] [--recursive]
                     <file>...
```

Сече титлове који се преклапају тако да **ниједан интервал `[start, end]` више не
прекрива други**. То је `Make Ranges Disjoint…` у прозору — [исти мотор](../subedit-gui/lines.md#раздвајање-интервала).

**Ништа се не скраћује нити продужава**: у сваком тренутку датотека приказује исте текстове као пре. Мења се
само начин на који је временска линија исечена:

- сече се на **сваком почетку и сваком крају** титлова једне групе која се преклапа;
- интервал који покрива **један једини** титл остаје тај титл;
- интервал који покрива **више њих** постаје **један** титл чији су редови њихови, **по
  редоследу њихових почетака** — најранији у реду 1, следећи у реду 2;
- титл који се ни са чим не преклапа не дира се, а редослед осталих се чува.

<!-- example: subedit-cli disjoint --help -->
```console
$ subedit-cli disjoint --help
Cut overlapping subtitles so that no range overlaps another, merging what overlaps
Usage: subedit-cli disjoint [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to make disjoint

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --minimum TIME              Shortest interval kept; a shorter one is taken over by a neighbour (default 0.08)
  --max-lines N               Most lines of a merged subtitle; more are shown in windows, 0 for no limit (default 2)
  --dashes                    Put a dash before the text of each merged subtitle
  --sort                      Put the subtitles in order of their start first; without it, a file out of order is refused
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Аргументи и опције

| Опција | Обавезно | Вредности | Подразумевано |
| :----- | :----- | :------ | :----- |
| `<file>...` | да | једна или више путања | — |
| `--minimum` | не | трајање: `0.08`, `1.5`, `00:00:00,080`. Нула чува сваки интервал | `0.08` (око два кадра) |
| `--max-lines` | не | цео број: највећи број редова спојеног титла; `0` укида ограничење | `2` |
| `--dashes` | не | опција без вредности: цртица испред текста који сваки титл уноси у спојени титл | искључено |
| `--sort` | не | опција без вредности: **најпре** поређа титлове по редоследу њихових почетака — види [Потребна је уређена датотека](#потребна-је-уређена-датотека) | искључено |
| `--recursive`, `-r` | не | опција без вредности | искључено — види [Обрада стабла директоријума](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **једна од три**, осим са `--dry-run` | види [Покретање](invocation.md#одредиште) | — |
| `--dry-run` | не | опција без вредности: рачуна и јавља, ништа не пише — види [Прво видети, па писати](invocation.md#прво-видети-па-писати) | искључено |

<!-- example: printf '1\n00:00:00,000 --> 00:00:05,000\nA\n\n2\n00:00:03,000 --> 00:00:08,000\nB\n\n' > chevauche.srt; subedit-cli disjoint --output disjoint.srt chevauche.srt; cat disjoint.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:05,000\nA\n\n2\n00:00:03,000 --> 00:00:08,000\nB\n\n' > chevauche.srt; subedit-cli disjoint --output disjoint.srt chevauche.srt; cat disjoint.srt
chevauche.srt: 2 subtitles split, 1 merged -> disjoint.srt
1
00:00:00,000 --> 00:00:03,000
A

2
00:00:03,000 --> 00:00:05,000
A
B

3
00:00:05,000 --> 00:00:08,000
B
```

Титлови `A [0, 5]` и `B [3, 8]` дају три раздвојена титла: `A` сам, `A` и `B` заједно, `B` сам. Ниједан
тренутак се не губи нити додаје.

## Шта операција одлучује

**Ред се никада не удваја.** Два титла са истим текстом дају један једини ред тамо где се
преклапају, а два суседна комада који приказују исто чине само један титл. Текст титла
исеченог на више комада иначе је **поновљен у сваком од њих**: то је оно што је на екрану у
том тренутку.

**Врло кратак интервал преузима сусед.** Испод вредности `--minimum` интервал нестаје: сусед
**продужава свој интервал да га покрије**, тако да ниједан тренутак не остане празан и ништа се не помери за више од тог
прага. Бира се сусед чији редови **већ садрже** редове интервала, а иначе
најдужи. Текст интервала губи се само ако га не носи ниједан сусед, и то за мање од прага.
**Цео титл краћи од прага никада се не преузима**: оставља се и броји.

**Више редова од ограничења: прозорчићи.** Спојени титл који би имао више од `--max-lines` редова
приказује се **у узастопним прозорчићима** те величине, у једнаким деловима трајања. Три реда током три
секунде:

<!-- example: printf '1\n00:00:00,000 --> 00:00:03,000\nA\n\n2\n00:00:00,000 --> 00:00:03,000\nB\n\n3\n00:00:00,000 --> 00:00:03,000\nC\n\n' > trois.srt; subedit-cli disjoint --output fenetres.srt trois.srt; cat fenetres.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:03,000\nA\n\n2\n00:00:00,000 --> 00:00:03,000\nB\n\n3\n00:00:00,000 --> 00:00:03,000\nC\n\n' > trois.srt; subedit-cli disjoint --output fenetres.srt trois.srt; cat fenetres.srt
trois.srt: 0 subtitles split, 1 merged; 1 shown in windows for holding too many lines -> fenetres.srt
1
00:00:00,000 --> 00:00:01,500
A
B

2
00:00:01,500 --> 00:00:03,000
B
C
```

Четири реда дају три прозорчића, и тако редом: прозорчић клизи по један ред. Ништа се не
скраћује нити поново сече.

**Превод прати исте резове**, пошто титл носи оба текста. **Оно што формат носи
изнад тога** — стил, положај — јесте оно првог титла који покрива; губитак се броји када
се спојени титлови разликују.

**Титл чији крај долази пре почетка** не погађа се: оставља се где јесте и броји.

## Потребна је уређена датотека

Групе су низови суседа: када редослед није уређен, суседи нису они који се преклапају. Датотека
чији почеци нису по реду зато се **одбија**, а порука то каже:

<!-- example: printf '1\n00:00:03,000 --> 00:00:08,000\nB\n\n2\n00:00:00,000 --> 00:00:05,000\nA\n\n' > desordre.srt; subedit-cli disjoint --output x.srt desordre.srt; echo "code $?" -->
```console
$ printf '1\n00:00:03,000 --> 00:00:08,000\nB\n\n2\n00:00:00,000 --> 00:00:05,000\nA\n\n' > desordre.srt; subedit-cli disjoint --output x.srt desordre.srt; echo "code $?"
desordre.srt: The subtitles are not in the order of their start: sort them first. Use --sort to do that here.
code 2
```

Опција `--sort` прво обавља ређање — исто што и [`sort`](sort.md) — и то јавља. Језгро никада не ређа само од себе.

Други пролаз не мења ништа: када су интервали раздвојени, нема више шта да се сече, а
порука гласи `already disjoint`.

## Излаз

**Стандардни излаз** — ништа: резултат је записана датотека.

| Ниво | Шта се додаје на излаз грешака |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles split, M merged -> <destination>`, а после `;` оно што није могло да се уради до краја: `K shown in windows for holding too many lines`, `K very short taken over by a neighbour`, `K subtitles shorter than the limit left whole`, `K merged with styles or positions that differ, the first kept`, `K subtitles ending before the start left alone` — или `<path>: already disjoint -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, затим **свака дијагностика читања** |

`N subtitles split` броји првобитне титлове исечене на више комада, а `M merged` комаде који
приказују више титлова заједно.

### У JSON-у

Са [`--format json`](invocation.md#излаз-који-скрипта-може-да-чита), `counts` сваке датотеке:

| Број | Шта каже |
| :----- | :----------- |
| `subtitles` | титлови датотеке која се пише |
| `split`, `merged` | као у реченици |
| `windowed` | комади приказани у прозорчићима |
| `absorbed` | интервали које је преузео сусед |
| `too_short` | титлови краћи од прага, остављени цели |
| `ending_before_start` | титлови чији крај долази пре почетка, остављени на миру |
| `extras_dropped` | спојени комади чији се стил или положај разликују |
| `moved` | само са `--sort`: места која су се променила |

## Излазни кодови

Они самог алата: `0` ако је свака датотека обрађена — и она без преклапања јесте —, `2` ако
ниједна није, `3` ако су само неке, `1` при грешци у употреби.

## Грешке

| Шта их изазива | Порука |
| :------------------- | :------ |
| неуређена датотека без `--sort` | `<path>: The subtitles are not in the order of their start: sort them first. Use --sort to do that here.` |
| `--minimum` није трајање или је негативан | `--minimum: …` |
| `--max-lines` није цео број | `--max-lines: "…" is not a number of lines: expected a whole number, 0 for no limit` |

Нечитљива датотека пада као код сваке подкоманде; грешке одредишта заједничке су
подкомандама које пишу: види [Покретање](invocation.md#одредиште).

## У осталим командама

[`convert`](convert.md#раздвајање-интервала), [`append`](append.md#раздвајање-интервала) и
[`split-file`](split-file.md#раздвајање-интервала) прихватају `--disjoint`, што пре писања ради исто са
подразумеваним подешавањима.

На лоше сегментираној датотеци — текст исечен на комаде који следе један за другим, дат без редоследа —, само ређање и сечење
дају редове који прескачу с једног комада на други: најпре [`join-repeats`](join-repeats.md), или
[`repair`](repair.md), који обавља сва три корака.
