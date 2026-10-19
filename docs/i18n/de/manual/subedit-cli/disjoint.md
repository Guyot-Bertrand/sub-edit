# `disjoint`

<!-- machine-translated -->
> Diese Seite ist eine maschinelle Übersetzung und wurde noch von keiner Leserin und keinem Leser dieser Sprache geprüft.

```
subedit-cli disjoint (--output FILE | --output-dir DIR | --in-place)
                     [--minimum TIME] [--max-lines N] [--dashes] [--sort]
                     [--dry-run] [--recursive]
                     <file>...
```

Schneidet die überlappenden Untertitel so, dass **sich kein Bereich `[start, end]` mehr mit einem
anderen überschneidet**. Es ist `Make Ranges Disjoint…` des Fensters — [derselbe Mechanismus](../subedit-gui/lines.md#bereiche-überschneidungsfrei-machen).

**Nichts wird gekürzt oder verlängert**: In jedem Augenblick zeigt die Datei dieselben Texte wie vorher. Was
sich ändert, ist die Art, wie die Zeitachse geschnitten wird:

- sie wird an **jedem Beginn und jedem Ende** der Untertitel einer überlappenden Gruppe geschnitten;
- ein Intervall, das von **einem einzigen** Untertitel abgedeckt wird, bleibt dieser Untertitel;
- ein Intervall, das von **mehreren** abgedeckt wird, wird zu **einem** Untertitel, dessen Zeilen die ihren sind, **in
  der Reihenfolge ihrer Anfänge** — der früheste in Zeile 1, der nächste in Zeile 2;
- ein Untertitel, der nichts überlappt, wird nicht angerührt, und die Reihenfolge der übrigen bleibt erhalten.

<!-- example: subedit-cli disjoint --help -->
```console
$ subedit-cli disjoint --help
Cut overlapping subtitles so that no range overlaps another, merging what overlaps
Usage: subedit-cli disjoint [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to make disjoint

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --minimum TIME              Shortest interval kept; a shorter one is taken over by a neighbour (default 0.08)
  --max-lines N               Most lines of a merged subtitle; more are shown in windows, 0 for no limit (default 2)
  --dashes                    Put a dash before the text of each merged subtitle
  --sort                      Put the subtitles in order of their start first; without it, a file out of order is refused
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
| `--max-lines` | nein | eine ganze Zahl: die meisten Zeilen eines zusammengeführten Untertitels; `0` hebt die Grenze auf | `2` |
| `--dashes` | nein | ein Schalter: ein Strich vor dem Text, den jeder Untertitel in einen zusammengeführten Untertitel einbringt | aus |
| `--sort` | nein | ein Schalter: bringt die Untertitel **zuerst** in die Reihenfolge ihres Beginns — siehe [Eine geordnete Datei ist nötig](#eine-geordnete-datei-ist-nötig) | aus |
| `--recursive`, `-r` | nein | ein Schalter | aus — siehe [Einen Verzeichnisbaum verarbeiten](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **eine der drei**, außer mit `--dry-run` | siehe [Aufruf](invocation.md#das-ziel) | — |
| `--dry-run` | nein | ein Schalter: berechnet und sagt, schreibt nichts — siehe [Vor dem Schreiben nachsehen](invocation.md#vor-dem-schreiben-nachsehen) | aus |

<!-- example: printf '1\n00:00:00,000 --> 00:00:05,000\nA\n\n2\n00:00:03,000 --> 00:00:08,000\nB\n\n' > chevauche.srt; subedit-cli disjoint --output disjoint.srt chevauche.srt; cat disjoint.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:05,000\nA\n\n2\n00:00:03,000 --> 00:00:08,000\nB\n\n' > chevauche.srt; subedit-cli disjoint --output disjoint.srt chevauche.srt; cat disjoint.srt
chevauche.srt: 2 subtitles split, 1 merged -> disjoint.srt
1
00:00:00,000 --> 00:00:03,000
A

2
00:00:03,000 --> 00:00:05,000
A
B

3
00:00:05,000 --> 00:00:08,000
B
```

`A [0, 5]` und `B [3, 8]` ergeben drei überschneidungsfreie Untertitel: `A` allein, `A` und `B` zusammen, `B` allein. Kein
Augenblick geht verloren oder kommt hinzu.

## Was die Operation entscheidet

**Eine Zeile wird nie verdoppelt.** Zwei Untertitel mit demselben Text ergeben dort, wo sie sich
überlappen, eine einzige Zeile, und zwei benachbarte Stücke, die dasselbe zeigen, ergeben nur einen Untertitel. Der Text eines
in mehrere Stücke geschnittenen Untertitels wird ansonsten **in jedem wiederholt**: Er ist das, was in diesem
Augenblick auf dem Bildschirm steht.

**Ein sehr kurzes Intervall wird von einem Nachbarn übernommen.** Unter `--minimum` verschwindet ein Intervall: Der Nachbar
**dehnt seinen Bereich aus, um es abzudecken**, sodass kein Augenblick leer bleibt und sich nichts um mehr als diese
Schwelle bewegt. Gewählt wird der Nachbar, dessen Zeilen die des Intervalls **schon enthalten**, sonst der
längste. Der Text des Intervalls geht nur verloren, wenn kein Nachbar ihn trägt, und dann für weniger als die Schwelle.
**Ein ganzer Untertitel, der kürzer als die Schwelle ist, wird nie übernommen**: Er wird gelassen und gezählt.

**Mehr Zeilen als die Grenze: Fenster.** Ein zusammengeführter Untertitel, der mehr als `--max-lines` Zeilen hätte,
wird **in aufeinanderfolgenden Fenstern** dieser Größe gezeigt, in gleichen Anteilen der Dauer. Drei Zeilen über drei
Sekunden:

<!-- example: printf '1\n00:00:00,000 --> 00:00:03,000\nA\n\n2\n00:00:00,000 --> 00:00:03,000\nB\n\n3\n00:00:00,000 --> 00:00:03,000\nC\n\n' > trois.srt; subedit-cli disjoint --output fenetres.srt trois.srt; cat fenetres.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:03,000\nA\n\n2\n00:00:00,000 --> 00:00:03,000\nB\n\n3\n00:00:00,000 --> 00:00:03,000\nC\n\n' > trois.srt; subedit-cli disjoint --output fenetres.srt trois.srt; cat fenetres.srt
trois.srt: 0 subtitles split, 1 merged; 1 shown in windows for holding too many lines -> fenetres.srt
1
00:00:00,000 --> 00:00:01,500
A
B

2
00:00:01,500 --> 00:00:03,000
B
C
```

Vier Zeilen ergeben drei Fenster, und so weiter: Das Fenster gleitet jeweils um eine Zeile weiter. Nichts wird
abgeschnitten oder erneut geschnitten.

**Die Übersetzung folgt denselben Schnitten**, da ein Untertitel beide Texte trägt. **Was das Format
zusätzlich trägt** — ein Stil, eine Position — ist das des ersten abdeckenden Untertitels; der Verlust wird gezählt, wenn
sich die zusammengeführten Untertitel unterscheiden.

**Bei einem Untertitel, dessen Ende vor seinem Beginn liegt,** wird nicht geraten: Er wird gelassen, wo er ist, und gezählt.

## Eine geordnete Datei ist nötig

Die Gruppen sind Folgen von Nachbarn: Ohne Ordnung sind die Nachbarn nicht diejenigen, die sich überlappen. Eine Datei,
deren Anfänge nicht geordnet sind, wird daher **abgelehnt**, und die Meldung sagt es:

<!-- example: printf '1\n00:00:03,000 --> 00:00:08,000\nB\n\n2\n00:00:00,000 --> 00:00:05,000\nA\n\n' > desordre.srt; subedit-cli disjoint --output x.srt desordre.srt; echo "code $?" -->
```console
$ printf '1\n00:00:03,000 --> 00:00:08,000\nB\n\n2\n00:00:00,000 --> 00:00:05,000\nA\n\n' > desordre.srt; subedit-cli disjoint --output x.srt desordre.srt; echo "code $?"
desordre.srt: The subtitles are not in the order of their start: sort them first. Use --sort to do that here.
code 2
```

`--sort` ordnet zuerst — wie [`sort`](sort.md) — und sagt es. Der Kern ordnet nie von sich aus.

Ein zweiter Durchgang ändert nichts: Sind die Bereiche einmal überschneidungsfrei, bleibt nichts zu schneiden, und der
Satz lautet `already disjoint`.

## Ausgabe

**Standardausgabe** — nichts: Das Ergebnis ist die geschriebene Datei.

| Stufe | Was auf der Fehlerausgabe hinzukommt |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles split, M merged -> <destination>`, gefolgt nach einem `;` von dem, was nicht vollständig getan werden konnte: `K shown in windows for holding too many lines`, `K very short taken over by a neighbour`, `K subtitles shorter than the limit left whole`, `K merged with styles or positions that differ, the first kept`, `K subtitles ending before the start left alone` — oder `<path>: already disjoint -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, dann **jede Diagnose beim Lesen** |

`N subtitles split` zählt die ursprünglichen Untertitel, die in mehrere Stücke geschnitten wurden, `M merged` die Stücke, die
mehrere Untertitel zusammen zeigen.

### In JSON

Mit [`--format json`](invocation.md#ausgabe-die-ein-skript-lesen-kann) die `counts` jeder Datei:

| Zählung | Was sie sagt |
| :----- | :----------- |
| `subtitles` | die Untertitel der geschriebenen Datei |
| `split`, `merged` | wie im Satz |
| `windowed` | die in Fenstern gezeigten Stücke |
| `absorbed` | die von einem Nachbarn übernommenen Intervalle |
| `too_short` | die Untertitel, die kürzer als die Schwelle sind, ganz gelassen |
| `ending_before_start` | die Untertitel, deren Ende vor ihrem Beginn liegt, unangetastet gelassen |
| `extras_dropped` | die zusammengeführten Stücke, deren Stil oder Position sich unterscheiden |
| `moved` | nur mit `--sort`: die Plätze, die sich geändert haben |

## Exit-Codes

Die des Werkzeugs: `0`, wenn jede Datei verarbeitet wurde — eine Datei ohne Überlappungen ist es —, `2`, wenn
keine, `3`, wenn nur einige, `1` bei einem Aufruffehler.

## Fehler

| Was sie auslöst | Meldung |
| :------------------- | :------ |
| eine ungeordnete Datei ohne `--sort` | `<path>: The subtitles are not in the order of their start: sort them first. Use --sort to do that here.` |
| `--minimum` ist keine Dauer oder ist negativ | `--minimum: …` |
| `--max-lines` ist keine ganze Zahl | `--max-lines: "…" is not a number of lines: expected a whole number, 0 for no limit` |

Eine unlesbare Datei scheitert wie bei jedem Unterbefehl; die Fehler des Ziels sind den
Unterbefehlen gemeinsam, die schreiben: siehe [Aufruf](invocation.md#das-ziel).

## In den anderen Befehlen

[`convert`](convert.md#bereiche-überschneidungsfrei-machen), [`append`](append.md#bereiche-überschneidungsfrei-machen) und
[`split-file`](split-file.md#bereiche-überschneidungsfrei-machen) nehmen `--disjoint` an, das dasselbe mit
den Standardeinstellungen tut, vor dem Schreiben.

Bei einer schlecht segmentierten Datei — ein in aufeinanderfolgende Stücke geschnittener Text, ungeordnet geliefert — ergeben Ordnen und Schneiden
allein Zeilen, die von einem Stück zum nächsten umspringen: zuerst [`join-repeats`](join-repeats.md), oder
[`repair`](repair.md), das alle drei verkettet.
