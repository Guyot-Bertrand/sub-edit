# Könyvtárfa feldolgozása

<!-- machine-translated -->
> Ez az oldal gépi fordítás, amelyet a nyelv egyik olvasója még nem nézett át.

```
subedit-cli <subcommand> --recursive [options] <directory>...
```

Minden alparancs, amely fájlköteget olvas — az `inspect` is, de nem az
[`append`](append.md), a [`split-file`](split-file.md) és a [`pair`](pair.md), amelyeknek az aritása egy páré vagy
egy filmé —, **könyvtárat** is elfogad fájl helyett, a `--recursive` (`-r`) kapcsolóval.
Ez az oldal azt mondja el, mit vesz fel a bejárás, milyen sorrendben, és hová ír.

**Az alábbiak közül semmi sem változtat azon, amit egy fájllal tesznek**: mindegyiket külön olvassa be,
dolgozza fel és írja ki, az egyik hibája nem állítja meg a többit, és a
négy kilépési kód az [indításnál](invocation.md#kilépési-kódok) leírt.
A bejárás útvonallistát állít elő, amelyet a köteg többi része úgy fogyaszt, mintha
begépelték volna.

## A `--recursive` nélkül a könyvtárat elutasítja

Használati hiba, `1` kód, egyetlen fájl sincs feldolgozva vagy kiírva:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?" -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?"
films: is a directory: use --recursive
code=1
```

## Amit a bejárás felvesz

A bejárás **rekurzív, determinisztikus és meglepetésmentes**:

- **a sorrend** — az egyes könyvtárak elemei a nevük sorrendjében következnek,
  **bájtról bájtra** (a nagybetűk a kisbetűk előtt, a gép nyelvétől
  függetlenül), az alkönyvtár pedig a helyén kerül bejárásra, mélységi
  sorrendben. Két futtatás ugyanazokat a bájtokat adja minden
  gépen;
- **a szimbolikus hivatkozásokat nem követi** — sem fájlra, sem
  könyvtárra mutatót: az a fa, amely a saját egyik szülőjére mutató hivatkozást tartalmaz,
  soha nem érne véget;
- **a rejtett fájlokat és könyvtárakat figyelmen kívül hagyja** — azokat, amelyeknek a neve
  ponttal kezdődik;
- **csak az ismert formátumok kiterjesztései maradnak**: `.srt`, `.vtt`,
  `.ssa`, `.ass`, `.lrc`, `.sub`, kis- és nagybetűtől függetlenül. **A `.txt` ki van zárva**: két
  formátum neve, de mindenekelőtt a világ minden `LISEZMOI.txt` fájljáé, és egy olyan köteg, amely
  mindegyiken hibázna, használhatatlan volna.

**A parancssorban megnevezett fájlt sosem szűri.** Megnevezni azt jelenti,
hogy kell: a `subedit-cli shift --by 1 --in-place notes.txt` feldolgozza, bármi is
a kiterjesztése, és csak akkor panaszkodik, ha a tartalma egyetlen formátumba sem illik.

**Ami félre lett téve, azt megszámolja**, és a `-vv` szinten közli:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli -vv inspect --recursive films 2>&1 | head -1 -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli -vv inspect --recursive films 2>&1 | head -1
films: 1 file left aside, with no extension of a known format
```

Az a könyvtár, amelyben nincs mit bejárni, az alapértelmezett szinten közli ezt
(`<directory>: holds no file in a format this tool walks for`), és ez nem hiba.

## Hová ír a bejárás

**A fa megmarad az `--output-dir` alatt**: a fájl útvonala,
**a megadott könyvtárhoz viszonyítva**, megismétlődik a kimeneti könyvtár alatt. A megadott
könyvtár neve nem része ennek — ez a `rsync` záró
perjeles szemantikája, nem a `cp -r`-é: a célt kell megnevezni, és ahhoz kell hozzáfűzni
a kívánt szintet (`--output-dir sortie/films`).

<!-- example: mkdir -p films/a/b; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/x.srt; cp films/x.srt films/a/x.srt; cp films/x.srt films/a/b/y.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli shift --by 1 --recursive --output-dir sortie films; find sortie -type f | sort -->
```console
$ mkdir -p films/a/b; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/x.srt; cp films/x.srt films/a/x.srt; cp films/x.srt films/a/b/y.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli shift --by 1 --recursive --output-dir sortie films; find sortie -type f | sort
films/a/b/y.srt: 1 subtitle shifted by 1.000 s -> sortie/a/b/y.srt
films/a/x.srt: 1 subtitle shifted by 1.000 s -> sortie/a/x.srt
films/x.srt: 1 subtitle shifted by 1.000 s -> sortie/x.srt
3 of 3 files shifted
sortie/a/b/y.srt
sortie/a/x.srt
sortie/x.srt
```

A kimeneti könyvtár és a fa könyvtárai **létrejönnek**, ha hiányoznak. Az
`--in-place` esetén minden megtartott fájl a saját helyére íródik vissza. A kiterjesztés, amelyet a
`convert` megváltoztat, a fában is megváltozik: a WebVTT-be átalakított `films/a/x.srt`
`sortie/a/x.vtt` néven íródik.

**A kimeneti könyvtár, ha a bejárt fán belül van, ki van zárva a bejárásból**:
a második futtatás nem olvassa be újra azt, amit az első írt.

## Két könyvtár és az ütközések

A [cél](invocation.md#a-cél) szabálya dönt: két fájlt, amely
ugyanarra az útvonalra kerülne, **bármi kiírása előtt** elutasít, `1` kóddal, a
két bemenet és a cél megnevezésével.

<!-- example: mkdir -p un/a deux/a; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > un/a/x.srt; cp un/a/x.srt deux/a/x.srt; subedit-cli shift --by 1 -r --output-dir sortie un deux; echo "code=$?" -->
```console
$ mkdir -p un/a deux/a; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > un/a/x.srt; cp un/a/x.srt deux/a/x.srt; subedit-cli shift --by 1 -r --output-dir sortie un deux; echo "code=$?"
sortie/a/x.srt: would be written by both un/a/x.srt and deux/a/x.srt
code=1
```

## Mennyibe kerül

A köteg szekvenciális marad: az időtartama a fájljai időtartamainak összege. A
`make bench` teljesítménymérés a bejárást és ugyanazt a munkát egyetlen fájlban méri; a számok
a [teljesítménynaplóban](../../../../mesures/performances.md) találhatók.
