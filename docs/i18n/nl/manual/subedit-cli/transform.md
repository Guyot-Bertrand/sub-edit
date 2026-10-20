# `transform`

<!-- machine-translated -->
> Deze pagina is een machinevertaling en is nog niet nagelezen door een lezer van de taal.

```
subedit-cli transform --first <index>=<time> --last <index>=<time>
                      (--output FILE | --output-dir DIR | --in-place)
                      [--dry-run]
                      [--recursive]
                      <file>...
```

Corrigeert **elke** positie van het bestand aan de hand van twee punten waarvan de juiste waarde
bekend is. Het is de bewerking waar een wegdrijvende ondertiteling om vraagt
— aan het begin goed, aan het einde fout — en waar [`shift`](shift.md), dat alles met
hetzelfde bedrag verplaatst, niets aan kan doen.

Het formaat van het gelezen bestand blijft **behouden**: het formaat wijzigen is
het werk van [`convert`](convert.md).

<!-- example: subedit-cli transform --help -->
```console
$ subedit-cli transform --help
Correct every position from two points known to be right
Usage: subedit-cli transform [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to transform

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --first TEXT REQUIRED       Earlier reference, as <index>=<time>: 1=00:00:01.000
  --last TEXT REQUIRED        Later reference, as <index>=<time>: 3=00:00:10.000
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
| `--first` | **ja** | een referentie, `<index>=<time>` | — |
| `--last` | **ja** | een referentie, `<index>=<time>` | — |
| `--output` / `--output-dir` / `--in-place` | **een van de drie**, behalve met `--dry-run` | zie [Aanroep](invocation.md#de-bestemming) | — |
| `--dry-run` | nee | een vlag: berekent en meldt, schrijft niets — zie [Kijken voor het schrijven](invocation.md#kijken-voor-het-schrijven) | uit |

Er is geen korte vorm: deze twee opties worden alleen voluit
geschreven.

## Een referentie schrijven

Een referentie noemt een ondertitel en zegt waar zijn **begin** nu moet vallen:

```
<index>=<time>
```

**De index wordt vanaf 1 geteld**, zoals hij wordt getoond en zoals een SubRip-bestand
nummert. `0` wordt geweigerd, en ook een index voorbij de laatste
ondertitel.

**De tijd wordt geschreven zoals die van [`shift`](shift.md)** — seconden of
tijdcode, het decimaalteken een punt, de resolutie de milliseconde:

| Vorm | Voorbeelden |
| :---- | :------- |
| seconden | `10`, `2.999` |
| tijdcode | `00:00:10.000`, `01:39:37.040` |

`--first 3=10` en `--first 3=00:00:10.000` zeggen hetzelfde.

`--first` en `--last` noemen twee punten, geen interval. Hun namen zeggen
het verwachte gebruik — een referentie aan het begin van het bestand, de andere aan het einde — en geen
beperking: ze verwisselen geeft hetzelfde resultaat, en **alles buiten
de twee referenties volgt zoals wat ertussen ligt**.

## Wat de andere posities worden

De twee referenties leggen een schaalfactor vast, en elke positie volgt die:

```
t′ = y₁ + (t − x₁) × r      with r = (y₂ − y₁) / (x₂ − x₁)
```

`x₁`, `x₂` zijn de huidige begintijden van de twee genoemde ondertitels, `y₁`, `y₂` de
gevraagde tijden. De factor wordt als exacte breuk bewaard en **er wordt maar één keer
afgerond**, op de milliseconde; daardoor vallen de twee
referenties **exact** op de gevraagde tijden en niet tot op een duizendste
(zie [ADR 0013](../../../../adr/0013-mise-a-l-echelle-exacte-des-positions.md)).

Elke ondertitel blijft in beeld **naar verhouding** van hoe lang hij er stond:
het einde volgt dezelfde schaal als het begin. Een omrekening is geen
verschuiving en heeft haar eigenschap niet.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,001 --> 00:00:07,000\nDeux.\n\n3\n00:00:09,000 --> 00:00:11,000\nTrois.\n\n' > a.srt; subedit-cli transform --first 1=00:00:01.000 --last 3=00:00:10.000 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,001 --> 00:00:07,000\nDeux.\n\n3\n00:00:09,000 --> 00:00:11,000\nTrois.\n\n' > a.srt; subedit-cli transform --first 1=00:00:01.000 --last 3=00:00:10.000 --output b.srt a.srt; cat b.srt
a.srt: 3 subtitles transformed onto 1=00:00:01.000 and 3=00:00:10.000 -> b.srt
1
00:00:01,000 --> 00:00:03,250
Un.

2
00:00:05,501 --> 00:00:07,750
Deux.

3
00:00:10,000 --> 00:00:12,250
Trois.
```

De eerste referentie verschuift niet, de derde gaat van 9 s naar 10 s: de factor is
`9/8`, en de tweede ondertitel — die niemand noemde — gaat van
`5,001` naar `5,501`, dat is `1000 + 4001 × 9/8` eenmaal afgerond.

