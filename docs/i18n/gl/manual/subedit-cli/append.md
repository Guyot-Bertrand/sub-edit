# `append`

<!-- machine-translated -->
> Esta páxina é unha tradución automática e aínda non a revisou ningún lector da lingua.

```
subedit-cli append --output FILE
                   [--dry-run] [--sort] [--disjoint]
                   <base> <file>...
```

Pon ficheiros **despois do primeiro**, nun **único** ficheiro: a película en dúas partes, convertida nun
único conxunto de subtítulos. É a orde `Append File…` da xanela: [o mesmo motor](../subedit-gui/operations.md#append-file),
os mesmos recontos, aplicado tantas veces como ficheiros haxa que engadir.

- **O primeiro ficheiro é a base.** Dálle ao resultado **o seu formato, a súa codificación, a súa marca
  de orde de bytes e os seus finais de liña**.
- **Non se insire nada entre dous ficheiros**: cada un desprázase ata **o final do último subtítulo do que
  o precede**, e as súas posicións conservan a súa orde e o seu espazamento. O terceiro vai despois do segundo, non da
  base: os ficheiros suceden na orde en que se dan.
- **Cada ficheiro engadido pasa pola conversión** ao formato da base: tradúcense as súas etiquetas,
  e o que ese formato non pode escribir dise, ficheiro por ficheiro, coas palabras da xanela.
- **Os ficheiros engadidos non teñen tradución**: un ficheiro aberto só non ten ningunha que dar.
- **Un ficheiro pódese dar varias veces**, ou engadirse a si mesmo: nada o prohibe.

**A suborde `append` non é un lote.** N entradas, **unha** saída: o bucle das outras subordes (un resultado
por ficheiro, e un fallo que deixa continuar os demais) non ten sentido aquí. Disto despréndense tres cousas:

- **o destino é `--output`, e nada máis.** As opcións `--output-dir` e `--in-place` non existen para
  `append` e rexéitanse como calquera opción descoñecida, código `1`; o mesmo vale para `--recursive`: a orde dos
  ficheiros é a dada, e unha árbore percorrida non ten ningunha;
- **todo ou nada**: o primeiro ficheiro que non se pode ler detén a execución, e **non se escribe nada**. Unha
  película en tres partes á que lle falta unha non é unha película en dúas;
- **unha soa liña por execución** para o resultado e, en JSON, **un só obxecto**.

<!-- example: subedit-cli append --help -->
```console
$ subedit-cli append --help
Put subtitle files one after another into a single file
Usage: subedit-cli append [OPTIONS] files...

Positionals:
  files BASE FILE...          The base file, then the files to put after it, in order

Options:
  -h,--help                   Print this help message and exit
  --output FILE               File to write
  --dry-run                   Work out and say what would be written, and write nothing
  --sort                      Put the subtitles in order of their start before writing
  --disjoint                  Cut overlapping subtitles so that no range overlaps another, before writing
```

## Argumentos e opcións

| Opción | Obrigatorio | Valores | Predeterminado |
| :----- | :----- | :------ | :----- |
| `<base> <file>...` | si, **polo menos dous** | rutas: a base, e despois o que se pon tras ela, en orde | — |
| `--output` | **si**, agás con `--dry-run` | unha ruta de ficheiro; o seu directorio créase se falta | — |
| `--dry-run` | non | un indicador: calcula e di, non escribe nada — véxase [Ver antes de escribir](invocation.md#ver-antes-de-escribir) | desactivado |
| `--sort` | non | un indicador: pon o conxunto na orde dos seus inicios **unha vez engadido todo**, véxase [Ordenar o resultado](#ordenar-o-resultado) | desactivado |
| `--disjoint` | non | un indicador: corta os subtítulos que se solapan **unha vez engadido e ordenado todo**, véxase [Facer as franxas disxuntas](#facer-as-franxas-disxuntas) | desactivado |

**O destino non pode ser ningunha das entradas**, incluída a base: escribir sobre un ficheiro que se está
a ler rexéitase antes de ler nada, código `1`. Un `--output` existente distinto das entradas
sobrescríbese, como en todas partes.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n' > partie1.srt; printf '1\n00:00:00,500 --> 00:00:01,500\ntrois\n\n' > partie2.srt; subedit-cli append --output film.srt partie1.srt partie2.srt; cat film.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n' > partie1.srt; printf '1\n00:00:00,500 --> 00:00:01,500\ntrois\n\n' > partie2.srt; subedit-cli append --output film.srt partie1.srt partie2.srt; cat film.srt
partie2.srt: appended 1 subtitle
partie1.srt: 3 subtitles from 2 files -> film.srt
1
00:00:01,000 --> 00:00:02,000
un

2
00:00:03,000 --> 00:00:04,000
deux

3
00:00:04,500 --> 00:00:05,500
trois
```

O último subtítulo de `partie1.srt` remata aos 4 s; o de `partie2.srt`, que comezaba aos 0,5 s, comeza aos
4,5 s.

## Ordenar o resultado

Cada ficheiro desprázase **desde o final do que o precede**: a orde que pode romperse é, polo tanto, a dos propios ficheiros. A opción `--sort`
pon o resultado na orde dos inicios, **dunha soa vez, despois da última adición**, como faría [`sort`](sort.md): de forma estable, e sen cambiar nada
no contido.

Nunca por defecto. Coa opción, a base dio no primeiro nivel de narración, `<base>: N subtitles moved` ou `<base>: already in order`, e
o rexistro `--format json` leva un reconto `moved`, **ausente sen a opción**.

## Facer as franxas disxuntas

Nunca por defecto. A opción fai o que fai [`disjoint`](disjoint.md), **cos seus valores predeterminados**, unha vez engadido todo, sobre o conxunto. Necesita un ficheiro ordenado: se non, rexéitase, e `--sort`, que vai primeiro, ordénao previamente. O ficheiro dio no primeiro nivel de narración (`<path>: N subtitles split, M merged`), e o rexistro `--format json` leva os recontos `split` e `merged`, **ausentes sen a opción**.

## Cando os formatos difiren

O resultado está no **formato da base**, sexa cal sexa o dos demais. O que custa o cruzamento
dise ficheiro por ficheiro, coas palabras da xanela: a mesma frase que `Append File…` e que
[pegar desde outro formato](../subedit-gui/clipboard.md#as-etiquetas-doutro-formato-tradúcense):

```text
d.ass: appended 1 subtitle; Advanced SSA into SubRip: 1 tag dropped, the header was dropped
```

O que se perde depende só do formato de **cada** ficheiro engadido: unha base en Advanced SSA recibe un
ficheiro SubRip sen perder nada, pero non ao revés. Para escoller o formato do resultado, poña primeiro un ficheiro
que xa estea nese formato, ou converta despois con [`convert`](convert.md).

## Saída

**Saída estándar** — nada: o resultado é o ficheiro escrito.

| Nivel | O que se engade na saída de erro |
| :----- | :------------------------------------- |
| 1 | unha liña por ficheiro engadido — `<path>: appended N subtitles`, seguida, cando o formato custou algo, de `; <source format> into <base format>: <what is lost>` — e despois `<base>: N subtitles from M files -> <destination>` (con `--dry-run`, `(dry run, nothing written)` en lugar de `-> <destination>`) |
| 2 | `<base>: SubRip, UTF-8, no BOM, LF line endings kept` — o formato, a codificación, a marca e os finais de liña da base, repostos tal como estaban |
| 3 | `<base>: N bytes read, M written` (todos os ficheiros xuntos), e despois **todos os diagnósticos de lectura** — véxase [Invocación](invocation.md#diagnósticos-de-lectura) |

### En JSON

Con [`--format json`](invocation.md#saída-que-pode-ler-un-script), **un só obxecto** para toda a execución,
cuxo `file` é **a base**. Os seus `counts`:

| Reconto | O que di |
| :----- | :----------- |
| `files` | os ficheiros da execución, incluída a base |
| `subtitles` | os subtítulos do resultado |
| `appended` | os que proveñen dos ficheiros engadidos |
| `lost_ends`, `lost_header` | `0` ou `1`: un dos ficheiros engadidos perdeuno |
| `joined_lines`, `lost_tags`, `lost_fields` | a suma do que perderon os ficheiros engadidos |
| `furthest_ms` | o maior movemento dunha posición, todos os ficheiros xuntos |

**Unha matriz `inputs`** acompaña os `counts`, fóra deles: un elemento por ficheiro engadido, en orde,
`{"file", "counts"}`, onde `counts` leva `appended` e despois as mesmas perdas, **só para ese ficheiro**. É o que
di, nunha execución de dez ficheiros, cal custou que. Os avisos de lectura de todos os ficheiros
xúntanse en `warnings`.

Un fallo tamén é un obxecto, **un só**: o do primeiro ficheiro que non se puido ler.

## Códigos de saída

| Código | Cando |
| :--- | :---- |
| `0` | o resultado escríbese, ou calcúlase con `--dry-run` |
| `1` | erro de uso: menos de dous ficheiros, sen `--output`, unha opción que `append` non ten, un destino que é unha das entradas |
| `2` | **non se puido ler un ficheiro**, ou non se puido escribir o destino: non se escribe nada. Non hai código `3`: unha execución que só escribe un ficheiro ten éxito ou non |

## Erros

| O que os provoca | Mensaxe |
| :------------------- | :------ |
| un só ficheiro dado | `append needs a base file and at least one file to put after it` |
| nin `--output` nin `--dry-run` | `no destination given: append writes one file, use --output` |
| `--output` nomea unha entrada | `<destination>: would be written over the input <input>` |
| un ficheiro que falta, ilexible ou dun formato descoñecido | `<path>: does not exist`, `<path>: is in no format this tool knows`… — o mesmo que en todas partes |

**Un ficheiro baleiro non se engade**: ningún formato le sen polo menos un subtítulo, e o ficheiro
rexéitase como calquera ficheiro que non se pode ler (`<path>: holds nothing recognisable as a subtitle`), o que
detén a execución e non escribe nada.
