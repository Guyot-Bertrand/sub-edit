# `split-file`

<!-- machine-translated -->
> Ez az oldal gépi fordítás, amelyet a nyelv egyik olvasója még nem nézett át.

```
subedit-cli split-file --at N --head FILE --tail FILE
                       [--dry-run] [--sort] [--disjoint]
                       <file>
```

Egy fájlt **kettévág**: egy elejére és egy végére. Ez az ablak `Split Project…` művelete — [ugyanaz a motor](../subedit-gui/operations.md#split-project),
ugyanazok a számok, ugyanaz az elutasítás — és **az [`append`](append.md) pontos fordítottja**: a vég hozzáfűzése az
elejéhez visszaadja a kiindulási fájlt.

- **Az `--at N` a vég első feliratsorának száma**, úgy számozva, ahogyan az ablak táblázata
  számozza: 1-től. Az `--at 3` az 1. és a 2. feliratsort az elejébe, a harmadikat és a többit a
  végébe teszi. **A másodiktól az utolsóig** terjed: a vágáshoz mindkét oldalon kell egy feliratsor.
- **A véget visszahozza a nullpontra**: a fejben maradt utolsó feliratsor végének mínusz értékével tolja el,
  a pozíciók sorrendje és távolsága megmarad. Ettől lesz a `split-file` az `append` fordítottja.
- **Mindkét fél megtartja a beolvasott fájl formátumát, kódolását, bájtsorrend-jelölőjét és sorvégeit**, és
  íráskor 1-től újra van számozva, mint minden visszaírt fájl.
- **Két kimenet, tehát saját cél**: a `--head` és a `--tail`, **mindkettő kötelező**. Az `--output`,
  az `--output-dir`, az `--in-place` és a `--recursive` nem létezik a `split-file` esetén, és úgy utasítja el, mint bármely ismeretlen
  kapcsolót, `1` kóddal: nincs könyvtár, amelyben két megnevezett fájl osztozhatna, és nincs felülírandó bemenet sem.

<!-- example: subedit-cli split-file --help -->
```console
$ subedit-cli split-file --help
Cut a subtitle file in two, a head and a tail
Usage: subedit-cli split-file [OPTIONS] file

Positionals:
  file TEXT REQUIRED          Subtitle file to cut

Options:
  -h,--help                   Print this help message and exit
  --at N                      Number of the first subtitle of the tail, as the table numbers them
  --head FILE                 File to write the subtitles before the cut to
  --tail FILE                 File to write the subtitles from the cut on to
  --dry-run                   Work out and say what would be written, and write nothing
  --sort                      Put the subtitles in order of their start before cutting; --at counts in that order
  --disjoint                  Cut overlapping subtitles so that no range overlaps another, before cutting
```

## Argumentumok és kapcsolók

| Kapcsoló | Kötelező | Értékek | Alapértelmezés |
| :----- | :----- | :------ | :----- |
| `<file>` | igen | **egyetlen** útvonal | — |
| `--at` | igen | egész szám, 2-től a fájl feliratsorainak számáig | — |
| `--head`, `--tail` | **igen, mindkettő**, kivéve a `--dry-run` esetén | fájlútvonalak; a könyvtáruk létrejön, ha hiányzik | — |
| `--dry-run` | nem | jelzőkapcsoló: kiszámítja és közli, de nem ír semmit — lásd [Megnézés írás előtt](invocation.md#megnézés-írás-előtt) | ki |
| `--sort` | nem | jelzőkapcsoló: a feliratsorokat a kezdetük sorrendjébe teszi **a vágás előtt**; az `--at` ebben a sorrendben számol | ki |
| `--disjoint` | nem | jelzőkapcsoló: az átfedő feliratsorokat levágja **a vágás előtt**; az `--at` a megmaradtakban számol, lásd [A tartományok átfedésmentessé tétele](#a-tartományok-átfedésmentessé-tétele) | ki |

**A `--dry-run` esetén nem kötelező cél**: egy sem adható meg, de nem csak az egyik a kettő közül —
a `--head` és a `--tail` együtt jár. A megadottakat úgy ítéli meg, mint a `--dry-run` nélkül.

**A két cél nem lehet sem a bemenet, sem ugyanaz a fájl**: az olvasott fájl
felülírását, vagy a vég fejre írását bármi beolvasása előtt elutasítja, `1` kóddal.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n3\n00:00:06,000 --> 00:00:07,000\ntrois\n\n4\n00:00:08,000 --> 00:00:09,000\nquatre\n\n' > film.srt; subedit-cli split-file --at 3 --head tete.srt --tail queue.srt film.srt; cat queue.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n3\n00:00:06,000 --> 00:00:07,000\ntrois\n\n4\n00:00:08,000 --> 00:00:09,000\nquatre\n\n' > film.srt; subedit-cli split-file --at 3 --head tete.srt --tail queue.srt film.srt; cat queue.srt
film.srt: split at subtitle 3: 2 subtitles in the head, 2 subtitles in the tail -> tete.srt, queue.srt
1
00:00:02,000 --> 00:00:03,000
trois

2
00:00:04,000 --> 00:00:05,000
quatre
```

A második feliratsor, amely a fejben maradt, 4 s-nál ér véget: a vég 4 s-mal visszamozdul, és a harmadik feliratsor,
amely 6 s-nál kezdődött, 2 s-nál kezdődik.

## Vágás időrendben

Az `--at` a feliratsorokat **úgy számozza, ahogyan a fájl adja őket**. Egy olyan fájlnál, amelynek a sorrendje megbomlott, a fej ezért nem az időben korábbi
fele. A `--sort` a feliratsorokat a kezdetük sorrendjébe teszi **a vágás megítélése előtt** — stabilan, ahogyan a [`sort`](sort.md) —, és
az `--at` ezután ebben a sorrendben számol.

Alapértelmezés szerint soha. A kapcsolóval a fájl az első tájékoztatási szinten közli, hány feliratsor mozdult el (`<path>: N subtitles moved`
vagy `<path>: already in order`), és a `--format json` rekord `moved` számot hordoz, **a kapcsoló nélkül hiányzik**.

## A tartományok átfedésmentessé tétele

Alapértelmezés szerint soha. A kapcsoló azt teszi, amit a [`disjoint`](disjoint.md), **az alapértelmezett beállításaival**, a vágás megítélése előtt, a rendezés után: az `--at` az így kapottban számol. Rendezett fájl kell hozzá: különben elutasítja, és a `--sort` — amely előbb jön — előtte rendezi. A fájl az első tájékoztatási szinten közli ezt (`<path>: N subtitles split, M merged`), és a `--format json` rekord `split` és `merged` számot hordoz, **a kapcsoló nélkül hiányzik**.

## Amikor a vágást elutasítja

**A vágásnál átfedő két felet nem vágja el.** Ha a vég egy feliratsora azelőtt kezdődött,
hogy a hátrahagyott utolsó feliratsor véget ért volna, az eltolás a videó eleje elé vinné — negatív pozícióba,
amelyet egyetlen formátum sem ír. A `split-file` elutasítja, **megnevezve a feliratsort**, az ablak szavaival:

```text
o.srt: Cannot split at subtitle 3: subtitle 3 would fall before the start of the video. Cut somewhere else.
```

A kód `2`, és **semmi sem íródik ki**. Máshol kell vágni. A pontosan a kezdetre érkezés megengedett: a nulla
pozíció.

**A fájlon kívüli `--at` értéket ugyanígy utasítja el**, a fájl beolvasása után: az `--at 1` — az első feliratsor nem
kezdhet véget, nem maradna semmi — és az utolsó feliratsoron túli `--at`, amely üres véget
hagyna. Az egyetlen feliratsorból álló fájl nem vágható.

## Kimenet

**Szabványos kimenet** — semmi: az eredmény a két megírt fájl.

| Szint | Mi kerül még a hibakimenetre |
| :----- | :------------------------------------- |
| 1 | `<path>: split at subtitle N: H subtitles in the head, T subtitles in the tail -> <head>, <tail>`; a `--dry-run` esetén `(dry run, nothing written)` áll a `-> …` helyén |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — a beolvasott fájl formátuma, kódolása, jelölője és sorvége, úgy visszaírva, ahogyan volt |
| 3 | `<path>: N bytes read, H and T written`, majd **minden olvasási diagnosztika** — lásd [Indítás](invocation.md#olvasási-diagnosztika) |

### JSON-ban

A [`--format json`](invocation.md#szkript-által-olvasható-kimenet) esetén **egyetlen objektum**, amelynek a `file` mezője a vágott fájl.
**A `destination` a fej** — a fájl, amivé a bemenet válik — és **a `tail` a vég**, útvonal, vagy `null` a
`--dry-run` esetén, mint a `destination`. A `counts`: `subtitles` (a beolvasott fájlé), `head` és `tail` (az egyes
felekéi).

## Kilépési kódok

| Kód | Mikor |
| :--- | :---- |
| `0` | mindkét fájl ki van írva — vagy `--dry-run` esetén kiszámítva |
| `1` | használati hiba: nincs `--head` és `--tail`, a `split-file` számára ismeretlen kapcsoló, a bemenettel vagy a másik céllal azonos cél, nem pozitív egész szám `--at` |
| `2` | a fájlt nem sikerült beolvasni, **a vágást elutasították** (a fájlon kívüli érték vagy átfedés), vagy egy célt nem sikerült megírni. `3` kód nincs: egyetlen fájlt dolgoz fel |

**A fejet írja ki először**: ha a lemez a véget visszautasítja, a fej megmarad mögötte, és a kód `2`.

## Hibák

| Mi váltja ki | Üzenet |
| :------------------- | :------ |
| sem `--head`, sem `--tail` | `no destination given: split-file writes two files, use --head and --tail` |
| csak az egyik a kettő közül, `--dry-run` esetén | `--head and --tail go together: give both, or neither with --dry-run` |
| a két cél ugyanaz a fájl | `<path>: named as both the head and the tail` |
| egy cél a bemenet | `<path>: would be written over the input <input>` |
| a fájlon kívüli `--at` | `<file>: --at 9: the file holds 3 subtitles, and the tail can start from 2 to 3` |
| egyetlen feliratsorból álló fájl | `<file>: a file of 1 subtitle cannot be split: a cut needs a subtitle on each side` |
| átfedés | `<file>: Cannot split at subtitle N: subtitle M would fall before the start of the video. Cut somewhere else.` |
| hiányzó, olvashatatlan vagy ismeretlen formátumú fájl | `<path>: does not exist`… — ugyanaz, mint mindenütt |
