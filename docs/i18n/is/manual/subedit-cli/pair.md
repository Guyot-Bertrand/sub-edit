# `pair`

<!-- machine-translated -->
> Þessi síða er vélþýðing sem enginn lesandi tungumálsins hefur enn farið yfir.

```
subedit-cli pair (--output FILE | --output-dir DIR | --in-place)
                 -t FILE [--align-method position|number]
                 [--dry-run]
                 <main>
```

Skrifar **þýðinguna endurtímasetta á staðsetningar aðalskrárinnar** og segir hvernig línur hennar fundu
skjátexta sinn. Þetta er `Open Translation…` í glugganum — [sama pörunin](../subedit-gui/files.md#að-opna-þýðingu),
sama setningin — og síðan er niðurstaðan skrifuð, sem glugginn gerir með `Save Translation`.

**Þetta er það eina sem pörun gerir sem skrá unnin ein og sér gerir ekki.** Hinar undirskipanirnar
sem taka við `-t` breyta þýðingunni sem þær lesa — [`case`](case.md), [`replace`](replace.md)… —; `pair` breytir
engu í textanum: hún **leggur** þýðinguna á aðalskrána og skrifar það sem lagningin skilaði. Aðalskráin er
lesin til að taka við línunum, **aldrei skrifuð**.

- **Það sem er skrifað er þýðingin**, á **sinni eigin slóð og á sínu eigin sniði**: snið, stafakóðun og
  línulok þýðingarskrárinnar. Skjátextar hennar eru **á staðsetningum aðalskrárinnar**: lína sem
  lenti við hlið merkisins fær aftur mörk skjátextans sem hún þýðir.
- **`-t` er nauðsynlegt.** `pair` án þýðingar hefur ekkert til að leggja: það er notkunarvilla, kóði `1`.
- **Ein aðalskrá**, og ekkert `--recursive`: par er tvær skrár, og tré hefur enga
  leið til að segja hvaða þýðing fer með hvaða aðalskrá. Fyrir runu er ein ræsing á hvert par.
- **`--output` nefnir skrá þýðingarinnar sem er skrifuð, `--output-dir` setur grunnheiti hennar þangað, `--in-place`
  endurskrifar hana**, eins og hjá hverri undirskipun sem tekur við `-t` — sjá [Þýðing](invocation.md#þýðing).
- **`--document` er ekki til**: aðeins eitt skjal er í sigti, þýðingin.

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

## Röksemdir og valkostir

| Valkostur | Nauðsynlegt | Gildi | Sjálfgefið |
| :----- | :----- | :------ | :----- |
| `<main>` | já | **ein** slóð | — |
| `-t`, `--translation-file` | **já** | þýðingarskráin | — |
| `--align-method` | nei | `position` eða `number` — sjá [Þýðing](invocation.md#þýðing) | `position` |
| `--output` / `--output-dir` / `--in-place` | **eitt af þremur**, nema með `--dry-run` | sjá [Ræsing](invocation.md#áfangastaðurinn) | — |
| `--dry-run` | nei | rofi: reiknar og segir frá, skrifar ekkert — sjá [Að sjá áður en skrifað er](invocation.md#að-sjá-áður-en-skrifað-er) | slökkt |

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

Þýðingin var 0,3 s of sein: hver lína fann skjátextann sem miðja hennar skarast við, og
skráin sem er skrifuð ber staðsetningar aðalskrárinnar.

## Hvað samræmingin segir

Setningin er **sú sem glugginn segir** þegar hann opnar þýðingu, hver hluti hennar aðeins þegar hann er ekki
núll — sjá [`inspect -t`](inspect.md#þýðing--t) fyrir töflu yfir hluta hennar. **Hún er skrifuð
frá stigi 1**, en ekki á stigi 2 eins og hjá hinum undirskipununum: hér er samræmingin ekki
smáatriði vinnslunnar, hún er niðurstaðan.

**Hvað samræmingin gerir við skrána sem er skrifuð**:

- **skjátexti sem enginn þýddi er skrifaður sem blokk án texta**, sem les sig án nokkurs fráviks;
- **skjátexti sem fæddist af línu** — línu sem ekkert náði til — er skrifaður með **sínum eigin staðsetningum**, hver sem
  aðferðin er: skráin sem er skrifuð getur því talið fleiri skjátexta en aðalskráin;
- **allt sem skjalið bar sem þýðingu er hreinsað** áður en lagt er, sem á ekki við á skipanalínunni:
  aðalskráin er lesin ein og sér.

**Aðferðirnar tvær** gefa sömu skrána þegar hver lína lendir á sínum skjátexta, en ekki
þegar lína vantar eða þýðingin er færð til — þess vegna er aðferðin val, og þess vegna er
staðsetning sjálfgefin. Átta tilvikin í `src/test/data/paires/` telja þau upp, með skránni sem er væntanleg fyrir
hvora aðferð.

## Úttak

**Staðalúttak** — ekkert: niðurstaðan er skráin sem er skrifuð.

| Stig | Hvað bætist við á villuúttakið |
| :----- | :------------------------------------- |
| 1 | `<translation>: translation: N lines attached; … -> <destination>` — **slóðin er slóð þýðingarinnar**, sem er skráin sem er skrifuð; með `--dry-run`, `(dry run, nothing written)` í stað `-> <destination>` |
| 2 | `<translation>: SubRip, UTF-8, no BOM, LF line endings kept` — snið, stafakóðun, merki og línulok **þýðingarinnar**, sett aftur eins og þau voru |
| 3 | `<translation>: N bytes read, M written`, síðan **sérhvert greiningarboð við lestur** aðalskrárinnar og þýðingarinnar — sjá [Ræsing](invocation.md#að-lesa-greiningarboð) |

### Á JSON-sniði

Með [`--format json`](invocation.md#úttak-sem-skriftur-geta-lesið) kemur **einn hlutur**, þar sem `file` er þýðingin og
`destination` skráin sem er skrifuð. `counts.subtitles` telur **skjátexta skrárinnar sem er skrifuð**, þar með talda þá sem lína
vakti til lífs. Lykillinn `alignment` — `{"file", "method", "attached", "born", "untranslated", "out_of_order"}` —
ber sömu talningar og setningin, eins og lykillinn `translation` hjá [`inspect`](inspect.md#á-json-sniði).

## Skilakóðar

Þeir sömu og hjá tólinu: `0` ef þýðingin er skrifuð — eða reiknuð, með `--dry-run` —, `2` ef ekki tókst að lesa aðra
skrána eða ekki tókst að skrifa áfangastaðinn, `1` við notkunarvillu. **Illa samræmd þýðing er ekki
bilun**: hún er skrifuð, og setningin segir hvað fann ekki stað sinn.

## Villur

| Hvað veldur þeim | Skilaboð |
| :------------------- | :------ |
| ekkert `-t` | `pair needs the translation file to lay over the main one: use -t` |
| `--document` | `The following argument was not expected: --document` — það er ekki til hjá `pair` |
| fleiri en ein aðalskrá | `The following argument was not expected: <path>` |
| þýðingin er aðalskráin | `<translation>: the file is already open as the main document`, kóði `2` |
| önnur skráin vantar, er ólæsileg eða á óþekktu sniði | það sama og alls staðar, með slóð skrárinnar sem er sökudólgurinn |
