# `replace`

<!-- machine-translated -->
> Ca págine es un traduction machinal e it ancor ne ha esset revidet de un lector del lingue.

```
subedit-cli replace [--regex] [--case-sensitive] [--range N-M|N-]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    <pattern> <replacement> <file>...
```

Substitue un textu in li subtitules **sin rupter un etiquette**. It es li
operation `Replace All` del [sercha](../subedit-gui/find-and-replace.md) del fenestre, con su regules:

- **li sercha es in li textu visibil** — quo li spectator lee. Li etiquettes ne es parte
  de it: serchar `<i>` trova nequo, e serchar `Bonjour` trova un «Bonjour» del quel un mitá
  es in italic;
- **li substitution es in li textu fonte**, li del file, conservante li etiquettes u les
  esset. Un etiquette quel decupa li parol serchat inclúde li tot substitution; un stil quel
  tange li concordantie covre li tot substitution; quo li concordantie ne tange
  ne move. Li regules, casu por casu, es les del sercha del fenestre.

**Ne existe subcomande `find`**: por vider u un modelle es sin changear nequo,
it es [`--dry-run`](invocation.md#vider-ante-scrir) quel lista chascun subtitul concernet.

<!-- example: subedit-cli replace --help -->
```console
$ subedit-cli replace --help
Replace a text in the subtitles, without breaking a tag
Usage: subedit-cli replace [OPTIONS] pattern replacement files...

Positionals:
  pattern TEXT REQUIRED       Text to look for in what is shown
  replacement TEXT REQUIRED   What replaces it, tags allowed
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --regex                     Read the text to look for as a regular expression
  --case-sensitive            Tell capitals from small letters
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
| `<pattern>` | yes | li textu a serchar in quo es monstrat; **ne vacui** | — |
| `<replacement>` | yes | quo substitue it, etiquettes permisset; posse esser vacui | — |
| `<file>...` | yes | un o plu chemins | — |
| `--recursive`, `-r` | no | un flag | off — vide [Tractar un arbore](lots.md) |
| `--regex` | no | un flag: li modelle es un expression regulari ICU | textu simplic |
| `--case-sensitive` | no | un flag: distinde majusculas de minusculas | les es ignorat |
| `--range` | no | `N-M`, o `N-` til li fine — vide [`adjust`](adjust.md#--range) | li tot file |
| `--document` | no | `main` o `translation` — vide [Un traduction](invocation.md#un-traduction) | `main` |
| `-t`, `--translation-file` | con `--document translation` | un file de traduction, por **un sol** input | — |
| `--align-method` | no, e solmen con `-t` | `position` o `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **un del tri**, except con `--dry-run` | vide [Invocation](invocation.md#li-destination) | — |
| `--dry-run` | no | un flag: calcula e dis, scri null — vide [Vider ante scrir](invocation.md#vider-ante-scrir) | off |

**Un modelle quel comensa con un strec** es scrit pos `--`: `subedit-cli replace -- -- "—" film.srt`.

**Li textu principal per predefinition; li traduction con `--document translation -t FILE`.** It es tunc
li traduction quel es changeat e scrit, e solmen it — vide [Un traduction](invocation.md#un-traduction).

### Textu simplic, o expression

Per predefinition li modelle es **leet quam it es**: `a.b` sercha un `a`, un punctu, un `b`. Li casu
de littere es ignorat con li sam finessa quam in li fenestre — `É` e `é` es un sol littere.

Con `--regex`, li modelle es un expression regulari **ICU**, leet quam Gaupol lee it: `.` trapassa
un ruptura de linea, `^` e `$` vale a chascun linea de un subtitul. In li substitution, e
solmen ibi:

| Scrir | Por dir |
| :----- | :-------- |
| `$1` to `$9` | li gruppes del concordantie |
| `$0` | li tot concordantie |
| `\n` | un ruptura de linea |
| `\$`, `\\` | un `$`, un backslash |

Atende al apostrofes del shell: `'$1 EUR'`, e ne `"$1 EUR"`, quel lasserea li shell leer `$1`.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nPrix: 12 euros, et 7 euros.\n\n' > prix.srt; subedit-cli replace --regex '([0-9]+) euros' '$1 EUR' --output euros.srt prix.srt; cat euros.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nPrix: 12 euros, et 7 euros.\n\n' > prix.srt; subedit-cli replace --regex '([0-9]+) euros' '$1 EUR' --output euros.srt prix.srt; cat euros.srt
prix.srt: replaced 2 matches -> euros.srt
1
00:00:01,000 --> 00:00:02,000
Prix: 12 EUR, et 7 EUR.
```

## Etiquettes

Un textu u un etiquette decupa li parol serchat:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace Bonjour Salut --output salut.srt balises.srt; cat salut.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace Bonjour Salut --output salut.srt balises.srt; cat salut.srt
balises.srt: replaced 2 matches -> salut.srt
1
00:00:01,000 --> 00:00:02,000
<i>Salut</i> tout le monde

2
00:00:03,000 --> 00:00:04,000
Salut <i>Marie</i>, ca va
```

In li prim subtitul, li etiquette decupat «Bonjour»: it inclúde ora «Salut».
In li duesim, it ne es tanget e resta u it es. **Un etiquette lassat aperit
resta aperit**: reparar un marcation dubtosi esset un altri operation.

Li substitution es leet in **li etiquettes del formate del file**: `<i>…</i>` por SubRip,
`{\i1}…` por Advanced SSA. It es li sol manere de formatar quo on scri.

## Nequo trovat, nequo a changear

Du responses queles ne es fallimentes — li code de exeunte es `0` e **li file es scrit
totsam**, un destination donat es un destination scrit:

| Situation | Frase |
| :-------- | :----- |
| li modelle es nulloc | `"zzz" not found` |
| it es ibi, ma chascun concordantie es substituet per it self | `nothing to change` |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli replace zzz x --output b.srt a.srt; subedit-cli replace Bonjour Bonjour --output c.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli replace zzz x --output b.srt a.srt; subedit-cli replace Bonjour Bonjour --output c.srt a.srt
a.srt: "zzz" not found -> b.srt
a.srt: nothing to change -> c.srt
```

## `--dry-run`

Li exeunte standard lista **chascun subtitul del quel li textu va changear**, ante e pos, quam
les es in li file — etiquettes includet:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace --dry-run Bonjour Salut balises.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace --dry-run Bonjour Salut balises.srt
balises.srt: replaced 2 matches (dry run, nothing written)
balises.srt: subtitle 1
- <i>Bon</i>jour tout le monde
+ <i>Salut</i> tout le monde
balises.srt: subtitle 2
- Bonjour <i>Marie</i>, ca va
+ Salut <i>Marie</i>, ca va
```

Li forme del liste es descrit in [Vider ante scrir](invocation.md#vider-ante-scrir);
in JSON, it es `changes`.

## Output

**Exeunte standard** — null: li resultate es li file scrit. Con `--dry-run`, li liste de
changes supra.

| Nivelle | Quo es adjuntet al exeunte de error |
| :----- | :------------------------------------- |
| 1 | `<path>: replaced N matches -> <destination>`, or `"<pattern>" not found`, or `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — li formate, li codification, li marca e li fines de linea del file leet, remetet quam les esset |
| 3 | `<path>: N bytes read, M written`, poy **omni diagnose de lectura** — vide [Invocation](invocation.md#diagnoses-de-lectura) |

`N` cunta **li concordanties substituet in li subtitules queles textu ha changeat** — it es li
númere quel li fenestre dis anc.

### In JSON

Con [`--format json`](invocation.md#un-output-quel-un-script-posse-leer), li objecte de chascun file porta:

| Clave | Contenete |
| :-- | :------ |
| `counts.replaced` | li concordanties substituet, in li subtitules queles textu ha changeat: li `N` del frase |
| `counts.matched` | li concordanties trovat, **che li substitution ha changeat li textu o ne** — to es quo distinde «nequo trovat» (`0`) de «nequo a changear» (`replaced` a `0`, `matched` positiv) |
| `changes` | li subtitules changeat: `{"subtitle", "document", "before", "after"}`, **vacui** quande nequo ha changeat |

## Codes de exeunte

Les del instrument: `0` si omni file esset tractat — un file u nequo esset trovat
es it —, `2` si null, `3` si solmen alcunes, `1` in cas de un error de usage.

## Errores

| Quo it causa | Missage |
| :------------------ | :------ |
| modelle vacui | `pattern: nothing to look for` |
| expression illegibil | `pattern: not a regular expression (<ICU's reason>)` |
| intervalle mal scrit o foris li file | vide [`adjust`](adjust.md#--range) |

**Li modelle es leet ante li prim file**: un expression illegibil es un error de usage
(code `1`), nequo es leet ni scrit.

Li errores del destination es comun a su li subcomandes quel scri:
vide [Invocation](invocation.md#li-destination).
