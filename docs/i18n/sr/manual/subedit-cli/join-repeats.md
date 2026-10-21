# `join-repeats`

<!-- machine-translated -->
> Ова страница је машински превод који још није прегледао читалац језика.

```
subedit-cli join-repeats (--output FILE | --output-dir DIR | --in-place)
                         [--dry-run]
                         [--recursive]
                         <file>...
```

Од титлова који један за другим приказују исто прави **један једини титл**: текст исечен на комаде који
следе један за другим, какав остављају неки алати за сегментацију. То је `Join Repeated Subtitles` у прозору —
[исти мотор](../subedit-gui/lines.md#спајање-поновљених-титлова).

**Понављање** је титл који има **исти текст, исти превод и иста подешавања формата** (стил, положај,
подешавања реплике) као други титл, **и који почиње најкасније када тај други завршава**: додирују се или
се преклапају. Низ понављања постаје **онај који почиње први**, а његов крај се помера
на најкаснији крај низа; остали нестају.

- **Редослед у датотеци не мења ништа и не мења се**: задржава се титл који почиње први,
  ма где били остали. Уређена датотека остаје уређена.
- **Ништа се не спаја грешком**: други текст, други превод, друга подешавања, размак између два
  комада, титл без текста или чији крај долази пре почетка — у свим тим случајевима остају раздвојени.
- **Ради се пре осталог**: ређање и [сечење преклапања](disjoint.md) боље читају чист низ.
  [`repair`](repair.md) обавља сва три корака, а [`convert`](convert.md#раздвајање-интервала) га прихвата као опцију (`--join-repeats`, `--repair`).

<!-- example: subedit-cli join-repeats --help -->
```console
$ subedit-cli join-repeats --help
Join the subtitles that repeat each other, one after the other, into one
Usage: subedit-cli join-repeats [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to join the repeats of

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
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
| `--output` / `--output-dir` / `--in-place` | **једна од три**, осим са `--dry-run` | види [Покретање](invocation.md#одредиште) | — |
| `--dry-run` | не | опција без вредности: рачуна и јавља, ништа не пише — види [Прво видети, па писати](invocation.md#прво-видети-па-писати) | искључено |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:04,000 --> 00:00:06,000\nAlpha\n\n4\n00:00:08,000 --> 00:00:09,000\nBeta\n\n' > morceaux.srt; subedit-cli join-repeats --output joint.srt morceaux.srt; cat joint.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:04,000 --> 00:00:06,000\nAlpha\n\n4\n00:00:08,000 --> 00:00:09,000\nBeta\n\n' > morceaux.srt; subedit-cli join-repeats --output joint.srt morceaux.srt; cat joint.srt
morceaux.srt: 2 repeated subtitles joined -> joint.srt
1
00:00:01,000 --> 00:00:06,000
Alpha

2
00:00:08,000 --> 00:00:09,000
Beta
```

Три комада текста `Alpha`, који следе један за другим од 1 s до 6 s, сада чине само један. Текст `Beta`, одвојен размаком,
остао је. Други пролаз не мења ништа и то јавља: `no repeated subtitles`.

## Излаз

**Стандардни излаз** — ништа: резултат је записана датотека.

| Ниво | Шта се додаје на излаз грешака |
| :----- | :------------------------------------- |
| 1 | `<path>: N repeated subtitles joined -> <destination>`, или `<path>: no repeated subtitles -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, затим **свака дијагностика читања** |

### У JSON-у

Са [`--format json`](invocation.md#излаз-који-скрипта-може-да-чита), `counts` сваке датотеке: `subtitles` (титлови датотеке
која се пише), `joined` (уклоњени титлови јер их други покрива) и `chains` (спојени низови).

## Излазни кодови

Они самог алата: `0` ако је свака датотека обрађена — и она без понављања јесте —, `2` ако ниједна није,
`3` ако су само неке, `1` при грешци у употреби.

## Грешке

Нема ниједне посебне за `join-repeats`: нечитљива датотека пада као код сваке подкоманде, а грешке
одредишта заједничке су подкомандама које пишу: види [Покретање](invocation.md#одредиште).
