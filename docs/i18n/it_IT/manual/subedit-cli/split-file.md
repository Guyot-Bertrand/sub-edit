# `split-file`

<!-- machine-translated -->
> Questa pagina è una traduzione automatica e non è ancora stata rivista da un lettore della lingua.

```
subedit-cli split-file --at N --head FILE --tail FILE
                       [--dry-run] [--sort] [--disjoint]
                       <file>
```

Taglia un file **in due**: una testa e una coda. È `Split Project…` della finestra — [lo stesso motore](../subedit-gui/operations.md#split-project),
gli stessi conteggi, lo stesso rifiuto — e **l'inverso esatto di [`append`](append.md)**: accodare la coda alla
testa restituisce il file di partenza.

- **`--at N` è il numero del primo sottotitolo della coda**, numerato come lo numera la tabella
  della finestra: da 1. `--at 3` mette i sottotitoli 1 e 2 nella testa, e il terzo e i successivi nella
  coda. Va **dal secondo all'ultimo**: un taglio richiede un sottotitolo da ogni lato.
- **La coda viene riportata all'origine**: viene spostata della fine dell'ultimo sottotitolo rimasto nella testa, con segno negativo,
  e le sue posizioni conservano il loro ordine e la loro distanza. È questo che fa di `split-file` l'inverso di `append`.
- **Entrambe le metà conservano il formato, la codifica, l'indicatore dell'ordine dei byte e i terminatori di riga del file letto**, e
  vengono rinumerate da 1 dalla scrittura, come qualsiasi file riscritto.
- **Due output, quindi una destinazione propria**: `--head` e `--tail`, **entrambe obbligatorie**. `--output`,
  `--output-dir`, `--in-place` e `--recursive` non esistono per `split-file` e vengono rifiutate come qualsiasi
  opzione sconosciuta, codice `1`: non c'è né una directory in cui far confluire due file con nome, né un input da sovrascrivere.

<!-- example: subedit-cli split-file --help -->
```console
$ subedit-cli split-file --help
Cut a subtitle file in two, a head and a tail
Usage: subedit-cli split-file [OPTIONS] file

Positionals:
  file TEXT REQUIRED          Subtitle file to cut

Options:
  -h,--help                   Print this help message and exit
  --at N                      Number of the first subtitle of the tail, as the table numbers them
  --head FILE                 File to write the subtitles before the cut to
  --tail FILE                 File to write the subtitles from the cut on to
  --dry-run                   Work out and say what would be written, and write nothing
  --sort                      Put the subtitles in order of their start before cutting; --at counts in that order
  --disjoint                  Cut overlapping subtitles so that no range overlaps another, before cutting
```

## Argomenti e opzioni

| Opzione | Obbligatoria | Valori | Predefinito |
| :----- | :----- | :------ | :----- |
| `<file>` | sì | **un solo** percorso | — |
| `--at` | sì | un numero intero, da 2 al numero di sottotitoli del file | — |
| `--head`, `--tail` | **sì, entrambe**, tranne con `--dry-run` | percorsi di file; la loro directory viene creata se manca | — |
| `--dry-run` | no | un'opzione senza valore: calcola e dice, non scrive nulla — vedi [Vedere prima di scrivere](invocation.md#vedere-prima-di-scrivere) | disattivata |
| `--sort` | no | un'opzione senza valore: mette i sottotitoli nell'ordine del loro inizio **prima di tagliare**; `--at` conta in quell'ordine | disattivata |
| `--disjoint` | no | un'opzione senza valore: taglia i sottotitoli sovrapposti **prima di tagliare**; `--at` conta in ciò che resta, vedi [Rendere disgiunti gli intervalli](#rendere-disgiunti-gli-intervalli) | disattivata |

**Con `--dry-run`, nessuna destinazione è obbligatoria**: non se ne può indicare alcuna, ma non una sola delle due —
`--head` e `--tail` vanno insieme. Quelle indicate vengono giudicate come senza `--dry-run`.

**Le due destinazioni non possono essere né l'input né lo stesso file**: scrivere sopra ciò che si sta
leggendo, o scrivere la coda sopra la testa, viene rifiutato prima che venga letto qualsiasi cosa, codice `1`.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n3\n00:00:06,000 --> 00:00:07,000\ntrois\n\n4\n00:00:08,000 --> 00:00:09,000\nquatre\n\n' > film.srt; subedit-cli split-file --at 3 --head tete.srt --tail queue.srt film.srt; cat queue.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n3\n00:00:06,000 --> 00:00:07,000\ntrois\n\n4\n00:00:08,000 --> 00:00:09,000\nquatre\n\n' > film.srt; subedit-cli split-file --at 3 --head tete.srt --tail queue.srt film.srt; cat queue.srt
film.srt: split at subtitle 3: 2 subtitles in the head, 2 subtitles in the tail -> tete.srt, queue.srt
1
00:00:02,000 --> 00:00:03,000
trois

2
00:00:04,000 --> 00:00:05,000
quatre
```

Il secondo sottotitolo, rimasto nella testa, termina a 4 s: la coda torna indietro di 4 s, e il terzo sottotitolo,
che iniziava a 6 s, inizia a 2 s.

## Tagliare in ordine temporale

`--at` numera i sottotitoli **come li dà il file**. Su un file il cui ordine è rotto, la testa non è quindi la metà che viene per prima
nel tempo. `--sort` mette i sottotitoli nell'ordine dei loro inizi **prima di giudicare il taglio** — in modo stabile, come fa [`sort`](sort.md) —, e
`--at` conta poi in quell'ordine.

Mai per impostazione predefinita. Con l'opzione, il file dice al primo livello di narrazione quanti sottotitoli si sono spostati (`<path>: N subtitles moved`
oppure `<path>: already in order`), e il record di `--format json` porta un conteggio `moved`, **assente senza l'opzione**.

## Rendere disgiunti gli intervalli

Mai per impostazione predefinita. L'opzione fa ciò che fa [`disjoint`](disjoint.md), **con le sue impostazioni predefinite**, prima di giudicare il taglio, dopo l'ordinamento: `--at` conta in ciò che ne risulta. Richiede un file in ordine: altrimenti viene rifiutata, e `--sort` — che viene prima — lo mette prima in ordine. Il file lo dice al primo livello di narrazione (`<path>: N subtitles split, M merged`), e il record di `--format json` porta i conteggi `split` e `merged`, **assenti senza l'opzione**.

## Quando il taglio viene rifiutato

**Due metà che si sovrappongono al taglio non vengono tagliate.** Se un sottotitolo della coda iniziava prima
della fine dell'ultimo sottotitolo lasciato indietro, lo spostamento lo porterebbe prima dell'inizio del video — una posizione negativa,
che nessun formato scrive. `split-file` rifiuta **indicando il sottotitolo**, con le parole della finestra:

```text
o.srt: Cannot split at subtitle 3: subtitle 3 would fall before the start of the video. Cut somewhere else.
```

Il codice è `2` e **nulla viene scritto**. Tagliare altrove. Cadere esattamente sull'inizio è ammesso: zero è una
posizione.

**`--at` fuori dal file viene rifiutato allo stesso modo**, una volta letto il file: `--at 1` — il primo sottotitolo non può
iniziare una coda, non resterebbe nulla — e `--at` oltre l'ultimo sottotitolo, che lascerebbe una coda
vuota. Un file di un solo sottotitolo non può essere tagliato.

## Output

**Output standard** — nulla: il risultato sono i due file scritti.

| Livello | Che cosa si aggiunge sull'output di errore |
| :----- | :------------------------------------- |
| 1 | `<path>: split at subtitle N: H subtitles in the head, T subtitles in the tail -> <head>, <tail>`; con `--dry-run`, `(dry run, nothing written)` al posto di `-> …` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — il formato, la codifica, l'indicatore e i terminatori di riga del file letto, rimessi com'erano |
| 3 | `<path>: N bytes read, H and T written`, poi **ogni diagnostica di lettura** — vedi [Invocazione](invocation.md#diagnostica-di-lettura) |

### In JSON

Con [`--format json`](invocation.md#un-output-che-uno-script-può-leggere), **un solo oggetto**, il cui `file` è il file tagliato.
**`destination` è la testa** — il file in cui diventa l'input — e **`tail` la coda**, un percorso, oppure `null` con
`--dry-run` come `destination`. I `counts`: `subtitles` (quelli del file letto), `head` e `tail` (quelli di ciascuna
metà).

## Codici di uscita

| Codice | Quando |
| :--- | :---- |
| `0` | entrambi i file sono scritti — o calcolati, con `--dry-run` |
| `1` | errore d'uso: mancano `--head` e `--tail`, un'opzione che `split-file` non ha, una destinazione che è l'input o l'altra destinazione, un `--at` che non è un numero intero positivo |
| `2` | il file non ha potuto essere letto, **il taglio è rifiutato** (fuori dal file, o sovrapposizione), oppure una destinazione non ha potuto essere scritta. Non esiste il codice `3`: viene elaborato un solo file |

**La testa viene scritta per prima**: un disco che rifiuta la coda lascia la testa dietro di sé, e il codice è `2`.

## Errori

| Che cosa li provoca | Messaggio |
| :------------------- | :------ |
| né `--head` né `--tail` | `no destination given: split-file writes two files, use --head and --tail` |
| una sola delle due, con `--dry-run` | `--head and --tail go together: give both, or neither with --dry-run` |
| le due destinazioni sono lo stesso file | `<path>: named as both the head and the tail` |
| una destinazione è l'input | `<path>: would be written over the input <input>` |
| `--at` fuori dal file | `<file>: --at 9: the file holds 3 subtitles, and the tail can start from 2 to 3` |
| un file di un solo sottotitolo | `<file>: a file of 1 subtitle cannot be split: a cut needs a subtitle on each side` |
| sovrapposizione | `<file>: Cannot split at subtitle N: subtitle M would fall before the start of the video. Cut somewhere else.` |
| un file mancante, illeggibile o di formato sconosciuto | `<path>: does not exist`… — lo stesso di ovunque |
