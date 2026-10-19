# `inspect`

<!-- machine-translated -->
> Esta páxina é unha tradución automática e aínda non a revisou ningún lector da lingua.

```
subedit-cli inspect [--frame-rate RATE] [--recursive] [--detail]
                   [-t FILE] [--align-method position|number] <file>...
```

Informa de que está feito cada ficheiro. **Non modifica nada e non escribe ningún ficheiro.**

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

| Argumento | Obrigatorio | Valor | Predeterminado |
| :------- | :----- | :----- | :----- |
| `<file>...` | si | un ou varios camiños de ficheiros de subtítulos | — |
| `--recursive`, `-r` | non | unha marca | desactivada — véxase [Procesar unha árbore](lots.md) |
| `--detail` | non | unha marca: nomea cada anomalía, en lugar de contalas — véxase [O que indica `anomalies`](#o-que-indica-anomalies) | desactivada |

Non indicar ningún camiño é un erro de uso, de aí o código `1`.

## Opcións

| Opción | Obrigatorio | Valor | Predeterminado |
| :----- | :------ | :----- | :----- |
| `--frame-rate` | non | unha taxa en fotogramas por segundo: `25`, `23.976`, `29.97` | 23,976 |

**Só serve para os ficheiros contados en fotogramas**, é dicir MicroDVD, o único dos nove. Un
ficheiro MicroDVD non leva máis que números de fotograma; a taxa á que se len decide **cada
posición do informe**, comezando por `span`. Indicada noutro caso, non cambia nada e non é un
erro.

Un valor que non designa nada é un erro de uso, de aí o código `1`:

<!-- example: subedit-cli inspect --frame-rate douze film.sub; echo $? -->
```console
$ subedit-cli inspect --frame-rate douze film.sub; echo $?
--frame-rate: "douze" is not a frame rate: expected frames per second, like 25 or 23.976
1
```

É a mesma opción que en [`convert`](convert.md), palabra por palabra, porque é a mesma taxa.
`convert` úsaa tamén para **escribir**.

## Unha tradución: `-t`

| Opción | Obrigatorio | Valor | Predeterminado |
| :----- | :------ | :----- | :----- |
| `-t`, `--translation-file` | non | un ficheiro de tradución, para **unha soa** entrada | — |
| `--align-method` | non, e só con `-t` | `position` ou `number` | `position` |

Con `-t`, a tradución colócase sobre o ficheiro **como fai a xanela ao abrila** — véxase
[Unha tradución](invocation.md#unha-tradución) — e o informe di **como atoparon as súas liñas o
seu subtítulo**. Así, un script sabe se unha tradución está aliñada antes de corrixila.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour.\n\n' > film.fr.srt; subedit-cli --quiet inspect -t film.fr.srt film.srt | tail -3 -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour.\n\n' > film.fr.srt; subedit-cli --quiet inspect -t film.fr.srt film.srt | tail -3
  anomalies: none
  translation file: film.fr.srt, matched by position
  translation: 1 line attached; 1 subtitle left without a translation
```

A primeira liña engadida nomea o ficheiro e o método; a segunda é **a frase que di a xanela**
cando abre unha tradución, cada parte só cando non é cero:

| Parte | O que conta |
| :----- | :---------------- |
| `N lines attached` | as liñas da tradución que atoparon un subtítulo do ficheiro principal |
| `N subtitles born of a line` | as liñas que non atoparon ningún e deron orixe ao seu propio, nas súas propias posicións |
| `N subtitles left without a translation` | os subtítulos do ficheiro principal aos que non chegou ningunha liña |
| `N lines out of order` | as liñas que viñan despois dunha posterior no ficheiro |

Unha tradución que non leva ningunha liña di `translation: the file holds no line`. **O informe
do propio ficheiro segue sendo o do ficheiro só**: un subtítulo nacido dunha liña non se conta en
`subtitles`.

Os erros de `-t` son os dunha lectura, ditos co camiño da tradución, e este:
`<translation>: the file is already open as the main document`, cando o ficheiro indicado é o principal.
`--align-method` sen `-t` é un erro de uso, e tamén o é `-t` con varias entradas — véxase
[Unha tradución](invocation.md#unha-tradución).

## Saída

O informe vai á **saída estándar** — é o resultado da orde, e escríbese en todos os niveis,
`--quiet` incluído. Un bloque por ficheiro, precedido polo camiño:

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

| Campo | O que di |
| :---- | :----------- |
| `format` | un dos formatos que acepta `--to`, escrito por extenso: `SubRip`, `WebVTT`, `SubViewer 2`, `Sub Station Alpha`, `Advanced SSA`, `MicroDVD`, `MPL2`, `TMPlayer`, `LRC` |
| `encoding` | a codificación lida, seguida de **onde vén a resposta**: `, from its byte order mark` se o ficheiro a declara, `, as asked for` se a impuxo `--encoding`, `, detected` se se sopesou |
| `byte order mark` | `present` ou `absent` |
| `line endings` | `LF`, `CRLF` ou `CR`, seguido de `, mixed from line N` se o ficheiro os mestura |
| `subtitles` | o número de subtítulos lidos |
| `span` | desde o inicio máis temperán ata o final máis tardío, `HH:MM:SS.mmm` |
| `frame rate grid` | a taxa de fotogramas sobre a que se calcularon as posicións, **deducida** — véxase máis abaixo |
| `frame rate` | para un ficheiro contado en fotogramas, a taxa á que se leu e **de onde vén** — substitúe a liña anterior, véxase máis abaixo |
| `anomalies` | `none` na mesma liña, ou o que está mal: **un tipo por liña co seu reconto**, sangrado baixo `anomalies:`; con `--detail`, **unha anomalía por liña**, subtítulo a subtítulo |

**`span` non é «do primeiro ao último»** senón do máis temperán ao máis tardío: nun ficheiro cuxa
orde está rota, os dous difiren, e só o segundo di a verdade sobre o que cobre o ficheiro.

**`anomalies`, con `--detail`, conta os subtítulos desde 1**, como os numera o ficheiro — e non
as liñas do ficheiro. Un número de subtítulo sobrevive a un cambio; un número de liña non.
Véxase máis abaixo.

## O que indica `frame rate grid`

**Un ficheiro de subtítulos non declara a súa taxa de fotogramas.** SubRip non ten cabeceira, a
de WebVTT é texto libre. Esta liña, polo tanto, non le nada: **deduce**, medindo se as posicións
caen nunha grade de fotogramas.

O que se mide son os **inicios**. Alí onde existe unha grade están nela sen excepción, mentres que
os finais se afastan dela a miúdo — un final calcúlase con frecuencia mediante unha regra de
velocidade de lectura en lugar de colocarse nun fotograma.

A primeira liña di sempre o mesmo: a taxa escollida, o veredicto e a concentración que o sostén.

| Veredicto | O que significa |
| :------ | :----------------- |
| `clean` | as posicións están nunha grade |
| `partial` | unha parte delas está, o resto non |
| `none` | ningún candidato explica nada |

`none` ten **dúas causas, e non son a mesma resposta**: ou ningunha grade encaixa — a liña dá
entón o mellor candidato e a súa puntuación, para que se vexa por canto falla — ou o ficheiro ten
demasiado poucos subtítulos para poder dicir nada del. Dous inicios parecen sempre perfectamente
aliñados; iso non proba nada.

**Un ficheiro regular nunha taxa non estándar dá `none`, e é intencionado.** O conxunto de
candidatos é pechado — as oito taxas que ofrece a conversión — e indicar a menos errónea delas
sería dar unha resposta errónea alí onde «non o sei» é a correcta.

### As liñas seguintes, cando teñen algo que dicir

| Liña | Cando aparece |
| :---- | :------------------ |
| `grid offset` | as posicións están na grade **salvo unha constante**: o ficheiro desprazouse, e aquí está canto |
| `also fits` | outro candidato encaixa igual de ben, porque é un múltiplo enteiro do escollido |
| `too short a span to separate` | a extensión do ficheiro non permite distinguir a taxa escollida das nomeadas |
| `off the grid` | cantos inicios se afastan da grade, e en cantas **series** |

**`also fits` non é unha dúbida.** Unha grade a 25 fotogramas por segundo está *incluída* nunha
grade a 50: un ficheiro temporizado a 25 encaixa, polo tanto, nas dúas. O contrario é falso — un
50 real non encaixa en 25 — así que se escolle a máis baixa, e a outra noméase en lugar de
calala. As únicas parellas afectadas son 25 e 50, 29,97 e 59,94, 30 e 60.

**`too short a span to separate` protexe contra unha resposta demasiado segura.** 23,976 e 24
afástanse un milisegundo por segundo de película: nunha longametraxe sepáranse sen dúbida, en dez
segundos non se poden distinguir. A liña di cal segue sendo posible.

**`off the grid` distingue dúas historias que a porcentaxe soa confunde.** Moitas series dun só
inicio son posicións corrixidas a man, unha a unha. Unhas poucas series longas son unha sección
con tempos recalculados ou un ficheiro montado a partir doutros dous. O número de series é o que
as separa.

### Un exemplo

Doce liñas colocadas nunha grade a 25 fotogramas por segundo — polo tanto en posicións múltiplos
de 40 milisegundos — repartidas en case nove minutos:

<!-- example: k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > grille.srt; subedit-cli --quiet inspect grille.srt | tail -3 -->
```console
$ k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > grille.srt; subedit-cli --quiet inspect grille.srt | tail -3
  frame rate grid: 25 fps, clean (100.0%)
  also fits: 50 fps, of which this rate is a whole divisor
  anomalies: none
```

O exemplo fabrica o seu propio ficheiro en lugar de ler un do repositorio, e é deliberado: pódese
reproducir tal como está, e o bucle mostra o que é unha grade mellor ca unha frase — posicións
que son múltiplos da duración dun fotograma.

## O que indica `frame rate` e por que non é unha grade

**Só un dos nove formatos conta en fotogramas**: MicroDVD, cada liña do cal leva dous números de
fotograma e nada máis. Nun ficheiro así, `inspect` non escribe `frame rate grid` senón
`frame rate`, e as dúas liñas nunca se mostran xuntas.

**O motivo cabe nunha frase.** As posicións dun ficheiro MicroDVD calculáronse *a partir* dos
seus fotogramas, a unha taxa dada; deducilas dunha grade devolvería esa mesma taxa, tal como se
acaba de proporcionar. Sería unha cifra dada, disfrazada de medición.

O que se escribe no seu lugar é **a taxa de lectura e de onde vén** — a mesma forma que a liña
`encoding`:

| O que di a liña | Cando |
| :------------------ | :---- |
| `frame rate: 25 fps, as asked for` | impúxoa `--frame-rate` |
| `frame rate: 24000/1001 fps, assumed` | ninguén a dixo; é o valor predeterminado |

**Dúas respostas e non tres**, alí onde `encoding` ten tres: un ficheiro MicroDVD non declara
nada. Ese é todo o problema — a taxa vén necesariamente de fóra, e o informe di cal das dúas
procedencias externas.

**A taxa decide todo o resto do informe.** `span` pasa por ela, e dúas lecturas do mesmo ficheiro
a dúas taxas dan dúas extensións:

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

## O que indica `anomalies`

Tres cousas poden estar mal nun documento, e cada unha repárase dun xeito distinto. **De maneira
predeterminada, `inspect` cóntaas**, unha liña por tipo, na orde da táboa: un ficheiro mal feito
leva centos delas, e saber cantas hai de cada tipo é o que di que hai que executar.
**`--detail` nomea cada unha.**

| O que se escribe | Con `--detail` | O que significa |
| :--------------- | :-------------- | :------------------ |
| `subtitle ends before it starts: N` | `subtitle N ends before it starts` | o final vén antes do inicio |
| `subtitle starts before the previous one ends: N` | `subtitle N starts before the previous one ends` | sólapase co anterior |
| `subtitle starts before the previous one starts: N` | `subtitle N starts before the previous one starts` | rompe a orde do ficheiro — [`sort`](sort.md) restabléceo |

Un tipo ausente non se escribe. Na columna da esquerda, `N` é o número de subtítulos; na
seguinte, o número dun subtítulo.

**O mesmo subtítulo pode aparecer dúas veces**, e é intencionado: o que comeza antes de que
comece o anterior tamén comeza antes de que este remate. O primeiro punto resólvese movendo o
subtítulo, o segundo cambiando unha duración.

**O subtítulo nomeado é o que está mal colocado**, non aquel respecto do cal o está. Con inicios
en `0`, `4 s`, `2 s`, `3 s`: o terceiro rompe a orde, o cuarto si segue ao terceiro e polo tanto
non se nomea — non hai nada que facer con el.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect desordre.srt | tail -3 -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect desordre.srt | tail -3
  anomalies:
    subtitle starts before the previous one ends: 1
    subtitle starts before the previous one starts: 1
```

E con `--detail`, o subtítulo nomeado:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect --detail desordre.srt | tail -3 -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect --detail desordre.srt | tail -3
  anomalies:
    subtitle 3 starts before the previous one ends
    subtitle 3 starts before the previous one starts
```

**Unha liña por anomalía con `--detail`**, porque cen achados pegados nunha liña non se poden ler
nin buscar. Un documento sen defectos conserva a súa única liña, `anomalies: none`, que busca un
script. **O rexistro `--format json` lista sempre cada anomalía**: un script le o detalle, e
`--detail` só cambia o texto.

**Non é un diagnóstico de lectura.** Os diagnósticos din o que atopou a lectura e sinalan unha
**liña do ficheiro**; as anomalías din o que *é* o documento e sinalan un **subtítulo**. Un
ficheiro cuxa orde está rota non está mal formado por iso: a lectura non ten nada que dicir del.

## En JSON

Con [`--format json`](invocation.md#saída-que-pode-ler-un-script), `inspect` escribe **un obxecto
por ficheiro** en lugar dos bloques de arriba; a narración segue na saída de erro. As claves
describen o que di o texto, e **ningunha se ten que extraer dunha frase**:

| Clave | Contido |
| :-- | :------ |
| `format` | o identificador do formato, o que acepta `convert --to`: `srt`, `vtt`… |
| `encoding` | `{"charset", "origin", "byte_order_mark"}` — `origin` é `byte-order-mark`, `asked` ou `detected` |
| `line_endings` | `{"kind", "mixed_from_line"}` — `lf`, `crlf` ou `cr`; a liña onde aparece un segundo tipo, ou `null` |
| `subtitles` | o número de subtítulos |
| `span_ms` | `{"start", "end"}` — o primeiro inicio e o último final, en milisegundos |
| `frame_rate` | para un ficheiro contado en fotogramas: `{"rate", "origin"}` (`asked` ou `assumed`); se non, `null` |
| `grid` | para os demais: a grade deducida, ou `null` para un ficheiro contado en fotogramas |
| `anomalies` | unha matriz de `{"subtitle", "kind"}` — `end-before-start`, `overlapping-subtitles`, `out-of-order` |
| `translation` | **só con `-t`**: `{"file", "method", "attached", "born", "untranslated", "out_of_order"}` — `method` é `position` ou `number`, e os catro recontos son os da frase de arriba |
| `warnings` | o que atopou a lectura, en todos os niveis |

`grid` leva `verdict` (`clean`, `partial` ou `silent`), `enough_starts`, `rate` (unha cadea, ou
`null` cando non se nomea nada), `concentration_permille` (milésimas, `null` sen inicios
suficientes), `offset_ms`, `also_fits` (a taxa da que esta é un divisor enteiro, ou `null`),
`not_separated` (as taxas que unha extensión demasiado curta non separa), `strays` (os inicios
fóra da grade) e `starts`.

<!-- example: printf '{25}{50}Un.\n{75}{100}Deux.\n' > f.sub; subedit-cli --format json inspect --frame-rate 25 f.sub 2>/dev/null -->
```console
$ printf '{25}{50}Un.\n{75}{100}Deux.\n' > f.sub; subedit-cli --format json inspect --frame-rate 25 f.sub 2>/dev/null
{"schema":1,"command":"inspect","file":"f.sub","ok":true,"format":"microdvd","encoding":{"charset":"UTF-8","origin":"detected","byte_order_mark":false},"line_endings":{"kind":"lf","mixed_from_line":null},"subtitles":2,"span_ms":{"start":1000,"end":4000},"frame_rate":{"rate":"25","origin":"asked"},"grid":null,"anomalies":[],"warnings":[]}
```

## Narración

Na saída de erro, segundo o nivel pedido:

| Nivel | O que se engade |
| :----- | :-------------- |
| 1 | `<path>: N subtitles` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings` |
| 3 | `<path>: N bytes read`, `<path>: N diagnostic(s) while reading`, e despois **cada un deles** — véxase [Invocación](invocation.md#diagnósticos-de-lectura) |

## Códigos de saída

Os da ferramenta: `0` se se leron todos os ficheiros, `2` se non se leu ningún, `3` se só se
leron algúns, `1` nun erro de uso.

## Erros

Os dunha lectura — do ficheiro, e da tradución cando se indica `-t` —, e os de `-t`: véxase
máis arriba. `inspect` non escribe nada que se poida rexeitar.

| O que o provoca | Mensaxe |
| :------------------ | :------ |
| ningún camiño indicado | `files is required`, seguido dunha indicación cara a `--help` |
| ficheiro ausente | `<path>: does not exist` |
| ficheiro ilexible | `<path>: cannot be opened: permission denied` |
| bytes que non se descodifican | `<path>: cannot be decoded in the chosen encoding` |
| formato non recoñecido | `<path>: is in no format this tool knows` |
| nada que se pareza a un subtítulo | `<path>: holds nothing recognisable as a subtitle` |

Son as mesmas mensaxes para as dezasete subordes: a receita para abrir escríbese unha soa vez,
no núcleo.

## Un exemplo que falla

Un ficheiro baleiro non corresponde a ningún formato. Non se indica como baleiro: rexéitase, e
noméase.

<!-- example: : > vide.srt; subedit-cli inspect vide.srt; echo "code=$?" -->
```console
$ : > vide.srt; subedit-cli inspect vide.srt; echo "code=$?"
vide.srt: is in no format this tool knows
code=2
```
