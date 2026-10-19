# `transform`

<!-- machine-translated -->
> Ez az oldal gépi fordítás, amelyet a nyelv egyik olvasója még nem nézett át.

```
subedit-cli transform --first <index>=<time> --last <index>=<time>
                      (--output FILE | --output-dir DIR | --in-place)
                      [--dry-run]
                      [--recursive]
                      <file>...
```

A fájl **minden** pozícióját kijavítja két olyan pont alapján, amelynek a helyes értéke
ismert. Ez az a művelet, amelyet a csúszó feliratozás igényel
— az elején helyes, a végén téves —, és amelyre a [`shift`](shift.md), amely mindent
ugyanannyival mozgat, képtelen.

A beolvasott fájl formátuma **megmarad**: a formátum megváltoztatása a
[`convert`](convert.md) dolga.

<!-- example: subedit-cli transform --help -->
```console
$ subedit-cli transform --help
Correct every position from two points known to be right
Usage: subedit-cli transform [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to transform

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --first TEXT REQUIRED       Earlier reference, as <index>=<time>: 1=00:00:01.000
  --last TEXT REQUIRED        Later reference, as <index>=<time>: 3=00:00:10.000
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
| `--first` | **igen** | referenciapont, `<index>=<time>` | — |
| `--last` | **igen** | referenciapont, `<index>=<time>` | — |
| `--output` / `--output-dir` / `--in-place` | **a három közül egy**, kivéve a `--dry-run` esetén | lásd [Indítás](invocation.md#a-cél) | — |
| `--dry-run` | nem | jelzőkapcsoló: kiszámítja és közli, de nem ír semmit — lásd [Megnézés írás előtt](invocation.md#megnézés-írás-előtt) | ki |

Rövid alakjuk nincs: ez a két kapcsoló csak teljes
alakban írható.

## Referenciapont megadása

A referenciapont megnevez egy feliratsort, és megmondja, hová kell most esnie a **kezdetének**:

```
<index>=<time>
```

**Az indexet 1-től számolja**, ahogyan megjelenik, és ahogyan egy SubRip
fájl számozza. A `0`-t elutasítja, ahogyan az utolsó feliratsoron túli
indexet is.

**Az időt úgy kell megadni, mint a [`shift`](shift.md) esetében** — másodperc vagy
időbélyeg, a tizedeselválasztó pont, a felbontás az ezredmásodperc:

| Alak | Példák |
| :---- | :------- |
| másodperc | `10`, `2.999` |
| időbélyeg | `00:00:10.000`, `01:39:37.040` |

A `--first 3=10` és a `--first 3=00:00:10.000` ugyanazt jelenti.

A `--first` és a `--last` két pontot nevez meg, nem egy intervallumot. A nevük
a várt használatot mondja — az egyik referenciapont a fájl elején, a másik a végén —, és nem
megkötést: a felcserélésük ugyanazt az eredményt adja, és **a két referenciaponton kívül
eső minden pozíció úgy követi őket, mint amelyik köztük van**.

## Mivé válnak a többi pozíciók

A két referenciapont egy léptéktényezőt állít be, és minden pozíció ezt követi:

```
t′ = y₁ + (t − x₁) × r      with r = (y₂ − y₁) / (x₂ − x₁)
```

Az `x₁`, `x₂` a két megnevezett feliratsor jelenlegi kezdete, az `y₁`, `y₂` pedig a kért
idők. A tényezőt pontos racionális számként kezeli, és **a kerekítés
csak egyszer történik**, ezredmásodpercre; ettől esik a két
referenciapont **pontosan** a kért időkre, és nem ezred pontossággal
(lásd az [ADR 0013](../../../../adr/0013-mise-a-l-echelle-exacte-des-positions.md) dokumentumot).

Minden feliratsor **arányosan** marad a képernyőn ahhoz képest, ameddig ott maradt:
a vége ugyanazt a léptéket követi, mint a kezdete. Az átalakítás nem
eltolás, és nem rendelkezik annak a tulajdonságával.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,001 --> 00:00:07,000\nDeux.\n\n3\n00:00:09,000 --> 00:00:11,000\nTrois.\n\n' > a.srt; subedit-cli transform --first 1=00:00:01.000 --last 3=00:00:10.000 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,001 --> 00:00:07,000\nDeux.\n\n3\n00:00:09,000 --> 00:00:11,000\nTrois.\n\n' > a.srt; subedit-cli transform --first 1=00:00:01.000 --last 3=00:00:10.000 --output b.srt a.srt; cat b.srt
a.srt: 3 subtitles transformed onto 1=00:00:01.000 and 3=00:00:10.000 -> b.srt
1
00:00:01,000 --> 00:00:03,250
Un.

2
00:00:05,501 --> 00:00:07,750
Deux.

3
00:00:10,000 --> 00:00:12,250
Trois.
```

Az első referenciapont nem mozdul, a harmadik 9 s-ról 10 s-ra kerül: a tényező
`9/8`, és a második feliratsor — amelyet senki sem nevezett meg —
`5,001`-ről `5,501`-re megy, vagyis `1000 + 4001 × 9/8`, egyszer kerekítve.

