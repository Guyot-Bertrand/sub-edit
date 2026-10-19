# `disjoint`

<!-- machine-translated -->
> Esta páxina é unha tradución automática e aínda non a revisou ningún lector da lingua.

```
subedit-cli disjoint (--output FILE | --output-dir DIR | --in-place)
                     [--minimum TIME] [--max-lines N] [--dashes] [--sort]
                     [--dry-run] [--recursive]
                     <file>...
```

Corta os subtítulos que se solapan para que **ningunha franxa `[start, end]` se solape xa
con outra**. É a orde `Make Ranges Disjoint…` da xanela: [o mesmo motor](../subedit-gui/lines.md#facer-as-franxas-disxuntas).

**Non se acurta nin se alonga nada**: en cada momento, o ficheiro mostra os mesmos textos ca antes. O que
cambia é a maneira de cortar a liña de tempo:

- córtase en **cada inicio e cada final** dos subtítulos dun grupo que se solapa;
- un intervalo cuberto por **un só** subtítulo segue sendo ese subtítulo;
- un intervalo cuberto por **varios** convértese nun **único** subtítulo cuxas liñas son as deles, **na
  orde dos seus inicios**: o máis temperán na liña 1, o seguinte na liña 2;
- un subtítulo que non se solapa con nada non se toca, e a orde do resto consérvase.

<!-- example: subedit-cli disjoint --help -->
```console
$ subedit-cli disjoint --help
Cut overlapping subtitles so that no range overlaps another, merging what overlaps
Usage: subedit-cli disjoint [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to make disjoint

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --minimum TIME              Shortest interval kept; a shorter one is taken over by a neighbour (default 0.08)
  --max-lines N               Most lines of a merged subtitle; more are shown in windows, 0 for no limit (default 2)
  --dashes                    Put a dash before the text of each merged subtitle
  --sort                      Put the subtitles in order of their start first; without it, a file out of order is refused
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Argumentos e opcións

| Opción | Obrigatorio | Valores | Predeterminado |
| :----- | :----- | :------ | :----- |
| `<file>...` | si | unha ou máis rutas | — |
| `--minimum` | non | unha duración: `0.08`, `1.5`, `00:00:00,080`. Cero conserva todos os intervalos | `0.08` (uns dous fotogramas) |
| `--max-lines` | non | un número enteiro: o máximo de liñas dun subtítulo fusionado; `0` elimina o límite | `2` |
| `--dashes` | non | un indicador: un trazo diante do texto que cada subtítulo achega a un subtítulo fusionado | desactivado |
| `--sort` | non | un indicador: pon os subtítulos na orde do seu inicio **primeiro** — véxase [Cómpre un ficheiro ordenado](#cómpre-un-ficheiro-ordenado) | desactivado |
| `--recursive`, `-r` | non | un indicador | desactivado — véxase [Procesar unha árbore](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **unha das tres**, agás con `--dry-run` | véxase [Invocación](invocation.md#o-destino) | — |
| `--dry-run` | non | un indicador: calcula e di, non escribe nada — véxase [Ver antes de escribir](invocation.md#ver-antes-de-escribir) | desactivado |

<!-- example: printf '1\n00:00:00,000 --> 00:00:05,000\nA\n\n2\n00:00:03,000 --> 00:00:08,000\nB\n\n' > chevauche.srt; subedit-cli disjoint --output disjoint.srt chevauche.srt; cat disjoint.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:05,000\nA\n\n2\n00:00:03,000 --> 00:00:08,000\nB\n\n' > chevauche.srt; subedit-cli disjoint --output disjoint.srt chevauche.srt; cat disjoint.srt
chevauche.srt: 2 subtitles split, 1 merged -> disjoint.srt
1
00:00:00,000 --> 00:00:03,000
A

2
00:00:03,000 --> 00:00:05,000
A
B

3
00:00:05,000 --> 00:00:08,000
B
```

`A [0, 5]` e `B [3, 8]` dan tres subtítulos disxuntos: `A` só, `A` e `B` xuntos, `B` só. Non se
perde nin se engade ningún momento.

## O que decide a operación

**Unha liña nunca se duplica.** Dous subtítulos co mesmo texto dan unha soa liña onde se
solapan, e dous anacos veciños que mostran o mesmo forman un só subtítulo. Pola contra, o texto dun
subtítulo cortado en varios anacos **repítese en cada un**: é o que hai na pantalla nese
momento.

**Un veciño absorbe un intervalo moi curto.** Por debaixo de `--minimum`, un intervalo desaparece: o veciño
**estende a súa franxa para cubrilo**, de modo que ningún momento queda baleiro e nada se move máis ca ese
limiar. O veciño escollido é aquel cuxas liñas **xa conteñen** as do intervalo; se non, o
máis longo. O texto do intervalo só se perde se ningún veciño o leva, e daquela durante menos có limiar.
**Un subtítulo enteiro máis curto có limiar nunca se absorbe**: déixase e cóntase.

**Máis liñas có límite: xanelas.** Un subtítulo fusionado que tería máis de `--max-lines` liñas
móstrase **en xanelas sucesivas** dese tamaño, en partes iguais da duración. Tres liñas en tres
segundos:

<!-- example: printf '1\n00:00:00,000 --> 00:00:03,000\nA\n\n2\n00:00:00,000 --> 00:00:03,000\nB\n\n3\n00:00:00,000 --> 00:00:03,000\nC\n\n' > trois.srt; subedit-cli disjoint --output fenetres.srt trois.srt; cat fenetres.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:03,000\nA\n\n2\n00:00:00,000 --> 00:00:03,000\nB\n\n3\n00:00:00,000 --> 00:00:03,000\nC\n\n' > trois.srt; subedit-cli disjoint --output fenetres.srt trois.srt; cat fenetres.srt
trois.srt: 0 subtitles split, 1 merged; 1 shown in windows for holding too many lines -> fenetres.srt
1
00:00:00,000 --> 00:00:01,500
A
B

2
00:00:01,500 --> 00:00:03,000
B
C
```

Catro liñas dan tres xanelas, e así sucesivamente: a xanela desprázase unha liña cada vez. Nada se
trunca nin se volve cortar.

**A tradución segue os mesmos cortes**, xa que un subtítulo leva os dous textos. **O que o formato leva
ademais** (un estilo, unha posición) é o do primeiro subtítulo que cobre; a perda cóntase cando
os subtítulos fusionados difiren.

**Un subtítulo cuxo final vén antes do seu inicio** non se adiviña: déixase onde está, e cóntase.

## Cómpre un ficheiro ordenado

Os grupos son series de veciños: fóra de orde, os veciños non son os que se solapan. Un ficheiro
cuxos inicios non están en orde **rexéitase**, polo tanto, e a mensaxe dino:

<!-- example: printf '1\n00:00:03,000 --> 00:00:08,000\nB\n\n2\n00:00:00,000 --> 00:00:05,000\nA\n\n' > desordre.srt; subedit-cli disjoint --output x.srt desordre.srt; echo "code $?" -->
```console
$ printf '1\n00:00:03,000 --> 00:00:08,000\nB\n\n2\n00:00:00,000 --> 00:00:05,000\nA\n\n' > desordre.srt; subedit-cli disjoint --output x.srt desordre.srt; echo "code $?"
desordre.srt: The subtitles are not in the order of their start: sort them first. Use --sort to do that here.
code 2
```

A opción `--sort` ordena primeiro (o mesmo que [`sort`](sort.md)), e dio. O núcleo nunca ordena por si só.

Unha segunda pasada non cambia nada: unha vez que as franxas son disxuntas, non queda nada que cortar, e a
frase di `already disjoint`.

## Saída

**Saída estándar** — nada: o resultado é o ficheiro escrito.

| Nivel | O que se engade na saída de erro |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles split, M merged -> <destination>`, seguido, despois dun `;`, do que non se puido facer por completo: `K shown in windows for holding too many lines`, `K very short taken over by a neighbour`, `K subtitles shorter than the limit left whole`, `K merged with styles or positions that differ, the first kept`, `K subtitles ending before the start left alone` — ou `<path>: already disjoint -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, e despois **todos os diagnósticos de lectura** |

`N subtitles split` conta os subtítulos orixinais cortados en varios anacos, e `M merged` os anacos que
mostran varios subtítulos xuntos.

### En JSON

Con [`--format json`](invocation.md#saída-que-pode-ler-un-script), os `counts` de cada ficheiro:

| Reconto | O que di |
| :----- | :----------- |
| `subtitles` | os subtítulos do ficheiro escrito |
| `split`, `merged` | como na frase |
| `windowed` | os anacos mostrados en xanelas |
| `absorbed` | os intervalos absorbidos por un veciño |
| `too_short` | os subtítulos máis curtos có limiar, deixados enteiros |
| `ending_before_start` | os subtítulos cuxo final vén antes do seu inicio, deixados tal cal |
| `extras_dropped` | os anacos fusionados cuxo estilo ou posición difiren |
| `moved` | só con `--sort`: os lugares que cambiaron |

## Códigos de saída

Os da ferramenta: `0` se se procesaron todos os ficheiros (un ficheiro sen solapamentos procésase), `2` se
non se procesou ningún, `3` se só algúns, `1` nun erro de uso.

## Erros

| O que os provoca | Mensaxe |
| :------------------- | :------ |
| un ficheiro fóra de orde sen `--sort` | `<path>: The subtitles are not in the order of their start: sort them first. Use --sort to do that here.` |
| `--minimum` non é unha duración, ou é negativa | `--minimum: …` |
| `--max-lines` non é un número enteiro | `--max-lines: "…" is not a number of lines: expected a whole number, 0 for no limit` |

Un ficheiro ilexible falla como en calquera suborde; os erros do destino son comúns ás
subordes que escriben: véxase [Invocación](invocation.md#o-destino).

## Nas outras ordes

[`convert`](convert.md#facer-as-franxas-disxuntas), [`append`](append.md#facer-as-franxas-disxuntas) e
[`split-file`](split-file.md#facer-as-franxas-disxuntas) aceptan `--disjoint`, que fai o mesmo cos
valores predeterminados, antes de escribir.

Nun ficheiro mal segmentado (un texto cortado en anacos que se suceden, dados fóra de orde), ordenar e cortar
por si sós dan liñas que saltan dun anaco ao seguinte: primeiro [`join-repeats`](join-repeats.md), ou
[`repair`](repair.md), que encadea os tres.
