# `transcribe`

<!-- machine-translated -->
> Deze pagina is een machinevertaling en is nog niet nagelezen door een lezer van de taal.

```
subedit-cli transcribe --output FILE
                       [--to FORMAT]
                       [--audio-track N]
                       [--language CODE]
                       [--start TIME] [--end TIME]
                       [--suppress-notes | --keep-notes]
                       [--model FILE]
                       [--vad-model FILE]
                       [--engine NAME]
                       [--keep-audio FILE]
                       [--raw]
                       <input>

subedit-cli transcribe --list-languages [--model FILE] [--engine NAME]
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
- **Een taal wordt getypt als twee letters — `fr`, `en`, `de` —, of als de code van drie letters die een bestand opgeeft, `fra`,
  `eng`, die als die van twee letters wordt gelezen.** Al het andere wordt geweigerd voordat het werk begint. **De detectie
  luistert alleen naar de eerste dertig seconden**: een film die opent met muziek of stilte kan voor een
  andere taal worden aangezien, en de engine schrijft de spraak dan *vertaald* in die taal — een Franse film die voor
  Engels wordt aangezien komt als Engels uit, of als `[SPEAKING FRENCH]`. Geef de taal op als u haar kent.
- **Een deel van de film kan alleen worden getranscribeerd**, met `--start` en `--end` — zie
  [Een deel transcriberen](#een-deel-transcriberen). Zo probeert u een model op twee minuten voordat u het op
  twee uur loslaat.
- **De talen die een engine biedt kunnen worden opgesomd**, met `--list-languages` — zie
  [De talen opsommen](#de-talen-opsommen).
- **Geluiden worden standaard niet als notities geschreven**: de engine krijgt de opdracht dat niet te doen (`--suppress-notes`; zijn eigen optie is
  `--suppress-nst`), zodat `(Sonnerie de téléphone)` of `[MUSIC PLAYING]` niet in de ondertitels opduiken, tussen ronde haakjes zoals tussen vierkante.
  `--keep-notes` schrijft ze wel, voor ondertitels voor doven en slechthorenden. Als de notities verboden zijn, schrijft de engine `...` voor een passage
  zonder spraak, en **de nabewerking laat de stukken weg die geen letter en geen cijfer hebben** — `...`, `…`, `♪` — voordat ze samenvoegt en knipt;
  `--raw` houdt ze, zoals de stukken van de engine zijn gekomen.
- **De segmenten worden samengevoegd en geknipt** volgens de grenzen van de instellingen (`transcription.merge-gap-ms`,
  `transcription.max-duration-ms`, `transcription.max-characters`): twee stukken van één zin die dicht bij
  elkaar liggen worden één ondertitel, en een stuk dat te lang of te breed is wordt geknipt — eerst bij een zinseinde, dan bij een komma, een puntkomma of een dubbele punt, en midden in een zin alleen als er geen leesteken is om bij te knippen —, waarbij een knip wordt vermeden die een ondertitel achterlaat die te klein is om te lezen. De drie stappen gebeuren in volgorde, elk over het hele transcript: weglaten, samenvoegen, knippen. Een ondertitel die een stuk begint behoudt zijn begin, en die het afsluit behoudt zijn einde. **`--raw`
  schrijft de segmenten van de engine zoals ze kwamen**, één per ondertitel, en zo ziet u ook wat het
  samenvoegen doet.

<!-- example: subedit-cli transcribe --help -->
```console
$ subedit-cli transcribe --help
Write the speech of a video or an audio file as subtitles
Usage: subedit-cli transcribe [OPTIONS] [input]

Positionals:
  input TEXT                  Video or audio file to transcribe

Options:
  -h,--help                   Print this help message and exit
  --output FILE               Subtitle file to write
  --to TEXT:{srt,vtt,subviewer2,ssa,ass,microdvd,mpl2,tmplayer,lrc}
                              Format to write; the one the extension of --output says by default
  --audio-track N             Audio track of a video, counted from 0; the first by default
  --start TIME                Where to begin, as 2:00, 1:02:03 or seconds; the beginning by default
  --end TIME                  Where to stop, as 2:00, 1:02:03 or seconds; the end by default
  --language CODE             Language of the speech, two letters: fr, en, de; detected from the first 30 seconds by default
  --suppress-notes            Do not write sounds as notes, such as (phone ringing); the transcription.suppress-notes setting by default
  --keep-notes                Write sounds as notes, against the setting
  --model FILE                Model file; the transcription.model setting by default
  --vad-model FILE            Voice-activity model that skips silences; the transcription.vad-model setting by default
  --engine NAME               Engine; the transcription.engine setting by default
  --keep-audio FILE           Keep the extracted audio in this file
  --list-languages            List the languages the engine offers for the model, and transcribe nothing
  --raw                       Write the segments as the engine cut them
