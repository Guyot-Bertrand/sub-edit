# Een mappenstructuur verwerken

<!-- machine-translated -->
> Deze pagina is een machinevertaling en is nog niet nagelezen door een lezer van de taal.

```
subedit-cli <subcommand> --recursive [options] <directory>...
```

Elk subcommando dat een reeks bestanden leest — `inspect` inbegrepen, en niet
[`append`](append.md), [`split-file`](split-file.md) en [`pair`](pair.md), waarvan de aantallen die van een paar of
van een film zijn — accepteert een **map** in plaats van een bestand, met `--recursive` (`-r`).
Deze pagina zegt wat de doorloop meeneemt, in welke volgorde en waar hij schrijft.

**Niets van wat volgt verandert wat er met een bestand gebeurt**: elk bestand wordt
afzonderlijk gelezen, verwerkt en geschreven, de mislukking van het ene stopt de andere niet, en de
vier afsluitcodes zijn die van [de aanroep](invocation.md#afsluitcodes).
De doorloop levert een lijst paden op, die de rest van de reeks verwerkt alsof ze
waren getypt.

## Zonder `--recursive` wordt een map geweigerd

Gebruiksfout, code `1`, geen enkel bestand verwerkt of geschreven:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?" -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?"
films: is a directory: use --recursive
code=1
```

## Wat de doorloop meeneemt

De doorloop is **recursief, deterministisch en zonder verrassingen**:

- **de volgorde** — de items van elke map worden genomen in de volgorde van hun
  namen, **byte voor byte** (hoofdletters vóór kleine letters, ongeacht de
  taal van de machine), en een submap wordt op zijn plaats doorlopen, eerst
  de diepte in. Twee runs geven dezelfde bytes, op elke
  machine;
- **symbolische koppelingen worden niet gevolgd** — niet naar een bestand en niet naar een
  map: een boomstructuur die een koppeling naar een van haar bovenliggende mappen bevat, zou geen
  einde hebben;
- **verborgen bestanden en mappen worden genegeerd** — die waarvan de naam met
  een punt begint;
- **alleen de extensies van de bekende formaten blijven over**: `.srt`, `.vtt`,
  `.ssa`, `.ass`, `.lrc`, `.sub`, ongeacht hoofdletters. **`.txt` is uitgesloten**: het
  noemt twee formaten, maar vooral elk `LISEZMOI.txt` ter wereld, en een reeks die
  op elk ervan mislukte, zou onbruikbaar zijn.

**Een bestand dat op de opdrachtregel is genoemd, wordt nooit gefilterd.** Het noemen
is het willen: `subedit-cli shift --by 1 --in-place notes.txt` verwerkt het, wat
zijn extensie ook is, en klaagt alleen als de inhoud in geen enkel formaat is.

**Wat terzijde is gelaten, wordt geteld** en op niveau `-vv` gemeld:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli -vv inspect --recursive films 2>&1 | head -1 -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli -vv inspect --recursive films 2>&1 | head -1
films: 1 file left aside, with no extension of a known format
```

Een map die niets bevat om te doorlopen, zegt dat op het standaardniveau
(`<directory>: holds no file in a format this tool walks for`) en dat is geen fout.

## Waar de doorloop schrijft

**De boomstructuur blijft behouden onder `--output-dir`**: het pad van een bestand,
**relatief ten opzichte van de opgegeven map**, wordt onder de uitvoermap nagebootst. De naam
van de opgegeven map maakt er geen deel van uit — dit is de semantiek van `rsync` met een
slash aan het eind, niet die van `cp -r`: men noemt de bestemming en voegt er het
niveau aan toe dat men wil (`--output-dir sortie/films`).

<!-- example: mkdir -p films/a/b; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/x.srt; cp films/x.srt films/a/x.srt; cp films/x.srt films/a/b/y.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli shift --by 1 --recursive --output-dir sortie films; find sortie -type f | sort -->
```console
$ mkdir -p films/a/b; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/x.srt; cp films/x.srt films/a/x.srt; cp films/x.srt films/a/b/y.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli shift --by 1 --recursive --output-dir sortie films; find sortie -type f | sort
films/a/b/y.srt: 1 subtitle shifted by 1.000 s -> sortie/a/b/y.srt
films/a/x.srt: 1 subtitle shifted by 1.000 s -> sortie/a/x.srt
films/x.srt: 1 subtitle shifted by 1.000 s -> sortie/x.srt
3 of 3 files shifted
sortie/a/b/y.srt
sortie/a/x.srt
sortie/x.srt
```

De uitvoermap en die van de boomstructuur worden **aangemaakt** als ze ontbreken. Met
`--in-place` wordt elk bewaard bestand op zijn plaats teruggeschreven. Een extensie die
`convert` wijzigt, wijzigt ook in de boomstructuur: `films/a/x.srt` geconverteerd
naar WebVTT wordt geschreven als `sortie/a/x.vtt`.

**De uitvoermap wordt, als ze zich in de doorlopen boomstructuur bevindt, van de doorloop uitgesloten**:
een tweede run leest niet opnieuw wat de eerste heeft geschreven.

## Twee mappen en botsingen

De regel van [de bestemming](invocation.md#de-bestemming) beslist: twee bestanden
die op hetzelfde pad zouden uitkomen, worden geweigerd **voordat er iets wordt geschreven**, code `1`, met de
twee invoerbestanden en de bestemming genoemd.

<!-- example: mkdir -p un/a deux/a; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > un/a/x.srt; cp un/a/x.srt deux/a/x.srt; subedit-cli shift --by 1 -r --output-dir sortie un deux; echo "code=$?" -->
```console
$ mkdir -p un/a deux/a; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > un/a/x.srt; cp un/a/x.srt deux/a/x.srt; subedit-cli shift --by 1 -r --output-dir sortie un deux; echo "code=$?"
sortie/a/x.srt: would be written by both un/a/x.srt and deux/a/x.srt
code=1
```

## Wat het kost

De reeks blijft sequentieel: de duur ervan is de som van die van de bestanden. De
benchmark `make bench` meet de doorloop en hetzelfde werk in één bestand; de cijfers
staan in het [prestatielogboek](../../../../mesures/performances.md).
