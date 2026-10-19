# Das verknüpfte Video

<!-- machine-translated -->
> Diese Seite ist eine maschinelle Übersetzung und wurde noch von keiner Leserin und keinem Leser dieser Sprache geprüft.

Das Menü **Video** enthält zunächst zwei Befehle, und die Handgriffe, die den Film steuern
— siehe [Der Player](player.md#sich-im-film-bewegen).

| Befehl | Tastenkürzel | Was er tut |
| :------ | :------- | :----------- |
| Video auswählen … | keines | wählt die Videodatei, zu der das Dokument gehört |
| Wiedergabe / Pause | `Ctrl+P` | spielt das Video ab oder hält es an — siehe [Der Player](player.md#abspielen-und-anhalten) |

Das verknüpfte Video ist **eine Eigenschaft des geöffneten Dokuments**, keine Einstellung
der Anwendung: Das Öffnen einer anderen Untertiteldatei beginnt von vorn. Es
wird in keine Datei geschrieben und verschwindet mit dem Fenster.

## Die Tonspur

Ein Film kann mehrere Tonspuren tragen — die Originalfassung, eine Synchronfassung, einen Kommentar. Das
Untermenü **`Video ▸ Audio ▸ Sprache`** listet sie auf, **ein Eintrag je Spur**, und **markiert die, die läuft**:

| Was die Datei angibt | Was der Eintrag sagt |
| :------------------ | :------------------ |
| eine Sprache und einen Titel | `1: fra — Original` |
| nur eine Sprache | `2: eng` |
| nur einen Titel | `3: Commentary` |
| keines von beiden | `1` |

Die Nummer ist **der Platz der Spur im Film**, eine nach der anderen, in der Reihenfolge der Datei: Zwei Spuren
können `eng` sagen, und eine Datei, die über ihre Bezeichnungen schweigt, hat dennoch eine erste und eine zweite.

**Einen Eintrag zu wählen, spielt diese Spur ab.** Die Markierung ist die Antwort des Players, nicht der Klick: Sie nennt die
Spur, die gerade läuft.

- **Das Untermenü ist deaktiviert**, wenn es kein Video oder keine Tonspur gibt. Bei einer einzigen Spur bleibt es aktiviert und
  zeigt sie, angekreuzt: Es nennt die Sprache des Films. Die zwei Handgriffe der Lautstärke, die daneben stehen, bleiben
  in jedem Fall aktiviert: siehe [Die Lautstärke](player.md#die-lautstärke).
- **Die Spur wird nicht gemerkt**, weder von einem Video zum nächsten noch von einer Sitzung zur nächsten: Eine Spurnummer ist
  die ihrer Datei, und sie auf den nächsten Film zu übertragen, würde dort wählen, was auch immer diese Nummer trägt.
  Zu einem Tab zurückzukehren, öffnet seinen Film wieder auf der Spur, die der Player von selbst wählt.
- Kein Tastenkürzel: Man wechselt selten die Sprache, und das Menü genügt.

## Ein Video wählen

Der Dialog öffnet sich im Verzeichnis der Untertiteldatei und filtert nach
den erkannten Videoendungen:

    .avi .flv .m2ts .mkv .mov .mp4 .ogg .ogv .vob .webm

Er bietet auch an, alle Dateien anzuzeigen. Den Dialog abzubrechen, ändert
nichts.

Die gewählte Datei wird **sofort geöffnet**: Das Bild erscheint im Fenster,
angehalten auf seinem ersten Augenblick. Siehe [Der Player](player.md).

**Eine Videodatei auf dem Fenster abzulegen, tut dasselbe** — siehe
[Dateien auf dem Fenster ablegen](files.md#dateien-auf-dem-fenster-ablegen).

**Das Video ist das des Projekts des aktuellen Tabs.** Jeder Tab behält
sein eigenes; den Tab zu wechseln, wechselt den Film. **Zu einem Tab zurückzukehren, setzt seinen
Film dort fort, wo Sie ihn verlassen haben**, angehalten, wie bei jedem Öffnen. Ein anderer Film,
der in der Zwischenzeit für dieses Projekt gewählt wurde, beginnt seinerseits am Anfang.

## Der automatische Vorschlag

Wenn eine Untertiteldatei geöffnet wird, **sofern kein Video gewählt wurde**,
sucht das Fenster eines im selben Verzeichnis: eine Videodatei, deren Name
ohne Endung ein Präfix des unseren ist, bis zu einem Punkt.

| Untertiteldatei | Gefundenes Video | Warum |
| :------------ | :---------- | :-- |
| `film.fr.srt` | `film.mkv` | `film` ist ein Präfix von `film.fr` |
| `film.en.forced.srt` | `film.mkv` | ebenso, unabhängig von der Anzahl der Segmente |
| `film.fr.srt` | `film.fr.mkv` statt `film.mkv` | der längste Name gewinnt |
| `film.fr.srt` | keines, wenn `film.mkv` **und** `film.mp4` vorhanden sind | zwei Kandidaten lassen sich nicht unterscheiden |
| `film.fr.srt` | keines, wenn nur `fil.mkv` vorhanden ist | `fil` endet nicht an einem Punkt |

**Ein Vorschlag ist keine Wahl.** Er lässt sich durch einen anderen
Vorschlag ersetzen — eine andere Datei öffnen, unter einem anderen Namen speichern. Eine ausdrückliche
Wahl dagegen wird nie überschrieben: Sobald `Select Video…` benutzt wurde, kommt der
automatische Vorschlag nicht mehr hinter ihr zurück.

Das macht die Vermutung annehmbar: Sie lässt sich korrigieren, und ihre
Korrektur hält.

## Was die Statusleiste zeigt

Rechts in der Statusleiste sagt eine Zeile, wozu das Dokument gehört. Sie
ist die letzte von dreien: Die [Kodierung](files.md#die-kodierung-in-der-statusleiste)
der gelesenen Datei kommt zuerst, das aus den Positionen abgeleitete [Bildraster](frame-grid.md)
danach.

| Situation | Was geschrieben wird |
| :-------- | :-------------- |
| ein Video ist verknüpft | `Video: film.mkv` |
| … und `ffprobe` liefert seine Bildrate | `Video: film.mkv, 24000/1001 fps` |
| keines | `No video` |

**Die Bildrate steht beim Film statt neben ihm.** Sie sind eine einzige Tatsache — wozu
dieses Dokument gehört —, und eine dritte Angabe würde sie auf dieselbe Stufe stellen wie
das [aus den Positionen abgeleitete Raster](frame-grid.md), das eine andere ist.

## Wenn es keinen Film gibt

Das Fenster zeigt eine Schaltfläche **`Select Video…`** dort, wo das Bild stünde, statt
gar nichts. Es ist derselbe Befehl wie der des Menüs `Video`, in
Reichweite der Maus: Ein Fehlen, gegen das man nichts tun kann, ist schwer von einem
Defekt zu unterscheiden.

Die Schaltfläche verschwindet, sobald ein Film geöffnet ist, und das Bild nimmt ihren **ganzen**
Platz ein: die Höhe, die die Tabelle über ihm lässt — die, die
[`window.table-share`](preferences.md) von einer Sitzung zur nächsten gemerkt hat —, und
nie weniger als die Mindesthöhe des Bildes, die die Tabelle notfalls abtritt.
Wenn der Film verschwindet, nimmt die Schaltfläche denselben Platz wieder ein.

**Der Name der Datei, nie ihr Pfad.** Ein Pfad von zweihundert Zeichen
würde alles andere aus der Leiste drängen; der vollständige Pfad ist der, den der
Dialog gezeigt hat.

## `ffmpeg` ist nicht erforderlich

`subedit-gui` befragt `ffprobe` — mit `ffmpeg` geliefert — zu einer einzigen
Sache: **der Bildrate, die der Container angibt**. Sie wird einmal
je Film gelesen, wenn er verknüpft wird.

| Mit `ffprobe` | Ohne |
| :------------- | :------ |
| die Statusleiste schreibt die Bildrate neben den Namen des Films | sie schreibt nur den Namen |
| der Umrechnungsdialog schlägt die Bildrate des Films im unteren Feld vor | er schlägt nichts vor und öffnet sich wie zuvor |
| der Einrastdialog öffnet sich auf ihr | er öffnet sich auf der Bildrate des Projekts |

**Nichts anderes hängt davon ab, und keine Operation lehnt ab.** Der Player seinerseits
braucht `ffprobe` überhaupt nicht: Er stützt sich auf `libmpv`, das eine
Abhängigkeit des Builds ist. Die Dauer des Films, die verwendet wird, um zu melden, was
über das Ende hinausgeht, kommt vom Player und nicht von `ffprobe`.

Es ist daher kein eingeschränkter Modus, sondern **der normale Betrieb** eines Rechners,
der `ffmpeg` nie installiert hat: Die Angabe kommt vom Benutzer, und `ffprobe`
schlägt sie vor, wenn es vorhanden ist.

Eine gelesene Bildrate ist ein **exakter Bruch** — `24000/1001`, nicht `23.976`.
Das ist der Grund, warum `ffprobe` für diese Antwort beibehalten wird: Der
Player weiß dasselbe als Gleitkommazahl, und eine angenäherte Bildrate
würde die ganze Datei leicht daneben neu timen.
