# `snap`

<!-- machine-translated -->
> Questa pagina è una traduzione automatica e non è ancora stata rivista da un lettore della lingua.

```
subedit-cli snap --rate <rate>
                 (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 <file>...
```

Mette **ogni marca temporale** — inizio e fine — sul fotogramma più vicino della
frequenza dei fotogrammi indicata. Nulla viene risincronizzato: ogni posizione si sposta al
massimo di mezzo fotogramma.

## `snap` non è `framerate`, e scambiarli non si nota

I due comandi hanno la stessa forma — file, una destinazione, una frequenza dei fotogrammi — e fanno cose opposte.

| | [`framerate`](framerate.md) | `snap` |
| :--- | :-------------------------- | :----- |
| che cosa calcola | `t' = t x input rate / output rate` | il fotogramma più vicino, sulla griglia di `--rate` |
| di quanto sposta | in proporzione al tempo — **parecchi secondi** su un lungometraggio | al massimo mezzo fotogramma, **ovunque** |
| per quale file | un file la cui **sincronizzazione è sbagliata** | un file la cui sincronizzazione è **giusta** e la cui griglia è sbagliata |

**Il caso d'uso di `snap`**, in chiaro: un film a 25 fotogrammi al
secondo, un file di sottotitoli scritto su una griglia a 24, righe già
più o meno al loro posto, a pochi millisecondi di distanza. Non c'è nulla da risincronizzare
— solo da rimettere le marche temporali sui fotogrammi giusti.

**Il caso d'uso di `framerate`** è l'altro: un file sincronizzato per un master a
25 che si vuole riprodurre su una copia a 23,976, e la cui sincronizzazione deriva di
parecchi secondi verso la fine del film.

Prendere l'uno per l'altro **non provoca alcun errore**: entrambi
accettano il file e scrivono un risultato. La riga di narrazione è ciò che
li distingue — `snap` dice sempre di quanto ha spostato le posizioni.

<!-- example: subedit-cli snap --help -->
```console
$ subedit-cli snap --help
Move every position onto the nearest frame of a frame rate (see framerate)
Usage: subedit-cli snap [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to align

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --rate TEXT REQUIRED        Frame rate to align on: 25, 23.976
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Argomenti e opzioni

| Opzione | Obbligatoria | Valori | Predefinito |
| :----- | :----- | :------ | :----- |
| `<file>...` | sì | uno o più percorsi | — |
| `--recursive`, `-r` | no | un'opzione senza valore | disattivata — vedi [Elaborare un albero di directory](lots.md) |
| `--rate` | **sì** | una frequenza dei fotogrammi in fotogrammi al secondo, scritta come per [`framerate`](framerate.md#scrivere-una-frequenza-dei-fotogrammi) | — |
| `--output` / `--output-dir` / `--in-place` | **una delle tre**, tranne con `--dry-run` | vedi [Invocazione](invocation.md#la-destinazione) | — |
| `--dry-run` | no | un'opzione senza valore: calcola e dice, non scrive nulla — vedi [Vedere prima di scrivere](invocation.md#vedere-prima-di-scrivere) | disattivata |

`--rate` accetta **qualsiasi frequenza dei fotogrammi valida**, e non solo le otto
standard: è una scelta dell'utente, come per `framerate`. Una frequenza dei fotogrammi
nulla o negativa viene rifiutata.

Il formato del file letto viene **conservato**: cambiare formato è compito di
[`convert`](convert.md).

## Output

**Output standard** — nulla: il risultato è il file scritto. Con `--dry-run`, nulla neppure: non c'è alcun file, e la riga di narrazione lo dice — vedi [Vedere prima di scrivere](invocation.md#vedere-prima-di-scrivere).

| Livello | Che cosa si aggiunge sull'output di errore |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles aligned on 25 fps, M positions moved, by at most 20 ms -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — il formato, la codifica, l'indicatore e i terminatori di riga del file letto, rimessi com'erano |
| 3 | `<path>: N bytes read, M written`, poi **ogni diagnostica di lettura** — vedi [Invocazione](invocation.md#diagnostica-di-lettura) |

**I due numeri sono il senso della riga.** Mezzo fotogramma a 25 fotogrammi al
secondo sono venti millisecondi, e nulla di più grande può esservi scritto. Chi
voleva `framerate` e ha digitato `snap` legge «by at most 20 ms» là dove si aspettava dei
secondi, e lo capisce subito.

## Un esempio

Una riga a `00:00:01,010`, allineata su 25 fotogrammi al secondo — cioè su un
multiplo di quaranta millisecondi:

<!-- example: printf '1\n00:00:01,010 --> 00:00:03,020\nUn.\n\n' > a.srt; subedit-cli snap --rate 25 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,010 --> 00:00:03,020\nUn.\n\n' > a.srt; subedit-cli snap --rate 25 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle aligned on 25 fps, 2 positions moved, by at most 20 ms -> b.srt
1
00:00:01,000 --> 00:00:03,040
Un.
```

## Due proprietà

**Allineare due volte non cambia nulla la seconda volta.** Una posizione già su un
fotogramma ci resta: è la stessa operazione applicata allo stesso risultato.

**L'ordine dei sottotitoli viene conservato.** L'arrotondamento al fotogramma più vicino è
monotono, quindi due inizi distanti almeno un fotogramma restano tali. Più
vicini, possono fondersi — il che è un'anomalia che
[`inspect`](inspect.md#che-cosa-segnala-anomalies) sa nominare, e non
un'inversione.

### In JSON

Con [`--format json`](invocation.md#un-output-che-uno-script-può-leggere), l'oggetto di ogni file porta `counts.subtitles`, `counts.moved` (le posizioni spostate) e `counts.furthest_ms` (lo spostamento maggiore, in millisecondi). La tabella completa è quella di [Invocazione](invocation.md#che-cosa-ogni-sottocomando-mette-in-counts).

## Codici di uscita

Quelli dello strumento: `0` se ogni file è stato scritto, `2` se nessuno, `3` se
solo alcuni, `1` in caso di errore d'uso.

## Errori

Quelli di una frequenza dei fotogrammi, parola per parola quelli di [`framerate`](framerate.md#errori)
— entrambi leggono lo stesso valore per lo stesso percorso:

| Che cosa lo provoca | Messaggio |
| :------------------ | :------ |
| frequenza dei fotogrammi nulla o negativa | `"…" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed` |
| virgola decimale | `"…" is not a frame rate: use a decimal point and not a comma, in every language` |
| testo che non è un numero | `"…" is not a frame rate: expected frames per second, like 25 or 23.976` |
| frequenza dei fotogrammi del tutto sproporzionata | `"…" is not a frame rate: no video runs at that many frames per second` |

**Nulla viene scritto senza una delle tre opzioni di destinazione**, e i relativi errori
sono comuni ai sottocomandi che scrivono:
vedi [Invocazione](invocation.md#la-destinazione).
