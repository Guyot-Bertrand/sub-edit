# `hearing-impaired`

<!-- machine-translated -->
> Diese Seite ist eine maschinelle Übersetzung und wurde noch von keiner Leserin und keinem Leser dieser Sprache geprüft.

```
subedit-cli hearing-impaired
                  (--output FILE | --output-dir DIR | --in-place)
                  [--dry-run]
                  [--recursive] [--parentheses]
                  [--document main|translation] [-t FILE] [--align-method position|number]
                  <file>...
```

Entfernt die **Hinweise für Hörgeschädigte** — die zwischen eckigen Klammern beschriebenen Geräusche — aus der ganzen
Datei. **Runde Klammern bleiben standardmäßig erhalten**: Sie werden ebenso oft für eine geflüsterte
Replik wie für ein beschriebenes Geräusch verwendet, und sie zu entfernen, würde Dialog löschen. `--parentheses` entfernt sie
ebenfalls, für eine Datei, die ihre Geräusche darin beschreibt.

Was der Unterbefehl nicht tut: Liedtexte zwischen Rautenzeichen, den Namen des Sprechers
vor einem Doppelpunkt, das erneute Großschreiben, das Korrigieren von OCR-Fehlern. Diese
Muster sind die des Assistenten
[`Correct Texts…`](../subedit-gui/correct-texts.md) des Fensters, den die Befehlszeile
unter dem Namen [`correct`](correct.md) anbietet; und hier lässt sich nichts einstellen: Die
Transformation ist festgelegt, nicht konfigurierbar.

