# `pair`

<!-- machine-translated -->
> Ca págine es un traduction machinal e it ancor ne ha esset revidet de un lector del lingue.

```
subedit-cli pair (--output FILE | --output-dir DIR | --in-place)
                 -t FILE [--align-method position|number]
                 [--dry-run]
                 <main>
```

Scri **li traduction retemporisat sur li positiones del file principal**, e dis qualmen su lineas ha trovat lor
subtitul. It es `Open Translation…` del fenestre — [li sam accoplament](../subedit-gui/files.md#aperter-un-traduction),
li sam frase — sequet per li scrition del resultate, quel li fenestre fa per `Save Translation`.

**It es li sol chose quel li accoplament fa e quel un file tractat solmen ne fa.** Li altri subcomandes
queles prende `-t` changea li traduction quel les lee — [`case`](case.md), [`replace`](replace.md)… —; `pair` changea
null in li textu: it **pone** li traduction sur li file principal e scri quo li position ha produet. Li file principal es
leet por recive li lineas, **nequande scrit**.

- **Quo es scrit es li traduction**, a **su propri chemin e in su propri formate**: li formate, li codification e
  li fines de linea del file de traduction. Su subtitules es **al positiones del file principal**: un linea quel
  ha cadet apu li marca recive li limites del subtitul quel it tradue.
- **`-t` es obligatori.** `pair` sin traduction ha null a pone: it es un error de usage, code `1`.
- **Un sol file principal**, e null `--recursive`: un par es du files, e un arbore ne have
  manere de dir qual traduction va con qual file principal. Por un lot, un invocation per par.
- **`--output` nomina li file del traduction scrit, `--output-dir` pone ibi su nómine de base, `--in-place`
  scri it denov**, quam por omni subcomande quel prende `-t` — vide [Un traduction](invocation.md#un-traduction).
- **`--document` ne existe**: ne es que un sol document cielat, li traduction.

<!-- example: subedit-cli pair --help -->
```console
$ subedit-cli pair --help
Write a translation at the positions of its main file, and say how it lined up
Usage: subedit-cli pair [OPTIONS] main

Positionals:
  main TEXT REQUIRED          The main subtitle file

Options:
  -h,--help                   Print this help message and exit
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
| `<main>` | yes | **un sol** chemin | — |
| `-t`, `--translation-file` | **yes** | li file de traduction | — |
| `--align-method` | no | `position` o `number` — vide [Un traduction](invocation.md#un-traduction) | `position` |
| `--output` / `--output-dir` / `--in-place` | **un del tri**, except con `--dry-run` | vide [Invocation](invocation.md#li-destination) | — |
| `--dry-run` | no | un flag: calcula e dis, scri null — vide [Vider ante scrir](invocation.md#vider-ante-scrir) | off |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,300 --> 00:00:02,300\nBonjour.\n\n2\n00:00:03,300 --> 00:00:04,300\nAu revoir.\n\n' > film.fr.srt; subedit-cli pair film.srt -t film.fr.srt --output recale.fr.srt; cat recale.fr.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,300 --> 00:00:02,300\nBonjour.\n\n2\n00:00:03,300 --> 00:00:04,300\nAu revoir.\n\n' > film.fr.srt; subedit-cli pair film.srt -t film.fr.srt --output recale.fr.srt; cat recale.fr.srt
film.fr.srt: translation: 2 lines attached -> recale.fr.srt
1
00:00:01,000 --> 00:00:02,000
Bonjour.

2
00:00:03,000 --> 00:00:04,000
Au revoir.
```

Li traduction esset 0,3 s in retard: chascun linea ha trovat li subtitul quel su medie superpone, e li
file scrit porta li positiones del file principal.

## Quo li alliniament dis

Li frase es **li quel li fenestre dis** quande it aperte un traduction, chascun parte solmen si it ne es
zero — vide [`inspect -t`](inspect.md#un-traduction--t) por li tabelle de su partes. **It es scrit
ja del nivelle 1**, e ne al nivelle 2 quam por li altri subcomandes: ci li alliniament ne es un
detail del tractation, it es li resultate.

**Quo li alliniament fa al file scrit**:

- **un subtitul quel null person ha traductet es scrit quam un bloc sin textu**, quel es leet sin anomalie;
- **un subtitul nascet de un linea** — un linea quel nequo ha atinget — es scrit con **su propri positiones**, quelcunc
  li metode: li file scrit posse dunc contener plu subtitules que li file principal;
- **quo li document portava quam traduction es vacuat** ante li pone, quo ne es applicabil in li linea de comande:
  li file principal es leet sol.

**Li du metodes** dona li sam file quande chascun linea cade sur su subtitul, e ne
quande un linea manca o li traduction es displaciat — pro to li metode es un electe, e pro to
position es li predefinit. Li ot casus de `src/test/data/paires/` les lista, con li file expectat por
chascun metode.

## Output

**Exeunte standard** — null: li resultate es li file scrit.

| Nivelle | Quo es adjuntet al exeunte de error |
| :----- | :------------------------------------- |
| 1 | `<translation>: translation: N lines attached; … -> <destination>` — **li chemin es li del traduction**, quel es li file scrit; con `--dry-run`, `(dry run, nothing written)` in loc de `-> <destination>` |
| 2 | `<translation>: SubRip, UTF-8, no BOM, LF line endings kept` — li formate, li codification, li marca e li fines de linea **del traduction**, remetet quam les esset |
| 3 | `<translation>: N bytes read, M written`, poy **omni diagnose de lectura** del file principal e del traduction — vide [Invocation](invocation.md#diagnoses-de-lectura) |

### In JSON

Con [`--format json`](invocation.md#un-output-quel-un-script-posse-leer), **un sol objecte**, quel `file` es li traduction e
`destination` li file scrit. `counts.subtitles` cunta **li subtitules del file scrit**, anc les queles un linea
ha vivificat. Li clave `alignment` — `{"file", "method", "attached", "born", "untranslated", "out_of_order"}` —
porta li sam cuntages quam li frase, quam li clave `translation` de [`inspect`](inspect.md#in-json).

## Codes de exeunte

Les del instrument: `0` si li traduction es scrit — o calculat, con `--dry-run` —, `2` si un del du files ne posset
esser leet o si li destination ne posset esser scrit, `1` in cas de un error de usage. **Un traduction mal alliniat ne es
un falliment**: it es scrit, e li frase dis quo ne ha trovat su loc.

## Errores

| Quo les causa | Missage |
| :------------------- | :------ |
| null `-t` | `pair needs the translation file to lay over the main one: use -t` |
| `--document` | `The following argument was not expected: --document` — it ne existe por `pair` |
| plu que un file principal | `The following argument was not expected: <path>` |
| li traduction es li file principal | `<translation>: the file is already open as the main document`, code `2` |
| un del files manca, es illegibil o de un formate ne conosset | li sam quam omniloc, con li chemin del file in fallida |
