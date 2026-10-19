# `shift`

<!-- machine-translated -->
> Ez az oldal gépi fordítás, amelyet a nyelv egyik olvasója még nem nézett át.

```
subedit-cli shift (--by <time> | --to-grid)
                  (--output FILE | --output-dir DIR | --in-place)
                  [--dry-run]
                  [--recursive]
                  <file>...
```

A fájl **minden** pozícióját ugyanannyival tolja el, akár egyik, akár
másik irányba. Egyetlen feliratsor sem marad hosszabban vagy rövidebben a képernyőn:
ettől eltolás az eltolás.

<!-- example: subedit-cli shift --help -->
```console
$ subedit-cli shift --help
Move every position of a file by a fixed amount
Usage: subedit-cli shift [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to shift

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --by TEXT                   Amount to move by: 2.999, -7.001, or 00:00:07.001
  --to-grid                   Move by the amount that puts the positions back on their frame grid
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
| `--by` | **a kettő közül egy** | előjeles időtartam, lásd alább | — |
| `--to-grid` | **a kettő közül egy** | jelzőkapcsoló: az értéket megméri, lásd alább | — |
| `--output` / `--output-dir` / `--in-place` | **a három közül egy**, kivéve a `--dry-run` esetén | lásd [Indítás](invocation.md#a-cél) | — |
| `--dry-run` | nem | jelzőkapcsoló: kiszámítja és közli, de nem ír semmit — lásd [Megnézés írás előtt](invocation.md#megnézés-írás-előtt) | ki |

A beolvasott fájl formátuma **megmarad**: a formátum megváltoztatása a
[`convert`](convert.md) dolga.

## `--to-grid`: megmért érték megadott helyett

Az a fájl, amelynek a pozíciói egy képkockarácson állnak **egy állandó
eltéréssel**, el lett tolva, és ez az állandó a pozíciókból magukból megmérhető.
A `--to-grid` megméri és alkalmazza, anélkül, hogy bárkinek ismernie kellene.

Az érték **fájlonként más**: két fájl, amelyet ugyanarról a rácsról
két különböző mennyiséggel toltak el, mind a sajátjával tér vissza. A
tájékoztató sor közli, a megtalált képkockasebességgel együtt.

**Az út a legrövidebb.** Fél képkockán túl a legközelebbi rácsvonal
a következő: a korrekció előre megy, ahelyett hogy majdnem egy egész
képkockányit hátrálna.

<!-- example: k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40 + 7)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > decale.srt; subedit-cli shift --to-grid --output recale.srt decale.srt -->
```console
$ k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40 + 7)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > decale.srt; subedit-cli shift --to-grid --output recale.srt decale.srt
decale.srt: 12 subtitles shifted by -0.007 s onto their 25 fps grid -> recale.srt
```

**Az ismert rácson kívüli fájlt elutasítja, és megmondja az okát.** Nincs
mihez visszatérni, és a zajon mért fázis véletlenszerűen tolná
el a fájlt. Lásd az [`inspect`](inspect.md#amit-a-frame-rate-grid-jelez)
leírását, hogy a művelet kipróbálása előtt kiderüljön, mit ad egy fájl.

A `--by` és a `--to-grid` egyaránt azt mondja meg, mennyivel kell eltolni: a kettő együtt
használati hiba, és az is, ha egyik sincs megadva.

**A `--to-grid` nem azonos a [`snap`](snap.md) paranccsal.** Az egész fájlt
ugyanannyival tolja el, és a relatív időzítést **pontosan** megtartja; a `snap`
minden pozíciót külön mozgat, és menet közben elnyeli azokat, amelyeket
kézzel korrigáltak. Tiszta fájlon a kettő ugyanazt az eredményt adja;
részleges fájlon nem.

## Az időtartam megadása

Két alak van, és csak kettő:

| Alak | Példák |
| :---- | :------- |
| másodperc | `30`, `2.999`, `-7.001`, `+1.5` |
| időbélyeg | `00:00:07.001`, `01:39:37.040`, `-00:00:07.001` |

Mindkettő ugyanazt jelenti: a `--by 7.001` és a `--by 00:00:07.001` ugyanazt
a fájlt állítja elő.

**A tizedeselválasztó a pont**, mert a parancssor angolul beszél.
A vesszőt elutasítja, nem értelmezi: a világ fele a `7,001` alakot
hétezer-egynek írja, és hét másodpercként olvasni találgatás volna —
ennek a felének a számára téves.

**A felbontás az ezredmásodperc.** A finomabbat elutasítja, nem kerekíti
csendben: a hallgatólagos kerekítés olyan eredményt adna, amely semmiben sem felel meg annak,
amit kértek.

<!-- example: printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle shifted by -7.001 s -> b.srt
1
00:00:02,999 --> 00:00:04,999
Bonjour.
```

## A nullpont előtti eltolást elutasítja

A nulla alá menő eltolás `2` kódot ad, és **semmit sem ír ki**. Az üzenet megnevezi
azt a feliratsort, amely nem bírta az eltolást, ez pedig egyúttal azt is megmondja,
meddig lehet elmenni.

Az ok nem elv: a nullpont előtti pozíció így íródik:
`-00:00:01,000`, olyan alak, amelyet a kilenc formátum egyike sem definiál, és egyetlen olvasó sem
fogad el. A mag megengedi az ilyen pozíciókat — megtiltásuk a szerkesztési
műveletet különleges esetté tenné —, de a fájl más, mint a
memóriabeli modell.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; echo "code=$?"
a.srt: subtitle 1 would start before the origin, which no subtitle file can hold
code=2
```

## Kimenet

**Szabványos kimenet** — semmi: az eredmény a megírt fájl. A `--dry-run` esetén szintén semmi: nincs fájl, és ezt a tájékoztató sor mondja ki — lásd [Megnézés írás előtt](invocation.md#megnézés-írás-előtt).

| Szint | Mi kerül még a hibakimenetre |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles shifted by 2.999 s -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — a beolvasott fájl formátuma, kódolása, jelölője és sorvége, úgy visszaírva, ahogyan volt |
| 3 | `<path>: N bytes read, M written`, majd **minden olvasási diagnosztika** — lásd [Indítás](invocation.md#olvasási-diagnosztika) |

### JSON-ban

A [`--format json`](invocation.md#szkript-által-olvasható-kimenet) esetén minden fájl objektuma tartalmazza a `counts.subtitles` (a fájl összes feliratsora) és a `counts.shifted_by_ms` (az eltolás egész ezredmásodpercben, **előjellel**) mezőt. A teljes táblázat az [Indítás](invocation.md#amit-az-egyes-alparancsok-a-counts-mezőbe-írnak) oldalán található.

## Kilépési kódok

Az eszköz általános kódjai: `0`, ha minden fájl ki lett írva, `2`, ha egy
sem, `3`, ha csak néhány, `1` használati hiba esetén.

## Hibák

| Mi váltja ki | Üzenet |
| :------------------ | :------ |
| tizedesvessző | `"…" is not a time: use a decimal point and not a comma, in every language` |
| az ezredmásodpercnél finomabb érték | `"…" is not a time: positions are held to the millisecond, no finer` |
| nem időtartam szöveg | `"…" is not a time: expected seconds like 2.999, or a timestamp like 00:00:07.001` |
| a nullpont előtti eltolás | `<path>: subtitle N would start before the origin, which no subtitle file can hold` |

**A három célkapcsoló egyike nélkül semmi sem íródik ki**, és ezek
hibái közösek az íráshoz használt alparancsokkal:
lásd [Indítás](invocation.md#a-cél).
