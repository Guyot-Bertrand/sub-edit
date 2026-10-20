# `italics`

<!-- machine-translated -->
> Deze pagina is een machinevertaling en is nog niet nagelezen door een lezer van de taal.

```
subedit-cli italics (--on | --off) [--range N-M|N-]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    <file>...
```

Zet de teksten **cursief**, of **haalt hun cursief eruit**, in de tags van het formaat van het bestand.
Het is `Italic` van het [venster](../subedit-gui/operations.md).

**Twee woorden en geen schakelaar**: de knop van het venster vraagt de kern welke kant het
op gaat; hier zegt men het zelf. `--on` en `--off` sluiten elkaar uit, en er is er een nodig — weglaten of beide opgeven
is een gebruiksfout (code `1`).

- **`--on`** omhult elke tekst met één enkel paar tags, **nadat de tags die er al
  in zaten zijn verwijderd**: een tekst die al cursief is, wordt niet twee keer omhuld, en cursief dat in tweeën
  was geknipt, wordt in één stuk opnieuw gemaakt. Over meerdere regels dekt één paar het geheel.
- **`--off`** verwijdert de cursieve tags; **de rest van een tag blijft behouden** (`{\b1\i1}` geeft
  `{\b1}`).
- De tekst wordt geschreven **zoals het formaat van het bestand cursief schrijft**: `<i>…</i>` voor SubRip, WebVTT en
  SubViewer 2, `{\i1}…{\i0}` voor de twee Sub Station Alpha's, `{Y:i}` voor MicroDVD.

<!-- example: subedit-cli italics --help -->
```console
$ subedit-cli italics --help
Put the texts in italics, or take their italics out
Usage: subedit-cli italics [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --on                        Put the texts in italics
  --off                       Take the italics out
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
| `--on` / `--off` | **een van de twee** | vlaggen | — |
| `--range` | nee | `N-M`, of `N-` tot het einde — zie [`adjust`](adjust.md#--range) | het hele bestand |
| `--document` | nee | `main` of `translation` — zie [Een vertaling](invocation.md#een-vertaling) | `main` |
| `-t`, `--translation-file` | met `--document translation` | een vertaalbestand, voor **één enkele** invoer | — |
| `--align-method` | nee, en alleen met `-t` | `position` of `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **een van de drie**, behalve met `--dry-run` | zie [Aanroep](invocation.md#de-bestemming) | — |
| `--dry-run` | nee | een vlag: berekent en meldt, schrijft niets — zie [Kijken voor het schrijven](invocation.md#kijken-voor-het-schrijven) | uit |

**Standaard de hoofdtekst; de vertaling met `--document translation -t FILE`.** Dan is het
de vertaling die wordt gewijzigd en geschreven, en alleen die — zie [Een vertaling](invocation.md#een-vertaling).

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour\nMarie\n\n' > a.srt; subedit-cli italics --on --output i.srt a.srt; cat i.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour\nMarie\n\n' > a.srt; subedit-cli italics --on --output i.srt a.srt; cat i.srt
a.srt: 2 subtitles put in italics -> i.srt
1
00:00:01,000 --> 00:00:02,000
<i>bonjour marie</i>

2
00:00:03,000 --> 00:00:04,000
<i>Bonjour
Marie</i>
```

## Een formaat zonder stijl weigert

**TMPlayer en LRC schrijven geen stijl**: er is geen cursief om aan te zetten en geen cursief om te verwijderen. Het
venster maakt het item grijs in plaats van het te verbergen; de opdrachtregel **weigert, bestand
voor bestand, met de reden**, en de andere bestanden van de reeks gaan door. Wat telt is wat het
formaat kan bevatten, niet de manier waarop het het schrijft: SubRip en WebVTT schrijven cursief op dezelfde
manier, en zeggen niet hetzelfde over kleur.

<!-- example: printf '00:00:01:Hello there\n00:00:03:Bye\n' > a.txt; subedit-cli italics --on --output-dir sortie a.txt; echo "code=$?" -->
```console
$ printf '00:00:01:Hello there\n00:00:03:Bye\n' > a.txt; subedit-cli italics --on --output-dir sortie a.txt; echo "code=$?"
a.txt: TMPlayer writes no style: there are no italics to put on
code=2
```

In JSON draagt de mislukking de identifier **`no-style`**.

## Uitvoer

**Standaarduitvoer** — niets: het resultaat is het geschreven bestand. Met `--dry-run` de lijst met
wijzigingen, beschreven in [Kijken voor het schrijven](invocation.md#kijken-voor-het-schrijven).

| Niveau | Wat er op de foutuitvoer wordt toegevoegd |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles put in italics -> <destination>`, of `N subtitles taken out of italics`, of `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — het formaat, de codering, de markering en de regeleinden van het gelezen bestand, teruggezet zoals ze waren |
| 3 | `<path>: N bytes read, M written`, daarna **elke diagnostische melding bij het lezen** — zie [Aanroep](invocation.md#diagnostische-meldingen-lezen) |

`N` is het aantal ondertitels **waarvan de tekst is veranderd**, afgelezen van de bewerking zelf.

### In JSON

Met [`--format json`](invocation.md#uitvoer-die-een-script-kan-lezen) bevat het object van elk bestand
`counts.changed` — de `N` van de zin — en `changes`, de lijst van gewijzigde ondertitels
(`{"subtitle", "document", "before", "after"}`, **leeg** wanneer niets is veranderd).

## Afsluitcodes

Die van het hulpprogramma: `0` als elk bestand is verwerkt — een bestand waarin niets verandert, telt mee —,
`2` als geen enkel, `3` als slechts enkele, `1` bij een gebruiksfout.

## Fouten

| Wat het veroorzaakt | Melding |
| :------------------ | :------ |
| noch `--on` noch `--off` | `italics needs --on to put the texts in italics, or --off` |
| beide | `--on and --off say opposite things; give one of them` |
| een formaat zonder stijl | `<path>: TMPlayer writes no style: there are no italics to put on` (of `take out`) |
| bereik slecht geschreven of buiten het bestand | zie [`adjust`](adjust.md#--range) |

De fouten van de bestemming zijn gemeenschappelijk voor de subcommando's die schrijven:
zie [Aanroep](invocation.md#de-bestemming).
