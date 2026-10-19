# `adjust`

<!-- machine-translated -->
> Esta página es una traducción automática y todavía no la ha revisado ningún lector del idioma.

```
subedit-cli adjust [--speed CPS|off] [--shorten | --no-shorten] [--lengthen | --no-lengthen]
                   [--minimum TIME|off] [--maximum TIME|off] [--gap TIME|off]
                   [--range N-M|N-]
                   (--output FILE | --output-dir DIR | --in-place)
                   [--dry-run]
                   [--recursive]
                   <file>...
```

Ajusta la **duración** de cada subtítulo a una velocidad de lectura y a unos límites: un
subtítulo demasiado corto para leerse se alarga, un subtítulo que sale de la pantalla
demasiado tarde puede acortarse, y dos subtítulos no se solapan. **Solo se mueve
el fin**: mover el inicio desplazaría el subtítulo, que es el trabajo de
[`shift`](shift.md). Es la operación `Adjust Durations…` de la
[ventana](../subedit-gui/operations.md#adjust-durations), con los mismos valores por defecto.

## Las cuatro restricciones, y su orden

Se aplican **en este orden**, cada una fijando el fin, y la última gana:

1. **la velocidad de lectura**: la duración que necesita el texto, a tantos caracteres por
   segundo (las etiquetas no cuentan, los saltos de línea sí);
2. **el mínimo**: una duración inferior se sube hasta el mínimo;
3. **el máximo**: una duración superior se baja hasta el máximo;
4. **la separación**: el tiempo que se deja antes del subtítulo siguiente. Va la última y
   **gana**: dos subtítulos mostrados a la vez son un defecto que el espectador ve,
   un subtítulo un poco corto no lo es.

El último subtítulo del archivo no tiene siguiente, y por tanto no tiene separación que mantener. El siguiente
de un subtítulo es el del archivo, **lo elija o no** `--range`.

| Restricción | Opción | Por defecto | Desactivarla |
| :--------- | :----- | :----- | :------- |
| velocidad | `--speed CPS` | **15** caracteres por segundo | `--speed off` |
| sentido de la velocidad | `--shorten`, `--no-shorten`, `--lengthen`, `--no-lengthen` | solo **alarga** | — |
| mínimo | `--minimum TIME` | **1,5 s** | `--minimum off` |
| máximo | `--maximum TIME` | **ninguno** | `--maximum off` |
| separación | `--gap TIME` | **0** | `--gap off` |

Son los ajustes de Gaupol y de la primera apertura de la ventana. **`off` desactiva una
restricción: deja de comprobarse.** No es ponerla a cero: un mínimo de `0`
es un mínimo de cero, una separación de `0` prohíbe cualquier solapamiento.

## Lo que aporta el archivo de configuración

**Los valores por defecto anteriores son los integrados.** Por defecto, `adjust` lee la configuración de la operación en
el archivo de configuración (los valores que `Adjust Durations…` recuerda en la ventana, o que
escribió [`config set`](config.md)), y estos ocupan el lugar de los valores por defecto integrados. **Una opción
dada en la línea siempre gana**, después el archivo y después el valor por defecto integrado. Véase
[El archivo de configuración](invocation.md#el-archivo-de-configuración) para saber dónde está el archivo, `--config` y
`--no-config`.

| Clave | Lo que aporta | Opción que la anula |
| :-- | :--------------- | :----------------------- |
| `duration-adjust.speed` | la velocidad de lectura, en caracteres por segundo | `--speed CPS` |
| `duration-adjust.lengthen`, `duration-adjust.shorten` | en qué sentido puede mover la velocidad un fin; las dos a `false` desactivan la velocidad | `--lengthen`, `--no-lengthen`, `--shorten`, `--no-shorten`, `--speed off`; dar `--speed CPS` o un sentido vuelve a activar una velocidad que el archivo desactivó, alargando |
| `duration-adjust.minimum-enabled`, `duration-adjust.minimum-ms` | la duración más corta, o ninguna | `--minimum TIME`, `--minimum off` |
| `duration-adjust.maximum-enabled`, `duration-adjust.maximum-ms` | la duración más larga, o ninguna | `--maximum TIME`, `--maximum off` |
| `duration-adjust.gap-enabled`, `duration-adjust.gap-ms` | la separación mínima, o ninguna | `--gap TIME`, `--gap off` |

**Cada clave tiene una opción que la deshace.** Cada sentido tiene un par de indicadores que lo fijan
diga lo que diga el archivo: `--shorten` y `--no-shorten` para acortar, `--lengthen` y
`--no-lengthen` para alargar; un indicador no dado deja el sentido del archivo como está.
`--maximum off` quita un máximo que el archivo activa. Con `--format json`, `constraints` dice
los ajustes realmente usados, vengan de donde vengan, y `settings` el archivo que se leyó:

<!-- example: printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine/settings.conf config set duration-adjust.speed 12 2>/dev/null; subedit-cli --config mine/settings.conf --format json adjust --dry-run a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine/settings.conf config set duration-adjust.speed 12 2>/dev/null; subedit-cli --config mine/settings.conf --format json adjust --dry-run a.srt
settings: read mine/settings.conf
a.srt: adjusted the durations of 1 subtitle (dry run, nothing written)
{"schema":1,"command":"adjust","file":"a.srt","ok":true,"dry_run":true,"destination":null,"counts":{"subtitles":1,"adjusted":1,"sacrificed":{"speed":0,"minimum":0,"gap":0}},"constraints":{"speed":{"cps":"12","lengthen":true,"shorten":false},"minimum_ms":1500,"maximum_ms":null,"gap_ms":0},"settings":"mine/settings.conf","warnings":[]}
```

Los demás ejemplos de esta página se ejecutan con `--no-config`, para que digan lo que hacen los
valores por defecto integrados, sea cual sea la configuración de la máquina que los ejecuta.

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

## Argumentos y opciones

| Opción | Obligatoria | Valores | Por defecto |
| :----- | :----- | :------ | :----- |
| `<file>...` | sí | una o varias rutas | — |
| `--recursive`, `-r` | no | un indicador | desactivado: véase [Procesar un árbol](lots.md) |
| `--speed` | no | un número de caracteres por segundo, **mayor que cero** (`15`, `12.5`), u `off` | `15` |
| `--shorten` | no | un indicador: la velocidad también puede **adelantar** un fin | desactivado |
| `--no-shorten` | no | un indicador: la velocidad ya no **adelanta** un fin, diga lo que diga el archivo de configuración | desactivado |
| `--lengthen` | no | un indicador: la velocidad **retrasa** un fin, diga lo que diga el archivo de configuración | desactivado |
| `--no-lengthen` | no | un indicador: la velocidad ya no **retrasa** un fin | desactivado |
| `--minimum` | no | una duración de cero o más, u `off` | `1.5` |
| `--maximum` | no | una duración **mayor que cero**, u `off` | ninguno |
| `--gap` | no | una duración de cero o más, u `off` | `0` |
| `--range` | no | `N-M`, o `N-` hasta el final; véase más abajo | todo el archivo |
| `--output` / `--output-dir` / `--in-place` | **una de las tres**, salvo con `--dry-run` | véase [Invocación](invocation.md#el-destino) | — |
| `--dry-run` | no | un indicador: calcula y dice, no escribe nada; véase [Ver antes de escribir](invocation.md#ver-antes-de-escribir) | desactivado |

Una duración se escribe como para [`shift`](shift.md#cómo-se-escribe-una-duración): `2.5`, `00:00:02.500`.

**Sin ninguna restricción activa, `adjust` se rechaza** (código `1`, no se escribe nada): no
habría nada a lo que ajustarse, y un comando que no hace nada diciendo que ha tenido éxito es la peor
respuesta. Es el caso de `--speed off --minimum off --gap off` sin máximo, y de
`--no-lengthen` solo con el mínimo y la separación desactivados: una velocidad que no mueve ningún
fin no restringe nada.

`--shorten`, `--no-shorten`, `--lengthen` y `--no-lengthen` dicen en qué sentido mueve la velocidad un
fin; solo tienen sentido con una velocidad, y **`--speed off` los rechaza**. Un indicador dado con su
contrario (`--shorten --no-shorten`, `--lengthen --no-lengthen`) también se rechaza.

### `--range`

`--range 3-7` limita el ajuste a los subtítulos 3 a 7, **contados desde 1, límites
incluidos**; `--range 3-` llega hasta el último. Un solo subtítulo se escribe `4-4`.

- Un intervalo **que termina antes de empezar**, un `0`, un signo o un número suelto se
  rechazan como error de uso (código `1`).
- **Vale para todos los archivos del lote**, lo que solo tiene sentido si los archivos
  se parecen. Se juzga **frente a cada archivo**: un intervalo que va más allá de ese archivo se
  rechaza **antes de tocarlo**, nombrando el límite
  (`range 3-9 ends after the last subtitle: the file has 4`), ese archivo falla (código `2` o `3`, identificador
  `range-out-of-bounds`) y los demás siguen.
- Los recuentos del informe solo se refieren a los subtítulos elegidos.

## Lo que se sacrifica

Dos restricciones pueden contradecirse en el mismo subtítulo: un mínimo de 1,5 s no cabe
en los 0,8 s que deja el subtítulo siguiente. **El orden decide, y el informe dice
qué cedió**, que es lo que Gaupol no hace: las incumple en silencio.

Cada restricción solo se nombra si **al menos un subtítulo no la obtuvo**, una vez
aplicadas las demás. El máximo no tiene recuento: se aplica antes de la separación, que solo
puede acortar, así que siempre se cumple. Un mínimo superior al máximo cede ante el máximo y se
cuenta como mínimo.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --output ajuste.srt courts.srt; grep -e '-->' ajuste.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --output ajuste.srt courts.srt; grep -e '-->' ajuste.srt
courts.srt: adjusted the durations of 3 subtitles; could not satisfy the minimum duration in 1 subtitle -> ajuste.srt
00:00:00,000 --> 00:00:02,000
00:00:10,000 --> 00:00:14,000
00:00:14,200 --> 00:00:15,000
00:00:15,000 --> 00:00:16,500
```

El tercer subtítulo querría terminar a los 15,7 s para durar 1,5 s, pero el cuarto empieza a los
15,0 s: **la separación gana**, termina a los 15,0 s, y el informe nombra el mínimo.

Desactivar la separación deja ganar al mínimo, y los dos subtítulos se solapan:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --gap off --output libre.srt courts.srt; grep -e '-->' libre.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --gap off --output libre.srt courts.srt; grep -e '-->' libre.srt
courts.srt: adjusted the durations of 3 subtitles -> libre.srt
00:00:00,000 --> 00:00:02,000
00:00:10,000 --> 00:00:14,000
00:00:14,200 --> 00:00:15,700
00:00:15,000 --> 00:00:16,500
```

Y no se sacrifica ni se ajusta nada cuando el archivo ya cabe dentro de sus límites:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --speed off --minimum off --output intact.srt courts.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --speed off --minimum off --output intact.srt courts.srt
courts.srt: no duration to adjust -> intact.srt
```

El archivo se escribe tal cual: **un destino dado es un destino escrito**.

## Salida

**Salida estándar**: nada, el resultado es el archivo escrito.

| Nivel | Lo que se añade en la salida de error |
| :----- | :------------------------------------- |
| 1 | `<path>: adjusted the durations of N subtitles; could not satisfy …  -> <destination>`, o `<path>: no duration to adjust -> <destination>` y, en primer lugar, el archivo de configuración leído (`settings: read <path>`); véase [El archivo de configuración](invocation.md#el-archivo-de-configuración) |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept`: el formato, la codificación, la marca y los finales de línea del archivo leído, restituidos tal como estaban |
| 3 | `<path>: N bytes read, M written`, y después **cada diagnóstico de lectura**; véase [Invocación](invocation.md#diagnósticos-de-lectura) |

La frase es la de la ventana: `N` es el número de subtítulos cuyo fin se movió, y el
resto, ausente cuando nada cedió, enumera lo que no se mantuvo (`the reading speed`,
`the minimum duration`, `the gap`) con el número de subtítulos afectados.

### En JSON

Con [`--format json`](invocation.md#una-salida-que-un-script-puede-leer), el objeto de cada archivo
lleva, además del envoltorio:

| Clave | Contenido |
| :-- | :------ |
| `counts.subtitles` | los subtítulos **a los que se apunta**: todo el archivo, o el intervalo |
| `counts.adjusted` | aquellos cuyo fin se movió |
| `counts.sacrificed.speed`, `.minimum`, `.gap` | los subtítulos que siguen sin cumplir esta restricción, una vez aplicadas las demás, **incluidos aquellos cuyo fin no se movió** |
| `constraints.speed` | `{"cps", "lengthen", "shorten"}`: los caracteres por segundo **como cadena** (`"15"`, `"12.5"`), o `null` si está desactivada |
| `constraints.minimum_ms`, `.maximum_ms`, `.gap_ms` | las duraciones usadas, en **milisegundos enteros**, o `null` si están desactivadas |
| `settings` | el archivo de configuración que se leyó, o `null` cuando no se leyó ninguno |

`constraints` dice **los ajustes realmente usados**, valores por defecto incluidos: dos cálculos
sobre los mismos subtítulos se comparan así sin leer la línea de comandos.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config --format json adjust --dry-run --maximum 1.8 courts.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config --format json adjust --dry-run --maximum 1.8 courts.srt
courts.srt: adjusted the durations of 4 subtitles; could not satisfy the reading speed in 1 subtitle, the minimum duration in 1 subtitle (dry run, nothing written)
{"schema":1,"command":"adjust","file":"courts.srt","ok":true,"dry_run":true,"destination":null,"counts":{"subtitles":4,"adjusted":4,"sacrificed":{"speed":1,"minimum":1,"gap":0}},"constraints":{"speed":{"cps":"15","lengthen":true,"shorten":false},"minimum_ms":1500,"maximum_ms":1800,"gap_ms":0},"settings":null,"warnings":[]}
```

## Códigos de salida

Los de la herramienta: `0` si se procesaron todos los archivos, `2` si ninguno, `3` si solo
algunos, `1` en un error de uso. Un archivo que ya cabe dentro de sus límites se considera procesado.

## Errores

| Qué lo provoca | Mensaje |
| :------------------ | :------ |
| un archivo nombrado por `--config` que no existe (el archivo propio del usuario, si falta, da los valores por defecto) | `settings file not found: <FILE>` |
| ninguna restricción activa | `adjust has nothing to adjust to: every constraint is off, or the reading speed is told to move no end` |
| una velocidad que no lo es | `--speed: "…" is not a reading speed: expected a number of characters per second above zero, like 15 or 12.5, or off` |
| un indicador de sentido con `--speed off` | `--lengthen, --no-lengthen, --shorten and --no-shorten say which way the reading speed may move an end, and --speed off switches the reading speed off` |
| un indicador de sentido con su contrario | `--shorten and --no-shorten say opposite things; give one of them` (igualmente `--lengthen` y `--no-lengthen`) |
| una duración que no lo es | `--minimum: "…" is not a time: …` (igualmente `--maximum`, `--gap`) |
| duración negativa | `--minimum: "-1" is not a duration of zero or more` (igualmente `--gap`) |
| máximo nulo o negativo | `--maximum: "0" is not a duration above zero` |
| intervalo mal escrito | `--range: "…" is not a range: write it N-M, or N- to go to the end, counted from 1 and inclusive` |
| intervalo que termina antes de empezar | `--range: "5-3" is not a range: it ends before it starts` |
| intervalo fuera del archivo | `<path>: range N-M starts after the last subtitle: the file has K`, o `… ends after …` |

Los errores del destino son comunes a los subcomandos que escriben:
véase [Invocación](invocation.md#el-destino).
