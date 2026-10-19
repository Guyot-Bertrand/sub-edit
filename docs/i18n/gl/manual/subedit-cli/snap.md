# `snap`

<!-- machine-translated -->
> Esta páxina é unha tradución automática e aínda non a revisou ningún lector da lingua.

```
subedit-cli snap --rate <rate>
                 (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 <file>...
```

Pon **cada marca de tempo** — inicio e final — no fotograma máis próximo da
taxa de fotogramas indicada. Non se recalcula ningún tempo: cada posición móvese como moito medio
fotograma.

## `snap` non é `framerate`, e confundilos non se nota

As dúas ordes teñen a mesma forma — ficheiros, un destino, unha taxa de fotogramas — e fan cousas opostas.

| | [`framerate`](framerate.md) | `snap` |
| :--- | :-------------------------- | :----- |
| o que calcula | `t' = t x input rate / output rate` | o fotograma máis próximo, na grade de `--rate` |
| canto despraza | en proporción ao tempo — **varios segundos** nunha longametraxe | medio fotograma como moito, **en todas partes** |
| para que ficheiro | un ficheiro cuxa **temporización é incorrecta** | un ficheiro cuxa temporización é **correcta** e cuxa grade é incorrecta |

**O caso de uso de `snap`**, en detalle: unha película a 25 fotogramas por
segundo, un ficheiro de subtítulos escrito nunha grade a 24, liñas xa
máis ou menos no seu sitio con poucos milisegundos de diferenza. Non hai tempos que recalcular
— só hai que volver poñer as marcas de tempo nos fotogramas correctos.

**O caso de uso de `framerate`** é o outro: un ficheiro temporizado para un máster a
25 que se quere reproducir nunha copia a 23,976, e cuxa temporización se desvía
varios segundos ao final da película.

Tomar un polo outro **non provoca ningún erro**: os dous
aceptan o ficheiro e escriben un resultado. A liña de narración é o que
os distingue — `snap` di sempre canto moveu as posicións.

<!-- example: subedit-cli snap --help -->
```console
$ subedit-cli snap --help
Move every position onto the nearest frame of a frame rate (see framerate)
Usage: subedit-cli snap [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to align

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --rate TEXT REQUIRED        Frame rate to align on: 25, 23.976
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
| `--rate` | **si** | unha taxa de fotogramas en fotogramas por segundo, escrita como para [`framerate`](framerate.md#escribir-unha-taxa-de-fotogramas) | — |
| `--output` / `--output-dir` / `--in-place` | **unha das tres**, agás con `--dry-run` | véxase [Invocación](invocation.md#o-destino) | — |
| `--dry-run` | non | un indicador: calcula e di, non escribe nada — véxase [Ver antes de escribir](invocation.md#ver-antes-de-escribir) | desactivado |

`--rate` acepta **calquera taxa de fotogramas válida**, e non só as oito
estándar: é a elección do usuario, como para `framerate`. Rexéitase unha taxa de fotogramas
nula ou negativa.

O formato do ficheiro lido **consérvase**: cambiar o formato é o traballo de
[`convert`](convert.md).

## Saída

**Saída estándar** — nada: o resultado é o ficheiro escrito. Con `--dry-run`, tampouco nada: non hai ficheiro, e a liña de narración dio — véxase [Ver antes de escribir](invocation.md#ver-antes-de-escribir).

| Nivel | O que se engade na saída de erro |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles aligned on 25 fps, M positions moved, by at most 20 ms -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — o formato, a codificación, a marca e os finais de liña do ficheiro lido, repostos como estaban |
| 3 | `<path>: N bytes read, M written`, despois **cada diagnóstico de lectura** — véxase [Invocación](invocation.md#diagnósticos-de-lectura) |

**Os dous números son a razón de ser da liña.** Medio fotograma a 25 fotogramas por
segundo son vinte milisegundos, e alí non se pode escribir nada maior. Quen
quería `framerate` e escribiu `snap` le «by at most 20 ms» onde esperaba
segundos, e decátase de inmediato.

## Un exemplo

Unha liña en `00:00:01,010`, aliñada a 25 fotogramas por segundo — polo tanto nun
múltiplo de corenta milisegundos:

<!-- example: printf '1\n00:00:01,010 --> 00:00:03,020\nUn.\n\n' > a.srt; subedit-cli snap --rate 25 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,010 --> 00:00:03,020\nUn.\n\n' > a.srt; subedit-cli snap --rate 25 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle aligned on 25 fps, 2 positions moved, by at most 20 ms -> b.srt
1
00:00:01,000 --> 00:00:03,040
Un.
```

## Dúas propiedades

**Aliñar dúas veces non cambia nada a segunda vez.** Unha posición que xa está nun
fotograma queda alí: é a mesma operación aplicada ao mesmo resultado.

**Consérvase a orde dos subtítulos.** O redondeo ao fotograma máis próximo é
monótono, así que dous inicios separados polo menos por un fotograma seguen séndoo. Máis
próximos, poden fundirse — o que é unha anomalía que
[`inspect`](inspect.md#o-que-indica-anomalies) sabe nomear, e non unha
inversión.

### En JSON

Con [`--format json`](invocation.md#saída-que-pode-ler-un-script), o obxecto de cada ficheiro leva `counts.subtitles`, `counts.moved` (as posicións movidas) e `counts.furthest_ms` (o maior desprazamento, en milisegundos). A táboa completa está en [Invocación](invocation.md#o-que-pon-cada-suborde-en-counts).

## Códigos de saída

Os da ferramenta: `0` se se escribiron todos os ficheiros, `2` se non se escribiu ningún, `3` se
só se escribiron algúns, `1` nun erro de uso.

## Erros

Os dunha taxa de fotogramas, palabra por palabra os de [`framerate`](framerate.md#erros)
— os dous len o mesmo valor polo mesmo camiño:

| O que o provoca | Mensaxe |
| :------------------ | :------ |
| taxa de fotogramas nula ou negativa | `"…" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed` |
| coma decimal | `"…" is not a frame rate: use a decimal point and not a comma, in every language` |
| texto que non é un número | `"…" is not a frame rate: expected frames per second, like 25 or 23.976` |
| taxa de fotogramas desmesurada | `"…" is not a frame rate: no video runs at that many frames per second` |

**Non se escribe nada sen unha das tres opcións de destino**, e os seus erros
son comúns ás subordes que escriben:
véxase [Invocación](invocation.md#o-destino).
