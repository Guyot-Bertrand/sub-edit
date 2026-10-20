# `adjust`

<!-- machine-translated -->
> Questa pagina è una traduzione automatica e non è ancora stata rivista da un lettore della lingua.

```
subedit-cli adjust [--speed CPS|off] [--shorten | --no-shorten] [--lengthen | --no-lengthen]
                   [--minimum TIME|off] [--maximum TIME|off] [--gap TIME|off]
                   [--range N-M|N-]
                   (--output FILE | --output-dir DIR | --in-place)
                   [--dry-run]
                   [--recursive]
                   <file>...
```

Regola la **durata** di ogni sottotitolo secondo una velocità di lettura e dei limiti: un
sottotitolo troppo breve per essere letto viene allungato, un sottotitolo che lascia lo schermo
troppo tardi può essere accorciato, e due sottotitoli non si sovrappongono. **Si sposta
solo la fine**: spostare l'inizio significherebbe spostare il sottotitolo, che è compito di
[`shift`](shift.md). È l'operazione `Adjust Durations…` della
[finestra](../subedit-gui/operations.md#adjust-durations), con gli stessi valori predefiniti.

## I quattro vincoli e il loro ordine

Si applicano **in quest'ordine**, ciascuno fissando la fine, l'ultimo prevale:

1. **la velocità di lettura** — la durata di cui il testo ha bisogno, a tanti caratteri al
   secondo (i tag non contano, gli a capo sì);
2. **il minimo** — una durata inferiore viene portata al minimo;
3. **il massimo** — una durata superiore viene riportata al massimo;
4. **l'intervallo** — il tempo lasciato prima del sottotitolo successivo. Viene per ultimo e
   **prevale**: due sottotitoli mostrati insieme sono un difetto che lo spettatore vede,
   un sottotitolo un po' breve no.

L'ultimo sottotitolo del file non ha un successivo, e quindi nessun intervallo da rispettare. Il successivo
di un sottotitolo è quello che lo segue nel file, **scelto o no** da `--range`.

| Vincolo | Opzione | Predefinito | Disattivarlo |
| :--------- | :----- | :----- | :------- |
| velocità | `--speed CPS` | **15** caratteri al secondo | `--speed off` |
| direzione della velocità | `--shorten`, `--no-shorten`, `--lengthen`, `--no-lengthen` | **allunga** soltanto | — |
| minimo | `--minimum TIME` | **1,5 s** | `--minimum off` |
| massimo | `--maximum TIME` | **nessuno** | `--maximum off` |
| intervallo | `--gap TIME` | **0** | `--gap off` |

Sono le impostazioni di Gaupol e della prima apertura della finestra. **`off` disattiva un
vincolo: non viene più verificato.** Non significa impostarlo a zero — un minimo di `0`
è un minimo di zero, un intervallo di `0` vieta qualsiasi sovrapposizione.

## Che cosa fornisce il file delle impostazioni

**I valori predefiniti qui sopra sono quelli incorporati.** Per impostazione predefinita `adjust` legge le impostazioni dell'operazione dal
file delle impostazioni — i valori che `Adjust Durations…` ricorda nella finestra, o che
[`config set`](config.md) ha scritto — e questi prendono il posto dei valori predefiniti incorporati. **Un'opzione
data sulla riga prevale sempre**, poi viene il file, poi il valore predefinito incorporato. Vedi
[Il file delle impostazioni](invocation.md#il-file-delle-impostazioni) per sapere dov'è il file, `--config` e
`--no-config`.

| Chiave | Che cosa fornisce | Opzione che la annulla |
| :-- | :--------------- | :----------------------- |
| `duration-adjust.speed` | la velocità di lettura, in caratteri al secondo | `--speed CPS` |
| `duration-adjust.lengthen`, `duration-adjust.shorten` | in quale direzione la velocità può spostare una fine; con entrambe `false` la velocità è disattivata | `--lengthen`, `--no-lengthen`, `--shorten`, `--no-shorten`, `--speed off`; dare `--speed CPS` o una direzione riattiva una velocità che il file aveva disattivato, con allungamento |
| `duration-adjust.minimum-enabled`, `duration-adjust.minimum-ms` | la durata più breve, oppure nessuna | `--minimum TIME`, `--minimum off` |
| `duration-adjust.maximum-enabled`, `duration-adjust.maximum-ms` | la durata più lunga, oppure nessuna | `--maximum TIME`, `--maximum off` |
| `duration-adjust.gap-enabled`, `duration-adjust.gap-ms` | l'intervallo minimo, oppure nessuno | `--gap TIME`, `--gap off` |

**Ogni chiave ha un'opzione che la annulla.** Ogni direzione ha una coppia di opzioni che la impostano
qualunque cosa dica il file: `--shorten` e `--no-shorten` per l'accorciamento, `--lengthen` e
`--no-lengthen` per l'allungamento; un'opzione non data lascia la direzione del file così com'è.
`--maximum off` elimina un massimo che il file attiva. Con `--format json`, `constraints` indica
le impostazioni realmente usate, da qualunque parte provengano, e `settings` il file che è stato letto:

<!-- example: printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine/settings.conf config set duration-adjust.speed 12 2>/dev/null; subedit-cli --config mine/settings.conf --format json adjust --dry-run a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine/settings.conf config set duration-adjust.speed 12 2>/dev/null; subedit-cli --config mine/settings.conf --format json adjust --dry-run a.srt
settings: read mine/settings.conf
a.srt: adjusted the durations of 1 subtitle (dry run, nothing written)
{"schema":1,"command":"adjust","file":"a.srt","ok":true,"dry_run":true,"destination":null,"counts":{"subtitles":1,"adjusted":1,"sacrificed":{"speed":0,"minimum":0,"gap":0}},"constraints":{"speed":{"cps":"12","lengthen":true,"shorten":false},"minimum_ms":1500,"maximum_ms":null,"gap_ms":0},"settings":"mine/settings.conf","warnings":[]}
```

Gli altri esempi di questa pagina sono eseguiti con `--no-config`, in modo che mostrino che cosa fanno i valori predefiniti
incorporati, qualunque siano le impostazioni della macchina che li esegue.

<!-- example: subedit-cli adjust --help -->
```console
$ subedit-cli adjust --help
Bring the duration of every subtitle within a reading speed and limits
Usage: subedit-cli adjust [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to adjust

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --speed CPS|off             Reading speed in characters per second, or off (default 15)
  --shorten                   Let the reading speed bring an end earlier
  --no-shorten                Do not let the reading speed bring an end earlier
  --lengthen                  Let the reading speed move an end later
  --no-lengthen               Do not let the reading speed move an end later
  --minimum TIME|off          Shortest duration, or off (default 1.5)
  --maximum TIME|off          Longest duration, or off (default: none)
  --gap TIME|off              Least time left before the next subtitle, or off (default 0)
  --range N-M|N-              Act only on subtitles N to M, or N to the end
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
| `--speed` | no | un numero di caratteri al secondo, **maggiore di zero** (`15`, `12.5`), oppure `off` | `15` |
| `--shorten` | no | un'opzione senza valore: la velocità può anche **portare** una fine **prima** | disattivata |
| `--no-shorten` | no | un'opzione senza valore: la velocità non **porta** più una fine **prima**, qualunque cosa dica il file delle impostazioni | disattivata |
| `--lengthen` | no | un'opzione senza valore: la velocità **sposta** una fine **dopo**, qualunque cosa dica il file delle impostazioni | disattivata |
| `--no-lengthen` | no | un'opzione senza valore: la velocità non **sposta** più una fine **dopo** | disattivata |
| `--minimum` | no | una durata pari o superiore a zero, oppure `off` | `1.5` |
| `--maximum` | no | una durata **maggiore di zero**, oppure `off` | nessuno |
| `--gap` | no | una durata pari o superiore a zero, oppure `off` | `0` |
| `--range` | no | `N-M`, oppure `N-` fino alla fine — vedi sotto | l'intero file |
| `--output` / `--output-dir` / `--in-place` | **una delle tre**, tranne con `--dry-run` | vedi [Invocazione](invocation.md#la-destinazione) | — |
| `--dry-run` | no | un'opzione senza valore: calcola e riferisce, non scrive nulla — vedi [Vedere prima di scrivere](invocation.md#vedere-prima-di-scrivere) | disattivata |

Una durata si scrive come per [`shift`](shift.md#scrivere-una-durata): `2.5`, `00:00:02.500`.

**Senza alcun vincolo attivo, `adjust` viene rifiutato** (codice `1`, nulla viene scritto): non ci
sarebbe nulla a cui adeguarsi, e un comando che non fa nulla dicendo di essere riuscito è la peggiore
risposta possibile. È il caso di `--speed off --minimum off --gap off` senza un massimo, e di
`--no-lengthen` da solo con il minimo e l'intervallo disattivati: una velocità che non sposta
alcuna fine non vincola nulla.

`--shorten`, `--no-shorten`, `--lengthen` e `--no-lengthen` indicano in quale direzione la velocità sposta una
fine; hanno senso solo con una velocità, e **`--speed off` li rifiuta**. Anche un'opzione data insieme alla sua
opposta — `--shorten --no-shorten`, `--lengthen --no-lengthen` — viene rifiutata.

### `--range`

`--range 3-7` limita la regolazione ai sottotitoli da 3 a 7, **contati a partire da 1, estremi
inclusi**; `--range 3-` arriva fino all'ultimo. Un singolo sottotitolo si scrive `4-4`.

- Un intervallo **che finisce prima di cominciare**, uno `0`, un segno o un numero isolato vengono
  rifiutati come errore di utilizzo (codice `1`).
- **Vale per ogni file del blocco**, il che ha senso solo se i file
  si assomigliano. Viene valutato **rispetto a ciascun file**: un intervallo che supera quel file viene
  rifiutato **prima che lo si tocchi**, indicando il limite —
  `range 3-9 ends after the last subtitle: the file has 4` —, quel file fallisce (codice `2` o `3`, identificatore
  `range-out-of-bounds`) e gli altri proseguono.
- I conteggi del rapporto riguardano solo i sottotitoli scelti.

## Che cosa viene sacrificato

Due vincoli possono contraddirsi sullo stesso sottotitolo: un minimo di 1,5 s non sta
negli 0,8 s che il sottotitolo successivo lascia. **L'ordine decide, e il rapporto dice
che cosa ha ceduto** — cosa che Gaupol non fa: li infrange in silenzio.

Ogni vincolo è nominato solo se **almeno un sottotitolo non lo ha ottenuto**, una volta
applicati gli altri. Il massimo non ha conteggio: viene applicato prima dell'intervallo, che può
solo accorciare, quindi vale sempre. Un minimo superiore al massimo cede al massimo e
viene contato come minimo.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --output ajuste.srt courts.srt; grep -e '-->' ajuste.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --output ajuste.srt courts.srt; grep -e '-->' ajuste.srt
courts.srt: adjusted the durations of 3 subtitles; could not satisfy the minimum duration in 1 subtitle -> ajuste.srt
00:00:00,000 --> 00:00:02,000
00:00:10,000 --> 00:00:14,000
00:00:14,200 --> 00:00:15,000
00:00:15,000 --> 00:00:16,500
```

Il terzo sottotitolo vorrebbe finire a 15,7 s per durare 1,5 s, ma il quarto comincia a
15,0 s: **prevale l'intervallo**, finisce a 15,0 s, e il rapporto nomina il minimo.

Disattivando l'intervallo prevale il minimo — e i due sottotitoli si sovrappongono:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --gap off --output libre.srt courts.srt; grep -e '-->' libre.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --gap off --output libre.srt courts.srt; grep -e '-->' libre.srt
courts.srt: adjusted the durations of 3 subtitles -> libre.srt
00:00:00,000 --> 00:00:02,000
00:00:10,000 --> 00:00:14,000
00:00:14,200 --> 00:00:15,700
00:00:15,000 --> 00:00:16,500
```

E nulla viene sacrificato né regolato quando il file rispetta già i suoi limiti:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --speed off --minimum off --output intact.srt courts.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --speed off --minimum off --output intact.srt courts.srt
courts.srt: no duration to adjust -> intact.srt
```

Il file viene scritto così com'è: **una destinazione data è una destinazione scritta**.

## Output

**Output standard** — nulla: il risultato è il file scritto.

| Livello | Che cosa si aggiunge sull'output degli errori |
| :----- | :------------------------------------- |
| 1 | `<path>: adjusted the durations of N subtitles; could not satisfy …  -> <destination>`, oppure `<path>: no duration to adjust -> <destination>` — e, prima di tutto, il file delle impostazioni letto — `settings: read <path>` — vedi [Il file delle impostazioni](invocation.md#il-file-delle-impostazioni) |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — il formato, la codifica, l'indicatore e i terminatori di riga del file letto, rimessi com'erano |
| 3 | `<path>: N bytes read, M written`, poi **ogni diagnostica di lettura** — vedi [Invocazione](invocation.md#diagnostica-di-lettura) |

La frase è quella della finestra: `N` è il numero di sottotitoli la cui fine si è spostata, e il
resto, assente quando nulla ha ceduto, elenca ciò che non è stato rispettato — `the reading speed`,
`the minimum duration`, `the gap` — con il numero di sottotitoli interessati.

### In JSON

Con [`--format json`](invocation.md#un-output-che-uno-script-può-leggere), l'oggetto di ciascun file
contiene, oltre all'involucro:

| Chiave | Contenuto |
| :-- | :------ |
| `counts.subtitles` | i sottotitoli **presi di mira**: l'intero file, oppure l'intervallo |
| `counts.adjusted` | quelli la cui fine si è spostata |
| `counts.sacrificed.speed`, `.minimum`, `.gap` | i sottotitoli che ancora non rispettano questo vincolo, una volta applicati gli altri, **compresi quelli la cui fine non si è spostata** |
| `constraints.speed` | `{"cps", "lengthen", "shorten"}` — i caratteri al secondo **come stringa** (`"15"`, `"12.5"`) — oppure `null` se disattivata |
| `constraints.minimum_ms`, `.maximum_ms`, `.gap_ms` | le durate usate, in **millisecondi interi**, oppure `null` se disattivate |
| `settings` | il file delle impostazioni che è stato letto, oppure `null` se non ne è stato letto nessuno |

`constraints` indica **le impostazioni realmente usate**, valori predefiniti compresi: due calcoli
sugli stessi sottotitoli si confrontano quindi senza leggere la riga di comando.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config --format json adjust --dry-run --maximum 1.8 courts.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config --format json adjust --dry-run --maximum 1.8 courts.srt
courts.srt: adjusted the durations of 4 subtitles; could not satisfy the reading speed in 1 subtitle, the minimum duration in 1 subtitle (dry run, nothing written)
{"schema":1,"command":"adjust","file":"courts.srt","ok":true,"dry_run":true,"destination":null,"counts":{"subtitles":4,"adjusted":4,"sacrificed":{"speed":1,"minimum":1,"gap":0}},"constraints":{"speed":{"cps":"15","lengthen":true,"shorten":false},"minimum_ms":1500,"maximum_ms":1800,"gap_ms":0},"settings":null,"warnings":[]}
```

## Codici di uscita

Quelli dello strumento: `0` se ogni file è stato elaborato, `2` se nessuno, `3` se solo
alcuni, `1` per un errore di utilizzo. Un file che rispetta già i suoi limiti è elaborato.

## Errori

| Che cosa lo provoca | Messaggio |
| :------------------ | :------ |
| un file indicato da `--config` che non esiste — il file dell'utente, mancante, dà i valori predefiniti | `settings file not found: <FILE>` |
| nessun vincolo attivo | `adjust has nothing to adjust to: every constraint is off, or the reading speed is told to move no end` |
| una velocità che non lo è | `--speed: "…" is not a reading speed: expected a number of characters per second above zero, like 15 or 12.5, or off` |
| un'opzione di direzione con `--speed off` | `--lengthen, --no-lengthen, --shorten and --no-shorten say which way the reading speed may move an end, and --speed off switches the reading speed off` |
| un'opzione di direzione con la sua opposta | `--shorten and --no-shorten say opposite things; give one of them` (analogamente `--lengthen` e `--no-lengthen`) |
| una durata che non lo è | `--minimum: "…" is not a time: …` (analogamente `--maximum`, `--gap`) |
| durata negativa | `--minimum: "-1" is not a duration of zero or more` (analogamente `--gap`) |
| massimo nullo o negativo | `--maximum: "0" is not a duration above zero` |
| intervallo scritto male | `--range: "…" is not a range: write it N-M, or N- to go to the end, counted from 1 and inclusive` |
| intervallo che finisce prima di cominciare | `--range: "5-3" is not a range: it ends before it starts` |
| intervallo fuori dal file | `<path>: range N-M starts after the last subtitle: the file has K`, oppure `… ends after …` |

Gli errori della destinazione sono comuni ai sottocomandi che scrivono:
vedi [Invocazione](invocation.md#la-destinazione).
