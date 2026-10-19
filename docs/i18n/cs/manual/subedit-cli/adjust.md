# `adjust`

<!-- machine-translated -->
> Tato stránka je strojovým překladem a dosud ji nezkontroloval žádný čtenář tohoto jazyka.

```
subedit-cli adjust [--speed CPS|off] [--shorten | --no-shorten] [--lengthen | --no-lengthen]
                   [--minimum TIME|off] [--maximum TIME|off] [--gap TIME|off]
                   [--range N-M|N-]
                   (--output FILE | --output-dir DIR | --in-place)
                   [--dry-run]
                   [--recursive]
                   <file>...
```

Upraví **dobu trvání** každého titulku podle rychlosti čtení a mezí:
titulek příliš krátký na přečtení se prodlouží, titulek, který z obrazovky odchází
příliš pozdě, lze zkrátit, a dva titulky se nepřekrývají. **Posouvá se jen
konec**: posunutí začátku by posunulo titulek, což je práce podpříkazu
[`shift`](shift.md). Jde o operaci `Adjust Durations…`
[okna](../subedit-gui/operations.md#adjust-durations), se stejnými výchozími hodnotami.

## Čtyři omezení a jejich pořadí

Uplatňují se **v tomto pořadí**, každé nastavuje konec a poslední vítězí:

1. **rychlost čtení** — doba trvání, kterou text potřebuje, při tolika a tolika znacích za
   sekundu (značky se nepočítají, zalomení řádků ano);
2. **minimum** — doba trvání pod ním se zvýší na minimum;
3. **maximum** — doba trvání nad ním se sníží na maximum;
4. **mezera** — čas ponechaný před dalším titulkem. Přichází poslední a
   **vítězí**: dva titulky zobrazené současně jsou vada, kterou divák vidí,
   trochu krátký titulek ne.

Poslední titulek souboru nemá žádný další, a tedy žádnou mezeru, kterou by měl dodržet. Dalším
titulkem titulku je ten v souboru, **ať je vybraný přes `--range`, nebo ne**.

| Omezení | Volba | Výchozí | Vypnutí |
| :--------- | :----- | :----- | :------- |
| rychlost | `--speed CPS` | **15** znaků za sekundu | `--speed off` |
| směr rychlosti | `--shorten`, `--no-shorten`, `--lengthen`, `--no-lengthen` | jen **prodlužuje** | — |
| minimum | `--minimum TIME` | **1,5 s** | `--minimum off` |
| maximum | `--maximum TIME` | **žádné** | `--maximum off` |
| mezera | `--gap TIME` | **0** | `--gap off` |

Jde o nastavení Gaupolu a prvního otevření okna. **`off` omezení
vypne: už se neověřuje.** To není nastavení na nulu — minimum `0`
je minimum nula, mezera `0` zakazuje jakýkoli překryv.

## Co dodává soubor nastavení

**Výše uvedené výchozí hodnoty jsou vestavěné.** Ve výchozím stavu `adjust` čte nastavení operací
ze souboru nastavení — hodnoty, které si `Adjust Durations…` pamatuje v okně nebo které
zapsal [`config set`](config.md) — a ty nahradí vestavěné výchozí hodnoty. **Volba
zadaná na řádku vždy vítězí**, pak soubor, pak vestavěná výchozí hodnota. Viz
[Soubor nastavení](invocation.md#soubor-nastavení), kde se soubor nachází, a `--config` a
`--no-config`.

| Klíč | Co dodává | Volba, která ho přepíše |
| :-- | :--------------- | :----------------------- |
| `duration-adjust.speed` | rychlost čtení ve znacích za sekundu | `--speed CPS` |
| `duration-adjust.lengthen`, `duration-adjust.shorten` | kterým směrem smí rychlost posunout konec; obě `false` rychlost vypnou | `--lengthen`, `--no-lengthen`, `--shorten`, `--no-shorten`, `--speed off`; zadání `--speed CPS` nebo směru znovu zapne rychlost, kterou soubor vypnul, a to s prodlužováním |
| `duration-adjust.minimum-enabled`, `duration-adjust.minimum-ms` | nejkratší doba trvání, nebo žádná | `--minimum TIME`, `--minimum off` |
| `duration-adjust.maximum-enabled`, `duration-adjust.maximum-ms` | nejdelší doba trvání, nebo žádná | `--maximum TIME`, `--maximum off` |
| `duration-adjust.gap-enabled`, `duration-adjust.gap-ms` | nejmenší mezera, nebo žádná | `--gap TIME`, `--gap off` |

**Každý klíč má volbu, která ho ruší.** Každý směr má dvojici přepínačů, které ho nastaví,
ať soubor říká cokoli: `--shorten` a `--no-shorten` pro zkracování, `--lengthen` a
`--no-lengthen` pro prodlužování; nezadaný přepínač ponechá směr ze souboru tak, jak je.
`--maximum off` zruší maximum, které soubor zapíná. S `--format json` uvádí `constraints`
skutečně použitá nastavení, ať pocházejí odkudkoli, a `settings` soubor, který byl přečten:

<!-- example: printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine/settings.conf config set duration-adjust.speed 12 2>/dev/null; subedit-cli --config mine/settings.conf --format json adjust --dry-run a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine/settings.conf config set duration-adjust.speed 12 2>/dev/null; subedit-cli --config mine/settings.conf --format json adjust --dry-run a.srt
settings: read mine/settings.conf
a.srt: adjusted the durations of 1 subtitle (dry run, nothing written)
{"schema":1,"command":"adjust","file":"a.srt","ok":true,"dry_run":true,"destination":null,"counts":{"subtitles":1,"adjusted":1,"sacrificed":{"speed":0,"minimum":0,"gap":0}},"constraints":{"speed":{"cps":"12","lengthen":true,"shorten":false},"minimum_ms":1500,"maximum_ms":null,"gap_ms":0},"settings":"mine/settings.conf","warnings":[]}
```

Ostatní příklady na této stránce se spouštějí s `--no-config`, aby ukazovaly, co dělají vestavěné
výchozí hodnoty, bez ohledu na nastavení počítače, na kterém běží.

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

## Argumenty a volby

| Volba | Povinná | Hodnoty | Výchozí |
| :----- | :----- | :------ | :----- |
| `<file>...` | ano | jedna nebo více cest | — |
| `--recursive`, `-r` | ne | přepínač | vypnuto — viz [Zpracování stromu](lots.md) |
| `--speed` | ne | počet znaků za sekundu, **větší než nula** (`15`, `12.5`), nebo `off` | `15` |
| `--shorten` | ne | přepínač: rychlost smí konec také **přiblížit** (posunout **dříve**) | vypnuto |
| `--no-shorten` | ne | přepínač: rychlost už konec **nepřibližuje** (neposouvá **dříve**), ať soubor nastavení říká cokoli | vypnuto |
| `--lengthen` | ne | přepínač: rychlost **posouvá** konec **později**, ať soubor nastavení říká cokoli | vypnuto |
| `--no-lengthen` | ne | přepínač: rychlost už konec **neposouvá později** | vypnuto |
| `--minimum` | ne | doba trvání nula nebo více, nebo `off` | `1.5` |
| `--maximum` | ne | doba trvání **větší než nula**, nebo `off` | žádné |
| `--gap` | ne | doba trvání nula nebo více, nebo `off` | `0` |
| `--range` | ne | `N-M` nebo `N-` až do konce — viz níže | celý soubor |
| `--output` / `--output-dir` / `--in-place` | **jedna ze tří**, kromě `--dry-run` | viz [Spuštění](invocation.md#cíl-zápisu) | — |
| `--dry-run` | ne | přepínač: spočítá a vypíše, nic nezapíše — viz [Náhled před zápisem](invocation.md#náhled-před-zápisem) | vypnuto |

Doba trvání se zapisuje jako u podpříkazu [`shift`](shift.md#zápis-doby-trvání): `2.5`, `00:00:02.500`.

**Bez jediného aktivního omezení se `adjust` odmítne** (kód `1`, nic se nezapíše): nebylo by
k čemu upravovat, a příkaz, který nic nedělá a přitom hlásí úspěch, je ta nejhorší
odpověď. To je případ `--speed off --minimum off --gap off` bez maxima a
samotného `--no-lengthen` s vypnutým minimem a mezerou: rychlost, která neposouvá žádný
konec, nic neomezuje.

`--shorten`, `--no-shorten`, `--lengthen` a `--no-lengthen` říkají, kterým směrem rychlost posouvá
konec; mají smysl jen s rychlostí a **`--speed off` je odmítne**. Přepínač zadaný se svým
opakem — `--shorten --no-shorten`, `--lengthen --no-lengthen` — se rovněž odmítne.

### `--range`

`--range 3-7` omezí úpravu na titulky 3 až 7, **počítané od 1, včetně
mezí**; `--range 3-` jde až k poslednímu. Jediný titulek se zapíše `4-4`.

- Rozsah, **který končí dříve, než začíná**, `0`, znaménko nebo samotné číslo se
  odmítnou jako chyba použití (kód `1`).
- **Platí pro každý soubor dávky**, což má smysl jen tehdy, když si soubory
  jsou podobné. Posuzuje se **vůči každému souboru**: rozsah, který přesahuje daný soubor, se
  odmítne **dříve, než se ho kdo dotkne**, s uvedením meze —
  `range 3-9 ends after the last subtitle: the file has 4` —, tento soubor selže (kód `2` nebo `3`, identifikátor
  `range-out-of-bounds`) a ostatní pokračují.
- Počty ve zprávě se týkají jen vybraných titulků.

## Co se obětuje

Dvě omezení si mohou u téhož titulku odporovat: minimum 1,5 s se nevejde
do 0,8 s, které ponechává další titulek. **Rozhoduje pořadí a zpráva říká,
co ustoupilo** — což Gaupol nedělá: porušuje je potichu.

Každé omezení se uvede jen tehdy, když **ho alespoň jeden titulek nedostal**, poté
co se uplatní ostatní. Maximum nemá žádný počet: uplatňuje se před mezerou, která může
jen zkracovat, takže platí vždy. Minimum nad maximem ustoupí maximu a
počítá se jako minimum.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --output ajuste.srt courts.srt; grep -e '-->' ajuste.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --output ajuste.srt courts.srt; grep -e '-->' ajuste.srt
courts.srt: adjusted the durations of 3 subtitles; could not satisfy the minimum duration in 1 subtitle -> ajuste.srt
00:00:00,000 --> 00:00:02,000
00:00:10,000 --> 00:00:14,000
00:00:14,200 --> 00:00:15,000
00:00:15,000 --> 00:00:16,500
```

Třetí titulek by chtěl skončit v 15,7 s, aby trval 1,5 s, ale čtvrtý začíná v
15,0 s: **vítězí mezera**, skončí v 15,0 s a zpráva uvede minimum.

Vypnutí mezery nechá vyhrát minimum — a oba titulky se překrývají:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --gap off --output libre.srt courts.srt; grep -e '-->' libre.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --gap off --output libre.srt courts.srt; grep -e '-->' libre.srt
courts.srt: adjusted the durations of 3 subtitles -> libre.srt
00:00:00,000 --> 00:00:02,000
00:00:10,000 --> 00:00:14,000
00:00:14,200 --> 00:00:15,700
00:00:15,000 --> 00:00:16,500
```

A nic se neobětuje ani neupraví, když se soubor už vejde do svých mezí:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --speed off --minimum off --output intact.srt courts.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --speed off --minimum off --output intact.srt courts.srt
courts.srt: no duration to adjust -> intact.srt
```

Soubor se zapíše tak, jak je: **zadaný cíl zápisu je zapsaný cíl zápisu**.

## Výstup

**Standardní výstup** — nic: výsledkem je zapsaný soubor.

| Úroveň | Co se přidá na chybový výstup |
| :----- | :------------------------------------- |
| 1 | `<path>: adjusted the durations of N subtitles; could not satisfy …  -> <destination>` nebo `<path>: no duration to adjust -> <destination>` — a jako první přečtený soubor nastavení — `settings: read <path>` — viz [Soubor nastavení](invocation.md#soubor-nastavení) |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — formát, kódování, značka a konce řádků přečteného souboru, vrácené tak, jak byly |
| 3 | `<path>: N bytes read, M written`, pak **každé hlášení při čtení** — viz [Spuštění](invocation.md#hlášení-při-čtení) |

Věta je stejná jako v okně: `N` je počet titulků, jejichž konec se posunul, a
zbytek, který chybí, když nic neustoupilo, vypisuje, co nebylo dodrženo — `the reading speed`,
`the minimum duration`, `the gap` — s počtem dotčených titulků.

### Ve formátu JSON

S [`--format json`](invocation.md#výstup-čitelný-pro-skript) nese objekt každého souboru
kromě obálky:

| Klíč | Obsah |
| :-- | :------ |
| `counts.subtitles` | titulky, **na které se cílí**: celý soubor, nebo rozsah |
| `counts.adjusted` | ty, jejichž konec se posunul |
| `counts.sacrificed.speed`, `.minimum`, `.gap` | titulky, které toto omezení po uplatnění ostatních stále nedodržují, **včetně těch, jejichž konec se neposunul** |
| `constraints.speed` | `{"cps", "lengthen", "shorten"}` — znaky za sekundu **jako řetězec** (`"15"`, `"12.5"`) — nebo `null`, je-li vypnutá |
| `constraints.minimum_ms`, `.maximum_ms`, `.gap_ms` | použité doby trvání v **celých milisekundách**, nebo `null`, je-li vypnuto |
| `settings` | soubor nastavení, který byl přečten, nebo `null`, když žádný přečten nebyl |

`constraints` uvádí **skutečně použitá nastavení**, včetně výchozích: dva výpočty
nad týmiž titulky se pak dají porovnat bez čtení příkazového řádku.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config --format json adjust --dry-run --maximum 1.8 courts.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config --format json adjust --dry-run --maximum 1.8 courts.srt
courts.srt: adjusted the durations of 4 subtitles; could not satisfy the reading speed in 1 subtitle, the minimum duration in 1 subtitle (dry run, nothing written)
{"schema":1,"command":"adjust","file":"courts.srt","ok":true,"dry_run":true,"destination":null,"counts":{"subtitles":4,"adjusted":4,"sacrificed":{"speed":1,"minimum":1,"gap":0}},"constraints":{"speed":{"cps":"15","lengthen":true,"shorten":false},"minimum_ms":1500,"maximum_ms":1800,"gap_ms":0},"settings":null,"warnings":[]}
```

## Návratové kódy

Kódy nástroje: `0`, pokud byly zpracovány všechny soubory, `2`, pokud žádný, `3`, pokud jen
některé, `1` při chybě použití. Soubor, který se už vejde do svých mezí, je zpracovaný.

## Chyby

| Co to způsobí | Hlášení |
| :------------------ | :------ |
| soubor zadaný přes `--config`, který neexistuje — chybějící vlastní soubor uživatele dává výchozí hodnoty | `settings file not found: <FILE>` |
| žádné aktivní omezení | `adjust has nothing to adjust to: every constraint is off, or the reading speed is told to move no end` |
| rychlost, která rychlostí není | `--speed: "…" is not a reading speed: expected a number of characters per second above zero, like 15 or 12.5, or off` |
| přepínač směru s `--speed off` | `--lengthen, --no-lengthen, --shorten and --no-shorten say which way the reading speed may move an end, and --speed off switches the reading speed off` |
| přepínač směru se svým opakem | `--shorten and --no-shorten say opposite things; give one of them` (stejně `--lengthen` a `--no-lengthen`) |
| doba trvání, která dobou trvání není | `--minimum: "…" is not a time: …` (stejně `--maximum`, `--gap`) |
| záporná doba trvání | `--minimum: "-1" is not a duration of zero or more` (stejně `--gap`) |
| nulové nebo záporné maximum | `--maximum: "0" is not a duration above zero` |
| špatně zapsaný rozsah | `--range: "…" is not a range: write it N-M, or N- to go to the end, counted from 1 and inclusive` |
| rozsah, který končí dříve, než začíná | `--range: "5-3" is not a range: it ends before it starts` |
| rozsah mimo soubor | `<path>: range N-M starts after the last subtitle: the file has K` nebo `… ends after …` |

Chyby cíle zápisu jsou společné podpříkazům, které zapisují:
viz [Spuštění](invocation.md#cíl-zápisu).
