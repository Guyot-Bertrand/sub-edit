# `config`

<!-- machine-translated -->
> Deze pagina is een machinevertaling en is nog niet nagelezen door een lezer van de taal.

```
subedit-cli [--config FILE | --no-config] [--format text|json] config list
subedit-cli [--config FILE | --no-config] [--format text|json] config show
subedit-cli [--config FILE] config set KEY VALUE
subedit-cli [--config FILE] config unset KEY
```

Toont, weergeeft en wijzigt de **bewerkingsinstellingen**: het deel van het instellingenbestand dat het
venster en de opdrachtregel delen. `Adjust Durations…` en `Correct Texts…` onthouden hun
waarden daar, en [`adjust`](adjust.md) en [`correct`](correct.md) lezen ze standaard — zie
[Het instellingenbestand](invocation.md#het-instellingenbestand).

**Wat het niet doet:** het leest of schrijft nooit een **weergavevoorkeur** (de
geometrie van het venster, de kolommen, het thema, de zoekopties, de spellingcontrole, de video…). Die blijven de
zaak van het venster, worden in het venster ingesteld en beschreven in [de handleiding ervan](../subedit-gui/preferences.md#het-bestand).
Het raakt ook nooit een ondertitelbestand aan.

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

## Subcommando's

| Subcommando | Wat het doet | Bestand |
| :---------- | :----------- | :--- |
| `config list` | schrijft elke bewerkingssleutel, met waarvoor hij dient, wat hij accepteert, zijn standaardwaarde en wie hem gebruikt — **geen waarde** | leest er geen |
| `config show` | schrijft het gebruikte bestand en of het bestaat, daarna elke bewerkingssleutel met zijn waarde, waar de waarde vandaan komt, en zijn standaardwaarde | leest het |
| `config set KEY VALUE` | controleert `VALUE`, schrijft het bestand daarna opnieuw met `KEY = VALUE`, waarbij elke andere instelling behouden blijft | leest en schrijft het |
| `config unset KEY` | zet `KEY` terug op zijn standaardwaarde, uitgecommentarieerd geschreven zoals het bestand een standaardwaarde altijd schrijft | leest en schrijft het |

| Optie | Verplicht | Waarden | Standaard |
| :----- | :------- | :----- | :------ |
| `KEY` | ja, voor `set` en `unset` | een bewerkingssleutel, zoals `config list` hem noemt | — |
| `VALUE` | ja, voor `set` | wat `config list` zegt dat de sleutel accepteert | — |

`config` heeft geen eigen optie. De algemene opties die **vóór** `config` worden geschreven, kiezen het
bestand en de vorm van de uitvoer:

| Algemene optie | Effect op `config` |
| :------------ | :----------------- |
| `--config FILE` | `show`, `set` en `unset` werken op `FILE` in plaats van op het bestand van de gebruiker, zelfs als het nog niet bestaat; `list` negeert het |
| `--no-config` | `list` werkt zoals gewoonlijk; `show` toont de standaardwaarden, met `File: none (--no-config)`; `set` en `unset` worden geweigerd: er is geen bestand om naar te schrijven |
| `--format json` | `list` en `show` schrijven één JSON-object per regel; `set` en `unset` schrijven niets op de standaarduitvoer |
| `-q`, `--quiet` | `set` en `unset` bevestigen niet meer wat ze hebben geschreven |

## Welk bestand

Zonder `--config` het bestand van de gebruiker:

```
$XDG_CONFIG_HOME/subedit/settings.conf
```

of `~/.config/subedit/settings.conf` als `XDG_CONFIG_HOME` niet is ingesteld, leeg is of geen absoluut
pad is. Het is het bestand dat het venster leest en schrijft; `config show` schrijft het pad op zijn eerste regel:

<!-- example: XDG_CONFIG_HOME=/home/vous/.config subedit-cli config show | head -1 -->
```console
$ XDG_CONFIG_HOME=/home/vous/.config subedit-cli config show | head -1
File: /home/vous/.config/subedit/settings.conf (not found)
```

Als noch `XDG_CONFIG_HOME` noch `HOME` een plaats opgeeft, is er geen bestand van de gebruiker: `show`, `set`
en `unset` worden geweigerd, en `--config FILE` noemt er een.

## `config list`

**De catalogus van de bewerkingssleutels**, in de volgorde van het bestand: elke sleutel, daarna vier ingesprongen
regels — waarvoor hij dient, `Accepts:` (de gesloten verzameling, of de vorm en de grenzen), `Default:` en
`Used by:` (de `subedit-cli`-commando's en de vensteracties die hem lezen). Het leest geen bestand en
heeft er geen nodig, en werkt met `--no-config`.

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

Het somt **45 sleutels** op:

<!-- example: subedit-cli config list | grep -c '^[a-z]' -->
```console
$ subedit-cli config list | grep -c '^[a-z]'
45
```

**Vijfentwintig ervan worden door de opdrachtregel gelezen**; de twintig andere worden alleen door het venster gelezen,
en hun regel `Used by:` zegt het, met de optie van `correct` die hun plaats inneemt:

| Sleutel | Gelezen door |
| :-- | :------ |
| `duration-adjust.speed`, `duration-adjust.lengthen`, `duration-adjust.shorten` | [`adjust`](adjust.md) en `Adjust Durations…` |
| `duration-adjust.minimum-enabled`, `duration-adjust.minimum-ms` | `adjust` en `Adjust Durations…` |
| `duration-adjust.maximum-enabled`, `duration-adjust.maximum-ms` | `adjust` en `Adjust Durations…` |
| `duration-adjust.gap-enabled`, `duration-adjust.gap-ms` | `adjust` en `Adjust Durations…` |
| `correction.human`, `correction.ocr` | [`correct`](correct.md) en `Correct Texts…` |
| `correction.remove-blank` | `correct` en `Correct Texts…` |
| `correction.line-break.max-lines` | `correct` en `Correct Texts…` |
| `transcription.engine`, `transcription.executable`, `transcription.model`, `transcription.vad-model`, `transcription.language`, `transcription.suppress-notes`, `transcription.merge-gap-ms`, `transcription.max-duration-ms`, `transcription.max-characters` | [`transcribe`](transcribe.md) en `Transcribe Audio…` |
| `alignment.engine`, `alignment.min-match`, `alignment.doubt-gap` | [`sync`](sync.md) |
| elke andere sleutel `correction.*` | alleen `Correct Texts…` — `correct` neemt `--tasks`, `--code`, `--max-length`, `--skip-length`, `--skip-lines`, `--language`, `--enable` en `--disable` van zijn eigen regel |

De aantallen regels, `correction.line-break.max-lines` en `correction.line-break.skip-max-lines`,
zijn gehele getallen van 1 tot 1000 — de grenzen van `correct --max-lines` —, en de regellengtes,
`correction.line-break.max-length` en `correction.line-break.skip-max-length`, zijn getallen boven
0: `set` weigert de rest, en een met de hand bewerkt bestand dat er een bevat, behoudt de standaardwaarde.

`correct` neemt de taken, de code en de regellengte met opzet van zijn eigen regel: op een opdrachtregel
is vooraf niets aangevinkt, en het venster meet regels in em terwijl de opdrachtregel
tekens telt. `correction.activations`, de lijst van patroonvakjes die het venster beheert, is geen
sleutel van de catalogus: `correct` heeft `--enable` en `--disable`.

Met `--format json` één object per sleutel — `key`, `description`, `accepts`, `default`, `used_by`,
en `surface`, `"both"` of `"window-only"`:

<!-- example: subedit-cli --format json config list | head -1 -->
```console
$ subedit-cli --format json config list | head -1
{"schema":1,"command":"config","key":"duration-adjust.speed","description":"Reading speed the durations are brought to.","accepts":"a number of characters per second, above 0, decimals allowed","default":"15","used_by":"subedit-cli adjust; Adjust Durations…","surface":"both"}
```

## `config show`

**Het gebruikte bestand, daarna elke bewerkingssleutel met zijn waarde**: `File: <path> (found)`,
`(not found)` of `(unreadable: <reason>)`, daarna één regel per sleutel —
`[file]` en de standaardwaarde als het bestand hem instelt, `[default]` als het dat niet doet. Een sleutel die het bestand
bevat met een waarde die het niet kan lezen, toont zijn standaardwaarde. Weergavevoorkeuren worden niet getoond.

<!-- example: subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --config mine/settings.conf config show | head -4 -->
```console
$ subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --config mine/settings.conf config show | head -4
duration-adjust.speed = 12
File: mine/settings.conf (found)
duration-adjust.speed = 12   [file]   (default 15)
duration-adjust.lengthen = true   [default]
duration-adjust.shorten = false   [default]
```

Een ontbrekend bestand is geen fout: `show` schrijft de standaardwaarden, en de code is `0` — het bestand
van de gebruiker, anders dan een met `--config` genoemd `FILE`, dat [`adjust`](adjust.md) en [`correct`](correct.md)
juist weigeren (zie [Het instellingenbestand](invocation.md#het-instellingenbestand)). Met
`--no-config`:

<!-- example: subedit-cli --no-config config show | head -2 -->
```console
$ subedit-cli --no-config config show | head -2
File: none (--no-config)
duration-adjust.speed = 15   [default]
```

Met `--format json` een eerste object `{"file", "exists"}` — `file` is `null` met `--no-config` —
daarna één object per sleutel: `key`, `value`, `source` (`"file"` of `"default"`), `default`,
`used_by`, `surface`.

<!-- example: subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --format json --config mine/settings.conf config show | head -2 -->
```console
$ subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --format json --config mine/settings.conf config show | head -2
duration-adjust.speed = 12
{"schema":1,"command":"config","file":"mine/settings.conf","exists":true}
{"schema":1,"command":"config","key":"duration-adjust.speed","value":"12","source":"file","default":"15","used_by":"subedit-cli adjust; Adjust Durations…","surface":"both"}
```

## `config set` en `config unset`

**`set` controleert de waarde met de eigen lezer van het bestand** — de lezer die het venster gebruikt — en weigert
haar als ze niet kan worden gelezen, voordat er iets is geschreven. **`unset`** schrijft de standaardwaarde terug,
uitgecommentarieerd: de sleutel volgt dan de standaardwaarde, ook als een latere versie die wijzigt. Een
sleutel die al op zijn standaardwaarde staat, ongedaan maken lukt en wijzigt niets.

Beide **schrijven het bestand geheel en atomair opnieuw**: elke weergavevoorkeur die er al in staat, blijft behouden,
het bewerkingsblok komt eerst en het weergaveblok daarna, elk onder zijn banier. **Een waarde
die het bestand niet kon lezen, en een onbekende sleutel, blijven niet behouden** — het herschrijven is dat van het venster, dat ze
ook laat vallen. **Elke vervallen waarde wordt gemeld**, op de standaardfoutuitvoer op het standaardniveau, weergavevoorkeuren
inbegrepen: `<key>: unreadable value '<value>' replaced by its default`. Het bestand en
zijn map worden aangemaakt als ze ontbreken.

De bevestiging gaat naar de foutuitvoer, op het standaardniveau: `KEY = VALUE` voor `set`,
`KEY reset to its default (DEFAULT)` voor `unset`.

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

## Afsluitcodes

| Code | Betekenis |
| :--- | :------ |
| `0` | gedaan — ook `show` op een ontbrekend bestand en `unset` op een sleutel die op zijn standaardwaarde staat |
| `1` | gebruiksfout: geen subcommando, een ontbrekend argument, een geweigerde sleutel of waarde, `--no-config` met `set` of `unset`, `--config` met een lege naam, geen bestand van de gebruiker en geen `--config`; er wordt niets geschreven |
| `2` | het bestand kan niet worden gelezen of geschreven; er wordt niets geschreven |

## Fouten

| Wat het veroorzaakt | Wat er op de foutuitvoer wordt geschreven |
| :--------------- | :------------------------------------- |
| een weergavevoorkeur | `<key> is a display preference; set it in the window` |
| een onbekende sleutel | `unknown setting <key>; subedit-cli config list names them` |
| een waarde die de sleutel niet accepteert | `cannot read '<value>' for <key>; subedit-cli config list says what it accepts` |
| `correction.activations` | `correction.activations is managed by the window; use --enable/--disable on correct` |
| `set` of `unset` met `--no-config` | `--no-config has nothing to write to` |
| er kan geen bestand van de gebruiker worden gevonden, en geen `--config` | `no settings file could be located; give one with --config FILE` |
| `--config` met `--no-config` | `--config excludes --no-config` |
| `--config` met een lege naam | `--config needs a file name` |
| een bestand dat bestaat en niet kan worden gelezen, voor `set` of `unset` | `<path>: cannot be read; nothing written` |
| een bestand dat niet kan worden geschreven | `<path>: cannot be written`, of `<path>: cannot be written: permission denied` |

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
