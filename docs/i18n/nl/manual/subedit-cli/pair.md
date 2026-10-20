# `pair`

<!-- machine-translated -->
> Deze pagina is een machinevertaling en is nog niet nagelezen door een lezer van de taal.

```
subedit-cli pair (--output FILE | --output-dir DIR | --in-place)
                 -t FILE [--align-method position|number]
                 [--dry-run]
                 <main>
```

Schrijft **de vertaling opnieuw getimed op de posities van het hoofdbestand** en zegt hoe haar regels hun
ondertitel hebben gevonden. Het is `Open Translation…` van het venster — [dezelfde koppeling](../subedit-gui/files.md#een-vertaling-openen),
dezelfde zin — gevolgd door het schrijven van het resultaat, wat het venster doet via `Save Translation`.

**Het is het enige wat koppelen doet dat een afzonderlijk verwerkt bestand niet doet.** De andere subcommando's
die `-t` accepteren, wijzigen de vertaling die ze lezen — [`case`](case.md), [`replace`](replace.md)… —; `pair` wijzigt
niets aan de tekst: het **legt** de vertaling op het hoofdbestand en schrijft wat het leggen heeft opgeleverd. Het hoofdbestand wordt
gelezen om de regels te ontvangen en **nooit geschreven**.

- **Wat wordt geschreven is de vertaling**, op **haar eigen pad en in haar eigen formaat**: het formaat, de codering en
  de regeleinden van het vertaalbestand. Haar ondertitels staan **op de posities van het hoofdbestand**: een regel die
  naast de streep viel, krijgt de grenzen terug van de ondertitel die hij vertaalt.
- **`-t` is verplicht.** `pair` zonder vertaling heeft niets om te leggen: het is een gebruiksfout, code `1`.
- **Een enkel hoofdbestand**, en geen `--recursive`: een paar is twee bestanden, en een boomstructuur kan niet
  zeggen welke vertaling bij welk hoofdbestand hoort. Voor een reeks, één aanroep per paar.
- **`--output` noemt het bestand van de geschreven vertaling, `--output-dir` zet er de basisnaam van neer, `--in-place`
  herschrijft het**, zoals bij elk subcommando dat `-t` accepteert — zie [Een vertaling](invocation.md#een-vertaling).
- **`--document` bestaat niet**: er is maar één document als doel, de vertaling.

<!-- example: subedit-cli pair --help -->
```console
$ subedit-cli pair --help
Write a translation at the positions of its main file, and say how it lined up
Usage: subedit-cli pair [OPTIONS] main

Positionals:
  main TEXT REQUIRED          The main subtitle file

Options:
  -h,--help                   Print this help message and exit
  -t,--translation-file FILE  Translation file to lay over the subtitle file, for a single input
  --align-method position|number
                              How the lines of the translation find their subtitles
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Argumenten en opties

| Optie | Verplicht | Waarden | Standaard |
| :----- | :----- | :------ | :----- |
| `<main>` | ja | **een enkel** pad | — |
| `-t`, `--translation-file` | **ja** | het vertaalbestand | — |
| `--align-method` | nee | `position` of `number` — zie [Een vertaling](invocation.md#een-vertaling) | `position` |
| `--output` / `--output-dir` / `--in-place` | **een van de drie**, behalve met `--dry-run` | zie [Aanroep](invocation.md#de-bestemming) | — |
| `--dry-run` | nee | een vlag: berekent en meldt, schrijft niets — zie [Kijken voor het schrijven](invocation.md#kijken-voor-het-schrijven) | uit |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,300 --> 00:00:02,300\nBonjour.\n\n2\n00:00:03,300 --> 00:00:04,300\nAu revoir.\n\n' > film.fr.srt; subedit-cli pair film.srt -t film.fr.srt --output recale.fr.srt; cat recale.fr.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,300 --> 00:00:02,300\nBonjour.\n\n2\n00:00:03,300 --> 00:00:04,300\nAu revoir.\n\n' > film.fr.srt; subedit-cli pair film.srt -t film.fr.srt --output recale.fr.srt; cat recale.fr.srt
film.fr.srt: translation: 2 lines attached -> recale.fr.srt
1
00:00:01,000 --> 00:00:02,000
Bonjour.

2
00:00:03,000 --> 00:00:04,000
Au revoir.
```

De vertaling liep 0,3 s achter: elke regel vond de ondertitel waarmee zijn midden overlapt, en het
geschreven bestand draagt de posities van het hoofdbestand.

## Wat de uitlijning meldt

De zin is **die welke het venster zegt** wanneer het een vertaling opent, elk deel alleen als het niet
nul is — zie [`inspect -t`](inspect.md#een-vertaling--t) voor de tabel van haar delen. **Hij wordt geschreven
vanaf niveau 1**, en niet op niveau 2 zoals bij de andere subcommando's: hier is de uitlijning geen
detail van de verwerking, maar het resultaat.

**Wat de uitlijning met het geschreven bestand doet**:

- **een ondertitel die niemand heeft vertaald, wordt geschreven als een blok zonder tekst**, dat zonder afwijking leest;
- **een ondertitel geboren uit een regel** — een regel die door niets werd bereikt — wordt geschreven met **zijn eigen posities**, welke
  methode ook: het geschreven bestand kan dus meer ondertitels tellen dan het hoofdbestand;
- **wat het document als vertaling bevatte, wordt gewist** vóór het leggen, wat op de opdrachtregel niet van toepassing is:
  het hoofdbestand wordt alleen gelezen.

**De twee methoden** geven hetzelfde bestand wanneer de regels elk op hun ondertitel vallen, en niet
wanneer een regel ontbreekt of de vertaling verschoven is — daarom is de methode een keuze, en daarom is
positie de standaard. De acht gevallen van `src/test/data/paires/` sommen ze op, met het verwachte bestand voor
elke methode.

## Uitvoer

**Standaarduitvoer** — niets: het resultaat is het geschreven bestand.

| Niveau | Wat er op de foutuitvoer wordt toegevoegd |
| :----- | :------------------------------------- |
| 1 | `<translation>: translation: N lines attached; … -> <destination>` — **het pad is dat van de vertaling**, het geschreven bestand; met `--dry-run`, `(dry run, nothing written)` in plaats van `-> <destination>` |
| 2 | `<translation>: SubRip, UTF-8, no BOM, LF line endings kept` — het formaat, de codering, de markering en de regeleinden **van de vertaling**, teruggezet zoals ze waren |
| 3 | `<translation>: N bytes read, M written`, daarna **elke diagnostische melding van het lezen** van het hoofdbestand en van de vertaling — zie [Aanroep](invocation.md#diagnostische-meldingen-lezen) |

### In JSON

Met [`--format json`](invocation.md#uitvoer-die-een-script-kan-lezen) **een enkel object**, waarvan `file` de vertaling is en
`destination` het geschreven bestand. `counts.subtitles` telt **de ondertitels van het geschreven bestand**, ook die welke een regel
tot leven bracht. De sleutel `alignment` — `{"file", "method", "attached", "born", "untranslated", "out_of_order"}` —
draagt dezelfde tellingen als de zin, net als de sleutel `translation` van [`inspect`](inspect.md#in-json).

## Afsluitcodes

Die van het hulpprogramma: `0` als de vertaling is geschreven — of berekend, met `--dry-run` —, `2` als een van de twee bestanden niet
kon worden gelezen of als de bestemming niet kon worden geschreven, `1` bij een gebruiksfout. **Een slecht uitgelijnde vertaling is geen
mislukking**: ze wordt geschreven, en de zin zegt wat zijn plaats niet vond.

## Fouten

| Wat het veroorzaakt | Melding |
| :------------------- | :------ |
| geen `-t` | `pair needs the translation file to lay over the main one: use -t` |
| `--document` | `The following argument was not expected: --document` — het bestaat niet voor `pair` |
| meer dan één hoofdbestand | `The following argument was not expected: <path>` |
| de vertaling is het hoofdbestand | `<translation>: the file is already open as the main document`, code `2` |
| een van de bestanden ontbreekt, is onleesbaar of heeft een onbekend formaat | hetzelfde als overal, met het pad van het bestand dat de fout veroorzaakt |
