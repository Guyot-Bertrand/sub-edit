# `adjust`

<!-- machine-translated -->
> Aquesta pàgina és una traducció automàtica i encara no l'ha revisada cap lector de la llengua.

```
subedit-cli adjust [--speed CPS|off] [--shorten | --no-shorten] [--lengthen | --no-lengthen]
                   [--minimum TIME|off] [--maximum TIME|off] [--gap TIME|off]
                   [--range N-M|N-]
                   (--output FILE | --output-dir DIR | --in-place)
                   [--dry-run]
                   [--recursive]
                   <file>...
```

Ajusta la **durada** de cada subtítol a una velocitat de lectura i a uns límits: un
subtítol massa curt per ser llegit s'allarga, un subtítol que surt de la pantalla
massa tard es pot escurçar, i dos subtítols no s'encavalquen. **Només es mou
el final**: moure el començament mouria el subtítol, que és la feina de
[`shift`](shift.md). És l'operació `Adjust Durations…` de la
[finestra](../subedit-gui/operations.md#adjust-durations), amb els mateixos valors per defecte.

## Les quatre restriccions, i el seu ordre

S'apliquen **en aquest ordre**, cadascuna fixant el final, i l'última guanya:

1. **la velocitat de lectura** — la durada que necessita el text, a tants caràcters per
   segon (les etiquetes no compten, els salts de línia sí);
2. **el mínim** — una durada inferior es porta fins al mínim;
3. **el màxim** — una durada superior es redueix fins al màxim;
4. **l'interval** — el temps que queda abans del subtítol següent. Va l'últim i
   **guanya**: dos subtítols mostrats alhora són un defecte que l'espectador veu,
   un subtítol una mica curt no ho és.

L'últim subtítol del fitxer no té següent, i per tant cap interval per mantenir. El següent
d'un subtítol és el del fitxer, **triat o no** per `--range`.

| Restricció | Opció | Per defecte | Desactivar-la |
| :--------- | :----- | :----- | :------- |
| velocitat | `--speed CPS` | **15** caràcters per segon | `--speed off` |
| direcció de la velocitat | `--shorten`, `--no-shorten`, `--lengthen`, `--no-lengthen` | només **allarga** | — |
| mínim | `--minimum TIME` | **1,5 s** | `--minimum off` |
| màxim | `--maximum TIME` | **cap** | `--maximum off` |
| interval | `--gap TIME` | **0** | `--gap off` |

Aquests són els paràmetres de Gaupol i de la primera obertura de la finestra. **`off` desactiva una
restricció: ja no es comprova.** Això no és posar-la a zero — un mínim de `0`
és un mínim de zero, un interval de `0` prohibeix qualsevol encavalcament.

## Què aporta el fitxer de configuració

**Els valors per defecte anteriors són els integrats.** Per defecte `adjust` llegeix la configuració de les operacions
del fitxer de configuració — els valors que `Adjust Durations…` recorda a la finestra, o que
[`config set`](config.md) va escriure — i aquests ocupen el lloc dels valors per defecte integrats. **Una opció
donada a la línia sempre guanya**; després el fitxer, i després el valor per defecte integrat. Vegeu
[El fitxer de configuració](invocation.md#el-fitxer-de-configuració) per saber on és el fitxer, `--config` i
`--no-config`.

| Clau | Què aporta | Opció que la substitueix |
| :-- | :--------------- | :----------------------- |
| `duration-adjust.speed` | la velocitat de lectura, en caràcters per segon | `--speed CPS` |
| `duration-adjust.lengthen`, `duration-adjust.shorten` | en quin sentit la velocitat pot moure un final; totes dues a `false` desactiven la velocitat | `--lengthen`, `--no-lengthen`, `--shorten`, `--no-shorten`, `--speed off`; donar `--speed CPS` o una direcció torna a activar una velocitat que el fitxer havia desactivat, en el sentit d'allargar |
| `duration-adjust.minimum-enabled`, `duration-adjust.minimum-ms` | la durada més curta, o cap | `--minimum TIME`, `--minimum off` |
| `duration-adjust.maximum-enabled`, `duration-adjust.maximum-ms` | la durada més llarga, o cap | `--maximum TIME`, `--maximum off` |
| `duration-adjust.gap-enabled`, `duration-adjust.gap-ms` | l'interval mínim, o cap | `--gap TIME`, `--gap off` |

**Cada clau té una opció que l'anul·la.** Cada direcció té un parell d'indicadors que la fixen
digui el que digui el fitxer: `--shorten` i `--no-shorten` per escurçar, `--lengthen` i
`--no-lengthen` per allargar; un indicador no donat deixa la direcció del fitxer tal com és.
`--maximum off` elimina un màxim que el fitxer activa. Amb `--format json`, `constraints` diu
els paràmetres realment usats, vinguin d'on vinguin, i `settings` el fitxer que s'ha llegit:

<!-- example: printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine/settings.conf config set duration-adjust.speed 12 2>/dev/null; subedit-cli --config mine/settings.conf --format json adjust --dry-run a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine/settings.conf config set duration-adjust.speed 12 2>/dev/null; subedit-cli --config mine/settings.conf --format json adjust --dry-run a.srt
settings: read mine/settings.conf
a.srt: adjusted the durations of 1 subtitle (dry run, nothing written)
{"schema":1,"command":"adjust","file":"a.srt","ok":true,"dry_run":true,"destination":null,"counts":{"subtitles":1,"adjusted":1,"sacrificed":{"speed":0,"minimum":0,"gap":0}},"constraints":{"speed":{"cps":"12","lengthen":true,"shorten":false},"minimum_ms":1500,"maximum_ms":null,"gap_ms":0},"settings":"mine/settings.conf","warnings":[]}
```

Els altres exemples d'aquesta pàgina s'executen amb `--no-config`, perquè diguin què fan els valors
per defecte integrats, sigui quina sigui la configuració de la màquina que els executa.

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

## Arguments i opcions

| Opció | Obligatori | Valors | Per defecte |
| :----- | :----- | :------ | :----- |
| `<file>...` | sí | un o més camins | — |
| `--recursive`, `-r` | no | un indicador | desactivat — vegeu [Tractar un arbre](lots.md) |
| `--speed` | no | un nombre de caràcters per segon, **més gran que zero** (`15`, `12.5`), o `off` | `15` |
| `--shorten` | no | un indicador: la velocitat també pot **avançar** un final | desactivat |
| `--no-shorten` | no | un indicador: la velocitat ja no **avança** cap final, digui el que digui el fitxer de configuració | desactivat |
| `--lengthen` | no | un indicador: la velocitat **endarrereix** un final, digui el que digui el fitxer de configuració | desactivat |
| `--no-lengthen` | no | un indicador: la velocitat ja no **endarrereix** cap final | desactivat |
| `--minimum` | no | una durada de zero o més, o `off` | `1.5` |
| `--maximum` | no | una durada **més gran que zero**, o `off` | cap |
| `--gap` | no | una durada de zero o més, o `off` | `0` |
| `--range` | no | `N-M`, o `N-` fins al final — vegeu més avall | tot el fitxer |
| `--output` / `--output-dir` / `--in-place` | **una de les tres**, excepte amb `--dry-run` | vegeu [Invocació](invocation.md#la-destinació) | — |
| `--dry-run` | no | un indicador: calcula i diu, no escriu res — vegeu [Veure abans d'escriure](invocation.md#veure-abans-descriure) | desactivat |

Una durada s'escriu com per a [`shift`](shift.md#escriure-una-durada): `2.5`, `00:00:02.500`.

**Sense cap restricció activa, `adjust` es refusa** (codi `1`, no s'escriu res): no hi hauria
res a què ajustar-se, i una ordre que no fa res mentre diu que ha tingut èxit és la pitjor
resposta. És el cas de `--speed off --minimum off --gap off` sense màxim, i de
`--no-lengthen` sol amb el mínim i l'interval desactivats: una velocitat que no mou cap
final no restringeix res.

`--shorten`, `--no-shorten`, `--lengthen` i `--no-lengthen` diuen en quin sentit la velocitat mou un
final; només tenen sentit amb una velocitat, i **`--speed off` els refusa**. Un indicador donat amb el seu
contrari — `--shorten --no-shorten`, `--lengthen --no-lengthen` — també es refusa.

### `--range`

`--range 3-7` limita l'ajust als subtítols del 3 al 7, **comptats a partir d'1, límits
inclosos**; `--range 3-` va fins a l'últim. Un sol subtítol s'escriu `4-4`.

- Una franja **que acaba abans de començar**, un `0`, un signe o un nombre sol es
  refusen com a error d'ús (codi `1`).
- **Val per a tots els fitxers del lot**, cosa que només té sentit si els fitxers
  s'assemblen. Es jutja **respecte de cada fitxer**: una franja que va més enllà d'aquest fitxer es
  refusa **abans de tocar-lo**, anomenant el límit —
  `range 3-9 ends after the last subtitle: the file has 4` —, aquest fitxer falla (codi `2` o `3`, identificador
  `range-out-of-bounds`) i els altres continuen.
- Els recomptes de l'informe només afecten els subtítols triats.

## Què se sacrifica

Dues restriccions es poden contradir sobre un mateix subtítol: un mínim d'1,5 s no cap
en els 0,8 s que deixa el subtítol següent. **L'ordre decideix, i l'informe diu
què ha cedit** — cosa que Gaupol no fa: les trenca en silenci.

Cada restricció només s'anomena si **almenys un subtítol no l'ha obtinguda**, un cop
aplicades les altres. El màxim no té recompte: s'aplica abans de l'interval, que només
pot escurçar, de manera que sempre es compleix. Un mínim superior al màxim cedeix davant del màxim i
es compta com a mínim.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --output ajuste.srt courts.srt; grep -e '-->' ajuste.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --output ajuste.srt courts.srt; grep -e '-->' ajuste.srt
courts.srt: adjusted the durations of 3 subtitles; could not satisfy the minimum duration in 1 subtitle -> ajuste.srt
00:00:00,000 --> 00:00:02,000
00:00:10,000 --> 00:00:14,000
00:00:14,200 --> 00:00:15,000
00:00:15,000 --> 00:00:16,500
```

El tercer subtítol voldria acabar a 15,7 s per durar 1,5 s, però el quart comença a
15,0 s: **l'interval guanya**, acaba a 15,0 s, i l'informe anomena el mínim.

Desactivar l'interval deixa guanyar el mínim — i els dos subtítols s'encavalquen:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --gap off --output libre.srt courts.srt; grep -e '-->' libre.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --gap off --output libre.srt courts.srt; grep -e '-->' libre.srt
courts.srt: adjusted the durations of 3 subtitles -> libre.srt
00:00:00,000 --> 00:00:02,000
00:00:10,000 --> 00:00:14,000
00:00:14,200 --> 00:00:15,700
00:00:15,000 --> 00:00:16,500
```

I no se sacrifica ni s'ajusta res quan el fitxer ja és dins dels seus límits:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --speed off --minimum off --output intact.srt courts.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --speed off --minimum off --output intact.srt courts.srt
courts.srt: no duration to adjust -> intact.srt
```

El fitxer s'escriu tal com és: **una destinació donada és una destinació escrita**.

## Sortida

**Sortida estàndard** — res: el resultat és el fitxer escrit.

| Nivell | Què s'afegeix a la sortida d'error |
| :----- | :------------------------------------- |
| 1 | `<path>: adjusted the durations of N subtitles; could not satisfy …  -> <destination>`, o `<path>: no duration to adjust -> <destination>` — i, primer, el fitxer de configuració llegit — `settings: read <path>` — vegeu [El fitxer de configuració](invocation.md#el-fitxer-de-configuració) |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — el format, la codificació, la marca i els finals de línia del fitxer llegit, restituïts tal com eren |
| 3 | `<path>: N bytes read, M written`, i després **cada diagnòstic de lectura** — vegeu [Invocació](invocation.md#diagnòstics-de-lectura) |

La frase és la de la finestra: `N` és el nombre de subtítols el final dels quals s'ha mogut, i la
resta, absent quan res no ha cedit, enumera el que no s'ha mantingut — `the reading speed`,
`the minimum duration`, `the gap` — amb el nombre de subtítols afectats.

### En JSON

Amb [`--format json`](invocation.md#sortida-llegible-per-un-script), l'objecte de cada fitxer
porta, a més de l'embolcall:

| Clau | Contingut |
| :-- | :------ |
| `counts.subtitles` | els subtítols **apuntats**: tot el fitxer, o la franja |
| `counts.adjusted` | aquells el final dels quals s'ha mogut |
| `counts.sacrificed.speed`, `.minimum`, `.gap` | els subtítols que encara no compleixen aquesta restricció, un cop aplicades les altres, **inclosos aquells el final dels quals no s'ha mogut** |
| `constraints.speed` | `{"cps", "lengthen", "shorten"}` — els caràcters per segon **com a cadena** (`"15"`, `"12.5"`) — o `null` si està desactivada |
| `constraints.minimum_ms`, `.maximum_ms`, `.gap_ms` | les durades usades, en **mil·lisegons enters**, o `null` si estan desactivades |
| `settings` | el fitxer de configuració que s'ha llegit, o `null` quan no se n'ha llegit cap |

`constraints` diu **els paràmetres realment usats**, valors per defecte inclosos: aleshores dos càlculs
sobre els mateixos subtítols es comparen sense llegir la línia d'ordres.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config --format json adjust --dry-run --maximum 1.8 courts.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config --format json adjust --dry-run --maximum 1.8 courts.srt
courts.srt: adjusted the durations of 4 subtitles; could not satisfy the reading speed in 1 subtitle, the minimum duration in 1 subtitle (dry run, nothing written)
{"schema":1,"command":"adjust","file":"courts.srt","ok":true,"dry_run":true,"destination":null,"counts":{"subtitles":4,"adjusted":4,"sacrificed":{"speed":1,"minimum":1,"gap":0}},"constraints":{"speed":{"cps":"15","lengthen":true,"shorten":false},"minimum_ms":1500,"maximum_ms":1800,"gap_ms":0},"settings":null,"warnings":[]}
```

## Codis de sortida

Els de l'eina: `0` si s'han tractat tots els fitxers, `2` si no se n'ha tractat cap, `3` si només
alguns, `1` en un error d'ús. Un fitxer que ja és dins dels seus límits es considera tractat.

## Errors

| Què el provoca | Missatge |
| :------------------ | :------ |
| un fitxer anomenat per `--config` que no existeix — el fitxer propi de l'usuari, si falta, dona els valors per defecte | `settings file not found: <FILE>` |
| cap restricció activa | `adjust has nothing to adjust to: every constraint is off, or the reading speed is told to move no end` |
| una velocitat que no ho és | `--speed: "…" is not a reading speed: expected a number of characters per second above zero, like 15 or 12.5, or off` |
| un indicador de direcció amb `--speed off` | `--lengthen, --no-lengthen, --shorten and --no-shorten say which way the reading speed may move an end, and --speed off switches the reading speed off` |
| un indicador de direcció amb el seu contrari | `--shorten and --no-shorten say opposite things; give one of them` (igualment `--lengthen` i `--no-lengthen`) |
| una durada que no ho és | `--minimum: "…" is not a time: …` (igualment `--maximum`, `--gap`) |
| durada negativa | `--minimum: "-1" is not a duration of zero or more` (igualment `--gap`) |
| màxim nul o negatiu | `--maximum: "0" is not a duration above zero` |
| franja mal escrita | `--range: "…" is not a range: write it N-M, or N- to go to the end, counted from 1 and inclusive` |
| franja que acaba abans de començar | `--range: "5-3" is not a range: it ends before it starts` |
| franja fora del fitxer | `<path>: range N-M starts after the last subtitle: the file has K`, o `… ends after …` |

Els errors de la destinació són comuns a les subordres que escriuen:
vegeu [Invocació](invocation.md#la-destinació).
