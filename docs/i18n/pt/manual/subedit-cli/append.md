# `append`

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revista por nenhum leitor do idioma.

```
subedit-cli append --output FILE
                   [--dry-run] [--sort] [--disjoint]
                   <base> <file>...
```

Coloca ficheiros **depois do primeiro**, num **único** ficheiro: o filme em duas partes, transformado num
único conjunto de legendas. É o `Append File…` da janela — [o mesmo motor](../subedit-gui/operations.md#append-file),
as mesmas contagens — aplicado tantas vezes quantos os ficheiros a anexar.

- **O primeiro ficheiro é a base.** Dá ao resultado **o seu formato, a sua codificação, a sua marca de ordem
  de bytes e os seus fins de linha**.
- **Nada é inserido entre dois ficheiros**: cada um é deslocado pelo **fim da última legenda do que
  o precede**, mantendo as suas posições a ordem e o espaçamento. O terceiro segue o segundo, não a
  base: os ficheiros seguem-se pela ordem em que são dados.
- **Cada ficheiro anexado passa pela conversão** para o formato da base: as suas etiquetas são convertidas,
  e o que esse formato não consegue escrever é dito, ficheiro a ficheiro, com as palavras da janela.
- **Os ficheiros anexados não têm tradução**: um ficheiro aberto sozinho não tem nenhuma para dar.
- **Um ficheiro pode ser dado várias vezes**, ou anexado a si próprio: nada o proíbe.

**`append` não é um lote.** N entradas, **uma** saída: o ciclo dos outros subcomandos — um resultado
por ficheiro, e uma falha que deixa os outros continuar — não faz sentido aqui. Disto decorrem três coisas:

- **o destino é `--output`, e nada mais.** `--output-dir` e `--in-place` não existem para
  `append` e são recusados como qualquer opção desconhecida, código `1`; `--recursive` também: a ordem dos
  ficheiros é a dada, e uma árvore percorrida não tem nenhuma;
- **tudo ou nada**: o primeiro ficheiro que não puder ser lido interrompe a execução, e **nada é escrito**. Um
  filme em três partes, uma das quais falta, não é um filme em duas;
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

| Opção | Obrigatório | Valores | Predefinição |
| :----- | :----- | :------ | :----- |
| `<base> <file>...` | sim, **pelo menos dois** | caminhos: a base, depois o que se lhe segue, por ordem | — |
| `--output` | **sim**, exceto com `--dry-run` | um caminho de ficheiro; a sua pasta é criada se não existir | — |
| `--dry-run` | não | um indicador: calcula e diz, não escreve nada — ver [Ver antes de escrever](invocation.md#ver-antes-de-escrever) | desativado |
| `--sort` | não | um indicador: ordena o conjunto pelo início **depois de tudo anexado**, ver [Ordenar o resultado](#ordenar-o-resultado) | desativado |
| `--disjoint` | não | um indicador: corta as legendas sobrepostas **depois de tudo anexado e ordenado**, ver [Tornar os intervalos disjuntos](#tornar-os-intervalos-disjuntos) | desativado |

**O destino não pode ser nenhuma das entradas**, a base incluída: escrever por cima de um ficheiro que se está a
ler é recusado antes de ler seja o que for, código `1`. Um `--output` existente e distinto das entradas é
substituído, como em todo o lado.

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

A última legenda de `partie1.srt` termina aos 4 s; a de `partie2.srt`, que começava aos 0,5 s, começa aos
4,5 s.

## Ordenar o resultado

Cada ficheiro é deslocado **a partir do fim do que o precede**: a ordem que se pode quebrar é, portanto, a dos próprios ficheiros. `--sort`
põe o resultado pela ordem dos inícios, **de uma só vez, depois do último anexo**, como o faria [`sort`](sort.md) — estável e sem alterar nada
no conteúdo.

Nunca por predefinição. Com a opção, a base di-lo no primeiro nível de narração, `<base>: N subtitles moved` ou `<base>: already in order`, e
o registo de `--format json` traz uma contagem `moved`, **ausente sem a opção**.

## Tornar os intervalos disjuntos

Nunca por predefinição. A opção faz o que [`disjoint`](disjoint.md) faz, **com as suas definições predefinidas**, depois de tudo anexado, sobre o conjunto. Precisa de um ficheiro ordenado: caso contrário, é recusada, e `--sort` — que vem primeiro — ordena-o previamente. O ficheiro di-lo no primeiro nível de narração (`<path>: N subtitles split, M merged`), e o registo de `--format json` traz as contagens `split` e `merged`, **ausentes sem a opção**.

## Quando os formatos diferem

O resultado está **no formato da base**, sejam quais forem os dos outros. O que a passagem custa é
dito ficheiro a ficheiro, com as palavras da janela — a mesma frase que `Append File…` e que
[colar a partir de outro formato](../subedit-gui/clipboard.md#as-etiquetas-de-outro-formato-são-convertidas):

```text
d.ass: appended 1 subtitle; Advanced SSA into SubRip: 1 tag dropped, the header was dropped
```

O que se perde depende apenas do formato de **cada** ficheiro anexado: uma base em Advanced SSA recebe um
ficheiro SubRip sem perder nada, o inverso não. Para escolher o formato do resultado, ponha primeiro um ficheiro
que já esteja nesse formato, ou converta depois com [`convert`](convert.md).

## Saída

**Saída padrão** — nada: o resultado é o ficheiro escrito.

| Nível | O que se acrescenta à saída de erros |
| :----- | :------------------------------------- |
| 1 | uma linha por ficheiro anexado — `<path>: appended N subtitles`, seguida, quando o formato custou alguma coisa, de `; <source format> into <base format>: <what is lost>` — depois `<base>: N subtitles from M files -> <destination>` (com `--dry-run`, `(dry run, nothing written)` em vez de `-> <destination>`) |
| 2 | `<base>: SubRip, UTF-8, no BOM, LF line endings kept` — o formato, a codificação, a marca e os fins de linha da base, repostos como estavam |
| 3 | `<base>: N bytes read, M written` (todos os ficheiros em conjunto), depois **cada diagnóstico de leitura** — ver [Invocação](invocation.md#ler-os-diagnósticos) |

### Em JSON

Com [`--format json`](invocation.md#uma-saída-que-um-script-pode-ler), **um único objeto** para toda a execução,
cujo `file` é **a base**. As suas `counts`:

| Contagem | O que diz |
| :----- | :----------- |
| `files` | os ficheiros da execução, a base incluída |
| `subtitles` | as legendas do resultado |
| `appended` | as que vêm dos ficheiros anexados |
| `lost_ends`, `lost_header` | `0` ou `1`: um dos ficheiros anexados perdeu-o |
| `joined_lines`, `lost_tags`, `lost_fields` | a soma do que os ficheiros anexados perderam |
| `furthest_ms` | o maior deslocamento de uma posição, todos os ficheiros em conjunto |

**Uma matriz `inputs`** acompanha as `counts`, fora delas: um elemento por ficheiro anexado, por ordem,
`{"file", "counts"}` — com `counts` a trazer `appended`, depois as mesmas perdas, **só para esse ficheiro**. É o que
diz, numa execução de dez ficheiros, qual deles custou o quê. Os avisos de leitura de todos os ficheiros
são reunidos em `warnings`.

Uma falha também é um objeto, **um único**: o do primeiro ficheiro que não pôde ser lido.

## Códigos de saída

| Código | Quando |
| :--- | :---- |
| `0` | o resultado é escrito — ou calculado, com `--dry-run` |
| `1` | erro de utilização: menos de dois ficheiros, nenhum `--output`, uma opção que `append` não tem, um destino que é uma das entradas |
| `2` | **um ficheiro não pôde ser lido**, ou o destino não pôde ser escrito: nada é escrito. Não há código `3`: uma execução que escreve um único ficheiro tem êxito ou não |

## Erros

| O que os desencadeia | Mensagem |
| :------------------- | :------ |
| um único ficheiro dado | `append needs a base file and at least one file to put after it` |
| nem `--output` nem `--dry-run` | `no destination given: append writes one file, use --output` |
| `--output` designa uma entrada | `<destination>: would be written over the input <input>` |
| um ficheiro em falta, ilegível ou de formato desconhecido | `<path>: does not exist`, `<path>: is in no format this tool knows`… — o mesmo que em todo o lado |

**Um ficheiro vazio não é anexado**: nenhum formato lê sem pelo menos uma legenda, e o ficheiro é
recusado como qualquer ficheiro que não possa ser lido — `<path>: holds nothing recognisable as a subtitle` —, o que
interrompe a execução e não escreve nada.
