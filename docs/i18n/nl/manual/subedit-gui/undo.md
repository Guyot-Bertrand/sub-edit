# Ongedaan maken en opnieuw uitvoeren

<!-- machine-translated -->
> Deze pagina is een machinevertaling en is nog niet nagelezen door een lezer van de taal.

Elke bewerking komt in een geschiedenis en kan ongedaan worden gemaakt. Een bewerkte cel telt
als **één** item: het typen wordt niet toetsaanslag voor toetsaanslag vastgelegd, alleen de
bevestiging telt.

| Actie | Sneltoets | Waar |
| :---- | :-------- | :--- |
| Ongedaan maken | `Ctrl+Z` | menu **Aanpassen**, werkbalk |
| Opnieuw | `Ctrl+Shift+Z` | menu **Aanpassen**, werkbalk |

## De actie noemt wat ze ongedaan maakt

Het menu zegt niet “Ongedaan maken” maar **“Ongedaan maken: een tekst bewerken”**, naar de
bewerking die ongedaan zal worden gemaakt. De knop van de werkbalk houdt het kale woord — zijn
breedte zou anders de laatste bewerking volgen, en hij zou onder de
aanwijzer bewegen — maar zijn knopinfo draagt het volledige label.

| Bewerking | Wat de actie meldt |
| :-------- | :----------------- |
| een bewerkte tekstcel | `editing a text` |
| een begin, een einde | `editing a start`, `editing an end` |
| een duur | `editing a duration` |
| een invoeging van regels | `inserting` |
| een verwijdering van regels | `removing` |
| een verschuiving, `Shift Whole File onto Grid` inbegrepen | `shifting` |
| een omrekening | `transforming` |
| een omzetting van de framesnelheid | `converting the frame rate` |
| een uitlijning op een framesnelheid | `aligning on the frame rate` |
| een verwijdering van vermeldingen | `removing hearing-impaired mentions` |
| cursief zetten | `putting in italics` |
| cursief weghalen | `taking italics out` |
| een wijziging van hoofdlettergebruik | `changing the case` |
| streepjes toegevoegd, verwijderd | `adding dialogue dashes`, `removing dialogue dashes` |
| een samenvoeging van regels | `merging` |
| een splitsing van een regel | `splitting` |
| teksten geknipt, geplakt | `cutting texts`, `pasting texts` |
| een aanpassing van duren | `adjusting durations` |
| een vertaling openen | `opening a translation` |
| een bestand toevoegen | `appending a file` |
| het project splitsen | `splitting the project` |
| een correctie die in `Correct Texts…` is toegepast, of door `Check Spelling…` is gemaakt | `correcting texts` |
| een vervanging, alle vervangingen | `replacing`, `replacing all` |

**Zevenentwintig labels, en meer kan het venster niet voortbrengen.** De kern noemt
een achtentwintigste — `sorting` — dat geen enkele actie van het venster bereikt: sorteren
gebeurt alleen onder een strikt volgordebeleid, terwijl het venster zijn documenten
opent onder het soepele beleid, dat wanorde meldt in plaats van haar te herstellen.

**`Shift Whole File onto Grid` wordt niet van een verschuiving onderscheiden**, en dat is
juist: het is er een, waarvan de omvang is gemeten in plaats van getypt.

Dit zijn dezelfde woorden als die van de opdrachtregel: ze worden één keer geschreven,
in de kern.

Wanneer er niets ongedaan te maken is, luidt de actie gewoon “Ongedaan maken” **en is ze
uitgeschakeld**: haar sneltoets doet niets.

## Het wijzigingsteken

De venstertitel draagt een teken zolang het bestand verschilt van dat op
schijf — een sterretje, of wat het platform daarvoor gebruikt. **Het tabblad
ook**, `premier.srt*`: de titel spreekt alleen over het project waarnaar u kijkt,
het label vertelt over die waarnaar u niet kijkt.

**Elk tabblad heeft zijn eigen geschiedenis.** Ongedaan maken maakt alleen ongedaan wat het project van het
huidige tabblad heeft ontvangen — een `Replace All` die over alle projecten is uitgevoerd
inbegrepen: die laat in elk project dat hij heeft aangeraakt één item achter.

**Het verdwijnt als u terugkeert naar het beginpunt.** Dit is geen detail van
de weergave: het aantal wijzigingen wordt geteld, niet vastgelegd als ja of nee, zodat
teruggaan het op nul kan brengen en het teken verdwijnt.

## Twee dingen om te weten

**Het referentiepunt is de laatste keer opslaan**, of het openen van het bestand als dat
er niet is geweest. Opslaan verplaatst het: het teken verdwijnt, en daarna ongedaan maken
brengt het terug.

Een bewerking na een ongedaanmaking **wist wat er opnieuw uit te voeren was**: opnieuw uitvoeren
zou een commando herhalen waarvan de beginstand niet meer bestaat. Dat is het
gedrag van elke editor, en het is bewust.

**Zolang een cel geopend is**, bevestigen `Undo` en `Redo` — vanuit het menu of de werkbalk —
eerst de invoer, zoals de andere gebaren van
[Een cel bewerken](editing.md). Wat zojuist is getypt, wordt dan het laatste
item van de geschiedenis: `Undo` maakt dat ongedaan, niet de bewerking daarvoor; `Redo` heeft
niets meer om opnieuw uit te voeren, want een invoer wist wat er opnieuw uit te voeren was. Een geopende
cel waarin niets is getypt laat geen item na: `Undo` maakt dan de vorige
bewerking ongedaan.

De geschiedenis bewaart duizend items en vergeet de oudste daarboven.
