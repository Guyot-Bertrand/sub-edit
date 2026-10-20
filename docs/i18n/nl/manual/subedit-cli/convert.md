# `convert`

<!-- machine-translated -->
> Deze pagina is een machinevertaling en is nog niet nagelezen door een lezer van de taal.

```
subedit-cli convert --to <format>
                    [--line-endings unix|windows|mac] [--to-encoding NAME]
                    [--bom | --no-bom] [--sort] [--disjoint]
                    [--join-repeats] [--repair]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    <file>...
```

Schrijft elk bestand in het gevraagde formaat en de gevraagde vorm. **Wijzigt nooit
de invoer**, tenzij `--in-place` er uitdrukkelijk om vraagt.

<!-- example: subedit-cli convert --help -->
```console
$ subedit-cli convert --help
Write a subtitle file out in another format or shape
Usage: subedit-cli convert [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to convert

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --to TEXT:{srt,vtt,subviewer2,ssa,ass,microdvd,mpl2,tmplayer,lrc} REQUIRED
                              Format to write
  --line-endings TEXT:{unix,windows,mac}
                              Line endings to write; the source's by default
  --to-encoding NAME          Encoding to write; the source's by default
  --frame-rate RATE           Frame rate of a file counted in frames: 25, 23.976
  --bom                       Write a byte order mark
  --no-bom                    Write no byte order mark
  --sort                      Put the subtitles in order of their start before writing
  --disjoint                  Cut overlapping subtitles so that no range overlaps another, before writing
  --join-repeats              Join the subtitles that repeat each other into one, before writing
  --repair                    Join the repeats, sort, then make the ranges disjoint, before writing
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
| `--to` | **ja** | een naam uit de tabel hieronder, en niets anders | — |
| `--line-endings` | nee | `unix`, `windows` of `mac` | die van het gelezen bestand |
| `--to-encoding` | nee | elke codering die ICU kan schrijven, behalve die welke hun eigen markering schrijven | die van het gelezen bestand |
| `--bom` / `--no-bom` | nee | vlaggen, die elkaar uitsluiten | wat het gelezen bestand bevatte |
| `--sort` | nee | een vlag: zet de ondertitels **vóór het schrijven** op volgorde van hun begin, zie [Sorteren bij het schrijven](#sorteren-bij-het-schrijven) | uit |
| `--join-repeats` | nee | een vlag: maakt van de opeenvolgende stukken van één tekst één enkele ondertitel, **vóór al het andere**, zie [De bereiken niet-overlappend maken](#de-bereiken-niet-overlappend-maken) | uit |
| `--repair` | nee | een vlag: hetzelfde als `--join-repeats --sort --disjoint`, in die volgorde | uit |
| `--disjoint` | nee | een vlag: knipt de overlappende ondertitels af **vóór het schrijven**, zie [De bereiken niet-overlappend maken](#de-bereiken-niet-overlappend-maken) | uit |
| `--output` / `--output-dir` / `--in-place` | **een van de drie**, behalve met `--dry-run` | zie [Aanroep](invocation.md#de-bestemming) | — |
| `--dry-run` | nee | een vlag: berekent en meldt, schrijft niets — zie [Kijken voor het schrijven](invocation.md#kijken-voor-het-schrijven) | uit |

`mac` noemt de wagenterugloop alleen (`\r`), het regeleinde van het klassieke Mac OS.

## De formaten die `--to` accepteert

| Naam | Formaat | Geschreven extensie |
| :-- | :----- | :--------------- |
| `srt` | SubRip | `.srt` |
| `vtt` | WebVTT | `.vtt` |
| `subviewer2` | SubViewer 2.0 | `.sub` |
| `ssa` | Sub Station Alpha | `.ssa` |
| `ass` | Advanced SSA | `.ass` |
| `mpl2` | MPL2 | `.txt` |
| `microdvd` | MicroDVD | `.sub` |
| `tmplayer` | TMPlayer | `.txt` |
| `lrc` | LRC | `.lrc` |

**Een naam, en geen extensie.** Twee extensies noemen elk twee formaten
— `.sub` is ook die van MicroDVD, `.txt` die van MPL2 en van
TMPlayer — dus een optie die extensies aannam, zou niet weten welk formaat
wordt bedoeld. De namen hierboven zijn ondubbelzinnig, en komen overeen met de
extensie overal waar de extensie ook ondubbelzinnig is.

**De lijst is gesloten.** Het is die van Gaupol, en alle negen staan erin.

## De twee formaten zonder einde

**TMPlayer en LRC schrijven slechts één positie per regel**, het begin. Dat
is aan beide kanten te zien.

**Bij het lezen** worden de einden afgeleid: dat van een regel is het begin van
de volgende, en de laatste krijgt vijf seconden. Dat is wat Gaupol doet.
Wij melden het ook, met een diagnostische melding die `--verbose` toont bij de drie
subcommando's:

```
file.lrc: carries no end times; each one was taken from the next start, settled by the reader
```

**Bij het schrijven** worden de einden niet geschreven, bij gebrek aan een plaats om ze neer te zetten.
Omzetten naar een van deze twee formaten verliest ze dus, en een heen-en-terugreis
vindt ze niet terug — ze worden opnieuw afgeleid.

**LRC verliest nog iets**: het kan geen regeleinde bevatten.
Een regel dialoog over twee regels wordt op één geschreven, de twee regels samengevoegd met
een spatie. Dat verlies kan evenmin worden hersteld.

Geen van beide heeft een woordenschat voor cursief.

## `--frame-rate`, en het ene formaat dat in frames telt

**MicroDVD telt niet in seconden**: zijn posities *zijn* framenummers,
en geen enkel MicroDVD-bestand vermeldt de framerate waarmee ze werden
geteld. `--frame-rate` geeft haar, en dient aan beide kanten: het is de
framerate waarmee een `.sub` in frames wordt gelezen, en die waarmee een
omzetting naar MicroDVD de frames telt.

| Optie | Verplicht | Waarden | Standaard |
| :----- | :----- | :------ | :----- |
| `--frame-rate` | nee | een framerate in frames per seconde: `25`, `23.976`, `29.97` | zie hieronder |

**Bij het lezen, zonder `--frame-rate`, wordt het bestand gelezen op 23.976** — de waarde van
Gaupol — en niveau `-vvv` meldt het:
`counts in frames and states no rate; it was read at`. Het is de enige plaats in het programma waar elke getoonde positie
op een aanname berust die het bestand niet kan bevestigen.

**Bij het schrijven, zonder `--frame-rate`, drie gevallen en drie antwoorden:**

| Het uitgangsbestand | Wat er gebeurt |
| :------------------- | :-------------- |
| is al MicroDVD | de framerate die het las, schrijft het terug, en de frames komen identiek terug |
| is tijdgebaseerd, en zijn posities vallen op een raster | het raster wordt genomen, en `-vv` meldt welk |
| is tijdgebaseerd, zonder raster | **weigering**, code 2: `writing frames needs a frame rate, and the positions fall on no grid to take one from — give --frame-rate` |

De weigering is bewust. Het enige wat overbleef, was een getal verzinnen, en
elke regel van het bestand zou verschuiven met hoezeer dat getal ernaast zat.

## Wat een omzetting verliest

**Omzetten naar een ander formaat is nooit zonder verlies**, en wat niet behouden blijft,
hangt af van het paar:

| Wat er niet doorheen komt | Waarom |
| :--------------------- | :------- |
| de header | hij heeft alleen zin in zijn eigen formaat: `[INFORMATION]` is geen WebVTT-header |
| de formaatspecifieke gegevens | de coördinaten van SubRip, de identificatiecode en de instellingen van een WebVTT-cue hebben elders geen equivalent |
| de precisie | SubViewer 2, de twee Sub Station Alpha's en LRC schrijven op de honderdste, MPL2 op de tiende, TMPlayer op de seconde, MicroDVD op het frame; `01:00:00,017` komt terug als `01:00:00,020` |
| het einde van een regel | TMPlayer en LRC hebben er geen veld voor; de volgende lezing leidt het opnieuw af |
| de regeleinden | alleen LRC: zijn regels passen op één regel, en de twee worden met een spatie aan elkaar geplakt |
| de tags die het doelformaat niet kan schrijven | WebVTT heeft geen `<font color>`, TMPlayer en LRC hebben helemaal niets |
| de opmaak | `{\pos(x,y)}`, `{\an8}`, `<v Marie>`, `<ruby>`: ze overleven een heen-en-terugreis in hun eigen formaat, en verdwijnen bij een omzetting |

**Een bestand in zijn eigen formaat terugschrijven verliest niets** — dat is de garantie
waarop al het andere rust, en `--to srt` op een `.srt` geeft dezelfde bytes terug. Er wordt zelfs
niets gedecodeerd: de tekst blijft de ruwe tekenreeks die is gelezen.

## Tags worden omgezet, niet gekopieerd

**Elk formaat schrijft cursief op zijn eigen manier**, en omzetten zonder om te zetten
zou een `{\i1}` in een `.srt` laten staan: tekst die de gebruiker ziet, in een
bestand dat nooit zal weten hoe het die moet interpreteren. Wat doorkomt, zijn **zes
dingen** — vet, cursief, onderstreept, kleur, lettertype en grootte:

| Formaat | Vet | Cursief | Onderstreept | Kleur | Lettertype | Grootte |
| :----- | :--: | :------: | :------: | :-----: | :----: | :----: |
| SubRip | `<b>` | `<i>` | `<u>` | `<font color="#RRGGBB">` | — | — |
| WebVTT | `<b>` | `<i>` | `<u>` | — | — | — |
| SubViewer 2 | `<b>` | `<i>` | `<u>` | `<font color="#RRGGBB">` | — | — |
| Sub Station Alpha | `{\b1}` | `{\i1}` | — | `{\c&HBBGGRR&}` | `{\fnNAME}` | `{\fsN}` |
| Advanced SSA | `{\b1}` | `{\i1}` | `{\u1}` | `{\c&HBBGGRR&}` | `{\fnNAME}` | `{\fsN}` |
| MicroDVD | `{Y:b}` | `{Y:i}` | `{Y:u}` | `{C:$BBGGRR}` | `{F:NAME}` | `{S:N}` |
| MPL2 | `\` | `/` | `_` | `{C:$BBGGRR}` | `{F:NAME}` | `{S:N}` |
| TMPlayer | — | — | — | — | — | — |
| LRC | — | — | — | — | — | — |

**Een streepje is een aangekondigd verlies.** Een gekleurde `.srt` omzetten naar WebVTT verwijdert
de kleur en meldt dat; het geproduceerde bestand is WebVTT, geen SubRip in vermomming.

**MicroDVD en MPL2 kunnen een deel van een regel niet opmaken.** Hun tags
hebben geen einde: `{y:i}` maakt alles wat volgt schuin tot het einde van de regel, en
`{Y:i}` tot het einde van de ondertitel. Een regel waarvan maar de helft
vet was, wordt dus zonder enig vet geschreven, in plaats van met vet dat overloopt.

## Wat de conversie zegt te hebben verloren

**Ze meldt het als ze verliest, en zwijgt als ze niets verliest.** Een rapport
dat bij elke aanroep verschijnt, is een rapport dat niemand leest.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,500\n<i>sur deux</i>\nlignes\n\n' > a.srt; subedit-cli convert --to lrc --output b.lrc a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,500\n<i>sur deux</i>\nlignes\n\n' > a.srt; subedit-cli convert --to lrc --output b.lrc a.srt
a.srt: 1 subtitle written as LRC -> b.lrc
a.srt: ends are not carried by LRC, line breaks were joined in 1 subtitle, 1 tag dropped
```

