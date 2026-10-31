# `sync`

<!-- machine-translated -->
> Questa pagina è una traduzione automatica e non è ancora stata rivista da un lettore della lingua.

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

**Risincronizza un file di sottotitoli sull'audio di un film.** Il caso per cui esiste è il file `.srt` scaricato dalla rete,
sfasato all'inizio e alla fine, che un unico spostamento non rimette a posto: ogni sottotitolo viene cercato in ciò che
viene detto, e prende il posto in cui viene detto.

Ascolta l'audio con un motore di trascrizione — lo stesso di [`transcribe`](transcribe.md) — e **abbina le parole di
ogni sottotitolo alle parole che ha udito**. Nulla viene tradotto: i sottotitoli devono essere nella lingua parlata.

**Nulla di ciò che serve è fornito con il programma**: il motore, un modello e `ffmpeg` sono quelli di
[Installare gli strumenti di trascrizione](install-transcription.md), e la loro assenza non rompe nulla — il comando
dice che cosa manca, con il rimando a quella pagina, non scrive nulla ed esce con un codice diverso da zero.

- **Cambiano solo gli inizi e le fini.** Mai il testo, lo stile, il numero dei sottotitoli o il loro ordine.
- **Un sottotitolo che l'audio non fornisce conserva i suoi tempi** — un titolo di coda che nessuno pronuncia, una
  canzone, una battuta in un'altra lingua. **Nessuna posizione viene mai inventata, né indovinata dai vicini**: la riga
  resta dove il file la metteva, e il comando lo dice. *Non* è un fallimento, e il codice di uscita è `0`.
- **Un sottotitolo è trovato quando almeno la metà delle sue parole è udita** (`--min-match`, `alignment.min-match`), e
  almeno una. Maiuscole, accenti, punteggiatura e marcatori non contano; una parola sbagliata di una lettera in una parola
  lunga conta come udita.
- **La fine è dove termina l'ultima parola.** Il motore data le parole in modo approssimativo, qualche centesimo di
  secondo prima della fine propria del sottotitolo, quindi un sottotitolo può risultare un po' più corto di prima.
- **Un sottotitolo che andrebbe a sbattere contro il successivo viene accorciato fino all'inizio del successivo.** Uno
  che si sovrappone ancora al successivo, cosa che l'accorciamento non può curare, viene lasciato e segnalato.
- **Un file di cui nessun sottotitolo è stato trovato viene rifiutato e non scritto**: è quasi certamente l'audio
  sbagliato o la lingua sbagliata.
