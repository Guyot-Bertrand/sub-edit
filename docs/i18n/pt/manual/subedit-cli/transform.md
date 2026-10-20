# `transform`

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revista por nenhum leitor do idioma.

```
subedit-cli transform --first <index>=<time> --last <index>=<time>
                      (--output FILE | --output-dir DIR | --in-place)
                      [--dry-run]
                      [--recursive]
                      <file>...
```

Corrige **todas** as posições do ficheiro a partir de dois pontos cujo valor certo
é conhecido. É a operação que uma sincronização com desvio progressivo exige
— certa no início, errada no fim — onde [`shift`](shift.md), que move tudo da
mesma quantidade, nada pode fazer.

O formato do ficheiro lido é **mantido**: mudar o formato é trabalho de
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

| Opção | Obrigatório | Valores | Predefinição |
| :----- | :----- | :------ | :----- |
| `<file>...` | sim | um ou mais caminhos | — |
| `--recursive`, `-r` | não | um indicador | desativado — ver [Processar uma árvore de pastas](lots.md) |
| `--first` | **sim** | uma referência, `<index>=<time>` | — |
| `--last` | **sim** | uma referência, `<index>=<time>` | — |
| `--output` / `--output-dir` / `--in-place` | **uma das três**, exceto com `--dry-run` | ver [Invocação](invocation.md#o-destino) | — |
| `--dry-run` | não | um indicador: calcula e diz, não escreve nada — ver [Ver antes de escrever](invocation.md#ver-antes-de-escrever) | desativado |

Não há forma curta: estas duas opções só se escrevem por
extenso.

## Escrever uma referência

Uma referência nomeia uma legenda e diz onde o seu **início** deve passar a cair:

```
<index>=<time>
```

**O índice conta-se a partir de 1**, tal como é mostrado e como um ficheiro
SubRip o numera. `0` é recusado, e também um índice para além da última
legenda.

**O tempo escreve-se como o de [`shift`](shift.md)** — segundos ou
marca de tempo, o separador decimal um ponto, a resolução o milissegundo:

| Forma | Exemplos |
| :---- | :------- |
| segundos | `10`, `2.999` |
| marca de tempo | `00:00:10.000`, `01:39:37.040` |

`--first 3=10` e `--first 3=00:00:10.000` dizem a mesma coisa.

`--first` e `--last` nomeiam dois pontos, não um intervalo. Os seus nomes dizem
a utilização esperada — uma referência no início do ficheiro, a outra no fim — e não uma
restrição: trocá-los dá o mesmo resultado, e **tudo o que fica fora
das duas referências acompanha-as como o que está entre elas**.

## No que se tornam as outras posições

As duas referências fixam um fator de escala, e todas as posições o seguem:

```
t′ = y₁ + (t − x₁) × r      with r = (y₂ − y₁) / (x₂ − x₁)
```

`x₁`, `x₂` são os inícios atuais das duas legendas nomeadas, `y₁`, `y₂` os
tempos pedidos. O fator é mantido como um racional exato e **o arredondamento
faz-se uma só vez**, ao milissegundo; é isso que faz com que as duas
referências caiam **exatamente** nos tempos pedidos e não com a aproximação de um milésimo
(ver [ADR 0013](../../../../adr/0013-mise-a-l-echelle-exacte-des-positions.md)).

Cada legenda permanece no ecrã **proporcionalmente** ao tempo que lá permanecia:
o fim segue a mesma escala que o início. Uma transformação não é um
deslocamento, e não tem a sua propriedade.

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

A primeira referência não se move, a terceira passa de 9 s para 10 s: o fator é
`9/8`, e a segunda legenda — que ninguém nomeou — passa de
`5,001` a `5,501`, isto é, `1000 + 4001 × 9/8` arredondado uma só vez.

## Duas referências que não o são

| Situação | Quando é detetada | Código |
| :-------- | :------------- | :--- |
| `--first` e `--last` nomeiam o mesmo índice | antes de qualquer leitura | `1` |
| um índice está para além da última legenda | quando o ficheiro é aberto | `2` ou `3` |
| as duas legendas nomeadas começam no mesmo instante | quando o ficheiro é aberto | `2` ou `3` |

Os três casos são recusados pela mesma razão: `x₂ − x₁` é zero, e dois
pontos coincidentes não definem nenhuma reta. Devolver uma divisão por zero disfarçada de
resultado seria pior do que recusar.

O primeiro caso é um **erro de utilização** — as duas opções estão em falta sejam quais forem
os ficheiros, pelo que se deve uma única mensagem, e não uma por ficheiro. Os
outros dois dependem do ficheiro aberto, e contam, portanto, como uma
falha de processamento.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n' > a.srt; subedit-cli transform --first 2=1.000 --last 2=4.000 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n' > a.srt; subedit-cli transform --first 2=1.000 --last 2=4.000 --output b.srt a.srt; echo "code=$?"
--first and --last both name subtitle 2, and two references on one subtitle define no transform
code=1
```

Um índice fora dos limites é recusado **nomeando o limite**, o que diz ao
mesmo tempo o que se podia ter escrito:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,000 --> 00:00:07,000\nDeux.\n\n' > a.srt; subedit-cli transform --first 1=1.000 --last 9=10.000 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,000 --> 00:00:07,000\nDeux.\n\n' > a.srt; subedit-cli transform --first 1=1.000 --last 9=10.000 --output b.srt a.srt; echo "code=$?"
a.srt: subtitle 9 is past the end: the file holds 2 subtitles
code=2
```

## Uma posição anterior à origem é recusada

Como em [`shift`](shift.md), e pela mesma razão: uma posição negativa
escreve-se `-00:00:01,000`, uma forma que nenhum dos nove formatos define e que nenhum
leitor aceita. O caso surge aqui quando uma legenda se situa **antes
da primeira referência** e a escala a empurra para além de zero. Nada é
escrito, o código é `2`, e a mensagem nomeia a legenda em falta.

## Saída

**Saída padrão** — nada: o resultado é o ficheiro escrito. Com `--dry-run`, também nada: não há ficheiro, e a linha de narração di-lo — ver [Ver antes de escrever](invocation.md#ver-antes-de-escrever).

| Nível | O que se acrescenta à saída de erros |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles transformed onto 1=00:00:01.000 and 3=00:00:10.000 -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — o formato, a codificação, a marca e os fins de linha do ficheiro lido, repostos como estavam |
| 3 | `<path>: N bytes read, M written`, depois **todos os diagnósticos de leitura** — ver [Invocação](invocation.md#ler-os-diagnósticos) |

As referências são escritas de volta com o ponto decimal, seja como for que tenham sido escritas.

### Em JSON

Com [`--format json`](invocation.md#uma-saída-que-um-script-pode-ler), o objeto de cada ficheiro traz `counts.subtitles` (todas as legendas do ficheiro). A tabela completa é a de [Invocação](invocation.md#o-que-cada-subcomando-põe-em-counts).

## Códigos de saída

Os da ferramenta: `0` se todos os ficheiros foram escritos, `2` se nenhum, `3` se
apenas alguns, `1` num erro de utilização.

## Erros

| O que o desencadeia | Mensagem |
| :------------------ | :------ |
| referência sem `=` | `"…" is not a reference: write it <index>=<time>, as in 3=00:00:10.000` |
| referência a que falta uma das metades | `"…" is missing one of its two halves: write it <index>=<time>, as in 3=00:00:10.000` |
| índice `0` | `"0" is not a subtitle number: subtitles are counted from 1, as the file shows them` |
| índice que não é um número inteiro | `"…" is not a subtitle number: expected a whole number, counted from 1 as the file shows them` |
| tempo inválido | os de [`shift`](shift.md) |
| `--first` e `--last` no mesmo índice | `--first and --last both name subtitle N, and two references on one subtitle define no transform` |
| índice fora dos limites | `<path>: subtitle N is past the end: the file holds M subtitles` |
| duas legendas que começam juntas | `<path>: subtitles N and M start at the same moment, so they define no transform` |
| posição anterior à origem | `<path>: subtitle N would land before the origin, which no subtitle file can hold` |

**Nada é escrito sem uma das três opções de destino**, e os seus
erros são comuns aos subcomandos que escrevem:
ver [Invocação](invocation.md#o-destino).
