# `config`

<!-- machine-translated -->
> Diese Seite ist eine maschinelle Übersetzung und wurde noch von keiner Leserin und keinem Leser dieser Sprache geprüft.

```
subedit-cli [--config FILE | --no-config] [--format text|json] config list
subedit-cli [--config FILE | --no-config] [--format text|json] config show
subedit-cli [--config FILE] config set KEY VALUE
subedit-cli [--config FILE] config unset KEY
```

Listet, zeigt und ändert die **Einstellungen der Operationen**: den Teil der Einstellungsdatei, den
das Fenster und die Befehlszeile teilen. `Adjust Durations…` und `Correct Texts…` merken sich dort ihre
Werte, und [`adjust`](adjust.md) und [`correct`](correct.md) lesen sie standardmäßig — siehe
[Die Einstellungsdatei](invocation.md#die-einstellungsdatei).

**Was es nicht tut:** Es liest und schreibt niemals eine **Anzeigeeinstellung** (Geometrie des Fensters,
Spalten, Design, Suchoptionen, Rechtschreibprüfung, Video…). Diese bleiben Sache des Fensters,
werden im Fenster gesetzt und in [seinem Handbuch](../subedit-gui/preferences.md#die-datei) beschrieben.
Es rührt auch nie eine Untertiteldatei an.

<!-- example: subedit-cli config --help -->
```console
$ subedit-cli config --help
List, show and change the operation settings
Usage: subedit-cli config [OPTIONS] SUBCOMMAND

Options:
  -h,--help                   Print this help message and exit

Subcommands:
  list                        Write every operation key, what it is for, what it accepts and its default
  show                        Write the settings file used, then every operation key with its value
  set                         Write KEY = VALUE into the settings file, keeping every other setting
  unset                       Put KEY back to its default in the settings file, keeping every other setting
```

## Unterbefehle

| Unterbefehl | Was er tut | Datei |
| :---------- | :----------- | :--- |
| `config list` | schreibt jeden Operationsschlüssel, mit seinem Zweck, dem, was er annimmt, seinem Standardwert und wer ihn verwendet — **kein Wert** | liest keine |
| `config show` | schreibt die verwendete Datei und ob sie existiert, dann jeden Operationsschlüssel mit seinem Wert, der Herkunft des Werts und seinem Standardwert | liest sie |
| `config set KEY VALUE` | prüft `VALUE` und schreibt dann die Datei mit `KEY = VALUE` neu, wobei jede andere Einstellung erhalten bleibt | liest und schreibt sie |
| `config unset KEY` | setzt `KEY` auf seinen Standardwert zurück, auskommentiert geschrieben, so wie die Datei einen Standardwert immer schreibt | liest und schreibt sie |

| Option | Erforderlich | Werte | Standard |
| :----- | :------- | :----- | :------ |
| `KEY` | ja, für `set` und `unset` | ein Operationsschlüssel, so wie `config list` ihn nennt | — |
| `VALUE` | ja, für `set` | was der Schlüssel laut `config list` annimmt | — |

`config` hat keine eigene Option. Die **vor** `config` geschriebenen globalen Optionen wählen die
Datei und die Form der Ausgabe:

| Globale Option | Wirkung auf `config` |
| :------------ | :----------------- |
| `--config FILE` | `show`, `set` und `unset` wirken auf `FILE` statt auf die Datei des Benutzers, auch wenn sie noch nicht existiert; `list` ignoriert sie |
| `--no-config` | `list` funktioniert wie gewohnt; `show` zeigt die Standardwerte, mit `File: none (--no-config)`; `set` und `unset` werden abgelehnt: Es gibt keine Datei, in die geschrieben werden könnte |
| `--format json` | `list` und `show` schreiben ein JSON-Objekt pro Zeile; `set` und `unset` schreiben nichts auf die Standardausgabe |
| `-q`, `--quiet` | `set` und `unset` bestätigen nicht mehr, was sie geschrieben haben |

## Welche Datei

Ohne `--config` die Datei des Benutzers:

```
$XDG_CONFIG_HOME/subedit/settings.conf
```

oder `~/.config/subedit/settings.conf`, wenn `XDG_CONFIG_HOME` nicht gesetzt, leer oder kein absoluter
Pfad ist. Es ist die Datei, die das Fenster liest und schreibt; `config show` schreibt ihren Pfad in seine erste Zeile:

<!-- example: XDG_CONFIG_HOME=/home/vous/.config subedit-cli config show | head -1 -->
```console
$ XDG_CONFIG_HOME=/home/vous/.config subedit-cli config show | head -1
File: /home/vous/.config/subedit/settings.conf (not found)
```

Wenn weder `XDG_CONFIG_HOME` noch `HOME` einen Ort ergibt, gibt es keine Datei des Benutzers: `show`, `set`
und `unset` werden abgelehnt, und `--config FILE` nennt eine.

## `config list`

**Der Katalog der Operationsschlüssel**, in der Reihenfolge der Datei: jeder Schlüssel, dann vier eingerückte
Zeilen — wozu er dient, `Accepts:` (die geschlossene Menge, oder die Form und die Grenzen), `Default:` und
`Used by:` (die Befehle von `subedit-cli` und die Aktionen des Fensters, die ihn lesen). Er liest keine Datei,
braucht keine und funktioniert mit `--no-config`.

<!-- example: subedit-cli config list | head -10 -->
```console
$ subedit-cli config list | head -10
duration-adjust.speed
    Reading speed the durations are brought to.
    Accepts: a number of characters per second, above 0, decimals allowed
    Default: 15
    Used by: subedit-cli adjust; Adjust Durations…
duration-adjust.lengthen
    Let the reading speed move an end later.
    Accepts: true or false
    Default: true
    Used by: subedit-cli adjust; Adjust Durations…
```

Er listet **42 Schlüssel** auf:

<!-- example: subedit-cli config list | grep -c '^[a-z]' -->
```console
$ subedit-cli config list | grep -c '^[a-z]'
42
```

**Zweiundzwanzig davon werden von der Befehlszeile gelesen**; die zwanzig anderen liest nur das Fenster,
und ihre Zeile `Used by:` sagt es, zusammen mit der Option von `correct`, die an ihre Stelle tritt:

| Schlüssel | Gelesen von |
| :-- | :------ |
| `duration-adjust.speed`, `duration-adjust.lengthen`, `duration-adjust.shorten` | [`adjust`](adjust.md) und `Adjust Durations…` |
| `duration-adjust.minimum-enabled`, `duration-adjust.minimum-ms` | `adjust` und `Adjust Durations…` |
| `duration-adjust.maximum-enabled`, `duration-adjust.maximum-ms` | `adjust` und `Adjust Durations…` |
| `duration-adjust.gap-enabled`, `duration-adjust.gap-ms` | `adjust` und `Adjust Durations…` |
| `correction.human`, `correction.ocr` | [`correct`](correct.md) und `Correct Texts…` |
| `correction.remove-blank` | `correct` und `Correct Texts…` |
| `correction.line-break.max-lines` | `correct` und `Correct Texts…` |
| `transcription.engine`, `transcription.executable`, `transcription.model`, `transcription.vad-model`, `transcription.language`, `transcription.suppress-notes`, `transcription.merge-gap-ms`, `transcription.max-duration-ms`, `transcription.max-characters` | [`transcribe`](transcribe.md) und `Transcribe Audio…` |
| jeder andere Schlüssel `correction.*` | nur `Correct Texts…` — `correct` nimmt `--tasks`, `--code`, `--max-length`, `--skip-length`, `--skip-lines`, `--language`, `--enable` und `--disable` aus seiner eigenen Zeile |

Die Zeilenzahlen, `correction.line-break.max-lines` und `correction.line-break.skip-max-lines`,
sind ganze Zahlen von 1 bis 1000 — die Grenzen von `correct --max-lines` —, und die Zeilenlängen,
`correction.line-break.max-length` und `correction.line-break.skip-max-length`, sind Zahlen größer als
0: `set` lehnt alles andere ab, und eine von Hand bearbeitete Datei, die einen solchen Wert enthält, behält den Standardwert.

`correct` nimmt die Aufgaben, den Code und die Zeilenlänge absichtlich aus seiner eigenen Zeile: Auf einer Befehlszeile ist
nichts vorab angekreuzt, und das Fenster misst Zeilen in ems, wo die Befehlszeile
Zeichen zählt. `correction.activations`, die Liste der Musterkästchen, die das Fenster verwaltet, ist kein
Schlüssel des Katalogs: `correct` hat `--enable` und `--disable`.

Mit `--format json` ein Objekt pro Schlüssel — `key`, `description`, `accepts`, `default`, `used_by`
und `surface`, `"both"` oder `"window-only"`:

<!-- example: subedit-cli --format json config list | head -1 -->
```console
$ subedit-cli --format json config list | head -1
{"schema":1,"command":"config","key":"duration-adjust.speed","description":"Reading speed the durations are brought to.","accepts":"a number of characters per second, above 0, decimals allowed","default":"15","used_by":"subedit-cli adjust; Adjust Durations…","surface":"both"}
```

## `config show`

**Die verwendete Datei, dann jeder Operationsschlüssel mit seinem Wert**: `File: <path> (found)`,
`(not found)` oder `(unreadable: <reason>)`, dann eine Zeile pro Schlüssel —
`[file]` und der Standardwert, wenn die Datei ihn setzt, `[default]`, wenn nicht. Ein Schlüssel, den die Datei
mit einem Wert enthält, den sie nicht lesen kann, zeigt seinen Standardwert. Anzeigeeinstellungen werden nicht aufgelistet.

<!-- example: subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --config mine/settings.conf config show | head -4 -->
```console
$ subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --config mine/settings.conf config show | head -4
duration-adjust.speed = 12
File: mine/settings.conf (found)
duration-adjust.speed = 12   [file]   (default 15)
duration-adjust.lengthen = true   [default]
duration-adjust.shorten = false   [default]
```

Eine fehlende Datei ist kein Fehler: `show` schreibt die Standardwerte, und der Code ist `0` — sowohl für die Datei
des Benutzers als auch für eine mit `--config` genannte `FILE`, die [`adjust`](adjust.md) und [`correct`](correct.md)
dagegen ablehnen (siehe [Die Einstellungsdatei](invocation.md#die-einstellungsdatei)). Mit
`--no-config`:

<!-- example: subedit-cli --no-config config show | head -2 -->
```console
$ subedit-cli --no-config config show | head -2
File: none (--no-config)
duration-adjust.speed = 15   [default]
```

Mit `--format json` zuerst ein Objekt `{"file", "exists"}` — `file` ist `null` mit `--no-config` —,
dann ein Objekt pro Schlüssel: `key`, `value`, `source` (`"file"` oder `"default"`), `default`,
`used_by`, `surface`.

<!-- example: subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --format json --config mine/settings.conf config show | head -2 -->
```console
$ subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --format json --config mine/settings.conf config show | head -2
duration-adjust.speed = 12
{"schema":1,"command":"config","file":"mine/settings.conf","exists":true}
{"schema":1,"command":"config","key":"duration-adjust.speed","value":"12","source":"file","default":"15","used_by":"subedit-cli adjust; Adjust Durations…","surface":"both"}
```

## `config set` und `config unset`

**`set` prüft den Wert mit dem Lesen der Datei selbst** — demselben, das das Fenster verwendet — und lehnt
ihn ab, wenn er nicht gelesen werden kann, bevor irgendetwas geschrieben wird. **`unset`** schreibt den Standardwert zurück,
auskommentiert: Der Schlüssel folgt dann dem Standardwert, auch wenn eine spätere Version ihn ändert. Einen
Schlüssel zurückzusetzen, der schon auf seinem Standardwert steht, gelingt und ändert nichts.

Beide **schreiben die Datei vollständig und atomar neu**: Jede bereits darin enthaltene Anzeigeeinstellung bleibt erhalten,
der Operationsblock kommt zuerst und der Anzeigeblock danach, jeder unter seiner Überschrift. **Ein Wert,
den die Datei nicht lesen konnte, und ein unbekannter Schlüssel werden nicht behalten** — das Neuschreiben ist das des Fensters, das
sie ebenfalls verwirft. **Jeder verworfene Wert wird gemeldet**, auf der Standardfehlerausgabe auf der Standardstufe, Anzeigeeinstellungen
eingeschlossen: `<key>: unreadable value '<value>' replaced by its default`. Die Datei und
ihr Verzeichnis werden angelegt, wenn sie fehlen.

Die Bestätigung geht auf die Standardfehlerausgabe, auf der Standardstufe: `KEY = VALUE` für `set`,
`KEY reset to its default (DEFAULT)` für `unset`.

<!-- example: subedit-cli --config mine/settings.conf config set duration-adjust.gap-ms 100; grep -e '^[a-z]' -e '^# [A-Z][a-z]* [a-z]*:' mine/settings.conf; subedit-cli --config mine/settings.conf config unset duration-adjust.gap-ms; grep gap-ms mine/settings.conf -->
```console
$ subedit-cli --config mine/settings.conf config set duration-adjust.gap-ms 100; grep -e '^[a-z]' -e '^# [A-Z][a-z]* [a-z]*:' mine/settings.conf; subedit-cli --config mine/settings.conf config unset duration-adjust.gap-ms; grep gap-ms mine/settings.conf
duration-adjust.gap-ms = 100
# Operation settings: shared by the window and the command line (subedit-cli reads these).
duration-adjust.gap-ms = 100
# Display preferences: the window's own, the command line never reads them.
duration-adjust.gap-ms reset to its default (0)
#duration-adjust.gap-ms = 0
#transcription.merge-gap-ms = 400
```

## Exit-Codes

| Code | Bedeutung |
| :--- | :------ |
| `0` | erledigt — einschließlich `show` bei einer fehlenden Datei und `unset` bei einem Schlüssel auf seinem Standardwert |
| `1` | Aufruffehler: kein Unterbefehl, ein fehlendes Argument, ein abgelehnter Schlüssel oder Wert, `--no-config` mit `set` oder `unset`, `--config` mit einem leeren Namen, keine Datei des Benutzers und kein `--config`; es wird nichts geschrieben |
| `2` | die Datei kann nicht gelesen oder geschrieben werden; es wird nichts geschrieben |

## Fehler

| Was ihn auslöst | Was auf die Standardfehlerausgabe geschrieben wird |
| :--------------- | :------------------------------------- |
| eine Anzeigeeinstellung | `<key> is a display preference; set it in the window` |
| ein unbekannter Schlüssel | `unknown setting <key>; subedit-cli config list names them` |
| ein Wert, den der Schlüssel nicht annimmt | `cannot read '<value>' for <key>; subedit-cli config list says what it accepts` |
| `correction.activations` | `correction.activations is managed by the window; use --enable/--disable on correct` |
| `set` oder `unset` mit `--no-config` | `--no-config has nothing to write to` |
| es lässt sich keine Datei des Benutzers finden, und kein `--config` | `no settings file could be located; give one with --config FILE` |
| `--config` mit `--no-config` | `--config excludes --no-config` |
| `--config` mit einem leeren Namen | `--config needs a file name` |
| eine Datei, die existiert und nicht gelesen werden kann, für `set` oder `unset` | `<path>: cannot be read; nothing written` |
| eine Datei, die nicht geschrieben werden kann | `<path>: cannot be written`, oder `<path>: cannot be written: permission denied` |

<!-- example: subedit-cli --config mine/settings.conf config set general.theme dark; echo "code=$?"; subedit-cli --config mine/settings.conf config set duration-adjust.speed fast; echo "code=$?"; subedit-cli --no-config config unset duration-adjust.speed; echo "code=$?" -->
```console
$ subedit-cli --config mine/settings.conf config set general.theme dark; echo "code=$?"; subedit-cli --config mine/settings.conf config set duration-adjust.speed fast; echo "code=$?"; subedit-cli --no-config config unset duration-adjust.speed; echo "code=$?"
general.theme is a display preference; set it in the window
code=1
cannot read 'fast' for duration-adjust.speed; subedit-cli config list says what it accepts
code=1
--no-config has nothing to write to
code=1
```
