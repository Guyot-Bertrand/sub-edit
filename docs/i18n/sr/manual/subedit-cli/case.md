# `case`

<!-- machine-translated -->
> Ова страница је машински превод који још није прегледао читалац језика.

```
subedit-cli case --to title|sentence|upper|lower [--range N-M|N-]
                 (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 [--document main|translation] [-t FILE] [--align-method position|number]
                 <file>...
```

Поставља текстове у **велика слова сваке речи, прво слово реченице, велика слова или мала слова**, не дирајући
ниједну ознаку. То је `Алати ▸ Величина слова` у [прозору](../subedit-gui/operations.md), са његовим правилима.

| `--to` | Дејство | `bonjour marie` постаје |
| :----- | :---- | :---------------------- |
| `title` | свака реч добија велико слово, остало се спушта | `Bonjour Marie` |
| `sentence` | прво слово текста, и само оно; остало се спушта | `Bonjour marie` |
| `upper` | све великим словима | `BONJOUR MARIE` |
| `lower` | све малим словима | `bonjour marie` |

- **Оно што долази пре првог слова или прве цифре не дира се**: цртица
  разговора, почетни наводник или заграда остају (`- bonjour` даје `- Bonjour`).
- **Ознаке нису текст**: не мењају величину слова — `<I>` остаје `<I>` — и не
  мењају место: ознака која пресеца реч и после ње пресеца.
- **ICU одлучује где реч почиње**: `l'été sera chaud` даје `L'été Sera Chaud`, а акценти
  остају (`éléonore` даје `Éléonore`).

<!-- example: subedit-cli case --help -->
```console
$ subedit-cli case --help
Put the texts in title, sentence, upper or lower case, tags intact
Usage: subedit-cli case [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --to title|sentence|upper|lower
                              The case to put the texts in
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
| `--to` | **да** | `title`, `sentence`, `upper`, `lower` | — |
| `--range` | не | `N-M`, или `N-` до краја — види [`adjust`](adjust.md#--range) | цела датотека |
| `--document` | не | `main` или `translation` — види [Превод](invocation.md#превод) | `main` |
| `-t`, `--translation-file` | уз `--document translation` | датотека превода, за **један једини** улаз | — |
| `--align-method` | не, и само уз `-t` | `position` или `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **једна од три**, осим уз `--dry-run` | види [Покретање](invocation.md#одредиште) | — |
| `--dry-run` | не | опција без вредности: израчуна и каже, не пише ништа — види [Прво видети, па писати](invocation.md#прво-видети-па-писати) | искључено |

**Подразумевано главни текст; превод помоћу `--document translation -t FILE`.** Тада се
мења и записује превод, и само он — види [Превод](invocation.md#превод).

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\n- BONJOUR MARIE\n\n' > a.srt; subedit-cli case --to title --output titre.srt a.srt; cat titre.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\n- BONJOUR MARIE\n\n' > a.srt; subedit-cli case --to title --output titre.srt a.srt; cat titre.srt
a.srt: 2 subtitles recased -> titre.srt
1
00:00:01,000 --> 00:00:02,000
<i>Bon</i>jour Marie

2
00:00:03,000 --> 00:00:04,000
- Bonjour Marie
```

Датотека у којој се **ниједан текст не мења** — све је већ у траженој величини слова — записује се каква јесте и
то јавља:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBONJOUR\n\n' > a.srt; subedit-cli case --to upper --output b.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBONJOUR\n\n' > a.srt; subedit-cli case --to upper --output b.srt a.srt
a.srt: nothing to change -> b.srt
```

## Излаз

**Стандардни излаз** — ништа: резултат је записана датотека. Уз `--dry-run`, списак
измена, описан у [Прво видети, па писати](invocation.md#прво-видети-па-писати).

| Ниво | Шта се додаје на излаз за грешке |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles recased -> <destination>`, или `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — формат, кодирање, ознака и завршеци редова прочитане датотеке, враћени какви су били |
| 3 | `<path>: N bytes read, M written`, затим **свака дијагностика читања** — види [Покретање](invocation.md#читање-дијагностике) |

`N` је број титлова **чији се текст променио**, очитан из саме операције.

### У JSON-у

Уз [`--format json`](invocation.md#излаз-који-скрипта-може-да-чита), објекат сваке датотеке носи
`counts.changed` — `N` из реченице — и `changes`, списак измењених титлова
(`{"subtitle", "document", "before", "after"}`, **празан** када се ништа није променило).

## Излазни кодови

Они самог алата: `0` ако је свака датотека обрађена — датотека у којој се ништа не мења јесте —,
`2` ако ниједна, `3` ако само неке, `1` код грешке у употреби.

## Грешке

| Шта је изазива | Порука |
| :------------------ | :------ |
| недостаје `--to` | `--to is required` |
| вредност која није једна од четири | `--to: … not in {title,sentence,upper,lower}` |
| интервал лоше написан или изван датотеке | види [`adjust`](adjust.md#--range) |

Грешке одредишта заједничке су подкомандама које пишу:
види [Покретање](invocation.md#одредиште).
