# `framerate`

<!-- machine-translated -->
> Aquesta pàgina és una traducció automàtica i encara no l'ha revisada cap lector de la llengua.

> **Això no és [`snap`](snap.md).** Aquesta ordre **canvia l'escala del
> fitxer**: serveix quan la sincronització és incorrecta i deriva diversos
> segons cap al final de la pel·lícula. `snap` torna a posar les marques de temps a la graella més
> propera sense resincronitzar res, i serveix quan la sincronització ja és
> correcta. Totes dues tenen la mateixa forma, una o dues freqüències de fotogrames, i confondre-les no provoca
> cap error.

```
subedit-cli framerate --from <fps> --to <fps>
                      (--output FILE | --output-dir DIR | --in-place)
                      [--dry-run]
                      [--recursive]
                      <file>...
```

Resincronitza un fitxer sincronitzat per a una freqüència de fotogrames cap a una altra. És
l'operació que demana un fitxer de subtítols quan s'ha obtingut per a una còpia que
no és la de la pel·lícula que es té — un DVD PAL davant d'una transferència NTSC, per
exemple.

El format del fitxer llegit **es conserva**: canviar el format és la feina de
[`convert`](convert.md).

<!-- example: subedit-cli framerate --help -->
```console
$ subedit-cli framerate --help
Re-time a file mastered at one frame rate for another
Usage: subedit-cli framerate [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to re-time

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --from TEXT REQUIRED        Frame rate the file is timed at: 25, 23.976
  --to TEXT REQUIRED          Frame rate to time it for: 24, 29.97
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
| `--from` | **sí** | una freqüència de fotogrames, vegeu més avall | — |
| `--to` | **sí** | una freqüència de fotogrames, vegeu més avall | — |
| `--output` / `--output-dir` / `--in-place` | **una de les tres**, excepte amb `--dry-run` | vegeu [Invocació](invocation.md#la-destinació) | — |
| `--dry-run` | no | un indicador: calcula i diu, no escriu res — vegeu [Veure abans d'escriure](invocation.md#veure-abans-descriure) | desactivat |

`--from` és la freqüència de fotogrames per a la qual el fitxer **està** sincronitzat, `--to` aquella
per a la qual es vol. Intercanviar-les desplaça el fitxer en
l'altre sentit — uns set segons en un llargmetratge entre 25 i
23,976 — i res no ho assenyala abans de veure la pel·lícula.

## Escriure una freqüència de fotogrames

En fotogrames per segon, amb decimals, amb el punt com a separador:

```
25      24      23.976      29.97      60
```

**Una freqüència de fotogrames nul·la o negativa es refusa**: no és una freqüència de fotogrames.

### Les tres freqüències NTSC no es poden escriure exactament

`23.976`, `29.97` i `59.94` són **etiquetes**. Les freqüències de fotogrames que
anomenen són `24000/1001`, `30000/1001` i `60000/1001`, cap de les quals no té
una escriptura decimal finita. L'eina les reconeix com a tals:

| Què s'escriu | Què anomena |
| :------------- | :------------------ |
| `23.976` | `24000/1001` — cinema transferit a vídeo NTSC |
| `24` | `24` — cinema |
| `25` | `25` — vídeo PAL i SECAM |
| `29.97` | `30000/1001` — vídeo NTSC |
| `30`, `50`, `60` | elles mateixes |
| `59.94` | `60000/1001` |

El reconeixement es basa en **el valor i no en les lletres**: `23.9760`
anomena la mateixa freqüència de fotogrames que `23.976`.

Qualsevol altre decimal es pren literalment — `23.9` no és l'estàndard
de ningú, no hi ha res a interpretar-hi. Això és també el que fa necessària la taula
anterior: sense ella, `23.976` seria `23976/1000`, i la línia d'ordres
diria una cosa diferent de la finestra amb les mateixes paraules.

## Què esdevenen les posicions

```
t′ = round( t × (from / to) )
```

Una pel·lícula masteritzada a 23,976 i reproduïda a 25 fotogrames per segon va més de pressa,
de manera que els seus subtítols arriben abans; en l'altre sentit, arriben
més tard.

**El factor és un racional exacte, i l'arrodoniment només es fa una vegada.** El
camí que sembla equivalent — convertir la posició en un número de fotograma a la
freqüència d'entrada, i després tornar als mil·lisegons a la freqüència de sortida — no
ho és: quantifica cada posició a la graella d'entrada i s'equivoca fins a
mig fotograma, 21 ms mesurats. Vegeu
l'[ADR 0013](../../../../adr/0013-mise-a-l-echelle-exacte-des-positions.md).

A la pràctica, de 25 a 23,976, el factor és `1001/960`:

| Posició llegida | Escrita | Què donaria passar pels fotogrames |
| -----------: | -----: | -----------------------------------------: |
| `1 010 ms` | `1 053 ms` | `1 043 ms` |
| `3 600 017 ms` | `3 753 768 ms` | `3 753 750 ms` |

<!-- example: printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 25 --to 23.976 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 25 --to 23.976 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle retimed from 25 to 24000/1001 fps -> b.srt
1
00:00:01,053 --> 00:00:02,106
Un.
```

## Sortida

**Sortida estàndard** — res: el resultat és el fitxer escrit. Amb `--dry-run`, tampoc res: no hi ha cap fitxer, i la línia de narració ho diu — vegeu [Veure abans d'escriure](invocation.md#veure-abans-descriure).

| Nivell | Què s'afegeix a la sortida d'error |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles retimed from 25 to 24000/1001 fps -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — el format, la codificació, la marca i els finals de línia del fitxer llegit, restituïts tal com eren |
| 3 | `<path>: N bytes read, M written`, i després **cada diagnòstic de lectura** — vegeu [Invocació](invocation.md#diagnòstics-de-lectura) |

**La narració anomena la freqüència de fotogrames exacta, no l'etiqueta teclejada.** Escriure
`23.976` en aquesta línia informaria d'una conversió que no s'ha fet; és
l'únic lloc on es veu quina de les dues s'ha fet servir. Una freqüència de fotogrames que un
decimal escriu exactament s'hi mostra com a decimal, les altres com a fracció.

### En JSON

Amb [`--format json`](invocation.md#sortida-llegible-per-un-script), l'objecte de cada fitxer porta `counts.subtitles` (tots els subtítols del fitxer). La taula completa és la d'[Invocació](invocation.md#què-posa-cada-subordre-a-counts).

## Codis de sortida

Els de l'eina: `0` si s'han escrit tots els fitxers, `2` si no se n'ha escrit cap, `3` si
només alguns, `1` en un error d'ús.

Cap refús no depèn del contingut d'un fitxer: dues freqüències de fotogrames sempre defineixen
un factor. És l'única de les subordres que escriuen que té tots els
refusos abans de la primera lectura.

<!-- example: printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 0 --to 25 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 0 --to 25 --output b.srt a.srt; echo "code=$?"
"0" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed
code=1
```

## Errors

| Què el provoca | Missatge |
| :------------------ | :------ |
| freqüència de fotogrames nul·la o negativa | `"…" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed` |
| coma decimal | `"…" is not a frame rate: use a decimal point and not a comma, in every language` |
| text que no és un nombre | `"…" is not a frame rate: expected frames per second, like 25 or 23.976` |
| freqüència de fotogrames desproporcionada | `"…" is not a frame rate: no video runs at that many frames per second` |

**No s'escriu res sense una de les tres opcions de destinació**, i els seus
errors són comuns a les subordres que escriuen:
vegeu [Invocació](invocation.md#la-destinació).
