# `correct`

<!-- machine-translated -->
> Questa pagina è una traduzione automatica e non è ancora stata rivista da un lettore della lingua.

```
subedit-cli correct --tasks LIST --code CODE
                    [--classes human|ocr|human,ocr]
                    [--enable NAME]... [--disable NAME]...
                    [--language CODE]
                    [--max-length N] [--max-lines N]
                    [--skip-length N|off] [--skip-lines N|off]
                    [--keep-blank-subtitles | --remove-blank-subtitles]
                    [--range N-M|N-]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    <file>...
```

Corregge i testi con i **modelli di Gaupol**, come l'assistente
[`Correct Texts…`](../subedit-gui/correct-texts.md) della finestra — stessi modelli,
stesso motore, stessi conteggi. Qui esistono sei operazioni: le **indicazioni per
non udenti**, l'**unione** e la **divisione di parole**, gli **errori comuni**,
l'**uso delle maiuscole** e la **divisione delle righe**.

<!-- example: subedit-cli correct --help -->
```console
$ subedit-cli correct --help
Correct the texts with the patterns of the correction assistant
Usage: subedit-cli correct [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to correct

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --tasks LIST                The tasks to run, in a list: none is chosen beforehand
  --code CODE                 The cascade of patterns the tasks read: Zyyy, Latn, Latn-en, Latn-en-US
  --classes human|ocr|human,ocr
                              The classes of common errors to apply: both by default
  --enable NAME               Switch a pattern on, by its English name or type:name; repeatable
  --disable NAME              Switch a pattern off, by its English name or type:name; repeatable
  --language CODE             The dictionary join-words and split-words read: fr_FR, en_US, en...
  --max-length N              Longest line of line-break, in characters; no default, Gaupol's is in ems
  --max-lines N               Most lines of line-break: 3, as Gaupol's
  --skip-length N|off         Leave alone a subtitle whose longest line is within this, or off: --max-length
  --skip-lines N|off          Leave alone a subtitle whose line count is within this, or off: --max-lines
  --keep-blank-subtitles      Leave the subtitles the correction empties, empty, instead of removing them
  --remove-blank-subtitles    Remove the subtitles the correction empties, whatever the settings file says
  --range N-M|N-              Act only on subtitles N to M, or N to the end
  --document main|translation The document to change: the main one, or the translation given by -t
  -t,--translation-file FILE  Translation file to lay over the subtitle file, for a single input
  --align-method position|number
                              How the lines of the translation find their subtitles
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Che cosa viene scelto e che cosa no

**Nulla è spuntato in anticipo.** La finestra apre le sue caselle come l'utente le ha lasciate; una
riga di comando prende le proprie operazioni da eseguire, il codice, la lunghezza di riga e la lingua dalla propria riga,
mai dal file delle impostazioni: né le operazioni spuntate un giorno nella finestra, né le caselle
dei modelli attivate lì (`correction.activations`), né l'elenco di sostituzioni del correttore ortografico vengono
letti, e da quella parte non si scrive nulla.

**Quattro impostazioni provengono dal file delle impostazioni, per impostazione predefinita** — i valori che `Correct Texts…`
ricorda, o che [`config set`](config.md) ha scritto. **Un'opzione sulla riga prevale sempre**, poi
il file, poi il valore predefinito incorporato; vedi [Il file delle impostazioni](invocation.md#il-file-delle-impostazioni)
per sapere dov'è, `--config` e `--no-config`.

| Chiave | Che cosa fornisce | Opzione che la annulla |
| :-- | :--------------- | :----------------------- |
| `correction.human`, `correction.ocr` | le classi di errori comuni applicate | `--classes` |
| `correction.remove-blank` | se i sottotitoli svuotati dalla correzione vengono rimossi | `--keep-blank-subtitles`, `--remove-blank-subtitles` |
| `correction.line-break.max-lines` | il massimo di righe di `line-break` | `--max-lines` |

- **Ciascuna delle quattro ha un'opzione che la annulla**, qualunque cosa dica il file: `--classes human,ocr`
  applica entrambe le classi, `--remove-blank-subtitles` rimuove i sottotitoli svuotati, `--max-lines 3`
  ripristina le righe di Gaupol. `--keep-blank-subtitles` e `--remove-blank-subtitles` insieme sono un
  errore di utilizzo.
- **Entrambe le classi disattivate nel file vengono ignorate, e lo si dice** al livello 1 —
  `settings: correction.human and correction.ocr are both false, ignored` —: `--classes` non potrebbe scriverlo, e
  non lascerebbe nulla da fare a common-errors. Vengono applicate invece entrambe le classi.
- **Un numero di righe fuori dall'intervallo da 1 a 1000 non viene affatto letto dal file**: il lettore del file
  lo rifiuta — `settings: correction.line-break.max-lines: cannot read 'N', default kept`, al
  livello 1 — e si usa il 3 incorporato.

Gli esempi di questa pagina sono eseguiti con `--no-config`, in modo che mostrino che cosa fanno i valori
predefiniti incorporati, qualunque siano le impostazioni della macchina che li esegue.

| Opzione | Obbligatorio | Valori | Predefinito |
| :----- | :----- | :------ | :----- |
| `<file>...` | sì | uno o più percorsi | — |
| `--tasks` | **sì** | un elenco separato da virgole: `mentions`, `join-words`, `split-words`, `common-errors`, `capitalization`, `line-break` | nessuna operazione |
| `--code` | **sì** con un'operazione a modelli, e **non applicabile** altrimenti | `Script[-language[-COUNTRY]]`: `Zyyy`, `Latn`, `Latn-en`, `Latn-en-US` | — |
| `--language` | **sì** con `join-words` o `split-words`, e **non applicabile** altrimenti | un codice di lingua: `fr`, `fr_FR`, `en_US` | — |
| `--classes` | no | `human`, `ocr` oppure `human,ocr` | entrambe — oppure ciò che dice il file delle impostazioni |
| `--enable`, `--disable` | no, ripetibili | il nome inglese di un modello, oppure `type:name` | i file `.conf` forniti |
| `--max-length` | **sì** con `line-break` | un numero di **caratteri** maggiore di zero | **nessuno** — vedi sotto |
| `--max-lines` | no | un numero intero di righe, maggiore di zero | 3 — oppure ciò che dice il file delle impostazioni |
| `--skip-length`, `--skip-lines` | no | un numero, oppure `off` | gli stessi limiti di `--max-length` e `--max-lines` |
| `--keep-blank-subtitles` | no | un'opzione senza valore: i sottotitoli svuotati restano, vuoti | i sottotitoli svuotati vengono rimossi — a meno che il file delle impostazioni non dica altrimenti |
| `--remove-blank-subtitles` | no | un'opzione senza valore: i sottotitoli svuotati vengono rimossi, qualunque cosa dica il file delle impostazioni; rifiutata con `--keep-blank-subtitles` | disattivata |
| `--range` | no | `N-M`, oppure `N-` fino alla fine — vedi [`adjust`](adjust.md#--range) | l'intero file |
| `--document` | no | `main` oppure `translation` — vedi [Una traduzione](invocation.md#una-traduzione) | `main` |
| `-t`, `--translation-file` | con `--document translation` | un file di traduzione, per **un solo** input | — |
| `--align-method` | no, e solo con `-t` | `position` oppure `number` | `position` |
| `--recursive`, `-r` | no | un'opzione senza valore | disattivata — vedi [Elaborare un albero di directory](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **una delle tre**, tranne con `--dry-run` | vedi [Invocazione](invocation.md#la-destinazione) | — |
| `--dry-run` | no | un'opzione senza valore: calcola e riferisce, non scrive nulla — vedi [Vedere prima di scrivere](invocation.md#vedere-prima-di-scrivere) | disattivata |

### Le operazioni da eseguire

**L'ordine dell'elenco non fa differenza**: le operazioni si applicano nell'ordine
di Gaupol — le indicazioni, poi l'unione e la divisione di parole, poi gli errori comuni, poi l'uso delle maiuscole, poi la divisione delle righe —, ciascuna sul
testo lasciato dalla precedente. Un sottotitolo svuotato dalle indicazioni non ha
alcun ruolo in ciò che segue.

| Operazione | Che cosa fa |
| :---- | :-------------- |
| `mentions` | rimuove i suoni tra parentesi quadre, e tra parentesi tonde quando `Sound in parentheses` è attivato (la scansione di [`hearing-impaired`](hearing-impaired.md)), poi i modelli di testi di canzoni e di parlanti |
| `join-words` | riunisce una parola tagliata in due, in base al dizionario di `--language` |
| `split-words` | divide due parole attaccate, in base allo stesso dizionario |
| `common-errors` | corregge gli errori comuni: spazi, punteggiatura, legature, errori di OCR |
| `capitalization` | mette la maiuscola dove la lingua la richiede |
| `line-break` | taglia di nuovo le righe di un sottotitolo in modo che nessuna superi `--max-length`, valutando i modelli di interruzione del codice |

### Il codice dei modelli

**`--code` indica la cascata**: `Zyyy` per qualsiasi scrittura, poi la scrittura, la lingua
e il paese — `Latn-en-US` legge i modelli di `Zyyy`, di `Latn`, di `Latn-en` e di
`Latn-en-US`. **È obbligatorio**, perché una riga di comando non ha una lingua di ambiente:
un valore predefinito universale applicherebbe *meno* di quanto si creda, senza dirlo. Vale per
ogni operazione della stessa esecuzione; un'operazione che vuole un codice proprio è una seconda
esecuzione. Un codice scritto male è un errore di utilizzo:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code francais --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code francais --output b.srt a.srt; echo "code=$?"
--code: "francais" is not a pattern code: expected Script[-language[-COUNTRY]], like Zyyy, Latn or Latn-en-US
code=1
```

