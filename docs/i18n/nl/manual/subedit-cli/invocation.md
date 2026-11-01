# Aanroep

<!-- machine-translated -->
> Deze pagina is een machinevertaling en is nog niet nagelezen door een lezer van de taal.

```
subedit-cli [global options] <subcommand> [options] <file>...
```

Zonder subcommando schrijft het hulpprogramma zijn hulp en stopt.

<!-- example: subedit-cli -->
```console
$ subedit-cli
Read, inspect and retime subtitle files.
Usage: subedit-cli [OPTIONS] [SUBCOMMAND]

Options:
  -h,--help                   Print this help message and exit
  --version                   Display program version information and exit
  -v                          Say more: -v is the default, -vv details, -vvv debugs
  -q,--quiet                  Say nothing but errors
  --encoding NAME             Encoding to read the files in; detected by default
  --format text|json          Form of the result on standard output: text or json
  --config FILE               Read the operation settings from FILE instead of the user's settings file
  --no-config Excludes: --config
                              Ignore any settings file: use the built-in defaults
  --list-encodings            Write the encodings that can be read and written, one a line, and stop

Subcommands:
  inspect                     Report what a subtitle file is made of
  convert                     Write a subtitle file out in another format or shape
  shift                       Move every position of a file by a fixed amount
  transform                   Correct every position from two points known to be right
  framerate                   Re-time a file mastered at one frame rate for another
  snap                        Move every position onto the nearest frame of a frame rate (see framerate)
  hearing-impaired            Remove the sounds described between square brackets
  adjust                      Bring the duration of every subtitle within a reading speed and limits
  replace                     Replace a text in the subtitles, without breaking a tag
  case                        Put the texts in title, sentence, upper or lower case, tags intact
  italics                     Put the texts in italics, or take their italics out
  dialogue-dashes             Put dialogue dashes on the lines, or take them off
  sort                        Put the subtitles in the order of their start, keeping ties as they are
  join-repeats                Join the subtitles that repeat each other, one after the other, into one
  disjoint                    Cut overlapping subtitles so that no range overlaps another, merging what overlaps
  repair                      Join the repeated subtitles, put them in order, then make the ranges disjoint
  correct                     Correct the texts with the patterns of the correction assistant
  append                      Put subtitle files one after another into a single file
  split-file                  Cut a subtitle file in two, a head and a tail
  pair                        Write a translation at the positions of its main file, and say how it lined up
  transcribe                  Write the speech of a video or an audio file as subtitles
  sync                        Re-time the subtitles of a file on the audio of a film
  config                      List, show and change the operation settings
```

## Taal

De opdrachtregel spreekt de taal van zijn omgeving: **zijn hulp, zijn toelichting en zijn
fouten** volgen de gevraagde taal. De voorbeelden van deze handleiding worden in het Engels uitgevoerd — de
blokken worden gegenereerd door `make manual`, nooit met de hand gekopieerd.

**Hoe de taal wordt gekozen.** De eerste van `LC_ALL`, `LC_MESSAGES` en `LANG` die is ingesteld en
niet leeg is, is de landinstelling; `LANGUAGE`, indien ingesteld, somt dan de gewenste talen op, gescheiden door
dubbele punten (`LANGUAGE=pt_BR:fr`), en wordt als eerste gelezen. Dit is de volgorde die gettext volgt, door het
programma zelf gelezen: de landinstelling hoeft niet op de machine te zijn gegenereerd.

| Regel | Effect |
| :--- | :----- |
| codering en modifier | genegeerd: `fr_FR.UTF-8@euro` zoekt naar `fr_FR`, dan `fr` |
| `LC_ALL=C`, landinstelling `C` of `POSIX`, of helemaal geen landinstelling | Engels, en `LANGUAGE` wordt niet gelezen |
| een item dat het Engels noemt (`en`, `en_GB`) | Engels: wat erop volgt in `LANGUAGE` wordt niet gelezen |
| een taal waarvan de catalogus niet is geïnstalleerd | de volgende gevraagde, en Engels na de laatste, **zonder een woord** |
| een melding die de catalogus niet vertaalt | getoond in het Engels |

**De naam van een correctiepatroon** dat `correct` niet kon toepassen, volgt de taal
ook, wanneer de catalogus hem kent — de patronen die met het programma worden meegeleverd; een eigen patroon wordt
genoemd zoals zijn bestand het schrijft. De patroonbestanden zelf worden nooit herschreven.

De catalogi worden geïnstalleerd onder `<prefix>/share/subedit/locale/<language>/LC_MESSAGES/subedit.mo`
en gevonden vanuit de eigen map van het uitvoerbare bestand. Engels is ingebouwd. Deze versie installeert `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR`, `ru`, `sr`, `tr` en `zh_CN` — Catalaans,
Tsjechisch, Duits, Spaans, Fins, Frans, Galicisch, Hebreeuws, Hongaars, Interlingue, IJslands, Italiaans, Nederlands, Pools, Portugees, Braziliaans Portugees, Russisch, Servisch, Turks en Vereenvoudigd Chinees — volledig vertaald. Frans is nagelezen
door een lezer van de taal; Catalaans, Tsjechisch, Duits, Spaans, Fins, Galicisch, Hebreeuws, Hongaars, Interlingue, IJslands, Italiaans, Nederlands, Pools, Portugees, Braziliaans Portugees, Russisch, Servisch, Turks en Vereenvoudigd Chinees zijn
machinevertaald en nog door geen lezer van de taal nagelezen.
<!-- machine-translated: ca -->
<!-- machine-translated: cs -->
<!-- machine-translated: de -->
<!-- machine-translated: es -->
<!-- machine-translated: fi -->
<!-- machine-translated: gl -->
<!-- machine-translated: he -->
<!-- machine-translated: hu -->
<!-- machine-translated: ie -->
<!-- machine-translated: is -->
<!-- machine-translated: it_IT -->
<!-- machine-translated: nl -->
<!-- machine-translated: pl -->
<!-- machine-translated: pt -->
<!-- machine-translated: pt_BR -->
<!-- machine-translated: ru -->
<!-- machine-translated: sr -->
<!-- machine-translated: tr -->
<!-- machine-translated: zh_CN -->
Onder `LANG=fr_FR.UTF-8` begint `subedit-cli --help` met “Lire,
inspecter et recaler des fichiers de sous-titres.” Een script dat de woorden van de tekstuitvoer
leest, stelt `LANGUAGE=en` in (of `LC_ALL=C`) — of leest `--format json`, dat nooit verandert.

**Wat nooit de taal volgt** — wat een script leest:

