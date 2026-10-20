# `shift`

<!-- machine-translated -->
> Ca págine es un traduction machinal e it ancor ne ha esset revidet de un lector del lingue.

```
subedit-cli shift (--by <time> | --to-grid)
                  (--output FILE | --output-dir DIR | --in-place)
                  [--dry-run]
                  [--recursive]
                  <file>...
```

Move **chascun** position del file del sam quantité, in un direction o in
li altri. Null subtitul resta sur ecran plu long o plu curt:
to es quo fa un displaciament un displaciament.

<!-- example: subedit-cli shift --help -->
```console
$ subedit-cli shift --help
Move every position of a file by a fixed amount
Usage: subedit-cli shift [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to shift

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --by TEXT                   Amount to move by: 2.999, -7.001, or 00:00:07.001
  --to-grid                   Move by the amount that puts the positions back on their frame grid
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
| `--by` | **un del du** | un duration con signe, vide infra | — |
| `--to-grid` | **un del du** | flag: li quantité es mesurat, vide infra | — |
| `--output` / `--output-dir` / `--in-place` | **un del tri**, except con `--dry-run` | vide [Invocation](invocation.md#li-destination) | — |
| `--dry-run` | no | un flag: calcula e dis, scri null — vide [Vider ante scrir](invocation.md#vider-ante-scrir) | off |

Li formate del file leet es **conservat**: changear li formate es li labor de
[`convert`](convert.md).

## `--to-grid`: li quantité mesurat in loc de donat

Un file queles positiones es sur un grille de fotogrammas **a un
constante apu** esset displaciat, e ti constante posse esser mesurat sur li positiones self.
`--to-grid` mesura it e aplica it, sin que alqui deve saver it.

Li quantité es **specific a chascun file**: du files displaciat foris li sam
grille per du quantités diferent revene chascun per su propri. It es
scrit in li narration, con li frequentie de fotogrammas trovat.

**Li via es li plu curt.** Ultra un demi fotogramma, li plu proxim linea del grille
es li sequent: li correction ea avan in loc de recuar de
quasi un tot fotogramma.

<!-- example: k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40 + 7)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > decale.srt; subedit-cli shift --to-grid --output recale.srt decale.srt -->
```console
$ k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40 + 7)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > decale.srt; subedit-cli shift --to-grid --output recale.srt decale.srt
decale.srt: 12 subtitles shifted by -0.007 s onto their 25 fps grid -> recale.srt
```

**Un file sur null grille conosset es refusat, con li motive.** Ne es
nequo a rejunter, e un fase mesurat sur bruita move li file
a hasarde. Vide [`inspect`](inspect.md#quo-frame-rate-grid-reporta) por
saver quo un file dona ante que provar li operation.

`--by` e `--to-grid` ambes dis de quant displaciar: donar les ensemble
es un error de usage, e anc donar null de les.

**`--to-grid` ne es [`snap`](snap.md).** It displacia li tot file
per li sam quantité e conserva li temporisation relativ **exactmen**; `snap`
move chascun position independentmen e absorbe in passante les queles esset
corecet a manu. Sur un file nett, li du dona li sam
resultate; sur un file partial, ne.

## Qualmen scrir un duration

Du formes, e solmen du:

| Forme | Exemples |
| :---- | :------- |
| secundas | `30`, `2.999`, `-7.001`, `+1.5` |
| marca de tempore | `00:00:07.001`, `01:39:37.040`, `-00:00:07.001` |

Ambes dis li sam chose: `--by 7.001` e `--by 00:00:07.001` produe
li sam file.

**Li separator decimal es li punctu**, li linea de comande parlante anglesi.
Un comma es refusat in loc de interpretat: li mitá del munde scri `7,001`
por sett mil un, e leer it quam sett secundas esset un conjectura —
un fals por ti mitá.

**Li resolution es li millisecunda.** Plu fin es refusat in loc de arondat
tacitmen: un arondation tacit dona un resultate quel corresponde a nequo de quo
esset demandat.

<!-- example: printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle shifted by -7.001 s -> b.srt
1
00:00:02,999 --> 00:00:04,999
Bonjour.
```

## Un displaciament ante li origine es refusat

Retroear ultra zero dona code `2` e **scri nequo**. Li missage nomina
li subtitul quel ne posset accepter li displaciament, quo dis simultanmen
til u on posse ear.

Li ration ne es un principie: un position ante li origine es scrit
`-00:00:01,000`, un forme quel null del nov formates define e quel null lectores
accepta. Li nucleo permisse talmen positiones — refusar les fat de un
operation de redaction un casu special — ma un file es altri chose que un
modelle in memorie.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; echo "code=$?"
a.srt: subtitle 1 would start before the origin, which no subtitle file can hold
code=2
```

## Output

**Exeunte standard** — null: li resultate es li file scrit. Con `--dry-run`, anc null: ne es file, e li linea de narration dis it — vide [Vider ante scrir](invocation.md#vider-ante-scrir).

| Nivelle | Quo es adjuntet al exeunte de error |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles shifted by 2.999 s -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — li formate, li codification, li marca e li fines de linea del file leet, remetet quam les esset |
| 3 | `<path>: N bytes read, M written`, poy **omni diagnose de lectura** — vide [Invocation](invocation.md#diagnoses-de-lectura) |

### In JSON

Con [`--format json`](invocation.md#un-output-quel-un-script-posse-leer), li objecte de chascun file porta `counts.subtitles` (omni subtitul del file) e `counts.shifted_by_ms` (li displaciament, in millisecundas integri, **con signe**). Li tabelle complet es li de [Invocation](invocation.md#quo-chascun-subcomande-pone-in-counts).

## Codes de exeunte

Les del instrument: `0` si omni file esset scrit, `2` si null, `3` si
solmen alcunes, `1` in cas de un error de usage.

## Errores

| Quo it causa | Missage |
| :------------------ | :------ |
| comma decimal | `"…" is not a time: use a decimal point and not a comma, in every language` |
| plu fin que li millisecunda | `"…" is not a time: positions are held to the millisecond, no finer` |
| textu quel ne es un duration | `"…" is not a time: expected seconds like 2.999, or a timestamp like 00:00:07.001` |
| displaciament ante li origine | `<path>: subtitle N would start before the origin, which no subtitle file can hold` |

**Nequo es scrit sin un del tri optiones de destination**, e lor
errores es comun a su li subcomandes quel scri:
vide [Invocation](invocation.md#li-destination).
