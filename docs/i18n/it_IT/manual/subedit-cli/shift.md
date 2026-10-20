# `shift`

<!-- machine-translated -->
> Questa pagina è una traduzione automatica e non è ancora stata rivista da un lettore della lingua.

```
subedit-cli shift (--by <time> | --to-grid)
                  (--output FILE | --output-dir DIR | --in-place)
                  [--dry-run]
                  [--recursive]
                  <file>...
```

Sposta **ogni** posizione del file della stessa quantità, in un senso o
nell'altro. Nessun sottotitolo resta sullo schermo né più a lungo né meno a lungo:
è questo che fa di uno spostamento uno spostamento.

<!-- example: subedit-cli shift --help -->
```console
$ subedit-cli shift --help
Move every position of a file by a fixed amount
Usage: subedit-cli shift [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to shift

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --by TEXT                   Amount to move by: 2.999, -7.001, or 00:00:07.001
  --to-grid                   Move by the amount that puts the positions back on their frame grid
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
| `--by` | **una delle due** | una durata con segno, vedi sotto | — |
| `--to-grid` | **una delle due** | opzione senza valore: la quantità viene misurata, vedi sotto | — |
| `--output` / `--output-dir` / `--in-place` | **una delle tre**, tranne con `--dry-run` | vedi [Invocazione](invocation.md#la-destinazione) | — |
| `--dry-run` | no | un'opzione senza valore: calcola e dice, non scrive nulla — vedi [Vedere prima di scrivere](invocation.md#vedere-prima-di-scrivere) | disattivata |

Il formato del file letto viene **conservato**: cambiare formato è compito di
[`convert`](convert.md).

## `--to-grid`: la quantità misurata anziché indicata

Un file le cui posizioni sono su una griglia di fotogrammi **a meno di una
costante** è stato spostato, e quella costante si può misurare sulle posizioni stesse.
`--to-grid` la misura e la applica, senza che nessuno debba conoscerla.

La quantità è **propria di ogni file**: due file spostati fuori dalla stessa
griglia di due quantità diverse tornano ciascuno con la propria. Viene
scritta nella narrazione, insieme alla frequenza dei fotogrammi trovata.

**Il percorso è il più breve.** Oltre mezzo fotogramma, la linea più vicina della griglia
è la successiva: la correzione va in avanti anziché tornare indietro di
quasi un intero fotogramma.

<!-- example: k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40 + 7)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > decale.srt; subedit-cli shift --to-grid --output recale.srt decale.srt -->
```console
$ k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40 + 7)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > decale.srt; subedit-cli shift --to-grid --output recale.srt decale.srt
decale.srt: 12 subtitles shifted by -0.007 s onto their 25 fps grid -> recale.srt
```

**Un file su nessuna griglia nota viene rifiutato, con la spiegazione.** Non c'è
nulla da ricondurre, e una fase misurata sul rumore sposterebbe il file
a caso. Vedi [`inspect`](inspect.md#che-cosa-segnala-frame-rate-grid) per
sapere che cosa fornisce un file prima di tentare l'operazione.

`--by` e `--to-grid` dicono entrambi di quanto spostare: indicarli insieme
è un errore d'uso, e lo è anche non indicarne nessuno.

**`--to-grid` non è [`snap`](snap.md).** Sposta l'intero file
della stessa quantità e conserva **esattamente** la sincronizzazione relativa; `snap`
sposta ogni posizione in modo indipendente e assorbe strada facendo quelle che erano
state corrette a mano. Su un file pulito i due danno lo stesso
risultato; su un file parziale, no.

## Scrivere una durata

Due forme, e soltanto due:

| Forma | Esempi |
| :---- | :------- |
| secondi | `30`, `2.999`, `-7.001`, `+1.5` |
| marca temporale | `00:00:07.001`, `01:39:37.040`, `-00:00:07.001` |

Dicono entrambe la stessa cosa: `--by 7.001` e `--by 00:00:07.001` producono
lo stesso file.

**Il separatore decimale è il punto**, perché la riga di comando parla inglese.
Una virgola viene rifiutata anziché interpretata: metà del mondo scrive `7,001`
per settemilauno, e leggerlo come sette secondi sarebbe un'ipotesi —
sbagliata per quella metà.

**La risoluzione è il millisecondo.** Una risoluzione più fine viene rifiutata anziché arrotondata
in silenzio: un arrotondamento tacito darebbe un risultato che non corrisponde a nulla di ciò che
era stato chiesto.

<!-- example: printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle shifted by -7.001 s -> b.srt
1
00:00:02,999 --> 00:00:04,999
Bonjour.
```

## Uno spostamento prima dell'origine viene rifiutato

Tornare indietro oltre lo zero dà il codice `2` e **non scrive nulla**. Il messaggio indica
il sottotitolo che non ha potuto subire lo spostamento, e dice così allo stesso tempo
fin dove si può arrivare.

Il motivo non è un principio: una posizione prima dell'origine si scrive
`-00:00:01,000`, una forma che nessuno dei nove formati definisce e che nessun lettore
accetta. Il nucleo ammette tali posizioni — rifiutarle farebbe di
un'operazione di modifica un caso particolare — ma un file è una cosa diversa da un
modello in memoria.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; echo "code=$?"
a.srt: subtitle 1 would start before the origin, which no subtitle file can hold
code=2
```

## Output

**Output standard** — nulla: il risultato è il file scritto. Con `--dry-run`, nulla neppure: non c'è alcun file, e la riga di narrazione lo dice — vedi [Vedere prima di scrivere](invocation.md#vedere-prima-di-scrivere).

| Livello | Che cosa si aggiunge sull'output di errore |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles shifted by 2.999 s -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — il formato, la codifica, l'indicatore e i terminatori di riga del file letto, rimessi com'erano |
| 3 | `<path>: N bytes read, M written`, poi **ogni diagnostica di lettura** — vedi [Invocazione](invocation.md#diagnostica-di-lettura) |

### In JSON

Con [`--format json`](invocation.md#un-output-che-uno-script-può-leggere), l'oggetto di ogni file porta `counts.subtitles` (ogni sottotitolo del file) e `counts.shifted_by_ms` (lo spostamento, in millisecondi interi, **con segno**). La tabella completa è quella di [Invocazione](invocation.md#che-cosa-ogni-sottocomando-mette-in-counts).

## Codici di uscita

Quelli dello strumento: `0` se ogni file è stato scritto, `2` se nessuno, `3` se
solo alcuni, `1` in caso di errore d'uso.

## Errori

| Che cosa lo provoca | Messaggio |
| :------------------ | :------ |
| virgola decimale | `"…" is not a time: use a decimal point and not a comma, in every language` |
| più fine del millisecondo | `"…" is not a time: positions are held to the millisecond, no finer` |
| testo che non è una durata | `"…" is not a time: expected seconds like 2.999, or a timestamp like 00:00:07.001` |
| spostamento prima dell'origine | `<path>: subtitle N would start before the origin, which no subtitle file can hold` |

**Nulla viene scritto senza una delle tre opzioni di destinazione**, e i relativi
errori sono comuni ai sottocomandi che scrivono:
vedi [Invocazione](invocation.md#la-destinazione).