| Nooit vertaald | Waarom |
| :--------------- | :-- |
| de uitvoer van `--format json`: sleutels, waarden, en de `message` en `detail` van mislukkingen en waarschuwingen | dezelfde bytes in elke taal; de woorden zijn Engels |
| afsluitcodes | de vier van [Afsluitcodes](#afsluitcodes), in elke taal |
| de ingetypte namen: subcommando's, opties, hun waarden (`off`, `srt`), metavariabelen (`FILE`, `TEXT`, `OPTIONS`) | een opdrachtregel die één keer is geschreven, werkt onder elke taal |
| de namen van correctiepatronen die `correct --enable` en `--disable` accepteren | een patroon wordt aangeroepen met de Engelse naam die zijn bestand bevat |
| de namen van formaten en coderingen | een formaat wordt bij zijn naam genoemd |
| de eigen diagnostische meldingen van CLI11 — een ontbrekend verplicht argument, een onbekende optie, een waarde die een optie niet accepteert — en zijn `Run with --help for more information.` | verwoord binnen de parser waarop de opdrachtregel is gebouwd; ze blijven Engels |

Het Engels vragen, wat de omgeving ook is:

<!-- example: LC_ALL=C LANGUAGE=fr subedit-cli shift --output-dir out a.srt; echo "code=$?" -->
```console
$ LC_ALL=C LANGUAGE=fr subedit-cli shift --output-dir out a.srt; echo "code=$?"
shift needs --by, or --to-grid to work the amount out from the positions
code=1
```

## Algemene opties

**Ze worden vóór de naam van het subcommando geschreven, en alleen daar.**
`subedit-cli --encoding cp1252 inspect film.srt` is de vorm;
`subedit-cli inspect --encoding cp1252 film.srt` wordt geweigerd, met
`The following argument was not expected: --encoding`. Hetzelfde geldt voor
`--version`, `-v` en `--quiet`.

**`--help` is de uitzondering, en eigenlijk is het er geen**:
`subedit-cli inspect --help` wordt geaccepteerd omdat elk subcommando een eigen optie heeft, en het is de
hulp van het subcommando die wordt geschreven, niet die van het hulpprogramma.

| Optie | Effect |
| :----- | :---- |
| `-h`, `--help` | schrijft de hulp en stopt; die van het subcommando als er een wordt genoemd |
| `--version` | schrijft `subedit <version>`, de bibliotheken waartegen het hulpprogramma draait en de geïnstalleerde talen, en stopt — zie [`--version`](#--version) |
| `-q`, `--quiet` | niveau 0 — helemaal geen toelichting |
| `-v`, `-vv`, `-vvv` | niveaus 1 tot 3; niveau 1 is de standaardwaarde |
| `--encoding NAME` | leest de bestanden in deze codering, in plaats van haar te raden |
| `--list-encodings` | schrijft de coderingen die het hulpprogramma kan lezen en schrijven, een per regel, en **stopt** (code `0`) — zie [`--list-encodings`](#--list-encodings) |
| `--format text\|json` | de vorm van het resultaat op de standaarduitvoer: de tekst, of één JSON-object per bestand — zie [Uitvoer die een script kan lezen](#uitvoer-die-een-script-kan-lezen) |
| `--config FILE` | leest de bewerkingsinstellingen uit `FILE` in plaats van uit het instellingenbestand van de gebruiker — zie [Het instellingenbestand](#het-instellingenbestand) |
| `--no-config` | leest geen instellingenbestand: elke bewerkingsinstelling die niet op de regel is opgegeven, krijgt zijn ingebouwde standaardwaarde — zie [Het instellingenbestand](#het-instellingenbestand) |

`--quiet` en `-v` in dezelfde aanroep worden geweigerd: twee tegengestelde bedoelingen
worden niet ten gunste van de laatst geschreven beslecht.

`--config` en `--no-config` in dezelfde aanroep worden ook geweigerd, met
`--config excludes --no-config`, en dat geldt ook voor `--config` met een lege naam, met
`--config needs a file name`: een lege naam duidt geen bestand aan, en het eigen bestand van de gebruiker wordt niet
in de plaats ervan gelezen.

### Het instellingenbestand

**[`adjust`](adjust.md) en [`correct`](correct.md) lezen standaard de bewerkingsinstellingen van het
instellingenbestand** — de waarden die `Adjust Durations…` en `Correct Texts…` in
het venster onthouden, of die [`config set`](config.md) heeft geschreven. Geen enkel ander subcommando leest het. Het bestand is:

```
$XDG_CONFIG_HOME/subedit/settings.conf
```

of `~/.config/subedit/settings.conf` wanneer `XDG_CONFIG_HOME` niet is ingesteld, leeg is of geen absoluut
pad is — het bestand van het venster, beschreven in [de handleiding ervan](../subedit-gui/preferences.md#het-bestand).

**Prioriteit, de sterkste eerst: de optie op de opdrachtregel, dan het bestand, dan de ingebouwde
standaardwaarde.** Een optie wint altijd: `--speed off` schakelt een snelheid uit die het bestand instelt. Alleen het
**bewerkingsblok** van het bestand wordt gelezen; de weergavevoorkeuren (geometrie, kolommen, thema…)
nooit. [`config list`](config.md#config-list) noemt de sleutels, en welke ervan de opdrachtregel
leest: de negen sleutels `duration-adjust.*`, `correction.human`, `correction.ocr`,
`correction.remove-blank` en `correction.line-break.max-lines`. **Elk ervan heeft een optie
die het voor één aanroep overschrijft**, hoe het bestand het ook instelt — de tabellen van
[`adjust`](adjust.md#wat-het-instellingenbestand-aanlevert) en
[`correct`](correct.md#wat-wordt-gekozen-en-wat-niet) noemen ze.

**Het bestand van de gebruiker lezen laat een commando nooit mislukken.** Een ontbrekend bestand geeft de standaardwaarden; een bestand
dat niet kan worden gelezen geeft de standaardwaarden en zegt dat; een waarde die niet kan worden gelezen behoudt zijn standaardwaarde en wordt genoemd; een
onbekende sleutel wordt genegeerd. Het bestand wordt nooit door `adjust` of `correct` geschreven.

**Wat is gelezen, wordt gemeld**, op het standaardniveau 1, op de foutuitvoer, vóór het eerste bestand:

| Geval | Regel |
| :--- | :--- |
| het bestand is gelezen | `settings: read <path>` |
| een waarde erin kon niet worden gelezen | `settings: <key>: cannot read '<value>', default kept` |
| het bestand van de gebruiker bestaat niet | `settings: <path> not found, defaults used` |
| het bestand kan niet worden gelezen | `settings: <path> unreadable, defaults used` |
| `--no-config` | niets |

Met [`--format json`](#uitvoer-die-een-script-kan-lezen) bevat het object van elk bestand van `adjust` en
`correct` `settings`: het pad dat is gelezen, of `null` wanneer er geen was.


**`--no-config` maakt een uitvoering reproduceerbaar**: dezelfde regel geeft op elke
machine hetzelfde resultaat, wat de gebruiker ook in het venster heeft ingesteld. Gebruik het in scripts, tests en bugrapporten — of
`--config FILE` met een bestand dat erbij hoort.

**`--config FILE` noemt een ander bestand**, op dezelfde manier gelezen — **behalve dat het moet bestaan**. Een
naam die op de opdrachtregel wordt opgegeven, is geen verzoek om de standaardwaarden: wanneer `FILE` niet bestaat,
stoppen `adjust` en `correct` voordat een invoer wordt gelezen, schrijven `settings file not found: <FILE>` op
de foutuitvoer, en sluiten af met de gebruikscode `1`. Een `FILE` die bestaat en niet kan worden gelezen,
geeft de standaardwaarden en zegt dat, zoals het bestand van de gebruiker. [`config show`](config.md#config-show)
toont een ontbrekend `FILE` nog steeds als `(not found)`, en [`config set`](config.md#config-set-en-config-unset) maakt het nog steeds
aan.

<!-- example: printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine.conf adjust --dry-run a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine.conf adjust --dry-run a.srt; echo "code=$?"
settings file not found: mine.conf
code=1
```

### `--version`

**De versie van het programma, alleen op de eerste regel** — `subedit <version>`, wat de uitvoer was tot
0.15.38 en wat een script leest. Daarna **een regel per bibliotheek waartegen het hulpprogramma draait**, daarna de geïnstalleerde
interfacetalen, op de standaarduitvoer; het hulpprogramma stopt, met code `0`. Een subcommando dat ook is opgegeven,
wordt niet uitgevoerd, en `-q` en `-v` veranderen niets.

<!-- example: subedit-cli --version | head -n 1 -->
```console
$ subedit-cli --version | head -n 1
subedit 1.2.0
```

Het geheel, op een machine met ICU 74.2 en Enchant 2.3.3 — de nummers zijn die van de machine:

```console
$ subedit-cli --version
subedit <version>
  ICU      74.2 (built with 74.2)
  Enchant  2.3.3 (built with 2.3.3)
  CLI11    2.4.1 (built with 2.4.1)
  Languages: en, fr
```

| Bibliotheek | Waarvoor het hulpprogramma haar gebruikt |
| :------ | :------------------------ |
| `ICU` | de coderingen lezen en schrijven, ze detecteren, de hoofdletters en kleine letters van letters wijzigen |
| `Enchant` | de spellingcontrole van `correct` |
| `CLI11` | deze opdrachtregel lezen |

- **Elke regel geeft twee versies**: degene die tijdens de uitvoering is gevonden, eerst, en degene waartegen het programma is
  gecompileerd, tussen haakjes. Ze verschillen wanneer de bibliotheek van de machine is bijgewerkt nadat het
  programma werd gebouwd — het eerste wat in een bugrapport moet staan.
- **De headers van Enchant bevatten geen versie**: degene tussen haakjes is degene die `pkg-config` meldde
  toen de build werd geconfigureerd.
- **CLI11 is in het programma gecompileerd**, zijn headers zijn alles wat ervan bestaat: zijn twee versies zijn
  altijd dezelfde.
- **Noch Qt noch libmpv**: de opdrachtregel linkt geen van beide, en somt alleen op waartegen hij draait.
  Het venster somt ze op — zie [de handleiding ervan](../subedit-gui/invocation.md#--version).
- **`Languages`** somt `en` op, de brontaal, die geen catalogus nodig heeft, daarna elke catalogus
  die onder `<prefix>/share/subedit/locale` is geïnstalleerd en leesbaar is, per code — de talen die
  `Aanpassen ▸ Instellingen…` aanbiedt.
- **Wat de taal volgt**: `built with` en `Languages`. De eerste regel, de namen, de
  nummers en de taalcodes nooit.
- **Met `--format json`**, vóór of na `--version`, **één object op één regel**: `schema`
  (`1`), `command` (`"version"`), `version`, `libraries` — een array van
  `{"name":"ICU","loaded":"74.2","built":"74.2"}`, in de volgorde van de tekst — en `languages`, een
  array van codes. Niets erin wordt vertaald.

<!-- example: subedit-cli --version --format json | grep -o '"name":"[^"]*"' -->
```console
$ subedit-cli --version --format json | grep -o '"name":"[^"]*"'
"name":"ICU"
"name":"Enchant"
"name":"CLI11"
```

### `--list-encodings`

**De coderingen die het hulpprogramma kan lezen en schrijven, een per regel, op naam gesorteerd**, op de standaarduitvoer —
daarna stopt het hulpprogramma, met code `0`, zoals `--version` doet: het is het antwoord, geen stap. Een
subcommando dat ook is opgegeven, wordt niet uitgevoerd.

<!-- example: subedit-cli --list-encodings | grep '^UTF' | head -4 -->
```console
$ subedit-cli --list-encodings | grep '^UTF' | head -4
UTF-16BE
UTF-16LE
UTF-32BE
UTF-32LE
```

- **Het zijn de namen die `--encoding` accepteert**, de canonieke; hun aliassen (`cp1252`) worden ook geaccepteerd,
  zonder te worden opgesomd.
- **Het aantal hangt af van de geïnstalleerde ICU**, en geen enkele naam wordt beloofd behalve die welke elke ICU heeft
  (`UTF-8`, `UTF-16LE`, `windows-1252`…). De handleiding geeft dus geen aantal.
- **`UTF-16` en `UTF-32` staan er niet in**: ICU kent ze, maar hun converter schrijft een eigen markering,
  en het model weigert ze — zie hieronder. Een lijst om uit te kiezen biedt niet aan wat het volgende veld
  zou weigeren.
- **Met `--format json`**, **een object per codering**:
  `{"schema":1,"command":"list-encodings","name":"UTF-8"}`. Het is niet het rapport van een bestand: deze
  objecten **bevatten noch `file` noch `ok`**, de enige uitzondering op de envelop van
  [de vorm](#de-vorm). Het zijn nog steeds objecten van een per regel, een per item.
- **Het toelichtingsniveau verandert niets**: een lijst is een resultaat, en `-q` neemt haar niet weg.

### `--encoding`, en wat het niet kan afdwingen

**Het geldt voor elk subcommando**, omdat ze allemaal lezen: een bestand waarvan de
codering verkeerd is geraden, blijft verkeerd wat men er ook mee doet, en een optie
die alleen `inspect` had, zou een discrepantie zonder oplossing achterlaten.

**De naam is die van ICU, en al zijn aliassen worden geaccepteerd**: `cp1252`,
`windows-1252` en `WINDOWS 1252` duiden dezelfde codering aan. De verzameling staat
nergens geschreven: het is degene die de ICU van de machine kan omzetten, en
[`--list-encodings`](#--list-encodings) somt haar op. Een naam die ze niet kan omzetten, wordt
geweigerd voordat één bestand wordt gelezen:

```console
$ subedit-cli --encoding klingon-1 inspect film.srt
no encoding is named "klingon-1"
```

**Een uitzondering, en maar één: de codering moet haar bytevolgorde noemen.**
`UTF-16` en `UTF-32` zijn namen die ICU kent en waarvan de converter **een eigen
markering** schrijft. Zo'n codering kan in dit model niet bestaan — de markering is
geen keuze meer, `--no-bom` zou worden geaccepteerd en genegeerd — dus beide worden
geweigerd, voor het lezen en voor het schrijven:

```console
$ subedit-cli --encoding UTF-16 inspect film.srt
"UTF-16" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do
```

`UTF-16LE` en `UTF-16BE` lezen en schrijven dezelfde bytes, markering inbegrepen; wat
de precieze naam toevoegt, is dat men weet welke.

**Een bytevolgordemarkering wint het van de optie.** Het is het enige wat een ondertitelbestand
over zichzelf declareert, en het anders lezen dan het zich declareert, zou
de aanroeper gehoorzamen tegen het bestand in. Het verschil wordt **gemeld**, als
diagnostische melding:

```console
$ subedit-cli -vvv --encoding cp1252 inspect avec-bom.srt
avec-bom.srt: a byte order mark that contradicts the encoding asked for ("UTF-16LE"), settled by the reader
```

Dat is wat Gaupol doet — het begint het lezen opnieuw met de codering van
de markering — maar zonder het te zeggen; hier is het verschil tussen wat is gevraagd en
wat is gedaan geen stilte.

## Subcommando's

| Subcommando | Wat het doet | Schrijft het? |
| :------------ | :-------------- | :----------- |
| [`inspect`](inspect.md) | meldt waaruit een bestand bestaat, zonder iets te wijzigen | nee |
| [`convert`](convert.md) | schrijft een bestand in een ander formaat, of een andere vorm | ja |
| [`shift`](shift.md) | verplaatst elke positie met hetzelfde bedrag | ja |
| [`transform`](transform.md) | corrigeert elke positie uit twee punten waarvan bekend is dat ze juist zijn | ja |
| [`framerate`](framerate.md) | timet een bestand opnieuw van de ene framerate naar de andere | ja |
| [`snap`](snap.md) | verplaatst elke tijdstempel naar het dichtstbijzijnde frame | ja |
| [`hearing-impaired`](hearing-impaired.md) | verwijdert de vermeldingen voor slechthorenden | ja |
| [`adjust`](adjust.md) | past de duren aan een leessnelheid en aan limieten aan | ja |
| [`replace`](replace.md) | vervangt een tekst in de ondertitels, zonder een tag te breken | ja |
| [`case`](case.md) | zet de teksten in elk woord met hoofdletter, zin met hoofdletter, hoofdletters of kleine letters | ja |
| [`italics`](italics.md) | zet de teksten cursief, of haalt hun cursief eruit | ja |
| [`dialogue-dashes`](dialogue-dashes.md) | zet dialoogstreepjes erop, of haalt ze eraf | ja |
| [`sort`](sort.md) | zet de ondertitels terug in de volgorde van hun begin | ja |
| [`join-repeats`](join-repeats.md) | maakt één enkele ondertitel van de stukken van één tekst die op elkaar volgen | ja |
| [`disjoint`](disjoint.md) | knipt de overlappende ondertitels zo bij dat geen enkel bereik nog een ander overlapt | ja |
| [`repair`](repair.md) | schakelt `join-repeats`, `sort` en `disjoint` aaneen | ja |
| [`correct`](correct.md) | corrigeert de teksten met de patronen van Gaupol: vermeldingen, woorden samenvoegen en splitsen, veelgemaakte fouten, hoofdletters, regelafbreking | ja |
| [`append`](append.md) | zet bestanden achter het eerste, in **één enkel** bestand | ja |
| [`split-file`](split-file.md) | knipt een bestand in **twee**: een kop en een staart | ja |
| [`pair`](pair.md) | schrijft een vertaling op de posities van haar hoofdbestand, en zegt hoe haar regels zijn uitgelijnd | ja |
| [`transcribe`](transcribe.md) | schrijft de gesproken tekst van een video of een audiobestand als ondertitels | ja — een eigen bestemming, `--output` |
| [`sync`](sync.md) | hertimet de ondertitels van een bestand op de audio van een film | ja |
| [`config`](config.md) | somt de bewerkingsinstellingen van het instellingenbestand op, toont en wijzigt ze | alleen het instellingenbestand |

Alle drieëntwintig staan erin; de hulp van het hulpprogramma somt ze in dezelfde volgorde op.

**Slechts één raakt niets aan**, en dat is de rechterkolom: `inspect` leest en
meldt. De andere eenentwintig schrijven, en wat volgt geldt voor achttien ervan:
[`append`](append.md) en [`split-file`](split-file.md) hebben elk een eigen arity — N bestanden voor
**één** resultaat, één bestand voor **twee** — en dus een eigen bestemming, die ze op hun pagina beschrijven;
[`transcribe`](transcribe.md) maakt een nieuw bestand van een video of een audiobestand, en neemt alleen `--output`.
[`config`](config.md) schrijft geen ondertitelbestand: het leest en wijzigt het instellingenbestand, en
niets van wat volgt is erop van toepassing.

## De bestemming

De achttien subcommando's die schrijven — [`convert`](convert.md),
[`shift`](shift.md), [`transform`](transform.md), [`framerate`](framerate.md),
[`snap`](snap.md), [`hearing-impaired`](hearing-impaired.md), [`adjust`](adjust.md),
[`replace`](replace.md), [`case`](case.md), [`italics`](italics.md),
[`dialogue-dashes`](dialogue-dashes.md), [`sort`](sort.md), [`join-repeats`](join-repeats.md),
[`disjoint`](disjoint.md), [`repair`](repair.md), [`correct`](correct.md), [`pair`](pair.md), [`sync`](sync.md) — nemen hun bestemming op dezelfde manier. ([`append`](append.md) en
[`split-file`](split-file.md) hebben een eigen, op hun pagina.) [`inspect`](inspect.md) schrijft geen bestand en
accepteert geen van deze opties.

**Een bestemming die al bestaat, wordt overschreven**, zonder vraag en zonder optie om dat
te vermijden: het schrijven is atomair — het bestand is nooit half geschreven, en een
onderbroken schrijfbewerking laat het oude intact —, maar de vorige inhoud gaat
verloren. Dit is geen gebrek dat men laat voortbestaan: het is de regel, en een commando
opnieuw uitvoeren op dezelfde uitvoermap voert het opnieuw over zichzelf uit.

**Er wordt nooit iets geschreven zonder uitdrukkelijke bestemming.** De drie manieren om haar
op te geven sluiten elkaar uit, en er is er een nodig: zonder wordt niets geschreven en is de afsluit-
code `1` — behalve met [`--dry-run`](#kijken-voor-het-schrijven), dat niets schrijft
en dus geen vereist.

| Optie | Waar het resultaat heen gaat |
| :----- | :---------------- |
| `--output FILE` | in dit bestand, onder deze exacte naam; een enkele invoer |
| `--output-dir DIR` | in deze map, onder de basisnaam van de invoer |
| `--in-place` | over de invoer, door atomair schrijven |

Een hulpprogramma dat zijn invoer overschrijft omdat men het niets heeft gezegd, is een hulpprogramma dat men
de tweede keer niet meer gebruikt. De weigering kost een regel en redt het
bestand.

`--output` met meerdere invoeren wordt geweigerd: het laatste bestand zou over de
vorige worden geschreven. Bij een reeks is `--output-dir` de enige van de drie die zinvol is, naast
`--in-place`.

**De uitvoermap wordt aangemaakt** als ze ontbreekt, ouders inbegrepen — voor
`--output-dir`, en voor de map waarin `--output` schrijft. Dat gebeurt zodra de
opdrachtregel is gevalideerd en **vóór het eerste bestand**. Als het niet kan,
wordt dat één keer gemeld, wordt geen bestand gelezen, en is de afsluitcode
`2`.

**De reeks wordt als geheel beoordeeld vóór het schrijven.** De bestemming van elke invoer
wordt eerst berekend, en twee weigeringen volgen daaruit — gebruiksfouten, code
`1`, **geen bestand geschreven** en geen map aangemaakt:

- **twee invoeren die in hetzelfde bestand zouden belanden** — `a/film.srt` en
  `b/film.srt` met `--output-dir`, of `a/film.srt` en `b/film.vtt` die beide
  naar WebVTT zijn geconverteerd, waarvan de uiteindelijke bestemming van extensie is veranderd. De
  melding noemt de bestemming en beide invoeren; de tweede hernoemen of
  de laatste laten winnen zou een bestand verliezen zonder het te zeggen, en weigeren
  kost een regel;
- **een bestemming die een van de invoeren is**, zonder `--in-place` — ook
  via `--output-dir .`, via `--output` of via een symbolische koppeling. Over
  zijn invoer schrijven is een handeling die men benoemt.

Twee paden duiden hetzelfde bestand aan wanneer het systeem het zegt, niet wanneer ze
op dezelfde manier zijn geschreven: `in/../in/film.srt` en `in/film.srt` zijn één enkel bestand.

**De extensie volgt het geschreven formaat.** Ze verandert alleen voor
[`convert`](convert.md), het enige subcommando dat van formaat verandert; de
andere behouden het formaat van het gelezen bestand, en dus zijn extensie.

| Wat het veroorzaakt | Melding |
| :------------------ | :------ |
| geen bestemming, zonder `--dry-run` | `no destination given: use --output, --output-dir or --in-place` |
| twee bestemmingen | `--output, --output-dir and --in-place exclude one another` |
| `--output` bij een reeks | `--output names one file but several were given: use --output-dir instead` |
| twee invoeren, een bestemming | `<destination>: would be written by both <input> and <input>` |
| een invoer zou worden overschreven | `<destination>: would be written over the input <input>` — dezelfde zin voor [`append`](append.md) en [`split-file`](split-file.md) |
| uitvoermap die niet kan worden aangemaakt | `<directory>: cannot be created: permission denied`, of `: cannot be created` |
| bestemming die niet kan worden geschreven | `<path>: <destination>: cannot be written: permission denied`, of `: cannot be written` |
| teken dat in de geschreven codering ontbreekt | `<path>: holds a character the chosen encoding cannot write` |

**De laatste verdient een zin.** Een bestand wordt teruggeschreven in **de codering waarin het
is gelezen**, net als met zijn regeleinden en zijn markering. Als een
wijziging er een teken in heeft gebracht dat deze codering niet kent — een
`ł` in Latin-1 —, mislukt het schrijven en **wordt niets geschreven**. Het vervangen door
een `?` zou tekst verliezen voor de ogen van wie hem zojuist heeft geschreven.

## Kijken voor het schrijven

**`--dry-run`** wordt door elk van de zestien subcommando's die schrijven geaccepteerd, en betekent overal
hetzelfde: **lezen, berekenen, melden, geen bestand schrijven**. Men voegt het toe aan de
regel die men van plan was uit te voeren; toepassen is dezelfde regel opnieuw uitvoeren zonder. De berekening is
deterministisch: wat een `--dry-run` liet zien, is wat de volgende uitvoering zal schrijven, zolang het
bestand niet is veranderd.

- **Het schrijft geen bestand en maakt geen map aan**, `--in-place` inbegrepen: het invoer-
  bestand blijft zoals het was.
- **Het vereist geen bestemming.** Er kan er een worden opgegeven, en **ze wordt dan gecontroleerd als zonder
  `--dry-run`**: twee invoeren voor dezelfde bestemming, een invoer die zou worden overschreven,
  `--output` bij een reeks, twee manieren om te zeggen waar — een opdrachtregel die zonder `--dry-run` wordt geweigerd,
  wordt er ook mee geweigerd, met dezelfde zin en dezelfde code `1`.
- **De afsluitcode is die van een echte uitvoering**: `0` wanneer alles is berekend, `2` of `3`
  wanneer bestanden niet konden worden gelezen of verwerkt, `1` bij gebruik. Een teken dat
  de codering van het bestand niet kon schrijven, laat een proefrun mislukken zoals een echte.
  **Dat er wijzigingen bestaan, verandert de code niet**: de vier codes zeggen of het hulpprogramma
  is geslaagd, niet of het bestand voldoet.
- **De toelichting zegt het.** Waar een echte uitvoering `-> <destination>` schrijft, schrijft een proefrun
  `(dry run, nothing written)`.

<!-- example: printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by 1 --dry-run a.srt; ls -- *.srt -->
```console
$ printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by 1 --dry-run a.srt; ls -- *.srt
a.srt: 1 subtitle shifted by 1.000 s (dry run, nothing written)
a.srt
```

**Bij een subcommando dat teksten wijzigt** —
[`hearing-impaired`](hearing-impaired.md), [`replace`](replace.md), [`case`](case.md),
[`italics`](italics.md), [`dialogue-dashes`](dialogue-dashes.md) en [`correct`](correct.md) —, **bevat de standaarduitvoer de voorgestelde
wijzigingen**. Een blok per gewijzigde ondertitel, in de volgorde van het bestand:

```
<path>: subtitle <number>
- <the text before, line by line>
+ <the text after, line by line>
```

- het **nummer** is dat wat de ondertitel in het gelezen bestand heeft, vanaf één geteld;
- elke regel van de tekst ervoor draagt `- ` ervoor, elke regel van de tekst erna `+ `: een tekst
  van meerdere regels leest zoals hij is, en een regel die zelf met een dialoog-
  streepje begint, geeft `- - `;
- **een ondertitel die de bewerking zou verwijderen**, zegt dat op zijn eerste regel,
  `<path>: subtitle <number> (removed)`, en heeft geen `+`-regels;
- een ondertitel van de vertaling, met `--document translation`, kondigt zichzelf aan als `(translation)` op dezelfde
  plaats — zie [Een vertaling](#een-vertaling);
- de eerste regel van elk blok begint met het pad, zodat `grep '^film.srt: '`
  de blokken van één bestand in een reeks vindt;
- **geen blok** wanneer niets zou veranderen.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\n- [Grincement]\n- Qui est là ?\n\n' > mentions.srt; subedit-cli hearing-impaired --dry-run mentions.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\n- [Grincement]\n- Qui est là ?\n\n' > mentions.srt; subedit-cli hearing-impaired --dry-run mentions.srt
mentions.srt: 2 subtitles cleaned, 1 removed (dry run, nothing written)
mentions.srt: subtitle 1 (removed)
- [Bruit de pas]
mentions.srt: subtitle 2
- Attends [il tousse] Marie.
+ Attends Marie.
mentions.srt: subtitle 3
- - [Grincement]
- - Qui est là ?
+ Qui est là ?
```

**De lijst is dezelfde in JSON**: het object van het bestand bevat een array `changes`, zie
[de vorm](#de-vorm), met `after` op `null` voor een verwijderde ondertitel. **Ze wordt alleen
berekend wanneer `--dry-run` of `--format json` erom vraagt**: een reeks van duizenden
ondertitels bouwt haar niet voor niets.

Bij **de andere subcommando's** — [`convert`](convert.md), [`shift`](shift.md),
[`transform`](transform.md), [`framerate`](framerate.md), [`snap`](snap.md),
[`adjust`](adjust.md), [`sort`](sort.md), [`append`](append.md), [`split-file`](split-file.md), [`pair`](pair.md) —, bevat de standaard-
uitvoer wat die van een echte uitvoering zou bevatten, dat wil zeggen niets in tekst; in
JSON het object van elk bestand met `"dry_run":true`, `"destination":null` en dezelfde
`counts`.

## Twee uitvoerkanalen, twee rollen

| Uitvoer | Wat ze bevat |
| :----- | :--------------- |
| standaard | **het resultaat, en alleen het resultaat** — het rapport van `inspect`, de voorgestelde wijzigingen van een tekst-[`--dry-run`](#kijken-voor-het-schrijven); met `--format json` een object per bestand — een per uitvoering voor [`append`](append.md), [`split-file`](split-file.md) en [`pair`](pair.md) |
| fout | **al het andere** — de toelichting, de waarschuwingen, de fouten |

Het is deze scheiding die toelaat het resultaat om te leiden zonder het verhaal erin te krijgen:

```console
$ subedit-cli inspect *.srt > rapport.txt
```

Het rapport gaat naar het bestand, de toelichting blijft op het scherm.

## Uitvoer die een script kan lezen

**`--format json`** vervangt de tekst van de standaarduitvoer door **één JSON-object per
invoerbestand**, in het formaat JSON Lines: een compact object per regel, in UTF-8 zonder
markering, het regeleinde is `\n`. `text` is de standaardwaarde en verandert niet. Het is een
algemene optie, en wordt dus vóór het subcommando geschreven; een andere waarde dan
`text` en `json` is een gebruiksfout, code `1`.

<!-- example: printf '1\n00:00:10,000 --> 00:00:12,000\nUn.\n\n' > a.srt; subedit-cli --format json shift --by 1 --output-dir sortie a.srt absent.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:10,000 --> 00:00:12,000\nUn.\n\n' > a.srt; subedit-cli --format json shift --by 1 --output-dir sortie a.srt absent.srt; echo "code=$?"
a.srt: 1 subtitle shifted by 1.000 s -> sortie/a.srt
{"schema":1,"command":"shift","file":"a.srt","ok":true,"dry_run":false,"destination":"sortie/a.srt","counts":{"subtitles":1,"shifted_by_ms":1000},"warnings":[]}
absent.srt: does not exist
{"schema":1,"command":"shift","file":"absent.srt","ok":false,"error":{"kind":"not-found","message":"absent.srt: does not exist"}}
1 of 2 files shifted, 1 failed
code=3
```

**Een object per invoer, nooit nul, nooit twee**, of de invoer nu is geslaagd of niet, in
de volgorde waarin de invoeren zijn opgegeven — of gevonden, met [`--recursive`](lots.md). Men
telt de regels, men vindt zijn bestanden. **Drie subcommando's vormen de uitzondering, omdat hun
arity niet die van een reeks is: [`append`](append.md) (N invoeren, een uitvoer), [`split-file`](split-file.md)
(een invoer, twee uitvoeren) en [`pair`](pair.md) (twee bestanden, een uitvoer) schrijven **een object voor de hele
uitvoering**, dat het bestand noemt dat hun pagina aangeeft.** **De subcommando's die herschrijven, schrijven dus ook
op de standaarduitvoer**, wat hun contract in tekst niet deed: hun
bestand blijft hun resultaat, en het object is het rapport ervan.

**De toelichting verandert niet.** `-q`, `-v`, `-vv` en `-vvv` werken op de foutuitvoer,
in tekst, zoals zonder `--format`; **de standaarduitvoer varieert er niet mee**.
De diagnostische meldingen bij het lezen, die de tekst op niveau 3 houdt, zijn gegevens en staan
**altijd** in `warnings`. Een gebruiksfout schrijft geen object: ze wordt ontdekt
voordat een bestand wordt aangeraakt, dus er is niets om haar aan te hechten; ze gaat naar de foutuitvoer
met code `1`, en de standaarduitvoer blijft leeg.

### De vorm

| Sleutel | Inhoud | Aanwezig |
| :-- | :------ | :------- |
| `schema` | geheel getal, de versie van deze vorm — `1` | altijd |
| `command` | de naam van het subcommando | altijd |
| `file` | het pad **zoals opgegeven**, of zoals de doorloop van een map het samenstelde | altijd |
| `ok` | boolean | altijd |
| `error` | `{"kind", "message"}` | als `ok` onwaar is |
| `dry_run` | boolean: waar voor een [`--dry-run`](#kijken-voor-het-schrijven) | als `ok` waar is, bij een subcommando dat schrijft |
| `destination` | het geschreven pad, of `null` wanneer `dry_run` waar is: er is niets geschreven | idem |
| `counts` | een object van **gehele getallen**, specifiek voor het subcommando | idem |
| `changes` | een array van `{"subtitle", "document", "before", "after"}`: de gewijzigde teksten, zie hieronder | bij een subcommando dat teksten wijzigt |
| `warnings` | een array van `{"kind", "line"?, "detail"?, "settled"?}` | als `ok` waar is |
| `alignment` | hoe een vertaling is gelegd, zie [Een vertaling](#een-vertaling) | met `-t`, en voor `pair` |
| `constraints` | de gebruikte instellingen | `adjust` |
| `settings` | het instellingenbestand dat is gelezen, of `null` wanneer er geen was — zie [Het instellingenbestand](#het-instellingenbestand) | `adjust`, `correct`, als `ok` waar is |
| `inputs`, `tail` | een element per toegevoegd bestand; het pad van de staart | `append`; `split-file` |

`line` ontbreekt bij een diagnostische melding over het hele bestand; `detail` ontbreekt wanneer er
niets toe te voegen valt; `settled` is `true` wanneer het lezen iets **heeft beslecht** — een
opnieuw gegenereerde nummering —, en ontbreekt wanneer het de zaken liet zoals ze stonden.

**`changes`** somt, in de volgorde van het bestand, elke ondertitel op waarvan de tekst verandert:
`subtitle` is het nummer in het gelezen bestand, vanaf één geteld; `document` is `"main"`
of `"translation"`; `before` is de tekst ervoor; `after` de tekst erna, of **`null` wanneer
de ondertitel wordt verwijderd**. De regeleinden van een tekst zijn er `\n`. Het wordt
geschreven bij een echte uitvoering zoals bij een [`--dry-run`](#kijken-voor-het-schrijven), en is **leeg
(`[]`) wanneer niets verandert**: de aanwezigheid ervan zegt dat het subcommando zijn wijzigingen opsomt,
de afwezigheid dat het er geen heeft.

**Nergens een drijvendekommagetal.** Een positie of een duur is een geheel getal van
**milliseconden** (`…_ms`); een telling is een geheel getal; een framerate is een **string**
(`"25"`, `"24000/1001"`) omdat ze exact is, wat een rationaal drijvendekommagetal niet
kan bevatten; een rasterconcentratie is een geheel getal in **duizendsten** — `1000` is een
perfect raster. Twee uitvoeren vergelijken dan byte voor byte, zonder afronding.

**Een pad dat geen UTF-8 is** — Linux staat het toe — kan niet in JSON worden voorgesteld: het
wordt geschreven met U+FFFD in plaats van de ongeldige bytes, en het object bevat de waarschuwing
`path-not-utf8`. De getrouwheid van zo'n pad wordt niet beloofd, en dat wordt gezegd.

### Wat elk subcommando in `counts` zet

| Subcommando | Sleutels van `counts` |
| :------------ | :--------------- |
| [`shift`](shift.md) | `subtitles`, `shifted_by_ms` (met teken) |
| [`transform`](transform.md) | `subtitles` |
| [`framerate`](framerate.md) | `subtitles` |
| [`snap`](snap.md) | `subtitles`, `moved` (verplaatste posities), `furthest_ms` (de grootste verplaatsing) |
| [`hearing-impaired`](hearing-impaired.md) | `cleaned` (herschreven teksten), `removed` (verwijderde ondertitels) |
| [`replace`](replace.md) | `replaced` (vervangen overeenkomsten in de gewijzigde ondertitels), `matched` (gevonden overeenkomsten) |
| [`case`](case.md), [`italics`](italics.md), [`dialogue-dashes`](dialogue-dashes.md) | `changed` (ondertitels waarvan de tekst is veranderd) |
| [`correct`](correct.md) | `corrected` (gewijzigde teksten), `removed` (verwijderde ondertitels) — nooit overeenkomsten |
| [`sort`](sort.md) | `subtitles`, `moved` (de plaatsen die van ondertitel zijn veranderd) |
| [`adjust`](adjust.md) | `subtitles` (de beoogde ondertitels), `adjusted` (die waarvan het einde is verschoven), daarna de groep `sacrificed`: `speed`, `minimum`, `gap`; en, buiten `counts`, `constraints` — de gebruikte instellingen |
| [`convert`](convert.md) | `subtitles`, daarna wat de conversie heeft verloren: `lost_ends` en `lost_header` (0 of 1), `joined_lines`, `lost_tags`, `lost_fields`, `furthest_ms` |
| [`append`](append.md) | `files`, `subtitles`, `appended`, daarna `lost_ends`, `lost_header`, `joined_lines`, `lost_tags`, `lost_fields`, `furthest_ms` opgeteld; en, buiten `counts`, `inputs` — een element per toegevoegd bestand |
| [`split-file`](split-file.md) | `subtitles`, `head`, `tail`; en, buiten `counts`, `tail` — het pad van de staart (`destination` is de kop) |
| [`pair`](pair.md) | `subtitles` (die van het geschreven bestand); en, buiten `counts`, `alignment` |
| [`inspect`](inspect.md) | geen `counts`: de beschrijving van het bestand, zie zijn pagina |

### Foutidentifiers

Een `error.kind` is er een van deze; **een lezer behandelt een onbekende identifier als
"andere"**.

| `kind` | Wat het is |
| :----- | :----------- |
| `not-found`, `permission-denied`, `io` | het systeem weigerde het bestand te lezen — of te schrijven |
| `unknown-format`, `no-subtitle-found`, `undecodable` | het bestand is gelezen en laat zich niet lezen |
| `unencodable` | een teken dat de gekozen codering niet kan schrijven |
| `no-byte-order-mark`, `no-frame-rate` | wat `convert` niet kan schrijven zonder het te horen |
| `before-the-origin`, `beyond-the-end`, `no-transform`, `no-grid` | een bewerking die niet op dit bestand kan worden toegepast |
| `range-out-of-bounds` | een `--range` die dit bestand niet bevat |
| `no-style` | een formaat dat geen stijl schrijft (TMPlayer, LRC), voor `italics` |
| `refused` | elke andere weigering van een bewerking |

### Wat wordt beloofd, en wat niet

**Beloofd zolang `schema` `1` is**: de namen van de sleutels, hun type en hun eenheid; de betekenis
van `ok`, `file` en `destination`; **een object per invoer, in de volgorde van de invoeren** — een per uitvoering voor de drie subcommando's met een eigen arity;
de `kind`-identifiers die bestaan; determinisme — dezelfde invoeren, dezelfde argumenten,
dezelfde bytes.

**Niet beloofd**: de **tekst van de meldingen** (`message`, `detail`), Engels voor een mens,
die wordt gecorrigeerd wanneer het nodig is — een script dat hem vergelijkt, is bij voorbaat stuk; de **volgorde van
de sleutels** in een object, vast maar niet gegarandeerd door JSON; de aanwezigheid van **sleutels die nog niet
zijn toegevoegd**.

**Een lezer negeert dus een onbekende sleutel**, en behandelt een onbekende `kind` als "andere":
een sleutel of identifier toevoegen verandert `schema` niet. Een sleutel verwijderen of hernoemen,
een type of een eenheid wijzigen, een identifier verwijderen verhoogt het — dat is een breuk,
aangekondigd in de CHANGELOG en hier.

## Niveaus van toelichting

Elk niveau **bevat het vorige**: een stap hoger toevoegen vervangt nooit
iets.

| Niveau | Hoe | Wat de foutuitvoer bevat |
| :----- | :------ | :------------------------------ |
| 0 | `--quiet` | niets, **behalve de fouten** |
| 1 | standaard, of `-v` | een regel per verwerkt bestand, en een samenvatting zodra het er meerdere zijn |
| 2 | `-vv` | en wat is herkend: formaat, codering, BOM, regeleinden |
| 3 | `-vvv` | en het debugspoor: gelezen en geschreven bytes, en **elke diagnostische melding bij het lezen** |

**Fouten worden nooit tot zwijgen gebracht**, `--quiet` inbegrepen. Een commando dat stilzwijgend
mislukt, zou niets dan zijn afsluitcode achterlaten.

De samenvatting verschijnt pas vanaf twee bestanden: bij een enkele invoer zou ze de regel
ervoor herhalen.

## Diagnostische meldingen lezen

Ondertitelformaten worden **zo goed mogelijk** gelezen: tegenover een afwijking stopt de
lezer niet — hij beslist, of laat de zaken zoals ze staan, en zegt dat. Waarop hij
stuitte, wordt uitgevoerd op **niveau 3**, een regel per diagnostische melding, op de foutuitvoer:

```
a.srt: 2 diagnostics while reading
a.srt: line 6: SubRip numbers that do not follow ("7"), settled by the reader
a.srt: line 9: a line that fits nowhere, left as it stands
```

Alle achttien subcommando's melden ze, niet alleen [`inspect`](inspect.md):
een bestand dat zo goed mogelijk is gelezen en daarna teruggeschreven, heeft dezelfde beslissingen doorlopen, en ze stil houden
zou doen geloven dat er niets is gebeurd.

**Een diagnostische melding is nooit een mislukking.** Het bestand is gelezen, het commando
is geslaagd, de afsluitcode is `0`. Daarom staan ze op het meest
gedetailleerde niveau: lager zouden ze de regel begraven die zegt wat er echt is
gedaan.

### Wat elke regel bevat

| Deel | Wat het zegt |
| :----- | :------------- |
| `line N` | waar, vanaf 1 geteld, zoals een editor het toont; **afwezig** voor wat het hele bestand betreft |
| de zin | waarop is gestuit, onder de twaalf categorieën hieronder |
| `("…")` | de foute tekst van het bestand, wanneer de categorie niet volstaat; afgekapt op 80 bytes |
| het einde | wat eraan is gedaan: `settled by the reader`, of `left as it stands` |

**Twee categorieën hebben geen regel**, en het zijn de twee over de
codering: de bytes zijn afgewogen, of hun markering gelezen, voordat ook maar één
regel van het bestand bestond. Hun regel begint dus met de zin, zonder `line N`
ervoor — er is geen plaats om te noemen.

**Het einde van de regel is wat het meest telt.** `settled by the reader` betekent
dat de lezer heeft beslist en dat het geschreven bestand zijn beslissing bevat — een
ontbrekende nummering wordt opnieuw gegenereerd. `left as it stands` betekent dat hij niets
heeft aangeraakt omdat **alleen u kunt beslissen**: een ondertitel die eindigt voordat hij
begint, blijft zoals hij is.

### Het regelnummer van een blok

Een afwijking over **een heel blok** wordt verankerd aan zijn timingregel,
en niet aan de exacte regel die haar veroorzaakt. Een inconsistente nummering
op regel 5 wordt dus gemeld op regel 6, die van de timingregel die
volgt — het foute nummer staat in de `("…")`. Alleen de afwijkingen over
**een regel** — een onleesbare timingregel, tekst vóór de eerste — worden
op zichzelf gemeld.

### De twaalf categorieën

| Zin | Wat het veroorzaakt |
| :----- | :------------------ |
| `a line that fits nowhere` | een regel die in geen enkel blok past |
| `a timing line that could not be read` | een onleesbare timingregel |
| `a SubRip block without its number` | ontbrekende nummering, bij het schrijven opnieuw gegenereerd |
| `SubRip numbers that do not follow` | nummering die springt |
| `text before the first timing line` | tekst vóór de eerste timingregel |
| `a WebVTT block of an unknown kind` | een niet-herkend WebVTT-blok |
| `declares an event column this tool cannot fill` | een gebeurteniskolom van Sub Station Alpha die hier niets vult |
| `counts in frames and states no rate; it was read at` | een bestand in frames, dat geen framerate vermeldt — de gebruikte is het detail |
| `carries no end times; each one was taken from the next start` | een formaat zonder einden, TMPlayer of LRC: **geen enkel einde in de tabel komt uit het bestand** |
| `more than one kind of line ending` | gemengde regeleinden |
| `an encoding nothing declared` | de codering is voorgesteld door de bytes af te wegen, en is niet UTF-8 |
| `a byte order mark that contradicts the encoding asked for` | het bestand bevat een markering, en `--encoding` noemde een andere — de markering wint |

**De drie in het midden zijn die van fase 9**, en de laatste twee van die
drie hebben het niet over een regel maar over het hele bestand: ze zeggen dat een
getoonde waarde niet is gelezen. Een gevulde kolom `End` bij een LRC, een
positie bij een MicroDVD — het bestand bevatte er niets van.

**Een bewerking kan er eigen toevoegen**, wat geen diagnostische melding bij het lezen is:
[`correct`](correct.md#een-patroon-dat-niet-kan-worden-toegepast) zegt, met de ondertitel, het patroon
dat het opgaf. Het wordt gemeld op niveau 1 en draagt, in het object van het bestand, de identifier
`pattern-failed` — nooit een mislukking van het bestand.

## Een vertaling

Een vertaalbestand wordt verwerkt **zoals elk ondertitelbestand**:
`subedit-cli case --to sentence film.fr.srt` wijzigt het zonder iets nieuws. Wat het koppelen toevoegt, en wat
drie opties zeggen, is beperkt: **weten hoe de regels van een vertaling op de
ondertitels van het hoofdbestand vallen** — dat is `inspect -t` —, en **de vertaling wijzigen terwijl ze
op haar hoofdbestand gelegd blijft**, met dezelfde regels als het venster.

| Optie | Waar | Waarde | Standaard |
| :----- | :---- | :----- | :----- |
| `-t`, `--translation-file` | `inspect`, `hearing-impaired`, `replace`, `case`, `italics`, `dialogue-dashes`, `correct`, `pair` | het vertaalbestand | — |
| `--document` | de zes tekstsubcommando's, niet `inspect` noch `pair` | `main` of `translation` | `main` |
| `--align-method` | zoals `-t` | `position` of `number` | `position` |

- **`position`** vergelijkt het midden van elke regel met de grenzen van de ondertitels: een ontbrekende regel verschuift
  de volgende niet. **`number`** stuurt regel *n* naar ondertitel *n*, zonder naar de
  posities te kijken. Positie is de standaardwaarde, om de reden die het venster heeft: een ontbrekende regel in het
  midden verschuift de hele rest van een vertaling die op nummer is gekoppeld.
- **`--document translation` vereist `-t`, en `-t` vereist `--document translation`** bij een tekst-
  subcommando — het ene zonder het andere is een gebruiksfout, code `1`, omdat een gelezen bestand dat geen enkele handeling
  gebruikt, een omissie is en geen voorkeur. `-t` bij `inspect` heeft geen `--document`: het
  meldt. **`--align-method` zonder `-t` is ook een gebruiksfout.**
- **`-t` noemt een bestand, en heeft dus alleen zin voor een enkele invoer**: met meerdere invoeren, of
  met een map die door `--recursive` is genomen, wordt het geweigerd zoals `--output`, **voordat een bestand
  wordt gelezen**. Voer voor een reeks een aanroep per paar uit.
- **Een vertaalbestand dat identiek is aan het hoofdbestand wordt geweigerd** — hoe zijn
  pad ook is geschreven: `<translation>: the file is already open as the main document`, code `2`.
- **Alleen het beoogde document wordt geschreven**, op **zijn eigen pad en in zijn eigen formaat**: het formaat,
  de codering en de regeleinden van het vertaalbestand. `--output` noemt het bestand van de
  vertaling, `--output-dir` plaatst de basisnaam ervan daar, `--in-place` herschrijft het; **het hoofd-
  bestand wordt nooit aangeraakt**. De paden van het rapport zijn die van de vertaling.
- **De ondertitels die het koppelen tot leven brengt** — een regel die niets heeft bereikt — worden in
  de vertaling geschreven met **hun eigen posities**, welke methode ook; **een ondertitel die niemand
  heeft vertaald, wordt geschreven als een blok zonder tekst**, dat zonder afwijking leest.
- **`--encoding` en `--frame-rate` gelden voor beide bestanden.**
- De **positie**-subcommando's — `shift`, `transform`, `framerate`, `snap`, `adjust` — hebben
  deze opties niet: geef ze beide bestanden, zoals bij een reeks,
  `subedit-cli shift --by 2 film.srt film.fr.srt --output-dir out/`.
- **De vertaling opnieuw getimed op het hoofdbestand schrijven** — het enige wat het koppelen doet en een bestand alleen
  niet — is het werk van [`pair`](pair.md), dat noch `--document` noch `--recursive` heeft.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\n[Un oiseau] Bonjour.\n\n2\n00:00:03,000 --> 00:00:04,000\nAu revoir.\n\n' > film.fr.srt; subedit-cli hearing-impaired film.srt -t film.fr.srt --document translation --output propre.fr.srt; cat propre.fr.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\n[Un oiseau] Bonjour.\n\n2\n00:00:03,000 --> 00:00:04,000\nAu revoir.\n\n' > film.fr.srt; subedit-cli hearing-impaired film.srt -t film.fr.srt --document translation --output propre.fr.srt; cat propre.fr.srt
film.fr.srt: 1 subtitle cleaned, 0 removed -> propre.fr.srt
1
00:00:01,000 --> 00:00:02,000
Bonjour.

2
00:00:03,000 --> 00:00:04,000
Au revoir.
```

Met `--format json` bevat het object van elk bestand ook de sleutel `alignment`, die zegt hoe de
vertaling is gelegd — `{"file", "method", "attached", "born", "untranslated", "out_of_order"}`, zoals
de sleutel `translation` van [`inspect`](inspect.md#in-json) — en met **`--dry-run`** somt `changes` op
welke ondertitels van de vertaling, elk met `"document":"translation"`.

## Meerdere bestanden

Elk subcommando accepteert meerdere paden, behalve [`append`](append.md), [`split-file`](split-file.md) en
[`pair`](pair.md), waarvan de arity die van hun pagina is. Elk wordt
onafhankelijk verwerkt: de mislukking van een onderbreekt de andere niet, en de mislukkingen
worden gemeld met vermelding van het bestand en de reden.

**Een map staat voor de bestanden die ze bevat, met `--recursive` (`-r`)**:
zonder deze optie is er een opgeven een gebruiksfout, code `1`, en wordt niets
verwerkt — het weigeren is beter dan niets verwerken en `0` teruggeven:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?" -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?"
films: is a directory: use --recursive
code=1
```

De doorloop, de structuur die onder `--output-dir` behouden blijft en wat terzijde wordt gelaten,
worden beschreven in [Een mappenstructuur verwerken](lots.md).

## De man-pagina's

Een installatie plaatst **een man-pagina per subcommando** naast `subedit-cli(1)`:
`subedit-cli-shift(1)`, `subedit-cli-convert(1)` en de andere, een voor elke rij van
[de tabel met subcommando's](#subcommandos). `subedit-cli(1)` bevat dit hoofdstuk — de algemene opties,
de uitvoeren, de afsluitcodes — en elke andere pagina bevat het hoofdstuk van zijn subcommando, zijn
voorbeelden inbegrepen. De pagina van het venster, `subedit-gui(1)`, staat ernaast.

```console
$ man subedit-cli
$ man subedit-cli-shift
```

**De pagina's worden niet los van deze handleiding geschreven**: ze worden eruit gegenereerd wanneer het
programma wordt gebouwd, dus ze zeggen wat zij zegt, en kunnen er niet van afwijken. De sectie BESTANDEN van elke
pagina noemt de map waarin de handleiding is geïnstalleerd.

| Pagina's | Waar ze zijn geïnstalleerd |
| :---- | :----------------------- |
| Engels | `share/man/man1` |
| elke geïnstalleerde taal — Catalaans, Tsjechisch, Duits, Spaans, Fins, Frans, Galicisch, Hebreeuws, Hongaars, Interlingue, IJslands, Italiaans, Nederlands, Pools, Portugees, Braziliaans Portugees, Russisch, Servisch, Turks, Vereenvoudigd Chinees | `share/man/<language>/man1` — `share/man/fr/man1` |

**`man` kiest de taal uit de landinstelling**, zoals het programma: onder een Franse landinstelling
toont `man subedit-cli` de Franse pagina, en een taal waarvan de pagina's niet zijn geïnstalleerd, krijgt de
Engelse. `-L` vraagt een taal, wat de landinstelling ook is:

```console
$ man -L fr subedit-cli-shift
```

## Afsluitcodes

| Code | Betekenis |
| :--- | :------------ |
| `0` | alles is geslaagd |
| `1` | gebruiksfout — onbekende optie, ongeldige waarde, verboden combinatie |
| `2` | geen enkel bestand kon worden verwerkt |
| `3` | sommige bestanden zijn verwerkt, andere niet |

`2` en `3` worden onderscheiden zodat een script weet hoe het moet handelen zonder de uitvoer opnieuw te lezen:
“niets werkte” en “er ontbreekt er een” vragen niet om dezelfde reactie.

Een gebruiksfout wordt ontdekt **vóór elke verwerking**: ze laat nooit een
reeks half verwerkt achter.

## Fouten

| Wat het veroorzaakt | Wat er op de foutuitvoer wordt geschreven |
| :------------------ | :--------------------------------------- |
| onbekende optie of onbekend subcommando | de foute naam, en een verwijzing naar `--help` |
| `--quiet` met `-v` | `--quiet and -v ask for opposite things; give one or the other` |
| `--config` met `--no-config` | `--config excludes --no-config` |
| `--config` met een lege naam | `--config needs a file name` |
| ontbrekend bestand | `<path>: does not exist` |
| onleesbaar bestand | `<path>: cannot be opened: permission denied` |
| codering die een eigen markering schrijft | `"<name>" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do` |
| bytes die niet decoderen | `<path>: cannot be decoded in the chosen encoding` |
| formaat niet herkend | `<path>: is in no format this tool knows` |
| niets wat op een ondertitel lijkt | `<path>: holds nothing recognisable as a subtitle` |
| `--document translation` zonder `-t` | `--document translation needs the translation file: use -t` |
| `-t` zonder `--document translation` | `-t names a translation to change: use --document translation` |
| `--align-method` zonder `-t` | `--align-method needs a translation file: use -t` |
| `-t` met meerdere invoeren, of een map | `-t names one file but several inputs were given: use one invocation per pair` |
| vertaling identiek aan het hoofdbestand | `<translation>: the file is already open as the main document` |
