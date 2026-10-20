# `case`

<!-- machine-translated -->
> Ca págine es un traduction machinal e it ancor ne ha esset revidet de un lector del lingue.

```
subedit-cli case --to title|sentence|upper|lower [--range N-M|N-]
                 (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 [--document main|translation] [-t FILE] [--align-method position|number]
                 <file>...
```

Mette li textus in **majusculas de titul, majuscul de frase, majusculas o minusculas**, sin tuchar un
etiquette. It es `Instrumentarium ▸ Min/MAJ` del [fenestre](../subedit-gui/operations.md), con su regules.

| `--to` | Efecte | `bonjour marie` deveni |
| :----- | :---- | :---------------------- |
| `title` | chascun parol prende un majuscul, li resta devale | `Bonjour Marie` |
| `sentence` | li unesim littere del textu, e it solmen; li resta devale | `Bonjour marie` |
| `upper` | omnicos in majusculas | `BONJOUR MARIE` |
| `lower` | omnicos in minusculas | `bonjour marie` |

- **Quo veni ante li unesim littere o li unesim cifre ne es tuchat**: un strec de
  dialog, un signe de citation aperient o un parentese resta (`- bonjour` dona `- Bonjour`).
- **Li etiquettes ne es textu**: ili ne changea casu — `<I>` resta `<I>` — e ne
  changea loc: un etiquette quel divide un parol ancor divide it pos.
- **ICU decide u un parol comensa**: `l'été sera chaud` dona `L'été Sera Chaud`, e li accentus
  resta (`éléonore` dona `Éléonore`).

<!-- example: subedit-cli case --help -->
```console
$ subedit-cli case --help
Put the texts in title, sentence, upper or lower case, tags intact
Usage: subedit-cli case [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --to title|sentence|upper|lower
                              The case to put the texts in
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
| `--to` | **yes** | `title`, `sentence`, `upper`, `lower` | — |
| `--range` | no | `N-M`, o `N-` til li fin — vide [`adjust`](adjust.md#--range) | li tot file |
| `--document` | no | `main` o `translation` — vide [Un traduction](invocation.md#un-traduction) | `main` |
| `-t`, `--translation-file` | con `--document translation` | un file de traduction, por **un sol** input | — |
| `--align-method` | no, e solmen con `-t` | `position` o `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **un del tri**, except con `--dry-run` | vide [Invocation](invocation.md#li-destination) | — |
| `--dry-run` | no | un flag: calcula e di, scri nullcos — vide [Vider ante scrir](invocation.md#vider-ante-scrir) | off |

**Li textu principal per predefinit; li traduction con `--document translation -t FILE`.** Alor it es
li traduction quel es changeat e scrit, e solmen it — vide [Un traduction](invocation.md#un-traduction).

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\n- BONJOUR MARIE\n\n' > a.srt; subedit-cli case --to title --output titre.srt a.srt; cat titre.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\n- BONJOUR MARIE\n\n' > a.srt; subedit-cli case --to title --output titre.srt a.srt; cat titre.srt
a.srt: 2 subtitles recased -> titre.srt
1
00:00:01,000 --> 00:00:02,000
<i>Bon</i>jour Marie

2
00:00:03,000 --> 00:00:04,000
- Bonjour Marie
```

Un file u **null textu changea** — omnicos es ja in li casu demandat — es scrit quam it es e
di it:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBONJOUR\n\n' > a.srt; subedit-cli case --to upper --output b.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBONJOUR\n\n' > a.srt; subedit-cli case --to upper --output b.srt a.srt
a.srt: nothing to change -> b.srt
```

## Output

**Output standard** — nullcos: li resultate es li file scrit. Con `--dry-run`, li liste de
changes, descrit in [Vider ante scrir](invocation.md#vider-ante-scrir).

| Nivelle | Quo es adjuntet al output de error |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles recased -> <destination>`, o `nothing to change` |
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
| `--to` mancant | `--to is required` |
| un valore quel ne es un del quar | `--to: … not in {title,sentence,upper,lower}` |
| intervalle mal scrit o foris li file | vide [`adjust`](adjust.md#--range) |

Li errores del destination es comun al subcomandes quel scri:
vide [Invocation](invocation.md#li-destination).
