# `split-file`

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revisada por um leitor do idioma.

```
subedit-cli split-file --at N --head FILE --tail FILE
                       [--dry-run] [--sort] [--disjoint]
                       <file>
```

Corta um arquivo **em dois**: uma cabeça e uma cauda. É o `Split Project…` da janela — [o mesmo motor](../subedit-gui/operations.md#split-project),
as mesmas contagens, a mesma recusa — e **o inverso exato de [`append`](append.md)**: anexar a cauda à
cabeça devolve o arquivo de partida.

- **`--at N` é o número da primeira legenda da cauda**, numerado como a tabela da janela
  o numera: a partir de 1. `--at 3` coloca as legendas 1 e 2 na cabeça, e a terceira e as demais na
  cauda. Vai **da segunda à última**: um corte precisa de uma legenda de cada lado.
- **A cauda é trazida de volta à origem**: ela é deslocada pelo fim da última legenda que ficou na cabeça, com sinal negativo,
  mantendo suas posições a ordem e o espaçamento. É isso que faz de `split-file` o inverso de `append`.
- **As duas metades mantêm o formato, a codificação, a marca de ordem de bytes e os finais de linha do arquivo lido**, e
  são renumeradas a partir de 1 pela gravação, como qualquer arquivo gravado de novo.
- **Duas saídas, portanto um destino próprio para cada**: `--head` e `--tail`, **ambos obrigatórios**. `--output`,
  `--output-dir`, `--in-place` e `--recursive` não existem em `split-file` e são recusados como qualquer
  opção desconhecida, código `1`: não há diretório para repartir dois arquivos nomeados, nem entrada para sobrescrever.

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

| Opção | Obrigatório | Valores | Padrão |
| :----- | :----- | :------ | :----- |
| `<file>` | sim | **um único** caminho | — |
| `--at` | sim | um número inteiro, de 2 ao número de legendas do arquivo | — |
| `--head`, `--tail` | **sim, ambos**, exceto com `--dry-run` | caminhos de arquivo; o diretório deles é criado se não existir | — |
| `--dry-run` | não | uma opção sem valor: calcula e informa, não grava nada — veja [Ver antes de escrever](invocation.md#ver-antes-de-escrever) | desativado |
| `--sort` | não | uma opção sem valor: coloca as legendas na ordem de seu início **antes de cortar**; `--at` conta nessa ordem | desativado |
| `--disjoint` | não | uma opção sem valor: corta as legendas sobrepostas **antes de cortar**; `--at` conta no que sobra, veja [Tornar os intervalos disjuntos](#tornar-os-intervalos-disjuntos) | desativado |

**Com `--dry-run`, nenhum destino é exigido**: nenhum pode ser informado, mas não apenas um dos dois —
`--head` e `--tail` andam juntos. Os que forem informados são julgados como sem `--dry-run`.

**Os dois destinos não podem ser nem a entrada nem o mesmo arquivo**: gravar sobre o que se
está lendo, ou gravar a cauda sobre a cabeça, é recusado antes de qualquer leitura, código `1`.

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

A segunda legenda, que ficou na cabeça, termina em 4 s: a cauda recua 4 s, e a terceira legenda,
que começava em 6 s, passa a começar em 2 s.

## Cortar em ordem temporal

`--at` numera as legendas **como o arquivo as fornece**. Em um arquivo de ordem quebrada, a cabeça não é, portanto, a metade mais antiga
no tempo. `--sort` coloca as legendas na ordem de seus inícios **antes de julgar o corte** — de forma estável, como faz [`sort`](sort.md) —, e
`--at` passa então a contar nessa ordem.

Nunca por padrão. Com a opção, o arquivo informa no primeiro nível de narração quantas legendas se moveram (`<path>: N subtitles moved`
ou `<path>: already in order`), e o registro de `--format json` traz uma contagem `moved`, **ausente sem a opção**.

## Tornar os intervalos disjuntos

Nunca por padrão. A opção faz o que [`disjoint`](disjoint.md) faz, **com suas configurações padrão**, antes de julgar o corte, depois da ordenação: `--at` conta no que isso resulta. Ela precisa de um arquivo em ordem: caso contrário é recusada, e `--sort` — que vem primeiro — o coloca em ordem de antemão. O arquivo o informa no primeiro nível de narração (`<path>: N subtitles split, M merged`), e o registro de `--format json` traz as contagens `split` e `merged`, **ausentes sem a opção**.

## Quando o corte é recusado

**Duas metades que se sobrepõem no corte não são cortadas.** Se uma legenda da cauda começava antes do
fim da última legenda deixada para trás, o deslocamento a levaria para antes do início do vídeo — uma posição negativa,
que nenhum formato grava. `split-file` recusa **nomeando a legenda**, nas palavras da janela:

```text
o.srt: Cannot split at subtitle 3: subtitle 3 would fall before the start of the video. Cut somewhere else.
```

O código é `2` e **nada é gravado**. Corte em outro lugar. Cair exatamente sobre o início é permitido: zero é uma
posição.

**`--at` fora do arquivo é recusado da mesma forma**, uma vez lido o arquivo: `--at 1` — a primeira legenda não pode
iniciar uma cauda, nada sobraria — e `--at` além da última legenda, que deixaria uma cauda
vazia. Um arquivo de uma única legenda não pode ser cortado.

## Saída

**Saída padrão** — nada: o resultado são os dois arquivos gravados.

| Nível | O que se acrescenta na saída de erro |
| :----- | :------------------------------------- |
| 1 | `<path>: split at subtitle N: H subtitles in the head, T subtitles in the tail -> <head>, <tail>`; com `--dry-run`, `(dry run, nothing written)` no lugar de `-> …` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — o formato, a codificação, a marca e os finais de linha do arquivo lido, restituídos como estavam |
| 3 | `<path>: N bytes read, H and T written`, depois **todo diagnóstico de leitura** — veja [Invocação](invocation.md#diagnósticos-de-leitura) |

### Em JSON

Com [`--format json`](invocation.md#uma-saída-que-um-script-pode-ler), **um único objeto**, cujo `file` é o arquivo cortado.
**`destination` é a cabeça** — o arquivo em que a entrada se transforma — e **`tail` a cauda**, um caminho, ou `null` com
`--dry-run`, como `destination`. Os `counts`: `subtitles` (os do arquivo lido), `head` e `tail` (os de cada
metade).

## Códigos de saída

| Código | Quando |
| :--- | :---- |
| `0` | os dois arquivos são gravados — ou calculados, com `--dry-run` |
| `1` | erro de uso: nenhum `--head` e `--tail`, uma opção que `split-file` não tem, um destino que é a entrada ou o outro destino, um `--at` que não é um número inteiro positivo |
| `2` | o arquivo não pôde ser lido, **o corte é recusado** (fora do arquivo, ou sobreposição), ou um destino não pôde ser gravado. Não há código `3`: um único arquivo é processado |

**A cabeça é gravada primeiro**: um disco que recusa a cauda deixa a cabeça gravada, e o código é `2`.

## Erros

| O que os provoca | Mensagem |
| :------------------- | :------ |
| nem `--head` nem `--tail` | `no destination given: split-file writes two files, use --head and --tail` |
| apenas um dos dois, com `--dry-run` | `--head and --tail go together: give both, or neither with --dry-run` |
| os dois destinos são o mesmo arquivo | `<path>: named as both the head and the tail` |
| um destino é a entrada | `<path>: would be written over the input <input>` |
| `--at` fora do arquivo | `<file>: --at 9: the file holds 3 subtitles, and the tail can start from 2 to 3` |
| um arquivo de uma legenda | `<file>: a file of 1 subtitle cannot be split: a cut needs a subtitle on each side` |
| sobreposição | `<file>: Cannot split at subtitle N: subtitle M would fall before the start of the video. Cut somewhere else.` |
| um arquivo ausente, ilegível ou de formato desconhecido | `<path>: does not exist`… — o mesmo de sempre |
