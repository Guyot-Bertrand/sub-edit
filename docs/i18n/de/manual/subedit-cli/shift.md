# `shift`

<!-- machine-translated -->
> Diese Seite ist eine maschinelle Übersetzung und wurde noch von keiner Leserin und keinem Leser dieser Sprache geprüft.

```
subedit-cli shift (--by <time> | --to-grid)
                  (--output FILE | --output-dir DIR | --in-place)
                  [--dry-run]
                  [--recursive]
                  <file>...
```

Verschiebt **jede** Position der Datei um denselben Betrag, in die eine oder
die andere Richtung. Kein Untertitel bleibt länger oder kürzer auf dem Bildschirm:
Genau das macht eine Verschiebung zu einer Verschiebung.

<!-- example: subedit-cli shift --help -->
```console
$ subedit-cli shift --help
Move every position of a file by a fixed amount
Usage: subedit-cli shift [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to shift

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --by TEXT                   Amount to move by: 2.999, -7.001, or 00:00:07.001
  --to-grid                   Move by the amount that puts the positions back on their frame grid
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
| `--by` | **eine der beiden** | eine Dauer mit Vorzeichen, siehe unten | — |
| `--to-grid` | **eine der beiden** | Schalter: Der Betrag wird gemessen, siehe unten | — |
| `--output` / `--output-dir` / `--in-place` | **eine der drei**, außer mit `--dry-run` | siehe [Aufruf](invocation.md#das-ziel) | — |
| `--dry-run` | nein | ein Schalter: berechnet und sagt, schreibt nichts — siehe [Vor dem Schreiben nachsehen](invocation.md#vor-dem-schreiben-nachsehen) | aus |

Das Format der gelesenen Datei wird **beibehalten**: Das Format zu ändern, ist die Aufgabe von
[`convert`](convert.md).

## `--to-grid`: der Betrag gemessen statt angegeben

Eine Datei, deren Positionen **bis auf eine Konstante** auf einem Raster von Bildern
liegen, wurde verschoben, und diese Konstante lässt sich an den Positionen selbst messen.
`--to-grid` misst sie und wendet sie an, ohne dass jemand sie kennen muss.

Der Betrag ist **jeder Datei eigen**: Zwei Dateien, die um zwei verschiedene Beträge
von demselben Raster verschoben wurden, kehren jede um ihren eigenen zurück. Er wird
in den Verlaufsmeldungen genannt, zusammen mit der gefundenen Bildrate.

**Der Weg ist der kürzeste.** Jenseits eines halben Bildes ist die nächste Rasterlinie
die folgende: Die Korrektur geht nach vorn, statt um fast ein ganzes Bild
zurückzugehen.

<!-- example: k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40 + 7)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > decale.srt; subedit-cli shift --to-grid --output recale.srt decale.srt -->
```console
$ k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40 + 7)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > decale.srt; subedit-cli shift --to-grid --output recale.srt decale.srt
decale.srt: 12 subtitles shifted by -0.007 s onto their 25 fps grid -> recale.srt
```

**Eine Datei auf keinem bekannten Raster wird abgelehnt, mit Angabe des Grundes.** Es
gibt nichts, wohin sie zurückkehren könnte, und eine am Rauschen gemessene Phase würde die Datei
zufällig verschieben. Siehe [`inspect`](inspect.md#was-frame-rate-grid-meldet), um zu
erfahren, was eine Datei ergibt, bevor Sie die Operation versuchen.

`--by` und `--to-grid` sagen beide, um wie viel verschoben wird: Beide zusammen anzugeben,
ist ein Aufruffehler, und keine von beiden anzugeben ebenso.

**`--to-grid` ist nicht [`snap`](snap.md).** Es verschiebt die ganze Datei
um denselben Betrag und bewahrt das relative Timing **genau**; `snap`
bewegt jede Position für sich und schluckt unterwegs diejenigen, die von Hand
korrigiert worden waren. Bei einer sauberen Datei ergeben beide dasselbe
Ergebnis; bei einer teilweise abweichenden Datei nicht.

## Eine Dauer schreiben

Zwei Formen, und nur zwei:

| Form | Beispiele |
| :---- | :------- |
| Sekunden | `30`, `2.999`, `-7.001`, `+1.5` |
| Zeitstempel | `00:00:07.001`, `01:39:37.040`, `-00:00:07.001` |

Beide sagen dasselbe: `--by 7.001` und `--by 00:00:07.001` erzeugen
dieselbe Datei.

**Das Dezimaltrennzeichen ist der Punkt**, denn die Befehlszeile spricht Englisch.
Ein Komma wird abgelehnt statt gedeutet: Die halbe Welt schreibt `7,001`
für siebentausendeins, und es als sieben Sekunden zu lesen, wäre ein Raten —
ein falsches für diese Hälfte.

**Die Auflösung ist die Millisekunde.** Feiner wird abgelehnt statt stillschweigend
gerundet: Ein stilles Runden ergäbe ein Ergebnis, das nichts von dem entspricht, was
verlangt wurde.

<!-- example: printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle shifted by -7.001 s -> b.srt
1
00:00:02,999 --> 00:00:04,999
Bonjour.
```

