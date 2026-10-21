# `italics`

<!-- machine-translated -->
> Ова страница је машински превод који још није прегледао читалац језика.

```
subedit-cli italics (--on | --off) [--range N-M|N-]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    <file>...
```

Ставља текстове у **искошено писмо** или им **уклања искошено писмо**, у ознакама формата датотеке.
То је `Italic` у [прозору](../subedit-gui/operations.md).

**Две речи, а не прекидач**: дугме прозора пита језгро на коју страну
иде; овде се то каже. Опције `--on` и `--off` се међусобно искључују, а једна је потребна — изостављање обе или навођење обе
је грешка у употреби (код `1`).

- **`--on`** обавија сваки текст једним паром ознака, **пошто уклони оне који
  су у њему били**: текст који је већ искошен не обавија се двапут, а искошено писмо исечено на два дела
  прави се поново у једном комаду. Преко више редова један пар покрива целину.
- **`--off`** уклања ознаке искошеног писма; **остатак ознаке се чува** (`{\b1\i1}` даје
  `{\b1}`).
- Текст се пише **како формат датотеке пише искошено писмо**: `<i>…</i>` за SubRip, WebVTT и
  SubViewer 2, `{\i1}…{\i0}` за оба Sub Station Alpha формата, `{Y:i}` за MicroDVD.

<!-- example: subedit-cli italics --help -->
```console
$ subedit-cli italics --help
Put the texts in italics, or take their italics out
Usage: subedit-cli italics [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --on                        Put the texts in italics
  --off                       Take the italics out
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
| `<file>...` | да | једна или више путања | — |
| `--recursive`, `-r` | не | опција без вредности | искључено — види [Обрада стабла директоријума](lots.md) |
| `--on` / `--off` | **једна од две** | опције без вредности | — |
| `--range` | не | `N-M`, или `N-` до краја — види [`adjust`](adjust.md#--range) | цела датотека |
| `--document` | не | `main` или `translation` — види [Превод](invocation.md#превод) | `main` |
| `-t`, `--translation-file` | са `--document translation` | датотека превода, за **један једини** улаз | — |
| `--align-method` | не, и само са `-t` | `position` или `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **једна од три**, осим са `--dry-run` | види [Покретање](invocation.md#одредиште) | — |
| `--dry-run` | не | опција без вредности: рачуна и јавља, ништа не пише — види [Прво видети, па писати](invocation.md#прво-видети-па-писати) | искључено |

**Подразумевано главни текст; превод са `--document translation -t FILE`.** Тада се
мења и пише превод, и само он — види [Превод](invocation.md#превод).

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour\nMarie\n\n' > a.srt; subedit-cli italics --on --output i.srt a.srt; cat i.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour\nMarie\n\n' > a.srt; subedit-cli italics --on --output i.srt a.srt; cat i.srt
a.srt: 2 subtitles put in italics -> i.srt
1
00:00:01,000 --> 00:00:02,000
<i>bonjour marie</i>

2
00:00:03,000 --> 00:00:04,000
<i>Bonjour
Marie</i>
```

## Формат без стила одбија

**TMPlayer и LRC не пишу никакав стил**: нема искошеног писма које би се ставило ни скинуло. Прозор
засивљује ставку уместо да је сакрије; командна линија **одбија, датотеку по
датотеку, уз разлог**, а остале датотеке серије настављају. Рачуна се оно што
формат може да носи, а не начин на који то пише: SubRip и WebVTT пишу искошено писмо
на исти начин, а о боји не кажу исто.

<!-- example: printf '00:00:01:Hello there\n00:00:03:Bye\n' > a.txt; subedit-cli italics --on --output-dir sortie a.txt; echo "code=$?" -->
```console
$ printf '00:00:01:Hello there\n00:00:03:Bye\n' > a.txt; subedit-cli italics --on --output-dir sortie a.txt; echo "code=$?"
a.txt: TMPlayer writes no style: there are no italics to put on
code=2
```

У JSON-у неуспех носи идентификатор **`no-style`**.

## Излаз

**Стандардни излаз** — ништа: резултат је записана датотека. Са `--dry-run`, списак
измена, описан у одељку [Прво видети, па писати](invocation.md#прво-видети-па-писати).

| Ниво | Шта се додаје на излаз грешака |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles put in italics -> <destination>`, или `N subtitles taken out of italics`, или `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — формат, кодирање, ознака и завршеци редова прочитане датотеке, враћени какви су били |
| 3 | `<path>: N bytes read, M written`, затим **свака дијагностика читања** — види [Покретање](invocation.md#читање-дијагностике) |

`N` је број титлова **чији се текст променио**, очитан из саме операције.

### У JSON-у

Са [`--format json`](invocation.md#излаз-који-скрипта-може-да-чита), објекат сваке датотеке носи
`counts.changed` — `N` из реченице — и `changes`, списак измењених титлова
(`{"subtitle", "document", "before", "after"}`, **празан** када се ништа није променило).

## Излазни кодови

Они самог алата: `0` ако је свака датотека обрађена — и она у којој се ништа не мења јесте —,
`2` ако ниједна није, `3` ако су само неке, `1` при грешци у употреби.

## Грешке

| Шта је изазива | Порука |
| :------------------ | :------ |
| ни `--on` ни `--off` | `italics needs --on to put the texts in italics, or --off` |
| обе | `--on and --off say opposite things; give one of them` |
| формат без стила | `<path>: TMPlayer writes no style: there are no italics to put on` (или `take out`) |
| интервал лоше написан или ван датотеке | види [`adjust`](adjust.md#--range) |

Грешке одредишта заједничке су подкомандама које пишу:
види [Покретање](invocation.md#одредиште).
