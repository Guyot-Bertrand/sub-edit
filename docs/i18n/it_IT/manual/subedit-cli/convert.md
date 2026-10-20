# `convert`

<!-- machine-translated -->
> Questa pagina è una traduzione automatica e non è ancora stata rivista da un lettore della lingua.

```
subedit-cli convert --to <format>
                    [--line-endings unix|windows|mac] [--to-encoding NAME]
                    [--bom | --no-bom] [--sort] [--disjoint]
                    [--join-repeats] [--repair]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    <file>...
```

Scrive ogni file nel formato e nella forma richiesti. **Non modifica mai
l'input**, a meno che `--in-place` non lo chieda esplicitamente.

<!-- example: subedit-cli convert --help -->
```console
$ subedit-cli convert --help
Write a subtitle file out in another format or shape
Usage: subedit-cli convert [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to convert

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --to TEXT:{srt,vtt,subviewer2,ssa,ass,microdvd,mpl2,tmplayer,lrc} REQUIRED
                              Format to write
  --line-endings TEXT:{unix,windows,mac}
                              Line endings to write; the source's by default
  --to-encoding NAME          Encoding to write; the source's by default
  --frame-rate RATE           Frame rate of a file counted in frames: 25, 23.976
  --bom                       Write a byte order mark
  --no-bom                    Write no byte order mark
  --sort                      Put the subtitles in order of their start before writing
  --disjoint                  Cut overlapping subtitles so that no range overlaps another, before writing
  --join-repeats              Join the subtitles that repeat each other into one, before writing
  --repair                    Join the repeats, sort, then make the ranges disjoint, before writing
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Argomenti e opzioni

| Opzione | Obbligatorio | Valori | Predefinito |
| :----- | :----- | :------ | :----- |
| `<file>...` | sì | uno o più percorsi | — |
| `--recursive`, `-r` | no | un'opzione senza valore | disattivata — vedi [Elaborare un albero di directory](lots.md) |
| `--to` | **sì** | un nome della tabella qui sotto, e nient'altro | — |
| `--line-endings` | no | `unix`, `windows` oppure `mac` | quelli del file letto |
| `--to-encoding` | no | qualsiasi codifica che ICU sappia scrivere, tranne quelle che scrivono un proprio indicatore | quella del file letto |
| `--bom` / `--no-bom` | no | opzioni senza valore, che si escludono a vicenda | ciò che il file letto conteneva |
| `--sort` | no | un'opzione senza valore: mette i sottotitoli nell'ordine del loro inizio **prima di scrivere**, vedi [Ordinare in scrittura](#ordinare-in-scrittura) | disattivata |
| `--join-repeats` | no | un'opzione senza valore: ricava un solo sottotitolo dai pezzi di uno stesso testo che si susseguono, **prima di ogni altra cosa**, vedi [Rendere disgiunti gli intervalli](#rendere-disgiunti-gli-intervalli) | disattivata |
| `--repair` | no | un'opzione senza valore: equivale a `--join-repeats --sort --disjoint`, in quest'ordine | disattivata |
| `--disjoint` | no | un'opzione senza valore: taglia i sottotitoli sovrapposti **prima di scrivere**, vedi [Rendere disgiunti gli intervalli](#rendere-disgiunti-gli-intervalli) | disattivata |
| `--output` / `--output-dir` / `--in-place` | **una delle tre**, tranne con `--dry-run` | vedi [Invocazione](invocation.md#la-destinazione) | — |
| `--dry-run` | no | un'opzione senza valore: calcola e riferisce, non scrive nulla — vedi [Vedere prima di scrivere](invocation.md#vedere-prima-di-scrivere) | disattivata |

`mac` indica il solo ritorno a capo (`\r`), il terminatore di riga del Mac OS classico.

## I formati accettati da `--to`

| Nome | Formato | Estensione scritta |
| :-- | :----- | :--------------- |
| `srt` | SubRip | `.srt` |
| `vtt` | WebVTT | `.vtt` |
| `subviewer2` | SubViewer 2.0 | `.sub` |
| `ssa` | Sub Station Alpha | `.ssa` |
| `ass` | Advanced SSA | `.ass` |
| `mpl2` | MPL2 | `.txt` |
| `microdvd` | MicroDVD | `.sub` |
| `tmplayer` | TMPlayer | `.txt` |
| `lrc` | LRC | `.lrc` |

**Un nome, e non un'estensione.** Due estensioni indicano ciascuna due formati
— `.sub` è anche quella di MicroDVD, `.txt` quella di MPL2 e di
TMPlayer — quindi un'opzione che accettasse estensioni non saprebbe quale
formato si chiede. I nomi qui sopra non sono ambigui, e coincidono con
l'estensione ovunque anche l'estensione non sia ambigua.

**L'elenco è chiuso.** È quello di Gaupol, e tutti e nove vi compaiono.

## I due formati che non hanno la fine

**TMPlayer e LRC scrivono una sola posizione per riga**, il suo inizio. Questo
si nota da entrambe le parti.

**In lettura**, i tempi di fine vengono dedotti: quella di una riga è l'inizio
della successiva, e l'ultima ottiene cinque secondi. È ciò che fa Gaupol.
Lo diciamo anche noi, con una diagnostica che `--verbose` mostra sui tre
sottocomandi:

```
file.lrc: carries no end times; each one was taken from the next start, settled by the reader
```

**In scrittura**, i tempi di fine non vengono scritti, per mancanza di un posto in cui metterli.
Convertire in uno di questi due formati li fa quindi perdere, e un'andata e ritorno non
li ritrova — li deduce di nuovo.

**LRC perde un'altra cosa**: non ha modo di contenere un a capo.
Una battuta su due righe viene scritta su una sola, le due righe unite da
uno spazio. Anche questa perdita non si può riparare.

Nessuno dei due ha un vocabolario per il corsivo.

## `--frame-rate` e l'unico formato che conta in fotogrammi

**MicroDVD non conta in secondi**: le sue posizioni *sono* numeri di
fotogrammi, e nessun file MicroDVD dichiara la frequenza con cui sono stati
contati. `--frame-rate` la fornisce, e serve da entrambe le parti: è la
frequenza con cui si legge un `.sub` in fotogrammi, e quella con cui una
conversione in MicroDVD conta i fotogrammi.

| Opzione | Obbligatorio | Valori | Predefinito |
| :----- | :----- | :------ | :----- |
| `--frame-rate` | no | una frequenza in fotogrammi al secondo: `25`, `23.976`, `29.97` | vedi sotto |

**In lettura, senza `--frame-rate`, il file viene letto a 23,976** — il valore
di Gaupol — e il livello `-vvv` lo dice:
`counts in frames and states no rate; it was read at`. È l'unico punto dello strumento in cui ogni posizione mostrata
poggia su un'ipotesi che il file non può confermare.

**In scrittura, senza `--frame-rate`, tre casi e tre risposte:**

| Il file di partenza | Che cosa succede |
| :------------------- | :-------------- |
| è già MicroDVD | la frequenza che l'ha letto lo riscrive, e i fotogrammi tornano identici |
| è basato sul tempo, e le sue posizioni cadono su una griglia | viene presa la griglia, e `-vv` dice quale |
| è basato sul tempo, senza griglia | **rifiuto**, codice 2: `writing frames needs a frame rate, and the positions fall on no grid to take one from — give --frame-rate` |

Il rifiuto è voluto. L'unico gesto rimasto sarebbe inventare una cifra, e
ogni riga del file si sposterebbe di quanto la cifra fosse sbagliata.

## Che cosa perde una conversione

**Convertire in un altro formato non è mai senza perdite**, e ciò che non sopravvive
dipende dalla coppia:

| Che cosa non passa | Perché |
| :--------------------- | :------- |
| l'intestazione | ha senso solo nel proprio formato: `[INFORMATION]` non è un'intestazione WebVTT |
| i dati specifici del formato | le coordinate di SubRip, l'identificatore e le impostazioni di un cue WebVTT non hanno equivalente altrove |
| la precisione | SubViewer 2, i due Sub Station Alpha e LRC scrivono al centesimo, MPL2 al decimo, TMPlayer al secondo, MicroDVD al fotogramma; `01:00:00,017` torna come `01:00:00,020` |
| la fine di una riga | TMPlayer e LRC non hanno un campo per essa; la lettura successiva la deduce di nuovo |
| gli a capo | solo LRC: le sue righe stanno su una riga sola, e le due vengono riunite da uno spazio |
| i tag che il formato di destinazione non sa scrivere | WebVTT non ha `<font color>`, TMPlayer e LRC non hanno proprio nulla |
| la disposizione | `{\pos(x,y)}`, `{\an8}`, `<v Marie>`, `<ruby>`: sopravvivono a un'andata e ritorno nel proprio formato, e scompaiono nella conversione |

**Riscrivere un file nel proprio formato non perde nulla** — è la garanzia
su cui si regge tutto il resto, e `--to srt` su un `.srt` restituisce gli stessi byte. Nulla
viene nemmeno decodificato: il testo resta la stringa grezza che è stata letta.

## I tag vengono tradotti, non copiati

**Ogni formato scrive il corsivo a modo suo**, e convertire senza tradurre
lascerebbe un `{\i1}` in un `.srt`: testo che l'utente vede, in un
file che non saprà mai interpretarlo. Ciò che passa sono **sei
cose** — grassetto, corsivo, sottolineato, colore, carattere e dimensione:

| Formato | Grassetto | Corsivo | Sottolineato | Colore | Carattere | Dimensione |
| :----- | :--: | :------: | :------: | :-----: | :----: | :----: |
| SubRip | `<b>` | `<i>` | `<u>` | `<font color="#RRGGBB">` | — | — |
| WebVTT | `<b>` | `<i>` | `<u>` | — | — | — |
| SubViewer 2 | `<b>` | `<i>` | `<u>` | `<font color="#RRGGBB">` | — | — |
| Sub Station Alpha | `{\b1}` | `{\i1}` | — | `{\c&HBBGGRR&}` | `{\fnNAME}` | `{\fsN}` |
| Advanced SSA | `{\b1}` | `{\i1}` | `{\u1}` | `{\c&HBBGGRR&}` | `{\fnNAME}` | `{\fsN}` |
| MicroDVD | `{Y:b}` | `{Y:i}` | `{Y:u}` | `{C:$BBGGRR}` | `{F:NAME}` | `{S:N}` |
| MPL2 | `\` | `/` | `_` | `{C:$BBGGRR}` | `{F:NAME}` | `{S:N}` |
| TMPlayer | — | — | — | — | — | — |
| LRC | — | — | — | — | — | — |

**Una perdita di tag è una perdita annunciata.** Convertire in WebVTT un `.srt` colorato elimina
il colore e lo dice; il file prodotto è WebVTT, non SubRip sotto mentite spoglie.

**MicroDVD e MPL2 non sanno formattare una parte di riga.** I loro tag
non hanno fine: `{y:i}` inclina tutto ciò che segue fino alla fine della riga, e
`{Y:i}` fino alla fine del sottotitolo. Una riga solo a metà
in grassetto viene quindi scritta senza alcun grassetto, anziché con un grassetto che trabocca.

## Che cosa la conversione dichiara di aver perso

**Lo dice quando perde, e tace quando non perde nulla.** Un rapporto
che compare a ogni chiamata è un rapporto che nessuno legge.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,500\n<i>sur deux</i>\nlignes\n\n' > a.srt; subedit-cli convert --to lrc --output b.lrc a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,500\n<i>sur deux</i>\nlignes\n\n' > a.srt; subedit-cli convert --to lrc --output b.lrc a.srt
a.srt: 1 subtitle written as LRC -> b.lrc
a.srt: ends are not carried by LRC, line breaks were joined in 1 subtitle, 1 tag dropped
```

