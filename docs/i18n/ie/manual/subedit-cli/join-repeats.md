# `join-repeats`

<!-- machine-translated -->
> Ca págine es un traduction machinal e it ancor ne ha esset revidet de un lector del lingue.

```
subedit-cli join-repeats (--output FILE | --output-dir DIR | --in-place)
                         [--dry-run]
                         [--recursive]
                         <file>...
```

Fa **un sol subtitul** ex li subtitules queles monstra li sam cose un pos li altri: un textu separat in pezzes queles
sequa se, quam alcun instrumentes de segmentation les lassa. It es `Join Repeated Subtitles` del fenestre —
[li sam motor](../subedit-gui/lines.md#unir-li-subtitules-repetit).

**Un repetition** es un subtitul quel have **li sam textu, li sam traduction e li sam parametres de formate** (un
stil, un position, parametres de cue) quam un altri, **e quel comensa al plu tard quande ti-ci fini**: les
tange o superpone se. Un succession de repetitiones deveni **li subtitul quel comensa primmen**, quel fin
es portat al fin li plu tardi del succession; li altres desapare.

- **Li órdine del file ne importa e ne es changeat**: li subtitul conservat es li quel comensa primmen,
  u que li altres es. Un file in órdine resta in órdine.
- **Nequo es unit per erra**: un altri textu, un altri traduction, altri parametres, un interstice inter du
  pezzes, un subtitul sin textu o quel fin veni ante su comense — in omni ti casus les resta separat.
- **A far ante li resto**: ordinar e [decuper li superpositiones](disjoint.md) leer mey un succession neti.
  [`repair`](repair.md) encatena tri, e [`convert`](convert.md#far-disjuncte-li-intervalles) accepta it quam option (`--join-repeats`, `--repair`).

<!-- example: subedit-cli join-repeats --help -->
```console
$ subedit-cli join-repeats --help
Join the subtitles that repeat each other, one after the other, into one
Usage: subedit-cli join-repeats [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to join the repeats of

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Arguments e optiones

| Option | Obligatori | Valores | Predefinit |
| :----- | :----- | :------ | :----- |
| `<file>...` | yes | un o plu camines | — |
| `--recursive`, `-r` | no | un flag | off — vide [Tractar un arbore](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **un del tri**, except con `--dry-run` | vide [Invocation](invocation.md#li-destination) | — |
| `--dry-run` | no | un flag: calcula e di, ne scri nequo — vide [Vider ante scrir](invocation.md#vider-ante-scrir) | off |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:04,000 --> 00:00:06,000\nAlpha\n\n4\n00:00:08,000 --> 00:00:09,000\nBeta\n\n' > morceaux.srt; subedit-cli join-repeats --output joint.srt morceaux.srt; cat joint.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:04,000 --> 00:00:06,000\nAlpha\n\n4\n00:00:08,000 --> 00:00:09,000\nBeta\n\n' > morceaux.srt; subedit-cli join-repeats --output joint.srt morceaux.srt; cat joint.srt
morceaux.srt: 2 repeated subtitles joined -> joint.srt
1
00:00:01,000 --> 00:00:06,000
Alpha

2
00:00:08,000 --> 00:00:09,000
Beta
```

Li tri pezzes de `Alpha`, queles sequa se de 1 s til 6 s, fa nu solmen un. `Beta`, separat per un interstice,
resta. Un duesim passage changea nequo e di it: `no repeated subtitles`.

## Output

**Exeunte standard** — nequo: li resultate es li file scrit.

| Nivelle | Quo es adjuntet al exeunte de error |
| :----- | :------------------------------------- |
| 1 | `<path>: N repeated subtitles joined -> <destination>`, o `<path>: no repeated subtitles -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, pois **chascun diagnose de lectura** |

### In JSON

Con [`--format json`](invocation.md#un-output-quel-un-script-posse-leer), li `counts` de chascun file: `subtitles` (ti del file
scrit), `joined` (li subtitules remuvet pro que un altri les covre) e `chains` (li successiones unit).

## Codes de exeunte

Ti del instrument: `0` si chascun file esset tractat — un file sin repetitiones es tal —, `2` si null, `3` si
solmen alcun, `1` in cas de error de usage.

## Errores

Null specific a `join-repeats`: un file illegibil falie quam por chascun subcomande, e li errores
del destination es comun al subcomandes quel scri: vide [Invocation](invocation.md#li-destination).
