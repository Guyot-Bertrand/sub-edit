# `shift`

<!-- machine-translated -->
> Deze pagina is een machinevertaling en is nog niet nagelezen door een lezer van de taal.

```
subedit-cli shift (--by <time> | --to-grid)
                  (--output FILE | --output-dir DIR | --in-place)
                  [--dry-run]
                  [--recursive]
                  <file>...
```

Verschuift **elke** positie van het bestand met hetzelfde bedrag, in de ene
of de andere richting. Geen enkele ondertitel blijft langer of korter in beeld:
juist dat maakt een verschuiving tot een verschuiving.

<!-- example: subedit-cli shift --help -->
```console
$ subedit-cli shift --help
Move every position of a file by a fixed amount
Usage: subedit-cli shift [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to shift

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --by TEXT                   Amount to move by: 2.999, -7.001, or 00:00:07.001
  --to-grid                   Move by the amount that puts the positions back on their frame grid
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
| `--by` | **een van de twee** | een duur met teken, zie hieronder | — |
| `--to-grid` | **een van de twee** | vlag: het bedrag wordt gemeten, zie hieronder | — |
| `--output` / `--output-dir` / `--in-place` | **een van de drie**, behalve met `--dry-run` | zie [Aanroep](invocation.md#de-bestemming) | — |
| `--dry-run` | nee | een vlag: berekent en meldt, schrijft niets — zie [Kijken voor het schrijven](invocation.md#kijken-voor-het-schrijven) | uit |

Het formaat van het gelezen bestand blijft **behouden**: het formaat wijzigen is
het werk van [`convert`](convert.md).

## `--to-grid`: het bedrag gemeten in plaats van opgegeven

Een bestand waarvan de posities op een raster van frames liggen, **op een
constante na**, is verschoven, en die constante kan op de posities zelf worden gemeten.
`--to-grid` meet haar en past haar toe, zonder dat iemand haar hoeft te kennen.

Het bedrag is **eigen aan elk bestand**: twee bestanden die met twee
verschillende hoeveelheden van hetzelfde raster zijn verschoven, komen elk met
hun eigen bedrag terug. Het wordt in de toelichting vermeld, samen met de
gevonden framerate.

**De kortste weg wordt genomen.** Voorbij een half frame is de dichtstbijzijnde
rasterlijn de volgende: de correctie gaat vooruit in plaats van bijna een
heel frame terug te gaan.

<!-- example: k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40 + 7)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > decale.srt; subedit-cli shift --to-grid --output recale.srt decale.srt -->
```console
$ k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40 + 7)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > decale.srt; subedit-cli shift --to-grid --output recale.srt decale.srt
decale.srt: 12 subtitles shifted by -0.007 s onto their 25 fps grid -> recale.srt
```

**Een bestand op geen enkel bekend raster wordt geweigerd, met de reden erbij.**
Er is niets om weer op elkaar aan te sluiten, en een fase die op ruis is gemeten
zou het bestand willekeurig verplaatsen. Zie [`inspect`](inspect.md#wat-frame-rate-grid-meldt) om
te weten wat een bestand oplevert voordat u de bewerking probeert.

`--by` en `--to-grid` geven allebei aan met hoeveel er wordt verschoven:
ze samen opgeven is een gebruiksfout, en geen van beide opgeven ook.

**`--to-grid` is niet [`snap`](snap.md).** Het verschuift het hele bestand
met hetzelfde bedrag en behoudt de onderlinge timing **exact**; `snap`
verplaatst elke positie afzonderlijk en neemt onderweg de posities mee die
met de hand waren gecorrigeerd. Op een schoon bestand geven de twee hetzelfde
resultaat; op een gedeeltelijk bestand niet.

## Een duur schrijven

Twee vormen, en alleen die twee:

| Vorm | Voorbeelden |
| :---- | :------- |
| seconden | `30`, `2.999`, `-7.001`, `+1.5` |
| tijdcode | `00:00:07.001`, `01:39:37.040`, `-00:00:07.001` |

Beide zeggen hetzelfde: `--by 7.001` en `--by 00:00:07.001` geven
hetzelfde bestand.

**Het decimaalteken is de punt**, want de opdrachtregel spreekt Engels.
Een komma wordt geweigerd in plaats van geïnterpreteerd: de halve wereld schrijft `7,001`
voor zevenduizend en een, en het lezen ervan als zeven seconden zou een gok zijn —
een foute voor die helft.

**De resolutie is de milliseconde.** Fijner wordt geweigerd in plaats van
stilzwijgend afgerond: een stilzwijgende afronding zou een resultaat geven dat
met niets van wat is gevraagd overeenkomt.

<!-- example: printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle shifted by -7.001 s -> b.srt
1
00:00:02,999 --> 00:00:04,999
Bonjour.
```

## Een verschuiving vóór de oorsprong wordt geweigerd

Voorbij nul teruggaan geeft code `2` en **schrijft niets**. De melding noemt
de ondertitel die de verschuiving niet kon verwerken, en zegt daarmee tegelijk
hoe ver men kan gaan.

De reden is geen principe: een positie vóór de oorsprong wordt geschreven als
`-00:00:01,000`, een vorm die geen van de negen formaten definieert en die geen
lezer accepteert. De kern staat zulke posities wel toe — ze weigeren zou van een
bewerking een bijzonder geval maken — maar een bestand is iets anders dan een
model in het geheugen.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; echo "code=$?"
a.srt: subtitle 1 would start before the origin, which no subtitle file can hold
code=2
```

## Uitvoer

**Standaarduitvoer** — niets: het resultaat is het geschreven bestand. Met `--dry-run` ook niets: er is geen bestand, en de toelichtingsregel zegt dat — zie [Kijken voor het schrijven](invocation.md#kijken-voor-het-schrijven).

| Niveau | Wat er op de foutuitvoer wordt toegevoegd |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles shifted by 2.999 s -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — het formaat, de codering, de markering en de regeleinden van het gelezen bestand, teruggezet zoals ze waren |
| 3 | `<path>: N bytes read, M written`, daarna **elke diagnostische melding van het lezen** — zie [Aanroep](invocation.md#diagnostische-meldingen-lezen) |

### In JSON

Met [`--format json`](invocation.md#uitvoer-die-een-script-kan-lezen) bevat het object van elk bestand `counts.subtitles` (elke ondertitel van het bestand) en `counts.shifted_by_ms` (de verschuiving, in hele milliseconden, **met teken**). De volledige tabel is die van [Aanroep](invocation.md#wat-elk-subcommando-in-counts-zet).

## Afsluitcodes

Die van het hulpprogramma: `0` als elk bestand is geschreven, `2` als geen enkel,
`3` als slechts enkele, `1` bij een gebruiksfout.

## Fouten

| Wat het veroorzaakt | Melding |
| :------------------ | :------ |
| decimale komma | `"…" is not a time: use a decimal point and not a comma, in every language` |
| fijner dan de milliseconde | `"…" is not a time: positions are held to the millisecond, no finer` |
| tekst die geen duur is | `"…" is not a time: expected seconds like 2.999, or a timestamp like 00:00:07.001` |
| verschuiving vóór de oorsprong | `<path>: subtitle N would start before the origin, which no subtitle file can hold` |

**Zonder een van de drie bestemmingsopties wordt niets geschreven**, en hun
fouten zijn gemeenschappelijk voor de subcommando's die schrijven:
zie [Aanroep](invocation.md#de-bestemming).
