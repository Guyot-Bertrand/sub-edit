# subedit-cli — manuale

<!-- machine-translated -->
> Questa pagina è una traduzione automatica e non è ancora stata rivista da un lettore della lingua.

Lo strumento da riga di comando di subedit: lavorare sui file di sottotitoli senza interfaccia
grafica, in blocco o da uno script. Per l'altro programma e per sapere
da dove cominciare, vedi
[il manuale](../index.md).

> **Stato attuale.** I diciassette sottocomandi di `subedit-cli` esistono: ispezionare,
> convertire, spostare, trasformare, risincronizzare la frequenza dei fotogrammi, allineare a una griglia di fotogrammi, rimuovere
> le indicazioni per non udenti, regolare le durate, sostituire un testo, cambiare le maiuscole e minuscole,
> il corsivo e i trattini di dialogo, rimettere in ordine i sottotitoli, correggere i
> testi con i modelli di Gaupol, mettere i file uno dopo l'altro, tagliare un
> file in due, e scrivere una traduzione alle posizioni del suo file principale. Questo manuale
> descrive ciò che esiste, mai ciò che è previsto; ciò che viene dopo è nella
> [tabella di marcia](../../../../feuille-de-route.md).

## Sezioni

| Sezione | Contenuto |
| :------ | :------ |
| [Installazione](installation.md) | compilare e installare lo strumento |
| [Invocazione](invocation.md) | opzioni globali, output, codici di uscita |
| [Il file delle impostazioni](invocation.md#il-file-delle-impostazioni) | ciò che `adjust` e `correct` leggono dal file delle impostazioni della finestra, `--config`, `--no-config` |
| [Elaborare un albero di directory](lots.md) | più file, un'intera directory, `--recursive` |
| [`inspect`](inspect.md) | riportare di che cosa è fatto un file |
| [`convert`](convert.md) | scrivere un file in un altro formato o in un'altra forma |
| [`shift`](shift.md) | spostare ogni posizione della stessa quantità |
| [`transform`](transform.md) | correggere ogni posizione a partire da due punti noti per essere esatti |
| [`framerate`](framerate.md) | risincronizzare un file da una frequenza dei fotogrammi a un'altra |
| [`snap`](snap.md) | riportare i tempi sui fotogrammi di una frequenza |
| [`hearing-impaired`](hearing-impaired.md) | rimuovere le indicazioni per non udenti |
| [`adjust`](adjust.md) | regolare le durate secondo una velocità di lettura e dei limiti |
| [`replace`](replace.md) | sostituire un testo senza rompere un tag |
| [`case`](case.md) | mettere i testi in iniziali maiuscole, in prima lettera maiuscola, in maiuscolo o in minuscolo |
| [`italics`](italics.md) | mettere i testi in corsivo, o toglierne il corsivo |
| [`dialogue-dashes`](dialogue-dashes.md) | aggiungere i trattini di dialogo, o toglierli |
| [`sort`](sort.md) | rimettere i sottotitoli nell'ordine del loro inizio |
| [`join-repeats`](join-repeats.md) | ricavare un solo sottotitolo dai pezzi di uno stesso testo che si susseguono |
| [`disjoint`](disjoint.md) | tagliare i sottotitoli sovrapposti in modo che nessun intervallo ne sovrapponga più un altro |
| [`repair`](repair.md) | concatenare `join-repeats`, `sort` e `disjoint` su un file mal segmentato, fuori ordine e con sovrapposizioni |
| [`correct`](correct.md) | correggere i testi con i modelli di Gaupol: indicazioni, unione e divisione di parole, errori comuni, uso delle maiuscole, divisione delle righe |
| [`append`](append.md) | mettere i file dopo il primo, in un unico file |
| [`split-file`](split-file.md) | tagliare un file in due, con la coda riportata all'origine |
| [`pair`](pair.md) | scrivere una traduzione alle posizioni del suo file principale, e dire come si sono allineate le sue righe |
| [`transcribe`](transcribe.md) | scrivere il parlato di un video o di un file audio come sottotitoli |
| [Installare gli strumenti di trascrizione](install-transcription.md) | il motore, il modello e `ffmpeg`, che il programma non fornisce |
| [`config`](config.md) | elencare, mostrare e modificare le impostazioni delle operazioni condivise con la finestra |
