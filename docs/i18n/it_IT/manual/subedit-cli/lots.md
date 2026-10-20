# Elaborare un albero di directory

<!-- machine-translated -->
> Questa pagina è una traduzione automatica e non è ancora stata rivista da un lettore della lingua.

```
subedit-cli <subcommand> --recursive [options] <directory>...
```

Ogni sottocomando che legge un gruppo di file — `inspect` compreso, ed escluso
[`append`](append.md), [`split-file`](split-file.md) e [`pair`](pair.md), la cui arità è quella di una coppia o
di un film — accetta una **directory** al posto di un file, con `--recursive` (`-r`).
Questa pagina dice che cosa la visita prende, in quale ordine e dove scrive.

**Nulla di quanto segue cambia ciò che viene fatto a un file**: ciascuno viene letto,
elaborato e scritto in modo indipendente, l'errore di uno non ferma gli altri, e i
quattro codici di uscita sono quelli dell'[invocazione](invocation.md#codici-di-uscita).
La visita produce un elenco di percorsi, che il resto dell'elaborazione consuma come se
fossero stati digitati.

## Senza `--recursive`, una directory viene rifiutata

Errore d'uso, codice `1`, nessun file elaborato né scritto:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?" -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?"
films: is a directory: use --recursive
code=1
```

## Che cosa prende la visita

La visita è **ricorsiva, deterministica e priva di sorprese**:

- **l'ordine** — le voci di ogni directory vengono prese nell'ordine dei loro
  nomi, **byte per byte** (le maiuscole prima delle minuscole, a prescindere dalla
  lingua della macchina), e una sottodirectory viene visitata al suo posto, in
  profondità. Due esecuzioni danno gli stessi byte, su qualsiasi
  macchina;
- **i collegamenti simbolici non vengono seguiti** — né verso un file, né verso una
  directory: un albero che contenesse un collegamento a una delle sue directory superiori non avrebbe
  fine;
- **i file e le directory nascosti vengono ignorati** — quelli il cui nome inizia con
  un punto;
- **vengono conservate solo le estensioni dei formati noti**: `.srt`, `.vtt`,
  `.ssa`, `.ass`, `.lrc`, `.sub`, senza distinzione tra maiuscole e minuscole. **`.txt` è escluso**: indica
  due formati, ma soprattutto ogni `LISEZMOI.txt` del mondo, e un'elaborazione che
  fallisse su ciascuno sarebbe inutilizzabile.

**Un file indicato sulla riga di comando non viene mai filtrato.** Indicarlo significa
volerlo: `subedit-cli shift --by 1 --in-place notes.txt` lo elabora, qualunque sia
la sua estensione, e si lamenta solo se il suo contenuto non è in alcun formato.

**Ciò che è stato lasciato da parte viene contato**, e comunicato al livello `-vv`:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli -vv inspect --recursive films 2>&1 | head -1 -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli -vv inspect --recursive films 2>&1 | head -1
films: 1 file left aside, with no extension of a known format
```

Una directory che non contiene nulla da visitare lo dice al livello predefinito
(`<directory>: holds no file in a format this tool walks for`) e non è un errore.

## Dove scrive la visita

**L'albero viene conservato sotto `--output-dir`**: il percorso di un file,
**relativo alla directory indicata**, viene riprodotto sotto la directory di output. Il nome
della directory indicata non ne fa parte — è la semantica di `rsync` con una
barra finale, non quella di `cp -r`: si indica la destinazione e vi si aggiunge il
livello desiderato (`--output-dir sortie/films`).

<!-- example: mkdir -p films/a/b; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/x.srt; cp films/x.srt films/a/x.srt; cp films/x.srt films/a/b/y.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli shift --by 1 --recursive --output-dir sortie films; find sortie -type f | sort -->
```console
$ mkdir -p films/a/b; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/x.srt; cp films/x.srt films/a/x.srt; cp films/x.srt films/a/b/y.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli shift --by 1 --recursive --output-dir sortie films; find sortie -type f | sort
films/a/b/y.srt: 1 subtitle shifted by 1.000 s -> sortie/a/b/y.srt
films/a/x.srt: 1 subtitle shifted by 1.000 s -> sortie/a/x.srt
films/x.srt: 1 subtitle shifted by 1.000 s -> sortie/x.srt
3 of 3 files shifted
sortie/a/b/y.srt
sortie/a/x.srt
sortie/x.srt
```

La directory di output e quelle dell'albero vengono **create** se mancano. Con
`--in-place`, ogni file conservato viene riscritto al suo posto. Un'estensione che
`convert` cambia cambia anche nell'albero: `films/a/x.srt` convertito
in WebVTT viene scritto `sortie/a/x.vtt`.

**La directory di output, se si trova dentro l'albero visitato, è esclusa dalla visita**:
una seconda esecuzione non rilegge ciò che la prima ha scritto.

## Due directory e collisioni

La regola della [destinazione](invocation.md#la-destinazione) decide: due file
che finirebbero allo stesso percorso vengono rifiutati **prima che venga scritto qualcosa**, codice `1`, con i
due input e la destinazione indicati.

<!-- example: mkdir -p un/a deux/a; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > un/a/x.srt; cp un/a/x.srt deux/a/x.srt; subedit-cli shift --by 1 -r --output-dir sortie un deux; echo "code=$?" -->
```console
$ mkdir -p un/a deux/a; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > un/a/x.srt; cp un/a/x.srt deux/a/x.srt; subedit-cli shift --by 1 -r --output-dir sortie un deux; echo "code=$?"
sortie/a/x.srt: would be written by both un/a/x.srt and deux/a/x.srt
code=1
```

## Quanto costa

L'elaborazione resta sequenziale: la sua durata è la somma di quelle dei suoi file. Il
benchmark `make bench` misura la visita e lo stesso lavoro su un singolo file; le cifre
sono nel [registro delle prestazioni](../../../../mesures/performances.md).
