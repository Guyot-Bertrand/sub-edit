# `dialogue-dashes`

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revista por nenhum leitor do idioma.

```
subedit-cli dialogue-dashes (--add | --remove) [--range N-M|N-]
                            (--output FILE | --output-dir DIR | --in-place)
                            [--dry-run]
                            [--recursive]
                            [--document main|translation] [-t FILE] [--align-method position|number]
                            <file>...
```

Põe um **hífen de diálogo** no início das linhas, ou retira-o. É o `Dialogue Dashes` da
[janela](../subedit-gui/operations.md).

**Duas palavras, como em [`italics`](italics.md)**: `--add` e `--remove` excluem-se mutuamente, e é preciso uma delas.

- **`--add`** põe um hífen à frente de cada linha que não o tenha: `Bonjour` dá `- Bonjour`, e
  `Bonjour\nMarie` dá `- Bonjour\n- Marie`. **Um hífen já presente não é duplicado.**
- **Reconhecem-se três travessões** — o hífen do teclado e os dois travessões compridos do tipógrafo —
  **e é o hífen que se escreve**: `— Bonjour` passa a `- Bonjour`. Os espaços a mais
  depois do travessão desaparecem.
- O hífen vai **à frente do texto, não à frente de uma etiqueta de abertura**: `<i>Bonjour</i>` dá
  `<i>- Bonjour</i>`. Nenhuma etiqueta se move.
- **`--remove`** retira os hífens iniciais, nas três formas.

<!-- example: subedit-cli dialogue-dashes --help -->
```console
$ subedit-cli dialogue-dashes --help
Put dialogue dashes on the lines, or take them off
Usage: subedit-cli dialogue-dashes [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --add                       Put a dialogue dash at the head of the lines
  --remove                    Take the dialogue dashes off
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

| Opção | Obrigatório | Valores | Predefinição |
| :----- | :----- | :------ | :----- |
| `<file>...` | sim | um ou mais caminhos | — |
| `--recursive`, `-r` | não | um indicador | desativado — ver [Processar uma árvore de pastas](lots.md) |
| `--add` / `--remove` | **uma das duas** | indicadores | — |
| `--range` | não | `N-M`, ou `N-` até ao fim — ver [`adjust`](adjust.md#--range) | o ficheiro inteiro |
| `--document` | não | `main` ou `translation` — ver [Uma tradução](invocation.md#uma-tradução) | `main` |
| `-t`, `--translation-file` | com `--document translation` | um ficheiro de tradução, para **uma única** entrada | — |
| `--align-method` | não, e só com `-t` | `position` ou `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **uma das três**, exceto com `--dry-run` | ver [Invocação](invocation.md#o-destino) | — |
| `--dry-run` | não | um indicador: calcula e diz, não escreve nada — ver [Ver antes de escrever](invocation.md#ver-antes-de-escrever) | desativado |

**O texto principal por predefinição; a tradução com `--document translation -t FILE`.** É então
a tradução que é alterada e escrita, e só ela — ver [Uma tradução](invocation.md#uma-tradução).

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\nMarie\n\n2\n00:00:03,000 --> 00:00:04,000\n<i>Salut</i>\n\n' > a.srt; subedit-cli dialogue-dashes --add --output t.srt a.srt; cat t.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\nMarie\n\n2\n00:00:03,000 --> 00:00:04,000\n<i>Salut</i>\n\n' > a.srt; subedit-cli dialogue-dashes --add --output t.srt a.srt; cat t.srt
a.srt: 2 subtitles dashed -> t.srt
1
00:00:01,000 --> 00:00:02,000
- Bonjour
- Marie

2
00:00:03,000 --> 00:00:04,000
<i>- Salut</i>
```

## Saída

**Saída padrão** — nada: o resultado é o ficheiro escrito. Com `--dry-run`, a lista das
alterações, descrita em [Ver antes de escrever](invocation.md#ver-antes-de-escrever).

| Nível | O que se acrescenta à saída de erros |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles dashed -> <destination>`, ou `N subtitles undashed`, ou `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — o formato, a codificação, a marca e os fins de linha do ficheiro lido, repostos como estavam |
| 3 | `<path>: N bytes read, M written`, depois **cada diagnóstico de leitura** — ver [Invocação](invocation.md#ler-os-diagnósticos) |

`N` é o número de legendas **cujo texto mudou**, lido da própria operação.

### Em JSON

Com [`--format json`](invocation.md#uma-saída-que-um-script-pode-ler), o objeto de cada ficheiro traz
`counts.changed` — o `N` da frase — e `changes`, a lista das legendas alteradas
(`{"subtitle", "document", "before", "after"}`, **vazia** quando nada mudou).

## Códigos de saída

Os da ferramenta: `0` se todos os ficheiros foram processados — um ficheiro em que nada muda conta como processado —,
`2` se nenhum, `3` se apenas alguns, `1` em caso de erro de utilização.

## Erros

| O que os desencadeia | Mensagem |
| :------------------ | :------ |
| nem `--add` nem `--remove` | `dialogue-dashes needs --add to put dashes on, or --remove` |
| ambos | `--add and --remove say opposite things; give one of them` |
| intervalo mal escrito ou fora do ficheiro | ver [`adjust`](adjust.md#--range) |

Os erros do destino são comuns aos subcomandos que escrevem:
ver [Invocação](invocation.md#o-destino).
