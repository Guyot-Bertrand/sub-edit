# `framerate`

<!-- machine-translated -->
> Questa pagina è una traduzione automatica e non è ancora stata rivista da un lettore della lingua.

> **Questo non è [`snap`](snap.md).** Questo comando **scala il
> file**: serve quando la sincronizzazione è sbagliata e slitta di alcuni
> secondi verso la fine del film. `snap` riporta i marcatori temporali sulla griglia
> più vicina senza risincronizzare nulla, e serve quando la sincronizzazione è già
> giusta. Entrambi hanno la stessa forma, una frequenza dei fotogrammi o due, e scambiarli
> non provoca alcun errore.

```
subedit-cli framerate --from <fps> --to <fps>
                      (--output FILE | --output-dir DIR | --in-place)
                      [--dry-run]
                      [--recursive]
                      <file>...
```

Risincronizza un file sincronizzato per una frequenza dei fotogrammi su un'altra. È
l'operazione che richiede un file di sottotitoli quando è stato preso per una copia che
non è quella del film che si possiede — un DVD PAL a fronte di un trasferimento NTSC, per
esempio.

Il formato del file letto viene **mantenuto**: cambiare formato è compito di
[`convert`](convert.md).

<!-- example: subedit-cli framerate --help -->
```console
$ subedit-cli framerate --help
Re-time a file mastered at one frame rate for another
Usage: subedit-cli framerate [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to re-time

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --from TEXT REQUIRED        Frame rate the file is timed at: 25, 23.976
  --to TEXT REQUIRED          Frame rate to time it for: 24, 29.97
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Argomenti e opzioni

| Opzione | Obbligatorio | Valori | Predefinito |
| :----- | :----- | :------ | :----- |
| `<file>...` | sì | uno o più percorsi | — |
| `--recursive`, `-r` | no | un'opzione senza valore | disattivata — vedere [Elaborare un albero di directory](lots.md) |
| `--from` | **sì** | una frequenza dei fotogrammi, vedere più avanti | — |
| `--to` | **sì** | una frequenza dei fotogrammi, vedere più avanti | — |
| `--output` / `--output-dir` / `--in-place` | **una delle tre**, tranne con `--dry-run` | vedere [Invocazione](invocation.md#la-destinazione) | — |
| `--dry-run` | no | un'opzione senza valore: calcola e riferisce, non scrive nulla — vedere [Vedere prima di scrivere](invocation.md#vedere-prima-di-scrivere) | disattivata |

`--from` è la frequenza dei fotogrammi per cui il file **è** sincronizzato, `--to` quella
per cui lo si vuole. Scambiarle sposta il file in
senso opposto — circa sette secondi su un lungometraggio tra 25 e
23.976 — e nulla lo segnala finché il film non viene guardato.

## Scrivere una frequenza dei fotogrammi

In fotogrammi al secondo, decimali compresi, con il punto come separatore:

```
25      24      23.976      29.97      60
```

**Una frequenza nulla o negativa viene rifiutata**: non è una frequenza dei fotogrammi.

### Le tre frequenze NTSC non si possono scrivere esattamente

`23.976`, `29.97` e `59.94` sono **etichette**. Le frequenze dei fotogrammi che
designano sono `24000/1001`, `30000/1001` e `60000/1001`, nessuna delle quali ha
una scrittura decimale finita. Lo strumento le riconosce come tali:

| Che cosa è scritto | Che cosa designa |
| :------------- | :------------------ |
| `23.976` | `24000/1001` — cinema trasferito su video NTSC |
| `24` | `24` — cinema |
| `25` | `25` — video PAL e SECAM |
| `29.97` | `30000/1001` — video NTSC |
| `30`, `50`, `60` | se stesse |
| `59.94` | `60000/1001` |

Il riconoscimento riguarda **il valore e non le lettere**: `23.9760`
designa la stessa frequenza dei fotogrammi di `23.976`.

Qualsiasi altro decimale è preso alla lettera — `23.9` non è lo standard di
nessuno, non c'è nulla da leggervi in più. È anche ciò che rende necessaria la tabella qui sopra:
senza di essa, `23.976` sarebbe `23976/1000`, e la riga di comando
direbbe qualcosa di diverso dalla finestra per le stesse parole.

## Che cosa diventano le posizioni

```
t′ = round( t × (from / to) )
```

Un film masterizzato a 23.976 e riprodotto a 25 fotogrammi al secondo scorre più veloce,
quindi i suoi sottotitoli arrivano prima; nel senso opposto, arrivano
dopo.

**Il fattore è un razionale esatto, e l'arrotondamento avviene una sola volta.** Il
percorso che sembra equivalente — convertire la posizione in un numero di fotogramma alla
frequenza di ingresso, poi di nuovo in millisecondi alla frequenza di uscita — non
lo è: quantizza ogni posizione sulla griglia di ingresso e sbaglia fino a
mezzo fotogramma, 21 ms misurati. Vedere
[ADR 0013](../../../../adr/0013-mise-a-l-echelle-exacte-des-positions.md).

In pratica, da 25 a 23.976, il fattore è `1001/960`:

| Posizione letta | Scritta | Che cosa darebbe il passaggio per i fotogrammi |
| -----------: | -----: | -----------------------------------------: |
| `1 010 ms` | `1 053 ms` | `1 043 ms` |
| `3 600 017 ms` | `3 753 768 ms` | `3 753 750 ms` |

<!-- example: printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 25 --to 23.976 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 25 --to 23.976 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle retimed from 25 to 24000/1001 fps -> b.srt
1
00:00:01,053 --> 00:00:02,106
Un.
```

