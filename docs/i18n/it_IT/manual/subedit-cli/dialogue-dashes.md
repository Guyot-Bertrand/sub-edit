# `dialogue-dashes`

<!-- machine-translated -->
> Questa pagina è una traduzione automatica e non è ancora stata rivista da un lettore della lingua.

```
subedit-cli dialogue-dashes (--add | --remove) [--range N-M|N-]
                            (--output FILE | --output-dir DIR | --in-place)
                            [--dry-run]
                            [--recursive]
                            [--document main|translation] [-t FILE] [--align-method position|number]
                            <file>...
```

Mette un **trattino di dialogo** all'inizio delle righe, o lo toglie. È `Dialogue Dashes` della
[finestra](../subedit-gui/operations.md).

**Due parole, come per [`italics`](italics.md)**: `--add` e `--remove` si escludono a vicenda, e ne serve una.

- **`--add`** mette un trattino davanti a ogni riga che non ne ha: `Bonjour` dà `- Bonjour`, e
  `Bonjour\nMarie` dà `- Bonjour\n- Marie`. **Un trattino già presente non viene raddoppiato.**
- **Si riconoscono tre trattini** — il trattino della tastiera e i due trattini lunghi del tipografo —
  **e viene scritto il trattino semplice**: `— Bonjour` diventa `- Bonjour`. Gli spazi in più
  dopo il trattino scompaiono.
- Il trattino va **davanti al testo, non davanti a un tag di apertura**: `<i>Bonjour</i>` dà
  `<i>- Bonjour</i>`. Nessun tag si sposta.
- **`--remove`** toglie i trattini iniziali, in tutte e tre le forme.

<!-- example: subedit-cli dialogue-dashes --help -->
```console
$ subedit-cli dialogue-dashes --help
Put dialogue dashes on the lines, or take them off
Usage: subedit-cli dialogue-dashes [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --add                       Put a dialogue dash at the head of the lines
  --remove                    Take the dialogue dashes off
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

## Argomenti e opzioni

| Opzione | Obbligatorio | Valori | Predefinito |
| :----- | :----- | :------ | :----- |
| `<file>...` | sì | uno o più percorsi | — |
| `--recursive`, `-r` | no | un'opzione senza valore | disattivata — vedi [Elaborare un albero di directory](lots.md) |
| `--add` / `--remove` | **una delle due** | opzioni senza valore | — |
| `--range` | no | `N-M`, oppure `N-` fino alla fine — vedi [`adjust`](adjust.md#--range) | l'intero file |
| `--document` | no | `main` oppure `translation` — vedi [Una traduzione](invocation.md#una-traduzione) | `main` |
| `-t`, `--translation-file` | con `--document translation` | un file di traduzione, per **un solo** input | — |
| `--align-method` | no, e solo con `-t` | `position` oppure `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **una delle tre**, tranne con `--dry-run` | vedi [Invocazione](invocation.md#la-destinazione) | — |
| `--dry-run` | no | un'opzione senza valore: calcola e riferisce, non scrive nulla — vedi [Vedere prima di scrivere](invocation.md#vedere-prima-di-scrivere) | disattivata |

**Il testo principale per impostazione predefinita; la traduzione con `--document translation -t FILE`.** È allora
la traduzione a essere modificata e scritta, e solo quella — vedi [Una traduzione](invocation.md#una-traduzione).

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\nMarie\n\n2\n00:00:03,000 --> 00:00:04,000\n<i>Salut</i>\n\n' > a.srt; subedit-cli dialogue-dashes --add --output t.srt a.srt; cat t.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\nMarie\n\n2\n00:00:03,000 --> 00:00:04,000\n<i>Salut</i>\n\n' > a.srt; subedit-cli dialogue-dashes --add --output t.srt a.srt; cat t.srt
a.srt: 2 subtitles dashed -> t.srt
1
00:00:01,000 --> 00:00:02,000
- Bonjour
- Marie

2
00:00:03,000 --> 00:00:04,000
<i>- Salut</i>
```

## Output

**Output standard** — nulla: il risultato è il file scritto. Con `--dry-run`, l'elenco delle
modifiche, descritto in [Vedere prima di scrivere](invocation.md#vedere-prima-di-scrivere).

| Livello | Che cosa si aggiunge sull'output degli errori |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles dashed -> <destination>`, oppure `N subtitles undashed`, oppure `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — il formato, la codifica, l'indicatore e i terminatori di riga del file letto, rimessi com'erano |
| 3 | `<path>: N bytes read, M written`, poi **ogni diagnostica di lettura** — vedi [Invocazione](invocation.md#diagnostica-di-lettura) |

`N` è il numero di sottotitoli **il cui testo è cambiato**, ricavato dall'operazione stessa.

### In JSON

Con [`--format json`](invocation.md#un-output-che-uno-script-può-leggere), l'oggetto di ciascun file contiene
`counts.changed` — l'`N` della frase — e `changes`, l'elenco dei sottotitoli modificati
(`{"subtitle", "document", "before", "after"}`, **vuoto** quando nulla è cambiato).

## Codici di uscita

Quelli dello strumento: `0` se ogni file è stato elaborato — un file in cui nulla cambia lo è —,
`2` se nessuno, `3` se solo alcuni, `1` per un errore di utilizzo.

## Errori

| Che cosa lo provoca | Messaggio |
| :------------------ | :------ |
| né `--add` né `--remove` | `dialogue-dashes needs --add to put dashes on, or --remove` |
| entrambi | `--add and --remove say opposite things; give one of them` |
| intervallo scritto male o fuori dal file | vedi [`adjust`](adjust.md#--range) |

Gli errori della destinazione sono comuni ai sottocomandi che scrivono:
vedi [Invocazione](invocation.md#la-destinazione).
