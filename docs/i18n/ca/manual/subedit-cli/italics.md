# `italics`

<!-- machine-translated -->
> Aquesta pàgina és una traducció automàtica i encara no l'ha revisada cap lector de la llengua.

```
subedit-cli italics (--on | --off) [--range N-M|N-]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    <file>...
```

Posa els textos **en cursiva**, o **els en treu la cursiva**, amb les etiquetes del format del fitxer.
És `Italic` de la [finestra](../subedit-gui/operations.md).

**Dues paraules i no un commutador**: el botó de la finestra pregunta al nucli en quin sentit
actua; aquí ho dieu vosaltres. `--on` i `--off` s'exclouen mútuament, i en cal un — ometre'l, o donar-los tots dos,
és un error d'ús (codi `1`).

- **`--on`** envolta cada text amb un sol parell d'etiquetes, **després d'eliminar les que
  hi havia**: un text que ja és en cursiva no s'envolta dues vegades, i una cursiva tallada en dos
  es refà d'una sola peça. Sobre diverses línies, un sol parell ho cobreix tot.
- **`--off`** elimina les etiquetes de cursiva; **la resta d'una etiqueta es conserva** (`{\b1\i1}` dona
  `{\b1}`).
- El text s'escriu **com el format del fitxer escriu la cursiva**: `<i>…</i>` per a SubRip, WebVTT i
  SubViewer 2, `{\i1}…{\i0}` per als dos Sub Station Alpha, `{Y:i}` per a MicroDVD.

<!-- example: subedit-cli italics --help -->
```console
$ subedit-cli italics --help
Put the texts in italics, or take their italics out
Usage: subedit-cli italics [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --on                        Put the texts in italics
  --off                       Take the italics out
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
| `--on` / `--off` | **una de les dues** | indicadors | — |
| `--range` | no | `N-M`, o `N-` fins al final — vegeu [`adjust`](adjust.md#--range) | tot el fitxer |
| `--document` | no | `main` o `translation` — vegeu [Una traducció](invocation.md#una-traducció) | `main` |
| `-t`, `--translation-file` | amb `--document translation` | un fitxer de traducció, per a **una sola** entrada | — |
| `--align-method` | no, i només amb `-t` | `position` o `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **una de les tres**, excepte amb `--dry-run` | vegeu [Invocació](invocation.md#la-destinació) | — |
| `--dry-run` | no | un indicador: calcula i diu, no escriu res — vegeu [Veure abans d'escriure](invocation.md#veure-abans-descriure) | desactivat |

**El text principal per defecte; la traducció amb `--document translation -t FILE`.** Aleshores és
la traducció la que es canvia i s'escriu, i només ella — vegeu [Una traducció](invocation.md#una-traducció).

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour\nMarie\n\n' > a.srt; subedit-cli italics --on --output i.srt a.srt; cat i.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour\nMarie\n\n' > a.srt; subedit-cli italics --on --output i.srt a.srt; cat i.srt
a.srt: 2 subtitles put in italics -> i.srt
1
00:00:01,000 --> 00:00:02,000
<i>bonjour marie</i>

2
00:00:03,000 --> 00:00:04,000
<i>Bonjour
Marie</i>
```

## Un format sense estil es refusa

**TMPlayer i LRC no escriuen cap estil**: no hi ha cursiva per posar ni cursiva per treure. La
finestra mostra l'entrada en gris en lloc d'amagar-la; la línia d'ordres **es refusa, fitxer per
fitxer, amb el motiu**, i els altres fitxers del lot continuen. El que compta és el que el
format pot portar, no la manera com ho escriu: SubRip i WebVTT escriuen la cursiva de la mateixa
manera, i no diuen el mateix sobre el color.

<!-- example: printf '00:00:01:Hello there\n00:00:03:Bye\n' > a.txt; subedit-cli italics --on --output-dir sortie a.txt; echo "code=$?" -->
```console
$ printf '00:00:01:Hello there\n00:00:03:Bye\n' > a.txt; subedit-cli italics --on --output-dir sortie a.txt; echo "code=$?"
a.txt: TMPlayer writes no style: there are no italics to put on
code=2
```

En JSON, l'error porta l'identificador **`no-style`**.

## Sortida

**Sortida estàndard** — res: el resultat és el fitxer escrit. Amb `--dry-run`, la llista de
canvis, descrita a [Veure abans d'escriure](invocation.md#veure-abans-descriure).

| Nivell | Què s'afegeix a la sortida d'error |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles put in italics -> <destination>`, o `N subtitles taken out of italics`, o `nothing to change` |
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
| ni `--on` ni `--off` | `italics needs --on to put the texts in italics, or --off` |
| tots dos | `--on and --off say opposite things; give one of them` |
| un format sense estil | `<path>: TMPlayer writes no style: there are no italics to put on` (o `take out`) |
| franja mal escrita o fora del fitxer | vegeu [`adjust`](adjust.md#--range) |

Els errors de la destinació són comuns a les subordres que escriuen:
vegeu [Invocació](invocation.md#la-destinació).