I modelli che l'utente mette in `$XDG_DATA_HOME/subedit/patterns` (oppure
`~/.local/share/subedit/patterns`) **si aggiungono ai modelli forniti**, come nella
finestra, e un modello fornito che non può essere letto viene segnalato al livello 1: `patterns: <file>, line N (<reason>)`.

### Unione e divisione di parole, e il dizionario

**Queste due operazioni non leggono alcun modello**: interrogano il **dizionario del sistema, tramite
Enchant** — una dipendenza dei pacchetti (`libenchant-2-2` su Debian e Ubuntu, `enchant2` su
Fedora), che `subedit-cli` condivide con la finestra. Non hanno quindi né un codice né un nome da impostare:
`--code` non ha senso senza un'operazione a modelli, e `--language` non ne ha senza una di queste due
— darne uno senza l'altro è un errore di utilizzo, codice `1`.

- **`join-words`** unisce una parola che il dizionario non conosce a quella che la precede o la
  segue, **quando una sola direzione dà una parola nota**: `hel lo` diventa `hello`; due direzioni possibili,
  oppure nessuna, non decidono nulla.
- **`split-words`** divide una parola che il dizionario non conosce **quando esattamente uno dei suoi
  suggerimenti è quella parola con uno spazio**: `hellothere` diventa `hello there`. Una parola con l'iniziale
  maiuscola seguita da minuscole viene lasciata stare: nella maggior parte dei casi è un nome.

