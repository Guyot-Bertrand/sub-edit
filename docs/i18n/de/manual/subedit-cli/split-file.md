# `split-file`

<!-- machine-translated -->
> Diese Seite ist eine maschinelle Übersetzung und wurde noch von keiner Leserin und keinem Leser dieser Sprache geprüft.

```
subedit-cli split-file --at N --head FILE --tail FILE
                       [--dry-run] [--sort] [--disjoint]
                       <file>
```

Schneidet eine Datei **in zwei Teile**: einen Anfang und einen Rest. Es ist `Split Project…` des Fensters — [derselbe Mechanismus](../subedit-gui/operations.md#split-project),
dieselben Zählungen, dieselbe Ablehnung — und **die genaue Umkehrung von [`append`](append.md)**: Den Rest an den
Anfang anzuhängen, ergibt wieder die Ausgangsdatei.

- **`--at N` ist die Nummer des ersten Untertitels des Rests**, so nummeriert, wie die Tabelle des Fensters
  nummeriert: ab 1. `--at 3` legt die Untertitel 1 und 2 in den Anfang und den dritten samt den übrigen in den
  Rest. Es reicht **vom zweiten bis zum letzten**: Ein Schnitt braucht auf jeder Seite einen Untertitel.
- **Der Rest wird an den Nullpunkt zurückgebracht**: Er wird um minus das Ende des letzten im Anfang verbliebenen Untertitels verschoben,
  wobei seine Positionen ihre Reihenfolge und ihre Abstände behalten. Das macht `split-file` zur Umkehrung von `append`.
- **Beide Hälften behalten das Format, die Kodierung, die Byte-Reihenfolge-Marke und die Zeilenenden der gelesenen Datei** und
  werden beim Schreiben ab 1 neu nummeriert, wie jede zurückgeschriebene Datei.
- **Zwei Ausgaben, also ein eigenes Ziel**: `--head` und `--tail`, **beide erforderlich**. `--output`,
  `--output-dir`, `--in-place` und `--recursive` gibt es für `split-file` nicht, und sie werden wie jede unbekannte
  Option abgelehnt, Code `1`: Es gibt weder ein Verzeichnis, auf das zwei benannte Dateien zu verteilen wären, noch eine zu überschreibende Eingabe.

<!-- example: subedit-cli split-file --help -->
```console
$ subedit-cli split-file --help
Cut a subtitle file in two, a head and a tail
Usage: subedit-cli split-file [OPTIONS] file

Positionals:
  file TEXT REQUIRED          Subtitle file to cut

Options:
  -h,--help                   Print this help message and exit
  --at N                      Number of the first subtitle of the tail, as the table numbers them
  --head FILE                 File to write the subtitles before the cut to
  --tail FILE                 File to write the subtitles from the cut on to
  --dry-run                   Work out and say what would be written, and write nothing
  --sort                      Put the subtitles in order of their start before cutting; --at counts in that order
  --disjoint                  Cut overlapping subtitles so that no range overlaps another, before cutting
```

## Argumente und Optionen

| Option | Erforderlich | Werte | Standard |
| :----- | :----- | :------ | :----- |
| `<file>` | ja | **ein einziger** Pfad | — |
| `--at` | ja | eine ganze Zahl, von 2 bis zur Anzahl der Untertitel der Datei | — |
| `--head`, `--tail` | **ja, beide**, außer mit `--dry-run` | Dateipfade; ihr Verzeichnis wird angelegt, falls es fehlt | — |
| `--dry-run` | nein | ein Schalter: berechnet und sagt, schreibt nichts — siehe [Vor dem Schreiben nachsehen](invocation.md#vor-dem-schreiben-nachsehen) | aus |
| `--sort` | nein | ein Schalter: bringt die Untertitel **vor dem Schneiden** in die Reihenfolge ihres Beginns; `--at` zählt in dieser Reihenfolge | aus |
| `--disjoint` | nein | ein Schalter: schneidet die überlappenden Untertitel **vor dem Schneiden**; `--at` zählt in dem, was übrig bleibt, siehe [Bereiche überschneidungsfrei machen](#bereiche-überschneidungsfrei-machen) | aus |

**Mit `--dry-run` ist kein Ziel erforderlich**: Es darf keines angegeben werden, aber nicht nur eines der beiden —
`--head` und `--tail` gehören zusammen. Die angegebenen werden beurteilt wie ohne `--dry-run`.

**Die beiden Ziele dürfen weder die Eingabe noch dieselbe Datei sein**: Über das zu schreiben, was man
gerade liest, oder den Rest über den Anfang zu schreiben, wird abgelehnt, bevor irgendetwas gelesen wird, Code `1`.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n3\n00:00:06,000 --> 00:00:07,000\ntrois\n\n4\n00:00:08,000 --> 00:00:09,000\nquatre\n\n' > film.srt; subedit-cli split-file --at 3 --head tete.srt --tail queue.srt film.srt; cat queue.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n3\n00:00:06,000 --> 00:00:07,000\ntrois\n\n4\n00:00:08,000 --> 00:00:09,000\nquatre\n\n' > film.srt; subedit-cli split-file --at 3 --head tete.srt --tail queue.srt film.srt; cat queue.srt
film.srt: split at subtitle 3: 2 subtitles in the head, 2 subtitles in the tail -> tete.srt, queue.srt
1
00:00:02,000 --> 00:00:03,000
trois

2
00:00:04,000 --> 00:00:05,000
quatre
```

Der zweite Untertitel, der im Anfang bleibt, endet bei 4 s: Der Rest geht um 4 s zurück, und der dritte Untertitel,
der bei 6 s begann, beginnt bei 2 s.

## In zeitlicher Reihenfolge schneiden

`--at` nummeriert die Untertitel **so, wie die Datei sie liefert**. Bei einer Datei, deren Reihenfolge gestört ist, ist der Anfang daher nicht die zeitlich
frühere Hälfte. `--sort` bringt die Untertitel **vor der Beurteilung des Schnitts** in die Reihenfolge ihrer Anfänge — stabil, wie es [`sort`](sort.md) tut —, und
`--at` zählt dann in dieser Reihenfolge.

Nie standardmäßig. Mit der Option sagt die Datei auf der ersten Ausführlichkeitsstufe, wie viele Untertitel sich bewegt haben (`<path>: N subtitles moved`
oder `<path>: already in order`), und der Datensatz von `--format json` enthält eine Zählung `moved`, **die ohne die Option fehlt**.

## Bereiche überschneidungsfrei machen

Nie standardmäßig. Die Option tut, was [`disjoint`](disjoint.md) tut, **mit dessen Standardeinstellungen**, vor der Beurteilung des Schnitts, nach dem Ordnen: `--at` zählt in dem, was sich daraus ergibt. Sie braucht eine geordnete Datei: Andernfalls wird sie abgelehnt, und `--sort` — das zuerst kommt — ordnet sie vorher. Die Datei sagt es auf der ersten Ausführlichkeitsstufe (`<path>: N subtitles split, M merged`), und der Datensatz von `--format json` enthält die Zählungen `split` und `merged`, **die ohne die Option fehlen**.

## Wenn der Schnitt abgelehnt wird

**Zwei Hälften, die sich am Schnitt überlappen, werden nicht geschnitten.** Begänne ein Untertitel des Rests vor
dem Ende des letzten zurückgelassenen Untertitels, würde ihn die Verschiebung vor den Beginn des Videos tragen — eine negative Position,
die kein Format schreibt. `split-file` lehnt ab und **nennt dabei den Untertitel**, in den Worten des Fensters:

```text
o.srt: Cannot split at subtitle 3: subtitle 3 would fall before the start of the video. Cut somewhere else.
```

Der Code ist `2`, und **nichts wird geschrieben**. Schneiden Sie anderswo. Genau auf dem Beginn zu landen, ist erlaubt: Null ist eine
Position.

**`--at` außerhalb der Datei wird ebenso abgelehnt**, sobald die Datei gelesen ist: `--at 1` — der erste Untertitel kann keinen
Rest beginnen, es bliebe nichts übrig — und `--at` jenseits des letzten Untertitels, was einen leeren
Rest hinterließe. Eine Datei mit einem einzigen Untertitel kann nicht geschnitten werden.

## Ausgabe

**Standardausgabe** — nichts: Das Ergebnis sind die beiden geschriebenen Dateien.

| Stufe | Was auf der Fehlerausgabe hinzukommt |
| :----- | :------------------------------------- |
| 1 | `<path>: split at subtitle N: H subtitles in the head, T subtitles in the tail -> <head>, <tail>`; mit `--dry-run` steht `(dry run, nothing written)` an der Stelle von `-> …` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — das Format, die Kodierung, die Marke und die Zeilenenden der gelesenen Datei, so wiederhergestellt, wie sie waren |
| 3 | `<path>: N bytes read, H and T written`, dann **jede Diagnose beim Lesen** — siehe [Aufruf](invocation.md#diagnosen-beim-lesen) |

### In JSON

Mit [`--format json`](invocation.md#ausgabe-die-ein-skript-lesen-kann) **ein einziges Objekt**, dessen `file` die geschnittene Datei ist.
**`destination` ist der Anfang** — die Datei, zu der die Eingabe wird — und **`tail` der Rest**, ein Pfad, oder `null` mit
`--dry-run` wie `destination`. Die `counts`: `subtitles` (die der gelesenen Datei), `head` und `tail` (die jeder
Hälfte).

## Exit-Codes

| Code | Wann |
| :--- | :---- |
| `0` | beide Dateien sind geschrieben — oder berechnet, mit `--dry-run` |
| `1` | Aufruffehler: kein `--head` und `--tail`, eine Option, die `split-file` nicht hat, ein Ziel, das die Eingabe oder das andere Ziel ist, ein `--at`, das keine positive ganze Zahl ist |
| `2` | die Datei konnte nicht gelesen werden, **der Schnitt wird abgelehnt** (außerhalb der Datei oder Überlappung), oder ein Ziel konnte nicht geschrieben werden. Es gibt keinen Code `3`: Es wird eine einzige Datei verarbeitet |

**Der Anfang wird zuerst geschrieben**: Ein Datenträger, der den Rest verweigert, lässt den Anfang zurück, und der Code ist `2`.

## Fehler

| Was sie auslöst | Meldung |
| :------------------- | :------ |
| weder `--head` noch `--tail` | `no destination given: split-file writes two files, use --head and --tail` |
| nur eines der beiden, mit `--dry-run` | `--head and --tail go together: give both, or neither with --dry-run` |
| die beiden Ziele sind dieselbe Datei | `<path>: named as both the head and the tail` |
| ein Ziel ist die Eingabe | `<path>: would be written over the input <input>` |
| `--at` außerhalb der Datei | `<file>: --at 9: the file holds 3 subtitles, and the tail can start from 2 to 3` |
| eine Datei mit einem Untertitel | `<file>: a file of 1 subtitle cannot be split: a cut needs a subtitle on each side` |
| Überlappung | `<file>: Cannot split at subtitle N: subtitle M would fall before the start of the video. Cut somewhere else.` |
| eine Datei fehlt, ist unlesbar oder hat ein unbekanntes Format | `<path>: does not exist`… — dieselben wie überall |
