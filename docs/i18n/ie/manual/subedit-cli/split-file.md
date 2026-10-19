# `split-file`

<!-- machine-translated -->
> Ca págine es un traduction machinal e it ancor ne ha esset revidet de un lector del lingue.

```
subedit-cli split-file --at N --head FILE --tail FILE
                       [--dry-run] [--sort] [--disjoint]
                       <file>
```

Separa un file **in du**: un cap e un cauda. It es `Split Project…` del fenestre — [li sam motor](../subedit-gui/operations.md#split-project),
li sam cuntages, li sam refusa — e **li exact invers de [`append`](append.md)**: apender li cauda al
cap dona retro li file de comense.

- **`--at N` es li númere del prim subtitul del cauda**, numerat quam li tabelle del fenestre
  les numera: ex 1. `--at 3` pone li subtitules 1 e 2 in li cap, e li tertie e li resta in li
  cauda. It va **del duesim al ultim**: un decupation besona un subtitul sur chascun latere.
- **Li cauda es reducet al origine**: it es displaciat per minus li fin del ultim subtitul lassat in li cap,
  su positiones conservante lor órdine e lor intervalles. To es quo fa de `split-file` li invers de `append`.
- **Ambi mitás conserva li formate, li codification, li marca de órdine de bytes e li fines de linea del file leet**, e
  es renumerat ex 1 durant li scrition, quam omni file scrit retro.
- **Du outputs, dunc un destination propri**: `--head` e `--tail`, **ambes obligatori**. `--output`,
  `--output-dir`, `--in-place` e `--recursive` ne existe por `split-file` e es refusat quam omni option
  ne conosset, code `1`: ne es ni directoria u partir du files nominat, ni input a surscrir.

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

## Arguments e optiones

| Option | Obligatori | Valores | Predefinit |
| :----- | :----- | :------ | :----- |
| `<file>` | yes | **un sol** chemin | — |
| `--at` | yes | un númere intere, de 2 al númere de subtitules del file | — |
| `--head`, `--tail` | **yes, ambes**, except con `--dry-run` | chemins de files; lor directoria es creat si it manca | — |
| `--dry-run` | no | un flag: calcula e dis, scri null — vide [Vider ante scrir](invocation.md#vider-ante-scrir) | off |
| `--sort` | no | un flag: pone li subtitules in li órdine de lor comense **ante decuper**; `--at` cunta in ti órdine | off |
| `--disjoint` | no | un flag: decupe li subtitules superponent **ante decuper**; `--at` cunta in quo resta, vide [Far disjuncte li intervalles](#far-disjuncte-li-intervalles) | off |

**Con `--dry-run`, null destination es obligatori**: on posse donar null, ma ne solmen un del du —
`--head` e `--tail` va ensemble. Les quel es donat es judicat quam sin `--dry-run`.

**Li du destinationes posse esser ni li input ni li sam file**: scrir sur quo on
lee, o scrir li cauda sur li cap, es refusat ante que alquo es leet, code `1`.

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

Li duesim subtitul, lassat in li cap, fini a 4 s: li cauda recua de 4 s, e li tertie subtitul,
quel comensat a 6 s, comensa a 2 s.

## Separar secun li órdine de tempore

`--at` numera li subtitules **quam li file les dona**. In un file del quel li órdine es rupt, li cap dunc ne es li mitá li plu precoce
in tempore. `--sort` pone li subtitules in li órdine de lor comenses **ante judicar li decupation** — stabil, quam [`sort`](sort.md) fa —, e
`--at` cunta poy in ti órdine.

Nequande predefinit. Con li option, li file dis al prim nivelle de narration quant subtitules ha movet (`<path>: N subtitles moved`
o `<path>: already in order`), e li registre `--format json` porta un cuntage `moved`, **absent sin li option**.

## Far disjuncte li intervalles

Nequande predefinit. Li option fa quo [`disjoint`](disjoint.md) fa, **con su parametres predefinit**, ante judicar li decupation, pos li ordination: `--at` cunta in quo to dona. It besona un file in órdine: altrimen it es refusat, e `--sort` — quel veni in prim — pone it in órdine ante. Li file dis it al prim nivelle de narration (`<path>: N subtitles split, M merged`), e li registre `--format json` porta li cuntages `split` e `merged`, **absent sin li option**.

## Quande li decupation es refusat

**Du mitás queles superpone al decupation ne es decupat.** Si un subtitul del cauda comensat ante
li fin del ultim subtitul lassat retro, li displaciament portarea it ante li comense del video — un position negativ,
quel null formate scri. `split-file` refusa **nominante li subtitul**, in li paroles del fenestre:

```text
o.srt: Cannot split at subtitle 3: subtitle 3 would fall before the start of the video. Cut somewhere else.
```

Li code es `2` e **nequo es scrit**. Decuper ad altri loc. Atterrar exactmen sur li comense es permisset: zero es un
position.

**`--at` foris li file es refusat in li sam manere**, quande li file es leet: `--at 1` — li prim subtitul ne posse
comensar un cauda, nequo resterea — e `--at` pos li ultim subtitul, quel lasserea un cauda
vacui. Un file de un sol subtitul ne posse esser decupat.

## Output

**Exeunte standard** — null: li resultate es li du files scrit.

| Nivelle | Quo es adjuntet al exeunte de error |
| :----- | :------------------------------------- |
| 1 | `<path>: split at subtitle N: H subtitles in the head, T subtitles in the tail -> <head>, <tail>`; con `--dry-run`, `(dry run, nothing written)` in loc de `-> …` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — li formate, li codification, li marca e li fines de linea del file leet, remetet quam les esset |
| 3 | `<path>: N bytes read, H and T written`, poy **omni diagnose de lectura** — vide [Invocation](invocation.md#diagnoses-de-lectura) |

### In JSON

Con [`--format json`](invocation.md#un-output-quel-un-script-posse-leer), **un sol objecte**, quel `file` es li file decupat.
**`destination` es li cap** — li file in quel li input deveni — e **`tail` li cauda**, un chemin, o `null` con
`--dry-run` quam `destination`. Li `counts`: `subtitles` (les del file leet), `head` e `tail` (les de chascun
mitá).

## Codes de exeunte

| Code | Quande |
| :--- | :---- |
| `0` | ambi files es scrit — o calculat, con `--dry-run` |
| `1` | error de usage: null `--head` e `--tail`, un option quel `split-file` ne have, un destination quel es li input o li altri destination, un `--at` quel ne es un númere intere positiv |
| `2` | li file ne posset esser leet, **li decupation es refusat** (foris li file, o superposition), o un destination ne posset esser scrit. Ne existe code `3`: un sol file es tractat |

**Li cap es scrit in prim**: un disco quel refusa li cauda lassa li cap detra it, e li code es `2`.

## Errores

| Quo les causa | Missage |
| :------------------- | :------ |
| ni `--head` ni `--tail` | `no destination given: split-file writes two files, use --head and --tail` |
| solmen un del du, con `--dry-run` | `--head and --tail go together: give both, or neither with --dry-run` |
| li du destinationes es li sam file | `<path>: named as both the head and the tail` |
| un destination es li input | `<path>: would be written over the input <input>` |
| `--at` foris li file | `<file>: --at 9: the file holds 3 subtitles, and the tail can start from 2 to 3` |
| un file de un subtitul | `<file>: a file of 1 subtitle cannot be split: a cut needs a subtitle on each side` |
| superposition | `<file>: Cannot split at subtitle N: subtitle M would fall before the start of the video. Cut somewhere else.` |
| un file manca, es illegibil o de un formate ne conosset | `<path>: does not exist`… — li sam quam omniloc |
