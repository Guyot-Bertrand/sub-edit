# Sincronizzare a partire dal video

<!-- machine-translated -->
> Questa pagina è una traduzione automatica e non è ancora stata rivista da un lettore della lingua.

*Si presume che il film sia aperto e che il lettore sia noto: vedi [Il lettore video](player.md).*

Si guarda il film, ci si ferma sul fotogramma in cui una riga deve cominciare, e **quella posizione diventa
l'inizio del sottotitolo**. Cinque gesti, nel menu **Video**, con i nomi che Gaupol dà loro. Si
appoggiano su ciò che esiste già: impostare una posizione è **lo stesso comando che digitare il valore nella
cella**, e inserire è **lo stesso che `Modifica ▸ Inserisci sottotitoli…`**.

| Comando | Scorciatoia | Che cosa fa |
| :------ | :------- | :----------- |
| Video ▸ Imposta l'inizio dalla posizione del video | `Alt+U` | imposta l'**inizio** del sottotitolo selezionato sulla posizione del film |
| Video ▸ Imposta la fine dalla posizione del video | `Alt+K` | imposta la sua **fine** sulla posizione del film |
| Video ▸ Inserisci sottotitolo alla posizione del video | `Alt+J` | inserisce un sottotitolo che **comincia** alla posizione, e lo seleziona |
| Video ▸ Seleziona il precedente dalla posizione del video | `Alt+Up` | seleziona l'ultimo sottotitolo che **comincia prima** della posizione |
| Video ▸ Seleziona il successivo dalla posizione del video | `Alt+Down` | seleziona il primo che **comincia dopo** la posizione |

**Tutti sono disattivati se non c'è alcun video aperto**, come i gesti del lettore. `Set Start…` e `Set End…`
sono disattivati anche finché nulla è selezionato: agiscono sulla **prima riga** della
selezione.

**Le scorciatoie sono quelle di Gaupol, con `Alt`**: là sono `U`, `K` e `J` — semplici lettere, che
la finestra intercetterebbe prima della cella in modifica, cosicché digitare una `k` in un testo
imposterebbe una fine. `Select Previous…` e `Select Next…` sono `Ctrl+Y` e `Ctrl+U` in Gaupol; `Ctrl+Y`
è un *ripeti* su alcuni sistemi, e le frecce con `Alt` dicono *prima* e *dopo* meglio di
due lettere. Una cella in modifica viene **confermata** prima di `Set Start…`, `Set End…` o
`Insert Subtitle…`, come prima di qualunque modifica.

## Impostare un estremo

