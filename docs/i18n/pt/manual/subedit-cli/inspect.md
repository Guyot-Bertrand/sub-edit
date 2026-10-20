# `inspect`

<!-- machine-translated -->
> Esta página é uma tradução automática e ainda não foi revista por nenhum leitor do idioma.

```
subedit-cli inspect [--frame-rate RATE] [--recursive] [--detail]
                   [-t FILE] [--align-method position|number] <file>...
```

Comunica de que é feito cada ficheiro. **Não altera nada e não escreve nenhum
ficheiro.**

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

| Argumento | Obrigatório | Valor | Predefinição |
| :------- | :----- | :----- | :----- |
| `<file>...` | sim | um ou mais caminhos de ficheiros de legendas | — |
| `--recursive`, `-r` | não | um indicador | desativado — ver [Processar uma árvore de pastas](lots.md) |
| `--detail` | não | um indicador: nomeia cada anomalia, em vez de as contar — ver [O que `anomalies` comunica](#o-que-anomalies-comunica) | desativado |

Nenhum caminho é um erro de utilização, portanto código `1`.

## Opções

| Opção | Obrigatória | Valor | Predefinição |
| :----- | :------ | :----- | :----- |
| `--frame-rate` | não | uma taxa em fotogramas por segundo: `25`, `23.976`, `29.97` | 23,976 |

**Só serve para ficheiros contados em fotogramas**, isto é, MicroDVD, o
único dos nove. Um ficheiro MicroDVD não traz mais do que números de fotograma; a
taxa a que são lidos decide **todas as posições do relatório**,
a começar por `span`. Indicada noutros casos, não altera nada e não é um
erro.

Um valor que não designa nada é um erro de utilização, portanto código `1`:

<!-- example: subedit-cli inspect --frame-rate douze film.sub; echo $? -->
```console
$ subedit-cli inspect --frame-rate douze film.sub; echo $?
--frame-rate: "douze" is not a frame rate: expected frames per second, like 25 or 23.976
1
```

É a mesma opção que em [`convert`](convert.md), palavra por palavra, porque
é a mesma taxa. O `convert` usa-a também para **escrever**.

## Uma tradução: `-t`

| Opção | Obrigatória | Valor | Predefinição |
| :----- | :------ | :----- | :----- |
| `-t`, `--translation-file` | não | um ficheiro de tradução, para **um único** ficheiro de entrada | — |
| `--align-method` | não, e só com `-t` | `position` ou `number` | `position` |

Com `-t`, a tradução é colocada sobre o ficheiro **como a janela faz ao abri-lo** — ver
[Uma tradução](invocation.md#uma-tradução) — e o relatório diz **como as suas linhas encontraram a
respetiva legenda**. Um script sabe assim se uma tradução está emparelhada antes de a corrigir.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour.\n\n' > film.fr.srt; subedit-cli --quiet inspect -t film.fr.srt film.srt | tail -3 -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour.\n\n' > film.fr.srt; subedit-cli --quiet inspect -t film.fr.srt film.srt | tail -3
  anomalies: none
  translation file: film.fr.srt, matched by position
  translation: 1 line attached; 1 subtitle left without a translation
```

A primeira linha acrescentada nomeia o ficheiro e o método; a segunda é **a frase que a janela
diz** quando abre uma tradução, cada parte só quando não é zero:

| Parte | O que conta |
| :----- | :---------------- |
| `N lines attached` | as linhas da tradução que encontraram uma legenda do ficheiro principal |
| `N subtitles born of a line` | as linhas que não encontraram nenhuma e deram origem à sua própria legenda, nas suas próprias posições |
| `N subtitles left without a translation` | as legendas do ficheiro principal a que nenhuma linha chegou |
| `N lines out of order` | as linhas que vieram depois de uma posterior no ficheiro |

Uma tradução que não traz nenhuma linha diz `translation: the file holds no line`. **O relatório do
próprio ficheiro continua a ser o do ficheiro sozinho**: uma legenda nascida de uma linha não é contada em
`subtitles`.

Os erros de `-t` são os de uma leitura, ditos com o caminho da tradução, e este:
`<translation>: the file is already open as the main document`, quando o ficheiro indicado é o principal.
`--align-method` sem `-t` é um erro de utilização, e o mesmo acontece com `-t` e várias entradas — ver
[Uma tradução](invocation.md#uma-tradução).

## Saída

O relatório vai para a **saída padrão** — é o resultado do comando, e
é escrito em todos os níveis, `--quiet` incluído. Um bloco por ficheiro, precedido
do caminho:

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

| Campo | O que diz |
| :---- | :----------- |
| `format` | um dos formatos que `--to` aceita, escrito por extenso: `SubRip`, `WebVTT`, `SubViewer 2`, `Sub Station Alpha`, `Advanced SSA`, `MicroDVD`, `MPL2`, `TMPlayer`, `LRC` |
| `encoding` | a codificação lida, seguida de **a origem da resposta**: `, from its byte order mark` se o ficheiro a declara, `, as asked for` se `--encoding` a impôs, `, detected` se foi avaliada |
| `byte order mark` | `present` ou `absent` |
| `line endings` | `LF`, `CRLF` ou `CR`, seguido de `, mixed from line N` se o ficheiro os mistura |
| `subtitles` | o número de legendas lidas |
| `span` | do início mais cedo ao fim mais tardio, `HH:MM:SS.mmm` |
| `frame rate grid` | a taxa de fotogramas sobre a qual as posições foram calculadas, **deduzida** — ver abaixo |
| `frame rate` | para um ficheiro contado em fotogramas, a taxa a que foi lido e **de onde vem** — substitui a linha anterior, ver abaixo |
| `anomalies` | `none` na mesma linha, ou o que está errado: **um tipo por linha com a respetiva contagem**, com avanço sob `anomalies:`; com `--detail`, **uma anomalia por linha**, legenda a legenda |

**`span` não é “do primeiro ao último”** mas do mais cedo ao mais tardio:
num ficheiro cuja ordem está quebrada, os dois diferem, e só o segundo diz
a verdade sobre o que o ficheiro cobre.

**`anomalies`, com `--detail`, conta as legendas a partir de 1**, como o ficheiro
as numera — e não as linhas do ficheiro. Um número de legenda sobrevive a uma
alteração; um número de linha não. Ver abaixo.

## O que `frame rate grid` comunica

**Um ficheiro de legendas não declara a sua taxa de fotogramas.** O SubRip não tem
cabeçalho, o do WebVTT é texto livre. Esta linha não lê, portanto, nada:
**deduz**, medindo se as posições caem numa grelha de fotogramas.

O que se mede são os **inícios**. Onde quer que exista uma grelha, estão nela
sem exceção, ao passo que os fins se afastam muitas vezes dela — um fim é
frequentemente calculado por uma regra de velocidade de leitura em vez de ser posto
num fotograma.

A primeira linha diz sempre a mesma coisa: a taxa escolhida, o veredicto,
e a concentração que o sustenta.

| Veredicto | O que significa |
| :------ | :----------------- |
| `clean` | as posições estão numa grelha |
| `partial` | parte delas está, o resto não |
| `none` | nenhum candidato explica seja o que for |

`none` tem **duas causas, e não são a mesma resposta**: ou nenhuma
grelha se ajusta — a linha dá então o melhor candidato e a sua pontuação,
para que se veja por quanto falha — ou o ficheiro tem poucas
legendas para que se possa dizer algo. Dois inícios parecem sempre
perfeitamente alinhados; isso não prova nada.

**Um ficheiro regular numa taxa não padronizada dá `none`, e é
intencional.** O conjunto de candidatos é fechado — as oito taxas que a
conversão propõe — e comunicar a menos errada delas seria dar
uma resposta errada onde “não sei” é a resposta certa.

### As linhas seguintes, quando têm algo a dizer

| Linha | Quando aparece |
| :---- | :------------------ |
| `grid offset` | as posições estão na grelha **a menos de uma constante**: o ficheiro foi deslocado, e aqui está de quanto |
| `also fits` | outro candidato se ajusta igualmente bem, por ser um múltiplo inteiro do escolhido |
| `too short a span to separate` | a duração do ficheiro não permite distinguir a taxa escolhida das indicadas |
| `off the grid` | quantos inícios se afastam da grelha, e em quantas **sequências** |

**`also fits` não é uma hesitação.** Uma grelha a 25 fotogramas por segundo está
*incluída* numa grelha a 50: um ficheiro sincronizado a 25 ajusta-se portanto às duas.
O inverso é falso — um verdadeiro 50 desmorona-se sobre 25 — pelo que é o mais baixo que
é escolhido, e o outro é nomeado em vez de ser calado. Os únicos
pares em causa são 25 e 50, 29,97 e 59,94, 30 e 60.

**`too short a span to separate` protege contra uma resposta demasiado confiante.** 23,976
e 24 afastam-se um milissegundo por segundo de filme: num filme de longa-metragem separam-se
sem dúvida, ao longo de dez segundos não se distinguem. A linha diz
qual continua a ser possível.

**`off the grid` distingue duas histórias que a percentagem sozinha confunde.**
Muitas sequências de um único início são posições corrigidas à mão,
uma a uma. Poucas sequências longas são uma secção ressincronizada ou um ficheiro
montado a partir de dois outros. O número de sequências é o que as separa.

### Um exemplo

Doze linhas postas numa grelha a 25 fotogramas por segundo — portanto em
posições que são múltiplos de 40 milissegundos — repartidas por quase nove minutos:

<!-- example: k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > grille.srt; subedit-cli --quiet inspect grille.srt | tail -3 -->
```console
$ k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > grille.srt; subedit-cli --quiet inspect grille.srt | tail -3
  frame rate grid: 25 fps, clean (100.0%)
  also fits: 50 fps, of which this rate is a whole divisor
  anomalies: none
```

O exemplo cria o seu próprio ficheiro em vez de ler um do repositório, e é
deliberado: pode ser repetido tal como está, e o ciclo mostra o que é uma grelha
melhor do que uma frase — posições que são múltiplos da duração de um fotograma.

## O que `frame rate` comunica, e porque não é uma grelha

**Só um dos nove formatos conta em fotogramas**: MicroDVD, cada linha do qual
traz dois números de fotograma e mais nada. Num tal ficheiro, o `inspect`
não escreve `frame rate grid` mas `frame rate`, e as duas linhas nunca
são mostradas em conjunto.

**A razão cabe numa frase.** As posições de um ficheiro MicroDVD foram
calculadas *a partir* dos seus fotogramas, a uma dada taxa; deduzi-las de uma
grelha devolveria essa mesma taxa, que acaba de ser fornecida. Seria
um número dado, disfarçado de medição.

O que se escreve em vez disso é **a taxa de leitura e de onde vem** —
a mesma forma que a linha `encoding`:

| O que a linha diz | Quando |
| :------------------ | :---- |
| `frame rate: 25 fps, as asked for` | `--frame-rate` impô-la |
| `frame rate: 24000/1001 fps, assumed` | ninguém a indicou; é a predefinida |

**Duas respostas e não três**, onde `encoding` tem três: um ficheiro MicroDVD
não declara nada. É esse todo o problema — a taxa vem necessariamente
de fora, e o relatório diz de qual dos dois exteriores.

**A taxa decide todo o resto do relatório.** `span` passa por ela, e duas
leituras do mesmo ficheiro a duas taxas dão dois intervalos:

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

## O que `anomalies` comunica

Três coisas podem estar erradas num documento, e cada uma repara-se de maneira diferente.
**Por predefinição, o `inspect` conta-as**, uma linha por tipo, pela ordem da tabela:
um ficheiro mal feito traz centenas delas, e saber quantas de cada tipo
é o que diz o que executar. **`--detail` nomeia-as uma a uma.**

| O que se escreve | Com `--detail` | O que significa |
| :--------------- | :-------------- | :------------------ |
| `subtitle ends before it starts: N` | `subtitle N ends before it starts` | o fim vem antes do início |
| `subtitle starts before the previous one ends: N` | `subtitle N starts before the previous one ends` | sobrepõe-se à anterior |
| `subtitle starts before the previous one starts: N` | `subtitle N starts before the previous one starts` | quebra a ordem do ficheiro — o [`sort`](sort.md) repõe-a |

Um tipo ausente não é escrito. Na coluna da esquerda, `N` é o número
de legendas; na seguinte, o número de uma legenda.

**A mesma legenda pode aparecer duas vezes**, e é intencional: a que
começa antes de a anterior ter começado também começa antes de esta ter
terminado. O primeiro ponto resolve-se deslocando a legenda, o segundo
alterando uma duração.

**A legenda nomeada é a que está fora do lugar**, não aquela em relação à qual
está fora do lugar. Em inícios a `0`, `4 s`, `2 s`, `3 s`: a terceira quebra a ordem, a
quarta segue de facto a terceira e por isso não é nomeada — não há nada a
fazer quanto a ela.

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

**Uma linha por anomalia com `--detail`**, porque uma centena de constatações
coladas numa linha não se podem nem ler nem pesquisar. Um documento sem defeito mantém
a sua única linha, `anomalies: none`, que um script procura. **O
registo de `--format json` lista sempre cada anomalia**: um script lê o pormenor, e
`--detail` só altera o texto.

**Não é um diagnóstico de leitura.** Os diagnósticos dizem o que a
leitura encontrou e apontam para uma **linha do ficheiro**; as anomalias dizem
o que o documento *é* e apontam para uma **legenda**. Um ficheiro cuja ordem
está quebrada não está por isso malformado: a leitura nada tem a dizer sobre isso.

## Em JSON

Com [`--format json`](invocation.md#uma-saída-que-um-script-pode-ler), o `inspect` escreve **um
objeto por ficheiro** em vez dos blocos acima; a narração fica na saída de erros.
As chaves descrevem o que o texto diz, e **nenhuma deve ser relida a partir de uma frase**:

| Chave | Conteúdo |
| :-- | :------ |
| `format` | o identificador do formato, o que `convert --to` aceita: `srt`, `vtt`… |
| `encoding` | `{"charset", "origin", "byte_order_mark"}` — `origin` é `byte-order-mark`, `asked` ou `detected` |
| `line_endings` | `{"kind", "mixed_from_line"}` — `lf`, `crlf` ou `cr`; a linha onde surge um segundo tipo, ou `null` |
| `subtitles` | o número de legendas |
| `span_ms` | `{"start", "end"}` — o primeiro início e o último fim, em milissegundos |
| `frame_rate` | para um ficheiro contado em fotogramas: `{"rate", "origin"}` (`asked` ou `assumed`); caso contrário `null` |
| `grid` | para os outros: a grelha deduzida, ou `null` para um ficheiro contado em fotogramas |
| `anomalies` | uma matriz de `{"subtitle", "kind"}` — `end-before-start`, `overlapping-subtitles`, `out-of-order` |
| `translation` | **só com `-t`**: `{"file", "method", "attached", "born", "untranslated", "out_of_order"}` — `method` é `position` ou `number`, e as quatro contagens são as da frase acima |
| `warnings` | o que a leitura encontrou, em todos os níveis |

`grid` traz `verdict` (`clean`, `partial` ou `silent`), `enough_starts`, `rate` (uma cadeia,
ou `null` quando nada é nomeado), `concentration_permille` (milésimos, `null` sem inícios
suficientes), `offset_ms`, `also_fits` (a taxa de que esta é um divisor inteiro, ou
`null`), `not_separated` (as taxas que uma duração demasiado curta não separa), `strays`
(os inícios fora da grelha) e `starts`.

<!-- example: printf '{25}{50}Un.\n{75}{100}Deux.\n' > f.sub; subedit-cli --format json inspect --frame-rate 25 f.sub 2>/dev/null -->
```console
$ printf '{25}{50}Un.\n{75}{100}Deux.\n' > f.sub; subedit-cli --format json inspect --frame-rate 25 f.sub 2>/dev/null
{"schema":1,"command":"inspect","file":"f.sub","ok":true,"format":"microdvd","encoding":{"charset":"UTF-8","origin":"detected","byte_order_mark":false},"line_endings":{"kind":"lf","mixed_from_line":null},"subtitles":2,"span_ms":{"start":1000,"end":4000},"frame_rate":{"rate":"25","origin":"asked"},"grid":null,"anomalies":[],"warnings":[]}
```

## Narração

Na saída de erros, consoante o nível pedido:

| Nível | O que se acrescenta |
| :----- | :-------------- |
| 1 | `<path>: N subtitles` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings` |
| 3 | `<path>: N bytes read`, `<path>: N diagnostic(s) while reading`, e depois **cada um deles** — ver [Invocação](invocation.md#ler-os-diagnósticos) |

## Códigos de saída

Os da ferramenta: `0` se todos os ficheiros foram lidos, `2` se nenhum, `3` se
apenas alguns, `1` perante um erro de utilização.

## Erros

Os de uma leitura — do ficheiro, e da tradução quando `-t` é indicado —, e
os de `-t`: ver acima. O `inspect` não escreve nada que possa ser recusado.

| O que os desencadeia | Mensagem |
| :------------------ | :------ |
| nenhum caminho indicado | `files is required`, seguido de uma indicação de `--help` |
| ficheiro em falta | `<path>: does not exist` |
| ficheiro ilegível | `<path>: cannot be opened: permission denied` |
| bytes que não se descodificam | `<path>: cannot be decoded in the chosen encoding` |
| formato não reconhecido | `<path>: is in no format this tool knows` |
| nada que se pareça com uma legenda | `<path>: holds nothing recognisable as a subtitle` |

São as mesmas mensagens para os dezassete subcomandos: a receita de abertura
está escrita uma só vez, no núcleo.

## Um exemplo que falha

Um ficheiro vazio não corresponde a nenhum formato. Não é comunicado como vazio:
é recusado, e nomeado.

<!-- example: : > vide.srt; subedit-cli inspect vide.srt; echo "code=$?" -->
```console
$ : > vide.srt; subedit-cli inspect vide.srt; echo "code=$?"
vide.srt: is in no format this tool knows
code=2
```