**Senza un dizionario per la lingua, l'esecuzione viene rifiutata**, codice `1`, **prima che un file venga
letto** e con le parole della finestra, che disattiva la funzione e lo dice — qui la riga di comando
rifiuta e lo dice, anziché tacere:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nhel lo\n\n' > a.srt; subedit-cli --no-config correct --tasks join-words --language qq_QQ --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nhel lo\n\n' > a.srt; subedit-cli --no-config correct --tasks join-words --language qq_QQ --output b.srt a.srt; echo "code=$?"
no dictionary for qq_QQ
code=1
```

**L'elenco di sostituzioni della finestra non viene né letto né scritto.** Si *riempie* con le
scelte dell'utente nel controllo ortografico; una riga di comando non ha una
scelta simile, e leggere l'elenco farebbe dipendere uno script da ciò che il suo autore vi ha messo una volta.

### Divisione delle righe

**Si misura in caratteri, ed è questa la differenza rispetto alla finestra.** La finestra misura in *em* —
una larghezza tarata sul carattere tipografico, a 0,55 em per lettera —, e il suo valore predefinito 24 è un valore
in em. Una riga di comando non ha un carattere tipografico: conta i caratteri, quelli del
testo. **Copiare il 24 come caratteri taglierebbe quasi tutto, senza
dirlo**; non esiste quindi **alcuna lunghezza predefinita**, e `--max-length` è obbligatorio con
`line-break`:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn texte\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn texte\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --output b.srt a.srt; echo "code=$?"
--max-length is required by the task line-break: Gaupol's 24 is a width in ems, and has no value in characters
code=1
```

