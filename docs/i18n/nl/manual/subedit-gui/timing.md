# Timing vanaf de video

<!-- machine-translated -->
> Deze pagina is een machinevertaling en is nog niet nagelezen door een lezer van de taal.

*Verondersteld wordt dat de film is geopend en de speler bekend is: zie [De speler](player.md).*

U kijkt naar de film, stopt op het frame waar een regel moet beginnen, en **die positie wordt
het begin van de ondertitel**. Vijf gebaren, in het menu **Video**, onder de namen die Gaupol ze geeft. Ze
steunen op wat al bestaat: een positie instellen is **hetzelfde commando als de waarde in de
cel typen**, en invoegen is **hetzelfde als `Aanpassen ▸ Ondertitels invoegen…`**.

| Commando | Sneltoets | Wat het doet |
| :------- | :-------- | :----------- |
| Video ▸ Begin vanaf videopositie instellen | `Alt+U` | stelt het **begin** van de geselecteerde ondertitel in op de positie van de film |
| Video ▸ Einde vanaf videopositie instellen | `Alt+K` | stelt het **einde** ervan in op de positie van de film |
| Video ▸ Ondertitel invoegen op videopositie | `Alt+J` | voegt een ondertitel in die **begint** op de positie, en selecteert die |
| Video ▸ Vorige selecteren vanaf videopositie | `Alt+Up` | selecteert de laatste ondertitel die **vóór** de positie **begint** |
| Video ▸ Volgende selecteren vanaf videopositie | `Alt+Down` | selecteert de eerste die **na** de positie **begint** |

**Alle zijn uitgeschakeld zonder geopende video**, zoals de gebaren van de speler. `Set Start…` en `Set End…`
zijn ook uitgeschakeld zolang niets is geselecteerd: ze werken op de **eerste regel** van de
selectie.

**De sneltoetsen zijn die van Gaupol, met `Alt`**: daar zijn het `U`, `K` en `J` — losse letters, die
het venster eerder zou opvangen dan de cel die wordt bewerkt, zodat een `k` typen in een
tekst een einde zou instellen. `Select Previous…` en `Select Next…` zijn `Ctrl+Y` en `Ctrl+U` in Gaupol; `Ctrl+Y`
is op sommige systemen een *opnieuw uitvoeren*, en de pijltjes met `Alt` zeggen *vóór* en *na* beter dan
twee letters. Een cel die wordt bewerkt, wordt **bevestigd** vóór `Set Start…`, `Set End…` of
`Insert Subtitle…`, zoals vóór elke wijziging.

## Een rand instellen

