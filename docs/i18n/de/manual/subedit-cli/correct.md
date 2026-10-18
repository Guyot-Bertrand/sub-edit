# `correct`

<!-- machine-translated -->
> Diese Seite ist eine maschinelle Übersetzung und wurde noch von keiner Leserin und keinem Leser dieser Sprache geprüft.

```
subedit-cli correct --tasks LIST --code CODE
                    [--classes human|ocr|human,ocr]
                    [--enable NAME]... [--disable NAME]...
                    [--language CODE]
                    [--max-length N] [--max-lines N]
                    [--skip-length N|off] [--skip-lines N|off]
                    [--keep-blank-subtitles | --remove-blank-subtitles]
                    [--range N-M|N-]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    <file>...
```

Korrigiert die Texte mit **den Mustern von Gaupol**, wie der Assistent
[`Correct Texts…`](../subedit-gui/correct-texts.md) des Fensters — dieselben Muster,
dieselbe Engine, dieselben Zählungen. Hier gibt es sechs Aufgaben: die **Hinweise für
Hörgeschädigte**, das **Zusammenfügen** und **Trennen von Wörtern**, die **häufigen Fehler**,
die **Großschreibung** und den **Zeilenumbruch**.

<!-- example: subedit-cli correct --help -->
```console
$ subedit-cli correct --help
Correct the texts with the patterns of the correction assistant
Usage: subedit-cli correct [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to correct

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --tasks LIST                The tasks to run, in a list: none is chosen beforehand
  --code CODE                 The cascade of patterns the tasks read: Zyyy, Latn, Latn-en, Latn-en-US
  --classes human|ocr|human,ocr
                              The classes of common errors to apply: both by default
  --enable NAME               Switch a pattern on, by its English name or type:name; repeatable
  --disable NAME              Switch a pattern off, by its English name or type:name; repeatable
  --language CODE             The dictionary join-words and split-words read: fr_FR, en_US, en...
  --max-length N              Longest line of line-break, in characters; no default, Gaupol's is in ems
  --max-lines N               Most lines of line-break: 3, as Gaupol's
  --skip-length N|off         Leave alone a subtitle whose longest line is within this, or off: --max-length
  --skip-lines N|off          Leave alone a subtitle whose line count is within this, or off: --max-lines
  --keep-blank-subtitles      Leave the subtitles the correction empties, empty, instead of removing them
  --remove-blank-subtitles    Remove the subtitles the correction empties, whatever the settings file says
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

## Was gewählt wird und was nicht

**Nichts ist vorab angekreuzt.** Das Fenster öffnet seine Kästchen so, wie der Benutzer sie gelassen hat; eine
Befehlszeile nimmt ihre Aufgaben, ihren Code, ihre Zeilenlänge und ihre Sprache aus ihrer eigenen Zeile,
niemals aus der Einstellungsdatei: Weder die eines Tages im Fenster angekreuzten Aufgaben noch die dort
geschalteten Musterkästchen (`correction.activations`) noch die Ersetzungsliste der Rechtschreibprüfung werden
gelesen, und auf dieser Seite wird nichts geschrieben.

**Vier Einstellungen kommen standardmäßig aus der Einstellungsdatei** — die Werte, die sich `Correct Texts…`
merkt oder die [`config set`](config.md) geschrieben hat. **Eine Option auf der Zeile gewinnt immer**, dann
die Datei, dann der eingebaute Standardwert; siehe [Die Einstellungsdatei](invocation.md#die-einstellungsdatei)
dazu, wo sie liegt, sowie zu `--config` und `--no-config`.

| Schlüssel | Was er liefert | Option, die ihn übersteuert |
| :-- | :--------------- | :----------------------- |
| `correction.human`, `correction.ocr` | die angewendeten Klassen häufiger Fehler | `--classes` |
| `correction.remove-blank` | ob die Untertitel, die die Korrektur leert, entfernt werden | `--keep-blank-subtitles`, `--remove-blank-subtitles` |
| `correction.line-break.max-lines` | die höchste Zeilenzahl von `line-break` | `--max-lines` |

- **Jede der vier hat eine Option, die sie aufhebt**, was auch immer die Datei sagt: `--classes human,ocr`
  wendet beide Klassen an, `--remove-blank-subtitles` entfernt die geleerten Untertitel, `--max-lines 3`
  stellt die Zeilen von Gaupol wieder her. `--keep-blank-subtitles` und `--remove-blank-subtitles` zusammen sind ein
  Aufruffehler.
- **Beide Klassen in der Datei ausgeschaltet wird ignoriert, und das wird gesagt**, auf Stufe 1 —
  `settings: correction.human and correction.ocr are both false, ignored` —: `--classes` könnte es nicht schreiben, und es würde
  common-errors nichts zu tun lassen. Stattdessen werden beide Klassen angewendet.
- **Eine Zeilenzahl außerhalb von 1 bis 1000 wird gar nicht aus der Datei gelesen**: Das Lesen der Datei
  lehnt sie ab — `settings: correction.line-break.max-lines: cannot read 'N', default kept`, auf
  Stufe 1 —, und der eingebaute Wert 3 wird verwendet.

Die Beispiele dieser Seite werden mit `--no-config` ausgeführt, damit sie zeigen, was die eingebauten
Standardwerte tun, unabhängig von den Einstellungen der Maschine, die sie ausführt.

| Option | Erforderlich | Werte | Standard |
| :----- | :----- | :------ | :----- |
| `<file>...` | ja | ein oder mehrere Pfade | — |
| `--tasks` | **ja** | eine durch Kommas getrennte Liste: `mentions`, `join-words`, `split-words`, `common-errors`, `capitalization`, `line-break` | keine Aufgabe |
| `--code` | **ja** mit einer Musteraufgabe, sonst **entfällt** | `Script[-language[-COUNTRY]]`: `Zyyy`, `Latn`, `Latn-en`, `Latn-en-US` | — |
| `--language` | **ja** mit `join-words` oder `split-words`, sonst **entfällt** | ein Sprachcode: `fr`, `fr_FR`, `en_US` | — |
| `--classes` | nein | `human`, `ocr` oder `human,ocr` | beide — oder was die Einstellungsdatei sagt |
| `--enable`, `--disable` | nein, wiederholbar | der englische Name eines Musters, oder `type:name` | die mitgelieferten `.conf`-Dateien |
| `--max-length` | **ja** mit `line-break` | eine Anzahl von **Zeichen** größer als null | **keiner** — siehe unten |
| `--max-lines` | nein | eine ganze Zahl von Zeilen, größer als null | 3 — oder was die Einstellungsdatei sagt |
| `--skip-length`, `--skip-lines` | nein | eine Zahl, oder `off` | dieselben Grenzen wie `--max-length` und `--max-lines` |
| `--keep-blank-subtitles` | nein | ein Schalter: Die geleerten Untertitel bleiben, leer | die geleerten Untertitel werden entfernt — sofern die Einstellungsdatei nichts anderes sagt |
| `--remove-blank-subtitles` | nein | ein Schalter: Die geleerten Untertitel werden entfernt, was auch immer die Einstellungsdatei sagt; abgelehnt mit `--keep-blank-subtitles` | aus |
| `--range` | nein | `N-M`, oder `N-` bis zum Ende — siehe [`adjust`](adjust.md#--range) | die ganze Datei |
| `--document` | nein | `main` oder `translation` — siehe [Eine Übersetzung](invocation.md#eine-übersetzung) | `main` |
| `-t`, `--translation-file` | mit `--document translation` | eine Übersetzungsdatei, für **eine einzige** Eingabe | — |
| `--align-method` | nein, und nur mit `-t` | `position` oder `number` | `position` |
| `--recursive`, `-r` | nein | ein Schalter | aus — siehe [Einen Verzeichnisbaum verarbeiten](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **eine der drei**, außer mit `--dry-run` | siehe [Aufruf](invocation.md#das-ziel) | — |
| `--dry-run` | nein | ein Schalter: berechnet und sagt, schreibt nichts — siehe [Vor dem Schreiben nachsehen](invocation.md#vor-dem-schreiben-nachsehen) | aus |

### Die Aufgaben

**Die Reihenfolge der Liste spielt keine Rolle**: Die Aufgaben werden in
der Reihenfolge von Gaupol angewendet — die Hinweise, dann das Zusammenfügen und Trennen von Wörtern, dann die häufigen Fehler, dann die Großschreibung, dann der Zeilenumbruch —, jede auf dem
Text, den die vorige übrig gelassen hat. Ein Untertitel, den die Hinweise geleert haben, spielt bei
allem Folgenden keine Rolle.

| Aufgabe | Was sie tut |
| :---- | :-------------- |
| `mentions` | entfernt die Geräusche zwischen eckigen Klammern, und zwischen runden Klammern, wenn `Sound in parentheses` eingeschaltet ist (der Durchgang von [`hearing-impaired`](hearing-impaired.md)), dann die Muster für Liedtexte und Sprecher |
| `join-words` | fügt ein in zwei Teile geschnittenes Wort wieder zusammen, anhand des Wörterbuchs von `--language` |
| `split-words` | trennt zwei aneinandergeklebte Wörter, anhand desselben Wörterbuchs |
| `common-errors` | korrigiert die häufigen Fehler: Leerzeichen, Satzzeichen, Ligaturen, OCR-Fehler |
| `capitalization` | setzt den Großbuchstaben dorthin, wo die Sprache ihn verlangt |
| `line-break` | schneidet die Zeilen eines Untertitels neu, sodass keine über `--max-length` hinausgeht, und wägt dabei die Umbruchmuster des Codes ab |

### Der Mustercode

**`--code` benennt die Kaskade**: `Zyyy` für jede Schrift, dann die Schrift, die Sprache
und das Land — `Latn-en-US` liest die Muster von `Zyyy`, von `Latn`, von `Latn-en` und von
`Latn-en-US`. **Er ist erforderlich**, weil eine Befehlszeile keine Umgebungssprache hat:
Ein universeller Standardwert würde *weniger* anwenden, als man glaubt, ohne es zu sagen. Er gilt für
jede Aufgabe desselben Laufs; eine Aufgabe, die einen eigenen Code möchte, ist ein zweiter
Lauf. Ein falsch geschriebener Code ist ein Aufruffehler:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code francais --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code francais --output b.srt a.srt; echo "code=$?"
--code: "francais" is not a pattern code: expected Script[-language[-COUNTRY]], like Zyyy, Latn or Latn-en-US
code=1
```

