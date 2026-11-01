# `sync`

<!-- machine-translated -->
> Ca págine es un traduction machinal e it ancor ne ha esset revidet de un lector del lingue.

```
subedit-cli sync (--video FILE | --audio FILE)
                 [--audio-track N]
                 [--language CODE]
                 [--start TIME] [--end TIME]
                 [--model FILE]
                 [--vad-model FILE]
                 [--engine NAME]
                 [--keep-audio FILE]
                 [--method METHOD]
                 [--min-match PERCENT]
                 [--doubt-gap MS]
                 [--report FILE]
                 [-r]
                 [--output FILE | --output-dir DIR | --in-place] [--dry-run]
                 <files>...
```

**Retemporisa li subtitules de un file sur li audio de un film.** Li casu por quel it existe es li `.srt` descargat
in rete, movet al comense e al fine, quel un sol movement ne remena. Du metodes it fa, e `--method` (o li parametre
`alignment.method`) elige inter les:

- **`speech`, li predefinit, pone li file sur li moment u alqui parla.** It demanda al son solmen u il ha parlada, e
  trova li movement quel fa li subtitules concordar con it, parte de film per parte de film: un decalage constant, un
  derive, un duesim mità quel es tard, e un file temporisat por un altri frequentie de images (25 por 23.976, e talis)
  es omnes corriget. **It ne depende del lingue** — li subtitules posse esser in francese e li film in anglesi —, it
  besona null transcription, e it prende secondes. It besona `ffmpeg`, li programma `whisper-vad-speech-segments` quel
  veni con whisper.cpp, e un model de activitá de voce (`--vad-model`, li parametre `transcription.vad-model`).
- **`words` fa corresponder li paroles de chascun subtitul con li paroles quel un motor ha audit.** It audi con un motor
  de transcription — li sam quam ti de [`transcribe`](transcribe.md) — e chascun subtitul prende li loc u su paroles
  es dit. Nequo es traductet: li subtitules deve esser in li lingue quel es parlat. It posse placiar un sol subtitul
  quel es foris de su loc in un file quel es altrimen corect, e it prende li témpor de un transcription.

**Quo `speech` ne fa es dit chascun vice quel it funde**, sur li exeunte de error e in li registre: it corrige li file
**in su tot**, nequande un subtitul a un vice. Un subtitul quel es foris de su loc por se self in un file altrimen
corect, o un titul monstrat sur un silentie (un carte de capitul, un legende quel tradue alquo sur li image), ne es
serchat e posse esser ancor fals, e li prim minutes posse resta foris quande li film comensa con musica o titules.
Verificar li resultate, o usar `--method words` quande li textu es in li lingue parlat.

**Nulquo de quo it besona es livrat con li programma**: li programmas, li model e `ffmpeg` es ti de
[Instalar li utensiles de transcription](install-transcription.md), e lor manca ne rumpe nequo — li comande dit quo
manca, con li indication a ti págine, scri null cosa e exeunte con un code ne-zero.

- **Solmen li comenses e li fines changea.** Nequande li textu, li stil, li númere de subtitules o lor órdine.
- **Un subtitul quel li audio ne da conserva su tempores** — un credit quel nequi dis, un cantu, un linea in un altri
  lingue. **Null position es nequande inventat, ni divinat ex li vicinos**: li linea resta u li file la havea, e li
  comande dit it. To *ne* es un fallimentu, e li code de exeunte es `0`.
- **Con `words`, un subtitul es trovat quande al minimum li mità de su paroles es audit** (`--min-match`,
  `alignment.min-match`), e al minimum un. Li casu, li accentus, li punctuation e li marcation ne importa; un parol quel
  diffère per un líttere de un long parol conta quam audit. `speech` plazza chascun subtitul, e prende null de
  `--language`, `--model`, `--engine` o `--min-match`.
- **Li fine es u li ultim parol fini.** Li motor temporisa li paroles approximativmen, alcun centesimes de seconde ante
  li fine propri del subtitul, dunc un subtitul posse exir un poc plu curt quam it esset.
- **Un subtitul quel entrarea in li sequent es decurtat til u li sequent comensa.** Un quel ancor superpone li
  sequent, quo decurtar ne posse curar, es lassat e dit.
- **Un file quel ne conveni al son es refusat e ne scrit**: con `words`, null subtitul esset trovat, quo es presque
  sempre li mal audio o li mal lingue — `speech` ne depende del lingue; con `speech`, li subtitules e li parlada have
  tro poc in comun, quo es presque sempre li mal film.