De regel verschijnt op het standaardniveau van uitvoerigheid, en op elk niveau erboven.
De items, in deze volgorde:

| Item | Wat het zegt |
| :---- | :----------- |
| `ends are not carried by <format>` | het doelformaat schrijft geen einde |
| `line breaks were joined in N subtitles` | alleen LRC: N ondertitels stonden op meerdere regels |
| `N tags dropped` | N tags konden niet worden geschreven, opmaak inbegrepen |
| `the header was dropped` | de header komt niet over een formaatgrens heen |
| `the <format> fields of N subtitles were dropped` | N ondertitels bevatten gegevens die specifiek zijn voor hun formaat |
| `positions moved by up to N ms` | de grootste afstand waarmee een positie is verschoven |

## De codering wijzigen

**`--to-encoding` kiest de geschreven codering; `--encoding` geeft de codering aan die
wordt gelezen.** Samen coderen de twee een bestand opnieuw:

```console
$ subedit-cli convert --to srt --encoding cp1252 --to-encoding utf-8 \
      --output-dir sortie film.srt
```

Zonder `--to-encoding` wordt het bestand geschreven in **de codering waarin het is gelezen**,
net als met zijn regeleinden en zijn markering: een bestand dat is omgezet zonder een
aanwijzing over de codering, geeft dezelfde tekens terug, met dezelfde bytes.

