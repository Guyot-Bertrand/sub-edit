# Il video associato

<!-- machine-translated -->
> Questa pagina è una traduzione automatica e non è ancora stata rivista da un lettore della lingua.

Il menu **Video** porta anzitutto tre comandi, e poi i gesti che guidano il film
— vedi [Il lettore video](player.md#spostarsi-nel-film).

| Comando | Scorciatoia | Che cosa fa |
| :------ | :------- | :----------- |
| Seleziona video… | nessuna | sceglie il file video a cui il documento è associato |
| Trascrivi audio… | nessuna | trascrive l'audio del film in sottotitoli che sostituiscono quelli del progetto — vedi [Trascrivere l'audio](#trascrivere-laudio) |
| Riproduci / Pausa | `Ctrl+P` | riproduce o ferma il video — vedi [Il lettore video](player.md#riprodurre-e-fermare) |

Il video associato è **una proprietà del documento aperto**, non un'impostazione
dell'applicazione: aprire un altro file di sottotitoli ricomincia da zero. Non
viene scritto in alcun file e scompare con la finestra.

## La traccia audio

Un film può avere più tracce sonore — la versione originale, un doppiaggio, un commento. Il
sottomenu **`Video ▸ Audio ▸ Lingua`** le elenca, **una voce per traccia**, e **segna quella in riproduzione**:

| Che cosa fornisce il file | Che cosa dice la voce |
| :------------------ | :------------------ |
| una lingua e un titolo | `1: fra — Original` |
| una lingua soltanto | `2: eng` |
| un titolo soltanto | `3: Commentary` |
| nessuno dei due | `1` |

Il numero è **la posizione della traccia nel film**, una per una, nell'ordine del file: due tracce
possono dire `eng`, e un file che non dice nulla delle proprie etichette ha comunque una prima e una seconda traccia.

**Scegliere una voce riproduce quella traccia.** Il segno è la risposta del lettore, non il clic: indica
la traccia in riproduzione.

- **Il sottomenu è disattivato** se non c'è alcun video o alcuna traccia audio. Con una sola traccia, resta attivo e
  la mostra, spuntata: indica la lingua del film. I due gesti del volume, che gli stanno accanto, restano
  attivi in ogni caso: vedi [Il volume](player.md#il-volume).
- **La traccia non viene ricordata** da un video al successivo, né da una sessione alla successiva: il numero di una traccia è
  quello del suo file, e riportarlo sul film seguente sceglierebbe qualunque traccia porti quel numero.
  Tornare a una scheda riapre il suo film sulla traccia che il lettore sceglie da sé.
- Nessuna scorciatoia: la lingua si cambia di rado, e il menu basta.

## Scegliere un video

La finestra di dialogo si apre sulla directory del file di sottotitoli e filtra sulle
estensioni video riconosciute:

    .avi .flv .m2ts .mkv .mov .mp4 .ogg .ogv .vob .webm

Offre anche di mostrare tutti i file. Annullare la finestra di dialogo non cambia
nulla.

Il file scelto viene **aperto subito**: l'immagine compare nella finestra,
in pausa sul suo primo istante. Vedi [Il lettore video](player.md).

**Trascinare un file video sulla finestra fa la stessa cosa** — vedi
[Trascinare file sulla finestra](files.md#trascinare-file-sulla-finestra).

**Il video è quello del progetto della scheda corrente.** Ogni scheda conserva
il proprio; cambiare scheda cambia il film. **Tornare a una scheda riprende il suo
film dal punto in cui lo si era lasciato**, in pausa, come ogni apertura. Un altro film
scelto per quel progetto nel frattempo, invece, riparte dall'inizio.

## La proposta automatica

Quando si apre un file di sottotitoli, **se non è stato scelto alcun video**,
la finestra ne cerca uno nella stessa directory: un file video il cui nome
senza estensione è un prefisso del nostro, fino a un punto.

| File di sottotitoli | Video trovato | Perché |
| :------------ | :---------- | :-- |
| `film.fr.srt` | `film.mkv` | `film` è un prefisso di `film.fr` |
| `film.en.forced.srt` | `film.mkv` | allo stesso modo, qualunque sia il numero di segmenti |
| `film.fr.srt` | `film.fr.mkv` anziché `film.mkv` | vince il nome più lungo |
| `film.fr.srt` | nessuno, se ci sono `film.mkv` **e** `film.mp4` | due candidati non si distinguono |
| `film.fr.srt` | nessuno, se c'è solo `fil.mkv` | `fil` non termina su un punto |

**Una proposta non è una scelta.** Si lascia sostituire da un'altra
proposta — aprendo un altro file, salvando con un altro nome. Una scelta
esplicita, invece, non viene mai sovrascritta: una volta usato `Select Video…`, la
proposta automatica non torna più dietro di essa.

È ciò che rende accettabile l'ipotesi: si può correggere, e la correzione resta.

## Che cosa mostra la barra di stato

A destra della barra di stato, una riga dice a che cosa è associato il documento. È
l'ultima di tre: la [codifica](files.md#la-codifica-nella-barra-di-stato)
del file letto viene per prima, poi la [griglia dei fotogrammi](frame-grid.md) dedotta dalle
posizioni.

| Situazione | Che cosa viene scritto |
| :-------- | :-------------- |
| un video è associato | `Video: film.mkv` |
| … e `ffprobe` ne fornisce la frequenza dei fotogrammi | `Video: film.mkv, 24000/1001 fps` |
| nessuno | `No video` |

**La frequenza dei fotogrammi accompagna il film anziché stargli accanto.** Sono un unico fatto — a che cosa
è associato questo documento — e una terza indicazione la metterebbe sullo stesso piano della
[griglia dedotta dalle posizioni](frame-grid.md), che è un'altra cosa.

**Il nome del file, mai il suo percorso.** Un percorso di duecento caratteri
spingerebbe fuori tutto il resto dalla barra; il percorso completo è quello che
la finestra di dialogo mostrava.

## Trascrivere l'audio

**`Transcribe Audio…`** scrive il parlato del film come sottotitoli. È disattivato finché non è aperto un film: è
l'audio del **film del progetto**, mai un file che si sceglierebbe — per trascrivere un
file audio, usa [`transcribe`](../subedit-cli/transcribe.md) da riga di comando.

La finestra di dialogo chiede tre cose e dice se può partire:

- **la traccia audio**, elencata come la elenca il menu [`Audio`](#la-traccia-audio), con la lingua
  e il titolo dichiarati dal file;
- **la lingua**, due lettere come `en` o `fr`. Parte dalla lingua della traccia scelta
  quando il file ne dichiara una, segue la traccia finché non ne hai digitata una, e
  **una riga sotto il campo dice da dove viene**. Quando il file non dichiara alcuna lingua per la traccia,
  la riga lo dice: il modello la rileverà allora dal suono, cosa che solo un modello multilingue
  sa fare, e conviene digitarla se la conosci. Con un modello solo per l'inglese, che non rileva nulla,
  la riga dice che la lingua va digitata;
- **la parte del film**, da un tempo a un altro — vedi [Provare prima una parte](#provare-prima-una-parte). Con entrambi i campi
  vuoti si trascrive l'intero film.

**Dice anche che cosa manca.** Tre righe dicono se il motore, `ffmpeg` e il modello ci sono, e
dove; quando uno non c'è, `OK` resta disattivato, e la finestra lo dice con il rimando a
[Installare gli strumenti di trascrizione](../subedit-cli/install-transcription.md). Un pulsante `Preferences…` apre
le [preferenze](preferences.md#trascrizione), dove si impostano il programma e il modello — sono fatti
sulla macchina, non scelte da rifare a ogni trascrizione — e la finestra ricontrolla quando si chiudono.

La lingua viene conservata per la volta successiva. Il motore stesso e i limiti dei sottotitoli sono
[impostazioni](preferences.md) (`transcription.*`).

**Quando il progetto contiene già dei sottotitoli, la finestra chiede prima di sostituirli**, e dice
quanti ne vanno persi. Chiede prima del lavoro, in modo che rifiutare non costi nulla. Quando il progetto è vuoto non c'è
nulla da chiedere.

Il lavoro si svolge **in background**, con una finestra e un pulsante `Cancel`; la finestra principale resta utilizzabile. **La finestra
dice a che punto è il lavoro**, in modo che tu sappia che è vivo e quanto tempo richiederà:

- **la fase**: prima “Estrazione dell'audio…” — la finestra è allora occupata, senza cifre, perché l'estrazione non dice nulla
  del suo avanzamento —, poi “Trascrizione dell'audio…”, con una barra;
- **la posizione**: “0:12:30 di 0:50:00”, fino a dove nel film è arrivato il motore, non appena ha prodotto il suo primo testo;
- **il tempo rimanente**, “circa 38 min rimanenti”: assente all'inizio, mostrato quando è stato fatto abbastanza lavoro per
  estrapolare, e calcolato sulla velocità dell'ultimo minuto; ha una precisione di qualche minuto, non è una promessa;
- **l'ultimo testo** ottenuto, in modo che tu possa vedere che il lavoro ha senso.

Su un film lungo con un modello grande su processore, i primi minuti possono essere lunghi: il motore sta caricando il modello e
lavorando sulle sue prime finestre di trenta secondi. La posizione e l'ultimo testo si muovono prima del percento proprio del
motore, che arriva solo a passi del 5 % del film. Quando termina, i sottotitoli del progetto vengono sostituiti
dalla trascrizione **in una sola operazione**: un solo `Undo` restituisce ciò che c'era, testi di
traduzione compresi. Una trascrizione che fallisce, o che annulli, non cambia nulla — il motivo viene
detto in una finestra, tranne che per un annullamento, che dice soltanto nella barra di stato che è stato annullato.

### Provare prima una parte

Un intervallo trasforma la trascrizione in una **prova**, per giudicare un modello o una lingua su due minuti prima di impegnarsi su
due ore. I campi “Parte del film” accettano un inizio e una fine, ciascuno scritto `2:00`, `1:02:03`, `2:00.5` o un numero
di secondi, `90`; ne è ammesso uno solo (da lì alla fine, o dall'inizio fino a lì). `OK` resta disattivato finché un campo non
è un tempo o la fine non viene dopo l'inizio, e una riga sotto i campi dice quale.

- **I sottotitoli mantengono il tempo del film**: ciò che è detto a 2:10 si trova a `0:02:10` nel risultato.
- **Nulla viene sostituito e nulla viene chiesto.** Il risultato si apre come **un progetto a sé**, in una nuova scheda, senza
  file; il progetto da cui proviene, e i suoi sottotitoli, restano come prima. Salvalo, o chiudilo.

**Ciò che serve non è fornito**: un motore (whisper.cpp), un modello e `ffmpeg` per estrarre l'audio.
Vedi [Installare gli strumenti di trascrizione](../subedit-cli/install-transcription.md).

## Quando non c'è alcun film

La finestra mostra un pulsante **`Select Video…`** dove andrebbe l'immagine, anziché
niente del tutto. È lo stesso comando di quello del menu `Video`, a
portata di mouse: un'assenza su cui non si può agire si distingue a fatica da un
guasto.

Il pulsante scompare non appena un film è aperto, e l'immagine occupa **tutto** il suo
spazio: l'altezza che la tabella lascia sopra di essa — quella che
[`window.table-share`](preferences.md) ha ricordato da una sessione alla successiva —, e
mai meno dell'altezza minima dell'immagine, che la tabella cede se serve.
Quando il film se ne va, il pulsante riprende lo stesso posto.

## `ffmpeg` non è obbligatorio

`subedit-gui` interroga `ffprobe` — fornito con `ffmpeg` — per una cosa sola:
**la frequenza dei fotogrammi che il contenitore dichiara**. Viene letta una volta
per film, al momento dell'associazione.

| Con `ffprobe` | Senza |
| :------------- | :------ |
| la barra di stato scrive la frequenza dei fotogrammi accanto al nome del film | scrive solo il nome |
| la finestra di conversione propone la frequenza dei fotogrammi del film nel campo in basso | non propone nulla, e si apre come prima |
| la finestra di allineamento si apre su di essa | si apre sulla frequenza dei fotogrammi del progetto |

**Nient'altro ne dipende, e nessuna operazione rifiuta.** Il lettore, dal canto suo,
non ha affatto bisogno di `ffprobe`: si appoggia a `libmpv`, che è una
dipendenza di compilazione. La durata del film, quella usata per segnalare ciò che
supera la fine, proviene dal lettore e non da `ffprobe`.

**La trascrizione è l'unica cosa che ha bisogno di `ffmpeg`**: estrae l'audio del film. Vedi
[Installare gli strumenti di trascrizione](../subedit-cli/install-transcription.md).

Non è quindi una modalità degradata ma **il funzionamento normale** di una macchina
che non ha mai installato `ffmpeg`: il dato viene dall'utente, e `ffprobe`
lo propone quando c'è.

Una frequenza dei fotogrammi letta è un **razionale esatto** — `24000/1001`, non `23.976`.
È il motivo per cui `ffprobe` viene mantenuto per questa risposta: il
lettore conosce la stessa cosa come numero in virgola mobile, e una frequenza dei fotogrammi approssimata
risincronizzerebbe l'intero file con un leggero errore.
