# `framerate`

<!-- machine-translated -->
> Deze pagina is een machinevertaling en is nog niet nagelezen door een lezer van de taal.

> **Dit is niet [`snap`](snap.md).** Dit commando **schaalt het
> bestand**: het is bedoeld voor als de timing niet klopt en tegen het einde van de film
> met enkele seconden uit de pas loopt. `snap` zet de tijdstempels terug op het dichtstbijzijnde
> raster zonder iets opnieuw te timen, en is bedoeld voor als de timing al
> goed is. Beide hebben dezelfde vorm, één framerate of twee, en ze door elkaar halen geeft
> geen fout.

```
subedit-cli framerate --from <fps> --to <fps>
                      (--output FILE | --output-dir DIR | --in-place)
                      [--dry-run]
                      [--recursive]
                      <file>...
```

Timet een bestand dat voor de ene framerate is getimed opnieuw voor een andere. Het is
de bewerking die een ondertitelbestand nodig heeft als het is gekozen voor een kopie die
niet die van de film is die men heeft — bijvoorbeeld een PAL-dvd tegenover een NTSC-transfer.

Het formaat van het gelezen bestand blijft **behouden**: het formaat wijzigen is het werk van
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

## Argumenten en opties

| Optie | Verplicht | Waarden | Standaard |
| :----- | :----- | :------ | :----- |
| `<file>...` | ja | een of meer paden | — |
| `--recursive`, `-r` | nee | een vlag | uit — zie [Een mappenstructuur verwerken](lots.md) |
| `--from` | **ja** | een framerate, zie hieronder | — |
| `--to` | **ja** | een framerate, zie hieronder | — |
| `--output` / `--output-dir` / `--in-place` | **een van de drie**, behalve met `--dry-run` | zie [Aanroep](invocation.md#de-bestemming) | — |
| `--dry-run` | nee | een vlag: berekent en meldt, schrijft niets — zie [Kijken voor het schrijven](invocation.md#kijken-voor-het-schrijven) | uit |

`--from` is de framerate waarvoor het bestand **is** getimed, `--to` degene
waarvoor het gewenst is. Ze verwisselen verschuift het bestand de
andere kant op — ongeveer zeven seconden bij een speelfilm tussen 25 en
23.976 — en niets waarschuwt ervoor voordat de film is bekeken.

## Een framerate schrijven

In frames per seconde, decimalen inbegrepen, met de punt als scheidingsteken:

```
25      24      23.976      29.97      60
```

**Een nul of negatieve framerate wordt geweigerd**: het is geen framerate.

### De drie NTSC-framerates kunnen niet exact worden geschreven

`23.976`, `29.97` en `59.94` zijn **etiketten**. De framerates die ze
aanduiden zijn `24000/1001`, `30000/1001` en `60000/1001`, waarvan geen enkele
een eindige decimale schrijfwijze heeft. Het hulpprogramma herkent ze als zodanig:

| Wat er wordt geschreven | Wat het aanduidt |
| :------------- | :------------------ |
| `23.976` | `24000/1001` — film getransfereerd naar NTSC-video |
| `24` | `24` — film |
| `25` | `25` — PAL- en SECAM-video |
| `29.97` | `30000/1001` — NTSC-video |
| `30`, `50`, `60` | zichzelf |
| `59.94` | `60000/1001` |

De herkenning gaat over **de waarde en niet over de letters**: `23.9760`
duidt dezelfde framerate aan als `23.976`.

Elke andere decimaal wordt letterlijk genomen — `23.9` is van niemand de
standaard, er valt niets in te lezen. Dat maakt de bovenstaande tabel ook
noodzakelijk: zonder haar zou `23.976` `23976/1000` zijn, en de opdrachtregel
zou voor dezelfde woorden iets anders zeggen dan het venster.

## Wat de posities worden

```
t′ = round( t × (from / to) )
```

Een film die op 23.976 is gemasterd en met 25 frames per seconde wordt afgespeeld, loopt sneller,
dus zijn ondertitels komen eerder; andersom komen ze
later.

**De factor is een exacte rationale breuk, en de afronding gebeurt maar één keer.** Het
pad dat gelijkwaardig lijkt — de positie omzetten naar een framenummer op de
invoersnelheid en dan terug naar milliseconden op de uitvoersnelheid — is
dat niet: het kwantiseert elke positie op het invoerraster en zit er tot
een half frame naast, 21 ms gemeten. Zie
[ADR 0013](../../../../adr/0013-mise-a-l-echelle-exacte-des-positions.md).

In de praktijk is de factor van 25 naar 23.976 `1001/960`:

| Gelezen positie | Geschreven | Wat via de frames zou geven |
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

## Uitvoer

**Standaarduitvoer** — niets: het resultaat is het geschreven bestand. Met `--dry-run` ook niets: er is geen bestand, en de toelichtingsregel zegt dat — zie [Kijken voor het schrijven](invocation.md#kijken-voor-het-schrijven).

| Niveau | Wat er op de foutuitvoer wordt toegevoegd |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles retimed from 25 to 24000/1001 fps -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — het formaat, de codering, de markering en de regeleinden van het gelezen bestand, teruggezet zoals ze waren |
| 3 | `<path>: N bytes read, M written`, daarna **elke diagnostische melding bij het lezen** — zie [Aanroep](invocation.md#diagnostische-meldingen-lezen) |

**De toelichting noemt de exacte framerate, niet het ingetypte etiket.** `23.976`
op die regel schrijven zou een conversie melden die niet heeft plaatsgevonden; het is
de enige plek waar men ziet welke van de twee is gebruikt. Een framerate die een
decimaal exact schrijft, wordt daar als decimaal getoond, de andere als breuk.

### In JSON

Met [`--format json`](invocation.md#uitvoer-die-een-script-kan-lezen) bevat het object van elk bestand `counts.subtitles` (elke ondertitel van het bestand). De volledige tabel is die van [Aanroep](invocation.md#wat-elk-subcommando-in-counts-zet).

## Afsluitcodes

Die van het hulpprogramma: `0` als elk bestand is geschreven, `2` als geen enkel, `3` als
slechts enkele, `1` bij een gebruiksfout.

Geen enkele weigering hangt af van de inhoud van een bestand: twee framerates bepalen altijd
een factor. Het is het enige van de subcommando's die schrijven waarvan alle
weigeringen vóór de eerste leesbewerking komen.

<!-- example: printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 0 --to 25 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 0 --to 25 --output b.srt a.srt; echo "code=$?"
"0" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed
code=1
```

## Fouten

| Wat het veroorzaakt | Melding |
| :------------------ | :------ |
| nul of negatieve framerate | `"…" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed` |
| decimale komma | `"…" is not a frame rate: use a decimal point and not a comma, in every language` |
| tekst die geen getal is | `"…" is not a frame rate: expected frames per second, like 25 or 23.976` |
| framerate buiten alle proporties | `"…" is not a frame rate: no video runs at that many frames per second` |

**Zonder een van de drie bestemmingsopties wordt niets geschreven**, en hun
fouten zijn gemeenschappelijk voor de subcommando's die schrijven:
zie [Aanroep](invocation.md#de-bestemming).
