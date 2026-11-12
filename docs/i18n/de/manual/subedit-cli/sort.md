# `sort`

<!-- machine-translated -->
> Diese Seite ist eine maschinelle Übersetzung und wurde noch von keiner Leserin und keinem Leser dieser Sprache geprüft.

```
subedit-cli sort (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 <file>...
```

Bringt die Untertitel wieder **in die Reihenfolge ihres Beginns**. Sonst ändert sich nichts: weder die Texte noch die
Positionen noch das Format der Datei. Die Untertitel werden beim Schreiben ab 1 **neu nummeriert**,
wie bei jeder zurückgeschriebenen Datei.

**Der Kern ordnet nie von sich aus**: Das Lesen einer Datei lässt ihre Reihenfolge, wie sie war, und
[`inspect`](inspect.md#was-anomalies-meldet) meldet einen Bruch der Reihenfolge mit
`subtitle N starts before the previous one starts`. `sort` ist **der absichtlich ausgeführte Handgriff**, für eine Datei
oder einen Stapel, deren Reihenfolge gestört ist — und er lässt sich im Fenster rückgängig machen, wie jede Operation.

- **Das Ordnen ist stabil**: Zwei Untertitel, die im selben Augenblick beginnen, **behalten die Reihenfolge, die die
  Datei ihnen gab**. Keiner kommt vor dem anderen; sie zu verschieben, wäre eine Entscheidung, die niemand
  verlangt hat.
- **`sort` nimmt weder einen Bereich noch ein Dokument an**: Es gibt nichts zu wählen, die Reihenfolge ist die der Anfänge.

<!-- example: subedit-cli sort --help -->
```console
$ subedit-cli sort --help
Put the subtitles in the order of their start, keeping ties as they are
Usage: subedit-cli sort [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to sort

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

<!-- example: printf '1\n00:00:05,000 --> 00:00:06,000\ncinq\n\n2\n00:00:01,000 --> 00:00:02,000\nun\n\n3\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > desordre.srt; subedit-cli sort --output ordre.srt desordre.srt; cat ordre.srt -->
```console
$ printf '1\n00:00:05,000 --> 00:00:06,000\ncinq\n\n2\n00:00:01,000 --> 00:00:02,000\nun\n\n3\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > desordre.srt; subedit-cli sort --output ordre.srt desordre.srt; cat ordre.srt
[subedit] [sort] [1/1] source: desordre.srt
[subedit] [sort] [1/1] result: 3 subtitles moved
[subedit] [sort] [1/1] dest: ordre.srt
1
00:00:01,000 --> 00:00:02,000
un

2
00:00:03,000 --> 00:00:04,000
trois

3
00:00:05,000 --> 00:00:06,000
cinq
```

## Was gezählt wird

**Gezählt werden die Plätze, deren Untertitel gewechselt hat**, so wie der Kern es meldet — was
der Verlauf des Fensters verschoben nennen würde. Ein einziger falsch platzierter Untertitel verschiebt mehrere: Im
obigen Beispiel behält keiner der drei Plätze seinen Untertitel, und der Satz lautet `3 subtitles moved`.

**Eine schon geordnete Datei wird trotzdem geschrieben** und sagt es: Ein angegebenes Ziel ist ein
geschriebenes Ziel, und `sort` zur Ausnahme zu machen, würde ein Skript zwingen zu wissen, welche
Unterbefehle manchmal eine Datei erzeugen und manchmal nichts.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > ordre.srt; subedit-cli sort --output copie.srt ordre.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > ordre.srt; subedit-cli sort --output copie.srt ordre.srt
[subedit] [sort] [1/1] source: ordre.srt
[subedit] [sort] [1/1] result: already in order
[subedit] [sort] [1/1] dest: copie.srt
```

## Ausgabe

**Standardausgabe** — nichts: Das Ergebnis ist die geschriebene Datei. Mit `--dry-run` ebenfalls nichts: Es gibt
keine Datei, und die Zeile der Verlaufsmeldungen sagt es — siehe [Vor dem Schreiben nachsehen](invocation.md#vor-dem-schreiben-nachsehen).

| Stufe | Was auf der Fehlerausgabe hinzukommt |
| :----- | :------------------------------------- |
| 1 | `source: <path>`, `result: N subtitles moved`, `dest: <destination>`, oder `source: <path>`, `result: already in order`, `dest: <destination>` |
| 2 | `format: SubRip`, `encoding: UTF-8`, `bom: absent`, `line-endings: LF` — das Format, die Kodierung, die Marke und die Zeilenenden der gelesenen Datei, so wiederhergestellt, wie sie waren |
| 3 | `bytes-read: N`, `bytes-written: M`, dann **jede Diagnose beim Lesen** — siehe [Aufruf](invocation.md#diagnosen-beim-lesen) |

### In JSON

Mit [`--format json`](invocation.md#ausgabe-die-ein-skript-lesen-kann) enthält das Objekt jeder Datei
`counts.subtitles` (jeder Untertitel der Datei) und `counts.moved` (die Plätze, die sich geändert haben;
`0` für eine schon geordnete Datei).

## Exit-Codes

Die des Werkzeugs: `0`, wenn jede Datei verarbeitet wurde — eine schon geordnete Datei ist es —, `2`, wenn
keine, `3`, wenn nur einige, `1` bei einem Aufruffehler.

## Fehler

Keine, die `sort` eigen wären: Eine unlesbare Datei scheitert wie bei jedem Unterbefehl, und die
Fehler des Ziels sind den Unterbefehlen gemeinsam, die schreiben:
siehe [Aufruf](invocation.md#das-ziel).