```

## Argumenten en opties

| Optie | Verplicht | Waarden | Standaard |
| :----- | :----- | :------ | :----- |
| `<input>` | ja, behalve met `--list-languages` | een video of een audiobestand | — |
| `--output` | ja, behalve met `--list-languages` | het ondertitelbestand om te schrijven; de map wordt aangemaakt als die ontbreekt | — |
| `--to` | nee | de korte naam van een formaat: `srt`, `vtt`, `subviewer2`, `ssa`, `ass`, `microdvd`, `mpl2`, `tmplayer`, `lrc` | het formaat dat de extensie van `--output` aangeeft, anders `srt` |
| `--audio-track` | nee | een geheel getal vanaf 0: de positie onder de audiosporen van de video | `0`, het eerste |
| `--language` | nee | twee letters, een ISO 639-1-code: `en`, `fr`; een code van drie letters zoals `fra` wordt als `fr` gelezen | de instelling `transcription.language`; leeg: gedetecteerd |
| `--suppress-notes` | nee | een vlag: geluiden niet als notities schrijven; geweigerd voor een engine waaraan dat niet kan worden opgedragen | de instelling `transcription.suppress-notes` |
| `--keep-notes` | nee | een vlag: ze wel schrijven, tegen de instelling in; het tegenovergestelde van `--suppress-notes`, en beide samen worden geweigerd | de instelling `transcription.suppress-notes` |
| `--list-languages` | nee | een vlag: de talen opsommen die de engine voor het model biedt, en niets transcriberen | uit |
| `--start` | nee | een tijd: `2:00`, `1:02:03`, `2:00.5`, of een aantal seconden, `90` | het begin van de audio |
| `--end` | nee | een tijd, geschreven zoals bij `--start`; hij moet na `--start` komen | het einde van de audio |
| `--model` | nee | het modelbestand | de instelling `transcription.model` |
| `--vad-model` | nee | een spraakactiviteitsmodel (een Silero-model voor whisper.cpp), waardoor de engine de stiltes overslaat voordat ze decodeert: haar segmenten volgen dan de spraak, en het eerste begint waar de spraak begint in plaats van bij 0. Het bestand moet bestaan, en een engine die het niet kan gebruiken weigert de optie bij naam | de instelling `transcription.vad-model` |
| `--engine` | nee | de naam van een engine: `whisper.cpp` | de instelling `transcription.engine` |
| `--keep-audio` | nee | een bestand om de uitgepakte audio in te bewaren, als mono 16-bits PCM in een WAV-bestand, met de samplefrequentie die de engine vraagt (16 kHz voor whisper.cpp); de map wordt aangemaakt als ze ontbreekt, en de naam mag `{rate}` bevatten, vervangen door die frequentie in hertz (`film.{rate}.wav` wordt geschreven als `film.16000.wav`) | de audio wordt nergens bewaard |
| `--raw` | nee | een vlag: de segmenten van de engine schrijven zonder samenvoegen en zonder knippen | uit |

De **instellingen** worden uit het instellingenbestand gelezen zoals die van [`adjust`](adjust.md) — zie
[het instellingenbestand](invocation.md#het-instellingenbestand) — en elk heeft hier zijn optie:

| Sleutel | Betekenis | Standaard |
| :-- | :------ | :------ |
| `transcription.engine` | de engine | `whisper.cpp` |
| `transcription.executable` | het programma van de engine, een absoluut pad; leeg: gezocht in het `PATH` | leeg |
| `transcription.model` | het modelbestand, een absoluut pad | leeg |
| `transcription.vad-model` | het spraakactiviteitsmodel, een absoluut pad; leeg: geen, en de stiltes worden niet overgeslagen. Een instelling die een engine niet kan gebruiken wordt zonder een woord genegeerd | leeg |
| `transcription.language` | de taal van de audio, twee letters; leeg: gedetecteerd | leeg |
| `transcription.suppress-notes` | geluiden niet als notities schrijven | `true` |
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

## Een deel transcriberen

`--start` en `--end` beperken het werk tot het deel van de film ertussen. Een van beide mag alleen worden opgegeven: een
`--start` alleen loopt tot het einde, een `--end` alleen begint bij het begin.

```bash
subedit-cli transcribe --start 2:00 --end 3:30 --output test.srt --model large-v3.bin film.mkv
```

- **Een tijd wordt geschreven als `M:SS` of `H:MM:SS`, beide met een decimaal deel na een punt (`2:00.5`), of als een aantal
  seconden (`90`, `90.5`)**. Een komma wordt geweigerd, zoals overal in het hulpmiddel.
- **De ondertitels behouden de tijd van de film**: de regel die op 2:10 van de film wordt gezegd, wordt geschreven op `00:02:10`, niet
  op `00:00:10`. Het bestand kan zoals het is over de hele film worden gelegd.
- **De randen zijn bij benadering**: de engine werkt met vensters van dertig seconden, dus wat hij teruggeeft wordt achteraf
  op het bereik afgeknipt; een zin die over een rand heen loopt, wordt daar afgeknipt.
- **Er begint niets als het bereik onbruikbaar is**: een grens die geen tijd is, of een `--end` die niet na
  `--start` komt, is een gebruiksfout die het probleem noemt, niets schrijft en noch `ffmpeg` noch de
  engine start.

De audio van de hele film wordt toch uitgepakt: het is de transcriptie, veel langer, die het bereik bespaart.

## De talen opsommen

`--list-languages` schrijft de talen die de engine **voor het model** biedt, één per regel: de code die `--language` neemt, een tab, en de naam
van de taal in de taal van de interface. Het heeft geen invoer en geen uitvoer nodig, en transcribeert niets.

```bash
subedit-cli transcribe --list-languages --model ggml-small.bin
```

```text
en	English
zh	Chinese
de	German
…
```

De lijst hangt af van de engine en van het model: ongeveer honderd talen voor een meertalig whisper.cpp-model, alleen `en` voor een model dat alleen
Engels kent. Een engine die zijn talen niet kan opsommen, somt er geen op. Een model dat niet is opgegeven of er niet is, is een fout, met de verwijzing naar de gids; alleen
de tekstvorm, er wordt niets geschreven met [`--format json`](invocation.md#uitvoer-die-een-script-kan-lezen).

## Uitvoer

**Standaarduitvoer** — niets: het resultaat is het geschreven bestand.

| Niveau | Wat er op de foutuitvoer wordt toegevoegd |
| :----- | :------------------------------------- |
| 1 | `<input>: N subtitles transcribed, language: xx -> <output>` |
| 2 | de stap en de voortgang, een regel per procent: `extracting the audio…`, dan `transcribing…`, dan `transcribing: 25%, 0:12:30 of 0:50:00, about 38 min left, elapsed: 0:05:12` — het procent, de bereikte positie in de audio, de resterende tijd en de verstreken tijd; **de regel wordt elke tien seconden herhaald als er niets nieuws is gekomen**, zodat een lange wachttijd niet voor een vastloper wordt gehouden |
| 3 | en elke nieuwe tekst die de engine krijgt, `last text: …` |

**Het procent is de positie van de engine in de audio, niet een telling van zijn eigen vensters**: de engine schrijft wat hij
heeft getranscribeerd gaandeweg, en de opdracht leest hoe ver die is, zodat het getal vanaf de eerste minuut beweegt. (De eigen
voortgang van de engine komt in stappen van 5 % van de audio, nadat een venster van dertig seconden klaar is: op zichzelf kan die
een kwartier op 0 % blijven bij een lange film met een groot model op een processor.) **De resterende tijd is eerst stil**: hij wordt
gegeven zodra er genoeg werk is gedaan om uit te extrapoleren, en volgt de snelheid van de laatste minuut, niet het gemiddelde van de
hele run.

### In JSON

Met [`--format json`](invocation.md#uitvoer-die-een-script-kan-lezen) (de globale optie, vóór het
subcommando geschreven) bevat het object van het bestand de gebruikte taal in `language`, `counts.subtitles` (de
geschreven ondertitels) en `counts.segments` (de stukken van de engine waaruit ze zijn gemaakt), en `audio` wanneer
`--keep-audio` de audio heeft bewaard.

## Mededelingen

Eén mededeling wordt op het eerste niveau gedaan, vóór het werk, omdat ze verandert wat u mag verwachten:

- **Met een model alleen voor Engels**: `This model is for English only: whatever language the film
  speaks, the result will be English, or notes in brackets. Choose a multilingual model in the preferences.` Een model waarvan de bestandsnaam eindigt op
  `.en` is er een.

## Afsluitcodes

Die van het hulpmiddel: `0` als het bestand is geschreven, `2` als dat niet kon — een hulpmiddel of een model dat ontbreekt, een
taal die het model niet aankan, een mislukte extractie of transcriptie —, `1` bij een gebruiksfout, zoals een
engine die niemand heeft geregistreerd of een bereik dat onbruikbaar is.

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
| `<input>` of `--output` ontbreekt | `input is required` of `--output is required` |
| `--suppress-notes` met `--keep-notes` | `--suppress-notes and --keep-notes say opposite things; give one of them` |
| `--suppress-notes` voor een engine waaraan dat niet kan worden opgedragen | `the engine NAME cannot be told not to write sounds as notes` |
| een taal die geen code is | `"TEXT" is not a language code: write two letters, such as fr, en, de or es` |
| een grens van het bereik die geen tijd is | `"TEXT" is not a time: write it as 2:00, 1:02:03 or 2:00.5, or in seconds` |
| `--end` niet na `--start` | `the end of the range must come after its start` |
| elke andere mislukking | `the transcription failed`, met wat de engine zei |

De verwijzing naar de gids is `See the manual page "Installing the transcription tools"
(install-transcription).`
