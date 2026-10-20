# `transform`

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revisada por um leitor do idioma.

```
subedit-cli transform --first <index>=<time> --last <index>=<time>
                      (--output FILE | --output-dir DIR | --in-place)
                      [--dry-run]
                      [--recursive]
                      <file>...
```

Corrige **todas** as posições do arquivo a partir de dois pontos cujo valor correto
é conhecido. É a operação que uma legendagem à deriva pede
— certa no início, errada no fim — e que [`shift`](shift.md), que move tudo pela
mesma quantidade, não pode fazer.

O formato do arquivo lido é **mantido**: mudar o formato é trabalho de
[`convert`](convert.md).

<!-- example: subedit-cli transform --help -->
```console
$ subedit-cli transform --help
Correct every position from two points known to be right
Usage: subedit-cli transform [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to transform

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --first TEXT REQUIRED       Earlier reference, as <index>=<time>: 1=00:00:01.000
  --last TEXT REQUIRED        Later reference, as <index>=<time>: 3=00:00:10.000
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Argumentos e opções

| Opção | Obrigatório | Valores | Padrão |
| :----- | :----- | :------ | :----- |
| `<file>...` | sim | um ou mais caminhos | — |
| `--recursive`, `-r` | não | uma opção sem valor | desativado — veja [Processar uma árvore de diretórios](lots.md) |
| `--first` | **sim** | uma referência, `<index>=<time>` | — |
| `--last` | **sim** | uma referência, `<index>=<time>` | — |
| `--output` / `--output-dir` / `--in-place` | **um dos três**, exceto com `--dry-run` | veja [Invocação](invocation.md#o-destino) | — |
| `--dry-run` | não | uma opção sem valor: calcula e informa, não grava nada — veja [Ver antes de escrever](invocation.md#ver-antes-de-escrever) | desativado |

Não há forma curta: essas duas opções só se escrevem por
extenso.

## Escrever uma referência

Uma referência nomeia uma legenda e diz onde seu **início** deve passar a cair:

```
<index>=<time>
```

**O índice é contado a partir de 1**, como é exibido e como um arquivo
SubRip o numera. `0` é recusado, e também um índice além da última
legenda.

**O tempo é escrito como o de [`shift`](shift.md)** — segundos ou
marca de tempo, o separador decimal sendo um ponto, a resolução o milissegundo:

| Forma | Exemplos |
| :---- | :------- |
| segundos | `10`, `2.999` |
| marca de tempo | `00:00:10.000`, `01:39:37.040` |

`--first 3=10` e `--first 3=00:00:10.000` dizem a mesma coisa.

`--first` e `--last` nomeiam dois pontos, não um intervalo. Seus nomes dizem
o uso esperado — uma referência no início do arquivo, a outra no fim — e não uma
restrição: trocá-las dá o mesmo resultado, e **tudo o que está fora
das duas referências acompanha como o que está entre elas**.

## No que as outras posições se transformam

As duas referências fixam um fator de escala, e todas as posições o seguem:

```
t′ = y₁ + (t − x₁) × r      with r = (y₂ − y₁) / (x₂ − x₁)
```

`x₁`, `x₂` são os inícios atuais das duas legendas nomeadas, `y₁`, `y₂` os
tempos pedidos. O fator é mantido como um racional exato e **o arredondamento
acontece uma única vez**, ao milissegundo; é isso que faz as duas
referências caírem **exatamente** nos tempos pedidos e não com a precisão de um milésimo
(veja [ADR 0013](../../../../adr/0013-mise-a-l-echelle-exacte-des-positions.md)).

Cada legenda permanece na tela **em proporção** ao tempo que nela permanecia:
o fim segue a mesma escala que o início. Uma transformação não é um
deslocamento, e não tem a propriedade dele.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,001 --> 00:00:07,000\nDeux.\n\n3\n00:00:09,000 --> 00:00:11,000\nTrois.\n\n' > a.srt; subedit-cli transform --first 1=00:00:01.000 --last 3=00:00:10.000 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,001 --> 00:00:07,000\nDeux.\n\n3\n00:00:09,000 --> 00:00:11,000\nTrois.\n\n' > a.srt; subedit-cli transform --first 1=00:00:01.000 --last 3=00:00:10.000 --output b.srt a.srt; cat b.srt
a.srt: 3 subtitles transformed onto 1=00:00:01.000 and 3=00:00:10.000 -> b.srt
1
00:00:01,000 --> 00:00:03,250
Un.

2
00:00:05,501 --> 00:00:07,750
Deux.

3
00:00:10,000 --> 00:00:12,250
Trois.
```

