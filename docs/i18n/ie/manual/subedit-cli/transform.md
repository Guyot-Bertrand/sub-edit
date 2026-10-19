# `transform`

<!-- machine-translated -->
> Ca págine es un traduction machinal e it ancor ne ha esset revidet de un lector del lingue.

```
subedit-cli transform --first <index>=<time> --last <index>=<time>
                      (--output FILE | --output-dir DIR | --in-place)
                      [--dry-run]
                      [--recursive]
                      <file>...
```

Corecte **chascun** position del file ex du punctus del quel li valore just
es conosset. It es li operation quel un subtitulation derivant besona
— just al comense, fals al fin — u [`shift`](shift.md), quel move omni chose del
sam quantité, ne posse far nequo.

Li formate del file leet es **conservat**: changear li formate es li labor de
[`convert`](convert.md).

<!-- example: subedit-cli transform --help -->
```console
$ subedit-cli transform --help
Correct every position from two points known to be right
Usage: subedit-cli transform [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to transform

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --first TEXT REQUIRED       Earlier reference, as <index>=<time>: 1=00:00:01.000
  --last TEXT REQUIRED        Later reference, as <index>=<time>: 3=00:00:10.000
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
| `--first` | **yes** | un referentie, `<index>=<time>` | — |
| `--last` | **yes** | un referentie, `<index>=<time>` | — |
| `--output` / `--output-dir` / `--in-place` | **un del tri**, except con `--dry-run` | vide [Invocation](invocation.md#li-destination) | — |
| `--dry-run` | no | un flag: calcula e dis, scri null — vide [Vider ante scrir](invocation.md#vider-ante-scrir) | off |

Ne existe forme curt: ti du optiones es solmen scrit in
tot.

## Qualmen scrir un referentie

Un referentie nomina un subtitul e dis u su **comense** deve cader ora:

```
<index>=<time>
```

**Li index es contat ex 1**, quam it es monstrat e quam un file SubRip
numera it. `0` es refusat, e anc un index pos li ultim
subtitul.

**Li tempore es scrit quam li de [`shift`](shift.md)** — secundas o
marca de tempore, li separator decimal un punctu, li resolution li millisecunda:

| Forme | Exemples |
| :---- | :------- |
| secundas | `10`, `2.999` |
| marca de tempore | `00:00:10.000`, `01:39:37.040` |

`--first 3=10` e `--first 3=00:00:10.000` dis li sam chose.

`--first` e `--last` nomina du punctus, ne un intervalle. Lor nómines dis
li usage expectat — un referentie al comense del file, li altri al fine — e ne un
restriction: permutar les dona li sam resultate, e **omni chose foris
li du referenties seque quam quo es inter les**.

## Quo li altri positiones deveni

Li du referenties determina un factor de scale, e chascun position seque it:

```
t′ = y₁ + (t − x₁) × r      with r = (y₂ − y₁) / (x₂ − x₁)
```

`x₁`, `x₂` es li comenses actual del du subtitules nominat, `y₁`, `y₂` li
tempores demandat. Li factor es tenet quam un rational exact e **li arondation
ocurre solmen un vice**, al millisecunda; to es quo fa que li du
referenties cade **exactmen** sur li tempores demandat e ne a un millesim apu
(vide [ADR 0013](../../../../adr/0013-mise-a-l-echelle-exacte-des-positions.md)).

Chascun subtitul resta sur ecran **proportionalmen** a quant long it restava ibi:
li fin seque li sam scale quam li comense. Un transformation ne es un
displaciament, e ne have su propriété.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,001 --> 00:00:07,000\nDeux.\n\n3\n00:00:09,000 --> 00:00:11,000\nTrois.\n\n' > a.srt; subedit-cli transform --first 1=00:00:01.000 --last 3=00:00:10.000 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,001 --> 00:00:07,000\nDeux.\n\n3\n00:00:09,000 --> 00:00:11,000\nTrois.\n\n' > a.srt; subedit-cli transform --first 1=00:00:01.000 --last 3=00:00:10.000 --output b.srt a.srt; cat b.srt
a.srt: 3 subtitles transformed onto 1=00:00:01.000 and 3=00:00:10.000 -> b.srt
1
00:00:01,000 --> 00:00:03,250
Un.

2
00:00:05,501 --> 00:00:07,750
Deux.

3
00:00:10,000 --> 00:00:12,250
Trois.
```

