# `replace`

<!-- machine-translated -->
> Deze pagina is een machinevertaling en is nog niet nagelezen door een lezer van de taal.

```
subedit-cli replace [--regex] [--case-sensitive] [--range N-M|N-]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    <pattern> <replacement> <file>...
```

Vervangt een tekst in de ondertitels **zonder een tag te breken**. Het is de
bewerking `Replace All` van het [zoeken](../subedit-gui/find-and-replace.md) in het venster, met dezelfde regels:

- **het zoeken gebeurt in de zichtbare tekst** — wat de kijker leest. Tags maken er geen
  deel van uit: zoeken naar `<i>` vindt niets, en zoeken naar `Bonjour` vindt een ‘Bonjour’ waarvan de helft
  cursief is;
- **de vervanging staat in de brontekst**, die van het bestand, waarbij de tags blijven waar ze
  waren. Een tag die het gezochte woord doorsnijdt, omvat de hele vervanging; een stijl die
  de overeenkomst raakt, bedekt de hele vervanging; wat de overeenkomst niet raakt,
  verschuift niet. De regels, geval voor geval, zijn die van het zoeken in het venster.

**Er is geen subcommando `find`**: om te zien waar een patroon staat zonder iets te wijzigen,
toont [`--dry-run`](invocation.md#kijken-voor-het-schrijven) elke betrokken ondertitel.

<!-- example: subedit-cli replace --help -->
```console
$ subedit-cli replace --help
Replace a text in the subtitles, without breaking a tag
Usage: subedit-cli replace [OPTIONS] pattern replacement files...

Positionals:
  pattern TEXT REQUIRED       Text to look for in what is shown
  replacement TEXT REQUIRED   What replaces it, tags allowed
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --regex                     Read the text to look for as a regular expression
  --case-sensitive            Tell capitals from small letters
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
| `<pattern>` | ja | de tekst om te zoeken in wat wordt getoond; **niet leeg** | — |
| `<replacement>` | ja | wat hem vervangt, tags toegestaan; mag leeg zijn | — |
| `<file>...` | ja | een of meer paden | — |
| `--recursive`, `-r` | nee | een vlag | uit — zie [Een mappenstructuur verwerken](lots.md) |
| `--regex` | nee | een vlag: het patroon is een reguliere expressie van ICU | gewone tekst |
| `--case-sensitive` | nee | een vlag: onderscheidt hoofdletters van kleine letters | ze worden genegeerd |
| `--range` | nee | `N-M`, of `N-` tot het einde — zie [`adjust`](adjust.md#--range) | het hele bestand |
| `--document` | nee | `main` of `translation` — zie [Een vertaling](invocation.md#een-vertaling) | `main` |
| `-t`, `--translation-file` | met `--document translation` | een vertaalbestand, voor **een enkele** invoer | — |
| `--align-method` | nee, en alleen met `-t` | `position` of `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **een van de drie**, behalve met `--dry-run` | zie [Aanroep](invocation.md#de-bestemming) | — |
| `--dry-run` | nee | een vlag: berekent en meldt, schrijft niets — zie [Kijken voor het schrijven](invocation.md#kijken-voor-het-schrijven) | uit |

**Een patroon dat met een streepje begint** wordt na `--` geschreven: `subedit-cli replace -- -- "—" film.srt`.

**Standaard de hoofdtekst; de vertaling met `--document translation -t FILE`.** Dan wordt
de vertaling gewijzigd en geschreven, en alleen zij — zie [Een vertaling](invocation.md#een-vertaling).

### Gewone tekst of expressie

Standaard wordt het patroon **gelezen zoals het er staat**: `a.b` zoekt een `a`, een punt, een `b`. Hoofdletters
worden even fijnzinnig genegeerd als in het venster — `É` en `é` zijn één letter.

Met `--regex` is het patroon een reguliere expressie van **ICU**, gelezen zoals Gaupol haar leest: `.` loopt over
een regeleinde heen, `^` en `$` gelden bij elke regel van een ondertitel. In de vervanging, en
alleen daar:

| Schrijf | Om te zeggen |
| :----- | :-------- |
| `$1` tot `$9` | de groepen van de overeenkomst |
| `$0` | de hele overeenkomst |
| `\n` | een regeleinde |
| `\$`, `\\` | een `$`, een backslash |

Let op de aanhalingstekens van de shell: `'$1 EUR'`, en niet `"$1 EUR"`, waarbij de shell `$1` zou lezen.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nPrix: 12 euros, et 7 euros.\n\n' > prix.srt; subedit-cli replace --regex '([0-9]+) euros' '$1 EUR' --output euros.srt prix.srt; cat euros.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nPrix: 12 euros, et 7 euros.\n\n' > prix.srt; subedit-cli replace --regex '([0-9]+) euros' '$1 EUR' --output euros.srt prix.srt; cat euros.srt
prix.srt: replaced 2 matches -> euros.srt
1
00:00:01,000 --> 00:00:02,000
Prix: 12 EUR, et 7 EUR.
```

## Tags

Een tekst waarin een tag het gezochte woord doorsnijdt:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace Bonjour Salut --output salut.srt balises.srt; cat salut.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace Bonjour Salut --output salut.srt balises.srt; cat salut.srt
balises.srt: replaced 2 matches -> salut.srt
1
00:00:01,000 --> 00:00:02,000
<i>Salut</i> tout le monde

2
00:00:03,000 --> 00:00:04,000
Salut <i>Marie</i>, ca va
```

In de eerste ondertitel sneed de tag ‘Bonjour’ door: hij omvat nu ‘Salut’.
In de tweede wordt hij niet geraakt en blijft hij waar hij is. **Een open gelaten tag
blijft open**: wankele opmaak herstellen zou een andere bewerking zijn.

De vervanging wordt gelezen in **de tags van het formaat van het bestand**: `<i>…</i>` voor SubRip,
`{\i1}…` voor Advanced SSA. Het is de enige manier om op te maken wat men schrijft.

## Niets gevonden, niets te wijzigen

Twee antwoorden die geen mislukking zijn — de afsluitcode is `0` en **het bestand wordt toch
geschreven**, want een opgegeven bestemming is een geschreven bestemming:

| Situatie | Zin |
| :-------- | :----- |
| het patroon staat nergens | `"zzz" not found` |
| het staat er, maar elke overeenkomst wordt door zichzelf vervangen | `nothing to change` |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli replace zzz x --output b.srt a.srt; subedit-cli replace Bonjour Bonjour --output c.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli replace zzz x --output b.srt a.srt; subedit-cli replace Bonjour Bonjour --output c.srt a.srt
a.srt: "zzz" not found -> b.srt
a.srt: nothing to change -> c.srt
```

## `--dry-run`

De standaarduitvoer somt **elke ondertitel op waarvan de tekst zou veranderen**, ervoor en erna, zoals
ze in het bestand staan — tags inbegrepen:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace --dry-run Bonjour Salut balises.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace --dry-run Bonjour Salut balises.srt
balises.srt: replaced 2 matches (dry run, nothing written)
balises.srt: subtitle 1
- <i>Bon</i>jour tout le monde
+ <i>Salut</i> tout le monde
balises.srt: subtitle 2
- Bonjour <i>Marie</i>, ca va
+ Salut <i>Marie</i>, ca va
```

De vorm van de lijst wordt beschreven in [Kijken voor het schrijven](invocation.md#kijken-voor-het-schrijven);
in JSON is het `changes`.

## Uitvoer

**Standaarduitvoer** — niets: het resultaat is het geschreven bestand. Met `--dry-run` de lijst met
wijzigingen hierboven.

| Niveau | Wat er op de foutuitvoer wordt toegevoegd |
| :----- | :------------------------------------- |
| 1 | `<path>: replaced N matches -> <destination>`, of `"<pattern>" not found`, of `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — het formaat, de codering, de markering en de regeleinden van het gelezen bestand, teruggezet zoals ze waren |
| 3 | `<path>: N bytes read, M written`, daarna **elke diagnostische melding van het lezen** — zie [Aanroep](invocation.md#diagnostische-meldingen-lezen) |

`N` telt **de vervangen overeenkomsten in de ondertitels waarvan de tekst is veranderd** — het is het
getal dat het venster ook noemt.

### In JSON

Met [`--format json`](invocation.md#uitvoer-die-een-script-kan-lezen) bevat het object van elk bestand:

| Sleutel | Inhoud |
| :-- | :------ |
| `counts.replaced` | de vervangen overeenkomsten, in de ondertitels waarvan de tekst is veranderd: de `N` van de zin |
| `counts.matched` | de gevonden overeenkomsten, **of hun vervanging de tekst nu heeft veranderd of niet** — daaraan onderscheidt men ‘niets gevonden’ (`0`) van ‘niets te wijzigen’ (`replaced` op `0`, `matched` positief) |
| `changes` | de gewijzigde ondertitels: `{"subtitle", "document", "before", "after"}`, **leeg** wanneer niets is gewijzigd |

## Afsluitcodes

Die van het hulpprogramma: `0` als elk bestand is verwerkt — een bestand waarin niets is gevonden, telt
mee —, `2` als geen enkel, `3` als slechts enkele, `1` bij een gebruiksfout.

## Fouten

| Wat het veroorzaakt | Melding |
| :------------------ | :------ |
| leeg patroon | `pattern: nothing to look for` |
| onleesbare expressie | `pattern: not a regular expression (<ICU's reason>)` |
| bereik slecht geschreven of buiten het bestand | zie [`adjust`](adjust.md#--range) |

**Het patroon wordt vóór het eerste bestand gelezen**: een onleesbare expressie is een gebruiksfout
(code `1`), er wordt niets gelezen of geschreven.

De fouten van de bestemming zijn gemeenschappelijk voor de subcommando's die schrijven:
zie [Aanroep](invocation.md#de-bestemming).
