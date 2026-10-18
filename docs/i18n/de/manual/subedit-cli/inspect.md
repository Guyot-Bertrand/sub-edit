# `inspect`

<!-- machine-translated -->
> Diese Seite ist eine maschinelle Übersetzung und wurde noch von keiner Leserin und keinem Leser dieser Sprache geprüft.

```
subedit-cli inspect [--frame-rate RATE] [--recursive] [--detail]
                   [-t FILE] [--align-method position|number] <file>...
```

Berichtet, woraus jede Datei besteht. **Ändert nichts und schreibt keine
Datei.**

<!-- example: subedit-cli inspect --help -->
```console
$ subedit-cli inspect --help
Report what a subtitle file is made of
Usage: subedit-cli inspect [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to report on

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --detail                    List every anomaly, subtitle by subtitle, instead of counting them
  --frame-rate RATE           Frame rate of a file counted in frames: 25, 23.976
  -t,--translation-file FILE  Translation file to lay over the subtitle file, for a single input
  --align-method position|number
                              How the lines of the translation find their subtitles
```

## Argumente

| Argument | Erforderlich | Wert | Standard |
| :------- | :----- | :----- | :----- |
| `<file>...` | ja | ein oder mehrere Pfade von Untertiteldateien | — |
| `--recursive`, `-r` | nein | ein Schalter | aus — siehe [Einen Verzeichnisbaum verarbeiten](lots.md) |
| `--detail` | nein | ein Schalter: nennt jede Auffälligkeit, statt sie zu zählen — siehe [Was `anomalies` meldet](#was-anomalies-meldet) | aus |

Kein Pfad ist ein Aufruffehler, daher Code `1`.

## Optionen

| Option | Erforderlich | Wert | Standard |
| :----- | :------ | :----- | :----- |
| `--frame-rate` | nein | eine Rate in Bildern pro Sekunde: `25`, `23.976`, `29.97` | 23,976 |

**Sie dient nur Dateien, die in Bildern gezählt werden**, also MicroDVD, dem
einzigen der neun Formate. Eine MicroDVD-Datei enthält nichts als Bildnummern; die
Rate, mit der sie gelesen werden, entscheidet über **jede Position des Berichts**,
angefangen bei `span`. Anderswo angegeben, ändert sie nichts und ist kein
Fehler.

Ein Wert, der nichts benennt, ist ein Aufruffehler, daher Code `1`:

<!-- example: subedit-cli inspect --frame-rate douze film.sub; echo $? -->
```console
$ subedit-cli inspect --frame-rate douze film.sub; echo $?
--frame-rate: "douze" is not a frame rate: expected frames per second, like 25 or 23.976
1
```

Es ist dieselbe Option wie bei [`convert`](convert.md), Wort für Wort, weil
es dieselbe Rate ist. `convert` verwendet sie auch zum **Schreiben**.

## Eine Übersetzung: `-t`

| Option | Erforderlich | Wert | Standard |
| :----- | :------ | :----- | :----- |
| `-t`, `--translation-file` | nein | eine Übersetzungsdatei, für **eine einzige** Eingabe | — |
| `--align-method` | nein, und nur mit `-t` | `position` oder `number` | `position` |

Mit `-t` wird die Übersetzung auf die Datei aufgelegt, **so wie es das Fenster beim Öffnen tut** — siehe
[Eine Übersetzung](invocation.md#eine-übersetzung) —, und der Bericht sagt, **wie ihre Zeilen ihren
Untertitel gefunden haben**. Ein Skript weiß so, ob eine Übersetzung ausgerichtet ist, bevor es sie korrigiert.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour.\n\n' > film.fr.srt; subedit-cli --quiet inspect -t film.fr.srt film.srt | tail -3 -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour.\n\n' > film.fr.srt; subedit-cli --quiet inspect -t film.fr.srt film.srt | tail -3
  anomalies: none
  translation file: film.fr.srt, matched by position
  translation: 1 line attached; 1 subtitle left without a translation
```

Die erste hinzugefügte Zeile nennt die Datei und die Methode; die zweite ist **der Satz, den das Fenster
sagt**, wenn es eine Übersetzung öffnet, jeder Teil nur, wenn er nicht null ist:

| Teil | Was er zählt |
| :----- | :---------------- |
| `N lines attached` | die Zeilen der Übersetzung, die einen Untertitel der Hauptdatei gefunden haben |
| `N subtitles born of a line` | die Zeilen, die keinen gefunden und einen eigenen hervorgebracht haben, an ihren eigenen Positionen |
| `N subtitles left without a translation` | die Untertitel der Hauptdatei, die keine Zeile erreicht hat |
| `N lines out of order` | die Zeilen, die in der Datei nach einer späteren kamen |

Eine Übersetzung, die keine Zeile enthält, sagt `translation: the file holds no line`. **Der Bericht über die
Datei selbst bleibt der über die Datei allein**: Ein aus einer Zeile hervorgegangener Untertitel wird nicht in
`subtitles` gezählt.

Die Fehler von `-t` sind die eines Lesevorgangs, mit dem Pfad der Übersetzung gemeldet, und dieser hier:
`<translation>: the file is already open as the main document`, wenn die angegebene Datei die Hauptdatei ist.
`--align-method` ohne `-t` ist ein Aufruffehler, ebenso `-t` mit mehreren Eingaben — siehe
[Eine Übersetzung](invocation.md#eine-übersetzung).

## Ausgabe

Der Bericht geht auf die **Standardausgabe** — er ist das Ergebnis des Befehls, und
er wird auf jeder Stufe geschrieben, `--quiet` eingeschlossen. Ein Block pro Datei, eingeleitet
vom Pfad:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,500\nLe canot dérive.\n\n2\n00:00:04,000 --> 00:00:06,200\nPersonne n a vu la côte.\n' > exemple.srt; subedit-cli --quiet inspect exemple.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,500\nLe canot dérive.\n\n2\n00:00:04,000 --> 00:00:06,200\nPersonne n a vu la côte.\n' > exemple.srt; subedit-cli --quiet inspect exemple.srt
exemple.srt
  format: SubRip
  encoding: UTF-8, detected
  byte order mark: absent
  line endings: LF
  subtitles: 2
  span: 00:00:01.000 -> 00:00:06.200
  frame rate grid: none (too few subtitles to tell)
  anomalies: none
```

| Feld | Was es sagt |
| :---- | :----------- |
| `format` | eines der Formate, die `--to` annimmt, ausgeschrieben: `SubRip`, `WebVTT`, `SubViewer 2`, `Sub Station Alpha`, `Advanced SSA`, `MicroDVD`, `MPL2`, `TMPlayer`, `LRC` |
| `encoding` | die gelesene Kodierung, gefolgt davon, **woher die Antwort kommt**: `, from its byte order mark`, wenn die Datei sie angibt, `, as asked for`, wenn `--encoding` sie vorgegeben hat, `, detected`, wenn sie abgewogen wurde |
| `byte order mark` | `present` oder `absent` |
| `line endings` | `LF`, `CRLF` oder `CR`, gefolgt von `, mixed from line N`, wenn die Datei sie mischt |
| `subtitles` | die Anzahl der gelesenen Untertitel |
| `span` | vom frühesten Beginn bis zum spätesten Ende, `HH:MM:SS.mmm` |
| `frame rate grid` | die Bildrate, auf der die Positionen berechnet wurden, **abgeleitet** — siehe unten |
| `frame rate` | für eine in Bildern gezählte Datei die Rate, mit der sie gelesen wurde, und **woher sie kommt** — ersetzt die vorige Zeile, siehe unten |
| `anomalies` | `none` auf derselben Zeile, oder was nicht stimmt: **eine Art pro Zeile mit ihrer Anzahl**, eingerückt unter `anomalies:`; mit `--detail` **eine Auffälligkeit pro Zeile**, Untertitel für Untertitel |

**`span` ist nicht „vom ersten bis zum letzten“**, sondern vom frühesten bis zum spätesten:
Bei einer Datei, deren Reihenfolge gestört ist, unterscheiden sich die beiden, und nur das zweite sagt
die Wahrheit darüber, was die Datei abdeckt.

**`anomalies` zählt mit `--detail` die Untertitel ab 1**, so wie die Datei
sie nummeriert — und nicht die Zeilen der Datei. Eine Untertitelnummer übersteht eine
Änderung; eine Zeilennummer nicht. Siehe unten.

## Was `frame rate grid` meldet

**Eine Untertiteldatei gibt ihre Bildrate nicht an.** SubRip hat keinen
Kopf, der von WebVTT ist freier Text. Diese Zeile liest daher nichts:
Sie **leitet ab**, indem sie misst, ob die Positionen auf ein Raster von Bildern fallen.

Gemessen werden die **Anfänge**. Wo immer ein Raster existiert, liegen sie
ausnahmslos darauf, während die Enden oft davon abweichen — ein Ende wird
häufig durch eine Regel der Lesegeschwindigkeit berechnet, statt auf
ein Bild gesetzt zu werden.

Die erste Zeile sagt immer dasselbe: die gewählte Rate, das Urteil
und die Konzentration, die es stützt.

| Urteil | Was es bedeutet |
| :------ | :----------------- |
| `clean` | die Positionen liegen auf einem Raster |
| `partial` | ein Teil von ihnen liegt darauf, der Rest nicht |
| `none` | kein Kandidat erklärt irgendetwas |

`none` hat **zwei Ursachen, und sie sind nicht dieselbe Antwort**: Entweder passt kein
Raster — die Zeile nennt dann den besten Kandidaten und seine Bewertung,
damit man sieht, um wie viel er scheitert —, oder die Datei zählt zu wenige
Untertitel, als dass sich etwas über sie sagen ließe. Zwei Anfänge sehen immer
vollkommen ausgerichtet aus; das beweist nichts.

**Eine regelmäßige Datei auf einer nicht standardmäßigen Rate ergibt `none`, und das ist
gewollt.** Die Menge der Kandidaten ist geschlossen — die acht Raten, die die
Umrechnung anbietet —, und die am wenigsten falsche von ihnen zu melden, hieße, eine
falsche Antwort zu geben, wo „ich weiß es nicht“ die richtige ist.

### Die folgenden Zeilen, wenn sie etwas zu sagen haben

| Zeile | Wann sie erscheint |
| :---- | :------------------ |
| `grid offset` | die Positionen liegen auf dem Raster **bis auf eine Konstante**: Die Datei wurde verschoben, und hier steht, um wie viel |
| `also fits` | ein anderer Kandidat passt genauso gut, weil er ein ganzzahliges Vielfaches des gewählten ist |
| `too short a span to separate` | die Spanne der Datei erlaubt nicht, die gewählte Rate von den genannten zu unterscheiden |
| `off the grid` | wie viele Anfänge vom Raster abweichen, und in wie vielen **Folgen** |

**`also fits` ist kein Zögern.** Ein Raster mit 25 Bildern pro Sekunde ist in
einem Raster mit 50 *enthalten*: Eine auf 25 getimte Datei passt daher auf beide.
Das Umgekehrte ist falsch — echte 50 fallen auf 25 nicht zusammen —, also wird die niedrigste
gewählt, und die andere wird genannt, statt verschwiegen zu werden. Die einzigen
betroffenen Paare sind 25 und 50, 29,97 und 59,94, 30 und 60.

**`too short a span to separate` schützt vor einer übermütigen Antwort.** 23,976
und 24 driften um eine Millisekunde pro Sekunde Film auseinander: Bei einem Spielfilm
trennen sie sich zweifelsfrei, über zehn Sekunden lassen sie sich nicht unterscheiden. Die Zeile sagt,
welche möglich bleibt.

**`off the grid` unterscheidet zwei Geschichten, die der Prozentsatz allein verwechselt.**
Viele Folgen aus einem einzigen Anfang sind von Hand korrigierte Positionen,
eine nach der anderen. Wenige lange Folgen sind ein neu getimter Abschnitt oder eine Datei,
die aus zwei anderen zusammengesetzt wurde. Die Anzahl der Folgen ist es, die sie trennt.

### Ein Beispiel

Zwölf Zeilen, auf ein Raster mit 25 Bildern pro Sekunde gesetzt — also auf
Positionen, die Vielfache von 40 Millisekunden sind —, verteilt über fast neun Minuten:

<!-- example: k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > grille.srt; subedit-cli --quiet inspect grille.srt | tail -3 -->
```console
$ k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > grille.srt; subedit-cli --quiet inspect grille.srt | tail -3
  frame rate grid: 25 fps, clean (100.0%)
  also fits: 50 fps, of which this rate is a whole divisor
  anomalies: none
```

Das Beispiel erzeugt seine eigene Datei, statt eine aus dem Repository zu lesen, und das ist
Absicht: Es lässt sich so, wie es ist, erneut ausführen, und die Schleife zeigt, was ein Raster ist,
besser als ein Satz — Positionen, die Vielfache einer Bilddauer sind.

## Was `frame rate` meldet, und warum es kein Raster ist

**Nur eines der neun Formate zählt in Bildern**: MicroDVD, dessen jede Zeile
zwei Bildnummern enthält und sonst nichts. Bei einer solchen Datei schreibt `inspect`
nicht `frame rate grid`, sondern `frame rate`, und die beiden Zeilen werden
nie zusammen angezeigt.

**Der Grund passt in einen Satz.** Die Positionen einer MicroDVD-Datei wurden
*aus* ihren Bildern berechnet, mit einer gegebenen Rate; sie aus einem
Raster abzuleiten, würde genau diese Rate zurückgeben, so wie sie gerade geliefert wurde. Es wäre
eine vorgegebene Zahl, als Messung verkleidet.

Stattdessen wird **die Leserate und ihre Herkunft** geschrieben —
in derselben Form wie die Zeile `encoding`:

| Was die Zeile sagt | Wann |
| :------------------ | :---- |
| `frame rate: 25 fps, as asked for` | `--frame-rate` hat sie vorgegeben |
| `frame rate: 24000/1001 fps, assumed` | niemand hat sie angegeben; es ist der Standardwert |

**Zwei Antworten und nicht drei**, wo `encoding` drei hat: Eine MicroDVD-Datei
gibt nichts an. Das ist das ganze Problem — die Rate kommt zwangsläufig
von außen, und der Bericht sagt, von welchem der beiden Außen.

**Die Rate entscheidet über den ganzen Rest des Berichts.** `span` geht durch sie hindurch, und zwei
Lesevorgänge derselben Datei mit zwei Raten ergeben zwei Spannen:

<!-- example: printf '{25}{75}Première.\n{100}{150}Deuxième.\n{200}{260}Troisième.\n' > images.sub; subedit-cli --quiet inspect images.sub | tail -3; subedit-cli --quiet inspect --frame-rate 25 images.sub | tail -3 -->
```console
$ printf '{25}{75}Première.\n{100}{150}Deuxième.\n{200}{260}Troisième.\n' > images.sub; subedit-cli --quiet inspect images.sub | tail -3; subedit-cli --quiet inspect --frame-rate 25 images.sub | tail -3
  span: 00:00:01.043 -> 00:00:10.844
  frame rate: 24000/1001 fps, assumed
  anomalies: none
  span: 00:00:01.000 -> 00:00:10.400
  frame rate: 25 fps, as asked for
  anomalies: none
```

## Was `anomalies` meldet

In einem Dokument können drei Dinge nicht stimmen, und jedes wird anders behoben.
**Standardmäßig zählt `inspect` sie**, eine Zeile pro Art, in der Reihenfolge der Tabelle:
Eine schlecht gemachte Datei enthält Hunderte davon, und zu wissen, wie viele von jeder Art,
ist es, was sagt, was auszuführen ist. **`--detail` nennt sie einzeln.**

| Was geschrieben wird | Mit `--detail` | Was es bedeutet |
| :--------------- | :-------------- | :------------------ |
| `subtitle ends before it starts: N` | `subtitle N ends before it starts` | das Ende kommt vor dem Beginn |
| `subtitle starts before the previous one ends: N` | `subtitle N starts before the previous one ends` | er überlappt den vorigen |
| `subtitle starts before the previous one starts: N` | `subtitle N starts before the previous one starts` | er stört die Reihenfolge der Datei — [`sort`](sort.md) stellt sie wieder her |

Eine Art, die nicht vorkommt, wird nicht geschrieben. In der linken Spalte ist `N` die Anzahl
der Untertitel; in der nächsten die Nummer eines Untertitels.

**Derselbe Untertitel kann zweimal erscheinen**, und das ist gewollt: Derjenige, der
beginnt, bevor der vorige begonnen hat, beginnt auch, bevor dieser
geendet hat. Der erste Punkt wird behoben, indem man den Untertitel verschiebt, der zweite, indem man
eine Dauer ändert.

**Der genannte Untertitel ist derjenige, der falsch liegt**, nicht derjenige, gegenüber dem er
falsch liegt. Bei Anfängen bei `0`, `4 s`, `2 s`, `3 s`: Der dritte stört die Reihenfolge, der
vierte folgt sehr wohl auf den dritten und wird daher nicht genannt — an ihm ist nichts
zu tun.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect desordre.srt | tail -3 -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect desordre.srt | tail -3
  anomalies:
    subtitle starts before the previous one ends: 1
    subtitle starts before the previous one starts: 1
```

Und mit `--detail` der genannte Untertitel:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect --detail desordre.srt | tail -3 -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect --detail desordre.srt | tail -3
  anomalies:
    subtitle 3 starts before the previous one ends
    subtitle 3 starts before the previous one starts
```

**Eine Zeile pro Auffälligkeit mit `--detail`**, weil hundert Befunde,
auf eine Zeile geklebt, sich weder lesen noch durchsuchen lassen. Ein Dokument ohne Mangel behält
seine einzige Zeile, `anomalies: none`, nach der ein Skript sucht. **Der Datensatz von
`--format json` listet immer jede Auffälligkeit auf**: Ein Skript liest die Einzelheiten, und
`--detail` ändert nur den Text.

**Es ist keine Diagnose beim Lesen.** Die Diagnosen sagen, worauf das
Lesen gestoßen ist, und zeigen auf eine **Zeile der Datei**; die Auffälligkeiten sagen,
was das Dokument *ist*, und zeigen auf einen **Untertitel**. Eine Datei, deren Reihenfolge
gestört ist, ist deshalb noch nicht fehlerhaft aufgebaut: Das Lesen hat darüber nichts zu sagen.

## In JSON

Mit [`--format json`](invocation.md#ausgabe-die-ein-skript-lesen-kann) schreibt `inspect` **ein
Objekt pro Datei** statt der obigen Blöcke; die Verlaufsmeldungen bleiben auf der Fehlerausgabe.
Die Schlüssel beschreiben, was der Text sagt, und **keiner muss aus einem Satz zurückgelesen werden**:

| Schlüssel | Inhalt |
| :-- | :------ |
| `format` | die Kennung des Formats, diejenige, die `convert --to` annimmt: `srt`, `vtt`… |
| `encoding` | `{"charset", "origin", "byte_order_mark"}` — `origin` ist `byte-order-mark`, `asked` oder `detected` |
| `line_endings` | `{"kind", "mixed_from_line"}` — `lf`, `crlf` oder `cr`; die Zeile, in der eine zweite Art auftaucht, oder `null` |
| `subtitles` | die Anzahl der Untertitel |
| `span_ms` | `{"start", "end"}` — der erste Beginn und das letzte Ende, in Millisekunden |
| `frame_rate` | für eine in Bildern gezählte Datei: `{"rate", "origin"}` (`asked` oder `assumed`); sonst `null` |
| `grid` | für die anderen: das abgeleitete Raster, oder `null` für eine in Bildern gezählte Datei |
| `anomalies` | ein Array von `{"subtitle", "kind"}` — `end-before-start`, `overlapping-subtitles`, `out-of-order` |
| `translation` | **nur mit `-t`**: `{"file", "method", "attached", "born", "untranslated", "out_of_order"}` — `method` ist `position` oder `number`, und die vier Zählungen sind die des obigen Satzes |
| `warnings` | worauf das Lesen gestoßen ist, auf jeder Stufe |

`grid` enthält `verdict` (`clean`, `partial` oder `silent`), `enough_starts`, `rate` (eine Zeichenkette,
oder `null`, wenn nichts genannt wird), `concentration_permille` (Tausendstel, `null` ohne genügend
Anfänge), `offset_ms`, `also_fits` (die Rate, deren ganzzahliger Teiler diese ist, oder
`null`), `not_separated` (die Raten, die eine zu kurze Spanne nicht trennt), `strays`
(die Anfänge außerhalb des Rasters) und `starts`.

<!-- example: printf '{25}{50}Un.\n{75}{100}Deux.\n' > f.sub; subedit-cli --format json inspect --frame-rate 25 f.sub 2>/dev/null -->
```console
$ printf '{25}{50}Un.\n{75}{100}Deux.\n' > f.sub; subedit-cli --format json inspect --frame-rate 25 f.sub 2>/dev/null
{"schema":1,"command":"inspect","file":"f.sub","ok":true,"format":"microdvd","encoding":{"charset":"UTF-8","origin":"detected","byte_order_mark":false},"line_endings":{"kind":"lf","mixed_from_line":null},"subtitles":2,"span_ms":{"start":1000,"end":4000},"frame_rate":{"rate":"25","origin":"asked"},"grid":null,"anomalies":[],"warnings":[]}
```

## Verlaufsmeldungen

Auf der Fehlerausgabe, je nach verlangter Stufe:

| Stufe | Was hinzukommt |
| :----- | :-------------- |
| 1 | `<path>: N subtitles` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings` |
| 3 | `<path>: N bytes read`, `<path>: N diagnostic(s) while reading`, dann **jede von ihnen** — siehe [Aufruf](invocation.md#diagnosen-beim-lesen) |

## Exit-Codes

Die des Werkzeugs: `0`, wenn jede Datei gelesen wurde, `2`, wenn keine, `3`, wenn
nur einige, `1` bei einem Aufruffehler.

## Fehler

Die eines Lesevorgangs — der Datei, und der Übersetzung, wenn `-t` angegeben ist —, und
die von `-t`: siehe oben. `inspect` schreibt nichts, was abgelehnt werden könnte.

| Was ihn auslöst | Meldung |
| :------------------ | :------ |
| kein Pfad angegeben | `files is required`, gefolgt von einem Verweis auf `--help` |
| fehlende Datei | `<path>: does not exist` |
| unlesbare Datei | `<path>: cannot be opened: permission denied` |
| Bytes, die sich nicht dekodieren lassen | `<path>: cannot be decoded in the chosen encoding` |
| Format nicht erkannt | `<path>: is in no format this tool knows` |
| nichts, das wie ein Untertitel aussieht | `<path>: holds nothing recognisable as a subtitle` |

Es sind dieselben Meldungen für die siebzehn Unterbefehle: Das Rezept zum Öffnen
ist einmal geschrieben, im Kern.

## Ein Beispiel, das scheitert

Eine leere Datei passt zu keinem Format. Sie wird nicht als leer gemeldet:
Sie wird abgelehnt und genannt.

<!-- example: : > vide.srt; subedit-cli inspect vide.srt; echo "code=$?" -->
```console
$ : > vide.srt; subedit-cli inspect vide.srt; echo "code=$?"
vide.srt: is in no format this tool knows
code=2
```
