# `hearing-impaired`

<!-- machine-translated -->
> Ca págine es un traduction machinal e it ancor ne ha esset revidet de un lector del lingue.

```
subedit-cli hearing-impaired
                  (--output FILE | --output-dir DIR | --in-place)
                  [--dry-run]
                  [--recursive] [--parentheses]
                  [--document main|translation] [-t FILE] [--align-method position|number]
                  <file>...
```

Remove li **mentiones por surdes** — li sones descrit inter parentheses quadrat — ex li tot
file. **Li parentheses ronde resta predefinit**: les es usat tam sovente por un
parol susurrat quam por un son descrit, e remover les va deleer dialog. `--parentheses` les remove
anc, por un file quel descrie su sones in les.

Quo it ne fa: li textus de cantas inter signes de carré, li nómine del parlator
ante duo punctus, remetter majusculas, corecter errores de OCR. Ti
modelles es ti del assistente [`Correct Texts…`](../subedit-gui/correct-texts.md) del fenestre,
quel li linea de comande oferte sub li nómine [`correct`](correct.md); e nequo ci posse esser
parametrat: li transformation es decidet, ne configurabil.

<!-- example: subedit-cli hearing-impaired --help -->
```console
$ subedit-cli hearing-impaired --help
Remove the sounds described between square brackets
Usage: subedit-cli hearing-impaired [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to clean

Options:
  -h,--help                   Print this help message and exit
  --parentheses               Also remove the sounds between parentheses, often whispered lines
  -r,--recursive              Take directories as inputs, and every subtitle file in them
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
| `--parentheses` | no | un flag: remove anc li mentiones inter parentheses ronde | off |
| `--recursive`, `-r` | no | un flag | off — vide [Tractar un arbore](lots.md) |
| `--document` | no | `main` o `translation` — vide [Un traduction](invocation.md#un-traduction) | `main` |
| `-t`, `--translation-file` | con `--document translation` | un file de traduction, por **un sol** input | — |
| `--align-method` | no, e solmen con `-t` | `position` o `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **un del tri**, except con `--dry-run` | vide [Invocation](invocation.md#li-destination) | — |
| `--dry-run` | no | un flag: calcula e di, ne scri nequo — vide [Vider ante scrir](invocation.md#vider-ante-scrir) | off |

Li formate del file leet es **conservat**: changear li formate es li labor de
[`convert`](convert.md).

**Li textu principal predefinit; li traduction con `--document translation -t FILE`.** Tande it es
li traduction quel es changeat e scrit, e solmen it — vide [Un traduction](invocation.md#un-traduction).

**Un mention quel vacuarea un traduction scri it vacui, e conserva li subtitul**: li subtitul
resta in li file, sin textu. Li textu principal, in su parte, perde un subtitul vacuat in total, quam
infra.

## Quo es removet, e quo resta

Un mention es un parentese quadrat — o ronde, con `--parentheses` — e quo it inclusi, **ruptura
de linea includet**: in files real, un mention es sovente decupat per li
fine de un linea.

| Input | Output |
| :----- | :----- |
| `Bonjour [il tousse] Marie` | `Bonjour Marie` |
| `Bonjour[il tousse]Marie` | `Bonjour Marie` |
| `[Bruit de pas] Bonjour` | `Bonjour` |
| `A [un] [deux] B` | `A B` |

Li remotion lassa **exactmen un spacie inter quo it circumdat, e null a
li bordura de un linea**. It ne es un purgation del spacies del file: du spacies
quel li textu portat ja resta du spacies.

**Quande li mention chevalca li ruptura de linea, es li ruptura quel resta** — li
du lineas resta du lineas. Ca caso ne es un curiositá: tra quindec
subtitulationes real, chascun parentese quadrat quel sembla orfan esset un
mention decupat talmen.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nReculez ! [Il\nhurle] Tout de suite !\n\n' > a.srt; subedit-cli --quiet hearing-impaired --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nReculez ! [Il\nhurle] Tout de suite !\n\n' > a.srt; subedit-cli --quiet hearing-impaired --output b.srt a.srt; cat b.srt
1
00:00:01,000 --> 00:00:03,000
Reculez !
Tout de suite !
```

**Quo ne es un mention e ne move:**

| Caso | Quo evenit |
| :-- | :-------------- |
| contenete purmen numeric — `[1]`, `(12)`, `[ 156478 ]` | conservat quam it es, delimitatores includet: un referentie nequande nominat un son |
| un parentese ronde e quo it inclusi, sin `--parentheses` | conservat quam it es: `Venez (chut) ici` ne changea |
| un parentese quadrat o ronde quel nequo clude | lassat quam it es |
| un etiquette de formate — `<i>`, `</i>` | nequande removet |

Un blanc **inter** li cifres, un littre o un signe — `[1 2]`, `[1a]`,
`[-1]` — fa de it un mention. Anc un contenete vacui, `[]` o `( )`: null
cifre, dunc null referentie; nequo a dir, dunc nequo a conservar.

## Quo desapare in total

**Un linea vacuat per li remotion desapare**, e **un subtitul vacuat in
total es removet del file**. Li subtitules sequent es renumerat.

Un linea es considerat vacui quande omni quo resta in it es blancs, etiquettes de
formate, o un sol strec de dialog — `<i>[Musique]</i>` ne lassa un
`<i></i>` sur ecran.

**Li etiquettes es ti del formate del file.** `{\i1}[Musique]{\i0}` in un
Advanced SSA, `{Y:i}[Musique]` in un MicroDVD e `/[Musique]` in un MPL2 ne lassa
nequo anc. Contrarimen, un `<i>` in un formate quel scri null
etiquette — TMPlayer, LRC — es textu quel li spectator leer, e li linea quel
it porta ne es vacui.

**Un dialog reduet a un sol voce ne es plu un dialog**, e li strec quel resta
va via. Til que du voces resta, li strecs resta.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\nVoir [1] la note.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\nVoir [1] la note.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle cleaned, 1 removed -> b.srt
1
00:00:04,000 --> 00:00:06,000
Attends Marie.

2
00:00:07,000 --> 00:00:09,000
Voir [1] la note.
```

## Un file sin mention es scrit tamen

It es copiat identicmen, li code de exeunte es `0`, e li raporte di que
nequo esset a remover.

It es li regul del subcomandes quel scri: **un destination donat es
un destination scrit**. Far un exception ci obligarea un script a saver
qual del subcomandes alcun vez produe un file e alcun vez nequo.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nRien à signaler.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nRien à signaler.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt
a.srt: no mention to remove -> b.srt
```

## Output

**Exeunte standard** — nequo: li resultate es li file scrit. **Con `--dry-run`, it es li changes proposit**: por chascun subtitul quel textu changearea, su númere, li textu ante e li textu pos — vide [Vider ante scrir](invocation.md#vider-ante-scrir).

| Nivelle | Quo es adjuntet al exeunte de error |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles cleaned, M removed -> <destination>`, o `<path>: no mention to remove -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — li formate, li codification, li marca e li fines de linea del file leet, remetit quam ili esset |
| 3 | `<path>: N bytes read, M written`, pois **chascun diagnose de lectura** — vide [Invocation](invocation.md#diagnoses-de-lectura) |

`N` cunta li subtitules de quel textu changea, `M` ti quel li remotion
vacuat in total e queles dunc lassat li file.

### In JSON

Con [`--format json`](invocation.md#un-output-quel-un-script-posse-leer), li objecte de chascun file porta `counts.cleaned` (li textus rescrit) e `counts.removed` (li subtitules removet); **con `--dry-run`**, li clave `changes` anc lista li subtitules changeat, quam li exeunte standard del forme textu. Li tabelle complet es ti de [Invocation](invocation.md#quo-chascun-subcomande-pone-in-counts).

## Codes de exeunte

Ti del instrument: `0` si chascun file esset scrit, `2` si null, `3` si
solmen alcun, `1` in cas de error de usage.

## Errores

Ca subcomande have null error propri: it prende null
valore a analisar, e null textu posse far que it refusa un file. Quo
posse falir es li lectura o li scrition, quam por li altres.

**Nequo es scrit sin un del tri optiones de destination**, e lor errores
es comun al subcomandes quel scri:
vide [Invocation](invocation.md#li-destination).
