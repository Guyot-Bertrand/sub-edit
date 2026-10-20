# `dialogue-dashes`

<!-- machine-translated -->
> Deze pagina is een machinevertaling en is nog niet nagelezen door een lezer van de taal.

```
subedit-cli dialogue-dashes (--add | --remove) [--range N-M|N-]
                            (--output FILE | --output-dir DIR | --in-place)
                            [--dry-run]
                            [--recursive]
                            [--document main|translation] [-t FILE] [--align-method position|number]
                            <file>...
```

Zet een **dialoogstreepje** vooraan de regels, of haalt ze weg. Het is `Dialogue Dashes` van het
[venster](../subedit-gui/operations.md).

**Twee woorden, zoals bij [`italics`](italics.md)**: `--add` en `--remove` sluiten elkaar uit, en een van beide is nodig.

- **`--add`** zet een streepje voor elke regel die er geen heeft: `Bonjour` wordt `- Bonjour`, en
  `Bonjour\nMarie` wordt `- Bonjour\n- Marie`. **Een streepje dat er al staat, wordt niet verdubbeld.**
- **Drie streepjes worden herkend** — het koppelteken van het toetsenbord en de twee lange streepjes van de typograaf —
  **en het koppelteken is wat wordt geschreven**: `— Bonjour` wordt `- Bonjour`. Extra spaties
  na het streepje verdwijnen.
- Het streepje komt **vóór de tekst, niet vóór een openingstag**: `<i>Bonjour</i>` geeft
  `<i>- Bonjour</i>`. Geen enkele tag verschuift.
- **`--remove`** haalt de streepjes vooraan weg, in alle drie de vormen.

<!-- example: subedit-cli dialogue-dashes --help -->
```console
$ subedit-cli dialogue-dashes --help
Put dialogue dashes on the lines, or take them off
Usage: subedit-cli dialogue-dashes [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --add                       Put a dialogue dash at the head of the lines
  --remove                    Take the dialogue dashes off
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
| `--add` / `--remove` | **een van de twee** | vlaggen | — |
| `--range` | nee | `N-M`, of `N-` tot het einde — zie [`adjust`](adjust.md#--range) | het hele bestand |
| `--document` | nee | `main` of `translation` — zie [Een vertaling](invocation.md#een-vertaling) | `main` |
| `-t`, `--translation-file` | met `--document translation` | een vertaalbestand, voor **één enkel** invoerbestand | — |
| `--align-method` | nee, en alleen met `-t` | `position` of `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **een van de drie**, behalve met `--dry-run` | zie [Aanroep](invocation.md#de-bestemming) | — |
| `--dry-run` | nee | een vlag: berekent en meldt, schrijft niets — zie [Kijken voor het schrijven](invocation.md#kijken-voor-het-schrijven) | uit |

**Standaard de hoofdtekst; de vertaling met `--document translation -t FILE`.** Dan wordt de
vertaling gewijzigd en geschreven, en alleen zij — zie [Een vertaling](invocation.md#een-vertaling).

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\nMarie\n\n2\n00:00:03,000 --> 00:00:04,000\n<i>Salut</i>\n\n' > a.srt; subedit-cli dialogue-dashes --add --output t.srt a.srt; cat t.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\nMarie\n\n2\n00:00:03,000 --> 00:00:04,000\n<i>Salut</i>\n\n' > a.srt; subedit-cli dialogue-dashes --add --output t.srt a.srt; cat t.srt
a.srt: 2 subtitles dashed -> t.srt
1
00:00:01,000 --> 00:00:02,000
- Bonjour
- Marie

2
00:00:03,000 --> 00:00:04,000
<i>- Salut</i>
```

## Uitvoer

**Standaarduitvoer** — niets: het resultaat is het geschreven bestand. Met `--dry-run` de lijst van
wijzigingen, beschreven in [Kijken voor het schrijven](invocation.md#kijken-voor-het-schrijven).

| Niveau | Wat er op de foutuitvoer wordt toegevoegd |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles dashed -> <destination>`, of `N subtitles undashed`, of `nothing to change` |
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
| noch `--add` noch `--remove` | `dialogue-dashes needs --add to put dashes on, or --remove` |
| beide | `--add and --remove say opposite things; give one of them` |
| bereik slecht geschreven of buiten het bestand | zie [`adjust`](adjust.md#--range) |

De fouten van de bestemming zijn gemeenschappelijk voor de subcommando's die schrijven:
zie [Aanroep](invocation.md#de-bestemming).
