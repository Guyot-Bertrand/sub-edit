# `pair`

<!-- machine-translated -->
> Diese Seite ist eine maschinelle Übersetzung und wurde noch von keiner Leserin und keinem Leser dieser Sprache geprüft.

```
subedit-cli pair (--output FILE | --output-dir DIR | --in-place)
                 -t FILE [--align-method position|number]
                 [--dry-run]
                 <main>
```

Schreibt **die auf die Positionen der Hauptdatei umgetimte Übersetzung** und sagt, wie ihre Zeilen ihren
Untertitel gefunden haben. Es ist `Open Translation…` des Fensters — [dieselbe Paarung](../subedit-gui/files.md#öffnen-einer-übersetzung),
derselbe Satz —, gefolgt vom Schreiben des Ergebnisses, das das Fenster über `Save Translation` erledigt.

**Es ist das Einzige, was die Paarung tut und eine allein verarbeitete Datei nicht.** Die anderen Unterbefehle,
die `-t` annehmen, ändern die Übersetzung, die sie lesen — [`case`](case.md), [`replace`](replace.md) … —; `pair` ändert
nichts am Text: Es **legt** die Übersetzung auf die Hauptdatei auf und schreibt, was das Auflegen ergeben hat. Die Hauptdatei wird
gelesen, um die Zeilen aufzunehmen, und **nie geschrieben**.

- **Geschrieben wird die Übersetzung**, unter **ihrem eigenen Pfad und in ihrem eigenen Format**: das Format, die Kodierung und
  die Zeilenenden der Übersetzungsdatei. Ihre Untertitel stehen **an den Positionen der Hauptdatei**: Eine Zeile, die
  neben die Marke gefallen war, erhält die Grenzen des Untertitels zurück, den sie übersetzt.
- **`-t` ist erforderlich.** `pair` ohne Übersetzung hat nichts aufzulegen: Das ist ein Aufruffehler, Code `1`.
- **Eine einzige Hauptdatei** und kein `--recursive`: Ein Paar sind zwei Dateien, und ein Verzeichnisbaum kann
  nicht sagen, welche Übersetzung zu welcher Hauptdatei gehört. Für einen Stapel: ein Aufruf pro Paar.
- **`--output` nennt die Datei der geschriebenen Übersetzung, `--output-dir` legt ihren Basisnamen dort ab, `--in-place`
  schreibt sie neu**, wie bei jedem Unterbefehl, der `-t` annimmt — siehe [Eine Übersetzung](invocation.md#eine-übersetzung).
- **`--document` gibt es nicht**: Es gibt nur ein anvisiertes Dokument, die Übersetzung.

<!-- example: subedit-cli pair --help -->
```console
$ subedit-cli pair --help
Write a translation at the positions of its main file, and say how it lined up
Usage: subedit-cli pair [OPTIONS] main

Positionals:
  main TEXT REQUIRED          The main subtitle file

Options:
  -h,--help                   Print this help message and exit
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
| `<main>` | ja | **ein einziger** Pfad | — |
| `-t`, `--translation-file` | **ja** | die Übersetzungsdatei | — |
| `--align-method` | nein | `position` oder `number` — siehe [Eine Übersetzung](invocation.md#eine-übersetzung) | `position` |
| `--output` / `--output-dir` / `--in-place` | **eine der drei**, außer mit `--dry-run` | siehe [Aufruf](invocation.md#das-ziel) | — |
| `--dry-run` | nein | ein Schalter: berechnet und sagt, schreibt nichts — siehe [Vor dem Schreiben nachsehen](invocation.md#vor-dem-schreiben-nachsehen) | aus |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,300 --> 00:00:02,300\nBonjour.\n\n2\n00:00:03,300 --> 00:00:04,300\nAu revoir.\n\n' > film.fr.srt; subedit-cli pair film.srt -t film.fr.srt --output recale.fr.srt; cat recale.fr.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,300 --> 00:00:02,300\nBonjour.\n\n2\n00:00:03,300 --> 00:00:04,300\nAu revoir.\n\n' > film.fr.srt; subedit-cli pair film.srt -t film.fr.srt --output recale.fr.srt; cat recale.fr.srt
film.fr.srt: translation: 2 lines attached -> recale.fr.srt
1
00:00:01,000 --> 00:00:02,000
Bonjour.

2
00:00:03,000 --> 00:00:04,000
Au revoir.
```

Die Übersetzung lag 0,3 s zu spät: Jede Zeile hat den Untertitel gefunden, den ihre Mitte überlappt, und die
geschriebene Datei trägt die Positionen der Hauptdatei.

## Was die Zuordnung sagt

Der Satz ist **der, den das Fenster sagt**, wenn es eine Übersetzung öffnet, jeder Teil nur, wenn er nicht
null ist — siehe [`inspect -t`](inspect.md#eine-übersetzung--t) für die Tabelle seiner Teile. **Er wird
ab Stufe 1 geschrieben** und nicht auf Stufe 2 wie bei den anderen Unterbefehlen: Hier ist die Zuordnung kein
Detail der Verarbeitung, sie ist das Ergebnis.

**Was die Zuordnung mit der geschriebenen Datei macht**:

- **ein Untertitel, den niemand übersetzt hat, wird als Block ohne Text geschrieben**, der sich ohne Auffälligkeit lesen lässt;
- **ein aus einer Zeile entstandener Untertitel** — eine Zeile, die nichts erreicht hat — wird mit **seinen eigenen Positionen** geschrieben, was auch immer
  die Methode ist: Die geschriebene Datei kann daher mehr Untertitel zählen als die Hauptdatei;
- **was das Dokument als Übersetzung trug, wird vor dem Auflegen geleert**, was auf der Befehlszeile nicht zutrifft:
  Die Hauptdatei wird allein gelesen.

**Die beiden Methoden** ergeben dieselbe Datei, wenn die Zeilen jeweils auf ihren Untertitel fallen, und nicht,
wenn eine Zeile fehlt oder die Übersetzung verschoben ist — deshalb ist die Methode eine Wahl, und deshalb
ist die Position der Standard. Die acht Fälle von `src/test/data/paires/` zählen sie auf, mit der für
jede Methode erwarteten Datei.

## Ausgabe

**Standardausgabe** — nichts: Das Ergebnis ist die geschriebene Datei.

| Stufe | Was auf der Fehlerausgabe hinzukommt |
| :----- | :------------------------------------- |
| 1 | `<translation>: translation: N lines attached; … -> <destination>` — **der Pfad ist der der Übersetzung**, die die geschriebene Datei ist; mit `--dry-run` steht `(dry run, nothing written)` an der Stelle von `-> <destination>` |
| 2 | `<translation>: SubRip, UTF-8, no BOM, LF line endings kept` — das Format, die Kodierung, die Marke und die Zeilenenden **der Übersetzung**, so wiederhergestellt, wie sie waren |
| 3 | `<translation>: N bytes read, M written`, dann **jede Diagnose beim Lesen** der Hauptdatei und der Übersetzung — siehe [Aufruf](invocation.md#diagnosen-beim-lesen) |

### In JSON

Mit [`--format json`](invocation.md#ausgabe-die-ein-skript-lesen-kann) **ein einziges Objekt**, dessen `file` die Übersetzung und dessen
`destination` die geschriebene Datei ist. `counts.subtitles` zählt **die Untertitel der geschriebenen Datei**, die von einer Zeile
ins Leben gerufenen eingeschlossen. Der Schlüssel `alignment` — `{"file", "method", "attached", "born", "untranslated", "out_of_order"}` —
enthält dieselben Zählungen wie der Satz, wie der Schlüssel `translation` von [`inspect`](inspect.md#in-json).

## Exit-Codes

Die des Werkzeugs: `0`, wenn die Übersetzung geschrieben — oder, mit `--dry-run`, berechnet — ist, `2`, wenn eine der beiden Dateien nicht
gelesen oder das Ziel nicht geschrieben werden konnte, `1` bei einem Aufruffehler. **Eine schlecht zugeordnete Übersetzung ist kein
Fehlschlag**: Sie wird geschrieben, und der Satz sagt, was seinen Platz nicht gefunden hat.

## Fehler

| Was sie auslöst | Meldung |
| :------------------- | :------ |
| kein `-t` | `pair needs the translation file to lay over the main one: use -t` |
| `--document` | `The following argument was not expected: --document` — es existiert für `pair` nicht |
| mehr als eine Hauptdatei | `The following argument was not expected: <path>` |
| die Übersetzung ist die Hauptdatei | `<translation>: the file is already open as the main document`, Code `2` |
| eine der Dateien fehlt, ist unlesbar oder hat ein unbekanntes Format | dieselben wie überall, mit dem Pfad der betroffenen Datei |
