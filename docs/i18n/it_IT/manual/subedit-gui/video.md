# Il video associato

<!-- machine-translated -->
> Questa pagina è una traduzione automatica e non è ancora stata rivista da un lettore della lingua.

Il menu **Video** porta anzitutto due comandi, e poi i gesti che guidano il film
— vedi [Il lettore video](player.md#spostarsi-nel-film).

| Comando | Scorciatoia | Che cosa fa |
| :------ | :------- | :----------- |
| Seleziona video… | nessuna | sceglie il file video a cui il documento è associato |
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

Non è quindi una modalità degradata ma **il funzionamento normale** di una macchina
che non ha mai installato `ffmpeg`: il dato viene dall'utente, e `ffprobe`
lo propone quando c'è.

Una frequenza dei fotogrammi letta è un **razionale esatto** — `24000/1001`, non `23.976`.
È il motivo per cui `ffprobe` viene mantenuto per questa risposta: il
lettore conosce la stessa cosa come numero in virgola mobile, e una frequenza dei fotogrammi approssimata
risincronizzerebbe l'intero file con un leggero errore.
