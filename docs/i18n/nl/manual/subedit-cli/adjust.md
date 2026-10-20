# `adjust`

<!-- machine-translated -->
> Deze pagina is een machinevertaling en is nog niet nagelezen door een lezer van de taal.

```
subedit-cli adjust [--speed CPS|off] [--shorten | --no-shorten] [--lengthen | --no-lengthen]
                   [--minimum TIME|off] [--maximum TIME|off] [--gap TIME|off]
                   [--range N-M|N-]
                   (--output FILE | --output-dir DIR | --in-place)
                   [--dry-run]
                   [--recursive]
                   <file>...
```

Past de **duur** van elke ondertitel aan een leessnelheid en aan limieten aan: een
ondertitel die te kort is om te worden gelezen, wordt verlengd, een ondertitel die het scherm
te laat verlaat, kan worden ingekort, en twee ondertitels overlappen elkaar niet. **Alleen het
einde verschuift**: het begin verplaatsen zou de ondertitel verplaatsen, en dat is het werk van
[`shift`](shift.md). Het is de bewerking `Adjust Durations…` van het
[venster](../subedit-gui/operations.md#adjust-durations), met dezelfde standaardwaarden.

## De vier beperkingen, en hun volgorde

Ze worden **in deze volgorde** toegepast, elk stelt het einde in, en de laatste wint:

1. **de leessnelheid** — de duur die de tekst nodig heeft, bij zoveel tekens per
   seconde (tags tellen niet mee, regeleinden wel);
2. **het minimum** — een duur die eronder ligt, wordt tot het minimum verhoogd;
3. **het maximum** — een duur die erboven ligt, wordt tot het maximum verlaagd;
4. **de tussenruimte** — de tijd die vóór de volgende ondertitel overblijft. Ze komt als laatste en
   **ze wint**: twee ondertitels die samen worden getoond, zijn een fout die de kijker ziet,
   een ondertitel die wat kort is niet.

De laatste ondertitel van het bestand heeft geen volgende, en dus geen tussenruimte om aan te houden. De volgende
van een ondertitel is die in het bestand, **ongeacht of ze** door `--range` **is gekozen**.

| Beperking | Optie | Standaard | Uitschakelen |
| :--------- | :----- | :----- | :------- |
| snelheid | `--speed CPS` | **15** tekens per seconde | `--speed off` |
| richting van de snelheid | `--shorten`, `--no-shorten`, `--lengthen`, `--no-lengthen` | ze **verlengt** alleen | — |
| minimum | `--minimum TIME` | **1.5 s** | `--minimum off` |
| maximum | `--maximum TIME` | **geen** | `--maximum off` |
| tussenruimte | `--gap TIME` | **0** | `--gap off` |

Dit zijn de instellingen van Gaupol en van de eerste keer dat het venster wordt geopend. **`off` schakelt een
beperking uit: ze wordt niet meer gecontroleerd.** Dat is niet hetzelfde als haar op nul zetten — een minimum van `0`
is een minimum van nul, een tussenruimte van `0` verbiedt elke overlap.

## Wat het instellingenbestand aanlevert

**De standaardwaarden hierboven zijn de ingebouwde.** Standaard leest `adjust` de bewerkingsinstellingen van
het instellingenbestand — de waarden die `Adjust Durations…` in het venster onthoudt, of die
[`config set`](config.md) heeft geschreven — en die nemen de plaats in van de ingebouwde standaardwaarden. **Een optie
die op de opdrachtregel is opgegeven, wint altijd**, daarna het bestand, daarna de ingebouwde standaardwaarde. Zie
[Het instellingenbestand](invocation.md#het-instellingenbestand) voor de plaats van het bestand, `--config` en
`--no-config`.

| Sleutel | Wat hij aanlevert | Optie die hem overschrijft |
| :-- | :--------------- | :----------------------- |
| `duration-adjust.speed` | de leessnelheid, in tekens per seconde | `--speed CPS` |
| `duration-adjust.lengthen`, `duration-adjust.shorten` | in welke richting de snelheid een einde mag verschuiven; beide `false` schakelen de snelheid uit | `--lengthen`, `--no-lengthen`, `--shorten`, `--no-shorten`, `--speed off`; `--speed CPS` of een richting opgeven schakelt een snelheid die het bestand had uitgezet weer in, verlengend |
| `duration-adjust.minimum-enabled`, `duration-adjust.minimum-ms` | de kortste duur, of geen | `--minimum TIME`, `--minimum off` |
| `duration-adjust.maximum-enabled`, `duration-adjust.maximum-ms` | de langste duur, of geen | `--maximum TIME`, `--maximum off` |
| `duration-adjust.gap-enabled`, `duration-adjust.gap-ms` | de kleinste tussenruimte, of geen | `--gap TIME`, `--gap off` |

**Elke sleutel heeft een optie die hem ongedaan maakt.** Elke richting heeft een paar vlaggen die haar instellen,
wat het bestand ook zegt: `--shorten` en `--no-shorten` voor inkorten, `--lengthen` en
`--no-lengthen` voor verlengen; een vlag die niet is opgegeven, laat de richting van het bestand zoals ze is.
`--maximum off` laat een maximum vallen dat het bestand inschakelt. Met `--format json` meldt `constraints`
de instellingen die echt zijn gebruikt, waar ze ook vandaan komen, en `settings` het bestand dat is gelezen:

<!-- example: printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine/settings.conf config set duration-adjust.speed 12 2>/dev/null; subedit-cli --config mine/settings.conf --format json adjust --dry-run a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine/settings.conf config set duration-adjust.speed 12 2>/dev/null; subedit-cli --config mine/settings.conf --format json adjust --dry-run a.srt
settings: read mine/settings.conf
a.srt: adjusted the durations of 1 subtitle (dry run, nothing written)
{"schema":1,"command":"adjust","file":"a.srt","ok":true,"dry_run":true,"destination":null,"counts":{"subtitles":1,"adjusted":1,"sacrificed":{"speed":0,"minimum":0,"gap":0}},"constraints":{"speed":{"cps":"12","lengthen":true,"shorten":false},"minimum_ms":1500,"maximum_ms":null,"gap_ms":0},"settings":"mine/settings.conf","warnings":[]}
```

De andere voorbeelden op deze pagina worden uitgevoerd met `--no-config`, zodat ze laten zien wat de ingebouwde
standaardwaarden doen, wat de instellingen van de machine die ze uitvoert ook zijn.

<!-- example: subedit-cli adjust --help -->
```console
$ subedit-cli adjust --help
Bring the duration of every subtitle within a reading speed and limits
Usage: subedit-cli adjust [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to adjust

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --speed CPS|off             Reading speed in characters per second, or off (default 15)
  --shorten                   Let the reading speed bring an end earlier
  --no-shorten                Do not let the reading speed bring an end earlier
  --lengthen                  Let the reading speed move an end later
  --no-lengthen               Do not let the reading speed move an end later
  --minimum TIME|off          Shortest duration, or off (default 1.5)
  --maximum TIME|off          Longest duration, or off (default: none)
  --gap TIME|off              Least time left before the next subtitle, or off (default 0)
  --range N-M|N-              Act only on subtitles N to M, or N to the end
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
| `--speed` | nee | een aantal tekens per seconde, **boven nul** (`15`, `12.5`), of `off` | `15` |
| `--shorten` | nee | een vlag: de snelheid mag een einde ook **eerder brengen** | uit |
| `--no-shorten` | nee | een vlag: de snelheid brengt een einde niet meer **eerder**, wat het instellingenbestand ook zegt | uit |
| `--lengthen` | nee | een vlag: de snelheid **verschuift** een einde **later**, wat het instellingenbestand ook zegt | uit |
| `--no-lengthen` | nee | een vlag: de snelheid **verschuift** een einde niet meer **later** | uit |
| `--minimum` | nee | een duur van nul of meer, of `off` | `1.5` |
| `--maximum` | nee | een duur **boven nul**, of `off` | geen |
| `--gap` | nee | een duur van nul of meer, of `off` | `0` |
| `--range` | nee | `N-M`, of `N-` tot het einde — zie hieronder | het hele bestand |
| `--output` / `--output-dir` / `--in-place` | **een van de drie**, behalve met `--dry-run` | zie [Aanroep](invocation.md#de-bestemming) | — |
| `--dry-run` | nee | een vlag: berekent en meldt, schrijft niets — zie [Kijken voor het schrijven](invocation.md#kijken-voor-het-schrijven) | uit |

Een duur wordt geschreven zoals bij [`shift`](shift.md#een-duur-schrijven): `2.5`, `00:00:02.500`.

**Zonder enige actieve beperking wordt `adjust` geweigerd** (code `1`, niets geschreven): er zou
niets zijn om aan aan te passen, en een commando dat niets doet terwijl het meldt dat het is geslaagd, is het slechtste
antwoord. Dat is het geval bij `--speed off --minimum off --gap off` zonder maximum, en bij
`--no-lengthen` alleen, met het minimum en de tussenruimte uitgeschakeld: een snelheid die geen enkel
einde verschuift, beperkt niets.

`--shorten`, `--no-shorten`, `--lengthen` en `--no-lengthen` geven aan in welke richting de snelheid een
einde verschuift; ze hebben alleen zin met een snelheid, en **`--speed off` weigert ze**. Een vlag die samen met zijn
tegengestelde wordt opgegeven — `--shorten --no-shorten`, `--lengthen --no-lengthen` — wordt ook geweigerd.

### `--range`

`--range 3-7` beperkt de aanpassing tot ondertitels 3 tot en met 7, **geteld vanaf 1, grenzen
inbegrepen**; `--range 3-` loopt door tot de laatste. Een enkele ondertitel wordt geschreven als `4-4`.

- Een bereik **dat eindigt voor het begint**, een `0`, een teken of een los getal worden
  geweigerd als gebruiksfout (code `1`).
- **Het geldt voor elk bestand van de reeks**, wat alleen zin heeft als de bestanden
  op elkaar lijken. Het wordt **per bestand** beoordeeld: een bereik dat voorbij dat bestand gaat, wordt
  geweigerd **voordat het bestand wordt aangeraakt**, met vermelding van de grens —
  `range 3-9 ends after the last subtitle: the file has 4` —, dat bestand mislukt (code `2` of `3`, identificatiecode
  `range-out-of-bounds`) en de andere gaan door.
- De tellingen van het rapport hebben alleen betrekking op de gekozen ondertitels.

## Wat wordt opgeofferd

Twee beperkingen kunnen elkaar bij dezelfde ondertitel tegenspreken: een minimum van 1.5 s past niet
in de 0.8 s die de volgende ondertitel overlaat. **De volgorde beslist, en het rapport meldt
wat moest wijken** — wat Gaupol niet doet: het breekt ze stilzwijgend.

Een beperking wordt alleen genoemd als **ten minste één ondertitel haar niet heeft gekregen**, nadat
de andere zijn toegepast. Het maximum heeft geen telling: het wordt vóór de tussenruimte toegepast, die alleen
kan inkorten, dus het geldt altijd. Een minimum boven het maximum wijkt voor het maximum en wordt
als minimum geteld.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --output ajuste.srt courts.srt; grep -e '-->' ajuste.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --output ajuste.srt courts.srt; grep -e '-->' ajuste.srt
courts.srt: adjusted the durations of 3 subtitles; could not satisfy the minimum duration in 1 subtitle -> ajuste.srt
00:00:00,000 --> 00:00:02,000
00:00:10,000 --> 00:00:14,000
00:00:14,200 --> 00:00:15,000
00:00:15,000 --> 00:00:16,500
```

De derde ondertitel zou graag om 15.7 s eindigen om 1.5 s te duren, maar de vierde begint om
15.0 s: **de tussenruimte wint**, ze eindigt om 15.0 s, en het rapport noemt het minimum.

Als u de tussenruimte uitschakelt, wint het minimum — en overlappen de twee ondertitels elkaar:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --gap off --output libre.srt courts.srt; grep -e '-->' libre.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --gap off --output libre.srt courts.srt; grep -e '-->' libre.srt
courts.srt: adjusted the durations of 3 subtitles -> libre.srt
00:00:00,000 --> 00:00:02,000
00:00:10,000 --> 00:00:14,000
00:00:14,200 --> 00:00:15,700
00:00:15,000 --> 00:00:16,500
```

En er wordt niets opgeofferd noch aangepast als het bestand al binnen zijn limieten past:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --speed off --minimum off --output intact.srt courts.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --speed off --minimum off --output intact.srt courts.srt
courts.srt: no duration to adjust -> intact.srt
```

Het bestand wordt geschreven zoals het is: **een opgegeven bestemming is een geschreven bestemming**.

## Uitvoer

**Standaarduitvoer** — niets: het resultaat is het geschreven bestand.

| Niveau | Wat er op de foutuitvoer wordt toegevoegd |
| :----- | :------------------------------------- |
| 1 | `<path>: adjusted the durations of N subtitles; could not satisfy …  -> <destination>`, of `<path>: no duration to adjust -> <destination>` — en, eerst, het gelezen instellingenbestand — `settings: read <path>` — zie [Het instellingenbestand](invocation.md#het-instellingenbestand) |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — het formaat, de codering, de markering en de regeleinden van het gelezen bestand, teruggezet zoals ze waren |
| 3 | `<path>: N bytes read, M written`, daarna **elke leesmelding** — zie [Aanroep](invocation.md#diagnostische-meldingen-lezen) |

De zin is die van het venster: `N` is het aantal ondertitels waarvan het einde is verschoven, en de
rest, afwezig als niets moest wijken, somt op wat niet is gehaald — `the reading speed`,
`the minimum duration`, `the gap` — met het aantal betrokken ondertitels.

### In JSON

Met [`--format json`](invocation.md#uitvoer-die-een-script-kan-lezen) bevat het object van elk bestand,
naast de envelop:

| Sleutel | Inhoud |
| :-- | :------ |
| `counts.subtitles` | de ondertitels waar het **om gaat**: het hele bestand, of het bereik |
| `counts.adjusted` | die waarvan het einde is verschoven |
| `counts.sacrificed.speed`, `.minimum`, `.gap` | de ondertitels die deze beperking nog steeds niet halen, nadat de andere zijn toegepast, **inclusief die waarvan het einde niet is verschoven** |
| `constraints.speed` | `{"cps", "lengthen", "shorten"}` — de tekens per seconde **als tekenreeks** (`"15"`, `"12.5"`) — of `null` als uitgeschakeld |
| `constraints.minimum_ms`, `.maximum_ms`, `.gap_ms` | de gebruikte duren, in **hele milliseconden**, of `null` als uitgeschakeld |
| `settings` | het instellingenbestand dat is gelezen, of `null` als er geen was |

`constraints` meldt **de instellingen die echt zijn gebruikt**, standaardwaarden inbegrepen: twee berekeningen
over dezelfde ondertitels zijn dan te vergelijken zonder de opdrachtregel te lezen.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config --format json adjust --dry-run --maximum 1.8 courts.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config --format json adjust --dry-run --maximum 1.8 courts.srt
courts.srt: adjusted the durations of 4 subtitles; could not satisfy the reading speed in 1 subtitle, the minimum duration in 1 subtitle (dry run, nothing written)
{"schema":1,"command":"adjust","file":"courts.srt","ok":true,"dry_run":true,"destination":null,"counts":{"subtitles":4,"adjusted":4,"sacrificed":{"speed":1,"minimum":1,"gap":0}},"constraints":{"speed":{"cps":"15","lengthen":true,"shorten":false},"minimum_ms":1500,"maximum_ms":1800,"gap_ms":0},"settings":null,"warnings":[]}
```

## Afsluitcodes

Die van het programma: `0` als elk bestand is verwerkt, `2` als geen enkel, `3` als slechts
sommige, `1` bij een gebruiksfout. Een bestand dat al binnen zijn limieten past, wordt verwerkt.

## Fouten

| Wat het veroorzaakt | Melding |
| :------------------ | :------ |
| een bestand dat door `--config` wordt genoemd en niet bestaat — het eigen bestand van de gebruiker, ontbrekend, geeft de standaardwaarden | `settings file not found: <FILE>` |
| geen actieve beperking | `adjust has nothing to adjust to: every constraint is off, or the reading speed is told to move no end` |
| een snelheid die er geen is | `--speed: "…" is not a reading speed: expected a number of characters per second above zero, like 15 or 12.5, or off` |
| een richtingsvlag met `--speed off` | `--lengthen, --no-lengthen, --shorten and --no-shorten say which way the reading speed may move an end, and --speed off switches the reading speed off` |
| een richtingsvlag met zijn tegengestelde | `--shorten and --no-shorten say opposite things; give one of them` (evenzo `--lengthen` en `--no-lengthen`) |
| een duur die er geen is | `--minimum: "…" is not a time: …` (evenzo `--maximum`, `--gap`) |
| negatieve duur | `--minimum: "-1" is not a duration of zero or more` (evenzo `--gap`) |
| nul of negatief maximum | `--maximum: "0" is not a duration above zero` |
| slecht geschreven bereik | `--range: "…" is not a range: write it N-M, or N- to go to the end, counted from 1 and inclusive` |
| bereik dat eindigt voor het begint | `--range: "5-3" is not a range: it ends before it starts` |
| bereik buiten het bestand | `<path>: range N-M starts after the last subtitle: the file has K`, of `… ends after …` |

De fouten van de bestemming zijn gemeenschappelijk voor de subcommando's die schrijven:
zie [Aanroep](invocation.md#de-bestemming).
