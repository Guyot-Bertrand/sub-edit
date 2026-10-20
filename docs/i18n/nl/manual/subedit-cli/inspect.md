# `inspect`

<!-- machine-translated -->
> Deze pagina is een machinevertaling en is nog niet nagelezen door een lezer van de taal.

```
subedit-cli inspect [--frame-rate RATE] [--recursive] [--detail]
                   [-t FILE] [--align-method position|number] <file>...
```

Meldt waaruit elk bestand bestaat. **Wijzigt niets en schrijft geen
bestand.**

<!-- example: subedit-cli inspect --help -->
```console
$ subedit-cli inspect --help
Report what a subtitle file is made of
Usage: subedit-cli inspect [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to report on

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --detail                    List every anomaly, subtitle by subtitle, instead of counting them
  --frame-rate RATE           Frame rate of a file counted in frames: 25, 23.976
  -t,--translation-file FILE  Translation file to lay over the subtitle file, for a single input
  --align-method position|number
                              How the lines of the translation find their subtitles
```

## Argumenten

| Argument | Verplicht | Waarde | Standaard |
| :------- | :----- | :----- | :----- |
| `<file>...` | ja | een of meer paden van ondertitelbestanden | — |
| `--recursive`, `-r` | nee | een vlag | uit — zie [Een mappenstructuur verwerken](lots.md) |
| `--detail` | nee | een vlag: noemt elke afwijking, in plaats van ze te tellen — zie [Wat `anomalies` meldt](#wat-anomalies-meldt) | uit |

Geen pad is een gebruiksfout, dus code `1`.

## Opties

| Optie | Verplicht | Waarde | Standaard |
| :----- | :------ | :----- | :----- |
| `--frame-rate` | nee | een snelheid in frames per seconde: `25`, `23.976`, `29.97` | 23.976 |

**Ze dient alleen voor bestanden die in frames worden geteld**, dat wil zeggen MicroDVD, het
enige van de negen. Een MicroDVD-bestand bevat alleen framenummers; de
snelheid waarmee ze worden gelezen, bepaalt **elke positie van het rapport**,
te beginnen met `span`. Elders opgegeven verandert ze niets en is ze geen
fout.

Een waarde die niets aanduidt, is een gebruiksfout, dus code `1`:

<!-- example: subedit-cli inspect --frame-rate douze film.sub; echo $? -->
```console
$ subedit-cli inspect --frame-rate douze film.sub; echo $?
--frame-rate: "douze" is not a frame rate: expected frames per second, like 25 or 23.976
1
```

Het is dezelfde optie als bij [`convert`](convert.md), woord voor woord, omdat
het dezelfde snelheid is. `convert` gebruikt haar ook om te **schrijven**.

## Een vertaling: `-t`

| Optie | Verplicht | Waarde | Standaard |
| :----- | :------ | :----- | :----- |
| `-t`, `--translation-file` | nee | een vertaalbestand, voor **één enkele** invoer | — |
| `--align-method` | nee, en alleen met `-t` | `position` of `number` | `position` |

Met `-t` wordt de vertaling over het bestand gelegd **zoals het venster doet bij het openen** — zie
[Een vertaling](invocation.md#een-vertaling) — en het rapport meldt **hoe haar regels hun
ondertitel hebben gevonden**. Een script weet zo of een vertaling is uitgelijnd voordat het haar corrigeert.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour.\n\n' > film.fr.srt; subedit-cli --quiet inspect -t film.fr.srt film.srt | tail -3 -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour.\n\n' > film.fr.srt; subedit-cli --quiet inspect -t film.fr.srt film.srt | tail -3
  anomalies: none
  translation file: film.fr.srt, matched by position
  translation: 1 line attached; 1 subtitle left without a translation
```

De eerste toegevoegde regel noemt het bestand en de methode; de tweede is **de zin die het venster
zegt** wanneer het een vertaling opent, elk deel alleen wanneer het niet nul is:

| Deel | Wat het telt |
| :----- | :---------------- |
| `N lines attached` | de regels van de vertaling die een ondertitel van het hoofdbestand hebben gevonden |
| `N subtitles born of a line` | de regels die er geen hebben gevonden en hun eigen ondertitel hebben voortgebracht, op hun eigen posities |
| `N subtitles left without a translation` | de ondertitels van het hoofdbestand die geen enkele regel heeft bereikt |
| `N lines out of order` | de regels die in het bestand na een latere regel kwamen |

Een vertaling die geen enkele regel bevat, zegt `translation: the file holds no line`. **Het rapport van het
bestand zelf blijft dat van het bestand alleen**: een ondertitel die uit een regel is voortgekomen, wordt niet meegeteld in
`subtitles`.

De fouten van `-t` zijn die van een leesbewerking, gemeld met het pad van de vertaling, en deze:
`<translation>: the file is already open as the main document`, wanneer het opgegeven bestand het hoofdbestand is.
`--align-method` zonder `-t` is een gebruiksfout, en `-t` met meerdere invoeren ook — zie
[Een vertaling](invocation.md#een-vertaling).

## Uitvoer

Het rapport gaat naar de **standaarduitvoer** — het is het resultaat van het commando, en
het wordt op elk niveau geschreven, `--quiet` inbegrepen. Eén blok per bestand, voorafgegaan
door het pad:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,500\nLe canot dérive.\n\n2\n00:00:04,000 --> 00:00:06,200\nPersonne n a vu la côte.\n' > exemple.srt; subedit-cli --quiet inspect exemple.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,500\nLe canot dérive.\n\n2\n00:00:04,000 --> 00:00:06,200\nPersonne n a vu la côte.\n' > exemple.srt; subedit-cli --quiet inspect exemple.srt
exemple.srt
  format: SubRip
  encoding: UTF-8, detected
  byte order mark: absent
  line endings: LF
  subtitles: 2
  span: 00:00:01.000 -> 00:00:06.200
  frame rate grid: none (too few subtitles to tell)
  anomalies: none
```

| Veld | Wat het zegt |
| :---- | :----------- |
| `format` | een van de formaten die `--to` accepteert, voluit geschreven: `SubRip`, `WebVTT`, `SubViewer 2`, `Sub Station Alpha`, `Advanced SSA`, `MicroDVD`, `MPL2`, `TMPlayer`, `LRC` |
| `encoding` | de gelezen codering, gevolgd door **waar het antwoord vandaan komt**: `, from its byte order mark` als het bestand haar declareert, `, as asked for` als `--encoding` haar oplegde, `, detected` als ze is afgewogen |
| `byte order mark` | `present` of `absent` |
| `line endings` | `LF`, `CRLF` of `CR`, gevolgd door `, mixed from line N` als het bestand ze door elkaar gebruikt |
| `subtitles` | het aantal gelezen ondertitels |
| `span` | van het vroegste begin tot het laatste einde, `HH:MM:SS.mmm` |
| `frame rate grid` | de framerate waarop de posities zijn berekend, **afgeleid** — zie hieronder |
| `frame rate` | voor een bestand dat in frames wordt geteld, de snelheid waarmee het is gelezen en **waar die vandaan komt** — vervangt de vorige regel, zie hieronder |
| `anomalies` | `none` op dezelfde regel, of wat er mis is: **één soort per regel met zijn telling**, ingesprongen onder `anomalies:`; met `--detail`, **één afwijking per regel**, ondertitel voor ondertitel |

**`span` is niet "van de eerste tot de laatste"** maar van de vroegste tot de laatste:
bij een bestand waarvan de volgorde is verstoord, verschillen de twee, en alleen de tweede zegt
de waarheid over wat het bestand beslaat.

**`anomalies` telt, met `--detail`, de ondertitels vanaf 1**, zoals het bestand ze
nummert — en niet de regels van het bestand. Een ondertitelnummer overleeft een
wijziging; een regelnummer niet. Zie hieronder.

## Wat `frame rate grid` meldt

**Een ondertitelbestand declareert zijn framerate niet.** SubRip heeft geen
header, die van WebVTT is vrije tekst. Deze regel leest dus niets:
ze **leidt af**, door te meten of de posities op een framerooster vallen.

Gemeten worden de **begintijden**. Waar een raster bestaat, liggen ze er
zonder uitzondering op, terwijl de eindtijden er vaak van afwijken — een einde wordt
regelmatig berekend met een leessnelheidsregel in plaats van op
een frame te worden gezet.

De eerste regel zegt altijd hetzelfde: de gekozen snelheid, het oordeel,
en de concentratie die het ondersteunt.

| Oordeel | Wat het betekent |
| :------ | :----------------- |
| `clean` | de posities liggen op een raster |
| `partial` | een deel ervan wel, de rest niet |
| `none` | geen kandidaat verklaart iets |

`none` heeft **twee oorzaken, en het zijn niet dezelfde antwoorden**: ofwel past geen
enkel raster — de regel geeft dan de beste kandidaat en zijn score,
zodat men ziet hoeveel het mislukt — ofwel telt het bestand te weinig
ondertitels om er iets over te zeggen. Twee begintijden lijken altijd
perfect uitgelijnd; dat bewijst niets.

**Een regelmatig bestand met een niet-standaardsnelheid geeft `none`, en dat is
bedoeld.** De verzameling kandidaten is gesloten — de acht snelheden die de
conversie aanbiedt — en de minst foute ervan melden zou een
fout antwoord geven waar "ik weet het niet" het juiste is.

### De regels die volgen, wanneer ze iets te zeggen hebben

| Regel | Wanneer ze verschijnt |
| :---- | :------------------ |
| `grid offset` | de posities liggen op het raster **op een constante na**: het bestand is verschoven, en hier staat hoeveel |
| `also fits` | een andere kandidaat past even goed, omdat ze een geheel veelvoud van de gekozen kandidaat is |
| `too short a span to separate` | de spanwijdte van het bestand laat niet toe de gekozen snelheid te onderscheiden van de genoemde |
| `off the grid` | hoeveel begintijden van het raster afwijken, en in hoeveel **reeksen** |

**`also fits` is geen aarzeling.** Een raster van 25 frames per seconde is
*opgenomen* in een raster van 50: een bestand dat op 25 is getimed, past dus op beide.
Het omgekeerde is onwaar — een echte 50 valt op 25 uiteen — dus de laagste
wordt gekozen, en de andere wordt genoemd in plaats van verzwegen. De enige
paren die erbij horen zijn 25 en 50, 29.97 en 59.94, 30 en 60.

**`too short a span to separate` beschermt tegen een te zelfverzekerd antwoord.** 23.976
en 24 lopen één milliseconde per seconde film uiteen: bij een speelfilm
zijn ze zonder twijfel te onderscheiden, over tien seconden niet. De regel zegt
welke nog mogelijk blijft.

**`off the grid` onderscheidt twee verhalen die het percentage alleen verwart.**
Veel reeksen van één enkele begintijd zijn posities die met de hand zijn gecorrigeerd,
een voor een. Enkele lange reeksen zijn een opnieuw getimed gedeelte of een bestand
dat uit twee andere is samengesteld. Het aantal reeksen onderscheidt ze.

### Een voorbeeld

Twaalf regels op een raster van 25 frames per seconde gezet — dus op
posities die veelvouden van 40 milliseconden zijn — verspreid over bijna negen minuten:

<!-- example: k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > grille.srt; subedit-cli --quiet inspect grille.srt | tail -3 -->
```console
$ k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > grille.srt; subedit-cli --quiet inspect grille.srt | tail -3
  frame rate grid: 25 fps, clean (100.0%)
  also fits: 50 fps, of which this rate is a whole divisor
  anomalies: none
```

Het voorbeeld maakt zijn eigen bestand in plaats van er een uit de repository te lezen, en dat is
bewust: het kan zoals het is worden herhaald, en de lus toont wat een raster is
beter dan een zin — posities die veelvouden van een framelengte zijn.

## Wat `frame rate` meldt, en waarom het geen raster is

**Slechts één van de negen formaten telt in frames**: MicroDVD, waarvan elke regel
twee framenummers bevat en niets anders. Bij zo'n bestand schrijft `inspect`
niet `frame rate grid` maar `frame rate`, en de twee regels worden
nooit samen getoond.

**De reden past in één zin.** De posities van een MicroDVD-bestand zijn
berekend *uit* zijn frames, met een gegeven snelheid; ze afleiden uit een
raster zou diezelfde snelheid teruggeven, zoals ze zojuist is opgegeven. Het zou
een gegeven cijfer zijn, vermomd als meting.

Wat in plaats daarvan wordt geschreven, is **de leessnelheid en waar die vandaan komt** —
dezelfde vorm als de regel `encoding`:

| Wat de regel zegt | Wanneer |
| :------------------ | :---- |
| `frame rate: 25 fps, as asked for` | `--frame-rate` legde haar op |
| `frame rate: 24000/1001 fps, assumed` | niemand zei het; het is de standaardwaarde |

**Twee antwoorden en geen drie**, waar `encoding` er drie heeft: een MicroDVD-bestand
declareert niets. Dat is het hele probleem — de snelheid komt noodzakelijkerwijs
van buiten, en het rapport zegt van welke van de twee buitenkanten.

**De snelheid bepaalt de rest van het rapport.** `span` loopt erdoorheen, en twee
lezingen van hetzelfde bestand met twee snelheden geven twee spanwijdtes:

<!-- example: printf '{25}{75}Première.\n{100}{150}Deuxième.\n{200}{260}Troisième.\n' > images.sub; subedit-cli --quiet inspect images.sub | tail -3; subedit-cli --quiet inspect --frame-rate 25 images.sub | tail -3 -->
```console
$ printf '{25}{75}Première.\n{100}{150}Deuxième.\n{200}{260}Troisième.\n' > images.sub; subedit-cli --quiet inspect images.sub | tail -3; subedit-cli --quiet inspect --frame-rate 25 images.sub | tail -3
  span: 00:00:01.043 -> 00:00:10.844
  frame rate: 24000/1001 fps, assumed
  anomalies: none
  span: 00:00:01.000 -> 00:00:10.400
  frame rate: 25 fps, as asked for
  anomalies: none
```

## Wat `anomalies` meldt

Drie dingen kunnen er mis zijn in een document, en elk wordt anders hersteld.
**Standaard telt `inspect` ze**, een regel per soort, in de volgorde van de tabel:
een slecht gemaakt bestand bevat er honderden, en weten hoeveel van elke soort er zijn
zegt wat er moet worden uitgevoerd. **`--detail` noemt ze een voor een.**

| Wat er wordt geschreven | Met `--detail` | Wat het betekent |
| :--------------- | :-------------- | :------------------ |
| `subtitle ends before it starts: N` | `subtitle N ends before it starts` | het einde komt vóór het begin |
| `subtitle starts before the previous one ends: N` | `subtitle N starts before the previous one ends` | ze overlapt de vorige |
| `subtitle starts before the previous one starts: N` | `subtitle N starts before the previous one starts` | ze verstoort de volgorde van het bestand — [`sort`](sort.md) zet haar terug |

Een soort die ontbreekt, wordt niet geschreven. In de linkerkolom is `N` het aantal
ondertitels; in de volgende het nummer van een ondertitel.

**Dezelfde ondertitel kan twee keer voorkomen**, en dat is bedoeld: degene die
begint voordat de vorige is begonnen, begint ook voordat de vorige is
geëindigd. Het eerste punt wordt opgelost door de ondertitel te verplaatsen, het tweede door
een duur te wijzigen.

**De genoemde ondertitel is degene die verkeerd staat**, niet degene waartegenover hij
verkeerd staat. Bij begintijden `0`, `4 s`, `2 s`, `3 s`: de derde verstoort de volgorde, de
vierde volgt wel op de derde en wordt dus niet genoemd — daar is niets aan
te doen.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect desordre.srt | tail -3 -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect desordre.srt | tail -3
  anomalies:
    subtitle starts before the previous one ends: 1
    subtitle starts before the previous one starts: 1
```

En met `--detail` de genoemde ondertitel:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect --detail desordre.srt | tail -3 -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect --detail desordre.srt | tail -3
  anomalies:
    subtitle 3 starts before the previous one ends
    subtitle 3 starts before the previous one starts
```

**Een regel per afwijking met `--detail`**, omdat honderd bevindingen
op één regel noch te lezen noch te doorzoeken zijn. Een document zonder fout houdt
zijn ene regel, `anomalies: none`, waarnaar een script zoekt. **Het
record van `--format json` somt elke afwijking altijd op**: een script leest het detail, en
`--detail` verandert alleen de tekst.

**Het is geen diagnostische melding bij het lezen.** De diagnostische meldingen zeggen waar het
lezen op stuitte en wijzen naar een **regel van het bestand**; de afwijkingen zeggen
wat het document *is* en wijzen naar een **ondertitel**. Een bestand waarvan de volgorde
is verstoord, is daarom nog niet misvormd: het lezen heeft er niets over te zeggen.

## In JSON

Met [`--format json`](invocation.md#uitvoer-die-een-script-kan-lezen) schrijft `inspect` **één
object per bestand** in plaats van de bovenstaande blokken; de toelichting blijft op de foutuitvoer.
De sleutels beschrijven wat de tekst zegt, en **geen enkele hoeft uit een zin te worden teruggelezen**:

| Sleutel | Inhoud |
| :-- | :------ |
| `format` | de identifier van het formaat, degene die `convert --to` accepteert: `srt`, `vtt`… |
| `encoding` | `{"charset", "origin", "byte_order_mark"}` — `origin` is `byte-order-mark`, `asked` of `detected` |
| `line_endings` | `{"kind", "mixed_from_line"}` — `lf`, `crlf` of `cr`; de regel waar een tweede soort verschijnt, of `null` |
| `subtitles` | het aantal ondertitels |
| `span_ms` | `{"start", "end"}` — het eerste begin en het laatste einde, in milliseconden |
| `frame_rate` | voor een bestand dat in frames wordt geteld: `{"rate", "origin"}` (`asked` of `assumed`); anders `null` |
| `grid` | voor de andere: het afgeleide raster, of `null` voor een bestand dat in frames wordt geteld |
| `anomalies` | een array van `{"subtitle", "kind"}` — `end-before-start`, `overlapping-subtitles`, `out-of-order` |
| `translation` | **alleen met `-t`**: `{"file", "method", "attached", "born", "untranslated", "out_of_order"}` — `method` is `position` of `number`, en de vier tellingen zijn die van de bovenstaande zin |
| `warnings` | waar het lezen op stuitte, op elk niveau |

`grid` bevat `verdict` (`clean`, `partial` of `silent`), `enough_starts`, `rate` (een string,
of `null` wanneer niets wordt genoemd), `concentration_permille` (duizendsten, `null` zonder voldoende
begintijden), `offset_ms`, `also_fits` (de snelheid waarvan deze een geheel deelbaar deel is, of
`null`), `not_separated` (de snelheden die een te korte spanwijdte niet onderscheidt), `strays`
(de begintijden buiten het raster) en `starts`.

<!-- example: printf '{25}{50}Un.\n{75}{100}Deux.\n' > f.sub; subedit-cli --format json inspect --frame-rate 25 f.sub 2>/dev/null -->
```console
$ printf '{25}{50}Un.\n{75}{100}Deux.\n' > f.sub; subedit-cli --format json inspect --frame-rate 25 f.sub 2>/dev/null
{"schema":1,"command":"inspect","file":"f.sub","ok":true,"format":"microdvd","encoding":{"charset":"UTF-8","origin":"detected","byte_order_mark":false},"line_endings":{"kind":"lf","mixed_from_line":null},"subtitles":2,"span_ms":{"start":1000,"end":4000},"frame_rate":{"rate":"25","origin":"asked"},"grid":null,"anomalies":[],"warnings":[]}
```

## Toelichting

Op de foutuitvoer, naargelang het gevraagde niveau:

| Niveau | Wat er wordt toegevoegd |
| :----- | :-------------- |
| 1 | `<path>: N subtitles` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings` |
| 3 | `<path>: N bytes read`, `<path>: N diagnostic(s) while reading`, daarna **elk ervan** — zie [Aanroep](invocation.md#diagnostische-meldingen-lezen) |

## Afsluitcodes

Die van het hulpprogramma: `0` als elk bestand is gelezen, `2` als geen enkel, `3` als
slechts enkele, `1` bij een gebruiksfout.

## Fouten

Die van een leesbewerking — van het bestand, en van de vertaling wanneer `-t` is opgegeven — en
die van `-t`: zie hierboven. `inspect` schrijft niets wat kan worden geweigerd.

| Wat het veroorzaakt | Melding |
| :------------------ | :------ |
| geen pad opgegeven | `files is required`, gevolgd door een verwijzing naar `--help` |
| ontbrekend bestand | `<path>: does not exist` |
| onleesbaar bestand | `<path>: cannot be opened: permission denied` |
| bytes die niet decoderen | `<path>: cannot be decoded in the chosen encoding` |
| formaat niet herkend | `<path>: is in no format this tool knows` |
| niets wat op een ondertitel lijkt | `<path>: holds nothing recognisable as a subtitle` |

Dit zijn dezelfde meldingen voor de zeventien subcommando's: het recept om te openen
is één keer geschreven, in de kern.

## Een voorbeeld dat mislukt

Een leeg bestand komt met geen enkel formaat overeen. Het wordt niet als leeg gemeld:
het wordt geweigerd, en benoemd.

<!-- example: : > vide.srt; subedit-cli inspect vide.srt; echo "code=$?" -->
```console
$ : > vide.srt; subedit-cli inspect vide.srt; echo "code=$?"
vide.srt: is in no format this tool knows
code=2
```
