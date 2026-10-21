# Invocazione

<!-- machine-translated -->
> Questa pagina è una traduzione automatica e non è ancora stata rivista da un lettore della lingua.

```
subedit-gui [--version] [<file>]
```

| Argomento | Obbligatorio | Valore | Predefinito |
| :------- | :------- | :---- | :------ |
| `<file>` | no | il percorso di un file di sottotitoli da aprire | nessuno — la finestra si apre vuota |
| `--version` | no | — | — |

**Un solo file.** Gli argomenti successivi vengono ignorati: la finestra contiene
un solo progetto alla volta.

## `--version`

Scrive sullo standard output il blocco che scrive il `--version` della riga di comando, e
termina subito, senza aprire alcuna finestra. Codice di uscita `0`.

**`subedit <version>` da solo sulla prima riga**, poi una riga per ogni libreria con cui la
finestra funziona, con la versione trovata a tempo di esecuzione e, tra parentesi, quella
con cui è stata compilata; poi le lingue dell'interfaccia installate, per codice. Su
una macchina con Qt 6.4.2:

```console
$ subedit-gui --version
subedit <version>
  ICU      74.2 (built with 74.2)
  Enchant  2.3.3 (built with 2.3.3)
  Qt       6.4.2 (built with 6.4.2)
  libmpv   2.2 (built with 2.2)
  Languages: en, fr
```

