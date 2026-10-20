# Annulla e ripeti

<!-- machine-translated -->
> Questa pagina è una traduzione automatica e non è ancora stata rivista da un lettore della lingua.

Ogni operazione entra in una cronologia, e può essere annullata. Una cella modificata conta
come **una** voce: la digitazione non viene registrata tasto per tasto, conta solo
la conferma.

| Azione | Scorciatoia | Dove |
| :----- | :------- | :---- |
| Annulla | `Ctrl+Z` | menu **Modifica**, barra degli strumenti |
| Ripeti | `Ctrl+Shift+Z` | menu **Modifica**, barra degli strumenti |

## L'azione nomina ciò che annullerà

Il menu non dice «Annulla» ma **«Annulla: modifica di un testo»**, secondo
l'operazione che verrà annullata. Il pulsante della barra degli strumenti conserva la parola semplice — la sua
larghezza altrimenti seguirebbe l'ultima operazione, e si sposterebbe sotto il
puntatore — ma il suo suggerimento a comparsa riporta l'etichetta completa.

| Operazione | Che cosa legge l'azione |
| :-------- | :-------------------- |
| una cella di testo modificata | `editing a text` |
| un inizio, una fine | `editing a start`, `editing an end` |
| una durata | `editing a duration` |
| un inserimento di righe | `inserting` |
| una rimozione di righe | `removing` |
| uno spostamento, `Shift Whole File onto Grid` compreso | `shifting` |
| una trasformazione | `transforming` |
| una conversione della frequenza dei fotogrammi | `converting the frame rate` |
| un allineamento su una frequenza dei fotogrammi | `aligning on the frame rate` |
| una rimozione di indicazioni | `removing hearing-impaired mentions` |
| l'applicazione del corsivo | `putting in italics` |
| la rimozione del corsivo | `taking italics out` |
| un cambio di maiuscole e minuscole | `changing the case` |
| trattini aggiunti, rimossi | `adding dialogue dashes`, `removing dialogue dashes` |
| un'unione di righe | `merging` |
| una divisione di una riga | `splitting` |
| testi tagliati, incollati | `cutting texts`, `pasting texts` |
| una regolazione delle durate | `adjusting durations` |
| l'apertura di una traduzione | `opening a translation` |
| l'accodamento di un file | `appending a file` |
| la divisione del progetto | `splitting the project` |
| una correzione accettata in `Correct Texts…`, o eseguita da `Check Spelling…` | `correcting texts` |
| una sostituzione, tutte le sostituzioni | `replacing`, `replacing all` |

**Ventisette etichette, e questo è tutto ciò che la finestra può produrre.** Il nucleo ne nomina
una ventottesima — `sorting` — che nessuna azione della finestra raggiunge: l'ordinamento
avviene solo con una politica di ordine rigorosa, mentre la finestra apre i suoi documenti
con la politica tollerante, che segnala il disordine anziché ripararlo.

**`Shift Whole File onto Grid` non si distingue da uno spostamento**, ed è
corretto: ne è uno, la cui entità è stata misurata anziché digitata.

Sono le stesse parole della riga di comando: sono scritte una volta sola,
nel nucleo.

Quando non c'è nulla da annullare, l'azione legge semplicemente «Annulla» **ed è
disattivata**: la sua scorciatoia non fa nulla.

## Il segno di modifica

Il titolo della finestra porta un segno finché il file differisce da quello
su disco — un asterisco, o ciò che la piattaforma usa al suo posto. **Lo porta anche l'etichetta
della scheda**, `premier.srt*`: il titolo parla solo del progetto che si sta
guardando, l'etichetta informa su quelli che non si stanno guardando.

**Ogni scheda ha la propria cronologia.** Annulla annulla solo ciò che il progetto della
scheda corrente ha ricevuto — una `Replace All` fatta su tutti i progetti compresa:
lascia una voce in ciascun progetto che ha toccato.

**Scompare se si annulla fino al punto di partenza.** Non è un dettaglio di
visualizzazione: il numero di modifiche viene contato, non registrato come un sì o un no, per cui
tornare indietro può riportarlo a zero e il segno scompare.

## Due cose da sapere

**Il punto di riferimento è l'ultimo salvataggio**, o l'apertura del file se non
ce n'è stato alcuno. Salvare lo sposta: il segno scompare, e annullare in seguito
lo riporta.

Un'operazione eseguita dopo un annullamento **cancella ciò che c'era da ripetere**:
ripetere riprodurrebbe un comando il cui stato di partenza non esiste più. È il
comportamento di ogni editor, ed è voluto.

**Mentre una cella è aperta**, `Undo` e `Redo` — dal menu o dalla barra degli strumenti —
confermano prima l'input, come gli altri gesti di
[Modificare una cella](editing.md). Ciò che è appena stato digitato diventa allora l'ultima
voce della cronologia: `Undo` annulla quella, non l'operazione precedente; `Redo` non ha
più nulla da ripetere, poiché un input cancella ciò che c'era da ripetere. Una cella
aperta in cui non è stato digitato nulla non lascia alcuna voce: `Undo` annulla allora l'operazione
precedente.

La cronologia conserva mille voci e dimentica le più vecchie oltre tale limite.
