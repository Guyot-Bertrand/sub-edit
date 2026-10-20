# Aufruf

<!-- machine-translated -->
> Diese Seite ist eine maschinelle Übersetzung und wurde noch von keiner Leserin und keinem Leser dieser Sprache geprüft.

```
subedit-gui [--version] [<file>]
```

| Argument | Erforderlich | Wert | Standard |
| :------- | :------- | :---- | :------ |
| `<file>` | nein | der Pfad einer zu öffnenden Untertiteldatei | keiner — das Fenster öffnet sich leer |
| `--version` | nein | — | — |

**Eine einzige Datei.** Die folgenden Argumente werden ignoriert: Das Fenster enthält
jeweils nur ein Projekt.

## `--version`

Schreibt auf die Standardausgabe den Block, den `--version` der Befehlszeile schreibt, und
kehrt sofort zurück, ohne ein Fenster zu öffnen. Exit-Code `0`.

**`subedit <version>` allein in der ersten Zeile**, dann eine Zeile je Bibliothek, mit der das
Fenster läuft, mit der zur Laufzeit gefundenen Version und, in Klammern, der Version,
gegen die es kompiliert wurde; dann die installierten Sprachen der Oberfläche, nach Code. Auf
einem Rechner mit Qt 6.4.2:

```console
$ subedit-gui --version
subedit <version>
  ICU      74.2 (built with 74.2)
  Enchant  2.3.3 (built with 2.3.3)
  Qt       6.4.2 (built with 6.4.2)
  libmpv   2.2 (built with 2.2)
  Languages: en, fr
```