**Een teken dat de doelcodering niet kan schrijven, stopt het bestand**, en
er wordt niets geschreven. Een `ł` heeft geen plaats in Latin-1; het vervangen door een `?`
zou tekst verliezen tussen lezen en schrijven, zonder dat iemand het ziet.

**Een bytevolgordemarkering bestaat alleen voor de Unicode-coderingen.**
`--bom` met een `--to-encoding` die er geen heeft, wordt geweigerd in plaats van genegeerd
— zie de tabel met fouten hieronder.

**En de codering moet haar bytevolgorde noemen.** `UTF-16` en `UTF-32` zijn
namen die ICU kent, en waarvan de converter **een eigen markering** schrijft, wat er ook
wordt gevraagd: `--no-bom` zou dan worden geaccepteerd en genegeerd. Beide worden
dus geweigerd, en de melding noemt de uitvoer:

```console
$ subedit-cli convert --to srt --to-encoding UTF-16 --output copie.srt film.srt
"UTF-16" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do
```

`UTF-16LE`, `UTF-16BE`, `UTF-32LE` en `UTF-32BE` schrijven precies dezelfde
bytes, markering inbegrepen — maar onder het beheer van `--bom` en `--no-bom`. De
weigering kost dus niets dan een nauwkeuriger naam, en ze geldt voor lezen
evenals voor schrijven: `--encoding UTF-16` krijgt hetzelfde antwoord.

