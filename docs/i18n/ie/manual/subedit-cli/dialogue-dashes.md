# `dialogue-dashes`

<!-- machine-translated -->
> Ca págine es un traduction machinal e it ancor ne ha esset revidet de un lector del lingue.

```
subedit-cli dialogue-dashes (--add | --remove) [--range N-M|N-]
                            (--output FILE | --output-dir DIR | --in-place)
                            [--dry-run]
                            [--recursive]
                            [--document main|translation] [-t FILE] [--align-method position|number]
                            <file>...
```

Mette un **strec de dialog** al cap del lineas, o leva les. It es `Dialogue Dashes` del
[fenestre](../subedit-gui/operations.md).

**Du paroles, quam por [`italics`](italics.md)**: `--add` e `--remove` se exclude, e un es besonat.

- **`--add`** mette un strec ante chascun linea quel have null: `Bonjour` dona `- Bonjour`, e
  `Bonjour\nMarie` dona `- Bonjour\n- Marie`. **Un strec ja ibi ne es duplicat.**
- **Tri strecs es recognoscet** — li tracet-de-union del claviero e li du strecs long del
  tipografo — **e it es li tracet-de-union quel es scrit**: `— Bonjour` deveni `- Bonjour`. Spacies
  pluri pos li strec desapari.
- Li strec va **ante li textu, ne ante un etiquette aperient**: `<i>Bonjour</i>` dona
  `<i>- Bonjour</i>`. Null etiquette move se.
- **`--remove`** leva li strecs initial, in omni tri formes.

<!-- example: subedit-cli dialogue-dashes --help -->
```console
$ subedit-cli dialogue-dashes --help
Put dialogue dashes on the lines, or take them off
Usage: subedit-cli dialogue-dashes [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --add                       Put a dialogue dash at the head of the lines
  --remove                    Take the dialogue dashes off
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

## Argumentes e optiones

| Option | Obligatori | Valores | Predefinit |
| :----- | :----- | :------ | :----- |
| `<file>...` | yes | un o plu vias | — |
| `--recursive`, `-r` | no | un flag | off — vide [Tractar un arbore](lots.md) |
| `--add` / `--remove` | **un del du** | flags | — |
| `--range` | no | `N-M`, o `N-` til li fin — vide [`adjust`](adjust.md#--range) | li tot file |
| `--document` | no | `main` o `translation` — vide [Un traduction](invocation.md#un-traduction) | `main` |
| `-t`, `--translation-file` | con `--document translation` | un file de traduction, por **un sol** input | — |
| `--align-method` | no, e solmen con `-t` | `position` o `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **un del tri**, except con `--dry-run` | vide [Invocation](invocation.md#li-destination) | — |
| `--dry-run` | no | un flag: calcula e di, scri nullcos — vide [Vider ante scrir](invocation.md#vider-ante-scrir) | off |

**Li textu principal per predefinit; li traduction con `--document translation -t FILE`.** Alor it es
li traduction quel es changeat e scrit, e solmen it — vide [Un traduction](invocation.md#un-traduction).

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\nMarie\n\n2\n00:00:03,000 --> 00:00:04,000\n<i>Salut</i>\n\n' > a.srt; subedit-cli dialogue-dashes --add --output t.srt a.srt; cat t.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\nMarie\n\n2\n00:00:03,000 --> 00:00:04,000\n<i>Salut</i>\n\n' > a.srt; subedit-cli dialogue-dashes --add --output t.srt a.srt; cat t.srt
a.srt: 2 subtitles dashed -> t.srt
1
00:00:01,000 --> 00:00:02,000
- Bonjour
- Marie

2
00:00:03,000 --> 00:00:04,000
<i>- Salut</i>
```

## Output

**Output standard** — nullcos: li resultate es li file scrit. Con `--dry-run`, li liste de
changes, descrit in [Vider ante scrir](invocation.md#vider-ante-scrir).

| Nivelle | Quo es adjuntet al output de error |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles dashed -> <destination>`, o `N subtitles undashed`, o `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — li formate, li codification, li marca e li fines de linea del file leet, remisset quam ili esset |
| 3 | `<path>: N bytes read, M written`, poy **chascun diagnose de lectura** — vide [Invocation](invocation.md#diagnoses-de-lectura) |

`N` es li númere de subtitules **queles textu ha changeat**, leet del operation self.

### In JSON

Con [`--format json`](invocation.md#un-output-quel-un-script-posse-leer), li objecte de chascun file porta
`counts.changed` — li `N` del frase — e `changes`, li liste de subtitules changeat
(`{"subtitle", "document", "before", "after"}`, **vacui** quande nullcos ha changeat).

## Codes de exeunte

Celes del instrument: `0` si chascun file esset tractat — un file u nullcos changea li es —,
`2` si null, `3` si solmen alcunes, `1` in cas de error de usage.

## Errores

| Quo it causa | Missage |
| :------------------ | :------ |
| ni `--add` ni `--remove` | `dialogue-dashes needs --add to put dashes on, or --remove` |
| ambi | `--add and --remove say opposite things; give one of them` |
| intervalle mal scrit o foris li file | vide [`adjust`](adjust.md#--range) |

Li errores del destination es comun al subcomandes quel scri:
vide [Invocation](invocation.md#li-destination).
