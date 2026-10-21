# `sort`

<!-- machine-translated -->
> Ова страница је машински превод који још није прегледао читалац језика.

```
subedit-cli sort (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 <file>...
```

Враћа титлове **редом њиховог почетка**. Ништа друго се не мења: ни текстови, ни
положаји, ни формат датотеке. Титлови се при писању **изнова нумеришу** од 1,
као код сваке датотеке која се уписује назад.

**Језгро никада само не ређа**: читање датотеке оставља њен редослед какав је био, а
[`inspect`](inspect.md#шта-јавља-anomalies) јавља прекид редоследа поруком
`subtitle N starts before the previous one starts`. `sort` је **намеран потез** за датотеку
или серију чији је редослед нарушен — и може се опозвати, у прозору, као свака операција.

- **Ређање је стабилно**: два титла која почињу у истом тренутку **задржавају редослед који
  им је дала датотека**. Ниједан не долази пре другог; њихово померање била би одлука коју нико
  није тражио.
- **`sort` не прима ни интервал ни документ**: нема шта да се бира, редослед је редослед почетака.

<!-- example: subedit-cli sort --help -->
```console
$ subedit-cli sort --help
Put the subtitles in the order of their start, keeping ties as they are
Usage: subedit-cli sort [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to sort

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
| `--output` / `--output-dir` / `--in-place` | **једна од три**, осим уз `--dry-run` | види [Покретање](invocation.md#одредиште) | — |
| `--dry-run` | не | опција без вредности: израчуна и јави, ништа не пише — види [Прво видети, па писати](invocation.md#прво-видети-па-писати) | искључено |

<!-- example: printf '1\n00:00:05,000 --> 00:00:06,000\ncinq\n\n2\n00:00:01,000 --> 00:00:02,000\nun\n\n3\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > desordre.srt; subedit-cli sort --output ordre.srt desordre.srt; cat ordre.srt -->
```console
$ printf '1\n00:00:05,000 --> 00:00:06,000\ncinq\n\n2\n00:00:01,000 --> 00:00:02,000\nun\n\n3\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > desordre.srt; subedit-cli sort --output ordre.srt desordre.srt; cat ordre.srt
desordre.srt: 3 subtitles moved -> ordre.srt
1
00:00:01,000 --> 00:00:02,000
un

2
00:00:03,000 --> 00:00:04,000
trois

3
00:00:05,000 --> 00:00:06,000
cinq
```

## Шта се броји

**Броји се број места која су променила титл**, како га јавља језгро — оно што
би историја прозора назвала померањем. Један погрешно постављен титл помера више њих: у
горњем примеру ниједно од три места не задржава свој титл, па реченица каже `3 subtitles moved`.

**Датотека која је већ уређена ипак се пише**, и то јавља: задато одредиште је
написано одредиште, а чињење изузетка од подкоманде `sort` натерало би скрипту да зна које
подкоманде понекад праве датотеку, а понекад не праве ништа.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > ordre.srt; subedit-cli sort --output copie.srt ordre.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > ordre.srt; subedit-cli sort --output copie.srt ordre.srt
ordre.srt: already in order -> copie.srt
```

## Излаз

**Стандардни излаз** — ништа: резултат је написана датотека. Ни уз `--dry-run` ништа: ту
нема датотеке, а ред приповедања то каже — види [Прво видети, па писати](invocation.md#прво-видети-па-писати).

| Ниво | Шта се додаје на стандардној грешци |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles moved -> <destination>`, или `<path>: already in order -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — формат, кодирање, ознака и завршеци редова прочитане датотеке, враћени какви су били |
| 3 | `<path>: N bytes read, M written`, затим **свака дијагностика читања** — види [Покретање](invocation.md#читање-дијагностике) |

### У JSON-у

Уз [`--format json`](invocation.md#излаз-који-скрипта-може-да-чита), објекат сваке датотеке садржи
`counts.subtitles` (сваки титл датотеке) и `counts.moved` (места која су се променила;
`0` за датотеку која је већ уређена).

## Излазни кодови

Они из програма: `0` ако је свака датотека обрађена — већ уређена јесте —, `2` ако
ниједна, `3` ако само неке, `1` при грешци у употреби.

## Грешке

Ниједна својствена подкоманди `sort`: нечитљива датотека пропада као код сваке подкоманде, а
грешке одредишта заједничке су подкомандама које пишу:
види [Покретање](invocation.md#одредиште).
