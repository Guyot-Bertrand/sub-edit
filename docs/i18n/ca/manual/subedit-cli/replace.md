# `replace`

<!-- machine-translated -->
> Aquesta pàgina és una traducció automàtica i encara no l'ha revisada cap lector de la llengua.

```
subedit-cli replace [--regex] [--case-sensitive] [--range N-M|N-]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    <pattern> <replacement> <file>...
```

Substitueix un text als subtítols **sense trencar cap etiqueta**. És l'operació
`Replace All` de la [cerca](../subedit-gui/find-and-replace.md) de la finestra, amb les seves regles:

- **la cerca es fa en el text visible** — el que llegeix l'espectador. Les etiquetes no en
  formen part: cercar `<i>` no troba res, i cercar `Bonjour` troba un «Bonjour» la meitat del qual
  és en cursiva;
- **la substitució es fa en el text font**, el del fitxer, i manté les etiquetes on
  eren. Una etiqueta que talla la paraula cercada envolta tota la substitució; un estil que
  toca la coincidència cobreix tota la substitució; el que la coincidència no toca
  no es mou. Les regles, cas per cas, són les de la cerca de la finestra.

**No hi ha cap subordre `find`**: per veure on és un patró sense canviar res,
és [`--dry-run`](invocation.md#veure-abans-descriure) el que enumera cada subtítol afectat.

<!-- example: subedit-cli replace --help -->
```console
$ subedit-cli replace --help
Replace a text in the subtitles, without breaking a tag
Usage: subedit-cli replace [OPTIONS] pattern replacement files...

Positionals:
  pattern TEXT REQUIRED       Text to look for in what is shown
  replacement TEXT REQUIRED   What replaces it, tags allowed
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --regex                     Read the text to look for as a regular expression
  --case-sensitive            Tell capitals from small letters
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
| `<pattern>` | sí | el text que cal cercar en el que es mostra; **no buit** | — |
| `<replacement>` | sí | el que el substitueix, amb etiquetes permeses; pot ser buit | — |
| `<file>...` | sí | un o més camins | — |
| `--recursive`, `-r` | no | un indicador | desactivat — vegeu [Tractar un arbre](lots.md) |
| `--regex` | no | un indicador: el patró és una expressió regular ICU | text pla |
| `--case-sensitive` | no | un indicador: distingeix les majúscules de les minúscules | no es distingeixen |
| `--range` | no | `N-M`, o `N-` fins al final — vegeu [`adjust`](adjust.md#--range) | tot el fitxer |
| `--document` | no | `main` o `translation` — vegeu [Una traducció](invocation.md#una-traducció) | `main` |
| `-t`, `--translation-file` | amb `--document translation` | un fitxer de traducció, per a **una sola** entrada | — |
| `--align-method` | no, i només amb `-t` | `position` o `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **una de les tres**, excepte amb `--dry-run` | vegeu [Invocació](invocation.md#la-destinació) | — |
| `--dry-run` | no | un indicador: calcula i diu, no escriu res — vegeu [Veure abans d'escriure](invocation.md#veure-abans-descriure) | desactivat |

**Un patró que comença amb un guió** s'escriu després de `--`: `subedit-cli replace -- -- "—" film.srt`.

**El text principal per defecte; la traducció amb `--document translation -t FILE`.** Aleshores és
la traducció la que es canvia i s'escriu, i només ella — vegeu [Una traducció](invocation.md#una-traducció).

### Text pla, o expressió

Per defecte el patró **es llegeix tal com és**: `a.b` cerca una `a`, un punt, una `b`. Les majúscules i minúscules
s'ignoren amb la mateixa finesa que a la finestra — `É` i `é` són una sola lletra.

Amb `--regex`, el patró és una expressió regular **ICU**, llegida com la llegeix Gaupol: `.` travessa
un salt de línia, `^` i `$` valen a cada línia d'un subtítol. A la substitució, i
només aleshores:

| Escriviu | Per dir |
| :----- | :-------- |
| de `$1` a `$9` | els grups de la coincidència |
| `$0` | tota la coincidència |
| `\n` | un salt de línia |
| `\$`, `\\` | un `$`, una barra inversa |

Compte amb les cometes del shell: `'$1 EUR'`, i no `"$1 EUR"`, que deixaria que el shell llegís `$1`.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nPrix: 12 euros, et 7 euros.\n\n' > prix.srt; subedit-cli replace --regex '([0-9]+) euros' '$1 EUR' --output euros.srt prix.srt; cat euros.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nPrix: 12 euros, et 7 euros.\n\n' > prix.srt; subedit-cli replace --regex '([0-9]+) euros' '$1 EUR' --output euros.srt prix.srt; cat euros.srt
prix.srt: replaced 2 matches -> euros.srt
1
00:00:01,000 --> 00:00:02,000
Prix: 12 EUR, et 7 EUR.
```

## Etiquetes

Un text on una etiqueta talla la paraula cercada:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace Bonjour Salut --output salut.srt balises.srt; cat salut.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace Bonjour Salut --output salut.srt balises.srt; cat salut.srt
balises.srt: replaced 2 matches -> salut.srt
1
00:00:01,000 --> 00:00:02,000
<i>Salut</i> tout le monde

2
00:00:03,000 --> 00:00:04,000
Salut <i>Marie</i>, ca va
```

Al primer subtítol, l'etiqueta tallava «Bonjour»: ara envolta «Salut».
Al segon, no es toca i es queda on és. **Una etiqueta deixada oberta
es queda oberta**: reparar un marcatge defectuós seria una altra operació.

La substitució es llegeix en **les etiquetes del format del fitxer**: `<i>…</i>` per a SubRip,
`{\i1}…` per a Advanced SSA. És l'única manera de donar format al que s'escriu.

## No s'ha trobat res, no hi ha res a canviar

Dues respostes que no són fracassos — el codi de sortida és `0` i **el fitxer s'escriu
igualment**, ja que una destinació donada és una destinació escrita:

| Situació | Frase |
| :-------- | :----- |
| el patró no és enlloc | `"zzz" not found` |
| hi és, però cada coincidència se substitueix per ella mateixa | `nothing to change` |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli replace zzz x --output b.srt a.srt; subedit-cli replace Bonjour Bonjour --output c.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli replace zzz x --output b.srt a.srt; subedit-cli replace Bonjour Bonjour --output c.srt a.srt
a.srt: "zzz" not found -> b.srt
a.srt: nothing to change -> c.srt
```

## `--dry-run`

La sortida estàndard enumera **cada subtítol el text del qual canviaria**, abans i després, tal com
són al fitxer — etiquetes incloses:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace --dry-run Bonjour Salut balises.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace --dry-run Bonjour Salut balises.srt
balises.srt: replaced 2 matches (dry run, nothing written)
balises.srt: subtitle 1
- <i>Bon</i>jour tout le monde
+ <i>Salut</i> tout le monde
balises.srt: subtitle 2
- Bonjour <i>Marie</i>, ca va
+ Salut <i>Marie</i>, ca va
```

La forma de la llista es descriu a [Veure abans d'escriure](invocation.md#veure-abans-descriure);
en JSON, és `changes`.

## Sortida

**Sortida estàndard** — res: el resultat és el fitxer escrit. Amb `--dry-run`, la llista de
canvis anterior.

| Nivell | Què s'afegeix a la sortida d'error |
| :----- | :------------------------------------- |
| 1 | `<path>: replaced N matches -> <destination>`, o `"<pattern>" not found`, o `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — el format, la codificació, la marca i els finals de línia del fitxer llegit, restituïts tal com eren |
| 3 | `<path>: N bytes read, M written`, i després **cada diagnòstic de lectura** — vegeu [Invocació](invocation.md#diagnòstics-de-lectura) |

`N` compta **les coincidències substituïdes als subtítols el text dels quals ha canviat** — és el
nombre que també diu la finestra.

### En JSON

Amb [`--format json`](invocation.md#sortida-llegible-per-un-script), l'objecte de cada fitxer porta:

| Clau | Contingut |
| :-- | :------ |
| `counts.replaced` | les coincidències substituïdes, als subtítols el text dels quals ha canviat: la `N` de la frase |
| `counts.matched` | les coincidències trobades, **tant si la seva substitució ha canviat el text com si no** — és el que distingeix «no s'ha trobat res» (`0`) de «no hi ha res a canviar» (`replaced` a `0`, `matched` positiu) |
| `changes` | els subtítols canviats: `{"subtitle", "document", "before", "after"}`, **buit** quan no ha canviat res |

## Codis de sortida

Els de l'eina: `0` si s'han tractat tots els fitxers — un fitxer on no s'ha trobat res ho
és —, `2` si no se n'ha tractat cap, `3` si només alguns, `1` en un error d'ús.

## Errors

| Què el provoca | Missatge |
| :------------------ | :------ |
| patró buit | `pattern: nothing to look for` |
| expressió il·legible | `pattern: not a regular expression (<ICU's reason>)` |
| franja mal escrita o fora del fitxer | vegeu [`adjust`](adjust.md#--range) |

**El patró es llegeix abans del primer fitxer**: una expressió il·legible és un error d'ús
(codi `1`), no es llegeix ni s'escriu res.

Els errors de la destinació són comuns a les subordres que escriuen:
vegeu [Invocació](invocation.md#la-destinació).