- **Una voce della cronologia per gesto**: `Edit ▸ Undo` restituisce la posizione precedente, e nient'altro.
- **Le stesse regole della cella.** Un inizio impostato dopo la fine del sottotitolo, o una fine prima del suo
  inizio, **non viene rifiutato**: viene impostato, e la tabella segna il sottotitolo come segna quello
  il cui valore è stato digitato a mano — vedi [Le anomalie](table.md#le-anomalie). Nulla viene corretto in silenzio.
- **Una posizione che è già quella dell'estremo non fa nulla**, e non aggiunge nulla alla cronologia.
- **La posizione è sempre l'inizio di un fotogramma**: il lettore non indica mai una posizione
  all'interno di un fotogramma. Impostare un estremo, poi tornarvi — con `Seek Selection Start`, per esempio —
  **mostra lo stesso fotogramma**.

## Inserire alla posizione

Il nuovo sottotitolo **comincia alla posizione**, e dura **tre secondi**, oppure fino all'inizio del successivo
quando questo viene prima. Il suo testo è vuoto. Prende il suo posto nell'ordine del file: dopo
tutti quelli che cominciano a quella posizione o prima. L'inserimento è **una voce della cronologia**,
e il sottotitolo inserito viene selezionato: si può digitarlo subito.

## Selezionare a partire dalla posizione

Entrambi seguono l'ordine del file, non la selezione corrente. **Senza vicino da quel lato**, la
selezione va all'ultimo sottotitolo del file per `Select Next…`, al primo per
`Select Previous…` — il comportamento di Gaupol. Questi due gesti non spostano il film.

## Fotogramma per fotogramma

Avanzare e tornare indietro **di un fotogramma** — o di N, vedi sotto —, per impostare un estremo **sul fotogramma esatto**.
Due gesti, e due pulsanti nella barra di riproduzione, ai due lati del pulsante di riproduzione:

| Comando | Scorciatoia | Che cosa fa |
| :------ | :------- | :----------- |
| Video ▸ Fotogramma indietro | `Alt+Left` | fa tornare indietro il film del passo in fotogrammi |
| Video ▸ Fotogramma avanti | `Alt+Right` | lo fa avanzare del passo in fotogrammi |

- **Il passo è un numero di fotogrammi, almeno uno**: è l'impostazione `Frame step` delle
  [preferenze](preferences.md#il-lettore-video). Un passo di N fotogrammi mostra il fotogramma **N più avanti**, a 25 fotogrammi al
  secondo come a 23,976; **non è mai espresso in millisecondi**.
- **Il film si ferma ai suoi limiti**: tornare indietro dal primo fotogramma lo lascia lì, avanzare dall'ultimo
  anche. La riproduzione viene fermata dopo un passo.
- **I pulsanti della barra** — due icone, ai due lati di riproduci/pausa — **attivano le stesse azioni del menu**: stesso passo, disattivati
  senza video come le voci del menu, e lo stesso suggerimento a comparsa, che indica la scorciatoia. **Un dito che resta su un
  pulsante, o sul tasto, concatena i passi.**
- **Un passo alla volta.** Un tasto tenuto premuto si ripete più in fretta di quanto un passo venga eseguito: la prima ripetizione parte
  subito, quelle che arrivano mentre il passo è in corso **non si accumulano** — condividono un solo posto,
  l'ultimo, che parte dopo. Rilasciare il tasto ferma quindi l'immagine dove ci si trova, e non lascia che il film
  prosegua per inerzia.

## Spostare un estremo di un fotogramma

Per regolare un estremo con la tastiera: **l'inizio o la fine del sottotitolo selezionato**, di un passo, prima o dopo.

| Comando | Scorciatoia | Che cosa fa |
| :------ | :------- | :----------- |
| Video ▸ Anticipa l'inizio | `Alt+Q` | il sottotitolo **comincia un passo prima** |
| Video ▸ Posticipa l'inizio | `Alt+W` | **comincia un passo dopo** |
| Video ▸ Anticipa la fine | `Alt+Shift+Q` | **finisce un passo prima** |
| Video ▸ Posticipa la fine | `Alt+Shift+W` | **finisce un passo dopo** |

`Q` e `W` sono quelle di Gaupol per *prima* e *dopo*, con `Alt` per il motivo già detto; la fine
usa `Alt+Shift`, perché la `E` di Gaupol è qui la lettera di `Edit`.

- **Una voce della cronologia per spostamento**, annullabile. Il sottotitolo vicino e la sovrapposizione vengono segnalati
  **come nella digitazione**: l'estremo viene impostato, e la tabella segna ciò che non regge più — vedi
  [Le anomalie](table.md#le-anomalie).
- **Lo stesso passo del lettore**, quindi un fotogramma per impostazione predefinita. **Quanto vale un fotogramma** viene letto, in quest'ordine,
  dalla **frequenza dei fotogrammi dichiarata dal video**, da quella del **documento quando conta in fotogrammi** (MicroDVD), poi dalla
  **griglia che le posizioni disegnano**. **In mancanza di tutte e tre, il gesto rifiuta e lo dice**: scegliere una
  frequenza dei fotogrammi a caso sposterebbe qualunque estremo di qualcosa che non è un fotogramma.
- **Non richiede un video**, solo una selezione: si usa anche su un file contato in fotogrammi, senza
  film. Quando un film è aperto, **la riproduzione si porta sulla nuova posizione**, per mostrare il fotogramma che l'estremo
  ha appena assunto.
- Un estremo non va prima dell'origine: tornare indietro di un passo da meno di un fotogramma lo porta a zero.
