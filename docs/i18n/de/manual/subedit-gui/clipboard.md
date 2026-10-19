# Texte ausschneiden, kopieren und einfügen

<!-- machine-translated -->
> Diese Seite ist eine maschinelle Übersetzung und wurde noch von keiner Leserin und keinem Leser dieser Sprache geprüft.

Drei Einträge des Menüs `Edit`, unter `Undo` und `Redo`, und über
`Find and Replace…`.

| Eintrag | Tastenkürzel | Was er tut |
| :---- | :------- | :----------- |
| Texte ausschneiden | `Ctrl+X` | kopiert die Texte der Auswahl und leert sie dann |
| Texte kopieren | `Ctrl+C` | kopiert die Texte der Auswahl |
| Texte einfügen | `Ctrl+V` | schreibt die kopierten Texte ab der ersten ausgewählten Zeile |

**Es sind Texte, die übertragen werden, und nichts anderes**: kein Beginn, kein Ende, keine ganze
Zeile. Das Einfügen verschiebt daher keine Position — das Timing der Zeile, die
einen Text empfängt, bleibt ihr eigenes. So macht es Gaupol.

## Worauf sie wirken

**Mit einer Übersetzung, auf den Text der aktuellen Spalte**: die Übersetzung, wenn die
aktuelle Zelle in der Spalte `Translation` liegt, sonst der Haupttext —
siehe [Die Tabelle](table.md#der-text-auf-den-die-operationen-zielen). `Cut Texts` leert daher
die Übersetzung und lässt den Haupttext, und `Paste Texts` schreibt in die
Spalte, in der Sie sich befinden. **Die Zwischenablage trägt kein Dokument mit sich**: Sie
können eine Zelle des Haupttextes kopieren und in die Übersetzung einfügen, und
umgekehrt.

**Die Auswahl, und nie die ganze Datei.** Die drei Einträge sind deaktiviert, solange
nichts ausgewählt ist. An anderen Stellen des Fensters bedeutet „nichts ausgewählt“
„die ganze Datei“; hier würde ein unglückliches `Ctrl+X` alle Texte
des Dokuments leeren, und ein Einfügen hätte keine Zeile, an der es beginnen könnte.

Solange eine Zelle geöffnet ist, gehören `Ctrl+X`, `Ctrl+C` und `Ctrl+V`
dem Eingabefeld: Sie schneiden Zeichen aus, kopieren und fügen sie ein, so wie `Del`
dort ein Zeichen löscht und keinen Untertitel. Aus dem Menü gestartet, übernehmen die Einträge zuerst
die Eingabe: `Copy Texts` kopiert dann den Text so, wie er gerade
eingetippt wurde. Siehe [Eine Zelle bearbeiten](editing.md).

## Kopieren

`Copy Texts` kopiert den Text jeder ausgewählten Zeile, von der ersten bis zur
letzten. **Eine nicht zusammenhängende Auswahl behält ihre Form**: Eine aus
der Auswahl ausgelassene Zeile wird zu einem Loch, und das Einfügen wird sie nicht überschreiben.

**Eine ausgewählte Zeile, deren Text leer ist, wird ebenfalls zu einem Loch**:
Eingefügt, lässt es die Zielzeile, wie sie ist. Einen einzigen leeren Text zu kopieren, fügt daher
nichts ein.

Die Zwischenablage des Systems erhält diese Texte **als reinen Text, getrennt durch eine
leere Zeile** — ein Loch wird dort als leerer Text geschrieben, und deshalb kann ein leerer
Text nichts anderes als ein Loch sein: Das Einfügen ergibt dasselbe Ergebnis,
ob die Kopie aus diesem Fenster oder aus einem anderen Programm stammt. Was Sie hier kopieren, lässt sich
daher in jedes Programm einfügen, und was Sie anderswo kopieren, lässt sich hier einfügen.

Das Kopieren verändert das Dokument nicht und kommt nicht in den Verlauf.

## Ausschneiden

`Cut Texts` kopiert und **leert dann die Texte** der ausgewählten Zeilen. Die Zeilen
bleiben, mit ihren Positionen: Ausschneiden ist nicht Entfernen — um Zeilen zu
entfernen, gibt es [`Remove Subtitles`](lines.md#entfernen).

Texte auszuschneiden, die bereits alle leer sind, tut nichts und kommt nicht in den
Verlauf.

## Einfügen

`Paste Texts` schreibt die Texte der Zwischenablage **ab der ersten ausgewählten
Zeile**, eine Zeile je Text, nach unten.

| Was die Zwischenablage enthält | Was geschieht |
| :-------------------------- | :----------- |
| ein Text | er ersetzt den der Zeile |
| ein Loch, oder ein kopierter leerer Text | die Zeile behält ihren Text |
| mehr Texte, als Zeilen übrig sind | am Ende werden Zeilen hinzugefügt, um sie aufzunehmen |

Die hinzugefügten Zeilen entstehen wie Zeilen, die nach der letzten eingefügt werden: drei
Sekunden jede, eine nach der anderen. **Das Fenster sagt es**, in einem Informationsfenster:

```text
inserted 2 subtitles to fit the clipboard
```

Nach dem Einfügen **sind die geschriebenen Zeilen ausgewählt**.

Ein Einfügen, das nichts ändert — dieselben Texte auf denselben Zeilen —, kommt nicht in
den Verlauf.

### Tags eines anderen Formats werden übersetzt

Eine in diesem Fenster gemachte Kopie **merkt sich das Format ihres Dokuments**, und
diese Erinnerung übersteht das Öffnen einer anderen Datei. Das Einfügen in ein Dokument
eines anderen Formats übersetzt daher die Tags, wie `Save As…` es tut:
`{\i1}Bonjour{\i0}`, aus einem Advanced SSA kopiert, wird in einem
SubRip als `<i>Bonjour</i>` eingefügt.

**Was das Zielformat nicht schreiben kann, geht verloren, und es wird gesagt**, mit
den Worten von `Save As…`:

```text
pasting SubRip texts into LRC: 1 tag dropped
```

Anders als bei `Save As…` wird vorher nichts gefragt: Das Einfügen wird mit
`Ctrl+Z` rückgängig gemacht, was die Texte so zurückgibt, wie sie waren.

**Ein außerhalb von `subedit` kopierter Text hat kein Format**, und wird unverändert eingefügt:
Aus dem Nirgendwo gibt es nichts zu übersetzen. Das ist auch der Fall bei einer Kopie aus
diesem Fenster, die die Zwischenablage des Systems inzwischen ersetzt hat — es ist der
Inhalt der Zwischenablage des Systems, der zählt.

Wenn beide Meldungen fällig sind, passen sie in ein einziges Fenster, getrennt durch
ein Semikolon.

## Was die Aktion Rückgängig dazu sagt

Jeder Handgriff ist **ein** Eintrag im Verlauf, unabhängig von der Anzahl der
Zeilen, die er schreibt oder hinzufügt.

| Operation | Was `Undo` anzeigt |
| :-------- | :---------------- |
| Ausschneiden | `Undo: cutting texts` |
| Einfügen | `Undo: pasting texts` |

Siehe [Rückgängig und Wiederholen](undo.md).
