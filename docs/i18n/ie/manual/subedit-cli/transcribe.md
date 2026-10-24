# `transcribe`

<!-- machine-translated -->
> Ca págine es un traduction machinal e it ancor ne ha esset revidet de un lector del lingue.

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

Scri **li parol de un video, o de un file audio, quam file de subtitules**. Li audio es preparat,
dat a un motor quel trova quo es dit e quande, e li fragmentes quel it talla es fusionat in
subtitules quel respecte un duration e un long.

**Nulquo de quo it besona es livrat con li programma**: li motor (whisper.cpp), un model e `ffmpeg`
es instalat de l'usator, quel dit u es li model. [Instalar li utensiles de
transcription](install-transcription.md) dit u trovar chascun e quel model selecter. **Lor
manca ne rumpe nequo**: li comande dit quo manca, con li indication a ti págine, scri null cosa
e exeunte con un code ne-zero.

- **Li intrata es un video o un file audio.** De un video, `ffmpeg` extrae un pista audio; sin
  `ffmpeg`, un file audio quel li motor leer talmen (`wav`, `mp3`, `flac`, `ogg`) es ancor
  transcrit, ma un video ne, e null file ne plu quande un pista es petit.
- **Solmen un file a un vice**, e li destination es `--output`: il ne ha `--output-dir` o
  `--in-place`, pro que li resultate es un nov file e ne un reescritura.
- **Li lingue es detectet** si `--language` ne fortia it, e li lingue usat es dit. Un
  model fat solmen por anglesi ne detecte nequo e ne transcrive altri cosa: un lingue quel it ne posse
  acceptar es refusat *ante* que qualcunc labor comencea, nominante li límite del model.
- **Li segmentes es fusionat e tallit** secun li límites del parametres (`transcription.merge-gap-ms`,
  `transcription.max-duration-ms`, `transcription.max-characters`): du fragmentes de un sam frase quel es
  proxim es un subtitul, e un fragment tro long o tro large es tallit a un parol. **`--raw`
  scri li segmentes del motor talmen quam ili vennit**, un per subtitul, quel anc es li manie de vider quo
  fa li fusion.

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

## Arguments e optiones

| Option | Obligatori | Valores | Predefinit |
| :----- | :----- | :------ | :----- |
| `<input>` | yes | un video o un file audio | — |
| `--output` | yes | li file de subtitules a scrir; su dossier es creat si it manca | — |
| `--to` | no | li curt nómine de un formate: `srt`, `vtt`, `subviewer2`, `ssa`, `ass`, `microdvd`, `mpl2`, `tmplayer`, `lrc` | li formate quel li extension de `--output` indica, altrimen `srt` |
| `--audio-track` | no | un númere integri ex 0: li position inter li pistas audio del video | `0`, li unesim |
| `--language` | no | du líttere minuscul, un code ISO 639-1: `en`, `fr` | li parametre `transcription.language`; vacui: detectet |
| `--model` | no | li file de model | li parametre `transcription.model` |
| `--engine` | no | li nómine de un motor: `whisper.cpp` | li parametre `transcription.engine` |
| `--keep-audio` | no | un file in quel conservar li audio extraet, quam PCM mono de 16 bits a 16 kHz in un file WAV; su dossier es creat si it manca | li audio ne es conservat in null loc |
| `--raw` | no | un flag: scrir li segmentes del motor sin fusion e sin talliar | off |

Li **parametres** es leet del file de parametres quam ti de [`adjust`](adjust.md) — vide
[Li file de configuration](invocation.md#li-file-de-configuration) — e chascun have ci su option:

| Clave | Signification | Predefinit |
| :-- | :------ | :------ |
| `transcription.engine` | li motor | `whisper.cpp` |
| `transcription.executable` | li programma del motor, un chemin absolut; vacui: serchat in `PATH` | vacui |
| `transcription.model` | li file de model, un chemin absolut | vacui |
| `transcription.language` | li lingue del audio, du líttere; vacui: detectet | vacui |
| `transcription.merge-gap-ms` | fragmentes de un sam frase plu proxim ca ti númere de millisecondes es junit | `400` |
| `transcription.max-duration-ms` | li max long duration quel un subtitul resta sur li ecran, in millisecondes | `7000` |
| `transcription.max-characters` | li max númere de characteres de un subtitul, sin contar li ruptiones de linea | `84` |

## Selecter li audio

Un video posse portar plu pistas audio — li original, un commentario, un altri lingue. `--audio-track`
prende li **position inter li pistas audio** del file, ex 0, in li órdine in quel li file les lista. Un
pista quel li video ne have es refusat, **con li pistas quel it have**, por que li sequent apelle
es li ver:

```bash
subedit-cli transcribe --audio-track 4 --output film.srt --model small.bin film.mkv
```

```text
film.mkv: the video has no audio track 4
audio track 0: fra, Original
audio track 1: eng, Commentary
```

## Output

**Exeunte standard** — nequo: li resultate es li file scrit.

| Nivelle | Quo es adjuntet al exeunte de error |
| :---- | :-------------------------------- |
| 1 | `<input>: N subtitles transcribed, language: xx -> <output>` |
| 2 | li progress del motor, `transcribing: 25%`, quam it lo reporta — per passus de circa trenta secondes de audio |

### In JSON

Con [`--format json`](invocation.md#un-output-quel-un-script-posse-leer) (li option global, scrit ante li
subcomande), li objecte del file porta li lingue usat in `language`, `counts.subtitles` (li
subtitules scrit) e `counts.segments` (li fragmentes del motor ex quel ili esset fat), e `audio` quande
`--keep-audio` ha conservat li audio.

## Codes de exeunte

Ti del instrument: `0` si li file esset scrit, `2` si it ne posset esser — un instrument o un model mancant, un
lingue quel li model ne posse acceptar, un extraction o un transcription fallit —, `1` pro un errore de usation, quam un
motor quel nequi ha registrat.

## Errores

| Causa | Missage |
| :---- | :------ |
| null programma de motor trovat, o li configurat ne es ibi | `the transcription engine was not found`, poy li indication al guide |
| null model donat, o it ne es ibi | `no model was given, or the model file is not there`, poy li indication al guide |
| `ffmpeg` ne trovat por un video | `ffmpeg was not found, so the audio of a video cannot be extracted`, poy li indication |
| un pista quel li video ne have | `the video has no audio track N`, poy li pistas quel it have |
| un lingue quel li model ne posse transcrir | `the model cannot transcribe the language xx` |
| null lingue, e un model quel ne posse detecter un | `the model cannot detect the language: give one` |
| un motor quel nequi ha registrat | `no transcription engine is called NAME` |
| qualcunc altri fallimentu | `the transcription failed`, con quo li motor ha dit |

Li indication al guide es `See the manual page "Installing the transcription tools"
(install-transcription).`
