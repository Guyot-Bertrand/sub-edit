# `convert`

<!-- machine-translated -->
> Diese Seite ist eine maschinelle Übersetzung und wurde noch von keiner Leserin und keinem Leser dieser Sprache geprüft.

```
subedit-cli convert --to <format>
                    [--line-endings unix|windows|mac] [--to-encoding NAME]
                    [--bom | --no-bom] [--sort] [--disjoint]
                    [--join-repeats] [--repair]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    <file>...
```

Schreibt jede Datei im verlangten Format und in der verlangten Form. **Ändert niemals
die Eingabe**, es sei denn, `--in-place` verlangt es ausdrücklich.

<!-- example: subedit-cli convert --help -->
```console
$ subedit-cli convert --help
Write a subtitle file out in another format or shape
Usage: subedit-cli convert [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to convert

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --to TEXT:{srt,vtt,subviewer2,ssa,ass,microdvd,mpl2,tmplayer,lrc} REQUIRED
                              Format to write
  --line-endings TEXT:{unix,windows,mac}
                              Line endings to write; the source's by default
  --to-encoding NAME          Encoding to write; the source's by default
  --frame-rate RATE           Frame rate of a file counted in frames: 25, 23.976
  --bom                       Write a byte order mark
  --no-bom                    Write no byte order mark
  --sort                      Put the subtitles in order of their start before writing
  --disjoint                  Cut overlapping subtitles so that no range overlaps another, before writing
  --join-repeats              Join the subtitles that repeat each other into one, before writing
  --repair                    Join the repeats, sort, then make the ranges disjoint, before writing
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
| `--to` | **ja** | ein Name aus der Tabelle unten, und nichts anderes | — |
| `--line-endings` | nein | `unix`, `windows` oder `mac` | die der gelesenen Datei |
| `--to-encoding` | nein | jede Kodierung, die ICU schreiben kann, außer denen, die ihre eigene Marke schreiben | die der gelesenen Datei |
| `--bom` / `--no-bom` | nein | Schalter, die einander ausschließen | was die gelesene Datei trug |
| `--sort` | nein | ein Schalter: ordnet die Untertitel nach ihrem Beginn, **vor dem Schreiben**, siehe [Ordnen beim Schreiben](#ordnen-beim-schreiben) | aus |
| `--join-repeats` | nein | ein Schalter: macht aus den aufeinanderfolgenden Stücken eines Textes einen einzigen Untertitel, **vor allem anderen**, siehe [Bereiche überschneidungsfrei machen](#bereiche-überschneidungsfrei-machen) | aus |
| `--repair` | nein | ein Schalter: dasselbe wie `--join-repeats --sort --disjoint`, in dieser Reihenfolge | aus |
| `--disjoint` | nein | ein Schalter: schneidet die überlappenden Untertitel **vor dem Schreiben**, siehe [Bereiche überschneidungsfrei machen](#bereiche-überschneidungsfrei-machen) | aus |
| `--output` / `--output-dir` / `--in-place` | **eine der drei**, außer mit `--dry-run` | siehe [Aufruf](invocation.md#das-ziel) | — |
| `--dry-run` | nein | ein Schalter: berechnet und sagt, schreibt nichts — siehe [Vor dem Schreiben nachsehen](invocation.md#vor-dem-schreiben-nachsehen) | aus |

`mac` bezeichnet den Wagenrücklauf allein (`\r`), das Zeilenende des klassischen Mac OS.

## Die Formate, die `--to` annimmt

| Name | Format | Geschriebene Endung |
| :-- | :----- | :--------------- |
| `srt` | SubRip | `.srt` |
| `vtt` | WebVTT | `.vtt` |
| `subviewer2` | SubViewer 2.0 | `.sub` |
| `ssa` | Sub Station Alpha | `.ssa` |
| `ass` | Advanced SSA | `.ass` |
| `mpl2` | MPL2 | `.txt` |
| `microdvd` | MicroDVD | `.sub` |
| `tmplayer` | TMPlayer | `.txt` |
| `lrc` | LRC | `.lrc` |

**Ein Name, und keine Endung.** Zwei Endungen bezeichnen je zwei
Formate — `.sub` ist auch die von MicroDVD, `.txt` die von MPL2 und von
TMPlayer —, sodass eine Option, die Endungen annähme, nicht wüsste, welches
verlangt wird. Die obigen Namen sind eindeutig und stimmen mit der
Endung überein, wo immer auch die Endung eindeutig ist.

**Die Liste ist geschlossen.** Es ist die von Gaupol, und alle neun stehen darin.

## Die zwei Formate, die kein Ende tragen

**TMPlayer und LRC schreiben nur eine Position pro Zeile**, ihren Beginn. Das
zeigt sich auf beiden Seiten.

**Beim Lesen** werden die Enden abgeleitet: Das einer Zeile ist der Beginn
der nächsten, und die letzte erhält fünf Sekunden. So macht es Gaupol.
Wir sagen es außerdem, durch eine Diagnose, die `--verbose` bei den drei
Unterbefehlen anzeigt:

```
file.lrc: carries no end times; each one was taken from the next start, settled by the reader
```

**Beim Schreiben** werden die Enden nicht geschrieben, mangels eines Platzes dafür.
Die Konvertierung in eines dieser beiden Formate verliert sie daher, und ein Hin- und Rückweg
findet sie nicht wieder — er leitet sie erneut ab.

**LRC verliert noch etwas**: Es kann keinen Zeilenumbruch tragen.
Eine Dialogzeile auf zwei Zeilen wird auf einer geschrieben, die zwei Zeilen durch
ein Leerzeichen verbunden. Auch dieser Verlust lässt sich nicht beheben.

Keines der beiden hat ein Vokabular für Kursivschrift.

## `--frame-rate` und das einzige Format, das in Bildern zählt

**MicroDVD zählt nicht in Sekunden**: Seine Positionen *sind* Bildnummern,
und keine MicroDVD-Datei gibt die Rate an, mit der sie gezählt
wurden. `--frame-rate` gibt sie an und dient auf beiden Seiten: Es ist die
Rate, mit der eine in Bildern gezählte `.sub` gelesen wird, und die, mit der eine
Konvertierung nach MicroDVD die Bilder zählt.

| Option | Erforderlich | Werte | Standard |
| :----- | :----- | :------ | :----- |
| `--frame-rate` | nein | eine Rate in Bildern pro Sekunde: `25`, `23.976`, `29.97` | siehe unten |

**Beim Lesen ohne `--frame-rate` wird die Datei mit 23,976 gelesen** — dem Wert
von Gaupol —, und die Stufe `-vvv` sagt es:
`counts in frames and states no rate; it was read at`. Es ist die einzige Stelle des Werkzeugs, an der jede angezeigte Position
auf einer Annahme beruht, die die Datei nicht bestätigen kann.

**Beim Schreiben ohne `--frame-rate` drei Fälle und drei Antworten:**

| Die Ausgangsdatei | Was geschieht |
| :------------------- | :-------------- |
| ist bereits MicroDVD | die Rate, mit der sie gelesen wurde, schreibt sie zurück, und die Bilder kommen identisch zurück |
| ist zeitbasiert, und ihre Positionen fallen auf ein Raster | das Raster wird übernommen, und `-vv` sagt, welches |
| ist zeitbasiert, ohne Raster | **Ablehnung**, Code 2: `writing frames needs a frame rate, and the positions fall on no grid to take one from — give --frame-rate` |

Die Ablehnung ist Absicht. Die einzige verbleibende Geste wäre, eine Zahl zu erfinden, und
jede Zeile der Datei würde sich um so viel verschieben, wie diese falsch wäre.

## Was eine Konvertierung verliert

**Die Konvertierung in ein anderes Format ist niemals verlustfrei**, und was nicht übersteht,
hängt vom Paar ab:

| Was nicht durchkommt | Warum |
| :--------------------- | :------- |
| der Kopf | er ergibt nur in seinem eigenen Format Sinn: `[INFORMATION]` ist kein WebVTT-Kopf |
| die formatspezifischen Daten | die Koordinaten von SubRip, die Kennung und die Einstellungen eines WebVTT-Cues haben anderswo keine Entsprechung |
| die Genauigkeit | SubViewer 2, die beiden Sub Station Alphas und LRC schreiben auf die Hundertstelsekunde, MPL2 auf die Zehntelsekunde, TMPlayer auf die Sekunde, MicroDVD auf das Bild; `01:00:00,017` kommt als `01:00:00,020` zurück |
| das Ende einer Zeile | TMPlayer und LRC haben kein Feld dafür; das nächste Lesen leitet es erneut ab |
| die Zeilenumbrüche | nur LRC: Seine Zeilen passen auf eine Zeile, und die beiden werden durch ein Leerzeichen wieder zusammengeklebt |
| die Tags, die das Zielformat nicht schreiben kann | WebVTT hat kein `<font color>`, TMPlayer und LRC haben gar nichts |
| das Layout | `{\pos(x,y)}`, `{\an8}`, `<v Marie>`, `<ruby>`: Sie überstehen einen Hin- und Rückweg in ihrem eigenen Format und verschwinden bei der Konvertierung |

**Eine Datei in ihrem eigenen Format zurückzuschreiben, verliert nichts** — das ist die Garantie,
die alles Übrige trägt, und `--to srt` auf eine `.srt` ergibt dieselben Bytes. Es wird
nicht einmal etwas dekodiert: Der Text bleibt die rohe Zeichenkette, die gelesen wurde.

## Tags werden übersetzt, nicht kopiert

**Jedes Format schreibt Kursivschrift auf seine eigene Weise**, und eine Konvertierung ohne Übersetzung
würde ein `{\i1}` in einer `.srt` hinterlassen: Text, den der Benutzer sieht, in einer
Datei, die nie wissen wird, wie sie ihn deuten soll. Was durchkommt, sind **sechs
Dinge** — Fettschrift, Kursivschrift, Unterstreichung, Farbe, Schriftart und Größe:

| Format | Fett | Kursiv | Unterstrichen | Farbe | Schriftart | Größe |
| :----- | :--: | :------: | :------: | :-----: | :----: | :----: |
| SubRip | `<b>` | `<i>` | `<u>` | `<font color="#RRGGBB">` | — | — |
| WebVTT | `<b>` | `<i>` | `<u>` | — | — | — |
| SubViewer 2 | `<b>` | `<i>` | `<u>` | `<font color="#RRGGBB">` | — | — |
| Sub Station Alpha | `{\b1}` | `{\i1}` | — | `{\c&HBBGGRR&}` | `{\fnNAME}` | `{\fsN}` |
| Advanced SSA | `{\b1}` | `{\i1}` | `{\u1}` | `{\c&HBBGGRR&}` | `{\fnNAME}` | `{\fsN}` |
| MicroDVD | `{Y:b}` | `{Y:i}` | `{Y:u}` | `{C:$BBGGRR}` | `{F:NAME}` | `{S:N}` |
| MPL2 | `\` | `/` | `_` | `{C:$BBGGRR}` | `{F:NAME}` | `{S:N}` |
| TMPlayer | — | — | — | — | — | — |
| LRC | — | — | — | — | — | — |

**Ein Strich ist ein angekündigter Verlust.** Eine farbige `.srt` nach WebVTT zu konvertieren, entfernt
die Farbe und sagt es; die erzeugte Datei ist WebVTT, nicht verkleidetes SubRip.

**MicroDVD und MPL2 können keinen Teil einer Zeile gestalten.** Ihre Tags
haben kein Ende: `{y:i}` setzt alles Folgende bis zum Ende der Zeile schräg, und
`{Y:i}` bis zum Ende des Untertitels. Eine Zeile, die nur zur Hälfte
fett war, wird daher ganz ohne Fettschrift geschrieben, statt mit überlaufender Fettschrift.

## Was die Konvertierung als verloren meldet

**Sie sagt es, wenn sie verliert, und schweigt, wenn sie nichts verliert.** Ein Bericht,
der bei jedem Aufruf erscheint, ist ein Bericht, den niemand liest.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,500\n<i>sur deux</i>\nlignes\n\n' > a.srt; subedit-cli convert --to lrc --output b.lrc a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,500\n<i>sur deux</i>\nlignes\n\n' > a.srt; subedit-cli convert --to lrc --output b.lrc a.srt
a.srt: 1 subtitle written as LRC -> b.lrc
a.srt: ends are not carried by LRC, line breaks were joined in 1 subtitle, 1 tag dropped
```

