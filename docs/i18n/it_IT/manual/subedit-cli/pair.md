# `pair`

<!-- machine-translated -->
> Questa pagina è una traduzione automatica e non è ancora stata rivista da un lettore della lingua.

```
subedit-cli pair (--output FILE | --output-dir DIR | --in-place)
                 -t FILE [--align-method position|number]
                 [--dry-run]
                 <main>
```

Scrive **la traduzione risincronizzata sulle posizioni del file principale** e dice come le sue righe hanno trovato il
loro sottotitolo. È `Open Translation…` della finestra — [lo stesso abbinamento](../subedit-gui/files.md#aprire-una-traduzione),
con la stessa frase — seguito dalla scrittura del risultato, che la finestra esegue con `Save Translation`.

**È l'unica cosa che l'abbinamento fa e che un file elaborato da solo non fa.** Gli altri sottocomandi
che accettano `-t` modificano la traduzione che leggono — [`case`](case.md), [`replace`](replace.md)… —; `pair` non modifica
nulla del testo: **posa** la traduzione sul file principale e scrive ciò che la posa ha prodotto. Il file principale viene
letto per accogliere le righe, **mai scritto**.

- **Ciò che viene scritto è la traduzione**, **al suo percorso e nel suo formato**: il formato, la codifica e
  i terminatori di riga del file di traduzione. I suoi sottotitoli sono **alle posizioni del file principale**: una riga che
  era finita fuori bersaglio riprende i limiti del sottotitolo che traduce.
- **`-t` è obbligatorio.** `pair` senza traduzione non ha nulla da posare: è un errore d'uso, codice `1`.
- **Un solo file principale**, e nessun `--recursive`: una coppia è fatta di due file, e un albero non ha
  modo di dire quale traduzione vada con quale file principale. Per un'elaborazione in blocco, una invocazione per coppia.
- **`--output` indica il file della traduzione scritta, `--output-dir` vi mette il suo nome di base, `--in-place`
  la riscrive**, come per ogni sottocomando che accetta `-t` — vedi [Una traduzione](invocation.md#una-traduzione).
- **`--document` non esiste**: c'è un solo documento di destinazione, la traduzione.

<!-- example: subedit-cli pair --help -->
```console
$ subedit-cli pair --help
Write a translation at the positions of its main file, and say how it lined up
Usage: subedit-cli pair [OPTIONS] main

Positionals:
  main TEXT REQUIRED          The main subtitle file

Options:
  -h,--help                   Print this help message and exit
  -t,--translation-file FILE  Translation file to lay over the subtitle file, for a single input
  --align-method position|number
                              How the lines of the translation find their subtitles
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Argomenti e opzioni

| Opzione | Obbligatoria | Valori | Predefinito |
| :----- | :----- | :------ | :----- |
| `<main>` | sì | **un solo** percorso | — |
| `-t`, `--translation-file` | **sì** | il file di traduzione | — |
| `--align-method` | no | `position` o `number` — vedi [Una traduzione](invocation.md#una-traduzione) | `position` |
| `--output` / `--output-dir` / `--in-place` | **una delle tre**, tranne con `--dry-run` | vedi [Invocazione](invocation.md#la-destinazione) | — |
| `--dry-run` | no | un'opzione senza valore: calcola e dice, non scrive nulla — vedi [Vedere prima di scrivere](invocation.md#vedere-prima-di-scrivere) | disattivata |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,300 --> 00:00:02,300\nBonjour.\n\n2\n00:00:03,300 --> 00:00:04,300\nAu revoir.\n\n' > film.fr.srt; subedit-cli pair film.srt -t film.fr.srt --output recale.fr.srt; cat recale.fr.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,300 --> 00:00:02,300\nBonjour.\n\n2\n00:00:03,300 --> 00:00:04,300\nAu revoir.\n\n' > film.fr.srt; subedit-cli pair film.srt -t film.fr.srt --output recale.fr.srt; cat recale.fr.srt
film.fr.srt: translation: 2 lines attached -> recale.fr.srt
1
00:00:01,000 --> 00:00:02,000
Bonjour.

2
00:00:03,000 --> 00:00:04,000
Au revoir.
```

La traduzione era in ritardo di 0,3 s: ogni riga ha trovato il sottotitolo su cui cade il suo punto centrale, e il
file scritto porta le posizioni del file principale.

## Che cosa dice l'allineamento

La frase è **quella che dice la finestra** quando apre una traduzione, ogni parte solo quando non è
zero — vedi [`inspect -t`](inspect.md#una-traduzione--t) per la tabella delle sue parti. **Viene scritta
dal livello 1 in poi**, e non al livello 2 come per gli altri sottocomandi: qui l'allineamento non è un
dettaglio dell'elaborazione, è il risultato.

**Ciò che l'allineamento fa al file scritto**:

- **un sottotitolo che nessuno ha tradotto viene scritto come un blocco senza testo**, che si legge senza anomalie;
- **un sottotitolo nato da una riga** — una riga che nulla ha raggiunto — viene scritto con **le sue posizioni**, qualunque sia
  il metodo: il file scritto può quindi contare più sottotitoli del file principale;
- **ciò che il documento portava come traduzione viene cancellato** prima della posa, il che non vale per la riga di comando:
  il file principale viene letto da solo.

**I due metodi** danno lo stesso file quando le righe cadono ciascuna sul proprio sottotitolo, e non
quando manca una riga o la traduzione è sfasata — per questo il metodo è una scelta, e per questo
la posizione è il valore predefinito. Gli otto casi di `src/test/data/paires/` li elencano, con il file atteso per
ciascun metodo.

## Output

**Output standard** — nulla: il risultato è il file scritto.

| Livello | Che cosa si aggiunge sull'output di errore |
| :----- | :------------------------------------- |
| 1 | `<translation>: translation: N lines attached; … -> <destination>` — **il percorso è quello della traduzione**, che è il file scritto; con `--dry-run`, `(dry run, nothing written)` al posto di `-> <destination>` |
| 2 | `<translation>: SubRip, UTF-8, no BOM, LF line endings kept` — il formato, la codifica, l'indicatore e i terminatori di riga **della traduzione**, rimessi com'erano |
| 3 | `<translation>: N bytes read, M written`, poi **ogni diagnostica di lettura** del file principale e della traduzione — vedi [Invocazione](invocation.md#diagnostica-di-lettura) |

### In JSON

Con [`--format json`](invocation.md#un-output-che-uno-script-può-leggere), **un solo oggetto**, il cui `file` è la traduzione e
`destination` il file scritto. `counts.subtitles` conta **i sottotitoli del file scritto**, compresi quelli
fatti nascere da una riga. La chiave `alignment` — `{"file", "method", "attached", "born", "untranslated", "out_of_order"}` —
porta gli stessi conteggi della frase, come la chiave `translation` di [`inspect`](inspect.md#in-json).

## Codici di uscita

Quelli dello strumento: `0` se la traduzione è scritta — o calcolata, con `--dry-run` —, `2` se uno dei due file non ha potuto
essere letto o se la destinazione non ha potuto essere scritta, `1` in caso di errore d'uso. **Una traduzione allineata male non è
un errore**: viene scritta, e la frase dice che cosa non ha trovato il suo posto.

## Errori

| Che cosa li provoca | Messaggio |
| :------------------- | :------ |
| nessun `-t` | `pair needs the translation file to lay over the main one: use -t` |
| `--document` | `The following argument was not expected: --document` — non esiste per `pair` |
| più di un file principale | `The following argument was not expected: <path>` |
| la traduzione è il file principale | `<translation>: the file is already open as the main document`, codice `2` |
| uno dei file mancante, illeggibile o di formato sconosciuto | lo stesso di ovunque, con il percorso del file in questione |
