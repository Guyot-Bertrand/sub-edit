# Tractar un arbore

<!-- machine-translated -->
> Ca págine es un traduction machinal e it ancor ne ha esset revidet de un lector del lingue.

```
subedit-cli <subcommand> --recursive [options] <directory>...
```

Omni subcomande quel lee un lot de files — `inspect` includet, e ne
[`append`](append.md), [`split-file`](split-file.md) ni [`pair`](pair.md), queles arité es li de un par o
de un film — accepta un **directoria** in loc de un file, con `--recursive` (`-r`).
Ca págine dis quo li promenada prende, in qual órdine, e u it scri.

**Null de to quo seque changea quo es fat a un file**: chascun es leet,
tractat e scrit independentmen, li falliment de un ne halta li altris, e li
quar codes de exeunte es les del [invocation](invocation.md#codes-de-exeunte).
Li promenada produe un liste de chemins, quel li resta del lot consume quam si les
esset scrit a manu.

## Sin `--recursive`, un directoria es refusat

Error de usage, code `1`, null file tractat ni scrit:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?" -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?"
films: is a directory: use --recursive
code=1
```

## Quo li promenada prende

Li promenada es **recursiv, determinist e sin surprisas**:

- **li órdine** — li entratas de chascun directoria es prendet in li órdine de lor
  nómines, **byte por byte** (majusculas ante minusculas, sin regard al
  lingue del machine), e un subdirectoria es promenat in su loc, profundore
  avan omni. Du executiones dona li sam bytes, sur chascun
  machine;
- **ligamentes simbolic ne es sequet** — ni a un file, ni a un
  directoria: un arbore con un ligament a un de su parentes ne haveret
  fine;
- **files e directorias celat es ignorat** — les quel nómine comensa con
  un punctu;
- **solmen li extensiones de formates conosset es conservat**: `.srt`, `.vtt`,
  `.ssa`, `.ass`, `.lrc`, `.sub`, sin regard al casu de littere. **`.txt` es excludet**: it
  nomina du formates, ma super omni chose chascun `LISEZMOI.txt` del munde, e un lot quel
  falliva pro chascun de les esset ínusabil.

**Un file nominat in li linea de comande es nequande filtrat.** Nominar it es
voler it: `subedit-cli shift --by 1 --in-place notes.txt` tracta it, quelcunc que sia
su extension, e solmen se plange si su contenete es in null formate.

**Quo esset lassat a un latere es contat**, e dit al nivelle `-vv`:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli -vv inspect --recursive films 2>&1 | head -1 -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli -vv inspect --recursive films 2>&1 | head -1
films: 1 file left aside, with no extension of a known format
```

Un directoria sin nequo a promenar dis it al nivelle predefinit
(`<directory>: holds no file in a format this tool walks for`) e ne es un error.

## U li promenada scri

**Li arbore es conservat sub `--output-dir`**: li chemin de un file,
**relativ al directoria indicat**, es reproductet sub li directoria de output. Li nómine
del directoria indicat ne es parte de it — to es li semantica de `rsync` con un
barra final, ne li de `cp -r`: on nomina li destination, e adjunte a it li
nivelle desirat (`--output-dir sortie/films`).

<!-- example: mkdir -p films/a/b; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/x.srt; cp films/x.srt films/a/x.srt; cp films/x.srt films/a/b/y.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli shift --by 1 --recursive --output-dir sortie films; find sortie -type f | sort -->
```console
$ mkdir -p films/a/b; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/x.srt; cp films/x.srt films/a/x.srt; cp films/x.srt films/a/b/y.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli shift --by 1 --recursive --output-dir sortie films; find sortie -type f | sort
films/a/b/y.srt: 1 subtitle shifted by 1.000 s -> sortie/a/b/y.srt
films/a/x.srt: 1 subtitle shifted by 1.000 s -> sortie/a/x.srt
films/x.srt: 1 subtitle shifted by 1.000 s -> sortie/x.srt
3 of 3 files shifted
sortie/a/b/y.srt
sortie/a/x.srt
sortie/x.srt
```

Li directoria de output e les del arbore es **creat** si les manca. Con
`--in-place`, chascun file conservat es scrit retro in su loc. Un extension quel
`convert` changea changea anc in li arbore: `films/a/x.srt` convertet
a WebVTT es scrit `sortie/a/x.vtt`.

**Li directoria de output, si it es in li arbore promenat, es excludet del promenada**:
un duesim execution ne relee quo li prim ha scrit.

## Du directorias, e collisiones

Li regul del [destination](invocation.md#li-destination) decide it: du files
queles va fin al sam chemin es refusat **ante que alquo es scrit**, code `1`, con li
du inputs e li destination nominat.

<!-- example: mkdir -p un/a deux/a; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > un/a/x.srt; cp un/a/x.srt deux/a/x.srt; subedit-cli shift --by 1 -r --output-dir sortie un deux; echo "code=$?" -->
```console
$ mkdir -p un/a deux/a; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > un/a/x.srt; cp un/a/x.srt deux/a/x.srt; subedit-cli shift --by 1 -r --output-dir sortie un deux; echo "code=$?"
sortie/a/x.srt: would be written by both un/a/x.srt and deux/a/x.srt
code=1
```

## Quo it costa

Li lot resta sequential: su duration es li summa de les de su files. Li
benchmark `make bench` mesura li promenada e li sam labor in un sol file; li figures
es in li [registre de prestation](../../../../mesures/performances.md).
