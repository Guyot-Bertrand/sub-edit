# `adjust`

<!-- machine-translated -->
> Ca págine es un traduction machinal e it ancor ne ha esset revidet de un lector del lingue.

```
subedit-cli adjust [--speed CPS|off] [--shorten | --no-shorten] [--lengthen | --no-lengthen]
                   [--minimum TIME|off] [--maximum TIME|off] [--gap TIME|off]
                   [--range N-M|N-]
                   (--output FILE | --output-dir DIR | --in-place)
                   [--dry-run]
                   [--recursive]
                   <file>...
```

Ajusta li **duration** de chascun subtitul a un rapidesse de lectura e a limites: un
subtitul tro curt por esser leet es allongat, un subtitul quel quita li ecran
tro tard posse esser curtat, e du subtitules ne superpone se. **Solmen li
fin move se**: mover li comense moverea li subtitul, quo es li labor de
[`shift`](shift.md). It es li operation `Adjust Durations…` del
[fenestre](../subedit-gui/operations.md#adjust-durations), con li sam valores predefinit.

## Li quar restrictiones, e lor órdine

Les aplica se **in ci órdine**, chascun fixante li fin, e li ultim prevalente:

1. **li rapidesse de lectura** — li duration quel li textu besona, a tal númere de characteres per
   second (li etiquettes ne cunta, li ruptures de linea cunta);
2. **li minimum** — un duration infra it es portat til li minimum;
3. **li maximum** — un duration supra it es portat til li maximum;
4. **li interstice** — li tempore lassat ante li subtitul sequent. It veni ultim e
   **it prevale**: du subtitules monstrat together es un erra quel li spectator vide,
   un subtitul un poc curt ne.

Li ultim subtitul del file ne ha sequent, e dunc null interstice a conservar. Li sequent
de un subtitul es celi in li file, **selectet o ne** per `--range`.

| Restriction | Option | Predefinit | Desactivar it |
| :--------- | :----- | :----- | :------- |
| rapidesse | `--speed CPS` | **15** characteres per second | `--speed off` |
| direction del rapidesse | `--shorten`, `--no-shorten`, `--lengthen`, `--no-lengthen` | it solmen **allonga** | — |
| minimum | `--minimum TIME` | **1,5 s** | `--minimum off` |
| maximum | `--maximum TIME` | **null** | `--maximum off` |
| interstice | `--gap TIME` | **0** | `--gap off` |

Ti es li parametres de Gaupol e del unesim apertion del fenestre. **`off` desactiva un
restriction: it ne es plu controlat.** To ne es metter it a zero — un minimum de `0`
es un minimum de zero, un interstice de `0` interdi omni superposition.

## Quo li file de configuration provide

**Li valores predefinit ci-supra es celes integrat.** Per predefinit `adjust` lee li parametres de operation del
file de configuration — li valores quel `Adjust Durations…` memora in li fenestre, o quel
[`config set`](config.md) ha scrit — e les prende li loc del valores predefinit integrat. **Un option
dat sur li linea sempre prevale**, poy li file, poy li valore predefinit integrat. Vide
[Li file de configuration](invocation.md#li-file-de-configuration) por saver u li file es, `--config` e
`--no-config`.

| Clave | Quo it provide | Option quel it supplanta |
| :-- | :--------------- | :----------------------- |
| `duration-adjust.speed` | li rapidesse de lectura, in characteres per second | `--speed CPS` |
| `duration-adjust.lengthen`, `duration-adjust.shorten` | in qual direction li rapidesse posse mover un fin; si ambi es `false`, li rapidesse es desactivat | `--lengthen`, `--no-lengthen`, `--shorten`, `--no-shorten`, `--speed off`; dar `--speed CPS` o un direction reactiva un rapidesse quel li file desactivat, con allongation |
| `duration-adjust.minimum-enabled`, `duration-adjust.minimum-ms` | li duration li plu curt, o null | `--minimum TIME`, `--minimum off` |
| `duration-adjust.maximum-enabled`, `duration-adjust.maximum-ms` | li duration li plu long, o null | `--maximum TIME`, `--maximum off` |
| `duration-adjust.gap-enabled`, `duration-adjust.gap-ms` | li interstice minimal, o null | `--gap TIME`, `--gap off` |

**Chascun clave have un option quel li anulla.** Chascun direction have un par de flags quel
li fixa independentmen de quo li file di: `--shorten` e `--no-shorten` por curtar, `--lengthen` e
`--no-lengthen` por allongar; un flag ne dat lassa li direction del file quam it es.
`--maximum off` omisse un maximum quel li file activa. Con `--format json`, `constraints` di
li parametres realmen usat, ex u que ili veni, e `settings` li file quel esset leet:

<!-- example: printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine/settings.conf config set duration-adjust.speed 12 2>/dev/null; subedit-cli --config mine/settings.conf --format json adjust --dry-run a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine/settings.conf config set duration-adjust.speed 12 2>/dev/null; subedit-cli --config mine/settings.conf --format json adjust --dry-run a.srt
settings: read mine/settings.conf
a.srt: adjusted the durations of 1 subtitle (dry run, nothing written)
{"schema":1,"command":"adjust","file":"a.srt","ok":true,"dry_run":true,"destination":null,"counts":{"subtitles":1,"adjusted":1,"sacrificed":{"speed":0,"minimum":0,"gap":0}},"constraints":{"speed":{"cps":"12","lengthen":true,"shorten":false},"minimum_ms":1500,"maximum_ms":null,"gap_ms":0},"settings":"mine/settings.conf","warnings":[]}
```

Li altri exemples de ca págine es executet con `--no-config`, por que ili di quo li valores
predefinit integrat fa, independentmen del parametres del machine quel les executa.

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

## Argumentes e optiones

| Option | Obligatori | Valores | Predefinit |
| :----- | :----- | :------ | :----- |
| `<file>...` | yes | un o plu vias | — |
| `--recursive`, `-r` | no | un flag | off — vide [Tractar un arbore](lots.md) |
| `--speed` | no | un númere de characteres per second, **supra zero** (`15`, `12.5`), o `off` | `15` |
| `--shorten` | no | un flag: li rapidesse posse anc **portar** un fin **plu tosc** | off |
| `--no-shorten` | no | un flag: li rapidesse ne **porta** plu un fin **plu tosc**, independentmen del file de configuration | off |
| `--lengthen` | no | un flag: li rapidesse **move** un fin **plu tard**, independentmen del file de configuration | off |
| `--no-lengthen` | no | un flag: li rapidesse ne **move** plu un fin **plu tard** | off |
| `--minimum` | no | un duration de zero o plu, o `off` | `1.5` |
| `--maximum` | no | un duration **supra zero**, o `off` | null |
| `--gap` | no | un duration de zero o plu, o `off` | `0` |
| `--range` | no | `N-M`, o `N-` til li fin — vide infra | li tot file |
| `--output` / `--output-dir` / `--in-place` | **un del tri**, except con `--dry-run` | vide [Invocation](invocation.md#li-destination) | — |
| `--dry-run` | no | un flag: calcula e di, scri nullcos — vide [Vider ante scrir](invocation.md#vider-ante-scrir) | off |

Un duration es scrit quam por [`shift`](shift.md#qualmen-scrir-un-duration): `2.5`, `00:00:02.500`.

**Sin restriction activ, `adjust` es refusat** (code `1`, nullcos scrit): ne haveria quo ajustar a, e un comande quel fa nullcos dum dir que it ha successet es li pejor
respondition. Tal es li casu de `--speed off --minimum off --gap off` sin maximum, e de
`--no-lengthen` sol con li minimum e li interstice desactivat: un rapidesse quel move null
fin ne restringe nullcos.

`--shorten`, `--no-shorten`, `--lengthen` e `--no-lengthen` di in qual direction li rapidesse move un
fin; ili have sense solmen con un rapidesse, e **`--speed off` les refusa**. Un flag dat con su
contrari — `--shorten --no-shorten`, `--lengthen --no-lengthen` — es anc refusat.

### `--range`

`--range 3-7` limita li ajustage al subtitules 3 til 7, **contat ex 1, limites
includet**; `--range 3-` va til li ultim. Un sol subtitul es scrit `4-4`.

- Un intervalle **quel fini ante su comense**, un `0`, un signe o un sol númere es
  refusat quam error de usage (code `1`).
- **It vale por chascun file del lot**, quo have sense solmen si li files
  es simil. It es judicat **contra chascun file**: un intervalle quel passa li fin de ti file es
  refusat **ante que it es tuchat**, nominant li limite —
  `range 3-9 ends after the last subtitle: the file has 4` —, ti file falie (code `2` o `3`, identificator
  `range-out-of-bounds`) e li altres continua.
- Li cuntages del raporte concerne solmen li subtitules selectet.

## Quo es sacrificat

Du restrictiones posse contradir se sur li sam subtitul: un minimum de 1,5 s ne intra
in li 0,8 s quel li subtitul sequent lassa. **Li órdine decide, e li raporte di
quo ha cedet** — quo Gaupol ne fa: it les rupte silentiosimen.

Chascun restriction es nominat solmen si **al minus un subtitul ne l'ha obtenet**, pos que
li altres es aplicat. Li maximum have null cuntage: it es aplicat ante li interstice, quel posse
solmen curtar, dunc it sempre es respectat. Un minimum supra li maximum cede al maximum e
es contat quam minimum.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --output ajuste.srt courts.srt; grep -e '-->' ajuste.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --output ajuste.srt courts.srt; grep -e '-->' ajuste.srt
courts.srt: adjusted the durations of 3 subtitles; could not satisfy the minimum duration in 1 subtitle -> ajuste.srt
00:00:00,000 --> 00:00:02,000
00:00:10,000 --> 00:00:14,000
00:00:14,200 --> 00:00:15,000
00:00:15,000 --> 00:00:16,500
```

Li tertie subtitul vole finir a 15,7 s por durar 1,5 s, ma li quartesim comensa a
15,0 s: **li interstice prevale**, it fini a 15,0 s, e li raporte nómina li minimum.

Desactivar li interstice lassa li minimum prevaler — e li du subtitules superpone se:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --gap off --output libre.srt courts.srt; grep -e '-->' libre.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --gap off --output libre.srt courts.srt; grep -e '-->' libre.srt
courts.srt: adjusted the durations of 3 subtitles -> libre.srt
00:00:00,000 --> 00:00:02,000
00:00:10,000 --> 00:00:14,000
00:00:14,200 --> 00:00:15,700
00:00:15,000 --> 00:00:16,500
```

E nullcos es sacrificat o ajustat quande li file ja intra in su limites:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --speed off --minimum off --output intact.srt courts.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --speed off --minimum off --output intact.srt courts.srt
courts.srt: no duration to adjust -> intact.srt
```

Li file es scrit quam it es: **un destination dat es un destination scrit**.

## Output

**Output standard** — nullcos: li resultate es li file scrit.

| Nivelle | Quo es adjuntet al output de error |
| :----- | :------------------------------------- |
| 1 | `<path>: adjusted the durations of N subtitles; could not satisfy …  -> <destination>`, o `<path>: no duration to adjust -> <destination>` — e, unesim, li file de configuration leet — `settings: read <path>` — vide [Li file de configuration](invocation.md#li-file-de-configuration) |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — li formate, li codification, li marca e li fines de linea del file leet, remisset quam ili esset |
| 3 | `<path>: N bytes read, M written`, poy **chascun diagnose de lectura** — vide [Invocation](invocation.md#diagnoses-de-lectura) |

Li frase es celi del fenestre: `N` es li númere de subtitules quel fin ha movet, e li
resta, absent quande nullcos ha cedet, lista quo ne esset conservat — `the reading speed`,
`the minimum duration`, `the gap` — con li númere de subtitules concernit.

### In JSON

Con [`--format json`](invocation.md#un-output-quel-un-script-posse-leer), li objecte de chascun file
porta, in plu del inveloppe:

| Clave | Contenete |
| :-- | :------ |
| `counts.subtitles` | li subtitules **cielat**: li tot file, o li intervalle |
| `counts.adjusted` | celes queles fin ha movet |
| `counts.sacrificed.speed`, `.minimum`, `.gap` | li subtitules queles ancor ne respecta ti restriction, pos que li altres es aplicat, **includent celes queles fin ne ha movet** |
| `constraints.speed` | `{"cps", "lengthen", "shorten"}` — li characteres per second **quam textu** (`"15"`, `"12.5"`) — o `null` si desactivat |
| `constraints.minimum_ms`, `.maximum_ms`, `.gap_ms` | li durationes usat, in **millisecondes integri**, o `null` si desactivat |
| `settings` | li file de configuration quel esset leet, o `null` si null file esset leet |

`constraints` di **li parametres realmen usat**, valores predefinit includet: du calculationes
pri li sam subtitules dunc compara se sin leer li linea de comande.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config --format json adjust --dry-run --maximum 1.8 courts.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config --format json adjust --dry-run --maximum 1.8 courts.srt
courts.srt: adjusted the durations of 4 subtitles; could not satisfy the reading speed in 1 subtitle, the minimum duration in 1 subtitle (dry run, nothing written)
{"schema":1,"command":"adjust","file":"courts.srt","ok":true,"dry_run":true,"destination":null,"counts":{"subtitles":4,"adjusted":4,"sacrificed":{"speed":1,"minimum":1,"gap":0}},"constraints":{"speed":{"cps":"15","lengthen":true,"shorten":false},"minimum_ms":1500,"maximum_ms":1800,"gap_ms":0},"settings":null,"warnings":[]}
```

## Codes de exeunte

Celes del instrument: `0` si chascun file esset tractat, `2` si null, `3` si solmen
alcunes, `1` in cas de error de usage. Un file quel ja intra in su limites es tractat.

## Errores

| Quo it causa | Missage |
| :------------------ | :------ |
| un file nominat per `--config` quel ne existe — li propri file del usator, mancant, dona li valores predefinit | `settings file not found: <FILE>` |
| null restriction activ | `adjust has nothing to adjust to: every constraint is off, or the reading speed is told to move no end` |
| un rapidesse quel ne es un | `--speed: "…" is not a reading speed: expected a number of characters per second above zero, like 15 or 12.5, or off` |
| un flag de direction con `--speed off` | `--lengthen, --no-lengthen, --shorten and --no-shorten say which way the reading speed may move an end, and --speed off switches the reading speed off` |
| un flag de direction con su contrari | `--shorten and --no-shorten say opposite things; give one of them` (anc por `--lengthen` e `--no-lengthen`) |
| un duration quel ne es un | `--minimum: "…" is not a time: …` (anc por `--maximum`, `--gap`) |
| duration negativ | `--minimum: "-1" is not a duration of zero or more` (anc por `--gap`) |
| maximum zero o negativ | `--maximum: "0" is not a duration above zero` |
| intervalle mal scrit | `--range: "…" is not a range: write it N-M, or N- to go to the end, counted from 1 and inclusive` |
| intervalle quel fini ante su comense | `--range: "5-3" is not a range: it ends before it starts` |
| intervalle foris li file | `<path>: range N-M starts after the last subtitle: the file has K`, o `… ends after …` |

Li errores del destination es comun al subcomandes quel scri:
vide [Invocation](invocation.md#li-destination).