**`--max-lines` è 3**, il valore di Gaupol, che l'unità non cambia; la divisione delle righe ne prende
una in più quando tre non bastano. **L'esclusione** — un sottotitolo già entro i limiti viene
lasciato com'è — prende per impostazione predefinita **gli stessi limiti** (`--skip-length` = `--max-length`,
`--skip-lines` = `--max-lines`), come fa Gaupol; `--skip-length off` e `--skip-lines off`
la disattivano, e il testo viene allora riletto come uno solo e tagliato di nuovo:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nWhen the rain stopped, we walked home under the dark sky of the old town.\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --max-length 24 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nWhen the rain stopped, we walked home under the dark sky of the old town.\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --max-length 24 --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:03,000
When the rain
stopped, we walked
home under the dark
sky of the old town.
```

I modelli di interruzione sono quelli del codice: `Latn` non interrompe tra un numero e la sua unità né
dopo un trattino di dialogo, `Latn-en` non interrompe dopo un articolo, una preposizione, un titolo
o un aggettivo possessivo. **Sotto `Zyyy`, non esiste alcun modello di interruzione**: le righe sono
solo bilanciate. **Questa operazione non viene mai rifiutata per mancanza di un modello attivo**: i suoi modelli
valutano dove interrompere, l'interruzione avviene anche senza. Le quattro opzioni hanno senso solo con
`line-break`; date senza, sono un errore di utilizzo.

### Le classi e l'attivazione per nome

**`--classes` filtra gli errori comuni in modo definitivo**: togliere la spunta a `ocr` esclude i suoi
modelli dall'applicazione — la legatura `ﬁ` resta allora `ﬁ`. Gli altri tre tipi di modelli
non hanno alcuna classe.

**`--enable` e `--disable` impostano un modello per il suo nome inglese**, essendo il nome la chiave
(ADR 0037); un nome può designare più record, che condividono allora un'unica
casella. È necessario, ed è ciò che mette da parte i valori predefiniti della finestra: **i file
`.conf` forniti disattivano i modelli delle indicazioni**, per cui `--tasks mentions` da solo
non applicherebbe nulla. I due modelli "Sound in brackets" e "Sound in parentheses"
comandano ciascuno il proprio delimitatore — le parentesi quadre, le parentesi tonde — e si impostano con le stesse
opzioni: attivarne uno solo non rimuove l'altro.

Tre cose sono **errori di utilizzo**, codice `1`, segnalati prima che un file venga letto:

| Che cosa lo provoca | Messaggio |
| :------------------ | :------ |
| un nome che non designa alcun modello delle operazioni date, sotto il codice dato | `--enable: "<name>" names no pattern of the tasks given, under the code <code>` |
| un nome che designa modelli di due tipi — lo si scrive allora `type:name`, essendo il tipo `common-error`, `capitalization`, `hearing-impaired` o `line-break` | `--enable: "<name>" names patterns of several types: write type:name, …` |
| un nome dato a `--enable` e a `--disable` | `"<name>" is given to --enable and to --disable: choose one` |

**Un errore di battitura è rumoroso**: la finestra ignora un'impostazione obsoleta perché un'
impostazione può sopravvivere a un modello scomparso; un'invocazione non ha sopravvissuti.

**Un'operazione senza alcun modello attivo viene rifiutata**, codice `1` — "niente da fare" è una
risposta che si dice, non un successo silenzioso:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --output b.srt a.srt; echo "code=$?"
mentions: no pattern is active under the code Latn; switch one on with --enable
code=1
```

Lo stesso file, con la scansione delle parentesi quadre attivata:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --enable "Sound in brackets" --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --enable "Sound in brackets" --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:02,000
Bonjour
```

## Che cosa viene scritto e che cosa viene detto

Il formato, la codifica e i terminatori di riga del file letto vengono **conservati**. **I
sottotitoli svuotati dalla correzione vengono rimossi**, come la casella predefinita
della finestra; `--keep-blank-subtitles` li lascia vuoti, e `--remove-blank-subtitles` li rimuove
anche quando il file delle impostazioni li conserva. **Una traduzione svuotata resta svuotata**,
e conserva il suo sottotitolo.

**Il conteggio è quello della finestra** — "Edited N and removed M subtitles": testi
modificati e sottotitoli rimossi, **mai le corrispondenze**. Un file in cui nulla
cambia viene comunque scritto: una destinazione data è una destinazione scritta.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\neﬀet  de ﬁn\n\n2\n00:00:03,000 --> 00:00:04,000\nPropre.\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code Latn --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\neﬀet  de ﬁn\n\n2\n00:00:03,000 --> 00:00:04,000\nPropre.\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code Latn --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:02,000
effet de fin

2
00:00:03,000 --> 00:00:04,000
Propre.
```

### Un modello che non può essere applicato

**Un modello che non può essere letto, non può essere tradotto o non termina viene nominato, con il
sottotitolo in cui ha rinunciato**, ed è qui che la riga di comando dice più della
finestra: la pagina di conferma nomina il modello una sola volta, senza il testo.

| Motivo | Che cosa significa |
| :----- | :------------------- |
| `cannot be translated` | la sua espressione usa ciò che la traduzione in ICU rifiuta |
| `will not compile` | il motore ha rifiutato la sua espressione |
| `has an invalid replacement` | la sua sostituzione non si legge come la legge Python |
| `timed out` | il motore ha rinunciato a quel testo |
| `never settled` | `Repeat` stava ancora cambiando il testo dopo cento passate |
| `grew the text too long` | il testo ha superato ciò che un sottotitolo può contenere |

