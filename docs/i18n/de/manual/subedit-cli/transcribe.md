# `transcribe`

<!-- machine-translated -->
> Diese Seite ist eine maschinelle Übersetzung und wurde noch von keinem Leser der Sprache geprüft.

```
subedit-cli transcribe --output FILE
                       [--to FORMAT]
                       [--audio-track N]
                       [--language CODE]
                       [--start TIME] [--end TIME]
                       [--suppress-notes | --keep-notes]
                       [--model FILE]
                       [--vad-model FILE]
                       [--engine NAME]
                       [--keep-audio FILE]
                       [--raw]
                       <input>

subedit-cli transcribe --list-languages [--model FILE] [--engine NAME]
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
- **Eine Sprache wird mit zwei Buchstaben eingegeben — `fr`, `en`, `de` — oder mit dem dreibuchstabigen Code, den eine Datei angibt, `fra`,
  `eng`, der wie der zweibuchstabige gelesen wird.** Alles andere wird vor jeder Arbeit abgelehnt. **Die Erkennung
  hört nur die ersten dreißig Sekunden ab**: Ein Film, der mit Musik oder Stille beginnt, kann für eine
  andere Sprache gehalten werden, und die Engine schreibt die Sprache dann *übersetzt* in diese Sprache — ein französischer Film, der
  für englisch gehalten wird, kommt auf Englisch heraus oder als `[SPEAKING FRENCH]`. Wenn Sie die Sprache kennen, geben Sie sie an.
- **Ein Teil des Films kann allein transkribiert werden**, mit `--start` und `--end` — siehe
  [Einen Teil transkribieren](#einen-teil-transkribieren). So probiert man ein Modell an zwei Minuten aus, bevor man es auf
  zwei Stunden anwendet.
- **Die Sprachen, die eine Engine anbietet, lassen sich auflisten**, mit `--list-languages` — siehe
  [Die Sprachen auflisten](#die-sprachen-auflisten).
- **Geräusche werden standardmäßig nicht als Anmerkungen geschrieben**: Die Engine bekommt die Anweisung, es nicht zu tun (`--suppress-notes`; ihre eigene Option heißt
  `--suppress-nst`), sodass `(Sonnerie de téléphone)` oder `[MUSIC PLAYING]` nicht in den Untertiteln auftauchen, weder in runden noch in eckigen Klammern.
  `--keep-notes` schreibt sie, für Untertitel für Hörgeschädigte. Sind die Anmerkungen verboten, schreibt die Engine `...` für eine Stelle ohne Sprache, und
  **die Nachbearbeitung verwirft die Stücke, die keinen Buchstaben und keine Ziffer haben** — `...`, `…`, `♪` — vor dem Zusammenfügen und Schneiden; `--raw`
  behält sie, so wie die Stücke der Engine kamen.
- **Die Segmente werden zusammengefügt und geschnitten** nach den Grenzen der Einstellungen (`transcription.merge-gap-ms`,
  `transcription.max-duration-ms`, `transcription.max-characters`): Zwei nahe beieinander liegende Stücke eines
  Satzes ergeben einen Untertitel, und ein zu langes oder zu breites Stück wird geschnitten — zuerst an einem Satzende, dann an einem Komma, Semikolon oder Doppelpunkt, und mitten in einem Satz nur, wenn es keine Satzzeichen zum Schneiden gibt —, wobei ein Schnitt vermieden wird, der einen zu kleinen, nicht mehr lesbaren Untertitel hinterlässt. Die drei Schritte laufen der Reihe nach ab, jeder über die ganze Transkription: verwerfen, zusammenfügen, schneiden. Ein Untertitel, der ein Stück beginnt, behält seinen Anfang, und einer, der es beendet, behält sein Ende. **`--raw`
  schreibt die Segmente der Engine, wie sie kamen**, eines pro Untertitel, und so sieht man auch, was das
  Zusammenfügen bewirkt.

<!-- example: subedit-cli transcribe --help -->
```console
$ subedit-cli transcribe --help
Write the speech of a video or an audio file as subtitles
Usage: subedit-cli transcribe [OPTIONS] [input]

Positionals:
  input TEXT                  Video or audio file to transcribe