I numeri non sono copiati qui: sono quelli della macchina, e questo manuale non ha alcun
blocco generato che tenga aggiornati quelli del programma stesso. Le librerie sono
quelle della riga di comando, ICU ed Enchant, e le due che solo la finestra collega — Qt, e
libmpv, il lettore video, indicata dalla versione della sua API client, quella che
`mpv.pc` annuncia. Il resto è descritto in
[il manuale della riga di comando](../subedit-cli/invocation.md#--version). Le parole
`built with` e `Languages` seguono la lingua della finestra; nient'altro lo fa.

`Aiuto ▸ Informazioni su subedit` mostra le stesse librerie, dalla stessa lista, in una tabella —
il nome, la versione caricata, la versione con cui è stata compilata, una riga ciascuna — poi le
lingue sotto; le sue intestazioni seguono la lingua della finestra.

## Aprire un file

```console
$ subedit-gui film.srt
```

I formati letti sono **i nove** che il filtro di apertura elenca — vedi
[Aprire e salvare](files.md#apertura). Il formato viene riconosciuto dal contenuto e
non dall'estensione, e **lo stesso vale per la codifica**: un BOM
se il file ne porta uno, altrimenti la codifica che i suoi byte suggeriscono.

## Quando l'apertura fallisce

Un file mancante, uno illeggibile, uno i cui byte non si decodificano in nessuna
codifica, o uno il cui contenuto nessun formato riconosce: **la finestra si apre
comunque, vuota**, e l'errore viene scritto sul flusso degli errori.

**Il messaggio nomina la causa**, con le stesse parole della riga di comando:

| Messaggio | Che cosa è successo |
| :------ | :------------ |
| `does not exist` | il percorso non designa alcun file |
| `cannot be opened: permission denied` | il sistema rifiuta di aprirlo |
| `cannot be read` | il sistema ha rifiutato per un altro motivo |
| `cannot be decoded in the chosen encoding` | i byte non si leggono nella codifica scelta |
| `is in no format this tool knows` | nessun formato riconosce il contenuto |
| `holds nothing recognisable as a subtitle` | il formato è riconosciuto, ma nulla al suo interno è un sottotitolo |

```console
$ subedit-gui notes.txt
subedit-gui: notes.txt: is in no format this tool knows
```

Aprirsi vuota anziché rifiutarsi di avviarsi: lo strumento serve ancora a
qualcosa, e il fallimento di un argomento non è un motivo per non avviarlo.

**Un file malformato ma leggibile si apre**, e ciò che la lettura ha recuperato
lungo la strada si può leggere in un pulsante della barra di stato — vedi
[la diagnostica di una lettura](files.md#la-diagnostica-di-una-lettura).

## Codici di uscita

| Codice | Quando |
| :--- | :--- |
| `0` | la finestra si è aperta ed è stata poi chiusa normalmente, oppure `--version` ha risposto |
| `2` | un errore imprevisto prima dell'apertura — Qt mancante, schermo non disponibile |

Il fallimento dell'apertura di un file **non cambia il codice di uscita**: la
finestra si è aperta, ed è questo che il codice indica.

## La pagina di manuale

Un'installazione colloca `subedit-gui(1)` sotto `share/man/man1`:

```console
$ man subedit-gui
```

Fornisce la forma di chiamata, `--version`, i messaggi di errore di apertura, i
codici di uscita, le due variabili d'ambiente e il file delle impostazioni.
**Non descrive la finestra** — lo fa questo manuale, e la pagina
rimanda qui.

## La piattaforma Qt

`QT_QPA_PLATFORM` sceglie su che cosa funziona la finestra. **Se la variabile è
impostata, `subedit-gui` la rispetta senza discutere.**

Se non lo è, lascia scegliere a Qt: il lettore integrato disegna l'immagine nella
finestra stessa, e non chiede nulla alla piattaforma — vedi
[Il lettore video](player.md).

`QT_QPA_PLATFORM=offscreen` fa funzionare la finestra dove non c'è alcun
server grafico — ed è ciò che fanno l'integrazione continua e i test.

## Lingua

La finestra parla la lingua del proprio ambiente, **scelta una sola volta, all'avvio**: cambiare le
variabili di una finestra in esecuzione non cambia nulla fino al prossimo avvio. Le variabili e le regole
sono quelle della riga di comando — `LANGUAGE`, `LC_ALL`, `LC_MESSAGES`, `LANG`, `LC_ALL=C` per
l'inglese, l'inglese **senza dire nulla** quando la lingua richiesta non è installata — descritte in
[il manuale della riga di comando](../subedit-cli/invocation.md#lingua). I cataloghi sono gli stessi
file, installati in `<prefix>/share/subedit/locale`.

**Una lingua scelta in `Modifica ▸ Preferenze…` ha la precedenza**, e l'ambiente viene letto solo
quando è `System default`, come lo è finché non se ne sceglie una: una lingua installata scelta lì
prevale su qualsiasi variabile, `LC_ALL=C` compresa. Anche la scelta si applica al prossimo avvio. Vedi
[La lingua dell'interfaccia](preferences.md#la-lingua-dellinterfaccia).

| Che cosa | Segue la lingua |
| :--- | :------------------- |
| i menu e le loro voci, con le lettere sottolineate; la barra degli strumenti; i suggerimenti a comparsa | sì |
| le colonne della tabella e di `View ▸ Columns`, le schede, la riga di stato, la barra di riproduzione | sì |
| i pulsanti standard che Qt disegna da sé (`OK`, `Cancel`) | sì, quando è installato il catalogo di Qt di quella lingua (`qtbase_<language>.qm`); altrimenti in inglese |
| le finestre di dialogo, le pagine di `Correct Texts…`, i titoli e i filtri delle finestre di dialogo dei file, le finestre di messaggio e ciò che dicono | sì |
| le righe che la finestra scrive sul flusso degli errori, `subedit-gui: …` | sì, tranne il nome del programma |
| i nomi dei modelli di correzione in `Correct Texts…`, e le loro descrizioni nei suggerimenti a comparsa | sì, per i modelli distribuiti con il programma; un modello proprio viene mostrato come lo scrive il suo file |
| i nomi delle codifiche, dei terminatori di riga e delle lingue dell'ortografia (`UTF-8`, `CRLF`, `en_US`) | mai: sono nomi, non parole |
| le scorciatoie da tastiera | mai: `Ctrl+S` è `Ctrl+S` in ogni lingua |
| un messaggio che il catalogo non traduce | mostrato in inglese |

Questa versione installa `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR`, `ru`, `sr`, `tr` e `zh_CN` — catalano, ceco, tedesco,
spagnolo, finlandese, francese, galiziano, ebraico, ungherese, interlingue, islandese, italiano, olandese, polacco, portoghese, portoghese brasiliano, russo, serbo, turco e cinese semplificato — tradotte per intero. Il francese è stato rivisto da un lettore della
lingua; catalano, ceco, tedesco, spagnolo, finlandese, galiziano, ebraico, ungherese, interlingue, islandese, italiano, olandese, polacco, portoghese, portoghese brasiliano, russo, serbo, turco e cinese semplificato sono tradotti automaticamente e non ancora
rivisti da un lettore della lingua.
<!-- machine-translated: ca -->
<!-- machine-translated: cs -->
<!-- machine-translated: de -->
<!-- machine-translated: es -->
<!-- machine-translated: fi -->
<!-- machine-translated: gl -->
<!-- machine-translated: he -->
<!-- machine-translated: hu -->
<!-- machine-translated: ie -->
<!-- machine-translated: is -->
<!-- machine-translated: it_IT -->
<!-- machine-translated: nl -->
<!-- machine-translated: pl -->
<!-- machine-translated: pt -->
<!-- machine-translated: pt_BR -->
<!-- machine-translated: ru -->
<!-- machine-translated: sr -->
<!-- machine-translated: tr -->
<!-- machine-translated: zh_CN -->
Su un desktop in francese la barra dei menu mostra
`&Fichier`, `É&dition`, `&Affichage`, `&Vidéo`, `&Outils`, `&Projets`, `Aid&e`. Le schermate di
questo manuale sono acquisite in italiano.
