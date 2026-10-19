# `append`

<!-- machine-translated -->
> Ez az oldal gépi fordítás, amelyet a nyelv egyik olvasója még nem nézett át.

```
subedit-cli append --output FILE
                   [--dry-run] [--sort] [--disjoint]
                   <base> <file>...
```
Fájlokat fűz **az első után**, **egyetlen** fájlba: a két részben lévő filmből
egyetlen feliratkészlet lesz. Az ablak `Append File…` művelete — [ugyanaz a motor](../subedit-gui/operations.md#append-file),
ugyanazok a számlálók —, annyiszor alkalmazva, ahány hozzáfűzendő fájl van.

- **Az első fájl az alap.** Ez adja az eredmény **formátumát, kódolását, bájtsorrend-jelölőjét és
  sorvégeit**.
- **Két fájl közé semmi sem kerül**: mindegyik **az előtte álló rész utolsó feliratsorának végével**
  tolódik el, a pozíciói megtartják a sorrendjüket és a távolságukat. A harmadik a másodikat követi, nem az
  alapot: a fájlok abban a sorrendben követik egymást, ahogyan meg vannak adva.
- **Minden hozzáfűzött fájl átalakításon megy át** az alap formátumára: a címkéi átalakulnak,
  és azt, amit az a formátum nem tud kiírni, a program fájlonként közli, az ablak szavaival.
- **A hozzáfűzött fájloknak nincs fordításuk**: az önállóan megnyitott fájlnak nincs mit adnia.
- **Egy fájl többször is megadható**, vagy önmagához is hozzáfűzhető: semmi sem tiltja.

**Az `append` nem köteg.** N bemenet, **egy** kimenet: a többi alparancs ciklusa — fájlonként egy eredmény,
és egy hiba, amely után a többi folytatódik — itt értelmetlen. Ebből három dolog következik:

- **a cél a `--output`, és semmi más.** Az `--output-dir` és az `--in-place` nem létezik az
  `append` alparancsnál, ezért mint bármely ismeretlen kapcsolót, elutasítja a program, `1` kóddal; ugyanez igaz a `--recursive` kapcsolóra: a
  fájlok sorrendje a megadott, a bejárt könyvtárfának pedig nincs sorrendje;
- **mindent vagy semmit**: az első fájl, amely nem olvasható, leállítja a futást, és **semmi sem íródik ki**. A
  három részben lévő film, amelynek az egyik része hiányzik, nem két részben lévő film;
- **futásonként egyetlen sor** az eredményről, és JSON-ban **egyetlen objektum**.

<!-- example: subedit-cli append --help -->
```console
$ subedit-cli append --help
Put subtitle files one after another into a single file
Usage: subedit-cli append [OPTIONS] files...

Positionals:
  files BASE FILE...          The base file, then the files to put after it, in order

Options:
  -h,--help                   Print this help message and exit
  --output FILE               File to write
  --dry-run                   Work out and say what would be written, and write nothing
  --sort                      Put the subtitles in order of their start before writing
  --disjoint                  Cut overlapping subtitles so that no range overlaps another, before writing
```
## Argumentumok és kapcsolók

| Kapcsoló | Kötelező | Értékek | Alapértelmezés |
| :----- | :----- | :------ | :----- |
| `<base> <file>...` | igen, **legalább kettő** | útvonalak: az alap, majd ami utána kerül, sorrendben | — |
| `--output` | **igen**, kivéve a `--dry-run` esetén | fájlútvonal; a könyvtára létrejön, ha hiányzik | — |
| `--dry-run` | nem | jelző: kiszámít és közöl, nem ír — lásd: [Megnézés írás előtt](invocation.md#megnézés-írás-előtt) | ki |
| `--sort` | nem | jelző: az egészet a kezdetek sorrendjébe állítja, **miután minden hozzá van fűzve**, lásd: [Az eredmény rendezése](#az-eredmény-rendezése) | ki |
| `--disjoint` | nem | jelző: levágja az átfedő feliratsorokat, **miután minden hozzá van fűzve és rendezve van**, lásd: [A tartományok átfedésmentessé tétele](#a-tartományok-átfedésmentessé-tétele) | ki |

**A cél nem lehet a bemenetek egyike sem**, az alapot is beleértve: a beolvasott fájl
fölé írást a program bármi beolvasása előtt elutasítja, `1` kóddal. A bemenetektől különböző, már létező `--output`
felülíródik, mint mindenhol.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n' > partie1.srt; printf '1\n00:00:00,500 --> 00:00:01,500\ntrois\n\n' > partie2.srt; subedit-cli append --output film.srt partie1.srt partie2.srt; cat film.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n' > partie1.srt; printf '1\n00:00:00,500 --> 00:00:01,500\ntrois\n\n' > partie2.srt; subedit-cli append --output film.srt partie1.srt partie2.srt; cat film.srt
partie2.srt: appended 1 subtitle
partie1.srt: 3 subtitles from 2 files -> film.srt
1
00:00:01,000 --> 00:00:02,000
un

2
00:00:03,000 --> 00:00:04,000
deux

3
00:00:04,500 --> 00:00:05,500
trois
```
A `partie1.srt` utolsó feliratsora 4 s-nál ér véget; a `partie2.srt` feliratsora, amely 0,5 s-nál kezdődött, 4,5 s-nál
kezdődik.

## Az eredmény rendezése

Minden fájl **az előtte álló rész végétől** tolódik el: a sorrend tehát a fájlok egymás közötti sorrendjénél sérülhet. A `--sort`
a kezdetek sorrendjébe állítja az eredményt, **egy menetben, az utolsó hozzáfűzés után**, ahogyan a [`sort`](sort.md) tenné — stabilan, a tartalom
megváltoztatása nélkül.

Alapértelmezés szerint soha. A kapcsolóval az alap ezt jelzi az első narrációs szinten: `<base>: N subtitles moved` vagy `<base>: already in order`, és
a `--format json` rekord tartalmaz egy `moved` számlálót, amely **a kapcsoló nélkül hiányzik**.

## A tartományok átfedésmentessé tétele

Alapértelmezés szerint soha. A kapcsoló azt teszi, amit a [`disjoint`](disjoint.md) **az alapértelmezett beállításaival**, miután minden hozzá van fűzve, az egészen. Rendezett fájlra van szüksége: különben a program elutasítja, és a `--sort` — amely előbb fut — előtte sorrendbe állítja. A fájl ezt jelzi az első narrációs szinten (`<path>: N subtitles split, M merged`), és a `--format json` rekord tartalmazza a `split` és a `merged` számlálót, amelyek **a kapcsoló nélkül hiányoznak**.

## Ha a formátumok eltérnek

Az eredmény **az alap formátumában** készül, bármi legyen a többié. Hogy mibe kerül az átlépés, azt
a program fájlonként közli, az ablak szavaival — ugyanazzal a mondattal, mint az `Append File…` és mint a
[más formátumból való beillesztés](../subedit-gui/clipboard.md#más-formátum-címkéit-átalakítja-a-program):

```text
d.ass: appended 1 subtitle; Advanced SSA into SubRip: 1 tag dropped, the header was dropped
```
Az, hogy mi vész el, csak **az egyes** hozzáfűzött fájlok formátumától függ: az Advanced SSA alapú fájl úgy kap
SubRip fájlt, hogy semmit sem veszít, fordítva nem. Az eredmény formátumának megválasztásához elöl olyan fájlt kell megadni,
amely már abban a formátumban van, vagy utólag át kell alakítani a [`convert`](convert.md) alparanccsal.

## Kimenet

**Szabványos kimenet** — semmi: az eredmény a kiírt fájl.

| Szint | Mi kerül még a hibakimenetre |
| :----- | :------------------------------------- |
| 1 | hozzáfűzött fájlonként egy sor — `<path>: appended N subtitles`, amelyet, ha a formátum valamibe került, `; <source format> into <base format>: <what is lost>` követ — majd `<base>: N subtitles from M files -> <destination>` (a `--dry-run` esetén `(dry run, nothing written)` áll a `-> <destination>` helyén) |
| 2 | `<base>: SubRip, UTF-8, no BOM, LF line endings kept` — az alap formátuma, kódolása, jelölője és sorvégei, az eredetiek szerint visszaírva |
| 3 | `<base>: N bytes read, M written` (az összes fájl együtt), majd **minden olvasási diagnosztika** — lásd: [Indítás](invocation.md#olvasási-diagnosztika) |

### JSON-ban

A [`--format json`](invocation.md#szkript-által-olvasható-kimenet) esetén a teljes futásról **egyetlen objektum** készül,
amelynek a `file` mezője **az alap**. A `counts` mezői:

| Számláló | Mit mond |
| :----- | :----------- |
| `files` | a futás fájljai, az alapot is beleértve |
| `subtitles` | az eredmény feliratsorai |
| `appended` | a hozzáfűzött fájlokból származók |
| `lost_ends`, `lost_header` | `0` vagy `1`: a hozzáfűzött fájlok egyike elvesztette |
| `joined_lines`, `lost_tags`, `lost_fields` | a hozzáfűzött fájlok által elvesztettek összege |
| `furthest_ms` | egy pozíció legnagyobb elmozdulása, az összes fájlt együtt nézve |

**Egy `inputs` tömb** tartozik a `counts` mellé, azon kívül: hozzáfűzött fájlonként egy elem, sorrendben,
`{"file", "counts"}` — a `counts` az `appended` mezőt tartalmazza, majd ugyanazokat a veszteségeket, **csak arra a fájlra nézve**. Ez
mondja meg a tíz fájlból álló futásban, hogy melyik mibe került. Az összes fájl olvasási figyelmeztetései
a `warnings` mezőben gyűlnek össze.

A hiba is objektum, **egyetlen**: azé az első fájlé, amely nem volt olvasható.

## Kilépési kódok

| Kód | Mikor |
| :--- | :---- |
| `0` | az eredmény ki van írva — vagy ki van számítva, a `--dry-run` esetén |
| `1` | használati hiba: kettőnél kevesebb fájl, hiányzó `--output`, az `append` alparancsnak nem létező kapcsoló, olyan cél, amely az egyik bemenet |
| `2` | **egy fájl nem volt olvasható**, vagy a célt nem lehetett kiírni: semmi sem íródik ki. Nincs `3` kód: az a futás, amely csak egy fájlt ír, vagy sikerül, vagy nem |

## Hibák

| Mi váltja ki | Üzenet |
| :------------------- | :------ |
| egyetlen megadott fájl | `append needs a base file and at least one file to put after it` |
| sem `--output`, sem `--dry-run` | `no destination given: append writes one file, use --output` |
| az `--output` egy bemenetet nevez meg | `<destination>: would be written over the input <input>` |
| hiányzó, olvashatatlan vagy ismeretlen formátumú fájl | `<path>: does not exist`, `<path>: is in no format this tool knows`… — ugyanaz, mint mindenhol |

**Az üres fájlt nem fűzi hozzá a program**: egyetlen formátum sem olvasható legalább egy feliratsor nélkül, és a fájlt
úgy utasítja el, mint bármely olvashatatlan fájlt — `<path>: holds nothing recognisable as a subtitle` —, ami
leállítja a futást, és semmit sem ír ki.