Options:
  -h,--help                   Print this help message and exit
  --output FILE               Subtitle file to write
  --to TEXT:{srt,vtt,subviewer2,ssa,ass,microdvd,mpl2,tmplayer,lrc}
                              Format to write; the one the extension of --output says by default
  --audio-track N             Audio track of a video, counted from 0; the first by default
  --start TIME                Where to begin, as 2:00, 1:02:03 or seconds; the beginning by default
  --end TIME                  Where to stop, as 2:00, 1:02:03 or seconds; the end by default
  --language CODE             Language of the speech, two letters: fr, en, de; detected from the first 30 seconds by default
  --suppress-notes            Do not write sounds as notes, such as (phone ringing); the transcription.suppress-notes setting by default
  --keep-notes                Write sounds as notes, against the setting
  --model FILE                Model file; the transcription.model setting by default
  --vad-model FILE            Voice-activity model that skips silences; the transcription.vad-model setting by default
  --engine NAME               Engine; the transcription.engine setting by default
  --keep-audio FILE           Keep the extracted audio in this file
  --list-languages            List the languages the engine offers for the model, and transcribe nothing
  --raw                       Write the segments as the engine cut them
```

## Argumente und Optionen

| Option | Erforderlich | Werte | Standard |
| :----- | :------- | :----- | :------ |
| `<input>` | ja, außer mit `--list-languages` | ein Video oder eine Audiodatei | — |
| `--output` | ja, außer mit `--list-languages` | die zu schreibende Untertiteldatei; ihr Verzeichnis wird angelegt, wenn es fehlt | — |
| `--to` | nein | der Kurzname eines Formats: `srt`, `vtt`, `subviewer2`, `ssa`, `ass`, `microdvd`, `mpl2`, `tmplayer`, `lrc` | das Format, das die Endung von `--output` angibt, sonst `srt` |
| `--audio-track` | nein | eine ganze Zahl ab 0: die Position unter den Tonspuren des Videos | `0`, die erste |
| `--language` | nein | zwei Buchstaben, ein ISO-639-1-Code: `en`, `fr`; ein dreibuchstabiger Code wie `fra` wird als `fr` gelesen | die Einstellung `transcription.language`; leer: erkannt |
| `--suppress-notes` | nein | ein Schalter: Geräusche nicht als Anmerkungen schreiben; abgelehnt bei einer Engine, der es nicht gesagt werden kann | die Einstellung `transcription.suppress-notes` |
| `--keep-notes` | nein | ein Schalter: sie schreiben, gegen die Einstellung; das Gegenteil von `--suppress-notes`, und beide zusammen werden abgelehnt | die Einstellung `transcription.suppress-notes` |
| `--list-languages` | nein | ein Schalter: die Sprachen auflisten, die die Engine für das Modell anbietet, und nichts transkribieren | aus |
| `--start` | no | eine Zeitangabe: `2:00`, `1:02:03`, `2:00.5` oder eine Anzahl Sekunden, `90` | der Anfang des Audios |
| `--end` | no | eine Zeitangabe, geschrieben wie bei `--start`; sie muss nach `--start` liegen | das Ende des Audios |
| `--model` | nein | die Modelldatei | die Einstellung `transcription.model` |
| `--vad-model` | nein | ein Modell zur Spracherkennung (ein Silero-Modell für whisper.cpp), das die Engine die Stille vor dem Dekodieren überspringen lässt: Ihre Segmente folgen dann der Sprache, und das erste beginnt dort, wo die Sprache beginnt, statt bei 0. Die Datei muss existieren, und eine Engine, die es nicht verwenden kann, lehnt die Option mit ihrem Namen ab | die Einstellung `transcription.vad-model` |
| `--engine` | nein | der Name einer Engine: `whisper.cpp` | die Einstellung `transcription.engine` |
| `--keep-audio` | nein | eine Datei, in der das extrahierte Audio behalten wird, als Mono-PCM mit 16 Bit in einer WAV-Datei, mit der Abtastrate, die die Engine verlangt (16 kHz bei whisper.cpp); ihr Verzeichnis wird angelegt, wenn es fehlt, und der Name kann `{rate}` tragen, ersetzt durch diese Rate in Hertz (`film.{rate}.wav` wird als `film.16000.wav` geschrieben) | das Audio wird nirgends behalten |
| `--raw` | nein | ein Schalter: die Segmente der Engine ohne Zusammenfügen und ohne Schneiden schreiben | aus |

Die **Einstellungen** werden wie die von [`adjust`](adjust.md) aus der Einstellungsdatei gelesen — siehe
[die Einstellungsdatei](invocation.md#die-einstellungsdatei) — und jede hat hier ihre Option:

| Schlüssel | Bedeutung | Standard |
| :-- | :------ | :------ |
| `transcription.engine` | die Engine | `whisper.cpp` |
| `transcription.executable` | das Programm der Engine, ein absoluter Pfad; leer: im `PATH` gesucht | leer |
| `transcription.model` | die Modelldatei, ein absoluter Pfad | leer |
| `transcription.vad-model` | das Modell zur Spracherkennung, ein absoluter Pfad; leer: keines, und die Stille wird nicht übersprungen. Eine Einstellung, die eine Engine nicht verwenden kann, wird wortlos verworfen | leer |
| `transcription.language` | die Sprache des Audios, zwei Buchstaben; leer: erkannt | leer |
| `transcription.suppress-notes` | Geräusche nicht als Anmerkungen schreiben | `true` |
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

## Die Sprachen auflisten

`--list-languages` schreibt die Sprachen, die die Engine **für das Modell** anbietet, eine pro Zeile: den Code, den `--language` nimmt, einen Tabulator und den Namen
der Sprache in der Sprache der Oberfläche. Es braucht weder eine Eingabe noch eine Ausgabe und transkribiert nichts.

```bash
subedit-cli transcribe --list-languages --model ggml-small.bin
```

```text
en	English
zh	Chinese
de	German
…
```

Die Liste hängt von der Engine und vom Modell ab: etwa hundert Sprachen bei einem mehrsprachigen whisper.cpp-Modell, nur `en` bei einem Modell nur für Englisch. Eine Engine, die ihre Sprachen nicht auflisten kann, listet keine auf. Ein Modell, das nicht angegeben ist oder nicht da ist, ist ein Fehler, mit dem Verweis auf die Anleitung; nur
die Textform, mit [`--format json`](invocation.md#ausgabe-die-ein-skript-lesen-kann) wird nichts geschrieben.

## Ausgabe

**Standardausgabe** — nichts: Das Ergebnis ist die geschriebene Datei.

| Stufe | Was auf der Fehlerausgabe hinzukommt |
| :---- | :-------------------------------- |
| 1 | `<input>: N subtitles transcribed, language: xx -> <output>` |
| 2 | der Schritt und der Fortschritt, eine Zeile pro Prozent: `extracting the audio…`, dann `transcribing…`, dann `transcribing: 25%, 0:12:30 of 0:50:00, about 38 min left, elapsed: 0:05:12` — das Prozent, die erreichte Position im Audio, die verbleibende Zeit und die vergangene Zeit; **die Zeile wird alle zehn Sekunden wiederholt, wenn nichts Neues gekommen ist**, damit ein langes Warten nicht für ein Einfrieren gehalten wird |
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

## Hinweise

Ein Hinweis wird auf der ersten Stufe vor der Arbeit ausgegeben, weil er ändert, was zu erwarten ist:

- **Mit einem Modell nur für Englisch**: `This model is for English only: whatever language the film
  speaks, the result will be English, or notes in brackets. Choose a multilingual model in the preferences.` Ein Modell,
  dessen Dateiname auf `.en` endet, ist eines.

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
| `<input>` oder `--output` fehlt | `input is required` oder `--output is required` |
| `--suppress-notes` mit `--keep-notes` | `--suppress-notes and --keep-notes say opposite things; give one of them` |
| `--suppress-notes` für eine Engine, der es nicht gesagt werden kann | `the engine NAME cannot be told not to write sounds as notes` |
| eine Sprache, die kein Code ist | `"TEXT" is not a language code: write two letters, such as fr, en, de or es` |
| eine Grenze des Bereichs, die keine Zeitangabe ist | `"TEXT" is not a time: write it as 2:00, 1:02:03 or 2:00.5, or in seconds` |
| `--end` nicht nach `--start` | `the end of the range must come after its start` |
| jeder andere Fehler | `the transcription failed`, mit dem, was die Engine gesagt hat |

Der Verweis auf die Anleitung ist `See the manual page "Installing the transcription tools"
(install-transcription).`
