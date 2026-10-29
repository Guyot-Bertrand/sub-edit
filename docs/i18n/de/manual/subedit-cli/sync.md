# `sync`

<!-- machine-translated -->
> Diese Seite ist eine maschinelle Übersetzung und wurde noch von keinem Leser der Sprache geprüft.

```
subedit-cli sync (--video FILE | --audio FILE)
                 [--audio-track N]
                 [--language CODE]
                 [--start TIME] [--end TIME]
                 [--model FILE]
                 [--vad-model FILE]
                 [--engine NAME]
                 [--keep-audio FILE]
                 [--min-match PERCENT]
                 [--doubt-gap MS]
                 [--report FILE]
                 [-r]
                 [--output FILE | --output-dir DIR | --in-place] [--dry-run]
                 <files>...
```

**Richtet eine Untertiteldatei am Audio eines Films neu aus.** Der Fall, für den er existiert, ist die online geladene `.srt`, die am
Anfang und am Ende verschoben ist und die eine einzige Verschiebung nicht zurechtrückt: Jeder Untertitel wird in dem gesucht, was gesagt wird,
und nimmt den Platz ein, an dem er gesagt wird.

Er hört das Audio mit einer Transkriptions-Engine ab — derselben wie bei [`transcribe`](transcribe.md) — und **gleicht
die Wörter jedes Untertitels mit den Wörtern ab, die sie gehört hat**. Nichts wird übersetzt: Die Untertitel müssen in der Sprache vorliegen, die
gesprochen wird.

**Nichts, was der Befehl braucht, wird mit dem Programm ausgeliefert**: Die Engine, ein Modell und `ffmpeg` sind die aus
[Die Transkriptionswerkzeuge installieren](install-transcription.md), und ihr Fehlen zerstört nichts — der Befehl sagt,
was fehlt, mit dem Verweis auf diese Seite, schreibt nichts und endet mit einem Code ungleich null.

- **Nur Anfänge und Enden ändern sich.** Nie der Text, der Stil, die Anzahl der Untertitel oder ihre Reihenfolge.
- **Ein Untertitel, den das Audio nicht liefert, behält seine Zeiten** — ein Abspann, den niemand spricht, ein Lied, eine Zeile in einer anderen Sprache.
  **Keine Position wird je erfunden, und keine wird aus den Nachbarn erraten**: Die Zeile bleibt dort, wo die Datei sie hatte,
  und der Befehl sagt es. Das ist *kein* Fehler, und der Exit-Code ist `0`.
- **Ein Untertitel gilt als gefunden, wenn mindestens die Hälfte seiner Wörter gehört wird** (`--min-match`, `alignment.min-match`), und
  mindestens eines. Groß- und Kleinschreibung, Akzente, Satzzeichen und Auszeichnung spielen keine Rolle; ein Wort, das bei einem langen Wort um einen Buchstaben abweicht, gilt als gehört.
- **Das Ende ist dort, wo das letzte Wort endet.** Die Engine datiert Wörter ungefähr, einige Hundertstelsekunden vor dem
  eigenen Ende des Untertitels, sodass ein Untertitel etwas kürzer ausfallen kann, als er war.
- **Ein Untertitel, der in den nächsten hineinlaufen würde, wird auf den Beginn des nächsten gekürzt.** Einer, der den
  nächsten trotzdem noch überschneidet, was das Kürzen nicht heilen kann, bleibt so und wird gemeldet.
- **Eine Datei, von der kein Untertitel gefunden wurde, wird abgelehnt und nicht geschrieben**: Es ist fast sicher das falsche Audio oder die
  falsche Sprache.
