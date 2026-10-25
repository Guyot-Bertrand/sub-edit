# `config`

<!-- machine-translated -->
> Ez az oldal gépi fordítás, amelyet a nyelv egyik olvasója még nem nézett át.

```
subedit-cli [--config FILE | --no-config] [--format text|json] config list
subedit-cli [--config FILE | --no-config] [--format text|json] config show
subedit-cli [--config FILE] config set KEY VALUE
subedit-cli [--config FILE] config unset KEY
```
Listázza, megjeleníti és módosítja a **műveleti beállításokat**: a beállításfájlnak azt a részét, amelyet az
ablak és a parancssor közösen használ. Az `Adjust Durations…` és a `Correct Texts…` ott jegyzi meg az
értékeit, az [`adjust`](adjust.md) és a [`correct`](correct.md) pedig alapértelmezés szerint onnan olvassa őket — lásd:
[A beállításfájl](invocation.md#a-beállításfájl).

**Amit nem csinál:** soha nem olvas és nem ír **megjelenítési beállítást** (az ablak
geometriáját, oszlopait, témáját, keresési beállításait, a helyesírás-ellenőrzést, a videót…). Ezek az ablak
dolgai, az ablakban állíthatók, és [a kézikönyvében](../subedit-gui/preferences.md#a-fájl) vannak leírva.
Feliratfájlhoz sem nyúl soha.

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
## Alparancsok

| Alparancs | Mit csinál | Fájl |
| :---------- | :----------- | :--- |
| `config list` | kiírja az összes műveleti kulcsot, hogy mire való, mit fogad el, mi az alapértelmezése és ki használja — **érték nélkül** | egyet sem olvas |
| `config show` | kiírja a használt fájlt és azt, hogy létezik-e, majd az összes műveleti kulcsot az értékével, az érték forrásával és az alapértelmezéssel | olvassa |
| `config set KEY VALUE` | ellenőrzi a `VALUE` értéket, majd újraírja a fájlt a `KEY = VALUE` sorral, minden más beállítást megtartva | olvassa és írja |
| `config unset KEY` | a `KEY` kulcsot visszaállítja az alapértelmezésére, kikommentezve írva, ahogyan a fájl mindig az alapértelmezést írja | olvassa és írja |

| Kapcsoló | Kötelező | Értékek | Alapértelmezés |
| :----- | :------- | :----- | :------ |
| `KEY` | igen, a `set` és az `unset` esetén | műveleti kulcs, ahogyan a `config list` megnevezi | — |
| `VALUE` | igen, a `set` esetén | amit a `config list` szerint a kulcs elfogad | — |

A `config`-nak nincs saját kapcsolója. A `config` **elé** írt globális kapcsolók választják ki a
fájlt és a kimenet alakját:

| Globális kapcsoló | Hatása a `config` alparancsra |
| :------------ | :----------------- |
| `--config FILE` | a `show`, a `set` és az `unset` a `FILE` fájlon működik a felhasználó fájlja helyett, akkor is, ha az még nem létezik; a `list` figyelmen kívül hagyja |
| `--no-config` | a `list` a szokott módon működik; a `show` az alapértelmezéseket mutatja, `File: none (--no-config)` sorral; a `set` és az `unset` elutasításra kerül: nincs fájl, amelybe írni lehetne |
| `--format json` | a `list` és a `show` soronként egy JSON-objektumot ír; a `set` és az `unset` semmit sem ír a szabványos kimenetre |
| `-q`, `--quiet` | a `set` és az `unset` többé nem erősíti meg, amit írt |

## Melyik fájl

`--config` nélkül a felhasználó fájlja:

```
$XDG_CONFIG_HOME/subedit/settings.conf
```
vagy a `~/.config/subedit/settings.conf`, ha az `XDG_CONFIG_HOME` nincs beállítva, üres, vagy nem abszolút
útvonal. Ez az a fájl, amelyet az ablak olvas és ír; a `config show` az első sorában kiírja az útvonalát:

<!-- example: XDG_CONFIG_HOME=/home/vous/.config subedit-cli config show | head -1 -->
```console
$ XDG_CONFIG_HOME=/home/vous/.config subedit-cli config show | head -1
File: /home/vous/.config/subedit/settings.conf (not found)
```
Ha sem az `XDG_CONFIG_HOME`, sem a `HOME` nem ad helyet, nincs felhasználói fájl: a `show`, a `set`
és az `unset` elutasításra kerül, és a `--config FILE` nevezhet meg egyet.

## `config list`

**A műveleti kulcsok katalógusa**, a fájl sorrendjében: minden kulcs, majd négy behúzott
sor — mire való, `Accepts:` (a zárt halmaz, vagy az alak és a határok), `Default:` és
`Used by:` (az azt olvasó `subedit-cli` parancsok és ablakműveletek). Nem olvas fájlt, és
nincs is rá szüksége, és a `--no-config` mellett is működik.

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
**42 kulcsot** listáz:

<!-- example: subedit-cli config list | grep -c '^[a-z]' -->
```console
$ subedit-cli config list | grep -c '^[a-z]'
42
```
**Huszonkettőt közülük a parancssor olvas**; a másik húszat csak az ablak olvassa,
és a `Used by:` soruk ezt mondja, megadva a `correct` azon kapcsolóját, amely a helyükre lép:

| Kulcs | Ki olvassa |
| :-- | :------ |
| `duration-adjust.speed`, `duration-adjust.lengthen`, `duration-adjust.shorten` | [`adjust`](adjust.md) és `Adjust Durations…` |
| `duration-adjust.minimum-enabled`, `duration-adjust.minimum-ms` | `adjust` és `Adjust Durations…` |
| `duration-adjust.maximum-enabled`, `duration-adjust.maximum-ms` | `adjust` és `Adjust Durations…` |
| `duration-adjust.gap-enabled`, `duration-adjust.gap-ms` | `adjust` és `Adjust Durations…` |
| `correction.human`, `correction.ocr` | [`correct`](correct.md) és `Correct Texts…` |
| `correction.remove-blank` | `correct` és `Correct Texts…` |
| `correction.line-break.max-lines` | `correct` és `Correct Texts…` |
| `transcription.engine`, `transcription.executable`, `transcription.model`, `transcription.vad-model`, `transcription.language`, `transcription.suppress-notes`, `transcription.merge-gap-ms`, `transcription.max-duration-ms`, `transcription.max-characters` | [`transcribe`](transcribe.md) és `Transcribe Audio…` |
| minden más `correction.*` kulcs | csak a `Correct Texts…` — a `correct` a `--tasks`, `--code`, `--max-length`, `--skip-length`, `--skip-lines`, `--language`, `--enable` és `--disable` kapcsolót a saját parancssorából veszi |

A sorok számai, a `correction.line-break.max-lines` és a `correction.line-break.skip-max-lines`,
1 és 1000 közötti egész számok — a `correct --max-lines` határai —, a sorhosszak, a
`correction.line-break.max-length` és a `correction.line-break.skip-max-length`, pedig nullánál nagyobb
számok: a többit a `set` elutasítja, és a kézzel szerkesztett fájl, amely ilyet tartalmaz, az alapértelmezést tartja meg.

A `correct` a feladatokat, a kódot és a sorhosszt szándékosan a saját parancssorából veszi: a parancssorban semmi sincs
előre bejelölve, és az ablak a sorokat emben méri, ahol a parancssor karaktereket számol. A `correction.activations`, az
ablak által kezelt mintamezők listája, nem katalóguskulcs: a `correct` rendelkezik az `--enable` és a `--disable` kapcsolóval.

A `--format json` esetén kulcsonként egy objektum — `key`, `description`, `accepts`, `default`, `used_by`
és `surface`, `"both"` vagy `"window-only"`:

<!-- example: subedit-cli --format json config list | head -1 -->
```console
$ subedit-cli --format json config list | head -1
{"schema":1,"command":"config","key":"duration-adjust.speed","description":"Reading speed the durations are brought to.","accepts":"a number of characters per second, above 0, decimals allowed","default":"15","used_by":"subedit-cli adjust; Adjust Durations…","surface":"both"}
```
## `config show`

**A használt fájl, majd minden műveleti kulcs az értékével**: `File: <path> (found)`,
`(not found)` vagy `(unreadable: <reason>)`, majd kulcsonként egy sor —
`[file]` és az alapértelmezés, ha a fájl beállítja, `[default]`, ha nem. Az a kulcs, amelyet a fájl
olvashatatlan értékkel tartalmaz, az alapértelmezését mutatja. A megjelenítési beállítások nincsenek felsorolva.

<!-- example: subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --config mine/settings.conf config show | head -4 -->
```console
$ subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --config mine/settings.conf config show | head -4
duration-adjust.speed = 12
File: mine/settings.conf (found)
duration-adjust.speed = 12   [file]   (default 15)
duration-adjust.lengthen = true   [default]
duration-adjust.shorten = false   [default]
```
A hiányzó fájl nem hiba: a `show` az alapértelmezéseket írja, és a kód `0` — a felhasználó
fájljára és a `--config` által megnevezett `FILE` fájlra is, amelyet az [`adjust`](adjust.md) és a [`correct`](correct.md) ezzel szemben
elutasít (lásd: [A beállításfájl](invocation.md#a-beállításfájl)). A
`--no-config` esetén:

<!-- example: subedit-cli --no-config config show | head -2 -->
```console
$ subedit-cli --no-config config show | head -2
File: none (--no-config)
duration-adjust.speed = 15   [default]
```
A `--format json` esetén egy első `{"file", "exists"}` objektum — a `file` értéke `null` a `--no-config` mellett —,
majd kulcsonként egy objektum: `key`, `value`, `source` (`"file"` vagy `"default"`), `default`,
`used_by`, `surface`.

<!-- example: subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --format json --config mine/settings.conf config show | head -2 -->
```console
$ subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --format json --config mine/settings.conf config show | head -2
duration-adjust.speed = 12
{"schema":1,"command":"config","file":"mine/settings.conf","exists":true}
{"schema":1,"command":"config","key":"duration-adjust.speed","value":"12","source":"file","default":"15","used_by":"subedit-cli adjust; Adjust Durations…","surface":"both"}
```
## `config set` és `config unset`

**A `set` a fájl saját olvasójával ellenőrzi az értéket** — ugyanazzal, amelyet az ablak használ —, és elutasítja,
ha nem olvasható, mielőtt bármi kiíródna. **Az `unset`** visszaírja az alapértelmezést,
kikommentezve: a kulcs ekkor az alapértelmezést követi akkor is, ha egy későbbi verzió megváltoztatja. Az alapértelmezésén lévő
kulcs visszaállítása sikeres, és semmit sem változtat.

Mindkettő **egészében és atomi módon írja újra a fájlt**: a benne lévő megjelenítési beállítások mind megmaradnak,
a műveleti blokk kerül előre, a megjelenítési blokk másodiknak, mindegyik a saját fejléce alatt. **A fájl által nem olvasható érték
és az ismeretlen kulcs nem marad meg** — az újraírás az ablaké, amely ezeket is eldobja. **Minden eldobott értéket jelez a program**, a szabványos hibakimeneten az alapértelmezett szinten, a megjelenítési
beállításokat is beleértve: `<key>: unreadable value '<value>' replaced by its default`. A fájl és
a könyvtára létrejön, ha hiányzik.

A megerősítés az alapértelmezett szinten a szabványos hibakimenetre megy: `KEY = VALUE` a `set` esetén,
`KEY reset to its default (DEFAULT)` az `unset` esetén.

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
## Kilépési kódok

| Kód | Jelentés |
| :--- | :------ |
| `0` | kész — a hiányzó fájlon végzett `show` és az alapértelmezésén lévő kulcson végzett `unset` is |
| `1` | használati hiba: nincs alparancs, hiányzó argumentum, elutasított kulcs vagy érték, `--no-config` a `set` vagy az `unset` mellett, üres nevű `--config`, nincs felhasználói fájl és nincs `--config`; semmi sem íródik ki |
| `2` | a fájl nem olvasható vagy nem írható; semmi sem íródik ki |

## Hibák

| Mi váltja ki | Mi íródik ki a szabványos hibakimenetre |
| :--------------- | :------------------------------------- |
| megjelenítési beállítás | `<key> is a display preference; set it in the window` |
| ismeretlen kulcs | `unknown setting <key>; subedit-cli config list names them` |
| érték, amelyet a kulcs nem fogad el | `cannot read '<value>' for <key>; subedit-cli config list says what it accepts` |
| `correction.activations` | `correction.activations is managed by the window; use --enable/--disable on correct` |
| `set` vagy `unset` a `--no-config` mellett | `--no-config has nothing to write to` |
| nem található felhasználói fájl, és nincs `--config` | `no settings file could be located; give one with --config FILE` |
| `--config` a `--no-config` mellett | `--config excludes --no-config` |
| `--config` üres névvel | `--config needs a file name` |
| létező, de nem olvasható fájl, a `set` vagy az `unset` esetén | `<path>: cannot be read; nothing written` |
| nem írható fájl | `<path>: cannot be written`, vagy `<path>: cannot be written: permission denied` |

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
