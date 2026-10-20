# Aanroep

<!-- machine-translated -->
> Deze pagina is een machinevertaling en is nog niet nagelezen door een lezer van de taal.

```
subedit-gui [--version] [<file>]
```

| Argument | Verplicht | Waarde | Standaard |
| :------- | :------- | :---- | :------ |
| `<file>` | nee | het pad van een te openen ondertitelbestand | geen — het venster opent leeg |
| `--version` | nee | — | — |

**Eén enkel bestand.** De volgende argumenten worden genegeerd: het venster houdt
maar één project tegelijk vast.

## `--version`

Schrijft op de standaarduitvoer het blok dat de `--version` van de opdrachtregel schrijft, en
keert direct terug, zonder een venster te openen. Afsluitcode `0`.

**`subedit <version>` alleen op de eerste regel**, daarna één regel per bibliotheek waartegen het
venster draait, met de versie die tijdens het uitvoeren is gevonden en, tussen haakjes, de
versie waartegen is gebouwd; daarna de geïnstalleerde interfacetalen, per code. Op
een machine met Qt 6.4.2:

```console
$ subedit-gui --version
subedit <version>
  ICU      74.2 (built with 74.2)
  Enchant  2.3.3 (built with 2.3.3)
  Qt       6.4.2 (built with 6.4.2)
  libmpv   2.2 (built with 2.2)
  Languages: en, fr
```