- **Eén geschiedenisitem per gebaar**: `Aanpassen ▸ Ongedaan maken` geeft de vorige positie terug, en niets anders.
- **Dezelfde regels als de cel.** Een begin dat na het einde van de ondertitel wordt ingesteld, of een einde vóór zijn
  begin, **wordt niet geweigerd**: het wordt ingesteld, en de tabel markeert de ondertitel zoals ze die markeert
  waarvan de waarde met de hand is getypt — zie [De afwijkingen](table.md#de-afwijkingen). Er wordt niets stilzwijgend gecorrigeerd.
- **Een positie die al die van de rand is, doet niets**, en voegt niets toe aan de geschiedenis.
- **De positie is altijd het begin van een frame**: de speler meldt nooit een positie
  binnen een frame. Een rand instellen en er dan naar terugkeren — bijvoorbeeld met `Seek Selection Start` —
  **toont hetzelfde frame**.

## Invoegen op de positie

De nieuwe ondertitel **begint op de positie**, en duurt **drie seconden**, of tot het begin van de volgende
wanneer die eerder komt. De tekst is leeg. Ze neemt haar plaats in de volgorde van het bestand in: na
alle ondertitels die op die positie of ervóór beginnen. Het invoegen is **één geschiedenisitem**,
en de ingevoegde ondertitel is geselecteerd: u kunt meteen typen.

## Selecteren vanaf de positie

Beide volgen de volgorde van het bestand, niet de huidige selectie. **Zonder buur aan die kant** gaat de
selectie naar de laatste ondertitel van het bestand voor `Select Next…`, en naar de eerste voor
`Select Previous…` — het gedrag van Gaupol. Deze twee gebaren verplaatsen de film niet.

## Frame voor frame

Vooruit en terug **met één frame** — of met N, zie hieronder —, om een rand **op het exacte frame** in te stellen.
Twee gebaren, en twee knoppen op de afspeelbalk, aan weerszijden van de afspeelknop:

| Commando | Sneltoets | Wat het doet |
| :------- | :-------- | :----------- |
| Video ▸ Stap terug | `Alt+Left` | zet de film terug met de framestap |
| Video ▸ Stap vooruit | `Alt+Right` | zet hem vooruit met de framestap |

- **De stap is een aantal frames, minstens één**: het is de instelling `Frame step` van de
  [instellingen](preferences.md#de-speler). Een stap van N frames toont het frame **N verder**, bij 25 frames per
  seconde zoals bij 23,976; ze wordt **nooit in milliseconden uitgedrukt**.
- **De film stopt aan zijn grenzen**: terugstappen vanaf het eerste frame laat hem daar, vooruitstappen vanaf het laatste
  ook. Het afspelen wordt na een stap gestopt.
- **De knoppen van de balk** — twee pictogrammen, aan weerszijden van afspelen/pauzeren — **activeren dezelfde acties als het menu**: dezelfde stap, uitgeschakeld
  zonder video zoals de items van het menu, en dezelfde knopinfo, die de sneltoets vermeldt. **Een vinger die op een
  knop blijft, of op de toets, laat de stappen op elkaar volgen.**
- **Eén stap tegelijk.** Een ingedrukt gehouden toets herhaalt sneller dan een stap wordt gezet: de eerste herhaling gaat meteen
  weg, de herhalingen die binnenkomen terwijl de stap wordt gezet **stapelen zich niet op** — ze delen één plaats, de
  laatste, die daarna wordt verzonden. De toets loslaten stopt het beeld dus waar u bent, en laat de film niet
  op zijn elan doorlopen.

## Een rand een frame verschuiven

Om een rand met het toetsenbord bij te stellen: **het begin of het einde van de geselecteerde ondertitel**, met één stap, eerder of later.

| Commando | Sneltoets | Wat het doet |
| :------- | :-------- | :----------- |
| Video ▸ Begin iets vervroegen | `Alt+Q` | de ondertitel **begint een stap eerder** |
| Video ▸ Begin iets uitstellen | `Alt+W` | ze **begint een stap later** |
| Video ▸ Einde iets vervroegen | `Alt+Shift+Q` | ze **eindigt een stap eerder** |
| Video ▸ Einde iets uitstellen | `Alt+Shift+W` | ze **eindigt een stap later** |

`Q` en `W` zijn die van Gaupol voor *eerder* en *later*, met `Alt` om de hierboven genoemde reden; het einde
neemt `Alt+Shift`, omdat de `E` van Gaupol hier de letter van `Edit` is.

- **Eén geschiedenisitem per verschuiving**, ongedaan te maken. De naburige ondertitel en de overlap worden gemeld
  **zoals bij het typen**: de rand wordt ingesteld, en de tabel markeert wat niet meer klopt — zie
  [De afwijkingen](table.md#de-afwijkingen).
- **Dezelfde stap als de speler**, dus standaard één frame. **Wat een frame waard is**, wordt gelezen, in deze volgorde,
  uit **de framesnelheid die de video opgeeft**, die van het **document wanneer het in frames telt** (MicroDVD), en daarna uit
  **het raster dat de posities tekenen**. **Bij geen van de drie weigert het gebaar en zegt dat**: een
  willekeurige framesnelheid kiezen zou elke rand verschuiven met iets wat geen frame is.
- **Het vereist geen video**, alleen een selectie: het wordt ook gebruikt op een bestand dat in frames telt, zonder
  film. Wanneer een film is geopend, **gaat het afspelen naar de nieuwe positie**, om het frame te tonen dat de rand
  zojuist heeft aangenomen.
- Een rand gaat niet vóór de oorsprong: met één stap terug vanaf minder dan een frame komt hij op nul.
