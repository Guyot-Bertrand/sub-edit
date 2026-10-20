# `transform`

<!-- machine-translated -->
> Questa pagina è una traduzione automatica e non è ancora stata rivista da un lettore della lingua.

```
subedit-cli transform --first <index>=<time> --last <index>=<time>
                      (--output FILE | --output-dir DIR | --in-place)
                      [--dry-run]
                      [--recursive]
                      <file>...
```

Corregge **ogni** posizione del file a partire da due punti il cui valore corretto
è noto. È l'operazione richiesta da una sincronizzazione che deriva
— giusta all'inizio, sbagliata alla fine — dove [`shift`](shift.md), che sposta tutto della
stessa quantità, non può fare nulla.

Il formato del file letto viene **conservato**: cambiare formato è compito di
[`convert`](convert.md).

<!-- example: subedit-cli transform --help -->
```console
$ subedit-cli transform --help
Correct every position from two points known to be right
Usage: subedit-cli transform [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to transform

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --first TEXT REQUIRED       Earlier reference, as <index>=<time>: 1=00:00:01.000
  --last TEXT REQUIRED        Later reference, as <index>=<time>: 3=00:00:10.000
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
| `--first` | **sì** | un riferimento, `<index>=<time>` | — |
| `--last` | **sì** | un riferimento, `<index>=<time>` | — |
| `--output` / `--output-dir` / `--in-place` | **una delle tre**, tranne con `--dry-run` | vedi [Invocazione](invocation.md#la-destinazione) | — |
| `--dry-run` | no | un'opzione senza valore: calcola e dice, non scrive nulla — vedi [Vedere prima di scrivere](invocation.md#vedere-prima-di-scrivere) | disattivata |

Non esiste una forma breve: queste due opzioni si scrivono solo per
esteso.

## Scrivere un riferimento

Un riferimento indica un sottotitolo e dice dove deve cadere ora il suo **inizio**:

```
<index>=<time>
```

**L'indice si conta da 1**, come viene mostrato e come lo numera un file
SubRip. `0` viene rifiutato, e lo stesso vale per un indice oltre l'ultimo
sottotitolo.

**Il tempo si scrive come quello di [`shift`](shift.md)** — secondi o
marca temporale, il separatore decimale è il punto, la risoluzione il millisecondo:

| Forma | Esempi |
| :---- | :------- |
| secondi | `10`, `2.999` |
| marca temporale | `00:00:10.000`, `01:39:37.040` |

`--first 3=10` e `--first 3=00:00:10.000` dicono la stessa cosa.

`--first` e `--last` indicano due punti, non un intervallo. I loro nomi dicono
l'uso previsto — un riferimento all'inizio del file, l'altro alla fine — e non un
vincolo: scambiarli dà lo stesso risultato, e **tutto ciò che sta fuori
dai due riferimenti segue come ciò che sta tra di essi**.

## Che cosa diventano le altre posizioni

I due riferimenti stabiliscono un fattore di scala, e ogni posizione lo segue:

```
t′ = y₁ + (t − x₁) × r      with r = (y₂ − y₁) / (x₂ − x₁)
```

`x₁`, `x₂` sono gli inizi attuali dei due sottotitoli indicati, `y₁`, `y₂` i
tempi richiesti. Il fattore è mantenuto come razionale esatto e **l'arrotondamento
avviene una sola volta**, al millisecondo; è questo che fa cadere i due
riferimenti **esattamente** sui tempi richiesti e non a meno di un millesimo
(vedi l'[ADR 0013](../../../../adr/0013-mise-a-l-echelle-exacte-des-positions.md)).

Ogni sottotitolo resta sullo schermo **in proporzione** a quanto vi era rimasto:
la fine segue la stessa scala dell'inizio. Una trasformazione non è uno
spostamento, e non ne ha la proprietà.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,001 --> 00:00:07,000\nDeux.\n\n3\n00:00:09,000 --> 00:00:11,000\nTrois.\n\n' > a.srt; subedit-cli transform --first 1=00:00:01.000 --last 3=00:00:10.000 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,001 --> 00:00:07,000\nDeux.\n\n3\n00:00:09,000 --> 00:00:11,000\nTrois.\n\n' > a.srt; subedit-cli transform --first 1=00:00:01.000 --last 3=00:00:10.000 --output b.srt a.srt; cat b.srt
a.srt: 3 subtitles transformed onto 1=00:00:01.000 and 3=00:00:10.000 -> b.srt
1
00:00:01,000 --> 00:00:03,250
Un.

2
00:00:05,501 --> 00:00:07,750
Deux.

3
00:00:10,000 --> 00:00:12,250
Trois.
```

Il primo riferimento non si muove, il terzo passa da 9 s a 10 s: il fattore è
`9/8`, e il secondo sottotitolo — che nessuno ha indicato — passa da
`5,001` a `5,501`, cioè `1000 + 4001 × 9/8` arrotondato una sola volta.

