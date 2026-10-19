# `sort`

<!-- machine-translated -->
> Esta páxina é unha tradución automática e aínda non a revisou ningún lector da lingua.

```
subedit-cli sort (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 <file>...
```

Volve poñer os subtítulos **na orde do seu inicio**. Non cambia nada máis: nin os textos, nin as
posicións, nin o formato do ficheiro. A escritura **renumera** os subtítulos desde 1,
como para calquera ficheiro que se volve escribir.

**O núcleo nunca ordena por iniciativa propia**: ler un ficheiro deixa a súa orde como estaba, e
[`inspect`](inspect.md#o-que-indica-anomalies) indica unha ruptura da orde con
`subtitle N starts before the previous one starts`. `sort` é **o xesto feito a propósito**, para un ficheiro
ou un lote cuxa orde está rota — e pódese desfacer, na xanela, como calquera operación.

- **A ordenación é estable**: dous subtítulos que comezan no mesmo momento **conservan a orde que lles
  daba o ficheiro**. Ningún vai antes ca o outro; movelos sería unha decisión que ninguén
  pediu.
- **`sort` non acepta nin franxa nin documento**: non hai nada que escoller, a orde é a dos inicios.

<!-- example: subedit-cli sort --help -->
```console
$ subedit-cli sort --help
Put the subtitles in the order of their start, keeping ties as they are
Usage: subedit-cli sort [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to sort

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

<!-- example: printf '1\n00:00:05,000 --> 00:00:06,000\ncinq\n\n2\n00:00:01,000 --> 00:00:02,000\nun\n\n3\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > desordre.srt; subedit-cli sort --output ordre.srt desordre.srt; cat ordre.srt -->
```console
$ printf '1\n00:00:05,000 --> 00:00:06,000\ncinq\n\n2\n00:00:01,000 --> 00:00:02,000\nun\n\n3\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > desordre.srt; subedit-cli sort --output ordre.srt desordre.srt; cat ordre.srt
desordre.srt: 3 subtitles moved -> ordre.srt
1
00:00:01,000 --> 00:00:02,000
un

2
00:00:03,000 --> 00:00:04,000
trois

3
00:00:05,000 --> 00:00:06,000
cinq
```

## O que se conta

**O reconto é o dos lugares que cambiaron de subtítulo**, tal como o indica o núcleo — o que
o historial da xanela chamaría movido. Un só subtítulo mal colocado despraza varios: no
exemplo anterior, ningún dos tres lugares conserva o seu subtítulo, e a frase di `3 subtitles moved`.

**Un ficheiro xa en orde escríbese igualmente**, e dio: un destino indicado é un
destino escrito, e facer de `sort` a excepción obrigaría a un script a saber que
subordes producen ás veces un ficheiro e ás veces nada.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > ordre.srt; subedit-cli sort --output copie.srt ordre.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > ordre.srt; subedit-cli sort --output copie.srt ordre.srt
ordre.srt: already in order -> copie.srt
```

## Saída

**Saída estándar** — nada: o resultado é o ficheiro escrito. Con `--dry-run`, tampouco nada: non hai
ficheiro, e a liña de narración dio — véxase [Ver antes de escribir](invocation.md#ver-antes-de-escribir).

| Nivel | O que se engade na saída de erro |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles moved -> <destination>`, ou `<path>: already in order -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — o formato, a codificación, a marca e os finais de liña do ficheiro lido, repostos como estaban |
| 3 | `<path>: N bytes read, M written`, despois **cada diagnóstico de lectura** — véxase [Invocación](invocation.md#diagnósticos-de-lectura) |

### En JSON

Con [`--format json`](invocation.md#saída-que-pode-ler-un-script), o obxecto de cada ficheiro leva
`counts.subtitles` (todos os subtítulos do ficheiro) e `counts.moved` (os lugares que cambiaron;
`0` para un ficheiro xa en orde).

## Códigos de saída

Os da ferramenta: `0` se se procesaron todos os ficheiros — un ficheiro xa en orde tamén conta —, `2` se non se procesou
ningún, `3` se só algúns, `1` nun erro de uso.

## Erros

Ningún propio de `sort`: un ficheiro ilexible falla como para calquera suborde, e os
erros do destino son comúns ás subordes que escriben:
véxase [Invocación](invocation.md#o-destino).