Al livello 1, sull'output degli errori:
`<path>: pattern "<name>" (<reason>) was not applied to subtitle N` — senza la parte finale quando il modello è messo da parte per ogni testo. **Il numero è
quello del file**, qualunque siano le operazioni che hanno rimosso sottotitoli prima. In JSON, è
un avviso `pattern-failed` del file, il cui `detail` contiene la frase. **Un modello
abbandonato non è un file fallito**: il file viene scritto, e il codice di uscita è `0`.

## Output

**Output standard** — nulla, essendo il risultato il file scritto. **Con `--dry-run`, sono le
modifiche proposte**, un blocco per sottotitolo: il suo numero, il testo prima, il testo dopo —
e `(removed)` senza riga `+` per un sottotitolo che la correzione rimuoverebbe; con
`--keep-blank-subtitles`, resta, vuoto. Vedi [Vedere prima di scrivere](invocation.md#vedere-prima-di-scrivere).

| Livello | Che cosa si aggiunge sull'output degli errori |
| :----- | :------------------------------------- |
| 1 | `<path>: Edited N and removed M subtitles -> <destination>`, i modelli che non hanno potuto essere applicati, i file di modelli che non possono essere letti — e, prima di tutto, il file delle impostazioni letto — `settings: read <path>` — e ciò che ne è stato ignorato |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, poi ogni diagnostica di lettura |

In JSON, `counts` contiene `corrected` (i testi modificati) e `removed` (i sottotitoli
rimossi), e `changes` i sottotitoli toccati. Con `-t`, la chiave `alignment` indica come è stata
disposta la traduzione.
`settings` indica il file delle impostazioni che è stato letto, oppure è `null` se non ne è stato letto nessuno.

## Codici di uscita

Quelli dello strumento: `0` se ogni file è stato scritto, `2` se nessuno, `3` se solo
alcuni, `1` per un errore di utilizzo.

## Errori

| Che cosa lo provoca | Che cosa viene scritto, sull'output degli errori |
| :------------------ | :--------------------------------------- |
| un file indicato da `--config` che non esiste — il file dell'utente, mancante, dà i valori predefiniti | `settings file not found: <FILE>` |
| `--tasks` mancante | `--tasks is required`, seguito da un rimando a `--help` |
| un'operazione sconosciuta | `--tasks: "<name>" is not a task: expected mentions, join-words, split-words, common-errors, capitalization or line-break` |
| `--code` mancante | `--code is required by the tasks that read patterns: <tasks>` |
| `--code` senza un'operazione a modelli | `--code is for the tasks that read patterns, and none was asked for` |
| `--language` mancante con `join-words` o `split-words` | `--language is required by the tasks that check words: <tasks>` |
| `--language` che non è un codice di lingua | `--language: "<value>" is not a language: expected a locale code, like fr, en_US or sr@Latn` |
| `--language` senza una di quelle due operazioni | `--language is for the tasks join-words and split-words, which were not asked for` |
| una lingua senza dizionario | `no dictionary for <language>` |
| un codice scritto male | `--code: "<code>" is not a pattern code: expected Script[-language[-COUNTRY]], like Zyyy, Latn or Latn-en-US` |
| `--max-length` mancante con `line-break` | `--max-length is required by the task line-break: Gaupol's 24 is a width in ems, and has no value in characters` |
| `--max-length` che non è un numero maggiore di zero | `--max-length: "<value>" is not a length: expected a number of characters greater than zero` |
| `--max-lines`, `--skip-lines` che non sono un numero intero di righe | `--max-lines: "<value>" is not a number of lines: expected a whole number greater than zero` |
| `--skip-length` o `--skip-lines` illeggibili | `--skip-length: "<value>" is not a bound: expected a number greater than zero, or off` |
| una delle quattro opzioni senza `line-break` | `--max-length is for the task line-break, which was not asked for` |
| una classe sconosciuta | `--classes: "<name>" is not a class: expected human, ocr or human,ocr` |
| `--keep-blank-subtitles` con `--remove-blank-subtitles` | `--keep-blank-subtitles and --remove-blank-subtitles say opposite things; give one of them` |
| il nome di un modello errato | vedi la tabella qui sopra |
| un'operazione senza un modello attivo | `<task>: no pattern is active under the code <code>; switch one on with --enable` |
| `--range`, `-t`, `--document`, la destinazione | quelli di [Invocazione](invocation.md#errori) e di [Una traduzione](invocation.md#una-traduzione) |
