# `append`

<!-- machine-translated -->
> Diese Seite ist eine maschinelle Übersetzung und wurde noch von keiner Leserin und keinem Leser dieser Sprache geprüft.

```
subedit-cli append --output FILE
                   [--dry-run] [--sort] [--disjoint]
                   <base> <file>...
```

Setzt Dateien **hinter die erste**, in **eine einzige** Datei: der Film in zwei Teilen, zu einem
einzigen Satz Untertitel gemacht. Es ist `Append File…` des Fensters — [derselbe Mechanismus](../subedit-gui/operations.md#append-file),
dieselben Zählungen —, so oft angewendet, wie es anzuhängende Dateien gibt.

- **Die erste Datei ist die Basis.** Sie gibt dem Ergebnis **ihr Format, ihre Kodierung, ihre
  Byte-Reihenfolge-Marke und ihre Zeilenenden**.
- **Zwischen zwei Dateien wird nichts eingefügt**: Jede wird um **das Ende des letzten Untertitels dessen,
  was ihr vorangeht,** verschoben, wobei ihre Positionen ihre Reihenfolge und ihre Abstände behalten. Die dritte folgt der zweiten, nicht der
  Basis: Die Dateien folgen einander in der Reihenfolge, in der sie angegeben werden.
- **Jede angehängte Datei durchläuft die Konvertierung** in das Format der Basis: Ihre Tags werden übersetzt,
  und was dieses Format nicht schreiben kann, wird Datei für Datei gesagt, in den Worten des Fensters.
- **Die angehängten Dateien haben keine Übersetzung**: Eine allein geöffnete Datei hat keine zu geben.
- **Eine Datei kann mehrmals angegeben** oder an sich selbst angehängt werden: Nichts verbietet es.

**`append` ist kein Stapel.** N Eingaben, **eine** Ausgabe: Die Schleife der anderen Unterbefehle — ein Ergebnis
pro Datei, und ein Fehlschlag, der die anderen weiterlaufen lässt — ergibt hier keinen Sinn. Daraus folgen drei Dinge:

- **das Ziel ist `--output`, und nichts anderes.** `--output-dir` und `--in-place` gibt es für
  `append` nicht, und sie werden wie jede unbekannte Option abgelehnt, Code `1`; `--recursive` ebenso: Die Reihenfolge der
  Dateien ist die angegebene, und ein durchlaufener Verzeichnisbaum hat keine;
- **alles oder nichts**: Die erste Datei, die nicht gelesen werden kann, hält den Lauf an, und **nichts wird geschrieben**. Ein
  Film in drei Teilen, von denen einer fehlt, ist kein Film in zwei;
- **eine einzige Zeile pro Lauf** für das Ergebnis, und in JSON **ein einziges Objekt**.

<!-- example: subedit-cli append --help -->
```console
$ subedit-cli append --help
Put subtitle files one after another into a single file
Usage: subedit-cli append [OPTIONS] files...

Positionals:
  files BASE FILE...          The base file, then the files to put after it, in order

Options:
  -h,--help                   Print this help message and exit
  --output FILE               File to write
  --dry-run                   Work out and say what would be written, and write nothing
  --sort                      Put the subtitles in order of their start before writing
  --disjoint                  Cut overlapping subtitles so that no range overlaps another, before writing
```

## Argumente und Optionen

| Option | Erforderlich | Werte | Standard |
| :----- | :----- | :------ | :----- |
| `<base> <file>...` | ja, **mindestens zwei** | Pfade: die Basis, dann was ihr nachgestellt wird, in dieser Reihenfolge | — |
| `--output` | **ja**, außer mit `--dry-run` | ein Dateipfad; sein Verzeichnis wird angelegt, falls es fehlt | — |
| `--dry-run` | nein | ein Schalter: berechnet und sagt, schreibt nichts — siehe [Vor dem Schreiben nachsehen](invocation.md#vor-dem-schreiben-nachsehen) | aus |
| `--sort` | nein | ein Schalter: bringt das Ganze in die Reihenfolge seiner Anfänge, **sobald alles angehängt ist**, siehe [Das Ergebnis ordnen](#das-ergebnis-ordnen) | aus |
| `--disjoint` | nein | ein Schalter: schneidet die überlappenden Untertitel, **sobald alles angehängt und geordnet ist**, siehe [Bereiche überschneidungsfrei machen](#bereiche-überschneidungsfrei-machen) | aus |

**Das Ziel darf keine der Eingaben sein**, die Basis eingeschlossen: Über eine Datei zu schreiben, die man
gerade liest, wird abgelehnt, bevor irgendetwas gelesen wird, Code `1`. Ein vorhandenes `--output`, das sich von den Eingaben unterscheidet, wird
überschrieben, wie überall.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n' > partie1.srt; printf '1\n00:00:00,500 --> 00:00:01,500\ntrois\n\n' > partie2.srt; subedit-cli append --output film.srt partie1.srt partie2.srt; cat film.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n' > partie1.srt; printf '1\n00:00:00,500 --> 00:00:01,500\ntrois\n\n' > partie2.srt; subedit-cli append --output film.srt partie1.srt partie2.srt; cat film.srt
partie2.srt: appended 1 subtitle
partie1.srt: 3 subtitles from 2 files -> film.srt
1
00:00:01,000 --> 00:00:02,000
un

2
00:00:03,000 --> 00:00:04,000
deux

3
00:00:04,500 --> 00:00:05,500
trois
```

Der letzte Untertitel von `partie1.srt` endet bei 4 s; der von `partie2.srt`, der bei 0,5 s begann, beginnt bei
4,5 s.

## Das Ergebnis ordnen

Jede Datei wird **ab dem Ende dessen, was ihr vorangeht,** verschoben: Die Reihenfolge, die brechen kann, ist also die der Dateien selbst. `--sort`
bringt das Ergebnis in die Reihenfolge der Anfänge, **in einem Zug, nach dem letzten Anhängen**, wie es [`sort`](sort.md) täte — stabil und ohne etwas
am Inhalt zu ändern.

Nie standardmäßig. Mit der Option sagt es die Basis auf der ersten Ausführlichkeitsstufe, `<base>: N subtitles moved` oder `<base>: already in order`, und
der Datensatz von `--format json` enthält eine Zählung `moved`, **die ohne die Option fehlt**.

## Bereiche überschneidungsfrei machen

Nie standardmäßig. Die Option tut, was [`disjoint`](disjoint.md) tut, **mit dessen Standardeinstellungen**, sobald alles angehängt ist, am Ganzen. Sie braucht eine geordnete Datei: Andernfalls wird sie abgelehnt, und `--sort` — das zuerst kommt — ordnet sie vorher. Die Datei sagt es auf der ersten Ausführlichkeitsstufe (`<path>: N subtitles split, M merged`), und der Datensatz von `--format json` enthält die Zählungen `split` und `merged`, **die ohne die Option fehlen**.

## Wenn sich die Formate unterscheiden

Das Ergebnis ist im **Format der Basis**, was auch immer das der anderen ist. Was der Übergang kostet, wird
Datei für Datei gesagt, in den Worten des Fensters — derselbe Satz wie bei `Append File…` und beim
[Einfügen aus einem anderen Format](../subedit-gui/clipboard.md#tags-eines-anderen-formats-werden-übersetzt):

```text
d.ass: appended 1 subtitle; Advanced SSA into SubRip: 1 tag dropped, the header was dropped
```

Was verloren geht, hängt nur vom Format **jeder** angehängten Datei ab: Eine Basis in Advanced SSA nimmt eine
SubRip-Datei auf, ohne etwas zu verlieren, nicht umgekehrt. Um das Format des Ergebnisses zu wählen, stellen Sie eine Datei voran,
die schon in diesem Format ist, oder konvertieren Sie danach mit [`convert`](convert.md).

## Ausgabe

**Standardausgabe** — nichts: Das Ergebnis ist die geschriebene Datei.

| Stufe | Was auf der Fehlerausgabe hinzukommt |
| :----- | :------------------------------------- |
| 1 | eine Zeile pro angehängter Datei — `<path>: appended N subtitles`, gefolgt, wenn das Format etwas gekostet hat, von `; <source format> into <base format>: <what is lost>` — dann `<base>: N subtitles from M files -> <destination>` (mit `--dry-run` steht `(dry run, nothing written)` an der Stelle von `-> <destination>`) |
| 2 | `<base>: SubRip, UTF-8, no BOM, LF line endings kept` — das Format, die Kodierung, die Marke und die Zeilenenden der Basis, so wiederhergestellt, wie sie waren |
| 3 | `<base>: N bytes read, M written` (alle Dateien zusammen), dann **jede Diagnose beim Lesen** — siehe [Aufruf](invocation.md#diagnosen-beim-lesen) |

### In JSON

Mit [`--format json`](invocation.md#ausgabe-die-ein-skript-lesen-kann) **ein einziges Objekt** für den ganzen Lauf,
dessen `file` **die Basis** ist. Seine `counts`:

| Zählung | Was sie sagt |
| :----- | :----------- |
| `files` | die Dateien des Laufs, die Basis eingeschlossen |
| `subtitles` | die Untertitel des Ergebnisses |
| `appended` | diejenigen, die aus den angehängten Dateien stammen |
| `lost_ends`, `lost_header` | `0` oder `1`: Eine der angehängten Dateien hat es verloren |
| `joined_lines`, `lost_tags`, `lost_fields` | die Summe dessen, was die angehängten Dateien verloren haben |
| `furthest_ms` | die größte Bewegung einer Position, alle Dateien zusammen |

**Ein Array `inputs`** begleitet die `counts`, außerhalb von ihnen: ein Element pro angehängter Datei, in der Reihenfolge,
`{"file", "counts"}` — wobei `counts` `appended` enthält, dann dieselben Verluste, **für diese Datei allein**. Daran sieht man
in einem Lauf von zehn Dateien, welche was gekostet hat. Die Warnungen beim Lesen aller Dateien
werden in `warnings` gesammelt.

Ein Fehlschlag ist ebenfalls ein Objekt, **ein einziges**: das der ersten Datei, die nicht gelesen werden konnte.

## Exit-Codes

| Code | Wann |
| :--- | :---- |
| `0` | das Ergebnis ist geschrieben — oder berechnet, mit `--dry-run` |
| `1` | Aufruffehler: weniger als zwei Dateien, kein `--output`, eine Option, die `append` nicht hat, ein Ziel, das eine der Eingaben ist |
| `2` | **eine Datei konnte nicht gelesen werden**, oder das Ziel konnte nicht geschrieben werden: Nichts wird geschrieben. Es gibt keinen Code `3`: Ein Lauf, der nur eine Datei schreibt, gelingt oder gelingt nicht |

## Fehler

| Was sie auslöst | Meldung |
| :------------------- | :------ |
| eine einzige Datei angegeben | `append needs a base file and at least one file to put after it` |
| weder `--output` noch `--dry-run` | `no destination given: append writes one file, use --output` |
| `--output` nennt eine Eingabe | `<destination>: would be written over the input <input>` |
| eine Datei fehlt, ist unlesbar oder hat ein unbekanntes Format | `<path>: does not exist`, `<path>: is in no format this tool knows`… — dieselben wie überall |

**Eine leere Datei wird nicht angehängt**: Kein Format liest ohne mindestens einen Untertitel, und die Datei wird
wie jede Datei abgelehnt, die nicht gelesen werden kann — `<path>: holds nothing recognisable as a subtitle` —, was
den Lauf anhält und nichts schreibt.
