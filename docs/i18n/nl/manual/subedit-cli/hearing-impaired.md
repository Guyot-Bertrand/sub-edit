# `hearing-impaired`

<!-- machine-translated -->
> Deze pagina is een machinevertaling en is nog niet nagelezen door een lezer van de taal.

```
subedit-cli hearing-impaired
                  (--output FILE | --output-dir DIR | --in-place)
                  [--dry-run]
                  [--recursive] [--parentheses]
                  [--document main|translation] [-t FILE] [--align-method position|number]
                  <file>...
```

Verwijdert de **vermeldingen voor slechthorenden** — de geluiden die tussen vierkante haken worden beschreven — uit het hele
bestand. **Haakjes blijven standaard staan**: ze worden even vaak gebruikt voor een gefluisterde
regel als voor een beschreven geluid, en ze verwijderen zou dialoog wissen. `--parentheses` verwijdert ze
ook, voor een bestand dat zijn geluiden erin beschrijft.

Wat het niet doet: songteksten tussen hekjes, de naam van de spreker
vóór een dubbele punt, opnieuw hoofdletters zetten, OCR-fouten corrigeren. Die
patronen zijn die van de assistent
[`Correct Texts…`](../subedit-gui/correct-texts.md) van het venster, die de opdrachtregel
aanbiedt onder de naam [`correct`](correct.md); en hier kan niets worden ingesteld: de
transformatie ligt vast, ze is niet configureerbaar.

