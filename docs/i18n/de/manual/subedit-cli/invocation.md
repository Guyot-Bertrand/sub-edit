# Aufruf

<!-- machine-translated -->
> Diese Seite ist eine maschinelle Übersetzung und wurde noch von keiner Leserin und keinem Leser dieser Sprache geprüft.

```
subedit-cli [global options] <subcommand> [options] <file>...
```

Ohne Unterbefehl gibt das Werkzeug seine Hilfe aus und beendet sich.

<!-- example: subedit-cli -->
```console
$ subedit-cli
Read, inspect and retime subtitle files.
Usage: subedit-cli [OPTIONS] [SUBCOMMAND]

Options:
  -h,--help                   Print this help message and exit
  --version                   Display program version information and exit
  -v                          Say more: -v is the default, -vv details, -vvv debugs
  -q,--quiet                  Say nothing but errors
  --encoding NAME             Encoding to read the files in; detected by default
  --format text|json          Form of the result on standard output: text or json
  --config FILE               Read the operation settings from FILE instead of the user's settings file
  --no-config Excludes: --config
                              Ignore any settings file: use the built-in defaults
  --list-encodings            Write the encodings that can be read and written, one a line, and stop

Subcommands:
  inspect                     Report what a subtitle file is made of
  convert                     Write a subtitle file out in another format or shape
  shift                       Move every position of a file by a fixed amount
  transform                   Correct every position from two points known to be right
  framerate                   Re-time a file mastered at one frame rate for another
  snap                        Move every position onto the nearest frame of a frame rate (see framerate)
  hearing-impaired            Remove the sounds described between square brackets
  adjust                      Bring the duration of every subtitle within a reading speed and limits
  replace                     Replace a text in the subtitles, without breaking a tag
  case                        Put the texts in title, sentence, upper or lower case, tags intact
  italics                     Put the texts in italics, or take their italics out
  dialogue-dashes             Put dialogue dashes on the lines, or take them off
  sort                        Put the subtitles in the order of their start, keeping ties as they are
  join-repeats                Join the subtitles that repeat each other, one after the other, into one
  disjoint                    Cut overlapping subtitles so that no range overlaps another, merging what overlaps
  repair                      Join the repeated subtitles, put them in order, then make the ranges disjoint
  correct                     Correct the texts with the patterns of the correction assistant
  append                      Put subtitle files one after another into a single file
  split-file                  Cut a subtitle file in two, a head and a tail
  pair                        Write a translation at the positions of its main file, and say how it lined up
  config                      List, show and change the operation settings
```

## Sprache

Die Befehlszeile spricht die Sprache ihrer Umgebung: **ihre Hilfe, ihre Verlaufsmeldungen und ihre
Fehler** folgen der verlangten Sprache. Die Beispiele dieses Handbuchs werden auf Englisch ausgeführt — die
Blöcke werden von `make manual` erzeugt, niemals von Hand kopiert.

**Wie die Sprache gewählt wird.** Die erste der Variablen `LC_ALL`, `LC_MESSAGES` und `LANG`, die gesetzt und
nicht leer ist, ist die Locale; `LANGUAGE` listet dann, wenn gesetzt, die gewünschten Sprachen auf, getrennt durch
Doppelpunkte (`LANGUAGE=pt_BR:fr`), und wird zuerst gelesen. Das ist die Reihenfolge, der gettext folgt, gelesen vom
Programm selbst: Die Locale muss auf dem Rechner nicht erzeugt worden sein.

| Regel | Wirkung |
| :--- | :----- |
| Kodierung und Modifikator | ignoriert: `fr_FR.UTF-8@euro` sucht `fr_FR`, dann `fr` |
| `LC_ALL=C`, Locale `C` oder `POSIX` oder gar keine Locale | Englisch, und `LANGUAGE` wird nicht gelesen |
| ein Eintrag, der Englisch nennt (`en`, `en_GB`) | Englisch: Was in `LANGUAGE` darauf folgt, wird nicht gelesen |
| eine Sprache, deren Katalog nicht installiert ist | die nächste verlangte, und nach der letzten Englisch, **ohne ein Wort** |
| eine Meldung, die der Katalog nicht übersetzt | auf Englisch angezeigt |

**Der Name eines Korrekturmusters**, das `correct` nach eigener Meldung nicht anwenden konnte, folgt ebenfalls der
Sprache, wenn der Katalog ihn kennt — bei den mit dem Programm ausgelieferten Mustern; ein eigenes Muster wird
so genannt, wie seine Datei es schreibt. Die Musterdateien selbst werden niemals umgeschrieben.

Englisch ist eingebaut, und die Kataloge der übrigen Sprachen werden unter `<prefix>/share/subedit/locale/<language>/LC_MESSAGES/subedit.mo`
installiert und vom Verzeichnis des Programms selbst aus gefunden. Diese Version installiert `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR`, `ru`, `sr`, `tr` und `zh_CN` — Katalanisch, Tschechisch,
Deutsch, Spanisch, Finnisch, Französisch, Galicisch, Hebräisch, Ungarisch, Interlingue, Isländisch, Italienisch, Niederländisch, Polnisch, Portugiesisch, Brasilianisches Portugiesisch, Russisch, Serbisch, Türkisch und Vereinfachtes Chinesisch —, vollständig übersetzt. Französisch wurde von einer Leserin
oder einem Leser der Sprache geprüft; Katalanisch, Tschechisch, Deutsch, Spanisch, Finnisch, Galicisch, Hebräisch, Ungarisch, Interlingue, Isländisch, Italienisch, Niederländisch, Polnisch, Portugiesisch, Brasilianisches Portugiesisch, Russisch, Serbisch, Türkisch und Vereinfachtes Chinesisch sind maschinell
übersetzt und noch nicht von einer Leserin oder einem Leser der Sprache geprüft.
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
<!-- machine-translated: ru -->
<!-- machine-translated: sr -->
<!-- machine-translated: tr -->
<!-- machine-translated: zh_CN -->
Unter `LANG=fr_FR.UTF-8` beginnt `subedit-cli --help` mit „Lire,
inspecter et recaler des fichiers de sous-titres.“ Ein Skript, das die Wörter der Textausgabe
liest, setzt `LANGUAGE=en` (oder `LC_ALL=C`) — oder liest `--format json`, das sich nie ändert.

**Was niemals der Sprache folgt** — das, was ein Skript liest:

| Niemals übersetzt | Warum |
| :--------------- | :-- |
| die Ausgabe von `--format json`: Schlüssel, Werte sowie `message` und `detail` von Fehlschlägen und Warnungen | dieselben Bytes in jeder Sprache; die Wörter sind englisch |
| die Exit-Codes | die vier aus [Exit-Codes](#exit-codes), in jeder Sprache |
| die eingetippten Namen: Unterbefehle, Optionen, ihre Werte (`off`, `srt`), Metavariablen (`FILE`, `TEXT`, `OPTIONS`) | eine einmal geschriebene Befehlszeile funktioniert unter jeder Sprache |
| die Namen der Korrekturmuster, die `correct --enable` und `--disable` annehmen | ein Muster wird mit dem englischen Namen aufgerufen, den seine Datei enthält |
| die Namen der Formate und Kodierungen | ein Format wird mit seinem Namen aufgerufen |
| die eigenen Diagnosen von CLI11 — ein fehlendes Pflichtargument, eine unbekannte Option, ein Wert, den eine Option nicht annimmt — und sein `Run with --help for more information.` | im Parser formuliert, auf dem die Befehlszeile aufbaut; sie bleiben englisch |

Englisch verlangen, unabhängig von der Umgebung:

<!-- example: LC_ALL=C LANGUAGE=fr subedit-cli shift --output-dir out a.srt; echo "code=$?" -->
```console
$ LC_ALL=C LANGUAGE=fr subedit-cli shift --output-dir out a.srt; echo "code=$?"
shift needs --by, or --to-grid to work the amount out from the positions
code=1
```

## Globale Optionen

**Sie werden vor den Namen des Unterbefehls geschrieben, und nur dorthin.**
`subedit-cli --encoding cp1252 inspect film.srt` ist die richtige Form;
`subedit-cli inspect --encoding cp1252 film.srt` wird abgelehnt, mit
`The following argument was not expected: --encoding`. Dasselbe gilt für
`--version`, `-v` und `--quiet`.

**`--help` ist die Ausnahme, und eigentlich ist es keine**:
`subedit-cli inspect --help` wird akzeptiert, weil jeder Unterbefehl seine eigene Hilfe hat, und es ist die
Hilfe des Unterbefehls, die ausgegeben wird, nicht die des Werkzeugs.

| Option | Wirkung |
| :----- | :---- |
| `-h`, `--help` | gibt die Hilfe aus und beendet sich; die des Unterbefehls, wenn einer genannt ist |
| `--version` | gibt `subedit <version>`, die Bibliotheken, mit denen das Werkzeug läuft, und die installierten Sprachen aus und beendet sich — siehe [`--version`](#--version) |
| `-q`, `--quiet` | Stufe 0 — überhaupt keine Verlaufsmeldungen |
| `-v`, `-vv`, `-vvv` | Stufen 1 bis 3; Stufe 1 ist der Standard |
| `--encoding NAME` | liest die Dateien in dieser Kodierung, statt sie zu erraten |
| `--list-encodings` | gibt die Kodierungen aus, die das Werkzeug lesen und schreiben kann, eine pro Zeile, und **beendet sich** (Code `0`) — siehe [`--list-encodings`](#--list-encodings) |
| `--format text\|json` | die Form des Ergebnisses auf der Standardausgabe: der Text oder ein JSON-Objekt pro Datei — siehe [Ausgabe, die ein Skript lesen kann](#ausgabe-die-ein-skript-lesen-kann) |
| `--config FILE` | liest die Einstellungen der Operationen aus `FILE` statt aus der Einstellungsdatei des Benutzers — siehe [Die Einstellungsdatei](#die-einstellungsdatei) |
| `--no-config` | liest keine Einstellungsdatei: Jede Einstellung einer Operation, die nicht auf der Zeile angegeben ist, nimmt ihren eingebauten Standardwert an — siehe [Die Einstellungsdatei](#die-einstellungsdatei) |

`--quiet` und `-v` im selben Aufruf werden abgelehnt: Zwei gegensätzliche Absichten
werden nicht zugunsten der zuletzt geschriebenen entschieden.

`--config` und `--no-config` im selben Aufruf werden ebenfalls abgelehnt, mit
`--config excludes --no-config`, ebenso `--config` mit einem leeren Namen, mit
`--config needs a file name`: Ein leerer Name benennt keine Datei, und die eigene Datei des Benutzers wird nicht an
ihrer Stelle gelesen.

### Die Einstellungsdatei

**[`adjust`](adjust.md) und [`correct`](correct.md) lesen standardmäßig die Einstellungen der Operationen aus der
Einstellungsdatei** — die Werte, die sich `Adjust Durations…` und `Correct Texts…` im
Fenster merken oder die [`config set`](config.md) geschrieben hat. Kein anderer Unterbefehl liest sie. Die Datei ist:

```
$XDG_CONFIG_HOME/subedit/settings.conf
```

oder `~/.config/subedit/settings.conf`, wenn `XDG_CONFIG_HOME` nicht gesetzt, leer oder kein absoluter
Pfad ist — die Datei des Fensters, beschrieben in [seinem Handbuch](../subedit-gui/preferences.md#die-datei).

**Vorrang, der stärkste zuerst: die Option auf der Befehlszeile, dann die Datei, dann der eingebaute
Standardwert.** Eine Option gewinnt immer: `--speed off` schaltet eine Geschwindigkeit aus, die die Datei setzt. Nur der
**Operationsblock** der Datei wird gelesen; die Anzeigeeinstellungen (Geometrie, Spalten, Design …)
niemals. [`config list`](config.md#config-list) nennt die Schlüssel und welche davon die Befehlszeile
liest: die neun Schlüssel `duration-adjust.*`, `correction.human`, `correction.ocr`,
`correction.remove-blank` und `correction.line-break.max-lines`. **Jeder von ihnen hat eine Option,
die ihn für einen Aufruf überschreibt**, wie auch immer die Datei ihn setzt — die Tabellen von
[`adjust`](adjust.md#was-die-einstellungsdatei-liefert) und
[`correct`](correct.md#was-gewählt-wird-und-was-nicht) nennen sie.

**Das Lesen der Datei des Benutzers lässt keinen Befehl scheitern.** Eine fehlende Datei ergibt die Standardwerte; eine Datei,
die nicht gelesen werden kann, ergibt die Standardwerte und meldet es; ein Wert, der nicht gelesen werden kann, behält seinen Standardwert und wird genannt; ein
unbekannter Schlüssel wird ignoriert. Die Datei wird niemals von `adjust` oder `correct` geschrieben.

**Was gelesen wurde, wird gemeldet**, auf der Standardstufe 1, auf der Standardfehlerausgabe, vor der ersten Datei:

| Fall | Zeile |
| :--- | :--- |
| die Datei wurde gelesen | `settings: read <path>` |
| ein Wert darin konnte nicht gelesen werden | `settings: <key>: cannot read '<value>', default kept` |
| die Datei des Benutzers existiert nicht | `settings: <path> not found, defaults used` |
| die Datei kann nicht gelesen werden | `settings: <path> unreadable, defaults used` |
| `--no-config` | nichts |

Mit [`--format json`](#ausgabe-die-ein-skript-lesen-kann) enthält das Objekt jeder Datei von `adjust` und
`correct` den Schlüssel `settings`: den gelesenen Pfad oder `null`, wenn keiner gelesen wurde.


**`--no-config` macht einen Lauf reproduzierbar**: Dieselbe Zeile ergibt auf jedem
Rechner dasselbe Ergebnis, was auch immer sein Benutzer im Fenster eingestellt hat. Verwenden Sie es in Skripten, Tests und Fehlerberichten — oder
`--config FILE` mit einer Datei, die sie begleitet.

**`--config FILE` benennt eine andere Datei**, die auf dieselbe Weise gelesen wird — **nur muss sie existieren**. Ein
auf der Befehlszeile angegebener Name ist keine Bitte um die Standardwerte: Wenn `FILE` nicht existiert,
halten `adjust` und `correct` an, bevor irgendeine Eingabe gelesen wird, schreiben `settings file not found: <FILE>` auf
die Standardfehlerausgabe und beenden sich mit dem Code `1` für Aufruffehler. Ein `FILE`, das existiert, aber nicht gelesen werden kann,
ergibt die Standardwerte und meldet es, wie es die Datei des Benutzers tut. [`config show`](config.md#config-show)
zeigt ein fehlendes `FILE` trotzdem als `(not found)` an, und [`config set`](config.md#config-set-und-config-unset) legt es
trotzdem an.

<!-- example: printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine.conf adjust --dry-run a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine.conf adjust --dry-run a.srt; echo "code=$?"
settings file not found: mine.conf
code=1
```

### `--version`

**Die Version des Programms, allein in der ersten Zeile** — `subedit <version>`, was die Ausgabe bis
0.15.38 war und was ein Skript liest. Dann **eine Zeile pro Bibliothek, mit der das Werkzeug läuft**, dann die installierten
Sprachen der Oberfläche, auf der Standardausgabe; das Werkzeug beendet sich mit Code `0`. Ein zusätzlich angegebener Unterbefehl wird
nicht ausgeführt, und `-q` und `-v` ändern nichts.

<!-- example: subedit-cli --version | head -n 1 -->
```console
$ subedit-cli --version | head -n 1
subedit 1.0.0
```

Die vollständige Ausgabe, auf einem Rechner mit ICU 74.2 und Enchant 2.3.3 — die Nummern sind die des Rechners:

```console
$ subedit-cli --version
subedit <version>
  ICU      74.2 (built with 74.2)
  Enchant  2.3.3 (built with 2.3.3)
  CLI11    2.4.1 (built with 2.4.1)
  Languages: en, fr
```

| Bibliothek | Wofür das Werkzeug sie verwendet |
| :------ | :------------------------ |
| `ICU` | Lesen und Schreiben der Kodierungen, ihre Erkennung, Ändern der Groß-/Kleinschreibung von Buchstaben |
| `Enchant` | die Rechtschreibprüfung von `correct` |
| `CLI11` | Lesen dieser Befehlszeile |

- **Jede Zeile nennt zwei Versionen**: zuerst die zur Laufzeit gefundene und in Klammern die, gegen die das Programm
  kompiliert wurde. Sie unterscheiden sich, wenn die Bibliothek des Rechners aktualisiert wurde, nachdem das
  Programm gebaut wurde — die erste Angabe für einen Fehlerbericht.
- **Die Header von Enchant tragen keine Version**: Die in Klammern ist die, die `pkg-config` gemeldet hat,
  als der Build konfiguriert wurde.
- **CLI11 ist in das Programm hineinkompiliert**, da es nur aus seinen Headern besteht: Seine beiden Versionen sind
  immer gleich.
- **Weder Qt noch libmpv**: Die Befehlszeile linkt keines von beiden und listet nur auf, womit sie läuft.
  Das Fenster listet sie auf — siehe [sein Handbuch](../subedit-gui/invocation.md#--version).
- **`Languages`** listet `en` auf, die Ausgangssprache, die keinen Katalog braucht, dann jeden unter
  `<prefix>/share/subedit/locale` installierten Katalog, der sich lesen lässt, nach Code — die Sprachen, die
  `Bearbeiten ▸ Einstellungen …` anbietet.
- **Was der Sprache folgt**: `built with` und `Languages`. Die erste Zeile, die Namen, die
  Nummern und die Sprachcodes niemals.
- **Mit `--format json`**, vor oder nach `--version`, **ein Objekt in einer Zeile**: `schema`
  (`1`), `command` (`"version"`), `version`, `libraries` — ein Array von
  `{"name":"ICU","loaded":"74.2","built":"74.2"}` in der Reihenfolge des Textes — und `languages`, ein
  Array von Codes. Nichts darin wird übersetzt.

<!-- example: subedit-cli --version --format json | grep -o '"name":"[^"]*"' -->
```console
$ subedit-cli --version --format json | grep -o '"name":"[^"]*"'
"name":"ICU"
"name":"Enchant"
"name":"CLI11"
```

### `--list-encodings`

**Die Kodierungen, die das Werkzeug lesen und schreiben kann, eine pro Zeile, nach Namen sortiert**, auf der Standardausgabe —
dann beendet sich das Werkzeug mit Code `0`, wie bei `--version`: Das ist die Antwort, kein Zwischenschritt. Ein
zusätzlich angegebener Unterbefehl wird nicht ausgeführt.

<!-- example: subedit-cli --list-encodings | grep '^UTF' | head -4 -->
```console
$ subedit-cli --list-encodings | grep '^UTF' | head -4
UTF-16BE
UTF-16LE
UTF-32BE
UTF-32LE
```

- **Es sind die Namen, die `--encoding` akzeptiert**, die kanonischen; ihre Aliasse (`cp1252`) werden ebenfalls akzeptiert,
  ohne aufgelistet zu werden.
- **Die Anzahl hängt vom installierten ICU ab**, und kein Name wird über die hinaus versprochen, die jedes ICU hat
  (`UTF-8`, `UTF-16LE`, `windows-1252` …). Das Handbuch nennt deshalb keine Anzahl.
- **`UTF-16` und `UTF-32` sind nicht darin**: ICU kennt sie, aber ihr Konverter schreibt eine eigene Marke,
  und das Modell lehnt sie ab — siehe unten. Eine Auswahlliste bietet nicht an, was das nächste Feld
  ablehnen würde.
- **Mit `--format json`**, **ein Objekt pro Kodierung**:
  `{"schema":1,"command":"list-encodings","name":"UTF-8"}`. Es ist nicht der Bericht über eine Datei: Diese
  Objekte **enthalten weder `file` noch `ok`**, die einzige Ausnahme von der Hülle aus
  [der Form](#die-form). Es bleibt ein Objekt pro Zeile, eines pro Eintrag.
- **Die Ausführlichkeitsstufe ändert nichts**: Eine Liste ist ein Ergebnis, und `-q` nimmt sie nicht weg.

### `--encoding` und was es nicht erzwingen kann

**Es gilt für jeden Unterbefehl**, denn alle lesen: Eine Datei, deren
Kodierung falsch erraten wird, bleibt falsch, was auch immer mit ihr geschieht, und eine Option,
die nur `inspect` hätte, ließe eine Fehldeutung ohne Abhilfe.

**Der Name ist der von ICU, und alle seine Aliasse werden akzeptiert**: `cp1252`,
`windows-1252` und `WINDOWS 1252` benennen dieselbe Kodierung. Die Menge ist nirgends
festgeschrieben: Es ist die, die das ICU des Rechners umwandeln kann, und
[`--list-encodings`](#--list-encodings) listet sie auf. Ein Name, den es nicht umwandeln kann, wird
abgelehnt, bevor eine einzige Datei gelesen wird:

```console
$ subedit-cli --encoding klingon-1 inspect film.srt
no encoding is named "klingon-1"
```

**Eine Ausnahme, und nur eine: Die Kodierung muss ihre Byte-Reihenfolge nennen.**
`UTF-16` und `UTF-32` sind Namen, die ICU kennt und deren Konverter **eine eigene
Marke** schreibt. Eine solche Kodierung hat in diesem Modell keinen Platz — die Marke wäre
keine Wahl mehr, `--no-bom` würde akzeptiert und missachtet —, daher werden beide
abgelehnt, beim Lesen wie beim Schreiben:

```console
$ subedit-cli --encoding UTF-16 inspect film.srt
"UTF-16" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do
```

`UTF-16LE` und `UTF-16BE` lesen und schreiben dieselben Bytes, Marke eingeschlossen; was
der genaue Name hinzufügt, ist, dass man weiß, welche.

**Eine Byte-Reihenfolge-Marke hat Vorrang vor der Option.** Sie ist das Einzige, was eine Untertiteldatei
über sich selbst angibt, und sie anders zu lesen, als sie sich selbst angibt,
hieße, dem Aufrufer gegen die Datei zu gehorchen. Die Abweichung wird **gemeldet**, als
Diagnose:

```console
$ subedit-cli -vvv --encoding cp1252 inspect avec-bom.srt
avec-bom.srt: a byte order mark that contradicts the encoding asked for ("UTF-16LE"), settled by the reader
```

Das tut auch Gaupol — es beginnt das Lesen mit der Kodierung der
Marke neu —, aber ohne es zu sagen; hier ist der Unterschied zwischen dem Verlangten und
dem Getanen kein Schweigen.

## Unterbefehle

| Unterbefehl | Was er tut | Schreibt er? |
| :------------ | :-------------- | :----------- |
| [`inspect`](inspect.md) | berichtet, woraus eine Datei besteht, ohne etwas zu ändern | nein |
| [`convert`](convert.md) | schreibt eine Datei in einem anderen Format oder einer anderen Form | ja |
| [`shift`](shift.md) | verschiebt jede Position um denselben Betrag | ja |
| [`transform`](transform.md) | korrigiert jede Position anhand zweier Punkte, die bekanntermaßen stimmen | ja |
| [`framerate`](framerate.md) | rechnet eine Datei von einer Bildrate auf eine andere um | ja |
| [`snap`](snap.md) | setzt jeden Zeitstempel auf das nächstgelegene Bild | ja |
| [`hearing-impaired`](hearing-impaired.md) | entfernt die Hinweise für Hörgeschädigte | ja |
| [`adjust`](adjust.md) | passt die Dauern an eine Lesegeschwindigkeit und an Grenzen an | ja |
| [`replace`](replace.md) | ersetzt einen Text in den Untertiteln, ohne ein Tag zu zerstören | ja |
| [`case`](case.md) | setzt die Texte in Titel-, Satz-, Groß- oder Kleinschreibung | ja |
| [`italics`](italics.md) | setzt die Texte kursiv oder entfernt ihre Kursivschrift | ja |
| [`dialogue-dashes`](dialogue-dashes.md) | setzt Dialogstriche oder entfernt sie | ja |
| [`sort`](sort.md) | ordnet die Untertitel wieder nach ihrem Beginn | ja |
| [`join-repeats`](join-repeats.md) | macht aus den aufeinanderfolgenden Stücken eines Textes einen einzigen Untertitel | ja |
| [`disjoint`](disjoint.md) | schneidet die überlappenden Untertitel so, dass sich kein Bereich mehr mit einem anderen überschneidet | ja |
| [`repair`](repair.md) | verkettet `join-repeats`, `sort` und `disjoint` | ja |
| [`correct`](correct.md) | korrigiert die Texte mit den Mustern von Gaupol: Hinweise, Wörter zusammenfügen und trennen, häufige Fehler, Großschreibung, Zeilenumbruch | ja |
| [`append`](append.md) | setzt Dateien hinter die erste, in **eine einzige** Datei | ja |
| [`split-file`](split-file.md) | schneidet eine Datei in **zwei** Teile: einen Anfang und einen Rest | ja |
| [`pair`](pair.md) | schreibt eine Übersetzung an die Positionen ihrer Hauptdatei und berichtet, wie ihre Zeilen zugeordnet wurden | ja |
| [`config`](config.md) | listet, zeigt und ändert die Einstellungen der Operationen in der Einstellungsdatei | nur die Einstellungsdatei |

Alle einundzwanzig sind da; die Hilfe des Werkzeugs listet sie in derselben Reihenfolge auf.

**Nur einer berührt nichts**, und das ist die rechte Spalte: `inspect` liest und
berichtet. Die anderen neunzehn schreiben, und was folgt, gilt für siebzehn von ihnen:
[`append`](append.md) und [`split-file`](split-file.md) haben jeweils ihre eigene Stelligkeit — N Dateien für
**ein** Ergebnis, eine Datei für **zwei** — und daher ihr eigenes Ziel, das sie auf ihrer Seite beschreiben.
[`config`](config.md) schreibt keine Untertiteldatei: Es liest und ändert die Einstellungsdatei, und
nichts von dem, was folgt, gilt dafür.

## Das Ziel

Die siebzehn Unterbefehle, die schreiben — [`convert`](convert.md),
[`shift`](shift.md), [`transform`](transform.md), [`framerate`](framerate.md),
[`snap`](snap.md), [`hearing-impaired`](hearing-impaired.md), [`adjust`](adjust.md),
[`replace`](replace.md), [`case`](case.md), [`italics`](italics.md),
[`dialogue-dashes`](dialogue-dashes.md), [`sort`](sort.md), [`join-repeats`](join-repeats.md),
[`disjoint`](disjoint.md), [`repair`](repair.md), [`correct`](correct.md), [`pair`](pair.md) —, nehmen ihr Ziel auf dieselbe Weise an. ([`append`](append.md) und
[`split-file`](split-file.md) haben ihr eigenes, auf ihrer Seite.) [`inspect`](inspect.md) schreibt keine Datei und
akzeptiert keine dieser Optionen.

**Ein Ziel, das bereits existiert, wird überschrieben**, ohne Rückfrage und ohne Option, um
das zu vermeiden: Das Schreiben ist atomar — die Datei wird niemals halb geschrieben, und ein
unterbrochenes Schreiben lässt die alte unversehrt —, aber der vorherige Inhalt geht
verloren. Das ist kein beibehaltener Mangel: Es ist die Regel, und einen Befehl
erneut auf dasselbe Ausgabeverzeichnis auszuführen, führt ihn erneut über sein eigenes Ergebnis aus.

**Ohne ausdrückliches Ziel wird niemals etwas geschrieben.** Die drei Arten,
es anzugeben, schließen einander aus, und eine ist nötig: Ohne sie wird nichts geschrieben, und der Exit-Code
ist `1` — außer mit [`--dry-run`](#vor-dem-schreiben-nachsehen), das nichts schreibt
und daher keines verlangt.

| Option | Wohin das Ergebnis geht |
| :----- | :---------------- |
| `--output FILE` | in diese Datei, unter genau diesem Namen; eine einzige Eingabe |
| `--output-dir DIR` | in dieses Verzeichnis, unter dem Basisnamen der Eingabe |
| `--in-place` | über die Eingabe, durch atomares Schreiben |

Ein Werkzeug, das seine Eingabe überschreibt, weil man ihm nichts gesagt hat, ist ein Werkzeug, das man
beim zweiten Mal nicht mehr benutzt. Die Ablehnung kostet eine Zeile und rettet die
Datei.

`--output` mit mehreren Eingaben wird abgelehnt: Die letzte Datei würde über die
vorherigen geschrieben. Bei einem Stapel ist `--output-dir` die einzige der drei Optionen, die
sinnvoll ist, neben `--in-place`.

**Das Ausgabeverzeichnis wird angelegt**, wenn es fehlt, übergeordnete Verzeichnisse eingeschlossen — für
`--output-dir` und für das Verzeichnis, in das `--output` schreibt. Das geschieht, sobald die
Befehlszeile geprüft ist, und **vor der ersten Datei**. Wenn es nicht möglich ist,
wird das einmal gemeldet, keine Datei wird gelesen, und der Exit-Code ist
`2`.

**Der Stapel wird vor dem Schreiben als Ganzes beurteilt.** Das Ziel jeder Eingabe
wird zuerst ermittelt, und daraus folgen zwei Ablehnungen — Aufruffehler, Code
`1`, **keine Datei geschrieben** und kein Verzeichnis angelegt:

- **zwei Eingaben, die in derselben Datei landen würden** — `a/film.srt` und
  `b/film.srt` mit `--output-dir`, oder `a/film.srt` und `b/film.vtt`, beide
  nach WebVTT konvertiert, deren endgültiges Ziel die Endung gewechselt hat. Die
  Meldung nennt das Ziel und beide Eingaben; die zweite umzubenennen oder
  die letzte gewinnen zu lassen, würde stillschweigend eine Datei verlieren, und die Ablehnung
  kostet eine Zeile;
- **ein Ziel, das eine der Eingaben ist**, ohne `--in-place` — auch
  über `--output-dir .`, über `--output` oder über einen symbolischen Link. Über die eigene
  Eingabe zu schreiben ist ein Vorgang, den man ausdrücklich benennt.

Zwei Pfade benennen dieselbe Datei, wenn das System es sagt, nicht wenn sie
gleich geschrieben sind: `in/../in/film.srt` und `in/film.srt` sind eine einzige Datei.

**Die Endung folgt dem geschriebenen Format.** Sie ändert sich nur bei
[`convert`](convert.md), dem einzigen Unterbefehl, der das Format ändert; die
anderen behalten das Format der gelesenen Datei und damit ihre Endung.

| Was sie auslöst | Meldung |
| :------------------ | :------ |
| kein Ziel, ohne `--dry-run` | `no destination given: use --output, --output-dir or --in-place` |
| zwei Ziele | `--output, --output-dir and --in-place exclude one another` |
| `--output` bei einem Stapel | `--output names one file but several were given: use --output-dir instead` |
| zwei Eingaben, ein Ziel | `<destination>: would be written by both <input> and <input>` |
| eine Eingabe würde überschrieben | `<destination>: would be written over the input <input>` — derselbe Satz bei [`append`](append.md) und [`split-file`](split-file.md) |
| Ausgabeverzeichnis, das nicht angelegt werden kann | `<directory>: cannot be created: permission denied` oder `: cannot be created` |
| Ziel, das nicht geschrieben werden kann | `<path>: <destination>: cannot be written: permission denied` oder `: cannot be written` |
| Zeichen, das in der geschriebenen Kodierung fehlt | `<path>: holds a character the chosen encoding cannot write` |

**Die letzte verdient einen Satz.** Eine Datei wird in **der Kodierung zurückgeschrieben, in der sie
gelesen wurde**, wie auch mit ihren Zeilenenden und ihrer Marke. Wenn eine
Änderung ein Zeichen in sie gebracht hat, das diese Kodierung nicht kennt — ein
`ł` in Latin-1 —, scheitert das Schreiben, und **nichts wird geschrieben**. Es
durch ein `?` zu ersetzen, hieße, Text vor den Augen dessen zu verlieren, der ihn gerade geschrieben hat.

## Vor dem Schreiben nachsehen

**`--dry-run`** wird von jedem der sechzehn Unterbefehle akzeptiert, die schreiben, und bedeutet überall
dasselbe: **lesen, berechnen, berichten, keine Datei schreiben**. Man fügt es der
Zeile hinzu, die man gerade ausführen wollte; anwenden heißt, dieselbe Zeile ohne es erneut auszuführen. Die Berechnung ist
deterministisch: Was ein `--dry-run` gezeigt hat, ist das, was der nächste Lauf schreiben wird, solange sich die
Datei nicht geändert hat.

- **Er schreibt keine Datei und legt kein Verzeichnis an**, `--in-place` eingeschlossen: Die Eingabedatei
  bleibt, wie sie war.
- **Er verlangt kein Ziel.** Eines kann angegeben werden, und **es wird dann geprüft wie ohne
  `--dry-run`**: zwei Eingaben für dasselbe Ziel, eine Eingabe, die überschrieben würde,
  `--output` bei einem Stapel, zwei Angaben darüber, wohin — eine Befehlszeile, die ohne `--dry-run` abgelehnt wird,
  wird auch mit ihm abgelehnt, mit demselben Satz und demselben Code `1`.
- **Der Exit-Code ist der eines echten Laufs**: `0`, wenn alles berechnet wurde, `2` oder `3`,
  wenn Dateien nicht gelesen oder verarbeitet werden konnten, `1` bei Aufruffehlern. Ein Zeichen, das
  die Kodierung der Datei nicht schreiben könnte, lässt einen Probelauf scheitern, wie es einen echten scheitern lässt.
  **Dass es Änderungen gibt, ändert den Code nicht**: Die vier Codes sagen, ob das Werkzeug
  erfolgreich war, nicht, ob die Datei den Anforderungen entspricht.
- **Die Verlaufsmeldungen sagen es.** Wo ein echter Lauf `-> <destination>` schreibt, schreibt ein
  Probelauf `(dry run, nothing written)`.

<!-- example: printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by 1 --dry-run a.srt; ls -- *.srt -->
```console
$ printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by 1 --dry-run a.srt; ls -- *.srt
a.srt: 1 subtitle shifted by 1.000 s (dry run, nothing written)
a.srt
```

**Bei einem Unterbefehl, der Texte ändert** —
[`hearing-impaired`](hearing-impaired.md), [`replace`](replace.md), [`case`](case.md),
[`italics`](italics.md), [`dialogue-dashes`](dialogue-dashes.md) und [`correct`](correct.md) —, **enthält die Standardausgabe die vorgeschlagenen
Änderungen**. Ein Block pro geändertem Untertitel, in der Reihenfolge der Datei:

```
<path>: subtitle <number>
- <the text before, line by line>
+ <the text after, line by line>
```

- die **Nummer** ist die, die der Untertitel in der gelesenen Datei hat, ab eins gezählt;
- jede Zeile des Textes davor trägt `- ` am Anfang, jede Zeile des Textes danach `+ `: Ein Text
  aus mehreren Zeilen liest sich, wie er ist, und eine Zeile, die selbst mit einem Dialogstrich
  beginnt, ergibt `- - `;
- **ein Untertitel, den die Operation entfernen würde**, sagt das in seiner ersten Zeile,
  `<path>: subtitle <number> (removed)`, und hat keine `+`-Zeilen;
- ein Untertitel der Übersetzung kündigt sich mit `--document translation` an derselben Stelle mit `(translation)`
  an — siehe [Eine Übersetzung](#eine-übersetzung);
- die erste Zeile jedes Blocks beginnt mit dem Pfad, sodass `grep '^film.srt: '`
  in einem Stapel die Blöcke einer Datei findet;
- **kein Block**, wenn sich nichts ändern würde.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\n- [Grincement]\n- Qui est là ?\n\n' > mentions.srt; subedit-cli hearing-impaired --dry-run mentions.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\n- [Grincement]\n- Qui est là ?\n\n' > mentions.srt; subedit-cli hearing-impaired --dry-run mentions.srt
mentions.srt: 2 subtitles cleaned, 1 removed (dry run, nothing written)
mentions.srt: subtitle 1 (removed)
- [Bruit de pas]
mentions.srt: subtitle 2
- Attends [il tousse] Marie.
+ Attends Marie.
mentions.srt: subtitle 3
- - [Grincement]
- - Qui est là ?
+ Qui est là ?
```

**Die Liste ist in JSON dieselbe**: Das Objekt der Datei enthält ein Array `changes`, siehe
[die Form](#die-form), mit `after` auf `null` für einen entfernten Untertitel. **Sie wird nur
berechnet, wenn `--dry-run` oder `--format json` sie verlangt**: Ein Stapel aus Tausenden von
Untertiteln baut sie nicht umsonst auf.

Bei **den anderen Unterbefehlen** — [`convert`](convert.md), [`shift`](shift.md),
[`transform`](transform.md), [`framerate`](framerate.md), [`snap`](snap.md),
[`adjust`](adjust.md), [`sort`](sort.md), [`append`](append.md), [`split-file`](split-file.md), [`pair`](pair.md) — enthält die Standardausgabe
das, was die eines echten Laufs enthalten würde, also nichts im Text; in
JSON das Objekt jeder Datei mit `"dry_run":true`, `"destination":null` und denselben
`counts`.

## Zwei Ausgaben, zwei Rollen

| Ausgabe | Was sie enthält |
| :----- | :--------------- |
| Standard | **das Ergebnis, und nur das Ergebnis** — den Bericht von `inspect`, die vorgeschlagenen Änderungen eines Text-[`--dry-run`](#vor-dem-schreiben-nachsehen); mit `--format json` ein Objekt pro Datei — eines pro Lauf bei [`append`](append.md), [`split-file`](split-file.md) und [`pair`](pair.md) |
| Fehler | **alles andere** — die Verlaufsmeldungen, die Warnungen, die Fehler |

Diese Aufteilung erlaubt es, das Ergebnis umzuleiten, ohne die Verlaufsmeldungen mit hineinzubekommen:

```console
$ subedit-cli inspect *.srt > rapport.txt
```

Der Bericht geht in die Datei, die Verlaufsmeldungen bleiben auf dem Bildschirm.

## Ausgabe, die ein Skript lesen kann

**`--format json`** ersetzt den Text der Standardausgabe durch **ein JSON-Objekt pro
Eingabedatei**, im Format JSON Lines: ein kompaktes Objekt pro Zeile, in UTF-8 ohne
Marke, das Zeilenende ist `\n`. `text` ist der Standard und ändert sich nicht. Es ist eine
globale Option und wird daher vor den Unterbefehl geschrieben; ein anderer Wert als
`text` und `json` ist ein Aufruffehler, Code `1`.

<!-- example: printf '1\n00:00:10,000 --> 00:00:12,000\nUn.\n\n' > a.srt; subedit-cli --format json shift --by 1 --output-dir sortie a.srt absent.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:10,000 --> 00:00:12,000\nUn.\n\n' > a.srt; subedit-cli --format json shift --by 1 --output-dir sortie a.srt absent.srt; echo "code=$?"
a.srt: 1 subtitle shifted by 1.000 s -> sortie/a.srt
{"schema":1,"command":"shift","file":"a.srt","ok":true,"dry_run":false,"destination":"sortie/a.srt","counts":{"subtitles":1,"shifted_by_ms":1000},"warnings":[]}
absent.srt: does not exist
{"schema":1,"command":"shift","file":"absent.srt","ok":false,"error":{"kind":"not-found","message":"absent.srt: does not exist"}}
1 of 2 files shifted, 1 failed
code=3
```

**Ein Objekt pro Eingabe, niemals keines, niemals zwei**, ob die Eingabe erfolgreich war oder nicht, in
der Reihenfolge, in der die Eingaben angegeben — oder mit [`--recursive`](lots.md) gefunden — wurden. Man
zählt die Zeilen, man findet seine Dateien wieder. **Drei Unterbefehle sind die Ausnahme, weil ihre
Stelligkeit nicht die eines Stapels ist: [`append`](append.md) (N Eingaben, eine Ausgabe), [`split-file`](split-file.md)
(eine Eingabe, zwei Ausgaben) und [`pair`](pair.md) (zwei Dateien, eine Ausgabe) schreiben **ein Objekt für den ganzen
Lauf**, das die Datei nennt, die ihre Seite angibt.** **Die Unterbefehle, die umschreiben, schreiben daher auch
auf die Standardausgabe**, was ihr Vertrag im Text nicht tat: Ihre
Datei bleibt ihr Ergebnis, und das Objekt ist der Bericht darüber.

**Die Verlaufsmeldungen ändern sich nicht.** `-q`, `-v`, `-vv` und `-vvv` wirken auf die
Fehlerausgabe, im Text, wie ohne `--format`; **die Standardausgabe hängt nicht von ihnen ab**.
Die Diagnosen beim Lesen, die der Text auf Stufe 3 hält, sind Daten und stehen
**immer** in `warnings`. Ein Aufruffehler schreibt kein Objekt: Er wird erkannt,
bevor irgendeine Datei berührt wird, es gibt also nichts, dem man ihn zuordnen könnte; er geht auf die
Fehlerausgabe mit Code `1`, und die Standardausgabe bleibt leer.

### Die Form

| Schlüssel | Inhalt | Vorhanden |
| :-- | :------ | :------- |
| `schema` | Ganzzahl, die Version dieser Form — `1` | immer |
| `command` | der Name des Unterbefehls | immer |
| `file` | der Pfad **wie angegeben** oder wie der Durchlauf eines Verzeichnisses ihn zusammengesetzt hat | immer |
| `ok` | Boolescher Wert | immer |
| `error` | `{"kind", "message"}` | wenn `ok` falsch ist |
| `dry_run` | Boolescher Wert: wahr bei einem [`--dry-run`](#vor-dem-schreiben-nachsehen) | wenn `ok` wahr ist, bei einem Unterbefehl, der schreibt |
| `destination` | der geschriebene Pfad oder `null`, wenn `dry_run` wahr ist: Es wurde nichts geschrieben | ebenso |
| `counts` | ein Objekt aus **Ganzzahlen**, spezifisch für den Unterbefehl | ebenso |
| `changes` | ein Array von `{"subtitle", "document", "before", "after"}`: die geänderten Texte, siehe unten | bei einem Unterbefehl, der Texte ändert |
| `warnings` | ein Array von `{"kind", "line"?, "detail"?, "settled"?}` | wenn `ok` wahr ist |
| `alignment` | wie eine Übersetzung aufgelegt wurde, siehe [Eine Übersetzung](#eine-übersetzung) | mit `-t` und bei `pair` |
| `constraints` | die verwendeten Einstellungen | `adjust` |
| `settings` | die gelesene Einstellungsdatei oder `null`, wenn keine gelesen wurde — siehe [Die Einstellungsdatei](#die-einstellungsdatei) | `adjust`, `correct`, wenn `ok` wahr ist |
| `inputs`, `tail` | ein Element pro angehängter Datei; der Pfad des Rests | `append`; `split-file` |

`line` fehlt bei einer Diagnose über die ganze Datei; `detail` fehlt, wenn es
nichts hinzuzufügen gibt; `settled` ist `true`, wenn das Lesen etwas **bereinigt** hat — eine
neu erzeugte Nummerierung —, und fehlt, wenn es die Dinge gelassen hat, wie sie sind.

**`changes`** listet in der Reihenfolge der Datei jeden Untertitel auf, dessen Text sich ändert:
`subtitle` ist seine Nummer in der gelesenen Datei, ab eins gezählt; `document` ist `"main"`
oder `"translation"`; `before` ist der Text davor; `after` der Text danach oder **`null`, wenn
der Untertitel entfernt wird**. Die Zeilenumbrüche eines Textes sind darin `\n`. Es wird
bei einem echten Lauf wie bei einem [`--dry-run`](#vor-dem-schreiben-nachsehen) geschrieben und ist **leer
(`[]`), wenn sich nichts ändert**: Sein Vorhandensein sagt, dass der Unterbefehl seine Änderungen auflistet,
sein Fehlen, dass er keine hat.

**Keine Gleitkommazahl, nirgends.** Eine Position oder eine Dauer ist eine Ganzzahl von
**Millisekunden** (`…_ms`); eine Anzahl ist eine Ganzzahl; eine Bildrate ist eine **Zeichenkette**
(`"25"`, `"24000/1001"`), weil sie exakt ist, was eine Gleitkommazahl bei einem Bruch nicht
leisten kann; eine Rasterkonzentration ist eine Ganzzahl in **Tausendsteln** — `1000` ist ein
perfektes Raster. Zwei Ausgaben lassen sich dann Byte für Byte vergleichen, ohne Rundung.

**Ein Pfad, der nicht UTF-8 ist** — Linux erlaubt das —, lässt sich in JSON nicht darstellen: Er
wird mit U+FFFD anstelle der ungültigen Bytes geschrieben, und das Objekt enthält die Warnung
`path-not-utf8`. Die Treue eines solchen Pfads wird nicht versprochen, und das wird gesagt.

### Was jeder Unterbefehl in `counts` angibt

| Unterbefehl | Schlüssel von `counts` |
| :------------ | :--------------- |
| [`shift`](shift.md) | `subtitles`, `shifted_by_ms` (mit Vorzeichen) |
| [`transform`](transform.md) | `subtitles` |
| [`framerate`](framerate.md) | `subtitles` |
| [`snap`](snap.md) | `subtitles`, `moved` (verschobene Positionen), `furthest_ms` (die größte Verschiebung) |
| [`hearing-impaired`](hearing-impaired.md) | `cleaned` (umgeschriebene Texte), `removed` (entfernte Untertitel) |
| [`replace`](replace.md) | `replaced` (ersetzte Treffer in den geänderten Untertiteln), `matched` (gefundene Treffer) |
| [`case`](case.md), [`italics`](italics.md), [`dialogue-dashes`](dialogue-dashes.md) | `changed` (Untertitel, deren Text sich geändert hat) |
| [`correct`](correct.md) | `corrected` (geänderte Texte), `removed` (entfernte Untertitel) — niemals Treffer |
| [`sort`](sort.md) | `subtitles`, `moved` (die Plätze, die den Untertitel gewechselt haben) |
| [`adjust`](adjust.md) | `subtitles` (die anvisierten Untertitel), `adjusted` (die, deren Ende sich verschoben hat), dann die Gruppe `sacrificed`: `speed`, `minimum`, `gap`; und, außerhalb von `counts`, `constraints` — die verwendeten Einstellungen |
| [`convert`](convert.md) | `subtitles`, dann was die Konvertierung verloren hat: `lost_ends` und `lost_header` (0 oder 1), `joined_lines`, `lost_tags`, `lost_fields`, `furthest_ms` |
| [`append`](append.md) | `files`, `subtitles`, `appended`, dann `lost_ends`, `lost_header`, `joined_lines`, `lost_tags`, `lost_fields`, `furthest_ms` aufsummiert; und, außerhalb von `counts`, `inputs` — ein Element pro angehängter Datei |
| [`split-file`](split-file.md) | `subtitles`, `head`, `tail`; und, außerhalb von `counts`, `tail` — der Pfad des Rests (`destination` ist der Anfang) |
| [`pair`](pair.md) | `subtitles` (die der geschriebenen Datei); und, außerhalb von `counts`, `alignment` |
| [`inspect`](inspect.md) | keine `counts`: die Beschreibung der Datei, siehe ihre Seite |

### Fehlerkennungen

Ein `error.kind` ist eine dieser Kennungen; **ein Leser behandelt eine unbekannte Kennung als
„sonstige“**.

| `kind` | Was es ist |
| :----- | :----------- |
| `not-found`, `permission-denied`, `io` | das System hat sich geweigert, die Datei zu lesen — oder zu schreiben |
| `unknown-format`, `no-subtitle-found`, `undecodable` | die Datei wird gelesen und lässt sich nicht lesen |
| `unencodable` | ein Zeichen, das die gewählte Kodierung nicht schreiben kann |
| `no-byte-order-mark`, `no-frame-rate` | was `convert` nicht schreiben kann, ohne dass man es ihm sagt |
| `before-the-origin`, `beyond-the-end`, `no-transform`, `no-grid` | eine Operation, die sich auf diese Datei nicht anwenden lässt |
| `range-out-of-bounds` | ein `--range`, den diese Datei nicht enthält |
| `no-style` | ein Format, das keinen Stil schreibt (TMPlayer, LRC), bei `italics` |
| `refused` | jede andere Ablehnung einer Operation |

### Was versprochen wird und was nicht

**Versprochen, solange `schema` den Wert `1` hat**: die Namen der Schlüssel, ihr Typ und ihre Einheit; die Bedeutung
von `ok`, `file` und `destination`; **ein Objekt pro Eingabe, in der Reihenfolge der Eingaben** — eines pro Lauf bei den drei Unterbefehlen mit eigener Stelligkeit;
die vorhandenen Kennungen `kind`; der Determinismus — gleiche Eingaben, gleiche Argumente,
gleiche Bytes.

**Nicht versprochen**: der **Text der Meldungen** (`message`, `detail`), Englisch für Menschen,
das korrigiert wird, wenn es nötig ist — ein Skript, das ihn vergleicht, ist von vornherein kaputt; die **Reihenfolge
der Schlüssel** in einem Objekt, fest, aber von JSON nicht garantiert; das Vorhandensein von **noch nicht hinzugefügten
Schlüsseln**.

**Ein Leser ignoriert daher einen unbekannten Schlüssel** und behandelt ein unbekanntes `kind` als „sonstige“:
Einen Schlüssel oder eine Kennung hinzuzufügen ändert `schema` nicht. Einen Schlüssel zu entfernen oder umzubenennen,
einen Typ oder eine Einheit zu ändern, eine Kennung zu entfernen erhöht es — das ist ein Bruch,
angekündigt im CHANGELOG und hier.

## Ausführlichkeitsstufen

Jede Stufe **enthält die vorherige**: Eine Stufe höher fügt hinzu, ersetzt
nie.

| Stufe | Wie | Was die Fehlerausgabe enthält |
| :----- | :------ | :------------------------------ |
| 0 | `--quiet` | nichts, **außer den Fehlern** |
| 1 | standardmäßig oder `-v` | eine Zeile pro verarbeiteter Datei und eine Zusammenfassung, sobald es mehrere sind |
| 2 | `-vv` | dazu, was erkannt wurde: Format, Kodierung, BOM, Zeilenenden |
| 3 | `-vvv` | dazu die Fehlersuchausgabe: gelesene und geschriebene Bytes und **jede Diagnose beim Lesen** |

**Fehler werden niemals unterdrückt, auch nicht mit `--quiet`.** Ein Befehl, der
stillschweigend scheitert, hinterließe nichts als seinen Exit-Code.

Die Zusammenfassung erscheint erst ab zwei Dateien: Bei einer einzigen Eingabe würde sie
die Zeile davor wiederholen.

## Diagnosen beim Lesen

Untertitelformate werden **so gut es geht** gelesen: Bei einer Auffälligkeit hält das
Lesen nicht an — es entscheidet oder lässt die Dinge, wie sie sind, und sagt es. Worauf
es gestoßen ist, wird auf **Stufe 3** ausgegeben, eine Zeile pro Diagnose, auf der
Fehlerausgabe:

```
a.srt: 2 diagnostics while reading
a.srt: line 6: SubRip numbers that do not follow ("7"), settled by the reader
a.srt: line 9: a line that fits nowhere, left as it stands
```

Alle siebzehn Unterbefehle melden sie, nicht nur [`inspect`](inspect.md):
Eine Datei, die so gut es geht gelesen und dann zurückgeschrieben wurde, hat dieselben Entscheidungen durchlaufen, und sie zu verschweigen,
ließe glauben, es sei nichts geschehen.

**Eine Diagnose ist niemals ein Fehlschlag.** Die Datei wurde gelesen, der Befehl
war erfolgreich, der Exit-Code ist `0`. Deshalb stehen sie auf der
ausführlichsten Stufe: Weiter unten würden sie die Zeile verschütten, die sagt, was wirklich
getan wurde.

### Was jede Zeile enthält

| Teil | Was er sagt |
| :----- | :------------- |
| `line N` | wo, ab 1 gezählt, wie ein Editor es anzeigt; **fehlt** bei dem, was die ganze Datei betrifft |
| der Satz | worauf gestoßen wurde, unter den zwölf Kategorien unten |
| `("…")` | der fehlerhafte Text der Datei, wenn die Kategorie nicht genügt; auf 80 Bytes gekürzt |
| das Ende | was damit geschehen ist: `settled by the reader` oder `left as it stands` |

**Zwei Kategorien haben keine Zeile**, und es sind die beiden zur
Kodierung: Die Bytes wurden abgewogen oder ihre Marke gelesen, bevor eine einzige
Zeile der Datei existierte. Ihre Zeile beginnt daher mit dem Satz, ohne `line N`
davor — es gibt keinen Ort, den man nennen könnte.

**Das Ende der Zeile ist das Wichtigste.** `settled by the reader` bedeutet,
dass das Lesen entschieden hat und dass die geschriebene Datei diese Entscheidung enthält — eine
fehlende Nummerierung wird neu erzeugt. `left as it stands` bedeutet, dass es nichts
angerührt hat, weil **nur Sie entscheiden können**: Ein Untertitel, der endet, bevor er
beginnt, bleibt, wie er ist.

### Die Zeilennummer eines Blocks

Eine Auffälligkeit, die **einen ganzen Block** betrifft, wird an seiner Zeitzeile
verankert und nicht an der genauen Zeile, die sie auslöst. Eine inkonsistente Nummerierung
in Zeile 5 wird daher in Zeile 6 gemeldet, der Zeile der
darauf folgenden Zeitzeile — die fehlerhafte Nummer steht in `("…")`. Nur die Auffälligkeiten, die
**eine Zeile** betreffen — eine unlesbare Zeitzeile, Text vor der ersten —, werden
an sich selbst gemeldet.

### Die zwölf Kategorien

| Satz | Was ihn auslöst |
| :----- | :------------------ |
| `a line that fits nowhere` | eine Zeile, die zu keinem Block gehört |
| `a timing line that could not be read` | eine unlesbare Zeitzeile |
| `a SubRip block without its number` | fehlende Nummerierung, beim Schreiben neu erzeugt |
| `SubRip numbers that do not follow` | eine Nummerierung, die springt |
| `text before the first timing line` | Text vor der ersten Zeitzeile |
| `a WebVTT block of an unknown kind` | ein nicht erkannter WebVTT-Block |
| `declares an event column this tool cannot fill` | eine Ereignisspalte von Sub Station Alpha, die hier nichts füllt |
| `counts in frames and states no rate; it was read at` | eine Datei in Bildern, die keine Bildrate angibt — die verwendete steht im Detail |
| `carries no end times; each one was taken from the next start` | ein Format ohne Enden, TMPlayer oder LRC: **Kein Ende in der Tabelle stammt aus der Datei** |
| `more than one kind of line ending` | gemischte Zeilenenden |
| `an encoding nothing declared` | die Kodierung wurde durch Abwägen der Bytes vorgeschlagen, und sie ist nicht UTF-8 |
| `a byte order mark that contradicts the encoding asked for` | die Datei trägt eine Marke, und `--encoding` hat eine andere genannt — die Marke gewinnt |

**Die drei in der Mitte sind die aus Phase 9**, und die letzten beiden dieser
drei sprechen nicht von einer Zeile, sondern von der ganzen Datei: Sie sagen, dass ein
angezeigter Wert nicht gelesen wurde. Eine gefüllte Spalte `End` bei einer LRC-Datei, eine
Position bei einer MicroDVD-Datei — die Datei enthielt nichts davon.

**Eine Operation kann ihre eigenen hinzufügen**, die keine Diagnose beim Lesen sind:
[`correct`](correct.md#ein-muster-das-nicht-angewendet-werden-kann) nennt mit dem Untertitel das Muster,
das aufgegeben hat. Das wird auf Stufe 1 gemeldet und trägt im Objekt der Datei die Kennung
`pattern-failed` — niemals ein Fehlschlag der Datei.

## Eine Übersetzung

Eine Übersetzungsdatei wird **wie jede Untertiteldatei** verarbeitet:
`subedit-cli case --to sentence film.fr.srt` ändert sie ohne irgendetwas Neues. Was die Paarung hinzufügt und was
drei Optionen ausdrücken, ist eng umgrenzt: **zu wissen, wie die Zeilen einer Übersetzung auf die
Untertitel der Hauptdatei fallen** — das ist `inspect -t` —, und **die Übersetzung zu ändern, während sie
auf ihre Hauptdatei aufgelegt bleibt**, mit denselben Regeln wie im Fenster.

| Option | Wo | Wert | Standard |
| :----- | :---- | :----- | :----- |
| `-t`, `--translation-file` | `inspect`, `hearing-impaired`, `replace`, `case`, `italics`, `dialogue-dashes`, `correct`, `pair` | die Übersetzungsdatei | — |
| `--document` | die sechs Textunterbefehle, weder `inspect` noch `pair` | `main` oder `translation` | `main` |
| `--align-method` | wie `-t` | `position` oder `number` | `position` |

- **`position`** vergleicht die Mitte jeder Zeile mit den Grenzen der Untertitel: Eine fehlende Zeile verschiebt
  die folgenden nicht. **`number`** schickt Zeile *n* an Untertitel *n*, ohne auf die
  Positionen zu schauen. Position ist der Standard, aus demselben Grund wie im Fenster: Eine fehlende Zeile in der
  Mitte verschiebt den ganzen Rest einer nach Nummer gepaarten Übersetzung.
- **`--document translation` verlangt `-t`, und `-t` verlangt `--document translation`** bei einem Text-Unterbefehl
  — das eine ohne das andere ist ein Aufruffehler, Code `1`, denn eine gelesene Datei, die kein Vorgang
  verwendet, ist ein Versehen und keine Vorliebe. `-t` bei `inspect` hat kein `--document`: Es
  berichtet. **`--align-method` ohne `-t` ist ebenfalls ein Aufruffehler.**
- **`-t` benennt eine Datei und ist daher nur bei einer einzigen Eingabe sinnvoll**: Bei mehreren Eingaben oder
  bei einem mit `--recursive` übernommenen Verzeichnis wird es wie `--output` abgelehnt, **bevor eine Datei
  gelesen wird**. Für einen Stapel rufen Sie das Programm einmal pro Paar auf.
- **Eine Übersetzungsdatei, die mit der Hauptdatei identisch ist, wird abgelehnt** — wie auch immer ihr
  Pfad geschrieben ist: `<translation>: the file is already open as the main document`, Code `2`.
- **Nur das anvisierte Dokument wird geschrieben**, unter **seinem eigenen Pfad und in seinem eigenen Format**: dem Format,
  der Kodierung und den Zeilenenden der Übersetzungsdatei. `--output` benennt die Datei der
  Übersetzung, `--output-dir` legt ihren Basisnamen dort ab, `--in-place` schreibt sie um; **die Hauptdatei
  wird niemals angerührt**. Die Pfade des Berichts sind die der Übersetzung.
- **Die Untertitel, die durch die Paarung entstehen** — eine Zeile, die nichts erreicht hat —, werden mit
  **ihren eigenen Positionen** in die Übersetzung geschrieben, welche Methode auch immer gilt; **ein Untertitel, den niemand
  übersetzt hat, wird als Block ohne Text geschrieben**, der sich ohne Auffälligkeit liest.
- **`--encoding` und `--frame-rate` gelten für beide Dateien.**
- Die **Positions**-Unterbefehle — `shift`, `transform`, `framerate`, `snap`, `adjust` — haben
  diese Optionen nicht: Geben Sie ihnen beide Dateien, wie bei einem Stapel,
  `subedit-cli shift --by 2 film.srt film.fr.srt --output-dir out/`.
- **Die Übersetzung zeitlich an die Hauptdatei angepasst zu schreiben** — das Einzige, was die Paarung kann und eine Datei allein
  nicht —, ist die Aufgabe von [`pair`](pair.md), das weder `--document` noch `--recursive` hat.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\n[Un oiseau] Bonjour.\n\n2\n00:00:03,000 --> 00:00:04,000\nAu revoir.\n\n' > film.fr.srt; subedit-cli hearing-impaired film.srt -t film.fr.srt --document translation --output propre.fr.srt; cat propre.fr.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\n[Un oiseau] Bonjour.\n\n2\n00:00:03,000 --> 00:00:04,000\nAu revoir.\n\n' > film.fr.srt; subedit-cli hearing-impaired film.srt -t film.fr.srt --document translation --output propre.fr.srt; cat propre.fr.srt
film.fr.srt: 1 subtitle cleaned, 0 removed -> propre.fr.srt
1
00:00:01,000 --> 00:00:02,000
Bonjour.

2
00:00:03,000 --> 00:00:04,000
Au revoir.
```

Mit `--format json` enthält das Objekt jeder Datei außerdem den Schlüssel `alignment`, der sagt, wie die
Übersetzung aufgelegt wurde — `{"file", "method", "attached", "born", "untranslated", "out_of_order"}`, wie
der Schlüssel `translation` von [`inspect`](inspect.md#in-json) —, und **mit `--dry-run`** listet `changes`
die Untertitel der Übersetzung auf, jeden mit `"document":"translation"`.

## Mehrere Dateien

Jeder Unterbefehl akzeptiert mehrere Pfade, außer [`append`](append.md), [`split-file`](split-file.md) und
[`pair`](pair.md), deren Stelligkeit die auf ihrer Seite ist. Jeder wird
unabhängig verarbeitet: Das Scheitern eines Pfads unterbricht die anderen nicht, und die Fehlschläge
werden mit dem Namen der Datei und dem Grund gemeldet.

**Ein Verzeichnis steht mit `--recursive` (`-r`) für die Dateien, die es enthält**:
Ohne diese Option ist es ein Aufruffehler, eines anzugeben, Code `1`, und nichts wird
verarbeitet — es abzulehnen ist besser, als nichts zu verarbeiten und `0` zurückzugeben:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?" -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?"
films: is a directory: use --recursive
code=1
```

Der Durchlauf, der unter `--output-dir` erhaltene Baum und das, was
übergangen wird, sind in [Einen Verzeichnisbaum verarbeiten](lots.md) beschrieben.

## Die Handbuchseiten

Eine Installation legt **eine Handbuchseite pro Unterbefehl** neben `subedit-cli(1)` ab:
`subedit-cli-shift(1)`, `subedit-cli-convert(1)` und die anderen, eine für jede Zeile
[der Tabelle der Unterbefehle](#unterbefehle). `subedit-cli(1)` enthält dieses Kapitel — die globalen Optionen,
die Ausgaben, die Exit-Codes —, und jede andere Seite enthält das Kapitel ihres Unterbefehls, seine
Beispiele eingeschlossen. Die Seite des Fensters, `subedit-gui(1)`, steht daneben.

```console
$ man subedit-cli
$ man subedit-cli-shift
```

**Die Seiten werden nicht getrennt von diesem Handbuch geschrieben**: Sie werden beim Bauen des
Programms daraus erzeugt, sagen also, was es sagt, und können nicht davon abweichen. Der Abschnitt FILES jeder
Seite nennt das Verzeichnis, in dem das Handbuch installiert ist.

| Seiten | Wo sie installiert werden |
| :---- | :----------------------- |
| Englisch | `share/man/man1` |
| jede installierte Sprache — Katalanisch, Tschechisch, Deutsch, Spanisch, Finnisch, Französisch, Galicisch, Hebräisch, Ungarisch, Interlingue, Isländisch, Italienisch, Niederländisch, Polnisch, Portugiesisch, Brasilianisches Portugiesisch, Russisch, Serbisch, Türkisch, Vereinfachtes Chinesisch | `share/man/<language>/man1` — `share/man/fr/man1` |

**`man` wählt die Sprache anhand der Locale**, wie das Programm: Unter einer französischen Locale
zeigt `man subedit-cli` die französische Seite, und eine Sprache, deren Seiten nicht installiert sind, erhält die
englischen. `-L` verlangt eine Sprache unabhängig von der Locale:

```console
$ man -L fr subedit-cli-shift
```

## Exit-Codes

| Code | Bedeutung |
| :--- | :------------ |
| `0` | alles ist gelungen |
| `1` | Aufruffehler — unbekannte Option, ungültiger Wert, verbotene Kombination |
| `2` | keine Datei konnte verarbeitet werden |
| `3` | einige Dateien wurden verarbeitet, andere nicht |

`2` und `3` werden unterschieden, damit ein Skript weiß, wie es handeln soll, ohne die Ausgabe erneut zu lesen:
„nichts hat funktioniert“ und „eine fehlt“ verlangen nicht dieselbe Reaktion.

Ein Aufruffehler wird **vor jeder Verarbeitung** erkannt: Er hinterlässt niemals einen
halb verarbeiteten Stapel.

## Fehler

| Was ihn auslöst | Was auf die Fehlerausgabe geschrieben wird |
| :------------------ | :--------------------------------------- |
| unbekannte Option oder unbekannter Unterbefehl | der fehlerhafte Name und ein Verweis auf `--help` |
| `--quiet` mit `-v` | `--quiet and -v ask for opposite things; give one or the other` |
| `--config` mit `--no-config` | `--config excludes --no-config` |
| `--config` mit einem leeren Namen | `--config needs a file name` |
| fehlende Datei | `<path>: does not exist` |
| unlesbare Datei | `<path>: cannot be opened: permission denied` |
| Kodierung, die ihre eigene Marke schreibt | `"<name>" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do` |
| Bytes, die sich nicht dekodieren lassen | `<path>: cannot be decoded in the chosen encoding` |
| Format nicht erkannt | `<path>: is in no format this tool knows` |
| nichts, was wie ein Untertitel aussieht | `<path>: holds nothing recognisable as a subtitle` |
| `--document translation` ohne `-t` | `--document translation needs the translation file: use -t` |
| `-t` ohne `--document translation` | `-t names a translation to change: use --document translation` |
| `--align-method` ohne `-t` | `--align-method needs a translation file: use -t` |
| `-t` mit mehreren Eingaben oder einem Verzeichnis | `-t names one file but several inputs were given: use one invocation per pair` |
| Übersetzung identisch mit der Hauptdatei | `<translation>: the file is already open as the main document` |