Die Zeile erscheint auf der Standard-Ausführlichkeitsstufe und auf jeder darüber.
Die Punkte, in dieser Reihenfolge:

| Punkt | Was er sagt |
| :---- | :----------- |
| `ends are not carried by <format>` | das Zielformat schreibt kein Ende |
| `line breaks were joined in N subtitles` | nur LRC: N Untertitel standen auf mehreren Zeilen |
| `N tags dropped` | N Tags konnten nicht geschrieben werden, Layout eingeschlossen |
| `the header was dropped` | der Kopf überquert keine Formatgrenze |
| `the <format> fields of N subtitles were dropped` | N Untertitel trugen formatspezifische Daten |
| `positions moved by up to N ms` | der größte Abstand, um den sich eine Position verschoben hat |

## Die Kodierung ändern

**`--to-encoding` wählt die geschriebene Kodierung; `--encoding` sagt diejenige, die
gelesen wird.** Beide zusammen kodieren eine Datei um:

```console
$ subedit-cli convert --to srt --encoding cp1252 --to-encoding utf-8 \
      --output-dir sortie film.srt
```

Ohne `--to-encoding` wird die Datei in **der Kodierung geschrieben, in der sie gelesen wurde**,
so wie mit ihren Zeilenenden und ihrer Marke: Eine ohne
Anweisung zur Kodierung konvertierte Datei ergibt dieselben Zeichen, in denselben Bytes.

