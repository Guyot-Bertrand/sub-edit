# `replace`

<!-- machine-translated -->
> Questa pagina è una traduzione automatica e non è ancora stata rivista da un lettore della lingua.

```
subedit-cli replace [--regex] [--case-sensitive] [--range N-M|N-]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    <pattern> <replacement> <file>...
```

Sostituisce un testo nei sottotitoli **senza spezzare un tag**. È l'operazione
`Replace All` della [ricerca](../subedit-gui/find-and-replace.md) della finestra, con le sue regole:

- **la ricerca avviene nel testo visibile** — quello che legge lo spettatore. I tag non ne fanno
  parte: cercare `<i>` non trova nulla, e cercare `Bonjour` trova un «Bonjour» di cui metà
  è in corsivo;
- **la sostituzione avviene nel testo sorgente**, quello del file, mantenendo i tag dove
  si trovavano. Un tag che taglia la parola cercata racchiude l'intera sostituzione; uno stile che
  tocca la corrispondenza copre l'intera sostituzione; ciò che la corrispondenza non tocca
  non si sposta. Le regole, caso per caso, sono quelle della ricerca della finestra.

**Non esiste un sottocomando `find`**: per vedere dove si trova un modello senza cambiare nulla,
è [`--dry-run`](invocation.md#vedere-prima-di-scrivere) che elenca ogni sottotitolo interessato.

<!-- example: subedit-cli replace --help -->
```console
$ subedit-cli replace --help
Replace a text in the subtitles, without breaking a tag
Usage: subedit-cli replace [OPTIONS] pattern replacement files...

Positionals:
  pattern TEXT REQUIRED       Text to look for in what is shown
  replacement TEXT REQUIRED   What replaces it, tags allowed
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --regex                     Read the text to look for as a regular expression
  --case-sensitive            Tell capitals from small letters
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

| Opzione | Obbligatoria | Valori | Predefinito |
| :----- | :----- | :------ | :----- |
| `<pattern>` | sì | il testo da cercare in ciò che viene mostrato; **non vuoto** | — |
| `<replacement>` | sì | ciò che lo sostituisce, tag ammessi; può essere vuoto | — |
| `<file>...` | sì | uno o più percorsi | — |
| `--recursive`, `-r` | no | un'opzione senza valore | disattivata — vedi [Elaborare un albero di directory](lots.md) |
| `--regex` | no | un'opzione senza valore: il modello è un'espressione regolare ICU | testo semplice |
| `--case-sensitive` | no | un'opzione senza valore: distingue le maiuscole dalle minuscole | vengono ignorate |
| `--range` | no | `N-M`, oppure `N-` fino alla fine — vedi [`adjust`](adjust.md#--range) | l'intero file |
| `--document` | no | `main` o `translation` — vedi [Una traduzione](invocation.md#una-traduzione) | `main` |
| `-t`, `--translation-file` | con `--document translation` | un file di traduzione, per **un solo** input | — |
| `--align-method` | no, e solo con `-t` | `position` o `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **una delle tre**, tranne con `--dry-run` | vedi [Invocazione](invocation.md#la-destinazione) | — |
| `--dry-run` | no | un'opzione senza valore: calcola e dice, non scrive nulla — vedi [Vedere prima di scrivere](invocation.md#vedere-prima-di-scrivere) | disattivata |

**Un modello che inizia con un trattino** si scrive dopo `--`: `subedit-cli replace -- -- "—" film.srt`.

**Il testo principale per impostazione predefinita; la traduzione con `--document translation -t FILE`.** È allora
la traduzione che viene modificata e scritta, e solo essa — vedi [Una traduzione](invocation.md#una-traduzione).

### Testo semplice o espressione

Per impostazione predefinita il modello viene **letto così com'è**: `a.b` cerca una `a`, un punto, una `b`. Le maiuscole e
le minuscole vengono ignorate con la stessa finezza della finestra — `É` e `é` sono una sola lettera.

Con `--regex`, il modello è un'espressione regolare **ICU**, letta come la legge Gaupol: `.` attraversa
un a capo, `^` e `$` valgono a ogni riga di un sottotitolo. Nella sostituzione, e
solo lì:

| Si scrive | Per dire |
| :----- | :-------- |
| `$1` a `$9` | i gruppi della corrispondenza |
| `$0` | l'intera corrispondenza |
| `\n` | un a capo |
| `\$`, `\\` | un `$`, una barra rovesciata |

Attenzione alle virgolette della shell: `'$1 EUR'`, e non `"$1 EUR"`, che lascerebbe alla shell la lettura di `$1`.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nPrix: 12 euros, et 7 euros.\n\n' > prix.srt; subedit-cli replace --regex '([0-9]+) euros' '$1 EUR' --output euros.srt prix.srt; cat euros.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nPrix: 12 euros, et 7 euros.\n\n' > prix.srt; subedit-cli replace --regex '([0-9]+) euros' '$1 EUR' --output euros.srt prix.srt; cat euros.srt
prix.srt: replaced 2 matches -> euros.srt
1
00:00:01,000 --> 00:00:02,000
Prix: 12 EUR, et 7 EUR.
```

## Tag

Un testo in cui un tag taglia la parola cercata:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace Bonjour Salut --output salut.srt balises.srt; cat salut.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace Bonjour Salut --output salut.srt balises.srt; cat salut.srt
balises.srt: replaced 2 matches -> salut.srt
1
00:00:01,000 --> 00:00:02,000
<i>Salut</i> tout le monde

2
00:00:03,000 --> 00:00:04,000
Salut <i>Marie</i>, ca va
```

Nel primo sottotitolo, il tag tagliava «Bonjour»: ora racchiude «Salut».
Nel secondo, non viene toccato e resta dov'è. **Un tag lasciato aperto
resta aperto**: riparare un markup instabile sarebbe un'altra operazione.

La sostituzione viene letta nei **tag del formato del file**: `<i>…</i>` per SubRip,
`{\i1}…` per Advanced SSA. È l'unico modo di formattare ciò che si scrive.

## Nulla trovato, nulla da cambiare

Due risposte che non sono errori — il codice di uscita è `0` e **il file viene scritto
comunque**, una destinazione indicata essendo una destinazione scritta:

| Situazione | Frase |
| :-------- | :----- |
| il modello non c'è da nessuna parte | `"zzz" not found` |
| c'è, ma ogni corrispondenza viene sostituita da se stessa | `nothing to change` |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli replace zzz x --output b.srt a.srt; subedit-cli replace Bonjour Bonjour --output c.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli replace zzz x --output b.srt a.srt; subedit-cli replace Bonjour Bonjour --output c.srt a.srt
a.srt: "zzz" not found -> b.srt
a.srt: nothing to change -> c.srt
```

## `--dry-run`

L'output standard elenca **ogni sottotitolo il cui testo cambierebbe**, prima e dopo, così com'è
nel file — tag compresi:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace --dry-run Bonjour Salut balises.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace --dry-run Bonjour Salut balises.srt
balises.srt: replaced 2 matches (dry run, nothing written)
balises.srt: subtitle 1
- <i>Bon</i>jour tout le monde
+ <i>Salut</i> tout le monde
balises.srt: subtitle 2
- Bonjour <i>Marie</i>, ca va
+ Salut <i>Marie</i>, ca va
```

La forma dell'elenco è descritta in [Vedere prima di scrivere](invocation.md#vedere-prima-di-scrivere);
in JSON, è `changes`.

## Output

**Output standard** — nulla: il risultato è il file scritto. Con `--dry-run`, l'elenco delle
modifiche qui sopra.

| Livello | Che cosa si aggiunge sull'output di errore |
| :----- | :------------------------------------- |
| 1 | `<path>: replaced N matches -> <destination>`, oppure `"<pattern>" not found`, oppure `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — il formato, la codifica, l'indicatore e i terminatori di riga del file letto, rimessi com'erano |
| 3 | `<path>: N bytes read, M written`, poi **ogni diagnostica di lettura** — vedi [Invocazione](invocation.md#diagnostica-di-lettura) |

`N` conta **le corrispondenze sostituite nei sottotitoli il cui testo è cambiato** — è il
numero che dice anche la finestra.

### In JSON

Con [`--format json`](invocation.md#un-output-che-uno-script-può-leggere), l'oggetto di ogni file porta:

| Chiave | Contenuto |
| :-- | :------ |
| `counts.replaced` | le corrispondenze sostituite, nei sottotitoli il cui testo è cambiato: la `N` della frase |
| `counts.matched` | le corrispondenze trovate, **che la loro sostituzione abbia cambiato il testo o no** — è ciò che distingue «nulla trovato» (`0`) da «nulla da cambiare» (`replaced` a `0`, `matched` positivo) |
| `changes` | i sottotitoli modificati: `{"subtitle", "document", "before", "after"}`, **vuoto** quando nulla è cambiato |

## Codici di uscita

Quelli dello strumento: `0` se ogni file è stato elaborato — un file in cui non è stato trovato nulla lo è
—, `2` se nessuno, `3` se solo alcuni, `1` in caso di errore d'uso.

## Errori

| Che cosa lo provoca | Messaggio |
| :------------------ | :------ |
| modello vuoto | `pattern: nothing to look for` |
| espressione illeggibile | `pattern: not a regular expression (<ICU's reason>)` |
| intervallo scritto male o fuori dal file | vedi [`adjust`](adjust.md#--range) |

**Il modello viene letto prima del primo file**: un'espressione illeggibile è un errore d'uso
(codice `1`), nulla viene letto né scritto.

Gli errori della destinazione sono comuni ai sottocomandi che scrivono:
vedi [Invocazione](invocation.md#la-destinazione).