Die Muster, die der Benutzer in `$XDG_DATA_HOME/subedit/patterns` (oder
`~/.local/share/subedit/patterns`) ablegt, **kommen zu den mitgelieferten Mustern hinzu**, wie im
Fenster, und ein mitgeliefertes Muster, das nicht gelesen werden kann, wird auf Stufe 1 gemeldet: `patterns: <file>, line N (<reason>)`.

### Wörter zusammenfügen und trennen, und das Wörterbuch

**Diese beiden Aufgaben lesen kein Muster**: Sie befragen das **Wörterbuch des Systems, über
Enchant** — eine Abhängigkeit der Pakete (`libenchant-2-2` unter Debian und Ubuntu, `enchant2` unter
Fedora), die `subedit-cli` mit dem Fenster teilt. Sie haben daher weder einen Code noch einen Namen zu setzen:
`--code` ergibt ohne eine Musteraufgabe keinen Sinn, und `--language` keinen ohne eine dieser beiden
— das eine ohne das andere angegeben ist ein Aufruffehler, Code `1`.

- **`join-words`** klebt ein Wort, das das Wörterbuch nicht kennt, an dasjenige, das ihm vorausgeht oder
  folgt, **wenn eine einzige Richtung ein bekanntes Wort ergibt**: Aus `hel lo` wird `hello`; zwei mögliche
  Richtungen, oder keine, entscheiden nichts.
