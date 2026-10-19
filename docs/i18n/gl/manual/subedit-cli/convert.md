# `convert`

<!-- machine-translated -->
> Esta páxina é unha tradución automática e aínda non a revisou ningún lector da lingua.

```
subedit-cli convert --to <format>
                    [--line-endings unix|windows|mac] [--to-encoding NAME]
                    [--bom | --no-bom] [--sort] [--disjoint]
                    [--join-repeats] [--repair]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    <file>...
```

Escribe cada ficheiro no formato e na forma pedidos. **Nunca modifica a entrada**, salvo que
`--in-place` o pida explicitamente.

<!-- example: subedit-cli convert --help -->
```console
$ subedit-cli convert --help
Write a subtitle file out in another format or shape
Usage: subedit-cli convert [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to convert

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --to TEXT:{srt,vtt,subviewer2,ssa,ass,microdvd,mpl2,tmplayer,lrc} REQUIRED
                              Format to write
  --line-endings TEXT:{unix,windows,mac}
                              Line endings to write; the source's by default
  --to-encoding NAME          Encoding to write; the source's by default
  --frame-rate RATE           Frame rate of a file counted in frames: 25, 23.976
  --bom                       Write a byte order mark
  --no-bom                    Write no byte order mark
  --sort                      Put the subtitles in order of their start before writing
  --disjoint                  Cut overlapping subtitles so that no range overlaps another, before writing
  --join-repeats              Join the subtitles that repeat each other into one, before writing
  --repair                    Join the repeats, sort, then make the ranges disjoint, before writing
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Argumentos e opcións

| Opción | Obrigatorio | Valores | Predeterminado |
| :----- | :----- | :------ | :----- |
| `<file>...` | si | un ou varios camiños | — |
| `--recursive`, `-r` | non | unha marca | desactivada — véxase [Procesar unha árbore](lots.md) |
| `--to` | **si** | un nome da táboa de máis abaixo, e nada máis | — |
| `--line-endings` | non | `unix`, `windows` ou `mac` | os do ficheiro lido |
| `--to-encoding` | non | calquera codificación que ICU poida escribir, agás as que escriben a súa propia marca | a do ficheiro lido |
| `--bom` / `--no-bom` | non | marcas, excluíntes entre si | o que levaba o ficheiro lido |
| `--sort` | non | unha marca: pon os subtítulos na orde do seu inicio **antes de escribir**, véxase [Ordenar ao escribir](#ordenar-ao-escribir) | desactivada |
| `--join-repeats` | non | unha marca: fai un único subtítulo dos anacos dun mesmo texto que se suceden, **antes de calquera outra cousa**, véxase [Facer as franxas disxuntas](#facer-as-franxas-disxuntas) | desactivada |
| `--repair` | non | unha marca: o mesmo que `--join-repeats --sort --disjoint`, nesa orde | desactivada |
| `--disjoint` | non | unha marca: corta os subtítulos que se solapan **antes de escribir**, véxase [Facer as franxas disxuntas](#facer-as-franxas-disxuntas) | desactivada |
| `--output` / `--output-dir` / `--in-place` | **unha das tres**, agás con `--dry-run` | véxase [Invocación](invocation.md#o-destino) | — |
| `--dry-run` | non | unha marca: calcula e di, non escribe nada — véxase [Ver antes de escribir](invocation.md#ver-antes-de-escribir) | desactivada |

`mac` designa o retorno de carro só (`\r`), o final de liña do Mac OS clásico.

## Os formatos que acepta `--to`

| Nome | Formato | Extensión escrita |
| :-- | :----- | :--------------- |
| `srt` | SubRip | `.srt` |
| `vtt` | WebVTT | `.vtt` |
| `subviewer2` | SubViewer 2.0 | `.sub` |
| `ssa` | Sub Station Alpha | `.ssa` |
| `ass` | Advanced SSA | `.ass` |
| `mpl2` | MPL2 | `.txt` |
| `microdvd` | MicroDVD | `.sub` |
| `tmplayer` | TMPlayer | `.txt` |
| `lrc` | LRC | `.lrc` |

**Un nome, e non unha extensión.** Dúas extensións designan dous formatos cada unha — `.sub` é
tamén a de MicroDVD, `.txt` a de MPL2 e a de TMPlayer —, así que unha opción que tomase
extensións non sabería cal se pide. Os nomes de arriba non son ambiguos, e coinciden coa
extensión alí onde a extensión tampouco o é.

**A lista é pechada.** É a de Gaupol, e os nove están nela.

## Os dous formatos que non levan final

**TMPlayer e LRC só escriben unha posición por liña**, o seu inicio. Isto nótase nos dous
sentidos.

**Na lectura**, os finais dedúcense: o dunha liña é o inicio da seguinte, e a última recibe
cinco segundos. É o que fai Gaupol. Tamén o dicimos, mediante un diagnóstico que `--verbose`
mostra nas tres subordes:

```
file.lrc: carries no end times; each one was taken from the next start, settled by the reader
```

**Na escritura**, os finais non se escriben, por falta dun sitio onde poñelos. Converter a un
destes dous formatos pérdeos, polo tanto, e unha ida e volta non os recupera — volve deducilos.

**LRC perde unha cousa máis**: non ten maneira de levar un salto de liña. Unha réplica en dúas
liñas escríbese nunha soa, coas dúas liñas unidas por un espazo. Esa perda tampouco se pode
reparar.

Ningún dos dous ten vocabulario para a cursiva.

## `--frame-rate` e o único formato que conta en fotogramas

**MicroDVD non conta en segundos**: as súas posicións *son* números de fotograma, e ningún
ficheiro MicroDVD declara a taxa á que se contaron. `--frame-rate` indícaa, e serve nos dous
sentidos: é a taxa á que se le un `.sub` en fotogramas, e aquela á que unha conversión a
MicroDVD conta os fotogramas.

| Opción | Obrigatorio | Valores | Predeterminado |
| :----- | :----- | :------ | :----- |
| `--frame-rate` | non | unha taxa en fotogramas por segundo: `25`, `23.976`, `29.97` | véxase máis abaixo |

**Na lectura, sen `--frame-rate`, o ficheiro lese a 23,976** — o valor de Gaupol — e o nivel
`-vvv` dio: `counts in frames and states no rate; it was read at`. É o único lugar da ferramenta
onde cada posición mostrada descansa nunha suposición que o ficheiro non pode confirmar.

**Na escritura, sen `--frame-rate`, tres casos e tres respostas:**

| O ficheiro de partida | O que pasa |
| :------------------- | :-------------- |
| xa é MicroDVD | a taxa coa que se leu é a que o volve escribir, e os fotogramas volven idénticos |
| está baseado no tempo, e as súas posicións caen nunha grade | tómase a grade, e `-vv` di cal |
| está baseado no tempo, sen grade | **rexeitamento**, código 2: `writing frames needs a frame rate, and the positions fall on no grid to take one from — give --frame-rate` |

O rexeitamento é deliberado. O único xesto que quedaría sería inventar unha cifra, e cada liña
do ficheiro moveríase tanto como esta fose errónea.

## O que perde unha conversión

**Converter a outro formato nunca é sen perdas**, e o que non sobrevive depende da parella:

| O que non pasa | Por que |
| :--------------------- | :------- |
| a cabeceira | só ten sentido no seu propio formato: `[INFORMATION]` non é unha cabeceira WebVTT |
| os datos propios do formato | as coordenadas de SubRip, o identificador e os axustes dunha indicación WebVTT non teñen equivalente noutro lugar |
| a precisión | SubViewer 2, os dous Sub Station Alpha e LRC escriben á centésima, MPL2 á décima, TMPlayer ao segundo, MicroDVD ao fotograma; `01:00:00,017` volve como `01:00:00,020` |
| o final dunha liña | TMPlayer e LRC non teñen ningún campo para el; a lectura seguinte volve deducilo |
| os saltos de liña | só LRC: as súas liñas caben nunha liña, e as dúas volven unirse cun espazo |
| as etiquetas que o formato de destino non pode escribir | WebVTT non ten `<font color>`, TMPlayer e LRC non teñen nada |
| a disposición | `{\pos(x,y)}`, `{\an8}`, `<v Marie>`, `<ruby>`: sobreviven a unha ida e volta no seu propio formato, e desaparecen na conversión |

**Volver escribir un ficheiro no seu propio formato non perde nada** — esa é a garantía que
sostén todo o demais, e `--to srt` sobre un `.srt` devolve os mesmos bytes. Nin sequera se
descodifica nada: o texto segue sendo a cadea bruta que se leu.

## As etiquetas tradúcense, non se copian

**Cada formato escribe a cursiva á súa maneira**, e converter sen traducir deixaría un `{\i1}`
nun `.srt`: texto que o usuario ve, nun ficheiro que nunca saberá interpretalo. O que pasa son
**seis cousas** — negra, cursiva, subliñado, cor, tipo de letra e tamaño:

| Formato | Negra | Cursiva | Subliñado | Cor | Tipo de letra | Tamaño |
| :----- | :--: | :------: | :------: | :-----: | :----: | :----: |
| SubRip | `<b>` | `<i>` | `<u>` | `<font color="#RRGGBB">` | — | — |
| WebVTT | `<b>` | `<i>` | `<u>` | — | — | — |
| SubViewer 2 | `<b>` | `<i>` | `<u>` | `<font color="#RRGGBB">` | — | — |
| Sub Station Alpha | `{\b1}` | `{\i1}` | — | `{\c&HBBGGRR&}` | `{\fnNAME}` | `{\fsN}` |
| Advanced SSA | `{\b1}` | `{\i1}` | `{\u1}` | `{\c&HBBGGRR&}` | `{\fnNAME}` | `{\fsN}` |
| MicroDVD | `{Y:b}` | `{Y:i}` | `{Y:u}` | `{C:$BBGGRR}` | `{F:NAME}` | `{S:N}` |
| MPL2 | `\` | `/` | `_` | `{C:$BBGGRR}` | `{F:NAME}` | `{S:N}` |
| TMPlayer | — | — | — | — | — | — |
| LRC | — | — | — | — | — | — |

**Un trazo é unha perda anunciada.** Converter un `.srt` con cor a WebVTT elimina a cor e dio;
o ficheiro producido é WebVTT, non SubRip disfrazado.

**MicroDVD e MPL2 non poden dar estilo a unha parte dunha liña.** As súas etiquetas non teñen
final: `{y:i}` inclina todo o que segue ata o final da liña, e `{Y:i}` ata o final do subtítulo.
Unha liña que só estaba en negra á metade escríbese, polo tanto, sen negra ningunha, en lugar de
cunha negra que se desborda.

## O que a conversión di que perdeu

**Dio cando perde, e cala cando non perde nada.** Un informe que aparece en cada chamada é un
informe que ninguén le.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,500\n<i>sur deux</i>\nlignes\n\n' > a.srt; subedit-cli convert --to lrc --output b.lrc a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,500\n<i>sur deux</i>\nlignes\n\n' > a.srt; subedit-cli convert --to lrc --output b.lrc a.srt
a.srt: 1 subtitle written as LRC -> b.lrc
a.srt: ends are not carried by LRC, line breaks were joined in 1 subtitle, 1 tag dropped
```