La riga compare al livello di dettaglio predefinito, e a ognuno di quelli superiori.
Le voci, in quest'ordine:

| Voce | Che cosa dice |
| :---- | :----------- |
| `ends are not carried by <format>` | il formato di destinazione non scrive alcuna fine |
| `line breaks were joined in N subtitles` | solo LRC: N sottotitoli erano su più righe |
| `N tags dropped` | N tag non hanno potuto essere scritti, disposizione compresa |
| `the header was dropped` | l'intestazione non attraversa il confine tra formati |
| `the <format> fields of N subtitles were dropped` | N sottotitoli contenevano dati specifici del loro formato |
| `positions moved by up to N ms` | la distanza massima di cui una posizione si è spostata |

## Cambiare la codifica

**`--to-encoding` sceglie la codifica scritta; `--encoding` indica quella che viene
letta.** Insieme, ricodificano un file:

```console
$ subedit-cli convert --to srt --encoding cp1252 --to-encoding utf-8 \
      --output-dir sortie film.srt
```

Senza `--to-encoding`, il file viene scritto nella **codifica in cui è stato letto**,
come avviene per i terminatori di riga e per l'indicatore: un file convertito senza
indicazioni sulla codifica restituisce gli stessi caratteri, agli stessi byte.

**Un carattere che la codifica di destinazione non sa scrivere interrompe il file**, e
nulla viene scritto. Una `ł` non ha posto in Latin-1; sostituirla con un `?`
farebbe perdere testo tra la lettura e la scrittura, senza che nessuno se ne accorga.

