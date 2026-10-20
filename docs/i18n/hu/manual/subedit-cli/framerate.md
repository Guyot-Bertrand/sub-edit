# `framerate`

<!-- machine-translated -->
> Ez az oldal gépi fordítás, amelyet a nyelv egyik olvasója még nem nézett át.

> **Ez nem a [`snap`](snap.md).** Ez a parancs **átskálázza a
> fájlt**: akkor való, ha az időzítés hibás, és a film végére több
> másodpercet csúszik. A `snap` az időbélyegeket a legközelebbi
> rácsra teszi vissza, anélkül hogy bármit újraidőzítene, és akkor való, ha az időzítés már
> helyes. Mindkettő alakja ugyanaz, egy vagy két képkockasebesség, és az összekeverésük
> nem okoz hibát.

```
subedit-cli framerate --from <fps> --to <fps>
                      (--output FILE | --output-dir DIR | --in-place)
                      [--dry-run]
                      [--recursive]
                      <file>...
```

Egy adott képkockasebességre időzített fájlt újraidőzít egy másikra. Ez az a művelet, amelyet a feliratfájl akkor kíván, ha olyan kópiához szerezték be, amely nem azonos a meglévő filmével — például egy PAL DVD-hez képest egy NTSC-átvitel esetén.

Az olvasott fájl formátuma **megmarad**: a formátum módosítása a [`convert`](convert.md) dolga.

<!-- example: subedit-cli framerate --help -->
```console
$ subedit-cli framerate --help
Re-time a file mastered at one frame rate for another
Usage: subedit-cli framerate [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to re-time

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --from TEXT REQUIRED        Frame rate the file is timed at: 25, 23.976
  --to TEXT REQUIRED          Frame rate to time it for: 24, 29.97
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Argumentumok és kapcsolók

| Kapcsoló | Kötelező | Értékek | Alapértelmezés |
| :----- | :----- | :------ | :----- |
| `<file>...` | igen | egy vagy több elérési út | — |
| `--recursive`, `-r` | nem | kapcsoló érték nélkül | ki — lásd [Könyvtárfa feldolgozása](lots.md) |
| `--from` | **igen** | képkockasebesség, lásd alább | — |
| `--to` | **igen** | képkockasebesség, lásd alább | — |
| `--output` / `--output-dir` / `--in-place` | **a három közül egy**, a `--dry-run` kapcsoló kivételével | lásd [Indítás](invocation.md#a-cél) | — |
| `--dry-run` | nem | kapcsoló érték nélkül: kiszámolja és közli, nem ír semmit — lásd [Megnézés írás előtt](invocation.md#megnézés-írás-előtt) | ki |

A `--from` az a képkockasebesség, amelyre a fájl **van** időzítve, a `--to` az, amelyre
szánják. Ha felcserélik őket, a fájl az ellenkező irányba csúszik el — egy egész estés filmen nagyjából hét másodpercet 25 és
23,976 között —, és semmi sem jelzi, mielőtt a filmet megnéznék.

## A képkockasebesség megadása

Képkocka/másodpercben, tizedesekkel, tizedesponttal elválasztva:

```
25      24      23.976      29.97      60
```

**A nulla vagy negatív képkockasebesség elutasításra kerül**: az nem képkockasebesség.

### A három NTSC-képkockasebesség nem írható le pontosan

A `23.976`, a `29.97` és az `59.94` **címkék**. Az általuk jelölt képkockasebességek a `24000/1001`, a `30000/1001` és a `60000/1001`, amelyek egyikének sincs véges tizedes alakja. Az eszköz ezeket annak ismeri fel:

| Mit írnak | Mit jelöl |
| :------------- | :------------------ |
| `23.976` | `24000/1001` — NTSC-videóra átvitt mozifilm |
| `24` | `24` — mozifilm |
| `25` | `25` — PAL- és SECAM-videó |
| `29.97` | `30000/1001` — NTSC-videó |
| `30`, `50`, `60` | önmaguk |
| `59.94` | `60000/1001` |

A felismerés **az értékre vonatkozik, nem a betűkre**: a `23.9760`
ugyanazt a képkockasebességet jelöli, mint a `23.976`.

Bármely más tizedes szám szó szerint értendő — a `23.9` senkinek sem szabványa, nincs mit kiolvasni belőle. Ettől szükséges a fenti táblázat: nélküle a `23.976` `23976/1000` lenne, és a parancssor mást mondana, mint az ablak ugyanazokra a szavakra.

## Mivé válnak a pozíciók

```
t′ = round( t × (from / to) )
```

Az a film, amelyet 23,976 képkocka/másodpercre masterelték, és 25 képkocka/másodperccel játszanak le, gyorsabban fut, így a feliratai korábban érkeznek; az ellenkező irányban később érkeznek.

**A tényező pontos racionális szám, és a kerekítés csak egyszer történik.** Az egyenértékűnek látszó út — a pozíciót a bemeneti sebességnél képkockaszámmá, majd a kimeneti sebességnél vissza ezredmásodperccé alakítani — nem az: minden pozíciót a bemeneti rácsra kvantál, és legfeljebb fél képkockával téved, mérten 21 ms-mal. Lásd [ADR 0013](../../../../adr/0013-mise-a-l-echelle-exacte-des-positions.md).

A gyakorlatban 25-ről 23,976-ra a tényező `1001/960`:

| Olvasott pozíció | Írt pozíció | Mit adna a képkockákon át számolás |
| -----------: | -----: | -----------------------------------------: |
| `1 010 ms` | `1 053 ms` | `1 043 ms` |
| `3 600 017 ms` | `3 753 768 ms` | `3 753 750 ms` |

<!-- example: printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 25 --to 23.976 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 25 --to 23.976 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle retimed from 25 to 24000/1001 fps -> b.srt
1
00:00:01,053 --> 00:00:02,106
Un.
```

