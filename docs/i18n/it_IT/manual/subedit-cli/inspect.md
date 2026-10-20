# `inspect`

<!-- machine-translated -->
> Questa pagina è una traduzione automatica e non è ancora stata rivista da un lettore della lingua.

```
subedit-cli inspect [--frame-rate RATE] [--recursive] [--detail]
                   [-t FILE] [--align-method position|number] <file>...
```

Riferisce di che cosa è fatto ciascun file. **Non cambia nulla e non scrive alcun
file.**

<!-- example: subedit-cli inspect --help -->
```console
$ subedit-cli inspect --help
Report what a subtitle file is made of
Usage: subedit-cli inspect [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to report on

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --detail                    List every anomaly, subtitle by subtitle, instead of counting them
  --frame-rate RATE           Frame rate of a file counted in frames: 25, 23.976
  -t,--translation-file FILE  Translation file to lay over the subtitle file, for a single input
  --align-method position|number
                              How the lines of the translation find their subtitles
```

## Argomenti

| Argomento | Obbligatorio | Valore | Predefinito |
| :------- | :----- | :----- | :----- |
| `<file>...` | sì | uno o più percorsi di file di sottotitoli | — |
| `--recursive`, `-r` | no | un'opzione senza valore | disattivata — vedere [Elaborare un albero di directory](lots.md) |
| `--detail` | no | un'opzione senza valore: nomina ogni anomalia, invece di contarle — vedere [Che cosa segnala `anomalies`](#che-cosa-segnala-anomalies) | disattivata |

Nessun percorso è un errore d'uso, quindi codice `1`.

## Opzioni

| Opzione | Obbligatorio | Valore | Predefinito |
| :----- | :------ | :----- | :----- |
| `--frame-rate` | no | una frequenza in fotogrammi al secondo: `25`, `23.976`, `29.97` | 23.976 |

**Serve solo per i file contati in fotogrammi**, cioè MicroDVD, l'unico
dei nove. Un file MicroDVD contiene soltanto numeri di fotogramma; la
frequenza con cui vengono letti decide **ogni posizione del rapporto**,
a cominciare da `span`. Se indicata altrove, non cambia nulla e non è un
errore.

Un valore che non designa nulla è un errore d'uso, quindi codice `1`:

<!-- example: subedit-cli inspect --frame-rate douze film.sub; echo $? -->
```console
$ subedit-cli inspect --frame-rate douze film.sub; echo $?
--frame-rate: "douze" is not a frame rate: expected frames per second, like 25 or 23.976
1
```

È la stessa opzione di [`convert`](convert.md), parola per parola, perché
è la stessa frequenza. `convert` la usa anche per **scrivere**.

## Una traduzione: `-t`

| Opzione | Obbligatorio | Valore | Predefinito |
| :----- | :------ | :----- | :----- |
| `-t`, `--translation-file` | no | un file di traduzione, per **un solo** input | — |
| `--align-method` | no, e solo con `-t` | `position` o `number` | `position` |

