# `repair`

<!-- machine-translated -->
> Questa pagina è una traduzione automatica e non è ancora stata rivista da un lettore della lingua.

```
subedit-cli repair (--output FILE | --output-dir DIR | --in-place)
                   [--minimum TIME] [--max-lines N] [--dashes]
                   [--dry-run] [--recursive]
                   <file>...
```

Fa in un colpo solo ciò che tre comandi fanno uno dopo l'altro: [`join-repeats`](join-repeats.md), poi
[`sort`](sort.md), poi [`disjoint`](disjoint.md). **In quest'ordine, che è l'unico che funziona**: il taglio legge
l'ordine degli inizi, ed è meglio che i pezzi ripetuti di uno stesso testo siano spariti prima. Su un file
segmentato male, fuori ordine e con sovrapposizioni, è il comando da eseguire.

Le impostazioni sono quelle di `disjoint`, che chiude la catena; `--sort` qui non esiste, perché l'ordinamento viene sempre eseguito.

<!-- example: subedit-cli repair --help -->
```console
$ subedit-cli repair --help
Join the repeated subtitles, put them in order, then make the ranges disjoint
Usage: subedit-cli repair [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to repair

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --minimum TIME              Shortest interval kept; a shorter one is taken over by a neighbour (default 0.08)
  --max-lines N               Most lines of a merged subtitle; more are shown in windows, 0 for no limit (default 2)
  --dashes                    Put a dash before the text of each merged subtitle
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Argomenti e opzioni

| Opzione | Obbligatoria | Valori | Predefinito |
| :----- | :----- | :------ | :----- |
| `<file>...` | sì | uno o più percorsi | — |
| `--minimum` | no | una durata: `0.08`, `1.5`, `00:00:00,080`. Zero conserva ogni intervallo | `0.08` (circa due fotogrammi) |
| `--max-lines` | no | un numero intero; `0` toglie il limite | `2` |
| `--dashes` | no | un'opzione senza valore: un trattino davanti al testo che ogni sottotitolo porta a un sottotitolo unito | disattivata |
| `--recursive`, `-r` | no | un'opzione senza valore | disattivata — vedi [Elaborare un albero di directory](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **una delle tre**, tranne con `--dry-run` | vedi [Invocazione](invocation.md#la-destinazione) | — |
| `--dry-run` | no | un'opzione senza valore: calcola e dice, non scrive nulla — vedi [Vedere prima di scrivere](invocation.md#vedere-prima-di-scrivere) | disattivata |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:01,000 --> 00:00:04,000\nBeta\n\n' > segments.srt; subedit-cli repair --output repare.srt segments.srt; cat repare.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:01,000 --> 00:00:04,000\nBeta\n\n' > segments.srt; subedit-cli repair --output repare.srt segments.srt; cat repare.srt
segments.srt: 1 repeated subtitle joined; already in order; 0 subtitles split, 1 merged -> repare.srt
1
00:00:01,000 --> 00:00:04,000
Alpha
Beta
```

`Alpha`, tagliato in due pezzi che si susseguono, torna a essere un unico sottotitolo; occupa allora lo stesso intervallo di `Beta`,
e i due ne formano uno solo, di due righe, il più antico per primo.

## Output

**Output standard** — nulla: il risultato è il file scritto. Sull'output di errore, al primo livello, **una frase
che concatena i tre conteggi**, separati da punti e virgola: ciò che `join-repeats`, `sort` e `disjoint` dicono ciascuno,
con le proprie parole — `N repeated subtitles joined; M subtitles moved; K subtitles split, L merged…`.

### In JSON

Con [`--format json`](invocation.md#un-output-che-uno-script-può-leggere), i `counts` di ogni file: `subtitles`, `joined`,
`moved`, poi quelli di [`disjoint`](disjoint.md#in-json) (`split`, `merged`, `windowed`, `absorbed`, `too_short`,
`ending_before_start`, `extras_dropped`).

## Codici di uscita ed errori

Quelli dello strumento. Nessun errore è specifico di `repair` oltre alle impostazioni, che vengono giudicate come quelle di `disjoint`:
`--minimum` che non è una durata o è negativo, `--max-lines` che non è un numero intero, codice `1`.
