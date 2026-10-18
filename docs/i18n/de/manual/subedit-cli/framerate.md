# `framerate`

<!-- machine-translated -->
> Diese Seite ist eine maschinelle Übersetzung und wurde noch von keiner Leserin und keinem Leser dieser Sprache geprüft.

> **Das ist nicht [`snap`](snap.md).** Dieser Befehl **skaliert die
> Datei**: Er ist für den Fall gedacht, dass das Timing falsch ist und bis zum Ende des Films um mehrere
> Sekunden abdriftet. `snap` setzt die Zeitstempel wieder auf das nächstgelegene
> Raster, ohne etwas umzurechnen, und ist für den Fall gedacht, dass das Timing schon
> stimmt. Beide haben dieselbe Form, eine Bildrate oder zwei, und eine Verwechslung verursacht
> keinen Fehler.

```
subedit-cli framerate --from <fps> --to <fps>
                      (--output FILE | --output-dir DIR | --in-place)
                      [--dry-run]
                      [--recursive]
                      <file>...
```

Rechnet eine für eine Bildrate getimte Datei auf eine andere um. Es ist
die Operation, die eine Untertiteldatei verlangt, wenn sie für eine Fassung erstellt wurde, die
nicht die des vorliegenden Films ist — etwa eine PAL-DVD gegenüber einer NTSC-Überspielung.

Das Format der gelesenen Datei wird **beibehalten**: Das Format zu ändern, ist die Aufgabe von
[`convert`](convert.md).

<!-- example: subedit-cli framerate --help -->
```console
$ subedit-cli framerate --help
Re-time a file mastered at one frame rate for another
Usage: subedit-cli framerate [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to re-time

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --from TEXT REQUIRED        Frame rate the file is timed at: 25, 23.976
  --to TEXT REQUIRED          Frame rate to time it for: 24, 29.97
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
| `--from` | **ja** | eine Bildrate, siehe unten | — |
| `--to` | **ja** | eine Bildrate, siehe unten | — |
| `--output` / `--output-dir` / `--in-place` | **eine der drei**, außer mit `--dry-run` | siehe [Aufruf](invocation.md#das-ziel) | — |
| `--dry-run` | nein | ein Schalter: berechnet und sagt, schreibt nichts — siehe [Vor dem Schreiben nachsehen](invocation.md#vor-dem-schreiben-nachsehen) | aus |

`--from` ist die Bildrate, für die die Datei getimt **ist**, `--to` die, für die
sie gewünscht wird. Sie zu vertauschen, verschiebt die Datei in die
andere Richtung — bei einem Spielfilm zwischen 25 und
23,976 um rund sieben Sekunden —, und nichts weist darauf hin, bevor der Film angesehen wird.

## Eine Bildrate schreiben

In Bildern pro Sekunde, Dezimalstellen eingeschlossen, mit dem Punkt als Trennzeichen:

```
25      24      23.976      29.97      60
```

**Eine Bildrate von null oder darunter wird abgelehnt**: Das ist keine Bildrate.

### Die drei NTSC-Bildraten lassen sich nicht genau schreiben

`23.976`, `29.97` und `59.94` sind **Bezeichnungen**. Die Bildraten, die sie
benennen, sind `24000/1001`, `30000/1001` und `60000/1001`, von denen keine
eine endliche Dezimalschreibweise hat. Das Werkzeug erkennt sie als solche:

| Was geschrieben wird | Was es benennt |
| :------------- | :------------------ |
| `23.976` | `24000/1001` — auf NTSC-Video übertragenes Kino |
| `24` | `24` — Kino |
| `25` | `25` — PAL- und SECAM-Video |
| `29.97` | `30000/1001` — NTSC-Video |
| `30`, `50`, `60` | sich selbst |
| `59.94` | `60000/1001` |

Das Erkennen betrifft **den Wert und nicht die Zeichen**: `23.9760`
benennt dieselbe Bildrate wie `23.976`.

Jede andere Dezimalzahl wird wörtlich genommen — `23.9` ist niemandes
Standard, es gibt nichts hineinzudeuten. Das macht auch die obige Tabelle
nötig: Ohne sie wäre `23.976` gleich `23976/1000`, und die Befehlszeile
sagte für dieselben Worte etwas anderes als das Fenster.

## Was aus den Positionen wird

```
t′ = round( t × (from / to) )
```

Ein mit 23,976 gemasterter und mit 25 Bildern pro Sekunde abgespielter Film läuft schneller,
also kommen seine Untertitel früher; in die andere Richtung kommen sie
später.

**Der Faktor ist eine exakte rationale Zahl, und gerundet wird nur einmal.** Der
Weg, der gleichwertig aussieht — die Position bei der Eingangsrate in eine Bildnummer umzurechnen,
dann bei der Ausgangsrate zurück in Millisekunden —, ist es
nicht: Er quantisiert jede Position auf dem Eingangsraster und liegt um bis zu
ein halbes Bild daneben, gemessen 21 ms. Siehe
[ADR 0013](../../../../adr/0013-mise-a-l-echelle-exacte-des-positions.md).

In der Praxis, von 25 auf 23,976, ist der Faktor `1001/960`:

| Gelesene Position | Geschrieben | Was der Weg über die Bilder ergäbe |
| -----------: | -----: | -----------------------------------------: |
| `1 010 ms` | `1 053 ms` | `1 043 ms` |
| `3 600 017 ms` | `3 753 768 ms` | `3 753 750 ms` |

<!-- example: printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 25 --to 23.976 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 25 --to 23.976 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle retimed from 25 to 24000/1001 fps -> b.srt
1
00:00:01,053 --> 00:00:02,106
Un.
```

