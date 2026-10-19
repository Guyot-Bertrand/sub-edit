# `repair`

<!-- machine-translated -->
> Diese Seite ist eine maschinelle Übersetzung und wurde noch von keiner Leserin und keinem Leser dieser Sprache geprüft.

```
subedit-cli repair (--output FILE | --output-dir DIR | --in-place)
                   [--minimum TIME] [--max-lines N] [--dashes]
                   [--dry-run] [--recursive]
                   <file>...
```

Erledigt in einem Zug, was drei Befehle nacheinander tun: [`join-repeats`](join-repeats.md), dann
[`sort`](sort.md), dann [`disjoint`](disjoint.md). **In dieser Reihenfolge, der einzigen, die funktioniert**: Das Schneiden liest
die Reihenfolge der Anfänge, und die wiederholten Stücke eines Textes sind besser vor beidem verschwunden. Bei einer Datei,
die schlecht segmentiert, ungeordnet und überlappend ist, ist es der Befehl, den man ausführt.

Die Einstellungen sind die von `disjoint`, das die Kette abschließt; `--sort` gibt es hier nicht, da immer geordnet wird.

<!-- example: subedit-cli repair --help -->
```console
$ subedit-cli repair --help
Join the repeated subtitles, put them in order, then make the ranges disjoint
Usage: subedit-cli repair [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to repair

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --minimum TIME              Shortest interval kept; a shorter one is taken over by a neighbour (default 0.08)
  --max-lines N               Most lines of a merged subtitle; more are shown in windows, 0 for no limit (default 2)
  --dashes                    Put a dash before the text of each merged subtitle
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Argumente und Optionen

| Option | Erforderlich | Werte | Standard |
| :----- | :----- | :------ | :----- |
| `<file>...` | ja | ein oder mehrere Pfade | — |
| `--minimum` | nein | eine Dauer: `0.08`, `1.5`, `00:00:00,080`. Null behält jedes Intervall | `0.08` (etwa zwei Bilder) |
| `--max-lines` | nein | eine ganze Zahl; `0` hebt die Grenze auf | `2` |
| `--dashes` | nein | ein Schalter: ein Strich vor dem Text, den jeder Untertitel in einen zusammengeführten Untertitel einbringt | aus |
| `--recursive`, `-r` | nein | ein Schalter | aus — siehe [Einen Verzeichnisbaum verarbeiten](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **eine der drei**, außer mit `--dry-run` | siehe [Aufruf](invocation.md#das-ziel) | — |
| `--dry-run` | nein | ein Schalter: berechnet und sagt, schreibt nichts — siehe [Vor dem Schreiben nachsehen](invocation.md#vor-dem-schreiben-nachsehen) | aus |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:01,000 --> 00:00:04,000\nBeta\n\n' > segments.srt; subedit-cli repair --output repare.srt segments.srt; cat repare.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:01,000 --> 00:00:04,000\nBeta\n\n' > segments.srt; subedit-cli repair --output repare.srt segments.srt; cat repare.srt
segments.srt: 1 repeated subtitle joined; already in order; 0 subtitles split, 1 merged -> repare.srt
1
00:00:01,000 --> 00:00:04,000
Alpha
Beta
```

`Alpha`, in zwei aufeinanderfolgende Stücke geschnitten, wird wieder ein einziger Untertitel; er nimmt dann denselben Bereich ein wie `Beta`,
und die beiden ergeben nur noch einen, mit zwei Zeilen, der früheste zuerst.

## Ausgabe

**Standardausgabe** — nichts: Das Ergebnis ist die geschriebene Datei. Auf der Fehlerausgabe, auf der ersten Stufe, **ein Satz,
der die drei Zählungen verkettet**, durch Semikolons getrennt: was `join-repeats`, `sort` und `disjoint` jeweils sagen,
in ihren eigenen Worten — `N repeated subtitles joined; M subtitles moved; K subtitles split, L merged…`.

### In JSON

Mit [`--format json`](invocation.md#ausgabe-die-ein-skript-lesen-kann) die `counts` jeder Datei: `subtitles`, `joined`,
`moved`, dann die von [`disjoint`](disjoint.md#in-json) (`split`, `merged`, `windowed`, `absorbed`, `too_short`,
`ending_before_start`, `extras_dropped`).

## Exit-Codes und Fehler

Die des Werkzeugs. Kein Fehler ist `repair` eigen, außer bei den Einstellungen, die wie die von `disjoint` beurteilt werden:
ein `--minimum`, das keine Dauer oder negativ ist, ein `--max-lines`, das keine ganze Zahl ist, Code `1`.