- **`split-words`** trennt ein Wort, das das Wörterbuch nicht kennt, **wenn genau einer seiner
  Vorschläge dieses Wort mit einem Leerzeichen ist**: Aus `hellothere` wird `hello there`. Ein Wort mit einem großen
  Anfangsbuchstaben, gefolgt von Kleinbuchstaben, bleibt unberührt: Meistens ist es ein Name.

**Ohne ein Wörterbuch für die Sprache wird der Lauf abgelehnt**, Code `1`, **bevor eine Datei
gelesen wird** und mit den Worten des Fensters, das die Funktion ausgraut und das sagt — hier lehnt die Befehlszeile
ab und sagt es, statt zu schweigen:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nhel lo\n\n' > a.srt; subedit-cli --no-config correct --tasks join-words --language qq_QQ --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nhel lo\n\n' > a.srt; subedit-cli --no-config correct --tasks join-words --language qq_QQ --output b.srt a.srt; echo "code=$?"
no dictionary for qq_QQ
code=1
```

**Die Ersetzungsliste des Fensters wird weder gelesen noch geschrieben.** Sie *füllt sich* durch die
Entscheidungen des Benutzers in der Rechtschreibprüfung; eine Befehlszeile hat keine solche
Entscheidung, und die Liste zu lesen würde ein Skript von dem abhängig machen, was sein Autor einmal hineingesetzt hat.

### Zeilenumbruch

**Er wird in Zeichen gemessen, und das ist der Unterschied zum Fenster.** Das Fenster misst in *ems* —
einer an der Schrift kalibrierten Breite, mit 0,55 em pro Buchstabe —, und sein Standardwert 24 ist ein Wert
in ems. Eine Befehlszeile hat keine Schrift: Sie zählt Zeichen, die des
Textes. **Die 24 als Zeichen zu übernehmen, würde fast alles schneiden, ohne
es zu sagen**; es gibt daher **keine Standardlänge**, und `--max-length` ist mit
`line-break` erforderlich:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn texte\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn texte\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --output b.srt a.srt; echo "code=$?"
--max-length is required by the task line-break: Gaupol's 24 is a width in ems, and has no value in characters
code=1
```

