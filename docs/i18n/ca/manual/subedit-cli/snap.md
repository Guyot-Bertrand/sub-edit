# `snap`

<!-- machine-translated -->
> Aquesta pàgina és una traducció automàtica i encara no l'ha revisada cap lector de la llengua.

```
subedit-cli snap --rate <rate>
                 (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 <file>...
```

Posa **cada marca de temps** — començament i final — sobre el fotograma més proper de la
freqüència de fotogrames donada. No es resincronitza res: cada posició es mou com a màxim mig
fotograma.

## `snap` no és `framerate`, i confondre'ls no es nota

Les dues ordres tenen la mateixa forma — fitxers, una destinació, una freqüència de fotogrames — i fan coses oposades.

| | [`framerate`](framerate.md) | `snap` |
| :--- | :-------------------------- | :----- |
| què calcula | `t' = t x input rate / output rate` | el fotograma més proper, a la graella de `--rate` |
| quant mou | en proporció al temps — **diversos segons** en un llargmetratge | com a màxim mig fotograma, **arreu** |
| per a quin fitxer | un fitxer la **sincronització del qual és incorrecta** | un fitxer la sincronització del qual és **correcta** i la graella del qual és incorrecta |

**El cas d'ús de `snap`**, explicat: una pel·lícula a 25 fotogrames per
segon, un fitxer de subtítols escrit sobre una graella a 24, amb les línies ja
més o menys al seu lloc amb uns quants mil·lisegons de diferència. No hi ha res a resincronitzar
— només cal tornar a posar les marques de temps als fotogrames correctes.

**El cas d'ús de `framerate`** és l'altre: un fitxer sincronitzat per a un màster a
25 que es vol reproduir en una còpia a 23,976, i la sincronització del qual deriva
diversos segons cap al final de la pel·lícula.

Prendre l'una per l'altra **no provoca cap error**: totes dues
accepten el fitxer i escriuen un resultat. La línia de narració és el que
les distingeix — `snap` sempre diu quant ha mogut les posicions.

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

## Arguments i opcions

| Opció | Obligatori | Valors | Per defecte |
| :----- | :----- | :------ | :----- |
| `<file>...` | sí | un o més camins | — |
| `--recursive`, `-r` | no | un indicador | desactivat — vegeu [Tractar un arbre](lots.md) |
| `--rate` | **sí** | una freqüència de fotogrames en fotogrames per segon, escrita com per a [`framerate`](framerate.md#escriure-una-freqüència-de-fotogrames) | — |
| `--output` / `--output-dir` / `--in-place` | **una de les tres**, excepte amb `--dry-run` | vegeu [Invocació](invocation.md#la-destinació) | — |
| `--dry-run` | no | un indicador: calcula i diu, no escriu res — vegeu [Veure abans d'escriure](invocation.md#veure-abans-descriure) | desactivat |

`--rate` accepta **qualsevol freqüència de fotogrames vàlida**, i no només les vuit
estàndard: és la tria de l'usuari, com per a `framerate`. Una freqüència de fotogrames nul·la o
negativa es refusa.

El format del fitxer llegit **es conserva**: canviar el format és la feina de
[`convert`](convert.md).

## Sortida

**Sortida estàndard** — res: el resultat és el fitxer escrit. Amb `--dry-run`, tampoc res: no hi ha cap fitxer, i la línia de narració ho diu — vegeu [Veure abans d'escriure](invocation.md#veure-abans-descriure).

| Nivell | Què s'afegeix a la sortida d'error |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles aligned on 25 fps, M positions moved, by at most 20 ms -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — el format, la codificació, la marca i els finals de línia del fitxer llegit, restituïts tal com eren |
| 3 | `<path>: N bytes read, M written`, i després **cada diagnòstic de lectura** — vegeu [Invocació](invocation.md#diagnòstics-de-lectura) |

**Els dos nombres són la raó de ser de la línia.** Mig fotograma a 25 fotogrames per
segon són vint mil·lisegons, i no s'hi pot escriure res més gran. Qui
volia `framerate` i ha teclejat `snap` llegeix «by at most 20 ms» on esperava
segons, i ho sap de seguida.

## Un exemple

Una línia a `00:00:01,010`, alineada a 25 fotogrames per segon — per tant sobre un
múltiple de quaranta mil·lisegons:

<!-- example: printf '1\n00:00:01,010 --> 00:00:03,020\nUn.\n\n' > a.srt; subedit-cli snap --rate 25 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,010 --> 00:00:03,020\nUn.\n\n' > a.srt; subedit-cli snap --rate 25 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle aligned on 25 fps, 2 positions moved, by at most 20 ms -> b.srt
1
00:00:01,000 --> 00:00:03,040
Un.
```

## Dues propietats

**Alinear dues vegades no canvia res la segona vegada.** Una posició que ja és sobre un
fotograma s'hi queda: és la mateixa operació aplicada al mateix resultat.

**L'ordre dels subtítols es conserva.** Arrodonir al fotograma més proper és
monòton, de manera que dos començaments separats almenys per un fotograma ho continuen estant. Més
a prop, es poden fondre — cosa que és una anomalia que
[`inspect`](inspect.md#què-indica-anomalies) sap anomenar, i no una
inversió.

### En JSON

Amb [`--format json`](invocation.md#sortida-llegible-per-un-script), l'objecte de cada fitxer porta `counts.subtitles`, `counts.moved` (les posicions mogudes) i `counts.furthest_ms` (el desplaçament més gran, en mil·lisegons). La taula completa és la d'[Invocació](invocation.md#què-posa-cada-subordre-a-counts).

## Codis de sortida

Els de l'eina: `0` si s'han escrit tots els fitxers, `2` si no se n'ha escrit cap, `3` si
només alguns, `1` en un error d'ús.

## Errors

Els d'una freqüència de fotogrames, paraula per paraula els de [`framerate`](framerate.md#errors)
— totes dues llegeixen el mateix valor pel mateix camí:

| Què el provoca | Missatge |
| :------------------ | :------ |
| freqüència de fotogrames nul·la o negativa | `"…" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed` |
| coma decimal | `"…" is not a frame rate: use a decimal point and not a comma, in every language` |
| text que no és un nombre | `"…" is not a frame rate: expected frames per second, like 25 or 23.976` |
| freqüència de fotogrames desproporcionada | `"…" is not a frame rate: no video runs at that many frames per second` |

**No s'escriu res sense una de les tres opcions de destinació**, i els seus errors
són comuns a les subordres que escriuen:
vegeu [Invocació](invocation.md#la-destinació).
