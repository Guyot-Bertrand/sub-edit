# `adjust`

<!-- machine-translated -->
> Esta páxina é unha tradución automática e aínda non a revisou ningún lector da lingua.

```
subedit-cli adjust [--speed CPS|off] [--shorten | --no-shorten] [--lengthen | --no-lengthen]
                   [--minimum TIME|off] [--maximum TIME|off] [--gap TIME|off]
                   [--range N-M|N-]
                   (--output FILE | --output-dir DIR | --in-place)
                   [--dry-run]
                   [--recursive]
                   <file>...
```

Axusta a **duración** de cada subtítulo a unha velocidade de lectura e a uns límites: un
subtítulo demasiado curto para lerse alóngase, un subtítulo que deixa a pantalla demasiado
tarde pode acurtarse, e dous subtítulos non se solapan. **Só se move o final**: mover o
inicio movería o subtítulo, o que é traballo de [`shift`](shift.md). É a operación
`Adjust Durations…` da [xanela](../subedit-gui/operations.md#adjust-durations), cos mesmos
valores predeterminados.

## As catro restricións e a súa orde

Aplícanse **nesta orde**, e cada unha fixa o final; gaña a última:

1. **a velocidade de lectura** — a duración que necesita o texto, a tantos caracteres por
   segundo (as etiquetas non contan, os saltos de liña si);
2. **o mínimo** — unha duración por debaixo del lévase ata o mínimo;
3. **o máximo** — unha duración por riba del redúcese ata o máximo;
4. **a separación** — o tempo que queda antes do subtítulo seguinte. Vén en último lugar e
   **gaña**: dous subtítulos mostrados á vez son un defecto que o espectador ve, un subtítulo
   un pouco curto non o é.

O último subtítulo do ficheiro non ten seguinte e, polo tanto, ningunha separación que
respectar. O seguinte dun subtítulo é o do ficheiro, **estea escollido ou non** por `--range`.

| Restrición | Opción | Predeterminado | Desactivala |
| :--------- | :----- | :----- | :------- |
| velocidade | `--speed CPS` | **15** caracteres por segundo | `--speed off` |
| sentido da velocidade | `--shorten`, `--no-shorten`, `--lengthen`, `--no-lengthen` | só **alonga** | — |
| mínimo | `--minimum TIME` | **1,5 s** | `--minimum off` |
| máximo | `--maximum TIME` | **ningún** | `--maximum off` |
| separación | `--gap TIME` | **0** | `--gap off` |

Son os axustes de Gaupol e da primeira apertura da xanela. **`off` desactiva unha restrición:
xa non se verifica.** Iso non é poñela a cero — un mínimo de `0` é un mínimo de cero, unha
separación de `0` prohibe calquera solapamento.

## O que achega o ficheiro de configuración

**Os valores predeterminados anteriores son os integrados.** De maneira predeterminada, `adjust`
le a configuración das operacións do ficheiro de configuración — os valores que
`Adjust Durations…` lembra na xanela, ou que escribiu [`config set`](config.md) — e estes ocupan
o lugar dos valores predeterminados integrados. **Unha opción indicada na liña gaña sempre**,
despois o ficheiro, despois o valor predeterminado integrado. Véxase
[O ficheiro de configuración](invocation.md#o-ficheiro-de-configuración) para saber onde está o
ficheiro, `--config` e `--no-config`.

| Clave | O que achega | Opción que a substitúe |
| :-- | :--------------- | :----------------------- |
| `duration-adjust.speed` | a velocidade de lectura, en caracteres por segundo | `--speed CPS` |
| `duration-adjust.lengthen`, `duration-adjust.shorten` | en que sentido pode mover a velocidade un final; as dúas a `false` desactivan a velocidade | `--lengthen`, `--no-lengthen`, `--shorten`, `--no-shorten`, `--speed off`; indicar `--speed CPS` ou un sentido volve activar, alongando, unha velocidade que o ficheiro desactivou |
| `duration-adjust.minimum-enabled`, `duration-adjust.minimum-ms` | a duración máis curta, ou ningunha | `--minimum TIME`, `--minimum off` |
| `duration-adjust.maximum-enabled`, `duration-adjust.maximum-ms` | a duración máis longa, ou ningunha | `--maximum TIME`, `--maximum off` |
| `duration-adjust.gap-enabled`, `duration-adjust.gap-ms` | a separación mínima, ou ningunha | `--gap TIME`, `--gap off` |

**Cada clave ten unha opción que a desfai.** Cada sentido ten un par de marcas que o fixan diga o
que diga o ficheiro: `--shorten` e `--no-shorten` para acurtar, `--lengthen` e `--no-lengthen`
para alongar; unha marca que non se indica deixa o sentido do ficheiro tal como está.
`--maximum off` descarta un máximo que o ficheiro activa. Con `--format json`, `constraints` di
os axustes que se usaron realmente, veñan de onde veñan, e `settings` o ficheiro que se leu:

<!-- example: printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine/settings.conf config set duration-adjust.speed 12 2>/dev/null; subedit-cli --config mine/settings.conf --format json adjust --dry-run a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine/settings.conf config set duration-adjust.speed 12 2>/dev/null; subedit-cli --config mine/settings.conf --format json adjust --dry-run a.srt
settings: read mine/settings.conf
a.srt: adjusted the durations of 1 subtitle (dry run, nothing written)
{"schema":1,"command":"adjust","file":"a.srt","ok":true,"dry_run":true,"destination":null,"counts":{"subtitles":1,"adjusted":1,"sacrificed":{"speed":0,"minimum":0,"gap":0}},"constraints":{"speed":{"cps":"12","lengthen":true,"shorten":false},"minimum_ms":1500,"maximum_ms":null,"gap_ms":0},"settings":"mine/settings.conf","warnings":[]}
```

Os demais exemplos desta páxina execútanse con `--no-config`, para que digan o que fan os
valores predeterminados integrados, sexa cal sexa a configuración da máquina que os executa.

<!-- example: subedit-cli adjust --help -->
```console
$ subedit-cli adjust --help
Bring the duration of every subtitle within a reading speed and limits
Usage: subedit-cli adjust [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to adjust

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --speed CPS|off             Reading speed in characters per second, or off (default 15)
  --shorten                   Let the reading speed bring an end earlier
  --no-shorten                Do not let the reading speed bring an end earlier
  --lengthen                  Let the reading speed move an end later
  --no-lengthen               Do not let the reading speed move an end later
  --minimum TIME|off          Shortest duration, or off (default 1.5)
  --maximum TIME|off          Longest duration, or off (default: none)
  --gap TIME|off              Least time left before the next subtitle, or off (default 0)
  --range N-M|N-              Act only on subtitles N to M, or N to the end
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
| `--speed` | non | un número de caracteres por segundo, **maior que cero** (`15`, `12.5`), ou `off` | `15` |
| `--shorten` | non | unha marca: a velocidade tamén pode **adiantar** un final | desactivada |
| `--no-shorten` | non | unha marca: a velocidade xa non **adianta** ningún final, diga o que diga o ficheiro de configuración | desactivada |
| `--lengthen` | non | unha marca: a velocidade **atrasa** un final, diga o que diga o ficheiro de configuración | desactivada |
| `--no-lengthen` | non | unha marca: a velocidade xa non **atrasa** ningún final | desactivada |
| `--minimum` | non | unha duración igual ou maior que cero, ou `off` | `1.5` |
| `--maximum` | non | unha duración **maior que cero**, ou `off` | ningún |
| `--gap` | non | unha duración igual ou maior que cero, ou `off` | `0` |
| `--range` | non | `N-M`, ou `N-` ata o final — véxase máis abaixo | o ficheiro enteiro |
| `--output` / `--output-dir` / `--in-place` | **unha das tres**, agás con `--dry-run` | véxase [Invocación](invocation.md#o-destino) | — |
| `--dry-run` | non | unha marca: calcula e di, non escribe nada — véxase [Ver antes de escribir](invocation.md#ver-antes-de-escribir) | desactivada |

Unha duración escríbese como para [`shift`](shift.md#escribir-unha-duración): `2.5`, `00:00:02.500`.

**Sen ningunha restrición activa, `adjust` rexéitase** (código `1`, non se escribe nada): non
habería nada ao que axustarse, e unha orde que non fai nada mentres di que tivo éxito é a peor
resposta. É o caso de `--speed off --minimum off --gap off` sen máximo, e de `--no-lengthen`
só, co mínimo e a separación desactivados: unha velocidade que non move ningún final non
restrinxe nada.

`--shorten`, `--no-shorten`, `--lengthen` e `--no-lengthen` din en que sentido move a velocidade
un final; só teñen sentido cunha velocidade, e **`--speed off` rexéitaas**. Unha marca indicada
coa súa contraria — `--shorten --no-shorten`, `--lengthen --no-lengthen` — tamén se rexeita.

### `--range`

`--range 3-7` limita o axuste aos subtítulos 3 a 7, **contados desde 1, límites incluídos**;
`--range 3-` vai ata o último. Un só subtítulo escríbese `4-4`.

- Unha franxa **que remata antes de comezar**, un `0`, un signo ou un número só
  rexéitanse como erro de uso (código `1`).
- **Vale para todos os ficheiros do lote**, o que só ten sentido se os ficheiros se parecen.
  Xúlgase **fronte a cada ficheiro**: unha franxa que vai máis alá dese ficheiro rexéitase
  **antes de tocalo**, indicando o límite —
  `range 3-9 ends after the last subtitle: the file has 4` —, ese ficheiro falla (código `2` ou `3`, identificador
  `range-out-of-bounds`) e os demais continúan.
- Os recontos do informe só afectan aos subtítulos escollidos.

## O que se sacrifica

Dúas restricións poden contradicirse no mesmo subtítulo: un mínimo de 1,5 s non cabe nos 0,8 s
que deixa o subtítulo seguinte. **A orde decide, e o informe di o que cedeu** — o que Gaupol
non fai: incúmpreas en silencio.

Cada restrición só se nomea se **polo menos un subtítulo non a obtivo**, unha vez aplicadas as
outras. O máximo non ten reconto: aplícase antes da separación, que só pode acurtar, así que
sempre se cumpre. Un mínimo por riba do máximo cede ante o máximo e cóntase como mínimo.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --output ajuste.srt courts.srt; grep -e '-->' ajuste.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --output ajuste.srt courts.srt; grep -e '-->' ajuste.srt
courts.srt: adjusted the durations of 3 subtitles; could not satisfy the minimum duration in 1 subtitle -> ajuste.srt
00:00:00,000 --> 00:00:02,000
00:00:10,000 --> 00:00:14,000
00:00:14,200 --> 00:00:15,000
00:00:15,000 --> 00:00:16,500
```

O terceiro subtítulo quixera rematar aos 15,7 s para durar 1,5 s, pero o cuarto comeza aos
15,0 s: **gaña a separación**, remata aos 15,0 s, e o informe nomea o mínimo.

Desactivar a separación deixa gañar o mínimo — e os dous subtítulos sólapanse:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --gap off --output libre.srt courts.srt; grep -e '-->' libre.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --gap off --output libre.srt courts.srt; grep -e '-->' libre.srt
courts.srt: adjusted the durations of 3 subtitles -> libre.srt
00:00:00,000 --> 00:00:02,000
00:00:10,000 --> 00:00:14,000
00:00:14,200 --> 00:00:15,700
00:00:15,000 --> 00:00:16,500
```

E non se sacrifica nin se axusta nada cando o ficheiro xa cabe nos seus límites:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --speed off --minimum off --output intact.srt courts.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --speed off --minimum off --output intact.srt courts.srt
courts.srt: no duration to adjust -> intact.srt
```

O ficheiro escríbese tal como está: **un destino indicado é un destino escrito**.

## Saída

**Saída estándar** — nada: o resultado é o ficheiro escrito.

| Nivel | O que se engade na saída de erro |
| :----- | :------------------------------------- |
| 1 | `<path>: adjusted the durations of N subtitles; could not satisfy …  -> <destination>`, ou `<path>: no duration to adjust -> <destination>` — e, antes, o ficheiro de configuración lido — `settings: read <path>` — véxase [O ficheiro de configuración](invocation.md#o-ficheiro-de-configuración) |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — o formato, a codificación, a marca e os finais de liña do ficheiro lido, devoltos tal como estaban |
| 3 | `<path>: N bytes read, M written`, e despois **cada diagnóstico de lectura** — véxase [Invocación](invocation.md#diagnósticos-de-lectura) |

A frase é a da xanela: `N` é o número de subtítulos cuxo final se moveu, e o resto, ausente
cando nada cedeu, lista o que non se respectou — `the reading speed`, `the minimum duration`,
`the gap` — co número de subtítulos afectados.

### En JSON

Con [`--format json`](invocation.md#saída-que-pode-ler-un-script), o obxecto de cada ficheiro
leva, ademais do envoltorio:

| Clave | Contido |
| :-- | :------ |
| `counts.subtitles` | os subtítulos **apuntados**: o ficheiro enteiro, ou a franxa |
| `counts.adjusted` | aqueles cuxo final se moveu |
| `counts.sacrificed.speed`, `.minimum`, `.gap` | os subtítulos que seguen sen respectar esta restrición, unha vez aplicadas as outras, **incluídos aqueles cuxo final non se moveu** |
| `constraints.speed` | `{"cps", "lengthen", "shorten"}` — os caracteres por segundo **como cadea** (`"15"`, `"12.5"`) — ou `null` se está desactivada |
| `constraints.minimum_ms`, `.maximum_ms`, `.gap_ms` | as duracións usadas, en **milisegundos enteiros**, ou `null` se están desactivadas |
| `settings` | o ficheiro de configuración que se leu, ou `null` cando non se leu ningún |

`constraints` di **os axustes que se usaron realmente**, valores predeterminados incluídos: así,
dous cálculos sobre os mesmos subtítulos compáranse sen ler a liña de ordes.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config --format json adjust --dry-run --maximum 1.8 courts.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config --format json adjust --dry-run --maximum 1.8 courts.srt
courts.srt: adjusted the durations of 4 subtitles; could not satisfy the reading speed in 1 subtitle, the minimum duration in 1 subtitle (dry run, nothing written)
{"schema":1,"command":"adjust","file":"courts.srt","ok":true,"dry_run":true,"destination":null,"counts":{"subtitles":4,"adjusted":4,"sacrificed":{"speed":1,"minimum":1,"gap":0}},"constraints":{"speed":{"cps":"15","lengthen":true,"shorten":false},"minimum_ms":1500,"maximum_ms":1800,"gap_ms":0},"settings":null,"warnings":[]}
```

## Códigos de saída

Os da ferramenta: `0` se se procesaron todos os ficheiros, `2` se non se procesou ningún, `3`
se só se procesaron algúns, `1` nun erro de uso. Un ficheiro que xa cabe nos seus límites
considérase procesado.

## Erros

| O que o provoca | Mensaxe |
| :------------------ | :------ |
| un ficheiro indicado con `--config` que non existe — o propio ficheiro do usuario, se falta, dá os valores predeterminados | `settings file not found: <FILE>` |
| ningunha restrición activa | `adjust has nothing to adjust to: every constraint is off, or the reading speed is told to move no end` |
| unha velocidade que non o é | `--speed: "…" is not a reading speed: expected a number of characters per second above zero, like 15 or 12.5, or off` |
| unha marca de sentido con `--speed off` | `--lengthen, --no-lengthen, --shorten and --no-shorten say which way the reading speed may move an end, and --speed off switches the reading speed off` |
| unha marca de sentido coa súa contraria | `--shorten and --no-shorten say opposite things; give one of them` (igualmente `--lengthen` e `--no-lengthen`) |
| unha duración que non o é | `--minimum: "…" is not a time: …` (igualmente `--maximum`, `--gap`) |
| duración negativa | `--minimum: "-1" is not a duration of zero or more` (igualmente `--gap`) |
| máximo cero ou negativo | `--maximum: "0" is not a duration above zero` |
| franxa mal escrita | `--range: "…" is not a range: write it N-M, or N- to go to the end, counted from 1 and inclusive` |
| franxa que remata antes de comezar | `--range: "5-3" is not a range: it ends before it starts` |
| franxa fóra do ficheiro | `<path>: range N-M starts after the last subtitle: the file has K`, ou `… ends after …` |

Os erros do destino son comúns ás subordes que escriben:
véxase [Invocación](invocation.md#o-destino).
