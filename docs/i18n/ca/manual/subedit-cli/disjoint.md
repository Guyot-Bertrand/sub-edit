# `disjoint`

<!-- machine-translated -->
> Aquesta pàgina és una traducció automàtica i encara no l'ha revisada cap lector de la llengua.

```
subedit-cli disjoint (--output FILE | --output-dir DIR | --in-place)
                     [--minimum TIME] [--max-lines N] [--dashes] [--sort]
                     [--dry-run] [--recursive]
                     <file>...
```

Talla els subtítols que s'encavalquen perquè **cap franja `[start, end]` no
s'encavalqui més amb una altra**. És `Make Ranges Disjoint…` de la finestra — [el mateix motor](../subedit-gui/lines.md#fer-disjuntes-les-franges).

**No s'escurça ni s'allarga res**: en cada moment, el fitxer mostra els mateixos textos que abans. El que
canvia és la manera de tallar la línia de temps:

- es talla a **cada començament i cada final** dels subtítols d'un grup que s'encavalca;
- un interval cobert per **un sol** subtítol continua sent aquest subtítol;
- un interval cobert per **diversos** esdevé **un** subtítol les línies del qual són les seves, **en
  l'ordre dels seus començaments** — el més primerenc a la línia 1, el següent a la línia 2;
- un subtítol que no s'encavalca amb res no es toca, i l'ordre de la resta es conserva.

<!-- example: subedit-cli disjoint --help -->
```console
$ subedit-cli disjoint --help
Cut overlapping subtitles so that no range overlaps another, merging what overlaps
Usage: subedit-cli disjoint [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to make disjoint

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --minimum TIME              Shortest interval kept; a shorter one is taken over by a neighbour (default 0.08)
  --max-lines N               Most lines of a merged subtitle; more are shown in windows, 0 for no limit (default 2)
  --dashes                    Put a dash before the text of each merged subtitle
  --sort                      Put the subtitles in order of their start first; without it, a file out of order is refused
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
| `--max-lines` | no | un nombre enter: el màxim de línies d'un subtítol fusionat; `0` elimina el límit | `2` |
| `--dashes` | no | un indicador: un guió davant del text que cada subtítol aporta a un subtítol fusionat | desactivat |
| `--sort` | no | un indicador: posa els subtítols en l'ordre del seu començament **primer** — vegeu [Cal un fitxer en ordre](#cal-un-fitxer-en-ordre) | desactivat |
| `--recursive`, `-r` | no | un indicador | desactivat — vegeu [Tractar un arbre](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **una de les tres**, excepte amb `--dry-run` | vegeu [Invocació](invocation.md#la-destinació) | — |
| `--dry-run` | no | un indicador: calcula i diu, no escriu res — vegeu [Veure abans d'escriure](invocation.md#veure-abans-descriure) | desactivat |

<!-- example: printf '1\n00:00:00,000 --> 00:00:05,000\nA\n\n2\n00:00:03,000 --> 00:00:08,000\nB\n\n' > chevauche.srt; subedit-cli disjoint --output disjoint.srt chevauche.srt; cat disjoint.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:05,000\nA\n\n2\n00:00:03,000 --> 00:00:08,000\nB\n\n' > chevauche.srt; subedit-cli disjoint --output disjoint.srt chevauche.srt; cat disjoint.srt
chevauche.srt: 2 subtitles split, 1 merged -> disjoint.srt
1
00:00:00,000 --> 00:00:03,000
A

2
00:00:03,000 --> 00:00:05,000
A
B

3
00:00:05,000 --> 00:00:08,000
B
```

`A [0, 5]` i `B [3, 8]` donen tres subtítols disjunts: `A` sol, `A` i `B` junts, `B` sol. Cap
moment no es perd ni s'afegeix.

## Què decideix l'operació

**Una línia no es duplica mai.** Dos subtítols amb el mateix text donen una sola línia on
s'encavalquen, i dos trossos veïns que mostren el mateix només fan un subtítol. Altrament, el text d'un
subtítol tallat en diversos trossos **es repeteix en cadascun**: és el que hi ha a la pantalla en aquell
moment.

**Un interval molt curt l'absorbeix un veí.** Per sota de `--minimum`, un interval desapareix: el veí
**estén la seva franja per cobrir-lo**, de manera que cap moment no queda buit i res no es mou més d'aquest
llindar. El veí triat és aquell les línies del qual **ja contenen** les de l'interval; si no n'hi ha cap, el
més llarg. El text de l'interval només es perd si cap veí no el porta, i aleshores per menys del llindar.
**Un subtítol sencer més curt que el llindar no s'absorbeix mai**: es deixa, i es compta.

**Més línies que el límit: finestres.** Un subtítol fusionat que tindria més de `--max-lines` línies
es mostra **en finestres successives** d'aquesta mida, en parts iguals de la durada. Tres línies en tres
segons:

