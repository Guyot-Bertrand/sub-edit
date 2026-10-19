# `italics`

<!-- machine-translated -->
> Ca págine es un traduction machinal e it ancor ne ha esset revidet de un lector del lingue.

```
subedit-cli italics (--on | --off) [--range N-M|N-]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    <file>...
```

Mette li textus **in italic**, o **remove lor italic**, in li etiquettes del formate del file.
It es `Italic` del [fenestre](../subedit-gui/operations.md).

**Du paroles e null commutator**: li buton del fenestre demanda al core in qual direction
it ova; ci on di it. `--on` e `--off` se exclude, e un es besonat — omisser it, o donar ambi,
es un error de usage (code `1`).

- **`--on`** inviluppa chascun textu in un sol par de etiquettes, **pos haver removet ti queles
  esset ja in it**: un textu ja in italic ne es inviluppat du vezes, e italic separat
  in du es refat in un pezze. Sur plu lineas, un par covre li tot.
- **`--off`** remove li etiquettes de italic; **li resto de un etiquette es conservat** (`{\b1\i1}` dona
  `{\b1}`).
- Li textu es scrit **quam li formate del file scri li italic**: `<i>…</i>` por SubRip, WebVTT e
  SubViewer 2, `{\i1}…{\i0}` por li du Sub Station Alpha, `{Y:i}` por MicroDVD.

<!-- example: subedit-cli italics --help -->
```console
$ subedit-cli italics --help
Put the texts in italics, or take their italics out
Usage: subedit-cli italics [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --on                        Put the texts in italics
  --off                       Take the italics out
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

## Arguments e optiones

| Option | Obligatori | Valores | Predefinit |
| :----- | :----- | :------ | :----- |
| `<file>...` | yes | un o plu camines | — |
| `--recursive`, `-r` | no | un flag | off — vide [Tractar un arbore](lots.md) |
| `--on` / `--off` | **un del du** | flags | — |
| `--range` | no | `N-M`, o `N-` til li fin — vide [`adjust`](adjust.md#--range) | li tot file |
| `--document` | no | `main` o `translation` — vide [Un traduction](invocation.md#un-traduction) | `main` |
| `-t`, `--translation-file` | con `--document translation` | un file de traduction, por **un sol** input | — |
| `--align-method` | no, e solmen con `-t` | `position` o `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **un del tri**, except con `--dry-run` | vide [Invocation](invocation.md#li-destination) | — |
| `--dry-run` | no | un flag: calcula e di, ne scri nequo — vide [Vider ante scrir](invocation.md#vider-ante-scrir) | off |

**Li textu principal predefinit; li traduction con `--document translation -t FILE`.** Tande it es
li traduction quel es changeat e scrit, e solmen it — vide [Un traduction](invocation.md#un-traduction).

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour\nMarie\n\n' > a.srt; subedit-cli italics --on --output i.srt a.srt; cat i.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour\nMarie\n\n' > a.srt; subedit-cli italics --on --output i.srt a.srt; cat i.srt
a.srt: 2 subtitles put in italics -> i.srt
1
00:00:01,000 --> 00:00:02,000
<i>bonjour marie</i>

2
00:00:03,000 --> 00:00:04,000
<i>Bonjour
Marie</i>
```

## Un formate sin stil refusa

**TMPlayer e LRC scri null stil**: ne hay italic a metter ni a remover. Li
fenestre fa grisi li entrata vice celar it; li linea de comande **refusa, file per
file, con li rason**, e li altri files del lot continua. Quo importa es quo li
formate posse portar, ne li manere in qual it scri: SubRip e WebVTT scri li italic
in li sam manere, e ne di li sam cose pri li colore.

<!-- example: printf '00:00:01:Hello there\n00:00:03:Bye\n' > a.txt; subedit-cli italics --on --output-dir sortie a.txt; echo "code=$?" -->
```console
$ printf '00:00:01:Hello there\n00:00:03:Bye\n' > a.txt; subedit-cli italics --on --output-dir sortie a.txt; echo "code=$?"
a.txt: TMPlayer writes no style: there are no italics to put on
code=2
```

In JSON, li fallimente porta li identificator **`no-style`**.

## Output

**Exeunte standard** — nequo: li resultate es li file scrit. Con `--dry-run`, li liste de
changes, descrit in [Vider ante scrir](invocation.md#vider-ante-scrir).

| Nivelle | Quo es adjuntet al exeunte de error |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles put in italics -> <destination>`, o `N subtitles taken out of italics`, o `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — li formate, li codification, li marca e li fines de linea del file leet, remetit quam ili esset |
| 3 | `<path>: N bytes read, M written`, pois **chascun diagnose de lectura** — vide [Invocation](invocation.md#diagnoses-de-lectura) |

`N` es li númere de subtitules **del quel textu changea**, leet del operation self.

### In JSON

Con [`--format json`](invocation.md#un-output-quel-un-script-posse-leer), li objecte de chascun file porta
`counts.changed` — li `N` del frase — e `changes`, li liste del subtitules changeat
(`{"subtitle", "document", "before", "after"}`, **vacui** si nequo changea).

## Codes de exeunte

Ti del instrument: `0` si chascun file esset tractat — un file u nequo changea es tal —,
`2` si null, `3` si solmen alcun, `1` in cas de error de usage.

## Errores

| Quo it causa | Missage |
| :------------------ | :------ |
| ni `--on` ni `--off` | `italics needs --on to put the texts in italics, or --off` |
| ambi | `--on and --off say opposite things; give one of them` |
| un formate sin stil | `<path>: TMPlayer writes no style: there are no italics to put on` (o `take out`) |
| intervallo mal scrit o foris li file | vide [`adjust`](adjust.md#--range) |

Li errores del destination es comun al subcomandes quel scri:
vide [Invocation](invocation.md#li-destination).
