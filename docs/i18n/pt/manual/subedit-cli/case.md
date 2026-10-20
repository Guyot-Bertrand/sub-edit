# `case`

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revista por nenhum leitor do idioma.

```
subedit-cli case --to title|sentence|upper|lower [--range N-M|N-]
                 (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 [--document main|translation] [-t FILE] [--align-method position|number]
                 <file>...
```

Põe os textos em **iniciais maiúsculas, maiúscula na frase, maiúsculas ou minúsculas**, sem tocar em
nenhuma etiqueta. É o `Ferramentas ▸ Maiúsculo/Minúsculo` da [janela](../subedit-gui/operations.md), com as suas regras.

| `--to` | Efeito | `bonjour marie` torna-se |
| :----- | :---- | :---------------------- |
| `title` | cada palavra recebe uma maiúscula, o resto passa a minúsculas | `Bonjour Marie` |
| `sentence` | a primeira letra do texto, e só ela; o resto passa a minúsculas | `Bonjour marie` |
| `upper` | tudo em maiúsculas | `BONJOUR MARIE` |
| `lower` | tudo em minúsculas | `bonjour marie` |

- **O que vem antes da primeira letra ou do primeiro algarismo não é tocado**: um hífen
  de diálogo, umas aspas de abertura ou um parêntese sobrevivem (`- bonjour` dá `- Bonjour`).
- **As etiquetas não são texto**: não mudam de maiúsculas/minúsculas — `<I>` continua `<I>` — e não
  mudam de lugar: uma etiqueta que corta uma palavra continua a cortá-la depois.
- **O ICU decide onde começa uma palavra**: `l'été sera chaud` dá `L'été Sera Chaud`, e os acentos
  mantêm-se (`éléonore` dá `Éléonore`).

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

| Opção | Obrigatório | Valores | Predefinição |
| :----- | :----- | :------ | :----- |
| `<file>...` | sim | um ou mais caminhos | — |
| `--recursive`, `-r` | não | um indicador | desativado — ver [Processar uma árvore de pastas](lots.md) |
| `--to` | **sim** | `title`, `sentence`, `upper`, `lower` | — |
| `--range` | não | `N-M`, ou `N-` até ao fim — ver [`adjust`](adjust.md#--range) | o ficheiro inteiro |
| `--document` | não | `main` ou `translation` — ver [Uma tradução](invocation.md#uma-tradução) | `main` |
| `-t`, `--translation-file` | com `--document translation` | um ficheiro de tradução, para **uma única** entrada | — |
| `--align-method` | não, e só com `-t` | `position` ou `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **uma das três**, exceto com `--dry-run` | ver [Invocação](invocation.md#o-destino) | — |
| `--dry-run` | não | um indicador: calcula e diz, não escreve nada — ver [Ver antes de escrever](invocation.md#ver-antes-de-escrever) | desativado |

**O texto principal por predefinição; a tradução com `--document translation -t FILE`.** É então
a tradução que é alterada e escrita, e só ela — ver [Uma tradução](invocation.md#uma-tradução).

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

Um ficheiro em que **nenhum texto muda** — tudo já está nas maiúsculas/minúsculas pedidas — é escrito tal como está e
di-lo:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBONJOUR\n\n' > a.srt; subedit-cli case --to upper --output b.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBONJOUR\n\n' > a.srt; subedit-cli case --to upper --output b.srt a.srt
a.srt: nothing to change -> b.srt
```

## Saída

**Saída padrão** — nada: o resultado é o ficheiro escrito. Com `--dry-run`, a lista das
alterações, descrita em [Ver antes de escrever](invocation.md#ver-antes-de-escrever).

| Nível | O que se acrescenta à saída de erros |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles recased -> <destination>`, ou `nothing to change` |
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
| `--to` em falta | `--to is required` |
| um valor que não é um dos quatro | `--to: … not in {title,sentence,upper,lower}` |
| intervalo mal escrito ou fora do ficheiro | ver [`adjust`](adjust.md#--range) |

Os erros do destino são comuns aos subcomandos que escrevem:
ver [Invocação](invocation.md#o-destino).
