# `transcribe`

<!-- machine-translated -->
> Diese Seite ist eine maschinelle Übersetzung und wurde noch von keinem Leser der Sprache geprüft.

```
subedit-cli transcribe --output FILE
                       [--to FORMAT]
                       [--audio-track N]
                       [--language CODE]
                       [--start TIME] [--end TIME]
                       [--model FILE]
                       [--engine NAME]
                       [--keep-audio FILE]
                       [--raw]
                       <input>
```

Schreibt **die Sprache eines Videos oder einer Audiodatei als Untertiteldatei**. Das Audio wird vorbereitet,
einer Engine übergeben, die herausfindet, was gesagt wird und wann, und die Stücke, die sie schneidet, werden zu
Untertiteln zusammengefügt, die eine Dauer und eine Länge einhalten.

**Nichts, was der Befehl braucht, wird mit dem Programm ausgeliefert**: Die Engine (whisper.cpp), ein Modell und `ffmpeg`
installiert der Benutzer, der sagt, wo das Modell liegt. [Die Transkriptionswerkzeuge
installieren](install-transcription.md) sagt, wo man jedes findet und welches Modell man wählt. **Ihr Fehlen
zerstört nichts**: Der Befehl sagt, was fehlt, mit dem Verweis auf diese Seite, schreibt nichts und
endet mit einem Code ungleich null.

- **Die Eingabe ist ein Video oder eine Audiodatei.** Aus einem Video extrahiert `ffmpeg` eine Tonspur; ohne
  `ffmpeg` wird eine Audiodatei, die die Engine unverändert liest (`wav`, `mp3`, `flac`, `ogg`), trotzdem
  transkribiert, ein Video aber nicht, und auch keine Datei, wenn eine Spur verlangt wird.
- **Nur eine Datei auf einmal**, und das Ziel ist `--output`: Es gibt weder `--output-dir` noch
  `--in-place`, da das Ergebnis eine neue Datei ist und kein Umschreiben.
- **Die Sprache wird erkannt**, es sei denn, `--language` erzwingt sie, und die verwendete Sprache wird genannt. Ein
  Modell, das nur für Englisch gemacht ist, erkennt nichts und transkribiert nichts anderes: Eine Sprache, die es nicht
  verarbeiten kann, wird *vor* jeder Arbeit abgelehnt, unter Nennung der Grenze des Modells.
