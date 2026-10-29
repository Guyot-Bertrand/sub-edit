# `config`

<!-- machine-translated -->
> Þessi síða er vélþýðing sem enginn lesandi tungumálsins hefur enn farið yfir.

```
subedit-cli [--config FILE | --no-config] [--format text|json] config list
subedit-cli [--config FILE | --no-config] [--format text|json] config show
subedit-cli [--config FILE] config set KEY VALUE
subedit-cli [--config FILE] config unset KEY
```
Telur upp, sýnir og breytir **stillingum aðgerða**: þeim hluta stillingaskrárinnar sem
glugginn og skipanalínan deila. `Adjust Durations…` og `Correct Texts…` muna gildi sín
þar, og [`adjust`](adjust.md) og [`correct`](correct.md) lesa þau sjálfgefið — sjá
[Stillingaskráin](invocation.md#stillingaskráin).

**Það sem hún gerir ekki:** hún les aldrei né skrifar **birtingarstillingu** (stærð og
staðsetningu gluggans, dálka, þema, leitarvalkosti, stafsetningarathugun, myndskeið…). Þær eru áfram
mál gluggans, stilltar í glugganum og lýst í [handbók þess](../subedit-gui/preferences.md#skráin).
Hún snertir heldur aldrei skjátextaskrá.

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

## Undirskipanir

| Undirskipun | Hvað hún gerir | Skrá |
| :---------- | :----------- | :--- |
| `config list` | skrifar hvern aðgerðarlykil, með því til hvers hann er, hvað hann tekur við, sjálfgefnu gildi hans og hver notar hann — **ekkert gildi** | les enga |
| `config show` | skrifar skrána sem er notuð og hvort hún er til, síðan hvern aðgerðarlykil með gildi hans, hvaðan gildið kemur og sjálfgefnu gildi hans | les hana |
| `config set KEY VALUE` | athugar `VALUE`, skrifar síðan skrána upp á nýtt með `KEY = VALUE`, og heldur öllum öðrum stillingum | les hana og skrifar |
| `config unset KEY` | setur `KEY` aftur á sjálfgefið gildi, skrifað út kommentað eins og skráin skrifar alltaf sjálfgefið gildi | les hana og skrifar |

| Valkostur | Nauðsynlegt | Gildi | Sjálfgefið |
| :----- | :------- | :----- | :------ |
| `KEY` | já, fyrir `set` og `unset` | aðgerðarlykill, eins og `config list` nefnir hann | — |
| `VALUE` | já, fyrir `set` | það sem `config list` segir að lykillinn taki við | — |

`config` hefur engan eigin valkost. Altæku valkostirnir sem eru skrifaðir **á undan** `config` velja
skrána og mynd úttaksins:

| Altækur valkostur | Áhrif á `config` |
| :------------ | :----------------- |
| `--config FILE` | `show`, `set` og `unset` verka á `FILE` í stað skrár notandans, jafnvel þótt hún sé ekki enn til; `list` hunsar hann |
| `--no-config` | `list` virkar sem fyrr; `show` sýnir sjálfgefnu gildin, með `File: none (--no-config)`; `set` og `unset` er hafnað: engin skrá er til að skrifa í |
| `--format json` | `list` og `show` skrifa einn JSON-hlut í línu; `set` og `unset` skrifa ekkert á staðalúttak |
| `-q`, `--quiet` | `set` og `unset` staðfesta ekki lengur það sem þau skrifuðu |

## Hvaða skrá

Án `--config` er það skrá notandans:

```
$XDG_CONFIG_HOME/subedit/settings.conf
```

eða `~/.config/subedit/settings.conf` þegar `XDG_CONFIG_HOME` er óstillt, tómt eða ekki alger
slóð. Það er skráin sem glugginn les og skrifar; `config show` skrifar slóð hennar í fyrstu línu:

<!-- example: XDG_CONFIG_HOME=/home/vous/.config subedit-cli config show | head -1 -->
```console
$ XDG_CONFIG_HOME=/home/vous/.config subedit-cli config show | head -1
File: /home/vous/.config/subedit/settings.conf (not found)
```

Þegar hvorki `XDG_CONFIG_HOME` né `HOME` gefur stað er engin skrá notandans til: `show`, `set`
og `unset` er hafnað, og `--config FILE` nefnir eina.

## `config list`

**Skrá yfir aðgerðarlyklana**, í röð skrárinnar: hver lykill, síðan fjórar inndregnar
línur — til hvers hann er, `Accepts:` (lokað mengi, eða formið og mörkin), `Default:` og
`Used by:` (`subedit-cli`-skipanirnar og gluggaaðgerðirnar sem lesa hann). Hún les enga skrá og
þarf enga, og virkar með `--no-config`.

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

Hún telur upp **45 lykla**:

<!-- example: subedit-cli config list | grep -c '^[a-z]' -->
```console
$ subedit-cli config list | grep -c '^[a-z]'
45
```

**Tuttugu og fimm þeirra eru lesnir af skipanalínunni**; hinir tuttugu eru lesnir af glugganum einum,
og `Used by:`-lína þeirra segir það, með valkosti `correct` sem kemur í þeirra stað:

| Lykill | Lesinn af |
| :-- | :------ |
| `duration-adjust.speed`, `duration-adjust.lengthen`, `duration-adjust.shorten` | [`adjust`](adjust.md) og `Adjust Durations…` |
| `duration-adjust.minimum-enabled`, `duration-adjust.minimum-ms` | `adjust` og `Adjust Durations…` |
| `duration-adjust.maximum-enabled`, `duration-adjust.maximum-ms` | `adjust` og `Adjust Durations…` |
| `duration-adjust.gap-enabled`, `duration-adjust.gap-ms` | `adjust` og `Adjust Durations…` |
| `correction.human`, `correction.ocr` | [`correct`](correct.md) og `Correct Texts…` |
| `correction.remove-blank` | `correct` og `Correct Texts…` |
| `correction.line-break.max-lines` | `correct` og `Correct Texts…` |
| `transcription.engine`, `transcription.executable`, `transcription.model`, `transcription.vad-model`, `transcription.language`, `transcription.suppress-notes`, `transcription.merge-gap-ms`, `transcription.max-duration-ms`, `transcription.max-characters` | [`transcribe`](transcribe.md) og `Transcribe Audio…` |
| `alignment.engine`, `alignment.min-match`, `alignment.doubt-gap` | [`sync`](sync.md) |
| hver annar `correction.*`-lykill | aðeins `Correct Texts…` — `correct` tekur `--tasks`, `--code`, `--max-length`, `--skip-length`, `--skip-lines`, `--language`, `--enable` og `--disable` úr sinni eigin línu |

Línufjöldinn, `correction.line-break.max-lines` og `correction.line-break.skip-max-lines`,
eru heilar tölur frá 1 til 1000 — mörk `correct --max-lines` —, og línulengdirnar,
`correction.line-break.max-length` og `correction.line-break.skip-max-length`, eru tölur yfir
0: `set` hafnar öllu öðru, og skrá sem er breytt í höndunum og geymir slíkt gildi heldur sjálfgefna gildinu.

`correct` tekur verkefnin, kóðann og línulengdina úr sinni eigin línu af ásettu ráði: ekkert er
hakað fyrirfram á skipanalínu, og glugginn mælir línur í ems þar sem skipanalínan
telur stafi. `correction.activations`, listinn yfir mynstrareitina sem glugginn sér um, er ekki
lykill skrárinnar: `correct` hefur `--enable` og `--disable`.

Með `--format json` kemur einn hlutur fyrir hvern lykil — `key`, `description`, `accepts`, `default`, `used_by`,
og `surface`, `"both"` eða `"window-only"`:

<!-- example: subedit-cli --format json config list | head -1 -->
```console
$ subedit-cli --format json config list | head -1
{"schema":1,"command":"config","key":"duration-adjust.speed","description":"Reading speed the durations are brought to.","accepts":"a number of characters per second, above 0, decimals allowed","default":"15","used_by":"subedit-cli adjust; Adjust Durations…","surface":"both"}
```

## `config show`

**Skráin sem er notuð, síðan hver aðgerðarlykill með gildi sínu**: `File: <path> (found)`,
`(not found)` eða `(unreadable: <reason>)`, síðan ein lína fyrir hvern lykil —
`[file]` og sjálfgefna gildið þegar skráin setur það, `[default]` þegar hún gerir það ekki. Lykill sem skráin
geymir með gildi sem ekki er hægt að lesa sýnir sjálfgefna gildið sitt. Birtingarstillingar eru ekki taldar upp.

<!-- example: subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --config mine/settings.conf config show | head -4 -->
```console
$ subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --config mine/settings.conf config show | head -4
duration-adjust.speed = 12
File: mine/settings.conf (found)
duration-adjust.speed = 12   [file]   (default 15)
duration-adjust.lengthen = true   [default]
duration-adjust.shorten = false   [default]
```

Skrá sem vantar er ekki villa: `show` skrifar sjálfgefnu gildin og kóðinn er `0` — skrá
notandans sem `FILE` nefnt með `--config`, sem [`adjust`](adjust.md) og [`correct`](correct.md) hafna aftur á móti
(sjá [Stillingaskráin](invocation.md#stillingaskráin)). Með
`--no-config`:

<!-- example: subedit-cli --no-config config show | head -2 -->
```console
$ subedit-cli --no-config config show | head -2
File: none (--no-config)
duration-adjust.speed = 15   [default]
```

Með `--format json` kemur fyrst hlutur `{"file", "exists"}` — `file` er `null` með `--no-config` —
síðan einn hlutur fyrir hvern lykil: `key`, `value`, `source` (`"file"` eða `"default"`), `default`,
`used_by`, `surface`.

<!-- example: subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --format json --config mine/settings.conf config show | head -2 -->
```console
$ subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --format json --config mine/settings.conf config show | head -2
duration-adjust.speed = 12
{"schema":1,"command":"config","file":"mine/settings.conf","exists":true}
{"schema":1,"command":"config","key":"duration-adjust.speed","value":"12","source":"file","default":"15","used_by":"subedit-cli adjust; Adjust Durations…","surface":"both"}
```

## `config set` og `config unset`

**`set` athugar gildið með lesara skrárinnar sjálfrar** — þeim sem glugginn notar — og hafnar
því ef ekki er hægt að lesa það, áður en nokkuð er skrifað. **`unset`** skrifar sjálfgefna gildið aftur,
kommentað: lykillinn fylgir þá sjálfgefna gildinu jafnvel þótt síðari útgáfa breyti því. Að afstilla
lykil sem er þegar á sjálfgefnu gildi heppnast og breytir engu.

Bæði **skrifa skrána upp á nýtt í heild og í einu lagi**: allar birtingarstillingar sem þegar eru í henni haldast,
aðgerðablokkin kemur fyrst og birtingarblokkin önnur, hvor undir sínum borða. **Gildi
sem skráin gat ekki lesið, og óþekktur lykill, haldast ekki** — endurskrifunin er gluggans, sem
fellir þau líka niður. **Hvert gildi sem fellur niður er nefnt**, á staðalvilluúttak á sjálfgefnu stigi, birtingarstillingar
meðtaldar: `<key>: unreadable value '<value>' replaced by its default`. Skráin og
mappan hennar eru búin til ef þær vantar.

Staðfestingin fer á staðalvilluúttak, á sjálfgefnu stigi: `KEY = VALUE` fyrir `set`,
`KEY reset to its default (DEFAULT)` fyrir `unset`.

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

## Skilakóðar

| Kóði | Merking |
| :--- | :------ |
| `0` | lokið — þar með talið `show` á skrá sem vantar og `unset` á lykil á sjálfgefnu gildi |
| `1` | notkunarvilla: engin undirskipun, röksemd vantar, lykli eða gildi hafnað, `--no-config` með `set` eða `unset`, `--config` með tómt nafn, engin skrá notandans og enginn `--config`; ekkert er skrifað |
| `2` | ekki er hægt að lesa eða skrifa skrána; ekkert er skrifað |

## Villur

| Hvað veldur henni | Hvað er skrifað, á staðalvilluúttak |
| :--------------- | :------------------------------------- |
| birtingarstilling | `<key> is a display preference; set it in the window` |
| óþekktur lykill | `unknown setting <key>; subedit-cli config list names them` |
| gildi sem lykillinn tekur ekki við | `cannot read '<value>' for <key>; subedit-cli config list says what it accepts` |
| `correction.activations` | `correction.activations is managed by the window; use --enable/--disable on correct` |
| `set` eða `unset` með `--no-config` | `--no-config has nothing to write to` |
| engin skrá notandans finnst, og enginn `--config` | `no settings file could be located; give one with --config FILE` |
| `--config` með `--no-config` | `--config excludes --no-config` |
| `--config` með tómt nafn | `--config needs a file name` |
| skrá sem er til og er ólæsileg, fyrir `set` eða `unset` | `<path>: cannot be read; nothing written` |
| skrá sem ekki er hægt að skrifa | `<path>: cannot be written`, eða `<path>: cannot be written: permission denied` |

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