**`--max-lines` ist 3**, der Wert von Gaupol, den die Einheit nicht ändert; der Zeilenumbruch nimmt
eine weitere hinzu, wenn drei nicht genügen. **Das Überspringen** — ein Untertitel, der schon innerhalb der Grenzen liegt, wird
so gelassen, wie er ist — übernimmt standardmäßig **dieselben Grenzen** (`--skip-length` = `--max-length`,
`--skip-lines` = `--max-lines`), wie Gaupol es tut; `--skip-length off` und `--skip-lines off`
schalten es aus, und der Text wird dann wieder als Ganzes gelesen und neu geschnitten:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nWhen the rain stopped, we walked home under the dark sky of the old town.\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --max-length 24 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nWhen the rain stopped, we walked home under the dark sky of the old town.\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --max-length 24 --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:03,000
When the rain
stopped, we walked
home under the dark
sky of the old town.
```

Die Umbruchmuster sind die des Codes: `Latn` bricht weder zwischen einer Zahl und ihrer Einheit noch
nach einem Dialogstrich um, `Latn-en` bricht nicht nach einem Artikel, einer Präposition, einem Titel
oder einem Possessivbegleiter um. **Unter `Zyyy` gibt es kein Umbruchmuster**: Die Zeilen werden
nur ausgeglichen. **Diese Aufgabe wird nie mangels eines aktiven Musters abgelehnt**: Ihre Muster
wägen ab, wo umgebrochen wird, der Umbruch geschieht auch ohne sie. Die vier Optionen ergeben nur mit
`line-break` einen Sinn; ohne sie angegeben, sind sie ein Aufruffehler.

### Die Klassen und das Schalten nach Namen

**`--classes` filtert die häufigen Fehler endgültig**: `ocr` abzuwählen nimmt seine
Muster aus der Anwendung heraus — die Ligatur `ﬁ` bleibt dann `ﬁ`. Die drei anderen Mustertypen
tragen keine Klasse.

**`--enable` und `--disable` setzen ein Muster über seinen englischen Namen**, wobei der Name der Schlüssel ist
(ADR 0037); ein Name kann mehrere Einträge bezeichnen, die sich dann ein einziges
Kästchen teilen. Das ist nötig, und es ist das, was die Standardwerte des Fensters beiseitelässt: **Die
mitgelieferten `.conf`-Dateien schalten die Hinweismuster aus**, sodass `--tasks mentions` allein
nichts anwenden würde. Die zwei Muster „Sound in brackets“ und „Sound in parentheses“
steuern jeweils ihr Begrenzungszeichen — die eckigen Klammern, die runden Klammern — und werden mit denselben
Optionen gesetzt: Eines allein einzuschalten entfernt das andere nicht.

Drei Dinge sind **Aufruffehler**, Code `1`, gemeldet, bevor eine Datei gelesen wird:

| Was ihn auslöst | Meldung |
| :------------------ | :------ |
| ein Name, der kein Muster der angegebenen Aufgaben unter dem angegebenen Code bezeichnet | `--enable: "<name>" names no pattern of the tasks given, under the code <code>` |
| ein Name, der Muster zweier Typen bezeichnet — er wird dann `type:name` geschrieben, wobei der Typ `common-error`, `capitalization`, `hearing-impaired` oder `line-break` ist | `--enable: "<name>" names patterns of several types: write type:name, …` |
| ein Name, der `--enable` und `--disable` gegeben wird | `"<name>" is given to --enable and to --disable: choose one` |

**Ein Tippfehler fällt laut auf**: Das Fenster ignoriert eine veraltete Einstellung, weil eine
Einstellung ein verschwundenes Muster überdauern kann; ein Aufruf hat keine Überbleibsel.

**Eine Aufgabe ohne jedes aktive Muster wird abgelehnt**, Code `1` — „nichts zu tun“ ist eine
Antwort, die ausgesprochen wird, kein stiller Erfolg:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --output b.srt a.srt; echo "code=$?"
mentions: no pattern is active under the code Latn; switch one on with --enable
code=1
```

