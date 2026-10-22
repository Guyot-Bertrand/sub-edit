# Invocazione

<!-- machine-translated -->
> Questa pagina è una traduzione automatica e non è ancora stata rivista da un lettore della lingua.

```
subedit-cli [global options] <subcommand> [options] <file>...
```

Senza un sottocomando, lo strumento scrive la propria guida e si ferma.

<!-- example: subedit-cli -->
```console
$ subedit-cli
Read, inspect and retime subtitle files.
Usage: subedit-cli [OPTIONS] [SUBCOMMAND]

Options:
  -h,--help                   Print this help message and exit
  --version                   Display program version information and exit
  -v                          Say more: -v is the default, -vv details, -vvv debugs
  -q,--quiet                  Say nothing but errors
  --encoding NAME             Encoding to read the files in; detected by default
  --format text|json          Form of the result on standard output: text or json
  --config FILE               Read the operation settings from FILE instead of the user's settings file
  --no-config Excludes: --config
                              Ignore any settings file: use the built-in defaults
  --list-encodings            Write the encodings that can be read and written, one a line, and stop

Subcommands:
  inspect                     Report what a subtitle file is made of
  convert                     Write a subtitle file out in another format or shape
  shift                       Move every position of a file by a fixed amount
  transform                   Correct every position from two points known to be right
  framerate                   Re-time a file mastered at one frame rate for another
  snap                        Move every position onto the nearest frame of a frame rate (see framerate)
  hearing-impaired            Remove the sounds described between square brackets
  adjust                      Bring the duration of every subtitle within a reading speed and limits
  replace                     Replace a text in the subtitles, without breaking a tag
  case                        Put the texts in title, sentence, upper or lower case, tags intact
  italics                     Put the texts in italics, or take their italics out
  dialogue-dashes             Put dialogue dashes on the lines, or take them off
  sort                        Put the subtitles in the order of their start, keeping ties as they are
  join-repeats                Join the subtitles that repeat each other, one after the other, into one
  disjoint                    Cut overlapping subtitles so that no range overlaps another, merging what overlaps
  repair                      Join the repeated subtitles, put them in order, then make the ranges disjoint
  correct                     Correct the texts with the patterns of the correction assistant
  append                      Put subtitle files one after another into a single file
  split-file                  Cut a subtitle file in two, a head and a tail
  pair                        Write a translation at the positions of its main file, and say how it lined up
  config                      List, show and change the operation settings
```

## Lingua

La riga di comando parla la lingua del proprio ambiente: **la sua guida, il suo resoconto e i suoi
errori** seguono la lingua richiesta. Gli esempi di questo manuale sono eseguiti in inglese — i
blocchi sono generati da `make manual`, mai copiati a mano.

**Come viene scelta la lingua.** La prima tra `LC_ALL`, `LC_MESSAGES` e `LANG` che sia impostata e
non vuota è il locale; `LANGUAGE`, quando è impostata, elenca poi le lingue desiderate, separate da
due punti (`LANGUAGE=pt_BR:fr`), e viene letta per prima. È l'ordine che segue gettext, letto dal
programma stesso: non occorre che il locale sia stato generato sulla macchina.

| Regola | Effetto |
| :--- | :----- |
| codifica e modificatore | ignorati: `fr_FR.UTF-8@euro` cerca `fr_FR`, poi `fr` |
| `LC_ALL=C`, locale `C` o `POSIX`, o nessun locale | inglese, e `LANGUAGE` non viene letta |
| una voce che nomina l'inglese (`en`, `en_GB`) | inglese: ciò che la segue in `LANGUAGE` non viene letto |
| una lingua il cui catalogo non è installato | la successiva richiesta, e l'inglese dopo l'ultima, **senza una parola** |
| un messaggio che il catalogo non traduce | mostrato in inglese |

**Il nome di un modello di correzione** che `correct` dice di non aver potuto applicare segue
anch'esso la lingua, quando il catalogo lo conosce — i modelli forniti con il programma; un modello
proprio è nominato come lo scrive il suo file. I file dei modelli non vengono mai riscritti.

I cataloghi sono installati in `<prefix>/share/subedit/locale/<language>/LC_MESSAGES/subedit.mo`
e cercati a partire dalla directory dell'eseguibile stesso. L'inglese è integrato. Questa versione installa `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR`, `ru`, `sr`, `tr` e `zh_CN` — catalano,
ceco, tedesco, spagnolo, finlandese, francese, galiziano, ebraico, ungherese, interlingue, islandese, italiano, olandese, polacco, portoghese, portoghese brasiliano, russo, serbo, turco e cinese semplificato — tradotte per intero. Il francese è stato rivisto
da un lettore della lingua; catalano, ceco, tedesco, spagnolo, finlandese, galiziano, ebraico, ungherese, interlingue, islandese, italiano, olandese, polacco, portoghese, portoghese brasiliano, russo, serbo, turco e cinese semplificato sono
tradotti automaticamente e non ancora rivisti da un lettore della lingua.
<!-- machine-translated: ca -->
<!-- machine-translated: cs -->
<!-- machine-translated: de -->
<!-- machine-translated: es -->
<!-- machine-translated: fi -->
<!-- machine-translated: gl -->
<!-- machine-translated: he -->
<!-- machine-translated: hu -->
<!-- machine-translated: ie -->
<!-- machine-translated: is -->
<!-- machine-translated: it_IT -->
<!-- machine-translated: nl -->
<!-- machine-translated: pl -->
<!-- machine-translated: pt -->
<!-- machine-translated: pt_BR -->
<!-- machine-translated: ru -->
<!-- machine-translated: sr -->
<!-- machine-translated: tr -->
<!-- machine-translated: zh_CN -->
Con `LANG=fr_FR.UTF-8`, `subedit-cli --help` comincia con "Lire,
inspecter et recaler des fichiers de sous-titres." Uno script che legge le parole dell'output
testuale imposta `LANGUAGE=en` (o `LC_ALL=C`) — oppure legge `--format json`, che non cambia mai.

**Ciò che non segue mai la lingua** — ciò che legge uno script:

