# `italics`

<!-- machine-translated -->
> Diese Seite ist eine maschinelle Übersetzung und wurde noch von keiner Leserin und keinem Leser dieser Sprache geprüft.

```
subedit-cli italics (--on | --off) [--range N-M|N-]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    <file>...
```

Setzt die Texte **kursiv** oder **nimmt ihre Kursivschrift heraus**, in den Tags des Formats der Datei.
Es ist `Italic` des [Fensters](../subedit-gui/operations.md).

**Zwei Wörter und kein Umschalter**: Die Schaltfläche des Fensters fragt den Kern, in welche Richtung sie
wirkt; hier sagt man es selbst. `--on` und `--off` schließen einander aus, und eines ist nötig — es wegzulassen oder beide anzugeben,
ist ein Aufruffehler (Code `1`).

- **`--on`** umschließt jeden Text mit einem einzigen Paar von Tags, **nachdem die darin
  vorhandenen entfernt wurden**: Ein schon kursiver Text wird nicht zweimal umschlossen, und eine in zwei Teile zerschnittene
  Kursivschrift wird in einem Stück neu gesetzt. Über mehrere Zeilen deckt ein Paar das Ganze ab.
- **`--off`** entfernt die Kursiv-Tags; **der Rest eines Tags bleibt erhalten** (`{\b1\i1}` ergibt
  `{\b1}`).
- Der Text wird **so geschrieben, wie das Format der Datei Kursivschrift schreibt**: `<i>…</i>` für SubRip, WebVTT und
  SubViewer 2, `{\i1}…{\i0}` für die beiden Sub Station Alpha, `{Y:i}` für MicroDVD.

<!-- example: subedit-cli italics --help -->
```console
$ subedit-cli italics --help
Put the texts in italics, or take their italics out
Usage: subedit-cli italics [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --on                        Put the texts in italics
  --off                       Take the italics out
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
| `--on` / `--off` | **eine der beiden** | Schalter | — |
| `--range` | nein | `N-M`, oder `N-` bis zum Ende — siehe [`adjust`](adjust.md#--range) | die ganze Datei |
| `--document` | nein | `main` oder `translation` — siehe [Eine Übersetzung](invocation.md#eine-übersetzung) | `main` |
| `-t`, `--translation-file` | mit `--document translation` | eine Übersetzungsdatei, für **eine einzige** Eingabe | — |
| `--align-method` | nein, und nur mit `-t` | `position` oder `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **eine der drei**, außer mit `--dry-run` | siehe [Aufruf](invocation.md#das-ziel) | — |
| `--dry-run` | nein | ein Schalter: berechnet und sagt, schreibt nichts — siehe [Vor dem Schreiben nachsehen](invocation.md#vor-dem-schreiben-nachsehen) | aus |

**Standardmäßig der Haupttext; die Übersetzung mit `--document translation -t FILE`.** Dann wird
die Übersetzung geändert und geschrieben, und nur sie — siehe [Eine Übersetzung](invocation.md#eine-übersetzung).

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour\nMarie\n\n' > a.srt; subedit-cli italics --on --output i.srt a.srt; cat i.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour\nMarie\n\n' > a.srt; subedit-cli italics --on --output i.srt a.srt; cat i.srt
a.srt: 2 subtitles put in italics -> i.srt
1
00:00:01,000 --> 00:00:02,000
<i>bonjour marie</i>

2
00:00:03,000 --> 00:00:04,000
<i>Bonjour
Marie</i>
```

## Ein Format ohne Stil lehnt ab

**TMPlayer und LRC schreiben keinen Stil**: Es gibt keine Kursivschrift zu setzen und keine herauszunehmen. Das
Fenster graut den Eintrag aus, statt ihn zu verbergen; die Befehlszeile **lehnt ab, Datei für
Datei, mit dem Grund**, und die anderen Dateien des Stapels laufen weiter. Was zählt, ist, was das
Format tragen kann, nicht die Art, wie es schreibt: SubRip und WebVTT schreiben Kursivschrift auf dieselbe
Weise und sagen über Farbe nicht dasselbe.

<!-- example: printf '00:00:01:Hello there\n00:00:03:Bye\n' > a.txt; subedit-cli italics --on --output-dir sortie a.txt; echo "code=$?" -->
```console
$ printf '00:00:01:Hello there\n00:00:03:Bye\n' > a.txt; subedit-cli italics --on --output-dir sortie a.txt; echo "code=$?"
a.txt: TMPlayer writes no style: there are no italics to put on
code=2
```

In JSON trägt der Fehlschlag die Kennung **`no-style`**.

## Ausgabe

**Standardausgabe** — nichts: Das Ergebnis ist die geschriebene Datei. Mit `--dry-run` die Liste der
Änderungen, beschrieben unter [Vor dem Schreiben nachsehen](invocation.md#vor-dem-schreiben-nachsehen).

| Stufe | Was auf der Fehlerausgabe hinzukommt |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles put in italics -> <destination>`, oder `N subtitles taken out of italics`, oder `nothing to change` |
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
| weder `--on` noch `--off` | `italics needs --on to put the texts in italics, or --off` |
| beide | `--on and --off say opposite things; give one of them` |
| ein Format ohne Stil | `<path>: TMPlayer writes no style: there are no italics to put on` (oder `take out`) |
| Bereich falsch geschrieben oder außerhalb der Datei | siehe [`adjust`](adjust.md#--range) |

Die Fehler des Ziels sind den Unterbefehlen gemeinsam, die schreiben:
siehe [Aufruf](invocation.md#das-ziel).
