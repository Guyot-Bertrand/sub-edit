# `config`

<!-- machine-translated -->
> Tato stránka je strojovým překladem a dosud ji nezkontroloval žádný čtenář tohoto jazyka.

```
subedit-cli [--config FILE | --no-config] [--format text|json] config list
subedit-cli [--config FILE | --no-config] [--format text|json] config show
subedit-cli [--config FILE] config set KEY VALUE
subedit-cli [--config FILE] config unset KEY
```

Vypisuje, zobrazuje a mění **nastavení operací**: tu část souboru nastavení, kterou
sdílejí okno a příkazový řádek. `Adjust Durations…` a `Correct Texts…` si tam pamatují své
hodnoty a [`adjust`](adjust.md) a [`correct`](correct.md) je ve výchozím stavu čtou — viz
[Soubor nastavení](invocation.md#soubor-nastavení).

**Co nedělá:** nikdy nečte ani nezapisuje **předvolbu zobrazení** (geometrii
okna, sloupce, motiv, volby hledání, kontrolu pravopisu, video…). Ty zůstávají věcí
okna, nastavují se v okně a popisuje je [jeho příručka](../subedit-gui/preferences.md#soubor).
Nikdy se také nedotkne souboru titulků.

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

## Podpříkazy

| Podpříkaz | Co dělá | Soubor |
| :---------- | :----------- | :--- |
| `config list` | vypíše každý klíč operace s tím, k čemu slouží, co přijímá, jeho výchozí hodnotou a tím, kdo ho používá — **žádnou hodnotu** | nečte žádný |
| `config show` | vypíše použitý soubor a zda existuje, pak každý klíč operace s jeho hodnotou, odkud hodnota pochází, a jeho výchozí hodnotou | čte ho |
| `config set KEY VALUE` | ověří `VALUE`, pak soubor přepíše s `KEY = VALUE` a zachová každé jiné nastavení | čte ho i zapisuje |
| `config unset KEY` | vrátí `KEY` na výchozí hodnotu, zapsanou jako komentář, tak jak soubor výchozí hodnotu zapisuje vždy | čte ho i zapisuje |

| Volba | Povinná | Hodnoty | Výchozí |
| :----- | :------- | :----- | :------ |
| `KEY` | ano, pro `set` a `unset` | klíč operace, tak jak ho uvádí `config list` | — |
| `VALUE` | ano, pro `set` | to, co podle `config list` klíč přijímá | — |

`config` nemá žádnou vlastní volbu. Globální volby zapsané **před** `config` volí
soubor a tvar výstupu:

| Globální volba | Účinek na `config` |
| :------------ | :----------------- |
| `--config FILE` | `show`, `set` a `unset` pracují se souborem `FILE` místo souboru uživatele, i když ještě neexistuje; `list` ji ignoruje |
| `--no-config` | `list` funguje jako obvykle; `show` zobrazí výchozí hodnoty s `File: none (--no-config)`; `set` a `unset` se odmítnou: není kam zapisovat |
| `--format json` | `list` a `show` zapíší jeden objekt JSON na řádek; `set` a `unset` na standardní výstup nezapíší nic |
| `-q`, `--quiet` | `set` a `unset` už nepotvrzují, co zapsaly |

## Který soubor

Bez `--config` soubor uživatele:

```
$XDG_CONFIG_HOME/subedit/settings.conf
```

nebo `~/.config/subedit/settings.conf`, když `XDG_CONFIG_HOME` není nastavená, je prázdná nebo není absolutní
cestou. Je to soubor, který čte a zapisuje okno; `config show` vypíše jeho cestu na prvním řádku:

<!-- example: XDG_CONFIG_HOME=/home/vous/.config subedit-cli config show | head -1 -->
```console
$ XDG_CONFIG_HOME=/home/vous/.config subedit-cli config show | head -1
File: /home/vous/.config/subedit/settings.conf (not found)
```

Když místo neurčí ani `XDG_CONFIG_HOME`, ani `HOME`, žádný soubor uživatele neexistuje: `show`, `set`
a `unset` se odmítnou a soubor určí `--config FILE`.

## `config list`

**Katalog klíčů operací**, v pořadí souboru: každý klíč, pak čtyři odsazené
řádky — k čemu slouží, `Accepts:` (uzavřená množina, nebo tvar a meze), `Default:` a
`Used by:` (příkazy `subedit-cli` a akce okna, které ho čtou). Nečte žádný soubor a žádný
nepotřebuje, a funguje s `--no-config`.

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

Vypisuje **33 klíčů**:

<!-- example: subedit-cli config list | grep -c '^[a-z]' -->
```console
$ subedit-cli config list | grep -c '^[a-z]'
33
```

**Třináct z nich čte příkazový řádek**; ostatních dvacet čte jen okno
a jejich řádek `Used by:` to říká, spolu s volbou podpříkazu `correct`, která je nahrazuje:

| Klíč | Čte ho |
| :-- | :------ |
| `duration-adjust.speed`, `duration-adjust.lengthen`, `duration-adjust.shorten` | [`adjust`](adjust.md) a `Adjust Durations…` |
| `duration-adjust.minimum-enabled`, `duration-adjust.minimum-ms` | `adjust` a `Adjust Durations…` |
| `duration-adjust.maximum-enabled`, `duration-adjust.maximum-ms` | `adjust` a `Adjust Durations…` |
| `duration-adjust.gap-enabled`, `duration-adjust.gap-ms` | `adjust` a `Adjust Durations…` |
| `correction.human`, `correction.ocr` | [`correct`](correct.md) a `Correct Texts…` |
| `correction.remove-blank` | `correct` a `Correct Texts…` |
| `correction.line-break.max-lines` | `correct` a `Correct Texts…` |
| každý jiný klíč `correction.*` | jen `Correct Texts…` — `correct` bere `--tasks`, `--code`, `--max-length`, `--skip-length`, `--skip-lines`, `--language`, `--enable` a `--disable` ze svého vlastního řádku |

Počty řádků, `correction.line-break.max-lines` a `correction.line-break.skip-max-lines`,
jsou celá čísla od 1 do 1000 — meze `correct --max-lines` —, a délky řádků,
`correction.line-break.max-length` a `correction.line-break.skip-max-length`, jsou čísla větší
než 0: `set` ostatní odmítne a ručně upravený soubor, který takovou hodnotu obsahuje, si ponechá výchozí hodnotu.

`correct` bere úlohy, kód a délku řádku ze svého vlastního řádku záměrně: na
příkazovém řádku není nic předem zaškrtnuto a okno měří řádky v em, kdežto příkazový řádek
počítá znaky. `correction.activations`, seznam políček vzorů, který spravuje okno, není
klíčem katalogu: `correct` má `--enable` a `--disable`.

S `--format json` jeden objekt na klíč — `key`, `description`, `accepts`, `default`, `used_by`
a `surface`, `"both"` nebo `"window-only"`:

<!-- example: subedit-cli --format json config list | head -1 -->
```console
$ subedit-cli --format json config list | head -1
{"schema":1,"command":"config","key":"duration-adjust.speed","description":"Reading speed the durations are brought to.","accepts":"a number of characters per second, above 0, decimals allowed","default":"15","used_by":"subedit-cli adjust; Adjust Durations…","surface":"both"}
```

## `config show`

**Použitý soubor, pak každý klíč operace s jeho hodnotou**: `File: <path> (found)`,
`(not found)` nebo `(unreadable: <reason>)`, pak jeden řádek na klíč —
`[file]` a výchozí hodnota, když ho soubor nastavuje, `[default]`, když ne. Klíč, který soubor
obsahuje s hodnotou, kterou nelze přečíst, ukazuje svou výchozí hodnotu. Předvolby zobrazení se nevypisují.

<!-- example: subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --config mine/settings.conf config show | head -4 -->
```console
$ subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --config mine/settings.conf config show | head -4
duration-adjust.speed = 12
File: mine/settings.conf (found)
duration-adjust.speed = 12   [file]   (default 15)
duration-adjust.lengthen = true   [default]
duration-adjust.shorten = false   [default]
```

Chybějící soubor není chyba: `show` vypíše výchozí hodnoty a kód je `0` — jak u souboru
uživatele, tak u souboru `FILE` zadaného přes `--config`, který naopak [`adjust`](adjust.md) a [`correct`](correct.md)
odmítnou (viz [Soubor nastavení](invocation.md#soubor-nastavení)). S
`--no-config`:

<!-- example: subedit-cli --no-config config show | head -2 -->
```console
$ subedit-cli --no-config config show | head -2
File: none (--no-config)
duration-adjust.speed = 15   [default]
```

S `--format json` první objekt `{"file", "exists"}` — `file` je `null` s `--no-config` —
a pak jeden objekt na klíč: `key`, `value`, `source` (`"file"` nebo `"default"`), `default`,
`used_by`, `surface`.

<!-- example: subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --format json --config mine/settings.conf config show | head -2 -->
```console
$ subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --format json --config mine/settings.conf config show | head -2
duration-adjust.speed = 12
{"schema":1,"command":"config","file":"mine/settings.conf","exists":true}
{"schema":1,"command":"config","key":"duration-adjust.speed","value":"12","source":"file","default":"15","used_by":"subedit-cli adjust; Adjust Durations…","surface":"both"}
```

## `config set` a `config unset`

**`set` ověří hodnotu vlastní čtečkou souboru** — čtečkou, kterou používá okno — a odmítne
ji, když ji nelze přečíst, dříve než se cokoli zapíše. **`unset`** zapíše zpět výchozí hodnotu,
jako komentář: klíč se pak řídí výchozí hodnotou, i když ji pozdější verze změní. Zrušení
klíče, který už má výchozí hodnotu, uspěje a nic nezmění.

Oba **přepíší soubor celý a atomicky**: každá předvolba zobrazení, která v něm už je, se zachová,
blok operací je první a blok zobrazení druhý, každý pod svým záhlavím. **Hodnota,
kterou soubor nedokázal přečíst, a neznámý klíč se nezachovají** — přepis je tentýž jako v okně, které
je také zahazuje. **Každá zahozená hodnota se ohlásí**, na chybovém výstupu na výchozí úrovni, včetně
předvoleb zobrazení: `<key>: unreadable value '<value>' replaced by its default`. Soubor a
jeho adresář se vytvoří, když chybějí.

Potvrzení jde na chybový výstup, na výchozí úrovni: `KEY = VALUE` pro `set`,
`KEY reset to its default (DEFAULT)` pro `unset`.

<!-- example: subedit-cli --config mine/settings.conf config set duration-adjust.gap-ms 100; grep -e '^[a-z]' -e '^# [A-Z][a-z]* [a-z]*:' mine/settings.conf; subedit-cli --config mine/settings.conf config unset duration-adjust.gap-ms; grep gap-ms mine/settings.conf -->
```console
$ subedit-cli --config mine/settings.conf config set duration-adjust.gap-ms 100; grep -e '^[a-z]' -e '^# [A-Z][a-z]* [a-z]*:' mine/settings.conf; subedit-cli --config mine/settings.conf config unset duration-adjust.gap-ms; grep gap-ms mine/settings.conf
duration-adjust.gap-ms = 100
# Operation settings: shared by the window and the command line (subedit-cli reads these).
duration-adjust.gap-ms = 100
# Display preferences: the window's own, the command line never reads them.
duration-adjust.gap-ms reset to its default (0)
#duration-adjust.gap-ms = 0
```

## Návratové kódy

| Kód | Význam |
| :--- | :------ |
| `0` | hotovo — včetně `show` u chybějícího souboru a `unset` u klíče s výchozí hodnotou |
| `1` | chyba použití: žádný podpříkaz, chybějící argument, odmítnutý klíč nebo hodnota, `--no-config` se `set` nebo `unset`, `--config` s prázdným názvem, žádný soubor uživatele a žádné `--config`; nic se nezapíše |
| `2` | soubor nelze přečíst nebo zapsat; nic se nezapíše |

## Chyby

| Co to způsobí | Co se zapíše na chybový výstup |
| :--------------- | :------------------------------------- |
| předvolba zobrazení | `<key> is a display preference; set it in the window` |
| neznámý klíč | `unknown setting <key>; subedit-cli config list names them` |
| hodnota, kterou klíč nepřijímá | `cannot read '<value>' for <key>; subedit-cli config list says what it accepts` |
| `correction.activations` | `correction.activations is managed by the window; use --enable/--disable on correct` |
| `set` nebo `unset` s `--no-config` | `--no-config has nothing to write to` |
| nelze najít žádný soubor uživatele a není zadáno `--config` | `no settings file could be located; give one with --config FILE` |
| `--config` s `--no-config` | `--config excludes --no-config` |
| `--config` s prázdným názvem | `--config needs a file name` |
| soubor, který existuje a nelze ho přečíst, pro `set` nebo `unset` | `<path>: cannot be read; nothing written` |
| soubor, který nelze zapsat | `<path>: cannot be written` nebo `<path>: cannot be written: permission denied` |

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
