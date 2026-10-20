# `case`

<!-- machine-translated -->
> Questa pagina è una traduzione automatica e non è ancora stata rivista da un lettore della lingua.

```
subedit-cli case --to title|sentence|upper|lower [--range N-M|N-]
                 (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 [--document main|translation] [-t FILE] [--align-method position|number]
                 <file>...
```

Mette i testi in **iniziali maiuscole, prima lettera maiuscola, maiuscolo o minuscolo**, senza toccare alcun
tag. È `Strumenti ▸ Maiuscole e minuscole` della [finestra](../subedit-gui/operations.md), con le sue regole.

| `--to` | Effetto | `bonjour marie` diventa |
| :----- | :---- | :---------------------- |
| `title` | ogni parola prende la maiuscola, il resto passa in minuscolo | `Bonjour Marie` |
| `sentence` | la prima lettera del testo, e solo quella; il resto passa in minuscolo | `Bonjour marie` |
| `upper` | tutto in maiuscolo | `BONJOUR MARIE` |
| `lower` | tutto in minuscolo | `bonjour marie` |

- **Ciò che precede la prima lettera o la prima cifra non viene toccato**: un trattino di
  dialogo, un virgolettato di apertura o una parentesi restano (`- bonjour` dà `- Bonjour`).
- **I tag non sono testo**: non cambiano maiuscole e minuscole — `<I>` resta `<I>` — e non
  cambiano posto: un tag che taglia una parola la taglia ancora dopo.
- **ICU decide dove comincia una parola**: `l'été sera chaud` dà `L'été Sera Chaud`, e gli accenti
  restano (`éléonore` dà `Éléonore`).

<!-- example: subedit-cli case --help -->
```console
$ subedit-cli case --help
Put the texts in title, sentence, upper or lower case, tags intact
Usage: subedit-cli case [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --to title|sentence|upper|lower
                              The case to put the texts in
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
| `--to` | **sì** | `title`, `sentence`, `upper`, `lower` | — |
| `--range` | no | `N-M`, oppure `N-` fino alla fine — vedi [`adjust`](adjust.md#--range) | l'intero file |
| `--document` | no | `main` oppure `translation` — vedi [Una traduzione](invocation.md#una-traduzione) | `main` |
| `-t`, `--translation-file` | con `--document translation` | un file di traduzione, per **un solo** input | — |
| `--align-method` | no, e solo con `-t` | `position` oppure `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **una delle tre**, tranne con `--dry-run` | vedi [Invocazione](invocation.md#la-destinazione) | — |
| `--dry-run` | no | un'opzione senza valore: calcola e riferisce, non scrive nulla — vedi [Vedere prima di scrivere](invocation.md#vedere-prima-di-scrivere) | disattivata |

**Il testo principale per impostazione predefinita; la traduzione con `--document translation -t FILE`.** È allora
la traduzione a essere modificata e scritta, e solo quella — vedi [Una traduzione](invocation.md#una-traduzione).

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\n- BONJOUR MARIE\n\n' > a.srt; subedit-cli case --to title --output titre.srt a.srt; cat titre.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\n- BONJOUR MARIE\n\n' > a.srt; subedit-cli case --to title --output titre.srt a.srt; cat titre.srt
a.srt: 2 subtitles recased -> titre.srt
1
00:00:01,000 --> 00:00:02,000
<i>Bon</i>jour Marie

2
00:00:03,000 --> 00:00:04,000
- Bonjour Marie
```

Un file in cui **nessun testo cambia** — tutto è già nelle maiuscole e minuscole richieste — viene scritto così com'è e
lo dice:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBONJOUR\n\n' > a.srt; subedit-cli case --to upper --output b.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBONJOUR\n\n' > a.srt; subedit-cli case --to upper --output b.srt a.srt
a.srt: nothing to change -> b.srt
```

## Output

**Output standard** — nulla: il risultato è il file scritto. Con `--dry-run`, l'elenco delle
modifiche, descritto in [Vedere prima di scrivere](invocation.md#vedere-prima-di-scrivere).

| Livello | Che cosa si aggiunge sull'output degli errori |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles recased -> <destination>`, oppure `nothing to change` |
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
| `--to` mancante | `--to is required` |
| un valore che non è uno dei quattro | `--to: … not in {title,sentence,upper,lower}` |
| intervallo scritto male o fuori dal file | vedi [`adjust`](adjust.md#--range) |

Gli errori della destinazione sono comuni ai sottocomandi che scrivono:
vedi [Invocazione](invocation.md#la-destinazione).