A liña aparece no nivel de detalle predeterminado, e en todos os superiores. Os elementos, nesta
orde:

| Elemento | O que di |
| :---- | :----------- |
| `ends are not carried by <format>` | o formato de destino non escribe ningún final |
| `line breaks were joined in N subtitles` | só LRC: N subtítulos estaban en varias liñas |
| `N tags dropped` | non se puideron escribir N etiquetas, disposición incluída |
| `the header was dropped` | a cabeceira non cruza a fronteira dun formato |
| `the <format> fields of N subtitles were dropped` | N subtítulos levaban datos propios do seu formato |
| `positions moved by up to N ms` | a maior distancia na que se moveu unha posición |

## Cambiar a codificación

**`--to-encoding` escolle a codificación escrita; `--encoding` di a que se le.** As dúas xuntas
recodifican un ficheiro:

```console
$ subedit-cli convert --to srt --encoding cp1252 --to-encoding utf-8 \
      --output-dir sortie film.srt
```

Sen `--to-encoding`, o ficheiro escríbese **na codificación na que se leu**, igual que cos seus
finais de liña e coa súa marca: un ficheiro convertido sen instrución sobre a codificación
devolve os mesmos caracteres, nos mesmos bytes.

**Un carácter que a codificación de destino non pode escribir detén o ficheiro**, e non se
escribe nada. Un `ł` non ten sitio en Latin-1; substituílo por un `?` perdería texto entre a
lectura e a escritura, sen que ninguén o vise.