A primeira referência não se move, a terceira vai de 9 s a 10 s: o fator é
`9/8`, e a segunda legenda — que ninguém nomeou — vai de
`5,001` a `5,501`, isto é, `1000 + 4001 × 9/8` arredondado uma única vez.

## Duas referências que não o são

| Situação | Quando é percebida | Código |
| :-------- | :------------- | :--- |
| `--first` e `--last` nomeiam o mesmo índice | antes de qualquer leitura | `1` |
| um índice está além da última legenda | quando o arquivo é aberto | `2` ou `3` |
| as duas legendas nomeadas começam no mesmo instante | quando o arquivo é aberto | `2` ou `3` |

Os três são recusados pelo mesmo motivo: `x₂ − x₁` é zero, e dois pontos
coincidentes não definem nenhuma reta. Devolver uma divisão por zero disfarçada de
resultado seria pior do que recusar.

O primeiro caso é um **erro de uso** — as duas opções estão erradas, quaisquer que
sejam os arquivos, de modo que se deve uma única mensagem, não uma por arquivo. Os
outros dois dependem do arquivo aberto e contam, portanto, como uma falha de
processamento.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n' > a.srt; subedit-cli transform --first 2=1.000 --last 2=4.000 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n' > a.srt; subedit-cli transform --first 2=1.000 --last 2=4.000 --output b.srt a.srt; echo "code=$?"
--first and --last both name subtitle 2, and two references on one subtitle define no transform
code=1
```

Um índice fora dos limites é recusado **nomeando o limite**, o que diz ao
mesmo tempo o que poderia ter sido escrito:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,000 --> 00:00:07,000\nDeux.\n\n' > a.srt; subedit-cli transform --first 1=1.000 --last 9=10.000 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,000 --> 00:00:07,000\nDeux.\n\n' > a.srt; subedit-cli transform --first 1=1.000 --last 9=10.000 --output b.srt a.srt; echo "code=$?"
a.srt: subtitle 9 is past the end: the file holds 2 subtitles
code=2
```

## Uma posição antes da origem é recusada

Como em [`shift`](shift.md), e pelo mesmo motivo: uma posição negativa
se escreveria `-00:00:01,000`, uma forma que nenhum dos nove formatos define e que nenhum
leitor aceita. O caso surge aqui quando uma legenda está **antes
da primeira referência** e a escala a empurra para além de zero. Nada é
gravado, o código é `2`, e a mensagem nomeia a legenda em questão.

## Saída

**Saída padrão** — nada: o resultado é o arquivo gravado. Com `--dry-run`, também nada: não há arquivo, e a linha de narração o diz — veja [Ver antes de escrever](invocation.md#ver-antes-de-escrever).

| Nível | O que se acrescenta na saída de erro |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles transformed onto 1=00:00:01.000 and 3=00:00:10.000 -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — o formato, a codificação, a marca e os finais de linha do arquivo lido, restituídos como estavam |
| 3 | `<path>: N bytes read, M written`, depois **todo diagnóstico de leitura** — veja [Invocação](invocation.md#diagnósticos-de-leitura) |

As referências são reescritas com o ponto decimal, qualquer que tenha sido a forma como foram digitadas.

### Em JSON

Com [`--format json`](invocation.md#uma-saída-que-um-script-pode-ler), o objeto de cada arquivo traz `counts.subtitles` (todas as legendas do arquivo). A tabela completa é a de [Invocação](invocation.md#o-que-cada-subcomando-coloca-em-counts).

## Códigos de saída

Os da ferramenta: `0` se todos os arquivos foram gravados, `2` se nenhum, `3` se
apenas alguns, `1` em caso de erro de uso.

## Erros

| O que o provoca | Mensagem |
| :------------------ | :------ |
| referência sem `=` | `"…" is not a reference: write it <index>=<time>, as in 3=00:00:10.000` |
| referência sem uma das duas metades | `"…" is missing one of its two halves: write it <index>=<time>, as in 3=00:00:10.000` |
| índice `0` | `"0" is not a subtitle number: subtitles are counted from 1, as the file shows them` |
| índice que não é um número inteiro | `"…" is not a subtitle number: expected a whole number, counted from 1 as the file shows them` |
| tempo inválido | os de [`shift`](shift.md) |
| `--first` e `--last` no mesmo índice | `--first and --last both name subtitle N, and two references on one subtitle define no transform` |
| índice fora dos limites | `<path>: subtitle N is past the end: the file holds M subtitles` |
| duas legendas que começam juntas | `<path>: subtitles N and M start at the same moment, so they define no transform` |
| posição antes da origem | `<path>: subtitle N would land before the origin, which no subtitle file can hold` |

**Nada é gravado sem uma das três opções de destino**, e os erros delas
são comuns aos subcomandos que gravam:
veja [Invocação](invocation.md#o-destino).
