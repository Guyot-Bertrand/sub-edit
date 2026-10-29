# `sync`

<!-- machine-translated -->
> Deze pagina is een machinevertaling en is nog niet nagelezen door een lezer van de taal.

```
subedit-cli sync (--video FILE | --audio FILE)
                 [--audio-track N]
                 [--language CODE]
                 [--start TIME] [--end TIME]
                 [--model FILE]
                 [--vad-model FILE]
                 [--engine NAME]
                 [--keep-audio FILE]
                 [--min-match PERCENT]
                 [--doubt-gap MS]
                 [--report FILE]
                 [-r]
                 [--output FILE | --output-dir DIR | --in-place] [--dry-run]
                 <files>...
```

**Hertimet een ondertitelbestand op de audio van een film.** Het geval waarvoor het bestaat is de online gedownloade
`.srt`, aan het begin en aan het einde verschoven, die een enkele verschuiving niet terugbrengt: elke ondertitel wordt
gezocht in wat er wordt gezegd, en neemt de plaats in waar hij wordt uitgesproken.

Het luistert naar de audio met een transcriptie-engine — dezelfde als die van [`transcribe`](transcribe.md) — en **koppelt
de woorden van elke ondertitel aan de woorden die het heeft gehoord**. Er wordt niets vertaald: de ondertitels moeten
in de gesproken taal staan.

**Niets van wat nodig is wordt met het programma meegeleverd**: de engine, een model en `ffmpeg` zijn die van
[De transcriptiehulpmiddelen installeren](install-transcription.md), en hun afwezigheid breekt niets — de opdracht zegt
wat er ontbreekt, met de verwijzing naar die pagina, schrijft niets en eindigt met een code die niet nul is.

- **Alleen begin- en eindtijden veranderen.** Nooit de tekst, de stijl, het aantal ondertitels of hun volgorde.
- **Een ondertitel die de audio niet geeft behoudt zijn tijden** — een generiek die niemand uitspreekt, een lied, een regel in een andere taal.
  **Er wordt nooit een positie verzonnen, en geen enkele wordt uit de buren afgeleid**: de regel blijft waar het bestand hem had,
  en de opdracht zegt het. Het is *geen* mislukking, en de afsluitcode is `0`.
- **Een ondertitel is gevonden wanneer minstens de helft van zijn woorden wordt gehoord** (`--min-match`, `alignment.min-match`), en
  minstens één. Hoofdletters, accenten, leestekens en opmaak doen er niet toe; een woord dat één letter afwijkt van een lang woord telt als gehoord.
- **Het einde is waar het laatste woord eindigt.** De engine timet woorden bij benadering, enkele honderdsten van een seconde vóór
  het eigen einde van de ondertitel, dus een ondertitel kan iets korter uitkomen dan hij was.
- **Een ondertitel die tegen de volgende zou aanlopen wordt ingekort tot waar de volgende begint.** Een ondertitel die de
  volgende nog steeds overlapt, wat inkorten niet verhelpt, blijft staan en wordt gemeld.
- **Een bestand waarvan geen enkele ondertitel is gevonden wordt geweigerd en niet geschreven**: het is vrijwel zeker de verkeerde audio of de
  verkeerde taal.
