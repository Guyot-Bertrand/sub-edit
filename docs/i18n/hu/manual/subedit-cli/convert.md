# `convert`

<!-- machine-translated -->
> Ez az oldal gépi fordítás, amelyet a nyelv egyik olvasója még nem nézett át.

```
subedit-cli convert --to <format>
                    [--line-endings unix|windows|mac] [--to-encoding NAME]
                    [--bom | --no-bom] [--sort] [--disjoint]
                    [--join-repeats] [--repair]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    <file>...
```
Minden fájlt a kért formátumban és alakban ír ki. **Soha nem változtatja meg
a bemenetet**, hacsak az `--in-place` kifejezetten nem kéri.

<!-- example: subedit-cli convert --help -->
```console
$ subedit-cli convert --help
Write a subtitle file out in another format or shape
Usage: subedit-cli convert [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to convert

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --to TEXT:{srt,vtt,subviewer2,ssa,ass,microdvd,mpl2,tmplayer,lrc} REQUIRED
                              Format to write
  --line-endings TEXT:{unix,windows,mac}
                              Line endings to write; the source's by default
  --to-encoding NAME          Encoding to write; the source's by default
  --frame-rate RATE           Frame rate of a file counted in frames: 25, 23.976
  --bom                       Write a byte order mark
  --no-bom                    Write no byte order mark
  --sort                      Put the subtitles in order of their start before writing
  --disjoint                  Cut overlapping subtitles so that no range overlaps another, before writing
  --join-repeats              Join the subtitles that repeat each other into one, before writing
  --repair                    Join the repeats, sort, then make the ranges disjoint, before writing
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
| `--to` | **igen** | az alábbi táblázat valamelyik neve, és semmi más | — |
| `--line-endings` | nem | `unix`, `windows` vagy `mac` | a beolvasott fájl sorvégei |
| `--to-encoding` | nem | bármely kódolás, amelyet az ICU ki tud írni, kivéve azokat, amelyek saját jelölőt írnak | a beolvasott fájl kódolása |
| `--bom` / `--no-bom` | nem | jelzők, egymást kizárják | amit a beolvasott fájl hordozott |
| `--sort` | nem | jelző: a feliratsorokat a kezdetük sorrendjébe állítja **íráskor**, lásd: [Rendezés íráskor](#rendezés-íráskor) | ki |
| `--join-repeats` | nem | jelző: az egymás után következő, azonos szöveg darabjaiból egyetlen feliratsort készít, **mindenek előtt**, lásd: [A tartományok átfedésmentessé tétele](#a-tartományok-átfedésmentessé-tétele) | ki |
| `--repair` | nem | jelző: ugyanaz, mint a `--join-repeats --sort --disjoint`, ebben a sorrendben | ki |
| `--disjoint` | nem | jelző: levágja az átfedő feliratsorokat **íráskor**, lásd: [A tartományok átfedésmentessé tétele](#a-tartományok-átfedésmentessé-tétele) | ki |
| `--output` / `--output-dir` / `--in-place` | **a háromból egy**, kivéve a `--dry-run` esetén | lásd: [Indítás](invocation.md#a-cél) | — |
| `--dry-run` | nem | jelző: kiszámít és közöl, nem ír — lásd: [Megnézés írás előtt](invocation.md#megnézés-írás-előtt) | ki |

A `mac` a magányos kocsivisszát (`\r`) nevezi meg, a klasszikus Mac OS sorvégét.

## A `--to` által elfogadott formátumok

| Név | Formátum | Kiírt kiterjesztés |
| :-- | :----- | :--------------- |
| `srt` | SubRip | `.srt` |
| `vtt` | WebVTT | `.vtt` |
| `subviewer2` | SubViewer 2.0 | `.sub` |
| `ssa` | Sub Station Alpha | `.ssa` |
| `ass` | Advanced SSA | `.ass` |
| `mpl2` | MPL2 | `.txt` |
| `microdvd` | MicroDVD | `.sub` |
| `tmplayer` | TMPlayer | `.txt` |
| `lrc` | LRC | `.lrc` |

**Név, és nem kiterjesztés.** Két kiterjesztés két-két formátumot nevez
meg — a `.sub` a MicroDVD-é is, a `.txt` az MPL2-é és a
TMPlayeré —, így a kiterjesztést fogadó kapcsoló nem tudná, melyiket
kérik. A fenti nevek egyértelműek, és egyeznek a
kiterjesztéssel is ott, ahol a kiterjesztés is egyértelmű.

**A lista zárt.** A Gaupol listája, és mind a kilenc szerepel benne.

## A két formátum, amely nem hordoz végpozíciót

**A TMPlayer és az LRC soronként csak egy pozíciót ír**, a kezdetet. Ez
mindkét oldalon megmutatkozik.

**Olvasáskor** a végpozíciók kikövetkeztetésre kerülnek: egy sor vége a
következő sor kezdete, az utolsó pedig öt másodpercet kap. Ezt teszi a Gaupol.
Mi is jelezzük, egy diagnosztikával, amelyet a `--verbose` mutat a három
alparancsnál:

```
file.lrc: carries no end times; each one was taken from the next start, settled by the reader
```
**Íráskor** a végpozíciók nem íródnak ki, mert nincs hová tenni őket.
Az átalakítás e két formátum egyikére ezért elveszíti őket, és az oda-vissza átalakítás
nem találja meg újra őket — újra kikövetkezteti.

**Az LRC még egy dolgot elveszít**: nem tud sortörést hordozni.
A két sorban írt párbeszédsor egy sorban íródik ki, a két sort
szóköz fűzi össze. Ez a veszteség sem pótolható.

Egyiknek sincs szókincse a dőlt betűkhöz.

## A `--frame-rate`, és az egyetlen formátum, amely képkockákban számol

**A MicroDVD nem másodpercekben számol**: a pozíciói képkockaszámok, és egyetlen MicroDVD fájl sem
jelzi, milyen sebességgel számolták őket. A `--frame-rate` megadja, és mindkét oldalon szolgál: ez az a
sebesség, amellyel a képkockákban számolt `.sub` beolvasásra kerül, és az, amellyel a
MicroDVD-re való átalakítás a képkockákat számolja.

| Kapcsoló | Kötelező | Értékek | Alapértelmezés |
| :----- | :----- | :------ | :----- |
| `--frame-rate` | nem | képkocka/másodperc sebesség: `25`, `23.976`, `29.97` | lásd alább |

**Olvasáskor `--frame-rate` nélkül a fájl 23,976 sebességgel olvasódik be** — a Gaupol
értéke —, és a `-vvv` szint ezt közli:
`counts in frames and states no rate; it was read at`. Ez az egyetlen hely az eszközben, ahol minden megjelenített pozíció
olyan feltevésen nyugszik, amelyet a fájl nem tud megerősíteni.

**Íráskor `--frame-rate` nélkül három eset és három válasz:**

| A kiinduló fájl | Mi történik |
| :------------------- | :-------------- |
| már MicroDVD | az a sebesség írja vissza, amely beolvasta, és a képkockák azonosan térnek vissza |
| idő alapú, és a pozíciói egy rácsra esnek | a rács lesz használva, és a `-vv` közli, melyik |
| idő alapú, rács nélkül | **elutasítás**, 2-es kód: `writing frames needs a frame rate, and the positions fall on no grid to take one from — give --frame-rate` |

Az elutasítás szándékos. Az egyetlen megmaradó lépés egy szám kitalálása lenne, és
a fájl minden sora annyival mozdulna el, amennyire a szám téves volt.

## Mit veszít el az átalakítás

**Másik formátumra átalakítani soha nem veszteségmentes**, és az, ami nem éli túl,
a párostól függ:

| Ami nem jut át | Miért |
| :--------------------- | :------- |
| a fejléc | csak a saját formátumában van értelme: az `[INFORMATION]` nem WebVTT fejléc |
| a formátumra jellemző adatok | a SubRip koordinátáinak, a WebVTT cue azonosítójának és beállításainak nincs megfelelője másutt |
| a pontosság | a SubViewer 2, a két Sub Station Alpha és az LRC századra, az MPL2 tizedre, a TMPlayer másodpercre, a MicroDVD képkockára ír; a `01:00:00,017` `01:00:00,020` lesz |
| a sor vége | a TMPlayernek és az LRC-nek nincs rá mezője; a következő olvasás újra kikövetkezteti |
| a sortörések | csak az LRC: a sorai egy sorba férnek, és a kettőt szóköz ragasztja össze |
| a címkék, amelyeket a célformátum nem tud kiírni | a WebVTT-nek nincs `<font color>`, a TMPlayernek és az LRC-nek semmi sincs |
| az elrendezés | `{\pos(x,y)}`, `{\an8}`, `<v Marie>`, `<ruby>`: a saját formátumukban túlélik az oda-vissza átalakítást, az átalakításkor eltűnnek |

**A fájl visszaírása a saját formátumában semmit sem veszít** — ez az a garancia,
amelyre minden más épül, és a `--to srt` egy `.srt` fájlon ugyanazokat a bájtokat adja vissza. Semmi
sem kerül még dekódolásra sem: a szöveg a beolvasott nyers karakterlánc marad.

## A címkék átalakulnak, nem másolódnak

**Minden formátum a maga módján írja a dőlt betűt**, és az átalakítás nélküli átalakítás
egy `{\i1}` jelet hagyna egy `.srt` fájlban: olyan szöveget, amelyet a felhasználó lát, olyan
fájlban, amely sosem fogja tudni értelmezni. Átjut **hat
dolog** — a félkövér, a dőlt betű, az aláhúzás, a szín, a betűtípus és a méret:

| Formátum | Félkövér | Dőlt | Aláhúzott | Szín | Betűtípus | Méret |
| :----- | :--: | :------: | :------: | :-----: | :----: | :----: |
| SubRip | `<b>` | `<i>` | `<u>` | `<font color="#RRGGBB">` | — | — |
| WebVTT | `<b>` | `<i>` | `<u>` | — | — | — |
| SubViewer 2 | `<b>` | `<i>` | `<u>` | `<font color="#RRGGBB">` | — | — |
| Sub Station Alpha | `{\b1}` | `{\i1}` | — | `{\c&HBBGGRR&}` | `{\fnNAME}` | `{\fsN}` |
| Advanced SSA | `{\b1}` | `{\i1}` | `{\u1}` | `{\c&HBBGGRR&}` | `{\fnNAME}` | `{\fsN}` |
| MicroDVD | `{Y:b}` | `{Y:i}` | `{Y:u}` | `{C:$BBGGRR}` | `{F:NAME}` | `{S:N}` |
| MPL2 | `\` | `/` | `_` | `{C:$BBGGRR}` | `{F:NAME}` | `{S:N}` |
| TMPlayer | — | — | — | — | — | — |
| LRC | — | — | — | — | — | — |

**A gondolatjel bejelentett veszteség.** A színes `.srt` WebVTT-re alakítása eltávolítja
a színt, és ezt közli; az előállított fájl WebVTT, nem álruhás SubRip.

**A MicroDVD és az MPL2 nem tudja a sor egy részét stílusossá tenni.** A címkéiknek
nincs végük: a `{y:i}` mindent dőlté tesz a sor végéig, az
`{Y:i}` pedig a feliratsor végéig. Az a sor, amelynek csak fele volt
félkövér, ezért egyáltalán nem félkövéren íródik ki, nem pedig túlcsorduló félkövérrel.

## Amiről az átalakítás azt jelzi, hogy elveszett

**Jelzi, ha veszít, és hallgat, ha nem veszít semmit.** Az a jelentés,
amely minden híváskor megjelenik, olyan jelentés, amelyet senki sem olvas.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,500\n<i>sur deux</i>\nlignes\n\n' > a.srt; subedit-cli convert --to lrc --output b.lrc a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,500\n<i>sur deux</i>\nlignes\n\n' > a.srt; subedit-cli convert --to lrc --output b.lrc a.srt
a.srt: 1 subtitle written as LRC -> b.lrc
a.srt: ends are not carried by LRC, line breaks were joined in 1 subtitle, 1 tag dropped
```
A sor az alapértelmezett bőbeszédűségi szinten jelenik meg, és minden magasabbon is.
A tételek ebben a sorrendben:

| Tétel | Mit mond |
| :---- | :----------- |
| `ends are not carried by <format>` | a célformátum nem ír végpozíciót |
| `line breaks were joined in N subtitles` | csak az LRC: N feliratsor több sorban volt |
| `N tags dropped` | N címkét nem lehetett kiírni, az elrendezést is beleértve |
| `the header was dropped` | a fejléc nem lép át a formátumhatáron |
| `the <format> fields of N subtitles were dropped` | N feliratsor a formátumára jellemző adatokat hordozott |
| `positions moved by up to N ms` | a legnagyobb távolság, amellyel egy pozíció elmozdult |

## A kódolás módosítása

**A `--to-encoding` a kiírt kódolást választja; az `--encoding` a beolvasottat
adja meg.** A kettő együtt újrakódol egy fájlt:

```console
$ subedit-cli convert --to srt --encoding cp1252 --to-encoding utf-8 \
      --output-dir sortie film.srt
```
A `--to-encoding` nélkül a fájl **abban a kódolásban íródik ki, amelyben beolvasták**,
ahogyan a sorvégeivel és a jelölőjével is történik: a kódolásra vonatkozó utasítás nélkül átalakított
fájl ugyanazokat a karaktereket adja vissza, ugyanazokkal a bájtokkal.

**Az a karakter, amelyet a célkódolás nem tud kiírni, leállítja a fájlt**, és
semmi sem íródik ki. Egy `ł`-nek nincs helye a Latin-1-ben; ha `?` jellel helyettesítenénk,
szöveg veszne el az olvasás és az írás között, anélkül hogy bárki látná.

