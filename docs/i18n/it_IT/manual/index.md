# subedit — manuale

<!-- machine-translated -->
> Questa pagina è una traduzione automatica e non è ancora stata rivista da un lettore della lingua.

subedit legge, ispeziona e risincronizza i file di sottotitoli. Si presenta come due
programmi che condividono lo stesso nucleo, e quindi le stesse regole e le stesse parole.

| Programma | A che serve | Il suo manuale |
| :-------- | :-------------- | :--------- |
| `subedit-gui` | aprire un file, vederlo in una tabella, modificarlo, confrontarlo con il film | [il manuale della finestra](subedit-gui/index.md) |
| `subedit-cli` | elaborare file in blocco, da uno script, senza interfaccia | [il manuale della riga di comando](subedit-cli/index.md) |

Entrambi leggono e scrivono **i nove formati** — SubRip, WebVTT, SubViewer 2,
Sub Station Alpha, Advanced SSA, MicroDVD, MPL2, TMPlayer, LRC — e
riconoscono il formato dal contenuto anziché dall'estensione. **Anche la codifica
è riconosciuta dal contenuto**: un file in Latin-1, in CP1252 o in UTF-16
si apre, viene indicato come è stato letto e viene riscritto nella sua codifica — un
file aperto e poi salvato senza modifiche non cambia di un solo byte.

## Da dove cominciare

