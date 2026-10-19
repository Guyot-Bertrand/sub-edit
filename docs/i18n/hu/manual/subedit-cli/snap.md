# `snap`

<!-- machine-translated -->
> Ez az oldal gépi fordítás, amelyet a nyelv egyik olvasója még nem nézett át.

```
subedit-cli snap --rate <rate>
                 (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 <file>...
```

**Minden időbélyeget** — kezdetet és véget — a megadott képkockasebesség
legközelebbi képkockájára igazít. Semmi sincs újraidőzítve: minden pozíció legfeljebb
fél képkockányit mozdul.

## A `snap` nem a `framerate`, és az összetévesztésük nem látszik

A két parancs alakja ugyanaz — fájlok, egy cél, egy képkockasebesség —, és ellentétes dolgot tesznek.

| | [`framerate`](framerate.md) | `snap` |
| :--- | :-------------------------- | :----- |
| mit számol | `t' = t x input rate / output rate` | a legközelebbi képkocka, a `--rate` rácsán |
| mennyivel mozdít | az idővel arányosan — egy egész estés filmen **több másodperccel** | legfeljebb fél képkockával, **mindenütt** |
| melyik fájlhoz | olyan fájlhoz, amelynek az **időzítése téves** | olyan fájlhoz, amelynek az időzítése **helyes**, a rácsa viszont téves |

**A `snap` felhasználási esete**, kifejtve: egy film 25 képkocka/másodperc
sebességgel, egy 24-es rácson írt feliratfájl, a sorok már
nagyjából a helyükön vannak, néhány ezredmásodperces pontossággal. Nincs mit újraidőzíteni
— csak az időbélyegeket kell a megfelelő képkockákra visszatenni.

**A `framerate` felhasználási esete** a másik: egy 25-ös masterre időzített fájl,
amelyet egy 23,976-os másolaton akarnak lejátszani, és amelynek az időzítése a film végére
több másodpercet csúszik.

Az egyiket a másik helyett használni **nem okoz hibát**: mindkettő
elfogadja a fájlt, és eredményt ír. A tájékoztató sor különbözteti meg
őket — a `snap` mindig megmondja, mennyivel mozdította el a pozíciókat.

<!-- example: subedit-cli snap --help -->
```console
$ subedit-cli snap --help
Move every position onto the nearest frame of a frame rate (see framerate)
Usage: subedit-cli snap [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to align

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --rate TEXT REQUIRED        Frame rate to align on: 25, 23.976
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Argumentumok és kapcsolók

| Kapcsoló | Kötelező | Értékek | Alapértelmezés |
| :----- | :----- | :------ | :----- |
| `<file>...` | igen | egy vagy több útvonal | — |
| `--recursive`, `-r` | nem | jelzőkapcsoló | ki — lásd [Könyvtárfa feldolgozása](lots.md) |
| `--rate` | **igen** | képkockasebesség képkocka/másodpercben, a [`framerate`](framerate.md#a-képkockasebesség-megadása) szerint megadva | — |
| `--output` / `--output-dir` / `--in-place` | **a három közül egy**, kivéve a `--dry-run` esetén | lásd [Indítás](invocation.md#a-cél) | — |
| `--dry-run` | nem | jelzőkapcsoló: kiszámítja és közli, de nem ír semmit — lásd [Megnézés írás előtt](invocation.md#megnézés-írás-előtt) | ki |

A `--rate` **bármely érvényes képkockasebességet** elfogadja, nem csak a nyolc
szabványosat: a felhasználó választása, mint a `framerate` esetében. A nulla vagy negatív
képkockasebességet elutasítja.

A beolvasott fájl formátuma **megmarad**: a formátum megváltoztatása a
[`convert`](convert.md) dolga.

## Kimenet

**Szabványos kimenet** — semmi: az eredmény a megírt fájl. A `--dry-run` esetén szintén semmi: nincs fájl, és ezt a tájékoztató sor mondja ki — lásd [Megnézés írás előtt](invocation.md#megnézés-írás-előtt).

| Szint | Mi kerül még a hibakimenetre |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles aligned on 25 fps, M positions moved, by at most 20 ms -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — a beolvasott fájl formátuma, kódolása, jelölője és sorvége, úgy visszaírva, ahogyan volt |
| 3 | `<path>: N bytes read, M written`, majd **minden olvasási diagnosztika** — lásd [Indítás](invocation.md#olvasási-diagnosztika) |

**A sor lényege a két szám.** Fél képkocka 25 képkocka/másodperc
mellett húsz ezredmásodperc, és ennél nagyobb ott nem írható. Aki
`framerate` parancsot akart, és `snap` parancsot gépelt, a „by at most 20 ms” szöveget olvassa ott, ahol másodperceket várt,
és rögtön tudja.

## Egy példa

Egy `00:00:01,010` időpontú sor, 25 képkocka/másodpercre igazítva — tehát
negyven ezredmásodperc többszörösére:

<!-- example: printf '1\n00:00:01,010 --> 00:00:03,020\nUn.\n\n' > a.srt; subedit-cli snap --rate 25 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,010 --> 00:00:03,020\nUn.\n\n' > a.srt; subedit-cli snap --rate 25 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle aligned on 25 fps, 2 positions moved, by at most 20 ms -> b.srt
1
00:00:01,000 --> 00:00:03,040
Un.
```

## Két tulajdonság

**A kétszeri igazítás másodszor már nem változtat semmit.** A már képkockán álló pozíció
ott marad: ugyanaz a művelet ugyanarra az eredményre alkalmazva.

**A feliratsorok sorrendje megmarad.** A legközelebbi képkockára kerekítés
monoton, így két kezdet, amely legalább egy képkockányira van egymástól, az is marad. Ennél
közelebb viszont összeolvadhatnak — ez olyan anomália, amelyet az
[`inspect`](inspect.md#amit-az-anomalies-jelez) meg tud nevezni, és nem
megfordulás.

### JSON-ban

A [`--format json`](invocation.md#szkript-által-olvasható-kimenet) esetén minden fájl objektuma tartalmazza a `counts.subtitles`, a `counts.moved` (az elmozdított pozíciók) és a `counts.furthest_ms` (a legnagyobb mozgás ezredmásodpercben) mezőt. A teljes táblázat az [Indítás](invocation.md#amit-az-egyes-alparancsok-a-counts-mezőbe-írnak) oldalán található.

## Kilépési kódok

Az eszköz általános kódjai: `0`, ha minden fájl ki lett írva, `2`, ha egy
sem, `3`, ha csak néhány, `1` használati hiba esetén.

## Hibák

A képkockasebesség hibái, szóról szóra a [`framerate`](framerate.md#hibák) hibái
— mindkettő ugyanazt az értéket olvassa ugyanazon az úton:

| Mi váltja ki | Üzenet |
| :------------------ | :------ |
| nulla vagy negatív képkockasebesség | `"…" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed` |
| tizedesvessző | `"…" is not a frame rate: use a decimal point and not a comma, in every language` |
| nem számnak tekinthető szöveg | `"…" is not a frame rate: expected frames per second, like 25 or 23.976` |
| mértéktelen képkockasebesség | `"…" is not a frame rate: no video runs at that many frames per second` |

**A három célkapcsoló egyike nélkül semmi sem íródik ki**, és ezek hibái
közösek az íráshoz használt alparancsokkal:
lásd [Indítás](invocation.md#a-cél).
