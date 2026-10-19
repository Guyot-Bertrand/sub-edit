# `join-repeats`

<!-- machine-translated -->
> Esta páxina é unha tradución automática e aínda non a revisou ningún lector da lingua.

```
subedit-cli join-repeats (--output FILE | --output-dir DIR | --in-place)
                         [--dry-run]
                         [--recursive]
                         <file>...
```

Fai **un só subtítulo** dos subtítulos que mostran o mesmo un tras outro: un texto cortado en anacos
que se suceden, tal como o deixan algunhas ferramentas de segmentación. É o `Join Repeated Subtitles` da xanela —
[o mesmo motor](../subedit-gui/lines.md#unir-os-subtítulos-repetidos).

**Unha repetición** é un subtítulo que ten **o mesmo texto, a mesma tradución e os mesmos axustes de formato** (un
estilo, unha posición, axustes de indicación) ca outro, **e que comeza como moi tarde cando ese remata**: tócanse
ou sólapanse. Unha serie de repeticións convértese **no que comeza primeiro**, cuxo final se
leva ata o final máis tardío da serie; os outros desaparecen.

- **A orde do ficheiro non inflúe, e non se cambia**: o subtítulo que se conserva é o que comeza primeiro,
  estean onde estean os outros. Un ficheiro en orde segue en orde.
- **Non se une nada por erro**: outro texto, outra tradución, outros axustes, unha separación entre dous
  anacos, un subtítulo sen texto ou cuxo final vai antes do seu inicio — en todos estes casos quedan separados.
- **Para facer antes do resto**: a ordenación e [cortar os solapamentos](disjoint.md) len mellor unha serie limpa.
  [`repair`](repair.md) encadea os tres, e [`convert`](convert.md#facer-as-franxas-disxuntas) acéptao como opción (`--join-repeats`, `--repair`).

<!-- example: subedit-cli join-repeats --help -->
```console
$ subedit-cli join-repeats --help
Join the subtitles that repeat each other, one after the other, into one
Usage: subedit-cli join-repeats [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to join the repeats of

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Argumentos e opcións

| Opción | Obrigatorio | Valores | Predeterminado |
| :----- | :----- | :------ | :----- |
| `<file>...` | si | unha ou máis rutas | — |
| `--recursive`, `-r` | non | un indicador | desactivado — véxase [Procesar unha árbore](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **unha das tres**, agás con `--dry-run` | véxase [Invocación](invocation.md#o-destino) | — |
| `--dry-run` | non | un indicador: calcula e di, non escribe nada — véxase [Ver antes de escribir](invocation.md#ver-antes-de-escribir) | desactivado |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:04,000 --> 00:00:06,000\nAlpha\n\n4\n00:00:08,000 --> 00:00:09,000\nBeta\n\n' > morceaux.srt; subedit-cli join-repeats --output joint.srt morceaux.srt; cat joint.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:04,000 --> 00:00:06,000\nAlpha\n\n4\n00:00:08,000 --> 00:00:09,000\nBeta\n\n' > morceaux.srt; subedit-cli join-repeats --output joint.srt morceaux.srt; cat joint.srt
morceaux.srt: 2 repeated subtitles joined -> joint.srt
1
00:00:01,000 --> 00:00:06,000
Alpha

2
00:00:08,000 --> 00:00:09,000
Beta
```

Os tres anacos de `Alpha`, que se suceden de 1 s a 6 s, agora forman un só. `Beta`, separado por unha separación de tempo,
quedou. Unha segunda pasada non cambia nada e dio: `no repeated subtitles`.

## Saída

**Saída estándar** — nada: o resultado é o ficheiro escrito.

| Nivel | O que se engade na saída de erro |
| :----- | :------------------------------------- |
| 1 | `<path>: N repeated subtitles joined -> <destination>`, ou `<path>: no repeated subtitles -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, despois **cada diagnóstico de lectura** |

### En JSON

Con [`--format json`](invocation.md#saída-que-pode-ler-un-script), os `counts` de cada ficheiro: `subtitles` (os do ficheiro
escrito), `joined` (os subtítulos eliminados porque outro os cobre) e `chains` (as series unidas).

## Códigos de saída

Os da ferramenta: `0` se se procesaron todos os ficheiros — un ficheiro sen repeticións tamén conta —, `2` se non se procesou ningún, `3` se
só algúns, `1` nun erro de uso.

## Erros

Ningún propio de `join-repeats`: un ficheiro ilexible falla como para calquera suborde, e os erros do
destino son comúns ás subordes que escriben: véxase [Invocación](invocation.md#o-destino).