| Che cosa si vuole | Dove andare |
| :----------------- | :------- |
| installare lo strumento | [Installazione](subedit-cli/installation.md) — vale per entrambi i programmi |
| aprire un file e vedere che cosa contiene | [La tabella](subedit-gui/table.md) |
| risincronizzare sottotitoli sfasati di una quantità costante | [Le operazioni](subedit-gui/operations.md#shift-positions), oppure [`shift`](subedit-cli/shift.md) |
| risincronizzare un file scritto per un'altra frequenza dei fotogrammi | [`framerate`](subedit-cli/framerate.md), e [`snap`](subedit-cli/snap.md) se la sincronizzazione è già corretta |
| sapere per quale frequenza dei fotogrammi è stato scritto un file | [La griglia dei fotogrammi](subedit-gui/frame-grid.md), oppure [`inspect`](subedit-cli/inspect.md) |
| rimuovere le indicazioni per non udenti | [Le operazioni](subedit-gui/operations.md#remove-hearing-impaired-mentions), oppure [`hearing-impaired`](subedit-cli/hearing-impaired.md) |
| correggere indicazioni, errori comuni, uso delle maiuscole e a capo in un solo passaggio | [`Correct Texts…`](subedit-gui/correct-texts.md), oppure [`correct`](subedit-cli/correct.md) |
| controllare l'ortografia parola per parola, con i dizionari del sistema | [`Check Spelling…`](subedit-gui/check-spelling.md) |
| allungare i sottotitoli troppo brevi per essere letti | [`Adjust Durations…`](subedit-gui/operations.md#adjust-durations), oppure [`adjust`](subedit-cli/adjust.md) |
| rimettere in ordine un file i cui sottotitoli non lo sono | [`sort`](subedit-cli/sort.md) |
| ricavare un solo sottotitolo dai pezzi di uno stesso testo che si susseguono | [`Join Repeated Subtitles`](subedit-gui/lines.md#unire-i-sottotitoli-ripetuti), oppure [`join-repeats`](subedit-cli/join-repeats.md) |
| raddrizzare in un colpo solo un file mal segmentato, fuori ordine e con sovrapposizioni | [`repair`](subedit-cli/repair.md) |
| rimuovere le sovrapposizioni tra sottotitoli | [`Make Ranges Disjoint…`](subedit-gui/lines.md#rendere-disgiunti-gli-intervalli), oppure [`disjoint`](subedit-cli/disjoint.md) |
| mettere in sequenza le due parti di un film | [`Append File…`](subedit-gui/operations.md#append-file), oppure [`append`](subedit-cli/append.md) |
| tagliare un film in due parti | [`Split Project…`](subedit-gui/operations.md#split-project), oppure [`split-file`](subedit-cli/split-file.md) |
| cambiare le maiuscole e minuscole, il corsivo o i trattini di dialogo di un intero file | [Le operazioni](subedit-gui/operations.md#italic), oppure [`case`](subedit-cli/case.md), [`italics`](subedit-cli/italics.md), [`dialogue-dashes`](subedit-cli/dialogue-dashes.md) |
| cercare una parola e sostituirla, senza rompere i tag | [Trova e sostituisci](subedit-gui/find-and-replace.md), oppure [`replace`](subedit-cli/replace.md) |
| tradurre accanto al testo originale | [Aprire una traduzione](subedit-gui/files.md#aprire-una-traduzione), oppure [`pair`](subedit-cli/pair.md) per risincronizzarla sul file principale |
| elaborare un centinaio di file in una volta sola | [Elaborare un albero di directory](subedit-cli/lots.md), poi [Invocazione](subedit-cli/invocation.md) |

**Tra `framerate` e `snap`, un errore passa inosservato.** Nel dubbio,
[`snap`](subedit-cli/snap.md#snap-non-è-framerate-e-scambiarli-non-si-nota)
si apre sul confronto tra i due.

## Che cosa promette questo manuale

**Descrive ciò che esiste, mai ciò che è previsto.** Ciò che verrà, e in
quale ordine, è nella [tabella di marcia](../../../feuille-de-route.md).

**Questo manuale è scritto in inglese, e tradotto.** Le sue traduzioni lo seguono
pagina per pagina, nella directory `docs/i18n/` del repository, un albero per lingua. I
programmi parlano la lingua del loro ambiente; i nomi che mostrano — sottocomandi,
opzioni, voci di menu — sono qui citati come il programma li scrive in inglese.

**Gli esempi e le immagini sono generati, il testo no.** I
blocchi `console` sono prodotti eseguendo davvero il comando; le
schermate, costruendo la vera finestra e fotografandola. Nessuno
dei due può quindi descrivere uno strumento che non esiste più. Ciò che resta senza rete
è il testo che li circonda, la cui correttezza poggia sulla rilettura.

**Le schermate mostrano entrambe le tavolozze**, `Light` e `Dark` — quelle
che [le preferenze](subedit-gui/preferences.md#il-tema) offrono.

## Le due superfici dicono le stesse parole

Un motivo per leggere l'uno o l'altro manuale usando l'altro programma: i
messaggi di errore, i nomi dei formati, i verdetti sulla griglia e le
etichette di annullamento sono scritti **una sola volta**, nel nucleo, ed entrambi i programmi
li citano. Un file rifiutato dà la stessa frase da una parte e dall'altra.

Ciò che differisce è quello che ciascuna superficie sa fare:

| | `subedit-gui` | `subedit-cli` |
| :--- | :------------ | :------------ |
| più file alla volta | sì — una scheda per progetto | sì |
| una traduzione accanto al file principale | sì | sì — `-t` sui sottocomandi di testo, e [`pair`](subedit-cli/pair.md) per risincronizzarla |
| accodare un file, dividere un progetto | sì | sì — [`append`](subedit-cli/append.md), [`split-file`](subedit-cli/split-file.md) |
| annullare | sì, mille voci | non applicabile — nulla viene modificato sul posto senza che lo si chieda |
| modificare a mano un testo o una posizione | sì | no |
| inserire, eliminare, unire e dividere righe | sì | no |
| trovare e sostituire | sì | sì — [`replace`](subedit-cli/replace.md) |
| tagliare, copiare e incollare testi | sì | no |
| regolare le durate, il corsivo, le maiuscole e minuscole, i trattini | sì | sì — [`adjust`](subedit-cli/adjust.md), [`italics`](subedit-cli/italics.md), [`case`](subedit-cli/case.md), [`dialogue-dashes`](subedit-cli/dialogue-dashes.md) |
| correggere i testi tramite modelli | sì — `Correct Texts…` | sì — [`correct`](subedit-cli/correct.md) |
| controllare l'ortografia parola per parola | sì — `Check Spelling…` | no |
| guardare il film mentre si sincronizza | sì | no |
| scrivere sopra l'input | `Save` | `--in-place` |
