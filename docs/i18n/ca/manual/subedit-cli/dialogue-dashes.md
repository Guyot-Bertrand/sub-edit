# `dialogue-dashes`

<!-- machine-translated -->
> Aquesta pàgina és una traducció automàtica i encara no l'ha revisada cap lector de la llengua.

```
subedit-cli dialogue-dashes (--add | --remove) [--range N-M|N-]
                            (--output FILE | --output-dir DIR | --in-place)
                            [--dry-run]
                            [--recursive]
                            [--document main|translation] [-t FILE] [--align-method position|number]
                            <file>...
```

Posa un **guió de diàleg** al començament de les línies, o els treu. És `Dialogue Dashes` de la
[finestra](../subedit-gui/operations.md).

**Dues paraules, com per a [`italics`](italics.md)**: `--add` i `--remove` s'exclouen mútuament, i en cal una.

- **`--add`** posa un guió davant de cada línia que no en té: `Bonjour` dona `- Bonjour`, i
  `Bonjour\nMarie` dona `- Bonjour\n- Marie`. **Un guió que ja hi és no es duplica.**
- **Es reconeixen tres guions** — el guionet del teclat i els dos guions llargs de la tipografia —
  **i el que s'escriu és el guionet**: `— Bonjour` esdevé `- Bonjour`. Els espais sobrers
  després del guió desapareixen.
- El guió va **davant del text, no davant d'una etiqueta d'obertura**: `<i>Bonjour</i>` dona
  `<i>- Bonjour</i>`. Cap etiqueta no es mou.
- **`--remove`** treu els guions inicials, en totes tres formes.

<!-- example: subedit-cli dialogue-dashes --help -->
```console
$ subedit-cli dialogue-dashes --help
Put dialogue dashes on the lines, or take them off
Usage: subedit-cli dialogue-dashes [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --add                       Put a dialogue dash at the head of the lines
  --remove                    Take the dialogue dashes off
  --range N-M|N-              Act only on subtitles N to M, or N to the end
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
| `--recursive`, `-r` | no | un indicador | desactivat — vegeu [Tractar un arbre](lots.md) |
| `--add` / `--remove` | **una de les dues** | indicadors | — |
| `--range` | no | `N-M`, o `N-` fins al final — vegeu [`adjust`](adjust.md#--range) | tot el fitxer |
| `--document` | no | `main` o `translation` — vegeu [Una traducció](invocation.md#una-traducció) | `main` |
| `-t`, `--translation-file` | amb `--document translation` | un fitxer de traducció, per a **una sola** entrada | — |
| `--align-method` | no, i només amb `-t` | `position` o `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **una de les tres**, excepte amb `--dry-run` | vegeu [Invocació](invocation.md#la-destinació) | — |
| `--dry-run` | no | un indicador: calcula i diu, no escriu res — vegeu [Veure abans d'escriure](invocation.md#veure-abans-descriure) | desactivat |

**El text principal per defecte; la traducció amb `--document translation -t FILE`.** Aleshores és
la traducció la que es canvia i s'escriu, i només ella — vegeu [Una traducció](invocation.md#una-traducció).

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\nMarie\n\n2\n00:00:03,000 --> 00:00:04,000\n<i>Salut</i>\n\n' > a.srt; subedit-cli dialogue-dashes --add --output t.srt a.srt; cat t.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\nMarie\n\n2\n00:00:03,000 --> 00:00:04,000\n<i>Salut</i>\n\n' > a.srt; subedit-cli dialogue-dashes --add --output t.srt a.srt; cat t.srt
a.srt: 2 subtitles dashed -> t.srt
1
00:00:01,000 --> 00:00:02,000
- Bonjour
- Marie

2
00:00:03,000 --> 00:00:04,000
<i>- Salut</i>
```

## Sortida

**Sortida estàndard** — res: el resultat és el fitxer escrit. Amb `--dry-run`, la llista de
canvis, descrita a [Veure abans d'escriure](invocation.md#veure-abans-descriure).

| Nivell | Què s'afegeix a la sortida d'error |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles dashed -> <destination>`, o `N subtitles undashed`, o `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — el format, la codificació, la marca i els finals de línia del fitxer llegit, restituïts tal com eren |
| 3 | `<path>: N bytes read, M written`, i després **cada diagnòstic de lectura** — vegeu [Invocació](invocation.md#diagnòstics-de-lectura) |

`N` és el nombre de subtítols **el text dels quals ha canviat**, llegit de la mateixa operació.

### En JSON

Amb [`--format json`](invocation.md#sortida-llegible-per-un-script), l'objecte de cada fitxer porta
`counts.changed` — la `N` de la frase — i `changes`, la llista dels subtítols canviats
(`{"subtitle", "document", "before", "after"}`, **buida** quan no ha canviat res).

## Codis de sortida

Els de l'eina: `0` si s'han tractat tots els fitxers — un fitxer on no canvia res ho és —,
`2` si no se n'ha tractat cap, `3` si només alguns, `1` en un error d'ús.

## Errors

| Què el provoca | Missatge |
| :------------------ | :------ |
| ni `--add` ni `--remove` | `dialogue-dashes needs --add to put dashes on, or --remove` |
| tots dos | `--add and --remove say opposite things; give one of them` |
| franja mal escrita o fora del fitxer | vegeu [`adjust`](adjust.md#--range) |

Els errors de la destinació són comuns a les subordres que escriuen:
vegeu [Invocació](invocation.md#la-destinació).
