# `case`

<!-- machine-translated -->
> Deze pagina is een machinevertaling en is nog niet nagelezen door een lezer van de taal.

```
subedit-cli case --to title|sentence|upper|lower [--range N-M|N-]
                 (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 [--document main|translation] [-t FILE] [--align-method position|number]
                 <file>...
```

Zet de teksten in **elk woord met hoofdletter, zin met hoofdletter, hoofdletters of kleine letters**, zonder een
tag aan te raken. Het is `Gereedschap ▸ Hoofdlettergebruik` van het [venster](../subedit-gui/operations.md), met zijn regels.

| `--to` | Effect | `bonjour marie` wordt |
| :----- | :---- | :---------------------- |
| `title` | elk woord krijgt een hoofdletter, de rest wordt kleine letters | `Bonjour Marie` |
| `sentence` | de eerste letter van de tekst, en alleen die; de rest wordt kleine letters | `Bonjour marie` |
| `upper` | alles in hoofdletters | `BONJOUR MARIE` |
| `lower` | alles in kleine letters | `bonjour marie` |

- **Wat vóór de eerste letter of het eerste cijfer staat, wordt niet aangeraakt**: een dialoogstreepje,
  een openingsaanhalingsteken of een haakje blijven behouden (`- bonjour` geeft `- Bonjour`).
- **Tags zijn geen tekst**: ze veranderen niet van hoofdletters — `<I>` blijft `<I>` — en
  veranderen niet van plaats: een tag die een woord doorsnijdt, snijdt het daarna nog steeds door.
- **ICU bepaalt waar een woord begint**: `l'été sera chaud` geeft `L'été Sera Chaud`, en de accenten
  blijven behouden (`éléonore` geeft `Éléonore`).

<!-- example: subedit-cli case --help -->
```console
$ subedit-cli case --help
Put the texts in title, sentence, upper or lower case, tags intact
Usage: subedit-cli case [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --to title|sentence|upper|lower
                              The case to put the texts in
  --range N-M|N-              Act only on subtitles N to M, or N to the end
  --document main|translation The document to change: the main one, or the translation given by -t
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
| `<file>...` | ja | een of meer paden | — |
| `--recursive`, `-r` | nee | een vlag | uit — zie [Een mappenstructuur verwerken](lots.md) |
| `--to` | **ja** | `title`, `sentence`, `upper`, `lower` | — |
| `--range` | nee | `N-M`, of `N-` tot het einde — zie [`adjust`](adjust.md#--range) | het hele bestand |
| `--document` | nee | `main` of `translation` — zie [Een vertaling](invocation.md#een-vertaling) | `main` |
| `-t`, `--translation-file` | met `--document translation` | een vertaalbestand, voor **één enkel** invoerbestand | — |
| `--align-method` | nee, en alleen met `-t` | `position` of `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **een van de drie**, behalve met `--dry-run` | zie [Aanroep](invocation.md#de-bestemming) | — |
| `--dry-run` | nee | een vlag: berekent en meldt, schrijft niets — zie [Kijken voor het schrijven](invocation.md#kijken-voor-het-schrijven) | uit |

**Standaard de hoofdtekst; de vertaling met `--document translation -t FILE`.** Dan wordt de
vertaling gewijzigd en geschreven, en alleen zij — zie [Een vertaling](invocation.md#een-vertaling).

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\n- BONJOUR MARIE\n\n' > a.srt; subedit-cli case --to title --output titre.srt a.srt; cat titre.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\n- BONJOUR MARIE\n\n' > a.srt; subedit-cli case --to title --output titre.srt a.srt; cat titre.srt
a.srt: 2 subtitles recased -> titre.srt
1
00:00:01,000 --> 00:00:02,000
<i>Bon</i>jour Marie

2
00:00:03,000 --> 00:00:04,000
- Bonjour Marie
```

Een bestand waarin **geen enkele tekst verandert** — alles staat al in de gevraagde schrijfwijze — wordt geschreven zoals het is en
meldt dat:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBONJOUR\n\n' > a.srt; subedit-cli case --to upper --output b.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBONJOUR\n\n' > a.srt; subedit-cli case --to upper --output b.srt a.srt
a.srt: nothing to change -> b.srt
```

## Uitvoer

**Standaarduitvoer** — niets: het resultaat is het geschreven bestand. Met `--dry-run` de lijst van
wijzigingen, beschreven in [Kijken voor het schrijven](invocation.md#kijken-voor-het-schrijven).

| Niveau | Wat er op de foutuitvoer wordt toegevoegd |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles recased -> <destination>`, of `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — het formaat, de codering, de markering en de regeleinden van het gelezen bestand, teruggezet zoals ze waren |
| 3 | `<path>: N bytes read, M written`, daarna **elke leesmelding** — zie [Aanroep](invocation.md#diagnostische-meldingen-lezen) |

`N` is het aantal ondertitels **waarvan de tekst is veranderd**, afgelezen van de bewerking zelf.

### In JSON

Met [`--format json`](invocation.md#uitvoer-die-een-script-kan-lezen) bevat het object van elk bestand
`counts.changed` — de `N` van de zin — en `changes`, de lijst van gewijzigde ondertitels
(`{"subtitle", "document", "before", "after"}`, **leeg** als er niets is veranderd).

## Afsluitcodes

Die van het programma: `0` als elk bestand is verwerkt — een bestand waarin niets verandert, is dat —,
`2` als geen enkel, `3` als slechts sommige, `1` bij een gebruiksfout.

## Fouten

| Wat het veroorzaakt | Melding |
| :------------------ | :------ |
| `--to` ontbreekt | `--to is required` |
| een waarde die niet een van de vier is | `--to: … not in {title,sentence,upper,lower}` |
| bereik slecht geschreven of buiten het bestand | zie [`adjust`](adjust.md#--range) |

De fouten van de bestemming zijn gemeenschappelijk voor de subcommando's die schrijven:
zie [Aanroep](invocation.md#de-bestemming).
