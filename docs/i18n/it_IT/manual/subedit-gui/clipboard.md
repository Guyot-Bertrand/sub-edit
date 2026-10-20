# Tagliare, copiare e incollare i testi

<!-- machine-translated -->
> Questa pagina è una traduzione automatica e non è ancora stata rivista da un lettore della lingua.

Tre voci del menu `Edit`, sotto `Undo` e `Redo`, e sopra
`Find and Replace…`.

| Voce | Scorciatoia | Che cosa fa |
| :---- | :------- | :----------- |
| Taglia testi | `Ctrl+X` | copia i testi della selezione, poi li svuota |
| Copia testi | `Ctrl+C` | copia i testi della selezione |
| Incolla testi | `Ctrl+V` | scrive i testi copiati a partire dalla prima riga selezionata |

**Sono i testi a viaggiare, e nient'altro**: nessun inizio, nessuna fine, nessuna riga
intera. Incollare non sposta quindi alcuna posizione — la sincronizzazione della riga che
riceve un testo resta la sua. È ciò che fa Gaupol.

## Su che cosa agiscono

**Con una traduzione, il testo della colonna corrente**: la traduzione se la
cella corrente si trova nella colonna `Translation`, il testo principale altrove —
vedi [La tabella](table.md#il-testo-su-cui-agiscono-le-operazioni). `Cut Texts` svuota quindi
la traduzione e lascia il testo principale, e `Paste Texts` scrive nella
colonna in cui ci si trova. **Gli appunti non portano alcun documento**: si può
copiare una cella del testo principale e incollarla nella traduzione, e
viceversa.

**La selezione, e mai l'intero file.** Le tre voci sono disattivate finché
nulla è selezionato. Altrove nella finestra, «nulla selezionato» significa
«l'intero file»; qui, un `Ctrl+X` sfortunato svuoterebbe tutti i testi
del documento, e un'incollatura non avrebbe alcuna riga da cui cominciare.

Mentre una cella è aperta, `Ctrl+X`, `Ctrl+C` e `Ctrl+V` appartengono
al campo di input: tagliano, copiano e incollano caratteri, così come `Del`
vi cancella un carattere e non un sottotitolo. Avviate dal menu, le voci confermano prima
l'input: `Copy Texts` copia allora il testo come è stato appena
digitato. Vedi [Modificare una cella](editing.md).

## Copiare

`Copy Texts` copia il testo di ogni riga selezionata, dalla prima all'
ultima. **Una selezione discontinua conserva la sua forma**: una riga esclusa
dalla selezione diventa un buco, e l'incollatura non lo sovrascrive.

**Anche una riga selezionata il cui testo è vuoto diventa un buco**:
incollata, lascia la riga di destinazione così com'è. Copiare un solo testo vuoto non
incolla quindi nulla.

Gli appunti del sistema ricevono questi testi **come testo semplice, separati da una
riga vuota** — un buco vi è scritto come un testo vuoto, ed è per questo che un testo
vuoto non può essere altro che un buco: l'incollatura dà lo stesso risultato
sia che la copia provenga da questa finestra sia che provenga da un altro programma. Ciò che si copia qui può
quindi essere incollato in qualunque programma, e ciò che si copia altrove può essere incollato qui.

Copiare non modifica il documento, e non entra nella cronologia.

## Tagliare

`Cut Texts` copia, poi **svuota i testi** delle righe selezionate. Le righe
restano, con le loro posizioni: tagliare non è rimuovere — per rimuovere
righe c'è [`Remove Subtitles`](lines.md#rimozione).

Tagliare testi che sono già tutti vuoti non fa nulla, e non entra nella
cronologia.

## Incollare

`Paste Texts` scrive i testi degli appunti **a partire dalla prima riga
selezionata**, un testo per riga, verso il basso.

| Che cosa contengono gli appunti | Che cosa succede |
| :-------------------------- | :----------- |
| un testo | sostituisce quello della riga |
| un buco, oppure un testo vuoto copiato | la riga conserva il suo testo |
| più testi di quante righe restano | vengono aggiunte righe in fondo per riceverli |

Le righe aggiunte nascono come le righe inserite dopo l'ultima: tre
secondi ciascuna, una dopo l'altra. **La finestra lo dice**, in una casella informativa:

```text
inserted 2 subtitles to fit the clipboard
```

Dopo l'incollatura, **le righe scritte sono selezionate**.

Un'incollatura che non cambia nulla — gli stessi testi sulle stesse righe — non entra nella
cronologia.

### I tag di un altro formato vengono tradotti

Una copia fatta in questa finestra **ricorda il formato del suo documento**, e
questa memoria sopravvive all'apertura di un altro file. Incollare in un documento di
un altro formato traduce quindi i tag, come fa `Save As…`:
`{\i1}Bonjour{\i0}` copiato da un Advanced SSA viene incollato come `<i>Bonjour</i>` in un
SubRip.

**Ciò che il formato di destinazione non può scrivere va perso, e viene detto**, con
le parole di `Save As…`:

```text
pasting SubRip texts into LRC: 1 tag dropped
```

A differenza di `Save As…`, nulla viene chiesto prima: l'incollatura si annulla con
`Ctrl+Z`, che restituisce i testi com'erano.

**Un testo copiato fuori da `subedit` non ha formato**, e viene incollato così com'è:
non c'è nulla da tradurre partendo dal nulla. È anche il caso di una copia fatta in
questa finestra che gli appunti del sistema hanno nel frattempo sostituito — conta il
contenuto degli appunti del sistema.

Quando entrambi i messaggi sono dovuti, stanno in un'unica casella, separati da
un punto e virgola.

## Che cosa ne dice l'azione di annullamento

Ogni gesto è **una** voce della cronologia, qualunque sia il numero di
righe che scrive o aggiunge.

| Operazione | Che cosa legge `Undo` |
| :-------- | :---------------- |
| taglio | `Undo: cutting texts` |
| incollatura | `Undo: pasting texts` |

Vedi [Annulla e ripeti](undo.md).
