# `transcribe`

<!-- machine-translated -->
> Questa pagina è una traduzione automatica e non è ancora stata rivista da un lettore della lingua.

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

Scrive **il parlato di un video, o di un file audio, come file di sottotitoli**. L'audio viene preparato,
affidato a un motore che trova che cosa viene detto e quando, e i pezzi che il motore taglia vengono uniti in
sottotitoli che rispettano una durata e una lunghezza.

**Nulla di ciò che serve è fornito con il programma**: il motore (whisper.cpp), un modello e `ffmpeg`
vengono installati dall'utente, che indica dove si trova il modello. [Installare gli strumenti di
trascrizione](install-transcription.md) dice dove trovare ciascuno e quale modello scegliere. **La loro assenza non
rompe nulla**: il comando dice che cosa manca, con il rimando a quella pagina, non scrive nulla ed
esce con un codice diverso da zero.

- **L'input è un video o un file audio.** Da un video, `ffmpeg` estrae una traccia audio; senza
  `ffmpeg`, un file audio che il motore legge così com'è (`wav`, `mp3`, `flac`, `ogg`) viene comunque
  trascritto, ma un video no, e nemmeno alcun file quando si chiede una traccia.
- **Un solo file alla volta**, e la destinazione è `--output`: non esistono né `--output-dir` né
  `--in-place`, poiché il risultato è un nuovo file e non una riscrittura.
- **La lingua viene rilevata** a meno che `--language` non la imponga, e la lingua usata viene indicata. Un
  modello fatto solo per l'inglese non rileva nulla e non trascrive altro: una lingua che non può
  gestire viene rifiutata *prima* di qualsiasi lavoro, indicando il limite del modello.
- **I segmenti vengono uniti e tagliati** secondo i limiti delle impostazioni (`transcription.merge-gap-ms`,
  `transcription.max-duration-ms`, `transcription.max-characters`): due pezzi di una stessa frase che sono
  vicini formano un solo sottotitolo, e un pezzo troppo lungo o troppo largo viene tagliato a una parola. **`--raw`
  scrive i segmenti del motore così come sono arrivati**, uno per sottotitolo, che è anche il modo per vedere che cosa
  fa l'unione.

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

## Argomenti e opzioni

| Opzione | Obbligatoria | Valori | Predefinito |
| :----- | :----- | :------ | :----- |
| `<input>` | sì | un video o un file audio | — |
| `--output` | sì | il file di sottotitoli da scrivere; la sua cartella viene creata se manca | — |
| `--to` | no | il nome breve di un formato: `srt`, `vtt`, `subviewer2`, `ssa`, `ass`, `microdvd`, `mpl2`, `tmplayer`, `lrc` | il formato indicato dall'estensione di `--output`, altrimenti `srt` |
| `--audio-track` | no | un numero intero da 0: la posizione tra le tracce audio del video | `0`, la prima |
| `--language` | no | due lettere minuscole, un codice ISO 639-1: `en`, `fr` | l'impostazione `transcription.language`; vuota: rilevata |
| `--model` | no | il file del modello | l'impostazione `transcription.model` |
| `--engine` | no | il nome di un motore: `whisper.cpp` | l'impostazione `transcription.engine` |
| `--keep-audio` | no | un file in cui conservare l'audio estratto, come PCM a 16 kHz mono a 16 bit in un file WAV; la sua cartella viene creata se manca | l'audio non viene conservato da nessuna parte |
| `--raw` | no | un'opzione senza valore: scrive i segmenti del motore senza unioni né tagli | disattivata |

Le **impostazioni** sono lette dal file delle impostazioni come quelle di [`adjust`](adjust.md) — vedi
[il file delle impostazioni](invocation.md#il-file-delle-impostazioni) — e ciascuna ha qui la sua opzione:

| Chiave | Significato | Predefinito |
| :-- | :------ | :------ |
| `transcription.engine` | il motore | `whisper.cpp` |
| `transcription.executable` | il programma del motore, un percorso assoluto; vuoto: cercato nel `PATH` | vuoto |
| `transcription.model` | il file del modello, un percorso assoluto | vuoto |
| `transcription.language` | la lingua dell'audio, due lettere; vuoto: rilevata | vuoto |
| `transcription.merge-gap-ms` | i pezzi di una stessa frase più vicini di questo numero di millisecondi vengono uniti | `400` |
| `transcription.max-duration-ms` | la durata massima di un sottotitolo sullo schermo, in millisecondi | `7000` |
| `transcription.max-characters` | il numero massimo di caratteri di un sottotitolo, a capo non contati | `84` |

## Scegliere l'audio

Un video può avere più tracce audio — l'originale, un commento, un'altra lingua. `--audio-track`
prende la **posizione tra le tracce audio** del file, da 0, nell'ordine in cui il file le elenca. Una
traccia che il video non ha viene rifiutata, **con quelle che ha**, in modo che la chiamata successiva
sia quella giusta:

```bash
subedit-cli transcribe --audio-track 4 --output film.srt --model small.bin film.mkv
```

```text
film.mkv: the video has no audio track 4
audio track 0: fra, Original
audio track 1: eng, Commentary
```

## Output

**Output standard** — niente: il risultato è il file scritto.

| Livello | Che cosa si aggiunge sull'output di errore |
| :----- | :------------------------------------- |
| 1 | `<input>: N subtitles transcribed, language: xx -> <output>` |
| 2 | l'avanzamento del motore, `transcribing: 25%`, come lo riporta — a passi di circa trenta secondi di audio |

### In JSON

Con [`--format json`](invocation.md#un-output-che-uno-script-può-leggere) (l'opzione globale, scritta prima del
sottocomando), l'oggetto del file porta la lingua usata in `language`, `counts.subtitles` (i
sottotitoli scritti) e `counts.segments` (i pezzi del motore da cui sono stati ricavati), e `audio` quando
`--keep-audio` ha conservato l'audio.

## Codici di uscita

Quelli dello strumento: `0` se il file è stato scritto, `2` se non è stato possibile — uno strumento o un modello mancante, una
lingua che il modello non può gestire, un'estrazione o una trascrizione non riuscita —, `1` per un errore d'uso, come un
motore che nessuno ha registrato.

## Errori

| Causa | Messaggio |
| :---- | :------ |
| nessun programma del motore trovato, o quello configurato non c'è | `the transcription engine was not found`, poi il rimando alla guida |
| nessun modello indicato, o non c'è | `no model was given, or the model file is not there`, poi il rimando alla guida |
| `ffmpeg` non trovato per un video | `ffmpeg was not found, so the audio of a video cannot be extracted`, poi il rimando |
| una traccia che il video non ha | `the video has no audio track N`, poi le tracce che ha |
| una lingua che il modello non può trascrivere | `the model cannot transcribe the language xx` |
| nessuna lingua, e un modello che non sa rilevarne una | `the model cannot detect the language: give one` |
| un motore che nessuno ha registrato | `no transcription engine is called NAME` |
| qualsiasi altro errore | `the transcription failed`, con ciò che ha detto il motore |

Il rimando alla guida è `See the manual page "Installing the transcription tools"
(install-transcription).`
