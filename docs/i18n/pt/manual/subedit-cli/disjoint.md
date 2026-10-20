# `disjoint`

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revista por nenhum leitor do idioma.

```
subedit-cli disjoint (--output FILE | --output-dir DIR | --in-place)
                     [--minimum TIME] [--max-lines N] [--dashes] [--sort]
                     [--dry-run] [--recursive]
                     <file>...
```

Corta as legendas sobrepostas de modo que **nenhum intervalo `[start, end]` se sobreponha
a outro**. É o `Make Ranges Disjoint…` da janela — [o mesmo motor](../subedit-gui/lines.md#tornar-os-intervalos-disjuntos).

**Nada é encurtado nem prolongado**: a cada instante, o ficheiro mostra os mesmos textos que antes. O que
muda é a maneira como a linha cronológica é cortada:

- corta-se em **cada início e em cada fim** das legendas de um grupo sobreposto;
- um intervalo coberto por **uma única** legenda continua a ser essa legenda;
- um intervalo coberto por **várias** passa a ser **uma** legenda cujas linhas são as delas, **pela
  ordem dos respetivos inícios** — a mais antiga na linha 1, a seguinte na linha 2;
- uma legenda que não se sobrepõe a nenhuma não é tocada, e a ordem das restantes é mantida.

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

| Opção | Obrigatória | Valores | Predefinição |
| :----- | :----- | :------ | :----- |
| `<file>...` | sim | um ou mais caminhos | — |
| `--minimum` | não | uma duração: `0.08`, `1.5`, `00:00:00,080`. Zero mantém todos os intervalos | `0.08` (cerca de dois fotogramas) |
| `--max-lines` | não | um número inteiro: o máximo de linhas de uma legenda unida; `0` levanta o limite | `2` |
| `--dashes` | não | um indicador: um hífen à frente do texto que cada legenda traz a uma legenda unida | desativado |
| `--sort` | não | um indicador: põe as legendas pela ordem do respetivo início **primeiro** — ver [É necessário um ficheiro ordenado](#é-necessário-um-ficheiro-ordenado) | desativado |
| `--recursive`, `-r` | não | um indicador | desativado — ver [Processar uma árvore de pastas](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **uma das três**, exceto com `--dry-run` | ver [Invocação](invocation.md#o-destino) | — |
| `--dry-run` | não | um indicador: calcula e comunica, não escreve nada — ver [Ver antes de escrever](invocation.md#ver-antes-de-escrever) | desativado |

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

**Uma linha nunca é duplicada.** Duas legendas com o mesmo texto dão uma única linha onde
se sobrepõem, e dois pedaços vizinhos que mostram o mesmo conteúdo formam apenas uma legenda. O texto de uma
legenda cortada em vários pedaços é, de resto, **repetido em cada um**: é o que está no ecrã nesse
instante.

**Um intervalo muito curto é absorvido por um vizinho.** Abaixo de `--minimum`, um intervalo desaparece: o vizinho
**alarga o seu intervalo para o cobrir**, de modo que nenhum instante fica vazio e nada se desloca mais do que esse
limiar. O vizinho escolhido é aquele cujas linhas **já contêm** as do intervalo, ou então o
mais longo. O texto do intervalo só se perde se nenhum vizinho o transportar, e então por menos do que o limiar.
**Uma legenda inteira mais curta do que o limiar nunca é absorvida**: fica como está, e é contada.

**Mais linhas do que o limite: janelas.** Uma legenda unida que teria mais de `--max-lines` linhas
é mostrada **em janelas sucessivas** desse tamanho, em partes iguais da duração. Três linhas ao longo de três
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

Quatro linhas dão três janelas, e assim por diante: a janela desliza uma linha de cada vez. Nada é
truncado nem cortado de novo.

**A tradução segue os mesmos cortes**, uma vez que uma legenda traz os dois textos. **O que o formato transporta
a mais** — um estilo, uma posição — é o da primeira legenda que cobre; a perda é contada quando
as legendas unidas diferem.

**Uma legenda cujo fim vem antes do início** não é adivinhada: fica onde está, e é contada.

## É necessário um ficheiro ordenado

Os grupos são sequências de vizinhas: fora de ordem, as vizinhas não são as que se sobrepõem. Um ficheiro
cujos inícios não estão por ordem é, por isso, **recusado**, e a mensagem di-lo:

<!-- example: printf '1\n00:00:03,000 --> 00:00:08,000\nB\n\n2\n00:00:00,000 --> 00:00:05,000\nA\n\n' > desordre.srt; subedit-cli disjoint --output x.srt desordre.srt; echo "code $?" -->
```console
$ printf '1\n00:00:03,000 --> 00:00:08,000\nB\n\n2\n00:00:00,000 --> 00:00:05,000\nA\n\n' > desordre.srt; subedit-cli disjoint --output x.srt desordre.srt; echo "code $?"
desordre.srt: The subtitles are not in the order of their start: sort them first. Use --sort to do that here.
code 2
```

`--sort` faz primeiro a ordenação — o mesmo que [`sort`](sort.md) — e di-lo. O núcleo nunca ordena por iniciativa própria.

Uma segunda passagem não altera nada: uma vez disjuntos os intervalos, não há mais nada a cortar, e a
frase diz `already disjoint`.

## Saída

**Saída padrão** — nada: o resultado é o ficheiro escrito.

| Nível | O que se acrescenta à saída de erros |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles split, M merged -> <destination>`, seguido, depois de um `;`, do que não pôde ser feito por completo: `K shown in windows for holding too many lines`, `K very short taken over by a neighbour`, `K subtitles shorter than the limit left whole`, `K merged with styles or positions that differ, the first kept`, `K subtitles ending before the start left alone` — ou `<path>: already disjoint -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, e depois **cada diagnóstico de leitura** |

`N subtitles split` conta as legendas originais cortadas em vários pedaços, `M merged` os pedaços que
mostram várias legendas em conjunto.

### Em JSON

Com [`--format json`](invocation.md#uma-saída-que-um-script-pode-ler), os `counts` de cada ficheiro:

| Contagem | O que diz |
| :----- | :----------- |
| `subtitles` | as legendas do ficheiro escrito |
| `split`, `merged` | como na frase |
| `windowed` | os pedaços mostrados em janelas |
| `absorbed` | os intervalos absorvidos por um vizinho |
| `too_short` | as legendas mais curtas do que o limiar, deixadas inteiras |
| `ending_before_start` | as legendas cujo fim vem antes do início, deixadas como estão |
| `extras_dropped` | os pedaços unidos cujo estilo ou posição diferem |
| `moved` | só com `--sort`: os lugares que mudaram |

## Códigos de saída

Os da ferramenta: `0` se todos os ficheiros foram processados — um ficheiro sem sobreposições conta como tal —, `2` se
nenhum, `3` se apenas alguns, `1` perante um erro de utilização.

## Erros

| O que os desencadeia | Mensagem |
| :------------------- | :------ |
| um ficheiro fora de ordem sem `--sort` | `<path>: The subtitles are not in the order of their start: sort them first. Use --sort to do that here.` |
| `--minimum` não é uma duração, ou é negativo | `--minimum: …` |
| `--max-lines` não é um número inteiro | `--max-lines: "…" is not a number of lines: expected a whole number, 0 for no limit` |

Um ficheiro ilegível falha como em qualquer subcomando; os erros do destino são comuns aos
subcomandos que escrevem: ver [Invocação](invocation.md#o-destino).

## Nos outros comandos

[`convert`](convert.md#tornar-os-intervalos-disjuntos), [`append`](append.md#tornar-os-intervalos-disjuntos) e
[`split-file`](split-file.md#tornar-os-intervalos-disjuntos) aceitam `--disjoint`, que faz o mesmo com
as definições predefinidas, antes de escrever.

Num ficheiro mal segmentado — um texto cortado em pedaços que se sucedem, dado fora de ordem —, ordenar e cortar
sozinhos dão linhas que passam de um pedaço para o outro: [`join-repeats`](join-repeats.md) primeiro, ou
[`repair`](repair.md), que encadeia as três.
