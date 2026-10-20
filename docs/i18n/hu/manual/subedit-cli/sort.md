# `sort`

<!-- machine-translated -->
> Ez az oldal gépi fordítás, amelyet a nyelv egyik olvasója még nem nézett át.

```
subedit-cli sort (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 <file>...
```

A feliratsorokat visszarendezi **a kezdetük sorrendjébe**. Semmi más nem változik: sem a szövegek, sem a
pozíciók, sem a fájl formátuma. A feliratsorok az íráskor **újra lesznek számozva** 1-től,
mint minden visszaírt fájl esetében.

**A mag sosem rendez magától**: egy fájl beolvasása érintetlenül hagyja a sorrendjét, és az
[`inspect`](inspect.md#amit-az-anomalies-jelez) a sorrend megtörését a
`subtitle N starts before the previous one starts` üzenettel jelzi. A `sort` **a szándékos beavatkozás** egy olyan fájlhoz
vagy köteghez, amelynek a sorrendje megbomlott — és az ablakban, mint minden művelet, visszavonható.

- **A rendezés stabil**: két feliratsor, amely ugyanabban a pillanatban kezdődik, **megtartja azt a sorrendet, amelyet a
  fájl adott nekik**. Egyik sem kerül a másik elé; az áthelyezésük olyan döntés volna, amelyet senki
  nem kért.
- **A `sort` nem fogad el sem tartományt, sem dokumentumot**: nincs mit választani, a sorrend a kezdetek sorrendje.

<!-- example: subedit-cli sort --help -->
```console
$ subedit-cli sort --help
Put the subtitles in the order of their start, keeping ties as they are
Usage: subedit-cli sort [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to sort

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
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
| `--output` / `--output-dir` / `--in-place` | **a három közül egy**, kivéve a `--dry-run` esetén | lásd [Indítás](invocation.md#a-cél) | — |
| `--dry-run` | nem | jelzőkapcsoló: kiszámítja és közli, de nem ír semmit — lásd [Megnézés írás előtt](invocation.md#megnézés-írás-előtt) | ki |

<!-- example: printf '1\n00:00:05,000 --> 00:00:06,000\ncinq\n\n2\n00:00:01,000 --> 00:00:02,000\nun\n\n3\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > desordre.srt; subedit-cli sort --output ordre.srt desordre.srt; cat ordre.srt -->
```console
$ printf '1\n00:00:05,000 --> 00:00:06,000\ncinq\n\n2\n00:00:01,000 --> 00:00:02,000\nun\n\n3\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > desordre.srt; subedit-cli sort --output ordre.srt desordre.srt; cat ordre.srt
desordre.srt: 3 subtitles moved -> ordre.srt
1
00:00:01,000 --> 00:00:02,000
un

2
00:00:03,000 --> 00:00:04,000
trois

3
00:00:05,000 --> 00:00:06,000
cinq
```

## Mit számol

**A szám azoké a helyeké, amelyeken másik feliratsor került**, ahogyan a mag jelenti — amit
az ablak előzményei áthelyezettnek neveznének. Egyetlen rossz helyen álló feliratsor többet is
elmozdít: a fenti példában a három hely közül egyik sem tartja meg a feliratsorát, és a mondat azt mondja: `3 subtitles moved`.

**A már rendezett fájl is kiíródik**, és ezt közli: a megadott cél
megírt célt jelent, és ha a `sort` kivétel volna, a szkriptnek tudnia kellene, hogy melyik
alparancs ír olykor fájlt, olykor meg semmit.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > ordre.srt; subedit-cli sort --output copie.srt ordre.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > ordre.srt; subedit-cli sort --output copie.srt ordre.srt
ordre.srt: already in order -> copie.srt
```

## Kimenet

**Szabványos kimenet** — semmi: az eredmény a megírt fájl. A `--dry-run` esetén szintén semmi: nincs
fájl, és ezt a tájékoztató sor mondja ki — lásd [Megnézés írás előtt](invocation.md#megnézés-írás-előtt).

| Szint | Mi kerül még a hibakimenetre |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles moved -> <destination>`, vagy `<path>: already in order -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — a beolvasott fájl formátuma, kódolása, jelölője és sorvége, úgy visszaírva, ahogyan volt |
| 3 | `<path>: N bytes read, M written`, majd **minden olvasási diagnosztika** — lásd [Indítás](invocation.md#olvasási-diagnosztika) |

### JSON-ban

A [`--format json`](invocation.md#szkript-által-olvasható-kimenet) esetén minden fájl objektuma tartalmazza a
`counts.subtitles` (a fájl összes feliratsora) és a `counts.moved` (a helyet cserélt sorok;
`0` a már rendezett fájlnál) mezőt.

## Kilépési kódok

Az eszköz általános kódjai: `0`, ha minden fájl feldolgozásra került — a már rendezett fájl is —, `2`, ha
egy sem, `3`, ha csak néhány, `1` használati hiba esetén.

## Hibák

A `sort`-nak nincs saját hibája: az olvashatatlan fájl úgy hibázik, mint bármelyik alparancsnál, a
cél hibái pedig közösek az íráshoz használt alparancsokkal:
lásd [Indítás](invocation.md#a-cél).