| Mai tradotto | Perché |
| :--------------- | :-- |
| l'output di `--format json`: chiavi, valori, e il `message` e il `detail` di errori e avvisi | gli stessi byte in ogni lingua; le parole sono inglesi |
| i codici di uscita | i quattro di [Codici di uscita](#codici-di-uscita), in ogni lingua |
| i nomi digitati: sottocomandi, opzioni, i loro valori (`off`, `srt`), metavariabili (`FILE`, `TEXT`, `OPTIONS`) | una riga di comando scritta una volta funziona in ogni lingua |
| i nomi dei modelli di correzione che `correct --enable` e `--disable` accettano | un modello si chiama con il nome inglese che il suo file contiene |
| i nomi dei formati e delle codifiche | un formato si chiama con il suo nome |
| le diagnostiche proprie di CLI11 — un argomento obbligatorio mancante, un'opzione sconosciuta, un valore che un'opzione non accetta — e il suo `Run with --help for more information.` | formulate all'interno dell'analizzatore su cui è costruita la riga di comando; restano in inglese |

Chiedere l'inglese, qualunque sia l'ambiente:

<!-- example: LC_ALL=C LANGUAGE=fr subedit-cli shift --output-dir out a.srt; echo "code=$?" -->
```console
$ LC_ALL=C LANGUAGE=fr subedit-cli shift --output-dir out a.srt; echo "code=$?"
shift needs --by, or --to-grid to work the amount out from the positions
code=1
```

## Opzioni globali

**Si scrivono prima del nome del sottocomando, e solo lì.**
`subedit-cli --encoding cp1252 inspect film.srt` è la forma corretta;
`subedit-cli inspect --encoding cp1252 film.srt` viene rifiutato, con
`The following argument was not expected: --encoding`. Lo stesso vale per
`--version`, `-v` e `--quiet`.

**`--help` è l'eccezione, e non lo è davvero**:
`subedit-cli inspect --help` è accettato perché ogni sottocomando ha la propria, ed è la
guida del sottocomando che scrive, non quella dello strumento.

| Opzione | Effetto |
| :----- | :---- |
| `-h`, `--help` | scrive la guida e si ferma; quella del sottocomando se ne è nominato uno |
| `--version` | scrive `subedit <version>`, le librerie con cui lo strumento gira e le lingue installate, e si ferma — vedere [`--version`](#--version) |
| `-q`, `--quiet` | livello 0 — nessun resoconto |
| `-v`, `-vv`, `-vvv` | livelli da 1 a 3; il livello 1 è quello predefinito |
| `--encoding NAME` | legge i file in questa codifica, invece di indovinarla |
| `--list-encodings` | scrive le codifiche che lo strumento può leggere e scrivere, una per riga, e **si ferma** (codice `0`) — vedere [`--list-encodings`](#--list-encodings) |
| `--format text\|json` | la forma del risultato sull'output standard: il testo, oppure un oggetto JSON per file — vedere [Un output che uno script può leggere](#un-output-che-uno-script-può-leggere) |
| `--config FILE` | legge le impostazioni delle operazioni da `FILE` invece che dal file delle impostazioni dell'utente — vedere [Il file delle impostazioni](#il-file-delle-impostazioni) |
| `--no-config` | non legge alcun file di impostazioni: ogni impostazione di operazione non data sulla riga assume il suo valore predefinito integrato — vedere [Il file delle impostazioni](#il-file-delle-impostazioni) |

`--quiet` e `-v` nella stessa invocazione vengono rifiutati: due intenzioni opposte
non si risolvono a favore dell'ultima scritta.

Vengono rifiutati anche `--config` e `--no-config` nella stessa invocazione, con
`--config excludes --no-config`, e così `--config` con un nome vuoto, con
`--config needs a file name`: un nome vuoto non designa alcun file, e il file dell'utente non viene letto
al suo posto.

### Il file delle impostazioni

**[`adjust`](adjust.md) e [`correct`](correct.md) leggono per impostazione predefinita le impostazioni delle operazioni del
file delle impostazioni** — i valori che `Adjust Durations…` e `Correct Texts…` ricordano nella
finestra, o che [`config set`](config.md) ha scritto. Nessun altro sottocomando lo legge. Il file è:

```
$XDG_CONFIG_HOME/subedit/settings.conf
```

oppure `~/.config/subedit/settings.conf` quando `XDG_CONFIG_HOME` non è impostata, è vuota o non è un percorso
assoluto — il file della finestra, descritto nel [suo manuale](../subedit-gui/preferences.md#il-file).

**Priorità, dalla più forte: l'opzione sulla riga di comando, poi il file, poi il valore
predefinito integrato.** Un'opzione vince sempre: `--speed off` disattiva una velocità fissata dal file. Viene letto solo il
**blocco delle operazioni** del file; le preferenze di visualizzazione (geometria, colonne, tema…)
non lo sono mai. [`config list`](config.md#config-list) nomina le chiavi, e quali di esse la riga
di comando legge: le nove chiavi `duration-adjust.*`, `correction.human`, `correction.ocr`,
`correction.remove-blank` e `correction.line-break.max-lines`. **Ciascuna ha un'opzione
che la sostituisce per una sola invocazione**, comunque il file la imposti — le tabelle di
[`adjust`](adjust.md#che-cosa-fornisce-il-file-delle-impostazioni) e
[`correct`](correct.md#che-cosa-viene-scelto-e-che-cosa-no) le nominano.

**Leggere il file dell'utente non fa mai fallire un comando.** Un file mancante dà i valori predefiniti; un file
che non si può leggere dà i valori predefiniti e lo dice; un valore che non si può leggere mantiene il suo valore predefinito ed è nominato; una
chiave sconosciuta è ignorata. Il file non viene mai scritto da `adjust` né da `correct`.

**Ciò che è stato letto viene detto**, al livello 1 predefinito, sull'errore standard, prima del primo file:

| Caso | Riga |
| :--- | :--- |
| il file è stato letto | `settings: read <path>` |
| un valore al suo interno non si è potuto leggere | `settings: <key>: cannot read '<value>', default kept` |
| il file dell'utente non esiste | `settings: <path> not found, defaults used` |
| il file non si può leggere | `settings: <path> unreadable, defaults used` |
| `--no-config` | nulla |

Con [`--format json`](#un-output-che-uno-script-può-leggere), l'oggetto di ciascun file di `adjust` e
`correct` contiene `settings`: il percorso letto, oppure `null` quando non ne è stato letto nessuno.


**`--no-config` rende riproducibile un'esecuzione**: la stessa riga dà lo stesso risultato su ogni
macchina, qualunque cosa l'utente abbia impostato nella finestra. Usarlo negli script, nei test e nelle segnalazioni di bug — oppure
`--config FILE` con un file che li accompagni.

**`--config FILE` nomina un altro file**, letto allo stesso modo — **tranne che deve esistere**. Un
nome dato sulla riga di comando non è una richiesta dei valori predefiniti: quando `FILE` non esiste,
`adjust` e `correct` si fermano prima di leggere qualsiasi input, scrivono `settings file not found: <FILE>` sull'
errore standard, e escono con il codice d'uso `1`. Un `FILE` che esiste e non si può leggere
dà i valori predefiniti e lo dice, come fa il file dell'utente. [`config show`](config.md#config-show)
mostra ancora un `FILE` mancante come `(not found)`, e [`config set`](config.md#config-set-e-config-unset) lo
crea ancora.

<!-- example: printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine.conf adjust --dry-run a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine.conf adjust --dry-run a.srt; echo "code=$?"
settings file not found: mine.conf
code=1
```

### `--version`

**La versione del programma, da sola sulla prima riga** — `subedit <version>`, ciò che era l'output fino alla
0.15.38 e ciò che legge uno script. Poi **una riga per ogni libreria con cui lo strumento gira**, poi le lingue
dell'interfaccia installate, sull'output standard; lo strumento si ferma, con codice `0`. Un sottocomando dato insieme
non viene eseguito, e `-q` e `-v` non cambiano nulla.

<!-- example: subedit-cli --version | head -n 1 -->
```console
$ subedit-cli --version | head -n 1
subedit 1.0.5
```

L'insieme, su una macchina con ICU 74.2 ed Enchant 2.3.3 — i numeri sono quelli della macchina:

```console
$ subedit-cli --version
subedit <version>
  ICU      74.2 (built with 74.2)
  Enchant  2.3.3 (built with 2.3.3)
  CLI11    2.4.1 (built with 2.4.1)
  Languages: en, fr
```

| Libreria | A che cosa serve allo strumento |
| :------ | :------------------------ |
| `ICU` | leggere e scrivere le codifiche, rilevarle, cambiare le maiuscole e minuscole delle lettere |
| `Enchant` | il correttore ortografico di `correct` |
| `CLI11` | leggere questa riga di comando |

- **Ogni riga dà due versioni**: quella trovata in fase di esecuzione, per prima, e quella con cui il programma è stato
  compilato, tra parentesi. Differiscono quando la libreria della macchina è stata aggiornata dopo la
  compilazione del programma — la prima cosa da fornire in una segnalazione di bug.
- **Le intestazioni di Enchant non portano alcuna versione**: quella tra parentesi è quella che `pkg-config` ha riportato
  quando la compilazione è stata configurata.
- **CLI11 è compilata nel programma**, essendo le sue intestazioni tutto ciò che ne esiste: le sue due versioni sono
  sempre le stesse.
- **Né Qt né libmpv**: la riga di comando non si collega a nessuna delle due, ed elenca solo ciò con cui gira.
  La finestra le elenca — vedere il [suo manuale](../subedit-gui/invocation.md#--version).
- **`Languages`** elenca `en`, la lingua sorgente, che non richiede alcun catalogo, poi ogni catalogo
  installato in `<prefix>/share/subedit/locale` che si legge, per codice — le lingue che
  `Edit ▸ Preferences…` propone.
- **Ciò che segue la lingua**: `built with` e `Languages`. La prima riga, i nomi, i
  numeri e i codici delle lingue non la seguono mai.
- **Con `--format json`**, prima o dopo `--version`, **un oggetto su una riga**: `schema`
  (`1`), `command` (`"version"`), `version`, `libraries` — un array di
  `{"name":"ICU","loaded":"74.2","built":"74.2"}`, nell'ordine del testo — e `languages`, un
  array di codici. Nulla vi è tradotto.

<!-- example: subedit-cli --version --format json | grep -o '"name":"[^"]*"' -->
```console
$ subedit-cli --version --format json | grep -o '"name":"[^"]*"'
"name":"ICU"
"name":"Enchant"
"name":"CLI11"
```

### `--list-encodings`

**Le codifiche che lo strumento può leggere e scrivere, una per riga, ordinate per nome**, sull'output standard —
poi lo strumento si ferma, con codice `0`, come fa `--version`: è la risposta, non una tappa. Un
sottocomando dato insieme non viene eseguito.

<!-- example: subedit-cli --list-encodings | grep '^UTF' | head -4 -->
```console
$ subedit-cli --list-encodings | grep '^UTF' | head -4
UTF-16BE
UTF-16LE
UTF-32BE
UTF-32LE
```

- **Sono i nomi che `--encoding` accetta**, quelli canonici; anche i loro alias (`cp1252`) sono accettati,
  senza essere elencati.
- **Il numero dipende dall'ICU installata**, e nessun nome è promesso oltre a quelli che ogni ICU ha
  (`UTF-8`, `UTF-16LE`, `windows-1252`…). Il manuale non dà quindi alcun numero.
- **`UTF-16` e `UTF-32` non ci sono**: ICU li conosce, ma il loro convertitore scrive un indicatore proprio,
  e il modello li rifiuta — vedere più avanti. Un elenco da cui scegliere non offre ciò che il passo
  successivo rifiuterebbe.
- **Con `--format json`**, **un oggetto per codifica**:
  `{"schema":1,"command":"list-encodings","name":"UTF-8"}`. Non è il rapporto di un file: questi
  oggetti **non contengono né `file` né `ok`**, l'unica eccezione alla struttura descritta in
  [La forma](#la-forma). Sono comunque un oggetto per riga, uno per voce.
- **Il livello del resoconto non cambia nulla**: un elenco è un risultato, e `-q` non lo toglie.

### `--encoding`, e ciò che non può forzare

**Si applica a ogni sottocomando**, perché tutti leggono: un file la cui
codifica è indovinata male resta sbagliato qualunque cosa gli si faccia, e un'opzione
portata solo da `inspect` lascerebbe una discordanza senza rimedio.

**Il nome è quello di ICU, e tutti i suoi alias sono accettati**: `cp1252`,
`windows-1252` e `WINDOWS 1252` designano la stessa codifica. L'insieme non è scritto
da nessuna parte: è quello che l'ICU della macchina può convertire, e
[`--list-encodings`](#--list-encodings) lo elenca. Un nome che non può convertire viene
rifiutato prima che venga letto un solo file:

```console
$ subedit-cli --encoding klingon-1 inspect film.srt
no encoding is named "klingon-1"
```

**Un'eccezione, e una sola: la codifica deve nominare il proprio ordine dei byte.**
`UTF-16` e `UTF-32` sono nomi che ICU conosce e il cui convertitore scrive **un indicatore
proprio**. Una codifica simile non può vivere in questo modello — l'indicatore smette
di essere una scelta, `--no-bom` sarebbe accettato e disobbedito — quindi entrambe vengono
rifiutate, in lettura come in scrittura:

```console
$ subedit-cli --encoding UTF-16 inspect film.srt
"UTF-16" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do
```

`UTF-16LE` e `UTF-16BE` leggono e scrivono gli stessi byte, indicatore compreso; ciò che
il nome preciso aggiunge è che si sa quale.

**Un indicatore dell'ordine dei byte prevale sull'opzione.** È l'unica cosa che un file di
sottotitoli dichiara di sé, e leggerlo diversamente da come si dichiara
significherebbe obbedire al chiamante contro il file. La discordanza viene **detta**, come
messaggio diagnostico:

```console
$ subedit-cli -vvv --encoding cp1252 inspect avec-bom.srt
avec-bom.srt: a byte order mark that contradicts the encoding asked for ("UTF-16LE"), settled by the reader
```

È ciò che fa Gaupol — ricomincia la lettura con la codifica
dell'indicatore — ma senza dirlo; qui la distanza tra ciò che è stato chiesto e
ciò che è stato fatto non è un silenzio.

## Sottocomandi

| Sottocomando | Che cosa fa | Scrive? |
| :------------ | :-------------- | :----------- |
| [`inspect`](inspect.md) | riferisce di che cosa è fatto un file, senza cambiare nulla | no |
| [`convert`](convert.md) | scrive un file in un altro formato, o in un'altra forma | sì |
| [`shift`](shift.md) | sposta ogni posizione della stessa quantità | sì |
| [`transform`](transform.md) | corregge ogni posizione a partire da due punti noti per essere esatti | sì |
| [`framerate`](framerate.md) | risincronizza un file da una frequenza dei fotogrammi a un'altra | sì |
| [`snap`](snap.md) | sposta ogni marcatore temporale sul fotogramma più vicino | sì |
| [`hearing-impaired`](hearing-impaired.md) | rimuove le indicazioni per non udenti | sì |
| [`adjust`](adjust.md) | regola le durate secondo una velocità di lettura e dei limiti | sì |
| [`replace`](replace.md) | sostituisce un testo nei sottotitoli, senza rompere un tag | sì |
| [`case`](case.md) | mette i testi in maiuscole iniziali, prima lettera maiuscola, maiuscolo o minuscolo | sì |
| [`italics`](italics.md) | mette i testi in corsivo, o ne toglie il corsivo | sì |
| [`dialogue-dashes`](dialogue-dashes.md) | mette o toglie i trattini di dialogo | sì |
| [`sort`](sort.md) | rimette i sottotitoli nell'ordine del loro inizio | sì |
| [`join-repeats`](join-repeats.md) | fa un unico sottotitolo dei pezzi di uno stesso testo che si susseguono | sì |
| [`disjoint`](disjoint.md) | taglia i sottotitoli sovrapposti in modo che nessun intervallo ne sovrapponga più un altro | sì |
| [`repair`](repair.md) | concatena `join-repeats`, `sort` e `disjoint` | sì |
| [`correct`](correct.md) | corregge i testi con i modelli di Gaupol: indicazioni, unione e divisione di parole, errori comuni, uso delle maiuscole, a capo | sì |
| [`append`](append.md) | accoda i file al primo, in **un unico** file | sì |
| [`split-file`](split-file.md) | taglia un file in **due**: una testa e una coda | sì |
| [`pair`](pair.md) | scrive una traduzione alle posizioni del suo file principale, e dice come le sue righe si sono allineate | sì |
| [`config`](config.md) | elenca, mostra e cambia le impostazioni delle operazioni del file delle impostazioni | solo il file delle impostazioni |

Ci sono tutti e ventuno; la guida dello strumento li elenca nello stesso ordine.

**Solo uno non tocca nulla**, ed è la colonna di destra: `inspect` legge e
riferisce. Gli altri diciannove scrivono, e ciò che segue vale per diciassette di essi:
[`append`](append.md) e [`split-file`](split-file.md) hanno ciascuno una propria arità — N file per
**un** risultato, un file per **due** — e quindi una propria destinazione, che descrivono nella loro pagina.
[`config`](config.md) non scrive alcun file di sottotitoli: legge e cambia il file delle impostazioni, e
nulla di ciò che segue si applica a esso.

## La destinazione

I diciassette sottocomandi che scrivono — [`convert`](convert.md),
[`shift`](shift.md), [`transform`](transform.md), [`framerate`](framerate.md),
[`snap`](snap.md), [`hearing-impaired`](hearing-impaired.md), [`adjust`](adjust.md),
[`replace`](replace.md), [`case`](case.md), [`italics`](italics.md),
[`dialogue-dashes`](dialogue-dashes.md), [`sort`](sort.md), [`join-repeats`](join-repeats.md),
[`disjoint`](disjoint.md), [`repair`](repair.md), [`correct`](correct.md), [`pair`](pair.md) — ricevono la destinazione allo stesso modo. ([`append`](append.md) e
[`split-file`](split-file.md) hanno la propria, nella loro pagina.) [`inspect`](inspect.md) non scrive alcun file e
non accetta nessuna di queste opzioni.

**Una destinazione che esiste già viene sovrascritta**, senza domande e senza un'opzione per
evitarlo: la scrittura è atomica — il file non è mai scritto a metà, e una
scrittura interrotta lascia intatto il vecchio —, ma il contenuto precedente va
perso. Non è un difetto tenuto lì: è la regola, e rieseguire un comando
sulla stessa directory di output lo riesegue sopra se stesso.

**Nulla viene mai scritto senza una destinazione esplicita.** I tre modi di
indicarla si escludono a vicenda, e ne serve uno: senza, nulla viene scritto e il codice
di uscita è `1` — tranne con [`--dry-run`](#vedere-prima-di-scrivere), che non scrive nulla
e quindi non ne richiede.

| Opzione | Dove va il risultato |
| :----- | :---------------- |
| `--output FILE` | in questo file, con questo nome esatto; un solo input |
| `--output-dir DIR` | in questa directory, con il nome di base dell'input |
| `--in-place` | sopra l'input, tramite scrittura atomica |

Uno strumento che sovrascrive il proprio input perché non gli è stato detto nulla è uno strumento che si
smette di usare la seconda volta. Il rifiuto costa una riga e salva il
file.

`--output` con più input viene rifiutato: l'ultimo file verrebbe scritto sopra
i precedenti. Con un lotto, `--output-dir` è l'unica delle tre che abbia
senso, insieme a `--in-place`.

**La directory di output viene creata** se manca, genitori compresi — per
`--output-dir`, e per la directory in cui scrive `--output`. Ciò avviene una volta che la
riga di comando è convalidata e **prima del primo file**. Se non si può,
viene detto una sola volta, nessun file viene letto, e il codice di uscita è
`2`.

**Il lotto viene giudicato nel suo insieme prima di scrivere.** La destinazione di ciascun input
viene calcolata per prima, e ne derivano due rifiuti — errori d'uso, codice
`1`, **nessun file scritto** e nessuna directory creata:

- **due input che finirebbero nello stesso file** — `a/film.srt` e
  `b/film.srt` con `--output-dir`, oppure `a/film.srt` e `b/film.vtt` convertiti entrambi
  in WebVTT, la cui destinazione finale ha cambiato estensione. Il
  messaggio nomina la destinazione e i due input; rinominare il secondo o
  lasciar vincere l'ultimo farebbe perdere un file senza dirlo, e rifiutare
  costa una riga;
- **una destinazione che è uno degli input**, senza `--in-place` — anche
  tramite `--output-dir .`, tramite `--output` o tramite un collegamento simbolico. Scrivere sopra
  il proprio input è un gesto che si nomina.

Due percorsi designano lo stesso file quando lo dice il sistema, non quando sono
scritti allo stesso modo: `in/../in/film.srt` e `in/film.srt` sono un unico file.

**L'estensione segue il formato scritto.** Cambia solo per
[`convert`](convert.md), l'unico sottocomando che cambia formato; gli
altri mantengono il formato del file letto, e quindi la sua estensione.

| Che cosa lo provoca | Messaggio |
| :------------------ | :------ |
| nessuna destinazione, senza `--dry-run` | `no destination given: use --output, --output-dir or --in-place` |
| due destinazioni | `--output, --output-dir and --in-place exclude one another` |
| `--output` su un lotto | `--output names one file but several were given: use --output-dir instead` |
| due input, una destinazione | `<destination>: would be written by both <input> and <input>` |
| un input verrebbe sovrascritto | `<destination>: would be written over the input <input>` — la stessa frase per [`append`](append.md) e [`split-file`](split-file.md) |
| directory di output che non si può creare | `<directory>: cannot be created: permission denied`, oppure `: cannot be created` |
| destinazione che non si può scrivere | `<path>: <destination>: cannot be written: permission denied`, oppure `: cannot be written` |
| carattere assente dalla codifica scritta | `<path>: holds a character the chosen encoding cannot write` |

**L'ultima merita una frase.** Un file viene riscritto nella **codifica in cui è stato
letto**, così come con i suoi terminatori di riga e il suo indicatore. Se una
modifica vi ha introdotto un carattere che questa codifica non conosce — una
`ł` in Latin-1 —, la scrittura fallisce e **nulla viene scritto**. Sostituirlo
con un `?` significherebbe perdere del testo sotto gli occhi di chi l'ha appena scritto.

## Vedere prima di scrivere

**`--dry-run`** è accettato da ciascuno dei sedici sottocomandi che scrivono, e significa la stessa
cosa ovunque: **leggere, calcolare, riferire, non scrivere alcun file**. Lo si aggiunge alla
riga che si stava per eseguire; applicare significa rieseguire la stessa riga senza di esso. Il calcolo è
deterministico: ciò che un `--dry-run` ha mostrato è ciò che l'esecuzione successiva scriverà, finché il
file non è cambiato.

- **Non scrive alcun file e non crea alcuna directory**, `--in-place` compreso: il file di
  input resta com'era.
- **Non richiede alcuna destinazione.** Se ne può dare una, e **viene allora verificata come senza
  `--dry-run`**: due input per la stessa destinazione, un input che verrebbe sovrascritto,
  `--output` su un lotto, due modi di dire dove — una riga di comando rifiutata senza `--dry-run`
  è rifiutata anche con esso, con la stessa frase e lo stesso codice `1`.
- **Il codice di uscita è quello di un'esecuzione reale**: `0` quando tutto è stato calcolato, `2` o `3`
  quando dei file non si sono potuti leggere o elaborare, `1` per l'uso. Un carattere che
  la codifica del file non poteva scrivere fa fallire una prova come fa fallire un'esecuzione reale.
  **Che esistano modifiche non cambia il codice**: i quattro codici dicono se lo strumento
  è riuscito, non se il file è conforme.
- **Il resoconto lo dice.** Dove un'esecuzione reale scrive `-> <destination>`, una prova
  scrive `(dry run, nothing written)`.

<!-- example: printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by 1 --dry-run a.srt; ls -- *.srt -->
```console
$ printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by 1 --dry-run a.srt; ls -- *.srt
a.srt: 1 subtitle shifted by 1.000 s (dry run, nothing written)
a.srt
```

**Su un sottocomando che cambia i testi** —
[`hearing-impaired`](hearing-impaired.md), [`replace`](replace.md), [`case`](case.md),
[`italics`](italics.md), [`dialogue-dashes`](dialogue-dashes.md) e [`correct`](correct.md) —, **l'output standard porta le modifiche
proposte**. Un blocco per sottotitolo modificato, nell'ordine del file:

```
<path>: subtitle <number>
- <the text before, line by line>
+ <the text after, line by line>
```

- il **numero** è quello che il sottotitolo ha nel file letto, contato a partire da uno;
- ogni riga del testo prima porta `- ` davanti, ogni riga del testo dopo `+ `: un testo
  di più righe si legge com'è, e una riga che inizia essa stessa con un trattino di
  dialogo dà `- - `;
- **un sottotitolo che l'operazione rimuoverebbe** lo dice sulla prima riga,
  `<path>: subtitle <number> (removed)`, e non ha righe `+`;
- un sottotitolo della traduzione, con `--document translation`, si annuncia `(translation)` nello stesso
  punto — vedere [Una traduzione](#una-traduzione);
- la prima riga di ogni blocco inizia con il percorso, in modo che `grep '^film.srt: '`
  trovi i blocchi di un file in un lotto;
- **nessun blocco** quando nulla cambierebbe.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\n- [Grincement]\n- Qui est là ?\n\n' > mentions.srt; subedit-cli hearing-impaired --dry-run mentions.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\n- [Grincement]\n- Qui est là ?\n\n' > mentions.srt; subedit-cli hearing-impaired --dry-run mentions.srt
mentions.srt: 2 subtitles cleaned, 1 removed (dry run, nothing written)
mentions.srt: subtitle 1 (removed)
- [Bruit de pas]
mentions.srt: subtitle 2
- Attends [il tousse] Marie.
+ Attends Marie.
mentions.srt: subtitle 3
- - [Grincement]
- - Qui est là ?
+ Qui est là ?
```

**L'elenco è lo stesso in JSON**: l'oggetto del file contiene un array `changes`, vedere
[La forma](#la-forma), con `after` impostato a `null` per un sottotitolo rimosso. **Viene calcolato
solo quando lo chiedono `--dry-run` o `--format json`**: un lotto di migliaia di
sottotitoli non lo costruisce per niente.

Sugli **altri sottocomandi** — [`convert`](convert.md), [`shift`](shift.md),
[`transform`](transform.md), [`framerate`](framerate.md), [`snap`](snap.md),
[`adjust`](adjust.md), [`sort`](sort.md), [`append`](append.md), [`split-file`](split-file.md), [`pair`](pair.md) —, l'output
standard porta ciò che porterebbe quello di un'esecuzione reale, cioè nulla in forma testuale; in
JSON, l'oggetto di ciascun file con `"dry_run":true`, `"destination":null` e gli stessi
`counts`.

## Due output, due ruoli

| Output | Che cosa porta |
| :----- | :--------------- |
| standard | **il risultato, e soltanto il risultato** — il rapporto di `inspect`, le modifiche proposte da un testo [`--dry-run`](#vedere-prima-di-scrivere); con `--format json`, un oggetto per file — uno per esecuzione per [`append`](append.md), [`split-file`](split-file.md) e [`pair`](pair.md) |
| errore | **tutto il resto** — il resoconto, gli avvisi, gli errori |

È questa separazione che permette di reindirizzare il risultato senza che ci finisca la cronaca:

```console
$ subedit-cli inspect *.srt > rapport.txt
```

Il rapporto va nel file, il resoconto resta sullo schermo.

## Un output che uno script può leggere

**`--format json`** sostituisce il testo dell'output standard con **un oggetto JSON per
file di input**, nel formato JSON Lines: un oggetto compatto per riga, in UTF-8 senza
indicatore, il terminatore di riga è `\n`. `text` è il valore predefinito e non cambia. È
un'opzione globale, e si scrive quindi prima del sottocomando; un valore diverso da
`text` e `json` è un errore d'uso, codice `1`.

<!-- example: printf '1\n00:00:10,000 --> 00:00:12,000\nUn.\n\n' > a.srt; subedit-cli --format json shift --by 1 --output-dir sortie a.srt absent.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:10,000 --> 00:00:12,000\nUn.\n\n' > a.srt; subedit-cli --format json shift --by 1 --output-dir sortie a.srt absent.srt; echo "code=$?"
a.srt: 1 subtitle shifted by 1.000 s -> sortie/a.srt
{"schema":1,"command":"shift","file":"a.srt","ok":true,"dry_run":false,"destination":"sortie/a.srt","counts":{"subtitles":1,"shifted_by_ms":1000},"warnings":[]}
absent.srt: does not exist
{"schema":1,"command":"shift","file":"absent.srt","ok":false,"error":{"kind":"not-found","message":"absent.srt: does not exist"}}
1 of 2 files shifted, 1 failed
code=3
```

**Un oggetto per input, mai zero, mai due**, che l'input sia riuscito o no, nell'ordine
in cui gli input sono stati dati — o trovati, con [`--recursive`](lots.md). Si
contano le righe, si trovano i propri file. **Tre sottocomandi sono l'eccezione, perché la loro
arità non è quella di un lotto: [`append`](append.md) (N input, un output), [`split-file`](split-file.md)
(un input, due output) e [`pair`](pair.md) (due file, un output) scrivono **un oggetto per l'intera
esecuzione**, che nomina il file indicato dalla loro pagina.** **I sottocomandi che riscrivono scrivono quindi anche
sull'output standard**, cosa che il loro contratto non faceva in forma testuale: il loro
file resta il loro risultato, e l'oggetto ne è il rapporto.

**Il resoconto non cambia.** `-q`, `-v`, `-vv` e `-vvv` agiscono sull'output di
errore, in forma testuale, come senza `--format`; **l'output standard non varia con essi**.
I messaggi diagnostici di lettura, che il testo tiene al livello 3, sono dati e sono
**sempre** in `warnings`. Un errore d'uso non scrive alcun oggetto: viene rilevato
prima che un file sia toccato, quindi non c'è nulla a cui associarlo; va sull'output di
errore con codice `1`, e l'output standard resta vuoto.

### La forma

| Chiave | Contenuto | Presente |
| :-- | :------ | :------- |
| `schema` | intero, la versione di questa forma — `1` | sempre |
| `command` | il nome del sottocomando | sempre |
| `file` | il percorso **come dato**, o come l'ha composto la visita di una directory | sempre |
| `ok` | booleano | sempre |
| `error` | `{"kind", "message"}` | se `ok` è falso |
| `dry_run` | booleano: vero per un [`--dry-run`](#vedere-prima-di-scrivere) | se `ok` è vero, su un sottocomando che scrive |
| `destination` | il percorso scritto, oppure `null` quando `dry_run` è vero: nulla è stato scritto | idem |
| `counts` | un oggetto di **interi**, specifico del sottocomando | idem |
| `changes` | un array di `{"subtitle", "document", "before", "after"}`: i testi modificati, vedere più avanti | su un sottocomando che cambia i testi |
| `warnings` | un array di `{"kind", "line"?, "detail"?, "settled"?}` | se `ok` è vero |
| `alignment` | come è stata sovrapposta una traduzione, vedere [Una traduzione](#una-traduzione) | con `-t`, e per `pair` |
| `constraints` | le impostazioni usate | `adjust` |
| `settings` | il file di impostazioni che è stato letto, oppure `null` quando non ne è stato letto nessuno — vedere [Il file delle impostazioni](#il-file-delle-impostazioni) | `adjust`, `correct`, se `ok` è vero |
| `inputs`, `tail` | un elemento per file accodato; il percorso della coda | `append`; `split-file` |

`line` è assente da una diagnostica che riguarda l'intero file; `detail` lo è quando non c'è
nulla da aggiungere; `settled` è `true` quando la lettura ha **risolto** qualcosa — una
numerazione rigenerata —, ed è assente quando ha lasciato le cose come stanno.

**`changes`** elenca, nell'ordine del file, ogni sottotitolo il cui testo cambia:
`subtitle` è il suo numero nel file letto, contato a partire da uno; `document` è `"main"`
oppure `"translation"`; `before` è il testo prima; `after` il testo dopo, oppure **`null` quando
il sottotitolo è rimosso**. Gli a capo di un testo vi sono `\n`. Viene
scritto per un'esecuzione reale come per un [`--dry-run`](#vedere-prima-di-scrivere), ed è **vuoto
(`[]`) quando nulla cambia**: la sua presenza dice che il sottocomando elenca le proprie modifiche,
la sua assenza che non ne ha.

**Nessun numero in virgola mobile, da nessuna parte.** Una posizione o una durata è un intero di
**millisecondi** (`…_ms`); un conteggio è un intero; una frequenza dei fotogrammi è una **stringa**
(`"25"`, `"24000/1001"`) perché è esatta, cosa che un razionale in virgola mobile non può
contenere; una concentrazione della griglia è un intero in **millesimi** — `1000` è una
griglia perfetta. Due output si confrontano allora byte per byte, senza arrotondamenti.

**Un percorso che non è UTF-8** — Linux lo consente — non può essere rappresentato in JSON: viene
scritto con U+FFFD al posto dei byte non validi, e l'oggetto contiene l'avviso
`path-not-utf8`. La fedeltà di un percorso simile non è promessa, e questo è detto.

### Che cosa ogni sottocomando mette in `counts`

| Sottocomando | Chiavi di `counts` |
| :------------ | :--------------- |
| [`shift`](shift.md) | `subtitles`, `shifted_by_ms` (con segno) |
| [`transform`](transform.md) | `subtitles` |
| [`framerate`](framerate.md) | `subtitles` |
| [`snap`](snap.md) | `subtitles`, `moved` (posizioni spostate), `furthest_ms` (lo spostamento maggiore) |
| [`hearing-impaired`](hearing-impaired.md) | `cleaned` (testi riscritti), `removed` (sottotitoli rimossi) |
| [`replace`](replace.md) | `replaced` (corrispondenze sostituite nei sottotitoli modificati), `matched` (corrispondenze trovate) |
| [`case`](case.md), [`italics`](italics.md), [`dialogue-dashes`](dialogue-dashes.md) | `changed` (sottotitoli il cui testo è cambiato) |
| [`correct`](correct.md) | `corrected` (testi cambiati), `removed` (sottotitoli rimossi) — mai le corrispondenze |
| [`sort`](sort.md) | `subtitles`, `moved` (i posti che hanno cambiato sottotitolo) |
| [`adjust`](adjust.md) | `subtitles` (i sottotitoli presi di mira), `adjusted` (quelli la cui fine si è spostata), poi il gruppo `sacrificed`: `speed`, `minimum`, `gap`; e, fuori da `counts`, `constraints` — le impostazioni usate |
| [`convert`](convert.md) | `subtitles`, poi ciò che la conversione ha perso: `lost_ends` e `lost_header` (0 o 1), `joined_lines`, `lost_tags`, `lost_fields`, `furthest_ms` |
| [`append`](append.md) | `files`, `subtitles`, `appended`, poi `lost_ends`, `lost_header`, `joined_lines`, `lost_tags`, `lost_fields`, `furthest_ms` sommati; e, fuori da `counts`, `inputs` — un elemento per file accodato |
| [`split-file`](split-file.md) | `subtitles`, `head`, `tail`; e, fuori da `counts`, `tail` — il percorso della coda (`destination` è la testa) |
| [`pair`](pair.md) | `subtitles` (quelli del file scritto); e, fuori da `counts`, `alignment` |
| [`inspect`](inspect.md) | nessun `counts`: la descrizione del file, vedere la sua pagina |

### Identificatori di errore

Un `error.kind` è uno di questi; **un lettore tratta un identificatore sconosciuto come
"altro"**.

| `kind` | Che cos'è |
| :----- | :----------- |
| `not-found`, `permission-denied`, `io` | il sistema ha rifiutato di leggere — o di scrivere — il file |
| `unknown-format`, `no-subtitle-found`, `undecodable` | il file è letto e non si lascia leggere |
| `unencodable` | un carattere che la codifica scelta non può scrivere |
| `no-byte-order-mark`, `no-frame-rate` | ciò che `convert` non può scrivere senza che gli venga detto |
| `before-the-origin`, `beyond-the-end`, `no-transform`, `no-grid` | un'operazione che non si può applicare a questo file |
| `range-out-of-bounds` | un `--range` che questo file non contiene |
| `no-style` | un formato che non scrive alcuno stile (TMPlayer, LRC), per `italics` |
| `refused` | qualsiasi altro rifiuto di un'operazione |

### Che cosa è promesso, e che cosa no

**Promesso finché `schema` vale `1`**: i nomi delle chiavi, il loro tipo e la loro unità; il significato
di `ok`, `file` e `destination`; **un oggetto per input, nell'ordine degli input** — uno per esecuzione per i tre sottocomandi con una propria arità;
gli identificatori `kind` che esistono; il determinismo — stessi input, stessi argomenti,
stessi byte.

**Non promesso**: il **testo dei messaggi** (`message`, `detail`), inglese per un essere
umano, che viene corretto quando serve — uno script che lo confronta è rotto in anticipo; l'**ordine delle
chiavi** in un oggetto, fisso ma non garantito da JSON; la presenza di **chiavi non ancora
aggiunte**.

**Un lettore ignora quindi una chiave sconosciuta**, e tratta un `kind` sconosciuto come "altro":
aggiungere una chiave o un identificatore non cambia `schema`. Rimuovere o rinominare una chiave,
cambiare un tipo o un'unità, rimuovere un identificatore lo incrementa — è una rottura,
annunciata nel CHANGELOG e qui.

## Livelli del resoconto

Ogni livello **contiene il precedente**: salire di un gradino aggiunge, non
sostituisce mai.

| Livello | Come | Che cosa porta l'output di errore |
| :----- | :------ | :------------------------------ |
| 0 | `--quiet` | nulla, **tranne gli errori** |
| 1 | per impostazione predefinita, o `-v` | una riga per file elaborato, e un riepilogo non appena ce n'è più di uno |
| 2 | `-vv` | e ciò che è stato riconosciuto: formato, codifica, BOM, terminatori di riga |
| 3 | `-vvv` | e la traccia di debug: byte letti e scritti, e **ogni messaggio diagnostico di lettura** |

**Gli errori non sono mai messi a tacere, `--quiet` compreso.** Un comando che fallisce
in silenzio non lascerebbe altro che il suo codice di uscita.

Il riepilogo compare solo da due file in poi: su un solo input, ripeterebbe
la riga che lo precede.

## Diagnostica di lettura

I formati dei sottotitoli sono letti **al meglio possibile**: di fronte a un'anomalia, il
lettore non si ferma — decide, oppure lascia le cose come stanno, e lo dice. Ciò che
ha incontrato esce al **livello 3**, una riga per messaggio diagnostico, sull'output di
errore:

```
a.srt: 2 diagnostics while reading
a.srt: line 6: SubRip numbers that do not follow ("7"), settled by the reader
a.srt: line 9: a line that fits nowhere, left as it stands
```

Tutti e diciassette i sottocomandi li riferiscono, non solo [`inspect`](inspect.md):
un file letto al meglio possibile e poi riscritto è passato per le stesse decisioni, e tacerle
farebbe credere che non sia successo nulla.

**Un messaggio diagnostico non è mai un fallimento.** Il file è stato letto, il comando
è riuscito, il codice di uscita è `0`. Per questo stanno al livello più
dettagliato: più in basso, seppellirebbero la riga che dice che cosa è stato davvero
fatto.

### Che cosa porta ogni riga

| Parte | Che cosa dice |
| :----- | :------------- |
| `line N` | dove, contato da 1, come lo mostra un editor; **assente** per ciò che riguarda l'intero file |
| la frase | che cosa è stato incontrato, tra le dodici categorie qui sotto |
| `("…")` | il testo difettoso del file, quando la categoria non basta; troncato a 80 byte |
| la fine | che cosa è stato fatto in proposito: `settled by the reader`, oppure `left as it stands` |

**Due categorie non hanno riga**, e sono le due che riguardano la
codifica: i byte sono stati valutati, o il loro indicatore letto, prima che esistesse una sola
riga del file. La loro riga inizia quindi con la frase, senza `line N`
davanti — non c'è alcun posto da nominare.

**La fine della riga è ciò che conta di più.** `settled by the reader` significa
che il lettore ha deciso e che il file scritto porta la sua decisione — una
numerazione mancante viene rigenerata. `left as it stands` significa che non ha toccato
nulla perché **solo l'utente può decidere**: un sottotitolo che finisce prima di
iniziare resta com'è.

### Il numero di riga di un blocco

Un'anomalia riguardante **un intero blocco** è ancorata alla sua riga di
sincronizzazione, e non alla riga esatta che la innesca. Una numerazione incoerente
scritta alla riga 5 viene quindi segnalata alla riga 6, quella della riga di sincronizzazione che
segue — il numero errato è nel `("…")`. Solo le anomalie riguardanti
**una riga** — una riga di sincronizzazione illeggibile, testo prima della prima — sono
segnalate su se stesse.

### Le dodici categorie

| Frase | Che cosa la innesca |
| :----- | :------------------ |
| `a line that fits nowhere` | una riga che non entra in nessun blocco |
| `a timing line that could not be read` | una riga di sincronizzazione illeggibile |
| `a SubRip block without its number` | numerazione mancante, rigenerata in scrittura |
| `SubRip numbers that do not follow` | numerazione che salta |
| `text before the first timing line` | testo prima della prima riga di sincronizzazione |
| `a WebVTT block of an unknown kind` | un blocco WebVTT non riconosciuto |
| `declares an event column this tool cannot fill` | una colonna di evento Sub Station Alpha che nulla qui riempie |
| `counts in frames and states no rate; it was read at` | un file in fotogrammi, che non indica alcuna frequenza dei fotogrammi — quella usata è il dettaglio |
| `carries no end times; each one was taken from the next start` | un formato senza fini, TMPlayer o LRC: **nessuna fine nella tabella viene dal file** |
| `more than one kind of line ending` | terminatori di riga misti |
| `an encoding nothing declared` | la codifica è stata proposta valutando i byte, e non è UTF-8 |
| `a byte order mark that contradicts the encoding asked for` | il file porta un indicatore, e `--encoding` ne ha nominato un altro — vince l'indicatore |

**Le tre nel mezzo sono quelle della fase 9**, e le ultime due di quelle
tre non parlano di una riga ma dell'intero file: dicono che un
valore mostrato non è stato letto. Una colonna `End` riempita su un LRC, una
posizione su un MicroDVD — il file non ne portava nulla.

**Un'operazione può aggiungere le proprie**, che non sono messaggi diagnostici di lettura:
[`correct`](correct.md#un-modello-che-non-può-essere-applicato) dice, con il sottotitolo, il modello
che ha rinunciato. Viene detto al livello 1 e porta, nell'oggetto del file, l'identificatore
`pattern-failed` — mai un fallimento del file.

## Una traduzione

Un file di traduzione viene trattato **come qualsiasi file di sottotitoli**:
`subedit-cli case --to sentence film.fr.srt` lo modifica senza nulla di nuovo. Ciò che l'abbinamento aggiunge, e ciò che
tre opzioni dicono, è limitato: **sapere come le righe di una traduzione cadono sui
sottotitoli del file principale** — è `inspect -t` —, e **modificare la traduzione tenendola
sovrapposta al suo file principale**, con le stesse regole della finestra.

| Opzione | Dove | Valore | Predefinito |
| :----- | :---- | :----- | :----- |
| `-t`, `--translation-file` | `inspect`, `hearing-impaired`, `replace`, `case`, `italics`, `dialogue-dashes`, `correct`, `pair` | il file di traduzione | — |
| `--document` | i sei sottocomandi di testo, non `inspect` né `pair` | `main` o `translation` | `main` |
| `--align-method` | come `-t` | `position` o `number` | `position` |

- **`position`** confronta il centro di ogni riga con i limiti dei sottotitoli: una riga mancante non
  sposta quelle che seguono. **`number`** manda la riga *n* al sottotitolo *n*, senza guardare le
  posizioni. La posizione è il valore predefinito, per il motivo che ha la finestra: una riga mancante nel
  mezzo sposta tutto il resto di una traduzione abbinata per numero.
- **`--document translation` richiede `-t`, e `-t` richiede `--document translation`** su un sottocomando
  di testo — l'uno senza l'altro è un errore d'uso, codice `1`, perché un file letto che nessun gesto
  usa è una dimenticanza e non una preferenza. `-t` su `inspect` non ha `--document`:
  riferisce. **Anche `--align-method` senza `-t` è un errore d'uso.**
- **`-t` nomina un file, e ha quindi senso solo per un singolo input**: con più input, oppure
  con una directory presa da `--recursive`, viene rifiutato come `--output`, **prima che un file sia
  letto**. Per un lotto, eseguire un'invocazione per coppia.
- **Un file di traduzione identico al file principale viene rifiutato** — comunque sia scritto il suo
  percorso: `<translation>: the file is already open as the main document`, codice `2`.
- **Viene scritto solo il documento preso di mira**, al **proprio percorso e nel proprio formato**: il formato,
  la codifica e i terminatori di riga del file di traduzione. `--output` nomina il file della
  traduzione, `--output-dir` vi mette il suo nome di base, `--in-place` lo riscrive; **il file principale
  non viene mai toccato**. I percorsi del rapporto sono quelli della traduzione.
- **I sottotitoli che l'abbinamento fa nascere** — una riga che nulla ha raggiunto — vengono scritti nella
  traduzione con **le proprie posizioni**, qualunque sia il metodo; **un sottotitolo che nessuno
  ha tradotto viene scritto come blocco senza testo**, che si legge senza anomalie.
- **`--encoding` e `--frame-rate` si applicano a entrambi i file.**
- I sottocomandi di **posizione** — `shift`, `transform`, `framerate`, `snap`, `adjust` — non hanno
  queste opzioni: dare loro entrambi i file, come per un lotto,
  `subedit-cli shift --by 2 film.srt film.fr.srt --output-dir out/`.
- **Scrivere la traduzione risincronizzata sul file principale** — l'unica cosa che l'abbinamento fa e che un file da solo
  non fa — è compito di [`pair`](pair.md), che non ha né `--document` né `--recursive`.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\n[Un oiseau] Bonjour.\n\n2\n00:00:03,000 --> 00:00:04,000\nAu revoir.\n\n' > film.fr.srt; subedit-cli hearing-impaired film.srt -t film.fr.srt --document translation --output propre.fr.srt; cat propre.fr.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\n[Un oiseau] Bonjour.\n\n2\n00:00:03,000 --> 00:00:04,000\nAu revoir.\n\n' > film.fr.srt; subedit-cli hearing-impaired film.srt -t film.fr.srt --document translation --output propre.fr.srt; cat propre.fr.srt
film.fr.srt: 1 subtitle cleaned, 0 removed -> propre.fr.srt
1
00:00:01,000 --> 00:00:02,000
Bonjour.

2
00:00:03,000 --> 00:00:04,000
Au revoir.
```

Con `--format json`, l'oggetto di ciascun file contiene anche la chiave `alignment`, che dice come la
traduzione è stata sovrapposta — `{"file", "method", "attached", "born", "untranslated", "out_of_order"}`, come
la chiave `translation` di [`inspect`](inspect.md#in-json) — e, **con `--dry-run`**, `changes` elenca
i sottotitoli della traduzione, ciascuno con `"document":"translation"`.

## Più file

Ogni sottocomando accetta più percorsi, tranne [`append`](append.md), [`split-file`](split-file.md) e
[`pair`](pair.md), la cui arità è quella indicata nella loro pagina. Ciascuno viene elaborato
in modo indipendente: il fallimento di uno non interrompe gli altri, e i fallimenti
vengono riferiti nominando il file e il motivo.

**Una directory sta per i file che contiene, con `--recursive` (`-r`)**:
senza questa opzione, darne una è un errore d'uso, codice `1`, e nulla viene
elaborato — rifiutarla è meglio che non elaborare nulla e restituire `0`:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?" -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?"
films: is a directory: use --recursive
code=1
```

La visita, l'albero mantenuto sotto `--output-dir` e ciò che viene lasciato
da parte sono descritti in [Elaborare un albero di directory](lots.md).

## Le pagine di manuale

Un'installazione mette **una pagina di manuale per sottocomando** accanto a `subedit-cli(1)`:
`subedit-cli-shift(1)`, `subedit-cli-convert(1)` e le altre, una per ogni riga della
[tabella dei sottocomandi](#sottocomandi). `subedit-cli(1)` contiene questo capitolo — le opzioni globali,
gli output, i codici di uscita — e ogni altra pagina contiene il capitolo del suo sottocomando, esempi
compresi. La pagina della finestra, `subedit-gui(1)`, è accanto a esse.

```console
$ man subedit-cli
$ man subedit-cli-shift
```

**Le pagine non sono scritte separatamente da questo manuale**: vengono generate da esso durante la
compilazione del programma, quindi dicono ciò che dice, e non possono discostarsene. La sezione FILES di ciascuna
pagina nomina la directory in cui il manuale è installato.

| Pagine | Dove sono installate |
| :---- | :----------------------- |
| inglese | `share/man/man1` |
| ogni lingua installata — catalano, ceco, tedesco, spagnolo, finlandese, francese, galiziano, ebraico, ungherese, interlingue, islandese, italiano, olandese, polacco, portoghese, portoghese brasiliano, russo, serbo, turco, cinese semplificato | `share/man/<language>/man1` — `share/man/fr/man1` |

**`man` sceglie la lingua dal locale**, come fa il programma: con un locale francese,
`man subedit-cli` mostra la pagina francese, e una lingua le cui pagine non sono installate riceve
quelle inglesi. `-L` chiede una lingua a prescindere dal locale:

```console
$ man -L fr subedit-cli-shift
```

## Codici di uscita

| Codice | Significato |
| :--- | :------------ |
| `0` | tutto è riuscito |
| `1` | errore d'uso — opzione sconosciuta, valore non valido, combinazione vietata |
| `2` | nessun file ha potuto essere elaborato |
| `3` | alcuni file sono stati elaborati, altri no |

`2` e `3` sono distinti affinché uno script sappia come agire senza rileggere l'output:
"non ha funzionato nulla" e "ne manca uno" non richiedono la stessa reazione.

Un errore d'uso viene rilevato **prima di qualsiasi elaborazione**: non lascia mai un
lotto elaborato a metà.

## Errori

| Che cosa lo provoca | Che cosa viene scritto, sull'output di errore |
| :------------------ | :--------------------------------------- |
| opzione o sottocomando sconosciuto | il nome errato, e un rimando a `--help` |
| `--quiet` con `-v` | `--quiet and -v ask for opposite things; give one or the other` |
| `--config` con `--no-config` | `--config excludes --no-config` |
| `--config` con un nome vuoto | `--config needs a file name` |
| file mancante | `<path>: does not exist` |
| file illeggibile | `<path>: cannot be opened: permission denied` |
| codifica che scrive un indicatore proprio | `"<name>" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do` |
| byte che non si decodificano | `<path>: cannot be decoded in the chosen encoding` |
| formato non riconosciuto | `<path>: is in no format this tool knows` |
| nulla che somigli a un sottotitolo | `<path>: holds nothing recognisable as a subtitle` |
| `--document translation` senza `-t` | `--document translation needs the translation file: use -t` |
| `-t` senza `--document translation` | `-t names a translation to change: use --document translation` |
| `--align-method` senza `-t` | `--align-method needs a translation file: use -t` |
| `-t` con più input, o una directory | `-t names one file but several inputs were given: use one invocation per pair` |
| traduzione identica al file principale | `<translation>: the file is already open as the main document` |