## Twee referenties die er geen zijn

| Situatie | Wanneer het wordt gezien | Code |
| :-------- | :------------- | :--- |
| `--first` en `--last` noemen dezelfde index | vóór elk lezen | `1` |
| een index ligt voorbij de laatste ondertitel | wanneer het bestand wordt geopend | `2` of `3` |
| de twee genoemde ondertitels beginnen op hetzelfde moment | wanneer het bestand wordt geopend | `2` of `3` |

Alle drie worden om dezelfde reden geweigerd: `x₂ − x₁` is nul, en twee
samenvallende punten bepalen geen lijn. Een deling door nul teruggeven als
resultaat vermomd zou erger zijn dan weigeren.

Het eerste geval is een **gebruiksfout** — de twee opties zijn fout, welke
bestanden er ook zijn, dus er is één enkele melding verschuldigd, niet één per bestand. De
andere twee hangen af van het geopende bestand en gelden daarom als een
verwerkingsfout.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n' > a.srt; subedit-cli transform --first 2=1.000 --last 2=4.000 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n' > a.srt; subedit-cli transform --first 2=1.000 --last 2=4.000 --output b.srt a.srt; echo "code=$?"
--first and --last both name subtitle 2, and two references on one subtitle define no transform
code=1
```

Een index buiten de grenzen wordt geweigerd **met vermelding van de grens**, wat tegelijk
zegt wat er had kunnen worden geschreven:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,000 --> 00:00:07,000\nDeux.\n\n' > a.srt; subedit-cli transform --first 1=1.000 --last 9=10.000 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,000 --> 00:00:07,000\nDeux.\n\n' > a.srt; subedit-cli transform --first 1=1.000 --last 9=10.000 --output b.srt a.srt; echo "code=$?"
a.srt: subtitle 9 is past the end: the file holds 2 subtitles
code=2
```

## Een positie vóór de oorsprong wordt geweigerd

Zoals bij [`shift`](shift.md), en om dezelfde reden: een negatieve positie
wordt geschreven als `-00:00:01,000`, een vorm die geen van de negen formaten definieert en die geen
lezer accepteert. Het geval doet zich hier voor wanneer een ondertitel **vóór
de eerste referentie** ligt en de schaal hem voorbij nul duwt. Er wordt niets
geschreven, de code is `2`, en de melding noemt de ondertitel die de fout veroorzaakt.

## Uitvoer

**Standaarduitvoer** — niets: het resultaat is het geschreven bestand. Met `--dry-run` ook niets: er is geen bestand, en de toelichtingsregel zegt dat — zie [Kijken voor het schrijven](invocation.md#kijken-voor-het-schrijven).

| Niveau | Wat er op de foutuitvoer wordt toegevoegd |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles transformed onto 1=00:00:01.000 and 3=00:00:10.000 -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — het formaat, de codering, de markering en de regeleinden van het gelezen bestand, teruggezet zoals ze waren |
| 3 | `<path>: N bytes read, M written`, daarna **elke diagnostische melding van het lezen** — zie [Aanroep](invocation.md#diagnostische-meldingen-lezen) |

De referenties worden met de decimale punt teruggeschreven, hoe ze ook zijn getypt.

### In JSON

Met [`--format json`](invocation.md#uitvoer-die-een-script-kan-lezen) bevat het object van elk bestand `counts.subtitles` (elke ondertitel van het bestand). De volledige tabel is die van [Aanroep](invocation.md#wat-elk-subcommando-in-counts-zet).

## Afsluitcodes

Die van het hulpprogramma: `0` als elk bestand is geschreven, `2` als geen enkel,
`3` als slechts enkele, `1` bij een gebruiksfout.

## Fouten

| Wat het veroorzaakt | Melding |
| :------------------ | :------ |
| referentie zonder `=` | `"…" is not a reference: write it <index>=<time>, as in 3=00:00:10.000` |
| referentie waarvan een helft ontbreekt | `"…" is missing one of its two halves: write it <index>=<time>, as in 3=00:00:10.000` |
| index `0` | `"0" is not a subtitle number: subtitles are counted from 1, as the file shows them` |
| index die geen geheel getal is | `"…" is not a subtitle number: expected a whole number, counted from 1 as the file shows them` |
| ongeldige tijd | die van [`shift`](shift.md) |
| `--first` en `--last` op dezelfde index | `--first and --last both name subtitle N, and two references on one subtitle define no transform` |
| index buiten de grenzen | `<path>: subtitle N is past the end: the file holds M subtitles` |
| twee ondertitels die samen beginnen | `<path>: subtitles N and M start at the same moment, so they define no transform` |
| positie vóór de oorsprong | `<path>: subtitle N would land before the origin, which no subtitle file can hold` |

**Zonder een van de drie bestemmingsopties wordt niets geschreven**, en hun
fouten zijn gemeenschappelijk voor de subcommando's die schrijven:
zie [Aanroep](invocation.md#de-bestemming).