**Un indicatore dell'ordine dei byte esiste solo per le codifiche Unicode.**
`--bom` con un `--to-encoding` che non ne ha viene rifiutato anziché ignorato
— vedi la tabella degli errori più sotto.

**E la codifica deve indicare il proprio ordine dei byte.** `UTF-16` e `UTF-32` sono
nomi che ICU conosce, il cui convertitore scrive **un indicatore proprio**, qualunque cosa
gli si chieda: `--no-bom` sarebbe quindi accettato e disobbedito. Entrambi vengono
quindi rifiutati, e il messaggio indica l'output:

```console
$ subedit-cli convert --to srt --to-encoding UTF-16 --output copie.srt film.srt
"UTF-16" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do
```

`UTF-16LE`, `UTF-16BE`, `UTF-32LE` e `UTF-32BE` scrivono esattamente gli stessi
byte, indicatore compreso — ma sotto il controllo di `--bom` e `--no-bom`. Il
rifiuto non costa quindi altro che un nome più preciso, e vale in lettura
come in scrittura: `--encoding UTF-16` ottiene la stessa risposta.

## La destinazione

Le tre opzioni e le loro regole sono comuni ai sottocomandi che
scrivono: vedi [Invocazione](invocation.md#la-destinazione). Nulla viene scritto
senza una di esse.

Ciò che è specifico di `convert`: **`--output-dir` cambia l'estensione secondo il
formato.** `convert --to vtt` su `a.srt` scrive `a.vtt`, mai un `a.srt` che contenga
WebVTT — un file il cui nome mente fa inciampare ogni altro strumento.

## La conversione sul posto è rifiutata

`--in-place` con un `--to` che cambia il formato dà il codice `1` e non scrive
nulla: sul posto non c'è un secondo nome che porti il nuovo formato, e
il file resterebbe sotto un'estensione che il suo contenuto non giustifica più.

`--in-place` si può comunque usare per cambiare **la forma senza il formato** —
riscrivere un `.srt` come `.srt` con altri terminatori di riga, per esempio.

Il rifiuto viene deciso **sulla sola estensione**, prima di qualsiasi lettura: un errore
di utilizzo non deve mai lasciare un blocco scritto a metà.

## Terminatori di riga e indicatore dell'ordine dei byte

Per impostazione predefinita, **il file scritto riprende ciò che il file letto conteneva**. Il modello
conserva entrambi in lettura; imporli per impostazione predefinita farebbe perdere, a ogni
conversione, un'informazione conservata di proposito.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli convert --to vtt --output b.vtt a.srt; cat b.vtt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli convert --to vtt --output b.vtt a.srt; cat b.vtt
a.srt: 1 subtitle written as WebVTT -> b.vtt
WEBVTT

00:01.000 --> 00:03.000
Bonjour.
```

## Ordinare in scrittura

`--sort` scrive il file **nell'ordine degli inizi**, qualunque sia il formato di destinazione. L'ordinamento avviene **su una copia**: il file letto non
viene toccato, e ciò che viene scritto è ciò che ne avrebbe fatto [`sort`](sort.md) — in modo stabile, due sottotitoli che cominciano insieme conservano il loro ordine.

**Mai per impostazione predefinita**: l'ordine di un file è quello del suo autore finché non si chiede altro. Con l'opzione, ogni file lo dice, al
primo livello di narrazione, `<path>: N subtitles moved` oppure `<path>: already in order`, e il record `--format json` contiene un
conteggio `moved` — **assente senza l'opzione**, per non cambiare la forma di ciò che gli script esistenti leggono.

## Rendere disgiunti gli intervalli

`--disjoint` taglia i sottotitoli sovrapposti in modo che nessun intervallo ne sovrapponga più un altro, **sulla stessa copia dell'ordinamento**: il file letto non viene toccato.

Mai per impostazione predefinita. L'opzione fa ciò che fa [`disjoint`](disjoint.md), **con le sue impostazioni predefinite**, prima di scrivere, dopo l'ordinamento. Richiede un file in ordine: altrimenti viene rifiutata, e `--sort` — che viene prima — lo mette in ordine in anticipo. Il file lo dice al primo livello di narrazione (`<path>: N subtitles split, M merged`), e il record `--format json` contiene i conteggi `split` e `merged`, **assenti senza l'opzione**.

**`--join-repeats` viene prima dell'ordinamento e del taglio**: un testo tagliato in pezzi che si susseguono viene prima ricostruito (vedi
[`join-repeats`](join-repeats.md)), poi i sottotitoli vengono ordinati, poi gli intervalli resi disgiunti. **`--repair` è tutte e tre le cose insieme**,
come [`repair`](repair.md), con le impostazioni predefinite del taglio. Con `--join-repeats`, ogni file dice
`<path>: N repeated subtitles joined` al primo livello di narrazione, e il record `--format json` contiene `counts.joined` — **assente senza l'opzione**.

## Output

**Output standard** — nulla: il risultato è il file scritto. Con `--dry-run`, nulla nemmeno allora: non c'è alcun file, e la riga di narrazione lo dice — vedi [Vedere prima di scrivere](invocation.md#vedere-prima-di-scrivere).

Sull'output degli errori, secondo il livello:

| Livello | Che cosa si aggiunge |
| :----- | :-------------- |
| 1 | `<path>: N subtitles written as WebVTT -> <destination>` |
| 2 | `<path>: SubRip -> WebVTT, UTF-8, no BOM, LF line endings` — la codifica indicata è quella scritta |
| 3 | `<path>: N bytes read, M written`, poi **ogni diagnostica di lettura** — vedi [Invocazione](invocation.md#diagnostica-di-lettura) |

### In JSON

Con [`--format json`](invocation.md#un-output-che-uno-script-può-leggere), l'oggetto di ciascun file contiene `counts.subtitles`, poi ciò che la conversione ha perso: `lost_ends` e `lost_header` (`0` oppure `1`), `joined_lines`, `lost_tags`, `lost_fields` e `furthest_ms` — le voci della [frase di perdita](#che-cosa-la-conversione-dichiara-di-aver-perso). La tabella completa è quella di [Invocazione](invocation.md#che-cosa-ogni-sottocomando-mette-in-counts).

## Codici di uscita

Quelli dello strumento: `0` se ogni file è stato scritto, `2` se nessuno, `3` se
solo alcuni, `1` per un errore di utilizzo.

## Errori

| Che cosa lo provoca | Messaggio |
| :------------------ | :------ |
| `--bom` con `--no-bom` | `--bom and --no-bom ask for opposite things; give one or the other` |
| `--in-place` che cambia il formato | `--in-place cannot change the format: the file would keep a name its content no longer matches` |
| `--bom` su una codifica che non ne ha | `<path>: <encoding> has no byte order mark to write` |
| `--to-encoding` che indica una codifica sconosciuta | `no encoding is named "<name>"` |
| `--to-encoding` che indica una codifica che scrive un proprio indicatore | `"<name>" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do` |
| carattere assente dalla codifica scritta | `<path>: holds a character the chosen encoding cannot write` |

**Rifiutare `--bom` su una codifica che non ne ha non è
pedanteria.** Un indicatore dell'ordine dei byte esiste per le codifiche Unicode e
per nessun'altra: chiederne uno su un file in Windows-1252 significa chiedere
qualcosa che non esiste. Il file verrebbe scritto senza, e nessuno
saprebbe che la richiesta non è stata soddisfatta.

Quelli della destinazione sono comuni ai sottocomandi che scrivono:
vedi [Invocazione](invocation.md#la-destinazione).

Gli errori di lettura sono quelli di [`inspect`](inspect.md): riguardano
il file di input e hanno gli stessi messaggi.
