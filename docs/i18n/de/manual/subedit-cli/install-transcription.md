# Die Transkriptionswerkzeuge installieren

<!-- machine-translated -->
> Diese Seite ist eine maschinelle Übersetzung und wurde noch von keinem Leser der Sprache geprüft.

Das Transkribieren von Audio in Untertitel — [`transcribe`](transcribe.md) in der Befehlszeile,
[`Transcribe Audio…`](../subedit-gui/video.md#das-audio-transkribieren) im Fenster — braucht drei
Dinge, die **subedit weder ausliefert noch herunterlädt**: eine Engine, ein Modell für diese Engine und
`ffmpeg`. Sie installieren sie, und Sie sagen subedit, wo das Modell liegt.

**Ohne sie geht nichts kaputt.** Der Rest des Programms funktioniert wie zuvor; nur die Transkription
schaltet sich ab, mit einer Meldung, die hierher verweist.

## Die Engine: whisper.cpp

[whisper.cpp](https://github.com/ggml-org/whisper.cpp) ist die Engine, die subedit ausführt. Es ist ein Programm,
`whisper-cli`, das auf einem gewöhnlichen Prozessor läuft — eine Grafikkarte macht es nur schneller. Bauen Sie es
so, wie es seine eigene Dokumentation beschreibt (ein C++-Compiler und CMake genügen):

```bash
git clone https://github.com/ggml-org/whisper.cpp
cd whisper.cpp
cmake -B build -DBUILD_SHARED_LIBS=OFF
cmake --build build -j --config Release
```

Das Programm ist dann `build/bin/whisper-cli`. Mit `-DBUILD_SHARED_LIBS=OFF` gebaut, wie oben, ist es eine einzelne Datei, die keine Bibliothek neben sich braucht, sodass Sie sie überallhin kopieren können. Legen Sie es dorthin, wo Ihr `PATH` es findet, oder **geben Sie seinen Pfad** subedit mit der Einstellung `transcription.executable` an — im Fenster haben die [Einstellungen](../subedit-gui/preferences.md#transkription) ein Feld dafür. Ein Build für eine Grafikkarte fügt der ersten `cmake`-Zeile eine Option hinzu: `-DGGML_CUDA=1` für eine NVIDIA-Karte, `-DGGML_VULKAN=1` für die meisten anderen; subedit führt aus, was Sie installiert haben, und wählt nichts für Sie.

Um nur das Programm zu behalten, **installieren** Sie es und löschen Sie den Klon:

```bash
cmake --install build --prefix ~/.local
```

`--prefix` sagt, wohin. `~/.local` braucht keine Administratorrechte, und sein Verzeichnis `bin` muss in Ihrem `PATH` liegen; ohne die Option ist der Standard `/usr/local`, das nach `sudo` verlangt. `whisper-cli` landet im Verzeichnis `bin` dieses Präfixes, neben Werkzeugen, die subedit nicht verwendet; sobald es dort ist, können der Klon und sein Verzeichnis `build` weg.

## Ein Modell

Die Engine liest ein **Modell**, eine Datei von einigen hundert Megabyte bis zu einigen Gigabyte, die Sie
einmal herunterladen. Die offiziellen Modelle sind aufgelistet unter
[huggingface.co/ggerganov/whisper.cpp](https://huggingface.co/ggerganov/whisper.cpp), und das
Repository der Engine hat ein Skript, das eines holt:

```bash
sh ./models/download-ggml-model.sh small
```

Geben Sie dann seinen Pfad mit `--model` in der Befehlszeile an, oder mit der Einstellung `transcription.model`,
oder in den [Einstellungen](../subedit-gui/preferences.md#transkription) des Fensters, die ihn sich merken.

### Ein Modell zur Spracherkennung, um dort zu beginnen, wo die Sprache beginnt

Optional lässt ein **Modell zur Spracherkennung** (Silero) die Engine die Stille überspringen, bevor sie dekodiert. Ohne eines beginnt Whisper seinen ersten Untertitel bei 0, was immer im Film zuerst kommt, Musik eingeschlossen, und rundet seine Zeiten auf ganze Fenster; mit einem beginnt der erste Untertitel dort, wo die Sprache beginnt, und die übrigen folgen ihm. Das Repository der Engine enthält ein Skript, das eines herunterlädt:

```bash
sh ./models/download-vad-model.sh silero-v6.2.0
```

Geben Sie seinen Pfad mit `--vad-model` an, mit der Einstellung `transcription.vad-model` oder in den [Einstellungen](../subedit-gui/preferences.md#transkription) des Fensters.

### Welches Modell für welche Sprache

Whisper wechselt das Modell nicht, sobald es die Sprache gefunden hat: **Das Modell, das Sie angeben, bestimmt, was es
kann.**

| Modell | Sprachen | Erkennt die Sprache | Größe |
| :---- | :-------- | :------------------- | :--- |
| `tiny`, `base`, `small`, `medium`, `large-v3`, `large-v3-turbo` | etwa hundert | ja | von etwa 75 MB bis etwa 3 GB |
| `tiny.en`, `base.en`, `small.en`, `medium.en` | nur Englisch | nein | wie die gleiche Größe oben |

- **Nehmen Sie für alles außer Englisch ein mehrsprachiges Modell** — eines, dessen Name kein `.en` hat. Ein Modell nur für
  Englisch lehnt eine andere Sprache ab: Der Befehl sagt es, bevor er irgendeine Arbeit leistet.
- **Die kleinsten Modelle sind außerhalb des Englischen spürbar schlechter.** Für Französisch, Deutsch, Spanisch und
  Ähnliches ist `small` das Minimum, und `medium` oder `large-v3-turbo` liefern viel bessere Ergebnisse zum Preis
  von mehr Speicher und mehr Zeit.
- **Die Spracherkennung ist nur so gut wie das Modell.** Ein kleines Modell kann sich bei einem Anfang aus
  Musik oder Stille irren; geben Sie die Sprache mit `--language` an, wenn Sie sie kennen.

## ffmpeg

[`ffmpeg`](https://ffmpeg.org/download.html) extrahiert die Tonspur eines Videos, und `ffprobe`, das
mitgeliefert wird, listet die Spuren auf. Installieren Sie es mit der Paketverwaltung Ihrer Distribution. Ohne es kann eine Audiodatei,
die die Engine unverändert liest (`wav`, `mp3`, `flac`, `ogg`), trotzdem transkribiert werden, ein Video
aber nicht.

## Prüfen, ob es funktioniert

```bash
subedit-cli transcribe --model /path/to/ggml-small.bin --output film.srt film.mkv
```

Fehlt etwas, sagt die Meldung was, und verweist auf diese Seite.
