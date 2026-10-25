# `transcribe`

<!-- machine-translated -->
> Ca págine es un traduction machinal e it ancor ne ha esset revidet de un lector del lingue.

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
- **Un lingue es tipat con du líttere — `fr`, `en`, `de` —, o con li code de tri líttere quel un file declara, `fra`,
  `eng`, quel es lect quam ti de du líttere.** Omnicos altri es refusat ante que qualcunc labor comencea. **Li detection
  escuta solmen li prim triant secondes**: un film quel comensa con musica o silentie posse esser prenet por un
  altri lingue, e li motor scri tande li parlada *tradutet* in ti lingue — un film francesi prenet por anglesi
  ixe in anglesi, o quam `[SPEAKING FRENCH]`. Quande vu save li lingue, dona it.
- **On posse transcrir solmen un parte del film**, con `--start` e `--end` — vide
  [Transcrir un parte](#transcrir-un-parte). It es li manie de provar un model sur du minutes ante lansar it sur
  du hores.
- **Li lingues quel un motor ofri posse esser listat**, con `--list-languages` — vide
  [Listar li lingues](#listar-li-lingues).
- **Li sones ne es scrit quam notes, per defecte**: li motor recive li órdine de ne far it (`--suppress-notes`; su propri option es
  `--suppress-nst`), por que `(Sonnerie de téléphone)` o `[MUSIC PLAYING]` ne apare in li subtitules, ni in parentheses ni in crochetes.
  `--keep-notes` scri les, por subtitules por li dur de audition. Con li notes prohibit, li motor scri `...` por un passage sin parol, e **li post-tractation lassa cader li fragmentes
  quel have null líttere e null cifre** — `...`, `…`, `♪` — ante junter e talliar; `--raw` conserva les, quam li fragmentes del motor ha venit.
- **Li segmentes es fusionat e tallit** secun li límites del parametres (`transcription.merge-gap-ms`,
  `transcription.max-duration-ms`, `transcription.max-characters`): du fragmentes de un sam frase quel es
  proxim es un subtitul, e un fragment tro long o tro large es tallit — unmen a un fine de frase, poy a un comma, un punctocomma o un duplic puncte, e in li medie de un frase solmen si ples ne es punctuation u talliar —, evitante un talliada quel lassa un subtitul tro small por esser leet. Li tri etapes es fat in órdine, chascun sur li tot transcription: lassar cader, unir, talliar. Un subtitul quel comensa un fragment conserva su comension, e ti quel fini it conserva su fine. **`--raw`
  scri li segmentes del motor talmen quam ili vennit**, un per subtitul, quel anc es li manie de vider quo
  fa li fusion.

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

## Arguments e optiones

| Option | Obligatori | Valores | Predefinit |
| :----- | :----- | :------ | :----- |
| `<input>` | si, except con `--list-languages` | un video o un file audio | — |
| `--output` | si, except con `--list-languages` | li file de subtitules a scrir; su dossier es creat si it manca | — |
| `--to` | no | li curt nómine de un formate: `srt`, `vtt`, `subviewer2`, `ssa`, `ass`, `microdvd`, `mpl2`, `tmplayer`, `lrc` | li formate quel li extension de `--output` indica, altrimen `srt` |
| `--audio-track` | no | un númere integri ex 0: li position inter li pistas audio del video | `0`, li unesim |
| `--language` | no | du líttere, un code ISO 639-1: `en`, `fr`; un code de tri líttere quam `fra` es lect quam `fr` | li parametre `transcription.language`; vacui: detectet |
| `--suppress-notes` | no | un flag: ne scrir li sones quam notes; refusat por un motor a quel on ne posse dir it | li parametre `transcription.suppress-notes` |
| `--keep-notes` | no | un flag: scrir les, contra li parametre; li contrari de `--suppress-notes`, e li du together es refusat | li parametre `transcription.suppress-notes` |
| `--list-languages` | no | un flag: listar li lingues quel li motor ofri por li model, e transcrir nequo | desactivat |
| `--start` | no | un témpor: `2:00`, `1:02:03`, `2:00.5`, o un númere de secondes, `90` | li comense del audio |
| `--end` | no | un témpor, scrit quam por `--start`; it deve venir pos `--start` | li fine del audio |
| `--model` | no | li file de model | li parametre `transcription.model` |
| `--vad-model` | no | un model de activitá de voce (un model Silero por whisper.cpp), quel fa que li machine saltea li silentias ante decodar: su segmentes dunc sequa li parol, e li unesim comensa u comensa li parol vice a 0. Li file deve exister, e un machine quel ne posse usar it refusa li optione per su nómine | li parametre `transcription.vad-model` |
| `--engine` | no | li nómine de un motor: `whisper.cpp` | li parametre `transcription.engine` |
| `--keep-audio` | no | un file por conservar li audio extraet, quam PCM mono de 16 bit in un file WAV, al frequentie de sampling quel li motor demanda (16 kHz por whisper.cpp); su directoria es creat si it manca, e li nómine posse portar `{rate}`, remplazzat per ti frequentie in hertz (`film.{rate}.wav` es scrit `film.16000.wav`) | li audio ne es conservat in null loc |
| `--raw` | no | un flag: scrir li segmentes del motor sin fusion e sin talliar | off |

Li **parametres** es leet del file de parametres quam ti de [`adjust`](adjust.md) — vide
[Li file de configuration](invocation.md#li-file-de-configuration) — e chascun have ci su option:

| Clave | Signification | Predefinit |
| :-- | :------ | :------ |
| `transcription.engine` | li motor | `whisper.cpp` |
| `transcription.executable` | li programma del motor, un chemin absolut; vacui: serchat in `PATH` | vacui |
| `transcription.model` | li file de model, un chemin absolut | vacui |
| `transcription.vad-model` | li model de activitá de voce, un chemin absolut; vacui: null, e li silentias ne es salteat. Un parametre quel un machine ne posse usar es lassat sin parol | vacui |
| `transcription.language` | li lingue del audio, du líttere; vacui: detectet | vacui |
| `transcription.suppress-notes` | ne scrir li sones quam notes | `true` |
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

## Transcrir un parte

`--start` e `--end` limita li labor al parte del film inter ambi. On posse dar solmen un: un `--start` sol va
til li fine, un `--end` sol comensa ye li comense.

```bash
subedit-cli transcribe --start 2:00 --end 3:30 --output test.srt --model large-v3.bin film.mkv
```

- **Un témpor es scrit `M:SS` o `H:MM:SS`, si on vole con un parte decimal pos un punctu (`2:00.5`), o quam un númere de
  secondes (`90`, `90.5`)**. Un comma es refusat, quam omnicubi in li instrument.
- **Li subtitules conserva li témpor del film**: li frase dit a 2:10 del film es scrit a `00:02:10`, ne a
  `00:00:10`. Li file posse esser posit talmen sur li tot film.
- **Li bordes es aproximativ**: li motor labora per fenestres de trenta secondes, dunc quo it retorna es tallit
  al interval pos: un frase quel transpassa un bord es tallit ibi.
- **Nequo comensa si li interval ne posse esser usat**: un límite quel ne es un témpor, o un `--end` quel ne es pos
  `--start`, es un errore de usation quel nomina li problema, scri nequo e lansa ni `ffmpeg` ni li motor.

Li audio del tot film es tamen extraet: li transcription, mult plu long, es quo li interval economisa.

## Listar li lingues

`--list-languages` scri li lingues quel li motor ofri **por li model**, un per linea: li code quel `--language` prende, un tabulator, e li nómine
del lingue in li lingue del interfacie. It besona null input ni output, e transcri nequo.

```bash
subedit-cli transcribe --list-languages --model ggml-small.bin
```

```text
en	English
zh	Chinese
de	German
…
```

Li liste depende del motor e del model: circa cent lingues por un model multilingual de whisper.cpp, solmen `en` por un model solmen por
anglesi. Un motor quel ne posse listar su lingues lista null. Un model quel ne es dat o ne es ibi es un errore, con li indication al guide; solmen li
forme textual, nequo es scrit con [`--format json`](invocation.md#un-output-quel-un-script-posse-leer).

## Output

**Exeunte standard** — nequo: li resultate es li file scrit.

| Nivelle | Quo es adjuntet al exeunte de error |
| :---- | :-------------------------------- |
| 1 | `<input>: N subtitles transcribed, language: xx -> <output>` |
| 2 | li pass e li progress, un linea per procent: `extracting the audio…`, pois `transcribing…`, pois `transcribing: 25%, 0:12:30 of 0:50:00, about 38 min left, elapsed: 0:05:12` — li procent, li position atinget in li audio, li témpor restant, e li témpor passat; **li linea es dit denov chascun dece secondes quande nequo nov ha venit**, por que un long atende ne esset prenet por un blocage |
| 3 | e chascun nov textu quel li motor obtene, `last text: …` |

**Li procent es li position del motor in li audio, ne un conte de su proprie fenestres**: li motor scri quo it ha
transcrit durant que it labora, e li comande lee u it es, tal que li númere move ja del prim minute. (Li progress
propri del motor veni per passus de 5 % del audio, pos que un fenestre de trenta secondes es finit: sol, it posse restar
a 0 % un quart de hora sur un long film con un grand model sur un processor.) **Li témpor restant es mutu in li comense**: it es dat
quande suficent labor es fat por extrapolar, e it seque li rapiditá del ultim minute, ne li mediana del tot execution.

### In JSON

Con [`--format json`](invocation.md#un-output-quel-un-script-posse-leer) (li option global, scrit ante li
subcomande), li objecte del file porta li lingue usat in `language`, `counts.subtitles` (li
subtitules scrit) e `counts.segments` (li fragmentes del motor ex quel ili esset fat), e `audio` quande
`--keep-audio` ha conservat li audio.

## Avisos

Un avise es dit al prim nivelle, ante li labor, nam it changea quo on deve atender:

- **Con un model solmen por anglesi**: `This model is for English only: whatever language the film
  speaks, the result will be English, or notes in brackets. Choose a multilingual model in the preferences.` Un model
  quel nómine de file fini con `.en` es un tal.

## Codes de exeunte

Ti del instrument: `0` si li file esset scrit, `2` si it ne posset esser — un instrument o un model mancant, un
lingue quel li model ne posse acceptar, un extraction o un transcription fallit —, `1` pro un errore de usation, quam un
motor quel nequi ha registrat o un interval quel ne posse esser usat.

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
| `<input>` o `--output` mancant | `input is required` o `--output is required` |
| `--suppress-notes` con `--keep-notes` | `--suppress-notes and --keep-notes say opposite things; give one of them` |
| `--suppress-notes` por un motor a quel on ne posse dir it | `the engine NAME cannot be told not to write sounds as notes` |
| un lingue quel ne es un code | `"TEXT" is not a language code: write two letters, such as fr, en, de or es` |
| un límite del interval quel ne es un témpor | `"TEXT" is not a time: write it as 2:00, 1:02:03 or 2:00.5, or in seconds` |
| `--end` ne pos `--start` | `the end of the range must come after its start` |
| qualcunc altri fallimentu | `the transcription failed`, con quo li motor ha dit |

Li indication al guide es `See the manual page "Installing the transcription tools"
(install-transcription).`
