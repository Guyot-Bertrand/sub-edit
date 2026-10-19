# `sort`

<!-- machine-translated -->
> Ca págine es un traduction machinal e it ancor ne ha esset revidet de un lector del lingue.

```
subedit-cli sort (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 <file>...
```

Remette li subtitules **in li órdine de lor comense**. Nul altri chose changea: ni li textus, ni li
positiones, ni li formate del file. Li subtitules es **renumerat** ex 1 durant li scrition,
quam por omni file scrit retro.

**Li nucleo nequande ordina per su self**: leer un file lassa su órdine quam it esset, e
[`inspect`](inspect.md#quo-anomalies-reporta) reporta un ruption de órdine con
`subtitle N starts before the previous one starts`. `sort` es **li gesto fat intentionalmen**, por un file
o un lot del quel li órdine es rupt — e it posse esser anullat, in li fenestre, quam omni operation.

- **Li ordination es stabil**: du subtitules queles comensa in li sam momente **conserva li órdine
  quel li file doneva les**. Null veni ante li altri; mover les esset un decision quel null person
  ha demandat.
- **`sort` accepta ni un intervalle ni un document**: ne es ulli chose a eliger, li órdine es li del comenses.

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

## Arguments e optiones

| Option | Obligatori | Valores | Predefinit |
| :----- | :----- | :------ | :----- |
| `<file>...` | yes | un o plu chemins | — |
| `--recursive`, `-r` | no | un flag | off — vide [Tractar un arbore](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **un del tri**, except con `--dry-run` | vide [Invocation](invocation.md#li-destination) | — |
| `--dry-run` | no | un flag: calcula e dis, scri null — vide [Vider ante scrir](invocation.md#vider-ante-scrir) | off |

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

## Quo es contat

**Li cuntage es li del locs quel ha changeat de subtitul**, quam li nucleo reporta it — to quo
li historie del fenestre nominarea mover. Un sol subtitul mal metet displacia plurs: in
li exemple supra, null del tri locs conserva su subtitul, e li frase dis `3 subtitles moved`.

**Un file ja in órdine es scrit totsam**, e it dis it: un destination indicat es un
destination scrit, e far de `sort` li exception forceat un script a saver quel
subcomandes alquande produe un file e alquande null.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > ordre.srt; subedit-cli sort --output copie.srt ordre.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > ordre.srt; subedit-cli sort --output copie.srt ordre.srt
ordre.srt: already in order -> copie.srt
```

## Output

**Exeunte standard** — null: li resultate es li file scrit. Con `--dry-run`, anc null: ne es
file, e li linea de narration dis it — vide [Vider ante scrir](invocation.md#vider-ante-scrir).

| Nivelle | Quo es adjuntet al exeunte de error |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles moved -> <destination>`, or `<path>: already in order -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — li formate, li codification, li marca e li fines de linea del file leet, remetet quam les esset |
| 3 | `<path>: N bytes read, M written`, poy **omni diagnose de lectura** — vide [Invocation](invocation.md#diagnoses-de-lectura) |

### In JSON

Con [`--format json`](invocation.md#un-output-quel-un-script-posse-leer), li objecte de chascun file porta
`counts.subtitles` (omni subtitul del file) e `counts.moved` (li locs quel ha changeat;
`0` por un file ja in órdine).

## Codes de exeunte

Les del instrument: `0` si omni file esset tractat — un file ja in órdine es it —, `2` si
null, `3` si solmen alcunes, `1` in cas de un error de usage.

## Errores

Null specific a `sort`: un file illegibil falie quam por omni subcomande, e li
errores del destination es comun a su li subcomandes quel scri:
vide [Invocation](invocation.md#li-destination).
