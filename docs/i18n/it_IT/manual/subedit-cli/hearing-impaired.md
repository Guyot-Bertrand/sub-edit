# `hearing-impaired`

<!-- machine-translated -->
> Questa pagina è una traduzione automatica e non è ancora stata rivista da un lettore della lingua.

```
subedit-cli hearing-impaired
                  (--output FILE | --output-dir DIR | --in-place)
                  [--dry-run]
                  [--recursive] [--parentheses]
                  [--document main|translation] [-t FILE] [--align-method position|number]
                  <file>...
```

Rimuove dall'intero file le **indicazioni per non udenti** — i suoni descritti tra parentesi quadre. **Le parentesi tonde
restano per impostazione predefinita**: si usano tanto spesso per una battuta sussurrata
quanto per un suono descritto, e rimuoverle cancellerebbe dei dialoghi. `--parentheses` le rimuove
a loro volta, per un file che descrive i suoi suoni con esse.

Ciò che non fa: i testi delle canzoni tra cancelletti, il nome del parlante
prima dei due punti, il ripristino delle maiuscole, la correzione degli errori di OCR. Questi
modelli sono quelli dell'assistente
[`Correct Texts…`](../subedit-gui/correct-texts.md) della finestra, che la riga di
comando offre con il nome [`correct`](correct.md); e qui nulla è configurabile: la
trasformazione è decisa, non regolabile.

