# `framerate`

<!-- machine-translated -->
> Ca págine es un traduction machinal e it ancor ne ha esset revidet de un lector del lingue.

> **To ne es [`snap`](snap.md).** Ca comande **muta li scale del
> file**: it es por quande li temporisation es erronic e deriva de plu secundes
> til li fin del film. `snap` remette li marcas de tempore sur li grille
> li plu proxim sin retemporisar quocunc, e es por quande li temporisation es ja
> corect. Ambi have li sam forme, un frequentie de fotogrammas o du, e confunder les causa
> null error.

```
subedit-cli framerate --from <fps> --to <fps>
                      (--output FILE | --output-dir DIR | --in-place)
                      [--dry-run]
                      [--recursive]
                      <file>...
```

Retemporisa un file temporisat por un frequentie de fotogrammas a un altri. It es
li operation quel un file de subtitules besona quande it esset prendet por un copie
quel ne es li del film quel on have — un DVD PAL contra un transferte NTSC, p.ex.

Li formate del file leet es **conservat**: changear li formate es li labor de
[`convert`](convert.md).

<!-- example: subedit-cli framerate --help -->
```console
$ subedit-cli framerate --help
Re-time a file mastered at one frame rate for another
Usage: subedit-cli framerate [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to re-time

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --from TEXT REQUIRED        Frame rate the file is timed at: 25, 23.976
  --to TEXT REQUIRED          Frame rate to time it for: 24, 29.97
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
| `--from` | **yes** | un frequentie de fotogrammas, vide infra | — |
| `--to` | **yes** | un frequentie de fotogrammas, vide infra | — |
| `--output` / `--output-dir` / `--in-place` | **un del tri**, except con `--dry-run` | vide [Invocation](invocation.md#li-destination) | — |
| `--dry-run` | no | un flag: calcula e di, ne scri nequo — vide [Vider ante scrir](invocation.md#vider-ante-scrir) | off |

`--from` es li frequentie de fotogrammas por quel li file **es** temporisat, `--to` ti por quel
it es desirat. Permutar les displacia li file in li
direction contrari — circa sep secundes sur un long film inter 25 e
23,976 — e nequo avisa it ante que on regarda li film.

## Qualmen scrir un frequentie de fotogrammas

In fotogrammas per second, decimales includet, con li punctu quam separator:

```
25      24      23.976      29.97      60
```

**Un frequentie nul o negativ es refusat**: it ne es un frequentie de fotogrammas.

### Li tri frequenties NTSC ne posse esser scrit exactmen

`23.976`, `29.97` e `59.94` es **etiquettes**. Li frequenties de fotogrammas quel
les nomina es `24000/1001`, `30000/1001` e `60000/1001`, nequi de queles have
un scrition decimal finit. Li instrument les recognosce quam tal:

| Quo es scrit | Quo it nomina |
| :------------- | :------------------ |
| `23.976` | `24000/1001` — cinema transferet a video NTSC |
| `24` | `24` — cinema |
| `25` | `25` — video PAL e SECAM |
| `29.97` | `30000/1001` — video NTSC |
| `30`, `50`, `60` | se self |
| `59.94` | `60000/1001` |

Li recognoscentie concerne **li valore e ne li littres**: `23.9760`
nomina li sam frequentie de fotogrammas quam `23.976`.

Omni altri decimal es prendet litteralmen — `23.9` es li standard de nequi,
nequo es a leer in it. To anc fa li tabelle supra
necessi: sin it, `23.976` esset `23976/1000`, e li linea de comande
dissi alquo altri que li fenestre por li sam paroles.

## Quo deveni li positiones

```
t′ = round( t × (from / to) )
```

Un film masterisat a 23,976 e reprodut a 25 fotogrammas per second corre plu rapid,
dunc su subtitules arriva plu tost; in li altri direction, les arriva plu
tard.

**Li factor es un rational exact, e li arrotundation evenit solmen un vez.** Li
via quel sembla equivalent — converter li position in un númere de fotogramma al
rate de input, pois retro in millisecundes al rate de output — ne
es tal: it quantisa chascun position sur li grille de input e erra til
un demí fotogramma, 21 ms mesurat. Vide
[ADR 0013](../../../../adr/0013-mise-a-l-echelle-exacte-des-positions.md).

In practica, de 25 a 23,976, li factor es `1001/960`:

| Position leet | Scrit | Quo dona passar per li fotogrammas |
| -----------: | -----: | -----------------------------------------: |
| `1 010 ms` | `1 053 ms` | `1 043 ms` |
| `3 600 017 ms` | `3 753 768 ms` | `3 753 750 ms` |

<!-- example: printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 25 --to 23.976 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 25 --to 23.976 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle retimed from 25 to 24000/1001 fps -> b.srt
1
00:00:01,053 --> 00:00:02,106
Un.
```

## Output

**Exeunte standard** — nequo: li resultate es li file scrit. Con `--dry-run`, nequo anc: ne hay file, e li linea de narration di it — vide [Vider ante scrir](invocation.md#vider-ante-scrir).

| Nivelle | Quo es adjuntet al exeunte de error |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles retimed from 25 to 24000/1001 fps -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — li formate, li codification, li marca e li fines de linea del file leet, remetit quam ili esset |
| 3 | `<path>: N bytes read, M written`, pois **chascun diagnose de lectura** — vide [Invocation](invocation.md#diagnoses-de-lectura) |

**Li narration nomina li frequentie de fotogrammas exact, ne li etiquette scrit.** Scrir
`23.976` in ti linea reportaret un conversion quel ne evenit; it es
li sol loc u on vide qual del du esset usat. Un frequentie de fotogrammas quel
un decimal scri exactmen es monstrat ibi quam decimal, li altres quam fraction.

### In JSON

Con [`--format json`](invocation.md#un-output-quel-un-script-posse-leer), li objecte de chascun file porta `counts.subtitles` (chascun subtitul del file). Li tabelle complet es ti de [Invocation](invocation.md#quo-chascun-subcomande-pone-in-counts).

## Codes de exeunte

Ti del instrument: `0` si chascun file esset scrit, `2` si null, `3` si
solmen alcun, `1` in cas de error de usage.

Null refusa depende del contenete de un file: du frequenties de fotogrammas definni sempre
un factor. It es li sol subcomande scrient de quel omni
refusas veni ante li prim lectura.

<!-- example: printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 0 --to 25 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 0 --to 25 --output b.srt a.srt; echo "code=$?"
"0" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed
code=1
```

## Errores

| Quo it causa | Missage |
| :------------------ | :------ |
| frequentie de fotogrammas nul o negativ | `"…" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed` |
| comma decimal | `"…" is not a frame rate: use a decimal point and not a comma, in every language` |
| textu quel ne es un númere | `"…" is not a frame rate: expected frames per second, like 25 or 23.976` |
| frequentie de fotogrammas desproportionat | `"…" is not a frame rate: no video runs at that many frames per second` |

**Nequo es scrit sin un del tri optiones de destination**, e lor
errores es comun al subcomandes quel scri:
vide [Invocation](invocation.md#li-destination).