## Két referenciapont, amely nem az

| Helyzet | Mikor derül ki | Kód |
| :-------- | :------------- | :--- |
| a `--first` és a `--last` ugyanazt az indexet nevezi meg | bármilyen olvasás előtt | `1` |
| egy index az utolsó feliratsoron túl van | a fájl megnyitásakor | `2` vagy `3` |
| a két megnevezett feliratsor ugyanabban a pillanatban kezdődik | a fájl megnyitásakor | `2` vagy `3` |

Mindhármat ugyanazért utasítja el: az `x₂ − x₁` nulla, és két
egybeeső pont nem határoz meg egyenest. Nullával osztást eredménynek
álcázva visszaadni rosszabb volna, mint elutasítani.

Az első eset **használati hiba** — a két kapcsoló hibás, bármilyenek is
a fájlok, ezért egyetlen üzenet jár, nem fájlonként egy. A
másik kettő a megnyitott fájltól függ, ezért feldolgozási
hibának számít.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n' > a.srt; subedit-cli transform --first 2=1.000 --last 2=4.000 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n' > a.srt; subedit-cli transform --first 2=1.000 --last 2=4.000 --output b.srt a.srt; echo "code=$?"
--first and --last both name subtitle 2, and two references on one subtitle define no transform
code=1
```

A határon kívüli indexet **a határ megnevezésével** utasítja el, ez pedig egyúttal
azt is megmondja, mit lehetett volna írni:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,000 --> 00:00:07,000\nDeux.\n\n' > a.srt; subedit-cli transform --first 1=1.000 --last 9=10.000 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,000 --> 00:00:07,000\nDeux.\n\n' > a.srt; subedit-cli transform --first 1=1.000 --last 9=10.000 --output b.srt a.srt; echo "code=$?"
a.srt: subtitle 9 is past the end: the file holds 2 subtitles
code=2
```

## A nullpont előtti pozíciót elutasítja

Ahogyan a [`shift`](shift.md) esetében, és ugyanazért: a negatív pozíció
így íródik: `-00:00:01,000`, olyan alak, amelyet a kilenc formátum egyike sem definiál, és egyetlen
olvasó sem fogad el. Az eset akkor áll elő, ha egy feliratsor **az első
referenciapont előtt** van, és a lépték nulla alá tolja. Semmi sem
íródik ki, a kód `2`, és az üzenet megnevezi a hibás feliratsort.

## Kimenet

**Szabványos kimenet** — semmi: az eredmény a megírt fájl. A `--dry-run` esetén szintén semmi: nincs fájl, és ezt a tájékoztató sor mondja ki — lásd [Megnézés írás előtt](invocation.md#megnézés-írás-előtt).

| Szint | Mi kerül még a hibakimenetre |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles transformed onto 1=00:00:01.000 and 3=00:00:10.000 -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — a beolvasott fájl formátuma, kódolása, jelölője és sorvége, úgy visszaírva, ahogyan volt |
| 3 | `<path>: N bytes read, M written`, majd **minden olvasási diagnosztika** — lásd [Indítás](invocation.md#olvasási-diagnosztika) |

A referenciapontokat tizedesponttal írja vissza, bárhogyan gépelték is be őket.

### JSON-ban

A [`--format json`](invocation.md#szkript-által-olvasható-kimenet) esetén minden fájl objektuma tartalmazza a `counts.subtitles` mezőt (a fájl összes feliratsora). A teljes táblázat az [Indítás](invocation.md#amit-az-egyes-alparancsok-a-counts-mezőbe-írnak) oldalán található.

## Kilépési kódok

Az eszköz általános kódjai: `0`, ha minden fájl ki lett írva, `2`, ha egy
sem, `3`, ha csak néhány, `1` használati hiba esetén.

## Hibák

| Mi váltja ki | Üzenet |
| :------------------ | :------ |
| `=` nélküli referenciapont | `"…" is not a reference: write it <index>=<time>, as in 3=00:00:10.000` |
| a referenciapont egyik fele hiányzik | `"…" is missing one of its two halves: write it <index>=<time>, as in 3=00:00:10.000` |
| `0` index | `"0" is not a subtitle number: subtitles are counted from 1, as the file shows them` |
| nem egész szám index | `"…" is not a subtitle number: expected a whole number, counted from 1 as the file shows them` |
| érvénytelen idő | a [`shift`](shift.md) hibái |
| a `--first` és a `--last` ugyanazon az indexen | `--first and --last both name subtitle N, and two references on one subtitle define no transform` |
| határon kívüli index | `<path>: subtitle N is past the end: the file holds M subtitles` |
| két egyszerre kezdődő feliratsor | `<path>: subtitles N and M start at the same moment, so they define no transform` |
| nullpont előtti pozíció | `<path>: subtitle N would land before the origin, which no subtitle file can hold` |

**A három célkapcsoló egyike nélkül semmi sem íródik ki**, és ezek
hibái közösek az íráshoz használt alparancsokkal:
lásd [Indítás](invocation.md#a-cél).
