# `italics`

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revista por nenhum leitor do idioma.

```
subedit-cli italics (--on | --off) [--range N-M|N-]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    <file>...
```

Põe os textos **em itálico**, ou **tira-lhes o itálico**, nas etiquetas do formato do ficheiro.
É o `Italic` da [janela](../subedit-gui/operations.md).

**Duas palavras e não um comutador**: o botão da janela pergunta ao núcleo em que sentido
atua; aqui diz-se. `--on` e `--off` excluem-se mutuamente, e é preciso uma delas — omiti-las, ou dar as duas,
é um erro de utilização (código `1`).

- **`--on`** envolve cada texto num único par de etiquetas, **depois de remover as que
  lá estavam**: um texto já em itálico não é envolvido duas vezes, e um itálico cortado em dois
  é refeito num só bloco. Em várias linhas, um único par cobre o conjunto.
- **`--off`** remove as etiquetas de itálico; **o resto de uma etiqueta é mantido** (`{\b1\i1}` dá
  `{\b1}`).
- O texto é escrito **como o formato do ficheiro escreve o itálico**: `<i>…</i>` para SubRip, WebVTT e
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

| Opção | Obrigatória | Valores | Predefinição |
| :----- | :----- | :------ | :----- |
| `<file>...` | sim | um ou mais caminhos | — |
| `--recursive`, `-r` | não | um indicador | desativado — ver [Processar uma árvore de pastas](lots.md) |
| `--on` / `--off` | **uma das duas** | indicadores | — |
| `--range` | não | `N-M`, ou `N-` até ao fim — ver [`adjust`](adjust.md#--range) | o ficheiro inteiro |
| `--document` | não | `main` ou `translation` — ver [Uma tradução](invocation.md#uma-tradução) | `main` |
| `-t`, `--translation-file` | com `--document translation` | um ficheiro de tradução, para **um único** ficheiro de entrada | — |
| `--align-method` | não, e só com `-t` | `position` ou `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **uma das três**, exceto com `--dry-run` | ver [Invocação](invocation.md#o-destino) | — |
| `--dry-run` | não | um indicador: calcula e comunica, não escreve nada — ver [Ver antes de escrever](invocation.md#ver-antes-de-escrever) | desativado |

**O texto principal por predefinição; a tradução com `--document translation -t FILE`.** É então
a tradução que é alterada e escrita, e só ela — ver [Uma tradução](invocation.md#uma-tradução).

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

**TMPlayer e LRC não escrevem estilo**: não há itálico para pôr nem itálico para tirar. A
janela esbate a entrada em vez de a esconder; a linha de comandos **recusa, ficheiro a
ficheiro, indicando o motivo**, e os outros ficheiros do lote prosseguem. O que conta é o que o
formato pode conter, não a maneira de o escrever: SubRip e WebVTT escrevem o itálico da mesma
maneira, e não dizem o mesmo sobre a cor.

<!-- example: printf '00:00:01:Hello there\n00:00:03:Bye\n' > a.txt; subedit-cli italics --on --output-dir sortie a.txt; echo "code=$?" -->
```console
$ printf '00:00:01:Hello there\n00:00:03:Bye\n' > a.txt; subedit-cli italics --on --output-dir sortie a.txt; echo "code=$?"
a.txt: TMPlayer writes no style: there are no italics to put on
code=2
```

Em JSON, a falha traz o identificador **`no-style`**.

## Saída

**Saída padrão** — nada: o resultado é o ficheiro escrito. Com `--dry-run`, a lista das
alterações, descrita em [Ver antes de escrever](invocation.md#ver-antes-de-escrever).

| Nível | O que se acrescenta à saída de erros |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles put in italics -> <destination>`, ou `N subtitles taken out of italics`, ou `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — o formato, a codificação, a marca e os fins de linha do ficheiro lido, repostos como estavam |
| 3 | `<path>: N bytes read, M written`, e depois **cada diagnóstico de leitura** — ver [Invocação](invocation.md#ler-os-diagnósticos) |

`N` é o número de legendas **cujo texto mudou**, lido a partir da própria operação.

### Em JSON

Com [`--format json`](invocation.md#uma-saída-que-um-script-pode-ler), o objeto de cada ficheiro traz
`counts.changed` — o `N` da frase — e `changes`, a lista das legendas alteradas
(`{"subtitle", "document", "before", "after"}`, **vazia** quando nada mudou).

## Códigos de saída

Os da ferramenta: `0` se todos os ficheiros foram processados — um ficheiro onde nada muda conta como tal —,
`2` se nenhum, `3` se apenas alguns, `1` perante um erro de utilização.

## Erros

| O que os desencadeia | Mensagem |
| :------------------ | :------ |
| nem `--on` nem `--off` | `italics needs --on to put the texts in italics, or --off` |
| ambas | `--on and --off say opposite things; give one of them` |
| um formato sem estilo | `<path>: TMPlayer writes no style: there are no italics to put on` (ou `take out`) |
| intervalo mal escrito ou fora do ficheiro | ver [`adjust`](adjust.md#--range) |

Os erros do destino são comuns aos subcomandos que escrevem:
ver [Invocação](invocation.md#o-destino).
