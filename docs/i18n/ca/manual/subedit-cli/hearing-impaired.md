# `hearing-impaired`

<!-- machine-translated -->
> Aquesta pàgina és una traducció automàtica i encara no l'ha revisada cap lector de la llengua.

```
subedit-cli hearing-impaired
                  (--output FILE | --output-dir DIR | --in-place)
                  [--dry-run]
                  [--recursive] [--parentheses]
                  [--document main|translation] [-t FILE] [--align-method position|number]
                  <file>...
```

Elimina les **mencions per a persones sordes** — els sons descrits entre claudàtors — de tot el
fitxer. **Els parèntesis es mantenen per defecte**: s'usen tant per a una rèplica
xiuxiuejada com per a un so descrit, i eliminar-los esborraria diàleg. `--parentheses` també els
elimina, per a un fitxer que hi descriu els seus sons.

El que no fa: les lletres de cançons entre coixinets, el nom de qui parla
davant de dos punts, tornar a posar majúscules, corregir errors d'OCR. Aquests
patrons són els de l'assistent
[`Correct Texts…`](../subedit-gui/correct-texts.md) de la finestra, que la línia
d'ordres ofereix amb el nom de [`correct`](correct.md); i aquí no es pot configurar res: la
transformació està decidida, no és configurable.

