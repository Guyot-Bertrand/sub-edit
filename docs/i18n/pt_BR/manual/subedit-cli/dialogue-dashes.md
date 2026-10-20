# `dialogue-dashes`

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revisada por um leitor do idioma.

```
subedit-cli dialogue-dashes (--add | --remove) [--range N-M|N-]
                            (--output FILE | --output-dir DIR | --in-place)
                            [--dry-run]
                            [--recursive]
                            [--document main|translation] [-t FILE] [--align-method position|number]
                            <file>...
```

Coloca um **travessão de diálogo** no início das linhas, ou os retira. É o `Dialogue Dashes` da
[janela](../subedit-gui/operations.md).

**Duas palavras, como em [`italics`](italics.md)**: `--add` e `--remove` se excluem mutuamente, e uma delas é necessária.

- **`--add`** coloca um travessão na frente de cada linha que não tem um: `Bonjour` resulta em `- Bonjour`, e
  `Bonjour\nMarie` resulta em `- Bonjour\n- Marie`. **Um travessão já presente não é duplicado.**
- **Três travessões são reconhecidos** — o hífen do teclado e os dois travessões longos do tipógrafo —
  **e o que é escrito é o hífen**: `— Bonjour` se torna `- Bonjour`. Os espaços extras
  depois do travessão desaparecem.
- O travessão vai **na frente do texto, não na frente de uma marcação de abertura**: `<i>Bonjour</i>` resulta em
  `<i>- Bonjour</i>`. Nenhuma marcação se move.
- **`--remove`** retira os travessões iniciais, nas três formas.

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

| Opção | Obrigatória | Valores | Padrão |
| :----- | :----- | :------ | :----- |
| `<file>...` | sim | um ou mais caminhos | — |
| `--recursive`, `-r` | não | uma opção sem valor | desativada — veja [Processar uma árvore de diretórios](lots.md) |
| `--add` / `--remove` | **uma das duas** | opções sem valor | — |
| `--range` | não | `N-M`, ou `N-` até o fim — veja [`adjust`](adjust.md#--range) | o arquivo inteiro |
| `--document` | não | `main` ou `translation` — veja [Uma tradução](invocation.md#uma-tradução) | `main` |
| `-t`, `--translation-file` | com `--document translation` | um arquivo de tradução, para **uma única** entrada | — |
| `--align-method` | não, e somente com `-t` | `position` ou `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **uma das três**, exceto com `--dry-run` | veja [Invocação](invocation.md#o-destino) | — |
| `--dry-run` | não | uma opção sem valor: calcula e informa, não grava nada — veja [Ver antes de escrever](invocation.md#ver-antes-de-escrever) | desativada |

**O texto principal por padrão; a tradução com `--document translation -t FILE`.** Passa a ser
então a tradução que é alterada e gravada, e somente ela — veja [Uma tradução](invocation.md#uma-tradução).

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

**Saída padrão** — nada: o resultado é o arquivo gravado. Com `--dry-run`, a lista das
alterações, descrita em [Ver antes de escrever](invocation.md#ver-antes-de-escrever).

| Nível | O que se acrescenta no erro padrão |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles dashed -> <destination>`, ou `N subtitles undashed`, ou `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — o formato, a codificação, a marca e os finais de linha do arquivo lido, recolocados como estavam |
| 3 | `<path>: N bytes read, M written`, depois **todos os diagnósticos de leitura** — veja [Invocação](invocation.md#diagnósticos-de-leitura) |

`N` é o número de legendas **cujo texto mudou**, lido da própria operação.

### Em JSON

Com [`--format json`](invocation.md#uma-saída-que-um-script-pode-ler), o objeto de cada arquivo traz
`counts.changed` — o `N` da frase — e `changes`, a lista das legendas alteradas
(`{"subtitle", "document", "before", "after"}`, **vazia** quando nada mudou).

## Códigos de saída

Os da ferramenta: `0` se todos os arquivos foram processados — um arquivo em que nada muda é processado —,
`2` se nenhum, `3` se apenas alguns, `1` em caso de erro de uso.

## Erros

| O que o provoca | Mensagem |
| :------------------ | :------ |
| nem `--add` nem `--remove` | `dialogue-dashes needs --add to put dashes on, or --remove` |
| ambos | `--add and --remove say opposite things; give one of them` |
| intervalo mal escrito ou fora do arquivo | veja [`adjust`](adjust.md#--range) |

Os erros do destino são comuns aos subcomandos que gravam:
veja [Invocação](invocation.md#o-destino).
