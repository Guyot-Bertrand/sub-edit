# `disjoint`

<!-- machine-translated -->
> Deze pagina is een machinevertaling en is nog niet nagelezen door een lezer van de taal.

```
subedit-cli disjoint (--output FILE | --output-dir DIR | --in-place)
                     [--minimum TIME] [--max-lines N] [--dashes] [--sort]
                     [--dry-run] [--recursive]
                     <file>...
```

Knipt de overlappende ondertitels zo bij dat **geen enkel bereik `[start, end]` nog een ander bereik overlapt**. Het is `Make Ranges Disjoint…` van het venster — [dezelfde logica](../subedit-gui/lines.md#de-bereiken-niet-overlappend-maken).

**Niets wordt ingekort of verlengd**: op elk moment toont het bestand dezelfde teksten als voorheen. Wat
verandert, is de manier waarop de tijdlijn wordt geknipt:

- ze wordt geknipt bij **elk begin en elk einde** van de ondertitels van een overlappende groep;
- een interval dat door **één enkele** ondertitel wordt gedekt, blijft die ondertitel;
- een interval dat door **meerdere** wordt gedekt, wordt **één** ondertitel waarvan de regels die van die ondertitels zijn, **in
  de volgorde van hun begin** — de vroegste op regel 1, de volgende op regel 2;
- een ondertitel die niets overlapt, wordt niet aangeraakt, en de volgorde van de rest blijft behouden.

<!-- example: subedit-cli disjoint --help -->
```console
$ subedit-cli disjoint --help
Cut overlapping subtitles so that no range overlaps another, merging what overlaps
Usage: subedit-cli disjoint [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to make disjoint

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --minimum TIME              Shortest interval kept; a shorter one is taken over by a neighbour (default 0.08)
  --max-lines N               Most lines of a merged subtitle; more are shown in windows, 0 for no limit (default 2)
  --dashes                    Put a dash before the text of each merged subtitle
  --sort                      Put the subtitles in order of their start first; without it, a file out of order is refused
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Argumenten en opties

| Optie | Verplicht | Waarden | Standaard |
| :----- | :----- | :------ | :----- |
| `<file>...` | ja | een of meer paden | — |
| `--minimum` | nee | een duur: `0.08`, `1.5`, `00:00:00,080`. Nul behoudt elk interval | `0.08` (ongeveer twee frames) |
| `--max-lines` | nee | een geheel getal: het maximale aantal regels van een samengevoegde ondertitel; `0` heft de limiet op | `2` |
| `--dashes` | nee | een vlag: een streepje voor de tekst die elke ondertitel in een samengevoegde ondertitel inbrengt | uit |
| `--sort` | nee | een vlag: zet de ondertitels **eerst** in de volgorde van hun begin — zie [Een geordend bestand is nodig](#een-geordend-bestand-is-nodig) | uit |
| `--recursive`, `-r` | nee | een vlag | uit — zie [Een mappenstructuur verwerken](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **een van de drie**, behalve met `--dry-run` | zie [Aanroep](invocation.md#de-bestemming) | — |
| `--dry-run` | nee | een vlag: berekent en meldt, schrijft niets — zie [Kijken voor het schrijven](invocation.md#kijken-voor-het-schrijven) | uit |

<!-- example: printf '1\n00:00:00,000 --> 00:00:05,000\nA\n\n2\n00:00:03,000 --> 00:00:08,000\nB\n\n' > chevauche.srt; subedit-cli disjoint --output disjoint.srt chevauche.srt; cat disjoint.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:05,000\nA\n\n2\n00:00:03,000 --> 00:00:08,000\nB\n\n' > chevauche.srt; subedit-cli disjoint --output disjoint.srt chevauche.srt; cat disjoint.srt
chevauche.srt: 2 subtitles split, 1 merged -> disjoint.srt
1
00:00:00,000 --> 00:00:03,000
A

2
00:00:03,000 --> 00:00:05,000
A
B

3
00:00:05,000 --> 00:00:08,000
B
```

`A [0, 5]` en `B [3, 8]` geven drie niet-overlappende ondertitels: `A` alleen, `A` en `B` samen, `B` alleen. Geen
enkel moment gaat verloren of wordt toegevoegd.

## Wat de bewerking beslist

**Een regel wordt nooit gedupliceerd.** Twee ondertitels met dezelfde tekst geven één enkele regel waar ze
overlappen, en twee naburige stukken die hetzelfde tonen, vormen slechts één ondertitel. De tekst van een
ondertitel die in meerdere stukken is geknipt, wordt verder **in elk stuk herhaald**: het is wat op dat moment
in beeld staat.

**Een heel kort interval wordt door een buur overgenomen.** Onder `--minimum` verdwijnt een interval: de buur
**breidt zijn bereik uit om het te dekken**, zodat geen enkel moment leeg blijft en niets met meer dan die
drempel verschuift. De gekozen buur is die waarvan de regels die van het interval **al bevatten**, anders de
langste. De tekst van het interval gaat alleen verloren als geen enkele buur hem draagt, en dan voor minder dan de drempel.
**Een hele ondertitel die korter is dan de drempel wordt nooit overgenomen**: hij blijft staan en wordt geteld.

**Meer regels dan de limiet: vensters.** Een samengevoegde ondertitel die meer dan `--max-lines` regels zou hebben,
wordt **in opeenvolgende vensters** van die grootte getoond, in gelijke delen van de duur. Drie regels over drie
seconden:

<!-- example: printf '1\n00:00:00,000 --> 00:00:03,000\nA\n\n2\n00:00:00,000 --> 00:00:03,000\nB\n\n3\n00:00:00,000 --> 00:00:03,000\nC\n\n' > trois.srt; subedit-cli disjoint --output fenetres.srt trois.srt; cat fenetres.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:03,000\nA\n\n2\n00:00:00,000 --> 00:00:03,000\nB\n\n3\n00:00:00,000 --> 00:00:03,000\nC\n\n' > trois.srt; subedit-cli disjoint --output fenetres.srt trois.srt; cat fenetres.srt
trois.srt: 0 subtitles split, 1 merged; 1 shown in windows for holding too many lines -> fenetres.srt
1
00:00:00,000 --> 00:00:01,500
A
B

2
00:00:01,500 --> 00:00:03,000
B
C
```

Vier regels geven drie vensters, enzovoort: het venster schuift telkens één regel op. Niets wordt
afgekapt of opnieuw geknipt.

**De vertaling volgt dezelfde sneden**, aangezien een ondertitel beide teksten draagt. **Wat het formaat er
verder bij draagt** — een stijl, een positie — is dat van de eerste ondertitel die dekt; het verlies wordt geteld wanneer
de samengevoegde ondertitels verschillen.

**Een ondertitel waarvan het einde vóór het begin ligt**, wordt niet geraden: hij blijft waar hij is en wordt geteld.

## Een geordend bestand is nodig

De groepen zijn reeksen van buren: buiten volgorde zijn de buren niet degene die overlappen. Een bestand
waarvan de begintijden niet op volgorde staan, wordt daarom **geweigerd**, en de melding zegt dat:

<!-- example: printf '1\n00:00:03,000 --> 00:00:08,000\nB\n\n2\n00:00:00,000 --> 00:00:05,000\nA\n\n' > desordre.srt; subedit-cli disjoint --output x.srt desordre.srt; echo "code $?" -->
```console
$ printf '1\n00:00:03,000 --> 00:00:08,000\nB\n\n2\n00:00:00,000 --> 00:00:05,000\nA\n\n' > desordre.srt; subedit-cli disjoint --output x.srt desordre.srt; echo "code $?"
desordre.srt: The subtitles are not in the order of their start: sort them first. Use --sort to do that here.
code 2
```

`--sort` sorteert eerst — hetzelfde als [`sort`](sort.md) — en meldt dat. De kern sorteert nooit uit zichzelf.

Een tweede doorgang verandert niets: zodra de bereiken niet-overlappend zijn, valt er niets meer te knippen, en
de zin zegt `already disjoint`.

## Uitvoer

**Standaarduitvoer** — niets: het resultaat is het geschreven bestand.

| Niveau | Wat er op de foutuitvoer wordt toegevoegd |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles split, M merged -> <destination>`, gevolgd na een `;` door wat niet volledig kon worden gedaan: `K shown in windows for holding too many lines`, `K very short taken over by a neighbour`, `K subtitles shorter than the limit left whole`, `K merged with styles or positions that differ, the first kept`, `K subtitles ending before the start left alone` — of `<path>: already disjoint -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, daarna **elke diagnostische melding bij het lezen** |

`N subtitles split` telt de oorspronkelijke ondertitels die in meerdere stukken zijn geknipt, `M merged` de stukken die
meerdere ondertitels samen tonen.

### In JSON

Met [`--format json`](invocation.md#uitvoer-die-een-script-kan-lezen) de `counts` van elk bestand:

| Telling | Wat het zegt |
| :----- | :----------- |
| `subtitles` | de ondertitels van het geschreven bestand |
| `split`, `merged` | zoals in de zin |
| `windowed` | de stukken die in vensters worden getoond |
| `absorbed` | de intervallen die door een buur zijn overgenomen |
| `too_short` | de ondertitels die korter zijn dan de drempel, heel gelaten |
| `ending_before_start` | de ondertitels waarvan het einde vóór het begin ligt, met rust gelaten |
| `extras_dropped` | de samengevoegde stukken waarvan stijl of positie verschilt |
| `moved` | alleen met `--sort`: de plaatsen die zijn veranderd |

## Afsluitcodes

Die van het hulpprogramma: `0` als elk bestand is verwerkt — een bestand zonder overlappingen telt mee —, `2` als
geen enkel, `3` als slechts enkele, `1` bij een gebruiksfout.

## Fouten

| Wat het veroorzaakt | Melding |
| :------------------- | :------ |
| een bestand buiten volgorde zonder `--sort` | `<path>: The subtitles are not in the order of their start: sort them first. Use --sort to do that here.` |
| `--minimum` is geen duur, of is negatief | `--minimum: …` |
| `--max-lines` is geen geheel getal | `--max-lines: "…" is not a number of lines: expected a whole number, 0 for no limit` |

Een onleesbaar bestand mislukt zoals bij elk subcommando; de fouten van de bestemming zijn gemeenschappelijk voor de
subcommando's die schrijven: zie [Aanroep](invocation.md#de-bestemming).

## In de andere commando's

[`convert`](convert.md#de-bereiken-niet-overlappend-maken), [`append`](append.md#de-bereiken-niet-overlappend-maken) en
[`split-file`](split-file.md#de-bereiken-niet-overlappend-maken) accepteren `--disjoint`, dat hetzelfde doet met
de standaardinstellingen, vóór het schrijven.

Bij een slecht gesegmenteerd bestand — een tekst die in opeenvolgende stukken is geknipt, buiten volgorde aangeleverd —, geven sorteren en knippen
alleen regels die van het ene stuk naar het andere wisselen: eerst [`join-repeats`](join-repeats.md), of
[`repair`](repair.md), dat alle drie aaneenschakelt.