- **Si può ascoltare solo una parte del film**, con `--start` e `--end`, come per
  [`transcribe`](transcribe.md#trascrivere-una-parte): solo i sottotitoli che **iniziano** nell'intervallo vengono
  cercati e toccati, gli altri restano come sono e non vengono contati.
- **L'audio viene ascoltato una volta per ogni file indicato.** Per risincronizzare più file sullo stesso film,
  aspettati di attendere per ciascuno.

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

## Argomenti e opzioni

| Opzione | Obbligatoria | Valori | Predefinito |
| :----- | :------- | :----- | :------ |
| `<files>` | sì | i file di sottotitoli da risincronizzare; con `-r`, le directory | — |
| `--recursive`, `-r` | no | un'opzione senza valore | disattivata — vedi [Elaborare un albero di directory](lots.md) |
| `--video` | uno tra `--video` e `--audio` | il film da ascoltare | — |
| `--audio` | uno tra `--video` e `--audio` | un file audio da ascoltare, al posto di un film | — |
| `--audio-track` | no | un numero intero da 0: la posizione tra le tracce audio del film | `0`, la prima |
| `--language` | no | due lettere, un codice ISO 639-1: `en`, `fr` | l'impostazione `transcription.language`; vuota: rilevata |
| `--start`, `--end` | no | un tempo: `2:00`, `1:02:03`, `2:00.5`, o un numero di secondi; `--end` deve venire dopo `--start` | tutto l'audio |
| `--model` | no | il file del modello | l'impostazione `transcription.model` |
| `--vad-model` | no | un modello di attività vocale, che fa saltare i silenzi al motore; con esso le parole seguono più da vicino il parlato | l'impostazione `transcription.vad-model` |
| `--engine` | no | il nome del motore che ascolta: `whisper.cpp` | l'impostazione `transcription.engine` |
| `--keep-audio` | no | un file in cui conservare l'audio estratto; la sua directory viene creata se manca | l'audio non viene conservato da nessuna parte |
| `--min-match` | no | un numero intero da 1 a 100: la quota delle parole di un sottotitolo che devono essere udite, in percentuale | l'impostazione `alignment.min-match`, `50` |
| `--doubt-gap` | no | un numero intero di millisecondi, 0 o più: quanto lo spostamento di un sottotitolo può differire da quello dei suoi vicini prima di essere segnalato | l'impostazione `alignment.doubt-gap`, `2000` |
| `--report` | no | un file in cui scrivere i sottotitoli da controllare, uno per riga: il numero del sottotitolo, una tabulazione e il motivo; un solo file di sottotitoli alla volta | non viene scritto nulla |
| `--output` / `--output-dir` / `--in-place` | **una delle tre**, tranne con `--dry-run` | vedi [Invocazione](invocation.md#la-destinazione) | — |
| `--dry-run` | no | un'opzione senza valore: calcola e dice, non scrive nulla — vedi [Vedere prima di scrivere](invocation.md#vedere-prima-di-scrivere) | disattivata |

La destinazione — `--output`, `--output-dir`, `--in-place`, `--dry-run` — e `-r` sono quelle degli altri comandi che
riscrivono un file: vedi [la destinazione](invocation.md#la-destinazione). Il file conserva il suo formato, la sua
codifica e i suoi fine riga.

Le **impostazioni** si leggono dal file delle impostazioni — vedi [il file delle impostazioni](invocation.md#il-file-delle-impostazioni) — e
ciascuna ha qui la sua opzione. Il motore, il modello e il modello di attività vocale sono quelli di `transcribe`.

| Chiave | Significato | Predefinito |
| :-- | :------ | :------ |
| `alignment.engine` | il motore di allineamento; ce n'è uno, `transcript-matching`, che ascolta e abbina | `transcript-matching` |
| `alignment.min-match` | la quota delle parole di un sottotitolo che devono essere udite perché sia trovato, in percentuale | `50` |
| `alignment.doubt-gap` | quanto, in millisecondi, lo spostamento di un sottotitolo può discostarsi da quello dei suoi vicini prima di essere segnalato | `2000` |

## Che cosa viene segnalato

Il comando dice quali sottotitoli controllare, e perché. Un sottotitolo è segnalato per la **prima** di queste ragioni che vale:

| Perché | Significato |
| :-- | :------ |
| non trovato nell'audio, lasciato com'era | meno di `--min-match` delle sue parole sono state udite: conserva i suoi tempi |
| udito solo in parte | trovato, ma meno dell'80 % delle sue parole è stato udito |
| spostato in modo diverso dai suoi vicini | trovato, ma il suo spostamento si discosta di oltre `--doubt-gap` dallo spostamento mediano dei due sottotitoli per lato |

Un sottotitolo può anche essere **accorciato fino all'inizio del sottotitolo successivo**, o **sovrapporsi ancora al sottotitolo successivo**.

Due cifre raccontano la storia del file nel suo insieme. **Lo spostamento mediano** è il valore centrale di quanto si
sono spostati i sottotitoli: un file semplicemente in ritardo si legge qui, e solo qui. **La deriva** è lo spostamento
mediano dell'ultimo decimo dei sottotitoli meno quello del primo decimo: è vicina a zero per uno sfasamento costante, e
grande per un file che era in ritardo sempre di più lungo il film.

## Output

**Output standard** — niente: il risultato è il file scritto.

| Livello | Che cosa viene detto sull'output di errore |
| :---- | :------------------------------- |
| 1 | `<file>: N of M subtitles placed on the audio, K to check; median shift X s, drift Y s -> <destination>` |
| 2 | la fase e l'avanzamento, come per [`transcribe`](transcribe.md#output) |

### In JSON

Con [`--format json`](invocation.md#un-output-che-uno-script-può-leggere), l'oggetto del file contiene `counts`: `subtitles`
(cercati), `placed` (collocati), `moved` (i cui tempi sono cambiati), `partial`, `out_of_line`, `not_found`,
`median_shift_ms` e `drift_ms`; e `flagged`, un elenco di `{"subtitle": N, "kind": "…"}` dove il numero è la
posizione del sottotitolo nel file e `kind` è uno tra `not-found`, `partial`, `out-of-line`, `cut-back` e `overlaps`.
Gli id sono garantiti; le frasi no.

## Esempio

```bash
subedit-cli sync --video film.mkv --model small.bin --vad-model silero.bin \
    --report to-check.txt --output film.synced.srt film.srt
```

## Codici di uscita

Quelli dello strumento: `0` se il file è stato scritto, con sottotitoli segnalati o no, `2` se non è stato possibile — uno
strumento o un modello mancante, un'estrazione o un ascolto non riuscito, nessun sottotitolo trovato —, `1` per un errore
d'uso, come l'assenza di `--video` e `--audio` (o la presenza di entrambi), un'opzione fuori dal suo intervallo, o un
intervallo inutilizzabile.

## Errori

| Causa | Messaggio |
| :---- | :------ |
| né `--video` né `--audio`, o entrambi | `sync needs the audio to listen to: give --video or --audio, and not both` |
| nessun programma del motore trovato, o quello configurato non c'è | `the transcription engine was not found`, poi il rimando alla guida |
| nessun modello indicato, o non c'è | `no model was given, or the model file is not there`, poi il rimando alla guida |
| `ffmpeg` non trovato per un film | `ffmpeg was not found, so the audio of a video cannot be extracted`, poi il rimando |
| una traccia che il film non ha | `the video has no audio track N`, poi le tracce che ha |
| una lingua che il modello non può trascrivere | `the model cannot transcribe the language xx` |
| un motore che nessuno ha registrato | `no transcription engine is called NAME`, o `there is no alignment engine called NAME` |
| nessuno dei sottotitoli trovato | `none of the N subtitles was found in the audio: is it the right audio, and the right language?` |
| `--min-match` fuori da 1–100, `--doubt-gap` negativo | l'errore d'uso dell'opzione |
| `--report` con più di un file di sottotitoli | `--report names one file: give one subtitle file when you use it` |