<!-- example: subedit-cli hearing-impaired --help -->
```console
$ subedit-cli hearing-impaired --help
Remove the sounds described between square brackets
Usage: subedit-cli hearing-impaired [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to clean

Options:
  -h,--help                   Print this help message and exit
  --parentheses               Also remove the sounds between parentheses, often whispered lines
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --document main|translation The document to change: the main one, or the translation given by -t
  -t,--translation-file FILE  Translation file to lay over the subtitle file, for a single input
  --align-method position|number
                              How the lines of the translation find their subtitles
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Arguments i opcions

| Opció | Obligatori | Valors | Per defecte |
| :----- | :----- | :------ | :----- |
| `<file>...` | sí | un o més camins | — |
| `--parentheses` | no | un indicador: també elimina les mencions entre parèntesis | desactivat |
| `--recursive`, `-r` | no | un indicador | desactivat — vegeu [Tractar un arbre](lots.md) |
| `--document` | no | `main` o `translation` — vegeu [Una traducció](invocation.md#una-traducció) | `main` |
| `-t`, `--translation-file` | amb `--document translation` | un fitxer de traducció, per a **una sola** entrada | — |
| `--align-method` | no, i només amb `-t` | `position` o `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **una de les tres**, excepte amb `--dry-run` | vegeu [Invocació](invocation.md#la-destinació) | — |
| `--dry-run` | no | un indicador: calcula i diu, no escriu res — vegeu [Veure abans d'escriure](invocation.md#veure-abans-descriure) | desactivat |

El format del fitxer llegit **es conserva**: canviar el format és la feina de
[`convert`](convert.md).

**El text principal per defecte; la traducció amb `--document translation -t FILE`.** Aleshores és
la traducció la que es canvia i s'escriu, i només ella — vegeu [Una traducció](invocation.md#una-traducció).

**Una menció que buidaria una traducció l'escriu buida, i conserva el subtítol**: el subtítol
es queda al fitxer, sense text. El text principal, en canvi, perd un subtítol buidat del tot, com
es veu més avall.

## Què s'elimina, i què es queda

Una menció és un claudàtor — o un parèntesi, amb `--parentheses` — i el que tanca, **salt de
línia inclòs**: en els fitxers reals, una menció sovint queda tallada pel
final d'una línia.

| Entrada | Sortida |
| :----- | :----- |
| `Bonjour [il tousse] Marie` | `Bonjour Marie` |
| `Bonjour[il tousse]Marie` | `Bonjour Marie` |
| `[Bruit de pas] Bonjour` | `Bonjour` |
| `A [un] [deux] B` | `A B` |

L'eliminació deixa **exactament un espai entre el que l'envoltava, i res a
la vora d'una línia**. No és una neteja dels espais del fitxer: dos espais
que el text ja portava continuen sent dos espais.

**Quan la menció travessa el salt de línia, el que queda és el salt** — les
dues línies continuen sent dues línies. Aquest cas no és una curiositat: en quinze
subtitulacions reals, cada claudàtor que semblava orfe era una
menció tallada d'aquesta manera.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nReculez ! [Il\nhurle] Tout de suite !\n\n' > a.srt; subedit-cli --quiet hearing-impaired --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nReculez ! [Il\nhurle] Tout de suite !\n\n' > a.srt; subedit-cli --quiet hearing-impaired --output b.srt a.srt; cat b.srt
1
00:00:01,000 --> 00:00:03,000
Reculez !
Tout de suite !
```

**El que no és una menció i no es mou:**

| Cas | Què passa |
| :-- | :-------------- |
| contingut purament numèric — `[1]`, `(12)`, `[ 156478 ]` | es conserva tal com és, delimitadors inclosos: una referència no ha anomenat mai un so |
| un parèntesi i el que tanca, sense `--parentheses` | es conserva tal com és: `Venez (chut) ici` no canvia |
| un claudàtor o un parèntesi que res no tanca | es deixa tal com és |
| una etiqueta de format — `<i>`, `</i>` | no s'elimina mai |

Un espai en blanc **entre** les xifres, una lletra o un signe — `[1 2]`, `[1a]`,
`[-1]` — en fan una menció. També un contingut buit, `[]` o `( )`: cap
xifra, per tant no és una referència; res a dir, per tant res a conservar.

## Què desapareix del tot

**Una línia que l'eliminació ha buidat desapareix**, i **un subtítol buidat
del tot s'elimina del fitxer**. Els subtítols que segueixen es renumeren.

Una línia es considera buida quan tot el que hi queda són espais en blanc, etiquetes de
format, o un guió de diàleg solitari — `<i>[Musique]</i>` no deixa cap
`<i></i>` a la pantalla.

**Les etiquetes són les del format del fitxer.** `{\i1}[Musique]{\i0}` en un
Advanced SSA, `{Y:i}[Musique]` en un MicroDVD i `/[Musique]` en un MPL2 tampoc no deixen
res. A la inversa, un `<i>` en un format que no escriu cap
etiqueta — TMPlayer, LRC — és text que l'espectador llegeix, i la línia que
el porta no és buida.

**Un diàleg reduït a una sola veu ja no ho és**, i el guió que queda
desapareix. Mentre quedin dues veus, els guions es mantenen.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\nVoir [1] la note.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\nVoir [1] la note.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle cleaned, 1 removed -> b.srt
1
00:00:04,000 --> 00:00:06,000
Attends Marie.

2
00:00:07,000 --> 00:00:09,000
Voir [1] la note.
```

## Un fitxer sense cap menció s'escriu igualment

Es copia idèntic, el codi de sortida és `0`, i l'informe diu que no hi havia
res a eliminar.

És la regla de les subordres que escriuen: **una destinació donada és
una destinació escrita**. Fer-hi una excepció obligaria un script a saber
quina de les subordres de vegades produeix un fitxer i de vegades res.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nRien à signaler.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nRien à signaler.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt
a.srt: no mention to remove -> b.srt
```

## Sortida

**Sortida estàndard** — res: el resultat és el fitxer escrit. **Amb `--dry-run`, són els canvis proposats**: per a cada subtítol el text del qual canviaria, el seu número, el text d'abans i el text de després — vegeu [Veure abans d'escriure](invocation.md#veure-abans-descriure).

| Nivell | Què s'afegeix a la sortida d'error |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles cleaned, M removed -> <destination>`, o `<path>: no mention to remove -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — el format, la codificació, la marca i els finals de línia del fitxer llegit, restituïts tal com eren |
| 3 | `<path>: N bytes read, M written`, i després **cada diagnòstic de lectura** — vegeu [Invocació](invocation.md#diagnòstics-de-lectura) |

`N` compta els subtítols el text dels quals ha canviat, `M` els que l'eliminació
ha buidat del tot i que, per tant, han sortit del fitxer.

### En JSON

Amb [`--format json`](invocation.md#sortida-llegible-per-un-script), l'objecte de cada fitxer porta `counts.cleaned` (els textos reescrits) i `counts.removed` (els subtítols eliminats); **amb `--dry-run`**, la clau `changes` també enumera els subtítols canviats, com la sortida estàndard de la forma de text. La taula completa és la d'[Invocació](invocation.md#què-posa-cada-subordre-a-counts).

## Codis de sortida

Els de l'eina: `0` si s'han escrit tots els fitxers, `2` si no se n'ha escrit cap, `3` si
només alguns, `1` en un error d'ús.

## Errors

Aquesta subordre no té cap error propi: no pren cap
valor per analitzar, i cap text no li pot fer refusar un fitxer. El que
pot fallar és la lectura o l'escriptura, com per a les altres.

**No s'escriu res sense una de les tres opcions de destinació**, i els seus errors
són comuns a les subordres que escriuen:
vegeu [Invocació](invocation.md#la-destinació).
