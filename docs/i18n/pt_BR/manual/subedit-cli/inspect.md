# `inspect`

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revisada por um leitor do idioma.

```
subedit-cli inspect [--frame-rate RATE] [--recursive] [--detail]
                   [-t FILE] [--align-method position|number] <file>...
```

Relata do que cada arquivo é feito. **Não altera nada e não grava nenhum
arquivo.**

<!-- example: subedit-cli inspect --help -->
```console
$ subedit-cli inspect --help
Report what a subtitle file is made of
Usage: subedit-cli inspect [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to report on

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --detail                    List every anomaly, subtitle by subtitle, instead of counting them
  --frame-rate RATE           Frame rate of a file counted in frames: 25, 23.976
  -t,--translation-file FILE  Translation file to lay over the subtitle file, for a single input
  --align-method position|number
                              How the lines of the translation find their subtitles
```

## Argumentos

| Argumento | Obrigatório | Valor | Padrão |
| :------- | :----- | :----- | :----- |
| `<file>...` | sim | um ou mais caminhos de arquivos de legendas | — |
| `--recursive`, `-r` | não | uma opção sem valor | desativada — veja [Processar uma árvore de diretórios](lots.md) |
| `--detail` | não | uma opção sem valor: nomeia cada anomalia, em vez de contá-las — veja [O que `anomalies` relata](#o-que-anomalies-relata) | desativada |

Nenhum caminho é um erro de uso, portanto código `1`.

## Opções

| Opção | Obrigatório | Valor | Padrão |
| :----- | :------ | :----- | :----- |
| `--frame-rate` | não | uma taxa em quadros por segundo: `25`, `23.976`, `29.97` | 23.976 |

**Ela só serve para arquivos contados em quadros**, isto é, MicroDVD, o
único dos nove. Um arquivo MicroDVD não traz nada além de números de quadros; a
taxa com que eles são lidos decide **cada posição do relatório**,
a começar por `span`. Indicada em outros casos, não muda nada e não é
um erro.

Um valor que não designa nada é um erro de uso, portanto código `1`:

<!-- example: subedit-cli inspect --frame-rate douze film.sub; echo $? -->
```console
$ subedit-cli inspect --frame-rate douze film.sub; echo $?
--frame-rate: "douze" is not a frame rate: expected frames per second, like 25 or 23.976
1
```

É a mesma opção que em [`convert`](convert.md), palavra por palavra, porque
é a mesma taxa. `convert` também a usa para **gravar**.

## Uma tradução: `-t`

| Opção | Obrigatório | Valor | Padrão |
| :----- | :------ | :----- | :----- |
| `-t`, `--translation-file` | não | um arquivo de tradução, para **uma única** entrada | — |
| `--align-method` | não, e somente com `-t` | `position` ou `number` | `position` |

Com `-t`, a tradução é colocada sobre o arquivo **como a janela faz ao abri-la** — veja
[Uma tradução](invocation.md#uma-tradução) — e o relatório diz **como suas linhas encontraram sua
legenda**. Um script sabe assim se uma tradução está alinhada antes de corrigi-la.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour.\n\n' > film.fr.srt; subedit-cli --quiet inspect -t film.fr.srt film.srt | tail -3 -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour.\n\n' > film.fr.srt; subedit-cli --quiet inspect -t film.fr.srt film.srt | tail -3
  anomalies: none
  translation file: film.fr.srt, matched by position
  translation: 1 line attached; 1 subtitle left without a translation
```

A primeira linha acrescentada nomeia o arquivo e o método; a segunda é **a frase que a janela
diz** ao abrir uma tradução, cada parte somente quando não é zero:

| Parte | O que conta |
| :----- | :---------------- |
| `N lines attached` | as linhas da tradução que encontraram uma legenda do arquivo principal |
| `N subtitles born of a line` | as linhas que não encontraram nenhuma e deram origem a uma legenda própria, em suas próprias posições |
| `N subtitles left without a translation` | as legendas do arquivo principal que nenhuma linha alcançou |
| `N lines out of order` | as linhas que vinham depois de uma linha posterior no arquivo |

Uma tradução que não traz nenhuma linha diz `translation: the file holds no line`. **O relatório do
próprio arquivo continua sendo o do arquivo sozinho**: uma legenda nascida de uma linha não é contada em
`subtitles`.

Os erros de `-t` são os de uma leitura, ditos com o caminho da tradução, e este:
`<translation>: the file is already open as the main document`, quando o arquivo indicado é o principal.
`--align-method` sem `-t` é um erro de uso, e `-t` com várias entradas também — veja
[Uma tradução](invocation.md#uma-tradução).

## Saída

O relatório vai para a **saída padrão** — é o resultado do comando, e
é escrito em todos os níveis, `--quiet` incluído. Um bloco por arquivo, precedido
pelo caminho:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,500\nLe canot dérive.\n\n2\n00:00:04,000 --> 00:00:06,200\nPersonne n a vu la côte.\n' > exemple.srt; subedit-cli --quiet inspect exemple.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,500\nLe canot dérive.\n\n2\n00:00:04,000 --> 00:00:06,200\nPersonne n a vu la côte.\n' > exemple.srt; subedit-cli --quiet inspect exemple.srt
exemple.srt
  format: SubRip
  encoding: UTF-8, detected
  byte order mark: absent
  line endings: LF
  subtitles: 2
  span: 00:00:01.000 -> 00:00:06.200
  frame rate grid: none (too few subtitles to tell)
  anomalies: none
```

| Campo | O que informa |
| :---- | :----------- |
| `format` | um dos formatos que `--to` aceita, escrito por extenso: `SubRip`, `WebVTT`, `SubViewer 2`, `Sub Station Alpha`, `Advanced SSA`, `MicroDVD`, `MPL2`, `TMPlayer`, `LRC` |
| `encoding` | a codificação lida, seguida de **onde vem a resposta**: `, from its byte order mark` se o arquivo a declara, `, as asked for` se `--encoding` a impôs, `, detected` se foi avaliada |
| `byte order mark` | `present` ou `absent` |
| `line endings` | `LF`, `CRLF` ou `CR`, seguido de `, mixed from line N` se o arquivo os mistura |
| `subtitles` | o número de legendas lidas |
| `span` | do início mais cedo ao fim mais tardio, `HH:MM:SS.mmm` |
| `frame rate grid` | a taxa de quadros sobre a qual as posições foram calculadas, **deduzida** — veja abaixo |
| `frame rate` | para um arquivo contado em quadros, a taxa com que foi lido e **de onde ela vem** — substitui a linha anterior, veja abaixo |
| `anomalies` | `none` na mesma linha, ou o que está errado: **um tipo por linha com sua contagem**, recuado sob `anomalies:`; com `--detail`, **uma anomalia por linha**, legenda por legenda |

**`span` não é "da primeira à última"** mas do início mais cedo ao fim mais tardio:
em um arquivo cuja ordem está quebrada, os dois diferem, e somente o segundo diz
a verdade sobre o que o arquivo cobre.

**`anomalies`, com `--detail`, conta as legendas a partir de 1**, como o arquivo
as numera — e não as linhas do arquivo. O número de uma legenda sobrevive a uma
alteração; o número de uma linha, não. Veja abaixo.

## O que `frame rate grid` relata

**Um arquivo de legendas não declara sua taxa de quadros.** O SubRip não tem
cabeçalho, o do WebVTT é texto livre. Esta linha, portanto, não lê nada:
ela **deduz**, medindo se as posições caem sobre uma grade de quadros.

O que se mede são os **inícios**. Onde existe uma grade, eles estão sobre ela
sem exceção, ao passo que os fins costumam se afastar dela — um fim é
com frequência calculado por uma regra de velocidade de leitura, e não colocado sobre
um quadro.

A primeira linha diz sempre a mesma coisa: a taxa escolhida, o veredito
e a concentração que o sustenta.

| Veredito | O que significa |
| :------ | :----------------- |
| `clean` | as posições estão sobre uma grade |
| `partial` | parte delas está, o resto não |
| `none` | nenhum candidato explica algo |

`none` tem **duas causas, e elas não são a mesma resposta**: ou nenhuma
grade se ajusta — a linha dá então o melhor candidato e sua pontuação,
para que se veja por quanto ele falha — ou o arquivo tem legendas em número insuficiente
para que se possa dizer algo sobre ele. Dois inícios sempre parecem
perfeitamente alinhados; isso não prova nada.

**Um arquivo regular em uma taxa fora do padrão dá `none`, e isso é
intencional.** O conjunto de candidatos é fechado — as oito taxas que a
conversão oferece — e relatar a menos errada delas seria dar
uma resposta errada onde "não sei" é a resposta certa.

### As linhas seguintes, quando têm algo a dizer

| Linha | Quando aparece |
| :---- | :------------------ |
| `grid offset` | as posições estão sobre a grade **a menos de uma constante**: o arquivo foi deslocado, e eis de quanto |
| `also fits` | outro candidato se ajusta igualmente bem, por ser um múltiplo inteiro do escolhido |
| `too short a span to separate` | a duração total do arquivo não permite distinguir a taxa escolhida das nomeadas |
| `off the grid` | quantos inícios se afastam da grade, e em quantas **sequências** |

**`also fits` não é uma hesitação.** Uma grade a 25 quadros por segundo está
*incluída* em uma grade a 50: um arquivo sincronizado a 25 se ajusta, portanto, às duas.
O inverso é falso — um 50 real desmorona sobre 25 — e por isso é a mais baixa que
é escolhida, e a outra é nomeada em vez de silenciada. Os únicos
pares envolvidos são 25 e 50, 29.97 e 59.94, 30 e 60.

**`too short a span to separate` protege contra uma resposta confiante demais.** 23.976
e 24 se afastam um milissegundo por segundo de filme: em um longa-metragem elas
se separam sem dúvida, ao longo de dez segundos não se distinguem. A linha diz
qual delas continua possível.

**`off the grid` distingue duas histórias que a porcentagem sozinha confunde.**
Muitas sequências de um único início são posições corrigidas à mão,
uma a uma. Poucas sequências longas são uma seção ressincronizada ou um arquivo
montado a partir de dois outros. O número de sequências é o que as separa.

### Um exemplo

Doze linhas colocadas sobre uma grade a 25 quadros por segundo — portanto em
posições que são múltiplos de 40 milissegundos — distribuídas por quase nove minutos:

<!-- example: k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > grille.srt; subedit-cli --quiet inspect grille.srt | tail -3 -->
```console
$ k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > grille.srt; subedit-cli --quiet inspect grille.srt | tail -3
  frame rate grid: 25 fps, clean (100.0%)
  also fits: 50 fps, of which this rate is a whole divisor
  anomalies: none
```

O exemplo cria seu próprio arquivo em vez de ler um do repositório, e isso é
deliberado: ele pode ser reproduzido tal como está, e o laço mostra o que é uma grade
melhor do que uma frase — posições que são múltiplos da duração de um quadro.

## O que `frame rate` relata, e por que não é uma grade

**Apenas um dos nove formatos conta em quadros**: o MicroDVD, cada linha do qual
traz dois números de quadro e nada mais. Em tal arquivo, `inspect`
não escreve `frame rate grid` mas `frame rate`, e as duas linhas nunca
são exibidas juntas.

**A razão cabe em uma frase.** As posições de um arquivo MicroDVD foram
calculadas *a partir* de seus quadros, a uma dada taxa; deduzi-las de uma
grade devolveria essa mesma taxa, tal como acaba de ser fornecida. Seria
um número dado, disfarçado de medição.

O que se escreve em seu lugar é **a taxa de leitura e de onde ela vem** —
a mesma forma da linha `encoding`:

| O que a linha diz | Quando |
| :------------------ | :---- |
| `frame rate: 25 fps, as asked for` | `--frame-rate` a impôs |
| `frame rate: 24000/1001 fps, assumed` | ninguém a disse; é a padrão |

**Duas respostas e não três**, onde `encoding` tem três: um arquivo MicroDVD
não declara nada. Esse é todo o problema — a taxa vem necessariamente
de fora, e o relatório diz de qual dos dois lados de fora.

**A taxa decide todo o resto do relatório.** `span` passa por ela, e duas
leituras do mesmo arquivo a duas taxas dão dois valores de duração total:

<!-- example: printf '{25}{75}Première.\n{100}{150}Deuxième.\n{200}{260}Troisième.\n' > images.sub; subedit-cli --quiet inspect images.sub | tail -3; subedit-cli --quiet inspect --frame-rate 25 images.sub | tail -3 -->
```console
$ printf '{25}{75}Première.\n{100}{150}Deuxième.\n{200}{260}Troisième.\n' > images.sub; subedit-cli --quiet inspect images.sub | tail -3; subedit-cli --quiet inspect --frame-rate 25 images.sub | tail -3
  span: 00:00:01.043 -> 00:00:10.844
  frame rate: 24000/1001 fps, assumed
  anomalies: none
  span: 00:00:01.000 -> 00:00:10.400
  frame rate: 25 fps, as asked for
  anomalies: none
```

## O que `anomalies` relata

Três coisas podem estar erradas em um documento, e cada uma é reparada de maneira diferente.
**Por padrão, `inspect` as conta**, uma linha por tipo, na ordem da tabela:
um arquivo mal feito carrega centenas delas, e saber quantas há de cada tipo
é o que diz o que executar. **`--detail` as nomeia uma a uma.**

| O que é escrito | Com `--detail` | O que significa |
| :--------------- | :-------------- | :------------------ |
| `subtitle ends before it starts: N` | `subtitle N ends before it starts` | o fim vem antes do início |
| `subtitle starts before the previous one ends: N` | `subtitle N starts before the previous one ends` | ela se sobrepõe à anterior |
| `subtitle starts before the previous one starts: N` | `subtitle N starts before the previous one starts` | ela quebra a ordem do arquivo — [`sort`](sort.md) a recoloca no lugar |

Um tipo ausente não é escrito. Na coluna da esquerda, `N` é o número
de legendas; na seguinte, o número de uma legenda.

**Uma mesma legenda pode aparecer duas vezes**, e isso é intencional: a que
começa antes de a anterior ter começado também começa antes de ela ter
terminado. O primeiro ponto se resolve movendo a legenda, o segundo
alterando uma duração.

**A legenda nomeada é a que está fora do lugar**, não aquela em relação à qual
ela está fora do lugar. Com inícios em `0`, `4 s`, `2 s`, `3 s`: a terceira quebra a ordem, a
quarta realmente segue a terceira e portanto não é nomeada — não há nada
a fazer a seu respeito.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect desordre.srt | tail -3 -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect desordre.srt | tail -3
  anomalies:
    subtitle starts before the previous one ends: 1
    subtitle starts before the previous one starts: 1
```

E com `--detail`, a legenda nomeada:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect --detail desordre.srt | tail -3 -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect --detail desordre.srt | tail -3
  anomalies:
    subtitle 3 starts before the previous one ends
    subtitle 3 starts before the previous one starts
```

**Uma linha por anomalia com `--detail`**, porque cem constatações
coladas em uma linha não podem ser lidas nem pesquisadas. Um documento sem defeito mantém
sua linha única, `anomalies: none`, que um script procura. **O registro de
`--format json` sempre lista cada anomalia**: um script lê o detalhe, e
`--detail` só muda o texto.

**Não é um diagnóstico de leitura.** Os diagnósticos dizem o que a
leitura encontrou e apontam para uma **linha do arquivo**; as anomalias dizem
o que o documento *é* e apontam para uma **legenda**. Um arquivo cuja ordem
está quebrada não é por isso malformado: a leitura não tem nada a dizer sobre ele.

## Em JSON

Com [`--format json`](invocation.md#uma-saída-que-um-script-pode-ler), `inspect` escreve **um
objeto por arquivo** no lugar dos blocos acima; a narração continua na saída de erro.
As chaves descrevem o que o texto diz, e **nenhuma deve ser relida a partir de uma frase**:

| Chave | Conteúdo |
| :-- | :------ |
| `format` | o identificador do formato, o que `convert --to` aceita: `srt`, `vtt`… |
| `encoding` | `{"charset", "origin", "byte_order_mark"}` — `origin` é `byte-order-mark`, `asked` ou `detected` |
| `line_endings` | `{"kind", "mixed_from_line"}` — `lf`, `crlf` ou `cr`; a linha em que um segundo tipo aparece, ou `null` |
| `subtitles` | o número de legendas |
| `span_ms` | `{"start", "end"}` — o primeiro início e o último fim, em milissegundos |
| `frame_rate` | para um arquivo contado em quadros: `{"rate", "origin"}` (`asked` ou `assumed`); caso contrário `null` |
| `grid` | para os demais: a grade deduzida, ou `null` para um arquivo contado em quadros |
| `anomalies` | uma matriz de `{"subtitle", "kind"}` — `end-before-start`, `overlapping-subtitles`, `out-of-order` |
| `translation` | **somente com `-t`**: `{"file", "method", "attached", "born", "untranslated", "out_of_order"}` — `method` é `position` ou `number`, e as quatro contagens são as da frase acima |
| `warnings` | o que a leitura encontrou, em todos os níveis |

`grid` traz `verdict` (`clean`, `partial` ou `silent`), `enough_starts`, `rate` (uma cadeia de caracteres,
ou `null` quando nada é nomeado), `concentration_permille` (milésimos, `null` sem inícios
suficientes), `offset_ms`, `also_fits` (a taxa da qual esta é um divisor inteiro, ou
`null`), `not_separated` (as taxas que uma duração total curta demais não separa), `strays`
(os inícios fora da grade) e `starts`.

<!-- example: printf '{25}{50}Un.\n{75}{100}Deux.\n' > f.sub; subedit-cli --format json inspect --frame-rate 25 f.sub 2>/dev/null -->
```console
$ printf '{25}{50}Un.\n{75}{100}Deux.\n' > f.sub; subedit-cli --format json inspect --frame-rate 25 f.sub 2>/dev/null
{"schema":1,"command":"inspect","file":"f.sub","ok":true,"format":"microdvd","encoding":{"charset":"UTF-8","origin":"detected","byte_order_mark":false},"line_endings":{"kind":"lf","mixed_from_line":null},"subtitles":2,"span_ms":{"start":1000,"end":4000},"frame_rate":{"rate":"25","origin":"asked"},"grid":null,"anomalies":[],"warnings":[]}
```

## Narração

Na saída de erro, conforme o nível pedido:

| Nível | O que se acrescenta |
| :----- | :-------------- |
| 1 | `<path>: N subtitles` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings` |
| 3 | `<path>: N bytes read`, `<path>: N diagnostic(s) while reading`, depois **cada um deles** — veja [Invocação](invocation.md#diagnósticos-de-leitura) |

## Códigos de saída

Os da ferramenta: `0` se todos os arquivos foram lidos, `2` se nenhum, `3` se
apenas alguns, `1` em caso de erro de uso.

## Erros

Os de uma leitura — do arquivo, e da tradução quando `-t` é indicado —, e
os de `-t`: veja acima. `inspect` não grava nada que possa ser recusado.

| O que o provoca | Mensagem |
| :------------------ | :------ |
| nenhum caminho indicado | `files is required`, seguido de uma indicação para `--help` |
| arquivo ausente | `<path>: does not exist` |
| arquivo ilegível | `<path>: cannot be opened: permission denied` |
| bytes que não decodificam | `<path>: cannot be decoded in the chosen encoding` |
| formato não reconhecido | `<path>: is in no format this tool knows` |
| nada que se pareça com uma legenda | `<path>: holds nothing recognisable as a subtitle` |

São as mesmas mensagens para os dezessete subcomandos: a receita para abrir
está escrita uma só vez, no núcleo.

## Um exemplo que falha

Um arquivo vazio não corresponde a nenhum formato. Ele não é relatado como vazio:
é recusado, e nomeado.

<!-- example: : > vide.srt; subedit-cli inspect vide.srt; echo "code=$?" -->
```console
$ : > vide.srt; subedit-cli inspect vide.srt; echo "code=$?"
vide.srt: is in no format this tool knows
code=2
```
