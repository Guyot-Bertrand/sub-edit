# Teksten knippen, kopiëren en plakken

<!-- machine-translated -->
> Deze pagina is een machinevertaling en is nog niet nagelezen door een lezer van de taal.

Drie items van het menu `Edit`, onder `Undo` en `Redo`, en boven
`Find and Replace…`.

| Item | Sneltoets | Wat het doet |
| :--- | :-------- | :----------- |
| Teksten knippen | `Ctrl+X` | kopieert de teksten van de selectie en maakt ze daarna leeg |
| Teksten kopiëren | `Ctrl+C` | kopieert de teksten van de selectie |
| Teksten plakken | `Ctrl+V` | schrijft de gekopieerde teksten vanaf de eerste geselecteerde regel |

**Alleen teksten reizen mee, en niets anders**: geen begin, geen einde, geen hele
regel. Plakken verschuift dus geen enkele positie — de timing van de regel die
een tekst ontvangt, blijft de zijne. Dat is wat Gaupol doet.

## Waarop ze inwerken

**Met een vertaling, de tekst van de huidige kolom**: de vertaling als de
huidige cel in de kolom `Translation` staat, elders de hoofdtekst —
zie [De tabel](table.md#de-tekst-waarop-de-bewerkingen-zich-richten). `Cut Texts` maakt dus de
vertaling leeg en laat de hoofdtekst staan, en `Paste Texts` schrijft in de
kolom waarin u zich bevindt. **Het klembord bevat geen document**: u
kunt een cel van de hoofdtekst kopiëren en in de vertaling plakken, en
andersom.

**De selectie, en nooit het hele bestand.** De drie items zijn uitgeschakeld zolang
niets is geselecteerd. Elders in het venster betekent “niets geselecteerd”
“het hele bestand”; hier zou een ongelukkige `Ctrl+X` alle teksten
van het document leegmaken, en zou een plakbewerking geen regel hebben om mee te beginnen.

Zolang een cel geopend is, horen `Ctrl+X`, `Ctrl+C` en `Ctrl+V` bij
het invoerveld: ze knippen, kopiëren en plakken tekens, net zoals `Del`
daar een teken wist en geen ondertitel. Gestart vanuit het menu bevestigen de items eerst
de invoer: `Copy Texts` kopieert dan de tekst zoals die zojuist is
getypt. Zie [Een cel bewerken](editing.md).

## Kopiëren

`Copy Texts` kopieert de tekst van elke geselecteerde regel, van de eerste tot de
laatste. **Een niet-aaneengesloten selectie behoudt haar vorm**: een regel die buiten
de selectie is gelaten, wordt een gat, en het plakken overschrijft hem niet.

**Een geselecteerde regel waarvan de tekst leeg is, wordt ook een gat**:
geplakt laat ze de doelregel zoals hij is. Eén enkele lege tekst kopiëren
plakt dus niets.

Het systeemklembord ontvangt deze teksten **als platte tekst, gescheiden door een
lege regel** — een gat wordt daar als lege tekst geschreven, en daarom kan een lege
tekst niets anders zijn dan een gat: het plakken geeft hetzelfde resultaat
of de kopie nu uit dit venster komt of uit een ander programma. Wat u hier kopieert, kan
dus in elk programma worden geplakt, en wat u elders kopieert, kan hier worden geplakt.

Kopiëren wijzigt het document niet en komt niet in de geschiedenis.

## Knippen

`Cut Texts` kopieert en **maakt daarna de teksten leeg** van de geselecteerde regels. De regels
blijven, met hun posities: knippen is niet verwijderen — om regels te
verwijderen is er [`Remove Subtitles`](lines.md#verwijderen).

Teksten knippen die al allemaal leeg zijn, doet niets en komt niet in de
geschiedenis.

## Plakken

`Paste Texts` schrijft de teksten van het klembord **vanaf de eerste geselecteerde
regel**, één regel per tekst, naar beneden.

| Wat het klembord bevat | Wat er gebeurt |
| :--------------------- | :------------- |
| een tekst | ze vervangt die van de regel |
| een gat, of een gekopieerde lege tekst | de regel behoudt zijn tekst |
| meer teksten dan er regels over zijn | aan het einde worden regels toegevoegd om ze te ontvangen |

De toegevoegde regels ontstaan zoals regels die na de laatste worden ingevoegd: drie
seconden elk, de ene na de andere. **Het venster meldt dat**, in een informatievenster:

```text
inserted 2 subtitles to fit the clipboard
```

Na het plakken zijn **de geschreven regels geselecteerd**.

Een plakbewerking die niets wijzigt — dezelfde teksten op dezelfde regels — komt niet in
de geschiedenis.

### Tags van een ander formaat worden omgezet

Een kopie die in dit venster is gemaakt, **onthoudt het formaat van haar document**, en
dat geheugen blijft bestaan als een ander bestand wordt geopend. Plakken in een document van
een ander formaat zet de tags dus om, zoals `Save As…` doet:
`{\i1}Bonjour{\i0}` gekopieerd uit een Advanced SSA wordt geplakt als `<i>Bonjour</i>` in een
SubRip.

**Wat het doelformaat niet kan schrijven, gaat verloren, en dat wordt gezegd**, in
de woorden van `Save As…`:

```text
pasting SubRip texts into LRC: 1 tag dropped
```

Anders dan bij `Save As…` wordt er vooraf niets gevraagd: het plakken wordt ongedaan gemaakt
met `Ctrl+Z`, wat de teksten teruggeeft zoals ze waren.

**Een tekst die buiten `subedit` is gekopieerd, heeft geen formaat**, en wordt ongewijzigd geplakt:
er valt niets om te zetten vanuit het niets. Dat geldt ook voor een kopie uit
dit venster die sindsdien door het systeemklembord is vervangen — de
inhoud van het systeemklembord telt.

Wanneer beide meldingen verschuldigd zijn, passen ze in één venster, gescheiden door
een puntkomma.

## Wat de actie Ongedaan maken erover zegt

Elk gebaar is **één** geschiedenisitem, ongeacht het aantal
regels dat het schrijft of toevoegt.

| Bewerking | Wat `Undo` meldt |
| :-------- | :--------------- |
| knippen | `Undo: cutting texts` |
| plakken | `Undo: pasting texts` |

Zie [Ongedaan maken en opnieuw uitvoeren](undo.md).