<!-- example: subedit-cli hearing-impaired --help -->
```console
$ subedit-cli hearing-impaired --help
Remove the sounds described between square brackets
Usage: subedit-cli hearing-impaired [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to clean

Options:
  -h,--help                   Print this help message and exit
  --parentheses               Also remove the sounds between parentheses, often whispered lines
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --document main|translation The document to change: the main one, or the translation given by -t
  -t,--translation-file FILE  Translation file to lay over the subtitle file, for a single input
  --align-method position|number
                              How the lines of the translation find their subtitles
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Argumente und Optionen

| Option | Erforderlich | Werte | Standard |
| :----- | :----- | :------ | :----- |
| `<file>...` | ja | ein oder mehrere Pfade | — |
| `--parentheses` | nein | ein Schalter: entfernt auch die Hinweise zwischen runden Klammern | aus |
| `--recursive`, `-r` | nein | ein Schalter | aus — siehe [Einen Verzeichnisbaum verarbeiten](lots.md) |
| `--document` | nein | `main` oder `translation` — siehe [Eine Übersetzung](invocation.md#eine-übersetzung) | `main` |
| `-t`, `--translation-file` | mit `--document translation` | eine Übersetzungsdatei, für **eine einzige** Eingabe | — |
| `--align-method` | nein, und nur mit `-t` | `position` oder `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **eine der drei**, außer mit `--dry-run` | siehe [Aufruf](invocation.md#das-ziel) | — |
| `--dry-run` | nein | ein Schalter: berechnet und sagt, schreibt nichts — siehe [Vor dem Schreiben nachsehen](invocation.md#vor-dem-schreiben-nachsehen) | aus |

Das Format der gelesenen Datei wird **beibehalten**: Das Format zu ändern, ist die Aufgabe von
[`convert`](convert.md).

**Standardmäßig der Haupttext; die Übersetzung mit `--document translation -t FILE`.** Dann wird
die Übersetzung geändert und geschrieben, und nur sie — siehe [Eine Übersetzung](invocation.md#eine-übersetzung).

**Ein Hinweis, der eine Übersetzung leeren würde, schreibt sie leer und behält den Untertitel**: Der Untertitel
bleibt ohne Text in der Datei. Der Haupttext dagegen verliert einen ganz geleerten Untertitel, wie
unten beschrieben.

## Was entfernt wird und was bleibt

Ein Hinweis ist eine eckige Klammer — oder eine runde, mit `--parentheses` — samt dem, was sie umschließt, **Zeilenumbruch
eingeschlossen**: In echten Dateien wird ein Hinweis oft durch das
Ende einer Zeile zerschnitten.

| Eingabe | Ausgabe |
| :----- | :----- |
| `Bonjour [il tousse] Marie` | `Bonjour Marie` |
| `Bonjour[il tousse]Marie` | `Bonjour Marie` |
| `[Bruit de pas] Bonjour` | `Bonjour` |
| `A [un] [deux] B` | `A B` |

Das Entfernen lässt **genau ein Leerzeichen zwischen dem, was den Hinweis umgab, und nichts am
Rand einer Zeile**. Es ist keine Bereinigung der Leerzeichen der Datei: Zwei Leerzeichen,
die der Text schon enthielt, bleiben zwei Leerzeichen.

**Wenn der Hinweis über den Zeilenumbruch reicht, bleibt der Umbruch** — die
zwei Zeilen bleiben zwei Zeilen. Dieser Fall ist keine Kuriosität: In fünfzehn
echten Untertitelungen war jede scheinbar verwaiste eckige Klammer ein so
zerschnittener Hinweis.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nReculez ! [Il\nhurle] Tout de suite !\n\n' > a.srt; subedit-cli --quiet hearing-impaired --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nReculez ! [Il\nhurle] Tout de suite !\n\n' > a.srt; subedit-cli --quiet hearing-impaired --output b.srt a.srt; cat b.srt
1
00:00:01,000 --> 00:00:03,000
Reculez !
Tout de suite !
```

**Was kein Hinweis ist und sich nicht bewegt:**

| Fall | Was geschieht |
| :-- | :-------------- |
| rein numerischer Inhalt — `[1]`, `(12)`, `[ 156478 ]` | bleibt, wie er ist, Begrenzer eingeschlossen: Ein Verweis hat nie ein Geräusch benannt |
| eine runde Klammer samt Inhalt, ohne `--parentheses` | bleibt, wie sie ist: `Venez (chut) ici` ändert sich nicht |
| eine eckige oder runde Klammer, die nichts schließt | bleibt, wie sie ist |
| ein Tag des Formats — `<i>`, `</i>` | wird nie entfernt |

Ein Leerraum **zwischen** den Ziffern, ein Buchstabe oder ein Zeichen — `[1 2]`, `[1a]`,
`[-1]` — machen daraus einen Hinweis. Ebenso ein leerer Inhalt, `[]` oder `( )`: keine
Ziffer, also kein Verweis; nichts zu sagen, also nichts zu behalten.

## Was ganz verschwindet

**Eine Zeile, die das Entfernen geleert hat, verschwindet**, und **ein ganz geleerter
Untertitel wird aus der Datei entfernt**. Die folgenden Untertitel werden neu nummeriert.

Eine Zeile gilt als leer, wenn in ihr nur noch Leerraum, Tags des Formats
oder ein einzelner Dialogstrich übrig sind — `<i>[Musique]</i>` hinterlässt kein
`<i></i>` auf dem Bildschirm.

**Die Tags sind die des Formats der Datei.** `{\i1}[Musique]{\i0}` in einer
Advanced-SSA-Datei, `{Y:i}[Musique]` in einer MicroDVD-Datei und `/[Musique]` in einer MPL2-Datei hinterlassen
ebenfalls nichts. Umgekehrt ist ein `<i>` in einem Format, das kein
Tag schreibt — TMPlayer, LRC —, Text, den der Zuschauer liest, und die Zeile, die
es enthält, ist nicht leer.

**Ein auf eine einzige Stimme reduzierter Dialog ist keiner mehr**, und der verbleibende Strich
verschwindet. Solange zwei Stimmen bleiben, bleiben die Striche.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\nVoir [1] la note.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\nVoir [1] la note.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle cleaned, 1 removed -> b.srt
1
00:00:04,000 --> 00:00:06,000
Attends Marie.

2
00:00:07,000 --> 00:00:09,000
Voir [1] la note.
```

## Eine Datei ohne jeden Hinweis wird trotzdem geschrieben

Sie wird unverändert kopiert, der Exit-Code ist `0`, und der Bericht sagt, dass es
nichts zu entfernen gab.

Das ist die Regel der Unterbefehle, die schreiben: **Ein angegebenes Ziel ist
ein geschriebenes Ziel**. Hier eine Ausnahme zu machen, würde ein Skript zwingen zu wissen,
welcher der Unterbefehle manchmal eine Datei erzeugt und manchmal nichts.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nRien à signaler.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nRien à signaler.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt
a.srt: no mention to remove -> b.srt
```

## Ausgabe

**Standardausgabe** — nichts: Das Ergebnis ist die geschriebene Datei. **Mit `--dry-run` sind es die vorgeschlagenen Änderungen**: für jeden Untertitel, dessen Text sich ändern würde, seine Nummer, der Text vorher und der Text nachher — siehe [Vor dem Schreiben nachsehen](invocation.md#vor-dem-schreiben-nachsehen).

| Stufe | Was auf der Fehlerausgabe hinzukommt |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles cleaned, M removed -> <destination>`, oder `<path>: no mention to remove -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — das Format, die Kodierung, die Marke und die Zeilenenden der gelesenen Datei, so wiederhergestellt, wie sie waren |
| 3 | `<path>: N bytes read, M written`, dann **jede Diagnose beim Lesen** — siehe [Aufruf](invocation.md#diagnosen-beim-lesen) |

`N` zählt die Untertitel, deren Text sich geändert hat, `M` diejenigen, die das Entfernen
ganz geleert hat und die daher die Datei verlassen haben.

### In JSON

Mit [`--format json`](invocation.md#ausgabe-die-ein-skript-lesen-kann) enthält das Objekt jeder Datei `counts.cleaned` (die umgeschriebenen Texte) und `counts.removed` (die entfernten Untertitel); **mit `--dry-run`** listet der Schlüssel `changes` außerdem die geänderten Untertitel auf, wie die Standardausgabe der Textform. Die vollständige Tabelle steht unter [Aufruf](invocation.md#was-jeder-unterbefehl-in-counts-angibt).

## Exit-Codes

Die des Werkzeugs: `0`, wenn jede Datei geschrieben wurde, `2`, wenn keine, `3`, wenn
nur einige, `1` bei einem Aufruffehler.

## Fehler

Dieser Unterbefehl hat keinen eigenen Fehler: Er nimmt keinen
zu zerlegenden Wert an, und kein Text kann ihn eine Datei ablehnen lassen. Was
scheitern kann, ist das Lesen oder das Schreiben, wie bei den anderen.

**Ohne eine der drei Zieloptionen wird nichts geschrieben**, und ihre Fehler
sind den Unterbefehlen gemeinsam, die schreiben:
siehe [Aufruf](invocation.md#das-ziel).
