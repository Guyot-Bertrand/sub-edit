# `case`

<!-- machine-translated -->
> Aquesta pàgina és una traducció automàtica i encara no l'ha revisada cap lector de la llengua.

```
subedit-cli case --to title|sentence|upper|lower [--range N-M|N-]
                 (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 [--document main|translation] [-t FILE] [--align-method position|number]
                 <file>...
```

Posa els textos en **format de títol, format de frase, majúscules o minúscules**, sense tocar cap
etiqueta. És `Eines ▸ Majúscules i minúscules` de la [finestra](../subedit-gui/operations.md), amb les seves regles.

| `--to` | Efecte | `bonjour marie` esdevé |
| :----- | :---- | :---------------------- |
| `title` | cada paraula pren una majúscula, la resta passa a minúscula | `Bonjour Marie` |
| `sentence` | la primera lletra del text, i només ella; la resta passa a minúscula | `Bonjour marie` |
| `upper` | tot en majúscules | `BONJOUR MARIE` |
| `lower` | tot en minúscules | `bonjour marie` |

- **El que ve abans de la primera lletra o de la primera xifra no es toca**: un guió de
  diàleg, unes cometes d'obertura o un parèntesi sobreviuen (`- bonjour` dona `- Bonjour`).
- **Les etiquetes no són text**: no canvien de majúscules — `<I>` continua sent `<I>` — ni
  de lloc: una etiqueta que talla una paraula la continua tallant després.
- **ICU decideix on comença una paraula**: `l'été sera chaud` dona `L'été Sera Chaud`, i els accents
  es mantenen (`éléonore` dona `Éléonore`).

<!-- example: subedit-cli case --help -->
```console
$ subedit-cli case --help
Put the texts in title, sentence, upper or lower case, tags intact
Usage: subedit-cli case [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --to title|sentence|upper|lower
                              The case to put the texts in
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
| `--to` | **sí** | `title`, `sentence`, `upper`, `lower` | — |
| `--range` | no | `N-M`, o `N-` fins al final — vegeu [`adjust`](adjust.md#--range) | tot el fitxer |
| `--document` | no | `main` o `translation` — vegeu [Una traducció](invocation.md#una-traducció) | `main` |
| `-t`, `--translation-file` | amb `--document translation` | un fitxer de traducció, per a **una sola** entrada | — |
| `--align-method` | no, i només amb `-t` | `position` o `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **una de les tres**, excepte amb `--dry-run` | vegeu [Invocació](invocation.md#la-destinació) | — |
| `--dry-run` | no | un indicador: calcula i diu, no escriu res — vegeu [Veure abans d'escriure](invocation.md#veure-abans-descriure) | desactivat |

**El text principal per defecte; la traducció amb `--document translation -t FILE`.** Aleshores és
la traducció la que es canvia i s'escriu, i només ella — vegeu [Una traducció](invocation.md#una-traducció).

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\n- BONJOUR MARIE\n\n' > a.srt; subedit-cli case --to title --output titre.srt a.srt; cat titre.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\n- BONJOUR MARIE\n\n' > a.srt; subedit-cli case --to title --output titre.srt a.srt; cat titre.srt
a.srt: 2 subtitles recased -> titre.srt
1
00:00:01,000 --> 00:00:02,000
<i>Bon</i>jour Marie

2
00:00:03,000 --> 00:00:04,000
- Bonjour Marie
```

Un fitxer on **no canvia cap text** — tot ja és en les majúscules o minúscules demanades — s'escriu tal com és i
ho diu:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBONJOUR\n\n' > a.srt; subedit-cli case --to upper --output b.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBONJOUR\n\n' > a.srt; subedit-cli case --to upper --output b.srt a.srt
a.srt: nothing to change -> b.srt
```

## Sortida

**Sortida estàndard** — res: el resultat és el fitxer escrit. Amb `--dry-run`, la llista de
canvis, descrita a [Veure abans d'escriure](invocation.md#veure-abans-descriure).

| Nivell | Què s'afegeix a la sortida d'error |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles recased -> <destination>`, o `nothing to change` |
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
| falta `--to` | `--to is required` |
| un valor que no és cap dels quatre | `--to: … not in {title,sentence,upper,lower}` |
| franja mal escrita o fora del fitxer | vegeu [`adjust`](adjust.md#--range) |

Els errors de la destinació són comuns a les subordres que escriuen:
vegeu [Invocació](invocation.md#la-destinació).
