# `pair`

<!-- machine-translated -->
> Ez az oldal gépi fordítás, amelyet a nyelv egyik olvasója még nem nézett át.

```
subedit-cli pair (--output FILE | --output-dir DIR | --in-place)
                 -t FILE [--align-method position|number]
                 [--dry-run]
                 <main>
```

Kiírja **a főfájl pozícióira újraidőzített fordítást**, és közli, hogyan találták meg a sorai a
feliratsorukat. Ez az ablak `Open Translation…` művelete — [ugyanaz a párosítás](../subedit-gui/files.md#fordítás-megnyitása),
ugyanaz a mondat —, amelyet az eredmény kiírása követ; az ablakban ezt a `Save Translation` végzi.

**Ez az egyetlen dolog, amit a párosítás tesz, és amit egy önmagában feldolgozott fájl nem.** A többi `-t` kapcsolót
elfogadó alparancs megváltoztatja a beolvasott fordítást — [`case`](case.md), [`replace`](replace.md)… —; a `pair`
a szövegen semmit sem változtat: a fordítást **ráfekteti** a főfájlra, és kiírja, amit a ráfektetés eredményezett. A főfájlt
azért olvassa be, hogy fogadja a sorokat, de **sosem írja ki**.

- **Amit kiír, az a fordítás**, a **saját útvonalán és a saját formátumában**: a fordításfájl formátumával, kódolásával és
  sorvégeivel. A feliratsorai **a főfájl pozícióin** állnak: az a sor, amely
  a jel mellé esett, visszakapja annak a feliratsornak a határait, amelyet fordít.
- **A `-t` kötelező.** A fordítás nélküli `pair`-nek nincs mit ráfektetnie: használati hiba, `1` kód.
- **Egyetlen főfájl**, és nincs `--recursive`: a pár két fájl, és egy fának nincs
  módja megmondani, melyik fordítás melyik főfájlhoz tartozik. Köteghez páronként egy-egy hívás kell.
- **Az `--output` a kiírt fordításfájl nevét adja meg, az `--output-dir` oda teszi az alapnevét, az `--in-place`
  újraírja azt**, mint minden `-t` kapcsolót elfogadó alparancsnál — lásd [Fordítás](invocation.md#fordítás).
- **A `--document` nem létezik**: csak egy célzott dokumentum van, a fordítás.

<!-- example: subedit-cli pair --help -->
```console
$ subedit-cli pair --help
Write a translation at the positions of its main file, and say how it lined up
Usage: subedit-cli pair [OPTIONS] main

Positionals:
  main TEXT REQUIRED          The main subtitle file

Options:
  -h,--help                   Print this help message and exit
  -t,--translation-file FILE  Translation file to lay over the subtitle file, for a single input
  --align-method position|number
                              How the lines of the translation find their subtitles
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Argumentumok és kapcsolók

| Kapcsoló | Kötelező | Értékek | Alapértelmezés |
| :----- | :----- | :------ | :----- |
| `<main>` | igen | **egyetlen** útvonal | — |
| `-t`, `--translation-file` | **igen** | a fordításfájl | — |
| `--align-method` | nem | `position` vagy `number` — lásd [Fordítás](invocation.md#fordítás) | `position` |
| `--output` / `--output-dir` / `--in-place` | **a három közül egy**, kivéve a `--dry-run` esetén | lásd [Indítás](invocation.md#a-cél) | — |
| `--dry-run` | nem | jelzőkapcsoló: kiszámítja és közli, de nem ír semmit — lásd [Megnézés írás előtt](invocation.md#megnézés-írás-előtt) | ki |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,300 --> 00:00:02,300\nBonjour.\n\n2\n00:00:03,300 --> 00:00:04,300\nAu revoir.\n\n' > film.fr.srt; subedit-cli pair film.srt -t film.fr.srt --output recale.fr.srt; cat recale.fr.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,300 --> 00:00:02,300\nBonjour.\n\n2\n00:00:03,300 --> 00:00:04,300\nAu revoir.\n\n' > film.fr.srt; subedit-cli pair film.srt -t film.fr.srt --output recale.fr.srt; cat recale.fr.srt
film.fr.srt: translation: 2 lines attached -> recale.fr.srt
1
00:00:01,000 --> 00:00:02,000
Bonjour.

2
00:00:03,000 --> 00:00:04,000
Au revoir.
```

A fordítás 0,3 másodpercet késett: minden sor megtalálta azt a feliratsort, amelyet a közepe átfed, és a
kiírt fájl a főfájl pozícióit viseli.

## Amit az illesztés közöl

A mondat **ugyanaz, amelyet az ablak mond**, amikor fordítást nyit meg, minden része csak akkor szerepel, ha nem
nulla — a részeinek táblázatát lásd az [`inspect -t`](inspect.md#fordítás--t) leírásánál. **Az 1. szinttől kezdve ki van írva**,
és nem a 2. szinten, mint a többi alparancsnál: itt az illesztés nem a
feldolgozás részlete, hanem az eredmény.

**Amit az illesztés a kiírt fájllal tesz**:

- **a feliratsor, amelyet senki sem fordított, szöveg nélküli blokként íródik ki**, amelyet anomália nélkül olvas be a program;
- **a sorból született feliratsor** — az a sor, amelyhez semmi sem ért el — a **saját pozícióival** íródik ki, bármi is
  a módszer: a kiírt fájl tehát több feliratsort számlálhat, mint a főfájl;
- **amit a dokumentum fordításként hordozott, az törlődik** a ráfektetés előtt, ami a parancssorban nem alkalmazható:
  a főfájlt egyedül olvassa be.

**A két módszer** ugyanazt a fájlt adja, ha a sorok mind a saját feliratsorukra esnek, és nem
ugyanazt, ha egy sor hiányzik vagy a fordítás el van csúszva — ezért választás a módszer, és ezért
a pozíció az alapértelmezés. A `src/test/data/paires/` nyolc esete sorolja fel őket, módszerenként a
várt fájllal.

## Kimenet

**Szabványos kimenet** — semmi: az eredmény a megírt fájl.

| Szint | Mi kerül még a hibakimenetre |
| :----- | :------------------------------------- |
| 1 | `<translation>: translation: N lines attached; … -> <destination>` — **az útvonal a fordításé**, amely a kiírt fájl; a `--dry-run` esetén `(dry run, nothing written)` áll a `-> <destination>` helyén |
| 2 | `<translation>: SubRip, UTF-8, no BOM, LF line endings kept` — **a fordítás** formátuma, kódolása, jelölője és sorvége, úgy visszaírva, ahogyan volt |
| 3 | `<translation>: N bytes read, M written`, majd a főfájl és a fordítás **minden olvasási diagnosztikája** — lásd [Indítás](invocation.md#olvasási-diagnosztika) |

### JSON-ban

A [`--format json`](invocation.md#szkript-által-olvasható-kimenet) esetén **egyetlen objektum**, amelynek a `file` mezője a fordítás, a
`destination` mezője pedig a kiírt fájl. A `counts.subtitles` **a kiírt fájl feliratsorait** számolja, a sorból
életre kelteket is. Az `alignment` kulcs — `{"file", "method", "attached", "born", "untranslated", "out_of_order"}` —
ugyanazokat a számokat hordozza, mint a mondat, ahogyan az [`inspect`](inspect.md#json-ban) `translation` kulcsa.

## Kilépési kódok

Az eszköz általános kódjai: `0`, ha a fordítás ki van írva — vagy `--dry-run` esetén kiszámítva —, `2`, ha a két fájl egyikét nem
sikerült beolvasni, vagy a célt nem sikerült megírni, `1` használati hiba esetén. **A rosszul illesztett fordítás nem
hiba**: kiírja, és a mondat közli, mi nem találta meg a helyét.

## Hibák

| Mi váltja ki | Üzenet |
| :------------------- | :------ |
| nincs `-t` | `pair needs the translation file to lay over the main one: use -t` |
| `--document` | `The following argument was not expected: --document` — a `pair` esetén nem létezik |
| egynél több főfájl | `The following argument was not expected: <path>` |
| a fordítás a főfájl | `<translation>: the file is already open as the main document`, `2` kód |
| a fájlok egyike hiányzik, olvashatatlan vagy ismeretlen formátumú | ugyanaz, mint mindenütt, a hibás fájl útvonalával |
