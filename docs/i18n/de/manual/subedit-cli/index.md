# subedit-cli — Handbuch

<!-- machine-translated -->
> Diese Seite ist eine maschinelle Übersetzung und wurde noch von keiner Leserin und keinem Leser dieser Sprache geprüft.

Das Befehlszeilenwerkzeug von subedit: Arbeit an Untertiteldateien ohne grafische
Oberfläche, stapelweise oder aus einem Skript. Zum anderen Programm und um zu erfahren,
wo Sie anfangen, siehe
[das Handbuch](../index.md).

> **Aktueller Stand.** Die siebzehn Unterbefehle von `subedit-cli` existieren: untersuchen,
> konvertieren, verschieben, transformieren, die Bildrate umrechnen, an einem Bildraster einrasten, die
> Hinweise für Hörgeschädigte entfernen, die Dauern anpassen, einen Text ersetzen, die Schreibweise,
> die Kursivschrift und die Dialogstriche ändern, die Untertitel wieder ordnen, die
> Texte mit den Mustern von Gaupol korrigieren, Dateien aneinanderhängen, eine
> Datei in zwei Teile schneiden und eine Übersetzung an die Positionen ihrer Hauptdatei schreiben. Dieses Handbuch
> beschreibt, was existiert, niemals, was geplant ist; was als Nächstes kommt, steht in
> der [Roadmap](../../../../feuille-de-route.md).

## Abschnitte

| Abschnitt | Inhalt |
| :------ | :------ |
| [Installation](installation.md) | das Werkzeug bauen und installieren |
| [Aufruf](invocation.md) | globale Optionen, Ausgaben, Exit-Codes |
| [Die Einstellungsdatei](invocation.md#die-einstellungsdatei) | was `adjust` und `correct` aus der Einstellungsdatei des Fensters lesen, `--config`, `--no-config` |
| [Einen Verzeichnisbaum verarbeiten](lots.md) | mehrere Dateien, ein ganzes Verzeichnis, `--recursive` |
| [`inspect`](inspect.md) | berichten, woraus eine Datei besteht |
| [`convert`](convert.md) | eine Datei in einem anderen Format oder einer anderen Form schreiben |
| [`shift`](shift.md) | jede Position um denselben Betrag verschieben |
| [`transform`](transform.md) | jede Position anhand zweier Punkte korrigieren, die bekanntermaßen stimmen |
| [`framerate`](framerate.md) | eine Datei von einer Bildrate auf eine andere umrechnen |
| [`snap`](snap.md) | die Zeitstempel wieder auf die Bilder einer Bildrate setzen |
| [`hearing-impaired`](hearing-impaired.md) | die Hinweise für Hörgeschädigte entfernen |
| [`adjust`](adjust.md) | die Dauern an eine Lesegeschwindigkeit und an Grenzen anpassen |
| [`replace`](replace.md) | einen Text ersetzen, ohne ein Tag zu zerstören |
| [`case`](case.md) | die Texte in Titel-, Satz-, Groß- oder Kleinschreibung setzen |
| [`italics`](italics.md) | die Texte kursiv setzen oder ihre Kursivschrift entfernen |
| [`dialogue-dashes`](dialogue-dashes.md) | Dialogstriche setzen oder entfernen |
| [`sort`](sort.md) | die Untertitel wieder nach ihrem Beginn ordnen |
| [`join-repeats`](join-repeats.md) | aus den aufeinanderfolgenden Stücken eines Textes einen einzigen Untertitel machen |
| [`disjoint`](disjoint.md) | die überlappenden Untertitel so schneiden, dass sich kein Bereich mehr mit einem anderen überschneidet |
| [`repair`](repair.md) | `join-repeats`, `sort` und `disjoint` auf eine Datei verketten, die schlecht segmentiert, ungeordnet und überlappend ist |
| [`correct`](correct.md) | die Texte mit den Mustern von Gaupol korrigieren: Hinweise, Wörter zusammenfügen und trennen, häufige Fehler, Großschreibung, Zeilenumbruch |
| [`append`](append.md) | Dateien hinter die erste setzen, in eine einzige Datei |
| [`split-file`](split-file.md) | eine Datei in zwei Teile schneiden, der Rest auf den Nullpunkt zurückgesetzt |
| [`pair`](pair.md) | eine Übersetzung an die Positionen ihrer Hauptdatei schreiben und berichten, wie ihre Zeilen zugeordnet wurden |
| [`transcribe`](transcribe.md) | die Sprache eines Videos oder einer Audiodatei als Untertitel schreiben |
| [`sync`](sync.md) | die Untertitel einer Datei am Ton eines Films neu ausrichten |
| [Die Transkriptionswerkzeuge installieren](install-transcription.md) | die Engine, das Modell und `ffmpeg`, die das Programm nicht ausliefert |
| [`config`](config.md) | die mit dem Fenster geteilten Einstellungen der Operationen auflisten, anzeigen und ändern |
