# Installare gli strumenti di trascrizione

<!-- machine-translated -->
> Questa pagina è una traduzione automatica e non è ancora stata rivista da un lettore della lingua.

Trascrivere l'audio in sottotitoli — [`transcribe`](transcribe.md) da riga di comando,
[`Transcribe Audio…`](../subedit-gui/video.md#trascrivere-laudio) nella finestra — richiede tre
cose che **subedit non fornisce e non scarica**: un motore, un modello per quel motore e
`ffmpeg`. Li installi tu, e indichi a subedit dove si trova il modello.

**Senza di essi non si rompe nulla.** Il resto del programma funziona come prima; solo la trascrizione
si disattiva, con un messaggio che rimanda qui.

## Il motore: whisper.cpp

[whisper.cpp](https://github.com/ggml-org/whisper.cpp) è il motore che subedit esegue. È un programma,
`whisper-cli`, che gira su un normale processore — una scheda grafica lo rende solo più veloce. Compilalo
come dice la sua stessa documentazione (bastano un compilatore C++ e CMake):

```bash
git clone https://github.com/ggml-org/whisper.cpp
cd whisper.cpp
cmake -B build -DBUILD_SHARED_LIBS=OFF
cmake --build build -j --config Release
```

Il programma si trova poi in `build/bin/whisper-cli`. Compilato con `-DBUILD_SHARED_LIBS=OFF`, come sopra, è un singolo file che non ha bisogno di alcuna libreria accanto, quindi puoi copiarlo ovunque. Mettilo dove il tuo `PATH` lo trova, oppure **indica il suo percorso** a subedit con l'impostazione `transcription.executable` — nella finestra, le [preferenze](../subedit-gui/preferences.md#trascrizione) hanno un campo apposito. Una compilazione per scheda grafica aggiunge un'opzione alla prima riga di `cmake`: `-DGGML_CUDA=1` per una scheda NVIDIA, `-DGGML_VULKAN=1` per la maggior parte delle altre; subedit esegue quella che hai installato e non sceglie nulla al posto tuo.

Per tenere solo il programma, **installalo** ed elimina il clone:

```bash
cmake --install build --prefix ~/.local
```

`--prefix` dice dove. `~/.local` non richiede diritti di amministratore, e la sua directory `bin` deve essere nel tuo `PATH`; senza l'opzione il valore predefinito è `/usr/local`, che chiede `sudo`. `whisper-cli` finisce nella directory `bin` di quel prefisso, accanto a strumenti che subedit non usa; una volta lì, il clone e la sua directory `build` possono sparire.

## Un modello

Il motore legge un **modello**, un file da diverse centinaia di megabyte a qualche gigabyte che
scarichi una volta sola. I modelli ufficiali sono elencati su
[huggingface.co/ggerganov/whisper.cpp](https://huggingface.co/ggerganov/whisper.cpp), e il
repository del motore ha uno script che ne scarica uno:

```bash
sh ./models/download-ggml-model.sh small
```

Poi indica il suo percorso con `--model` da riga di comando, oppure con l'impostazione `transcription.model`,
o nelle [preferenze](../subedit-gui/preferences.md#trascrizione) della finestra, che lo ricordano.

### Un modello di attività vocale, per iniziare dove inizia il parlato

Facoltativamente, un **modello di attività vocale** (Silero) permette al motore di saltare i silenzi prima di decodificare. Senza, whisper inizia il suo primo sottotitolo a 0, qualunque cosa venga per prima nel film, musica compresa, e arrotonda i suoi tempi a finestre intere; con uno, il primo sottotitolo inizia dove inizia il parlato e gli altri lo seguono. Il repository del motore ha uno script che ne scarica uno:

```bash
sh ./models/download-vad-model.sh silero-v6.2.0
```

Indica il suo percorso con `--vad-model`, con l'impostazione `transcription.vad-model` o nelle [preferenze](../subedit-gui/preferences.md#trascrizione) della finestra.

### Quale modello per quale lingua

Whisper non cambia modello una volta trovata la lingua: **il modello che indichi decide che cosa
può fare.**

| Modello | Lingue | Rileva la lingua | Dimensione |
| :---- | :-------- | :------------------- | :--- |
| `tiny`, `base`, `small`, `medium`, `large-v3`, `large-v3-turbo` | un centinaio | sì | da circa 75 MB a circa 3 GB |
| `tiny.en`, `base.en`, `small.en`, `medium.en` | solo inglese | no | come la stessa dimensione sopra |

- **Per qualsiasi lingua diversa dall'inglese, scegli un modello multilingue** — uno il cui nome non ha `.en`. Un
  modello solo inglese rifiuta un'altra lingua: il comando lo dice prima di fare qualsiasi lavoro.
- **I modelli più piccoli sono nettamente peggiori fuori dall'inglese.** Per il francese, il tedesco, lo spagnolo
  e simili, `small` è un minimo, e `medium` o `large-v3-turbo` danno risultati molto migliori al prezzo
  di più memoria e più tempo.
- **Il rilevamento della lingua vale quanto il modello.** Un modello piccolo può sbagliare su un inizio fatto di
  musica o di silenzio; indica la lingua con `--language` quando la conosci.

## ffmpeg

[`ffmpeg`](https://ffmpeg.org/download.html) estrae la traccia audio di un video, e `ffprobe`, che viene con
lui, ne elenca le tracce. Installalo con il gestore di pacchetti della tua distribuzione. Senza di esso, un file
audio che il motore legge così com'è (`wav`, `mp3`, `flac`, `ogg`) può ancora essere trascritto, ma un video
no.

## Verificare che funzioni

```bash
subedit-cli transcribe --model /path/to/ggml-small.bin --output film.srt film.mkv
```

Se manca qualcosa, il messaggio dice che cosa, e rimanda a questa pagina.
