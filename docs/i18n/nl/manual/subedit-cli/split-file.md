# `split-file`

<!-- machine-translated -->
> Deze pagina is een machinevertaling en is nog niet nagelezen door een lezer van de taal.

```
subedit-cli split-file --at N --head FILE --tail FILE
                       [--dry-run] [--sort] [--disjoint]
                       <file>
```

Knipt een bestand **in tweeën**: een kop en een staart. Het is `Split Project…` van het venster — [dezelfde engine](../subedit-gui/operations.md#split-project),
dezelfde tellingen, dezelfde weigering — en **het exacte omgekeerde van [`append`](append.md)**: de staart aan de
kop toevoegen geeft het oorspronkelijke bestand terug.

- **`--at N` is het nummer van de eerste ondertitel van de staart**, genummerd zoals de tabel van het venster
  ze nummert: vanaf 1. `--at 3` zet ondertitels 1 en 2 in de kop, en de derde en de rest in de
  staart. Het loopt **van de tweede tot de laatste**: een knip heeft aan elke kant een ondertitel nodig.
- **De staart wordt teruggebracht naar de oorsprong**: hij wordt verschoven met min het einde van de laatste ondertitel die in de kop blijft,
  waarbij zijn posities hun volgorde en onderlinge afstand behouden. Daardoor is `split-file` het omgekeerde van `append`.
- **Beide helften behouden het formaat, de codering, de bytevolgordemarkering en de regeleinden van het gelezen bestand**, en
  worden bij het schrijven vanaf 1 opnieuw genummerd, zoals elk teruggeschreven bestand.
- **Twee uitvoeren, dus een eigen bestemming**: `--head` en `--tail`, **beide verplicht**. `--output`,
  `--output-dir`, `--in-place` en `--recursive` bestaan niet voor `split-file` en worden geweigerd als elke onbekende
  optie, code `1`: er is geen map om twee benoemde bestanden in te delen, en geen invoer om te overschrijven.

<!-- example: subedit-cli split-file --help -->
```console
$ subedit-cli split-file --help
Cut a subtitle file in two, a head and a tail
Usage: subedit-cli split-file [OPTIONS] file

Positionals:
  file TEXT REQUIRED          Subtitle file to cut

Options:
  -h,--help                   Print this help message and exit
  --at N                      Number of the first subtitle of the tail, as the table numbers them
  --head FILE                 File to write the subtitles before the cut to
  --tail FILE                 File to write the subtitles from the cut on to
  --dry-run                   Work out and say what would be written, and write nothing
  --sort                      Put the subtitles in order of their start before cutting; --at counts in that order
  --disjoint                  Cut overlapping subtitles so that no range overlaps another, before cutting
```

## Argumenten en opties

| Optie | Verplicht | Waarden | Standaard |
| :----- | :----- | :------ | :----- |
| `<file>` | ja | **een enkel** pad | — |
| `--at` | ja | een geheel getal, van 2 tot het aantal ondertitels van het bestand | — |
| `--head`, `--tail` | **ja, beide**, behalve met `--dry-run` | bestandspaden; hun map wordt aangemaakt als ze ontbreekt | — |
| `--dry-run` | nee | een vlag: berekent en meldt, schrijft niets — zie [Kijken voor het schrijven](invocation.md#kijken-voor-het-schrijven) | uit |
| `--sort` | nee | een vlag: zet de ondertitels in de volgorde van hun begin **vóór het knippen**; `--at` telt in die volgorde | uit |
| `--disjoint` | nee | een vlag: knipt de overlappende ondertitels **vóór het knippen**; `--at` telt in wat overblijft, zie [De bereiken niet-overlappend maken](#de-bereiken-niet-overlappend-maken) | uit |

**Met `--dry-run` is geen bestemming vereist**: er mag er geen worden opgegeven, maar niet slechts een van de twee —
`--head` en `--tail` horen bij elkaar. Die welke worden opgegeven, worden beoordeeld alsof er geen `--dry-run` was.

**De twee bestemmingen kunnen niet de invoer zijn en ook niet hetzelfde bestand**: schrijven over wat men
leest, of de staart over de kop schrijven, wordt geweigerd voordat er iets wordt gelezen, code `1`.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n3\n00:00:06,000 --> 00:00:07,000\ntrois\n\n4\n00:00:08,000 --> 00:00:09,000\nquatre\n\n' > film.srt; subedit-cli split-file --at 3 --head tete.srt --tail queue.srt film.srt; cat queue.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n3\n00:00:06,000 --> 00:00:07,000\ntrois\n\n4\n00:00:08,000 --> 00:00:09,000\nquatre\n\n' > film.srt; subedit-cli split-file --at 3 --head tete.srt --tail queue.srt film.srt; cat queue.srt
film.srt: split at subtitle 3: 2 subtitles in the head, 2 subtitles in the tail -> tete.srt, queue.srt
1
00:00:02,000 --> 00:00:03,000
trois

2
00:00:04,000 --> 00:00:05,000
quatre
```

De tweede ondertitel, die in de kop blijft, eindigt op 4 s: de staart gaat 4 s terug, en de derde ondertitel,
die op 6 s begon, begint op 2 s.

## Knippen in tijdsvolgorde

`--at` nummert de ondertitels **zoals het bestand ze geeft**. Bij een bestand waarvan de volgorde gebroken is, is de kop dus niet de helft die het vroegst
in de tijd ligt. `--sort` zet de ondertitels in de volgorde van hun begin **voordat de knip wordt beoordeeld** — stabiel, zoals [`sort`](sort.md) doet — en
`--at` telt dan in die volgorde.

Nooit standaard. Met de optie zegt het bestand op het eerste niveau van de toelichting hoeveel ondertitels zijn verplaatst (`<path>: N subtitles moved`
of `<path>: already in order`), en het record van `--format json` draagt een telling `moved`, **afwezig zonder de optie**.

## De bereiken niet-overlappend maken

Nooit standaard. De optie doet wat [`disjoint`](disjoint.md) doet, **met zijn standaardinstellingen**, voordat de knip wordt beoordeeld, na het sorteren: `--at` telt in wat dat oplevert. Ze heeft een geordend bestand nodig: anders wordt ze geweigerd, en `--sort` — dat eerst komt — zet het vooraf in volgorde. Het bestand zegt dat op het eerste niveau van de toelichting (`<path>: N subtitles split, M merged`), en het record van `--format json` draagt de tellingen `split` en `merged`, **afwezig zonder de optie**.

## Wanneer de knip wordt geweigerd

**Twee helften die bij de knip overlappen, worden niet geknipt.** Als een ondertitel van de staart begon vóór
het einde van de laatste ondertitel die achterblijft, zou de verschuiving hem vóór het begin van de video brengen — een negatieve positie,
die geen enkel formaat schrijft. `split-file` weigert **met vermelding van de ondertitel**, in de woorden van het venster:

```text
o.srt: Cannot split at subtitle 3: subtitle 3 would fall before the start of the video. Cut somewhere else.
```

De code is `2` en **er wordt niets geschreven**. Knip ergens anders. Precies op het begin uitkomen is toegestaan: nul is een
positie.

**`--at` buiten het bestand wordt op dezelfde manier geweigerd**, zodra het bestand is gelezen: `--at 1` — de eerste ondertitel kan geen
staart beginnen, er zou niets overblijven — en `--at` voorbij de laatste ondertitel, wat een lege
staart zou geven. Een bestand van één enkele ondertitel kan niet worden geknipt.

## Uitvoer

**Standaarduitvoer** — niets: het resultaat zijn de twee geschreven bestanden.

| Niveau | Wat er op de foutuitvoer wordt toegevoegd |
| :----- | :------------------------------------- |
| 1 | `<path>: split at subtitle N: H subtitles in the head, T subtitles in the tail -> <head>, <tail>`; met `--dry-run`, `(dry run, nothing written)` in plaats van `-> …` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — het formaat, de codering, de markering en de regeleinden van het gelezen bestand, teruggezet zoals ze waren |
| 3 | `<path>: N bytes read, H and T written`, daarna **elke diagnostische melding van het lezen** — zie [Aanroep](invocation.md#diagnostische-meldingen-lezen) |

### In JSON

Met [`--format json`](invocation.md#uitvoer-die-een-script-kan-lezen) **een enkel object**, waarvan `file` het geknipte bestand is.
**`destination` is de kop** — het bestand dat de invoer wordt — en **`tail` de staart**, een pad, of `null` met
`--dry-run` zoals `destination`. De `counts`: `subtitles` (die van het gelezen bestand), `head` en `tail` (die van elke
helft).

## Afsluitcodes

| Code | Wanneer |
| :--- | :---- |
| `0` | beide bestanden zijn geschreven — of berekend, met `--dry-run` |
| `1` | gebruiksfout: geen `--head` en `--tail`, een optie die `split-file` niet heeft, een bestemming die de invoer of de andere bestemming is, een `--at` die geen positief geheel getal is |
| `2` | het bestand kon niet worden gelezen, **de knip wordt geweigerd** (buiten het bestand, of overlap), of een bestemming kon niet worden geschreven. Er is geen code `3`: er wordt één enkel bestand verwerkt |

**De kop wordt eerst geschreven**: een schijf die de staart weigert, laat de kop achter, en de code is `2`.

## Fouten

| Wat het veroorzaakt | Melding |
| :------------------- | :------ |
| noch `--head` noch `--tail` | `no destination given: split-file writes two files, use --head and --tail` |
| slechts een van de twee, met `--dry-run` | `--head and --tail go together: give both, or neither with --dry-run` |
| de twee bestemmingen zijn hetzelfde bestand | `<path>: named as both the head and the tail` |
| een bestemming is de invoer | `<path>: would be written over the input <input>` |
| `--at` buiten het bestand | `<file>: --at 9: the file holds 3 subtitles, and the tail can start from 2 to 3` |
| een bestand van één ondertitel | `<file>: a file of 1 subtitle cannot be split: a cut needs a subtitle on each side` |
| overlap | `<file>: Cannot split at subtitle N: subtitle M would fall before the start of the video. Cut somewhere else.` |
| een bestand ontbreekt, is onleesbaar of heeft een onbekend formaat | `<path>: does not exist`… — hetzelfde als overal |