- **On posse audir solmen un parte del film**, con `--start` e `--end`, quam por
  [`transcribe`](transcribe.md#transcrir-un-parte): solmen li subtitules quel **comensa** in li interval es serchat e
  tanget, li altres resta quam ili es e ne es contat.
- **Li audio es audit un vice por chascun file dat.** Por retemporisar plu files sur li sam film, expecta atender
  por chascun.

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
  --method METHOD             How the subtitles are put in line: speech (from when there is speech, whatever the language) or words (from the words heard); the alignment.method setting by default
  --min-match PERCENT         Share of a subtitle's words that must be heard, in percent; the alignment.min-match setting by default
  --doubt-gap MS              How far a subtitle may move from its neighbours' shift before it is flagged, in milliseconds; the alignment.doubt-gap setting by default
  --report FILE               Write the subtitles to check to this file
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```


## Arguments e optiones

| Option | Obligatori | Valores | Predefinit |
| :----- | :------- | :----- | :------ |
| `<files>` | si | li files de subtitules a retemporisar; con `-r`, directorias | — |
| `--recursive`, `-r` | no | un flag | off — vide [Tractar un arbore](lots.md) |
| `--video` | un de `--video` e `--audio` | li film a audir | — |
| `--audio` | un de `--video` e `--audio` | un file audio a audir, vice un film | — |
| `--audio-track` | no | un númere integri ex 0: li position inter li pistas audio del film | `0`, li unesim |
| `--language` | no | du líttere, un code ISO 639-1: `en`, `fr` | li parametre `transcription.language`; vacui: detectet |
| `--start`, `--end` | no | un témpor: `2:00`, `1:02:03`, `2:00.5`, o un númere de secondes; `--end` deve venir pos `--start` | li tot audio |
| `--model` | no | li file de model | li parametre `transcription.model` |
| `--vad-model` | no | un model de activitá de voce, quel fa que li motor saltea li silentias; con it li paroles seque plu strictmen li parol | li parametre `transcription.vad-model` |
| `--engine` | no | li nómine del motor quel audi: `whisper.cpp` | li parametre `transcription.engine` |
| `--keep-audio` | no | un file por conservar li audio extraet; su directoria es creat si it manca | li audio ne es conservat in null loc |
| `--method` | no | `speech`: pone li file sur li moment u il ha parlada, in omni lingue; `words`: fa corresponder li paroles con quo esset audit, in li lingue parlat | li parametre `alignment.method`, `speech` |
| `--min-match` | no | un númere integri de 1 til 100: li parte del paroles de un subtitul quel deve esser audit, in procent | li parametre `alignment.min-match`, `50` |
| `--doubt-gap` | no | un númere integri de millisecondes, 0 o plu: quant li movement de un subtitul posse divergir del movement de su vicinos ante que it es marcat | li parametre `alignment.doubt-gap`, `2000` |
| `--report` | no | un file in quel scrir li subtitules a verificar, un per linea: li númere del subtitul, un tabulator, e pro quo; un file de subtitules a un vice | null cosa es scrit |
| `--output` / `--output-dir` / `--in-place` | **un del tri**, except con `--dry-run` | vide [Invocation](invocation.md#li-destination) | — |
| `--dry-run` | no | un flag: calcula e dis, scri null — vide [Vider ante scrir](invocation.md#vider-ante-scrir) | off |

Li destination — `--output`, `--output-dir`, `--in-place`, `--dry-run` — e `-r` es ti del altri comandes quel rescri un
file: vide [li destination](invocation.md#li-destination). Li file conserva su formate, su codification e su
fines de linea.

Li **parametres** es leet del file de parametres — vide
[li file de configuration](invocation.md#li-file-de-configuration) — e chascun have ci su option. Li motor, li model e
li model de activitá de voce es ti de `transcribe`.

| Clave | Signification | Predefinit |
| :-- | :------ | :------ |
| `alignment.engine` | li motor de allineament del metode `words`; il ha un, `transcript-matching`, quel audi e fa corresponder | `transcript-matching` |
| `alignment.method` | qualmen li subtitules es mett in linea: `speech` o `words` | `speech` |
| `alignment.min-match` | li parte del paroles de un subtitul quel deve esser audit por que it es trovat, in procent | `50` |
| `alignment.doubt-gap` | quant, in millisecondes, li movement de un subtitul posse divergir del movement de su vicinos ante que it es marcat | `2000` |

## Quo es marcat

Li comande dit qual subtitules verificar, e pro quo. Un subtitul es marcat por li **prim** de ti rasones quel vale:

| Pro quo | Signification |
| :-- | :------ |
| ne trovat in li audio, lassat quam it esset | min de `--min-match` de su paroles esset audit: it conserva su tempores |
| audit solmen in parte | trovat, ma min de 80 % de su paroles esset audit |
| movet altrimen que su vicinos | trovat, ma su movement diverge del movement median del du subtitules de chascun costa per plu de `--doubt-gap` |

Con `speech` chascun subtitul es plazzat, e solmen li ultim de ti rasones posse valer: u li file changea su movement per
plu quam `--doubt-gap` de un subtitul al sequent (un duesim mità tard), li subtitules de chascun costa del passu es
marcat.

Un subtitul posse anc esser **decurtat til li comense del sequent subtitul**, o **ancor superponer li sequent subtitul**.

Du númeres conta li histoire del file in su tot. **Li movement median** es li medie de quant li subtitules ha movet: un
file simplicmen tard li lee ci, e null altrubi. **Li derive** es li movement median del ultim dessim del subtitules
minus ti del prim dessim: it es proxim a zero por un decalage constant, e grand por un file quel esset tard per plu e
plu durant li film.

## Output

**Exeunte standard** — nequo: li resultate es li file scrit.

| Nivelle | Quo es dit sur li exeunte de error |
| :---- | :-------------------------------- |
| 1 | `<file>: N of M subtitles placed on the audio, K to check; median shift X s, drift Y s -> <destination>` — e, quande un frequentie de images esset corriget, li proportion; con `speech`, **li avise de quo it ne fa**, sur un linea a part |
| 2 | li pass e li progress, quam por [`transcribe`](transcribe.md#output) |

### In JSON

Con [`--format json`](invocation.md#un-output-quel-un-script-posse-leer), li objecte del file porta `counts`:
`subtitles` (serchat), `placed` (qui have un loc), `moved` (de quel li tempores ha changeat), `partial`, `out_of_line`,
`not_found`, `median_shift_ms` e `drift_ms`; e `flagged`, un liste de `{"subtitle": N, "kind": "…"}` u li númere es li
position del subtitul in li file e `kind` es un de `not-found`, `partial`, `out-of-line`, `cut-back` e `overlaps`.
Con `speech`, `warnings` porta un intrada de `kind` `speech-only`. Li ids es promisset; li frases no.

## Exemple

```bash
subedit-cli sync --video film.mkv --vad-model silero.bin \
    --report to-check.txt --output film.synced.srt film.srt
```

Por plazzar chascun subtitul secun su paroles, por un textu in li lingue parlat:

```bash
subedit-cli sync --method words --video film.mkv --model small.bin --vad-model silero.bin \
    --output film.synced.srt film.srt
```

## Codes de exeunte

Ti del instrument: `0` si li file esset scrit, subtitules marcat o no, `2` si it ne posset esser — un instrument o un
model mancant, un extraction o un audition fallit, un file quel ne conveni al son (null de su subtitules trovat, o tro
poc in comun con li parlada) —, `1` pro un errore de usation, quam ni `--video` ni `--audio` (o ambi), un option foris
de su interval, o un interval quel ne posse esser usat.

## Errores

| Causa | Missage |
| :---- | :------ |
| ni `--video` ni `--audio`, o ambi | `sync needs the audio to listen to: give --video or --audio, and not both` |
| null programma de motor trovat, o li configurat ne es ibi | `the transcription engine was not found`, poy li indication al guide |
| null model donat, o it ne es ibi | `no model was given, or the model file is not there`, poy li indication al guide |
| `ffmpeg` ne trovat por un film | `ffmpeg was not found, so the audio of a video cannot be extracted`, poy li indication |
| un pista quel li film ne have | `the video has no audio track N`, poy li pistas quel it have |
| un lingue quel li model ne posse transcrir | `the model cannot transcribe the language xx` |
| un motor quel nequi ha registrat | `no transcription engine is called NAME`, o `there is no alignment engine called NAME` |
| null subtitul trovat (`words`) | `none of the N subtitles was found in the audio: is it the right audio, and the right language?`, poy `The method based on speech does not depend on the language.` |
| li model de activitá de voce ne nominat, o li programma quel trova li parlada ne trovat (`speech`) | `Finding where there is speech needs a voice-activity model: name one in the transcription.vad-model setting.` o `Finding where there is speech needs the program whisper-vad-speech-segments, which comes with whisper.cpp, and it was not found.`, poy li indication al guide |
| li subtitules e li parlada have tro poc in comun (`speech`) | `The sound and the subtitles have too little in common to be put in line: is it the right film?` |
| `--min-match` foris 1 til 100, `--doubt-gap` negativ | li errore de usation del option |
| `--report` con plu de un file de subtitules | `--report names one file: give one subtitle file when you use it` |
