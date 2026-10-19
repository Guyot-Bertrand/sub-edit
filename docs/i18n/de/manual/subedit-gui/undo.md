# Rückgängig und Wiederholen

<!-- machine-translated -->
> Diese Seite ist eine maschinelle Übersetzung und wurde noch von keiner Leserin und keinem Leser dieser Sprache geprüft.

Jede Operation kommt in einen Verlauf und lässt sich rückgängig machen. Eine bearbeitete Zelle zählt
als **ein** Eintrag: Das Tippen wird nicht Tastendruck für Tastendruck aufgezeichnet, nur das
Übernehmen zählt.

| Aktion | Tastenkürzel | Wo |
| :----- | :------- | :---- |
| Rückgängig | `Ctrl+Z` | Menü **Bearbeiten**, Werkzeugleiste |
| Wiederholen | `Ctrl+Shift+Z` | Menü **Bearbeiten**, Werkzeugleiste |

## Die Aktion nennt, was sie rückgängig machen wird

Das Menü sagt nicht „Rückgängig“, sondern **„Rückgängig: Bearbeiten eines Textes“**, nach der
Operation, die rückgängig gemacht wird. Die Schaltfläche der Werkzeugleiste behält das bloße Wort — ihre
Breite würde sonst der letzten Operation folgen, und sie würde sich unter dem
Mauszeiger verschieben —, aber ihr Tooltip trägt die vollständige Beschriftung.

| Operation | Was die Aktion anzeigt |
| :-------- | :-------------------- |
| eine bearbeitete Textzelle | `editing a text` |
| ein Beginn, ein Ende | `editing a start`, `editing an end` |
| eine Dauer | `editing a duration` |
| ein Einfügen von Zeilen | `inserting` |
| ein Entfernen von Zeilen | `removing` |
| eine Verschiebung, `Shift Whole File onto Grid` eingeschlossen | `shifting` |
| eine Transformation | `transforming` |
| eine Umrechnung der Bildrate | `converting the frame rate` |
| ein Einrasten an einer Bildrate | `aligning on the frame rate` |
| ein Entfernen von Hinweisen | `removing hearing-impaired mentions` |
| kursiv setzen | `putting in italics` |
| die Kursivschrift herausnehmen | `taking italics out` |
| eine Änderung der Groß-/Kleinschreibung | `changing the case` |
| hinzugefügte, entfernte Striche | `adding dialogue dashes`, `removing dialogue dashes` |
| ein Zusammenführen von Zeilen | `merging` |
| ein Teilen einer Zeile | `splitting` |
| ausgeschnittene, eingefügte Texte | `cutting texts`, `pasting texts` |
| eine Anpassung der Dauern | `adjusting durations` |
| das Öffnen einer Übersetzung | `opening a translation` |
| das Anhängen einer Datei | `appending a file` |
| das Teilen des Projekts | `splitting the project` |
| eine in `Correct Texts…` angenommene oder von `Check Spelling…` vorgenommene Korrektur | `correcting texts` |
| eine Ersetzung, alle Ersetzungen | `replacing`, `replacing all` |

**Siebenundzwanzig Beschriftungen, und das ist alles, was das Fenster erzeugen kann.** Der Kern nennt
eine achtundzwanzigste — `sorting` —, die keine Aktion des Fensters erreicht: Sortiert wird
nur unter einer strengen Ordnungsrichtlinie, während das Fenster seine Dokumente
unter der nachsichtigen Richtlinie öffnet, die Unordnung meldet, statt sie zu beheben.

**`Shift Whole File onto Grid` wird nicht von einer Verschiebung unterschieden**, und das ist
zutreffend: Es ist eine, deren Betrag gemessen statt eingetippt wurde.

Es sind dieselben Wörter wie die der Befehlszeile: Sie werden einmal geschrieben,
im Kern.

Wenn es nichts rückgängig zu machen gibt, zeigt die Aktion schlicht „Rückgängig“ **und ist
deaktiviert**: Ihr Tastenkürzel tut nichts.

## Die Änderungsmarkierung

Der Fenstertitel trägt eine Markierung, solange sich die Datei von der auf der
Festplatte unterscheidet — ein Sternchen, oder was die Plattform stattdessen verwendet. **Ebenso die Beschriftung
des Tabs**, `premier.srt*`: Der Titel spricht nur vom Projekt, das Sie gerade
ansehen, die Beschriftung erzählt von denen, die Sie nicht ansehen.

**Jeder Tab hat seinen eigenen Verlauf.** Rückgängig macht nur rückgängig, was das Projekt des
aktuellen Tabs erhalten hat — ein über alle Projekte ausgeführtes `Replace All` eingeschlossen: Es
hinterlässt in jedem Projekt, das es berührt hat, einen Eintrag.

**Sie verschwindet, wenn Sie bis zum Ausgangspunkt rückgängig machen.** Das ist keine Kleinigkeit
der Anzeige: Die Anzahl der Änderungen wird gezählt, nicht als Ja oder Nein festgehalten, sodass
das Zurückgehen sie wieder auf null bringen kann, und die Markierung verschwindet.

## Zwei Dinge, die man wissen sollte

**Der Bezugspunkt ist das letzte Speichern**, oder das Öffnen der Datei, wenn es
keines gab. Das Speichern verschiebt ihn: Die Markierung verschwindet, und ein Rückgängigmachen danach
bringt sie zurück.

Eine Operation, die nach einem Rückgängigmachen ausgeführt wird, **löscht, was es zu wiederholen gab**: Wiederholen
würde einen Befehl erneut abspielen, dessen Ausgangszustand nicht mehr existiert. Es ist das
Verhalten jedes Editors, und es ist gewollt.

**Solange eine Zelle geöffnet ist**, übernehmen `Undo` und `Redo` — aus dem Menü oder der Werkzeugleiste —
zuerst die Eingabe, wie die anderen Handgriffe von
[Eine Zelle bearbeiten](editing.md). Was gerade eingetippt wurde, wird dann der letzte
Eintrag des Verlaufs: `Undo` macht ihn rückgängig, nicht die Operation davor; `Redo` hat
nichts mehr zu wiederholen, da eine Eingabe löscht, was es zu wiederholen gab. Eine geöffnete
Zelle, in die nichts eingetippt wurde, hinterlässt keinen Eintrag: `Undo` macht dann die vorherige
Operation rückgängig.

Der Verlauf behält tausend Einträge und vergisst darüber hinaus die ältesten.
