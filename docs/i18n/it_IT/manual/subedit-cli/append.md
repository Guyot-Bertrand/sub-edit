# `append`

<!-- machine-translated -->
> Questa pagina è una traduzione automatica e non è ancora stata rivista da un lettore della lingua.

```
subedit-cli append --output FILE
                   [--dry-run] [--sort] [--disjoint]
                   <base> <file>...
```

Mette i file **dopo il primo**, in **un unico** file: il film in due parti, trasformato in un
unico insieme di sottotitoli. È `Append File…` della finestra — [lo stesso motore](../subedit-gui/operations.md#append-file),
gli stessi conteggi — applicato tante volte quanti sono i file da accodare.

- **Il primo file è la base.** Dà al risultato **il suo formato, la sua codifica, il suo indicatore dell'ordine
  dei byte e i suoi terminatori di riga**.
- **Nulla viene inserito tra due file**: ciascuno è spostato della **fine dell'ultimo sottotitolo di ciò che
  lo precede**, e le sue posizioni conservano l'ordine e la distanza reciproca. Il terzo segue il secondo, non la
  base: i file si susseguono nell'ordine in cui vengono dati.
- **Ogni file accodato passa attraverso la conversione** al formato della base: i suoi tag sono tradotti,
  e ciò che quel formato non può scrivere viene detto, file per file, con le parole della finestra.
- **I file accodati non hanno traduzione**: un file aperto da solo non ne ha da dare.
- **Un file può essere dato più volte**, o accodato a se stesso: nulla lo vieta.

**`append` non è un'elaborazione in blocco.** N input, **un** output: il ciclo degli altri sottocomandi — un risultato
per file, e un errore che lascia proseguire gli altri — qui non ha senso. Ne derivano tre cose:

- **la destinazione è `--output`, e nient'altro.** `--output-dir` e `--in-place` non esistono per
  `append` e vengono rifiutati come qualsiasi opzione sconosciuta, codice `1`; così anche `--recursive`: l'ordine dei
  file è quello dato, e un albero percorso non ne ha uno;
- **tutto o niente**: il primo file che non può essere letto interrompe l'esecuzione, e **nulla viene scritto**. Un
  film in tre parti di cui una manca non è un film in due;
- **una sola riga per esecuzione** per il risultato, e, in JSON, **un solo oggetto**.

<!-- example: subedit-cli append --help -->
```console
$ subedit-cli append --help
Put subtitle files one after another into a single file
Usage: subedit-cli append [OPTIONS] files...

Positionals:
  files BASE FILE...          The base file, then the files to put after it, in order

Options:
  -h,--help                   Print this help message and exit
  --output FILE               File to write
  --dry-run                   Work out and say what would be written, and write nothing
  --sort                      Put the subtitles in order of their start before writing
  --disjoint                  Cut overlapping subtitles so that no range overlaps another, before writing
```

## Argomenti e opzioni

| Opzione | Obbligatorio | Valori | Predefinito |
| :----- | :----- | :------ | :----- |
| `<base> <file>...` | sì, **almeno due** | percorsi: la base, poi ciò che viene messo dopo, in ordine | — |
| `--output` | **sì**, tranne con `--dry-run` | un percorso di file; la sua directory viene creata se manca | — |
| `--dry-run` | no | un'opzione senza valore: calcola e riferisce, non scrive nulla — vedi [Vedere prima di scrivere](invocation.md#vedere-prima-di-scrivere) | disattivata |
| `--sort` | no | un'opzione senza valore: mette l'insieme nell'ordine degli inizi **una volta accodato tutto**, vedi [Ordinare il risultato](#ordinare-il-risultato) | disattivata |
| `--disjoint` | no | un'opzione senza valore: taglia i sottotitoli sovrapposti **una volta accodato e ordinato tutto**, vedi [Rendere disgiunti gli intervalli](#rendere-disgiunti-gli-intervalli) | disattivata |

**La destinazione non può essere nessuno degli input**, base compresa: scrivere sopra un file che si sta
leggendo viene rifiutato prima di leggere qualsiasi cosa, codice `1`. Un `--output` esistente e distinto dagli input viene
sovrascritto, come ovunque.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n' > partie1.srt; printf '1\n00:00:00,500 --> 00:00:01,500\ntrois\n\n' > partie2.srt; subedit-cli append --output film.srt partie1.srt partie2.srt; cat film.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n' > partie1.srt; printf '1\n00:00:00,500 --> 00:00:01,500\ntrois\n\n' > partie2.srt; subedit-cli append --output film.srt partie1.srt partie2.srt; cat film.srt
partie2.srt: appended 1 subtitle
partie1.srt: 3 subtitles from 2 files -> film.srt
1
00:00:01,000 --> 00:00:02,000
un

2
00:00:03,000 --> 00:00:04,000
deux

3
00:00:04,500 --> 00:00:05,500
trois
```

L'ultimo sottotitolo di `partie1.srt` finisce a 4 s; quello di `partie2.srt`, che cominciava a 0,5 s, comincia a
4,5 s.

## Ordinare il risultato

Ogni file è spostato **a partire dalla fine di ciò che lo precede**: l'ordine che può rompersi è quindi quello dei file stessi. `--sort`
mette il risultato nell'ordine degli inizi, **in un colpo solo, dopo l'ultimo accodamento**, come farebbe [`sort`](sort.md) — in modo stabile, e senza cambiare nulla
del contenuto.

Mai per impostazione predefinita. Con l'opzione, la base lo dice al primo livello di narrazione, `<base>: N subtitles moved` oppure `<base>: already in order`, e
il record `--format json` contiene un conteggio `moved`, **assente senza l'opzione**.

## Rendere disgiunti gli intervalli

Mai per impostazione predefinita. L'opzione fa ciò che fa [`disjoint`](disjoint.md), **con le sue impostazioni predefinite**, una volta accodato tutto, sull'insieme. Richiede un file in ordine: altrimenti viene rifiutata, e `--sort` — che viene prima — lo mette in ordine in anticipo. Il file lo dice al primo livello di narrazione (`<path>: N subtitles split, M merged`), e il record `--format json` contiene i conteggi `split` e `merged`, **assenti senza l'opzione**.

## Quando i formati differiscono

Il risultato è nel **formato della base**, qualunque sia quello degli altri. Ciò che costa il passaggio è
detto file per file, con le parole della finestra — la stessa frase di `Append File…` e di
[incollare da un altro formato](../subedit-gui/clipboard.md#i-tag-di-un-altro-formato-vengono-tradotti):

```text
d.ass: appended 1 subtitle; Advanced SSA into SubRip: 1 tag dropped, the header was dropped
```

Ciò che si perde dipende solo dal formato di **ciascun** file accodato: una base in Advanced SSA riceve un
file SubRip senza perdere nulla, non il contrario. Per scegliere il formato del risultato, mettere per primo un file
che è già in quel formato, oppure convertire dopo con [`convert`](convert.md).

## Output

**Output standard** — nulla: il risultato è il file scritto.

| Livello | Che cosa si aggiunge sull'output degli errori |
| :----- | :------------------------------------- |
| 1 | una riga per file accodato — `<path>: appended N subtitles`, seguita, quando il formato è costato qualcosa, da `; <source format> into <base format>: <what is lost>` — poi `<base>: N subtitles from M files -> <destination>` (con `--dry-run`, `(dry run, nothing written)` al posto di `-> <destination>`) |
| 2 | `<base>: SubRip, UTF-8, no BOM, LF line endings kept` — il formato, la codifica, l'indicatore e i terminatori di riga della base, rimessi com'erano |
| 3 | `<base>: N bytes read, M written` (tutti i file insieme), poi **ogni diagnostica di lettura** — vedi [Invocazione](invocation.md#diagnostica-di-lettura) |

### In JSON

Con [`--format json`](invocation.md#un-output-che-uno-script-può-leggere), **un solo oggetto** per l'intera esecuzione,
il cui `file` è **la base**. I suoi `counts`:

| Conteggio | Che cosa dice |
| :----- | :----------- |
| `files` | i file dell'esecuzione, base compresa |
| `subtitles` | i sottotitoli del risultato |
| `appended` | quelli che provengono dai file accodati |
| `lost_ends`, `lost_header` | `0` oppure `1`: uno dei file accodati lo ha perso |
| `joined_lines`, `lost_tags`, `lost_fields` | la somma di ciò che i file accodati hanno perso |
| `furthest_ms` | il massimo spostamento di una posizione, tutti i file insieme |

**Un array `inputs`** accompagna i `counts`, all'esterno: un elemento per file accodato, in ordine,
`{"file", "counts"}` — con `counts` che contiene `appended`, poi le stesse perdite, **per quel solo file**. È ciò che
dice, in un'esecuzione di dieci file, quale è costato che cosa. Gli avvisi di lettura di tutti i file
sono raccolti in `warnings`.

Anche un errore è un oggetto, **uno solo**: quello del primo file che non ha potuto essere letto.

## Codici di uscita

| Codice | Quando |
| :--- | :---- |
| `0` | il risultato è scritto — oppure calcolato, con `--dry-run` |
| `1` | errore di utilizzo: meno di due file, nessun `--output`, un'opzione che `append` non ha, una destinazione che è uno degli input |
| `2` | **un file non ha potuto essere letto**, oppure la destinazione non ha potuto essere scritta: nulla viene scritto. Non esiste il codice `3`: un'esecuzione che scrive un solo file riesce oppure no |

## Errori

| Che cosa li provoca | Messaggio |
| :------------------- | :------ |
| un solo file dato | `append needs a base file and at least one file to put after it` |
| né `--output` né `--dry-run` | `no destination given: append writes one file, use --output` |
| `--output` indica un input | `<destination>: would be written over the input <input>` |
| un file mancante, illeggibile o di formato sconosciuto | `<path>: does not exist`, `<path>: is in no format this tool knows`… — gli stessi di ovunque |

**Un file vuoto non viene accodato**: nessun formato si legge senza almeno un sottotitolo, e il file viene
rifiutato come qualsiasi file che non può essere letto — `<path>: holds nothing recognisable as a subtitle` —, il che
interrompe l'esecuzione e non scrive nulla.
