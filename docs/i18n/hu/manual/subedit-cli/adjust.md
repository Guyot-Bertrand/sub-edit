# `adjust`

<!-- machine-translated -->
> Ez az oldal gépi fordítás, amelyet a nyelv egyik olvasója még nem nézett át.

```
subedit-cli adjust [--speed CPS|off] [--shorten | --no-shorten] [--lengthen | --no-lengthen]
                   [--minimum TIME|off] [--maximum TIME|off] [--gap TIME|off]
                   [--range N-M|N-]
                   (--output FILE | --output-dir DIR | --in-place)
                   [--dry-run]
                   [--recursive]
                   <file>...
```
Minden feliratsor **időtartamát** egy olvasási sebességhez és korlátokhoz igazítja: az olvasáshoz
túl rövid feliratsort meghosszabbítja, a túl későn eltűnő feliratsort le lehet
rövidíteni, és két feliratsor nem fedi át egymást. **Csak a
vég mozdul**: a kezdet mozgatása magát a feliratsort tolná el, ami a
[`shift`](shift.md) dolga. Az [ablak](../subedit-gui/operations.md#adjust-durations)
`Adjust Durations…` művelete, ugyanazokkal az alapértelmezésekkel.

## A négy megkötés és a sorrendjük

**Ebben a sorrendben** érvényesülnek, mindegyik a véget állítja be, és az utolsó győz:

1. **az olvasási sebesség** — az az időtartam, amelyet a szöveg igényel, ennyi karakter/másodperc
   mellett (a címkék nem számítanak, a sortörések igen);
2. **a legkisebb időtartam** — az ez alatti időtartamot a program a legkisebb értékre emeli;
3. **a legnagyobb időtartam** — az e fölötti időtartamot a program a legnagyobb értékre csökkenti;
4. **a rés** — a következő feliratsor előtt meghagyott idő. Ez jön utoljára, és
   **ez győz**: két egyszerre mutatott feliratsor olyan hiba, amelyet a néző lát,
   egy kissé rövid feliratsor nem az.

A fájl utolsó feliratsorának nincs következője, így nincs betartandó rés sem. Egy feliratsor következője
a fájlbeli következő, **akár ki van választva a `--range` kapcsolóval, akár nem**.

| Megkötés | Kapcsoló | Alapértelmezés | Kikapcsolás |
| :--------- | :----- | :----- | :------- |
| sebesség | `--speed CPS` | **15** karakter/másodperc | `--speed off` |
| a sebesség iránya | `--shorten`, `--no-shorten`, `--lengthen`, `--no-lengthen` | csak **meghosszabbít** | — |
| legkisebb időtartam | `--minimum TIME` | **1,5 s** | `--minimum off` |
| legnagyobb időtartam | `--maximum TIME` | **nincs** | `--maximum off` |
| rés | `--gap TIME` | **0** | `--gap off` |

Ezek a Gaupol és az ablak első megnyitásának beállításai. **Az `off` kikapcsol egy
megkötést: többé nem ellenőrzi a program.** Ez nem ugyanaz, mint nullára állítani — a `0` legkisebb időtartam
nulla hosszú legkisebb időtartam, a `0` rés pedig minden átfedést tilt.

## Amit a beállításfájl megad

**A fenti alapértelmezések a beépítettek.** Az `adjust` alapértelmezés szerint a
beállításfájl műveleti beállításait olvassa — azokat az értékeket, amelyeket az `Adjust Durations…` megjegyez az ablakban, vagy amelyeket a
[`config set`](config.md) írt —, és ezek lépnek a beépített alapértelmezések helyébe. **A parancssorban
megadott kapcsoló mindig győz**, utána a fájl, utána a beépített alapértelmezés. Hogy hol van a fájl,
a `--config` és a `--no-config` kapcsolóról lásd: [A beállításfájl](invocation.md#a-beállításfájl).

| Kulcs | Mit ad meg | Felülíró kapcsoló |
| :-- | :--------------- | :----------------------- |
| `duration-adjust.speed` | az olvasási sebesség, karakter/másodpercben | `--speed CPS` |
| `duration-adjust.lengthen`, `duration-adjust.shorten` | a sebesség milyen irányba mozgathatja a véget; ha mindkettő `false`, a sebesség ki van kapcsolva | `--lengthen`, `--no-lengthen`, `--shorten`, `--no-shorten`, `--speed off`; a `--speed CPS` vagy valamelyik irány megadása újra bekapcsolja a fájl által kikapcsolt sebességet, meghosszabbítással |
| `duration-adjust.minimum-enabled`, `duration-adjust.minimum-ms` | a legrövidebb időtartam, vagy nincs | `--minimum TIME`, `--minimum off` |
| `duration-adjust.maximum-enabled`, `duration-adjust.maximum-ms` | a leghosszabb időtartam, vagy nincs | `--maximum TIME`, `--maximum off` |
| `duration-adjust.gap-enabled`, `duration-adjust.gap-ms` | a legkisebb rés, vagy nincs | `--gap TIME`, `--gap off` |

**Minden kulcshoz van egy kapcsoló, amely hatástalanítja.** Minden irányhoz tartozik egy
jelzőpár, amely a fájl tartalmától függetlenül beállítja: `--shorten` és `--no-shorten` a
rövidítéshez, `--lengthen` és `--no-lengthen` a hosszabbításhoz; a meg nem adott jelző a fájl irányát
változatlanul hagyja. A `--maximum off` eldobja a fájl által bekapcsolt legnagyobb időtartamot. A `--format json` esetén a `constraints` mező
a ténylegesen használt beállításokat közli, bárhonnan is származnak, a `settings` mező pedig az elolvasott fájlt:

<!-- example: printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine/settings.conf config set duration-adjust.speed 12 2>/dev/null; subedit-cli --config mine/settings.conf --format json adjust --dry-run a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine/settings.conf config set duration-adjust.speed 12 2>/dev/null; subedit-cli --config mine/settings.conf --format json adjust --dry-run a.srt
settings: read mine/settings.conf
a.srt: adjusted the durations of 1 subtitle (dry run, nothing written)
{"schema":1,"command":"adjust","file":"a.srt","ok":true,"dry_run":true,"destination":null,"counts":{"subtitles":1,"adjusted":1,"sacrificed":{"speed":0,"minimum":0,"gap":0}},"constraints":{"speed":{"cps":"12","lengthen":true,"shorten":false},"minimum_ms":1500,"maximum_ms":null,"gap_ms":0},"settings":"mine/settings.conf","warnings":[]}
```
A lap többi példája `--no-config` kapcsolóval fut, hogy azt mutassa, mit tesznek a beépített
alapértelmezések, függetlenül a futtató gép beállításaitól.

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
## Argumentumok és kapcsolók

| Kapcsoló | Kötelező | Értékek | Alapértelmezés |
| :----- | :----- | :------ | :----- |
| `<file>...` | igen | egy vagy több útvonal | — |
| `--recursive`, `-r` | nem | jelző | ki — lásd: [Könyvtárfa feldolgozása](lots.md) |
| `--speed` | nem | karakter/másodperc szám, **nullánál nagyobb** (`15`, `12.5`), vagy `off` | `15` |
| `--shorten` | nem | jelző: a sebesség a véget **korábbra is hozhatja** | ki |
| `--no-shorten` | nem | jelző: a sebesség többé **nem hozza** korábbra a véget, bármit mond a beállításfájl | ki |
| `--lengthen` | nem | jelző: a sebesség **későbbre tolja** a véget, bármit mond a beállításfájl | ki |
| `--no-lengthen` | nem | jelző: a sebesség többé **nem tolja** későbbre a véget | ki |
| `--minimum` | nem | nulla vagy nagyobb időtartam, vagy `off` | `1.5` |
| `--maximum` | nem | **nullánál nagyobb** időtartam, vagy `off` | nincs |
| `--gap` | nem | nulla vagy nagyobb időtartam, vagy `off` | `0` |
| `--range` | nem | `N-M`, vagy `N-` a végéig — lásd alább | az egész fájl |
| `--output` / `--output-dir` / `--in-place` | **a háromból egy**, kivéve a `--dry-run` esetén | lásd: [Indítás](invocation.md#a-cél) | — |
| `--dry-run` | nem | jelző: kiszámít és közöl, nem ír — lásd: [Megnézés írás előtt](invocation.md#megnézés-írás-előtt) | ki |

Az időtartam megadása ugyanaz, mint a [`shift`](shift.md#az-időtartam-megadása) esetében: `2.5`, `00:00:02.500`.

**Aktív megkötés nélkül az `adjust` elutasításra kerül** (`1` kód, semmi sem íródik ki): nem lenne
mihez igazítani, és az a parancs, amely semmit sem tesz, miközben sikert jelez, a lehető legrosszabb
válasz. Ez a helyzet a `--speed off --minimum off --gap off` esetén legnagyobb időtartam nélkül, és a
`--no-lengthen` önmagában, kikapcsolt legkisebb időtartammal és réssel: az a sebesség, amely
egyetlen véget sem mozgat, semmit sem korlátoz.

A `--shorten`, a `--no-shorten`, a `--lengthen` és a `--no-lengthen` azt mondja meg, hogy a sebesség milyen irányba mozgatja a
véget; csak sebességgel van értelmük, és **a `--speed off` elutasítja őket**. Az ellentétével együtt megadott jelzőt — `--shorten --no-shorten`,
`--lengthen --no-lengthen` — szintén elutasítja a program.

### `--range`

A `--range 3-7` az igazítást a 3.–7. feliratsorra korlátozza, **1-től számolva, a határokat
beleértve**; a `--range 3-` az utolsóig tart. Egyetlen feliratsort `4-4` alakban kell írni.

- Az a tartomány, **amely a kezdete előtt ér véget**, a `0`, az előjel vagy az egyedüli szám
  használati hibaként utasítódik el (`1` kód).
- **A köteg minden fájlára érvényes**, ami csak akkor van értelme, ha a fájlok
  hasonlóak. **Minden fájlhoz külön** mérlegeli a program: az a tartomány, amely túlmegy a fájlon,
  **a módosítás előtt** elutasításra kerül, megnevezve a határt —
  `range 3-9 ends after the last subtitle: the file has 4` —, az a fájl hibázik (`2` vagy `3` kód, azonosító
  `range-out-of-bounds`), a többi pedig folytatódik.
- A jelentés számlálói csak a kiválasztott feliratsorokra vonatkoznak.

## Mit áldoz fel a program

Két megkötés ellentmondhat egymásnak ugyanazon a feliratsoron: az 1,5 s legkisebb időtartam nem fér el
abban a 0,8 s-ban, amelyet a következő feliratsor meghagy. **A sorrend dönt, és a jelentés közli,
mi engedett** — amit a Gaupol nem tesz: ő csendben megszegi őket.

A program csak azt a megkötést nevezi meg, amelyet **legalább egy feliratsor nem kapott meg**, miután
a többi érvényesült. A legnagyobb időtartamnak nincs számlálója: a rés előtt érvényesül, amely
csak rövidíthet, így mindig teljesül. A legnagyobb időtartamnál nagyobb legkisebb időtartam enged a legnagyobbnak, és
legkisebb időtartamként számít.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --output ajuste.srt courts.srt; grep -e '-->' ajuste.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --output ajuste.srt courts.srt; grep -e '-->' ajuste.srt
courts.srt: adjusted the durations of 3 subtitles; could not satisfy the minimum duration in 1 subtitle -> ajuste.srt
00:00:00,000 --> 00:00:02,000
00:00:10,000 --> 00:00:14,000
00:00:14,200 --> 00:00:15,000
00:00:15,000 --> 00:00:16,500
```
A harmadik feliratsor 15,7 s-nál szeretne véget érni, hogy 1,5 s-ig tartson, de a negyedik 15,0 s-nál
kezdődik: **a rés győz**, a vége 15,0 s, és a jelentés a legkisebb időtartamot nevezi meg.

A rés kikapcsolása a legkisebb időtartamnak ad elsőbbséget — és a két feliratsor átfedi egymást:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --gap off --output libre.srt courts.srt; grep -e '-->' libre.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --gap off --output libre.srt courts.srt; grep -e '-->' libre.srt
courts.srt: adjusted the durations of 3 subtitles -> libre.srt
00:00:00,000 --> 00:00:02,000
00:00:10,000 --> 00:00:14,000
00:00:14,200 --> 00:00:15,700
00:00:15,000 --> 00:00:16,500
```
És semmi sem áldozódik fel, és semmi sem módosul, ha a fájl már belefér a korlátaiba:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --speed off --minimum off --output intact.srt courts.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --speed off --minimum off --output intact.srt courts.srt
courts.srt: no duration to adjust -> intact.srt
```
A fájl változatlanul íródik ki: **a megadott cél kiírt cél**.

## Kimenet

**Szabványos kimenet** — semmi: az eredmény a kiírt fájl.

| Szint | Mi kerül még a hibakimenetre |
| :----- | :------------------------------------- |
| 1 | `<path>: adjusted the durations of N subtitles; could not satisfy …  -> <destination>`, vagy `<path>: no duration to adjust -> <destination>` — és elsőként az elolvasott beállításfájl — `settings: read <path>` — lásd: [A beállításfájl](invocation.md#a-beállításfájl) |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — a beolvasott fájl formátuma, kódolása, jelölője és sorvégei, az eredetiek szerint visszaírva |
| 3 | `<path>: N bytes read, M written`, majd **minden olvasási diagnosztika** — lásd: [Indítás](invocation.md#olvasási-diagnosztika) |

A mondat az ablaké: az `N` azoknak a feliratsoroknak a száma, amelyeknek a vége elmozdult, a
többi, amely hiányzik, ha semmi sem engedett, felsorolja, mi nem teljesült — `the reading speed`,
`the minimum duration`, `the gap` — az érintett feliratsorok számával.

### JSON-ban

A [`--format json`](invocation.md#szkript-által-olvasható-kimenet) esetén minden fájl objektuma
a borítékon kívül a következőket tartalmazza:

| Kulcs | Tartalom |
| :-- | :------ |
| `counts.subtitles` | a **célzott** feliratsorok: az egész fájl, vagy a tartomány |
| `counts.adjusted` | azok, amelyeknek a vége elmozdult |
| `counts.sacrificed.speed`, `.minimum`, `.gap` | azok a feliratsorok, amelyek a többi megkötés érvényesülése után sem teljesítik ezt a megkötést, **azokat is beleértve, amelyeknek a vége nem mozdult** |
| `constraints.speed` | `{"cps", "lengthen", "shorten"}` — a karakter/másodperc **szövegként** (`"15"`, `"12.5"`) — vagy `null`, ha ki van kapcsolva |
| `constraints.minimum_ms`, `.maximum_ms`, `.gap_ms` | a használt időtartamok **egész ezredmásodpercben**, vagy `null`, ha ki vannak kapcsolva |
| `settings` | az elolvasott beállításfájl, vagy `null`, ha nem volt |

A `constraints` a **ténylegesen használt beállításokat** közli, az alapértelmezéseket is: két számítás
ugyanazokról a feliratsorokról így a parancssor olvasása nélkül összehasonlítható.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config --format json adjust --dry-run --maximum 1.8 courts.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config --format json adjust --dry-run --maximum 1.8 courts.srt
courts.srt: adjusted the durations of 4 subtitles; could not satisfy the reading speed in 1 subtitle, the minimum duration in 1 subtitle (dry run, nothing written)
{"schema":1,"command":"adjust","file":"courts.srt","ok":true,"dry_run":true,"destination":null,"counts":{"subtitles":4,"adjusted":4,"sacrificed":{"speed":1,"minimum":1,"gap":0}},"constraints":{"speed":{"cps":"15","lengthen":true,"shorten":false},"minimum_ms":1500,"maximum_ms":1800,"gap_ms":0},"settings":null,"warnings":[]}
```
## Kilépési kódok

Az eszköz általános kódjai: `0`, ha minden fájl feldolgozása megtörtént, `2`, ha egyiké sem, `3`, ha csak
néhányé, `1` használati hiba esetén. Az a fájl, amely már belefér a korlátaiba, feldolgozottnak számít.

## Hibák

| Mi váltja ki | Üzenet |
| :------------------ | :------ |
| a `--config` által megnevezett, nem létező fájl — a felhasználó saját, hiányzó fájlja az alapértelmezéseket adja | `settings file not found: <FILE>` |
| nincs aktív megkötés | `adjust has nothing to adjust to: every constraint is off, or the reading speed is told to move no end` |
| nem sebesség értékű sebesség | `--speed: "…" is not a reading speed: expected a number of characters per second above zero, like 15 or 12.5, or off` |
| irányjelző a `--speed off` mellett | `--lengthen, --no-lengthen, --shorten and --no-shorten say which way the reading speed may move an end, and --speed off switches the reading speed off` |
| irányjelző az ellentétével | `--shorten and --no-shorten say opposite things; give one of them` (ugyanígy a `--lengthen` és a `--no-lengthen`) |
| nem időtartam értékű időtartam | `--minimum: "…" is not a time: …` (ugyanígy a `--maximum`, a `--gap`) |
| negatív időtartam | `--minimum: "-1" is not a duration of zero or more` (ugyanígy a `--gap`) |
| nulla vagy negatív legnagyobb időtartam | `--maximum: "0" is not a duration above zero` |
| rosszul írt tartomány | `--range: "…" is not a range: write it N-M, or N- to go to the end, counted from 1 and inclusive` |
| a kezdete előtt végződő tartomány | `--range: "5-3" is not a range: it ends before it starts` |
| a fájlon kívüli tartomány | `<path>: range N-M starts after the last subtitle: the file has K`, vagy `… ends after …` |

A cél hibái közösek az író alparancsokban:
lásd: [Indítás](invocation.md#a-cél).
