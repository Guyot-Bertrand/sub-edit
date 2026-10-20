# `replace`

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revista por nenhum leitor do idioma.

```
subedit-cli replace [--regex] [--case-sensitive] [--range N-M|N-]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    <pattern> <replacement> <file>...
```

Substitui um texto nas legendas **sem partir uma etiqueta**. É a
operação `Replace All` da [pesquisa](../subedit-gui/find-and-replace.md) da janela, com as suas regras:

- **a pesquisa faz-se no texto visível** — o que o espectador lê. As etiquetas não fazem
  parte dele: procurar `<i>` não encontra nada, e procurar `Bonjour` encontra um “Bonjour” cuja metade
  está em itálico;
- **a substituição faz-se no texto de origem**, o do ficheiro, mantendo as etiquetas onde
  estavam. Uma etiqueta que corta a palavra procurada envolve toda a substituição; um estilo que
  toca na correspondência cobre toda a substituição; o que a correspondência não toca
  não se move. As regras, caso a caso, são as da pesquisa da janela.

**Não existe um subcomando `find`**: para ver onde está um padrão sem alterar nada,
é [`--dry-run`](invocation.md#ver-antes-de-escrever) que lista cada legenda abrangida.

<!-- example: subedit-cli replace --help -->
```console
$ subedit-cli replace --help
Replace a text in the subtitles, without breaking a tag
Usage: subedit-cli replace [OPTIONS] pattern replacement files...

Positionals:
  pattern TEXT REQUIRED       Text to look for in what is shown
  replacement TEXT REQUIRED   What replaces it, tags allowed
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --regex                     Read the text to look for as a regular expression
  --case-sensitive            Tell capitals from small letters
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
| `<pattern>` | sim | o texto a procurar no que é mostrado; **não vazio** | — |
| `<replacement>` | sim | o que o substitui, com etiquetas permitidas; pode ser vazio | — |
| `<file>...` | sim | um ou mais caminhos | — |
| `--recursive`, `-r` | não | um indicador | desativado — ver [Processar uma árvore de pastas](lots.md) |
| `--regex` | não | um indicador: o padrão é uma expressão regular ICU | texto simples |
| `--case-sensitive` | não | um indicador: distingue maiúsculas de minúsculas | são ignoradas |
| `--range` | não | `N-M`, ou `N-` até ao fim — ver [`adjust`](adjust.md#--range) | o ficheiro inteiro |
| `--document` | não | `main` ou `translation` — ver [Uma tradução](invocation.md#uma-tradução) | `main` |
| `-t`, `--translation-file` | com `--document translation` | um ficheiro de tradução, para **uma única** entrada | — |
| `--align-method` | não, e só com `-t` | `position` ou `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **uma das três**, exceto com `--dry-run` | ver [Invocação](invocation.md#o-destino) | — |
| `--dry-run` | não | um indicador: calcula e diz, não escreve nada — ver [Ver antes de escrever](invocation.md#ver-antes-de-escrever) | desativado |

**Um padrão que começa por um hífen** escreve-se depois de `--`: `subedit-cli replace -- -- "—" film.srt`.

**O texto principal por predefinição; a tradução com `--document translation -t FILE`.** É então
a tradução que é alterada e escrita, e só ela — ver [Uma tradução](invocation.md#uma-tradução).

### Texto simples ou expressão

Por predefinição o padrão é **lido tal como está**: `a.b` procura um `a`, um ponto, um `b`. As maiúsculas e as minúsculas
são ignoradas com a mesma finura que na janela — `É` e `é` são uma única letra.

Com `--regex`, o padrão é uma expressão regular **ICU**, lida como o Gaupol a lê: `.` atravessa
uma quebra de linha, `^` e `$` valem em cada linha de uma legenda. Na substituição, e
só nela:

| Escrever | Para dizer |
| :----- | :-------- |
| `$1` a `$9` | os grupos da correspondência |
| `$0` | toda a correspondência |
| `\n` | uma quebra de linha |
| `\$`, `\\` | um `$`, uma barra invertida |

Atenção às aspas da shell: `'$1 EUR'`, e não `"$1 EUR"`, que deixaria a shell ler `$1`.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nPrix: 12 euros, et 7 euros.\n\n' > prix.srt; subedit-cli replace --regex '([0-9]+) euros' '$1 EUR' --output euros.srt prix.srt; cat euros.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nPrix: 12 euros, et 7 euros.\n\n' > prix.srt; subedit-cli replace --regex '([0-9]+) euros' '$1 EUR' --output euros.srt prix.srt; cat euros.srt
prix.srt: replaced 2 matches -> euros.srt
1
00:00:01,000 --> 00:00:02,000
Prix: 12 EUR, et 7 EUR.
```

## Etiquetas

Um texto em que uma etiqueta corta a palavra procurada:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace Bonjour Salut --output salut.srt balises.srt; cat salut.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace Bonjour Salut --output salut.srt balises.srt; cat salut.srt
balises.srt: replaced 2 matches -> salut.srt
1
00:00:01,000 --> 00:00:02,000
<i>Salut</i> tout le monde

2
00:00:03,000 --> 00:00:04,000
Salut <i>Marie</i>, ca va
```

Na primeira legenda, a etiqueta cortava “Bonjour”: passa a envolver “Salut”.
Na segunda, não é tocada e fica onde está. **Uma etiqueta deixada aberta
fica aberta**: reparar marcação deficiente seria outra operação.

A substituição é lida **nas etiquetas do formato do ficheiro**: `<i>…</i>` para SubRip,
`{\i1}…` para Advanced SSA. É a única maneira de formatar o que se escreve.

## Nada encontrado, nada a alterar

Duas respostas que não são falhas — o código de saída é `0` e **o ficheiro é escrito
na mesma**, pois um destino indicado é um destino escrito:

| Situação | Frase |
| :-------- | :----- |
| o padrão não está em lado nenhum | `"zzz" not found` |
| está lá, mas cada correspondência é substituída por si própria | `nothing to change` |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli replace zzz x --output b.srt a.srt; subedit-cli replace Bonjour Bonjour --output c.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli replace zzz x --output b.srt a.srt; subedit-cli replace Bonjour Bonjour --output c.srt a.srt
a.srt: "zzz" not found -> b.srt
a.srt: nothing to change -> c.srt
```

## `--dry-run`

A saída padrão lista **cada legenda cujo texto mudaria**, antes e depois, tal como
estão no ficheiro — etiquetas incluídas:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace --dry-run Bonjour Salut balises.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace --dry-run Bonjour Salut balises.srt
balises.srt: replaced 2 matches (dry run, nothing written)
balises.srt: subtitle 1
- <i>Bon</i>jour tout le monde
+ <i>Salut</i> tout le monde
balises.srt: subtitle 2
- Bonjour <i>Marie</i>, ca va
+ Salut <i>Marie</i>, ca va
```

A forma da lista está descrita em [Ver antes de escrever](invocation.md#ver-antes-de-escrever);
em JSON, é `changes`.

## Saída

**Saída padrão** — nada: o resultado é o ficheiro escrito. Com `--dry-run`, a lista das
alterações acima.

| Nível | O que se acrescenta à saída de erros |
| :----- | :------------------------------------- |
| 1 | `<path>: replaced N matches -> <destination>`, ou `"<pattern>" not found`, ou `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — o formato, a codificação, a marca e os fins de linha do ficheiro lido, repostos como estavam |
| 3 | `<path>: N bytes read, M written`, depois **todos os diagnósticos de leitura** — ver [Invocação](invocation.md#ler-os-diagnósticos) |

`N` conta **as correspondências substituídas nas legendas cujo texto mudou** — é o
número que a janela também diz.

### Em JSON

Com [`--format json`](invocation.md#uma-saída-que-um-script-pode-ler), o objeto de cada ficheiro traz:

| Chave | Conteúdo |
| :-- | :------ |
| `counts.replaced` | as correspondências substituídas, nas legendas cujo texto mudou: o `N` da frase |
| `counts.matched` | as correspondências encontradas, **quer a sua substituição tenha alterado o texto quer não** — é isso que distingue “nada encontrado” (`0`) de “nada a alterar” (`replaced` a `0`, `matched` positivo) |
| `changes` | as legendas alteradas: `{"subtitle", "document", "before", "after"}`, **vazio** quando nada mudou |

## Códigos de saída

Os da ferramenta: `0` se todos os ficheiros foram processados — um ficheiro em que nada foi encontrado
conta —, `2` se nenhum, `3` se apenas alguns, `1` num erro de utilização.

## Erros

| O que o desencadeia | Mensagem |
| :------------------ | :------ |
| padrão vazio | `pattern: nothing to look for` |
| expressão ilegível | `pattern: not a regular expression (<ICU's reason>)` |
| intervalo mal escrito ou fora do ficheiro | ver [`adjust`](adjust.md#--range) |

**O padrão é lido antes do primeiro ficheiro**: uma expressão ilegível é um erro de utilização
(código `1`), nada é lido nem escrito.

Os erros do destino são comuns aos subcomandos que escrevem:
ver [Invocação](invocation.md#o-destino).
