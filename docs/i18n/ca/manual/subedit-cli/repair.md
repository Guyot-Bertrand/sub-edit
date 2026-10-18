# `repair`

<!-- machine-translated -->
> Aquesta pàgina és una traducció automàtica i encara no l'ha revisada cap lector de la llengua.

```
subedit-cli repair (--output FILE | --output-dir DIR | --in-place)
                   [--minimum TIME] [--max-lines N] [--dashes]
                   [--dry-run] [--recursive]
                   <file>...
```

Fa, d'una sola vegada, el que fan tres ordres l'una darrere l'altra: [`join-repeats`](join-repeats.md), després
[`sort`](sort.md), després [`disjoint`](disjoint.md). **En aquest ordre, que és l'únic que funciona**: el tall llegeix
l'ordre dels començaments, i els trossos repetits d'un mateix text és millor que hagin desaparegut abans de totes dues. En un fitxer
mal segmentat, fora d'ordre i amb encavalcaments, és l'ordre que cal executar.

Els paràmetres són els de `disjoint`, que tanca la cadena; `--sort` aquí no existeix, ja que l'ordenació sempre es fa.

<!-- example: subedit-cli repair --help -->
```console
$ subedit-cli repair --help
Join the repeated subtitles, put them in order, then make the ranges disjoint
Usage: subedit-cli repair [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to repair

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --minimum TIME              Shortest interval kept; a shorter one is taken over by a neighbour (default 0.08)
  --max-lines N               Most lines of a merged subtitle; more are shown in windows, 0 for no limit (default 2)
  --dashes                    Put a dash before the text of each merged subtitle
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Arguments i opcions

| Opció | Obligatori | Valors | Per defecte |
| :----- | :----- | :------ | :----- |
| `<file>...` | sí | un o més camins | — |
| `--minimum` | no | una durada: `0.08`, `1.5`, `00:00:00,080`. Zero conserva tots els intervals | `0.08` (uns dos fotogrames) |
| `--max-lines` | no | un nombre enter; `0` elimina el límit | `2` |
| `--dashes` | no | un indicador: un guió davant del text que cada subtítol aporta a un subtítol fusionat | desactivat |
| `--recursive`, `-r` | no | un indicador | desactivat — vegeu [Tractar un arbre](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **una de les tres**, excepte amb `--dry-run` | vegeu [Invocació](invocation.md#la-destinació) | — |
| `--dry-run` | no | un indicador: calcula i diu, no escriu res — vegeu [Veure abans d'escriure](invocation.md#veure-abans-descriure) | desactivat |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:01,000 --> 00:00:04,000\nBeta\n\n' > segments.srt; subedit-cli repair --output repare.srt segments.srt; cat repare.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:01,000 --> 00:00:04,000\nBeta\n\n' > segments.srt; subedit-cli repair --output repare.srt segments.srt; cat repare.srt
segments.srt: 1 repeated subtitle joined; already in order; 0 subtitles split, 1 merged -> repare.srt
1
00:00:01,000 --> 00:00:04,000
Alpha
Beta
```

`Alpha`, tallat en dos trossos que se succeeixen, torna a ser un sol subtítol; aleshores ocupa la mateixa franja que `Beta`,
i tots dos només en fan un, de dues línies, el més primerenc primer.

## Sortida

**Sortida estàndard** — res: el resultat és el fitxer escrit. A la sortida d'error, al primer nivell, **una frase
que encadena els tres recomptes**, separats per punts i comes: el que diuen `join-repeats`, `sort` i `disjoint` cadascuna,
amb les seves pròpies paraules — `N repeated subtitles joined; M subtitles moved; K subtitles split, L merged…`.

### En JSON

Amb [`--format json`](invocation.md#sortida-llegible-per-un-script), els `counts` de cada fitxer: `subtitles`, `joined`,
`moved`, i després els de [`disjoint`](disjoint.md#en-json) (`split`, `merged`, `windowed`, `absorbed`, `too_short`,
`ending_before_start`, `extras_dropped`).

## Codis de sortida i errors

Els de l'eina. Cap error no és propi de `repair` més enllà dels paràmetres, que es jutgen com els de `disjoint`:
un `--minimum` que no és una durada o és negatiu, un `--max-lines` que no és un nombre enter, codi `1`.