## Eine Verschiebung vor den Nullpunkt wird abgelehnt

Hinter null zurückzugehen, ergibt den Code `2` und **schreibt nichts**. Die Meldung nennt
den Untertitel, der die Verschiebung nicht aufnehmen konnte, und sagt damit zugleich,
wie weit man gehen kann.

Der Grund ist kein Prinzip: Eine Position vor dem Nullpunkt wird
`-00:00:01,000` geschrieben, eine Form, die keines der neun Formate definiert und die kein Leseprogramm
annimmt. Der Kern lässt solche Positionen durchaus zu — sie abzulehnen, würde aus einer
Bearbeitungsoperation einen Sonderfall machen —, aber eine Datei ist etwas anderes als ein
Modell im Speicher.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; echo "code=$?"
a.srt: subtitle 1 would start before the origin, which no subtitle file can hold
code=2
```

## Ausgabe

**Standardausgabe** — nichts: Das Ergebnis ist die geschriebene Datei. Mit `--dry-run` ebenfalls nichts: Es gibt keine Datei, und die Zeile der Verlaufsmeldungen sagt es — siehe [Vor dem Schreiben nachsehen](invocation.md#vor-dem-schreiben-nachsehen).

| Stufe | Was auf der Fehlerausgabe hinzukommt |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles shifted by 2.999 s -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — das Format, die Kodierung, die Marke und die Zeilenenden der gelesenen Datei, so wiederhergestellt, wie sie waren |
| 3 | `<path>: N bytes read, M written`, dann **jede Diagnose beim Lesen** — siehe [Aufruf](invocation.md#diagnosen-beim-lesen) |

### In JSON

Mit [`--format json`](invocation.md#ausgabe-die-ein-skript-lesen-kann) enthält das Objekt jeder Datei `counts.subtitles` (jeder Untertitel der Datei) und `counts.shifted_by_ms` (die Verschiebung, in ganzen Millisekunden, **mit Vorzeichen**). Die vollständige Tabelle steht unter [Aufruf](invocation.md#was-jeder-unterbefehl-in-counts-angibt).

## Exit-Codes

Die des Werkzeugs: `0`, wenn jede Datei geschrieben wurde, `2`, wenn keine, `3`, wenn
nur einige, `1` bei einem Aufruffehler.

## Fehler

| Was ihn auslöst | Meldung |
| :------------------ | :------ |
| Dezimalkomma | `"…" is not a time: use a decimal point and not a comma, in every language` |
| feiner als die Millisekunde | `"…" is not a time: positions are held to the millisecond, no finer` |
| Text, der keine Dauer ist | `"…" is not a time: expected seconds like 2.999, or a timestamp like 00:00:07.001` |
| Verschiebung vor den Nullpunkt | `<path>: subtitle N would start before the origin, which no subtitle file can hold` |

**Ohne eine der drei Zieloptionen wird nichts geschrieben**, und ihre
Fehler sind den Unterbefehlen gemeinsam, die schreiben:
siehe [Aufruf](invocation.md#das-ziel).
