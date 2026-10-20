# `correct`

<!-- machine-translated -->
> Ez az oldal gépi fordítás, amelyet a nyelv egyik olvasója még nem nézett át.

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
A szövegeket a **Gaupol mintáival** javítja, mint az ablak
[`Correct Texts…`](../subedit-gui/correct-texts.md) segédje — ugyanazok a minták,
ugyanaz a motor, ugyanazok a számlálók. Hat feladat létezik itt: a
**hallássérülteknek szóló utalások**, a **szavak egyesítése** és **szétválasztása**, a **gyakori hibák**, a
**nagybetűs írás** és a **sortörés**.

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
## Mi kerül kiválasztásra, és mi nem

**Semmi sincs előre bejelölve.** Az ablak úgy nyitja meg a mezőit, ahogyan a felhasználó hagyta; a
parancssor a feladatait, a kódját, a sorhosszát és a nyelvét a saját parancssorából veszi,
soha nem a beállításfájlból: sem az ablakban egyszer bejelölt feladatok, sem az ott
átkapcsolt mintamezők (`correction.activations`), sem a helyesírás-ellenőrző cserelistája nem
olvasódik be, és erre az oldalra semmi sem íródik vissza.

**Négy beállítás alapértelmezés szerint a beállításfájlból jön** — azok az értékek, amelyeket a `Correct Texts…`
megjegyez, vagy amelyeket a [`config set`](config.md) írt. **A parancssori kapcsoló mindig győz**, utána
a fájl, utána a beépített alapértelmezés; hogy hol van a fájl, a `--config` és a `--no-config` kapcsolóról lásd:
[A beállításfájl](invocation.md#a-beállításfájl).

| Kulcs | Mit ad meg | Felülíró kapcsoló |
| :-- | :--------------- | :----------------------- |
| `correction.human`, `correction.ocr` | az alkalmazott gyakori hibaosztályok | `--classes` |
| `correction.remove-blank` | eltávolításra kerülnek-e a javítás által kiürített feliratsorok | `--keep-blank-subtitles`, `--remove-blank-subtitles` |
| `correction.line-break.max-lines` | a `line-break` legtöbb sora | `--max-lines` |

- **Mind a négyhez van kapcsoló, amely hatástalanítja**, bármit mond a fájl: a `--classes human,ocr`
  mindkét osztályt alkalmazza, a `--remove-blank-subtitles` eltávolítja a kiürített feliratsorokat, a `--max-lines 3`
  visszaadja a Gaupol soraszámát. A `--keep-blank-subtitles` és a `--remove-blank-subtitles` együtt
  használati hiba.
- **A fájlban mindkét osztály kikapcsolása figyelmen kívül marad, és ezt jelzi** az 1. szinten —
  `settings: correction.human and correction.ocr are both false, ignored` —: a `--classes` nem tudná kiírni, és
  a common-errors számára nem hagyna tennivalót. Helyette mindkét osztály alkalmazásra kerül.
- **Az 1 és 1000 közé nem eső sorszámot a program egyáltalán nem olvassa be a fájlból**: a fájl olvasója
  elutasítja — `settings: correction.line-break.max-lines: cannot read 'N', default kept`, az 1. szinten —,
  és a beépített 3 érvényes.

A lap példái `--no-config` kapcsolóval futnak, hogy azt mutassák, mit tesznek a beépített
alapértelmezések, függetlenül a futtató gép beállításaitól.

| Kapcsoló | Kötelező | Értékek | Alapértelmezés |
| :----- | :----- | :------ | :----- |
| `<file>...` | igen | egy vagy több útvonal | — |
| `--tasks` | **igen** | vesszővel elválasztott lista: `mentions`, `join-words`, `split-words`, `common-errors`, `capitalization`, `line-break` | nincs feladat |
| `--code` | **igen** mintafeladat esetén, egyébként **nem alkalmazható** | `Script[-language[-COUNTRY]]`: `Zyyy`, `Latn`, `Latn-en`, `Latn-en-US` | — |
| `--language` | **igen** a `join-words` vagy a `split-words` esetén, egyébként **nem alkalmazható** | nyelvkód: `fr`, `fr_FR`, `en_US` | — |
| `--classes` | nem | `human`, `ocr` vagy `human,ocr` | mindkettő — vagy amit a beállításfájl mond |
| `--enable`, `--disable` | nem, ismételhető | a minta angol neve, vagy `type:name` | a szállított `.conf` fájlok |
| `--max-length` | **igen** a `line-break` esetén | **karakterek** nullánál nagyobb száma | **nincs** — lásd alább |
| `--max-lines` | nem | sorok egész száma, nullánál nagyobb | 3 — vagy amit a beállításfájl mond |
| `--skip-length`, `--skip-lines` | nem | szám, vagy `off` | ugyanazok a határok, mint a `--max-length` és a `--max-lines` |
| `--keep-blank-subtitles` | nem | jelző: a kiürített feliratsorok üresen maradnak | a kiürített feliratsorok eltávolításra kerülnek — kivéve, ha a beállításfájl mást mond |
| `--remove-blank-subtitles` | nem | jelző: a kiürített feliratsorok eltávolításra kerülnek, bármit mond a beállításfájl; a `--keep-blank-subtitles` mellett elutasításra kerül | ki |
| `--range` | nem | `N-M`, vagy `N-` a végéig — lásd: [`adjust`](adjust.md#--range) | az egész fájl |
| `--document` | nem | `main` vagy `translation` — lásd: [Fordítás](invocation.md#fordítás) | `main` |
| `-t`, `--translation-file` | a `--document translation` esetén | fordításfájl, **egyetlen** bemenethez | — |
| `--align-method` | nem, és csak a `-t` mellett | `position` vagy `number` | `position` |
| `--recursive`, `-r` | nem | jelző | ki — lásd: [Könyvtárfa feldolgozása](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **a háromból egy**, kivéve a `--dry-run` esetén | lásd: [Indítás](invocation.md#a-cél) | — |
| `--dry-run` | nem | jelző: kiszámít és közöl, nem ír — lásd: [Megnézés írás előtt](invocation.md#megnézés-írás-előtt) | ki |

### A feladatok

**A lista sorrendje nem számít**: a feladatok a Gaupol
sorrendjében érvényesülnek — az utalások, majd a szavak egyesítése és szétválasztása, majd a gyakori hibák, majd a nagybetűs írás, majd a sortörés —, mindegyik
az előző által meghagyott szövegen. Az a feliratsor, amelyet az utalások kiürítettek, nem
vesz részt a továbbiakban.

| Feladat | Mit csinál |
| :---- | :-------------- |
| `mentions` | eltávolítja a szögletes zárójelek közötti hangokat, és a kerek zárójelek közöttieket, ha a `Sound in parentheses` be van kapcsolva (a [`hearing-impaired`](hearing-impaired.md) söprése), majd a dalszöveg- és beszélőmintákat |
| `join-words` | visszaragasztja a kettévágott szót, a `--language` szótára alapján |
| `split-words` | szétválasztja az összeragadt két szót, ugyanazon szótár alapján |
| `common-errors` | javítja a gyakori hibákat: szóközök, írásjelek, ligatúrák, OCR-hibák |
| `capitalization` | kiteszi a nagybetűt, ahol a nyelv megköveteli |
| `line-break` | újra szétvágja a feliratsor sorait úgy, hogy egyik se lépje túl a `--max-length` értéket, mérlegelve a kód töréspontmintáit |

### A mintakód

**A `--code` nevezi meg a kaszkádot**: a `Zyyy` bármely írásrendszerre, majd az írásrendszer, a nyelv
és az ország — a `Latn-en-US` a `Zyyy`, a `Latn`, a `Latn-en` és a
`Latn-en-US` mintáit olvassa. **Kötelező**, mert a parancssornak nincs környezeti nyelve:
az általános alapértelmezés *kevesebbet* alkalmazna, mint hinnénk, anélkül hogy ezt közölné. Egy futás
minden feladatára érvényes; az a feladat, amely saját kódot kíván, egy második
futás. A rosszul írt kód használati hiba:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code francais --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code francais --output b.srt a.srt; echo "code=$?"
--code: "francais" is not a pattern code: expected Script[-language[-COUNTRY]], like Zyyy, Latn or Latn-en-US
code=1
```
A mintákat, amelyeket a felhasználó a `$XDG_DATA_HOME/subedit/patterns` (vagy a
`~/.local/share/subedit/patterns`) könyvtárba tesz, **hozzáadódnak a szállított mintákhoz**, mint az
ablakban, és az 1. szinten jelzi a program a nem olvasható szállított mintát: `patterns: <file>, line N (<reason>)`.

### Szavak egyesítése és szétválasztása, és a szótár

**Ez a két feladat nem olvas mintát**: a **rendszer szótárát kérdezik le az
Enchanton keresztül** — ez a csomagok függősége (`libenchant-2-2` Debianon és Ubuntun, `enchant2`
Fedorán), amelyet a `subedit-cli` megoszt az ablakkal. Ezért nincs sem kódjuk, sem beállítandó nevük:
a `--code` mintafeladat nélkül értelmetlen, a `--language` pedig e két feladat egyike nélkül az —
a másik nélkül megadott kapcsoló használati hiba, `1` kóddal.

- **A `join-words`** hozzáragasztja a szótár által nem ismert szót az azt megelőző vagy követő szóhoz,
  **ha egyetlen irány ad ismert szót**: a `hel lo` `hello` lesz; két lehetséges
  irány, vagy egy sem, nem dönt el semmit.
- **A `split-words`** szétválaszt egy, a szótár által nem ismert szót, **ha javaslatai közül pontosan egy
  az a szó szóközzel**: a `hellothere` `hello there` lesz. A nagy kezdőbetűvel és utána kisbetűkkel írt szót
  érintetlenül hagyja: leggyakrabban név.

**Az a futás, amelyhez a nyelvnek nincs szótára, elutasításra kerül**, `1` kóddal, **bármely fájl
beolvasása előtt**, és az ablak szavaival, amely szürkévé teszi a funkciót, és ezt közli — itt a parancssor
utasít el, és ezt közli, ahelyett hogy hallgatna:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nhel lo\n\n' > a.srt; subedit-cli --no-config correct --tasks join-words --language qq_QQ --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nhel lo\n\n' > a.srt; subedit-cli --no-config correct --tasks join-words --language qq_QQ --output b.srt a.srt; echo "code=$?"
no dictionary for qq_QQ
code=1
```
**Az ablak cserelistáját a program sem olvassa, sem nem írja.** A felhasználó
választásaiból *töltődik fel* a helyesírás-ellenőrzés során; a parancssornak nincs ilyen
választása, és a lista beolvasása a szkriptet attól tenné függővé, amit a szerzője valaha beletett.

### Sortörés

**Karakterekben mér, és ez a különbség az ablakhoz képest.** Az ablak *emben* mér —
a betűtípusra kalibrált szélességben, betűnként 0,55 emmel —, és az alapértelmezett 24 em-ben megadott
érték. A parancssornak nincs betűtípusa: karaktereket számol, a szöveg
karaktereit. **A 24 karakterként való átmásolása szinte mindent szétvágna, anélkül hogy ezt
közölné**; ezért **nincs alapértelmezett hossz**, és a `--max-length` kötelező a
`line-break` esetén:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn texte\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn texte\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --output b.srt a.srt; echo "code=$?"
--max-length is required by the task line-break: Gaupol's 24 is a width in ems, and has no value in characters
code=1
```
**A `--max-lines` 3**, a Gaupol értéke, amelyen a mértékegység nem változtat; a sortörés eggyel
többet vesz, ha háromnál több kell. **A kihagyás** — a már a korlátokon belüli feliratsor
érintetlenül marad — alapértelmezés szerint **ugyanazokat a határokat** veszi (`--skip-length` = `--max-length`,
`--skip-lines` = `--max-lines`), ahogyan a Gaupol; a `--skip-length off` és a `--skip-lines off`
kikapcsolja, és a szöveg ekkor egyként újraolvasásra és újravágásra kerül:

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
A töréspontminták a kód mintái: a `Latn` nem tör szám és mértékegysége között, sem
párbeszéd-kötőjel után, a `Latn-en` nem tör névelő, elöljárószó, cím vagy birtokos
determináns után. **A `Zyyy` alatt nincs töréspontminta**: a sorok
csak kiegyenlítődnek. **Ez a feladat soha nem kerül elutasításra aktív minta hiányában**: a mintái
azt mérlegelik, hol törjön, a törés nélkülük is megtörténik. A négy kapcsoló csak
a `line-break` mellett értelmes; anélkül megadva használati hibát okoz.

### Az osztályok, és a névszerinti kapcsolás

**A `--classes` véglegesen szűri a gyakori hibákat**: az `ocr` bejelölésének megszüntetése kiveszi a
mintáit az alkalmazásból — az `ﬁ` ligatúra ekkor `ﬁ` marad. A másik három mintatípus
nem hordoz osztályt.

**Az `--enable` és a `--disable` a minta angol nevével állít be egy mintát**, a név a kulcs
(0037. ADR); egy név több rekordot is jelölhet, amelyek ekkor egyetlen
mezőn osztoznak. Szükséges, és ez az, ami félreteszi az ablak alapértelmezéseit: **a
szállított `.conf` fájlok kikapcsolják az utalásmintákat**, így a puszta `--tasks mentions`
semmit sem alkalmazna. A „Sound in brackets” és a „Sound in parentheses” két minta
mindegyike a saját elválasztóját vezérli — a szögletes zárójelet, a kerek zárójelet —, és ugyanazok a
kapcsolók állítják őket: az egyik önálló bekapcsolása nem távolítja el a másikat.

Három dolog **használati hiba**, `1` kóddal, bármely fájl beolvasása előtt közölve:

| Mi váltja ki | Üzenet |
| :------------------ | :------ |
| olyan név, amely nem jelöl mintát a megadott feladatok közül, a megadott kód alatt | `--enable: "<name>" names no pattern of the tasks given, under the code <code>` |
| olyan név, amely két típus mintáit jelöli — ekkor `type:name` alakban kell írni, a típus a `common-error`, a `capitalization`, a `hearing-impaired` vagy a `line-break` | `--enable: "<name>" names patterns of several types: write type:name, …` |
| az `--enable` és a `--disable` kapcsolónak is megadott név | `"<name>" is given to --enable and to --disable: choose one` |

**A gépelési hiba hangos**: az ablak figyelmen kívül hagyja az elavult beállítást, mert egy
beállítás túlélheti az eltűnt mintát; a parancsnak nincs túlélője.

**Az aktív minta nélküli feladat elutasításra kerül**, `1` kóddal — a „nincs tennivaló” olyan
válasz, amelyet ki kell mondani, nem néma siker:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --output b.srt a.srt; echo "code=$?"
mentions: no pattern is active under the code Latn; switch one on with --enable
code=1
```
Ugyanaz a fájl, bekapcsolt szögletes zárójeles söpréssel:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --enable "Sound in brackets" --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --enable "Sound in brackets" --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:02,000
Bonjour
```
## Mi íródik ki, és mi hangzik el

A beolvasott fájl formátuma, kódolása és sorvégei **megmaradnak**. **A javítás által kiürített
feliratsorok eltávolításra kerülnek**, mint az ablak alapértelmezett
mezőjénél; a `--keep-blank-subtitles` üresen hagyja őket, a `--remove-blank-subtitles` pedig eltávolítja őket
akkor is, ha a beállításfájl megtartja. **A kiürített fordítás kiürítve marad**,
és megtartja a feliratsorát.

**A számláló az ablaké** — „Edited N and removed M subtitles”: módosított
szövegek és eltávolított feliratsorok, **soha nem találatok**. Az a fájl, amelyben semmi sem
változik, mégis kiíródik: a megadott cél kiírt cél.

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
### Minta, amely nem alkalmazható

**Az a minta, amely nem olvasható, nem fordítható le vagy nem fejeződik be, névvel szerepel, azzal a
feliratsorral együtt, ahol feladta**, és ebben a parancssor többet mond, mint az
ablak: a megerősítő lap csak egyszer nevezi meg a mintát, a szöveg nélkül.

| Ok | Mit jelent |
| :----- | :------------------- |
| `cannot be translated` | a kifejezése olyat használ, amit az ICU-ra fordítás elutasít |
| `will not compile` | a motor elutasította a kifejezését |
| `has an invalid replacement` | a cseréje nem úgy olvasható, ahogyan a Python olvassa |
| `timed out` | a motor feladta ezen a szövegen |
| `never settled` | a `Repeat` száz menet után is változtatta a szöveget |
| `grew the text too long` | a szöveg túlment azon, amit egy feliratsor elbír |

Az 1. szinten, a hibakimeneten:
`<path>: pattern "<name>" (<reason>) was not applied to subtitle N` — a vége nélkül, ha a minta minden szövegnél félre van téve. **A szám a
fájlé**, bármely feladat távolított is el feliratsorokat előtte. JSON-ban ez a fájl egy
`pattern-failed` figyelmeztetése, amelynek `detail` mezője hordozza a mondatot. **A feladott
minta nem hibás fájl**: a fájl kiíródik, és a kilépési kód `0`.

## Kimenet

**Szabványos kimenet** — semmi, az eredmény a kiírt fájl. **A `--dry-run` esetén a javasolt
módosítások**, feliratsoronként egy blokk: a száma, a szöveg előtte, a szöveg utána —
és `(removed)` `+` sor nélkül azon feliratsornál, amelyet a javítás eltávolítana; a
`--keep-blank-subtitles` esetén üresen marad. Lásd: [Megnézés írás előtt](invocation.md#megnézés-írás-előtt).

| Szint | Mi kerül még a hibakimenetre |
| :----- | :------------------------------------- |
| 1 | `<path>: Edited N and removed M subtitles -> <destination>`, a minták, amelyeket nem lehetett alkalmazni, az olvashatatlan mintafájlok — és elsőként az elolvasott beállításfájl — `settings: read <path>` — és ami belőle figyelmen kívül maradt |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, majd minden olvasási diagnosztika |

JSON-ban a `counts` tartalmazza a `corrected` (a módosított szövegek) és a `removed` (az eltávolított
feliratsorok) mezőt, a `changes` pedig az érintett feliratsorokat. A `-t` esetén az `alignment` kulcs azt mondja meg, hogyan
került a fordítás a helyére.
A `settings` az elolvasott beállításfájlt nevezi meg, vagy `null`, ha nem volt.

## Kilépési kódok

Az eszköz általános kódjai: `0`, ha minden fájl kiírása megtörtént, `2`, ha egyiké sem, `3`, ha csak
néhányé, `1` használati hiba esetén.

## Hibák

| Mi váltja ki | Mi íródik ki a hibakimenetre |
| :------------------ | :--------------------------------------- |
| a `--config` által megnevezett, nem létező fájl — a felhasználó saját, hiányzó fájlja az alapértelmezéseket adja | `settings file not found: <FILE>` |
| hiányzó `--tasks` | `--tasks is required`, majd egy utalás a `--help` kapcsolóra |
| ismeretlen feladat | `--tasks: "<name>" is not a task: expected mentions, join-words, split-words, common-errors, capitalization or line-break` |
| hiányzó `--code` | `--code is required by the tasks that read patterns: <tasks>` |
| `--code` mintafeladat nélkül | `--code is for the tasks that read patterns, and none was asked for` |
| hiányzó `--language` a `join-words` vagy a `split-words` mellett | `--language is required by the tasks that check words: <tasks>` |
| nyelvkódnak nem tekinthető `--language` | `--language: "<value>" is not a language: expected a locale code, like fr, en_US or sr@Latn` |
| `--language` e két feladat egyike nélkül | `--language is for the tasks join-words and split-words, which were not asked for` |
| szótár nélküli nyelv | `no dictionary for <language>` |
| rosszul írt kód | `--code: "<code>" is not a pattern code: expected Script[-language[-COUNTRY]], like Zyyy, Latn or Latn-en-US` |
| hiányzó `--max-length` a `line-break` mellett | `--max-length is required by the task line-break: Gaupol's 24 is a width in ems, and has no value in characters` |
| nullánál nagyobb számnak nem tekinthető `--max-length` | `--max-length: "<value>" is not a length: expected a number of characters greater than zero` |
| sorok egész számának nem tekinthető `--max-lines`, `--skip-lines` | `--max-lines: "<value>" is not a number of lines: expected a whole number greater than zero` |
| olvashatatlan `--skip-length` vagy `--skip-lines` | `--skip-length: "<value>" is not a bound: expected a number greater than zero, or off` |
| a négy kapcsoló egyike `line-break` nélkül | `--max-length is for the task line-break, which was not asked for` |
| ismeretlen osztály | `--classes: "<name>" is not a class: expected human, ocr or human,ocr` |
| `--keep-blank-subtitles` a `--remove-blank-subtitles` mellett | `--keep-blank-subtitles and --remove-blank-subtitles say opposite things; give one of them` |
| hibás mintanév | lásd a fenti táblázatot |
| aktív minta nélküli feladat | `<task>: no pattern is active under the code <code>; switch one on with --enable` |
| `--range`, `-t`, `--document`, a cél | az [Indítás](invocation.md#hibák) és a [Fordítás](invocation.md#fordítás) hibái |