## Due riferimenti che non lo sono

| Situazione | Quando viene rilevata | Codice |
| :-------- | :------------- | :--- |
| `--first` e `--last` indicano lo stesso indice | prima di qualsiasi lettura | `1` |
| un indice è oltre l'ultimo sottotitolo | all'apertura del file | `2` o `3` |
| i due sottotitoli indicati iniziano nello stesso istante | all'apertura del file | `2` o `3` |

Tutti e tre vengono rifiutati per lo stesso motivo: `x₂ − x₁` è zero, e due
punti coincidenti non definiscono alcuna retta. Restituire una divisione per zero travestita da
risultato sarebbe peggio che rifiutare.

Il primo caso è un **errore d'uso** — le due opzioni sono in difetto qualunque siano
i file, quindi è dovuto un solo messaggio, non uno per file. Gli
altri due dipendono dal file aperto, e contano quindi come un
errore di elaborazione.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n' > a.srt; subedit-cli transform --first 2=1.000 --last 2=4.000 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n' > a.srt; subedit-cli transform --first 2=1.000 --last 2=4.000 --output b.srt a.srt; echo "code=$?"
--first and --last both name subtitle 2, and two references on one subtitle define no transform
code=1
```

Un indice fuori dai limiti viene rifiutato **indicando il limite**, il che dice allo
stesso tempo che cosa si sarebbe potuto scrivere:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,000 --> 00:00:07,000\nDeux.\n\n' > a.srt; subedit-cli transform --first 1=1.000 --last 9=10.000 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,000 --> 00:00:07,000\nDeux.\n\n' > a.srt; subedit-cli transform --first 1=1.000 --last 9=10.000 --output b.srt a.srt; echo "code=$?"
a.srt: subtitle 9 is past the end: the file holds 2 subtitles
code=2
```

## Una posizione prima dell'origine viene rifiutata

Come per [`shift`](shift.md), e per lo stesso motivo: una posizione negativa
si scrive `-00:00:01,000`, una forma che nessuno dei nove formati definisce e che nessun
lettore accetta. Il caso si presenta qui quando un sottotitolo si trova **prima
del primo riferimento** e la scala lo spinge oltre lo zero. Nulla viene
scritto, il codice è `2`, e il messaggio indica il sottotitolo in difetto.

## Output

**Output standard** — nulla: il risultato è il file scritto. Con `--dry-run`, nulla neppure: non c'è alcun file, e la riga di narrazione lo dice — vedi [Vedere prima di scrivere](invocation.md#vedere-prima-di-scrivere).

| Livello | Che cosa si aggiunge sull'output di errore |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles transformed onto 1=00:00:01.000 and 3=00:00:10.000 -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — il formato, la codifica, l'indicatore e i terminatori di riga del file letto, rimessi com'erano |
| 3 | `<path>: N bytes read, M written`, poi **ogni diagnostica di lettura** — vedi [Invocazione](invocation.md#diagnostica-di-lettura) |

I riferimenti vengono riscritti con il punto decimale, comunque siano stati digitati.

### In JSON

Con [`--format json`](invocation.md#un-output-che-uno-script-può-leggere), l'oggetto di ogni file porta `counts.subtitles` (ogni sottotitolo del file). La tabella completa è quella di [Invocazione](invocation.md#che-cosa-ogni-sottocomando-mette-in-counts).

## Codici di uscita

Quelli dello strumento: `0` se ogni file è stato scritto, `2` se nessuno, `3` se
solo alcuni, `1` in caso di errore d'uso.

## Errori

| Che cosa lo provoca | Messaggio |
| :------------------ | :------ |
| riferimento senza `=` | `"…" is not a reference: write it <index>=<time>, as in 3=00:00:10.000` |
| riferimento a cui manca una metà | `"…" is missing one of its two halves: write it <index>=<time>, as in 3=00:00:10.000` |
| indice `0` | `"0" is not a subtitle number: subtitles are counted from 1, as the file shows them` |
| indice che non è un numero intero | `"…" is not a subtitle number: expected a whole number, counted from 1 as the file shows them` |
| tempo non valido | quelli di [`shift`](shift.md) |
| `--first` e `--last` sullo stesso indice | `--first and --last both name subtitle N, and two references on one subtitle define no transform` |
| indice fuori dai limiti | `<path>: subtitle N is past the end: the file holds M subtitles` |
| due sottotitoli che iniziano insieme | `<path>: subtitles N and M start at the same moment, so they define no transform` |
| posizione prima dell'origine | `<path>: subtitle N would land before the origin, which no subtitle file can hold` |

**Nulla viene scritto senza una delle tre opzioni di destinazione**, e i relativi
errori sono comuni ai sottocomandi che scrivono:
vedi [Invocazione](invocation.md#la-destinazione).