<!-- example: printf '1\n00:00:00,000 --> 00:00:03,000\nA\n\n2\n00:00:00,000 --> 00:00:03,000\nB\n\n3\n00:00:00,000 --> 00:00:03,000\nC\n\n' > trois.srt; subedit-cli disjoint --output fenetres.srt trois.srt; cat fenetres.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:03,000\nA\n\n2\n00:00:00,000 --> 00:00:03,000\nB\n\n3\n00:00:00,000 --> 00:00:03,000\nC\n\n' > trois.srt; subedit-cli disjoint --output fenetres.srt trois.srt; cat fenetres.srt
trois.srt: 0 subtitles split, 1 merged; 1 shown in windows for holding too many lines -> fenetres.srt
1
00:00:00,000 --> 00:00:01,500
A
B

2
00:00:01,500 --> 00:00:03,000
B
C
```

Quatre línies donen tres finestres, i així successivament: la finestra llisca una línia cada vegada. No es
trunca ni es torna a tallar res.

**La traducció segueix els mateixos talls**, ja que un subtítol porta tots dos textos. **El que el format porta
a més** — un estil, una posició — és el del primer subtítol que cobreix; la pèrdua es compta quan
els subtítols fusionats són diferents.

**Un subtítol el final del qual arriba abans del començament** no s'endevina: es deixa on és, i es compta.

## Cal un fitxer en ordre

Els grups són sèries de veïns: fora d'ordre, els veïns no són els que s'encavalquen. Un fitxer
els començaments del qual no estan en ordre es **refusa**, doncs, i el missatge ho diu:

<!-- example: printf '1\n00:00:03,000 --> 00:00:08,000\nB\n\n2\n00:00:00,000 --> 00:00:05,000\nA\n\n' > desordre.srt; subedit-cli disjoint --output x.srt desordre.srt; echo "code $?" -->
```console
$ printf '1\n00:00:03,000 --> 00:00:08,000\nB\n\n2\n00:00:00,000 --> 00:00:05,000\nA\n\n' > desordre.srt; subedit-cli disjoint --output x.srt desordre.srt; echo "code $?"
desordre.srt: The subtitles are not in the order of their start: sort them first. Use --sort to do that here.
code 2
```

`--sort` primer ordena — el mateix que [`sort`](sort.md) —, i ho diu. El nucli no ordena mai pel seu compte.

Una segona passada no canvia res: un cop les franges són disjuntes, no queda res a tallar, i la
frase diu `already disjoint`.

## Sortida

**Sortida estàndard** — res: el resultat és el fitxer escrit.

| Nivell | Què s'afegeix a la sortida d'error |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles split, M merged -> <destination>`, seguit després d'un `;` del que no s'ha pogut fer del tot: `K shown in windows for holding too many lines`, `K very short taken over by a neighbour`, `K subtitles shorter than the limit left whole`, `K merged with styles or positions that differ, the first kept`, `K subtitles ending before the start left alone` — o `<path>: already disjoint -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, i després **cada diagnòstic de lectura** |

`N subtitles split` compta els subtítols originals tallats en diversos trossos, `M merged` els trossos que
mostren diversos subtítols junts.

### En JSON

Amb [`--format json`](invocation.md#sortida-llegible-per-un-script), els `counts` de cada fitxer:

| Recompte | Què diu |
| :----- | :----------- |
| `subtitles` | els subtítols del fitxer escrit |
| `split`, `merged` | com a la frase |
| `windowed` | els trossos mostrats en finestres |
| `absorbed` | els intervals absorbits per un veí |
| `too_short` | els subtítols més curts que el llindar, deixats sencers |
| `ending_before_start` | els subtítols el final dels quals arriba abans del començament, deixats tal com són |
| `extras_dropped` | els trossos fusionats l'estil o la posició dels quals són diferents |
| `moved` | només amb `--sort`: els llocs que han canviat |

## Codis de sortida

Els de l'eina: `0` si s'han tractat tots els fitxers — un fitxer sense encavalcaments ho és —, `2` si
no se n'ha tractat cap, `3` si només alguns, `1` en un error d'ús.

## Errors

| Què els provoca | Missatge |
| :------------------- | :------ |
| un fitxer fora d'ordre sense `--sort` | `<path>: The subtitles are not in the order of their start: sort them first. Use --sort to do that here.` |
| `--minimum` no és una durada, o és negatiu | `--minimum: …` |
| `--max-lines` no és un nombre enter | `--max-lines: "…" is not a number of lines: expected a whole number, 0 for no limit` |

Un fitxer il·legible falla com per a qualsevol subordre; els errors de la destinació són comuns a les
subordres que escriuen: vegeu [Invocació](invocation.md#la-destinació).

## A les altres ordres

[`convert`](convert.md#fer-disjuntes-les-franges), [`append`](append.md#fer-disjuntes-les-franges) i
[`split-file`](split-file.md#fer-disjuntes-les-franges) accepten `--disjoint`, que fa el mateix amb
els paràmetres per defecte, abans d'escriure.

En un fitxer mal segmentat — un text tallat en trossos que se succeeixen, donats fora d'ordre —, ordenar i tallar
sols donen línies que s'alternen d'un tros al següent: primer [`join-repeats`](join-repeats.md), o bé
[`repair`](repair.md), que encadena les tres.
