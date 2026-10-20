# `split-file`

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revista por nenhum leitor do idioma.

```
subedit-cli split-file --at N --head FILE --tail FILE
                       [--dry-run] [--sort] [--disjoint]
                       <file>
```

Corta um ficheiro **em dois**: uma cabeça e uma cauda. É o `Split Project…` da janela — [o mesmo motor](../subedit-gui/operations.md#split-project),
as mesmas contagens, a mesma recusa — e **o inverso exato de [`append`](append.md)**: anexar a cauda à
cabeça devolve o ficheiro de partida.

- **`--at N` é o número da primeira legenda da cauda**, numerado como a tabela da janela
  o numera: a partir de 1. `--at 3` põe as legendas 1 e 2 na cabeça, e a terceira e as restantes na
  cauda. Vai **da segunda à última**: um corte precisa de uma legenda de cada lado.
- **A cauda é trazida de volta à origem**: é deslocada do valor do fim da última legenda que fica na cabeça, com sinal negativo,
  conservando as suas posições a ordem e o espaçamento. É isso que faz de `split-file` o inverso de `append`.
- **As duas metades mantêm o formato, a codificação, a marca de ordem de bytes e os fins de linha do ficheiro lido**, e
  são renumeradas a partir de 1 pela escrita, como qualquer ficheiro escrito de volta.
- **Duas saídas, portanto um destino próprio**: `--head` e `--tail`, **ambos obrigatórios**. `--output`,
  `--output-dir`, `--in-place` e `--recursive` não existem para `split-file` e são recusados como qualquer opção
  desconhecida, código `1`: não há nem uma pasta onde repartir dois ficheiros com nome, nem uma entrada a sobrescrever.

<!-- example: subedit-cli split-file --help -->
```console
$ subedit-cli split-file --help
Cut a subtitle file in two, a head and a tail
Usage: subedit-cli split-file [OPTIONS] file

Positionals:
  file TEXT REQUIRED          Subtitle file to cut

Options:
  -h,--help                   Print this help message and exit
  --at N                      Number of the first subtitle of the tail, as the table numbers them
  --head FILE                 File to write the subtitles before the cut to
  --tail FILE                 File to write the subtitles from the cut on to
  --dry-run                   Work out and say what would be written, and write nothing
  --sort                      Put the subtitles in order of their start before cutting; --at counts in that order
  --disjoint                  Cut overlapping subtitles so that no range overlaps another, before cutting
```

## Argumentos e opções

| Opção | Obrigatório | Valores | Predefinição |
| :----- | :----- | :------ | :----- |
| `<file>` | sim | um **único** caminho | — |
| `--at` | sim | um número inteiro, de 2 ao número de legendas do ficheiro | — |
| `--head`, `--tail` | **sim, ambos**, exceto com `--dry-run` | caminhos de ficheiros; a sua pasta é criada se não existir | — |
| `--dry-run` | não | um indicador: calcula e diz, não escreve nada — ver [Ver antes de escrever](invocation.md#ver-antes-de-escrever) | desativado |
| `--sort` | não | um indicador: põe as legendas por ordem do seu início **antes de cortar**; `--at` conta nessa ordem | desativado |
| `--disjoint` | não | um indicador: corta as legendas sobrepostas **antes de cortar**; `--at` conta no que sobra, ver [Tornar os intervalos disjuntos](#tornar-os-intervalos-disjuntos) | desativado |

**Com `--dry-run`, nenhum destino é obrigatório**: pode não se indicar nenhum, mas não só um dos dois —
`--head` e `--tail` vão juntos. Os que forem indicados são avaliados como sem `--dry-run`.

**Os dois destinos não podem ser nem a entrada nem o mesmo ficheiro**: escrever sobre o que se
está a ler, ou escrever a cauda sobre a cabeça, é recusado antes de se ler fosse o que fosse, código `1`.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n3\n00:00:06,000 --> 00:00:07,000\ntrois\n\n4\n00:00:08,000 --> 00:00:09,000\nquatre\n\n' > film.srt; subedit-cli split-file --at 3 --head tete.srt --tail queue.srt film.srt; cat queue.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n3\n00:00:06,000 --> 00:00:07,000\ntrois\n\n4\n00:00:08,000 --> 00:00:09,000\nquatre\n\n' > film.srt; subedit-cli split-file --at 3 --head tete.srt --tail queue.srt film.srt; cat queue.srt
film.srt: split at subtitle 3: 2 subtitles in the head, 2 subtitles in the tail -> tete.srt, queue.srt
1
00:00:02,000 --> 00:00:03,000
trois

2
00:00:04,000 --> 00:00:05,000
quatre
```

A segunda legenda, que fica na cabeça, termina aos 4 s: a cauda recua 4 s, e a terceira legenda,
que começava aos 6 s, passa a começar aos 2 s.

## Cortar por ordem cronológica

`--at` numera as legendas **como o ficheiro as dá**. Num ficheiro cuja ordem está quebrada, a cabeça não é, portanto, a metade mais antiga
no tempo. `--sort` põe as legendas pela ordem dos seus inícios **antes de avaliar o corte** — de forma estável, como o faz [`sort`](sort.md) —, e
`--at` conta então nessa ordem.

Nunca por predefinição. Com a opção, o ficheiro diz, no primeiro nível de narração, quantas legendas se moveram (`<path>: N subtitles moved`
ou `<path>: already in order`), e o registo de `--format json` traz uma contagem `moved`, **ausente sem a opção**.

## Tornar os intervalos disjuntos

Nunca por predefinição. A opção faz o que [`disjoint`](disjoint.md) faz, **com as suas definições predefinidas**, antes de avaliar o corte, depois da ordenação: `--at` conta no que daí resulta. Exige um ficheiro ordenado: caso contrário é recusada, e `--sort` — que vem primeiro — põe-no antes por ordem. O ficheiro di-lo no primeiro nível de narração (`<path>: N subtitles split, M merged`), e o registo de `--format json` traz as contagens `split` e `merged`, **ausentes sem a opção**.

## Quando o corte é recusado

**Duas metades que se sobrepõem no corte não são cortadas.** Se uma legenda da cauda começasse antes do
fim da última legenda deixada para trás, o deslocamento levá-la-ia para antes do início do vídeo — uma posição negativa,
que nenhum formato escreve. `split-file` recusa **nomeando a legenda**, nas palavras da janela:

```text
o.srt: Cannot split at subtitle 3: subtitle 3 would fall before the start of the video. Cut somewhere else.
```

O código é `2` e **nada é escrito**. Corte-se noutro sítio. Cair exatamente no início é permitido: zero é uma
posição.

**`--at` fora do ficheiro é recusado da mesma maneira**, depois de o ficheiro ser lido: `--at 1` — a primeira legenda não pode
iniciar uma cauda, nada sobraria — e `--at` para além da última legenda, que deixaria uma cauda
vazia. Um ficheiro com uma única legenda não pode ser cortado.

## Saída

**Saída padrão** — nada: o resultado são os dois ficheiros escritos.

| Nível | O que se acrescenta à saída de erros |
| :----- | :------------------------------------- |
| 1 | `<path>: split at subtitle N: H subtitles in the head, T subtitles in the tail -> <head>, <tail>`; com `--dry-run`, `(dry run, nothing written)` no lugar de `-> …` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — o formato, a codificação, a marca e os fins de linha do ficheiro lido, repostos como estavam |
| 3 | `<path>: N bytes read, H and T written`, depois **todos os diagnósticos de leitura** — ver [Invocação](invocation.md#ler-os-diagnósticos) |

### Em JSON

Com [`--format json`](invocation.md#uma-saída-que-um-script-pode-ler), **um único objeto**, cujo `file` é o ficheiro cortado.
**`destination` é a cabeça** — o ficheiro em que a entrada se transforma — e **`tail` a cauda**, um caminho, ou `null` com
`--dry-run` tal como `destination`. Os `counts`: `subtitles` (as do ficheiro lido), `head` e `tail` (as de cada
metade).

## Códigos de saída

| Código | Quando |
| :--- | :---- |
| `0` | os dois ficheiros são escritos — ou calculados, com `--dry-run` |
| `1` | erro de utilização: falta `--head` e `--tail`, uma opção que `split-file` não tem, um destino que é a entrada ou o outro destino, um `--at` que não é um número inteiro positivo |
| `2` | o ficheiro não pôde ser lido, **o corte é recusado** (fora do ficheiro, ou sobreposição), ou um destino não pôde ser escrito. Não há código `3`: processa-se um único ficheiro |

**A cabeça é escrita em primeiro lugar**: um disco que recuse a cauda deixa a cabeça atrás de si, e o código é `2`.

## Erros

| O que os desencadeia | Mensagem |
| :------------------- | :------ |
| nem `--head` nem `--tail` | `no destination given: split-file writes two files, use --head and --tail` |
| só um dos dois, com `--dry-run` | `--head and --tail go together: give both, or neither with --dry-run` |
| os dois destinos são o mesmo ficheiro | `<path>: named as both the head and the tail` |
| um destino é a entrada | `<path>: would be written over the input <input>` |
| `--at` fora do ficheiro | `<file>: --at 9: the file holds 3 subtitles, and the tail can start from 2 to 3` |
| um ficheiro com uma legenda | `<file>: a file of 1 subtitle cannot be split: a cut needs a subtitle on each side` |
| sobreposição | `<file>: Cannot split at subtitle N: subtitle M would fall before the start of the video. Cut somewhere else.` |
| um ficheiro em falta, ilegível ou de formato desconhecido | `<path>: does not exist`… — o mesmo que em todo o lado |