Die Nummern werden hier nicht abgeschrieben: Sie sind die des Rechners, und dieses Handbuch hat keinen
erzeugten Block, der die des Programms selbst aktuell hielte. Die Bibliotheken sind
die der Befehlszeile, ICU und Enchant, und die zwei, die nur das Fenster linkt — Qt, und
libmpv, der Player, angegeben mit der Version seiner Client-API, der, die `mpv.pc`
ankündigt. Der Rest ist in
[dem Handbuch der Befehlszeile](../subedit-cli/invocation.md#--version) beschrieben. Die Wörter
`built with` und `Languages` folgen der Sprache des Fensters; sonst nichts.

`Hilfe ▸ Über subedit` zeigt dieselben Bibliotheken, aus derselben Liste, in einer Tabelle —
der Name, die geladene Version, die Version, gegen die gebaut wurde, je eine Zeile — und darunter die
Sprachen; ihre Überschriften folgen der Sprache des Fensters.

## Eine Datei öffnen

```console
$ subedit-gui film.srt
```

Die gelesenen Formate sind **die neun**, die der Filter zum Öffnen aufführt — siehe
[Öffnen und Speichern](files.md#öffnen). Das Format wird am Inhalt erkannt und
nicht an der Endung, und **ebenso die Kodierung**: an einer BOM,
wenn die Datei eine trägt, sonst an der Kodierung, die ihre Bytes nahelegen.

## Wenn das Öffnen scheitert

Eine fehlende Datei, eine unlesbare, eine, deren Bytes sich in keiner
Kodierung dekodieren lassen, oder eine, deren Inhalt kein Format erkennt: **Das Fenster öffnet sich
trotzdem, leer**, und das Scheitern wird auf den Fehlerkanal geschrieben.

**Die Meldung nennt die Ursache**, mit denselben Worten wie die Befehlszeile:

| Meldung | Was geschehen ist |
| :------ | :------------ |
| `does not exist` | der Pfad bezeichnet keine Datei |
| `cannot be opened: permission denied` | das System verweigert das Öffnen |
| `cannot be read` | das System hat aus einem anderen Grund verweigert |
| `cannot be decoded in the chosen encoding` | die Bytes lassen sich in der gewählten Kodierung nicht lesen |
| `is in no format this tool knows` | kein Format erkennt den Inhalt |
| `holds nothing recognisable as a subtitle` | das Format wird erkannt, aber nichts darin ist ein Untertitel |

```console
$ subedit-gui notes.txt
subedit-gui: notes.txt: is in no format this tool knows
```

Leer öffnen, statt den Start zu verweigern: Das Werkzeug taugt trotzdem zu
etwas, und das Scheitern eines Arguments ist kein Grund, nicht zu starten.

**Eine schlecht geformte, aber lesbare Datei öffnet sich**, und was das Lesen
unterwegs aufgefangen hat, lässt sich über eine Schaltfläche der Statusleiste lesen — siehe
[die Diagnosen beim Lesen](files.md#die-diagnosen-beim-lesen).

## Exit-Codes

| Code | Wann |
| :--- | :--- |
| `0` | das Fenster wurde geöffnet und dann normal geschlossen, oder `--version` hat geantwortet |
| `2` | ein unerwarteter Fehler vor dem Öffnen — Qt fehlt, Anzeige nicht verfügbar |

Das Scheitern beim Öffnen einer Datei **ändert den Exit-Code nicht**: Das
Fenster hat sich geöffnet, und das meldet der Code.

## Die Handbuchseite

Eine Installation legt `subedit-gui(1)` unter `share/man/man1` ab:

```console
$ man subedit-gui
```

Sie nennt die Aufrufform, `--version`, die Meldungen beim Scheitern des Öffnens, die
Exit-Codes, die zwei Umgebungsvariablen und die Einstellungsdatei.
**Sie beschreibt das Fenster nicht** — das tut dieses Handbuch, und die Seite
verweist hierher.

## Die Qt-Plattform

`QT_QPA_PLATFORM` wählt, worauf das Fenster läuft. **Ist die Variable
gesetzt, hält sich `subedit-gui` ohne Widerrede daran.**

Ist sie es nicht, lässt es Qt wählen: Der eingebaute Player zeichnet das Bild im
Fenster selbst und verlangt nichts von der Plattform — siehe
[Der Player](player.md).

`QT_QPA_PLATFORM=offscreen` lässt das Fenster dort laufen, wo es keinen
grafischen Server gibt — was die kontinuierliche Integration und die Tests tun.

## Sprache

Das Fenster spricht die Sprache seiner Umgebung, **einmal gewählt, beim Start**: Die
Variablen eines laufenden Fensters zu ändern, ändert nichts bis zum nächsten Start. Die Variablen und die Regeln
sind die der Befehlszeile — `LANGUAGE`, `LC_ALL`, `LC_MESSAGES`, `LANG`, `LC_ALL=C` für
Englisch, Englisch **ohne ein Wort**, wenn die verlangte Sprache nicht installiert ist — beschrieben in
[dem Handbuch der Befehlszeile](../subedit-cli/invocation.md#sprache). Die Kataloge sind dieselben
Dateien, installiert unter `<prefix>/share/subedit/locale`.

**Eine in `Bearbeiten ▸ Einstellungen …` gewählte Sprache hat Vorrang**, und die Umgebung wird nur
gelesen, wenn sie `System default` ist, was sie ist, bis eine gewählt wird: Eine dort gewählte installierte Sprache
setzt sich gegen jede Variable durch, `LC_ALL=C` eingeschlossen. Auch die Wahl gilt ab dem nächsten Start. Siehe
[Die Sprache der Oberfläche](preferences.md#die-sprache-der-oberfläche).

| Was | Folgt der Sprache |
| :--- | :------------------- |
| die Menüs und ihre Einträge, mit ihren unterstrichenen Buchstaben; die Werkzeugleiste; die Tooltips | ja |
| die Spalten der Tabelle und von `Ansicht ▸ Spalten`, die Tabs, die Statuszeile, die Wiedergabeleiste | ja |
| die Standardschaltflächen, die Qt selbst zeichnet (`OK`, `Cancel`) | ja, wenn Qts eigener Katalog dieser Sprache installiert ist (`qtbase_<language>.qm`); sonst Englisch |
| die Dialoge, die Seiten von `Correct Texts…`, die Titel und Filter der Dateidialoge, die Meldungsfenster und was sie sagen | ja |
| die Zeilen, die das Fenster auf seinen Fehlerkanal schreibt, `subedit-gui: …` | ja, mit Ausnahme des Programmnamens |
| die Namen der Korrekturmuster in `Correct Texts…` und ihre Beschreibungen in den Tooltips | ja, für die mit dem Programm gelieferten Muster; ein eigenes Muster wird so gezeigt, wie seine Datei es schreibt |
| die Namen von Kodierungen, Zeilenenden und Rechtschreibsprachen (`UTF-8`, `CRLF`, `en_US`) | nie: Es sind Namen, keine Wörter |
| die Tastenkürzel | nie: `Ctrl+S` ist `Ctrl+S` in jeder Sprache |
| eine Meldung, die der Katalog nicht übersetzt | auf Englisch gezeigt |

Diese Version installiert `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt` und `pt_BR` — Katalanisch, Tschechisch, Deutsch,
Spanisch, Finnisch, Französisch, Galicisch, Hebräisch, Ungarisch, Interlingue, Isländisch, Italienisch, Niederländisch, Polnisch, Portugiesisch und Brasilianisches Portugiesisch — vollständig übersetzt. Französisch wurde von einer Leserin oder einem
Leser der Sprache geprüft; Katalanisch, Tschechisch, Deutsch, Spanisch, Finnisch, Galicisch, Hebräisch, Ungarisch, Interlingue, Isländisch, Italienisch, Niederländisch, Polnisch, Portugiesisch und Brasilianisches Portugiesisch sind
maschinell übersetzt und noch von keiner Leserin und keinem Leser der Sprache geprüft.
<!-- machine-translated: ca -->
<!-- machine-translated: cs -->
<!-- machine-translated: de -->
<!-- machine-translated: es -->
<!-- machine-translated: fi -->
<!-- machine-translated: gl -->
<!-- machine-translated: he -->
<!-- machine-translated: hu -->
<!-- machine-translated: ie -->
<!-- machine-translated: is -->
<!-- machine-translated: it_IT -->
<!-- machine-translated: nl -->
<!-- machine-translated: pl -->
<!-- machine-translated: pt -->
<!-- machine-translated: pt_BR -->
Auf einer französischen Arbeitsumgebung lautet die Menüleiste
`&Fichier`, `É&dition`, `&Affichage`, `&Vidéo`, `&Outils`, `&Projets`, `Aid&e`. Die Bildschirmfotos
dieses Handbuchs werden auf Deutsch aufgenommen.
