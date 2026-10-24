# subedit — Handbuch

<!-- machine-translated -->
> Diese Seite ist eine maschinelle Übersetzung und wurde noch von keiner Leserin und keinem Leser dieser Sprache geprüft.

subedit liest, untersucht und passt die Zeiten von Untertiteldateien an. Es besteht aus zwei
Programmen, die denselben Kern teilen und damit dieselben Regeln und dieselben Wörter.

| Programm | Wozu | Sein Handbuch |
| :-------- | :-------------- | :--------- |
| `subedit-gui` | eine Datei öffnen, sie in einer Tabelle sehen, sie bearbeiten, sie zusammen mit dem Film ansehen | [das Handbuch des Fensters](subedit-gui/index.md) |
| `subedit-cli` | Dateien stapelweise verarbeiten, aus einem Skript, ohne Oberfläche | [das Handbuch der Befehlszeile](subedit-cli/index.md) |

Beide lesen und schreiben **die neun Formate** — SubRip, WebVTT, SubViewer 2,
Sub Station Alpha, Advanced SSA, MicroDVD, MPL2, TMPlayer, LRC — und
erkennen das Format an seinem Inhalt statt an seiner Endung. **Auch die Kodierung
wird am Inhalt erkannt**: Eine Datei in Latin-1, in CP1252 oder in UTF-16
öffnet sich, es wird angegeben, als was sie gelesen wurde, und sie wird in ihrer eigenen Kodierung zurückgeschrieben — eine
Datei, die geöffnet und ohne Änderung gespeichert wird, ändert sich um kein einziges Byte.

## Wo anfangen

