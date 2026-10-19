# Timing anhand des Videos

<!-- machine-translated -->
> Diese Seite ist eine maschinelle Übersetzung und wurde noch von keiner Leserin und keinem Leser dieser Sprache geprüft.

*Es wird vorausgesetzt, dass der Film geöffnet ist und der Player bekannt: siehe [Der Player](player.md).*

Sie sehen sich den Film an, halten auf dem Bild an, an dem eine Zeile beginnen soll, und **diese Position wird
zum Beginn des Untertitels**. Fünf Handgriffe, im Menü **Video**, unter den Namen, die Gaupol ihnen gibt. Sie
stützen sich auf das, was bereits existiert: Eine Position zu setzen, ist **derselbe Befehl, wie den Wert in die
Zelle einzutippen**, und das Einfügen ist **dasselbe wie `Bearbeiten ▸ Untertitel einfügen …`**.

| Befehl | Tastenkürzel | Was er tut |
| :------ | :------- | :----------- |
| Video ▸ Beginn auf Videoposition setzen | `Alt+U` | setzt den **Beginn** des ausgewählten Untertitels auf die Position des Films |
| Video ▸ Ende auf Videoposition setzen | `Alt+K` | setzt sein **Ende** auf die Position des Films |
| Video ▸ Untertitel an Videoposition einfügen | `Alt+J` | fügt einen Untertitel ein, der an der Position **beginnt**, und wählt ihn aus |
| Video ▸ Vorherigen ab Videoposition auswählen | `Alt+Up` | wählt den letzten Untertitel aus, der **vor** der Position **beginnt** |
| Video ▸ Nächsten ab Videoposition auswählen | `Alt+Down` | wählt den ersten aus, der **nach** der Position **beginnt** |

**Alle sind deaktiviert, solange kein Video geöffnet ist**, wie die Handgriffe des Players. `Set Start…` und `Set End…`
sind außerdem deaktiviert, solange nichts ausgewählt ist: Sie wirken auf die **erste Zeile** der
Auswahl.

**Die Tastenkürzel sind die von Gaupol, mit `Alt`**: Dort sind es `U`, `K` und `J` — bloße Buchstaben, die
das Fenster vor der gerade bearbeiteten Zelle abfangen würde, sodass das Tippen eines `k` in einem Text
ein Ende setzen würde. `Select Previous…` und `Select Next…` sind in Gaupol `Ctrl+Y` und `Ctrl+U`; `Ctrl+Y`
ist auf manchen Systemen ein *Wiederholen*, und die Pfeile mit `Alt` sagen *vor* und *nach* besser als
zwei Buchstaben. Eine gerade bearbeitete Zelle wird vor `Set Start…`, `Set End…` oder
`Insert Subtitle…` **übernommen**, wie vor jeder Änderung.

## Einen Rand setzen