**Ein Zeichen, das die Zielkodierung nicht schreiben kann, hält die Datei an**, und
es wird nichts geschrieben. Ein `ł` hat in Latin-1 keinen Platz; es durch ein `?` zu ersetzen,
würde zwischen Lesen und Schreiben Text verlieren, ohne dass es jemand sieht.

**Eine Byte-Reihenfolge-Marke gibt es nur für die Unicode-Kodierungen.**
`--bom` mit einem `--to-encoding`, das keine trägt, wird abgelehnt statt ignoriert
— siehe die Fehlertabelle unten.

**Und die Kodierung muss ihre Byte-Reihenfolge nennen.** `UTF-16` und `UTF-32` sind
Namen, die ICU kennt, deren Konverter **eine eigene Marke** schreibt, was auch immer
man von ihm verlangt: `--no-bom` würde dann angenommen und nicht befolgt. Beide werden
daher abgelehnt, und die Meldung nennt die Ausgabe:

```console
$ subedit-cli convert --to srt --to-encoding UTF-16 --output copie.srt film.srt
"UTF-16" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do
```

`UTF-16LE`, `UTF-16BE`, `UTF-32LE` und `UTF-32BE` schreiben genau dieselben
Bytes, Marke eingeschlossen — aber unter der Kontrolle von `--bom` und `--no-bom`. Die
Ablehnung kostet daher nichts als einen genaueren Namen, und sie gilt für das
Lesen wie für das Schreiben: `--encoding UTF-16` erhält dieselbe Antwort.