**A bájtsorrend-jelölő csak az Unicode kódolásokhoz létezik.**
A `--bom` olyan `--to-encoding` mellett, amely nem hordoz ilyet, elutasításra kerül, nem pedig figyelmen kívül marad
— lásd az alábbi hibatáblázatot.

**És a kódolásnak meg kell neveznie a bájtsorrendjét.** Az `UTF-16` és az `UTF-32`
az ICU által ismert nevek, amelyek átalakítója **saját jelölőt ír**, bármit
kérnek is tőle: a `--no-bom` így elfogadásra kerülne, és nem teljesülne. Mindkettő
ezért elutasításra kerül, és az üzenet megnevezi a kimenetet:

```console
$ subedit-cli convert --to srt --to-encoding UTF-16 --output copie.srt film.srt
"UTF-16" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do
```
Az `UTF-16LE`, az `UTF-16BE`, az `UTF-32LE` és az `UTF-32BE` pontosan ugyanazokat a
bájtokat írja, a jelölőt is beleértve — de a `--bom` és a `--no-bom` irányítása alatt. Az
elutasítás tehát nem kerül semmibe, csak egy pontosabb névbe, és az olvasásra ugyanúgy érvényes, mint az írásra:
az `--encoding UTF-16` ugyanazt a választ kapja.

