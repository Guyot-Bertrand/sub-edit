# `case`

<!-- machine-translated -->
> Þessi síða er vélþýðing sem enginn lesandi tungumálsins hefur enn farið yfir.

```
subedit-cli case --to title|sentence|upper|lower [--range N-M|N-]
                 (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 [--document main|translation] [-t FILE] [--align-method position|number]
                 <file>...
```
Setur textana í **hástafi í hverju orði, hástaf í upphafi setningar, hástafi eða lágstafi**, án þess að snerta
merki. Þetta er `Verkfæri ▸ Stafstaða` í [glugganum](../subedit-gui/operations.md), með reglum þess.

| `--to` | Áhrif | `bonjour marie` verður |
| :----- | :---- | :---------------------- |
| `title` | hvert orð fær hástaf, afgangurinn lækkar | `Bonjour Marie` |
| `sentence` | fyrsti stafur textans, og aðeins hann; afgangurinn lækkar | `Bonjour marie` |
| `upper` | allt í hástöfum | `BONJOUR MARIE` |
| `lower` | allt í lágstöfum | `bonjour marie` |

- **Það sem kemur á undan fyrsta staf eða fyrsta tölustaf er ekki snert**: þankastrik,
  upphafsgæsalappir eða svigi haldast (`- bonjour` gefur `- Bonjour`).
- **Merki eru ekki texti**: þau skipta ekki um stafstöðu — `<I>` helst `<I>` — og
  skipta ekki um stað: merki sem sker orð sker það áfram á eftir.
- **ICU ákveður hvar orð byrjar**: `l'été sera chaud` gefur `L'été Sera Chaud`, og broddarnir
  haldast (`éléonore` gefur `Éléonore`).

<!-- example: subedit-cli case --help -->
```console
$ subedit-cli case --help
Put the texts in title, sentence, upper or lower case, tags intact
Usage: subedit-cli case [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --to title|sentence|upper|lower
                              The case to put the texts in
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
| `--to` | **já** | `title`, `sentence`, `upper`, `lower` | — |
| `--range` | nei | `N-M`, eða `N-` til loka — sjá [`adjust`](adjust.md#--range) | öll skráin |
| `--document` | nei | `main` eða `translation` — sjá [Þýðing](invocation.md#þýðing) | `main` |
| `-t`, `--translation-file` | með `--document translation` | þýðingarskrá, fyrir **eitt** inntak | — |
| `--align-method` | nei, og aðeins með `-t` | `position` eða `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **eitt af þremur**, nema með `--dry-run` | sjá [Ræsing](invocation.md#áfangastaðurinn) | — |
| `--dry-run` | nei | fáni: reiknar og segir frá, skrifar ekkert — sjá [Að sjá áður en skrifað er](invocation.md#að-sjá-áður-en-skrifað-er) | slökkt |

**Aðaltextinn sjálfgefið; þýðingin með `--document translation -t FILE`.** Þá er það
þýðingin sem er breytt og skrifuð, og aðeins hún — sjá [Þýðing](invocation.md#þýðing).

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\n- BONJOUR MARIE\n\n' > a.srt; subedit-cli case --to title --output titre.srt a.srt; cat titre.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\n- BONJOUR MARIE\n\n' > a.srt; subedit-cli case --to title --output titre.srt a.srt; cat titre.srt
a.srt: 2 subtitles recased -> titre.srt
1
00:00:01,000 --> 00:00:02,000
<i>Bon</i>jour Marie

2
00:00:03,000 --> 00:00:04,000
- Bonjour Marie
```

Skrá þar sem **enginn texti breytist** — allt er þegar í umbeðinni stafstöðu — er skrifuð eins og hún er og
segir frá því:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBONJOUR\n\n' > a.srt; subedit-cli case --to upper --output b.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBONJOUR\n\n' > a.srt; subedit-cli case --to upper --output b.srt a.srt
a.srt: nothing to change -> b.srt
```

## Úttak

**Staðalúttak** — ekkert: niðurstaðan er skráin sem er skrifuð. Með `--dry-run` kemur listi yfir
breytingar, lýst í [Að sjá áður en skrifað er](invocation.md#að-sjá-áður-en-skrifað-er).

| Stig | Hverju er bætt við villuúttakið |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles recased -> <destination>`, eða `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — snið, stafakóðun, merki og línulok skrárinnar sem var lesin, sett aftur eins og þau voru |
| 3 | `<path>: N bytes read, M written`, síðan **hvert lesgreiningarboð** — sjá [Ræsing](invocation.md#að-lesa-greiningarboð) |

`N` er fjöldi skjátexta **sem texti breyttist í**, lesinn úr aðgerðinni sjálfri.

### Á JSON-sniði

Með [`--format json`](invocation.md#úttak-sem-skriftur-geta-lesið) ber hlutur hverrar skrár
`counts.changed` — `N` setningarinnar — og `changes`, listann yfir breytta skjátexta
(`{"subtitle", "document", "before", "after"}`, **tómur** þegar ekkert breyttist).

## Skilakóðar

Þeir sömu og í verkfærinu: `0` ef hver skrá var unnin — skrá þar sem ekkert breytist er það —,
`2` ef engin, `3` ef aðeins sumar, `1` við notkunarvillu.

## Villur

| Hvað veldur henni | Skilaboð |
| :------------------ | :------ |
| `--to` vantar | `--to is required` |
| gildi sem er ekkert af þessum fjórum | `--to: … not in {title,sentence,upper,lower}` |
| tímabil illa skrifað eða utan skrárinnar | sjá [`adjust`](adjust.md#--range) |

Villur áfangastaðarins eru sameiginlegar undirskipununum sem skrifa:
sjá [Ræsing](invocation.md#áfangastaðurinn).
