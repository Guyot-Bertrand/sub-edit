# `join-repeats`

<!-- machine-translated -->
> Questa pagina è una traduzione automatica e non è ancora stata rivista da un lettore della lingua.

```
subedit-cli join-repeats (--output FILE | --output-dir DIR | --in-place)
                         [--dry-run]
                         [--recursive]
                         <file>...
```

Riunisce in **un unico sottotitolo** i sottotitoli che mostrano la stessa cosa uno dopo l'altro: un testo spezzato in parti
che si susseguono, come lo lasciano alcuni strumenti di segmentazione. È il comando `Join Repeated Subtitles` della finestra —
[lo stesso motore](../subedit-gui/lines.md#unire-i-sottotitoli-ripetuti).

**Una ripetizione** è un sottotitolo che ha **lo stesso testo, la stessa traduzione e le stesse impostazioni di formato** (uno
stile, una posizione, impostazioni di cue) di un altro, **e che inizia al più tardi quando quello finisce**: si
toccano o si sovrappongono. Una serie di ripetizioni diventa **quella che inizia per prima**, la cui fine viene
portata alla fine più tarda della serie; gli altri scompaiono.

- **L'ordine del file non cambia nulla, e non viene modificato**: il sottotitolo mantenuto è quello che inizia per primo,
  ovunque si trovino gli altri. Un file in ordine resta in ordine.
- **Nulla viene unito per errore**: un altro testo, un'altra traduzione, altre impostazioni, un intervallo tra due
  parti, un sottotitolo senza testo o la cui fine precede l'inizio — in tutti questi casi restano separati.
- **Da fare prima del resto**: l'ordinamento e [il taglio delle sovrapposizioni](disjoint.md) leggono meglio una serie pulita.
  [`repair`](repair.md) li concatena tutti e tre, e [`convert`](convert.md#rendere-disgiunti-gli-intervalli) lo accetta come opzione (`--join-repeats`, `--repair`).

<!-- example: subedit-cli join-repeats --help -->
```console
$ subedit-cli join-repeats --help
Join the subtitles that repeat each other, one after the other, into one
Usage: subedit-cli join-repeats [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to join the repeats of

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
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
| `--output` / `--output-dir` / `--in-place` | **una delle tre**, tranne con `--dry-run` | vedere [Invocazione](invocation.md#la-destinazione) | — |
| `--dry-run` | no | un'opzione senza valore: calcola e riferisce, non scrive nulla — vedere [Vedere prima di scrivere](invocation.md#vedere-prima-di-scrivere) | disattivata |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:04,000 --> 00:00:06,000\nAlpha\n\n4\n00:00:08,000 --> 00:00:09,000\nBeta\n\n' > morceaux.srt; subedit-cli join-repeats --output joint.srt morceaux.srt; cat joint.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:04,000 --> 00:00:06,000\nAlpha\n\n4\n00:00:08,000 --> 00:00:09,000\nBeta\n\n' > morceaux.srt; subedit-cli join-repeats --output joint.srt morceaux.srt; cat joint.srt
morceaux.srt: 2 repeated subtitles joined -> joint.srt
1
00:00:01,000 --> 00:00:06,000
Alpha

2
00:00:08,000 --> 00:00:09,000
Beta
```

I tre pezzi di `Alpha`, che si susseguono da 1 s a 6 s, ora ne formano uno solo. `Beta`, separato da un intervallo,
è rimasto. Un secondo passaggio non cambia nulla e lo dice: `no repeated subtitles`.

## Output

**Output standard** — nulla: il risultato è il file scritto.

| Livello | Che cosa si aggiunge sull'errore standard |
| :----- | :------------------------------------- |
| 1 | `<path>: N repeated subtitles joined -> <destination>`, oppure `<path>: no repeated subtitles -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, poi **ogni messaggio diagnostico di lettura** |

### In JSON

Con [`--format json`](invocation.md#un-output-che-uno-script-può-leggere), i `counts` di ciascun file: `subtitles` (quelli del file
scritto), `joined` (i sottotitoli rimossi perché un altro li copre) e `chains` (le serie unite).

## Codici di uscita

Quelli dello strumento: `0` se ogni file è stato elaborato — un file senza ripetizioni lo è —, `2` se nessuno, `3` se
solo alcuni, `1` in caso di errore d'uso.

## Errori

Nessuno specifico di `join-repeats`: un file illeggibile fallisce come per qualsiasi sottocomando, e gli errori della
destinazione sono comuni ai sottocomandi che scrivono: vedere [Invocazione](invocation.md#la-destinazione).
