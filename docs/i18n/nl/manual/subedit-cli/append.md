# `append`

<!-- machine-translated -->
> Deze pagina is een machinevertaling en is nog niet nagelezen door een lezer van de taal.

```
subedit-cli append --output FILE
                   [--dry-run] [--sort] [--disjoint]
                   <base> <file>...
```

Zet bestanden **na het eerste** in **één enkel** bestand: de film in twee delen, omgezet in één
enkele reeks ondertitels. Het is `Append File…` van het venster — [dezelfde engine](../subedit-gui/operations.md#append-file),
dezelfde tellingen — toegepast zo vaak als er bestanden zijn om achteraan toe te voegen.

- **Het eerste bestand is de basis.** Het geeft het resultaat **zijn formaat, zijn codering, zijn
  bytevolgordemarkering en zijn regeleinden**.
- **Tussen twee bestanden wordt niets ingevoegd**: elk wordt verschoven met **het einde van de laatste ondertitel van wat
  eraan voorafgaat**, waarbij zijn posities hun volgorde en hun onderlinge afstand behouden. Het derde volgt het tweede, niet de
  basis: de bestanden volgen elkaar in de volgorde waarin ze zijn opgegeven.
- **Elk achteraan toegevoegd bestand wordt omgezet** naar het formaat van de basis: zijn tags worden omgezet,
  en wat dat formaat niet kan schrijven, wordt per bestand gemeld, in de woorden van het venster.
- **De achteraan toegevoegde bestanden hebben geen vertaling**: een bestand dat alleen is geopend, heeft er geen om te geven.
- **Een bestand kan meermaals worden opgegeven**, of aan zichzelf worden toegevoegd: niets verbiedt het.

**`append` is geen bulkverwerking.** N invoerbestanden, **één** uitvoer: de lus van de andere subcommando's — één resultaat
per bestand, en een mislukking waarbij de andere doorgaan — heeft hier geen zin. Daaruit volgen drie dingen:

- **de bestemming is `--output`, en niets anders.** `--output-dir` en `--in-place` bestaan niet voor
  `append` en worden geweigerd zoals elke onbekende optie, code `1`; `--recursive` evenzo: de volgorde van de
  bestanden is de opgegeven volgorde, en een doorlopen boomstructuur heeft er geen;
- **alles of niets**: het eerste bestand dat niet kan worden gelezen, stopt de run, en **er wordt niets geschreven**. Een
  film in drie delen waarvan er één ontbreekt, is geen film in twee delen;
- **één enkele regel per run** voor het resultaat, en, in JSON, **één enkel object**.

<!-- example: subedit-cli append --help -->
```console
$ subedit-cli append --help
Put subtitle files one after another into a single file
Usage: subedit-cli append [OPTIONS] files...

Positionals:
  files BASE FILE...          The base file, then the files to put after it, in order

Options:
  -h,--help                   Print this help message and exit
  --output FILE               File to write
  --dry-run                   Work out and say what would be written, and write nothing
  --sort                      Put the subtitles in order of their start before writing
  --disjoint                  Cut overlapping subtitles so that no range overlaps another, before writing
```

## Argumenten en opties

| Optie | Verplicht | Waarden | Standaard |
| :----- | :----- | :------ | :----- |
| `<base> <file>...` | ja, **minstens twee** | paden: de basis, daarna wat erachter wordt gezet, in volgorde | — |
| `--output` | **ja**, behalve met `--dry-run` | een bestandspad; de map wordt aangemaakt als ze ontbreekt | — |
| `--dry-run` | nee | een vlag: berekent en meldt, schrijft niets — zie [Kijken voor het schrijven](invocation.md#kijken-voor-het-schrijven) | uit |
| `--sort` | nee | een vlag: zet het geheel op volgorde van zijn begintijden **zodra alles is toegevoegd**, zie [Het resultaat sorteren](#het-resultaat-sorteren) | uit |
| `--disjoint` | nee | een vlag: knipt de overlappende ondertitels af **zodra alles is toegevoegd en gesorteerd**, zie [De bereiken niet-overlappend maken](#de-bereiken-niet-overlappend-maken) | uit |

**De bestemming mag geen van de invoerbestanden zijn**, de basis inbegrepen: schrijven over een bestand dat men
leest, wordt geweigerd voordat er iets is gelezen, code `1`. Een bestaand `--output` dat verschilt van de invoerbestanden wordt
overschreven, zoals overal.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n' > partie1.srt; printf '1\n00:00:00,500 --> 00:00:01,500\ntrois\n\n' > partie2.srt; subedit-cli append --output film.srt partie1.srt partie2.srt; cat film.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n' > partie1.srt; printf '1\n00:00:00,500 --> 00:00:01,500\ntrois\n\n' > partie2.srt; subedit-cli append --output film.srt partie1.srt partie2.srt; cat film.srt
partie2.srt: appended 1 subtitle
partie1.srt: 3 subtitles from 2 files -> film.srt
1
00:00:01,000 --> 00:00:02,000
un

2
00:00:03,000 --> 00:00:04,000
deux

3
00:00:04,500 --> 00:00:05,500
trois
```

De laatste ondertitel van `partie1.srt` eindigt na 4 s; die van `partie2.srt`, die om 0.5 s begon, begint
om 4.5 s.

## Het resultaat sorteren

Elk bestand wordt verschoven **vanaf het einde van wat eraan voorafgaat**: de volgorde die kan breken, is dus die van de bestanden zelf. `--sort`
zet het resultaat op volgorde van de begintijden, **in één keer, na de laatste toevoeging**, zoals [`sort`](sort.md) zou doen — stabiel, en zonder iets aan de
inhoud te wijzigen.

Nooit standaard. Met de optie meldt de basis het op het eerste niveau van de toelichting, `<base>: N subtitles moved` of `<base>: already in order`, en
het `--format json`-record bevat een telling `moved`, **afwezig zonder de optie**.

## De bereiken niet-overlappend maken

Nooit standaard. De optie doet wat [`disjoint`](disjoint.md) doet, **met zijn standaardinstellingen**, zodra alles is toegevoegd, op het geheel. Ze heeft een geordend bestand nodig: anders wordt ze geweigerd, en `--sort` — dat eerst komt — zet het vooraf op volgorde. Het bestand meldt het op het eerste niveau van de toelichting (`<path>: N subtitles split, M merged`), en het `--format json`-record bevat de tellingen `split` en `merged`, **afwezig zonder de optie**.

## Wanneer de formaten verschillen

Het resultaat is in **het formaat van de basis**, wat dat van de andere ook is. Wat het overgaan kost, wordt
per bestand gemeld, in de woorden van het venster — dezelfde zin als bij `Append File…` en bij
[plakken vanuit een ander formaat](../subedit-gui/clipboard.md#tags-van-een-ander-formaat-worden-omgezet):

```text
d.ass: appended 1 subtitle; Advanced SSA into SubRip: 1 tag dropped, the header was dropped
```

Wat verloren gaat, hangt alleen af van het formaat van **elk** achteraan toegevoegd bestand: een basis in Advanced SSA ontvangt een
SubRip-bestand zonder iets te verliezen, niet andersom. Om het formaat van het resultaat te kiezen, zet u eerst een bestand
dat al in dat formaat is, of zet u daarna om met [`convert`](convert.md).

## Uitvoer

**Standaarduitvoer** — niets: het resultaat is het geschreven bestand.

| Niveau | Wat er op de foutuitvoer wordt toegevoegd |
| :----- | :------------------------------------- |
| 1 | één regel per achteraan toegevoegd bestand — `<path>: appended N subtitles`, gevolgd, wanneer het formaat iets kostte, door `; <source format> into <base format>: <what is lost>` — daarna `<base>: N subtitles from M files -> <destination>` (met `--dry-run`, `(dry run, nothing written)` in plaats van `-> <destination>`) |
| 2 | `<base>: SubRip, UTF-8, no BOM, LF line endings kept` — het formaat, de codering, de markering en de regeleinden van de basis, teruggezet zoals ze waren |
| 3 | `<base>: N bytes read, M written` (alle bestanden samen), daarna **elke leesmelding** — zie [Aanroep](invocation.md#diagnostische-meldingen-lezen) |

### In JSON

Met [`--format json`](invocation.md#uitvoer-die-een-script-kan-lezen) **één enkel object** voor de hele run,
waarvan `file` **de basis** is. Zijn `counts`:

| Telling | Wat ze zegt |
| :----- | :----------- |
| `files` | de bestanden van de run, de basis inbegrepen |
| `subtitles` | de ondertitels van het resultaat |
| `appended` | die afkomstig zijn van de achteraan toegevoegde bestanden |
| `lost_ends`, `lost_header` | `0` of `1`: een van de achteraan toegevoegde bestanden verloor het |
| `joined_lines`, `lost_tags`, `lost_fields` | de som van wat de achteraan toegevoegde bestanden verloren |
| `furthest_ms` | de grootste verplaatsing van een positie, alle bestanden samen |

**Een array `inputs`** hoort bij de `counts`, erbuiten: één element per achteraan toegevoegd bestand, in volgorde,
`{"file", "counts"}` — waarbij `counts` `appended` bevat, daarna dezelfde verliezen, **voor dat bestand alleen**. Het is wat
in een run van tien bestanden zegt welk bestand wat kostte. De leeswaarschuwingen van alle bestanden
worden verzameld in `warnings`.

Een mislukking is ook een object, **één enkel**: dat van het eerste bestand dat niet kon worden gelezen.

## Afsluitcodes

| Code | Wanneer |
| :--- | :---- |
| `0` | het resultaat is geschreven — of berekend, met `--dry-run` |
| `1` | gebruiksfout: minder dan twee bestanden, geen `--output`, een optie die `append` niet heeft, een bestemming die een van de invoerbestanden is |
| `2` | **een bestand kon niet worden gelezen**, of de bestemming kon niet worden geschreven: er wordt niets geschreven. Er is geen code `3`: een run die maar één bestand schrijft, slaagt of slaagt niet |

## Fouten

| Wat ze veroorzaakt | Melding |
| :------------------- | :------ |
| één enkel bestand opgegeven | `append needs a base file and at least one file to put after it` |
| noch `--output` noch `--dry-run` | `no destination given: append writes one file, use --output` |
| `--output` noemt een invoerbestand | `<destination>: would be written over the input <input>` |
| een bestand dat ontbreekt, onleesbaar is of een onbekend formaat heeft | `<path>: does not exist`, `<path>: is in no format this tool knows`… — hetzelfde als overal |

**Een leeg bestand wordt niet achteraan toegevoegd**: geen enkel formaat leest zonder ten minste één ondertitel, en het bestand wordt
geweigerd zoals elk bestand dat niet kan worden gelezen — `<path>: holds nothing recognisable as a subtitle` —, wat
de run stopt en niets schrijft.