- **Een deel van de film kan alleen worden beluisterd**, met `--start` en `--end`, zoals bij
  [`transcribe`](transcribe.md#een-deel-transcriberen): alleen de ondertitels die in het bereik **beginnen** worden gezocht en
  aangeraakt, de andere blijven zoals ze zijn en worden niet geteld.
- **De audio wordt voor elk opgegeven bestand één keer beluisterd.** Om meerdere bestanden op dezelfde film te hertimen, moet u op
  elk wachten.

<!-- example: subedit-cli sync --help -->
```console
$ subedit-cli sync --help
Re-time the subtitles of a file on the audio of a film
Usage: subedit-cli sync [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to re-time

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --video FILE                Film to listen to
  --audio FILE                Audio file to listen to, instead of a film
  --audio-track N             Audio track of a film, counted from 0; the first by default
  --start TIME                Where to begin, as 2:00, 1:02:03 or seconds; the beginning by default
  --end TIME                  Where to stop, as 2:00, 1:02:03 or seconds; the end by default
  --language CODE             Language of the speech, two letters: fr, en, de; detected from the first 30 seconds by default
  --model FILE                Model file; the transcription.model setting by default
  --vad-model FILE            Voice-activity model that skips silences; the transcription.vad-model setting by default
  --engine NAME               Engine that listens; the transcription.engine setting by default
  --keep-audio FILE           Keep the extracted audio in this file
  --min-match PERCENT         Share of a subtitle's words that must be heard, in percent; the alignment.min-match setting by default
  --doubt-gap MS              How far a subtitle may move from its neighbours' shift before it is flagged, in milliseconds; the alignment.doubt-gap setting by default
  --report FILE               Write the subtitles to check to this file
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Argumenten en opties

| Optie | Verplicht | Waarden | Standaard |
| :----- | :------- | :----- | :------ |
| `<files>` | ja | de te hertimen ondertitelbestanden; met `-r`, mappen | — |
| `--recursive`, `-r` | nee | een vlag | uit — zie [Een mappenstructuur verwerken](lots.md) |
| `--video` | een van `--video` en `--audio` | de film om naar te luisteren | — |
| `--audio` | een van `--video` en `--audio` | een audiobestand om naar te luisteren, in plaats van een film | — |
| `--audio-track` | nee | een geheel getal vanaf 0: de positie onder de audiosporen van de film | `0`, het eerste |
| `--language` | nee | twee letters, een ISO 639-1-code: `en`, `fr` | de instelling `transcription.language`; leeg: gedetecteerd |
| `--start`, `--end` | nee | een tijd: `2:00`, `1:02:03`, `2:00.5`, of een aantal seconden; `--end` moet na `--start` komen | de hele audio |
| `--model` | nee | het modelbestand | de instelling `transcription.model` |
| `--vad-model` | nee | een spraakactiviteitsmodel, waardoor de engine de stiltes overslaat; daarmee volgen de woorden de spraak nauwkeuriger | de instelling `transcription.vad-model` |
| `--engine` | nee | de naam van de engine die luistert: `whisper.cpp` | de instelling `transcription.engine` |
| `--keep-audio` | nee | een bestand om de uitgepakte audio in te bewaren; de map wordt aangemaakt als ze ontbreekt | de audio wordt nergens bewaard |
| `--min-match` | nee | een geheel getal van 1 tot 100: het aandeel van de woorden van een ondertitel dat gehoord moet worden, in procenten | de instelling `alignment.min-match`, `50` |
| `--doubt-gap` | nee | een geheel aantal milliseconden, 0 of meer: hoever de verschuiving van een ondertitel mag afwijken van die van zijn buren voordat hij wordt gemarkeerd | de instelling `alignment.doubt-gap`, `2000` |
| `--report` | nee | een bestand om de te controleren ondertitels in te schrijven, één per regel: het nummer van de ondertitel, een tab, en waarom; één ondertitelbestand tegelijk | er wordt niets geschreven |
| `--output` / `--output-dir` / `--in-place` | **een van de drie**, behalve met `--dry-run` | zie [Aanroep](invocation.md#de-bestemming) | — |
| `--dry-run` | nee | een vlag: berekent en meldt, schrijft niets — zie [Kijken voor het schrijven](invocation.md#kijken-voor-het-schrijven) | uit |

De bestemming — `--output`, `--output-dir`, `--in-place`, `--dry-run` — en `-r` zijn die van de andere opdrachten die een
bestand herschrijven: zie [de bestemming](invocation.md#de-bestemming). Het bestand behoudt zijn formaat, zijn codering en zijn
regeleinden.

De **instellingen** worden uit het instellingenbestand gelezen — zie [het instellingenbestand](invocation.md#het-instellingenbestand) — en
elk heeft hier zijn optie. De engine, het model en het spraakactiviteitsmodel zijn die van `transcribe`.

| Sleutel | Betekenis | Standaard |
| :-- | :------ | :------ |
| `alignment.engine` | de uitlijn-engine; er is er één, `transcript-matching`, die luistert en koppelt | `transcript-matching` |
| `alignment.min-match` | het aandeel van de woorden van een ondertitel dat gehoord moet worden om hem gevonden te laten zijn, in procenten | `50` |
| `alignment.doubt-gap` | hoever, in milliseconden, de verschuiving van een ondertitel mag afwijken van die van zijn buren voordat hij wordt gemarkeerd | `2000` |

## Wat wordt gemarkeerd

De opdracht zegt welke ondertitels u moet controleren, en waarom. Een ondertitel wordt gemarkeerd om de **eerste** van deze redenen die opgaat:

| Waarom | Betekenis |
| :-- | :------ |
| not found in the audio, left as it was | minder dan `--min-match` van zijn woorden is gehoord: hij behoudt zijn tijden |
| only partly heard | gevonden, maar minder dan 80 % van zijn woorden is gehoord |
| moved differently from its neighbours | gevonden, maar zijn verschuiving wijkt met meer dan `--doubt-gap` af van de mediane verschuiving van de twee ondertitels aan weerszijden |

Een ondertitel kan ook **ingekort worden tot het begin van de volgende ondertitel**, of **de volgende ondertitel nog steeds overlappen**.

Twee getallen vertellen het verhaal van het bestand als geheel. **De mediane verschuiving** is het midden van hoever de ondertitels
zijn verschoven: een bestand dat gewoon te laat was, leest u hier en nergens anders. **De drift** is de mediane verschuiving van het laatste tiende van de
ondertitels min die van het eerste tiende: ze ligt dicht bij nul voor een constante afwijking, en is groot voor een bestand dat steeds
meer te laat was naarmate de film vorderde.

## Uitvoer

**Standaarduitvoer** — niets: het resultaat is het geschreven bestand.

| Niveau | Wat er op de foutuitvoer wordt gezegd |
| :---- | :------------------------------- |
| 1 | `<file>: N of M subtitles placed on the audio, K to check; median shift X s, drift Y s -> <destination>` |
| 2 | de stap en de voortgang, zoals bij [`transcribe`](transcribe.md#uitvoer) |

### In JSON

Met [`--format json`](invocation.md#uitvoer-die-een-script-kan-lezen) bevat het object van het bestand `counts`: `subtitles`
(gezocht), `placed` (een plaats gegeven), `moved` (waarvan de tijden zijn veranderd), `partial`, `out_of_line`, `not_found`,
`median_shift_ms` en `drift_ms`; en `flagged`, een lijst van `{"subtitle": N, "kind": "…"}` waarin het nummer de plaats van de
ondertitel in het bestand is en `kind` een van `not-found`, `partial`, `out-of-line`, `cut-back` en `overlaps` is.
De identifiers worden beloofd; de zinnen niet.

## Voorbeeld

```bash
subedit-cli sync --video film.mkv --model small.bin --vad-model silero.bin \
    --report to-check.txt --output film.synced.srt film.srt
```

## Afsluitcodes

Die van het hulpmiddel: `0` als het bestand is geschreven, met of zonder gemarkeerde ondertitels, `2` als dat niet kon — een hulpmiddel of een model dat ontbreekt, een
mislukte extractie of luisterbeurt, geen enkele ondertitel gevonden —, `1` bij een gebruiksfout, zoals noch
`--video` noch `--audio` (of beide), een optie buiten haar bereik, of een bereik dat onbruikbaar is.

## Fouten

| Oorzaak | Melding |
| :---- | :------ |
| noch `--video` noch `--audio`, of beide | `sync needs the audio to listen to: give --video or --audio, and not both` |
| geen programma van de engine gevonden, of het geconfigureerde is er niet | `the transcription engine was not found`, daarna de verwijzing naar de gids |
| geen model opgegeven, of het is er niet | `no model was given, or the model file is not there`, daarna de verwijzing naar de gids |
| `ffmpeg` niet gevonden voor een film | `ffmpeg was not found, so the audio of a video cannot be extracted`, daarna de verwijzing |
| een spoor dat de film niet heeft | `the video has no audio track N`, daarna de sporen die hij heeft |
| een taal die het model niet kan transcriberen | `the model cannot transcribe the language xx` |
| een engine die niemand heeft geregistreerd | `no transcription engine is called NAME`, of `there is no alignment engine called NAME` |
| geen enkele ondertitel gevonden | `none of the N subtitles was found in the audio: is it the right audio, and the right language?` |
| `--min-match` buiten 1 tot 100, `--doubt-gap` negatief | de gebruiksfout van de optie |
| `--report` met meer dan één ondertitelbestand | `--report names one file: give one subtitle file when you use it` |
