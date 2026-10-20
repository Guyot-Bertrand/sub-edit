# `replace`

<!-- machine-translated -->
> Þessi síða er vélþýðing sem enginn lesandi tungumálsins hefur enn farið yfir.

```
subedit-cli replace [--regex] [--case-sensitive] [--range N-M|N-]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    <pattern> <replacement> <file>...
```

Skiptir út texta í skjátextunum **án þess að brjóta merki**. Þetta er
aðgerðin `Replace All` í [leit](../subedit-gui/find-and-replace.md) gluggans, með reglum hennar:

- **leitað er í sýnilega textanum** — því sem áhorfandinn les. Merki eru ekki hluti
  af honum: leit að `<i>` finnur ekkert, og leit að `Bonjour` finnur „Bonjour“ þar sem helmingurinn
  er skáletraður;
- **útskiptingin fer fram í frumtextanum**, textanum í skránni, og merkin haldast þar sem þau
  voru. Merki sem sker orðið sem leitað er að umlykur alla útskiptinguna; stíll sem
  snertir samsvörunina nær yfir alla útskiptinguna; það sem samsvörunin snertir ekki
  hreyfist ekki. Reglurnar, tilvik fyrir tilvik, eru þær sömu og í leit gluggans.

**Engin undirskipun `find` er til**: til að sjá hvar mynstur er án þess að breyta nokkru
er það [`--dry-run`](invocation.md#að-sjá-áður-en-skrifað-er) sem sýnir lista yfir hvern skjátexta sem um ræðir.

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

## Röksemdir og valkostir

| Valkostur | Nauðsynlegt | Gildi | Sjálfgefið |
| :----- | :----- | :------ | :----- |
| `<pattern>` | já | textinn sem leitað er að í því sem er sýnt; **ekki tómur** | — |
| `<replacement>` | já | það sem kemur í staðinn, merki leyfð; má vera tómt | — |
| `<file>...` | já | ein eða fleiri slóðir | — |
| `--recursive`, `-r` | nei | rofi | slökkt — sjá [Vinnsla á möpputré](lots.md) |
| `--regex` | nei | rofi: mynstrið er ICU-reglulegt segðamynstur | venjulegur texti |
| `--case-sensitive` | nei | rofi: gerir greinarmun á hástöfum og lágstöfum | honum er sleppt |
| `--range` | nei | `N-M`, eða `N-` til loka — sjá [`adjust`](adjust.md#--range) | öll skráin |
| `--document` | nei | `main` eða `translation` — sjá [Þýðing](invocation.md#þýðing) | `main` |
| `-t`, `--translation-file` | með `--document translation` | þýðingarskrá, fyrir **eina** inntaksskrá | — |
| `--align-method` | nei, og aðeins með `-t` | `position` eða `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **eitt af þremur**, nema með `--dry-run` | sjá [Ræsing](invocation.md#áfangastaðurinn) | — |
| `--dry-run` | nei | rofi: reiknar og segir frá, skrifar ekkert — sjá [Að sjá áður en skrifað er](invocation.md#að-sjá-áður-en-skrifað-er) | slökkt |

**Mynstur sem byrjar á bandstriki** er skrifað á eftir `--`: `subedit-cli replace -- -- "—" film.srt`.

**Aðaltextinn sjálfgefið; þýðingin með `--document translation -t FILE`.** Þá er það
þýðingin sem er breytt og skrifuð, og hún ein — sjá [Þýðing](invocation.md#þýðing).

### Venjulegur texti eða segð

Sjálfgefið er mynstrið **lesið eins og það er**: `a.b` leitar að `a`, punkti og `b`. Stafstöðu
er sleppt af sömu nákvæmni og í glugganum — `É` og `é` eru einn og sami stafurinn.

Með `--regex` er mynstrið **ICU**-reglulegt segðamynstur, lesið eins og Gaupol les það: `.` fer yfir
línuskil, `^` og `$` gilda við hverja línu skjátexta. Í útskiptingunni, og
aðeins þar:

| Skrifað | Til að segja |
| :----- | :-------- |
| `$1` til `$9` | hópar samsvörunarinnar |
| `$0` | öll samsvörunin |
| `\n` | línuskil |
| `\$`, `\\` | `$`, bakstrik |

Gættu að gæsalöppum skeljarinnar: `'$1 EUR'`, en ekki `"$1 EUR"`, sem léti skelina lesa `$1`.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nPrix: 12 euros, et 7 euros.\n\n' > prix.srt; subedit-cli replace --regex '([0-9]+) euros' '$1 EUR' --output euros.srt prix.srt; cat euros.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nPrix: 12 euros, et 7 euros.\n\n' > prix.srt; subedit-cli replace --regex '([0-9]+) euros' '$1 EUR' --output euros.srt prix.srt; cat euros.srt
prix.srt: replaced 2 matches -> euros.srt
1
00:00:01,000 --> 00:00:02,000
Prix: 12 EUR, et 7 EUR.
```

## Merki

Texti þar sem merki sker orðið sem leitað er að:

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

Í fyrsta skjátextanum skar merkið „Bonjour“: nú umlykur það „Salut“.
Í þeim seinni er það ekki snert og helst þar sem það er. **Merki sem er skilið eftir opið
helst opið**: að lagfæra hrjáða markup væri önnur aðgerð.

Útskiptingin er lesin í **merkjum sniðs skrárinnar**: `<i>…</i>` fyrir SubRip,
`{\i1}…` fyrir Advanced SSA. Það er eina leiðin til að forsníða það sem maður skrifar.

## Ekkert fundið, ekkert að breyta

Tvö svör sem eru ekki bilun — skilakóðinn er `0` og **skráin er skrifuð
engu að síður**, því tilgreindur áfangastaður er skrifaður áfangastaður:

| Staða | Setning |
| :-------- | :----- |
| mynstrið er hvergi | `"zzz" not found` |
| það er til staðar, en hver samsvörun er leyst út með sjálfri sér | `nothing to change` |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli replace zzz x --output b.srt a.srt; subedit-cli replace Bonjour Bonjour --output c.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli replace zzz x --output b.srt a.srt; subedit-cli replace Bonjour Bonjour --output c.srt a.srt
a.srt: "zzz" not found -> b.srt
a.srt: nothing to change -> c.srt
```

## `--dry-run`

Staðalúttakið telur upp **hvern skjátexta sem texti myndi breytast í**, fyrir og eftir, eins og
hann er í skránni — merki meðtalin:

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

Form listans er lýst í [Að sjá áður en skrifað er](invocation.md#að-sjá-áður-en-skrifað-er);
á JSON-sniði er það `changes`.

## Úttak

**Staðalúttak** — ekkert: niðurstaðan er skráin sem er skrifuð. Með `--dry-run` er það listinn yfir
breytingar hér að ofan.

| Stig | Hvað bætist við á villuúttakið |
| :----- | :------------------------------------- |
| 1 | `<path>: replaced N matches -> <destination>`, eða `"<pattern>" not found`, eða `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — snið, stafakóðun, merki og línulok skrárinnar sem var lesin, sett aftur eins og þau voru |
| 3 | `<path>: N bytes read, M written`, síðan **sérhvert greiningarboð við lestur** — sjá [Ræsing](invocation.md#að-lesa-greiningarboð) |

`N` telur **samsvaranirnar sem var skipt út í skjátextunum sem texti breyttist í** — það er
sama talan og glugginn segir.

### Á JSON-sniði

Með [`--format json`](invocation.md#úttak-sem-skriftur-geta-lesið) ber hlutur hverrar skrár:

| Lykill | Innihald |
| :-- | :------ |
| `counts.replaced` | samsvaranirnar sem var skipt út í skjátextunum sem texti breyttist í: `N` í setningunni |
| `counts.matched` | samsvaranirnar sem fundust, **hvort sem útskipting þeirra breytti textanum eða ekki** — það er það sem greinir „ekkert fundið“ (`0`) frá „ekkert að breyta“ (`replaced` er `0`, `matched` jákvætt) |
| `changes` | skjátextarnir sem breyttust: `{"subtitle", "document", "before", "after"}`, **tómt** þegar ekkert breyttist |

## Skilakóðar

Þeir sömu og hjá tólinu: `0` ef sérhver skrá var unnin — skrá þar sem ekkert fannst er það
—, `2` ef engin, `3` ef aðeins sumar, `1` við notkunarvillu.

## Villur

| Hvað veldur henni | Skilaboð |
| :------------------ | :------ |
| tómt mynstur | `pattern: nothing to look for` |
| ólæsileg segð | `pattern: not a regular expression (<ICU's reason>)` |
| bil illa skrifað eða utan skrárinnar | sjá [`adjust`](adjust.md#--range) |

**Mynstrið er lesið áður en fyrsta skráin er lesin**: ólæsileg segð er notkunarvilla
(kóði `1`), ekkert er lesið né skrifað.

Villur áfangastaðarins eru sameiginlegar undirskipununum sem skrifa:
sjá [Ræsing](invocation.md#áfangastaðurinn).
