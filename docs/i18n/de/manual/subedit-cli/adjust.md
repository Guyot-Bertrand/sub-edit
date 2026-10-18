# `adjust`

<!-- machine-translated -->
> Diese Seite ist eine maschinelle Übersetzung und wurde noch von keiner Leserin und keinem Leser dieser Sprache geprüft.

```
subedit-cli adjust [--speed CPS|off] [--shorten | --no-shorten] [--lengthen | --no-lengthen]
                   [--minimum TIME|off] [--maximum TIME|off] [--gap TIME|off]
                   [--range N-M|N-]
                   (--output FILE | --output-dir DIR | --in-place)
                   [--dry-run]
                   [--recursive]
                   <file>...
```

Passt die **Dauer** jedes Untertitels an eine Lesegeschwindigkeit und an Grenzen an: Ein
Untertitel, der zu kurz ist, um gelesen zu werden, wird verlängert, ein Untertitel, der den Bildschirm
zu spät verlässt, kann gekürzt werden, und zwei Untertitel überlappen sich nicht. **Nur das
Ende bewegt sich**: Den Beginn zu bewegen, würde den Untertitel verschieben, und das ist die Aufgabe von
[`shift`](shift.md). Es ist die Operation `Adjust Durations…` des
[Fensters](../subedit-gui/operations.md#adjust-durations), mit denselben Standardwerten.

## Die vier Einschränkungen und ihre Reihenfolge

Sie werden **in dieser Reihenfolge** angewendet, jede setzt das Ende, die letzte gewinnt:

1. **die Lesegeschwindigkeit** — die Dauer, die der Text braucht, bei so und so vielen Zeichen pro
   Sekunde (Tags zählen nicht, Zeilenumbrüche schon);
2. **das Minimum** — eine Dauer darunter wird auf das Minimum angehoben;
3. **das Maximum** — eine Dauer darüber wird auf das Maximum gesenkt;
4. **der Abstand** — die Zeit, die vor dem nächsten Untertitel bleibt. Er kommt zuletzt und
   **er gewinnt**: Zwei zugleich angezeigte Untertitel sind ein Fehler, den der Zuschauer sieht,
   ein etwas zu kurzer Untertitel nicht.

Der letzte Untertitel der Datei hat keinen nächsten und damit keinen Abstand einzuhalten. Der nächste
eines Untertitels ist der in der Datei, **ob von `--range` gewählt oder nicht**.

| Einschränkung | Option | Standard | Ausschalten |
| :--------- | :----- | :----- | :------- |
| Geschwindigkeit | `--speed CPS` | **15** Zeichen pro Sekunde | `--speed off` |
| Richtung der Geschwindigkeit | `--shorten`, `--no-shorten`, `--lengthen`, `--no-lengthen` | sie **verlängert** nur | — |
| Minimum | `--minimum TIME` | **1,5 s** | `--minimum off` |
| Maximum | `--maximum TIME` | **keines** | `--maximum off` |
| Abstand | `--gap TIME` | **0** | `--gap off` |

Das sind die Einstellungen von Gaupol und des Fensters beim ersten Öffnen. **`off` schaltet eine
Einschränkung aus: Sie wird nicht mehr geprüft.** Das ist nicht dasselbe, wie sie auf null zu setzen — ein Minimum von `0`
ist ein Minimum von null, ein Abstand von `0` verbietet jede Überlappung.

## Was die Einstellungsdatei liefert

**Die obigen Standardwerte sind die eingebauten.** Standardmäßig liest `adjust` die Einstellungen der Operationen aus
der Einstellungsdatei — die Werte, die sich `Adjust Durations…` im Fenster merkt oder die
[`config set`](config.md) geschrieben hat —, und sie treten an die Stelle der eingebauten Standardwerte. **Eine auf der Zeile
angegebene Option gewinnt immer**, dann die Datei, dann der eingebaute Standardwert. Siehe
[Die Einstellungsdatei](invocation.md#die-einstellungsdatei) dazu, wo die Datei liegt, sowie zu `--config` und
`--no-config`.

| Schlüssel | Was er liefert | Option, die ihn übersteuert |
| :-- | :--------------- | :----------------------- |
| `duration-adjust.speed` | die Lesegeschwindigkeit, in Zeichen pro Sekunde | `--speed CPS` |
| `duration-adjust.lengthen`, `duration-adjust.shorten` | in welche Richtung die Geschwindigkeit ein Ende bewegen darf; beide `false` schalten die Geschwindigkeit aus | `--lengthen`, `--no-lengthen`, `--shorten`, `--no-shorten`, `--speed off`; die Angabe von `--speed CPS` oder einer Richtung schaltet eine von der Datei ausgeschaltete Geschwindigkeit wieder ein, verlängernd |
| `duration-adjust.minimum-enabled`, `duration-adjust.minimum-ms` | die kürzeste Dauer, oder keine | `--minimum TIME`, `--minimum off` |
| `duration-adjust.maximum-enabled`, `duration-adjust.maximum-ms` | die längste Dauer, oder keine | `--maximum TIME`, `--maximum off` |
| `duration-adjust.gap-enabled`, `duration-adjust.gap-ms` | der kleinste Abstand, oder keiner | `--gap TIME`, `--gap off` |

**Jeder Schlüssel hat eine Option, die ihn aufhebt.** Jede Richtung hat ein Paar von Schaltern, die sie
setzen, was auch immer die Datei sagt: `--shorten` und `--no-shorten` für das Kürzen, `--lengthen` und
`--no-lengthen` für das Verlängern; ein nicht angegebener Schalter lässt die Richtung der Datei, wie sie ist.
`--maximum off` verwirft ein Maximum, das die Datei einschaltet. Mit `--format json` sagt `constraints`,
welche Einstellungen wirklich verwendet wurden, woher sie auch kamen, und `settings`, welche Datei gelesen wurde:

<!-- example: printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine/settings.conf config set duration-adjust.speed 12 2>/dev/null; subedit-cli --config mine/settings.conf --format json adjust --dry-run a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine/settings.conf config set duration-adjust.speed 12 2>/dev/null; subedit-cli --config mine/settings.conf --format json adjust --dry-run a.srt
settings: read mine/settings.conf
a.srt: adjusted the durations of 1 subtitle (dry run, nothing written)
{"schema":1,"command":"adjust","file":"a.srt","ok":true,"dry_run":true,"destination":null,"counts":{"subtitles":1,"adjusted":1,"sacrificed":{"speed":0,"minimum":0,"gap":0}},"constraints":{"speed":{"cps":"12","lengthen":true,"shorten":false},"minimum_ms":1500,"maximum_ms":null,"gap_ms":0},"settings":"mine/settings.conf","warnings":[]}
```

Die übrigen Beispiele dieser Seite werden mit `--no-config` ausgeführt, damit sie zeigen, was die eingebauten
Standardwerte tun, unabhängig von den Einstellungen der Maschine, die sie ausführt.

<!-- example: subedit-cli adjust --help -->
```console
$ subedit-cli adjust --help
Bring the duration of every subtitle within a reading speed and limits
Usage: subedit-cli adjust [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to adjust

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --speed CPS|off             Reading speed in characters per second, or off (default 15)
  --shorten                   Let the reading speed bring an end earlier
  --no-shorten                Do not let the reading speed bring an end earlier
  --lengthen                  Let the reading speed move an end later
  --no-lengthen               Do not let the reading speed move an end later
  --minimum TIME|off          Shortest duration, or off (default 1.5)
  --maximum TIME|off          Longest duration, or off (default: none)
  --gap TIME|off              Least time left before the next subtitle, or off (default 0)
  --range N-M|N-              Act only on subtitles N to M, or N to the end
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Argumente und Optionen

| Option | Erforderlich | Werte | Standard |
| :----- | :----- | :------ | :----- |
| `<file>...` | ja | ein oder mehrere Pfade | — |
| `--recursive`, `-r` | nein | ein Schalter | aus — siehe [Einen Verzeichnisbaum verarbeiten](lots.md) |
| `--speed` | nein | eine Anzahl von Zeichen pro Sekunde, **größer als null** (`15`, `12.5`), oder `off` | `15` |
| `--shorten` | nein | ein Schalter: Die Geschwindigkeit darf ein Ende auch **vorziehen** | aus |
| `--no-shorten` | nein | ein Schalter: Die Geschwindigkeit **zieht** ein Ende nicht mehr **vor**, was auch immer die Einstellungsdatei sagt | aus |
| `--lengthen` | nein | ein Schalter: Die Geschwindigkeit **schiebt** ein Ende **nach hinten**, was auch immer die Einstellungsdatei sagt | aus |
| `--no-lengthen` | nein | ein Schalter: Die Geschwindigkeit **schiebt** ein Ende nicht mehr **nach hinten** | aus |
| `--minimum` | nein | eine Dauer von null oder mehr, oder `off` | `1.5` |
| `--maximum` | nein | eine Dauer **größer als null**, oder `off` | keines |
| `--gap` | nein | eine Dauer von null oder mehr, oder `off` | `0` |
| `--range` | nein | `N-M`, oder `N-` bis zum Ende — siehe unten | die ganze Datei |
| `--output` / `--output-dir` / `--in-place` | **eine der drei**, außer mit `--dry-run` | siehe [Aufruf](invocation.md#das-ziel) | — |
| `--dry-run` | nein | ein Schalter: berechnet und sagt, schreibt nichts — siehe [Vor dem Schreiben nachsehen](invocation.md#vor-dem-schreiben-nachsehen) | aus |

Eine Dauer wird wie bei [`shift`](shift.md#eine-dauer-schreiben) geschrieben: `2.5`, `00:00:02.500`.

**Ohne jede aktive Einschränkung wird `adjust` abgelehnt** (Code `1`, nichts geschrieben): Es gäbe
nichts, woran angepasst werden könnte, und ein Befehl, der nichts tut und dabei Erfolg meldet, ist die schlechteste
Antwort. Das ist der Fall bei `--speed off --minimum off --gap off` ohne Maximum, und bei
`--no-lengthen` allein mit ausgeschaltetem Minimum und Abstand: Eine Geschwindigkeit, die kein
Ende bewegt, schränkt nichts ein.

`--shorten`, `--no-shorten`, `--lengthen` und `--no-lengthen` sagen, in welche Richtung die Geschwindigkeit ein
Ende bewegt; sie ergeben nur mit einer Geschwindigkeit Sinn, und **`--speed off` lehnt sie ab**. Ein Schalter, der zusammen mit seinem
Gegenteil angegeben wird — `--shorten --no-shorten`, `--lengthen --no-lengthen` —, wird ebenfalls abgelehnt.

### `--range`

`--range 3-7` beschränkt die Anpassung auf die Untertitel 3 bis 7, **ab 1 gezählt, Grenzen
eingeschlossen**; `--range 3-` geht bis zum letzten. Ein einzelner Untertitel wird `4-4` geschrieben.

- Ein Bereich, **der endet, bevor er beginnt**, eine `0`, ein Vorzeichen oder eine einzelne Zahl werden
  als Aufruffehler abgelehnt (Code `1`).
- **Er gilt für jede Datei des Stapels**, was nur Sinn ergibt, wenn die Dateien
  einander ähneln. Er wird **an jeder Datei** gemessen: Ein Bereich, der über diese Datei hinausgeht, wird
  **abgelehnt, bevor sie angerührt wird**, unter Nennung der Grenze —
  `range 3-9 ends after the last subtitle: the file has 4` —, diese Datei scheitert (Code `2` oder `3`, Kennung
  `range-out-of-bounds`), und die anderen laufen weiter.
- Die Zählungen des Berichts betreffen nur die gewählten Untertitel.

## Was geopfert wird

Zwei Einschränkungen können sich beim selben Untertitel widersprechen: Ein Minimum von 1,5 s passt nicht
in die 0,8 s, die der nächste Untertitel lässt. **Die Reihenfolge entscheidet, und der Bericht sagt,
was nachgegeben hat** — was Gaupol nicht tut: Es bricht sie stillschweigend.

Jede Einschränkung wird nur genannt, wenn **mindestens ein Untertitel sie nicht erhalten hat**, nachdem
die anderen angewendet wurden. Das Maximum hat keine Zählung: Es wird vor dem Abstand angewendet, der
nur kürzen kann, also gilt es immer. Ein Minimum über dem Maximum weicht dem Maximum und wird
als Minimum gezählt.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --output ajuste.srt courts.srt; grep -e '-->' ajuste.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --output ajuste.srt courts.srt; grep -e '-->' ajuste.srt
courts.srt: adjusted the durations of 3 subtitles; could not satisfy the minimum duration in 1 subtitle -> ajuste.srt
00:00:00,000 --> 00:00:02,000
00:00:10,000 --> 00:00:14,000
00:00:14,200 --> 00:00:15,000
00:00:15,000 --> 00:00:16,500
```

Der dritte Untertitel würde gern bei 15,7 s enden, um 1,5 s zu dauern, aber der vierte beginnt bei
15,0 s: **Der Abstand gewinnt**, er endet bei 15,0 s, und der Bericht nennt das Minimum.

Den Abstand auszuschalten, lässt das Minimum gewinnen — und die beiden Untertitel überlappen sich:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --gap off --output libre.srt courts.srt; grep -e '-->' libre.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --gap off --output libre.srt courts.srt; grep -e '-->' libre.srt
courts.srt: adjusted the durations of 3 subtitles -> libre.srt
00:00:00,000 --> 00:00:02,000
00:00:10,000 --> 00:00:14,000
00:00:14,200 --> 00:00:15,700
00:00:15,000 --> 00:00:16,500
```

Und nichts wird geopfert oder angepasst, wenn die Datei schon innerhalb ihrer Grenzen liegt:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --speed off --minimum off --output intact.srt courts.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --speed off --minimum off --output intact.srt courts.srt
courts.srt: no duration to adjust -> intact.srt
```

Die Datei wird so geschrieben, wie sie ist: **Ein angegebenes Ziel ist ein geschriebenes Ziel**.

## Ausgabe

**Standardausgabe** — nichts: Das Ergebnis ist die geschriebene Datei.

| Stufe | Was auf der Fehlerausgabe hinzukommt |
| :----- | :------------------------------------- |
| 1 | `<path>: adjusted the durations of N subtitles; could not satisfy …  -> <destination>`, oder `<path>: no duration to adjust -> <destination>` — und zuerst die gelesene Einstellungsdatei — `settings: read <path>` — siehe [Die Einstellungsdatei](invocation.md#die-einstellungsdatei) |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — das Format, die Kodierung, die Marke und die Zeilenenden der gelesenen Datei, so wiederhergestellt, wie sie waren |
| 3 | `<path>: N bytes read, M written`, dann **jede Diagnose beim Lesen** — siehe [Aufruf](invocation.md#diagnosen-beim-lesen) |

Der Satz ist der des Fensters: `N` ist die Anzahl der Untertitel, deren Ende sich bewegt hat, und der
Rest, der fehlt, wenn nichts nachgegeben hat, zählt auf, was nicht eingehalten wurde — `the reading speed`,
`the minimum duration`, `the gap` —, mit der Anzahl der betroffenen Untertitel.

### In JSON

Mit [`--format json`](invocation.md#ausgabe-die-ein-skript-lesen-kann) enthält das Objekt jeder Datei
neben der Hülle:

| Schlüssel | Inhalt |
| :-- | :------ |
| `counts.subtitles` | die **anvisierten** Untertitel: die ganze Datei oder der Bereich |
| `counts.adjusted` | diejenigen, deren Ende sich bewegt hat |
| `counts.sacrificed.speed`, `.minimum`, `.gap` | die Untertitel, die diese Einschränkung immer noch nicht einhalten, nachdem die anderen angewendet wurden, **einschließlich derer, deren Ende sich nicht bewegt hat** |
| `constraints.speed` | `{"cps", "lengthen", "shorten"}` — die Zeichen pro Sekunde **als Zeichenkette** (`"15"`, `"12.5"`) — oder `null`, wenn ausgeschaltet |
| `constraints.minimum_ms`, `.maximum_ms`, `.gap_ms` | die verwendeten Dauern, in **ganzen Millisekunden**, oder `null`, wenn ausgeschaltet |
| `settings` | die gelesene Einstellungsdatei, oder `null`, wenn keine gelesen wurde |

`constraints` sagt, **welche Einstellungen wirklich verwendet wurden**, Standardwerte eingeschlossen: Zwei Berechnungen
über dieselben Untertitel lassen sich dann vergleichen, ohne die Befehlszeile zu lesen.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config --format json adjust --dry-run --maximum 1.8 courts.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config --format json adjust --dry-run --maximum 1.8 courts.srt
courts.srt: adjusted the durations of 4 subtitles; could not satisfy the reading speed in 1 subtitle, the minimum duration in 1 subtitle (dry run, nothing written)
{"schema":1,"command":"adjust","file":"courts.srt","ok":true,"dry_run":true,"destination":null,"counts":{"subtitles":4,"adjusted":4,"sacrificed":{"speed":1,"minimum":1,"gap":0}},"constraints":{"speed":{"cps":"15","lengthen":true,"shorten":false},"minimum_ms":1500,"maximum_ms":1800,"gap_ms":0},"settings":null,"warnings":[]}
```

## Exit-Codes

Die des Werkzeugs: `0`, wenn jede Datei verarbeitet wurde, `2`, wenn keine, `3`, wenn nur
einige, `1` bei einem Aufruffehler. Eine Datei, die schon innerhalb ihrer Grenzen liegt, gilt als verarbeitet.

## Fehler

| Was ihn auslöst | Meldung |
| :------------------ | :------ |
| eine mit `--config` genannte Datei, die nicht existiert — fehlt die eigene Datei des Benutzers, gelten die Standardwerte | `settings file not found: <FILE>` |
| keine aktive Einschränkung | `adjust has nothing to adjust to: every constraint is off, or the reading speed is told to move no end` |
| eine Geschwindigkeit, die keine ist | `--speed: "…" is not a reading speed: expected a number of characters per second above zero, like 15 or 12.5, or off` |
| ein Richtungsschalter mit `--speed off` | `--lengthen, --no-lengthen, --shorten and --no-shorten say which way the reading speed may move an end, and --speed off switches the reading speed off` |
| ein Richtungsschalter mit seinem Gegenteil | `--shorten and --no-shorten say opposite things; give one of them` (ebenso `--lengthen` und `--no-lengthen`) |
| eine Dauer, die keine ist | `--minimum: "…" is not a time: …` (ebenso `--maximum`, `--gap`) |
| negative Dauer | `--minimum: "-1" is not a duration of zero or more` (ebenso `--gap`) |
| Maximum null oder negativ | `--maximum: "0" is not a duration above zero` |
| falsch geschriebener Bereich | `--range: "…" is not a range: write it N-M, or N- to go to the end, counted from 1 and inclusive` |
| Bereich, der endet, bevor er beginnt | `--range: "5-3" is not a range: it ends before it starts` |
| Bereich außerhalb der Datei | `<path>: range N-M starts after the last subtitle: the file has K`, oder `… ends after …` |

Die Fehler des Ziels sind den Unterbefehlen gemeinsam, die schreiben:
siehe [Aufruf](invocation.md#das-ziel).