<!-- example: subedit-cli hearing-impaired --help -->
```console
$ subedit-cli hearing-impaired --help
Remove the sounds described between square brackets
Usage: subedit-cli hearing-impaired [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to clean

Options:
  -h,--help                   Print this help message and exit
  --parentheses               Also remove the sounds between parentheses, often whispered lines
  -r,--recursive              Take directories as inputs, and every subtitle file in them
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
| `--parentheses` | no | un'opzione senza valore: rimuove anche le indicazioni tra parentesi tonde | disattivata |
| `--recursive`, `-r` | no | un'opzione senza valore | disattivata — vedere [Elaborare un albero di directory](lots.md) |
| `--document` | no | `main` o `translation` — vedere [Una traduzione](invocation.md#una-traduzione) | `main` |
| `-t`, `--translation-file` | con `--document translation` | un file di traduzione, per **un solo** input | — |
| `--align-method` | no, e solo con `-t` | `position` o `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **una delle tre**, tranne con `--dry-run` | vedere [Invocazione](invocation.md#la-destinazione) | — |
| `--dry-run` | no | un'opzione senza valore: calcola e riferisce, non scrive nulla — vedere [Vedere prima di scrivere](invocation.md#vedere-prima-di-scrivere) | disattivata |

Il formato del file letto viene **mantenuto**: cambiare formato è compito di
[`convert`](convert.md).

**Per impostazione predefinita il testo principale; la traduzione con `--document translation -t FILE`.** In tal caso è
la traduzione a essere modificata e scritta, e soltanto essa — vedere [Una traduzione](invocation.md#una-traduzione).

**Un'indicazione che svuoterebbe una traduzione la scrive vuota, e mantiene il sottotitolo**: il sottotitolo
resta nel file, senza testo. Il testo principale, invece, perde un sottotitolo svuotato del tutto, come
più avanti.

## Che cosa viene rimosso e che cosa resta

Un'indicazione è una parentesi quadra — o una parentesi tonda, con `--parentheses` — e ciò che racchiude, **a capo
compreso**: nei file reali, un'indicazione è spesso interrotta dalla
fine di una riga.

| Input | Output |
| :----- | :----- |
| `Bonjour [il tousse] Marie` | `Bonjour Marie` |
| `Bonjour[il tousse]Marie` | `Bonjour Marie` |
| `[Bruit de pas] Bonjour` | `Bonjour` |
| `A [un] [deux] B` | `A B` |

La rimozione lascia **esattamente uno spazio tra ciò che la circondava, e nulla al
bordo di una riga**. Non è una pulizia degli spazi del file: due spazi
già presenti nel testo restano due spazi.

**Quando l'indicazione si trova a cavallo di un a capo, è l'a capo a restare** — le
due righe restano due righe. Questo caso non è una curiosità: in quindici sottotitolazioni
reali, ogni parentesi quadra che sembrava orfana era un'indicazione
interrotta in questo modo.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nReculez ! [Il\nhurle] Tout de suite !\n\n' > a.srt; subedit-cli --quiet hearing-impaired --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nReculez ! [Il\nhurle] Tout de suite !\n\n' > a.srt; subedit-cli --quiet hearing-impaired --output b.srt a.srt; cat b.srt
1
00:00:01,000 --> 00:00:03,000
Reculez !
Tout de suite !
```

**Ciò che non è un'indicazione e non si tocca:**

| Caso | Che cosa succede |
| :-- | :-------------- |
| contenuto puramente numerico — `[1]`, `(12)`, `[ 156478 ]` | mantenuto così com'è, delimitatori compresi: un riferimento non ha mai indicato un suono |
| una parentesi tonda e ciò che racchiude, senza `--parentheses` | mantenuta così com'è: `Venez (chut) ici` non cambia |
| una parentesi quadra o tonda che nulla chiude | lasciata com'è |
| un tag di formato — `<i>`, `</i>` | mai rimosso |

Uno spazio **tra** le cifre, una lettera o un segno — `[1 2]`, `[1a]`,
`[-1]` — ne fanno un'indicazione. Lo stesso vale per un contenuto vuoto, `[]` o `( )`: nessuna
cifra, quindi non è un riferimento; nulla da dire, quindi nulla da conservare.

## Che cosa scompare del tutto

**Una riga svuotata dalla rimozione scompare**, e **un sottotitolo svuotato
del tutto viene rimosso dal file**. I sottotitoli successivi vengono rinumerati.

Una riga conta come vuota quando tutto ciò che resta sono spazi, tag
di formato o un trattino di dialogo isolato — `<i>[Musique]</i>` non lascia un
`<i></i>` sullo schermo.

**I tag sono quelli del formato del file.** `{\i1}[Musique]{\i0}` in un
Advanced SSA, `{Y:i}[Musique]` in un MicroDVD e `/[Musique]` in un MPL2 non lasciano
nulla neppure loro. Al contrario, un `<i>` in un formato che non scrive alcun
tag — TMPlayer, LRC — è testo che lo spettatore legge, e la riga che
lo contiene non è vuota.

**Un dialogo ridotto a una sola voce non è più tale**, e il trattino che resta
scompare. Finché restano due voci, i trattini restano.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\nVoir [1] la note.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\nVoir [1] la note.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle cleaned, 1 removed -> b.srt
1
00:00:04,000 --> 00:00:06,000
Attends Marie.

2
00:00:07,000 --> 00:00:09,000
Voir [1] la note.
```

## Un file senza alcuna indicazione viene scritto comunque

Viene copiato in modo identico, il codice di uscita è `0`, e il rapporto dice che non c'era
nulla da rimuovere.

È la regola dei sottocomandi che scrivono: **una destinazione data è
una destinazione scritta**. Fare qui un'eccezione costringerebbe uno script a sapere
quali sottocomandi producono talvolta un file e talvolta nulla.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nRien à signaler.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nRien à signaler.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt
a.srt: no mention to remove -> b.srt
```

## Output

**Output standard** — nulla: il risultato è il file scritto. **Con `--dry-run`, sono le modifiche proposte**: per ogni sottotitolo il cui testo cambierebbe, il suo numero, il testo prima e il testo dopo — vedere [Vedere prima di scrivere](invocation.md#vedere-prima-di-scrivere).

| Livello | Che cosa si aggiunge sull'errore standard |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles cleaned, M removed -> <destination>`, oppure `<path>: no mention to remove -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — il formato, la codifica, l'indicatore e i terminatori di riga del file letto, rimessi com'erano |
| 3 | `<path>: N bytes read, M written`, poi **ogni messaggio diagnostico di lettura** — vedere [Invocazione](invocation.md#diagnostica-di-lettura) |

`N` conta i sottotitoli il cui testo è cambiato, `M` quelli che la rimozione ha
svuotato del tutto e che hanno quindi lasciato il file.

### In JSON

Con [`--format json`](invocation.md#un-output-che-uno-script-può-leggere), l'oggetto di ciascun file contiene `counts.cleaned` (i testi riscritti) e `counts.removed` (i sottotitoli rimossi); **con `--dry-run`**, la chiave `changes` elenca inoltre i sottotitoli modificati, come l'output standard della forma testuale. La tabella completa è quella di [Invocazione](invocation.md#che-cosa-ogni-sottocomando-mette-in-counts).

## Codici di uscita

Quelli dello strumento: `0` se ogni file è stato scritto, `2` se nessuno, `3` se
solo alcuni, `1` in caso di errore d'uso.

## Errori

Questo sottocomando non ha errori propri: non prende alcun
valore da interpretare, e nessun testo può indurlo a rifiutare un file. Ciò che
può fallire è la lettura o la scrittura, come per gli altri.

**Nulla viene scritto senza una delle tre opzioni di destinazione**, e i relativi errori
sono comuni ai sottocomandi che scrivono:
vedere [Invocazione](invocation.md#la-destinazione).