Dieselbe Datei, mit eingeschaltetem Durchgang für eckige Klammern:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --enable "Sound in brackets" --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --enable "Sound in brackets" --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:02,000
Bonjour
```

## Was geschrieben wird und was gesagt wird

Das Format, die Kodierung und die Zeilenenden der gelesenen Datei werden **beibehalten**. **Die
Untertitel, die die Korrektur leert, werden entfernt**, wie beim Standard-Kästchen des
Fensters; `--keep-blank-subtitles` lässt sie leer stehen, und `--remove-blank-subtitles` entfernt sie
auch dann, wenn die Einstellungsdatei sie behält. **Eine geleerte Übersetzung bleibt geleert**
und behält ihren Untertitel.

**Die Zählung ist die des Fensters** — „Edited N and removed M subtitles“: geänderte
Texte und entfernte Untertitel, **niemals Treffer**. Eine Datei, in der sich nichts
ändert, wird trotzdem geschrieben: Ein angegebenes Ziel ist ein geschriebenes Ziel.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\neﬀet  de ﬁn\n\n2\n00:00:03,000 --> 00:00:04,000\nPropre.\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code Latn --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\neﬀet  de ﬁn\n\n2\n00:00:03,000 --> 00:00:04,000\nPropre.\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code Latn --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:02,000
effet de fin

2
00:00:03,000 --> 00:00:04,000
Propre.
```

### Ein Muster, das nicht angewendet werden kann

**Ein Muster, das nicht gelesen werden kann, nicht übersetzt werden kann oder nicht zu Ende kommt, wird genannt, zusammen mit dem
Untertitel, an dem es aufgegeben hat**, und hier sagt die Befehlszeile mehr als das
Fenster: Die Bestätigungsseite nennt das Muster nur einmal, ohne den Text.

| Grund | Was er bedeutet |
| :----- | :------------------- |
| `cannot be translated` | sein Ausdruck verwendet etwas, das die Übersetzung nach ICU ablehnt |
| `will not compile` | die Engine hat seinen Ausdruck abgelehnt |
| `has an invalid replacement` | seine Ersetzung lässt sich nicht so lesen, wie Python sie liest |
| `timed out` | die Engine hat bei diesem Text aufgegeben |
| `never settled` | `Repeat` änderte den Text nach hundert Durchgängen noch immer |
| `grew the text too long` | der Text ging über das hinaus, was ein Untertitel fassen kann |

Auf Stufe 1, auf der Fehlerausgabe: 
`<path>: pattern "<name>" (<reason>) was not applied to subtitle N` — ohne das Ende, wenn das Muster für jeden Text beiseitegelegt wird. **Die Nummer ist die der
Datei**, unabhängig davon, welche Aufgaben zuvor Untertitel entfernt haben. In JSON ist es
eine Warnung `pattern-failed` der Datei, deren `detail` den Satz enthält. **Ein
aufgegebenes Muster ist keine gescheiterte Datei**: Die Datei wird geschrieben, und der Exit-Code ist `0`.

## Ausgabe

