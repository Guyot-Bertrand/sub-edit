# `sort`

<!-- machine-translated -->
> Deze pagina is een machinevertaling en is nog niet nagelezen door een lezer van de taal.

```
subedit-cli sort (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 <file>...
```

Zet de ondertitels terug **in de volgorde van hun begin**. Er verandert verder niets: niet de teksten, niet de
posities, niet het formaat van het bestand. De ondertitels worden bij het schrijven vanaf 1 **opnieuw genummerd**,
zoals bij elk teruggeschreven bestand.

**De kern sorteert nooit uit zichzelf**: het lezen van een bestand laat de volgorde zoals ze was, en
[`inspect`](inspect.md#wat-anomalies-meldt) meldt een gebroken volgorde met
`subtitle N starts before the previous one starts`. `sort` is **het gebaar dat bewust wordt gemaakt**, voor een bestand
of een reeks waarvan de volgorde gebroken is — en het kan, in het venster, ongedaan worden gemaakt zoals elke bewerking.

- **Het sorteren is stabiel**: twee ondertitels die op hetzelfde moment beginnen, **behouden de volgorde die het
  bestand ze gaf**. De ene komt niet vóór de andere; ze verplaatsen zou een beslissing zijn waar niemand
  om heeft gevraagd.
- **`sort` neemt geen bereik en geen document**: er valt niets te kiezen, de volgorde is die van de begintijden.

<!-- example: subedit-cli sort --help -->
```console
$ subedit-cli sort --help
Put the subtitles in the order of their start, keeping ties as they are
Usage: subedit-cli sort [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to sort

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

<!-- example: printf '1\n00:00:05,000 --> 00:00:06,000\ncinq\n\n2\n00:00:01,000 --> 00:00:02,000\nun\n\n3\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > desordre.srt; subedit-cli sort --output ordre.srt desordre.srt; cat ordre.srt -->
```console
$ printf '1\n00:00:05,000 --> 00:00:06,000\ncinq\n\n2\n00:00:01,000 --> 00:00:02,000\nun\n\n3\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > desordre.srt; subedit-cli sort --output ordre.srt desordre.srt; cat ordre.srt
desordre.srt: 3 subtitles moved -> ordre.srt
1
00:00:01,000 --> 00:00:02,000
un

2
00:00:03,000 --> 00:00:04,000
trois

3
00:00:05,000 --> 00:00:06,000
cinq
```

## Wat er wordt geteld

**De telling is die van de plaatsen die van ondertitel zijn veranderd**, zoals de kern ze meldt — wat
de geschiedenis van het venster verplaatst zou noemen. Een enkele verkeerd geplaatste ondertitel verschuift er meerdere: in
het voorbeeld hierboven behoudt geen van de drie plaatsen haar ondertitel, en de zin zegt `3 subtitles moved`.

**Een bestand dat al in volgorde staat, wordt toch geschreven** en zegt dat: een opgegeven bestemming is een
geschreven bestemming, en van `sort` de uitzondering maken zou een script dwingen te weten welke
subcommando's soms een bestand opleveren en soms niets.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > ordre.srt; subedit-cli sort --output copie.srt ordre.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > ordre.srt; subedit-cli sort --output copie.srt ordre.srt
ordre.srt: already in order -> copie.srt
```

## Uitvoer

**Standaarduitvoer** — niets: het resultaat is het geschreven bestand. Met `--dry-run` ook niets: er is
geen bestand, en de toelichtingsregel zegt dat — zie [Kijken voor het schrijven](invocation.md#kijken-voor-het-schrijven).

| Niveau | Wat er op de foutuitvoer wordt toegevoegd |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles moved -> <destination>`, of `<path>: already in order -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — het formaat, de codering, de markering en de regeleinden van het gelezen bestand, teruggezet zoals ze waren |
| 3 | `<path>: N bytes read, M written`, daarna **elke diagnostische melding van het lezen** — zie [Aanroep](invocation.md#diagnostische-meldingen-lezen) |

### In JSON

Met [`--format json`](invocation.md#uitvoer-die-een-script-kan-lezen) bevat het object van elk bestand
`counts.subtitles` (elke ondertitel van het bestand) en `counts.moved` (de plaatsen die zijn veranderd;
`0` voor een bestand dat al in volgorde staat).

## Afsluitcodes

Die van het hulpprogramma: `0` als elk bestand is verwerkt — een bestand dat al in volgorde staat, telt mee —, `2` als
geen enkel, `3` als slechts enkele, `1` bij een gebruiksfout.

## Fouten

Geen specifiek voor `sort`: een onleesbaar bestand mislukt zoals bij elk subcommando, en de
fouten van de bestemming zijn gemeenschappelijk voor de subcommando's die schrijven:
zie [Aanroep](invocation.md#de-bestemming).
