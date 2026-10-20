# `join-repeats`

<!-- machine-translated -->
> Deze pagina is een machinevertaling en is nog niet nagelezen door een lezer van de taal.

```
subedit-cli join-repeats (--output FILE | --output-dir DIR | --in-place)
                         [--dry-run]
                         [--recursive]
                         <file>...
```

Maakt **één enkele ondertitel** van de ondertitels die na elkaar hetzelfde tonen: een tekst die in stukken is geknipt
die op elkaar volgen, zoals sommige segmentatiehulpmiddelen hem achterlaten. Het is `Join Repeated Subtitles` van het venster —
[dezelfde logica](../subedit-gui/lines.md#herhaalde-ondertitels-samenvoegen).

**Een herhaling** is een ondertitel met **dezelfde tekst, dezelfde vertaling en dezelfde opmaakinstellingen** (een
stijl, een positie, cue-instellingen) als een andere, **en die uiterlijk begint wanneer die eindigt**: ze
sluiten aan of overlappen. Een reeks herhalingen wordt **degene die het eerst begint**, waarvan het einde wordt
verlengd tot het laatste einde van de reeks; de andere verdwijnen.

- **De volgorde van het bestand maakt niet uit en wordt niet gewijzigd**: de bewaarde ondertitel is degene die het eerst begint,
  waar de andere ook staan. Een bestand op volgorde blijft op volgorde.
- **Niets wordt per vergissing samengevoegd**: een andere tekst, een andere vertaling, andere instellingen, een tussenruimte tussen twee
  stukken, een ondertitel zonder tekst of waarvan het einde vóór het begin ligt — in al deze gevallen blijven ze gescheiden.
- **Vóór de rest uit te voeren**: sorteren en [de overlappingen wegknippen](disjoint.md) lezen een schone reeks beter.
  [`repair`](repair.md) schakelt alle drie aaneen, en [`convert`](convert.md#de-bereiken-niet-overlappend-maken) accepteert het als optie (`--join-repeats`, `--repair`).

<!-- example: subedit-cli join-repeats --help -->
```console
$ subedit-cli join-repeats --help
Join the subtitles that repeat each other, one after the other, into one
Usage: subedit-cli join-repeats [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to join the repeats of

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
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
| `--output` / `--output-dir` / `--in-place` | **een van de drie**, behalve met `--dry-run` | zie [Aanroep](invocation.md#de-bestemming) | — |
| `--dry-run` | nee | een vlag: berekent en meldt, schrijft niets — zie [Kijken voor het schrijven](invocation.md#kijken-voor-het-schrijven) | uit |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:04,000 --> 00:00:06,000\nAlpha\n\n4\n00:00:08,000 --> 00:00:09,000\nBeta\n\n' > morceaux.srt; subedit-cli join-repeats --output joint.srt morceaux.srt; cat joint.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:04,000 --> 00:00:06,000\nAlpha\n\n4\n00:00:08,000 --> 00:00:09,000\nBeta\n\n' > morceaux.srt; subedit-cli join-repeats --output joint.srt morceaux.srt; cat joint.srt
morceaux.srt: 2 repeated subtitles joined -> joint.srt
1
00:00:01,000 --> 00:00:06,000
Alpha

2
00:00:08,000 --> 00:00:09,000
Beta
```

De drie stukken `Alpha`, die van 1 s tot 6 s op elkaar volgen, vormen nu nog maar één. `Beta`, gescheiden door een tussenruimte,
is gebleven. Een tweede doorgang verandert niets en zegt dat: `no repeated subtitles`.

## Uitvoer

**Standaarduitvoer** — niets: het resultaat is het geschreven bestand.

| Niveau | Wat er op de foutuitvoer wordt toegevoegd |
| :----- | :------------------------------------- |
| 1 | `<path>: N repeated subtitles joined -> <destination>`, of `<path>: no repeated subtitles -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, daarna **elke diagnostische melding bij het lezen** |

### In JSON

Met [`--format json`](invocation.md#uitvoer-die-een-script-kan-lezen) de `counts` van elk bestand: `subtitles` (die van het
geschreven bestand), `joined` (de verwijderde ondertitels omdat een andere ze dekt) en `chains` (de samengevoegde reeksen).

## Afsluitcodes

Die van het hulpprogramma: `0` als elk bestand is verwerkt — een bestand zonder herhalingen telt mee —, `2` als geen enkel, `3` als
slechts enkele, `1` bij een gebruiksfout.

## Fouten

Geen specifieke voor `join-repeats`: een onleesbaar bestand mislukt zoals bij elk subcommando, en de fouten van
de bestemming zijn gemeenschappelijk voor de subcommando's die schrijven: zie [Aanroep](invocation.md#de-bestemming).
