# `dialogue-dashes`

<!-- machine-translated -->
> Þessi síða er vélþýðing sem enginn lesandi tungumálsins hefur enn farið yfir.

```
subedit-cli dialogue-dashes (--add | --remove) [--range N-M|N-]
                            (--output FILE | --output-dir DIR | --in-place)
                            [--dry-run]
                            [--recursive]
                            [--document main|translation] [-t FILE] [--align-method position|number]
                            <file>...
```
Setur **þankastrik** fremst í línurnar, eða tekur þau af. Þetta er `Dialogue Dashes` í
[glugganum](../subedit-gui/operations.md).

**Tvö orð, eins og í [`italics`](italics.md)**: `--add` og `--remove` útiloka hvort annað og annað þeirra er nauðsynlegt.

- **`--add`** setur strik fyrir framan hverja línu sem hefur ekkert: `Bonjour` gefur `- Bonjour`, og
  `Bonjour\nMarie` gefur `- Bonjour\n- Marie`. **Strik sem er þegar til staðar er ekki tvöfaldað.**
- **Þrjú strik eru þekkt** — bandstrik lyklaborðsins og tvö löng strik leturgerðarinnar —
  **og það er bandstrikið sem er skrifað**: `— Bonjour` verður `- Bonjour`. Aukabil
  á eftir strikinu hverfa.
- Strikið fer **fyrir framan textann, ekki fyrir framan upphafsmerki**: `<i>Bonjour</i>` gefur
  `<i>- Bonjour</i>`. Ekkert merki færist.
- **`--remove`** tekur upphafsstrikin af, í öllum þremur myndunum.

<!-- example: subedit-cli dialogue-dashes --help -->
```console
$ subedit-cli dialogue-dashes --help
Put dialogue dashes on the lines, or take them off
Usage: subedit-cli dialogue-dashes [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --add                       Put a dialogue dash at the head of the lines
  --remove                    Take the dialogue dashes off
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
| `--add` / `--remove` | **annað af tveimur** | fánar | — |
| `--range` | nei | `N-M`, eða `N-` til loka — sjá [`adjust`](adjust.md#--range) | öll skráin |
| `--document` | nei | `main` eða `translation` — sjá [Þýðing](invocation.md#þýðing) | `main` |
| `-t`, `--translation-file` | með `--document translation` | þýðingarskrá, fyrir **eitt** inntak | — |
| `--align-method` | nei, og aðeins með `-t` | `position` eða `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **eitt af þremur**, nema með `--dry-run` | sjá [Ræsing](invocation.md#áfangastaðurinn) | — |
| `--dry-run` | nei | fáni: reiknar og segir frá, skrifar ekkert — sjá [Að sjá áður en skrifað er](invocation.md#að-sjá-áður-en-skrifað-er) | slökkt |

**Aðaltextinn sjálfgefið; þýðingin með `--document translation -t FILE`.** Þá er það
þýðingin sem er breytt og skrifuð, og aðeins hún — sjá [Þýðing](invocation.md#þýðing).

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\nMarie\n\n2\n00:00:03,000 --> 00:00:04,000\n<i>Salut</i>\n\n' > a.srt; subedit-cli dialogue-dashes --add --output t.srt a.srt; cat t.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\nMarie\n\n2\n00:00:03,000 --> 00:00:04,000\n<i>Salut</i>\n\n' > a.srt; subedit-cli dialogue-dashes --add --output t.srt a.srt; cat t.srt
a.srt: 2 subtitles dashed -> t.srt
1
00:00:01,000 --> 00:00:02,000
- Bonjour
- Marie

2
00:00:03,000 --> 00:00:04,000
<i>- Salut</i>
```

## Úttak

**Staðalúttak** — ekkert: niðurstaðan er skráin sem er skrifuð. Með `--dry-run` kemur listi yfir
breytingar, lýst í [Að sjá áður en skrifað er](invocation.md#að-sjá-áður-en-skrifað-er).

| Stig | Hverju er bætt við villuúttakið |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles dashed -> <destination>`, eða `N subtitles undashed`, eða `nothing to change` |
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
| hvorki `--add` né `--remove` | `dialogue-dashes needs --add to put dashes on, or --remove` |
| bæði | `--add and --remove say opposite things; give one of them` |
| tímabil illa skrifað eða utan skrárinnar | sjá [`adjust`](adjust.md#--range) |

Villur áfangastaðarins eru sameiginlegar undirskipununum sem skrifa:
sjá [Ræsing](invocation.md#áfangastaðurinn).