## Das Ziel

Die drei Optionen und ihre Regeln sind den Unterbefehlen gemeinsam, die
schreiben: siehe [Aufruf](invocation.md#das-ziel). Ohne eine von ihnen wird
nichts geschrieben.

Was `convert` eigen ist: **`--output-dir` ändert die Endung mit dem
Format.** `convert --to vtt` auf `a.srt` schreibt `a.vtt`, niemals eine `a.srt`, die
WebVTT enthält — eine Datei, deren Name lügt, bringt jedes andere Werkzeug aus dem Tritt.

## Das Konvertieren an Ort und Stelle wird abgelehnt

`--in-place` mit einem `--to`, das das Format ändert, ergibt Code `1` und schreibt
nichts: An Ort und Stelle gibt es keinen zweiten Namen, der das neue Format tragen könnte, und
die Datei bliebe unter einer Endung, die ihr Inhalt nicht mehr rechtfertigt.

`--in-place` lässt sich dennoch verwenden, um **die Form ohne das Format** zu ändern —
etwa eine `.srt` als `.srt` mit anderen Zeilenenden zurückzuschreiben.

Die Ablehnung wird **allein anhand der Endung** entschieden, vor jedem Lesen: Ein Aufruffehler
darf niemals einen Stapel halb geschrieben zurücklassen.

## Zeilenenden und Byte-Reihenfolge-Marke

Standardmäßig **übernimmt die geschriebene Datei, was die gelesene Datei trug**. Das Modell
bewahrt beides beim Lesen; sie standardmäßig vorzugeben, würde bei jeder
Konvertierung eine absichtlich bewahrte Information verlieren.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli convert --to vtt --output b.vtt a.srt; cat b.vtt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli convert --to vtt --output b.vtt a.srt; cat b.vtt
a.srt: 1 subtitle written as WebVTT -> b.vtt
WEBVTT

00:01.000 --> 00:03.000
Bonjour.
```

## Ordnen beim Schreiben

`--sort` schreibt die Datei **in der Reihenfolge der Anfänge**, unabhängig vom Zielformat. Das Ordnen geschieht **auf einer Kopie**: Die gelesene Datei wird
nicht angerührt, und was geschrieben wird, ist das, was [`sort`](sort.md) daraus gemacht hätte — stabil, zwei Untertitel, die zugleich beginnen, behalten ihre Reihenfolge.

**Niemals standardmäßig**: Die Reihenfolge einer Datei ist die ihres Autors, solange nichts anderes verlangt wird. Mit der Option sagt es jede Datei auf der
ersten Ausführlichkeitsstufe, `<path>: N subtitles moved` oder `<path>: already in order`, und der Datensatz von `--format json` enthält eine
Zählung `moved` — **ohne die Option nicht vorhanden**, um die Form dessen, was bestehende Skripte lesen, nicht zu ändern.

## Bereiche überschneidungsfrei machen

`--disjoint` schneidet die überlappenden Untertitel so, dass sich kein Bereich mehr mit einem anderen überschneidet, **auf derselben Kopie wie das Ordnen**: Die gelesene Datei wird nicht angerührt.

Niemals standardmäßig. Die Option tut, was [`disjoint`](disjoint.md) tut, **mit dessen Standardeinstellungen**, vor dem Schreiben, nach dem Ordnen. Sie braucht eine geordnete Datei: Andernfalls wird sie abgelehnt, und `--sort` — das zuerst kommt — bringt sie vorher in Ordnung. Die Datei sagt es auf der ersten Ausführlichkeitsstufe (`<path>: N subtitles split, M merged`), und der Datensatz von `--format json` enthält die Zählungen `split` und `merged`, **ohne die Option nicht vorhanden**.

**`--join-repeats` kommt vor dem Ordnen und dem Schneiden**: Ein in aufeinanderfolgende Stücke geschnittener Text wird zuerst wiederhergestellt (siehe
[`join-repeats`](join-repeats.md)), dann werden die Untertitel geordnet, dann die Bereiche überschneidungsfrei gemacht. **`--repair` ist alle drei auf einmal**,
wie [`repair`](repair.md), mit den Standardeinstellungen des Schneidens. Mit `--join-repeats` sagt jede Datei
`<path>: N repeated subtitles joined` auf der ersten Ausführlichkeitsstufe, und der Datensatz von `--format json` enthält `counts.joined` — **ohne die Option nicht vorhanden**.

## Ausgabe

**Standardausgabe** — nichts: Das Ergebnis ist die geschriebene Datei. Mit `--dry-run` ebenfalls nichts: Es gibt keine Datei, und die Zeile der Verlaufsmeldungen sagt es — siehe [Vor dem Schreiben nachsehen](invocation.md#vor-dem-schreiben-nachsehen).

Auf der Fehlerausgabe, je nach Stufe:

| Stufe | Was hinzukommt |
| :----- | :-------------- |
| 1 | `<path>: N subtitles written as WebVTT -> <destination>` |
| 2 | `<path>: SubRip -> WebVTT, UTF-8, no BOM, LF line endings` — die genannte Kodierung ist die geschriebene |
| 3 | `<path>: N bytes read, M written`, dann **jede Diagnose beim Lesen** — siehe [Aufruf](invocation.md#diagnosen-beim-lesen) |

### In JSON

Mit [`--format json`](invocation.md#ausgabe-die-ein-skript-lesen-kann) enthält das Objekt jeder Datei `counts.subtitles`, dann das, was die Konvertierung verloren hat: `lost_ends` und `lost_header` (`0` oder `1`), `joined_lines`, `lost_tags`, `lost_fields` und `furthest_ms` — die Punkte [des Verlustsatzes](#was-die-konvertierung-als-verloren-meldet). Die vollständige Tabelle steht unter [Aufruf](invocation.md#was-jeder-unterbefehl-in-counts-angibt).

## Exit-Codes

Die des Werkzeugs: `0`, wenn jede Datei geschrieben wurde, `2`, wenn keine, `3`, wenn
nur einige, `1` bei einem Aufruffehler.

## Fehler

| Was ihn auslöst | Meldung |
| :------------------ | :------ |
| `--bom` mit `--no-bom` | `--bom and --no-bom ask for opposite things; give one or the other` |
| `--in-place`, das das Format ändert | `--in-place cannot change the format: the file would keep a name its content no longer matches` |
| `--bom` bei einer Kodierung, die keine hat | `<path>: <encoding> has no byte order mark to write` |
| `--to-encoding`, das eine unbekannte Kodierung nennt | `no encoding is named "<name>"` |
| `--to-encoding`, das eine Kodierung nennt, die ihre eigene Marke schreibt | `"<name>" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do` |
| in der geschriebenen Kodierung fehlendes Zeichen | `<path>: holds a character the chosen encoding cannot write` |

**`--bom` bei einer Kodierung abzulehnen, die keine trägt, ist keine
Pedanterie.** Eine Byte-Reihenfolge-Marke gibt es für die Unicode-Kodierungen und
für keine andere: Eine für eine Datei in Windows-1252 zu verlangen, heißt, etwas zu verlangen,
das es nicht gibt. Die Datei würde ohne sie geschrieben, und niemand würde
wissen, dass die Anforderung nicht erfüllt wurde.

Die des Ziels sind den Unterbefehlen gemeinsam, die schreiben:
siehe [Aufruf](invocation.md#das-ziel).

Die Lesefehler sind die von [`inspect`](inspect.md): Sie betreffen
die Eingabedatei und haben dieselben Meldungen.
