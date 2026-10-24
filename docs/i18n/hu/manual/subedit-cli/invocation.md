# Indítás

<!-- machine-translated -->
> Ez az oldal gépi fordítás, amelyet a nyelv egyik olvasója még nem nézett át.

```
subedit-cli [global options] <subcommand> [options] <file>...
```

Alparancs nélkül az eszköz kiírja a súgóját, és megáll.

<!-- example: subedit-cli -->
```console
$ subedit-cli
Read, inspect and retime subtitle files.
Usage: subedit-cli [OPTIONS] [SUBCOMMAND]

Options:
  -h,--help                   Print this help message and exit
  --version                   Display program version information and exit
  -v                          Say more: -v is the default, -vv details, -vvv debugs
  -q,--quiet                  Say nothing but errors
  --encoding NAME             Encoding to read the files in; detected by default
  --format text|json          Form of the result on standard output: text or json
  --config FILE               Read the operation settings from FILE instead of the user's settings file
  --no-config Excludes: --config
                              Ignore any settings file: use the built-in defaults
  --list-encodings            Write the encodings that can be read and written, one a line, and stop

Subcommands:
  inspect                     Report what a subtitle file is made of
  convert                     Write a subtitle file out in another format or shape
  shift                       Move every position of a file by a fixed amount
  transform                   Correct every position from two points known to be right
  framerate                   Re-time a file mastered at one frame rate for another
  snap                        Move every position onto the nearest frame of a frame rate (see framerate)
  hearing-impaired            Remove the sounds described between square brackets
  adjust                      Bring the duration of every subtitle within a reading speed and limits
  replace                     Replace a text in the subtitles, without breaking a tag
  case                        Put the texts in title, sentence, upper or lower case, tags intact
  italics                     Put the texts in italics, or take their italics out
  dialogue-dashes             Put dialogue dashes on the lines, or take them off
  sort                        Put the subtitles in the order of their start, keeping ties as they are
  join-repeats                Join the subtitles that repeat each other, one after the other, into one
  disjoint                    Cut overlapping subtitles so that no range overlaps another, merging what overlaps
  repair                      Join the repeated subtitles, put them in order, then make the ranges disjoint
  correct                     Correct the texts with the patterns of the correction assistant
  append                      Put subtitle files one after another into a single file
  split-file                  Cut a subtitle file in two, a head and a tail
  pair                        Write a translation at the positions of its main file, and say how it lined up
  transcribe                  Write the speech of a video or an audio file as subtitles
  config                      List, show and change the operation settings
```

## Nyelv

A parancssor a környezete nyelvén szól: **a súgója, a beszámolói és a hibái** a kért nyelvet követik. A kézikönyv példái angol nyelven futnak — a blokkokat a `make manual` állítja elő, soha nem kézzel másolják.

**Hogyan választódik ki a nyelv.** Az `LC_ALL`, az `LC_MESSAGES` és a `LANG` közül az első, amelyik be van állítva és nem üres, a területi beállítás; a `LANGUAGE`, ha be van állítva, ezután felsorolja a kívánt nyelveket, kettősponttal elválasztva (`LANGUAGE=pt_BR:fr`), és azt olvassák először. Ez a gettext követte sorrend, magával a programmal beolvasva: a területi beállítást nem kell legenerálni a gépen.

| Szabály | Hatás |
| :--- | :----- |
| kódolás és módosító | figyelmen kívül marad: az `fr_FR.UTF-8@euro` az `fr_FR`, majd az `fr` nyelvet keresi |
| `LC_ALL=C`, `C` vagy `POSIX` területi beállítás, vagy egyáltalán nincs területi beállítás | angol, és a `LANGUAGE` nincs beolvasva |
| angolt megnevező elem (`en`, `en_GB`) | angol: ami a `LANGUAGE` értékében utána következik, azt nem olvassák |
| olyan nyelv, amelynek a katalógusa nincs telepítve | a következő kért nyelv, az utolsó után pedig az angol, **egy szó nélkül** |
| olyan üzenet, amelyet a katalógus nem fordít | angolul jelenik meg |

**A javítási minta neve**, amelyről a `correct` közli, hogy nem tudta alkalmazni, szintén követi a nyelvet, ha a katalógus ismeri — a programmal szállított mintáknál; a saját minta úgy szerepel, ahogy a fájlja írja. Maguk a mintafájlok soha nem íródnak át.

A katalógusok a végrehajtható fájl saját könyvtárából találhatók meg, és ezen az útvonalon települnek: `<prefix>/share/subedit/locale/<language>/LC_MESSAGES/subedit.mo`. Az angol be van építve. Ez a verzió a következőket telepíti: `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR`, `ru`, `sr`, `tr` és `zh_CN` — katalán, cseh, német, spanyol, finn, francia, galíciai, héber, magyar, interlingue, izlandi, olasz, holland, lengyel, portugál, brazil portugál, orosz, szerb, török és egyszerűsített kínai —, teljesen lefordítva. A franciát a nyelv egyik olvasója átnézte; a katalán, a cseh, a német, a spanyol, a finn, a galíciai, a héber, a magyar, az interlingue, az izlandi, az olasz, a holland, a lengyel, a portugál, a brazil portugál, az orosz, a szerb, a török és az egyszerűsített kínai gépi fordítású, és a nyelv egyik olvasója még nem nézte át.
<!-- machine-translated: ca -->
<!-- machine-translated: cs -->
<!-- machine-translated: de -->
<!-- machine-translated: es -->
<!-- machine-translated: fi -->
<!-- machine-translated: gl -->
<!-- machine-translated: he -->
<!-- machine-translated: hu -->
<!-- machine-translated: ie -->
<!-- machine-translated: is -->
<!-- machine-translated: it_IT -->
<!-- machine-translated: nl -->
<!-- machine-translated: pl -->
<!-- machine-translated: pt -->
<!-- machine-translated: pt_BR -->
<!-- machine-translated: ru -->
<!-- machine-translated: sr -->
<!-- machine-translated: tr -->
<!-- machine-translated: zh_CN -->
A `LANG=fr_FR.UTF-8` beállítással a `subedit-cli --help` kimenete így kezdődik: „Lire, inspecter et recaler des fichiers de sous-titres.” Az a szkript, amely a szöveges kimenet szavait olvassa, a `LANGUAGE=en` (vagy `LC_ALL=C`) értéket állítja be — vagy a `--format json` kimenetet olvassa, amely sosem változik.

**Ami sosem követi a nyelvet** — amit a szkript olvas:

| Sosem fordított | Miért |
| :--------------- | :-- |
| a `--format json` kimenet: kulcsok, értékek, valamint a hibák és figyelmeztetések `message` és `detail` mezője | ugyanazok a bájtok minden nyelven; a szavak angolok |
| kilépési kódok | a [Kilépési kódok](#kilépési-kódok) négy kódja, minden nyelven |
| a beírt nevek: alparancsok, kapcsolók, azok értékei (`off`, `srt`), metaváltozók (`FILE`, `TEXT`, `OPTIONS`) | az egyszer megírt parancssor minden nyelven működik |
| a javítási minták nevei, amelyeket a `correct --enable` és a `--disable` kapcsoló vesz fel | a mintát az angol név hívja, amelyet a fájlja tartalmaz |
| a formátumok és kódolások nevei | a formátumot a neve hívja |
| a CLI11 saját diagnosztikája — hiányzó kötelező argumentum, ismeretlen kapcsoló, olyan érték, amelyet egy kapcsoló nem fogad el — és a `Run with --help for more information.` | abban az elemzőben fogalmazott, amelyre a parancssor épül; angolok maradnak |

Az angol kérése, bármi legyen a környezet:

<!-- example: LC_ALL=C LANGUAGE=fr subedit-cli shift --output-dir out a.srt; echo "code=$?" -->
```console
$ LC_ALL=C LANGUAGE=fr subedit-cli shift --output-dir out a.srt; echo "code=$?"
shift needs --by, or --to-grid to work the amount out from the positions
code=1
```

## Globális kapcsolók

**Az alparancs neve előtt állnak, és csakis ott.** A `subedit-cli --encoding cp1252 inspect film.srt` a helyes alak; a `subedit-cli inspect --encoding cp1252 film.srt` elutasításra kerül, ezzel: `The following argument was not expected: --encoding`. Ugyanez érvényes a `--version`, a `-v` és a `--quiet` kapcsolóra.

**A `--help` a kivétel, és nem is igazán az**: a `subedit-cli inspect --help` elfogadott, mert minden alparancsnak megvan a sajátja, és az alparancs súgóját írja ki, nem az eszközét.

| Kapcsoló | Hatás |
| :----- | :---- |
| `-h`, `--help` | kiírja a súgót, és megáll; az alparancsét, ha meg van nevezve |
| `--version` | kiírja a `subedit <version>` sort, a könyvtárakat, amelyekkel az eszköz fut, és a telepített nyelveket, és megáll — lásd [`--version`](#--version) |
| `-q`, `--quiet` | 0. szint — egyáltalán nincs beszámoló |
| `-v`, `-vv`, `-vvv` | 1–3. szint; az 1. szint az alapértelmezés |
| `--encoding NAME` | a fájlokat ebben a kódolásban olvassa, ahelyett hogy kitalálná |
| `--list-encodings` | kiírja a kódolásokat, amelyeket az eszköz olvasni és írni tud, soronként egyet, és **megáll** (`0` kód) — lásd [`--list-encodings`](#--list-encodings) |
| `--format text\|json` | a szabványos kimenet eredményének alakja: a szöveg, vagy fájlonként egy JSON-objektum — lásd [Szkript által olvasható kimenet](#szkript-által-olvasható-kimenet) |
| `--config FILE` | a műveleti beállításokat a `FILE` fájlból olvassa a felhasználó beállításfájlja helyett — lásd [A beállításfájl](#a-beállításfájl) |
| `--no-config` | nem olvas beállításfájlt: minden műveleti beállítás, amelyet nem adtak meg a parancssorban, a beépített alapértelmezést veszi fel — lásd [A beállításfájl](#a-beállításfájl) |

A `--quiet` és a `-v` ugyanabban a hívásban elutasításra kerül: két ellentétes szándékot nem az utoljára megírt javára döntenek el.

A `--config` és a `--no-config` ugyanabban a hívásban szintén elutasításra kerül, ezzel: `--config excludes --no-config`, és ugyanígy a `--config` üres névvel, ezzel: `--config needs a file name`: az üres név nem nevez meg fájlt, és a felhasználó saját fájlját nem olvassák helyette.

### A beállításfájl

**Az [`adjust`](adjust.md) és a [`correct`](correct.md) alapértelmezés szerint a beállításfájl műveleti beállításait olvassa** — azokat az értékeket, amelyeket az `Adjust Durations…` és a `Correct Texts…` megjegyez az ablakban, vagy amelyeket a [`config set`](config.md) megírt. Más alparancs nem olvassa. A fájl:

```
$XDG_CONFIG_HOME/subedit/settings.conf
```

vagy `~/.config/subedit/settings.conf`, ha az `XDG_CONFIG_HOME` nincs beállítva, üres, vagy nem abszolút útvonal — az ablak fájlja, amelyet [a kézikönyve](../subedit-gui/preferences.md#a-fájl) ír le.

**Elsőbbség, a legerősebbtől: a parancssori kapcsoló, majd a fájl, majd a beépített alapértelmezés.** A kapcsoló mindig nyer: a `--speed off` kikapcsolja a fájl által megadott sebességet. A fájlnak csak a **műveleti blokkját** olvassák; a megjelenítési beállításokat (geometria, oszlopok, téma…) soha. A [`config list`](config.md#config-list) megnevezi a kulcsokat, és azt, hogy a parancssor melyiket olvassa: a kilenc `duration-adjust.*` kulcsot, a `correction.human`, a `correction.ocr`, a `correction.remove-blank` és a `correction.line-break.max-lines` kulcsot. **Mindegyikhez van kapcsoló, amely egy hívásra felülírja**, bárhogyan állítja is be a fájl — az [`adjust`](adjust.md#amit-a-beállításfájl-megad) és a [`correct`](correct.md#mi-kerül-kiválasztásra-és-mi-nem) táblázatai megnevezik őket.

**A felhasználó fájljának olvasása soha nem buktat meg parancsot.** A hiányzó fájl az alapértelmezéseket adja; az olvashatatlan fájl az alapértelmezéseket adja, és ezt közli; az olvashatatlan érték megtartja az alapértelmezését, és megnevezik; az ismeretlen kulcsot figyelmen kívül hagyják. Az `adjust` és a `correct` soha nem írja a fájlt.

**Azt, hogy mit olvastak, közlik**, az alapértelmezett 1. szinten, a szabványos hibakimeneten, az első fájl előtt:

| Eset | Sor |
| :--- | :--- |
| a fájlt beolvasták | `settings: read <path>` |
| egy értéke nem volt olvasható | `settings: <key>: cannot read '<value>', default kept` |
| a felhasználó fájlja nem létezik | `settings: <path> not found, defaults used` |
| a fájl nem olvasható | `settings: <path> unreadable, defaults used` |
| `--no-config` | semmi |

A [`--format json`](#szkript-által-olvasható-kimenet) kapcsolóval az `adjust` és a `correct` minden fájlának objektuma tartalmazza a `settings` mezőt: a beolvasott útvonalat, vagy `null` értéket, ha egyet sem olvastak.


**A `--no-config` reprodukálhatóvá teszi a futtatást**: ugyanaz a sor ugyanazt az eredményt adja minden gépen, bármit állított is be a felhasználó az ablakban. Szkriptekben, tesztekben és hibajelentésekben ezt kell használni — vagy a `--config FILE` kapcsolót a velük együtt adott fájllal.

**A `--config FILE` másik fájlt nevez meg**, ugyanúgy olvasva — **azzal a kivétellel, hogy léteznie kell**. A parancssorban megadott név nem az alapértelmezések kérése: ha a `FILE` nem létezik, az `adjust` és a `correct` bármely bemenet olvasása előtt megáll, a szabványos hibakimenetre kiírja ezt: `settings file not found: <FILE>`, és az `1` használati kóddal lép ki. A létező, de nem olvasható `FILE` az alapértelmezéseket adja, és ezt közli, ahogy a felhasználó fájlja is. A [`config show`](config.md#config-show) a hiányzó `FILE` fájlt továbbra is `(not found)` alakban mutatja, a [`config set`](config.md#config-set-és-config-unset) pedig továbbra is létrehozza.

<!-- example: printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine.conf adjust --dry-run a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine.conf adjust --dry-run a.srt; echo "code=$?"
settings file not found: mine.conf
code=1
```

### `--version`

**A program verziója, egyedül az első sorban** — `subedit <version>`, ami a kimenet a 0.15.38 verzióig volt, és amit a szkript olvas. Utána **soronként egy könyvtár, amellyel az eszköz fut**, majd a telepített felületi nyelvek, a szabványos kimeneten; az eszköz megáll, `0` kóddal. A mellé megadott alparancs nem fut le, és a `-q` meg a `-v` semmit sem változtat.

<!-- example: subedit-cli --version | head -n 1 -->
```console
$ subedit-cli --version | head -n 1
subedit 1.0.13
```

Az egész, egy ICU 74.2 és Enchant 2.3.3 verziójú gépen — a számok a gép számai:

```console
$ subedit-cli --version
subedit <version>
  ICU      74.2 (built with 74.2)
  Enchant  2.3.3 (built with 2.3.3)
  CLI11    2.4.1 (built with 2.4.1)
  Languages: en, fr
```

| Könyvtár | Mire használja az eszköz |
| :------ | :------------------------ |
| `ICU` | a kódolások olvasása és írása, felismerése, a betűk kis- és nagybetűs alakjának módosítása |
| `Enchant` | a `correct` helyesírás-ellenőrzője |
| `CLI11` | ennek a parancssornak az olvasása |

- **Minden sor két verziót ad**: először azt, amelyet futásidőben találtak, zárójelben pedig azt, amellyel a programot lefordították. Eltérnek, ha a gép könyvtárát a program lefordítása után frissítették — ez az első, amit hibajelentésbe kell írni.
- **Az Enchant fejlécei nem hordoznak verziót**: a zárójelben álló az, amelyet a `pkg-config` jelentett a fordítás konfigurálásakor.
- **A CLI11 a programba van fordítva**, csak a fejlécei vannak meg belőle: a két verziója mindig ugyanaz.
- **Sem a Qt, sem a libmpv**: a parancssor egyikhez sem linkel, és csak azt sorolja fel, amivel fut. Az ablak felsorolja őket — lásd [a kézikönyvét](../subedit-gui/invocation.md#--version).
- **A `Languages`** az `en` nyelvet sorolja fel, a forrásnyelvet, amelynek nincs szüksége katalógusra, majd minden, a `<prefix>/share/subedit/locale` alatt telepített, olvasható katalógust, kód szerint — azokat a nyelveket, amelyeket az `Edit ▸ Preferences…` felkínál.
- **Ami követi a nyelvet**: a `built with` és a `Languages`. Az első sor, a nevek, a számok és a nyelvkódok soha.
- **A `--format json` kapcsolóval**, a `--version` előtt vagy után, **egy objektum egy sorban**: `schema` (`1`), `command` (`"version"`), `version`, `libraries` — `{"name":"ICU","loaded":"74.2","built":"74.2"}` elemek tömbje, a szöveg sorrendjében — és `languages`, kódok tömbje. Semmi sincs benne lefordítva.

<!-- example: subedit-cli --version --format json | grep -o '"name":"[^"]*"' -->
```console
$ subedit-cli --version --format json | grep -o '"name":"[^"]*"'
"name":"ICU"
"name":"Enchant"
"name":"CLI11"
```

### `--list-encodings`

**A kódolások, amelyeket az eszköz olvasni és írni tud, soronként egyet, név szerint rendezve**, a szabványos kimeneten — utána az eszköz megáll, `0` kóddal, ahogy a `--version` teszi: ez a válasz, nem egy lépés. A mellé megadott alparancs nem fut le.

<!-- example: subedit-cli --list-encodings | grep '^UTF' | head -4 -->
```console
$ subedit-cli --list-encodings | grep '^UTF' | head -4
UTF-16BE
UTF-16LE
UTF-32BE
UTF-32LE
```

- **Ezek azok a nevek, amelyeket az `--encoding` elfogad**, a kanonikusak; az álneveik (`cp1252`) szintén elfogadottak, felsorolás nélkül.
- **A szám a telepített ICU-tól függ**, és semmilyen névre nincs ígéret azokon túl, amelyeket minden ICU ismer (`UTF-8`, `UTF-16LE`, `windows-1252`…). A kézikönyv ezért nem ad számot.
- **Az `UTF-16` és az `UTF-32` nincs benne**: az ICU ismeri őket, de az átalakítójuk saját jelölőt ír, és a modell elutasítja őket — lásd alább. A választandó lista nem kínál fel olyat, amit a következő mező elutasítana.
- **A `--format json` kapcsolóval** **kódolásonként egy objektum**: `{"schema":1,"command":"list-encodings","name":"UTF-8"}`. Ez nem egy fájl jelentése: ezek az objektumok **sem a `file`, sem az `ok` mezőt nem hordozzák**, ami az egyetlen kivétel [az alak](#az-alak) borítéka alól. Továbbra is soronként egy objektum, bejegyzésenként egy.
- **A beszámolási szint semmit sem változtat**: a lista eredmény, és a `-q` nem veszi el.

### Az `--encoding`, és amit nem tud kikényszeríteni

**Minden alparancsra vonatkozik**, mert mind olvas: az a fájl, amelynek a kódolását rosszul találták ki, rossz marad, bármit tesznek vele, és egy csak az `inspect` által hordozott kapcsoló orvosság nélküli eltérést hagyna.

**A név az ICU-é, és minden álneve elfogadott**: a `cp1252`, a `windows-1252` és a `WINDOWS 1252` ugyanazt a kódolást nevezi meg. A halmaz sehol sincs leírva: az, amelyet a gép ICU-ja át tud alakítani, és a [`--list-encodings`](#--list-encodings) felsorolja. Az a név, amelyet nem tud átalakítani, egyetlen fájl olvasása előtt elutasításra kerül:

```console
$ subedit-cli --encoding klingon-1 inspect film.srt
no encoding is named "klingon-1"
```

**Egy kivétel van, és csak egy: a kódolásnak meg kell neveznie a bájtsorrendjét.** Az `UTF-16` és az `UTF-32` olyan nevek, amelyeket az ICU ismer, és amelyek átalakítója **saját jelölőt** ír. Ilyen kódolás nem élhet ebben a modellben — a jelölő megszűnik választás lenni, a `--no-bom` elfogadott és meg nem tartott lenne —, ezért mindkettő elutasításra kerül, olvasásra és írásra egyaránt:

```console
$ subedit-cli --encoding UTF-16 inspect film.srt
"UTF-16" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do
```

Az `UTF-16LE` és az `UTF-16BE` ugyanazokat a bájtokat olvassa és írja, a jelölőt is beleértve; a pontos név annyit tesz hozzá, hogy tudható, melyik.

**A bájtsorrend-jelölő erősebb a kapcsolónál.** Ez az egyetlen dolog, amelyet a feliratfájl önmagáról kijelent, és ha máshogy olvasnák, mint ahogy kijelenti magát, az a hívó követése lenne a fájl ellenében. Az eltérést **kimondják**, diagnosztikaként:

```console
$ subedit-cli -vvv --encoding cp1252 inspect avec-bom.srt
avec-bom.srt: a byte order mark that contradicts the encoding asked for ("UTF-16LE"), settled by the reader
```

Ezt teszi a Gaupol is — a jelölő kódolásával újrakezdi az olvasást —, de szó nélkül; itt a kért és az elvégzett közötti eltérés nem csend.

## Alparancsok

| Alparancs | Mit csinál | Ír? |
| :------------ | :-------------- | :----------- |
| [`inspect`](inspect.md) | jelenti, miből áll egy fájl, anélkül hogy bármit módosítana | nem |
| [`convert`](convert.md) | a fájlt másik formátumban vagy más alakban írja meg | igen |
| [`shift`](shift.md) | minden pozíciót ugyanannyival eltol | igen |
| [`transform`](transform.md) | minden pozíciót kijavít két ismerten helyes pontból | igen |
| [`framerate`](framerate.md) | újraidőzíti a fájlt az egyik képkockasebességről a másikra | igen |
| [`snap`](snap.md) | minden időbélyeget a legközelebbi képkockára tol | igen |
| [`hearing-impaired`](hearing-impaired.md) | eltávolítja a hallássérültek számára készült megjegyzéseket | igen |
| [`adjust`](adjust.md) | az időtartamokat olvasási sebességhez és korlátokhoz igazítja | igen |
| [`replace`](replace.md) | lecserél egy szöveget a feliratsorokban, címke megtörése nélkül | igen |
| [`case`](case.md) | a szövegeket címszerű, mondatszerű, nagybetűs vagy kisbetűs írásra állítja | igen |
| [`italics`](italics.md) | a szövegeket dőlt betűssé teszi, vagy kiveszi belőlük a dőlt betűt | igen |
| [`dialogue-dashes`](dialogue-dashes.md) | párbeszéd-kötőjeleket tesz fel, vagy levesz | igen |
| [`sort`](sort.md) | a feliratsorokat visszateszi a kezdetük sorrendjébe | igen |
| [`join-repeats`](join-repeats.md) | egyetlen feliratsorrá egyesíti az egymást követő darabokra vágott szöveg darabjait | igen |
| [`disjoint`](disjoint.md) | levágja az átfedő feliratsorokat, hogy egyetlen tartomány se fedjen át többé másikat | igen |
| [`repair`](repair.md) | láncba fűzi a `join-repeats`, a `sort` és a `disjoint` alparancsot | igen |
| [`correct`](correct.md) | a Gaupol mintáival javítja a szövegeket: megjegyzések, szavak egyesítése és szétválasztása, gyakori hibák, nagybetűs írás, sortörés | igen |
| [`append`](append.md) | az első fájl után fűz fájlokat **egyetlen** fájlba | igen |
| [`split-file`](split-file.md) | a fájlt **kettéhasítja**: egy elejére és egy végére | igen |
| [`pair`](pair.md) | a fordítást a főfájl pozícióin írja meg, és közli, hogyan illeszkedtek a sorai | igen |
| [`transcribe`](transcribe.md) | egy videó vagy egy hangfájl beszédét írja ki feliratként | igen — saját cél, az `--output` |
| [`config`](config.md) | felsorolja, megmutatja és módosítja a beállításfájl műveleti beállításait | csak a beállításfájlt |

Mind a huszonkettő itt van; az eszköz súgója ugyanebben a sorrendben sorolja fel őket.

**Csak egy nem érint semmit**, és ez a jobb oldali oszlop: az `inspect` olvas és jelent. A másik húsz ír, és ami következik, az tizenhétre érvényes közülük: az [`append`](append.md) és a [`split-file`](split-file.md) saját számosságú — N fájl **egy** eredményhez, egy fájl **kettőhöz** — és ezért saját célú, amelyet az oldaluk ír le; a [`transcribe`](transcribe.md) új fájlt készít egy videóból vagy egy hangfájlból, és csak az `--output` kapcsolót fogadja el. A [`config`](config.md) nem ír feliratfájlt: a beállításfájlt olvassa és módosítja, és semmi sem érvényes rá abból, ami következik.

## A cél

A tizenhét író alparancs — [`convert`](convert.md), [`shift`](shift.md), [`transform`](transform.md), [`framerate`](framerate.md), [`snap`](snap.md), [`hearing-impaired`](hearing-impaired.md), [`adjust`](adjust.md), [`replace`](replace.md), [`case`](case.md), [`italics`](italics.md), [`dialogue-dashes`](dialogue-dashes.md), [`sort`](sort.md), [`join-repeats`](join-repeats.md), [`disjoint`](disjoint.md), [`repair`](repair.md), [`correct`](correct.md), [`pair`](pair.md) — ugyanúgy veszi a célját. (Az [`append`](append.md) és a [`split-file`](split-file.md) saját céllal rendelkezik, az oldalukon.) Az [`inspect`](inspect.md) nem ír fájlt, és ezek közül egyik kapcsolót sem fogadja el.

**A már létező cél felülíródik**, kérdés nélkül, és nincs kapcsoló, amely ezt elkerülné: az írás atomi — a fájl sosem íródik félig, és a megszakadt írás érintetlenül hagyja a régit —, de az előző tartalom elvész. Ez nem megtartott hiba: ez a szabály, és ha ugyanarra a kimeneti könyvtárra újra lefuttatnak egy parancsot, az önmagára fut le újra.

**Kifejezett cél nélkül soha semmi sem íródik.** A megadás három módja kizárja egymást, és egy kell: nélküle semmi sem íródik, és a kilépési kód `1` — kivéve a [`--dry-run`](#megnézés-írás-előtt) kapcsolóval, amely semmit sem ír, és ezért egyet sem követel.

| Kapcsoló | Hová kerül az eredmény |
| :----- | :---------------- |
| `--output FILE` | ebbe a fájlba, pontosan ezen a néven; egyetlen bemenet |
| `--output-dir DIR` | ebbe a könyvtárba, a bemenet alapnevén |
| `--in-place` | a bemenet fölé, atomi írással |

Az az eszköz, amely felülírja a bemenetét, mert semmit sem mondtak neki, olyan eszköz, amelyet másodszor már nem használnak. Az elutasítás egy sorba kerül, és megmenti a fájlt.

Az `--output` több bemenettel elutasításra kerül: az utolsó fájl az előzők fölé íródna. Köteg esetén a három közül csak az `--output-dir` értelmes, az `--in-place` mellett.

**A kimeneti könyvtár létrejön**, ha hiányzik, a szülőivel együtt — az `--output-dir` kapcsolónál, és annál a könyvtárnál, amelybe az `--output` ír. Ez a parancssor ellenőrzése után és **az első fájl előtt** történik. Ha nem lehetséges, egyszer közlik, egyetlen fájlt sem olvasnak, és a kilépési kód `2`.

**A köteget írás előtt egészében megítélik.** Először kiszámolják az egyes bemenetek célját, és ebből két elutasítás következik — használati hibák, `1` kód, **egyetlen fájl sem íródik** és egyetlen könyvtár sem jön létre:

- **két bemenet, amely ugyanabba a fájlba kerülne** — az `a/film.srt` és a `b/film.srt` az `--output-dir` kapcsolóval, vagy az `a/film.srt` és a `b/film.vtt`, mindkettő WebVTT formátumra alakítva, amelynek végső célja kiterjesztést váltott. Az üzenet megnevezi a célt és mindkét bemenetet; a második átnevezése vagy az utolsó nyerése fájlt veszítene el szó nélkül, az elutasítás pedig egy sorba kerül;
- **olyan cél, amely az egyik bemenet**, `--in-place` nélkül — az `--output-dir .` kapcsolón, az `--output` kapcsolón vagy szimbolikus hivatkozáson át is. A saját bemenet fölé írni olyan mozdulat, amelyet megneveznek.

Két útvonal akkor nevezi meg ugyanazt a fájlt, ha a rendszer ezt mondja, nem ha egyformán írják őket: az `in/../in/film.srt` és az `in/film.srt` egyetlen fájl.

**A kiterjesztés a megírt formátumot követi.** Csak a [`convert`](convert.md) parancsnál változik, az egyetlen alparancsnál, amely formátumot vált; a többi megtartja az olvasott fájl formátumát, és így a kiterjesztését.

| Mi váltja ki | Üzenet |
| :------------------ | :------ |
| nincs cél, `--dry-run` nélkül | `no destination given: use --output, --output-dir or --in-place` |
| két cél | `--output, --output-dir and --in-place exclude one another` |
| `--output` köteggel | `--output names one file but several were given: use --output-dir instead` |
| két bemenet, egy cél | `<destination>: would be written by both <input> and <input>` |
| egy bemenet felülíródna | `<destination>: would be written over the input <input>` — ugyanez a mondat az [`append`](append.md) és a [`split-file`](split-file.md) esetén |
| nem létrehozható kimeneti könyvtár | `<directory>: cannot be created: permission denied`, vagy `: cannot be created` |
| nem írható cél | `<path>: <destination>: cannot be written: permission denied`, vagy `: cannot be written` |
| a megírt kódolásból hiányzó karakter | `<path>: holds a character the chosen encoding cannot write` |

**Az utolsó megér egy mondatot.** A fájl **abban a kódolásban íródik vissza, amelyben olvasták**, ahogy a sorvégeivel és a jelölőjével is. Ha egy módosítás olyan karaktert vitt bele, amelyet ez a kódolás nem ismer — például egy `ł` karaktert Latin-1-ben —, az írás hibázik, és **semmi sem íródik**. Ha `?` karakterre cserélnék, az szöveg elvesztése lenne annak szeme láttára, aki az imént megírta.

## Megnézés írás előtt

**A `--dry-run`** mind a tizenhat író alparancsnál elfogadott, és mindenhol ugyanazt jelenti: **olvas, számol, jelent, nem ír fájlt**. A sorhoz, amelyet futtatni készültek, hozzáteszik; az alkalmazás ugyanannak a sornak az újrafuttatása nélküle. A számítás determinisztikus: amit a `--dry-run` megmutatott, azt írja meg a következő futtatás, amíg a fájl nem változik.

- **Nem ír fájlt, és nem hoz létre könyvtárat**, az `--in-place` esetén sem: a bemeneti fájl úgy marad, ahogy volt.
- **Nem követel célt.** Meg lehet adni egyet, és ekkor **úgy ellenőrzik, mint a `--dry-run` nélkül**: két bemenet ugyanahhoz a célhoz, egy bemenet, amely felülíródna, `--output` köteggel, két mód a hely megadására — az a parancssor, amelyet a `--dry-run` nélkül elutasítanak, vele is elutasításra kerül, ugyanazzal a mondattal és ugyanazzal az `1` kóddal.
- **A kilépési kód a valódi futtatásé**: `0`, ha minden kiszámolásra került, `2` vagy `3`, ha fájlokat nem lehetett olvasni vagy feldolgozni, `1` használati hiba esetén. Az a karakter, amelyet a fájl kódolása nem tudna írni, éppúgy megbuktatja a próbafuttatást, mint a valódit. **Az, hogy vannak módosítások, nem változtatja a kódot**: a négy kód azt mondja, hogy az eszköz sikerrel járt-e, nem azt, hogy a fájl megfelel-e.
- **A beszámoló ezt közli.** Ahol a valódi futtatás `-> <destination>` szöveget ír, a próbafuttatás `(dry run, nothing written)` szöveget ír.

<!-- example: printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by 1 --dry-run a.srt; ls -- *.srt -->
```console
$ printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by 1 --dry-run a.srt; ls -- *.srt
a.srt: 1 subtitle shifted by 1.000 s (dry run, nothing written)
a.srt
```

**A szövegeket módosító alparancsnál** — [`hearing-impaired`](hearing-impaired.md), [`replace`](replace.md), [`case`](case.md), [`italics`](italics.md), [`dialogue-dashes`](dialogue-dashes.md) és [`correct`](correct.md) —, **a szabványos kimenet a javasolt módosításokat hordozza**. Módosított feliratsoronként egy blokk, a fájl sorrendjében:

```
<path>: subtitle <number>
- <the text before, line by line>
+ <the text after, line by line>
```

- a **szám** az, amelyet a feliratsor az olvasott fájlban visel, egytől számolva;
- az előtte álló szöveg minden sora elé `- ` kerül, az utána álló szöveg minden sora elé `+ `: a többsoros szöveg úgy olvasható, ahogy van, és az a sor, amely maga párbeszéd-kötőjellel kezdődik, `- - ` alakot ad;
- **az a feliratsor, amelyet a művelet eltávolítana**, ezt az első sorában közli: `<path>: subtitle <number> (removed)`, és nincs `+` sora;
- a fordítás feliratsora, a `--document translation` kapcsolóval, ugyanott `(translation)` jelzéssel jelentkezik — lásd [Fordítás](#fordítás);
- minden blokk első sora az elérési úttal kezdődik, hogy a `grep '^film.srt: '` megtalálja egy fájl blokkjait a kötegben;
- **nincs blokk**, ha semmi sem változna.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\n- [Grincement]\n- Qui est là ?\n\n' > mentions.srt; subedit-cli hearing-impaired --dry-run mentions.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\n- [Grincement]\n- Qui est là ?\n\n' > mentions.srt; subedit-cli hearing-impaired --dry-run mentions.srt
mentions.srt: 2 subtitles cleaned, 1 removed (dry run, nothing written)
mentions.srt: subtitle 1 (removed)
- [Bruit de pas]
mentions.srt: subtitle 2
- Attends [il tousse] Marie.
+ Attends Marie.
mentions.srt: subtitle 3
- - [Grincement]
- - Qui est là ?
+ Qui est là ?
```

**A lista JSON-ban ugyanez**: a fájl objektuma tartalmaz egy `changes` tömböt, lásd [az alakot](#az-alak), az `after` értéke `null` az eltávolított feliratsornál. **Csak akkor számolják ki, ha a `--dry-run` vagy a `--format json` kéri**: a több ezer feliratsoros köteg nem építi fel hiába.

A **többi alparancsnál** — [`convert`](convert.md), [`shift`](shift.md), [`transform`](transform.md), [`framerate`](framerate.md), [`snap`](snap.md), [`adjust`](adjust.md), [`sort`](sort.md), [`append`](append.md), [`split-file`](split-file.md), [`pair`](pair.md) — a szabványos kimenet azt hordozza, amit a valódi futtatásé hordozna, vagyis szövegben semmit; JSON-ban az egyes fájlok objektumát `"dry_run":true`, `"destination":null` és ugyanazokkal a `counts` értékekkel.

## Két kimenet, két szerep

| Kimenet | Mit hordoz |
| :----- | :--------------- |
| szabványos | **az eredmény, és csakis az eredmény** — az `inspect` jelentése, a szövegmódosító [`--dry-run`](#megnézés-írás-előtt) javasolt módosításai; a `--format json` kapcsolóval fájlonként egy objektum — futtatásonként egy az [`append`](append.md), a [`split-file`](split-file.md) és a [`pair`](pair.md) esetén |
| hiba | **minden más** — a beszámoló, a figyelmeztetések, a hibák |

Ez a megosztás teszi lehetővé, hogy az eredményt átirányítsák anélkül, hogy a történet is beleszivárogna:

```console
$ subedit-cli inspect *.srt > rapport.txt
```

A jelentés a fájlba kerül, a beszámoló a képernyőn marad.

## Szkript által olvasható kimenet

**A `--format json`** a szabványos kimenet szövegét **bemeneti fájlonként egy JSON-objektumra** cseréli, a JSON Lines formátumban: soronként egy tömör objektum, UTF-8 kódolásban jelölő nélkül, a sorvég `\n`. A `text` az alapértelmezés, és nem változik. Globális kapcsoló, ezért az alparancs előtt áll; a `text` és a `json` kivételével bármilyen érték használati hiba, `1` kód.

<!-- example: printf '1\n00:00:10,000 --> 00:00:12,000\nUn.\n\n' > a.srt; subedit-cli --format json shift --by 1 --output-dir sortie a.srt absent.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:10,000 --> 00:00:12,000\nUn.\n\n' > a.srt; subedit-cli --format json shift --by 1 --output-dir sortie a.srt absent.srt; echo "code=$?"
a.srt: 1 subtitle shifted by 1.000 s -> sortie/a.srt
{"schema":1,"command":"shift","file":"a.srt","ok":true,"dry_run":false,"destination":"sortie/a.srt","counts":{"subtitles":1,"shifted_by_ms":1000},"warnings":[]}
absent.srt: does not exist
{"schema":1,"command":"shift","file":"absent.srt","ok":false,"error":{"kind":"not-found","message":"absent.srt: does not exist"}}
1 of 2 files shifted, 1 failed
code=3
```

**Bemenetenként egy objektum, sosem nulla, sosem kettő**, akár sikerült a bemenet, akár nem, a bemenetek megadásának sorrendjében — vagy a [`--recursive`](lots.md) kapcsolóval megtalálásuk sorrendjében. Az ember a sorokat számolja, a fájljait megtalálja. **Három alparancs a kivétel, mert a számosságuk nem a kötegé: az [`append`](append.md) (N bemenet, egy kimenet), a [`split-file`](split-file.md) (egy bemenet, két kimenet) és a [`pair`](pair.md) (két fájl, egy kimenet) az egész futtatásra **egy objektumot** ír, amely azt a fájlt nevezi meg, amelyet az oldaluk mond.** **Az átíró alparancsok tehát a szabványos kimenetre is írnak**, amit a szerződésük szövegben nem tett: a fájl marad az eredményük, az objektum pedig a jelentés róla.

**A beszámoló nem változik.** A `-q`, a `-v`, a `-vv` és a `-vvv` a hibakimenetre hat, szövegben, mint a `--format` nélkül; **a szabványos kimenet nem változik velük**. Az olvasási diagnosztika, amelyet a szöveg a 3. szinten tart, adat, és **mindig** a `warnings` mezőben van. A használati hiba nem ír objektumot: bármely fájl érintése előtt észlelik, így nincs mihez csatolni; a hibakimenetre kerül, `1` kóddal, és a szabványos kimenet üres marad.

### Az alak

| Kulcs | Tartalom | Jelen van |
| :-- | :------ | :------- |
| `schema` | egész szám, ennek az alaknak a verziója — `1` | mindig |
| `command` | az alparancs neve | mindig |
| `file` | az elérési út **ahogy megadták**, vagy ahogy a könyvtár bejárása összeállította | mindig |
| `ok` | logikai érték | mindig |
| `error` | `{"kind", "message"}` | ha az `ok` hamis |
| `dry_run` | logikai érték: igaz a [`--dry-run`](#megnézés-írás-előtt) esetén | ha az `ok` igaz, író alparancsnál |
| `destination` | a megírt elérési út, vagy `null`, ha a `dry_run` igaz: semmi sem íródott | ugyanígy |
| `counts` | **egész számok** objektuma, az alparancsra jellemző | ugyanígy |
| `changes` | `{"subtitle", "document", "before", "after"}` elemek tömbje: a módosított szövegek, lásd alább | szövegeket módosító alparancsnál |
| `warnings` | `{"kind", "line"?, "detail"?, "settled"?}` elemek tömbje | ha az `ok` igaz |
| `alignment` | hogyan helyezték el a fordítást, lásd [Fordítás](#fordítás) | a `-t` kapcsolóval, és a `pair` esetén |
| `constraints` | a használt beállítások | `adjust` |
| `settings` | a beolvasott beállításfájl, vagy `null`, ha egyet sem olvastak — lásd [A beállításfájl](#a-beállításfájl) | `adjust`, `correct`, ha az `ok` igaz |
| `inputs`, `tail` | fűzött fájlonként egy elem; a vég elérési útja | `append`; `split-file` |

A `line` hiányzik az egész fájlra vonatkozó diagnosztikából; a `detail` akkor, ha nincs mit hozzáfűzni; a `settled` értéke `true`, ha az olvasás **eldöntött** valamit — újragenerált számozást —, és hiányzik, ha úgy hagyta a dolgokat, ahogy voltak.

**A `changes`** a fájl sorrendjében felsorol minden feliratsort, amelynek a szövege változik: a `subtitle` a száma az olvasott fájlban, egytől számolva; a `document` értéke `"main"` vagy `"translation"`; a `before` az előtte álló szöveg; az `after` az utána álló szöveg, vagy **`null`, ha a feliratsort eltávolítják**. A szöveg sortörései benne `\n` alakúak. Valódi futtatásnál úgy íródik, mint a [`--dry-run`](#megnézés-írás-előtt) esetén, és **üres (`[]`), ha semmi sem változik**: jelenléte azt mondja, hogy az alparancs felsorolja a módosításait, hiánya azt, hogy nincsenek.

**Sehol sincs lebegőpontos szám.** A pozíció vagy az időtartam **ezredmásodpercben** megadott egész szám (`…_ms`); a számláló egész szám; a képkockasebesség **szöveg** (`"25"`, `"24000/1001"`), mert pontos, amit a racionális lebegőpontos szám nem tud tartani; a rács koncentrációja **ezrelékben** megadott egész szám — az `1000` tökéletes rács. Két kimenet így bájtról bájtra összehasonlítható, kerekítés nélkül.

**Az az elérési út, amely nem UTF-8** — a Linux megengedi —, nem ábrázolható JSON-ban: az érvénytelen bájtok helyén U+FFFD karakterrel íródik, és az objektum hordozza a `path-not-utf8` figyelmeztetést. Az ilyen útvonal hűségét nem ígérik, és ezt kimondják.

### Amit az egyes alparancsok a `counts` mezőbe írnak

| Alparancs | A `counts` kulcsai |
| :------------ | :--------------- |
| [`shift`](shift.md) | `subtitles`, `shifted_by_ms` (előjeles) |
| [`transform`](transform.md) | `subtitles` |
| [`framerate`](framerate.md) | `subtitles` |
| [`snap`](snap.md) | `subtitles`, `moved` (elmozdított pozíciók), `furthest_ms` (a legnagyobb elmozdítás) |
| [`hearing-impaired`](hearing-impaired.md) | `cleaned` (átírt szövegek), `removed` (eltávolított feliratsorok) |
| [`replace`](replace.md) | `replaced` (a módosított feliratsorokban lecserélt találatok), `matched` (talált találatok) |
| [`case`](case.md), [`italics`](italics.md), [`dialogue-dashes`](dialogue-dashes.md) | `changed` (azok a feliratsorok, amelyeknek a szövege megváltozott) |
| [`correct`](correct.md) | `corrected` (módosított szövegek), `removed` (eltávolított feliratsorok) — sosem találatok |
| [`sort`](sort.md) | `subtitles`, `moved` (a helyek, ahol feliratsor változott) |
| [`adjust`](adjust.md) | `subtitles` (a megcélzott feliratsorok), `adjusted` (azok, amelyeknek a vége elmozdult), majd a `sacrificed` csoport: `speed`, `minimum`, `gap`; és a `counts` mezőn kívül a `constraints` — a használt beállítások |
| [`convert`](convert.md) | `subtitles`, majd amit az átalakítás elvesztett: `lost_ends` és `lost_header` (0 vagy 1), `joined_lines`, `lost_tags`, `lost_fields`, `furthest_ms` |
| [`append`](append.md) | `files`, `subtitles`, `appended`, majd `lost_ends`, `lost_header`, `joined_lines`, `lost_tags`, `lost_fields`, `furthest_ms` összegezve; és a `counts` mezőn kívül az `inputs` — fűzött fájlonként egy elem |
| [`split-file`](split-file.md) | `subtitles`, `head`, `tail`; és a `counts` mezőn kívül a `tail` — a vég elérési útja (a `destination` az eleje) |
| [`pair`](pair.md) | `subtitles` (a megírt fájlé); és a `counts` mezőn kívül az `alignment` |
| [`inspect`](inspect.md) | nincs `counts`: a fájl leírása, lásd az oldalát |

### Hibaazonosítók

Az `error.kind` ezek egyike; **az olvasó az ismeretlen azonosítót „más”-nak tekinti**.

| `kind` | Mi ez |
| :----- | :----------- |
| `not-found`, `permission-denied`, `io` | a rendszer megtagadta a fájl olvasását — vagy írását |
| `unknown-format`, `no-subtitle-found`, `undecodable` | a fájl beolvasott, és nem hagyja magát olvasni |
| `unencodable` | olyan karakter, amelyet a választott kódolás nem tud írni |
| `no-byte-order-mark`, `no-frame-rate` | amit a `convert` nem tud megírni, ha nem mondják meg neki |
| `before-the-origin`, `beyond-the-end`, `no-transform`, `no-grid` | olyan művelet, amely nem alkalmazható erre a fájlra |
| `range-out-of-bounds` | olyan `--range`, amelyet ez a fájl nem tartalmaz |
| `no-style` | olyan formátum, amely nem ír stílust (TMPlayer, LRC), az `italics` esetén |
| `refused` | a művelet bármely más elutasítása |

### Amit ígérnek, és amit nem

**Ígért, amíg a `schema` értéke `1`**: a kulcsok nevei, típusuk és mértékegységük; az `ok`, a `file` és a `destination` jelentése; **bemenetenként egy objektum, a bemenetek sorrendjében** — futtatásonként egy a három, saját számosságú alparancsnál; a létező `kind` azonosítók; a determinizmus — ugyanazok a bemenetek, ugyanazok az argumentumok, ugyanazok a bájtok.

**Nem ígért**: az **üzenetek szövege** (`message`, `detail`), emberi olvasásra szánt angol, amelyet javítanak, ha kell — az a szkript, amely összehasonlítja, előre elromlott; a **kulcsok sorrendje** az objektumban, rögzített, de a JSON nem garantálja; a **még nem hozzáadott kulcsok** jelenléte.

**Az olvasó ezért figyelmen kívül hagyja az ismeretlen kulcsot**, és az ismeretlen `kind` értéket „más”-nak tekinti: kulcs vagy azonosító hozzáadása nem változtatja a `schema` értékét. Kulcs eltávolítása vagy átnevezése, típus vagy mértékegység módosítása, azonosító eltávolítása növeli — ez törés, amelyet a CHANGELOG és ez az oldal bejelent.

## Beszámolási szintek

Minden szint **tartalmazza az előzőt**: egy fokkal feljebb lépve hozzátesz, sosem cserél.

| Szint | Hogyan | Mit hordoz a hibakimenet |
| :----- | :------ | :------------------------------ |
| 0 | `--quiet` | semmit, **a hibák kivételével** |
| 1 | alapértelmezés szerint, vagy `-v` | feldolgozott fájlonként egy sor, és összegzés, amint több van |
| 2 | `-vv` | és amit felismertek: formátum, kódolás, BOM, sorvégek |
| 3 | `-vvv` | és a hibakeresési nyom: olvasott és írt bájtok, és **minden olvasási diagnosztika** |

**A hibákat soha nem hallgatják el**, a `--quiet` kapcsolóval sem. Az a parancs, amely némán hibázik, a kilépési kódján kívül semmit sem hagyna.

Az összegzés csak két fájltól jelenik meg: egyetlen bemenetnél megismételné az előtte álló sort.

## Olvasási diagnosztika

A feliratformátumokat **a lehető legjobban** olvassák: az anomália előtt az olvasó nem áll meg — dönt, vagy úgy hagyja a dolgokat, ahogy vannak, és ezt közli. Amibe beleütközött, azt a **3. szinten** adja ki, diagnosztikánként egy sorban, a hibakimeneten:

```
a.srt: 2 diagnostics while reading
a.srt: line 6: SubRip numbers that do not follow ("7"), settled by the reader
a.srt: line 9: a line that fits nowhere, left as it stands
```

Mind a tizenhét alparancs jelenti őket, nemcsak az [`inspect`](inspect.md): a lehető legjobban beolvasott, majd visszaírt fájl ugyanezeken a döntéseken ment át, és ha csendben maradnának, azt lehetne hinni, hogy nem történt semmi.

**A diagnosztika sosem hiba.** A fájlt beolvasták, a parancs sikerrel járt, a kilépési kód `0`. Ezért a legrészletesebb szinten élnek: lejjebb elnyomnák azt a sort, amely azt mondja meg, mi történt valójában.

### Mit hordoz az egyes sor

| Rész | Mit mond |
| :----- | :------------- |
| `line N` | hol, 1-től számolva, ahogy a szerkesztő mutatja; **hiányzik** az egész fájlra vonatkozónál |
| a mondat | amibe beleütköztek, az alábbi tizenkét kategória közül |
| `("…")` | a fájl hibás szövege, ha a kategória nem elég; 80 bájtra csonkolva |
| a vég | mit tettek vele: `settled by the reader`, vagy `left as it stands` |

**Két kategóriának nincs sora**, és ezek a kódolásra vonatkozó kettő: a bájtokat megmérték, vagy a jelölőjüket kiolvasták, mielőtt a fájl egyetlen sora létezett volna. A soruk ezért a mondattal kezdődik, `line N` nélkül — nincs hely, amelyet meg lehetne nevezni.

**A sor vége számít a legtöbbet.** A `settled by the reader` azt jelenti, hogy az olvasó döntött, és a megírt fájl hordozza a döntését — a hiányzó számozás újragenerálódik. A `left as it stands` azt jelenti, hogy semmihez sem nyúlt, mert **csak a felhasználó dönthet**: az a feliratsor, amely azelőtt ér véget, hogy elkezdődne, úgy marad, ahogy van.

### A blokk sorszáma

Az **egész blokkra** vonatkozó anomália az időzítősorhoz van horgonyozva, és nem ahhoz a pontos sorhoz, amely kiváltja. Az 5. sorban megírt következetlen számozást ezért a 6. sorban jelentik, a rákövetkező időzítősoréban — a hibás szám a `("…")` részben van. Csak az **egy sorra** vonatkozó anomáliákat — olvashatatlan időzítősor, az első előtti szöveg — jelentik önmagukon.

### A tizenkét kategória

| Mondat | Mi váltja ki |
| :----- | :------------------ |
| `a line that fits nowhere` | olyan sor, amely egyetlen blokkba sem tartozik |
| `a timing line that could not be read` | olvashatatlan időzítősor |
| `a SubRip block without its number` | hiányzó számozás, íráskor újragenerálva |
| `SubRip numbers that do not follow` | ugráló számozás |
| `text before the first timing line` | szöveg az első időzítősor előtt |
| `a WebVTT block of an unknown kind` | fel nem ismert WebVTT-blokk |
| `declares an event column this tool cannot fill` | Sub Station Alpha eseményoszlop, amelyet itt semmi sem tölt ki |
| `counts in frames and states no rate; it was read at` | képkockákban számoló fájl, amely nem közöl képkockasebességet — a használt a részlet |
| `carries no end times; each one was taken from the next start` | vég nélküli formátum, TMPlayer vagy LRC: **a táblázatban egyetlen vég sem a fájlból származik** |
| `more than one kind of line ending` | vegyes sorvégek |
| `an encoding nothing declared` | a kódolást a bájtok mérlegelése javasolta, és nem UTF-8 |
| `a byte order mark that contradicts the encoding asked for` | a fájl jelölőt hordoz, és az `--encoding` másikat nevezett meg — a jelölő nyer |

**A középső három a 9. fázisé**, és e három közül az utolsó kettő nem egy sorról beszél, hanem az egész fájlról: azt mondják, hogy egy megjelenített értéket nem olvastak ki. Egy LRC-n kitöltött `End` oszlop, egy MicroDVD-n egy pozíció — a fájl semmit sem hordozott ezekből.

**Egy művelet hozzáadhatja a sajátját**, amely nem olvasási diagnosztika: a [`correct`](correct.md#minta-amely-nem-alkalmazható) közli, a feliratsorral együtt, azt a mintát, amely feladta. Az 1. szinten közlik, és a fájl objektumában a `pattern-failed` azonosítót hordozza — sosem a fájl hibája.

## Fordítás

A fordításfájlt **úgy dolgozzák fel, mint bármely feliratfájlt**: a `subedit-cli case --to sentence film.fr.srt` semmi újjal nem módosítja. Amit a párosítás hozzáad, és amit három kapcsoló kimond, az szűk: **tudni, hogyan esnek a fordítás sorai a főfájl feliratsoraira** — ez az `inspect -t` —, és **a fordítás módosítása, miközben a főfájlra helyezve tartják**, ugyanazokkal a szabályokkal, mint az ablak.

| Kapcsoló | Hol | Érték | Alapértelmezés |
| :----- | :---- | :----- | :----- |
| `-t`, `--translation-file` | `inspect`, `hearing-impaired`, `replace`, `case`, `italics`, `dialogue-dashes`, `correct`, `pair` | a fordításfájl | — |
| `--document` | a hat szövegalparancs, nem az `inspect` és nem a `pair` | `main` vagy `translation` | `main` |
| `--align-method` | mint a `-t` | `position` vagy `number` | `position` |

- **A `position`** minden sor közepét hasonlítja össze a feliratsorok határaival: a hiányzó sor nem tolja el a következőket. **A `number`** az *n*. sort az *n*. feliratsorra küldi, a pozíciók nézése nélkül. A pozíció az alapértelmezés, abból az okból, amiért az ablaké is: a közepén hiányzó sor eltolja a számmal párosított fordítás egész maradékát.
- **A `--document translation` megköveteli a `-t` kapcsolót, és a `-t` megköveteli a `--document translation` kapcsolót** a szöveges alparancsnál — az egyik a másik nélkül használati hiba, `1` kód, mert az olvasott fájl, amelyet egyetlen mozdulat sem használ, mulasztás, nem preferencia. Az `inspect` alparancson a `-t` mellé nincs `--document`: jelent. **Az `--align-method` a `-t` nélkül szintén használati hiba.**
- **A `-t` egy fájlt nevez meg, ezért csak egyetlen bemenetnél van értelme**: több bemenettel, vagy a `--recursive` által vett könyvtárral úgy utasítják el, mint az `--output` kapcsolót, **egyetlen fájl olvasása előtt**. Köteg esetén párosonként egy hívást kell futtatni.
- **A főfájllal azonos fordításfájl elutasításra kerül** — bárhogyan is írják az útvonalát: `<translation>: the file is already open as the main document`, `2` kód.
- **Csak a megcélzott dokumentum íródik meg**, **a saját útvonalán és a saját formátumában**: a fordításfájl formátumában, kódolásában és sorvégeivel. Az `--output` a fordítás fájlját nevezi meg, az `--output-dir` az alapnevét teszi oda, az `--in-place` átírja; **a főfájlhoz soha nem nyúlnak**. A jelentés útvonalai a fordításéi.
- **A feliratsorok, amelyeket a párosítás életre kelt** — az a sor, amelyet semmi sem ért el — a fordításba **a saját pozícióikkal** íródnak, bármelyik módszerrel; **az a feliratsor, amelyet senki sem fordított le, szöveg nélküli blokként íródik**, amely anomália nélkül olvasható.
- **Az `--encoding` és a `--frame-rate` mindkét fájlra vonatkozik.**
- A **pozíció-alparancsok** — `shift`, `transform`, `framerate`, `snap`, `adjust` — nem rendelkeznek ezekkel a kapcsolókkal: mindkét fájlt meg kell adni nekik, mint egy kötegnek, `subedit-cli shift --by 2 film.srt film.fr.srt --output-dir out/`.
- **A főfájlra újraidőzített fordítás megírása** — az az egy dolog, amit a párosítás tesz, és a fájl egyedül nem — a [`pair`](pair.md) dolga, amelynek sem `--document`, sem `--recursive` kapcsolója nincs.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\n[Un oiseau] Bonjour.\n\n2\n00:00:03,000 --> 00:00:04,000\nAu revoir.\n\n' > film.fr.srt; subedit-cli hearing-impaired film.srt -t film.fr.srt --document translation --output propre.fr.srt; cat propre.fr.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\n[Un oiseau] Bonjour.\n\n2\n00:00:03,000 --> 00:00:04,000\nAu revoir.\n\n' > film.fr.srt; subedit-cli hearing-impaired film.srt -t film.fr.srt --document translation --output propre.fr.srt; cat propre.fr.srt
film.fr.srt: 1 subtitle cleaned, 0 removed -> propre.fr.srt
1
00:00:01,000 --> 00:00:02,000
Bonjour.

2
00:00:03,000 --> 00:00:04,000
Au revoir.
```

A `--format json` kapcsolóval az egyes fájlok objektuma az `alignment` kulcsot is hordozza, amely megmondja, hogyan helyezték el a fordítást — `{"file", "method", "attached", "born", "untranslated", "out_of_order"}`, mint az [`inspect`](inspect.md#json-ban) `translation` kulcsa —, és **a `--dry-run` kapcsolóval** a `changes` felsorolja a fordítás feliratsorait, mindegyiket `"document":"translation"` jelzéssel.

## Több fájl

Minden alparancs több elérési utat is elfogad, kivéve az [`append`](append.md), a [`split-file`](split-file.md) és a [`pair`](pair.md) parancsot, amelyek számossága az oldalukon szereplő. Mindegyiket függetlenül dolgozzák fel: az egyik hibája nem szakítja meg a többit, és a hibákat a fájl és az ok megnevezésével jelentik.

**A könyvtár a benne lévő fájlokat jelenti, a `--recursive` (`-r`) kapcsolóval**: e kapcsoló nélkül egyet megadni használati hiba, `1` kód, és semmit sem dolgoznak fel — jobb elutasítani, mint semmit feldolgozni és `0` értéket visszaadni:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?" -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?"
films: is a directory: use --recursive
code=1
```

A bejárást, az `--output-dir` alatt megtartott fát és az, ami kimarad, a [Könyvtárfa feldolgozása](lots.md) írja le.

## A kézikönyvoldalak

A telepítés **alparancsonként egy kézikönyvoldalt** tesz a `subedit-cli(1)` mellé: `subedit-cli-shift(1)`, `subedit-cli-convert(1)` és a többi, [az alparancstáblázat](#alparancsok) minden sorához egyet. A `subedit-cli(1)` ezt a fejezetet tartalmazza — a globális kapcsolókat, a kimeneteket, a kilépési kódokat —, és minden más oldal az alparancsa fejezetét, a példáival együtt. Az ablak oldala, a `subedit-gui(1)`, mellettük van.

```console
$ man subedit-cli
$ man subedit-cli-shift
```

**Az oldalak nincsenek ettől a kézikönyvtől külön megírva**: a program fordításakor generálják őket belőle, így azt mondják, amit ő, és nem térhetnek el tőle. Az egyes oldalak FÁJLOK szakasza megnevezi azt a könyvtárat, amelybe a kézikönyv települ.

| Oldalak | Hová települnek |
| :---- | :----------------------- |
| angol | `share/man/man1` |
| minden telepített nyelv — katalán, cseh, német, spanyol, finn, francia, galíciai, héber, magyar, interlingue, izlandi, olasz, holland, lengyel, portugál, brazil portugál, orosz, szerb, török, egyszerűsített kínai | `share/man/<language>/man1` — `share/man/fr/man1` |

**A `man` a nyelvet a területi beállításból veszi**, ahogy a program is: francia területi beállítással a `man subedit-cli` a francia oldalt mutatja, és az a nyelv, amelynek az oldalai nincsenek telepítve, az angolokat kapja. A `-L` a területi beállítástól függetlenül kér nyelvet:

```console
$ man -L fr subedit-cli-shift
```

## Kilépési kódok

| Kód | Jelentés |
| :--- | :------------ |
| `0` | minden sikerült |
| `1` | használati hiba — ismeretlen kapcsoló, érvénytelen érték, tiltott kombináció |
| `2` | egyetlen fájlt sem lehetett feldolgozni |
| `3` | néhány fájlt feldolgoztak, másokat nem |

A `2` és a `3` azért különbözik, hogy a szkript tudja, mit tegyen, anélkül hogy újra elolvasná a kimenetet: a „semmi sem működött” és az „egy hiányzik” nem ugyanazt a reakciót kívánja.

A használati hibát **bármely feldolgozás előtt** észlelik: sosem hagy félig feldolgozott köteget.

## Hibák

| Mi váltja ki | Mi íródik ki a hibakimeneten |
| :------------------ | :--------------------------------------- |
| ismeretlen kapcsoló vagy alparancs | a hibás név, és a `--help` kapcsolóra utaló megjegyzés |
| `--quiet` a `-v` kapcsolóval | `--quiet and -v ask for opposite things; give one or the other` |
| `--config` a `--no-config` kapcsolóval | `--config excludes --no-config` |
| `--config` üres névvel | `--config needs a file name` |
| hiányzó fájl | `<path>: does not exist` |
| olvashatatlan fájl | `<path>: cannot be opened: permission denied` |
| saját jelölőt író kódolás | `"<name>" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do` |
| nem dekódolható bájtok | `<path>: cannot be decoded in the chosen encoding` |
| fel nem ismert formátum | `<path>: is in no format this tool knows` |
| semmi, ami feliratsorra hasonlítana | `<path>: holds nothing recognisable as a subtitle` |
| `--document translation` a `-t` nélkül | `--document translation needs the translation file: use -t` |
| `-t` a `--document translation` nélkül | `-t names a translation to change: use --document translation` |
| `--align-method` a `-t` nélkül | `--align-method needs a translation file: use -t` |
| `-t` több bemenettel, vagy könyvtárral | `-t names one file but several inputs were given: use one invocation per pair` |
| a főfájllal azonos fordítás | `<translation>: the file is already open as the main document` |
