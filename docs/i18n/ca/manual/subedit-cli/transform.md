# `transform`

<!-- machine-translated -->
> Aquesta pàgina és una traducció automàtica i encara no l'ha revisada cap lector de la llengua.

```
subedit-cli transform --first <index>=<time> --last <index>=<time>
                      (--output FILE | --output-dir DIR | --in-place)
                      [--dry-run]
                      [--recursive]
                      <file>...
```

Corregeix **totes** les posicions del fitxer a partir de dos punts el valor correcte dels quals
es coneix. És l'operació que demana una subtitulació que deriva
— correcta al començament, incorrecta al final —, on [`shift`](shift.md), que ho mou tot
la mateixa quantitat, no hi pot fer res.

El format del fitxer llegit **es conserva**: canviar el format és la feina de
[`convert`](convert.md).

<!-- example: subedit-cli transform --help -->
```console
$ subedit-cli transform --help
Correct every position from two points known to be right
Usage: subedit-cli transform [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to transform

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --first TEXT REQUIRED       Earlier reference, as <index>=<time>: 1=00:00:01.000
  --last TEXT REQUIRED        Later reference, as <index>=<time>: 3=00:00:10.000
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
| `--first` | **sí** | una referència, `<index>=<time>` | — |
| `--last` | **sí** | una referència, `<index>=<time>` | — |
| `--output` / `--output-dir` / `--in-place` | **una de les tres**, excepte amb `--dry-run` | vegeu [Invocació](invocation.md#la-destinació) | — |
| `--dry-run` | no | un indicador: calcula i diu, no escriu res — vegeu [Veure abans d'escriure](invocation.md#veure-abans-descriure) | desactivat |

No hi ha forma curta: aquestes dues opcions només s'escriuen
senceres.

## Escriure una referència

Una referència anomena un subtítol i diu on ha de caure ara el seu **començament**:

```
<index>=<time>
```

**L'índex es compta a partir d'1**, tal com es mostra i tal com el numera un fitxer
SubRip. `0` es refusa, i també un índex més enllà de l'últim
subtítol.

**El temps s'escriu com el de [`shift`](shift.md)** — segons o
marca de temps, amb el punt com a separador decimal i el mil·lisegon com a resolució:

| Forma | Exemples |
| :---- | :------- |
| segons | `10`, `2.999` |
| marca de temps | `00:00:10.000`, `01:39:37.040` |

`--first 3=10` i `--first 3=00:00:10.000` diuen el mateix.

`--first` i `--last` anomenen dos punts, no un interval. Els seus noms diuen
l'ús previst — una referència al començament del fitxer, l'altra al final — i no una
restricció: intercanviar-les dona el mateix resultat, i **tot el que queda fora de
les dues referències segueix com el que hi ha entre elles**.

## Què esdevenen les altres posicions

Les dues referències fixen un factor d'escala, i totes les posicions el segueixen:

```
t′ = y₁ + (t − x₁) × r      with r = (y₂ − y₁) / (x₂ − x₁)
```

`x₁`, `x₂` són els començaments actuals dels dos subtítols anomenats, `y₁`, `y₂` els
temps demanats. El factor es manté com un racional exacte i **l'arrodoniment
només es fa una vegada**, al mil·lisegon; és el que fa que les dues
referències caiguin **exactament** en els temps demanats i no amb un error d'una mil·lèsima
(vegeu l'[ADR 0013](../../../../adr/0013-mise-a-l-echelle-exacte-des-positions.md)).

Cada subtítol es queda a la pantalla **en proporció** al temps que hi era:
el final segueix la mateixa escala que el començament. Una transformació no és un
desplaçament, i no en té la propietat.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,001 --> 00:00:07,000\nDeux.\n\n3\n00:00:09,000 --> 00:00:11,000\nTrois.\n\n' > a.srt; subedit-cli transform --first 1=00:00:01.000 --last 3=00:00:10.000 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,001 --> 00:00:07,000\nDeux.\n\n3\n00:00:09,000 --> 00:00:11,000\nTrois.\n\n' > a.srt; subedit-cli transform --first 1=00:00:01.000 --last 3=00:00:10.000 --output b.srt a.srt; cat b.srt
a.srt: 3 subtitles transformed onto 1=00:00:01.000 and 3=00:00:10.000 -> b.srt
1
00:00:01,000 --> 00:00:03,250
Un.

2
00:00:05,501 --> 00:00:07,750
Deux.

3
00:00:10,000 --> 00:00:12,250
Trois.
```

La primera referència no es mou, la tercera passa de 9 s a 10 s: el factor és
`9/8`, i el segon subtítol — que ningú no ha anomenat — passa de
`5,001` a `5,501`, és a dir `1000 + 4001 × 9/8` arrodonit una sola vegada.

