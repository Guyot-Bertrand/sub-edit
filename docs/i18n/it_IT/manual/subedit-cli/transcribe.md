# `transcribe`

<!-- machine-translated -->
> Questa pagina è una traduzione automatica e non è ancora stata rivista da un lettore della lingua.

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
- **Una lingua si digita con due lettere — `fr`, `en`, `de` —, oppure con il codice di tre lettere che un file dichiara, `fra`,
  `eng`, letto come quello di due lettere.** Tutto il resto viene rifiutato prima di qualsiasi lavoro. **Il rilevamento
  ascolta solo i primi trenta secondi**: un film che si apre con musica o silenzio può essere scambiato per
  un'altra lingua, e il motore scrive allora il parlato *tradotto* in quella lingua — un film francese scambiato
  per inglese esce in inglese, o come `[SPEAKING FRENCH]`. Quando conosci la lingua, indicala.
- **Si può trascrivere solo una parte del film**, con `--start` e `--end` — vedi
  [Trascrivere una parte](#trascrivere-una-parte). È il modo per provare un modello su due minuti prima di lanciarlo su
  due ore.
- **Le lingue che un motore offre si possono elencare**, con `--list-languages` — vedi
  [Elencare le lingue](#elencare-le-lingue).
- **I suoni, per impostazione predefinita, non vengono scritti come note**: al motore è ordinato di non farlo (`--suppress-notes`; la sua opzione è
  `--suppress-nst`), in modo che `(Sonnerie de téléphone)` o `[MUSIC PLAYING]` non compaiano nei sottotitoli, tra parentesi tonde come tra quadre.
  `--keep-notes` le scrive, per sottotitoli per non udenti. Vietate le note, il motore scrive `...` per un passaggio senza parlato, e
  **la post-elaborazione scarta i pezzi che non hanno né lettere né cifre** — `...`, `…`, `♪` — prima di unire e tagliare; `--raw` li conserva,
  così come i pezzi del motore sono arrivati.
- **I segmenti vengono uniti e tagliati** secondo i limiti delle impostazioni (`transcription.merge-gap-ms`,
  `transcription.max-duration-ms`, `transcription.max-characters`): due pezzi di una stessa frase che sono
  vicini formano un solo sottotitolo, e un pezzo troppo lungo o troppo largo viene tagliato — prima a una fine di frase, poi a una virgola, un punto e virgola o due punti, e a metà frase solo quando non c'è punteggiatura dove tagliare —, evitando un taglio che lasci un sottotitolo troppo piccolo per essere letto. I tre passi si svolgono in ordine, ciascuno su tutta la trascrizione: scartare, unire, tagliare. Un sottotitolo che inizia un pezzo ne conserva l'inizio, e quello che lo termina ne conserva la fine. **`--raw`
  scrive i segmenti del motore così come sono arrivati**, uno per sottotitolo, che è anche il modo per vedere che cosa
  fa l'unione.

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

## Argomenti e opzioni

| Opzione | Obbligatoria | Valori | Predefinito |
| :----- | :----- | :------ | :----- |
| `<input>` | sì, tranne con `--list-languages` | un video o un file audio | — |
| `--output` | sì, tranne con `--list-languages` | il file di sottotitoli da scrivere; la sua cartella viene creata se manca | — |
| `--to` | no | il nome breve di un formato: `srt`, `vtt`, `subviewer2`, `ssa`, `ass`, `microdvd`, `mpl2`, `tmplayer`, `lrc` | il formato indicato dall'estensione di `--output`, altrimenti `srt` |
| `--audio-track` | no | un numero intero da 0: la posizione tra le tracce audio del video | `0`, la prima |
| `--language` | no | due lettere, un codice ISO 639-1: `en`, `fr`; un codice di tre lettere come `fra` è letto come `fr` | l'impostazione `transcription.language`; vuota: rilevata |
| `--suppress-notes` | no | un flag: non scrivere i suoni come note; rifiutato per un motore a cui non lo si può dire | l'impostazione `transcription.suppress-notes` |
| `--keep-notes` | no | un flag: scriverli, contro l'impostazione; l'opposto di `--suppress-notes`, e i due insieme sono rifiutati | l'impostazione `transcription.suppress-notes` |
| `--list-languages` | no | un flag: elencare le lingue che il motore offre per il modello, e non trascrivere nulla | disattivato |
| `--start` | no | un tempo: `2:00`, `1:02:03`, `2:00.5`, oppure un numero di secondi, `90` | l'inizio dell'audio |
| `--end` | no | un tempo, scritto come per `--start`; deve venire dopo `--start` | la fine dell'audio |
| `--model` | no | il file del modello | l'impostazione `transcription.model` |
| `--vad-model` | no | un modello di attività vocale (un modello Silero per whisper.cpp), che fa saltare i silenzi al motore prima di decodificare: i suoi segmenti seguono allora il parlato, e il primo inizia dove inizia il parlato invece che a 0. Il file deve esistere, e un motore che non può usarlo rifiuta l'opzione per nome | l'impostazione `transcription.vad-model` |
| `--engine` | no | il nome di un motore: `whisper.cpp` | l'impostazione `transcription.engine` |
| `--keep-audio` | no | un file in cui conservare l'audio estratto, come PCM mono a 16 bit in un file WAV, alla frequenza di campionamento richiesta dal motore (16 kHz per whisper.cpp); la sua cartella viene creata se manca, e il nome può contenere `{rate}`, sostituito da quella frequenza in hertz (`film.{rate}.wav` viene scritto `film.16000.wav`) | l'audio non viene conservato da nessuna parte |
| `--raw` | no | un'opzione senza valore: scrive i segmenti del motore senza unioni né tagli | disattivata |

Le **impostazioni** sono lette dal file delle impostazioni come quelle di [`adjust`](adjust.md) — vedi
[il file delle impostazioni](invocation.md#il-file-delle-impostazioni) — e ciascuna ha qui la sua opzione:

| Chiave | Significato | Predefinito |
| :-- | :------ | :------ |
| `transcription.engine` | il motore | `whisper.cpp` |
| `transcription.executable` | il programma del motore, un percorso assoluto; vuoto: cercato nel `PATH` | vuoto |
| `transcription.model` | il file del modello, un percorso assoluto | vuoto |
| `transcription.vad-model` | il modello di attività vocale, un percorso assoluto; vuoto: nessuno, e i silenzi non vengono saltati. Un'impostazione che un motore non può usare viene scartata senza una parola | vuoto |
| `transcription.language` | la lingua dell'audio, due lettere; vuoto: rilevata | vuoto |
| `transcription.suppress-notes` | non scrivere i suoni come note | `true` |
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

## Trascrivere una parte

`--start` e `--end` limitano il lavoro alla parte del film compresa tra i due. Si può dare uno solo dei due: un
`--start` da solo arriva fino alla fine, un `--end` da solo comincia dall'inizio.

```bash
subedit-cli transcribe --start 2:00 --end 3:30 --output test.srt --model large-v3.bin film.mkv
```

- **Un tempo si scrive `M:SS` o `H:MM:SS`, in entrambi i casi con una parte decimale dopo un punto (`2:00.5`), oppure come
  numero di secondi (`90`, `90.5`)**. Una virgola viene rifiutata, come ovunque nello strumento.
- **I sottotitoli mantengono il tempo del film**: la battuta detta a 2:10 del film viene scritta a `00:02:10`, non a
  `00:00:10`. Il file può essere sovrapposto così com'è all'intero film.
- **I bordi sono approssimativi**: il motore lavora su finestre di trenta secondi, quindi ciò che restituisce viene tagliato
  all'intervallo in un secondo momento; una frase a cavallo di un bordo viene tagliata lì.
- **Nulla parte se l'intervallo non è utilizzabile**: un estremo che non è un tempo, o un `--end` che non viene dopo
  `--start`, è un errore d'uso che nomina il problema, non scrive nulla e non lancia né `ffmpeg` né il
  motore.

L'audio dell'intero film viene comunque estratto: è la trascrizione, molto più lunga, che l'intervallo risparmia.

## Elencare le lingue

`--list-languages` scrive le lingue che il motore offre **per il modello**, una per riga: il codice che `--language` accetta, una tabulazione, e il nome
della lingua nella lingua dell'interfaccia. Non richiede né input né output, e non trascrive nulla.

```bash
subedit-cli transcribe --list-languages --model ggml-small.bin
```

```text
en	English
zh	Chinese
de	German
…
```

L'elenco dipende dal motore e dal modello: un centinaio di lingue per un modello whisper.cpp multilingue, solo `en` per un modello solo per
l'inglese. Un motore che non sa elencare le sue lingue non ne elenca nessuna. Un modello non indicato o assente è un errore, con il rimando alla guida; solo la
forma testuale, nulla viene scritto con [`--format json`](invocation.md#un-output-che-uno-script-può-leggere).

## Output

**Output standard** — niente: il risultato è il file scritto.

| Livello | Che cosa si aggiunge sull'output di errore |
| :----- | :------------------------------------- |
| 1 | `<input>: N subtitles transcribed, language: xx -> <output>` |
| 2 | la fase e l'avanzamento, una riga per ogni percento: `extracting the audio…`, poi `transcribing…`, poi `transcribing: 25%, 0:12:30 of 0:50:00, about 38 min left, elapsed: 0:05:12` — il percento, la posizione raggiunta nell'audio, il tempo rimanente e il tempo trascorso; **la riga viene ripetuta ogni dieci secondi quando non è arrivato nulla di nuovo**, in modo che una lunga attesa non sia scambiata per un blocco |
| 3 | e ogni nuovo testo ottenuto dal motore, `last text: …` |

**Il percento è la posizione del motore nell'audio, non un conteggio delle sue finestre**: il motore scrive ciò che ha
trascritto man mano, e il comando legge a che punto è arrivato, in modo che la cifra si muova fin dal primo minuto. (L'avanzamento
proprio del motore arriva a passi del 5 % dell'audio, dopo una finestra di trenta secondi: da solo, può restare
allo 0 % per un quarto d'ora su un film lungo con un modello grande su processore.) **Il tempo rimanente è muto all'inizio**: viene
dato quando è stato fatto abbastanza lavoro per estrapolare, e segue la velocità dell'ultimo minuto, non la media
dell'intera esecuzione.

### In JSON

Con [`--format json`](invocation.md#un-output-che-uno-script-può-leggere) (l'opzione globale, scritta prima del
sottocomando), l'oggetto del file porta la lingua usata in `language`, `counts.subtitles` (i
sottotitoli scritti) e `counts.segments` (i pezzi del motore da cui sono stati ricavati), e `audio` quando
`--keep-audio` ha conservato l'audio.

## Avvisi

Un avviso viene detto al primo livello, prima del lavoro, perché cambia ciò che c'è da aspettarsi:

- **Con un modello solo per l'inglese**: `This model is for English only: whatever language the film
  speaks, the result will be English, or notes in brackets. Choose a multilingual model in the preferences.` Un modello il cui nome di file termina
  con `.en` lo è.

## Codici di uscita

Quelli dello strumento: `0` se il file è stato scritto, `2` se non è stato possibile — uno strumento o un modello mancante, una
lingua che il modello non può gestire, un'estrazione o una trascrizione non riuscita —, `1` per un errore d'uso, come un
motore che nessuno ha registrato o un intervallo inutilizzabile.

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
| `<input>` o `--output` mancante | `input is required` o `--output is required` |
| `--suppress-notes` con `--keep-notes` | `--suppress-notes and --keep-notes say opposite things; give one of them` |
| `--suppress-notes` per un motore a cui non lo si può dire | `the engine NAME cannot be told not to write sounds as notes` |
| una lingua che non è un codice | `"TEXT" is not a language code: write two letters, such as fr, en, de or es` |
| un estremo dell'intervallo che non è un tempo | `"TEXT" is not a time: write it as 2:00, 1:02:03 or 2:00.5, or in seconds` |
| `--end` non dopo `--start` | `the end of the range must come after its start` |
| qualsiasi altro errore | `the transcription failed`, con ciò che ha detto il motore |

Il rimando alla guida è `See the manual page "Installing the transcription tools"
(install-transcription).`
