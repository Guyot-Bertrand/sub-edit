# `join-repeats`

<!-- machine-translated -->
> Diese Seite ist eine maschinelle Übersetzung und wurde noch von keiner Leserin und keinem Leser dieser Sprache geprüft.

```
subedit-cli join-repeats (--output FILE | --output-dir DIR | --in-place)
                         [--dry-run]
                         [--recursive]
                         <file>...
```

Macht aus den Untertiteln, die nacheinander dasselbe zeigen, **einen einzigen Untertitel**: ein in aufeinanderfolgende
Stücke geschnittener Text, wie ihn manche Segmentierungswerkzeuge hinterlassen. Es ist `Join Repeated Subtitles` des Fensters —
[derselbe Mechanismus](../subedit-gui/lines.md#zusammenfassen-wiederholter-untertitel).

**Eine Wiederholung** ist ein Untertitel, der **denselben Text, dieselbe Übersetzung und dieselben Formateinstellungen** (einen
Stil, eine Position, Cue-Einstellungen) hat wie ein anderer **und der spätestens dann beginnt, wenn jener endet**: Sie
berühren oder überlappen sich. Eine Folge von Wiederholungen wird zu **dem Untertitel, der zuerst beginnt**, dessen Ende
auf das späteste Ende der Folge gelegt wird; die anderen verschwinden.

- **Die Reihenfolge der Datei spielt keine Rolle und wird nicht geändert**: Behalten wird der Untertitel, der zuerst beginnt,
  wo auch immer die anderen stehen. Eine geordnete Datei bleibt geordnet.
- **Nichts wird versehentlich zusammengefasst**: ein anderer Text, eine andere Übersetzung, andere Einstellungen, ein Abstand zwischen zwei
  Stücken, ein Untertitel ohne Text oder einer, dessen Ende vor seinem Beginn liegt — in all diesen Fällen bleiben sie getrennt.
- **Vor dem Rest zu erledigen**: Das Ordnen und das [Schneiden der Überlappungen](disjoint.md) lesen eine saubere Folge besser.
  [`repair`](repair.md) verkettet alle drei, und [`convert`](convert.md#bereiche-überschneidungsfrei-machen) nimmt es als Option an (`--join-repeats`, `--repair`).

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

## Argumente und Optionen

| Option | Erforderlich | Werte | Standard |
| :----- | :----- | :------ | :----- |
| `<file>...` | ja | ein oder mehrere Pfade | — |
| `--recursive`, `-r` | nein | ein Schalter | aus — siehe [Einen Verzeichnisbaum verarbeiten](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **eine der drei**, außer mit `--dry-run` | siehe [Aufruf](invocation.md#das-ziel) | — |
| `--dry-run` | nein | ein Schalter: berechnet und sagt, schreibt nichts — siehe [Vor dem Schreiben nachsehen](invocation.md#vor-dem-schreiben-nachsehen) | aus |

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

Die drei Stücke von `Alpha`, die einander von 1 s bis 6 s folgen, ergeben jetzt nur noch eines. `Beta`, durch einen Abstand getrennt,
ist geblieben. Ein zweiter Durchgang ändert nichts und sagt es: `no repeated subtitles`.

## Ausgabe

**Standardausgabe** — nichts: Das Ergebnis ist die geschriebene Datei.

| Stufe | Was auf der Fehlerausgabe hinzukommt |
| :----- | :------------------------------------- |
| 1 | `<path>: N repeated subtitles joined -> <destination>`, oder `<path>: no repeated subtitles -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, dann **jede Diagnose beim Lesen** |

### In JSON

Mit [`--format json`](invocation.md#ausgabe-die-ein-skript-lesen-kann) die `counts` jeder Datei: `subtitles` (die der geschriebenen
Datei), `joined` (die entfernten Untertitel, weil ein anderer sie abdeckt) und `chains` (die zusammengefassten Folgen).

## Exit-Codes

Die des Werkzeugs: `0`, wenn jede Datei verarbeitet wurde — eine Datei ohne Wiederholungen ist es —, `2`, wenn keine, `3`, wenn
nur einige, `1` bei einem Aufruffehler.

## Fehler

Keine, die `join-repeats` eigen wären: Eine unlesbare Datei scheitert wie bei jedem Unterbefehl, und die Fehler
des Ziels sind den Unterbefehlen gemeinsam, die schreiben: siehe [Aufruf](invocation.md#das-ziel).
