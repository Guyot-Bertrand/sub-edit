# `hearing-impaired`

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revisada por um leitor do idioma.

```
subedit-cli hearing-impaired
                  (--output FILE | --output-dir DIR | --in-place)
                  [--dry-run]
                  [--recursive] [--parentheses]
                  [--document main|translation] [-t FILE] [--align-method position|number]
                  <file>...
```

Remove as **menções para deficientes auditivos** — os sons descritos entre colchetes — de todo o
arquivo. **Os parênteses permanecem por padrão**: são usados tanto para uma
fala sussurrada quanto para um som descrito, e removê-los apagaria diálogo. `--parentheses` os remove
também, para um arquivo que descreve seus sons com eles.

O que o comando não faz: letras de canções entre sinais de número, o nome do locutor
antes de dois-pontos, restabelecer as maiúsculas, corrigir erros de OCR. Esses
padrões são os do assistente
[`Correct Texts…`](../subedit-gui/correct-texts.md) da janela, que a linha de
comando oferece com o nome [`correct`](correct.md); e aqui nada pode ser configurado: a
transformação é decidida, não configurável.

<!-- example: subedit-cli hearing-impaired --help -->
```console
$ subedit-cli hearing-impaired --help
Remove the sounds described between square brackets
Usage: subedit-cli hearing-impaired [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to clean

Options:
  -h,--help                   Print this help message and exit
  --parentheses               Also remove the sounds between parentheses, often whispered lines
  -r,--recursive              Take directories as inputs, and every subtitle file in them
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
| `--parentheses` | não | uma opção sem valor: remove também as menções entre parênteses | desativada |
| `--recursive`, `-r` | não | uma opção sem valor | desativada — veja [Processar uma árvore de diretórios](lots.md) |
| `--document` | não | `main` ou `translation` — veja [Uma tradução](invocation.md#uma-tradução) | `main` |
| `-t`, `--translation-file` | com `--document translation` | um arquivo de tradução, para **uma única** entrada | — |
| `--align-method` | não, e somente com `-t` | `position` ou `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **uma das três**, exceto com `--dry-run` | veja [Invocação](invocation.md#o-destino) | — |
| `--dry-run` | não | uma opção sem valor: calcula e informa, não grava nada — veja [Ver antes de escrever](invocation.md#ver-antes-de-escrever) | desativada |

O formato do arquivo lido é **mantido**: mudar o formato é trabalho de
[`convert`](convert.md).

**O texto principal por padrão; a tradução com `--document translation -t FILE`.** É então
a tradução que é alterada e gravada, e somente ela — veja [Uma tradução](invocation.md#uma-tradução).

**Uma menção que esvaziaria uma tradução a grava vazia e mantém a legenda**: a legenda
continua no arquivo, sem texto. O texto principal, por sua vez, perde uma legenda inteiramente esvaziada, como
abaixo.

## O que é removido e o que permanece

Uma menção é um colchete — ou um parêntese, com `--parentheses` — e o que ele encerra, **quebra de
linha incluída**: em arquivos reais, uma menção costuma ser cortada pelo
fim de uma linha.

| Entrada | Saída |
| :----- | :----- |
| `Bonjour [il tousse] Marie` | `Bonjour Marie` |
| `Bonjour[il tousse]Marie` | `Bonjour Marie` |
| `[Bruit de pas] Bonjour` | `Bonjour` |
| `A [un] [deux] B` | `A B` |

A remoção deixa **exatamente um espaço entre o que a cercava, e nada na
borda de uma linha**. Não é uma limpeza dos espaços do arquivo: dois espaços
que o texto já continha continuam sendo dois espaços.

**Quando a menção atravessa a quebra de linha, é a quebra que permanece** — as
duas linhas continuam sendo duas linhas. Este caso não é uma curiosidade: em quinze
legendagens reais, todo colchete que parecia órfão era uma
menção cortada dessa maneira.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nReculez ! [Il\nhurle] Tout de suite !\n\n' > a.srt; subedit-cli --quiet hearing-impaired --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nReculez ! [Il\nhurle] Tout de suite !\n\n' > a.srt; subedit-cli --quiet hearing-impaired --output b.srt a.srt; cat b.srt
1
00:00:01,000 --> 00:00:03,000
Reculez !
Tout de suite !
```

**O que não é uma menção e não se mexe:**

| Caso | O que acontece |
| :-- | :-------------- |
| conteúdo puramente numérico — `[1]`, `(12)`, `[ 156478 ]` | mantido como está, delimitadores incluídos: uma referência nunca nomeou um som |
| um parêntese e o que ele encerra, sem `--parentheses` | mantido como está: `Venez (chut) ici` não muda |
| um colchete ou parêntese que nada fecha | deixado como está |
| uma marcação de formato — `<i>`, `</i>` | nunca removida |

Um espaço em branco **entre** os dígitos, uma letra ou um sinal — `[1 2]`, `[1a]`,
`[-1]` — faz dela uma menção. O mesmo vale para um conteúdo vazio, `[]` ou `( )`: nenhum
dígito, portanto não é uma referência; nada a dizer, portanto nada a manter.

## O que desaparece por inteiro

**Uma linha que a remoção esvaziou desaparece**, e **uma legenda inteiramente
esvaziada é removida do arquivo**. As legendas seguintes são renumeradas.

Uma linha conta como vazia quando tudo o que resta nela são espaços em branco, marcações de
formato ou um único travessão de diálogo — `<i>[Musique]</i>` não deixa um
`<i></i>` na tela.

**As marcações são as do formato do arquivo.** `{\i1}[Musique]{\i0}` em um
Advanced SSA, `{Y:i}[Musique]` em um MicroDVD e `/[Musique]` em um MPL2 também não
deixam nada. Em contrapartida, um `<i>` em um formato que não escreve nenhuma
marcação — TMPlayer, LRC — é texto que o espectador lê, e a linha que
o contém não está vazia.

**Um diálogo reduzido a uma única voz deixa de ser um diálogo**, e o travessão que sobra
é removido. Enquanto restarem duas vozes, os travessões permanecem.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\nVoir [1] la note.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\nVoir [1] la note.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle cleaned, 1 removed -> b.srt
1
00:00:04,000 --> 00:00:06,000
Attends Marie.

2
00:00:07,000 --> 00:00:09,000
Voir [1] la note.
```

## Um arquivo sem nenhuma menção é gravado mesmo assim

Ele é copiado de forma idêntica, o código de saída é `0`, e o relatório diz que não havia
nada a remover.

É a regra dos subcomandos que gravam: **um destino indicado é
um destino gravado**. Abrir aqui uma exceção obrigaria um script a saber
quais subcomandos às vezes produzem um arquivo e às vezes nada.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nRien à signaler.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nRien à signaler.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt
a.srt: no mention to remove -> b.srt
```

## Saída

**Saída padrão** — nada: o resultado é o arquivo gravado. **Com `--dry-run`, são as alterações propostas**: para cada legenda cujo texto mudaria, seu número, o texto antes e o texto depois — veja [Ver antes de escrever](invocation.md#ver-antes-de-escrever).

| Nível | O que se acrescenta na saída de erro |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles cleaned, M removed -> <destination>`, ou `<path>: no mention to remove -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — o formato, a codificação, a marca e os finais de linha do arquivo lido, restituídos como estavam |
| 3 | `<path>: N bytes read, M written`, depois **cada diagnóstico de leitura** — veja [Invocação](invocation.md#diagnósticos-de-leitura) |

`N` conta as legendas cujo texto mudou, `M` aquelas que a remoção
esvaziou por inteiro e que, por isso, saíram do arquivo.

### Em JSON

Com [`--format json`](invocation.md#uma-saída-que-um-script-pode-ler), o objeto de cada arquivo traz `counts.cleaned` (os textos reescritos) e `counts.removed` (as legendas removidas); **com `--dry-run`**, a chave `changes` lista também as legendas alteradas, como a saída padrão da forma em texto. A tabela completa é a de [Invocação](invocation.md#o-que-cada-subcomando-coloca-em-counts).

## Códigos de saída

Os da ferramenta: `0` se todos os arquivos foram gravados, `2` se nenhum, `3` se
apenas alguns, `1` em caso de erro de uso.

## Erros

Este subcomando não tem erro próprio: não recebe nenhum
valor a interpretar, e nenhum texto pode fazê-lo recusar um arquivo. O que
pode falhar é a leitura ou a gravação, como nos demais.

**Nada é gravado sem uma das três opções de destino**, e seus erros
são comuns aos subcomandos que gravam:
veja [Invocação](invocation.md#o-destino).
