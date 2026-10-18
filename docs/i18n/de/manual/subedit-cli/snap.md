# `snap`

<!-- machine-translated -->
> Diese Seite ist eine maschinelle Übersetzung und wurde noch von keiner Leserin und keinem Leser dieser Sprache geprüft.

```
subedit-cli snap --rate <rate>
                 (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 <file>...
```

Setzt **jeden Zeitstempel** — Beginn und Ende — auf das nächstgelegene Bild der
angegebenen Bildrate. Nichts wird umgerechnet: Jede Position bewegt sich um höchstens ein halbes
Bild.

## `snap` ist nicht `framerate`, und eine Verwechslung fällt nicht auf

Die beiden Befehle haben dieselbe Form — Dateien, ein Ziel, eine Bildrate — und tun Gegensätzliches.

| | [`framerate`](framerate.md) | `snap` |
| :--- | :-------------------------- | :----- |
| was er berechnet | `t' = t x input rate / output rate` | das nächstgelegene Bild, auf dem Raster von `--rate` |
| um wie viel er bewegt | im Verhältnis zur Zeit — **mehrere Sekunden** bei einem Spielfilm | höchstens ein halbes Bild, **überall** |
| für welche Datei | eine Datei, deren **Timing falsch ist** | eine Datei, deren Timing **stimmt** und deren Raster falsch ist |

**Der Anwendungsfall von `snap`**, ausgeschrieben: ein Film mit 25 Bildern pro
Sekunde, eine auf einem Raster von 24 geschriebene Untertiteldatei, Zeilen, die schon
mehr oder weniger an ihrem Platz sind, auf einige Millisekunden genau. Es gibt nichts umzurechnen
— nur die Zeitstempel wieder auf die richtigen Bilder zu setzen.

**Der Anwendungsfall von `framerate`** ist der andere: eine Datei, die für ein Master mit
25 getimt ist, die man auf einer Fassung mit 23,976 abspielen möchte und deren Timing bis zum Ende des Films
um mehrere Sekunden abdriftet.

Den einen für den anderen zu nehmen, **verursacht keinen Fehler**: Beide
nehmen die Datei an und schreiben ein Ergebnis. Die Zeile der Verlaufsmeldungen ist es, die
sie unterscheidet — `snap` sagt immer, um wie viel es die Positionen bewegt hat.

<!-- example: subedit-cli snap --help -->
```console
$ subedit-cli snap --help
Move every position onto the nearest frame of a frame rate (see framerate)
Usage: subedit-cli snap [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to align

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --rate TEXT REQUIRED        Frame rate to align on: 25, 23.976
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
| `--rate` | **ja** | eine Bildrate in Bildern pro Sekunde, geschrieben wie bei [`framerate`](framerate.md#eine-bildrate-schreiben) | — |
| `--output` / `--output-dir` / `--in-place` | **eine der drei**, außer mit `--dry-run` | siehe [Aufruf](invocation.md#das-ziel) | — |
| `--dry-run` | nein | ein Schalter: berechnet und sagt, schreibt nichts — siehe [Vor dem Schreiben nachsehen](invocation.md#vor-dem-schreiben-nachsehen) | aus |

`--rate` nimmt **jede gültige Bildrate** an, nicht nur die acht
Standardraten: Es ist die Wahl des Benutzers, wie bei `framerate`. Eine Bildrate von null
oder darunter wird abgelehnt.

Das Format der gelesenen Datei wird **beibehalten**: Das Format zu ändern, ist die Aufgabe von
[`convert`](convert.md).

## Ausgabe

**Standardausgabe** — nichts: Das Ergebnis ist die geschriebene Datei. Mit `--dry-run` ebenfalls nichts: Es gibt keine Datei, und die Zeile der Verlaufsmeldungen sagt es — siehe [Vor dem Schreiben nachsehen](invocation.md#vor-dem-schreiben-nachsehen).

| Stufe | Was auf der Fehlerausgabe hinzukommt |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles aligned on 25 fps, M positions moved, by at most 20 ms -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — das Format, die Kodierung, die Marke und die Zeilenenden der gelesenen Datei, so wiederhergestellt, wie sie waren |
| 3 | `<path>: N bytes read, M written`, dann **jede Diagnose beim Lesen** — siehe [Aufruf](invocation.md#diagnosen-beim-lesen) |

**Die beiden Zahlen sind der Sinn der Zeile.** Ein halbes Bild bei 25 Bildern pro
Sekunde sind zwanzig Millisekunden, und nichts Größeres kann dort stehen. Wer
`framerate` wollte und `snap` eingetippt hat, liest „by at most 20 ms“, wo er
Sekunden erwartete, und weiß sofort Bescheid.

## Ein Beispiel

Eine Zeile bei `00:00:01,010`, auf 25 Bilder pro Sekunde eingerastet — also auf ein
Vielfaches von vierzig Millisekunden:

<!-- example: printf '1\n00:00:01,010 --> 00:00:03,020\nUn.\n\n' > a.srt; subedit-cli snap --rate 25 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,010 --> 00:00:03,020\nUn.\n\n' > a.srt; subedit-cli snap --rate 25 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle aligned on 25 fps, 2 positions moved, by at most 20 ms -> b.srt
1
00:00:01,000 --> 00:00:03,040
Un.
```

## Zwei Eigenschaften

**Zweimaliges Einrasten ändert beim zweiten Mal nichts.** Eine Position, die schon auf einem
Bild liegt, bleibt dort: Es ist dieselbe Operation, auf dasselbe Ergebnis angewendet.

**Die Reihenfolge der Untertitel bleibt erhalten.** Das Runden auf das nächstgelegene Bild ist
monoton, also bleiben zwei Anfänge, die mindestens ein Bild auseinanderliegen, auseinander. Liegen sie näher
beieinander, können sie verschmelzen — das ist eine Auffälligkeit, die
[`inspect`](inspect.md#was-anomalies-meldet) zu benennen weiß, und keine
Vertauschung.

### In JSON

Mit [`--format json`](invocation.md#ausgabe-die-ein-skript-lesen-kann) enthält das Objekt jeder Datei `counts.subtitles`, `counts.moved` (die bewegten Positionen) und `counts.furthest_ms` (die größte Bewegung, in Millisekunden). Die vollständige Tabelle steht unter [Aufruf](invocation.md#was-jeder-unterbefehl-in-counts-angibt).

## Exit-Codes

Die des Werkzeugs: `0`, wenn jede Datei geschrieben wurde, `2`, wenn keine, `3`, wenn
nur einige, `1` bei einem Aufruffehler.

## Fehler

Die einer Bildrate, Wort für Wort die von [`framerate`](framerate.md#fehler)
— beide lesen denselben Wert auf demselben Weg:

| Was ihn auslöst | Meldung |
| :------------------ | :------ |
| Bildrate null oder negativ | `"…" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed` |
| Dezimalkomma | `"…" is not a frame rate: use a decimal point and not a comma, in every language` |
| Text, der keine Zahl ist | `"…" is not a frame rate: expected frames per second, like 25 or 23.976` |
| Bildrate jenseits jedes Maßes | `"…" is not a frame rate: no video runs at that many frames per second` |

**Ohne eine der drei Zieloptionen wird nichts geschrieben**, und ihre Fehler
sind den Unterbefehlen gemeinsam, die schreiben:
siehe [Aufruf](invocation.md#das-ziel).