## Ausgabe

**Standardausgabe** — nichts: Das Ergebnis ist die geschriebene Datei. Mit `--dry-run` ebenfalls nichts: Es gibt keine Datei, und die Zeile der Verlaufsmeldungen sagt es — siehe [Vor dem Schreiben nachsehen](invocation.md#vor-dem-schreiben-nachsehen).

| Stufe | Was auf der Fehlerausgabe hinzukommt |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles retimed from 25 to 24000/1001 fps -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — das Format, die Kodierung, die Marke und die Zeilenenden der gelesenen Datei, so wiederhergestellt, wie sie waren |
| 3 | `<path>: N bytes read, M written`, dann **jede Diagnose beim Lesen** — siehe [Aufruf](invocation.md#diagnosen-beim-lesen) |

**Die Verlaufsmeldungen nennen die exakte Bildrate, nicht die eingegebene Bezeichnung.** In dieser Zeile
`23.976` zu schreiben, würde eine Umrechnung melden, die nicht stattgefunden hat; es ist
die einzige Stelle, an der man sieht, welche der beiden verwendet wurde. Eine Bildrate, die eine
Dezimalzahl genau schreibt, wird dort als Dezimalzahl gezeigt, die anderen als Bruch.

### In JSON

Mit [`--format json`](invocation.md#ausgabe-die-ein-skript-lesen-kann) enthält das Objekt jeder Datei `counts.subtitles` (jeder Untertitel der Datei). Die vollständige Tabelle steht unter [Aufruf](invocation.md#was-jeder-unterbefehl-in-counts-angibt).

## Exit-Codes

Die des Werkzeugs: `0`, wenn jede Datei geschrieben wurde, `2`, wenn keine, `3`, wenn
nur einige, `1` bei einem Aufruffehler.

Keine Ablehnung hängt vom Inhalt einer Datei ab: Zwei Bildraten bestimmen immer
einen Faktor. Es ist der einzige der schreibenden Unterbefehle, dessen
Ablehnungen alle vor dem ersten Lesen kommen.

<!-- example: printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 0 --to 25 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 0 --to 25 --output b.srt a.srt; echo "code=$?"
"0" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed
code=1
```

## Fehler

| Was ihn auslöst | Meldung |
| :------------------ | :------ |
| Bildrate null oder negativ | `"…" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed` |
| Dezimalkomma | `"…" is not a frame rate: use a decimal point and not a comma, in every language` |
| Text, der keine Zahl ist | `"…" is not a frame rate: expected frames per second, like 25 or 23.976` |
| Bildrate jenseits jedes Maßes | `"…" is not a frame rate: no video runs at that many frames per second` |

**Ohne eine der drei Zieloptionen wird nichts geschrieben**, und ihre
Fehler sind den Unterbefehlen gemeinsam, die schreiben:
siehe [Aufruf](invocation.md#das-ziel).
