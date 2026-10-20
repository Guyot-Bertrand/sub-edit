# `append`

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revisada por um leitor do idioma.

```
subedit-cli append --output FILE
                   [--dry-run] [--sort] [--disjoint]
                   <base> <file>...
```

Coloca arquivos **após o primeiro**, em **um único** arquivo: o filme em duas partes, transformado em um
único conjunto de legendas. É o `Append File…` da janela — [o mesmo motor](../subedit-gui/operations.md#append-file),
as mesmas contagens — aplicado tantas vezes quantos forem os arquivos a anexar.

- **O primeiro arquivo é a base.** Ele dá ao resultado **seu formato, sua codificação, sua marca de
  ordem de bytes e seus finais de linha**.
- **Nada é inserido entre dois arquivos**: cada um é deslocado pelo **fim da última legenda do que
  vem antes dele**, com suas posições mantendo a ordem e o espaçamento. O terceiro segue o segundo, não a
  base: os arquivos se seguem na ordem em que são dados.
- **Cada arquivo anexado passa pela conversão** para o formato da base: suas marcações são traduzidas
  e o que esse formato não consegue escrever é dito, arquivo por arquivo, com as palavras da janela.
- **Os arquivos anexados não têm tradução**: um arquivo aberto sozinho não tem nenhuma a oferecer.
- **Um arquivo pode ser dado várias vezes**, ou anexado a si mesmo: nada o proíbe.

**`append` não é um lote.** N entradas, **uma** saída: o laço dos outros subcomandos — um resultado
por arquivo e uma falha que deixa os outros continuarem — não faz sentido aqui. Disso decorrem três coisas:

- **o destino é `--output`, e nada mais.** `--output-dir` e `--in-place` não existem para
  `append` e são recusados como qualquer opção desconhecida, código `1`; `--recursive` também: a ordem dos
  arquivos é a dada, e uma árvore percorrida não tem nenhuma;
- **tudo ou nada**: o primeiro arquivo que não puder ser lido interrompe a execução, e **nada é gravado**. Um
  filme em três partes, uma das quais está ausente, não é um filme em duas;
- **uma única linha por execução** para o resultado e, em JSON, **um único objeto**.

<!-- example: subedit-cli append --help -->
```console
$ subedit-cli append --help
Put subtitle files one after another into a single file
Usage: subedit-cli append [OPTIONS] files...

Positionals:
  files BASE FILE...          The base file, then the files to put after it, in order

Options:
  -h,--help                   Print this help message and exit
  --output FILE               File to write
  --dry-run                   Work out and say what would be written, and write nothing
  --sort                      Put the subtitles in order of their start before writing
  --disjoint                  Cut overlapping subtitles so that no range overlaps another, before writing
```

## Argumentos e opções

| Opção | Obrigatória | Valores | Padrão |
| :----- | :----- | :------ | :----- |
| `<base> <file>...` | sim, **pelo menos dois** | caminhos: a base e depois o que é colocado após ela, em ordem | — |
| `--output` | **sim**, exceto com `--dry-run` | um caminho de arquivo; seu diretório é criado se não existir | — |
| `--dry-run` | não | uma opção sem valor: calcula e informa, não grava nada — veja [Ver antes de escrever](invocation.md#ver-antes-de-escrever) | desativada |
| `--sort` | não | uma opção sem valor: coloca o conjunto na ordem de seus inícios **depois que tudo foi anexado**, veja [Ordenar o resultado](#ordenar-o-resultado) | desativada |
| `--disjoint` | não | uma opção sem valor: corta as legendas que se sobrepõem **depois que tudo foi anexado e ordenado**, veja [Tornar os intervalos disjuntos](#tornar-os-intervalos-disjuntos) | desativada |

**O destino não pode ser nenhuma das entradas**, a base inclusive: gravar sobre um arquivo que se
está lendo é recusado antes que qualquer coisa seja lida, código `1`. Um `--output` existente e distinto das entradas é
sobrescrito, como em todo lugar.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n' > partie1.srt; printf '1\n00:00:00,500 --> 00:00:01,500\ntrois\n\n' > partie2.srt; subedit-cli append --output film.srt partie1.srt partie2.srt; cat film.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n' > partie1.srt; printf '1\n00:00:00,500 --> 00:00:01,500\ntrois\n\n' > partie2.srt; subedit-cli append --output film.srt partie1.srt partie2.srt; cat film.srt
partie2.srt: appended 1 subtitle
partie1.srt: 3 subtitles from 2 files -> film.srt
1
00:00:01,000 --> 00:00:02,000
un

2
00:00:03,000 --> 00:00:04,000
deux

3
00:00:04,500 --> 00:00:05,500
trois
```

A última legenda de `partie1.srt` termina em 4 s; a de `partie2.srt`, que começava em 0.5 s, começa em
4.5 s.

## Ordenar o resultado

Cada arquivo é deslocado **a partir do fim do que vem antes dele**: a ordem que pode se quebrar é, portanto, a dos próprios arquivos. `--sort`
coloca o resultado na ordem dos inícios, **de uma só vez, depois do último anexo**, como [`sort`](sort.md) faria — de forma estável e sem alterar nada
no conteúdo.

Nunca por padrão. Com a opção, a base informa isso no primeiro nível de narração, `<base>: N subtitles moved` ou `<base>: already in order`, e
o registro de `--format json` traz uma contagem `moved`, **ausente sem a opção**.

## Tornar os intervalos disjuntos

Nunca por padrão. A opção faz o que [`disjoint`](disjoint.md) faz, **com seus valores padrão**, depois que tudo foi anexado, sobre o conjunto. Ela exige um arquivo em ordem: caso contrário é recusada, e `--sort` — que vem primeiro — o coloca em ordem antes. O arquivo informa isso no primeiro nível de narração (`<path>: N subtitles split, M merged`), e o registro de `--format json` traz as contagens `split` e `merged`, **ausentes sem a opção**.

## Quando os formatos diferem

O resultado está no **formato da base**, sejam quais forem os dos outros. O que a travessia custa é
dito arquivo por arquivo, com as palavras da janela — a mesma frase que `Append File…` e que
[colar a partir de outro formato](../subedit-gui/clipboard.md#as-marcações-de-outro-formato-são-traduzidas):

```text
d.ass: appended 1 subtitle; Advanced SSA into SubRip: 1 tag dropped, the header was dropped
```

O que se perde depende apenas do formato de **cada** arquivo anexado: uma base em Advanced SSA recebe um
arquivo SubRip sem perder nada, o inverso não vale. Para escolher o formato do resultado, coloque primeiro um arquivo
que já esteja nesse formato, ou converta depois com [`convert`](convert.md).

## Saída

**Saída padrão** — nada: o resultado é o arquivo gravado.

| Nível | O que se acrescenta no erro padrão |
| :----- | :------------------------------------- |
| 1 | uma linha por arquivo anexado — `<path>: appended N subtitles`, seguida, quando o formato custou alguma coisa, de `; <source format> into <base format>: <what is lost>` — depois `<base>: N subtitles from M files -> <destination>` (com `--dry-run`, `(dry run, nothing written)` no lugar de `-> <destination>`) |
| 2 | `<base>: SubRip, UTF-8, no BOM, LF line endings kept` — o formato, a codificação, a marca e os finais de linha da base, recolocados como estavam |
| 3 | `<base>: N bytes read, M written` (todos os arquivos somados), depois **todos os diagnósticos de leitura** — veja [Invocação](invocation.md#diagnósticos-de-leitura) |

### Em JSON

Com [`--format json`](invocation.md#uma-saída-que-um-script-pode-ler), **um único objeto** para a execução inteira,
cujo `file` é **a base**. Suas `counts`:

| Contagem | O que ela diz |
| :----- | :----------- |
| `files` | os arquivos da execução, a base inclusive |
| `subtitles` | as legendas do resultado |
| `appended` | as que vêm dos arquivos anexados |
| `lost_ends`, `lost_header` | `0` ou `1`: um dos arquivos anexados o perdeu |
| `joined_lines`, `lost_tags`, `lost_fields` | a soma do que os arquivos anexados perderam |
| `furthest_ms` | o maior deslocamento de uma posição, todos os arquivos somados |

**Um array `inputs`** acompanha as `counts`, fora delas: um elemento por arquivo anexado, em ordem,
`{"file", "counts"}` — `counts` trazendo `appended` e depois as mesmas perdas, **só para aquele arquivo**. É ele que
diz, numa execução de dez arquivos, qual deles custou o quê. Os avisos de leitura de todos os arquivos
são reunidos em `warnings`.

Uma falha também é um objeto, **um único**: o do primeiro arquivo que não pôde ser lido.

## Códigos de saída

| Código | Quando |
| :--- | :---- |
| `0` | o resultado foi gravado — ou calculado, com `--dry-run` |
| `1` | erro de uso: menos de dois arquivos, nenhum `--output`, uma opção que `append` não tem, um destino que é uma das entradas |
| `2` | **um arquivo não pôde ser lido**, ou o destino não pôde ser gravado: nada é gravado. Não existe código `3`: uma execução que grava um único arquivo tem sucesso ou não |

## Erros

| O que os provoca | Mensagem |
| :------------------- | :------ |
| um único arquivo dado | `append needs a base file and at least one file to put after it` |
| nem `--output` nem `--dry-run` | `no destination given: append writes one file, use --output` |
| `--output` nomeia uma entrada | `<destination>: would be written over the input <input>` |
| um arquivo ausente, ilegível ou de formato desconhecido | `<path>: does not exist`, `<path>: is in no format this tool knows`… — o mesmo que em todo lugar |

**Um arquivo vazio não é anexado**: nenhum formato é lido sem pelo menos uma legenda, e o arquivo é
recusado como qualquer arquivo que não possa ser lido — `<path>: holds nothing recognisable as a subtitle` —, o que
interrompe a execução e não grava nada.