- **Ein Teil des Films kann allein abgehört werden**, mit `--start` und `--end`, wie bei
  [`transcribe`](transcribe.md#einen-teil-transkribieren): Nur die Untertitel, die im Bereich **beginnen**, werden gesucht und
  angefasst, die anderen bleiben, wie sie sind, und werden nicht mitgezählt.
- **Das Audio wird für jede angegebene Datei einmal abgehört.** Wer mehrere Dateien am selben Film neu ausrichtet, muss
  jedes Mal warten.

<!-- example: subedit-cli sync --help -->
```console
$ subedit-cli sync --help
Re-time the subtitles of a file on the audio of a film
Usage: subedit-cli sync [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to re-time

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --video FILE                Film to listen to
  --audio FILE                Audio file to listen to, instead of a film
  --audio-track N             Audio track of a film, counted from 0; the first by default
  --start TIME                Where to begin, as 2:00, 1:02:03 or seconds; the beginning by default
  --end TIME                  Where to stop, as 2:00, 1:02:03 or seconds; the end by default
  --language CODE             Language of the speech, two letters: fr, en, de; detected from the first 30 seconds by default
  --model FILE                Model file; the transcription.model setting by default
  --vad-model FILE            Voice-activity model that skips silences; the transcription.vad-model setting by default
  --engine NAME               Engine that listens; the transcription.engine setting by default
  --keep-audio FILE           Keep the extracted audio in this file
  --min-match PERCENT         Share of a subtitle's words that must be heard, in percent; the alignment.min-match setting by default
  --doubt-gap MS              How far a subtitle may move from its neighbours' shift before it is flagged, in milliseconds; the alignment.doubt-gap setting by default
  --report FILE               Write the subtitles to check to this file
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Argumente und Optionen

| Option | Erforderlich | Werte | Standard |
| :----- | :------- | :----- | :------ |
| `<files>` | ja | die neu auszurichtenden Untertiteldateien; mit `-r` Verzeichnisse | — |
| `--recursive`, `-r` | nein | ein Schalter | aus — siehe [Einen Verzeichnisbaum verarbeiten](lots.md) |
| `--video` | eine von `--video` und `--audio` | der Film, der abgehört wird | — |
| `--audio` | eine von `--video` und `--audio` | eine Audiodatei, die statt eines Films abgehört wird | — |
| `--audio-track` | nein | eine ganze Zahl ab 0: die Position unter den Tonspuren des Films | `0`, die erste |
| `--language` | nein | zwei Buchstaben, ein ISO-639-1-Code: `en`, `fr` | die Einstellung `transcription.language`; leer: erkannt |
| `--start`, `--end` | nein | eine Zeitangabe: `2:00`, `1:02:03`, `2:00.5` oder eine Anzahl Sekunden; `--end` muss nach `--start` liegen | das ganze Audio |
| `--model` | nein | die Modelldatei | die Einstellung `transcription.model` |
| `--vad-model` | nein | ein Modell zur Spracherkennung, das die Engine die Stille überspringen lässt; mit ihm folgen die Wörter der Sprache genauer | die Einstellung `transcription.vad-model` |
| `--engine` | nein | der Name der Engine, die zuhört: `whisper.cpp` | die Einstellung `transcription.engine` |
| `--keep-audio` | nein | eine Datei, in der das extrahierte Audio behalten wird; ihr Verzeichnis wird angelegt, wenn es fehlt | das Audio wird nirgends behalten |
| `--min-match` | nein | eine ganze Zahl von 1 bis 100: der Anteil der Wörter eines Untertitels, die gehört werden müssen, in Prozent | die Einstellung `alignment.min-match`, `50` |
| `--doubt-gap` | nein | eine ganze Zahl von Millisekunden, 0 oder mehr: wie weit die Verschiebung eines Untertitels von der seiner Nachbarn abweichen darf, bevor er markiert wird | die Einstellung `alignment.doubt-gap`, `2000` |
| `--report` | nein | eine Datei, in die die zu prüfenden Untertitel geschrieben werden, einer pro Zeile: die Nummer des Untertitels, ein Tabulator und der Grund; jeweils eine Untertiteldatei | nichts wird geschrieben |
| `--output` / `--output-dir` / `--in-place` | **eine der drei**, außer mit `--dry-run` | siehe [Aufruf](invocation.md#das-ziel) | — |
| `--dry-run` | nein | ein Schalter: berechnet und sagt, schreibt nichts — siehe [Vor dem Schreiben nachsehen](invocation.md#vor-dem-schreiben-nachsehen) | aus |

Das Ziel — `--output`, `--output-dir`, `--in-place`, `--dry-run` — und `-r` sind die der anderen Befehle, die eine Datei
umschreiben: siehe [das Ziel](invocation.md#das-ziel). Die Datei behält ihr Format, ihre Kodierung und ihre
Zeilenenden.

Die **Einstellungen** werden aus der Einstellungsdatei gelesen — siehe [die Einstellungsdatei](invocation.md#die-einstellungsdatei) — und jede
hat hier ihre Option. Die Engine, das Modell und das Modell zur Spracherkennung sind die von `transcribe`.

| Schlüssel | Bedeutung | Standard |
| :-- | :------ | :------ |
| `alignment.engine` | die Ausrichtungs-Engine; es gibt eine, `transcript-matching`, die zuhört und abgleicht | `transcript-matching` |
| `alignment.min-match` | der Anteil der Wörter eines Untertitels, die gehört werden müssen, damit er gefunden wird, in Prozent | `50` |
| `alignment.doubt-gap` | wie weit, in Millisekunden, die Verschiebung eines Untertitels von der seiner Nachbarn abweichen darf, bevor er markiert wird | `2000` |

## Was markiert wird

Der Befehl sagt, welche Untertitel zu prüfen sind, und warum. Ein Untertitel wird für den **ersten** der folgenden Gründe markiert, der zutrifft:

| Grund | Bedeutung |
| :-- | :------ |
| not found in the audio, left as it was | weniger als `--min-match` seiner Wörter wurden gehört: er behält seine Zeiten |
| only partly heard | gefunden, aber weniger als 80 % seiner Wörter wurden gehört |
| moved differently from its neighbours | gefunden, aber seine Verschiebung weicht von der mittleren Verschiebung der zwei Untertitel auf jeder Seite um mehr als `--doubt-gap` ab |

Ein Untertitel kann außerdem **auf den Beginn des nächsten Untertitels gekürzt** werden oder **den nächsten Untertitel weiterhin überschneiden**.

Zwei Zahlen erzählen die Geschichte der Datei als Ganzes. **Die mittlere Verschiebung** ist die Mitte dessen, wie weit sich die Untertitel bewegt haben: Eine
Datei, die einfach zu spät war, zeigt sich hier und nirgends sonst. **Die Drift** ist die mittlere Verschiebung des letzten Zehntels der
Untertitel minus die des ersten Zehntels: Sie liegt bei einem konstanten Versatz nahe null und ist groß bei einer Datei, die im Lauf des Films
immer mehr zu spät war.

## Ausgabe

**Standardausgabe** — nichts: Das Ergebnis ist die geschriebene Datei.

| Stufe | Was auf der Fehlerausgabe gesagt wird |
| :---- | :------------------------------- |
| 1 | `<file>: N of M subtitles placed on the audio, K to check; median shift X s, drift Y s -> <destination>` |
| 2 | der Schritt und der Fortschritt, wie bei [`transcribe`](transcribe.md#ausgabe) |

### In JSON

Mit [`--format json`](invocation.md#ausgabe-die-ein-skript-lesen-kann) enthält das Objekt der Datei `counts`: `subtitles`
(gesucht), `placed` (mit einem Platz versehen), `moved` (deren Zeiten sich geändert haben), `partial`, `out_of_line`, `not_found`,
`median_shift_ms` und `drift_ms`; und `flagged`, eine Liste von `{"subtitle": N, "kind": "…"}`, wobei die Nummer der
Platz des Untertitels in der Datei ist und `kind` eines von `not-found`, `partial`, `out-of-line`, `cut-back` und `overlaps`.
Die Kennungen sind zugesagt; die Sätze nicht.

## Beispiel

```bash
subedit-cli sync --video film.mkv --model small.bin --vad-model silero.bin \
    --report to-check.txt --output film.synced.srt film.srt
```

## Exit-Codes

Die des Werkzeugs: `0`, wenn die Datei geschrieben wurde, mit markierten Untertiteln oder ohne, `2`, wenn dies nicht möglich war — ein fehlendes Werkzeug oder Modell, eine fehlgeschlagene Extraktion oder Abhörung, kein
Untertitel gefunden —, `1` bei einem Verwendungsfehler, etwa wenn weder
`--video` noch `--audio` angegeben ist (oder beide), bei einer Option außerhalb ihres Bereichs oder einem Bereich, der nicht verwendet werden kann.

## Fehler

| Ursache | Meldung |
| :---- | :------ |
| weder `--video` noch `--audio`, oder beide | `sync needs the audio to listen to: give --video or --audio, and not both` |
| kein Engine-Programm gefunden, oder das konfigurierte ist nicht vorhanden | `the transcription engine was not found`, dann der Verweis auf die Anleitung |
| kein Modell angegeben, oder es ist nicht vorhanden | `no model was given, or the model file is not there`, dann der Verweis auf die Anleitung |
| `ffmpeg` für einen Film nicht gefunden | `ffmpeg was not found, so the audio of a video cannot be extracted`, dann der Verweis |
| eine Spur, die der Film nicht hat | `the video has no audio track N`, dann die Spuren, die er hat |
| eine Sprache, die das Modell nicht transkribieren kann | `the model cannot transcribe the language xx` |
| eine Engine, die niemand registriert hat | `no transcription engine is called NAME`, oder `there is no alignment engine called NAME` |
| kein Untertitel gefunden | `none of the N subtitles was found in the audio: is it the right audio, and the right language?` |
| `--min-match` außerhalb von 1 bis 100, `--doubt-gap` negativ | der Verwendungsfehler der Option |
| `--report` mit mehr als einer Untertiteldatei | `--report names one file: give one subtitle file when you use it` |