## A cél

A három kapcsoló és a szabályaik közösek az író
alparancsokban: lásd: [Indítás](invocation.md#a-cél). Egyik nélkül sem íródik ki
semmi.

A `convert`-re jellemző: **az `--output-dir` a formátummal együtt változtatja a
kiterjesztést.** A `convert --to vtt` az `a.srt` fájlon `a.vtt` fájlt ír, soha nem WebVTT-t tartalmazó
`a.srt` fájlt — a hazudó nevű fájl minden más eszközt megzavar.

## A helyben átalakítás elutasítva

Az `--in-place` olyan `--to` mellett, amely megváltoztatja a formátumot, `1` kódot ad, és semmit
sem ír ki: helyben nincs második név, amely az új formátumot hordozná, és
a fájl olyan kiterjesztés alatt maradna, amelyet a tartalma már nem indokol.

Az `--in-place` mégis használható **az alak formátum nélküli módosítására** —
például egy `.srt` fájl `.srt` fájlként való visszaírására más sorvégekkel.

Az elutasítás **egyedül a kiterjesztés alapján** dől el, bármiféle olvasás előtt: a használati
hiba nem hagyhat félig megírt köteget.

## Sorvégek és bájtsorrend-jelölő

Alapértelmezés szerint **a kiírt fájl átveszi azt, amit a beolvasott fájl hordozott**. A modell
mindkettőt megtartja olvasáskor; ha alapértelmezés szerint rákényszerítenénk őket, minden
átalakításkor elveszne egy szándékosan megőrzött információ.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli convert --to vtt --output b.vtt a.srt; cat b.vtt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli convert --to vtt --output b.vtt a.srt; cat b.vtt
a.srt: 1 subtitle written as WebVTT -> b.vtt
WEBVTT

00:01.000 --> 00:03.000
Bonjour.
```
## Rendezés íráskor

A `--sort` a fájlt **a kezdetek sorrendjében** írja ki, bármi legyen a célformátum. A rendezés **másolaton** történik: a beolvasott fájl
érintetlen marad, és az íródik ki, amit a [`sort`](sort.md) készítene belőle — stabilan, két együtt kezdődő feliratsor megtartja a sorrendjét.

**Alapértelmezés szerint soha**: a fájl sorrendje a szerzőjéé, amíg más nem kéri. A kapcsolóval minden fájl ezt jelzi az
első narrációs szinten: `<path>: N subtitles moved` vagy `<path>: already in order`, és a `--format json` rekord tartalmaz egy
`moved` számlálót — **a kapcsoló nélkül hiányzik**, hogy a meglévő szkriptek által olvasott alak ne változzon.

## A tartományok átfedésmentessé tétele

A `--disjoint` levágja az átfedő feliratsorokat, hogy egyetlen tartomány se fedjen át többé egy másikat, **ugyanazon a másolaton, mint a rendezés**: a beolvasott fájl érintetlen marad.

Alapértelmezés szerint soha. A kapcsoló azt teszi, amit a [`disjoint`](disjoint.md) **az alapértelmezett beállításaival**, íráskor, a rendezés után. Rendezett fájlra van szüksége: különben a program elutasítja, és a `--sort` — amely előbb fut — előtte sorrendbe állítja. A fájl ezt jelzi az első narrációs szinten (`<path>: N subtitles split, M merged`), és a `--format json` rekord tartalmazza a `split` és a `merged` számlálót, amelyek **a kapcsoló nélkül hiányoznak**.

**A `--join-repeats` a rendezés és a vágás előtt jön**: az egymás után következő darabokra vágott szöveg előbb újraépül (lásd:
[`join-repeats`](join-repeats.md)), majd a feliratsorok rendezésre kerülnek, majd a tartományok átfedésmentessé válnak. **A `--repair` mind a hármat egyszerre jelenti**,
mint a [`repair`](repair.md), a vágás alapértelmezett beállításaival. A `--join-repeats` esetén minden fájl
`<path>: N repeated subtitles joined` sort közöl az első narrációs szinten, és a `--format json` rekord tartalmazza a `counts.joined` mezőt — **a kapcsoló nélkül hiányzik**.

## Kimenet

**Szabványos kimenet** — semmi: az eredmény a kiírt fájl. A `--dry-run` esetén sem semmi más: nincs fájl, és a narrációs sor ezt mondja — lásd: [Megnézés írás előtt](invocation.md#megnézés-írás-előtt).

A hibakimeneten, a szint szerint:

| Szint | Mi kerül hozzá |
| :----- | :-------------- |
| 1 | `<path>: N subtitles written as WebVTT -> <destination>` |
| 2 | `<path>: SubRip -> WebVTT, UTF-8, no BOM, LF line endings` — a megnevezett kódolás a kiírt kódolás |
| 3 | `<path>: N bytes read, M written`, majd **minden olvasási diagnosztika** — lásd: [Indítás](invocation.md#olvasási-diagnosztika) |

### JSON-ban

A [`--format json`](invocation.md#szkript-által-olvasható-kimenet) esetén minden fájl objektuma tartalmazza a `counts.subtitles` mezőt, majd azt, amit az átalakítás elveszített: `lost_ends` és `lost_header` (`0` vagy `1`), `joined_lines`, `lost_tags`, `lost_fields` és `furthest_ms` — a [veszteségmondat](#amiről-az-átalakítás-azt-jelzi-hogy-elveszett) tételei. A teljes táblázat az [Indítás](invocation.md#amit-az-egyes-alparancsok-a-counts-mezőbe-írnak) oldalán található.

## Kilépési kódok

Az eszköz általános kódjai: `0`, ha minden fájl kiírása megtörtént, `2`, ha egyiké sem, `3`, ha
csak néhányé, `1` használati hiba esetén.

## Hibák

| Mi váltja ki | Üzenet |
| :------------------ | :------ |
| `--bom` a `--no-bom` mellett | `--bom and --no-bom ask for opposite things; give one or the other` |
| a formátumot megváltoztató `--in-place` | `--in-place cannot change the format: the file would keep a name its content no longer matches` |
| `--bom` olyan kódoláson, amelynek nincs | `<path>: <encoding> has no byte order mark to write` |
| ismeretlen kódolást megnevező `--to-encoding` | `no encoding is named "<name>"` |
| saját jelölőt író kódolást megnevező `--to-encoding` | `"<name>" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do` |
| a kiírt kódolásból hiányzó karakter | `<path>: holds a character the chosen encoding cannot write` |

**A `--bom` elutasítása olyan kódoláson, amely nem hordoz ilyet, nem
szőrszálhasogatás.** A bájtsorrend-jelölő az Unicode kódolásokhoz létezik, és
máshoz nem: ha egy Windows-1252 fájlon kérnek ilyet, olyasmit kérnek,
ami nem létezik. A fájl jelölő nélkül íródna ki, és senki sem
tudná, hogy a kérés nem teljesült.

A cél hibái közösek az író alparancsokban:
lásd: [Indítás](invocation.md#a-cél).

Az olvasási hibák azonosak az [`inspect`](inspect.md) hibáival: a bemeneti fájlra
vonatkoznak, és ugyanazok az üzeneteik.
