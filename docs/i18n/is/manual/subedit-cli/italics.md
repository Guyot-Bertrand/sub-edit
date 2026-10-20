# `italics`

<!-- machine-translated -->
> Þessi síða er vélþýðing sem enginn lesandi tungumálsins hefur enn farið yfir.

```
subedit-cli italics (--on | --off) [--range N-M|N-]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    <file>...
```

Setur textana í **skáletur** eða **tekur skáletrið úr þeim**, með merkjum sniðs skrárinnar.
Þetta er `Italic` í [glugganum](../subedit-gui/operations.md).

**Tvö orð en ekki rofi**: hnappur gluggans spyr kjarnann í hvora áttina
skuli fara; hér segir maður það. Valkostirnir `--on` og `--off` útiloka hvor annan og annan þarf — ef hvorugur er gefinn eða báðir
er það notkunarvilla (kóði `1`).

- **`--on`** vefur hverjum texta í eitt merkjapar, **eftir að þau sem
  voru í honum hafa verið fjarlægð**: texti sem þegar er í skáletri er ekki vafinn tvisvar og skáletur sem er rofið í
  tvennt er endurgert í einu lagi. Yfir nokkrar línur nær eitt par yfir allt.
- **`--off`** fjarlægir merki skáletursins; **afgangurinn af merki er varðveittur** (`{\b1\i1}` verður
  `{\b1}`).
- Textinn er ritaður **eins og snið skrárinnar ritar skáletur**: `<i>…</i>` fyrir SubRip, WebVTT og
  SubViewer 2, `{\i1}…{\i0}` fyrir Sub Station Alpha sniðin tvö, `{Y:i}` fyrir MicroDVD.

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

## Röksemdir og valkostir

| Valkostur | Nauðsynlegt | Gildi | Sjálfgefið |
| :----- | :----- | :------ | :----- |
| `<file>...` | já | ein eða fleiri slóðir | — |
| `--recursive`, `-r` | nei | fáni | slökkt — sjá [Vinnsla á möpputré](lots.md) |
| `--on` / `--off` | **annar af tveimur** | fánar | — |
| `--range` | nei | `N-M`, eða `N-` til loka — sjá [`adjust`](adjust.md#--range) | öll skráin |
| `--document` | nei | `main` eða `translation` — sjá [Þýðing](invocation.md#þýðing) | `main` |
| `-t`, `--translation-file` | með `--document translation` | þýðingarskrá, fyrir **eina** inntaksskrá | — |
| `--align-method` | nei, og aðeins með `-t` | `position` eða `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **eitt af þremur**, nema með `--dry-run` | sjá [Ræsing](invocation.md#áfangastaðurinn) | — |
| `--dry-run` | nei | fáni: reiknar og tilkynnir, skrifar ekkert — sjá [Að sjá áður en skrifað er](invocation.md#að-sjá-áður-en-skrifað-er) | slökkt |

**Aðaltextinn sjálfgefið; þýðingin með `--document translation -t FILE`.** Þá er það
þýðingin sem er breytt og skrifuð, og hún ein — sjá [Þýðing](invocation.md#þýðing).

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

## Snið án stíls hafnar

**TMPlayer og LRC rita engan stíl**: það er ekkert skáletur til að setja á og ekkert til að taka af. Glugginn
grámerkir færsluna í stað þess að fela hana; skipanalínan **hafnar skrá fyrir
skrá, með ástæðunni**, og hinar skrárnar í runnunni halda áfram. Það sem skiptir máli er það sem
sniðið getur borið, ekki hvernig það ritar það: SubRip og WebVTT rita skáletur á sama
hátt og segja ekki það sama um lit.

<!-- example: printf '00:00:01:Hello there\n00:00:03:Bye\n' > a.txt; subedit-cli italics --on --output-dir sortie a.txt; echo "code=$?" -->
```console
$ printf '00:00:01:Hello there\n00:00:03:Bye\n' > a.txt; subedit-cli italics --on --output-dir sortie a.txt; echo "code=$?"
a.txt: TMPlayer writes no style: there are no italics to put on
code=2
```

Á JSON-sniði ber bilunin auðkennið **`no-style`**.

## Úttak

**Staðalúttak** — ekkert: niðurstaðan er skráin sem var skrifuð. Með `--dry-run` er það listi yfir
breytingar, lýstur í [Að sjá áður en skrifað er](invocation.md#að-sjá-áður-en-skrifað-er).

| Stig | Það sem bætist við á villuúttakinu |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles put in italics -> <destination>`, eða `N subtitles taken out of italics`, eða `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — snið, stafakóðun, merki og línulok skrárinnar sem var lesin, sett aftur eins og þau voru |
| 3 | `<path>: N bytes read, M written`, síðan **öll greiningarboð lesturs** — sjá [Ræsing](invocation.md#að-lesa-greiningarboð) |

Talan `N` er fjöldi skjátexta **þar sem textinn breyttist**, lesinn úr aðgerðinni sjálfri.

### Á JSON-sniði

Með [`--format json`](invocation.md#úttak-sem-skriftur-geta-lesið) ber hlutur hverrar skrár
`counts.changed` — töluna `N` úr setningunni — og `changes`, lista yfir skjátextana sem breyttust
(`{"subtitle", "document", "before", "after"}`, **tómur** þegar ekkert breyttist).

## Skilakóðar

Þeir sömu og hjá tólinu: `0` ef hver skrá var unnin — skrá þar sem ekkert breytist telst unnin —,
`2` ef engin, `3` ef aðeins sumar, `1` við notkunarvillu.

## Villur

| Hvað kallar þær fram | Skilaboð |
| :------------------ | :------ |
| hvorki `--on` né `--off` | `italics needs --on to put the texts in italics, or --off` |
| báðir | `--on and --off say opposite things; give one of them` |
| snið án stíls | `<path>: TMPlayer writes no style: there are no italics to put on` (eða `take out`) |
| bil rangt ritað eða utan skrárinnar | sjá [`adjust`](adjust.md#--range) |

Villur áfangastaðarins eru sameiginlegar undirskipunum sem skrifa:
sjá [Ræsing](invocation.md#áfangastaðurinn).
