# De gekoppelde video

<!-- machine-translated -->
> Deze pagina is een machinevertaling en is nog niet nagelezen door een lezer van de taal.

Het menu **Video** bevat eerst drie commando's, en daarna de gebaren die de film aansturen
— zie [De speler](player.md#door-de-film-navigeren).

| Commando | Sneltoets | Wat het doet |
| :------- | :-------- | :----------- |
| Video kiezen… | geen | kiest het videobestand waarbij het document hoort |
| Audio transcriberen… | geen | transcribeert de audio van de film tot ondertitels die die van het project vervangen — zie [De audio transcriberen](#de-audio-transcriberen) |
| Afspelen / Pauzeren | `Ctrl+P` | speelt de video af of stopt haar — zie [De speler](player.md#afspelen-en-stoppen) |

De gekoppelde video is **een eigenschap van het geopende document**, geen instelling van
het programma: een ander ondertitelbestand openen begint opnieuw. Ze
wordt in geen enkel bestand geschreven en verdwijnt met het venster.

## Het audiospoor

Een film kan meerdere geluidssporen bevatten — de originele versie, een nasynchronisatie, een commentaar. Het
submenu **`Video ▸ Audio ▸ Taal`** toont ze, **één item per spoor**, en **markeert het spoor dat wordt afgespeeld**:

| Wat het bestand geeft | Wat het item zegt |
| :-------------------- | :---------------- |
| een taal en een titel | `1: fra — Original` |
| alleen een taal | `2: eng` |
| alleen een titel | `3: Commentary` |
| geen van beide | `1` |

Het nummer is **de plaats van het spoor in de film**, telkens één verder, in de volgorde van het bestand: twee sporen
kunnen `eng` zeggen, en een bestand dat over zijn labels zwijgt heeft toch een eerste en een tweede.

**Een item kiezen speelt dat spoor af.** De markering is het antwoord van de speler, niet de klik: ze geeft aan
welk spoor wordt afgespeeld.

- **Het submenu is uitgeschakeld** zonder video of zonder audiospoor. Met één enkel spoor blijft het ingeschakeld en
  toont het dat spoor, aangevinkt: het zegt in welke taal de film is. De twee volumegebaren, die ernaast staan, blijven
  in elk geval ingeschakeld: zie [Het volume](player.md#het-volume).
- **Het spoor wordt niet onthouden** van de ene video naar de volgende, noch van de ene sessie naar de volgende: een spoornummer is
  dat van zijn bestand, en het overnemen naar de volgende film zou kiezen wat daar toevallig dat nummer draagt.
  Teruggaan naar een tabblad heropent zijn film op het spoor dat de speler zelf kiest.
- Geen sneltoets: u wisselt zelden van taal, en het menu volstaat.

## Een video kiezen

Het dialoogvenster opent in de map van het ondertitelbestand en filtert op
de herkende video-extensies:

    .avi .flv .m2ts .mkv .mov .mp4 .ogg .ogv .vob .webm

Het biedt ook aan alle bestanden te tonen. Het dialoogvenster annuleren verandert
niets.

Het gekozen bestand wordt **meteen geopend**: het beeld verschijnt in het venster,
gepauzeerd op zijn eerste moment. Zie [De speler](player.md).

**Een videobestand op het venster neerzetten doet hetzelfde** — zie
[Bestanden op het venster neerzetten](files.md#bestanden-op-het-venster-neerzetten).

**De video is die van het project van het huidige tabblad.** Elk tabblad houdt
de zijne; van tabblad wisselen wisselt van film. **Teruggaan naar een tabblad hervat zijn
film waar u hem hebt achtergelaten**, gepauzeerd, zoals bij elke opening. Een andere film
die in de tussentijd voor dat project is gekozen, begint van zijn kant vanaf het begin.

## Het automatische voorstel

Wanneer een ondertitelbestand wordt geopend, **als er geen video is gekozen**,
zoekt het venster er een in dezelfde map: een videobestand waarvan de naam
zonder extensie een voorvoegsel is van de onze, tot aan een punt.

| Ondertitelbestand | Gevonden video | Waarom |
| :---------------- | :------------- | :----- |
| `film.fr.srt` | `film.mkv` | `film` is een voorvoegsel van `film.fr` |
| `film.en.forced.srt` | `film.mkv` | idem, ongeacht het aantal segmenten |
| `film.fr.srt` | `film.fr.mkv` in plaats van `film.mkv` | de langste naam wint |
| `film.fr.srt` | geen, als `film.mkv` **en** `film.mp4` er zijn | twee kandidaten zijn niet van elkaar te onderscheiden |
| `film.fr.srt` | geen, als alleen `fil.mkv` er is | `fil` eindigt niet op een punt |

**Een voorstel is geen keuze.** Het laat zich vervangen door een ander
voorstel — een ander bestand openen, onder een andere naam opslaan. Een uitdrukkelijke
keuze daarentegen wordt nooit overschreven: zodra `Select Video…` is gebruikt, komt het
automatische voorstel er niet meer achteraan.

Dat maakt de gok aanvaardbaar: ze kan worden gecorrigeerd, en haar
correctie blijft staan.

## Wat de statusbalk toont

Rechts op de statusbalk zegt een regel waarbij het document hoort. Ze
is de laatste van drie: de [codering](files.md#de-codering-in-de-statusbalk)
van het gelezen bestand komt eerst, het [framerooster](frame-grid.md) dat uit de
posities wordt afgeleid daarna.

| Situatie | Wat er wordt geschreven |
| :------- | :---------------------- |
| een video is gekoppeld | `Video: film.mkv` |
| … en `ffprobe` geeft haar framesnelheid | `Video: film.mkv, 24000/1001 fps` |
| geen | `No video` |

**De framesnelheid hoort bij de film in plaats van ernaast te staan.** Ze vormen één feit — waarbij
dit document hoort — en een derde vermelding zou haar op dezelfde rang plaatsen als
het [raster dat uit de posities wordt afgeleid](frame-grid.md), wat een ander feit is.

## De audio transcriberen

**`Transcribe Audio…`** schrijft de gesproken tekst van de film als ondertitels. Het is uitgeschakeld zolang er geen film
is geopend: het is de audio van **de film van het project**, nooit een bestand dat u zou kiezen — om een
audiobestand te transcriberen gebruikt u [`transcribe`](../subedit-cli/transcribe.md) op de opdrachtregel.

Het dialoogvenster vraagt om drie dingen en zegt of het kan beginnen:

- **het audiospoor**, opgesomd zoals het menu [`Audio`](#het-audiospoor) ze opsomt, met de taal
  en de titel die het bestand opgeeft;
- **de taal**, twee letters zoals `en` of `fr` (`fra` wordt als `fr` gelezen). Ze begint met de taal van het gekozen spoor
  wanneer het bestand er een opgeeft, volgt het spoor zolang u er geen hebt getypt, en
  **een regel onder het veld zegt waar ze vandaan komt**. Geeft het bestand geen taal op voor het spoor,
  dan zegt de regel dat, en waarschuwt: het model zal ze dan detecteren uit de eerste dertig seconden geluid, wat alleen een
  meertalig model kan, en **een film die opent met muziek of stilte kan voor een andere taal worden aangezien en zijn spraak
  dan in die taal vertaald**. Typ de taal als u ze kent. Bij een model dat alleen Engels kent staat het veld vast op `en` en kan
  het niet worden gewijzigd, en de regel zegt dat de film als Engels uitkomt, welke taal hij ook spreekt;
- **het deel van de film**, van de ene tijd tot de andere — zie [Eerst een deel proberen](#eerst-een-deel-proberen). Beide velden
  leeg is de hele film.

**Het zegt ook wat er ontbreekt.** Drie regels zeggen of de engine, `ffmpeg` en het model er zijn, en
waar; als er een ontbreekt, blijft `OK` uitgeschakeld en zegt het dialoogvenster dat met de verwijzing naar
[De transcriptiehulpmiddelen installeren](../subedit-cli/install-transcription.md). Een knop `Preferences…` opent
de [instellingen](preferences.md#transcriptie), waar het programma en het model worden ingesteld — het zijn feiten
over de machine, geen keuzes die bij elke transcriptie opnieuw moeten worden gemaakt — en het dialoogvenster kijkt opnieuw wanneer ze sluiten.

De taal wordt voor de volgende keer bewaard. De engine zelf en de grenzen van de ondertitels zijn
[instellingen](preferences.md) (`transcription.*`).

**Wanneer het project al ondertitels bevat, vraagt het venster of het ze mag vervangen**, en zegt het hoeveel
er verdwijnen. Het vraagt vóór het werk, zodat weigeren niets kost. Wanneer het project leeg is, valt er
niets te vragen.

Het werk draait **op de achtergrond**, met een venstertje en een knop `Cancel`; het venster blijft eromheen bruikbaar. **Het venstertje
zegt hoe ver het werk is**, zodat u weet dat het leeft en hoe lang het zal duren:

- **de stap**: eerst “De audio uitpakken…” — het venstertje is dan bezig, zonder getal, want uitpakken zegt niets over hoe ver het
  is —, dan “De audio transcriberen…”, met een balk;
- **de resterende tijd**, “nog ongeveer 38 min”: eerst afwezig, getoond zodra de engine genoeg voortgang heeft gemeld om uit te extrapoleren, en
  berekend uit zijn snelheid sinds het begin, tot de laatste minuut; het is een nauwkeurigheid van enkele minuten, geen belofte;
- **de verstreken tijd**, “verstreken: 0:05:12”, die doorloopt ook als niets anders beweegt;
- **de positie**, “0:12:30 van 0:50:00”: hoe ver in de film de engine is, zodra hij zijn eerste tekst heeft geproduceerd, getoond **net boven de laatste tekst** waarbij ze hoort;
- **de laatste tekst** die is verkregen, zodat u kunt zien dat het werk zinvol is.

Bij een lange film met een groot model op een processor kunnen de eerste minuten lang zijn: de engine laadt het model en werkt aan zijn eerste
vensters van dertig seconden. De positie en de laatste tekst bewegen voordat het eigen procent van de engine dat doet, dat alleen in stappen
van 5 % van de film komt. Wanneer het klaar is, worden de ondertitels van het project vervangen
door de transcriptie **in één bewerking**: één `Undo` geeft terug wat er stond, vertaalteksten
inbegrepen. Een transcriptie die mislukt, of die u annuleert, verandert niets — de reden wordt
gemeld in een venstertje, behalve bij een annulering, die alleen in de statusbalk meldt dat ze is geannuleerd.

**Wanneer het grootste deel van wat de engine heeft geschreven notities tussen haakjes zijn** — `[MUSIC PLAYING]`, `[SPEAKING FRENCH]` —, zegt een venstertje
dat na het werk: de gebruikte taal is waarschijnlijk niet de gesproken taal, en de engine heeft de spraak mogelijk vertaald.
Voer het opnieuw uit met de taal ingevuld. De ondertitels staan toch in het project, en één `Undo` geeft terug wat er stond.

### Eerst een deel proberen

Met een bereik kunt u **een model of een taal op twee minuten beoordelen voordat u zich aan twee uur verbindt**. De velden “Deel van de film” nemen een begin en een einde, elk geschreven als `2:00`, `1:02:03`, `2:00.5` of een aantal
seconden, `90`; een van beide alleen is toegestaan (van daar tot het einde, of van het begin tot daar). `OK` blijft uitgeschakeld zolang een veld
geen tijd is of het einde niet na het begin komt, en een regel onder de velden zegt welke.

- **De ondertitels behouden de tijd van de film**: wat op 2:10 wordt gezegd, staat in het resultaat op `0:02:10`.
- **Het vervangt de ondertitels van het project zoals elke transcriptie**, na dezelfde vraag als er ondertitels zijn, en één
  `Undo` geeft ze terug. Om een bereik te proberen zonder uw werk aan te raken, bewaar eerst, of werk op een kopie.

**Wat het nodig heeft wordt niet meegeleverd**: een engine (whisper.cpp), een model en `ffmpeg` om de audio uit te pakken.
Zie [De transcriptiehulpmiddelen installeren](../subedit-cli/install-transcription.md).

## Wanneer er geen film is

Het venster toont een knop **`Select Video…`** waar het beeld zou komen, in plaats van
helemaal niets. Het is hetzelfde commando als dat van het menu `Video`, binnen
handbereik van de muis: een afwezigheid waar u niets aan kunt doen, is moeilijk te onderscheiden van
een fout.

De knop verdwijnt zodra een film is geopend, en het beeld neemt **al** zijn
plaats in: de hoogte die de tabel erboven laat — die welke
[`window.table-share`](preferences.md) van de ene sessie naar de volgende heeft onthouden —, en
nooit minder dan de minimale hoogte van het beeld, waaraan de tabel zo nodig toegeeft.
Wanneer de film verdwijnt, neemt de knop dezelfde plaats weer in.

**De naam van het bestand, nooit zijn pad.** Een pad van tweehonderd tekens
zou al het andere uit de balk duwen; het volledige pad is dat wat het
dialoogvenster toonde.

## `ffmpeg` is niet vereist

`subedit-gui` raadpleegt `ffprobe` — meegeleverd met `ffmpeg` — voor één ding en
niets anders: **de framesnelheid die de container opgeeft**. Ze wordt één keer
per film gelezen, wanneer die wordt gekoppeld.

| Met `ffprobe` | Zonder |
| :------------ | :----- |
| de statusbalk schrijft de framesnelheid naast de naam van de film | ze schrijft alleen de naam |
| het conversiedialoogvenster stelt de framesnelheid van de film voor in het onderste veld | het stelt niets voor, en opent zoals voorheen |
| het dialoogvenster voor de uitlijning opent erop | het opent op de framesnelheid van het project |

**Niets anders hangt ervan af, en geen enkele bewerking weigert.** De speler heeft
`ffprobe` helemaal niet nodig: hij steunt op `libmpv`, dat een
afhankelijkheid bij het bouwen is. De duur van de film, die waarmee wordt gemeld wat
voorbij het einde komt, komt van de speler en niet van `ffprobe`.

**De transcriptie is het enige dat wel `ffmpeg` nodig heeft**: het haalt de audio van de film eruit. Zie
[De transcriptiehulpmiddelen installeren](../subedit-cli/install-transcription.md).

Het is dus geen verminderde modus maar **de normale werking** van een machine
waarop `ffmpeg` nooit is geïnstalleerd: het gegeven komt van de gebruiker, en `ffprobe`
stelt het voor wanneer het aanwezig is.

Een gelezen framesnelheid is een **exacte breuk** — `24000/1001`, niet `23.976`.
Daarom wordt `ffprobe` voor dat antwoord behouden: de
speler kent hetzelfde gegeven als een kommagetal, en een benaderde framesnelheid
zou het hele bestand een beetje verkeerd opnieuw timen.