## De bestemming

De drie opties en hun regels zijn gemeenschappelijk voor de subcommando's die
schrijven: zie [Aanroep](invocation.md#de-bestemming). Zonder een van hen
wordt er niets geschreven.

Wat specifiek is voor `convert`: **`--output-dir` wijzigt de extensie met het
formaat.** `convert --to vtt` op `a.srt` schrijft `a.vtt`, nooit een `a.srt` die
WebVTT bevat — een bestand waarvan de naam liegt, brengt elk ander programma in verwarring.

## Ter plaatse omzetten wordt geweigerd

`--in-place` met een `--to` dat het formaat wijzigt, geeft code `1` en schrijft
niets: ter plaatse is er geen tweede naam om het nieuwe formaat te dragen, en
het bestand zou blijven staan onder een extensie die de inhoud niet meer rechtvaardigt.

`--in-place` kan nog altijd worden gebruikt om **de vorm te wijzigen zonder het formaat** —
bijvoorbeeld een `.srt` als `.srt` terugschrijven met andere regeleinden.

De weigering wordt beslist op **de extensie alleen**, vóór elke lezing: een gebruiksfout
mag nooit een reeks half geschreven achterlaten.

## Regeleinden en bytevolgordemarkering

Standaard **neemt het geschreven bestand over wat het gelezen bestand bevatte**. Het model
bewaart beide bij het lezen; ze standaard opleggen zou bij elke
omzetting een bewust bewaard stukje informatie verliezen.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli convert --to vtt --output b.vtt a.srt; cat b.vtt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli convert --to vtt --output b.vtt a.srt; cat b.vtt
a.srt: 1 subtitle written as WebVTT -> b.vtt
WEBVTT

00:01.000 --> 00:03.000
Bonjour.
```

## Sorteren bij het schrijven

`--sort` schrijft het bestand **op volgorde van de begintijden**, wat het doelformaat ook is. Het sorteren gebeurt **op een kopie**: het gelezen bestand wordt
niet aangeraakt, en wat wordt geschreven, is wat [`sort`](sort.md) ervan zou hebben gemaakt — stabiel, twee ondertitels die samen beginnen, behouden hun volgorde.

**Nooit standaard**: de volgorde van een bestand is die van zijn auteur zolang er niets anders wordt gevraagd. Met de optie meldt elk bestand het op het
eerste niveau van de toelichting, `<path>: N subtitles moved` of `<path>: already in order`, en het `--format json`-record bevat een
telling `moved` — **afwezig zonder de optie**, om de vorm van wat bestaande scripts lezen niet te wijzigen.

## De bereiken niet-overlappend maken

`--disjoint` knipt de overlappende ondertitels af zodat geen bereik nog een ander overlapt, **op dezelfde kopie als het sorteren**: het gelezen bestand wordt niet aangeraakt.

Nooit standaard. De optie doet wat [`disjoint`](disjoint.md) doet, **met zijn standaardinstellingen**, vóór het schrijven, na het sorteren. Ze heeft een geordend bestand nodig: anders wordt ze geweigerd, en `--sort` — dat eerst komt — zet het vooraf op volgorde. Het bestand meldt het op het eerste niveau van de toelichting (`<path>: N subtitles split, M merged`), en het `--format json`-record bevat de tellingen `split` en `merged`, **afwezig zonder de optie**.

**`--join-repeats` komt vóór het sorteren en het knippen**: een tekst die in opeenvolgende stukken is geknipt, wordt eerst weer opgebouwd (zie
[`join-repeats`](join-repeats.md)), daarna worden de ondertitels gesorteerd, daarna worden de bereiken niet-overlappend gemaakt. **`--repair` is alle drie tegelijk**,
zoals [`repair`](repair.md), met de standaardinstellingen van het knippen. Met `--join-repeats` meldt elk bestand
`<path>: N repeated subtitles joined` op het eerste niveau van de toelichting, en het `--format json`-record bevat `counts.joined` — **afwezig zonder de optie**.

## Uitvoer

**Standaarduitvoer** — niets: het resultaat is het geschreven bestand. Met `--dry-run` ook niets: er is geen bestand, en de toelichtingsregel meldt het — zie [Kijken voor het schrijven](invocation.md#kijken-voor-het-schrijven).

Op de foutuitvoer, naargelang het niveau:

| Niveau | Wat er wordt toegevoegd |
| :----- | :-------------- |
| 1 | `<path>: N subtitles written as WebVTT -> <destination>` |
| 2 | `<path>: SubRip -> WebVTT, UTF-8, no BOM, LF line endings` — de genoemde codering is die welke is geschreven |
| 3 | `<path>: N bytes read, M written`, daarna **elke leesmelding** — zie [Aanroep](invocation.md#diagnostische-meldingen-lezen) |

### In JSON

Met [`--format json`](invocation.md#uitvoer-die-een-script-kan-lezen) bevat het object van elk bestand `counts.subtitles`, daarna wat de omzetting heeft verloren: `lost_ends` en `lost_header` (`0` of `1`), `joined_lines`, `lost_tags`, `lost_fields` en `furthest_ms` — de items van [de zin over het verlies](#wat-de-conversie-zegt-te-hebben-verloren). De volledige tabel is die van [Aanroep](invocation.md#wat-elk-subcommando-in-counts-zet).

## Afsluitcodes

Die van het programma: `0` als elk bestand is geschreven, `2` als geen enkel, `3` als
slechts sommige, `1` bij een gebruiksfout.

## Fouten

| Wat het veroorzaakt | Melding |
| :------------------ | :------ |
| `--bom` met `--no-bom` | `--bom and --no-bom ask for opposite things; give one or the other` |
| `--in-place` dat het formaat wijzigt | `--in-place cannot change the format: the file would keep a name its content no longer matches` |
| `--bom` op een codering die er geen heeft | `<path>: <encoding> has no byte order mark to write` |
| `--to-encoding` die een onbekende codering noemt | `no encoding is named "<name>"` |
| `--to-encoding` die een codering noemt die haar eigen markering schrijft | `"<name>" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do` |
| teken dat in de geschreven codering ontbreekt | `<path>: holds a character the chosen encoding cannot write` |

**`--bom` weigeren op een codering die er geen heeft, is geen
pedanterie.** Een bytevolgordemarkering bestaat voor de Unicode-coderingen en
voor geen andere: er een vragen voor een bestand in Windows-1252 is vragen om
iets wat niet bestaat. Het bestand zou zonder worden geschreven, en niemand zou
weten dat het verzoek niet is ingewilligd.

Die van de bestemming zijn gemeenschappelijk voor de subcommando's die schrijven:
zie [Aanroep](invocation.md#de-bestemming).

De leesfouten zijn die van [`inspect`](inspect.md): ze betreffen
het invoerbestand en hebben dezelfde meldingen.