**Standardausgabe** — nichts, da das Ergebnis die geschriebene Datei ist. **Mit `--dry-run` sind es die
vorgeschlagenen Änderungen**, ein Block pro Untertitel: seine Nummer, der Text davor, der Text danach —
und `(removed)` ohne `+`-Zeile für einen Untertitel, den die Korrektur entfernen würde; mit
`--keep-blank-subtitles` bleibt er, leer. Siehe [Vor dem Schreiben nachsehen](invocation.md#vor-dem-schreiben-nachsehen).

| Stufe | Was auf der Fehlerausgabe hinzukommt |
| :----- | :------------------------------------- |
| 1 | `<path>: Edited N and removed M subtitles -> <destination>`, die Muster, die nicht angewendet werden konnten, die Musterdateien, die nicht gelesen werden können — und zuerst die gelesene Einstellungsdatei — `settings: read <path>` — und was davon ignoriert wurde |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, dann jede Diagnose beim Lesen |

In JSON enthält `counts` die Schlüssel `corrected` (die geänderten Texte) und `removed` (die entfernten
Untertitel), und `changes` die berührten Untertitel. Mit `-t` sagt der Schlüssel `alignment`, wie die
Übersetzung aufgelegt wurde.
`settings` nennt die gelesene Einstellungsdatei, oder ist `null`, wenn keine gelesen wurde.

## Exit-Codes

Die des Werkzeugs: `0`, wenn jede Datei geschrieben wurde, `2`, wenn keine, `3`, wenn nur
einige, `1` bei einem Aufruffehler.

## Fehler

| Was ihn auslöst | Was auf die Fehlerausgabe geschrieben wird |
| :------------------ | :--------------------------------------- |
| eine mit `--config` genannte Datei, die nicht existiert — fehlt die eigene Datei des Benutzers, gelten die Standardwerte | `settings file not found: <FILE>` |
| `--tasks` fehlt | `--tasks is required`, gefolgt von einem Verweis auf `--help` |
| eine unbekannte Aufgabe | `--tasks: "<name>" is not a task: expected mentions, join-words, split-words, common-errors, capitalization or line-break` |
| `--code` fehlt | `--code is required by the tasks that read patterns: <tasks>` |
| `--code` ohne eine Musteraufgabe | `--code is for the tasks that read patterns, and none was asked for` |
| `--language` fehlt bei `join-words` oder `split-words` | `--language is required by the tasks that check words: <tasks>` |
| `--language`, das kein Sprachcode ist | `--language: "<value>" is not a language: expected a locale code, like fr, en_US or sr@Latn` |
| `--language` ohne eine dieser beiden Aufgaben | `--language is for the tasks join-words and split-words, which were not asked for` |
| eine Sprache ohne Wörterbuch | `no dictionary for <language>` |
| ein falsch geschriebener Code | `--code: "<code>" is not a pattern code: expected Script[-language[-COUNTRY]], like Zyyy, Latn or Latn-en-US` |
| `--max-length` fehlt bei `line-break` | `--max-length is required by the task line-break: Gaupol's 24 is a width in ems, and has no value in characters` |
| `--max-length`, das keine Zahl größer als null ist | `--max-length: "<value>" is not a length: expected a number of characters greater than zero` |
| `--max-lines`, `--skip-lines`, die keine ganze Zahl von Zeilen sind | `--max-lines: "<value>" is not a number of lines: expected a whole number greater than zero` |
| `--skip-length` oder `--skip-lines` unlesbar | `--skip-length: "<value>" is not a bound: expected a number greater than zero, or off` |
| eine der vier Optionen ohne `line-break` | `--max-length is for the task line-break, which was not asked for` |
| eine unbekannte Klasse | `--classes: "<name>" is not a class: expected human, ocr or human,ocr` |
| `--keep-blank-subtitles` mit `--remove-blank-subtitles` | `--keep-blank-subtitles and --remove-blank-subtitles say opposite things; give one of them` |
| ein fehlerhafter Mustername | siehe die Tabelle oben |
| eine Aufgabe ohne aktives Muster | `<task>: no pattern is active under the code <code>; switch one on with --enable` |
| `--range`, `-t`, `--document`, das Ziel | die von [Aufruf](invocation.md#fehler) und von [Eine Übersetzung](invocation.md#eine-übersetzung) |
