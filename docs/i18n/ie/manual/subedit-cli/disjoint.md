# `disjoint`

<!-- machine-translated -->
> Ca págine es un traduction machinal e it ancor ne ha esset revidet de un lector del lingue.

```
subedit-cli disjoint (--output FILE | --output-dir DIR | --in-place)
                     [--minimum TIME] [--max-lines N] [--dashes] [--sort]
                     [--dry-run] [--recursive]
                     <file>...
```

Decupa li subtitules superponent se por que **null intervallo `[start, end]` se superpone
a un altri**. It es `Make Ranges Disjoint…` del fenestre — [li sam motor](../subedit-gui/lines.md#far-disjuncte-li-intervalles).

**Nequo es curtat ni allongat**: a chascun momente, li file monstra li sam textus quam ante. Quo
changea es li manere in qual li linea de tempore es decupat:

- it es decupat a **chascun comense e chascun fin** del subtitules de un gruppe superponent se;
- un intervalle covrit per **un sol** subtitul resta ti subtitul;
- un intervalle covrit per **plu** deveni **un** subtitul quel lineas es les de ili, **in
  li órdine de lor comenses** — li plu precoce sur linea 1, li sequent sur linea 2;
- un subtitul quel se superpone a null altri ne es tangit, e li órdine del resto es conservat.

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

## Arguments e optiones

| Option | Obligatori | Valores | Predefinit |
| :----- | :----- | :------ | :----- |
| `<file>...` | yes | un o plu camines | — |
| `--minimum` | no | un duration: `0.08`, `1.5`, `00:00:00,080`. Zero conserva chascun intervallo | `0.08` (circa du fotogrammas) |
| `--max-lines` | no | un númere integri: li max de lineas de un subtitul coalescet; `0` leva li limite | `2` |
| `--dashes` | no | un flag: un strec ante li textu quel chascun subtitul apporta a un subtitul coalescet | off |
| `--sort` | no | un flag: mette li subtitules in li órdine de lor comense **primmen** — vide [Un file in órdine es necessi](#un-file-in-órdine-es-necessi) | off |
| `--recursive`, `-r` | no | un flag | off — vide [Tractar un arbore](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **un del tri**, except con `--dry-run` | vide [Invocation](invocation.md#li-destination) | — |
| `--dry-run` | no | un flag: calcula e di, ne scri nequo — vide [Vider ante scrir](invocation.md#vider-ante-scrir) | off |

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

`A [0, 5]` e `B [3, 8]` dona tri subtitules disjuncte: `A` sol, `A` e `B` together, `B` sol. Null
momente es perdit ni adjuntet.

## Quo li operation decide

**Un linea es nequande duplicat.** Du subtitules con li sam textu dona un sol linea u ili
se superpone, e du pezzes vicin queles monstra li sam cose fa solmen un subtitul. Li textu de un
subtitul decupat in plu pezzes es in li altri casus **repetit in chascun**: it es quo es sur ecran a ti
momente.

**Un intervallo mult curt es prendet per un vicin.** Sub `--minimum`, un intervallo desapare: li vicin
**extende su intervallo por covrir it**, por que null momente resta vacui e nequo move se de plu quam ti
límite. Li vicin electet es ti quel lineas **contene ja** les del intervallo, altrimen li
plu long. Li textu del intervallo es perdit solmen si null vicin lo porta, e tande por mey quam li límite.
**Un subtitul tot intr curtar quam li límite es nequande prendet**: it es lassat, e contat.

**Plu lineas quam li limite: fenestres.** Un subtitul coalescet quel haverea plu quam `--max-lines` lineas
es monstrat **in fenestres successiv** de ti grandore, in partes egal del duration. Tri lineas durant tri
secundes:

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

Quar lineas dona tri fenestres, e talmen pluse: li fenestre glissa un linea a un vez. Nequo es
truncat ni decupat denov.

**Li traduction seque li sam decupationes**, pro que un subtitul porta ambi textus. **Quo li formate porta
in plu** — un stil, un position — es ti del prim subtitul quel covre; li perdition es contat quande
li subtitules coalescet diffre.

**Un subtitul quel fin veni ante su comense** ne es divinat: it es lassat u it es, e contat.

## Un file in órdine es necessi

Li gruppes es successiones de vicins: foris de órdine, li vicins ne es ti queles se superpone. Un file
quel comenses ne es in órdine es dunc **refusat**, e li missage di it:

<!-- example: printf '1\n00:00:03,000 --> 00:00:08,000\nB\n\n2\n00:00:00,000 --> 00:00:05,000\nA\n\n' > desordre.srt; subedit-cli disjoint --output x.srt desordre.srt; echo "code $?" -->
```console
$ printf '1\n00:00:03,000 --> 00:00:08,000\nB\n\n2\n00:00:00,000 --> 00:00:05,000\nA\n\n' > desordre.srt; subedit-cli disjoint --output x.srt desordre.srt; echo "code $?"
desordre.srt: The subtitles are not in the order of their start: sort them first. Use --sort to do that here.
code 2
```

`--sort` fa li ordination primmen — li sam quam [`sort`](sort.md) —, e di it. Li core nequande ordina per se self.

Un duesim passage changea nequo: quande li intervalles es disjuncte, null cose resta a decuper, e li
frase di `already disjoint`.

## Output

**Exeunte standard** — nequo: li resultate es li file scrit.

| Nivelle | Quo es adjuntet al exeunte de error |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles split, M merged -> <destination>`, sequet pos un `;` per quo ne posse esser fat in plen: `K shown in windows for holding too many lines`, `K very short taken over by a neighbour`, `K subtitles shorter than the limit left whole`, `K merged with styles or positions that differ, the first kept`, `K subtitles ending before the start left alone` — o `<path>: already disjoint -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, pois **chascun diagnose de lectura** |

`N subtitles split` cunta li subtitules original decupat in plu pezzes, `M merged` li pezzes queles
monstra plu subtitules together.

### In JSON

Con [`--format json`](invocation.md#un-output-quel-un-script-posse-leer), li `counts` de chascun file:

| Cuntage | Quo it di |
| :----- | :----------- |
| `subtitles` | li subtitules del file scrit |
| `split`, `merged` | quam in li frase |
| `windowed` | li pezzes monstrat in fenestres |
| `absorbed` | li intervalles prendet per un vicin |
| `too_short` | li subtitules plu curt quam li límite, lassat intact |
| `ending_before_start` | li subtitules quel fin veni ante lor comense, lassat sol |
| `extras_dropped` | li pezzes coalescet quel stil o position diffre |
| `moved` | solmen con `--sort`: li locs quel changea |

## Codes de exeunte

Ti del instrument: `0` si chascun file esset tractat — un file sin superpositiones es tal —, `2` si
null, `3` si solmen alcun, `1` in cas de error de usage.

## Errores

| Quo les causa | Missage |
| :------------------- | :------ |
| un file foris de órdine sin `--sort` | `<path>: The subtitles are not in the order of their start: sort them first. Use --sort to do that here.` |
| `--minimum` ne es un duration, o es negativ | `--minimum: …` |
| `--max-lines` ne es un númere integri | `--max-lines: "…" is not a number of lines: expected a whole number, 0 for no limit` |

Un file illegibil falie quam por chascun subcomande; li errores del destination es comun al
subcomandes quel scri: vide [Invocation](invocation.md#li-destination).

## In li altri comandes

[`convert`](convert.md#far-disjuncte-li-intervalles), [`append`](append.md#far-disjuncte-li-intervalles) e
[`split-file`](split-file.md#far-disjuncte-li-intervalles) accepta `--disjoint`, quel fa li sam cose con
li parametres predefinit, ante scrir.

Sur un file mal segmentat — un textu decupat in pezzes queles sequa se, donat foris de órdine —, ordinar e decuper
sol dona lineas queles oscilla de un pezze al altri: [`join-repeats`](join-repeats.md) primmen, o
[`repair`](repair.md), quel encatena tri.
