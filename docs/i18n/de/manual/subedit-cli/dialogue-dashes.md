# `dialogue-dashes`

<!-- machine-translated -->
> Diese Seite ist eine maschinelle Übersetzung und wurde noch von keiner Leserin und keinem Leser dieser Sprache geprüft.

```
subedit-cli dialogue-dashes (--add | --remove) [--range N-M|N-]
                            (--output FILE | --output-dir DIR | --in-place)
                            [--dry-run]
                            [--recursive]
                            [--document main|translation] [-t FILE] [--align-method position|number]
                            <file>...
```

Setzt einen **Dialogstrich** an den Anfang der Zeilen oder nimmt ihn weg. Es ist `Dialogue Dashes` des
[Fensters](../subedit-gui/operations.md).

**Zwei Wörter, wie bei [`italics`](italics.md)**: `--add` und `--remove` schließen einander aus, und eines ist nötig.

- **`--add`** setzt einen Strich vor jede Zeile, die keinen hat: `Bonjour` ergibt `- Bonjour`, und
  `Bonjour\nMarie` ergibt `- Bonjour\n- Marie`. **Ein schon vorhandener Strich wird nicht verdoppelt.**
- **Drei Striche werden erkannt** — der Bindestrich der Tastatur und die beiden langen Striche des Schriftsetzers —,
  **und geschrieben wird der Bindestrich**: `— Bonjour` wird zu `- Bonjour`. Überzählige Leerzeichen
  nach dem Strich verschwinden.
- Der Strich kommt **vor den Text, nicht vor ein öffnendes Tag**: `<i>Bonjour</i>` ergibt
  `<i>- Bonjour</i>`. Kein Tag bewegt sich.
- **`--remove`** nimmt die führenden Striche weg, in allen drei Formen.

<!-- example: subedit-cli dialogue-dashes --help -->
```console
$ subedit-cli dialogue-dashes --help
Put dialogue dashes on the lines, or take them off
Usage: subedit-cli dialogue-dashes [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --add                       Put a dialogue dash at the head of the lines
  --remove                    Take the dialogue dashes off
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
| `--add` / `--remove` | **eine der beiden** | Schalter | — |
| `--range` | nein | `N-M`, oder `N-` bis zum Ende — siehe [`adjust`](adjust.md#--range) | die ganze Datei |
| `--document` | nein | `main` oder `translation` — siehe [Eine Übersetzung](invocation.md#eine-übersetzung) | `main` |
| `-t`, `--translation-file` | mit `--document translation` | eine Übersetzungsdatei, für **eine einzige** Eingabe | — |
| `--align-method` | nein, und nur mit `-t` | `position` oder `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **eine der drei**, außer mit `--dry-run` | siehe [Aufruf](invocation.md#das-ziel) | — |
| `--dry-run` | nein | ein Schalter: berechnet und sagt, schreibt nichts — siehe [Vor dem Schreiben nachsehen](invocation.md#vor-dem-schreiben-nachsehen) | aus |

**Standardmäßig der Haupttext; die Übersetzung mit `--document translation -t FILE`.** Dann wird
die Übersetzung geändert und geschrieben, und nur sie — siehe [Eine Übersetzung](invocation.md#eine-übersetzung).

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\nMarie\n\n2\n00:00:03,000 --> 00:00:04,000\n<i>Salut</i>\n\n' > a.srt; subedit-cli dialogue-dashes --add --output t.srt a.srt; cat t.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\nMarie\n\n2\n00:00:03,000 --> 00:00:04,000\n<i>Salut</i>\n\n' > a.srt; subedit-cli dialogue-dashes --add --output t.srt a.srt; cat t.srt
a.srt: 2 subtitles dashed -> t.srt
1
00:00:01,000 --> 00:00:02,000
- Bonjour
- Marie

2
00:00:03,000 --> 00:00:04,000
<i>- Salut</i>
```

## Ausgabe

**Standardausgabe** — nichts: Das Ergebnis ist die geschriebene Datei. Mit `--dry-run` die Liste der
Änderungen, beschrieben unter [Vor dem Schreiben nachsehen](invocation.md#vor-dem-schreiben-nachsehen).

| Stufe | Was auf der Fehlerausgabe hinzukommt |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles dashed -> <destination>`, oder `N subtitles undashed`, oder `nothing to change` |
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
| weder `--add` noch `--remove` | `dialogue-dashes needs --add to put dashes on, or --remove` |
| beide | `--add and --remove say opposite things; give one of them` |
| Bereich falsch geschrieben oder außerhalb der Datei | siehe [`adjust`](adjust.md#--range) |

Die Fehler des Ziels sind den Unterbefehlen gemeinsam, die schreiben:
siehe [Aufruf](invocation.md#das-ziel).