| Was Sie wollen | Wohin Sie gehen |
| :----------------- | :------- |
| das Werkzeug installieren | [Installation](subedit-cli/installation.md) — gilt für beide Programme |
| eine Datei öffnen und ansehen, was sie enthält | [Die Tabelle](subedit-gui/table.md) |
| Untertitel zeitlich anpassen, die um einen festen Betrag verschoben sind | [Die Operationen](subedit-gui/operations.md#shift-positions) oder [`shift`](subedit-cli/shift.md) |
| eine Datei umrechnen, die für eine andere Bildrate geschrieben wurde | [`framerate`](subedit-cli/framerate.md), und [`snap`](subedit-cli/snap.md), wenn das Timing bereits stimmt |
| wissen, auf welche Bildrate hin eine Datei geschrieben wurde | [Das Bildraster](subedit-gui/frame-grid.md) oder [`inspect`](subedit-cli/inspect.md) |
| die Hinweise für Hörgeschädigte entfernen | [Die Operationen](subedit-gui/operations.md#remove-hearing-impaired-mentions) oder [`hearing-impaired`](subedit-cli/hearing-impaired.md) |
| Hinweise, häufige Fehler, Groß-/Kleinschreibung und Zeilenumbruch in einem einzigen Durchgang korrigieren | [`Correct Texts…`](subedit-gui/correct-texts.md) oder [`correct`](subedit-cli/correct.md) |
| die Rechtschreibung Wort für Wort prüfen, mit den Wörterbüchern des Systems | [`Check Spelling…`](subedit-gui/check-spelling.md) |
| Untertitel verlängern, die zu kurz sind, um gelesen zu werden | [`Adjust Durations…`](subedit-gui/operations.md#adjust-durations) oder [`adjust`](subedit-cli/adjust.md) |
| eine Datei wieder in Ordnung bringen, deren Untertitel es nicht sind | [`sort`](subedit-cli/sort.md) |
| aus den aufeinanderfolgenden Stücken eines Textes einen einzigen Untertitel machen | [`Join Repeated Subtitles`](subedit-gui/lines.md#zusammenfassen-wiederholter-untertitel) oder [`join-repeats`](subedit-cli/join-repeats.md) |
| eine Datei, die schlecht segmentiert, ungeordnet und überlappend ist, in einem Zug richten | [`repair`](subedit-cli/repair.md) |
| die Überlappungen zwischen Untertiteln entfernen | [`Make Ranges Disjoint…`](subedit-gui/lines.md#bereiche-überschneidungsfrei-machen) oder [`disjoint`](subedit-cli/disjoint.md) |
| die zwei Teile eines Films aneinanderhängen | [`Append File…`](subedit-gui/operations.md#append-file) oder [`append`](subedit-cli/append.md) |
| einen Film in zwei Teile schneiden | [`Split Project…`](subedit-gui/operations.md#split-project) oder [`split-file`](subedit-cli/split-file.md) |
| die Schreibweise, die Kursivschrift oder die Dialogstriche einer ganzen Datei ändern | [Die Operationen](subedit-gui/operations.md#italic) oder [`case`](subedit-cli/case.md), [`italics`](subedit-cli/italics.md), [`dialogue-dashes`](subedit-cli/dialogue-dashes.md) |
| ein Wort suchen und ersetzen, ohne die Tags zu zerstören | [Suchen und Ersetzen](subedit-gui/find-and-replace.md) oder [`replace`](subedit-cli/replace.md) |
| die Sprache eines Films als Untertitel schreiben | [`Transcribe Audio…`](subedit-gui/video.md#das-audio-transkribieren), oder [`transcribe`](subedit-cli/transcribe.md) — nach der [Installation der Werkzeuge](subedit-cli/install-transcription.md) |
| neben dem Originaltext übersetzen | [Öffnen einer Übersetzung](subedit-gui/files.md#öffnen-einer-übersetzung) oder [`pair`](subedit-cli/pair.md), um sie zeitlich an die Hauptdatei anzupassen |
| hundert Dateien auf einmal verarbeiten | [Einen Verzeichnisbaum verarbeiten](subedit-cli/lots.md), dann [Aufruf](subedit-cli/invocation.md) |

**Zwischen `framerate` und `snap` bleibt ein Fehler unbemerkt.** Wenn
Sie zögern: Die Seite [`snap`](subedit-cli/snap.md#snap-ist-nicht-framerate-und-eine-verwechslung-fällt-nicht-auf)
beginnt mit dem Vergleich der beiden.

## Was dieses Handbuch verspricht

**Es beschreibt, was existiert, niemals, was geplant ist.** Was kommen wird und in
welcher Reihenfolge, steht in der [Roadmap](../../../feuille-de-route.md).

**Dieses Handbuch ist auf Englisch geschrieben und wird übersetzt.** Seine Übersetzungen folgen ihm
Seite für Seite, im Verzeichnis `docs/i18n/` des Repositorys, ein Baum pro Sprache. Die
Programme sprechen die Sprache ihrer Umgebung; die Namen, die sie zeigen — Unterbefehle,
Optionen, Menübeschriftungen —, werden hier so zitiert, wie das Programm sie auf Englisch schreibt.

**Die Beispiele und die Bilder werden erzeugt, der Text nicht.** Die
`console`-Blöcke entstehen, indem der Befehl tatsächlich ausgeführt wird; die
Bildschirmfotos, indem das echte Fenster gebaut und fotografiert wird. Keines
von beiden kann daher ein Werkzeug beschreiben, das es nicht mehr gibt. Ohne Netz bleibt
der Text um sie herum, dessen Richtigkeit auf dem Korrekturlesen beruht.

**Die Bildschirmfotos zeigen beide Paletten**, `Light` und `Dark` — die,
die [die Einstellungen](subedit-gui/preferences.md#das-design) anbieten.

## Beide Oberflächen sagen dieselben Worte

Ein Grund, das eine Handbuch zu lesen, wenn man das andere Programm benutzt: Die
Fehlermeldungen, die Namen der Formate, die Urteile über das Raster und die Beschriftungen
von Rückgängig werden **einmal** geschrieben, im Kern, und beide Programme
zitieren sie. Eine abgelehnte Datei ergibt auf beiden Seiten denselben Satz.

Was sich unterscheidet, ist, was jede Oberfläche kann:

| | `subedit-gui` | `subedit-cli` |
| :--- | :------------ | :------------ |
| mehrere Dateien auf einmal | ja — ein Tab pro Projekt | ja |
| eine Übersetzung neben der Hauptdatei | ja | ja — `-t` bei den Textunterbefehlen, und [`pair`](subedit-cli/pair.md), um sie zeitlich anzupassen |
| eine Datei anhängen, ein Projekt teilen | ja | ja — [`append`](subedit-cli/append.md), [`split-file`](subedit-cli/split-file.md) |
| Rückgängig | ja, tausend Einträge | entfällt — nichts wird ungefragt an Ort und Stelle geändert |
| einen Text oder eine Position von Hand bearbeiten | ja | nein |
| Zeilen einfügen, löschen, zusammenführen und teilen | ja | nein |
| Suchen und Ersetzen | ja | ja — [`replace`](subedit-cli/replace.md) |
| Texte ausschneiden, kopieren und einfügen | ja | nein |
| die Dauern, die Kursivschrift, die Schreibweise, die Striche anpassen | ja | ja — [`adjust`](subedit-cli/adjust.md), [`italics`](subedit-cli/italics.md), [`case`](subedit-cli/case.md), [`dialogue-dashes`](subedit-cli/dialogue-dashes.md) |
| die Texte mit Mustern korrigieren | ja — `Correct Texts…` | ja — [`correct`](subedit-cli/correct.md) |
| die Rechtschreibung Wort für Wort prüfen | ja — `Check Spelling…` | nein |
| den Film beim Timing ansehen | ja | nein |
| über die Eingabe schreiben | `Save` | `--in-place` |
