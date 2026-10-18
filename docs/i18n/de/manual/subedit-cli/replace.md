# `replace`

<!-- machine-translated -->
> Diese Seite ist eine maschinelle Übersetzung und wurde noch von keiner Leserin und keinem Leser dieser Sprache geprüft.

```
subedit-cli replace [--regex] [--case-sensitive] [--range N-M|N-]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    <pattern> <replacement> <file>...
```

Ersetzt einen Text in den Untertiteln, **ohne ein Tag zu zerbrechen**. Es ist die
Operation `Replace All` der [Suche](../subedit-gui/find-and-replace.md) des Fensters, mit ihren Regeln:

- **gesucht wird im sichtbaren Text** — dem, was der Zuschauer liest. Tags gehören nicht
  dazu: Die Suche nach `<i>` findet nichts, und die Suche nach `Bonjour` findet ein „Bonjour“, dessen eine Hälfte
  kursiv ist;
- **ersetzt wird im Quelltext**, dem der Datei, wobei die Tags bleiben, wo sie
  waren. Ein Tag, das das gesuchte Wort zerschneidet, umschließt die ganze Ersetzung; ein Stil, der
  den Treffer berührt, deckt die ganze Ersetzung ab; was der Treffer nicht berührt,
  bewegt sich nicht. Die Regeln, Fall für Fall, sind die der Suche des Fensters.

