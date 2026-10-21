# `dialogue-dashes`

<!-- machine-translated -->
> Ова страница је машински превод који још није прегледао читалац језика.

```
subedit-cli dialogue-dashes (--add | --remove) [--range N-M|N-]
                            (--output FILE | --output-dir DIR | --in-place)
                            [--dry-run]
                            [--recursive]
                            [--document main|translation] [-t FILE] [--align-method position|number]
                            <file>...
```

Ставља **цртицу разговора** на почетак редова или је скида. То је `Dialogue Dashes` у
[прозору](../subedit-gui/operations.md).

**Две опције, као код подкоманде [`italics`](italics.md)**: `--add` и `--remove` се међусобно искључују, а једна је потребна.

- **`--add`** ставља цртицу испред сваког реда који је нема: `Bonjour` даје `- Bonjour`, а
  `Bonjour\nMarie` даје `- Bonjour\n- Marie`. **Цртица која већ постоји не удвостручује се.**
- **Препознају се три цртице** — цртица-спојница са тастатуре и две дуге цртице типографа —
  **а пише се цртица-спојница**: `— Bonjour` постаје `- Bonjour`. Вишак размака
  иза цртице нестаје.
- Цртица иде **испред текста, а не испред почетне ознаке**: `<i>Bonjour</i>` даје
  `<i>- Bonjour</i>`. Ниједна ознака се не помера.
- **`--remove`** скида почетне цртице, у сва три облика.

<!-- example: subedit-cli dialogue-dashes --help -->
```console
$ subedit-cli dialogue-dashes --help
Put dialogue dashes on the lines, or take them off
Usage: subedit-cli dialogue-dashes [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --add                       Put a dialogue dash at the head of the lines
  --remove                    Take the dialogue dashes off
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
| `--add` / `--remove` | **једна од две** | опције без вредности | — |
| `--range` | не | `N-M`, или `N-` до краја — види [`adjust`](adjust.md#--range) | цела датотека |
| `--document` | не | `main` или `translation` — види [Превод](invocation.md#превод) | `main` |
| `-t`, `--translation-file` | уз `--document translation` | датотека превода, за **један једини** улаз | — |
| `--align-method` | не, и само уз `-t` | `position` или `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **једна од три**, осим уз `--dry-run` | види [Покретање](invocation.md#одредиште) | — |
| `--dry-run` | не | опција без вредности: израчуна и каже, не пише ништа — види [Прво видети, па писати](invocation.md#прво-видети-па-писати) | искључено |

**Подразумевано главни текст; превод помоћу `--document translation -t FILE`.** Тада се
мења и записује превод, и само он — види [Превод](invocation.md#превод).

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\nMarie\n\n2\n00:00:03,000 --> 00:00:04,000\n<i>Salut</i>\n\n' > a.srt; subedit-cli dialogue-dashes --add --output t.srt a.srt; cat t.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\nMarie\n\n2\n00:00:03,000 --> 00:00:04,000\n<i>Salut</i>\n\n' > a.srt; subedit-cli dialogue-dashes --add --output t.srt a.srt; cat t.srt
a.srt: 2 subtitles dashed -> t.srt
1
00:00:01,000 --> 00:00:02,000
- Bonjour
- Marie

2
00:00:03,000 --> 00:00:04,000
<i>- Salut</i>
```

## Излаз

**Стандардни излаз** — ништа: резултат је записана датотека. Уз `--dry-run`, списак
измена, описан у [Прво видети, па писати](invocation.md#прво-видети-па-писати).

| Ниво | Шта се додаје на излаз за грешке |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles dashed -> <destination>`, или `N subtitles undashed`, или `nothing to change` |
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
| ни `--add` ни `--remove` | `dialogue-dashes needs --add to put dashes on, or --remove` |
| обе | `--add and --remove say opposite things; give one of them` |
| интервал лоше написан или изван датотеке | види [`adjust`](adjust.md#--range) |

Грешке одредишта заједничке су подкомандама које пишу:
види [Покретање](invocation.md#одредиште).
