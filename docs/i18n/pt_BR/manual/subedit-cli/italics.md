# `italics`

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revisada por um leitor do idioma.

```
subedit-cli italics (--on | --off) [--range N-M|N-]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    <file>...
```

Coloca os textos **em itálico**, ou **tira o itálico deles**, nas marcações do formato do arquivo.
É o item `Italic` da [janela](../subedit-gui/operations.md).

**Duas palavras e não um alternador**: o botão da janela pergunta ao núcleo em que sentido
atuar; aqui, o sentido é dito. `--on` e `--off` se excluem, e uma das duas é necessária — omiti-las, ou dar as duas,
é um erro de uso (código `1`).

- **`--on`** envolve cada texto em um único par de marcações, **depois de remover as que
  havia nele**: um texto já em itálico não é envolvido duas vezes, e um itálico cortado em dois
  é refeito em um só trecho. Em várias linhas, um único par cobre o conjunto.
- **`--off`** remove as marcações de itálico; **o resto de uma marcação é mantido** (`{\b1\i1}` dá
  `{\b1}`).
- O texto é escrito **como o formato do arquivo escreve o itálico**: `<i>…</i>` para SubRip, WebVTT e
  SubViewer 2, `{\i1}…{\i0}` para os dois Sub Station Alpha, `{Y:i}` para MicroDVD.

<!-- example: subedit-cli italics --help -->
```console
$ subedit-cli italics --help
Put the texts in italics, or take their italics out
Usage: subedit-cli italics [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --on                        Put the texts in italics
  --off                       Take the italics out
  --range N-M|N-              Act only on subtitles N to M, or N to the end
  --document main|translation The document to change: the main one, or the translation given by -t
  -t,--translation-file FILE  Translation file to lay over the subtitle file, for a single input
  --align-method position|number
                              How the lines of the translation find their subtitles
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Argumentos e opções

| Opção | Obrigatório | Valores | Padrão |
| :----- | :----- | :------ | :----- |
| `<file>...` | sim | um ou mais caminhos | — |
| `--recursive`, `-r` | não | uma opção sem valor | desativada — veja [Processar uma árvore de diretórios](lots.md) |
| `--on` / `--off` | **uma das duas** | opções sem valor | — |
| `--range` | não | `N-M`, ou `N-` até o fim — veja [`adjust`](adjust.md#--range) | o arquivo inteiro |
| `--document` | não | `main` ou `translation` — veja [Uma tradução](invocation.md#uma-tradução) | `main` |
| `-t`, `--translation-file` | com `--document translation` | um arquivo de tradução, para **uma única** entrada | — |
| `--align-method` | não, e somente com `-t` | `position` ou `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **uma das três**, exceto com `--dry-run` | veja [Invocação](invocation.md#o-destino) | — |
| `--dry-run` | não | uma opção sem valor: calcula e informa, não grava nada — veja [Ver antes de escrever](invocation.md#ver-antes-de-escrever) | desativada |

**O texto principal por padrão; a tradução com `--document translation -t FILE`.** É então
a tradução que é alterada e gravada, e somente ela — veja [Uma tradução](invocation.md#uma-tradução).

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour\nMarie\n\n' > a.srt; subedit-cli italics --on --output i.srt a.srt; cat i.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour\nMarie\n\n' > a.srt; subedit-cli italics --on --output i.srt a.srt; cat i.srt
a.srt: 2 subtitles put in italics -> i.srt
1
00:00:01,000 --> 00:00:02,000
<i>bonjour marie</i>

2
00:00:03,000 --> 00:00:04,000
<i>Bonjour
Marie</i>
```

## Um formato sem estilo recusa

**TMPlayer e LRC não escrevem estilo**: não há itálico a pôr nem itálico a tirar. A
janela deixa o item acinzentado em vez de ocultá-lo; a linha de comando **recusa, arquivo por
arquivo, com o motivo**, e os outros arquivos do lote seguem em frente. O que conta é o que o
formato pode carregar, não a maneira como o escreve: SubRip e WebVTT escrevem o itálico da mesma
maneira, e não dizem a mesma coisa sobre a cor.

<!-- example: printf '00:00:01:Hello there\n00:00:03:Bye\n' > a.txt; subedit-cli italics --on --output-dir sortie a.txt; echo "code=$?" -->
```console
$ printf '00:00:01:Hello there\n00:00:03:Bye\n' > a.txt; subedit-cli italics --on --output-dir sortie a.txt; echo "code=$?"
a.txt: TMPlayer writes no style: there are no italics to put on
code=2
```

Em JSON, a falha traz o identificador **`no-style`**.

## Saída

**Saída padrão** — nada: o resultado é o arquivo gravado. Com `--dry-run`, a lista das
alterações, descrita em [Ver antes de escrever](invocation.md#ver-antes-de-escrever).

| Nível | O que se acrescenta na saída de erro |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles put in italics -> <destination>`, ou `N subtitles taken out of italics`, ou `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — o formato, a codificação, a marca e os finais de linha do arquivo lido, restituídos como estavam |
| 3 | `<path>: N bytes read, M written`, depois **cada diagnóstico de leitura** — veja [Invocação](invocation.md#diagnósticos-de-leitura) |

`N` é o número de legendas **cujo texto mudou**, lido da própria operação.

### Em JSON

Com [`--format json`](invocation.md#uma-saída-que-um-script-pode-ler), o objeto de cada arquivo traz
`counts.changed` — o `N` da frase — e `changes`, a lista das legendas alteradas
(`{"subtitle", "document", "before", "after"}`, **vazia** quando nada mudou).

## Códigos de saída

Os da ferramenta: `0` se todos os arquivos foram processados — um arquivo em que nada muda conta —,
`2` se nenhum, `3` se apenas alguns, `1` em caso de erro de uso.

## Erros

| O que o provoca | Mensagem |
| :------------------ | :------ |
| nem `--on` nem `--off` | `italics needs --on to put the texts in italics, or --off` |
| as duas | `--on and --off say opposite things; give one of them` |
| um formato sem estilo | `<path>: TMPlayer writes no style: there are no italics to put on` (ou `take out`) |
| intervalo mal escrito ou fora do arquivo | veja [`adjust`](adjust.md#--range) |

Os erros do destino são comuns aos subcomandos que gravam:
veja [Invocação](invocation.md#o-destino).
