# `transcribe`

<!-- machine-translated -->
> Deze pagina is een machinevertaling en is nog niet nagelezen door een lezer van de taal.

```
subedit-cli transcribe --output FILE
                       [--to FORMAT]
                       [--audio-track N]
                       [--language CODE]
                       [--model FILE]
                       [--engine NAME]
                       [--keep-audio FILE]
                       [--raw]
                       <input>
```

Schrijft **de gesproken tekst van een video, of van een audiobestand, als ondertitelbestand**. De audio wordt
voorbereid, doorgegeven aan een engine die vindt wat er wordt gezegd en wanneer, en de stukken die de engine
knipt worden samengevoegd tot ondertitels die een duur en een lengte respecteren.

**Niets van wat nodig is wordt met het programma meegeleverd**: de engine (whisper.cpp), een model en `ffmpeg`
worden door de gebruiker geïnstalleerd, die zegt waar het model staat. [De transcriptiehulpmiddelen
installeren](install-transcription.md) zegt waar u elk vindt en welk model u kiest. **Als ze ontbreken, gaat er
niets stuk**: de opdracht zegt wat er ontbreekt, met de verwijzing naar die pagina, schrijft niets en
eindigt met een code die niet nul is.

- **De invoer is een video of een audiobestand.** Uit een video haalt `ffmpeg` één audiospoor; zonder
  `ffmpeg` wordt een audiobestand dat de engine rechtstreeks leest (`wav`, `mp3`, `flac`, `ogg`) toch
  getranscribeerd, maar een video niet, en ook geen enkel bestand wanneer een spoor wordt gevraagd.
- **Slechts één bestand tegelijk**, en de bestemming is `--output`: er is geen `--output-dir` of
  `--in-place`, omdat het resultaat een nieuw bestand is en geen herschrijving.
- **De taal wordt gedetecteerd** tenzij `--language` haar afdwingt, en de gebruikte taal wordt vermeld. Een
  model dat alleen voor het Engels is gemaakt detecteert niets en transcribeert niets anders: een taal die het
  niet aankan wordt geweigerd *vóór* enig werk, met vermelding van de grens van het model.
- **De segmenten worden samengevoegd en geknipt** volgens de grenzen van de instellingen (`transcription.merge-gap-ms`,
  `transcription.max-duration-ms`, `transcription.max-characters`): twee stukken van één zin die dicht bij
  elkaar liggen worden één ondertitel, en een stuk dat te lang of te breed is wordt bij een woord geknipt. **`--raw`
  schrijft de segmenten van de engine zoals ze kwamen**, één per ondertitel, en zo ziet u ook wat het
  samenvoegen doet.

<!-- example: subedit-cli transcribe --help -->
```console
$ subedit-cli transcribe --help
Write the speech of a video or an audio file as subtitles
Usage: subedit-cli transcribe [OPTIONS] input

Positionals:
  input TEXT REQUIRED         Video or audio file to transcribe

Options:
  -h,--help                   Print this help message and exit
  --output FILE               Subtitle file to write
  --to TEXT:{srt,vtt,subviewer2,ssa,ass,microdvd,mpl2,tmplayer,lrc}
                              Format to write; the one the extension of --output says by default
  --audio-track N             Audio track of a video, counted from 0; the first by default
  --language CODE             Language of the speech: fr, en; detected by default
  --model FILE                Model file; the transcription.model setting by default
  --engine NAME               Engine; the transcription.engine setting by default
  --keep-audio FILE           Keep the extracted audio in this file
  --raw                       Write the segments as the engine cut them
```

## Argumenten en opties

| Optie | Verplicht | Waarden | Standaard |
| :----- | :----- | :------ | :----- |
| `<input>` | ja | een video of een audiobestand | — |
| `--output` | ja | het ondertitelbestand om te schrijven; de map wordt aangemaakt als die ontbreekt | — |
| `--to` | nee | de korte naam van een formaat: `srt`, `vtt`, `subviewer2`, `ssa`, `ass`, `microdvd`, `mpl2`, `tmplayer`, `lrc` | het formaat dat de extensie van `--output` aangeeft, anders `srt` |
| `--audio-track` | nee | een geheel getal vanaf 0: de positie onder de audiosporen van de video | `0`, het eerste |
| `--language` | nee | twee kleine letters, een ISO 639-1-code: `en`, `fr` | de instelling `transcription.language`; leeg: gedetecteerd |
| `--model` | nee | het modelbestand | de instelling `transcription.model` |
| `--engine` | nee | de naam van een engine: `whisper.cpp` | de instelling `transcription.engine` |
| `--keep-audio` | nee | een bestand om de uitgepakte audio in te bewaren, als 16 kHz mono 16-bits PCM in een WAV-bestand; de map wordt aangemaakt als die ontbreekt | de audio wordt nergens bewaard |
| `--raw` | nee | een vlag: de segmenten van de engine schrijven zonder samenvoegen en zonder knippen | uit |