**Es gibt keinen Unterbefehl `find`**: Um zu sehen, wo ein Muster vorkommt, ohne etwas zu ändern,
listet [`--dry-run`](invocation.md#vor-dem-schreiben-nachsehen) jeden betroffenen Untertitel auf.

<!-- example: subedit-cli replace --help -->
```console
$ subedit-cli replace --help
Replace a text in the subtitles, without breaking a tag
Usage: subedit-cli replace [OPTIONS] pattern replacement files...

Positionals:
  pattern TEXT REQUIRED       Text to look for in what is shown
  replacement TEXT REQUIRED   What replaces it, tags allowed
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --regex                     Read the text to look for as a regular expression
  --case-sensitive            Tell capitals from small letters
  --range N-M|N-              Act only on subtitles N to M, or N to the end
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
| `<pattern>` | ja | der Text, der im Angezeigten gesucht wird; **nicht leer** | — |
| `<replacement>` | ja | was ihn ersetzt, Tags erlaubt; kann leer sein | — |
| `<file>...` | ja | ein oder mehrere Pfade | — |
| `--recursive`, `-r` | nein | ein Schalter | aus — siehe [Einen Verzeichnisbaum verarbeiten](lots.md) |
| `--regex` | nein | ein Schalter: Das Muster ist ein regulärer Ausdruck von ICU | einfacher Text |
| `--case-sensitive` | nein | ein Schalter: unterscheidet Groß- und Kleinbuchstaben | sie werden ignoriert |
| `--range` | nein | `N-M`, oder `N-` bis zum Ende — siehe [`adjust`](adjust.md#--range) | die ganze Datei |
| `--document` | nein | `main` oder `translation` — siehe [Eine Übersetzung](invocation.md#eine-übersetzung) | `main` |
| `-t`, `--translation-file` | mit `--document translation` | eine Übersetzungsdatei, für **eine einzige** Eingabe | — |
| `--align-method` | nein, und nur mit `-t` | `position` oder `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **eine der drei**, außer mit `--dry-run` | siehe [Aufruf](invocation.md#das-ziel) | — |
| `--dry-run` | nein | ein Schalter: berechnet und sagt, schreibt nichts — siehe [Vor dem Schreiben nachsehen](invocation.md#vor-dem-schreiben-nachsehen) | aus |

**Ein Muster, das mit einem Bindestrich beginnt**, wird nach `--` geschrieben: `subedit-cli replace -- -- "—" film.srt`.

**Standardmäßig der Haupttext; die Übersetzung mit `--document translation -t FILE`.** Dann wird
die Übersetzung geändert und geschrieben, und nur sie — siehe [Eine Übersetzung](invocation.md#eine-übersetzung).

### Einfacher Text oder Ausdruck

Standardmäßig wird das Muster **so gelesen, wie es ist**: `a.b` sucht ein `a`, einen Punkt, ein `b`. Die
Groß-/Kleinschreibung wird mit derselben Feinheit ignoriert wie im Fenster — `É` und `é` sind ein einziger Buchstabe.

Mit `--regex` ist das Muster ein regulärer Ausdruck von **ICU**, gelesen, wie Gaupol ihn liest: `.` überschreitet
einen Zeilenumbruch, `^` und `$` gelten an jeder Zeile eines Untertitels. In der Ersetzung, und
nur dann:

| Schreiben Sie | Um zu sagen |
| :----- | :-------- |
| `$1` bis `$9` | die Gruppen des Treffers |
| `$0` | den ganzen Treffer |
| `\n` | einen Zeilenumbruch |
| `\$`, `\\` | ein `$`, einen Backslash |

Achten Sie auf die Anführungszeichen der Shell: `'$1 EUR'`, und nicht `"$1 EUR"`, bei dem die Shell `$1` lesen würde.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nPrix: 12 euros, et 7 euros.\n\n' > prix.srt; subedit-cli replace --regex '([0-9]+) euros' '$1 EUR' --output euros.srt prix.srt; cat euros.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nPrix: 12 euros, et 7 euros.\n\n' > prix.srt; subedit-cli replace --regex '([0-9]+) euros' '$1 EUR' --output euros.srt prix.srt; cat euros.srt
prix.srt: replaced 2 matches -> euros.srt
1
00:00:01,000 --> 00:00:02,000
Prix: 12 EUR, et 7 EUR.
```

## Tags

Ein Text, in dem ein Tag das gesuchte Wort zerschneidet:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace Bonjour Salut --output salut.srt balises.srt; cat salut.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace Bonjour Salut --output salut.srt balises.srt; cat salut.srt
balises.srt: replaced 2 matches -> salut.srt
1
00:00:01,000 --> 00:00:02,000
<i>Salut</i> tout le monde

2
00:00:03,000 --> 00:00:04,000
Salut <i>Marie</i>, ca va
```

Im ersten Untertitel zerschnitt das Tag „Bonjour“: Jetzt umschließt es „Salut“.
Im zweiten wird es nicht berührt und bleibt, wo es ist. **Ein offen gelassenes Tag
bleibt offen**: Wackliges Markup zu reparieren, wäre eine andere Operation.

Die Ersetzung wird in **den Tags des Formats der Datei** gelesen: `<i>…</i>` für SubRip,
`{\i1}…` für Advanced SSA. Nur so lässt sich das Geschriebene formatieren.

## Nichts gefunden, nichts zu ändern

Zwei Antworten, die keine Fehlschläge sind — der Exit-Code ist `0`, und **die Datei wird
trotzdem geschrieben**, denn ein angegebenes Ziel ist ein geschriebenes Ziel:

| Situation | Satz |
| :-------- | :----- |
| das Muster kommt nirgends vor | `"zzz" not found` |
| es kommt vor, aber jeder Treffer wird durch sich selbst ersetzt | `nothing to change` |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli replace zzz x --output b.srt a.srt; subedit-cli replace Bonjour Bonjour --output c.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli replace zzz x --output b.srt a.srt; subedit-cli replace Bonjour Bonjour --output c.srt a.srt
a.srt: "zzz" not found -> b.srt
a.srt: nothing to change -> c.srt
```

## `--dry-run`

Die Standardausgabe listet **jeden Untertitel auf, dessen Text sich ändern würde**, vorher und nachher, so wie
sie in der Datei stehen — Tags eingeschlossen:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace --dry-run Bonjour Salut balises.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace --dry-run Bonjour Salut balises.srt
balises.srt: replaced 2 matches (dry run, nothing written)
balises.srt: subtitle 1
- <i>Bon</i>jour tout le monde
+ <i>Salut</i> tout le monde
balises.srt: subtitle 2
- Bonjour <i>Marie</i>, ca va
+ Salut <i>Marie</i>, ca va
```

Die Form der Liste ist unter [Vor dem Schreiben nachsehen](invocation.md#vor-dem-schreiben-nachsehen) beschrieben;
in JSON ist es `changes`.

## Ausgabe

**Standardausgabe** — nichts: Das Ergebnis ist die geschriebene Datei. Mit `--dry-run` die obige Liste der
Änderungen.

| Stufe | Was auf der Fehlerausgabe hinzukommt |
| :----- | :------------------------------------- |
| 1 | `<path>: replaced N matches -> <destination>`, oder `"<pattern>" not found`, oder `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — das Format, die Kodierung, die Marke und die Zeilenenden der gelesenen Datei, so wiederhergestellt, wie sie waren |
| 3 | `<path>: N bytes read, M written`, dann **jede Diagnose beim Lesen** — siehe [Aufruf](invocation.md#diagnosen-beim-lesen) |

`N` zählt **die ersetzten Treffer in den Untertiteln, deren Text sich geändert hat** — es ist die
Zahl, die auch das Fenster nennt.

### In JSON

Mit [`--format json`](invocation.md#ausgabe-die-ein-skript-lesen-kann) enthält das Objekt jeder Datei:

| Schlüssel | Inhalt |
| :-- | :------ |
| `counts.replaced` | die ersetzten Treffer in den Untertiteln, deren Text sich geändert hat: das `N` des Satzes |
| `counts.matched` | die gefundenen Treffer, **ob ihre Ersetzung den Text geändert hat oder nicht** — daran unterscheidet sich „nichts gefunden“ (`0`) von „nichts zu ändern“ (`replaced` bei `0`, `matched` positiv) |
| `changes` | die geänderten Untertitel: `{"subtitle", "document", "before", "after"}`, **leer**, wenn sich nichts geändert hat |

## Exit-Codes

Die des Werkzeugs: `0`, wenn jede Datei verarbeitet wurde — eine Datei, in der nichts gefunden wurde, ist es
—, `2`, wenn keine, `3`, wenn nur einige, `1` bei einem Aufruffehler.

## Fehler

| Was ihn auslöst | Meldung |
| :------------------ | :------ |
| leeres Muster | `pattern: nothing to look for` |
| unlesbarer Ausdruck | `pattern: not a regular expression (<ICU's reason>)` |
| Bereich falsch geschrieben oder außerhalb der Datei | siehe [`adjust`](adjust.md#--range) |

**Das Muster wird vor der ersten Datei gelesen**: Ein unlesbarer Ausdruck ist ein Aufruffehler
(Code `1`), nichts wird gelesen oder geschrieben.

Die Fehler des Ziels sind den Unterbefehlen gemeinsam, die schreiben:
siehe [Aufruf](invocation.md#das-ziel).
