# `sort`

<!-- machine-translated -->
> Questa pagina è una traduzione automatica e non è ancora stata rivista da un lettore della lingua.

```
subedit-cli sort (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 <file>...
```

Rimette i sottotitoli **nell'ordine del loro inizio**. Nient'altro cambia: né i testi, né le
posizioni, né il formato del file. I sottotitoli vengono **rinumerati** da 1 dalla scrittura,
come per qualsiasi file riscritto.

**Il nucleo non ordina mai di propria iniziativa**: leggere un file ne lascia l'ordine com'era, e
[`inspect`](inspect.md#che-cosa-segnala-anomalies) segnala una rottura dell'ordine con
`subtitle N starts before the previous one starts`. `sort` è **il gesto fatto di proposito**, per un file
o un gruppo di file il cui ordine è rotto — e si può annullare, nella finestra, come qualsiasi operazione.

- **L'ordinamento è stabile**: due sottotitoli che iniziano nello stesso istante **mantengono l'ordine che
  il file ha dato loro**. Nessuno dei due precede l'altro; spostarli sarebbe una decisione che nessuno
  ha chiesto.
- **`sort` non accetta né un intervallo né un documento**: non c'è nulla da scegliere, l'ordine è quello degli inizi.

<!-- example: subedit-cli sort --help -->
```console
$ subedit-cli sort --help
Put the subtitles in the order of their start, keeping ties as they are
Usage: subedit-cli sort [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to sort

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
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
| `--output` / `--output-dir` / `--in-place` | **una delle tre**, tranne con `--dry-run` | vedi [Invocazione](invocation.md#la-destinazione) | — |
| `--dry-run` | no | un'opzione senza valore: calcola e dice, non scrive nulla — vedi [Vedere prima di scrivere](invocation.md#vedere-prima-di-scrivere) | disattivata |

<!-- example: printf '1\n00:00:05,000 --> 00:00:06,000\ncinq\n\n2\n00:00:01,000 --> 00:00:02,000\nun\n\n3\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > desordre.srt; subedit-cli sort --output ordre.srt desordre.srt; cat ordre.srt -->
```console
$ printf '1\n00:00:05,000 --> 00:00:06,000\ncinq\n\n2\n00:00:01,000 --> 00:00:02,000\nun\n\n3\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > desordre.srt; subedit-cli sort --output ordre.srt desordre.srt; cat ordre.srt
desordre.srt: 3 subtitles moved -> ordre.srt
1
00:00:01,000 --> 00:00:02,000
un

2
00:00:03,000 --> 00:00:04,000
trois

3
00:00:05,000 --> 00:00:06,000
cinq
```

## Che cosa viene contato

**Il conteggio è quello dei posti che hanno cambiato sottotitolo**, come lo segnala il nucleo — ciò che
la cronologia della finestra chiamerebbe spostato. Un solo sottotitolo fuori posto ne sposta diversi: nell'esempio
qui sopra, nessuno dei tre posti conserva il proprio sottotitolo, e la frase dice `3 subtitles moved`.

**Un file già in ordine viene scritto comunque**, e lo dice: una destinazione indicata è una
destinazione scritta, e fare di `sort` l'eccezione obbligherebbe uno script a sapere quali
sottocomandi producono talvolta un file e talvolta nulla.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > ordre.srt; subedit-cli sort --output copie.srt ordre.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > ordre.srt; subedit-cli sort --output copie.srt ordre.srt
ordre.srt: already in order -> copie.srt
```

## Output

**Output standard** — nulla: il risultato è il file scritto. Con `--dry-run`, nulla neppure: non c'è
alcun file, e la riga di narrazione lo dice — vedi [Vedere prima di scrivere](invocation.md#vedere-prima-di-scrivere).

| Livello | Che cosa si aggiunge sull'output di errore |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles moved -> <destination>`, oppure `<path>: already in order -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — il formato, la codifica, l'indicatore e i terminatori di riga del file letto, rimessi com'erano |
| 3 | `<path>: N bytes read, M written`, poi **ogni diagnostica di lettura** — vedi [Invocazione](invocation.md#diagnostica-di-lettura) |

### In JSON

Con [`--format json`](invocation.md#un-output-che-uno-script-può-leggere), l'oggetto di ogni file porta
`counts.subtitles` (ogni sottotitolo del file) e `counts.moved` (i posti che sono cambiati;
`0` per un file già in ordine).

## Codici di uscita

Quelli dello strumento: `0` se ogni file è stato elaborato — un file già in ordine lo è —, `2` se
nessuno, `3` se solo alcuni, `1` in caso di errore d'uso.

## Errori

Nessuno specifico di `sort`: un file illeggibile fallisce come per qualsiasi sottocomando, e gli
errori della destinazione sono comuni ai sottocomandi che scrivono:
vedi [Invocazione](invocation.md#la-destinazione).
