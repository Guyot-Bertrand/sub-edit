# `italics`

<!-- machine-translated -->
> Questa pagina è una traduzione automatica e non è ancora stata rivista da un lettore della lingua.

```
subedit-cli italics (--on | --off) [--range N-M|N-]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    <file>...
```

Mette i testi **in corsivo**, oppure **ne toglie il corsivo**, nei tag del formato del file.
È il comando `Italic` della [finestra](../subedit-gui/operations.md).

**Due parole e non un interruttore**: il pulsante della finestra chiede al nucleo in quale
verso agire; qui lo si dice. `--on` e `--off` si escludono a vicenda, e ne serve uno — ometterlo, o darli entrambi,
è un errore d'uso (codice `1`).

- **`--on`** racchiude ogni testo in un'unica coppia di tag, **dopo aver rimosso quelli che
  c'erano**: un testo già in corsivo non viene racchiuso due volte, e un corsivo spezzato in due
  viene rifatto in un unico pezzo. Su più righe, una coppia copre l'insieme.
- **`--off`** rimuove i tag del corsivo; **il resto di un tag viene mantenuto** (`{\b1\i1}` dà
  `{\b1}`).
- Il testo viene scritto **come il formato del file scrive il corsivo**: `<i>…</i>` per SubRip, WebVTT e
  SubViewer 2, `{\i1}…{\i0}` per i due Sub Station Alpha, `{Y:i}` per MicroDVD.

<!-- example: subedit-cli italics --help -->
```console
$ subedit-cli italics --help
Put the texts in italics, or take their italics out
Usage: subedit-cli italics [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --on                        Put the texts in italics
  --off                       Take the italics out
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
| `--recursive`, `-r` | no | un'opzione senza valore | disattivata — vedere [Elaborare un albero di directory](lots.md) |
| `--on` / `--off` | **una delle due** | opzioni senza valore | — |
| `--range` | no | `N-M`, oppure `N-` fino alla fine — vedere [`adjust`](adjust.md#--range) | l'intero file |
| `--document` | no | `main` o `translation` — vedere [Una traduzione](invocation.md#una-traduzione) | `main` |
| `-t`, `--translation-file` | con `--document translation` | un file di traduzione, per **un solo** input | — |
| `--align-method` | no, e solo con `-t` | `position` o `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **una delle tre**, tranne con `--dry-run` | vedere [Invocazione](invocation.md#la-destinazione) | — |
| `--dry-run` | no | un'opzione senza valore: calcola e riferisce, non scrive nulla — vedere [Vedere prima di scrivere](invocation.md#vedere-prima-di-scrivere) | disattivata |

**Per impostazione predefinita il testo principale; la traduzione con `--document translation -t FILE`.** In tal caso è
la traduzione a essere modificata e scritta, e soltanto essa — vedere [Una traduzione](invocation.md#una-traduzione).

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour\nMarie\n\n' > a.srt; subedit-cli italics --on --output i.srt a.srt; cat i.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour\nMarie\n\n' > a.srt; subedit-cli italics --on --output i.srt a.srt; cat i.srt
a.srt: 2 subtitles put in italics -> i.srt
1
00:00:01,000 --> 00:00:02,000
<i>bonjour marie</i>

2
00:00:03,000 --> 00:00:04,000
<i>Bonjour
Marie</i>
```

## Un formato senza stile rifiuta

**TMPlayer e LRC non scrivono alcuno stile**: non c'è corsivo da applicare né da togliere. La
finestra mostra la voce in grigio invece di nasconderla; la riga di comando **rifiuta, file per
file, indicandone il motivo**, e gli altri file del lotto proseguono. Conta ciò che il
formato può portare, non il modo in cui lo scrive: SubRip e WebVTT scrivono il corsivo allo stesso
modo, e non dicono la stessa cosa a proposito del colore.

<!-- example: printf '00:00:01:Hello there\n00:00:03:Bye\n' > a.txt; subedit-cli italics --on --output-dir sortie a.txt; echo "code=$?" -->
```console
$ printf '00:00:01:Hello there\n00:00:03:Bye\n' > a.txt; subedit-cli italics --on --output-dir sortie a.txt; echo "code=$?"
a.txt: TMPlayer writes no style: there are no italics to put on
code=2
```

In JSON, l'errore riporta l'identificatore **`no-style`**.

## Output

**Output standard** — nulla: il risultato è il file scritto. Con `--dry-run`, l'elenco delle
modifiche, descritto in [Vedere prima di scrivere](invocation.md#vedere-prima-di-scrivere).

| Livello | Che cosa si aggiunge sull'errore standard |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles put in italics -> <destination>`, oppure `N subtitles taken out of italics`, oppure `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — il formato, la codifica, l'indicatore e i terminatori di riga del file letto, rimessi com'erano |
| 3 | `<path>: N bytes read, M written`, poi **ogni messaggio diagnostico di lettura** — vedere [Invocazione](invocation.md#diagnostica-di-lettura) |

`N` è il numero di sottotitoli **il cui testo è cambiato**, ricavato dall'operazione stessa.

### In JSON

Con [`--format json`](invocation.md#un-output-che-uno-script-può-leggere), l'oggetto di ciascun file contiene
`counts.changed` — l'`N` della frase — e `changes`, l'elenco dei sottotitoli modificati
(`{"subtitle", "document", "before", "after"}`, **vuoto** quando nulla è cambiato).

## Codici di uscita

Quelli dello strumento: `0` se ogni file è stato elaborato — lo è anche un file in cui nulla cambia —,
`2` se nessuno, `3` se solo alcuni, `1` in caso di errore d'uso.

## Errori

| Che cosa lo provoca | Messaggio |
| :------------------ | :------ |
| né `--on` né `--off` | `italics needs --on to put the texts in italics, or --off` |
| entrambi | `--on and --off say opposite things; give one of them` |
| un formato senza stile | `<path>: TMPlayer writes no style: there are no italics to put on` (oppure `take out`) |
| intervallo scritto male o fuori dal file | vedere [`adjust`](adjust.md#--range) |

Gli errori della destinazione sono comuni ai sottocomandi che scrivono:
vedere [Invocazione](invocation.md#la-destinazione).
