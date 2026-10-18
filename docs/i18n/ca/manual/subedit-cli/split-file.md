# `split-file`

<!-- machine-translated -->
> Aquesta pàgina és una traducció automàtica i encara no l'ha revisada cap lector de la llengua.

```
subedit-cli split-file --at N --head FILE --tail FILE
                       [--dry-run] [--sort] [--disjoint]
                       <file>
```

Talla un fitxer **en dos**: un cap i una cua. És `Split Project…` de la finestra — [el mateix motor](../subedit-gui/operations.md#split-project),
els mateixos recomptes, el mateix refús — i **l'invers exacte d'[`append`](append.md)**: afegir la cua al
cap torna a donar el fitxer de partida.

- **`--at N` és el número del primer subtítol de la cua**, numerat com el numera la taula
  de la finestra: a partir d'1. `--at 3` posa els subtítols 1 i 2 al cap, i el tercer i la resta a la
  cua. Va **del segon a l'últim**: un tall necessita un subtítol a cada costat.
- **La cua es porta de nou a l'origen**: es desplaça menys el final de l'últim subtítol que queda al cap,
  i les seves posicions conserven el seu ordre i la seva separació. Això és el que fa de `split-file` l'invers d'`append`.
- **Totes dues meitats conserven el format, la codificació, la marca d'ordre de bytes i els finals de línia del fitxer llegit**, i
  l'escriptura les renumera a partir d'1, com qualsevol fitxer que es torna a escriure.
- **Dues sortides, i per tant una destinació pròpia**: `--head` i `--tail`, **totes dues obligatòries**. `--output`,
  `--output-dir`, `--in-place` i `--recursive` no existeixen per a `split-file` i es refusen com qualsevol opció
  desconeguda, codi `1`: no hi ha ni un directori on repartir dos fitxers amb nom, ni una entrada per sobreescriure.

<!-- example: subedit-cli split-file --help -->
```console
$ subedit-cli split-file --help
Cut a subtitle file in two, a head and a tail
Usage: subedit-cli split-file [OPTIONS] file

Positionals:
  file TEXT REQUIRED          Subtitle file to cut

Options:
  -h,--help                   Print this help message and exit
  --at N                      Number of the first subtitle of the tail, as the table numbers them
  --head FILE                 File to write the subtitles before the cut to
  --tail FILE                 File to write the subtitles from the cut on to
  --dry-run                   Work out and say what would be written, and write nothing
  --sort                      Put the subtitles in order of their start before cutting; --at counts in that order
  --disjoint                  Cut overlapping subtitles so that no range overlaps another, before cutting
```

## Arguments i opcions

| Opció | Obligatori | Valors | Per defecte |
| :----- | :----- | :------ | :----- |
| `<file>` | sí | **un sol** camí | — |
| `--at` | sí | un nombre enter, de 2 al nombre de subtítols del fitxer | — |
| `--head`, `--tail` | **sí, totes dues**, excepte amb `--dry-run` | camins de fitxer; el seu directori es crea si falta | — |
| `--dry-run` | no | un indicador: calcula i diu, no escriu res — vegeu [Veure abans d'escriure](invocation.md#veure-abans-descriure) | desactivat |
| `--sort` | no | un indicador: posa els subtítols en l'ordre del seu començament **abans de tallar**; `--at` compta en aquest ordre | desactivat |
| `--disjoint` | no | un indicador: talla els subtítols que s'encavalquen **abans de tallar**; `--at` compta en el que en queda, vegeu [Fer disjuntes les franges](#fer-disjuntes-les-franges) | desactivat |

**Amb `--dry-run`, no cal cap destinació**: es pot no donar-ne cap, però no només una de les dues —
`--head` i `--tail` van juntes. Les que es donen es jutgen com sense `--dry-run`.

**Les dues destinacions no poden ser ni l'entrada ni el mateix fitxer**: escriure sobre el que s'està
llegint, o escriure la cua sobre el cap, es refusa abans de llegir res, codi `1`.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n3\n00:00:06,000 --> 00:00:07,000\ntrois\n\n4\n00:00:08,000 --> 00:00:09,000\nquatre\n\n' > film.srt; subedit-cli split-file --at 3 --head tete.srt --tail queue.srt film.srt; cat queue.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n3\n00:00:06,000 --> 00:00:07,000\ntrois\n\n4\n00:00:08,000 --> 00:00:09,000\nquatre\n\n' > film.srt; subedit-cli split-file --at 3 --head tete.srt --tail queue.srt film.srt; cat queue.srt
film.srt: split at subtitle 3: 2 subtitles in the head, 2 subtitles in the tail -> tete.srt, queue.srt
1
00:00:02,000 --> 00:00:03,000
trois

2
00:00:04,000 --> 00:00:05,000
quatre
```

El segon subtítol, que queda al cap, acaba a 4 s: la cua retrocedeix 4 s, i el tercer subtítol,
que començava a 6 s, comença a 2 s.

## Tallar en ordre temporal

`--at` numera els subtítols **tal com els dona el fitxer**. En un fitxer amb l'ordre trencat, el cap no és, doncs, la meitat més primerenca
en el temps. `--sort` posa els subtítols en l'ordre dels seus començaments **abans de jutjar el tall** — de manera estable, com ho fa [`sort`](sort.md) —, i
aleshores `--at` compta en aquest ordre.

Mai per defecte. Amb l'opció, el fitxer diu al primer nivell de narració quants subtítols s'han mogut (`<path>: N subtitles moved`
o `<path>: already in order`), i el registre de `--format json` porta un recompte `moved`, **absent sense l'opció**.

## Fer disjuntes les franges

Mai per defecte. L'opció fa el que fa [`disjoint`](disjoint.md), **amb els seus paràmetres per defecte**, abans de jutjar el tall, després de l'ordenació: `--at` compta en el que això dona. Necessita un fitxer en ordre: altrament es refusa, i `--sort` — que va primer — l'ordena abans. El fitxer ho diu al primer nivell de narració (`<path>: N subtitles split, M merged`), i el registre de `--format json` porta els recomptes `split` i `merged`, **absents sense l'opció**.

## Quan el tall es refusa

**Dues meitats que s'encavalquen al tall no es tallen.** Si un subtítol de la cua comencés abans
del final de l'últim subtítol que queda enrere, el desplaçament el portaria abans del començament del vídeo — una posició negativa,
que cap format no escriu. `split-file` es refusa **anomenant el subtítol**, amb les paraules de la finestra:

```text
o.srt: Cannot split at subtitle 3: subtitle 3 would fall before the start of the video. Cut somewhere else.
```

El codi és `2` i **no s'escriu res**. Talleu en un altre lloc. Arribar exactament al començament està permès: zero és una
posició.

**Un `--at` fora del fitxer es refusa de la mateixa manera**, un cop llegit el fitxer: `--at 1` — el primer subtítol no pot
començar una cua, no quedaria res — i un `--at` més enllà de l'últim subtítol, que deixaria una cua
buida. Un fitxer d'un sol subtítol no es pot tallar.

## Sortida

**Sortida estàndard** — res: el resultat són els dos fitxers escrits.

| Nivell | Què s'afegeix a la sortida d'error |
| :----- | :------------------------------------- |
| 1 | `<path>: split at subtitle N: H subtitles in the head, T subtitles in the tail -> <head>, <tail>`; amb `--dry-run`, `(dry run, nothing written)` en lloc de `-> …` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — el format, la codificació, la marca i els finals de línia del fitxer llegit, restituïts tal com eren |
| 3 | `<path>: N bytes read, H and T written`, i després **cada diagnòstic de lectura** — vegeu [Invocació](invocation.md#diagnòstics-de-lectura) |

### En JSON

Amb [`--format json`](invocation.md#sortida-llegible-per-un-script), **un sol objecte**, el `file` del qual és el fitxer tallat.
**`destination` és el cap** — el fitxer en què es converteix l'entrada — i **`tail` la cua**, un camí, o `null` amb
`--dry-run` com `destination`. Els `counts`: `subtitles` (els del fitxer llegit), `head` i `tail` (els de cada
meitat).

## Codis de sortida

| Codi | Quan |
| :--- | :---- |
| `0` | s'han escrit tots dos fitxers — o calculat, amb `--dry-run` |
| `1` | error d'ús: cap `--head` i `--tail`, una opció que `split-file` no té, una destinació que és l'entrada o l'altra destinació, un `--at` que no és un nombre enter positiu |
| `2` | el fitxer no s'ha pogut llegir, **el tall es refusa** (fora del fitxer, o encavalcament), o una destinació no s'ha pogut escriure. No hi ha codi `3`: es tracta un sol fitxer |

**El cap s'escriu primer**: un disc que refusa la cua deixa el cap escrit, i el codi és `2`.

## Errors

| Què els provoca | Missatge |
| :------------------- | :------ |
| ni `--head` ni `--tail` | `no destination given: split-file writes two files, use --head and --tail` |
| només una de les dues, amb `--dry-run` | `--head and --tail go together: give both, or neither with --dry-run` |
| les dues destinacions són el mateix fitxer | `<path>: named as both the head and the tail` |
| una destinació és l'entrada | `<path>: would be written over the input <input>` |
| `--at` fora del fitxer | `<file>: --at 9: the file holds 3 subtitles, and the tail can start from 2 to 3` |
| un fitxer d'un subtítol | `<file>: a file of 1 subtitle cannot be split: a cut needs a subtitle on each side` |
| encavalcament | `<file>: Cannot split at subtitle N: subtitle M would fall before the start of the video. Cut somewhere else.` |
| un fitxer que falta, il·legible o d'un format desconegut | `<path>: does not exist`… — el mateix que arreu |