**Unha marca de orde de bytes só existe para as codificacións Unicode.** `--bom` cunha opción
`--to-encoding` que non a leva rexéitase en lugar de ignorarse — véxase a táboa de erros de máis
abaixo.

**E a codificación ten que nomear a súa orde de bytes.** `UTF-16` e `UTF-32` son nomes que ICU
coñece, cuxo conversor escribe **unha marca propia**, pídaselle o que se lle pida: `--no-bom`
aceptaríase entón e desobedeceríase. As dúas rexéitanse, polo tanto, e a mensaxe nomea a saída:

```console
$ subedit-cli convert --to srt --to-encoding UTF-16 --output copie.srt film.srt
"UTF-16" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do
```

`UTF-16LE`, `UTF-16BE`, `UTF-32LE` e `UTF-32BE` escriben exactamente os mesmos bytes, marca
incluída — pero baixo o control de `--bom` e `--no-bom`. O rexeitamento non custa, polo tanto,
máis que un nome máis preciso, e vale tanto para a lectura como para a escritura:
`--encoding UTF-16` recibe a mesma resposta.

## O destino

As tres opcións e as súas regras son comúns ás subordes que escriben: véxase
[Invocación](invocation.md#o-destino). Non se escribe nada sen unha delas.

O propio de `convert`: **`--output-dir` cambia a extensión co formato.** `convert --to vtt`
sobre `a.srt` escribe `a.vtt`, nunca un `a.srt` que conteña WebVTT — un ficheiro cuxo nome
minte fai tropezar a calquera outra ferramenta.

## Rexéitase a conversión no seu sitio

`--in-place` cun `--to` que cambia o formato dá o código `1` e non escribe nada: no seu sitio,
non hai un segundo nome que leve o novo formato, e o ficheiro quedaría baixo unha extensión que
o seu contido xa non xustifica.

`--in-place` pódese seguir usando para cambiar **a forma sen o formato** — volver escribir un
`.srt` como `.srt` con outros finais de liña, por exemplo.

O rexeitamento decídese **só pola extensión**, antes de calquera lectura: un erro de uso nunca
debe deixar un lote a medio escribir.

## Finais de liña e marca de orde de bytes

De maneira predeterminada, **o ficheiro escrito retoma o que levaba o ficheiro lido**. O modelo
conserva as dúas cousas na lectura; impoñelas de maneira predeterminada perdería, en cada
conversión, unha información conservada adrede.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli convert --to vtt --output b.vtt a.srt; cat b.vtt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli convert --to vtt --output b.vtt a.srt; cat b.vtt
a.srt: 1 subtitle written as WebVTT -> b.vtt
WEBVTT

00:01.000 --> 00:03.000
Bonjour.
```

## Ordenar ao escribir

`--sort` escribe o ficheiro **na orde dos inicios**, sexa cal sexa o formato de destino. A
ordenación faise **sobre unha copia**: o ficheiro lido non se toca, e o que se escribe é o que
[`sort`](sort.md) faría del — estable, dous subtítulos que comezan á vez conservan a súa orde.

**Nunca de maneira predeterminada**: a orde dun ficheiro é a do seu autor mentres non se pida
outra cousa. Coa opción, cada ficheiro dio, no primeiro nivel de narración, `<path>: N subtitles moved` ou
`<path>: already in order`, e o rexistro `--format json` leva un reconto `moved` — **ausente sen a opción**,
para non cambiar a forma do que len os scripts existentes.

## Facer as franxas disxuntas

`--disjoint` corta os subtítulos que se solapan para que ningunha franxa se solape xa con outra,
**sobre a mesma copia que a ordenación**: o ficheiro lido non se toca.

Nunca de maneira predeterminada. A opción fai o que fai [`disjoint`](disjoint.md), **cos seus
axustes predeterminados**, antes de escribir, despois da ordenación. Necesita un ficheiro
ordenado: se non, rexéitase, e `--sort` — que vai primeiro — ordénao antes. O ficheiro dio no
primeiro nivel de narración (`<path>: N subtitles split, M merged`), e o rexistro `--format json`
leva os recontos `split` e `merged`, **ausentes sen a opción**.

**`--join-repeats` vai antes da ordenación e do corte**: un texto cortado en anacos que se
suceden reconstrúese primeiro (véxase [`join-repeats`](join-repeats.md)), despois ordénanse os
subtítulos, e despois fanse disxuntas as franxas. **`--repair` son as tres á vez**, como
[`repair`](repair.md), cos axustes predeterminados do corte. Con `--join-repeats`, cada ficheiro di
`<path>: N repeated subtitles joined` no primeiro nivel de narración, e o rexistro `--format json`
leva `counts.joined` — **ausente sen a opción**.

## Saída

**Saída estándar** — nada: o resultado é o ficheiro escrito. Con `--dry-run`, tampouco nada: non
hai ficheiro, e a liña de narración dio — véxase [Ver antes de escribir](invocation.md#ver-antes-de-escribir).

Na saída de erro, segundo o nivel:

| Nivel | O que se engade |
| :----- | :-------------- |
| 1 | `<path>: N subtitles written as WebVTT -> <destination>` |
| 2 | `<path>: SubRip -> WebVTT, UTF-8, no BOM, LF line endings` — a codificación nomeada é a escrita |
| 3 | `<path>: N bytes read, M written`, e despois **cada diagnóstico de lectura** — véxase [Invocación](invocation.md#diagnósticos-de-lectura) |

### En JSON

Con [`--format json`](invocation.md#saída-que-pode-ler-un-script), o obxecto de cada ficheiro
leva `counts.subtitles` e despois o que perdeu a conversión: `lost_ends` e `lost_header` (`0` ou
`1`), `joined_lines`, `lost_tags`, `lost_fields` e `furthest_ms` — os elementos da
[frase de perda](#o-que-a-conversión-di-que-perdeu). A táboa completa é a de
[Invocación](invocation.md#o-que-pon-cada-suborde-en-counts).

## Códigos de saída

Os da ferramenta: `0` se se escribiron todos os ficheiros, `2` se non se escribiu ningún, `3` se
só se escribiron algúns, `1` nun erro de uso.

## Erros

| O que o provoca | Mensaxe |
| :------------------ | :------ |
| `--bom` con `--no-bom` | `--bom and --no-bom ask for opposite things; give one or the other` |
| `--in-place` que cambia o formato | `--in-place cannot change the format: the file would keep a name its content no longer matches` |
| `--bom` nunha codificación que non ten marca | `<path>: <encoding> has no byte order mark to write` |
| `--to-encoding` que nomea unha codificación descoñecida | `no encoding is named "<name>"` |
| `--to-encoding` que nomea unha codificación que escribe a súa propia marca | `"<name>" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do` |
| carácter ausente da codificación escrita | `<path>: holds a character the chosen encoding cannot write` |

**Rexeitar `--bom` nunha codificación que non a leva non é pedantería.** Unha marca de orde de
bytes existe para as codificacións Unicode e para ningunha outra: pedila nun ficheiro en
Windows-1252 é pedir algo que non existe. O ficheiro escribiríase sen ela, e ninguén sabería que
a petición non se atendeu.

Os do destino son comúns ás subordes que escriben:
véxase [Invocación](invocation.md#o-destino).

Os erros de lectura son os de [`inspect`](inspect.md): afectan ao ficheiro de entrada e teñen as
mesmas mensaxes.
