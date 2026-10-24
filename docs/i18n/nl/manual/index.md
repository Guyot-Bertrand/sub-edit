# subedit — handleiding

<!-- machine-translated -->
> Deze pagina is een machinevertaling en is nog niet nagelezen door een lezer van de taal.

subedit leest en onderzoekt ondertitelbestanden en past er de timing van aan. Het bestaat uit twee
programma's die dezelfde kern delen, en dus dezelfde regels en dezelfde woorden.

| Programma | Waarvoor | De handleiding |
| :-------- | :-------------- | :--------- |
| `subedit-gui` | een bestand openen, het in een tabel zien, het bewerken, het naast de film bekijken | [de handleiding van het venster](subedit-gui/index.md) |
| `subedit-cli` | bestanden in bulk verwerken, vanuit een script, zonder interface | [de handleiding van de opdrachtregel](subedit-cli/index.md) |

Beide lezen en schrijven **de negen formaten** — SubRip, WebVTT, SubViewer 2,
Sub Station Alpha, Advanced SSA, MicroDVD, MPL2, TMPlayer, LRC — en
herkennen het formaat aan de inhoud in plaats van aan de extensie. **Ook de codering
wordt aan de inhoud herkend**: een bestand in Latin-1, in CP1252 of in UTF-16
wordt geopend, er wordt gemeld als wat het is gelezen, en het wordt in zijn eigen codering
teruggeschreven — een bestand dat wordt geopend en zonder wijziging weer opgeslagen, verandert niet met één byte.

## Waar te beginnen