- **Ein Teil des Films kann allein transkribiert werden**, mit `--start` und `--end` — siehe
  [Einen Teil transkribieren](#einen-teil-transkribieren). So probiert man ein Modell an zwei Minuten aus, bevor man es auf
  zwei Stunden anwendet.
- **Die Segmente werden zusammengefügt und geschnitten** nach den Grenzen der Einstellungen (`transcription.merge-gap-ms`,
  `transcription.max-duration-ms`, `transcription.max-characters`): Zwei nahe beieinander liegende Stücke eines
  Satzes ergeben einen Untertitel, und ein zu langes oder zu breites Stück wird an einem Wort geschnitten. **`--raw`
  schreibt die Segmente der Engine, wie sie kamen**, eines pro Untertitel, und so sieht man auch, was das
  Zusammenfügen bewirkt.

<!-- example: subedit-cli transcribe --help -->
```console
$ subedit-cli transcribe --help
Write the speech of a video or an audio file as subtitles
Usage: subedit-cli transcribe [OPTIONS] input

Positionals:
  input TEXT REQUIRED         Video or audio file to transcribe

Options:
  -h,--help                   Print this help message and exit
  --output FILE               Subtitle file to write
  --to TEXT:{srt,vtt,subviewer2,ssa,ass,microdvd,mpl2,tmplayer,lrc}
                              Format to write; the one the extension of --output says by default
  --audio-track N             Audio track of a video, counted from 0; the first by default
  --start TIME                Where to begin, as 2:00, 1:02:03 or seconds; the beginning by default
  --end TIME                  Where to stop, as 2:00, 1:02:03 or seconds; the end by default
  --language CODE             Language of the speech: fr, en; detected by default
  --model FILE                Model file; the transcription.model setting by default
  --engine NAME               Engine; the transcription.engine setting by default
  --keep-audio FILE           Keep the extracted audio in this file
  --raw                       Write the segments as the engine cut them
```

## Argumente und Optionen

| Option | Erforderlich | Werte | Standard |
| :----- | :------- | :----- | :------ |
| `<input>` | ja | ein Video oder eine Audiodatei | — |
| `--output` | ja | die zu schreibende Untertiteldatei; ihr Verzeichnis wird angelegt, wenn es fehlt | — |
| `--to` | nein | der Kurzname eines Formats: `srt`, `vtt`, `subviewer2`, `ssa`, `ass`, `microdvd`, `mpl2`, `tmplayer`, `lrc` | das Format, das die Endung von `--output` angibt, sonst `srt` |
| `--audio-track` | nein | eine ganze Zahl ab 0: die Position unter den Tonspuren des Videos | `0`, die erste |
| `--language` | nein | zwei Kleinbuchstaben, ein ISO-639-1-Code: `en`, `fr` | die Einstellung `transcription.language`; leer: erkannt |
| `--start` | no | eine Zeitangabe: `2:00`, `1:02:03`, `2:00.5` oder eine Anzahl Sekunden, `90` | der Anfang des Audios |
| `--end` | no | eine Zeitangabe, geschrieben wie bei `--start`; sie muss nach `--start` liegen | das Ende des Audios |
| `--model` | nein | die Modelldatei | die Einstellung `transcription.model` |
| `--engine` | nein | der Name einer Engine: `whisper.cpp` | die Einstellung `transcription.engine` |
| `--keep-audio` | nein | eine Datei, in der das extrahierte Audio behalten wird, als 16-kHz-Mono-PCM mit 16 Bit in einer WAV-Datei; ihr Verzeichnis wird angelegt, wenn es fehlt | das Audio wird nirgends behalten |
| `--raw` | nein | ein Schalter: die Segmente der Engine ohne Zusammenfügen und ohne Schneiden schreiben | aus |

Die **Einstellungen** werden wie die von [`adjust`](adjust.md) aus der Einstellungsdatei gelesen — siehe
[die Einstellungsdatei](invocation.md#die-einstellungsdatei) — und jede hat hier ihre Option:

| Schlüssel | Bedeutung | Standard |
| :-- | :------ | :------ |
| `transcription.engine` | die Engine | `whisper.cpp` |
| `transcription.executable` | das Programm der Engine, ein absoluter Pfad; leer: im `PATH` gesucht | leer |
| `transcription.model` | die Modelldatei, ein absoluter Pfad | leer |
| `transcription.language` | die Sprache des Audios, zwei Buchstaben; leer: erkannt | leer |
| `transcription.merge-gap-ms` | Teile eines Satzes, die näher als so viele Millisekunden beieinander liegen, werden zusammengefügt | `400` |
| `transcription.max-duration-ms` | die längste Dauer, die ein Untertitel auf dem Bildschirm bleibt, in Millisekunden | `7000` |
| `transcription.max-characters` | die höchste Zeichenzahl eines Untertitels, Zeilenumbrüche nicht mitgezählt | `84` |

## Das Audio wählen

Ein Video kann mehrere Tonspuren tragen — das Original, einen Kommentar, eine andere Sprache. `--audio-track`
nimmt die **Position unter den Tonspuren** der Datei, ab 0, in der Reihenfolge, in der die Datei sie auflistet. Eine
Spur, die das Video nicht hat, wird abgelehnt, **mit den Spuren, die es hat**, damit der nächste Aufruf
der richtige ist:

```bash
subedit-cli transcribe --audio-track 4 --output film.srt --model small.bin film.mkv
```

```text
film.mkv: the video has no audio track 4
audio track 0: fra, Original
audio track 1: eng, Commentary
```

## Einen Teil transkribieren

`--start` und `--end` beschränken die Arbeit auf den Teil des Films zwischen beiden. Eines davon allein genügt: Ein
`--start` allein geht bis zum Ende, ein `--end` allein beginnt am Anfang.

```bash
subedit-cli transcribe --start 2:00 --end 3:30 --output test.srt --model large-v3.bin film.mkv
```

- **Eine Zeitangabe wird als `M:SS`, `H:MM:SS`, jeweils mit einem Dezimalteil nach einem Punkt (`2:00.5`), oder als Anzahl
  Sekunden (`90`, `90.5`) geschrieben**. Ein Komma wird abgelehnt, wie überall im Werkzeug.
- **Die Untertitel behalten die Zeit des Films**: Die bei 2:10 des Films gesprochene Zeile wird bei `00:02:10` geschrieben, nicht
  bei `00:00:10`. Die Datei kann unverändert über den ganzen Film gelegt werden.
- **Die Ränder sind ungefähr**: Die Engine arbeitet mit Fenstern von dreißig Sekunden, daher wird das, was sie liefert,
  anschließend auf den Bereich zugeschnitten; ein Satz, der über einen Rand hinausgeht, wird dort geschnitten.
- **Nichts beginnt, wenn der Bereich unbrauchbar ist**: Eine Grenze, die keine Zeitangabe ist, oder ein `--end`, das nicht nach
  `--start` liegt, ist ein Verwendungsfehler, der das Problem nennt, nichts schreibt und weder `ffmpeg` noch die
  Engine startet.

Das Audio des ganzen Films wird weiterhin extrahiert: Eingespart wird die Transkription, die viel länger dauert.

## Ausgabe

**Standardausgabe** — nichts: Das Ergebnis ist die geschriebene Datei.

| Stufe | Was auf der Fehlerausgabe hinzukommt |
| :---- | :-------------------------------- |
| 1 | `<input>: N subtitles transcribed, language: xx -> <output>` |
| 2 | der Schritt und der Fortschritt, eine Zeile pro Prozent: `extracting the audio…`, dann `transcribing…`, dann `transcribing: 25%, 0:12:30 of 0:50:00, about 38 min left` — das Prozent, die erreichte Position im Audio und die verbleibende Zeit |
| 3 | und jeder neue Text, den die Engine erhält, `last text: …` |

**Das Prozent ist die Position der Engine im Audio, nicht eine Zählung ihrer eigenen Fenster**: Die Engine schreibt, was sie
transkribiert hat, fortlaufend, und der Befehl liest, wie weit das gekommen ist, sodass sich die Zahl von der ersten
Minute an bewegt. (Der eigene Fortschritt der Engine kommt in Schritten von 5 % des Audios, nachdem ein Fenster von dreißig Sekunden fertig ist:
allein kann er bei einem langen Film mit einem großen Modell auf einem Prozessor eine Viertelstunde auf 0 % bleiben.) **Die
verbleibende Zeit schweigt zu Beginn**: Sie wird angegeben, sobald genug erledigt ist, um zu extrapolieren, und folgt der Geschwindigkeit der
letzten Minute, nicht dem Durchschnitt des ganzen Laufs.

### In JSON

Mit [`--format json`](invocation.md#ausgabe-die-ein-skript-lesen-kann) (die globale Option, vor dem
Unterbefehl geschrieben) enthält das Objekt der Datei die verwendete Sprache in `language`, `counts.subtitles` (die
geschriebenen Untertitel) und `counts.segments` (die Stücke der Engine, aus denen sie gemacht wurden), sowie `audio`, wenn
`--keep-audio` das Audio behalten hat.

## Exit-Codes

Die des Werkzeugs: `0`, wenn die Datei geschrieben wurde, `2`, wenn dies nicht möglich war — ein fehlendes Werkzeug oder Modell, eine
Sprache, die das Modell nicht verarbeiten kann, eine fehlgeschlagene Extraktion oder Transkription —, `1` bei einem Verwendungsfehler, etwa
einer Engine, die niemand registriert hat, oder einem Bereich, der nicht verwendet werden kann.

## Fehler

| Ursache | Meldung |
| :---- | :------ |
| kein Engine-Programm gefunden, oder das konfigurierte ist nicht vorhanden | `the transcription engine was not found`, dann der Verweis auf die Anleitung |
| kein Modell angegeben, oder es ist nicht vorhanden | `no model was given, or the model file is not there`, dann der Verweis auf die Anleitung |
| `ffmpeg` für ein Video nicht gefunden | `ffmpeg was not found, so the audio of a video cannot be extracted`, dann der Verweis |
| eine Spur, die das Video nicht hat | `the video has no audio track N`, dann die Spuren, die es hat |
| eine Sprache, die das Modell nicht transkribieren kann | `the model cannot transcribe the language xx` |
| keine Sprache, und ein Modell, das keine erkennen kann | `the model cannot detect the language: give one` |
| eine Engine, die niemand registriert hat | `no transcription engine is called NAME` |
| eine Grenze des Bereichs, die keine Zeitangabe ist | `"TEXT" is not a time: write it as 2:00, 1:02:03 or 2:00.5, or in seconds` |
| `--end` nicht nach `--start` | `the end of the range must come after its start` |
| jeder andere Fehler | `the transcription failed`, mit dem, was die Engine gesagt hat |

Der Verweis auf die Anleitung ist `See the manual page "Installing the transcription tools"
(install-transcription).`
