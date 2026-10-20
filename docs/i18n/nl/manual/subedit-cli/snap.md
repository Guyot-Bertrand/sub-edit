# `snap`

<!-- machine-translated -->
> Deze pagina is een machinevertaling en is nog niet nagelezen door een lezer van de taal.

```
subedit-cli snap --rate <rate>
                 (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 <file>...
```

Zet **elke tijdstempel** — begin en einde — op het dichtstbijzijnde frame van de
opgegeven framerate. Er wordt niets opnieuw getimed: elke positie verschuift
hooguit een half frame.

## `snap` is niet `framerate`, en ze door elkaar halen valt niet op

De twee commando's hebben dezelfde vorm — bestanden, een bestemming, een framerate — en doen tegengestelde dingen.

| | [`framerate`](framerate.md) | `snap` |
| :--- | :-------------------------- | :----- |
| wat het berekent | `t' = t x input rate / output rate` | het dichtstbijzijnde frame, op het raster van `--rate` |
| hoeveel het verschuift | evenredig met de tijd — **meerdere seconden** bij een speelfilm | hooguit een half frame, **overal** |
| voor welk bestand | een bestand waarvan de **timing fout is** | een bestand waarvan de timing **goed is** en het raster fout |

**Het gebruik van `snap`**, uitgeschreven: een film met 25 frames per
seconde, een ondertitelbestand geschreven op een raster van 24, regels die al
min of meer op hun plaats staan, op enkele milliseconden na. Er valt niets opnieuw te timen
— alleen de tijdstempels moeten op de juiste frames worden teruggezet.

**Het gebruik van `framerate`** is het andere: een bestand getimed voor een master met
25 die men wil afspelen op een kopie met 23.976, en waarvan de timing tegen het einde van de film
meerdere seconden afdrijft.

Het ene voor het andere nemen **veroorzaakt geen fout**: beide
accepteren het bestand en schrijven een resultaat. De toelichtingsregel is wat ze
onderscheidt — `snap` zegt altijd hoeveel het de posities heeft verschoven.

<!-- example: subedit-cli snap --help -->
```console
$ subedit-cli snap --help
Move every position onto the nearest frame of a frame rate (see framerate)
Usage: subedit-cli snap [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to align

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --rate TEXT REQUIRED        Frame rate to align on: 25, 23.976
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
| `--rate` | **ja** | een framerate in frames per seconde, geschreven zoals bij [`framerate`](framerate.md#een-framerate-schrijven) | — |
| `--output` / `--output-dir` / `--in-place` | **een van de drie**, behalve met `--dry-run` | zie [Aanroep](invocation.md#de-bestemming) | — |
| `--dry-run` | nee | een vlag: berekent en meldt, schrijft niets — zie [Kijken voor het schrijven](invocation.md#kijken-voor-het-schrijven) | uit |

`--rate` accepteert **elke geldige framerate**, en niet alleen de acht
standaardwaarden: het is de keuze van de gebruiker, zoals bij `framerate`. Een nul of negatieve
framerate wordt geweigerd.

Het formaat van het gelezen bestand blijft **behouden**: het formaat wijzigen is
het werk van [`convert`](convert.md).

## Uitvoer

**Standaarduitvoer** — niets: het resultaat is het geschreven bestand. Met `--dry-run` ook niets: er is geen bestand, en de toelichtingsregel zegt dat — zie [Kijken voor het schrijven](invocation.md#kijken-voor-het-schrijven).

| Niveau | Wat er op de foutuitvoer wordt toegevoegd |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles aligned on 25 fps, M positions moved, by at most 20 ms -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — het formaat, de codering, de markering en de regeleinden van het gelezen bestand, teruggezet zoals ze waren |
| 3 | `<path>: N bytes read, M written`, daarna **elke diagnostische melding van het lezen** — zie [Aanroep](invocation.md#diagnostische-meldingen-lezen) |

**De twee getallen zijn de clou van de regel.** Een half frame bij 25 frames per
seconde is twintig milliseconden, en niets groters kan daar worden geschreven. Wie
`framerate` wilde en `snap` typte, leest ‘by at most 20 ms’ waar hij seconden
verwachtte, en weet het meteen.

## Een voorbeeld

Een regel op `00:00:01,010`, uitgelijnd op 25 frames per seconde — dus op een
veelvoud van veertig milliseconden:

<!-- example: printf '1\n00:00:01,010 --> 00:00:03,020\nUn.\n\n' > a.srt; subedit-cli snap --rate 25 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,010 --> 00:00:03,020\nUn.\n\n' > a.srt; subedit-cli snap --rate 25 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle aligned on 25 fps, 2 positions moved, by at most 20 ms -> b.srt
1
00:00:01,000 --> 00:00:03,040
Un.
```

## Twee eigenschappen

**Twee keer uitlijnen verandert de tweede keer niets.** Een positie die al op een
frame ligt, blijft daar: het is dezelfde bewerking op hetzelfde resultaat.

**De volgorde van de ondertitels blijft behouden.** Afronden naar het dichtstbijzijnde frame is
monotoon, dus twee begintijden die minstens een frame uit elkaar liggen, blijven dat. Dichter
bij elkaar kunnen ze samenvallen — een afwijking die
[`inspect`](inspect.md#wat-anomalies-meldt) kan benoemen, en geen
omkering.

### In JSON

Met [`--format json`](invocation.md#uitvoer-die-een-script-kan-lezen) bevat het object van elk bestand `counts.subtitles`, `counts.moved` (de verschoven posities) en `counts.furthest_ms` (de grootste verschuiving, in milliseconden). De volledige tabel is die van [Aanroep](invocation.md#wat-elk-subcommando-in-counts-zet).

## Afsluitcodes

Die van het hulpprogramma: `0` als elk bestand is geschreven, `2` als geen enkel,
`3` als slechts enkele, `1` bij een gebruiksfout.

## Fouten

Die van een framerate, woord voor woord die van [`framerate`](framerate.md#fouten)
— beide lezen dezelfde waarde langs hetzelfde pad:

| Wat het veroorzaakt | Melding |
| :------------------ | :------ |
| nul of negatieve framerate | `"…" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed` |
| decimale komma | `"…" is not a frame rate: use a decimal point and not a comma, in every language` |
| tekst die geen getal is | `"…" is not a frame rate: expected frames per second, like 25 or 23.976` |
| onevenredig grote framerate | `"…" is not a frame rate: no video runs at that many frames per second` |

**Zonder een van de drie bestemmingsopties wordt niets geschreven**, en hun fouten
zijn gemeenschappelijk voor de subcommando's die schrijven:
zie [Aanroep](invocation.md#de-bestemming).