## Output

**Output standard** — nulla: il risultato è il file scritto. Con `--dry-run`, nulla neppure allora: non c'è alcun file, e la riga di resoconto lo dice — vedere [Vedere prima di scrivere](invocation.md#vedere-prima-di-scrivere).

| Livello | Che cosa si aggiunge sull'errore standard |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles retimed from 25 to 24000/1001 fps -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — il formato, la codifica, l'indicatore e i terminatori di riga del file letto, rimessi com'erano |
| 3 | `<path>: N bytes read, M written`, poi **ogni messaggio diagnostico di lettura** — vedere [Invocazione](invocation.md#diagnostica-di-lettura) |

**Il resoconto nomina la frequenza dei fotogrammi esatta, non l'etichetta digitata.** Scrivere
`23.976` su quella riga segnalerebbe una conversione che non è avvenuta; è
l'unico punto in cui si vede quale delle due è stata usata. Una frequenza che
un decimale scrive esattamente vi compare come decimale, le altre come frazione.

### In JSON

Con [`--format json`](invocation.md#un-output-che-uno-script-può-leggere), l'oggetto di ciascun file contiene `counts.subtitles` (ogni sottotitolo del file). La tabella completa è quella di [Invocazione](invocation.md#che-cosa-ogni-sottocomando-mette-in-counts).

## Codici di uscita

Quelli dello strumento: `0` se ogni file è stato scritto, `2` se nessuno, `3` se
solo alcuni, `1` in caso di errore d'uso.

Nessun rifiuto dipende dal contenuto di un file: due frequenze dei fotogrammi definiscono sempre
un fattore. È l'unico tra i sottocomandi che scrivono i cui
rifiuti avvengono tutti prima della prima lettura.

<!-- example: printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 0 --to 25 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 0 --to 25 --output b.srt a.srt; echo "code=$?"
"0" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed
code=1
```

## Errori

| Che cosa lo provoca | Messaggio |
| :------------------ | :------ |
| frequenza dei fotogrammi nulla o negativa | `"…" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed` |
| virgola decimale | `"…" is not a frame rate: use a decimal point and not a comma, in every language` |
| testo che non è un numero | `"…" is not a frame rate: expected frames per second, like 25 or 23.976` |
| frequenza dei fotogrammi sproporzionata | `"…" is not a frame rate: no video runs at that many frames per second` |

**Nulla viene scritto senza una delle tre opzioni di destinazione**, e i relativi
errori sono comuni ai sottocomandi che scrivono:
vedere [Invocazione](invocation.md#la-destinazione).
