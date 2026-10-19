# Einen Verzeichnisbaum verarbeiten

<!-- machine-translated -->
> Diese Seite ist eine maschinelle Übersetzung und wurde noch von keiner Leserin und keinem Leser dieser Sprache geprüft.

```
subedit-cli <subcommand> --recursive [options] <directory>...
```

Jeder Unterbefehl, der einen Stapel von Dateien liest — `inspect` eingeschlossen, nicht aber
[`append`](append.md), [`split-file`](split-file.md) und [`pair`](pair.md), deren Stelligkeit die eines Paares oder
eines Films ist —, akzeptiert mit `--recursive` (`-r`) ein **Verzeichnis** anstelle einer Datei.
Diese Seite sagt, was der Durchlauf erfasst, in welcher Reihenfolge und wohin er schreibt.

**Nichts von dem, was folgt, ändert, was mit einer Datei geschieht**: Jede wird unabhängig gelesen,
verarbeitet und geschrieben, das Scheitern einer Datei hält die anderen nicht auf, und die
vier Exit-Codes sind die des [Aufrufs](invocation.md#exit-codes).
Der Durchlauf erzeugt eine Liste von Pfaden, die der Rest des Stapels verarbeitet, als wären sie
eingetippt worden.

## Ohne `--recursive` wird ein Verzeichnis abgelehnt

Aufruffehler, Code `1`, keine Datei verarbeitet oder geschrieben:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?" -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?"
films: is a directory: use --recursive
code=1
```

## Was der Durchlauf erfasst

Der Durchlauf ist **rekursiv, deterministisch und ohne Überraschungen**:

- **die Reihenfolge** — die Einträge jedes Verzeichnisses werden in der Reihenfolge ihrer
  Namen genommen, **Byte für Byte** (Großbuchstaben vor Kleinbuchstaben, unabhängig von der
  Sprache des Rechners), und ein Unterverzeichnis wird an seiner Stelle durchlaufen, zuerst in
  die Tiefe. Zwei Läufe ergeben dieselben Bytes, auf jedem
  Rechner;
- **symbolische Links werden nicht verfolgt** — weder zu einer Datei noch zu einem
  Verzeichnis: Ein Baum, der einen Link auf eines seiner übergeordneten Verzeichnisse enthält, hätte kein
  Ende;
- **versteckte Dateien und Verzeichnisse werden ignoriert** — die, deren Name mit
  einem Punkt beginnt;
- **nur die Endungen der bekannten Formate werden behalten**: `.srt`, `.vtt`,
  `.ssa`, `.ass`, `.lrc`, `.sub`, unabhängig von der Groß-/Kleinschreibung. **`.txt` ist ausgeschlossen**: Die Endung
  benennt zwei Formate, vor allem aber jede `LISEZMOI.txt` der Welt, und ein Stapel, der
  an jeder davon scheiterte, wäre unbrauchbar.

**Eine auf der Befehlszeile genannte Datei wird niemals gefiltert.** Sie zu nennen heißt,
sie zu wollen: `subedit-cli shift --by 1 --in-place notes.txt` verarbeitet sie, welche
Endung sie auch hat, und beschwert sich nur, wenn ihr Inhalt in keinem Format vorliegt.

**Was übergangen wurde, wird gezählt** und auf Stufe `-vv` gemeldet:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli -vv inspect --recursive films 2>&1 | head -1 -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli -vv inspect --recursive films 2>&1 | head -1
films: 1 file left aside, with no extension of a known format
```

Ein Verzeichnis, das nichts zu durchlaufen enthält, meldet das auf der Standardstufe
(`<directory>: holds no file in a format this tool walks for`) und ist kein Fehler.

## Wohin der Durchlauf schreibt

**Der Baum bleibt unter `--output-dir` erhalten**: Der Pfad einer Datei,
**relativ zum angegebenen Verzeichnis**, wird unter dem Ausgabeverzeichnis nachgebildet. Der Name
des angegebenen Verzeichnisses gehört nicht dazu — das ist die Semantik von `rsync` mit einem
abschließenden Schrägstrich, nicht die von `cp -r`: Man nennt das Ziel und fügt ihm die
gewünschte Ebene hinzu (`--output-dir sortie/films`).

<!-- example: mkdir -p films/a/b; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/x.srt; cp films/x.srt films/a/x.srt; cp films/x.srt films/a/b/y.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli shift --by 1 --recursive --output-dir sortie films; find sortie -type f | sort -->
```console
$ mkdir -p films/a/b; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/x.srt; cp films/x.srt films/a/x.srt; cp films/x.srt films/a/b/y.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli shift --by 1 --recursive --output-dir sortie films; find sortie -type f | sort
films/a/b/y.srt: 1 subtitle shifted by 1.000 s -> sortie/a/b/y.srt
films/a/x.srt: 1 subtitle shifted by 1.000 s -> sortie/a/x.srt
films/x.srt: 1 subtitle shifted by 1.000 s -> sortie/x.srt
3 of 3 files shifted
sortie/a/b/y.srt
sortie/a/x.srt
sortie/x.srt
```

Das Ausgabeverzeichnis und die Verzeichnisse des Baums werden **angelegt**, wenn sie fehlen. Mit
`--in-place` wird jede behaltene Datei an ihre Stelle zurückgeschrieben. Eine Endung, die
`convert` ändert, ändert sich auch im Baum: `films/a/x.srt`, nach
WebVTT konvertiert, wird als `sortie/a/x.vtt` geschrieben.

**Das Ausgabeverzeichnis wird, wenn es im durchlaufenen Baum liegt, vom Durchlauf ausgeschlossen**:
Ein zweiter Lauf liest nicht erneut, was der erste geschrieben hat.

## Zwei Verzeichnisse und Kollisionen

Die Regel des [Ziels](invocation.md#das-ziel) entscheidet: Zwei Dateien,
die am selben Pfad landen würden, werden **abgelehnt, bevor irgendetwas geschrieben wird**, Code `1`, die
beiden Eingaben und das Ziel werden genannt.

<!-- example: mkdir -p un/a deux/a; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > un/a/x.srt; cp un/a/x.srt deux/a/x.srt; subedit-cli shift --by 1 -r --output-dir sortie un deux; echo "code=$?" -->
```console
$ mkdir -p un/a deux/a; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > un/a/x.srt; cp un/a/x.srt deux/a/x.srt; subedit-cli shift --by 1 -r --output-dir sortie un deux; echo "code=$?"
sortie/a/x.srt: would be written by both un/a/x.srt and deux/a/x.srt
code=1
```

## Was es kostet

Der Stapel bleibt sequenziell: Seine Dauer ist die Summe der Dauern seiner Dateien. Der
Benchmark `make bench` misst den Durchlauf und dieselbe Arbeit in einer einzigen Datei; die Zahlen
stehen im [Leistungsprotokoll](../../../../mesures/performances.md).