## Kimenet

**Szabványos kimenet** — semmi: az eredmény a megírt fájl. A `--dry-run` kapcsolóval sem semmi: nincs fájl, és a beszámolósor ezt közli — lásd [Megnézés írás előtt](invocation.md#megnézés-írás-előtt).

| Szint | Mi kerül még a hibakimenetre |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles retimed from 25 to 24000/1001 fps -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — az olvasott fájl formátuma, kódolása, jelölője és sorvége, úgy visszaállítva, ahogy volt |
| 3 | `<path>: N bytes read, M written`, majd **minden olvasási diagnosztika** — lásd [Indítás](invocation.md#olvasási-diagnosztika) |

**A beszámoló a pontos képkockasebességet nevezi meg, nem a beírt címkét.** Ha ebben a sorban `23.976` állna, olyan átváltásról számolna be, amely nem történt meg; ez az egyetlen hely, ahol látszik, hogy a kettő közül melyiket használták. Az a képkockasebesség, amelyet egy tizedes szám pontosan leír, ott tizedes alakban jelenik meg, a többi törtként.

### JSON-ban

A [`--format json`](invocation.md#szkript-által-olvasható-kimenet) kapcsolóval az egyes fájlok objektuma tartalmazza a `counts.subtitles` mezőt (a fájl minden feliratsora). A teljes táblázat az [Indítás](invocation.md#amit-az-egyes-alparancsok-a-counts-mezőbe-írnak) oldalán található.

## Kilépési kódok

Az eszköz kódjai: `0`, ha minden fájl megírása megtörtént, `2`, ha egyé sem, `3`, ha csak egyeseké, `1` használati hiba esetén.

Egyetlen elutasítás sem függ a fájl tartalmától: két képkockasebesség mindig meghatároz egy tényezőt. Ez az egyetlen olyan író alparancs, amelynek minden elutasítása az első olvasás előtt történik.

<!-- example: printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 0 --to 25 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 0 --to 25 --output b.srt a.srt; echo "code=$?"
"0" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed
code=1
```

## Hibák

| Mi váltja ki | Üzenet |
| :------------------ | :------ |
| nulla vagy negatív képkockasebesség | `"…" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed` |
| tizedesvessző | `"…" is not a frame rate: use a decimal point and not a comma, in every language` |
| nem számot tartalmazó szöveg | `"…" is not a frame rate: expected frames per second, like 25 or 23.976` |
| mértéktelen képkockasebesség | `"…" is not a frame rate: no video runs at that many frames per second` |

**A három célkapcsoló egyike nélkül semmi sem íródik meg**, és ezek hibái közösek az író alparancsokban: lásd [Indítás](invocation.md#a-cél).