De **instellingen** worden uit het instellingenbestand gelezen zoals die van [`adjust`](adjust.md) — zie
[het instellingenbestand](invocation.md#het-instellingenbestand) — en elk heeft hier zijn optie:

| Sleutel | Betekenis | Standaard |
| :-- | :------ | :------ |
| `transcription.engine` | de engine | `whisper.cpp` |
| `transcription.executable` | het programma van de engine, een absoluut pad; leeg: gezocht in het `PATH` | leeg |
| `transcription.model` | het modelbestand, een absoluut pad | leeg |
| `transcription.language` | de taal van de audio, twee letters; leeg: gedetecteerd | leeg |
| `transcription.merge-gap-ms` | stukken van één zin die dichter bij elkaar liggen dan dit aantal milliseconden worden samengevoegd | `400` |
| `transcription.max-duration-ms` | de maximale tijd dat een ondertitel in beeld blijft, in milliseconden | `7000` |
| `transcription.max-characters` | het maximale aantal tekens van een ondertitel, regeleinden niet meegeteld | `84` |

## De audio kiezen

Een video kan meerdere audiosporen hebben — het origineel, een commentaar, een andere taal. `--audio-track`
neemt de **positie onder de audiosporen** van het bestand, vanaf 0, in de volgorde waarin het bestand ze
opsomt. Een spoor dat de video niet heeft wordt geweigerd, **met de sporen die hij wel heeft**, zodat de
volgende aanroep de juiste is:

```bash
subedit-cli transcribe --audio-track 4 --output film.srt --model small.bin film.mkv
```

```text
film.mkv: the video has no audio track 4
audio track 0: fra, Original
audio track 1: eng, Commentary
```

## Uitvoer

**Standaarduitvoer** — niets: het resultaat is het geschreven bestand.

| Niveau | Wat er op de foutuitvoer wordt toegevoegd |
| :----- | :------------------------------------- |
| 1 | `<input>: N subtitles transcribed, language: xx -> <output>` |
| 2 | de voortgang van de engine, `transcribing: 25%`, zoals hij die meldt — in stappen van ongeveer dertig seconden audio |

### In JSON

Met [`--format json`](invocation.md#uitvoer-die-een-script-kan-lezen) (de globale optie, vóór het
subcommando geschreven) bevat het object van het bestand de gebruikte taal in `language`, `counts.subtitles` (de
geschreven ondertitels) en `counts.segments` (de stukken van de engine waaruit ze zijn gemaakt), en `audio` wanneer
`--keep-audio` de audio heeft bewaard.

## Afsluitcodes

Die van het hulpmiddel: `0` als het bestand is geschreven, `2` als dat niet kon — een hulpmiddel of een model dat ontbreekt, een
taal die het model niet aankan, een mislukte extractie of transcriptie —, `1` bij een gebruiksfout, zoals een
engine die niemand heeft geregistreerd.

## Fouten

| Oorzaak | Melding |
| :---- | :------ |
| geen programma van de engine gevonden, of het geconfigureerde is er niet | `the transcription engine was not found`, daarna de verwijzing naar de gids |
| geen model opgegeven, of het is er niet | `no model was given, or the model file is not there`, daarna de verwijzing naar de gids |
| `ffmpeg` niet gevonden voor een video | `ffmpeg was not found, so the audio of a video cannot be extracted`, daarna de verwijzing |
| een spoor dat de video niet heeft | `the video has no audio track N`, daarna de sporen die hij heeft |
| een taal die het model niet kan transcriberen | `the model cannot transcribe the language xx` |
| geen taal, en een model dat er geen kan detecteren | `the model cannot detect the language: give one` |
| een engine die niemand heeft geregistreerd | `no transcription engine is called NAME` |
| elke andere mislukking | `the transcription failed`, met wat de engine zei |

De verwijzing naar de gids is `See the manual page "Installing the transcription tools"
(install-transcription).`
