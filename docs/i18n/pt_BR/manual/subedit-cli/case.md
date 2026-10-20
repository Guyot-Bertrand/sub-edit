# `case`

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revisada por um leitor do idioma.

```
subedit-cli case --to title|sentence|upper|lower [--range N-M|N-]
                 (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 [--document main|translation] [-t FILE] [--align-method position|number]
                 <file>...
```

Coloca os textos em **iniciais maiúsculas, primeira letra maiúscula, maiúsculas ou minúsculas**, sem tocar em
nenhuma marcação. É o `Ferramentas ▸ Maiúsculas/minúsculas` da [janela](../subedit-gui/operations.md), com suas regras.

| `--to` | Efeito | `bonjour marie` se torna |
| :----- | :---- | :---------------------- |
| `title` | cada palavra recebe uma maiúscula, o resto fica em minúsculas | `Bonjour Marie` |
| `sentence` | a primeira letra do texto, e somente ela; o resto fica em minúsculas | `Bonjour marie` |
| `upper` | tudo em maiúsculas | `BONJOUR MARIE` |
| `lower` | tudo em minúsculas | `bonjour marie` |

- **O que vem antes da primeira letra ou do primeiro dígito não é tocado**: um travessão
  de diálogo, aspas de abertura ou um parêntese são preservados (`- bonjour` resulta em `- Bonjour`).
- **As marcações não são texto**: elas não mudam de caixa — `<I>` continua `<I>` — e não
  mudam de lugar: uma marcação que corta uma palavra continua cortando-a depois.
- **O ICU decide onde uma palavra começa**: `l'été sera chaud` resulta em `L'été Sera Chaud`, e os acentos
  são mantidos (`éléonore` resulta em `Éléonore`).

<!-- example: subedit-cli case --help -->
```console
$ subedit-cli case --help
Put the texts in title, sentence, upper or lower case, tags intact
Usage: subedit-cli case [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --to title|sentence|upper|lower
                              The case to put the texts in
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
| `--to` | **sim** | `title`, `sentence`, `upper`, `lower` | — |
| `--range` | não | `N-M`, ou `N-` até o fim — veja [`adjust`](adjust.md#--range) | o arquivo inteiro |
| `--document` | não | `main` ou `translation` — veja [Uma tradução](invocation.md#uma-tradução) | `main` |
| `-t`, `--translation-file` | com `--document translation` | um arquivo de tradução, para **uma única** entrada | — |
| `--align-method` | não, e somente com `-t` | `position` ou `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **uma das três**, exceto com `--dry-run` | veja [Invocação](invocation.md#o-destino) | — |
| `--dry-run` | não | uma opção sem valor: calcula e informa, não grava nada — veja [Ver antes de escrever](invocation.md#ver-antes-de-escrever) | desativada |

**O texto principal por padrão; a tradução com `--document translation -t FILE`.** Passa a ser
então a tradução que é alterada e gravada, e somente ela — veja [Uma tradução](invocation.md#uma-tradução).

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\n- BONJOUR MARIE\n\n' > a.srt; subedit-cli case --to title --output titre.srt a.srt; cat titre.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\n- BONJOUR MARIE\n\n' > a.srt; subedit-cli case --to title --output titre.srt a.srt; cat titre.srt
a.srt: 2 subtitles recased -> titre.srt
1
00:00:01,000 --> 00:00:02,000
<i>Bon</i>jour Marie

2
00:00:03,000 --> 00:00:04,000
- Bonjour Marie
```

Um arquivo em que **nenhum texto muda** — tudo já está na caixa pedida — é gravado como está e
informa isso:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBONJOUR\n\n' > a.srt; subedit-cli case --to upper --output b.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBONJOUR\n\n' > a.srt; subedit-cli case --to upper --output b.srt a.srt
a.srt: nothing to change -> b.srt
```

## Saída

**Saída padrão** — nada: o resultado é o arquivo gravado. Com `--dry-run`, a lista das
alterações, descrita em [Ver antes de escrever](invocation.md#ver-antes-de-escrever).

| Nível | O que se acrescenta no erro padrão |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles recased -> <destination>`, ou `nothing to change` |
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
| `--to` ausente | `--to is required` |
| um valor que não é um dos quatro | `--to: … not in {title,sentence,upper,lower}` |
| intervalo mal escrito ou fora do arquivo | veja [`adjust`](adjust.md#--range) |

Os erros do destino são comuns aos subcomandos que gravam:
veja [Invocação](invocation.md#o-destino).