<!-- example: subedit-cli hearing-impaired --help -->
```console
$ subedit-cli hearing-impaired --help
Remove the sounds described between square brackets
Usage: subedit-cli hearing-impaired [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to clean

Options:
  -h,--help                   Print this help message and exit
  --parentheses               Also remove the sounds between parentheses, often whispered lines
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --document main|translation The document to change: the main one, or the translation given by -t
  -t,--translation-file FILE  Translation file to lay over the subtitle file, for a single input
  --align-method position|number
                              How the lines of the translation find their subtitles
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Argumenten en opties

| Optie | Verplicht | Waarden | Standaard |
| :----- | :----- | :------ | :----- |
| `<file>...` | ja | een of meer paden | — |
| `--parentheses` | nee | een vlag: verwijdert ook de vermeldingen tussen haakjes | uit |
| `--recursive`, `-r` | nee | een vlag | uit — zie [Een mappenstructuur verwerken](lots.md) |
| `--document` | nee | `main` of `translation` — zie [Een vertaling](invocation.md#een-vertaling) | `main` |
| `-t`, `--translation-file` | met `--document translation` | een vertaalbestand, voor **één enkele** invoer | — |
| `--align-method` | nee, en alleen met `-t` | `position` of `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **een van de drie**, behalve met `--dry-run` | zie [Aanroep](invocation.md#de-bestemming) | — |
| `--dry-run` | nee | een vlag: berekent en meldt, schrijft niets — zie [Kijken voor het schrijven](invocation.md#kijken-voor-het-schrijven) | uit |

Het formaat van het gelezen bestand blijft **behouden**: het formaat wijzigen is het werk van
[`convert`](convert.md).

**Standaard de hoofdtekst; de vertaling met `--document translation -t FILE`.** Dan is het
de vertaling die wordt gewijzigd en geschreven, en alleen die — zie [Een vertaling](invocation.md#een-vertaling).

**Een vermelding die een vertaling zou leegmaken, schrijft die leeg en behoudt de ondertitel**: de ondertitel
blijft in het bestand, zonder tekst. De hoofdtekst daarentegen verliest een volledig leeggemaakte ondertitel, zoals
hieronder.

## Wat wordt verwijderd, en wat blijft

Een vermelding is een vierkante haak — of een haakje, met `--parentheses` — en wat die omsluit, **regeleinde
inbegrepen**: in echte bestanden wordt een vermelding vaak door het
einde van een regel doorgesneden.

| Invoer | Uitvoer |
| :----- | :----- |
| `Bonjour [il tousse] Marie` | `Bonjour Marie` |
| `Bonjour[il tousse]Marie` | `Bonjour Marie` |
| `[Bruit de pas] Bonjour` | `Bonjour` |
| `A [un] [deux] B` | `A B` |

De verwijdering laat **precies één spatie tussen wat eromheen stond, en niets aan
de rand van een regel**. Het is geen opschoning van de spaties van het bestand: twee spaties
die de tekst al bevatte, blijven twee spaties.

**Als de vermelding over het regeleinde heen loopt, blijft het regeleinde staan** — de
twee regels blijven twee regels. Dit geval is geen curiositeit: in vijftien
echte ondertitelingen was elke vierkante haak die verweesd leek, een
vermelding die op die manier was doorgesneden.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nReculez ! [Il\nhurle] Tout de suite !\n\n' > a.srt; subedit-cli --quiet hearing-impaired --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nReculez ! [Il\nhurle] Tout de suite !\n\n' > a.srt; subedit-cli --quiet hearing-impaired --output b.srt a.srt; cat b.srt
1
00:00:01,000 --> 00:00:03,000
Reculez !
Tout de suite !
```

**Wat geen vermelding is en niet verandert:**

| Geval | Wat er gebeurt |
| :-- | :-------------- |
| puur numerieke inhoud — `[1]`, `(12)`, `[ 156478 ]` | blijft zoals het is, scheidingstekens inbegrepen: een verwijzing heeft nooit een geluid benoemd |
| een haakje en wat het omsluit, zonder `--parentheses` | blijft zoals het is: `Venez (chut) ici` verandert niet |
| een vierkante haak of een haakje dat niets sluit | blijft zoals het is |
| een opmaaktag — `<i>`, `</i>` | wordt nooit verwijderd |

Een spatie **tussen** de cijfers, een letter of een teken — `[1 2]`, `[1a]`,
`[-1]` — maakt er een vermelding van. Lege inhoud, `[]` of `( )`, ook: geen
cijfer, dus geen verwijzing; niets te zeggen, dus niets te bewaren.

## Wat helemaal verdwijnt

**Een regel die door de verwijdering leeg is geworden, verdwijnt**, en **een volledig
leeggemaakte ondertitel wordt uit het bestand verwijderd**. De ondertitels die volgen, worden hernummerd.

Een regel telt als leeg wanneer er alleen nog spaties, opmaaktags
of een alleenstaand dialoogstreepje in over zijn — `<i>[Musique]</i>` laat geen
`<i></i>` op het scherm achter.

**De tags zijn die van het formaat van het bestand.** `{\i1}[Musique]{\i0}` in een
Advanced SSA, `{Y:i}[Musique]` in een MicroDVD en `/[Musique]` in een MPL2 laten
evenmin iets over. Omgekeerd is een `<i>` in een formaat dat geen
tag schrijft — TMPlayer, LRC — tekst die de kijker leest, en de regel die
hem bevat is niet leeg.

**Een dialoog die tot één enkele stem is teruggebracht, is er geen meer**, en het streepje dat overblijft,
verdwijnt. Zolang er twee stemmen overblijven, blijven de streepjes staan.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\nVoir [1] la note.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\nVoir [1] la note.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle cleaned, 1 removed -> b.srt
1
00:00:04,000 --> 00:00:06,000
Attends Marie.

2
00:00:07,000 --> 00:00:09,000
Voir [1] la note.
```

## Een bestand zonder enige vermelding wordt toch geschreven

Het wordt identiek gekopieerd, de afsluitcode is `0`, en het rapport zegt dat er
niets te verwijderen was.

Het is de regel van de subcommando's die schrijven: **een opgegeven bestemming is
een geschreven bestemming**. Hier een uitzondering maken zou een script dwingen te weten
welke van de subcommando's soms een bestand oplevert en soms niets.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nRien à signaler.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nRien à signaler.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt
a.srt: no mention to remove -> b.srt
```

## Uitvoer

**Standaarduitvoer** — niets: het resultaat is het geschreven bestand. **Met `--dry-run` zijn het de voorgestelde wijzigingen**: voor elke ondertitel waarvan de tekst zou veranderen, zijn nummer, de tekst ervoor en de tekst erna — zie [Kijken voor het schrijven](invocation.md#kijken-voor-het-schrijven).

| Niveau | Wat er op de foutuitvoer wordt toegevoegd |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles cleaned, M removed -> <destination>`, of `<path>: no mention to remove -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — het formaat, de codering, de markering en de regeleinden van het gelezen bestand, teruggezet zoals ze waren |
| 3 | `<path>: N bytes read, M written`, daarna **elke diagnostische melding bij het lezen** — zie [Aanroep](invocation.md#diagnostische-meldingen-lezen) |

`N` telt de ondertitels waarvan de tekst is veranderd, `M` degene die door de verwijdering
volledig zijn leeggemaakt en die dus het bestand hebben verlaten.

### In JSON

Met [`--format json`](invocation.md#uitvoer-die-een-script-kan-lezen) bevat het object van elk bestand `counts.cleaned` (de herschreven teksten) en `counts.removed` (de verwijderde ondertitels); **met `--dry-run`** somt de sleutel `changes` bovendien de gewijzigde ondertitels op, zoals de standaarduitvoer van de tekstvorm. De volledige tabel is die van [Aanroep](invocation.md#wat-elk-subcommando-in-counts-zet).

## Afsluitcodes

Die van het hulpprogramma: `0` als elk bestand is geschreven, `2` als geen enkel, `3` als
slechts enkele, `1` bij een gebruiksfout.

## Fouten

Dit subcommando heeft geen eigen fout: het neemt geen
waarde om te parsen, en geen enkele tekst kan het een bestand laten weigeren. Wat
kan mislukken is het lezen of het schrijven, net als bij de andere.

**Zonder een van de drie bestemmingsopties wordt niets geschreven**, en hun fouten
zijn gemeenschappelijk voor de subcommando's die schrijven:
zie [Aanroep](invocation.md#de-bestemming).