## Dues referències que no ho són

| Situació | Quan es veu | Codi |
| :-------- | :------------- | :--- |
| `--first` i `--last` anomenen el mateix índex | abans de cap lectura | `1` |
| un índex és més enllà de l'últim subtítol | quan s'obre el fitxer | `2` o `3` |
| els dos subtítols anomenats comencen en el mateix moment | quan s'obre el fitxer | `2` o `3` |

Tots tres es refusen pel mateix motiu: `x₂ − x₁` és zero, i dos
punts que coincideixen no defineixen cap recta. Retornar una divisió per zero disfressada de
resultat seria pitjor que refusar.

El primer cas és un **error d'ús** — les dues opcions són errònies siguin quins siguin
els fitxers, de manera que cal un sol missatge, no un per fitxer. Els
altres dos depenen del fitxer obert, i per tant compten com un error de
tractament.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n' > a.srt; subedit-cli transform --first 2=1.000 --last 2=4.000 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n' > a.srt; subedit-cli transform --first 2=1.000 --last 2=4.000 --output b.srt a.srt; echo "code=$?"
--first and --last both name subtitle 2, and two references on one subtitle define no transform
code=1
```

Un índex fora de límits es refusa **anomenant el límit**, cosa que diu alhora
què s'hauria pogut escriure:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,000 --> 00:00:07,000\nDeux.\n\n' > a.srt; subedit-cli transform --first 1=1.000 --last 9=10.000 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,000 --> 00:00:07,000\nDeux.\n\n' > a.srt; subedit-cli transform --first 1=1.000 --last 9=10.000 --output b.srt a.srt; echo "code=$?"
a.srt: subtitle 9 is past the end: the file holds 2 subtitles
code=2
```

## Una posició abans de l'origen es refusa

Com per a [`shift`](shift.md), i pel mateix motiu: una posició negativa
s'escriu `-00:00:01,000`, una forma que cap dels nou formats no defineix i que cap
lector no accepta. El cas es presenta aquí quan un subtítol és **abans de
la primera referència** i l'escala l'empeny més enllà de zero. No s'escriu
res, el codi és `2`, i el missatge anomena el subtítol que falla.

## Sortida

**Sortida estàndard** — res: el resultat és el fitxer escrit. Amb `--dry-run`, tampoc res: no hi ha cap fitxer, i la línia de narració ho diu — vegeu [Veure abans d'escriure](invocation.md#veure-abans-descriure).

| Nivell | Què s'afegeix a la sortida d'error |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles transformed onto 1=00:00:01.000 and 3=00:00:10.000 -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — el format, la codificació, la marca i els finals de línia del fitxer llegit, restituïts tal com eren |
| 3 | `<path>: N bytes read, M written`, i després **cada diagnòstic de lectura** — vegeu [Invocació](invocation.md#diagnòstics-de-lectura) |

Les referències es tornen a escriure amb el punt decimal, sigui com sigui que s'hagin teclejat.

### En JSON

Amb [`--format json`](invocation.md#sortida-llegible-per-un-script), l'objecte de cada fitxer porta `counts.subtitles` (tots els subtítols del fitxer). La taula completa és la d'[Invocació](invocation.md#què-posa-cada-subordre-a-counts).

## Codis de sortida

Els de l'eina: `0` si s'han escrit tots els fitxers, `2` si no se n'ha escrit cap, `3` si
només alguns, `1` en un error d'ús.

## Errors

| Què el provoca | Missatge |
| :------------------ | :------ |
| referència sense `=` | `"…" is not a reference: write it <index>=<time>, as in 3=00:00:10.000` |
| referència a la qual falta una meitat | `"…" is missing one of its two halves: write it <index>=<time>, as in 3=00:00:10.000` |
| índex `0` | `"0" is not a subtitle number: subtitles are counted from 1, as the file shows them` |
| índex que no és un nombre enter | `"…" is not a subtitle number: expected a whole number, counted from 1 as the file shows them` |
| temps no vàlid | els de [`shift`](shift.md) |
| `--first` i `--last` sobre el mateix índex | `--first and --last both name subtitle N, and two references on one subtitle define no transform` |
| índex fora de límits | `<path>: subtitle N is past the end: the file holds M subtitles` |
| dos subtítols que comencen junts | `<path>: subtitles N and M start at the same moment, so they define no transform` |
| posició abans de l'origen | `<path>: subtitle N would land before the origin, which no subtitle file can hold` |

**No s'escriu res sense una de les tres opcions de destinació**, i els seus
errors són comuns a les subordres que escriuen:
vegeu [Invocació](invocation.md#la-destinació).
