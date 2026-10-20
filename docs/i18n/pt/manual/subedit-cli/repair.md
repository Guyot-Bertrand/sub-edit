# `repair`

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revista por nenhum leitor do idioma.

```
subedit-cli repair (--output FILE | --output-dir DIR | --in-place)
                   [--minimum TIME] [--max-lines N] [--dashes]
                   [--dry-run] [--recursive]
                   <file>...
```

Faz de uma só vez o que três comandos fazem um a seguir ao outro: [`join-repeats`](join-repeats.md), depois
[`sort`](sort.md), depois [`disjoint`](disjoint.md). **Por essa ordem, que é a única que funciona**: o corte lê
a ordem dos inícios, e os pedaços repetidos de um mesmo texto é melhor que tenham desaparecido antes de qualquer dos dois. Num ficheiro
mal segmentado, fora de ordem e com sobreposições, é o comando a executar.

As definições são as de `disjoint`, que termina a cadeia; `--sort` não existe aqui, pois a ordenação é sempre feita.

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

## Argumentos e opções

| Opção | Obrigatório | Valores | Predefinição |
| :----- | :----- | :------ | :----- |
| `<file>...` | sim | um ou mais caminhos | — |
| `--minimum` | não | uma duração: `0.08`, `1.5`, `00:00:00,080`. Zero mantém todos os intervalos | `0.08` (cerca de dois fotogramas) |
| `--max-lines` | não | um número inteiro; `0` retira o limite | `2` |
| `--dashes` | não | um indicador: um hífen à frente do texto que cada legenda traz a uma legenda unida | desativado |
| `--recursive`, `-r` | não | um indicador | desativado — ver [Processar uma árvore de pastas](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **uma das três**, exceto com `--dry-run` | ver [Invocação](invocation.md#o-destino) | — |
| `--dry-run` | não | um indicador: calcula e diz, não escreve nada — ver [Ver antes de escrever](invocation.md#ver-antes-de-escrever) | desativado |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:01,000 --> 00:00:04,000\nBeta\n\n' > segments.srt; subedit-cli repair --output repare.srt segments.srt; cat repare.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:01,000 --> 00:00:04,000\nBeta\n\n' > segments.srt; subedit-cli repair --output repare.srt segments.srt; cat repare.srt
segments.srt: 1 repeated subtitle joined; already in order; 0 subtitles split, 1 merged -> repare.srt
1
00:00:01,000 --> 00:00:04,000
Alpha
Beta
```

`Alpha`, cortado em dois pedaços consecutivos, volta a ser uma única legenda; ocupa então o mesmo intervalo que `Beta`,
e as duas dão apenas uma, de duas linhas, a mais cedo em primeiro lugar.

## Saída

**Saída padrão** — nada: o resultado é o ficheiro escrito. Na saída de erros, no primeiro nível, **uma frase
que encadeia as três contagens**, separadas por pontos e vírgulas: o que `join-repeats`, `sort` e `disjoint` dizem cada um,
com as suas próprias palavras — `N repeated subtitles joined; M subtitles moved; K subtitles split, L merged…`.

### Em JSON

Com [`--format json`](invocation.md#uma-saída-que-um-script-pode-ler), os `counts` de cada ficheiro: `subtitles`, `joined`,
`moved`, depois os de [`disjoint`](disjoint.md#em-json) (`split`, `merged`, `windowed`, `absorbed`, `too_short`,
`ending_before_start`, `extras_dropped`).

## Códigos de saída e erros

Os da ferramenta. Nenhum erro é específico de `repair`, além das definições, que são avaliadas como as de `disjoint`:
`--minimum` que não seja uma duração ou seja negativo, `--max-lines` que não seja um número inteiro, código `1`.