- **Ein Eintrag im Verlauf je Handgriff**: `Bearbeiten ▸ Rückgängig` gibt die vorherige Position zurück, und sonst nichts.
- **Dieselben Regeln wie in der Zelle.** Ein Beginn, der nach dem Ende des Untertitels gesetzt wird, oder ein Ende vor seinem
  Beginn, **wird nicht abgelehnt**: Er wird gesetzt, und die Tabelle markiert den Untertitel so, wie sie den markiert,
  dessen Wert von Hand eingetippt wurde — siehe [Die Anomalien](table.md#die-anomalien). Nichts wird stillschweigend korrigiert.
- **Eine Position, die bereits die des Randes ist, tut nichts**, und fügt dem Verlauf nichts hinzu.
- **Die Position ist immer der Anfang eines Bildes**: Der Player meldet nie eine Position
  innerhalb eines Bildes. Einen Rand zu setzen und dann zu ihm zurückzukehren — zum Beispiel mit `Seek Selection Start` —,
  **zeigt dasselbe Bild**.

## An der Position einfügen

Der neue Untertitel **beginnt an der Position** und dauert **drei Sekunden**, oder bis zum Beginn des nächsten,
wenn dieser früher kommt. Sein Text ist leer. Er nimmt seinen Platz in der Reihenfolge der Datei ein: nach
allen, die an dieser Position oder davor beginnen. Das Einfügen ist **ein Eintrag im Verlauf**,
und der eingefügte Untertitel ist ausgewählt: Sie können ihn sofort eintippen.

## Von der Position aus auswählen

Beide folgen der Reihenfolge der Datei, nicht der aktuellen Auswahl. **Ohne Nachbarn auf dieser Seite** geht
die Auswahl bei `Select Next…` auf den letzten Untertitel der Datei, bei
`Select Previous…` auf den ersten — das Verhalten von Gaupol. Diese beiden Handgriffe bewegen den Film nicht.

## Bild für Bild

Um **ein Bild** vor- und zurückgehen — oder um N, siehe unten —, um einen Rand **auf das genaue Bild** zu setzen.
Zwei Handgriffe, und zwei Schaltflächen in der Wiedergabeleiste, zu beiden Seiten der Wiedergabeschaltfläche:

| Befehl | Tastenkürzel | Was er tut |
| :------ | :------- | :----------- |
| Video ▸ Ein Bild zurück | `Alt+Left` | setzt den Film um den Bildschritt zurück |
| Video ▸ Ein Bild vor | `Alt+Right` | bringt ihn um den Bildschritt voran |

- **Der Schritt ist eine Anzahl von Bildern, mindestens eines**: Es ist die Einstellung `Frame step` der
  [Einstellungen](preferences.md#der-player). Ein Schritt von N Bildern zeigt das Bild **N weiter**, bei 25 Bildern pro
  Sekunde wie bei 23,976; er wird **nie in Millisekunden ausgedrückt**.
- **Der Film hält an seinen Grenzen an**: Ein Schritt zurück vom ersten Bild lässt ihn dort, ein Schritt vor vom letzten
  ebenso. Die Wiedergabe ist nach einem Schritt angehalten.
- **Die Schaltflächen der Leiste** — zwei Symbole, zu beiden Seiten von Wiedergabe/Pause — **lösen dieselben Aktionen aus wie das Menü**: derselbe Schritt, deaktiviert,
  wenn kein Video da ist, wie die Einträge des Menüs, und derselbe Tooltip, der das Tastenkürzel nennt. **Ein Finger, der auf einer
  Schaltfläche oder auf der Taste bleibt, reiht die Schritte aneinander.**
- **Ein Schritt nach dem anderen.** Eine gehaltene Taste wiederholt sich schneller, als ein Schritt ausgeführt wird: Die erste Wiederholung geht sofort
  hinaus, die, die eintreffen, während der Schritt ausgeführt wird, **stauen sich nicht** — sie teilen sich einen Platz, den
  letzten, der danach hinausgeht. Das Loslassen der Taste hält das Bild daher dort an, wo Sie sind, und lässt den Film nicht
  aus eigenem Schwung weiterlaufen.

## Einen Rand um ein Bild verschieben

Um einen Rand mit der Tastatur anzupassen: **den Beginn oder das Ende des ausgewählten Untertitels**, um einen Schritt, früher oder später.

| Befehl | Tastenkürzel | Was er tut |
| :------ | :------- | :----------- |
| Video ▸ Beginn früher | `Alt+Q` | der Untertitel **beginnt einen Schritt früher** |
| Video ▸ Beginn später | `Alt+W` | er **beginnt einen Schritt später** |
| Video ▸ Ende früher | `Alt+Shift+Q` | er **endet einen Schritt früher** |
| Video ▸ Ende später | `Alt+Shift+W` | er **endet einen Schritt später** |

`Q` und `W` sind die von Gaupol für *früher* und *später*, mit `Alt` aus dem oben genannten Grund; das Ende
nimmt `Alt+Shift`, weil Gaupols `E` hier der Buchstabe von `Edit` ist.

- **Ein Eintrag im Verlauf je Verschiebung**, rückgängig zu machen. Der benachbarte Untertitel und die Überschneidung werden
  **wie beim Tippen** gemeldet: Der Rand wird gesetzt, und die Tabelle markiert, was nicht mehr stimmt — siehe
  [Die Anomalien](table.md#die-anomalien).
- **Derselbe Schritt wie beim Player**, also standardmäßig ein Bild. **Was ein Bild wert ist**, wird in dieser Reihenfolge
  gelesen: aus **der Bildrate, die das Video angibt**, aus der des **Dokuments, wenn es in Bildern zählt** (MicroDVD), dann aus
  **dem Raster, das die Positionen zeichnen**. **Ohne eine der drei lehnt der Handgriff ab und sagt es**: Eine
  Bildrate aufs Geratewohl zu wählen, würde jeden Rand um etwas verschieben, das kein Bild ist.
- **Er erfordert kein Video**, nur eine Auswahl: Er dient auch bei einer in Bildern gezählten Datei, ohne
  Film. Wenn ein Film geöffnet ist, **geht die Wiedergabe an die neue Position**, um das Bild zu zeigen, das der Rand
  gerade angenommen hat.
- Ein Rand geht nicht vor den Nullpunkt: Ein Schritt zurück von weniger als einem Bild aus setzt ihn auf null.
