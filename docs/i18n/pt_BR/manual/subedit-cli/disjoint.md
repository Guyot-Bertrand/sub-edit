# `disjoint`

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revisada por um leitor do idioma.

```
subedit-cli disjoint (--output FILE | --output-dir DIR | --in-place)
                     [--minimum TIME] [--max-lines N] [--dashes] [--sort]
                     [--dry-run] [--recursive]
                     <file>...
```

Corta as legendas que se sobrepõem para que **nenhum intervalo `[start, end]` se sobreponha a outro**. É o item `Make Ranges Disjoint…` da janela — [o mesmo motor](../subedit-gui/lines.md#tornar-os-intervalos-disjuntos).

**Nada é encurtado nem alongado**: a cada instante, o arquivo mostra os mesmos textos de antes. O que
muda é a maneira como a linha do tempo é cortada:

- ela é cortada em **cada início e cada fim** das legendas de um grupo de sobreposição;
- um intervalo coberto por **uma única** legenda continua sendo essa legenda;
- um intervalo coberto por **várias** vira **uma** legenda cujas linhas são as delas, **na
  ordem de seus inícios** — a mais antiga na linha 1, a seguinte na linha 2;
- uma legenda que não se sobrepõe a nenhuma outra não é tocada, e a ordem das demais é mantida.

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

## Argumentos e opções

| Opção | Obrigatório | Valores | Padrão |
| :----- | :----- | :------ | :----- |
| `<file>...` | sim | um ou mais caminhos | — |
| `--minimum` | não | uma duração: `0.08`, `1.5`, `00:00:00,080`. Zero mantém todos os intervalos | `0.08` (cerca de dois quadros) |
| `--max-lines` | não | um número inteiro: o máximo de linhas de uma legenda unida; `0` remove o limite | `2` |
| `--dashes` | não | uma opção sem valor: um travessão diante do texto que cada legenda traz para uma legenda unida | desativada |
| `--sort` | não | uma opção sem valor: coloca as legendas na ordem de seu início **antes** — veja [É preciso um arquivo em ordem](#é-preciso-um-arquivo-em-ordem) | desativada |
| `--recursive`, `-r` | não | uma opção sem valor | desativada — veja [Processar uma árvore de diretórios](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **uma das três**, exceto com `--dry-run` | veja [Invocação](invocation.md#o-destino) | — |
| `--dry-run` | não | uma opção sem valor: calcula e informa, não grava nada — veja [Ver antes de escrever](invocation.md#ver-antes-de-escrever) | desativada |

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

`A [0, 5]` e `B [3, 8]` dão três legendas disjuntas: `A` sozinha, `A` e `B` juntas, `B` sozinha. Nenhum
instante é perdido nem acrescentado.

## O que a operação decide

**Uma linha nunca é duplicada.** Duas legendas com o mesmo texto dão uma única linha onde se
sobrepõem, e dois trechos vizinhos que mostram a mesma coisa formam uma só legenda. O texto de uma
legenda cortada em vários trechos é, fora isso, **repetido em cada um deles**: é o que está na tela
naquele instante.

**Um intervalo muito curto é absorvido por um vizinho.** Abaixo de `--minimum`, um intervalo desaparece: o vizinho
**estende seu intervalo para cobri-lo**, de modo que nenhum instante fica vazio e nada se desloca mais do que esse
limite. O vizinho escolhido é aquele cujas linhas **já contêm** as do intervalo; do contrário, o
mais longo. O texto do intervalo só se perde se nenhum vizinho o carrega, e então por menos que o limite.
**Uma legenda inteira mais curta que o limite nunca é absorvida**: ela é deixada como está e contada.

**Mais linhas que o limite: janelas.** Uma legenda unida que teria mais de `--max-lines` linhas
é exibida **em janelas sucessivas** desse tamanho, em partes iguais da duração. Três linhas ao longo de três
segundos:

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

Quatro linhas dão três janelas, e assim por diante: a janela desliza uma linha por vez. Nada é
truncado nem cortado de novo.

**A tradução segue os mesmos cortes**, já que uma legenda carrega os dois textos. **O que o formato carrega
além disso** — um estilo, uma posição — é o da primeira legenda que cobre o trecho; a perda é contada quando
as legendas unidas diferem.

**Uma legenda cujo fim vem antes do início** não é adivinhada: ela é deixada onde está e contada.

## É preciso um arquivo em ordem

Os grupos são sequências de vizinhas: fora de ordem, as vizinhas não são as que se sobrepõem. Um arquivo
cujos inícios não estão em ordem é, portanto, **recusado**, e a mensagem diz isso:

<!-- example: printf '1\n00:00:03,000 --> 00:00:08,000\nB\n\n2\n00:00:00,000 --> 00:00:05,000\nA\n\n' > desordre.srt; subedit-cli disjoint --output x.srt desordre.srt; echo "code $?" -->
```console
$ printf '1\n00:00:03,000 --> 00:00:08,000\nB\n\n2\n00:00:00,000 --> 00:00:05,000\nA\n\n' > desordre.srt; subedit-cli disjoint --output x.srt desordre.srt; echo "code $?"
desordre.srt: The subtitles are not in the order of their start: sort them first. Use --sort to do that here.
code 2
```

`--sort` faz primeiro a ordenação — como [`sort`](sort.md) — e informa isso. O núcleo nunca ordena por conta própria.

Uma segunda passagem não muda nada: uma vez disjuntos os intervalos, não resta nada a cortar,
e a frase diz `already disjoint`.

## Saída

**Saída padrão** — nada: o resultado é o arquivo gravado.

| Nível | O que se acrescenta na saída de erro |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles split, M merged -> <destination>`, seguido, após um `;`, do que não pôde ser feito por inteiro: `K shown in windows for holding too many lines`, `K very short taken over by a neighbour`, `K subtitles shorter than the limit left whole`, `K merged with styles or positions that differ, the first kept`, `K subtitles ending before the start left alone` — ou `<path>: already disjoint -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, depois **cada diagnóstico de leitura** |

`N subtitles split` conta as legendas originais cortadas em vários trechos, `M merged` os trechos que
mostram várias legendas juntas.

### Em JSON

Com [`--format json`](invocation.md#uma-saída-que-um-script-pode-ler), as `counts` de cada arquivo:

| Contagem | O que informa |
| :----- | :----------- |
| `subtitles` | as legendas do arquivo gravado |
| `split`, `merged` | como na frase |
| `windowed` | os trechos exibidos em janelas |
| `absorbed` | os intervalos absorvidos por um vizinho |
| `too_short` | as legendas mais curtas que o limite, deixadas inteiras |
| `ending_before_start` | as legendas cujo fim vem antes do início, deixadas como estão |
| `extras_dropped` | os trechos unidos cujo estilo ou posição diferem |
| `moved` | somente com `--sort`: as posições que mudaram |

## Códigos de saída

Os da ferramenta: `0` se todos os arquivos foram processados — um arquivo sem sobreposições conta —, `2` se
nenhum, `3` se apenas alguns, `1` em caso de erro de uso.

## Erros

| O que os provoca | Mensagem |
| :------------------- | :------ |
| um arquivo fora de ordem sem `--sort` | `<path>: The subtitles are not in the order of their start: sort them first. Use --sort to do that here.` |
| `--minimum` não é uma duração, ou é negativo | `--minimum: …` |
| `--max-lines` não é um número inteiro | `--max-lines: "…" is not a number of lines: expected a whole number, 0 for no limit` |

Um arquivo ilegível falha como em qualquer subcomando; os erros do destino são comuns aos
subcomandos que gravam: veja [Invocação](invocation.md#o-destino).

## Nos outros comandos

[`convert`](convert.md#tornar-os-intervalos-disjuntos), [`append`](append.md#tornar-os-intervalos-disjuntos) e
[`split-file`](split-file.md#tornar-os-intervalos-disjuntos) aceitam `--disjoint`, que faz a mesma coisa com
as configurações padrão, antes de gravar.

Em um arquivo mal segmentado — um texto cortado em pedaços que se seguem, fornecidos fora de ordem —, ordenar e cortar
sozinhos dão linhas que oscilam de um pedaço para o outro: [`join-repeats`](join-repeats.md) primeiro, ou
[`repair`](repair.md), que encadeia os três.
