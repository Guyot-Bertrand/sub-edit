# subedit-cli — handleiding

<!-- machine-translated -->
> Deze pagina is een machinevertaling en is nog niet nagelezen door een lezer van de taal.

Het opdrachtregelprogramma van subedit: werken aan ondertitelbestanden zonder grafische
interface, in bulk of vanuit een script. Voor het andere programma en om te weten
waar u moet beginnen, zie
[de handleiding](../index.md).

> **Huidige stand.** De zeventien subcommando's van `subedit-cli` bestaan: onderzoeken,
> omzetten, verschuiven, omrekenen, de framerate opnieuw timen, op een framerooster uitlijnen, de
> vermeldingen voor slechthorenden verwijderen, de duren aanpassen, een tekst vervangen, de hoofdletters,
> het cursief en de dialoogstreepjes wijzigen, de ondertitels weer op volgorde zetten, de
> teksten corrigeren met de patronen van Gaupol, bestanden achter elkaar zetten, een
> bestand in tweeën knippen, en een vertaling op de posities van haar hoofdbestand schrijven. Deze handleiding
> beschrijft wat bestaat, nooit wat gepland is; wat er hierna komt, staat in
> de [roadmap](../../../../feuille-de-route.md).

## Secties

| Sectie | Inhoud |
| :------ | :------ |
| [Installatie](installation.md) | het programma bouwen en installeren |
| [Aanroep](invocation.md) | algemene opties, uitvoer, afsluitcodes |
| [Het instellingenbestand](invocation.md#het-instellingenbestand) | wat `adjust` en `correct` uit het instellingenbestand van het venster lezen, `--config`, `--no-config` |
| [Een mappenstructuur verwerken](lots.md) | meerdere bestanden, een hele map, `--recursive` |
| [`inspect`](inspect.md) | melden waaruit een bestand bestaat |
| [`convert`](convert.md) | een bestand in een ander formaat of een andere vorm schrijven |
| [`shift`](shift.md) | elke positie met hetzelfde bedrag verplaatsen |
| [`transform`](transform.md) | elke positie corrigeren uitgaande van twee punten waarvan men weet dat ze kloppen |
| [`framerate`](framerate.md) | een bestand van de ene framerate naar de andere opnieuw timen |
| [`snap`](snap.md) | de tijdstempels weer op de frames van een framerate zetten |
| [`hearing-impaired`](hearing-impaired.md) | de vermeldingen voor slechthorenden verwijderen |
| [`adjust`](adjust.md) | de duren aanpassen aan een leessnelheid en aan limieten |
| [`replace`](replace.md) | een tekst vervangen zonder een tag te beschadigen |
| [`case`](case.md) | de teksten in elk woord met hoofdletter, zin met hoofdletter, hoofdletters of kleine letters zetten |
| [`italics`](italics.md) | de teksten cursief zetten, of hun cursief weghalen |
| [`dialogue-dashes`](dialogue-dashes.md) | dialoogstreepjes plaatsen, of weghalen |
| [`sort`](sort.md) | de ondertitels weer op de volgorde van hun begin zetten |
| [`join-repeats`](join-repeats.md) | van de opeenvolgende stukken van één tekst één enkele ondertitel maken |
| [`disjoint`](disjoint.md) | de overlappende ondertitels afknippen zodat geen bereik nog een ander overlapt |
| [`repair`](repair.md) | `join-repeats`, `sort` en `disjoint` na elkaar toepassen op een bestand dat slecht is opgedeeld, ongeordend is en overlapt |
| [`correct`](correct.md) | de teksten corrigeren met de patronen van Gaupol: vermeldingen, woorden samenvoegen en splitsen, veelvoorkomende fouten, hoofdletters, regelafbreking |
| [`append`](append.md) | bestanden na het eerste zetten, in één enkel bestand |
| [`split-file`](split-file.md) | een bestand in tweeën knippen, waarbij de staart naar de oorsprong wordt teruggebracht |
| [`pair`](pair.md) | een vertaling op de posities van haar hoofdbestand schrijven, en zeggen hoe haar regels zijn gekoppeld |
| [`transcribe`](transcribe.md) | de gesproken tekst van een video of een audiobestand als ondertitels schrijven |
| [`sync`](sync.md) | de ondertitels van een bestand hertimen op de audio van een film |
| [De transcriptiehulpmiddelen installeren](install-transcription.md) | de engine, het model en `ffmpeg`, die het programma niet meelevert |
| [`config`](config.md) | de bewerkingsinstellingen die met het venster worden gedeeld tonen, weergeven en wijzigen |
