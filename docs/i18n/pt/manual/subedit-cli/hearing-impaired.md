# `hearing-impaired`

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revista por nenhum leitor do idioma.

```
subedit-cli hearing-impaired
                  (--output FILE | --output-dir DIR | --in-place)
                  [--dry-run]
                  [--recursive] [--parentheses]
                  [--document main|translation] [-t FILE] [--align-method position|number]
                  <file>...
```

Remove as **menções para deficientes auditivos** — os sons descritos entre parênteses retos — de todo o
ficheiro. **Os parênteses curvos ficam por predefinição**: usam-se tantas vezes para uma
fala sussurrada como para um som descrito, e removê-los apagaria diálogo. `--parentheses` remove-os
também, para um ficheiro que descreve os seus sons com eles.

O que não faz: letras de canções entre cardinais, o nome do interlocutor
antes de dois pontos, voltar a pôr maiúsculas, corrigir erros de OCR. Esses
padrões são os do assistente
[`Correct Texts…`](../subedit-gui/correct-texts.md) da janela, que a linha
de comandos oferece com o nome [`correct`](correct.md); e aqui nada se pode definir: a
transformação está decidida, não é configurável.

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

| Opção | Obrigatória | Valores | Predefinição |
| :----- | :----- | :------ | :----- |
| `<file>...` | sim | um ou mais caminhos | — |
| `--parentheses` | não | um indicador: remove também as menções entre parênteses curvos | desativado |
| `--recursive`, `-r` | não | um indicador | desativado — ver [Processar uma árvore de pastas](lots.md) |
| `--document` | não | `main` ou `translation` — ver [Uma tradução](invocation.md#uma-tradução) | `main` |
| `-t`, `--translation-file` | com `--document translation` | um ficheiro de tradução, para **um único** ficheiro de entrada | — |
| `--align-method` | não, e só com `-t` | `position` ou `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **uma das três**, exceto com `--dry-run` | ver [Invocação](invocation.md#o-destino) | — |
| `--dry-run` | não | um indicador: calcula e comunica, não escreve nada — ver [Ver antes de escrever](invocation.md#ver-antes-de-escrever) | desativado |

O formato do ficheiro lido é **mantido**: mudar de formato é trabalho de
[`convert`](convert.md).

**O texto principal por predefinição; a tradução com `--document translation -t FILE`.** É então
a tradução que é alterada e escrita, e só ela — ver [Uma tradução](invocation.md#uma-tradução).

**Uma menção que esvaziaria uma tradução escreve-a vazia, e mantém a legenda**: a legenda
fica no ficheiro, sem texto. O texto principal, esse, perde uma legenda totalmente esvaziada, como
abaixo.

## O que é removido e o que fica

Uma menção é um parêntese reto — ou um parêntese curvo, com `--parentheses` — e o que ele encerra, **quebra
de linha incluída**: nos ficheiros reais, uma menção é muitas vezes cortada pelo
fim de uma linha.

| Entrada | Saída |
| :----- | :----- |
| `Bonjour [il tousse] Marie` | `Bonjour Marie` |
| `Bonjour[il tousse]Marie` | `Bonjour Marie` |
| `[Bruit de pas] Bonjour` | `Bonjour` |
| `A [un] [deux] B` | `A B` |

A remoção deixa **exatamente um espaço entre o que a rodeava, e nada na
margem de uma linha**. Não é uma limpeza dos espaços do ficheiro: dois espaços
que o texto já trazia continuam a ser dois espaços.

**Quando a menção atravessa a quebra de linha, é a quebra que fica** — as
duas linhas continuam a ser duas linhas. Este caso não é uma curiosidade: em quinze
legendagens reais, todos os parênteses retos que pareciam órfãos eram uma
menção cortada dessa maneira.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nReculez ! [Il\nhurle] Tout de suite !\n\n' > a.srt; subedit-cli --quiet hearing-impaired --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nReculez ! [Il\nhurle] Tout de suite !\n\n' > a.srt; subedit-cli --quiet hearing-impaired --output b.srt a.srt; cat b.srt
1
00:00:01,000 --> 00:00:03,000
Reculez !
Tout de suite !
```

**O que não é uma menção e não se move:**

| Caso | O que acontece |
| :-- | :-------------- |
| conteúdo puramente numérico — `[1]`, `(12)`, `[ 156478 ]` | mantido tal como está, delimitadores incluídos: uma referência nunca designou um som |
| um parêntese curvo e o que ele encerra, sem `--parentheses` | mantido tal como está: `Venez (chut) ici` não muda |
| um parêntese reto ou curvo que nada fecha | deixado como está |
| uma etiqueta de formato — `<i>`, `</i>` | nunca removida |

Um espaço **entre** os algarismos, uma letra ou um sinal — `[1 2]`, `[1a]`,
`[-1]` — fazem dela uma menção. O mesmo acontece com um conteúdo vazio, `[]` ou `( )`: sem
algarismo, logo não é uma referência; nada a dizer, logo nada a manter.

## O que desaparece por completo

**Uma linha que a remoção esvaziou desaparece**, e **uma legenda totalmente esvaziada
é removida do ficheiro**. As legendas seguintes são renumeradas.

Uma linha conta como vazia quando tudo o que nela resta são espaços, etiquetas de
formato, ou um único hífen de diálogo — `<i>[Musique]</i>` não deixa um
`<i></i>` no ecrã.

**As etiquetas são as do formato do ficheiro.** `{\i1}[Musique]{\i0}` num
Advanced SSA, `{Y:i}[Musique]` num MicroDVD e `/[Musique]` num MPL2 também não deixam
nada. Inversamente, um `<i>` num formato que não escreve
etiquetas — TMPlayer, LRC — é texto que o espectador lê, e a linha que
o contém não está vazia.

**Um diálogo reduzido a uma única voz já não o é**, e o hífen que resta
desaparece. Enquanto restarem duas vozes, os hífenes ficam.

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

## Um ficheiro sem qualquer menção é escrito na mesma

É copiado de forma idêntica, o código de saída é `0`, e o relatório diz que
não havia nada a remover.

É a regra dos subcomandos que escrevem: **um destino indicado é
um destino escrito**. Abrir aqui uma exceção obrigaria um script a saber
quais dos subcomandos produzem por vezes um ficheiro e por vezes nada.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nRien à signaler.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nRien à signaler.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt
a.srt: no mention to remove -> b.srt
```

## Saída

**Saída padrão** — nada: o resultado é o ficheiro escrito. **Com `--dry-run`, são as alterações propostas**: para cada legenda cujo texto mudaria, o seu número, o texto antes e o texto depois — ver [Ver antes de escrever](invocation.md#ver-antes-de-escrever).

| Nível | O que se acrescenta à saída de erros |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles cleaned, M removed -> <destination>`, ou `<path>: no mention to remove -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — o formato, a codificação, a marca e os fins de linha do ficheiro lido, repostos como estavam |
| 3 | `<path>: N bytes read, M written`, e depois **cada diagnóstico de leitura** — ver [Invocação](invocation.md#ler-os-diagnósticos) |

`N` conta as legendas cujo texto mudou, `M` aquelas que a remoção
esvaziou por completo e que, por isso, saíram do ficheiro.

### Em JSON

Com [`--format json`](invocation.md#uma-saída-que-um-script-pode-ler), o objeto de cada ficheiro traz `counts.cleaned` (os textos reescritos) e `counts.removed` (as legendas removidas); **com `--dry-run`**, a chave `changes` lista também as legendas alteradas, como a saída padrão da forma em texto. A tabela completa é a de [Invocação](invocation.md#o-que-cada-subcomando-põe-em-counts).

## Códigos de saída

Os da ferramenta: `0` se todos os ficheiros foram escritos, `2` se nenhum, `3` se
apenas alguns, `1` perante um erro de utilização.

## Erros

Este subcomando não tem erros próprios: não recebe nenhum
valor para analisar, e nenhum texto o pode levar a recusar um ficheiro. O que
pode falhar é a leitura ou a escrita, como nos outros.

**Nada é escrito sem uma das três opções de destino**, e os respetivos erros
são comuns aos subcomandos que escrevem:
ver [Invocação](invocation.md#o-destino).
