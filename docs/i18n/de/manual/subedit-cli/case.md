# `case`

<!-- machine-translated -->
> Diese Seite ist eine maschinelle Übersetzung und wurde noch von keiner Leserin und keinem Leser dieser Sprache geprüft.

```
subedit-cli case --to title|sentence|upper|lower [--range N-M|N-]
                 (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 [--document main|translation] [-t FILE] [--align-method position|number]
                 <file>...
```

Setzt die Texte in **Titelschreibweise, Satzschreibweise, Großbuchstaben oder Kleinbuchstaben**, ohne ein
Tag anzurühren. Es ist `Werkzeuge ▸ Groß-/Kleinschreibung` des [Fensters](../subedit-gui/operations.md), mit seinen Regeln.

| `--to` | Wirkung | `bonjour marie` wird zu |
| :----- | :---- | :---------------------- |
| `title` | jedes Wort erhält einen Großbuchstaben, der Rest wird klein | `Bonjour Marie` |
| `sentence` | der erste Buchstabe des Textes, und nur er; der Rest wird klein | `Bonjour marie` |
| `upper` | alles in Großbuchstaben | `BONJOUR MARIE` |
| `lower` | alles in Kleinbuchstaben | `bonjour marie` |

- **Was vor dem ersten Buchstaben oder der ersten Ziffer steht, wird nicht angerührt**: Ein Dialogstrich,
  ein öffnendes Anführungszeichen oder eine Klammer bleiben bestehen (`- bonjour` ergibt `- Bonjour`).
- **Tags sind kein Text**: Sie ändern ihre Schreibweise nicht — `<I>` bleibt `<I>` — und ändern
  nicht ihren Platz: Ein Tag, das ein Wort zerschneidet, zerschneidet es auch danach.
- **ICU entscheidet, wo ein Wort beginnt**: `l'été sera chaud` ergibt `L'été Sera Chaud`, und die Akzente
  bleiben erhalten (`éléonore` ergibt `Éléonore`).

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

## Argumente und Optionen

| Option | Erforderlich | Werte | Standard |
| :----- | :----- | :------ | :----- |
| `<file>...` | ja | ein oder mehrere Pfade | — |
| `--recursive`, `-r` | nein | ein Schalter | aus — siehe [Einen Verzeichnisbaum verarbeiten](lots.md) |
| `--to` | **ja** | `title`, `sentence`, `upper`, `lower` | — |
| `--range` | nein | `N-M`, oder `N-` bis zum Ende — siehe [`adjust`](adjust.md#--range) | die ganze Datei |
| `--document` | nein | `main` oder `translation` — siehe [Eine Übersetzung](invocation.md#eine-übersetzung) | `main` |
| `-t`, `--translation-file` | mit `--document translation` | eine Übersetzungsdatei, für **eine einzige** Eingabe | — |
| `--align-method` | nein, und nur mit `-t` | `position` oder `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **eine der drei**, außer mit `--dry-run` | siehe [Aufruf](invocation.md#das-ziel) | — |
| `--dry-run` | nein | ein Schalter: berechnet und sagt, schreibt nichts — siehe [Vor dem Schreiben nachsehen](invocation.md#vor-dem-schreiben-nachsehen) | aus |

**Standardmäßig der Haupttext; die Übersetzung mit `--document translation -t FILE`.** Dann wird
die Übersetzung geändert und geschrieben, und nur sie — siehe [Eine Übersetzung](invocation.md#eine-übersetzung).

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

Eine Datei, in der sich **kein Text ändert** — alles ist schon in der verlangten Schreibweise —, wird so geschrieben, wie sie ist, und
sagt es:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBONJOUR\n\n' > a.srt; subedit-cli case --to upper --output b.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBONJOUR\n\n' > a.srt; subedit-cli case --to upper --output b.srt a.srt
a.srt: nothing to change -> b.srt
```

## Ausgabe

**Standardausgabe** — nichts: Das Ergebnis ist die geschriebene Datei. Mit `--dry-run` die Liste der
Änderungen, beschrieben unter [Vor dem Schreiben nachsehen](invocation.md#vor-dem-schreiben-nachsehen).

| Stufe | Was auf der Fehlerausgabe hinzukommt |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles recased -> <destination>`, oder `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — das Format, die Kodierung, die Marke und die Zeilenenden der gelesenen Datei, so wiederhergestellt, wie sie waren |
| 3 | `<path>: N bytes read, M written`, dann **jede Diagnose beim Lesen** — siehe [Aufruf](invocation.md#diagnosen-beim-lesen) |

`N` ist die Anzahl der Untertitel, **deren Text sich geändert hat**, von der Operation selbst gelesen.

### In JSON

Mit [`--format json`](invocation.md#ausgabe-die-ein-skript-lesen-kann) enthält das Objekt jeder Datei
`counts.changed` — das `N` des Satzes — und `changes`, die Liste der geänderten Untertitel
(`{"subtitle", "document", "before", "after"}`, **leer**, wenn sich nichts geändert hat).

## Exit-Codes

Die des Werkzeugs: `0`, wenn jede Datei verarbeitet wurde — eine Datei, in der sich nichts ändert, ist es —,
`2`, wenn keine, `3`, wenn nur einige, `1` bei einem Aufruffehler.

## Fehler

| Was ihn auslöst | Meldung |
| :------------------ | :------ |
| `--to` fehlt | `--to is required` |
| ein Wert, der keiner der vier ist | `--to: … not in {title,sentence,upper,lower}` |
| Bereich falsch geschrieben oder außerhalb der Datei | siehe [`adjust`](adjust.md#--range) |

Die Fehler des Ziels sind den Unterbefehlen gemeinsam, die schreiben:
siehe [Aufruf](invocation.md#das-ziel).