Li prim referentie ne move, li tertie ea de 9 s a 10 s: li factor es
`9/8`, e li duesim subtitul — quel null person ha nominat — ea de
`5,001` a `5,501`, to es `1000 + 4001 × 9/8` arondat un vice.

## Du referenties queles ne es tal

| Situation | Quande it es vidit | Code |
| :-------- | :------------- | :--- |
| `--first` e `--last` nomina li sam index | ante omni lectura | `1` |
| un index es pos li ultim subtitul | quande li file es aperit | `2` o `3` |
| li du subtitules nominat comensa in li sam momente | quande li file es aperit | `2` o `3` |

Omni tri es refusat por li sam ration: `x₂ − x₁` es zero, e du punctus
coincidente defini null linea. Retornar un division per zero vestit quam un
resultate esset pejor que refusar.

Li prim casu es un **error de usage** — li du optiones es in fallida quelcunc
li files, dunc un sol missage es debit, ne un per file. Li
altri du depende del file aperit, e dunc compta quam un fallimente de
tractation.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n' > a.srt; subedit-cli transform --first 2=1.000 --last 2=4.000 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n' > a.srt; subedit-cli transform --first 2=1.000 --last 2=4.000 --output b.srt a.srt; echo "code=$?"
--first and --last both name subtitle 2, and two references on one subtitle define no transform
code=1
```

Un index foris li limites es refusat **nominante li limite**, quo dis simultanmen
quo posset haver esset scrit:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,000 --> 00:00:07,000\nDeux.\n\n' > a.srt; subedit-cli transform --first 1=1.000 --last 9=10.000 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,000 --> 00:00:07,000\nDeux.\n\n' > a.srt; subedit-cli transform --first 1=1.000 --last 9=10.000 --output b.srt a.srt; echo "code=$?"
a.srt: subtitle 9 is past the end: the file holds 2 subtitles
code=2
```

## Un position ante li origine es refusat

Quam por [`shift`](shift.md), e por li sam ration: un position negativ
es scrit `-00:00:01,000`, un forme quel null del nov formates define e quel null
lector accepta. Li casu sucede ci quande un subtitul es **ante
li prim referentie** e li scale pusha it ultra zero. Nequo es
scrit, li code es `2`, e li missage nomina li subtitul in fallida.

## Output

**Exeunte standard** — null: li resultate es li file scrit. Con `--dry-run`, anc null: ne es file, e li linea de narration dis it — vide [Vider ante scrir](invocation.md#vider-ante-scrir).

| Nivelle | Quo es adjuntet al exeunte de error |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles transformed onto 1=00:00:01.000 and 3=00:00:10.000 -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — li formate, li codification, li marca e li fines de linea del file leet, remetet quam les esset |
| 3 | `<path>: N bytes read, M written`, poy **omni diagnose de lectura** — vide [Invocation](invocation.md#diagnoses-de-lectura) |

Li referenties es scrit retro con li punctu decimal, quelcunc qualmen les esset tipat.

### In JSON

Con [`--format json`](invocation.md#un-output-quel-un-script-posse-leer), li objecte de chascun file porta `counts.subtitles` (omni subtitul del file). Li tabelle complet es li de [Invocation](invocation.md#quo-chascun-subcomande-pone-in-counts).

## Codes de exeunte

Les del instrument: `0` si omni file esset scrit, `2` si null, `3` si
solmen alcunes, `1` in cas de un error de usage.

## Errores

| Quo it causa | Missage |
| :------------------ | :------ |
| referentie sin `=` | `"…" is not a reference: write it <index>=<time>, as in 3=00:00:10.000` |
| referentie con un mitá mancant | `"…" is missing one of its two halves: write it <index>=<time>, as in 3=00:00:10.000` |
| index `0` | `"0" is not a subtitle number: subtitles are counted from 1, as the file shows them` |
| index quel ne es un númere intere | `"…" is not a subtitle number: expected a whole number, counted from 1 as the file shows them` |
| tempore ínvalid | les de [`shift`](shift.md) |
| `--first` e `--last` sur li sam index | `--first and --last both name subtitle N, and two references on one subtitle define no transform` |
| index foris li limites | `<path>: subtitle N is past the end: the file holds M subtitles` |
| du subtitules comensante ensemble | `<path>: subtitles N and M start at the same moment, so they define no transform` |
| position ante li origine | `<path>: subtitle N would land before the origin, which no subtitle file can hold` |

**Nequo es scrit sin un del tri optiones de destination**, e lor
errores es comun a su li subcomandes quel scri:
vide [Invocation](invocation.md#li-destination).