Con `-t`, la traduzione viene sovrapposta al file **come fa la finestra all'apertura** — vedere
[Una traduzione](invocation.md#una-traduzione) — e il rapporto dice **come le sue righe hanno trovato il
proprio sottotitolo**. Uno script sa così se una traduzione è allineata prima di correggerla.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour.\n\n' > film.fr.srt; subedit-cli --quiet inspect -t film.fr.srt film.srt | tail -3 -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour.\n\n' > film.fr.srt; subedit-cli --quiet inspect -t film.fr.srt film.srt | tail -3
  anomalies: none
  translation file: film.fr.srt, matched by position
  translation: 1 line attached; 1 subtitle left without a translation
```

La prima riga aggiunta nomina il file e il metodo; la seconda è **la frase che la finestra
pronuncia** quando apre una traduzione, ogni parte solo quando non è zero:

| Parte | Che cosa conta |
| :----- | :---------------- |
| `N lines attached` | le righe della traduzione che hanno trovato un sottotitolo del file principale |
| `N subtitles born of a line` | le righe che non ne hanno trovato nessuno e ne hanno generato uno proprio, alle proprie posizioni |
| `N subtitles left without a translation` | i sottotitoli del file principale che nessuna riga ha raggiunto |
| `N lines out of order` | le righe che venivano dopo una successiva nel file |

Una traduzione che non contiene alcuna riga dice `translation: the file holds no line`. **Il rapporto del
file stesso resta quello del solo file**: un sottotitolo nato da una riga non viene contato in
`subtitles`.

Gli errori di `-t` sono quelli di una lettura, detti con il percorso della traduzione, più questo:
`<translation>: the file is already open as the main document`, quando il file indicato è quello principale.
`--align-method` senza `-t` è un errore d'uso, e lo è anche `-t` con più input — vedere
[Una traduzione](invocation.md#una-traduzione).

## Output

Il rapporto va sull'**output standard** — è il risultato del comando, e
viene scritto a ogni livello, `--quiet` compreso. Un blocco per file, preceduto
dal percorso:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,500\nLe canot dérive.\n\n2\n00:00:04,000 --> 00:00:06,200\nPersonne n a vu la côte.\n' > exemple.srt; subedit-cli --quiet inspect exemple.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,500\nLe canot dérive.\n\n2\n00:00:04,000 --> 00:00:06,200\nPersonne n a vu la côte.\n' > exemple.srt; subedit-cli --quiet inspect exemple.srt
exemple.srt
  format: SubRip
  encoding: UTF-8, detected
  byte order mark: absent
  line endings: LF
  subtitles: 2
  span: 00:00:01.000 -> 00:00:06.200
  frame rate grid: none (too few subtitles to tell)
  anomalies: none
```

| Campo | Che cosa dice |
| :---- | :----------- |
| `format` | uno dei formati accettati da `--to`, scritto per esteso: `SubRip`, `WebVTT`, `SubViewer 2`, `Sub Station Alpha`, `Advanced SSA`, `MicroDVD`, `MPL2`, `TMPlayer`, `LRC` |
| `encoding` | la codifica letta, seguita da **l'origine della risposta**: `, from its byte order mark` se il file la dichiara, `, as asked for` se l'ha imposta `--encoding`, `, detected` se è stata valutata |
| `byte order mark` | `present` o `absent` |
| `line endings` | `LF`, `CRLF` o `CR`, seguito da `, mixed from line N` se il file li mescola |
| `subtitles` | il numero di sottotitoli letti |
| `span` | dal primo inizio all'ultima fine in ordine di tempo, `HH:MM:SS.mmm` |
| `frame rate grid` | la frequenza dei fotogrammi su cui sono state calcolate le posizioni, **dedotta** — vedere più avanti |
| `frame rate` | per un file contato in fotogrammi, la frequenza con cui è stato letto e **da dove viene** — sostituisce la riga precedente, vedere più avanti |
| `anomalies` | `none` sulla stessa riga, oppure ciò che non va: **un tipo per riga con il suo conteggio**, rientrato sotto `anomalies:`; con `--detail`, **un'anomalia per riga**, sottotitolo per sottotitolo |

**`span` non è "dal primo all'ultimo"** ma dal più anticipato al più tardivo:
in un file il cui ordine è rotto, i due differiscono, e solo il secondo dice
la verità su ciò che il file copre.

**`anomalies`, con `--detail`, conta i sottotitoli a partire da 1**, come li
numera il file — e non le righe del file. Il numero di un sottotitolo resiste a una
modifica; il numero di una riga no. Vedere più avanti.

## Che cosa segnala `frame rate grid`

**Un file di sottotitoli non dichiara la propria frequenza dei fotogrammi.** SubRip non ha
intestazione, quella di WebVTT è testo libero. Questa riga quindi non legge nulla:
**deduce**, misurando se le posizioni cadono su una griglia di fotogrammi.

Ciò che si misura sono gli **inizi**. Dove esiste una griglia, essi vi si trovano
senza eccezioni, mentre le fini se ne discostano spesso — una fine viene
frequentemente calcolata con una regola di velocità di lettura anziché posta su
un fotogramma.

La prima riga dice sempre la stessa cosa: la frequenza scelta, il verdetto,
e la concentrazione che lo sostiene.

| Verdetto | Che cosa significa |
| :------ | :----------------- |
| `clean` | le posizioni sono su una griglia |
| `partial` | una parte lo è, il resto no |
| `none` | nessun candidato spiega alcunché |

`none` ha **due cause, e non sono la stessa risposta**: o nessuna
griglia si adatta — la riga dà allora il miglior candidato e il suo punteggio,
così da vedere di quanto fallisce — oppure il file conta troppo pochi
sottotitoli perché se ne possa dire qualcosa. Due inizi sembrano sempre
perfettamente allineati; ciò non prova nulla.

**Un file regolare su una frequenza non standard dà `none`, ed è
voluto.** L'insieme dei candidati è chiuso — le otto frequenze che la
conversione offre — e riferire la meno sbagliata tra esse significherebbe dare
una risposta errata dove "non lo so" è la risposta giusta.

### Le righe successive, quando hanno qualcosa da dire

| Riga | Quando compare |
| :---- | :------------------ |
| `grid offset` | le posizioni sono sulla griglia **a meno di una costante**: il file è stato spostato, ed ecco di quanto |
| `also fits` | un altro candidato si adatta altrettanto bene, perché è un multiplo intero di quello scelto |
| `too short a span to separate` | l'estensione del file non consente di distinguere la frequenza scelta da quelle nominate |
| `off the grid` | quanti inizi si discostano dalla griglia, e in quante **serie** |

**`also fits` non è un'esitazione.** Una griglia a 25 fotogrammi al secondo è
*inclusa* in una griglia a 50: un file sincronizzato su 25 si adatta quindi a entrambe.
Il contrario è falso — un vero 50 collassa su 25 — perciò viene scelta la più bassa,
e l'altra viene nominata anziché taciuta. Le uniche
coppie interessate sono 25 e 50, 29.97 e 59.94, 30 e 60.

**`too short a span to separate` protegge da una risposta troppo sicura.** 23.976
e 24 divergono di un millisecondo per secondo di film: su un lungometraggio si
separano senza dubbio, su dieci secondi non si distinguono. La riga dice
quale rimane possibile.

**`off the grid` distingue due storie che la sola percentuale confonde.**
Molte serie di un solo inizio sono posizioni corrette a mano,
una per una. Poche serie lunghe sono una sezione risincronizzata o un file
assemblato da altri due. È il numero di serie a separarle.

### Un esempio

Dodici righe poste su una griglia a 25 fotogrammi al secondo — quindi a
posizioni che sono multipli di 40 millisecondi — distribuite su quasi nove minuti:

<!-- example: k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > grille.srt; subedit-cli --quiet inspect grille.srt | tail -3 -->
```console
$ k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > grille.srt; subedit-cli --quiet inspect grille.srt | tail -3
  frame rate grid: 25 fps, clean (100.0%)
  also fits: 50 fps, of which this rate is a whole divisor
  anomalies: none
```

L'esempio crea un proprio file anziché leggerne uno dal repository, ed è
voluto: si può rieseguire così com'è, e il ciclo mostra che cos'è una griglia
meglio di una frase — posizioni che sono multipli della durata di un fotogramma.

## Che cosa segnala `frame rate`, e perché non è una griglia

**Solo uno dei nove formati conta in fotogrammi**: MicroDVD, ogni riga del quale
porta due numeri di fotogramma e nient'altro. Su un file simile, `inspect`
non scrive `frame rate grid` ma `frame rate`, e le due righe non
compaiono mai insieme.

**Il motivo sta in una frase.** Le posizioni di un file MicroDVD sono state
calcolate *a partire* dai suoi fotogrammi, a una data frequenza; dedurle da una
griglia restituirebbe esattamente quella frequenza, appena fornita. Sarebbe
un dato fornito, travestito da misura.

Ciò che si scrive invece è **la frequenza di lettura e da dove viene** —
nella stessa forma della riga `encoding`:

| Che cosa dice la riga | Quando |
| :------------------ | :---- |
| `frame rate: 25 fps, as asked for` | l'ha imposta `--frame-rate` |
| `frame rate: 24000/1001 fps, assumed` | nessuno l'ha detta; è il valore predefinito |

**Due risposte e non tre**, dove `encoding` ne ha tre: un file MicroDVD
non dichiara nulla. È tutto il problema — la frequenza viene necessariamente
da fuori, e il rapporto dice quale dei due esterni.

**La frequenza decide tutto il resto del rapporto.** `span` passa da essa, e due
letture dello stesso file a due frequenze danno due estensioni:

<!-- example: printf '{25}{75}Première.\n{100}{150}Deuxième.\n{200}{260}Troisième.\n' > images.sub; subedit-cli --quiet inspect images.sub | tail -3; subedit-cli --quiet inspect --frame-rate 25 images.sub | tail -3 -->
```console
$ printf '{25}{75}Première.\n{100}{150}Deuxième.\n{200}{260}Troisième.\n' > images.sub; subedit-cli --quiet inspect images.sub | tail -3; subedit-cli --quiet inspect --frame-rate 25 images.sub | tail -3
  span: 00:00:01.043 -> 00:00:10.844
  frame rate: 24000/1001 fps, assumed
  anomalies: none
  span: 00:00:01.000 -> 00:00:10.400
  frame rate: 25 fps, as asked for
  anomalies: none
```

## Che cosa segnala `anomalies`

In un documento possono andare storte tre cose, e ciascuna si ripara in modo diverso.
**Per impostazione predefinita `inspect` le conta**, una riga per tipo, nell'ordine della tabella:
un file fatto male ne porta centinaia, e sapere quante ce ne sono di ogni tipo
è ciò che dice che cosa eseguire. **`--detail` le nomina una per una.**

| Che cosa è scritto | Con `--detail` | Che cosa significa |
| :--------------- | :-------------- | :------------------ |
| `subtitle ends before it starts: N` | `subtitle N ends before it starts` | la fine precede l'inizio |
| `subtitle starts before the previous one ends: N` | `subtitle N starts before the previous one ends` | si sovrappone al precedente |
| `subtitle starts before the previous one starts: N` | `subtitle N starts before the previous one starts` | rompe l'ordine del file — [`sort`](sort.md) lo rimette a posto |

Un tipo assente non viene scritto. Nella colonna di sinistra, `N` è il numero
di sottotitoli; in quella successiva, il numero di un sottotitolo.

**Lo stesso sottotitolo può comparire due volte**, ed è voluto: quello che
inizia prima che il precedente sia iniziato inizia anche prima che sia
finito. Il primo punto si risolve spostando il sottotitolo, il secondo
cambiando una durata.

**Il sottotitolo nominato è quello fuori posto**, non quello rispetto al quale
lo è. Con inizi a `0`, `4 s`, `2 s`, `3 s`: il terzo rompe l'ordine, il
quarto segue davvero il terzo e quindi non viene nominato — non c'è nulla da
fare.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect desordre.srt | tail -3 -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect desordre.srt | tail -3
  anomalies:
    subtitle starts before the previous one ends: 1
    subtitle starts before the previous one starts: 1
```

E con `--detail`, il sottotitolo nominato:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect --detail desordre.srt | tail -3 -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect --detail desordre.srt | tail -3
  anomalies:
    subtitle 3 starts before the previous one ends
    subtitle 3 starts before the previous one starts
```

**Una riga per anomalia con `--detail`**, perché un centinaio di rilevamenti
ammassati su una riga non si possono né leggere né cercare. Un documento senza difetti conserva
la sua unica riga, `anomalies: none`, che uno script cerca. **Il
record `--format json` elenca sempre ogni anomalia**: uno script legge il dettaglio, e
`--detail` cambia soltanto il testo.

**Non è un messaggio diagnostico di lettura.** I messaggi diagnostici dicono ciò che la
lettura ha incontrato e indicano una **riga del file**; le anomalie dicono
che cosa il documento *è* e indicano un **sottotitolo**. Un file il cui ordine
è rotto non è per questo malformato: la lettura non ha nulla da dire in proposito.

## In JSON

Con [`--format json`](invocation.md#un-output-che-uno-script-può-leggere), `inspect` scrive **un
oggetto per file** invece dei blocchi qui sopra; il resoconto resta sull'errore standard.
Le chiavi descrivono ciò che dice il testo, e **nessuna va riletta da una frase**:

| Chiave | Contenuto |
| :-- | :------ |
| `format` | l'identificatore del formato, quello che `convert --to` accetta: `srt`, `vtt`… |
| `encoding` | `{"charset", "origin", "byte_order_mark"}` — `origin` è `byte-order-mark`, `asked` o `detected` |
| `line_endings` | `{"kind", "mixed_from_line"}` — `lf`, `crlf` o `cr`; la riga in cui compare un secondo tipo, oppure `null` |
| `subtitles` | il numero di sottotitoli |
| `span_ms` | `{"start", "end"}` — il primo inizio e l'ultima fine, in millisecondi |
| `frame_rate` | per un file contato in fotogrammi: `{"rate", "origin"}` (`asked` o `assumed`); altrimenti `null` |
| `grid` | per gli altri: la griglia dedotta, oppure `null` per un file contato in fotogrammi |
| `anomalies` | un array di `{"subtitle", "kind"}` — `end-before-start`, `overlapping-subtitles`, `out-of-order` |
| `translation` | **solo con `-t`**: `{"file", "method", "attached", "born", "untranslated", "out_of_order"}` — `method` è `position` o `number`, e i quattro conteggi sono quelli della frase qui sopra |
| `warnings` | ciò che la lettura ha incontrato, a ogni livello |

`grid` contiene `verdict` (`clean`, `partial` o `silent`), `enough_starts`, `rate` (una stringa,
oppure `null` quando non viene nominata nessuna), `concentration_permille` (millesimi, `null` senza abbastanza
inizi), `offset_ms`, `also_fits` (la frequenza di cui questa è un divisore intero, oppure
`null`), `not_separated` (le frequenze che un'estensione troppo breve non separa), `strays`
(gli inizi fuori dalla griglia) e `starts`.

<!-- example: printf '{25}{50}Un.\n{75}{100}Deux.\n' > f.sub; subedit-cli --format json inspect --frame-rate 25 f.sub 2>/dev/null -->
```console
$ printf '{25}{50}Un.\n{75}{100}Deux.\n' > f.sub; subedit-cli --format json inspect --frame-rate 25 f.sub 2>/dev/null
{"schema":1,"command":"inspect","file":"f.sub","ok":true,"format":"microdvd","encoding":{"charset":"UTF-8","origin":"detected","byte_order_mark":false},"line_endings":{"kind":"lf","mixed_from_line":null},"subtitles":2,"span_ms":{"start":1000,"end":4000},"frame_rate":{"rate":"25","origin":"asked"},"grid":null,"anomalies":[],"warnings":[]}
```

## Resoconto

Sull'errore standard, secondo il livello richiesto:

| Livello | Che cosa si aggiunge |
| :----- | :-------------- |
| 1 | `<path>: N subtitles` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings` |
| 3 | `<path>: N bytes read`, `<path>: N diagnostic(s) while reading`, poi **ciascuno di essi** — vedere [Invocazione](invocation.md#diagnostica-di-lettura) |

## Codici di uscita

Quelli dello strumento: `0` se ogni file è stato letto, `2` se nessuno, `3` se
solo alcuni, `1` in caso di errore d'uso.

## Errori

Quelli di una lettura — del file, e della traduzione quando è dato `-t` —, e
quelli di `-t`: vedere più sopra. `inspect` non scrive nulla che possa essere rifiutato.

| Che cosa lo provoca | Messaggio |
| :------------------ | :------ |
| nessun percorso indicato | `files is required`, seguito da un rimando a `--help` |
| file mancante | `<path>: does not exist` |
| file illeggibile | `<path>: cannot be opened: permission denied` |
| byte che non si decodificano | `<path>: cannot be decoded in the chosen encoding` |
| formato non riconosciuto | `<path>: is in no format this tool knows` |
| nulla che somigli a un sottotitolo | `<path>: holds nothing recognisable as a subtitle` |

Sono gli stessi messaggi per i diciassette sottocomandi: la procedura di apertura
è scritta una sola volta, nel nucleo.

## Un esempio che fallisce

Un file vuoto non corrisponde ad alcun formato. Non viene segnalato come vuoto:
viene rifiutato, e nominato.

<!-- example: : > vide.srt; subedit-cli inspect vide.srt; echo "code=$?" -->
```console
$ : > vide.srt; subedit-cli inspect vide.srt; echo "code=$?"
vide.srt: is in no format this tool knows
code=2
```
