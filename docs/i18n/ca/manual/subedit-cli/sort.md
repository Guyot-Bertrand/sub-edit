# `sort`

<!-- machine-translated -->
> Aquesta pàgina és una traducció automàtica i encara no l'ha revisada cap lector de la llengua.

```
subedit-cli sort (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 <file>...
```

Torna a posar els subtítols **en l'ordre del seu començament**. No canvia res més: ni els textos, ni les
posicions, ni el format del fitxer. L'escriptura **renumera** els subtítols a partir d'1,
com qualsevol fitxer que es torna a escriure.

**El nucli no ordena mai pel seu compte**: llegir un fitxer en deixa l'ordre tal com era, i
[`inspect`](inspect.md#què-indica-anomalies) indica una ruptura de l'ordre amb
`subtitle N starts before the previous one starts`. `sort` és **el gest fet expressament**, per a un fitxer
o un lot amb l'ordre trencat — i es pot desfer, a la finestra, com qualsevol operació.

- **L'ordenació és estable**: dos subtítols que comencen en el mateix moment **conserven l'ordre que
  els donava el fitxer**. Cap no va abans de l'altre; moure'ls seria una decisió que ningú no
  ha demanat.
- **`sort` no pren ni franja ni document**: no hi ha res a triar, l'ordre és el dels començaments.

<!-- example: subedit-cli sort --help -->
```console
$ subedit-cli sort --help
Put the subtitles in the order of their start, keeping ties as they are
Usage: subedit-cli sort [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to sort

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
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
| `--output` / `--output-dir` / `--in-place` | **una de les tres**, excepte amb `--dry-run` | vegeu [Invocació](invocation.md#la-destinació) | — |
| `--dry-run` | no | un indicador: calcula i diu, no escriu res — vegeu [Veure abans d'escriure](invocation.md#veure-abans-descriure) | desactivat |

<!-- example: printf '1\n00:00:05,000 --> 00:00:06,000\ncinq\n\n2\n00:00:01,000 --> 00:00:02,000\nun\n\n3\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > desordre.srt; subedit-cli sort --output ordre.srt desordre.srt; cat ordre.srt -->
```console
$ printf '1\n00:00:05,000 --> 00:00:06,000\ncinq\n\n2\n00:00:01,000 --> 00:00:02,000\nun\n\n3\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > desordre.srt; subedit-cli sort --output ordre.srt desordre.srt; cat ordre.srt
desordre.srt: 3 subtitles moved -> ordre.srt
1
00:00:01,000 --> 00:00:02,000
un

2
00:00:03,000 --> 00:00:04,000
trois

3
00:00:05,000 --> 00:00:06,000
cinq
```

## Què es compta

**El recompte és el dels llocs que han canviat de subtítol**, tal com l'indica el nucli — el que
l'historial de la finestra anomenaria mogut. Un sol subtítol mal situat en desplaça diversos: en
l'exemple anterior, cap dels tres llocs no conserva el seu subtítol, i la frase diu `3 subtitles moved`.

**Un fitxer que ja és en ordre s'escriu igualment**, i ho diu: una destinació donada és una
destinació escrita, i fer de `sort` l'excepció obligaria un script a saber quines
subordres de vegades produeixen un fitxer i de vegades res.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > ordre.srt; subedit-cli sort --output copie.srt ordre.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > ordre.srt; subedit-cli sort --output copie.srt ordre.srt
ordre.srt: already in order -> copie.srt
```

## Sortida

**Sortida estàndard** — res: el resultat és el fitxer escrit. Amb `--dry-run`, tampoc res: no hi ha
cap fitxer, i la línia de narració ho diu — vegeu [Veure abans d'escriure](invocation.md#veure-abans-descriure).

| Nivell | Què s'afegeix a la sortida d'error |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles moved -> <destination>`, o `<path>: already in order -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — el format, la codificació, la marca i els finals de línia del fitxer llegit, restituïts tal com eren |
| 3 | `<path>: N bytes read, M written`, i després **cada diagnòstic de lectura** — vegeu [Invocació](invocation.md#diagnòstics-de-lectura) |

### En JSON

Amb [`--format json`](invocation.md#sortida-llegible-per-un-script), l'objecte de cada fitxer porta
`counts.subtitles` (tots els subtítols del fitxer) i `counts.moved` (els llocs que han canviat;
`0` per a un fitxer que ja és en ordre).

## Codis de sortida

Els de l'eina: `0` si s'han tractat tots els fitxers — un fitxer que ja és en ordre ho és —, `2` si
no se n'ha tractat cap, `3` si només alguns, `1` en un error d'ús.

## Errors

Cap de propi de `sort`: un fitxer il·legible falla com per a qualsevol subordre, i els
errors de la destinació són comuns a les subordres que escriuen:
vegeu [Invocació](invocation.md#la-destinació).
