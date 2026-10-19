# `split-file`

<!-- machine-translated -->
> Esta páxina é unha tradución automática e aínda non a revisou ningún lector da lingua.

```
subedit-cli split-file --at N --head FILE --tail FILE
                       [--dry-run] [--sort] [--disjoint]
                       <file>
```

Corta un ficheiro **en dous**: unha cabeza e unha cola. É a orde `Split Project…` da xanela: [o mesmo motor](../subedit-gui/operations.md#split-project),
os mesmos recontos, o mesmo rexeitamento; e **o inverso exacto de [`append`](append.md)**: engadir a cola á
cabeza devolve o ficheiro de partida.

- **`--at N` é o número do primeiro subtítulo da cola**, numerado como o numera a táboa
  da xanela: desde 1. `--at 3` pon os subtítulos 1 e 2 na cabeza, e o terceiro e o resto na
  cola. Vai **do segundo ao último**: un corte necesita un subtítulo a cada lado.
- **A cola lévase de volta á orixe**: desprázase en menos o final do último subtítulo que queda na cabeza,
  e as súas posicións conservan a súa orde e o seu espazamento. Iso é o que fai de `split-file` o inverso de `append`.
- **As dúas metades conservan o formato, a codificación, a marca de orde de bytes e os finais de liña do ficheiro lido**, e
  a escritura renumeraas desde 1, como calquera ficheiro que se volve escribir.
- **Dúas saídas, polo tanto un destino propio**: `--head` e `--tail`, **ambas obrigatorias**. As opcións `--output`,
  `--output-dir`, `--in-place` e `--recursive` non existen para `split-file` e rexéitanse como calquera opción
  descoñecida, código `1`: non hai nin un directorio no que repartir dous ficheiros con nome, nin unha entrada que sobrescribir.

<!-- example: subedit-cli split-file --help -->
```console
$ subedit-cli split-file --help
Cut a subtitle file in two, a head and a tail
Usage: subedit-cli split-file [OPTIONS] file

Positionals:
  file TEXT REQUIRED          Subtitle file to cut

Options:
  -h,--help                   Print this help message and exit
  --at N                      Number of the first subtitle of the tail, as the table numbers them
  --head FILE                 File to write the subtitles before the cut to
  --tail FILE                 File to write the subtitles from the cut on to
  --dry-run                   Work out and say what would be written, and write nothing
  --sort                      Put the subtitles in order of their start before cutting; --at counts in that order
  --disjoint                  Cut overlapping subtitles so that no range overlaps another, before cutting
```

## Argumentos e opcións

| Opción | Obrigatorio | Valores | Predeterminado |
| :----- | :----- | :------ | :----- |
| `<file>` | si | **unha soa** ruta | — |
| `--at` | si | un número enteiro, de 2 ao número de subtítulos do ficheiro | — |
| `--head`, `--tail` | **si, as dúas**, agás con `--dry-run` | rutas de ficheiro; o seu directorio créase se falta | — |
| `--dry-run` | non | un indicador: calcula e di, non escribe nada — véxase [Ver antes de escribir](invocation.md#ver-antes-de-escribir) | desactivado |
| `--sort` | non | un indicador: pon os subtítulos na orde do seu inicio **antes de cortar**; `--at` conta nesa orde | desactivado |
| `--disjoint` | non | un indicador: corta os subtítulos que se solapan **antes de cortar**; `--at` conta no que queda, véxase [Facer as franxas disxuntas](#facer-as-franxas-disxuntas) | desactivado |

**Con `--dry-run`, non se require ningún destino**: pódese non dar ningún, pero non só un dos dous;
`--head` e `--tail` van xuntas. As que se dan xúlganse como sen `--dry-run`.

**Os dous destinos non poden ser nin a entrada nin o mesmo ficheiro**: escribir sobre o que se está
a ler, ou escribir a cola sobre a cabeza, rexéitase antes de ler nada, código `1`.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n3\n00:00:06,000 --> 00:00:07,000\ntrois\n\n4\n00:00:08,000 --> 00:00:09,000\nquatre\n\n' > film.srt; subedit-cli split-file --at 3 --head tete.srt --tail queue.srt film.srt; cat queue.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n3\n00:00:06,000 --> 00:00:07,000\ntrois\n\n4\n00:00:08,000 --> 00:00:09,000\nquatre\n\n' > film.srt; subedit-cli split-file --at 3 --head tete.srt --tail queue.srt film.srt; cat queue.srt
film.srt: split at subtitle 3: 2 subtitles in the head, 2 subtitles in the tail -> tete.srt, queue.srt
1
00:00:02,000 --> 00:00:03,000
trois

2
00:00:04,000 --> 00:00:05,000
quatre
```

O segundo subtítulo, que queda na cabeza, remata aos 4 s: a cola retrocede 4 s, e o terceiro subtítulo,
que comezaba aos 6 s, comeza aos 2 s.

## Cortar en orde temporal

A opción `--at` numera os subtítulos **tal como os dá o ficheiro**. Nun ficheiro cuxa orde está rota, a cabeza non é, polo tanto, a metade máis temperá
no tempo. A opción `--sort` pon os subtítulos na orde dos seus inicios **antes de xulgar o corte** (de forma estable, como fai [`sort`](sort.md)), e
`--at` conta despois nesa orde.

Nunca por defecto. Coa opción, o ficheiro di no primeiro nivel de narración cantos subtítulos se moveron (`<path>: N subtitles moved`
ou `<path>: already in order`), e o rexistro `--format json` leva un reconto `moved`, **ausente sen a opción**.

## Facer as franxas disxuntas

Nunca por defecto. A opción fai o que fai [`disjoint`](disjoint.md), **cos seus valores predeterminados**, antes de xulgar o corte e despois da ordenación: `--at` conta no que iso dá. Necesita un ficheiro ordenado: se non, rexéitase, e `--sort`, que vai primeiro, ordénao previamente. O ficheiro dio no primeiro nivel de narración (`<path>: N subtitles split, M merged`), e o rexistro `--format json` leva os recontos `split` e `merged`, **ausentes sen a opción**.

## Cando se rexeita o corte

**Dúas metades que se solapan no corte non se cortan.** Se un subtítulo da cola comezase antes
do final do último subtítulo que queda atrás, o desprazamento levaríao antes do comezo do vídeo, a unha posición negativa
que ningún formato escribe. A suborde `split-file` rexéitao **nomeando o subtítulo**, coas palabras da xanela:

```text
o.srt: Cannot split at subtitle 3: subtitle 3 would fall before the start of the video. Cut somewhere else.
```

O código é `2` e **non se escribe nada**. Corte noutro sitio. Caer exactamente no comezo está permitido: o cero é unha
posición.

**Un `--at` fóra do ficheiro rexéitase do mesmo xeito**, unha vez lido o ficheiro: `--at 1` (o primeiro subtítulo non pode
comezar unha cola, non quedaría nada) e un `--at` máis alá do último subtítulo, que deixaría unha
cola baleira. Un ficheiro dun só subtítulo non se pode cortar.

## Saída

**Saída estándar** — nada: o resultado son os dous ficheiros escritos.

| Nivel | O que se engade na saída de erro |
| :----- | :------------------------------------- |
| 1 | `<path>: split at subtitle N: H subtitles in the head, T subtitles in the tail -> <head>, <tail>`; con `--dry-run`, `(dry run, nothing written)` en lugar de `-> …` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — o formato, a codificación, a marca e os finais de liña do ficheiro lido, repostos tal como estaban |
| 3 | `<path>: N bytes read, H and T written`, e despois **todos os diagnósticos de lectura** — véxase [Invocación](invocation.md#diagnósticos-de-lectura) |

### En JSON

Con [`--format json`](invocation.md#saída-que-pode-ler-un-script), **un só obxecto**, cuxo `file` é o ficheiro cortado.
**`destination` é a cabeza** (o ficheiro no que se converte a entrada) e **`tail` a cola**, unha ruta, ou `null` con
`--dry-run`, como `destination`. Os `counts`: `subtitles` (os do ficheiro lido), `head` e `tail` (os de cada
metade).

## Códigos de saída

| Código | Cando |
| :--- | :---- |
| `0` | escríbense os dous ficheiros, ou calcúlanse con `--dry-run` |
| `1` | erro de uso: sen `--head` nin `--tail`, unha opción que `split-file` non ten, un destino que é a entrada ou o outro destino, un `--at` que non é un número enteiro positivo |
| `2` | non se puido ler o ficheiro, **rexéitase o corte** (fóra do ficheiro, ou solapamento), ou non se puido escribir un destino. Non hai código `3`: procésase un só ficheiro |

**A cabeza escríbese primeiro**: un disco que rexeita a cola deixa a cabeza tras de si, e o código é `2`.

## Erros

| O que os provoca | Mensaxe |
| :------------------- | :------ |
| nin `--head` nin `--tail` | `no destination given: split-file writes two files, use --head and --tail` |
| só unha das dúas, con `--dry-run` | `--head and --tail go together: give both, or neither with --dry-run` |
| os dous destinos son o mesmo ficheiro | `<path>: named as both the head and the tail` |
| un destino é a entrada | `<path>: would be written over the input <input>` |
| `--at` fóra do ficheiro | `<file>: --at 9: the file holds 3 subtitles, and the tail can start from 2 to 3` |
| un ficheiro dun só subtítulo | `<file>: a file of 1 subtitle cannot be split: a cut needs a subtitle on each side` |
| solapamento | `<file>: Cannot split at subtitle N: subtitle M would fall before the start of the video. Cut somewhere else.` |
| un ficheiro que falta, ilexible ou dun formato descoñecido | `<path>: does not exist`… — o mesmo que en todas partes |