De nummers worden hier niet overgenomen: het zijn die van de machine, en deze handleiding heeft geen
gegenereerd blok dat dat van het programma zelf bijhoudt. De bibliotheken zijn
die van de opdrachtregel, ICU en Enchant, en de twee die alleen het venster linkt — Qt, en
libmpv, de speler, aangegeven door de versie van zijn client-API, die welke `mpv.pc`
meldt. De rest wordt beschreven in
[de handleiding van de opdrachtregel](../subedit-cli/invocation.md#--version). De woorden
`built with` en `Languages` volgen de taal van het venster; niets anders doet dat.

`Hulp ▸ Over subedit` toont dezelfde bibliotheken, uit dezelfde lijst, in een tabel —
de naam, de geladen versie, de versie waartegen is gebouwd, een rij per bibliotheek — en
daaronder de talen; de koppen ervan volgen de taal van het venster.

## Een bestand openen

```console
$ subedit-gui film.srt
```

De gelezen formaten zijn **de negen** die het openfilter opsomt — zie
[Openen en opslaan](files.md#openen). Het formaat wordt herkend aan de inhoud en
niet aan de extensie, en **de codering ook**: een BOM
als het bestand er een draagt, anders de codering die zijn bytes suggereren.

## Wanneer het openen mislukt

Een ontbrekend bestand, een onleesbaar bestand, een bestand waarvan de bytes in geen enkele
codering te decoderen zijn, of een bestand waarvan de inhoud door geen enkel formaat wordt herkend: **het venster opent
toch, leeg**, en de mislukking wordt op de foutenstroom geschreven.

**De melding noemt de oorzaak**, in dezelfde woorden als de opdrachtregel:

| Melding | Wat er is gebeurd |
| :------ | :------------ |
| `does not exist` | het pad wijst geen bestand aan |
| `cannot be opened: permission denied` | het systeem weigert het te openen |
| `cannot be read` | het systeem weigerde om een andere reden |
| `cannot be decoded in the chosen encoding` | de bytes zijn niet te lezen in de gekozen codering |
| `is in no format this tool knows` | geen enkel formaat herkent de inhoud |
| `holds nothing recognisable as a subtitle` | het formaat wordt herkend, maar niets erin is een ondertitel |

```console
$ subedit-gui notes.txt
subedit-gui: notes.txt: is in no format this tool knows
```

Leeg openen in plaats van weigeren te starten: het hulpmiddel is nog steeds ergens
goed voor, en de mislukking van een argument is geen reden om niet te starten.

**Een slecht gevormd maar leesbaar bestand opent**, en wat het lezen onderweg heeft
opgevangen, is te lezen in een knop van de statusbalk — zie
[de diagnostiek van een leesbewerking](files.md#de-diagnostiek-van-een-leesbewerking).

## Afsluitcodes

| Code | Wanneer |
| :--- | :------ |
| `0` | het venster is geopend en daarna normaal gesloten, of `--version` heeft geantwoord |
| `2` | een onverwachte fout vóór het openen — Qt ontbreekt, beeldscherm niet beschikbaar |

De mislukking van het openen van een bestand **wijzigt de afsluitcode niet**: het
venster is geopend, en dat is wat de code meldt.

## De man-pagina

Een installatie legt `subedit-gui(1)` neer onder `share/man/man1`:

```console
$ man subedit-gui
```

Ze geeft de aanroepvorm, `--version`, de meldingen bij een mislukte opening, de
afsluitcodes, de twee omgevingsvariabelen en het instellingenbestand.
**Ze beschrijft het venster niet** — deze handleiding doet dat, en de pagina
verwijst hiernaar.

## Het Qt-platform

`QT_QPA_PLATFORM` bepaalt waarop het venster draait. **Als de variabele
is ingesteld, respecteert `subedit-gui` die zonder argument.**

Als dat niet zo is, laat het Qt kiezen: de ingebouwde speler tekent het beeld in
het venster zelf, en vraagt niets van het platform — zie
[De speler](player.md).

`QT_QPA_PLATFORM=offscreen` laat het venster draaien waar geen
grafische server is — wat continue integratie en de tests doen.

## Taal

Het venster spreekt de taal van zijn omgeving, **eenmalig gekozen bij het starten**: het wijzigen van de
variabelen van een draaiend venster verandert niets tot de volgende start. De variabelen en de regels
zijn die van de opdrachtregel — `LANGUAGE`, `LC_ALL`, `LC_MESSAGES`, `LANG`, `LC_ALL=C` voor
Engels, Engels **zonder een woord** wanneer de gevraagde taal niet is geïnstalleerd — beschreven in
[de handleiding van de opdrachtregel](../subedit-cli/invocation.md#taal). De catalogi zijn dezelfde
bestanden, geïnstalleerd onder `<prefix>/share/subedit/locale`.

**Een taal die in `Aanpassen ▸ Instellingen…` is gekozen, gaat voor**, en de omgeving wordt alleen gelezen
wanneer die op `System default` staat, wat zo is tot er een is gekozen: een daar gekozen geïnstalleerde taal
wint van elke variabele, `LC_ALL=C` inbegrepen. De keuze geldt eveneens bij de volgende start. Zie
[De interfacetaal](preferences.md#de-interfacetaal).

| Wat | Volgt de taal |
| :-- | :------------ |
| de menu's en hun items, met hun onderstreepte letters; de werkbalk; de knopinfo | ja |
| de kolommen van de tabel en van `Beeld ▸ Kolommen`, de tabbladen, de statusregel, de afspeelbalk | ja |
| de standaardknoppen die Qt zelf tekent (`OK`, `Cancel`) | ja, wanneer de eigen catalogus van Qt voor die taal is geïnstalleerd (`qtbase_<language>.qm`); anders Engels |
| de dialoogvensters, de pagina's van `Correct Texts…`, de titels en filters van de bestandsdialogen, de berichtvensters en wat ze zeggen | ja |
| de regels die het venster op zijn foutenstroom schrijft, `subedit-gui: …` | ja, op de naam van het programma na |
| de namen van de correctiepatronen in `Correct Texts…`, en hun beschrijvingen in de knopinfo | ja, voor de patronen die met het programma worden meegeleverd; een eigen patroon wordt getoond zoals zijn bestand het schrijft |
| de namen van coderingen, regeleinden en spellingtalen (`UTF-8`, `CRLF`, `en_US`) | nooit: het zijn namen, geen woorden |
| de sneltoetsen | nooit: `Ctrl+S` is `Ctrl+S` in elke taal |
| een melding die de catalogus niet vertaalt | getoond in het Engels |

Deze versie installeert `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt` en `pt_BR` — Catalaans, Tsjechisch, Duits,
Spaans, Fins, Frans, Galicisch, Hebreeuws, Hongaars, Interlingue, IJslands, Italiaans, Nederlands, Pools, Portugees en Braziliaans Portugees — volledig vertaald. Frans is nagelezen door een lezer van de
taal; Catalaans, Tsjechisch, Duits, Spaans, Fins, Galicisch, Hebreeuws, Hongaars, Interlingue, IJslands, Italiaans, Nederlands, Pools, Portugees en Braziliaans Portugees zijn machinevertaald en nog door geen lezer van de taal
nagelezen.
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
Op een Franse desktop is de menubalk
`&Fichier`, `É&dition`, `&Affichage`, `&Vidéo`, `&Outils`, `&Projets`, `Aid&e`. De schermafbeeldingen
van deze handleiding zijn in het Nederlands gemaakt.
