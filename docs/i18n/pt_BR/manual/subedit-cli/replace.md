# `replace`

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revisada por um leitor do idioma.

```
subedit-cli replace [--regex] [--case-sensitive] [--range N-M|N-]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    <pattern> <replacement> <file>...
```

Substitui um texto nas legendas **sem quebrar uma marcação**. É a operação
`Replace All` da [pesquisa](../subedit-gui/find-and-replace.md) da janela, com suas regras:

- **a pesquisa se faz no texto visível** — o que o espectador lê. As marcações não fazem parte
  dele: procurar `<i>` não encontra nada, e procurar `Bonjour` encontra um "Bonjour" do qual metade
  está em itálico;
- **a substituição se faz no texto-fonte**, o do arquivo, mantendo as marcações onde
  estavam. Uma marcação que corta a palavra procurada envolve toda a substituição; um estilo que
  toca a correspondência cobre toda a substituição; o que a correspondência não toca
  não se move. As regras, caso a caso, são as da pesquisa da janela.

**Não existe um subcomando `find`**: para ver onde está um padrão sem alterar nada,
usa-se [`--dry-run`](invocation.md#ver-antes-de-escrever), que lista cada legenda envolvida.

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

| Opção | Obrigatório | Valores | Padrão |
| :----- | :----- | :------ | :----- |
| `<pattern>` | sim | o texto a procurar no que é exibido; **não vazio** | — |
| `<replacement>` | sim | o que o substitui, marcações permitidas; pode ser vazio | — |
| `<file>...` | sim | um ou mais caminhos | — |
| `--recursive`, `-r` | não | uma opção sem valor | desativado — veja [Processar uma árvore de diretórios](lots.md) |
| `--regex` | não | uma opção sem valor: o padrão é uma expressão regular ICU | texto simples |
| `--case-sensitive` | não | uma opção sem valor: distingue maiúsculas de minúsculas | elas são ignoradas |
| `--range` | não | `N-M`, ou `N-` até o fim — veja [`adjust`](adjust.md#--range) | o arquivo inteiro |
| `--document` | não | `main` ou `translation` — veja [Uma tradução](invocation.md#uma-tradução) | `main` |
| `-t`, `--translation-file` | com `--document translation` | um arquivo de tradução, para **uma única** entrada | — |
| `--align-method` | não, e somente com `-t` | `position` ou `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **um dos três**, exceto com `--dry-run` | veja [Invocação](invocation.md#o-destino) | — |
| `--dry-run` | não | uma opção sem valor: calcula e informa, não grava nada — veja [Ver antes de escrever](invocation.md#ver-antes-de-escrever) | desativado |

**Um padrão que começa com um hífen** é escrito depois de `--`: `subedit-cli replace -- -- "—" film.srt`.

**O texto principal por padrão; a tradução com `--document translation -t FILE`.** É então
a tradução que é alterada e gravada, e somente ela — veja [Uma tradução](invocation.md#uma-tradução).

### Texto simples ou expressão

Por padrão, a pesquisa é **literal**, o padrão sendo lido tal como está: `a.b` procura um `a`, um ponto, um `b`. As
maiúsculas e minúsculas são ignoradas com a mesma sutileza da janela — `É` e `é` são uma única letra.

Com `--regex`, o padrão é uma expressão regular **ICU**, lida como o Gaupol a lê: `.` atravessa
uma quebra de linha, `^` e `$` valem em cada linha de uma legenda. Na substituição, e
somente nela:

| Escrever | Para dizer |
| :----- | :-------- |
| `$1` a `$9` | os grupos da correspondência |
| `$0` | a correspondência inteira |
| `\n` | uma quebra de linha |
| `\$`, `\\` | um `$`, uma barra invertida |

Atenção às aspas do shell: `'$1 EUR'`, e não `"$1 EUR"`, que deixaria o shell ler `$1`.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nPrix: 12 euros, et 7 euros.\n\n' > prix.srt; subedit-cli replace --regex '([0-9]+) euros' '$1 EUR' --output euros.srt prix.srt; cat euros.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nPrix: 12 euros, et 7 euros.\n\n' > prix.srt; subedit-cli replace --regex '([0-9]+) euros' '$1 EUR' --output euros.srt prix.srt; cat euros.srt
prix.srt: replaced 2 matches -> euros.srt
1
00:00:01,000 --> 00:00:02,000
Prix: 12 EUR, et 7 EUR.
```

## Marcações

Um texto em que uma marcação corta a palavra procurada:

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

Na primeira legenda, a marcação cortava "Bonjour": agora ela envolve "Salut".
Na segunda, ela não é tocada e fica onde está. **Uma marcação deixada aberta
continua aberta**: consertar um código instável seria outra operação.

A substituição é lida nas **marcações do formato do arquivo**: `<i>…</i>` para SubRip,
`{\i1}…` para Advanced SSA. É a única maneira de formatar o que se escreve.

## Nada encontrado, nada a alterar

Duas respostas que não são falhas — o código de saída é `0` e **o arquivo é gravado
mesmo assim**, pois um destino informado é um destino gravado:

| Situação | Frase |
| :-------- | :----- |
| o padrão não está em lugar nenhum | `"zzz" not found` |
| ele está lá, mas cada correspondência é substituída por ela mesma | `nothing to change` |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli replace zzz x --output b.srt a.srt; subedit-cli replace Bonjour Bonjour --output c.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli replace zzz x --output b.srt a.srt; subedit-cli replace Bonjour Bonjour --output c.srt a.srt
a.srt: "zzz" not found -> b.srt
a.srt: nothing to change -> c.srt
```

## `--dry-run`

A saída padrão lista **cada legenda cujo texto mudaria**, antes e depois, tal como
estão no arquivo — marcações incluídas:

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

A forma da lista é descrita em [Ver antes de escrever](invocation.md#ver-antes-de-escrever);
em JSON, é `changes`.

## Saída

**Saída padrão** — nada: o resultado é o arquivo gravado. Com `--dry-run`, a lista das
alterações acima.

| Nível | O que se acrescenta na saída de erro |
| :----- | :------------------------------------- |
| 1 | `<path>: replaced N matches -> <destination>`, ou `"<pattern>" not found`, ou `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — o formato, a codificação, a marca e os finais de linha do arquivo lido, restituídos como estavam |
| 3 | `<path>: N bytes read, M written`, depois **todo diagnóstico de leitura** — veja [Invocação](invocation.md#diagnósticos-de-leitura) |

`N` conta **as correspondências substituídas nas legendas cujo texto mudou** — é o
número que a janela também informa.

### Em JSON

Com [`--format json`](invocation.md#uma-saída-que-um-script-pode-ler), o objeto de cada arquivo traz:

| Chave | Conteúdo |
| :-- | :------ |
| `counts.replaced` | as correspondências substituídas, nas legendas cujo texto mudou: o `N` da frase |
| `counts.matched` | as correspondências encontradas, **quer a substituição tenha alterado o texto ou não** — é isso que distingue "nada encontrado" (`0`) de "nada a alterar" (`replaced` em `0`, `matched` positivo) |
| `changes` | as legendas alteradas: `{"subtitle", "document", "before", "after"}`, **vazio** quando nada mudou |

## Códigos de saída

Os da ferramenta: `0` se todos os arquivos foram processados — um arquivo em que nada foi encontrado
conta como processado —, `2` se nenhum, `3` se apenas alguns, `1` em caso de erro de uso.

## Erros

| O que o provoca | Mensagem |
| :------------------ | :------ |
| padrão vazio | `pattern: nothing to look for` |
| expressão ilegível | `pattern: not a regular expression (<ICU's reason>)` |
| intervalo mal escrito ou fora do arquivo | veja [`adjust`](adjust.md#--range) |

**O padrão é lido antes do primeiro arquivo**: uma expressão ilegível é um erro de uso
(código `1`), nada é lido nem gravado.

Os erros do destino são comuns aos subcomandos que gravam:
veja [Invocação](invocation.md#o-destino).