| Wat u wilt | Waar u heen gaat |
| :----------------- | :------- |
| het hulpmiddel installeren | [Installatie](subedit-cli/installation.md) — geldt voor beide programma's |
| een bestand openen en bekijken wat het bevat | [De tabel](subedit-gui/table.md) |
| ondertitels opnieuw timen die met een vast bedrag zijn verschoven | [De bewerkingen](subedit-gui/operations.md#shift-positions), of [`shift`](subedit-cli/shift.md) |
| een bestand opnieuw timen dat voor een andere framerate is geschreven | [`framerate`](subedit-cli/framerate.md), en [`snap`](subedit-cli/snap.md) als de timing al klopt |
| weten tegen welke framerate een bestand is geschreven | [Het framerooster](subedit-gui/frame-grid.md), of [`inspect`](subedit-cli/inspect.md) |
| de vermeldingen voor slechthorenden verwijderen | [De bewerkingen](subedit-gui/operations.md#remove-hearing-impaired-mentions), of [`hearing-impaired`](subedit-cli/hearing-impaired.md) |
| vermeldingen, veelvoorkomende fouten, hoofdletters en regelafbreking in één keer corrigeren | [`Correct Texts…`](subedit-gui/correct-texts.md), of [`correct`](subedit-cli/correct.md) |
| de spelling woord voor woord controleren, met de woordenboeken van het systeem | [`Check Spelling…`](subedit-gui/check-spelling.md) |
| ondertitels verlengen die te kort zijn om te worden gelezen | [`Adjust Durations…`](subedit-gui/operations.md#adjust-durations), of [`adjust`](subedit-cli/adjust.md) |
| een bestand weer op volgorde zetten waarvan de ondertitels dat niet zijn | [`sort`](subedit-cli/sort.md) |
| van de opeenvolgende stukken van één tekst één enkele ondertitel maken | [`Join Repeated Subtitles`](subedit-gui/lines.md#herhaalde-ondertitels-samenvoegen), of [`join-repeats`](subedit-cli/join-repeats.md) |
| een bestand dat slecht is opgedeeld, ongeordend is en overlapt in één keer rechtzetten | [`repair`](subedit-cli/repair.md) |
| de overlappingen tussen ondertitels verwijderen | [`Make Ranges Disjoint…`](subedit-gui/lines.md#de-bereiken-niet-overlappend-maken), of [`disjoint`](subedit-cli/disjoint.md) |
| de twee delen van een film achter elkaar zetten | [`Append File…`](subedit-gui/operations.md#append-file), of [`append`](subedit-cli/append.md) |
| een film in twee delen knippen | [`Split Project…`](subedit-gui/operations.md#split-project), of [`split-file`](subedit-cli/split-file.md) |
| de hoofdletters, het cursief of de dialoogstreepjes van een heel bestand wijzigen | [De bewerkingen](subedit-gui/operations.md#italic), of [`case`](subedit-cli/case.md), [`italics`](subedit-cli/italics.md), [`dialogue-dashes`](subedit-cli/dialogue-dashes.md) |
| een woord zoeken en vervangen, zonder de tags te beschadigen | [Zoeken en vervangen](subedit-gui/find-and-replace.md), of [`replace`](subedit-cli/replace.md) |
| de gesproken tekst van een film als ondertitels schrijven | [`Transcribe Audio…`](subedit-gui/video.md#de-audio-transcriberen), of [`transcribe`](subedit-cli/transcribe.md) — na [de installatie van de hulpmiddelen](subedit-cli/install-transcription.md) |
| vertalen naast de oorspronkelijke tekst | [Een vertaling openen](subedit-gui/files.md#een-vertaling-openen), of [`pair`](subedit-cli/pair.md) om hem op het hoofdbestand opnieuw te timen |
| honderd bestanden tegelijk verwerken | [Een mappenstructuur verwerken](subedit-cli/lots.md), daarna [Aanroep](subedit-cli/invocation.md) |

**Tussen `framerate` en `snap` blijft een vergissing onopgemerkt.** Als u
twijfelt, begint [`snap`](subedit-cli/snap.md#snap-is-niet-framerate-en-ze-door-elkaar-halen-valt-niet-op)
met de vergelijking van de twee.

## Wat deze handleiding belooft

**Ze beschrijft wat bestaat, nooit wat gepland is.** Wat er komt, en in
welke volgorde, staat in de [roadmap](../../../feuille-de-route.md).

**Deze handleiding is in het Engels geschreven en wordt vertaald.** De vertalingen volgen haar
pagina voor pagina, in de map `docs/i18n/` van de repository, één boom per taal. De
programma's spreken de taal van hun omgeving; de namen die ze tonen — subcommando's,
opties, menulabels — worden hier geciteerd zoals het programma ze in het Engels schrijft.

**De voorbeelden en de afbeeldingen worden gegenereerd, de tekst niet.** De
`console`-blokken ontstaan doordat het commando werkelijk wordt uitgevoerd; de
schermafbeeldingen doordat het echte venster wordt gebouwd en gefotografeerd. Geen
van beide kan dus een hulpmiddel beschrijven dat niet meer bestaat. Zonder vangnet blijft
de tekst eromheen, waarvan de juistheid op het nalezen berust.

**De schermafbeeldingen tonen beide paletten**, `Light` en `Dark` — die
welke [de instellingen](subedit-gui/preferences.md#het-thema) aanbieden.

## De twee oppervlakken zeggen dezelfde woorden

Een reden om de ene handleiding te lezen terwijl u het andere programma gebruikt: de
foutmeldingen, de namen van de formaten, de oordelen over het raster en de labels
van Ongedaan maken worden **één keer** geschreven, in de kern, en beide programma's
citeren ze. Een geweigerd bestand levert aan beide kanten dezelfde zin op.

Wat verschilt, is wat elk oppervlak kan:

| | `subedit-gui` | `subedit-cli` |
| :--- | :------------ | :------------ |
| meerdere bestanden tegelijk | ja — één tabblad per project | ja |
| een vertaling naast het hoofdbestand | ja | ja — `-t` bij de tekstsubcommando's, en [`pair`](subedit-cli/pair.md) om haar opnieuw te timen |
| een bestand achteraan toevoegen, een project splitsen | ja | ja — [`append`](subedit-cli/append.md), [`split-file`](subedit-cli/split-file.md) |
| ongedaan maken | ja, duizend items | niet van toepassing — er wordt niets ter plaatse gewijzigd zonder erom te vragen |
| een tekst of een positie met de hand bewerken | ja | nee |
| regels invoegen, verwijderen, samenvoegen en splitsen | ja | nee |
| zoeken en vervangen | ja | ja — [`replace`](subedit-cli/replace.md) |
| teksten knippen, kopiëren en plakken | ja | nee |
| de duren, het cursief, de hoofdletters, de streepjes aanpassen | ja | ja — [`adjust`](subedit-cli/adjust.md), [`italics`](subedit-cli/italics.md), [`case`](subedit-cli/case.md), [`dialogue-dashes`](subedit-cli/dialogue-dashes.md) |
| de teksten met patronen corrigeren | ja — `Correct Texts…` | ja — [`correct`](subedit-cli/correct.md) |
| de spelling woord voor woord controleren | ja — `Check Spelling…` | nee |
| de film bekijken terwijl u timet | ja | nee |
| over de invoer heen schrijven | `Save` | `--in-place` |
