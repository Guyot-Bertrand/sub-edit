# `transform`

<!-- machine-translated -->
> Diese Seite ist eine maschinelle Übersetzung und wurde noch von keiner Leserin und keinem Leser dieser Sprache geprüft.

```
subedit-cli transform --first <index>=<time> --last <index>=<time>
                      (--output FILE | --output-dir DIR | --in-place)
                      [--dry-run]
                      [--recursive]
                      <file>...
```

Korrigiert **jede** Position der Datei anhand von zwei Punkten, deren richtiger Wert
bekannt ist. Es ist die Operation, die eine abdriftende Untertitelung verlangt
— am Anfang richtig, am Ende falsch —, bei der [`shift`](shift.md), das alles um
denselben Betrag bewegt, nichts ausrichten kann.

Das Format der gelesenen Datei wird **beibehalten**: Das Format zu ändern, ist die Aufgabe von
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

## Argumente und Optionen

| Option | Erforderlich | Werte | Standard |
| :----- | :----- | :------ | :----- |
| `<file>...` | ja | ein oder mehrere Pfade | — |
| `--recursive`, `-r` | nein | ein Schalter | aus — siehe [Einen Verzeichnisbaum verarbeiten](lots.md) |
| `--first` | **ja** | eine Referenz, `<index>=<time>` | — |
| `--last` | **ja** | eine Referenz, `<index>=<time>` | — |
| `--output` / `--output-dir` / `--in-place` | **eine der drei**, außer mit `--dry-run` | siehe [Aufruf](invocation.md#das-ziel) | — |
| `--dry-run` | nein | ein Schalter: berechnet und sagt, schreibt nichts — siehe [Vor dem Schreiben nachsehen](invocation.md#vor-dem-schreiben-nachsehen) | aus |

Es gibt keine Kurzform: Diese beiden Optionen werden nur
ausgeschrieben.

## Eine Referenz schreiben

Eine Referenz nennt einen Untertitel und sagt, wohin sein **Beginn** nun fallen muss:

```
<index>=<time>
```

**Der Index wird ab 1 gezählt**, so wie er angezeigt wird und wie eine SubRip-Datei
ihn nummeriert. `0` wird abgelehnt, ebenso ein Index jenseits des letzten
Untertitels.

**Die Zeit wird wie die von [`shift`](shift.md) geschrieben** — Sekunden oder
Zeitstempel, das Dezimaltrennzeichen ein Punkt, die Auflösung die Millisekunde:

| Form | Beispiele |
| :---- | :------- |
| Sekunden | `10`, `2.999` |
| Zeitstempel | `00:00:10.000`, `01:39:37.040` |

`--first 3=10` und `--first 3=00:00:10.000` sagen dasselbe.

`--first` und `--last` nennen zwei Punkte, kein Intervall. Ihre Namen sagen
die erwartete Verwendung — eine Referenz am Anfang der Datei, die andere am Ende — und keine
Einschränkung: Sie zu vertauschen, ergibt dasselbe Ergebnis, und **alles außerhalb
der beiden Referenzen folgt wie das, was zwischen ihnen liegt**.

## Was aus den anderen Positionen wird

Die beiden Referenzen legen einen Skalierungsfaktor fest, und jede Position folgt ihm:

```
t′ = y₁ + (t − x₁) × r      with r = (y₂ − y₁) / (x₂ − x₁)
```

`x₁`, `x₂` sind die aktuellen Anfänge der beiden genannten Untertitel, `y₁`, `y₂` die
verlangten Zeiten. Der Faktor wird als exakte rationale Zahl gehalten, und **gerundet
wird nur einmal**, auf die Millisekunde; das lässt die beiden
Referenzen **genau** auf die verlangten Zeiten fallen und nicht nur auf ein Tausendstel
genau (siehe [ADR 0013](../../../../adr/0013-mise-a-l-echelle-exacte-des-positions.md)).

Jeder Untertitel bleibt **im Verhältnis** dazu auf dem Bildschirm, wie lange er dort blieb:
Das Ende folgt derselben Skala wie der Beginn. Eine Transformation ist keine
Verschiebung und hat deren Eigenschaft nicht.

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

Die erste Referenz bewegt sich nicht, die dritte geht von 9 s auf 10 s: Der Faktor ist
`9/8`, und der zweite Untertitel — den niemand genannt hat — geht von
`5,001` auf `5,501`, also `1000 + 4001 × 9/8`, einmal gerundet.

## Zwei Referenzen, die keine sind

| Situation | Wann sie bemerkt wird | Code |
| :-------- | :------------- | :--- |
| `--first` und `--last` nennen denselben Index | vor jedem Lesen | `1` |
| ein Index liegt jenseits des letzten Untertitels | beim Öffnen der Datei | `2` oder `3` |
| die beiden genannten Untertitel beginnen im selben Augenblick | beim Öffnen der Datei | `2` oder `3` |

Alle drei werden aus demselben Grund abgelehnt: `x₂ − x₁` ist null, und zwei
zusammenfallende Punkte bestimmen keine Gerade. Eine als Ergebnis verkleidete Division
durch null zurückzugeben, wäre schlimmer als abzulehnen.

Der erste Fall ist ein **Aufruffehler** — die beiden Optionen sind schuld, was auch immer
die Dateien sind, also ist eine einzige Meldung fällig, nicht eine pro Datei. Die
beiden anderen hängen von der geöffneten Datei ab und zählen daher als
Fehlschlag der Verarbeitung.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n' > a.srt; subedit-cli transform --first 2=1.000 --last 2=4.000 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n' > a.srt; subedit-cli transform --first 2=1.000 --last 2=4.000 --output b.srt a.srt; echo "code=$?"
--first and --last both name subtitle 2, and two references on one subtitle define no transform
code=1
```

Ein Index außerhalb der Grenzen wird abgelehnt **unter Nennung der Grenze**, was zugleich
sagt, was hätte geschrieben werden können:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,000 --> 00:00:07,000\nDeux.\n\n' > a.srt; subedit-cli transform --first 1=1.000 --last 9=10.000 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,000 --> 00:00:07,000\nDeux.\n\n' > a.srt; subedit-cli transform --first 1=1.000 --last 9=10.000 --output b.srt a.srt; echo "code=$?"
a.srt: subtitle 9 is past the end: the file holds 2 subtitles
code=2
```

## Eine Position vor dem Nullpunkt wird abgelehnt

Wie bei [`shift`](shift.md), und aus demselben Grund: Eine negative Position
wird `-00:00:01,000` geschrieben, eine Form, die keines der neun Formate definiert und die kein
Leseprogramm annimmt. Der Fall tritt hier ein, wenn ein Untertitel **vor
der ersten Referenz** liegt und die Skalierung ihn über null hinaus schiebt. Nichts wird
geschrieben, der Code ist `2`, und die Meldung nennt den betroffenen Untertitel.

## Ausgabe

**Standardausgabe** — nichts: Das Ergebnis ist die geschriebene Datei. Mit `--dry-run` ebenfalls nichts: Es gibt keine Datei, und die Zeile der Verlaufsmeldungen sagt es — siehe [Vor dem Schreiben nachsehen](invocation.md#vor-dem-schreiben-nachsehen).

| Stufe | Was auf der Fehlerausgabe hinzukommt |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles transformed onto 1=00:00:01.000 and 3=00:00:10.000 -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — das Format, die Kodierung, die Marke und die Zeilenenden der gelesenen Datei, so wiederhergestellt, wie sie waren |
| 3 | `<path>: N bytes read, M written`, dann **jede Diagnose beim Lesen** — siehe [Aufruf](invocation.md#diagnosen-beim-lesen) |

Die Referenzen werden mit dem Dezimalpunkt zurückgeschrieben, wie auch immer sie eingegeben wurden.

### In JSON

Mit [`--format json`](invocation.md#ausgabe-die-ein-skript-lesen-kann) enthält das Objekt jeder Datei `counts.subtitles` (jeder Untertitel der Datei). Die vollständige Tabelle steht unter [Aufruf](invocation.md#was-jeder-unterbefehl-in-counts-angibt).

## Exit-Codes

Die des Werkzeugs: `0`, wenn jede Datei geschrieben wurde, `2`, wenn keine, `3`, wenn
nur einige, `1` bei einem Aufruffehler.

## Fehler

| Was ihn auslöst | Meldung |
| :------------------ | :------ |
| Referenz ohne `=` | `"…" is not a reference: write it <index>=<time>, as in 3=00:00:10.000` |
| Referenz, der eine Hälfte fehlt | `"…" is missing one of its two halves: write it <index>=<time>, as in 3=00:00:10.000` |
| Index `0` | `"0" is not a subtitle number: subtitles are counted from 1, as the file shows them` |
| Index, der keine ganze Zahl ist | `"…" is not a subtitle number: expected a whole number, counted from 1 as the file shows them` |
| ungültige Zeit | die von [`shift`](shift.md) |
| `--first` und `--last` mit demselben Index | `--first and --last both name subtitle N, and two references on one subtitle define no transform` |
| Index außerhalb der Grenzen | `<path>: subtitle N is past the end: the file holds M subtitles` |
| zwei Untertitel, die zusammen beginnen | `<path>: subtitles N and M start at the same moment, so they define no transform` |
| Position vor dem Nullpunkt | `<path>: subtitle N would land before the origin, which no subtitle file can hold` |

**Ohne eine der drei Zieloptionen wird nichts geschrieben**, und ihre
Fehler sind den Unterbefehlen gemeinsam, die schreiben:
siehe [Aufruf](invocation.md#das-ziel).
