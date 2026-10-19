# `repair`

<!-- machine-translated -->
> Esta páxina é unha tradución automática e aínda non a revisou ningún lector da lingua.

```
subedit-cli repair (--output FILE | --output-dir DIR | --in-place)
                   [--minimum TIME] [--max-lines N] [--dashes]
                   [--dry-run] [--recursive]
                   <file>...
```

Fai dunha soa vez o que fan tres ordes unha tras outra: [`join-repeats`](join-repeats.md), despois
[`sort`](sort.md), despois [`disjoint`](disjoint.md). **Nesa orde, que é a única que funciona**: o corte le
a orde dos inicios, e os anacos repetidos dun texto mellor que desaparezan antes de calquera das dúas. Nun ficheiro
mal segmentado, desordenado e con solapamentos, é a orde que hai que executar.

Os axustes son os de `disjoint`, que remata a cadea; `--sort` non existe aquí, xa que a ordenación faise sempre.

<!-- example: subedit-cli repair --help -->
```console
$ subedit-cli repair --help
Join the repeated subtitles, put them in order, then make the ranges disjoint
Usage: subedit-cli repair [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to repair

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --minimum TIME              Shortest interval kept; a shorter one is taken over by a neighbour (default 0.08)
  --max-lines N               Most lines of a merged subtitle; more are shown in windows, 0 for no limit (default 2)
  --dashes                    Put a dash before the text of each merged subtitle
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Argumentos e opcións

| Opción | Obrigatorio | Valores | Predeterminado |
| :----- | :----- | :------ | :----- |
| `<file>...` | si | unha ou máis rutas | — |
| `--minimum` | non | unha duración: `0.08`, `1.5`, `00:00:00,080`. Cero conserva todos os intervalos | `0.08` (uns dous fotogramas) |
| `--max-lines` | non | un número enteiro; `0` elimina o límite | `2` |
| `--dashes` | non | un indicador: un trazo diante do texto que cada subtítulo achega a un subtítulo fusionado | desactivado |
| `--recursive`, `-r` | non | un indicador | desactivado — véxase [Procesar unha árbore](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **unha das tres**, agás con `--dry-run` | véxase [Invocación](invocation.md#o-destino) | — |
| `--dry-run` | non | un indicador: calcula e di, non escribe nada — véxase [Ver antes de escribir](invocation.md#ver-antes-de-escribir) | desactivado |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:01,000 --> 00:00:04,000\nBeta\n\n' > segments.srt; subedit-cli repair --output repare.srt segments.srt; cat repare.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:01,000 --> 00:00:04,000\nBeta\n\n' > segments.srt; subedit-cli repair --output repare.srt segments.srt; cat repare.srt
segments.srt: 1 repeated subtitle joined; already in order; 0 subtitles split, 1 merged -> repare.srt
1
00:00:01,000 --> 00:00:04,000
Alpha
Beta
```

`Alpha`, cortado en dous anacos que se suceden, volve ser un só subtítulo; despois ocupa a mesma franxa ca `Beta`,
e os dous forman un só, de dúas liñas, o máis temperán primeiro.

## Saída

**Saída estándar** — nada: o resultado é o ficheiro escrito. Na saída de erro, no primeiro nivel, **unha frase
que encadea os tres recontos**, separados por punto e coma: o que din `join-repeats`, `sort` e `disjoint`, cada unha
coas súas propias palabras — `N repeated subtitles joined; M subtitles moved; K subtitles split, L merged…`.

### En JSON

Con [`--format json`](invocation.md#saída-que-pode-ler-un-script), os `counts` de cada ficheiro: `subtitles`, `joined`,
`moved`, despois os de [`disjoint`](disjoint.md#en-json) (`split`, `merged`, `windowed`, `absorbed`, `too_short`,
`ending_before_start`, `extras_dropped`).

## Códigos de saída e erros

Os da ferramenta. Ningún erro é propio de `repair` á parte dos axustes, que se xulgan como os de `disjoint`:
un `--minimum` que non é unha duración ou é negativo, un `--max-lines` que non é un número enteiro, código `1`.
