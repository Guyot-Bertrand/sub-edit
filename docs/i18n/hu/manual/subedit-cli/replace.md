# `replace`

<!-- machine-translated -->
> Ez az oldal gépi fordítás, amelyet a nyelv egyik olvasója még nem nézett át.

```
subedit-cli replace [--regex] [--case-sensitive] [--range N-M|N-]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    <pattern> <replacement> <file>...
```

Egy szöveget cserél le a feliratsorokban, **egyetlen címke megtörése nélkül**. Ez az ablak
[keresésének](../subedit-gui/find-and-replace.md) `Replace All` művelete, ugyanazokkal a szabályokkal:

- **a keresés a látható szövegben történik** — abban, amit a néző olvas. A címkék nem részei
  ennek: a `<i>` keresése semmit sem talál, a `Bonjour` keresése viszont megtalálja azt a „Bonjour” szót, amelynek fele
  dőlt betűs;
- **a csere a forrásszövegben történik**, a fájléban, a címkéket ott hagyva, ahol
  voltak. Az a címke, amely megtöri a keresett szót, az egész cserét körülöleli; az a stílus, amely
  érinti az egyezést, az egész cserét lefedi; amit az egyezés nem érint,
  az nem mozdul. A szabályok esetenként az ablak keresésének szabályai.

**Nincs `find` alparancs**: annak megnézéséhez, hogy hol van egy minta, anélkül hogy bármi megváltozna,
a [`--dry-run`](invocation.md#megnézés-írás-előtt) felsorolja az érintett feliratsorokat.

<!-- example: subedit-cli replace --help -->
```console
$ subedit-cli replace --help
Replace a text in the subtitles, without breaking a tag
Usage: subedit-cli replace [OPTIONS] pattern replacement files...

Positionals:
  pattern TEXT REQUIRED       Text to look for in what is shown
  replacement TEXT REQUIRED   What replaces it, tags allowed
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --regex                     Read the text to look for as a regular expression
  --case-sensitive            Tell capitals from small letters
  --range N-M|N-              Act only on subtitles N to M, or N to the end
  --document main|translation The document to change: the main one, or the translation given by -t
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
| `<pattern>` | igen | a megjelenített szövegben keresett szöveg; **nem lehet üres** | — |
| `<replacement>` | igen | ami a helyére kerül, címkék megengedettek; lehet üres | — |
| `<file>...` | igen | egy vagy több útvonal | — |
| `--recursive`, `-r` | nem | jelzőkapcsoló | ki — lásd [Könyvtárfa feldolgozása](lots.md) |
| `--regex` | nem | jelzőkapcsoló: a minta ICU reguláris kifejezés | egyszerű szöveg |
| `--case-sensitive` | nem | jelzőkapcsoló: megkülönbözteti a nagy- és kisbetűket | figyelmen kívül hagyja őket |
| `--range` | nem | `N-M`, vagy `N-` a végéig — lásd [`adjust`](adjust.md#--range) | az egész fájl |
| `--document` | nem | `main` vagy `translation` — lásd [Fordítás](invocation.md#fordítás) | `main` |
| `-t`, `--translation-file` | a `--document translation` esetén | fordításfájl, **egyetlen** bemenethez | — |
| `--align-method` | nem, és csak a `-t` mellett | `position` vagy `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **a három közül egy**, kivéve a `--dry-run` esetén | lásd [Indítás](invocation.md#a-cél) | — |
| `--dry-run` | nem | jelzőkapcsoló: kiszámítja és közli, de nem ír semmit — lásd [Megnézés írás előtt](invocation.md#megnézés-írás-előtt) | ki |

**A kötőjellel kezdődő mintát** a `--` után kell írni: `subedit-cli replace -- -- "—" film.srt`.

**Alapértelmezés szerint a főszöveg; a fordítás a `--document translation -t FILE` megadásával.** Ekkor
a fordítás módosul és íródik ki, és csak az — lásd [Fordítás](invocation.md#fordítás).

### Egyszerű szöveg vagy kifejezés

Alapértelmezés szerint a mintát **úgy olvassa, ahogy van**: az `a.b` egy `a` betűt, egy pontot és egy `b` betűt keres. A kis- és
nagybetűk figyelmen kívül hagyása ugyanolyan finom, mint az ablakban — az `É` és az `é` egyetlen betű.

A `--regex` esetén a minta **ICU** reguláris kifejezés, úgy olvasva, ahogyan a Gaupol olvassa: a `.` átlépi
a sortörést, a `^` és a `$` a feliratsor minden során érvényes. A cserében, és
csakis ott:

| Írás | Jelentés |
| :----- | :-------- |
| `$1` – `$9` | az egyezés csoportjai |
| `$0` | az egész egyezés |
| `\n` | sortörés |
| `\$`, `\\` | egy `$`, egy fordított perjel |

Ügyelni kell a shell idézőjeleire: `'$1 EUR'`, és nem `"$1 EUR"`, mert az utóbbinál a shell olvasná be a `$1` részt.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nPrix: 12 euros, et 7 euros.\n\n' > prix.srt; subedit-cli replace --regex '([0-9]+) euros' '$1 EUR' --output euros.srt prix.srt; cat euros.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nPrix: 12 euros, et 7 euros.\n\n' > prix.srt; subedit-cli replace --regex '([0-9]+) euros' '$1 EUR' --output euros.srt prix.srt; cat euros.srt
prix.srt: replaced 2 matches -> euros.srt
1
00:00:01,000 --> 00:00:02,000
Prix: 12 EUR, et 7 EUR.
```

## Címkék

Egy szöveg, ahol egy címke megtöri a keresett szót:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace Bonjour Salut --output salut.srt balises.srt; cat salut.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace Bonjour Salut --output salut.srt balises.srt; cat salut.srt
balises.srt: replaced 2 matches -> salut.srt
1
00:00:01,000 --> 00:00:02,000
<i>Salut</i> tout le monde

2
00:00:03,000 --> 00:00:04,000
Salut <i>Marie</i>, ca va
```

Az első feliratsorban a címke megtörte a „Bonjour” szót: most a „Salut” szót
öleli körül. A másodikban érintetlen, és a helyén marad. **A nyitva hagyott címke
nyitva marad**: a hibás jelölés javítása másik művelet volna.

A cserét **a fájl formátumának címkéivel** kell olvasni: `<i>…</i>` a SubRip,
`{\i1}…` az Advanced SSA esetén. Ez az egyetlen módja annak, hogy az ember a beírt szöveget formázza.

## Semmi sem található, semmi sem változik

Két válasz, amely nem hiba — a kilépési kód `0`, és **a fájl ennek ellenére
kiíródik**, mert a megadott cél megírt célt jelent:

| Helyzet | Mondat |
| :-------- | :----- |
| a minta sehol sincs | `"zzz" not found` |
| megvan, de minden egyezést önmagára cserélnek | `nothing to change` |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli replace zzz x --output b.srt a.srt; subedit-cli replace Bonjour Bonjour --output c.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli replace zzz x --output b.srt a.srt; subedit-cli replace Bonjour Bonjour --output c.srt a.srt
a.srt: "zzz" not found -> b.srt
a.srt: nothing to change -> c.srt
```

## `--dry-run`

A szabványos kimenet felsorolja **minden feliratsort, amelynek a szövege megváltozna**, előtte és utána, úgy,
ahogyan a fájlban áll — a címkékkel együtt:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace --dry-run Bonjour Salut balises.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace --dry-run Bonjour Salut balises.srt
balises.srt: replaced 2 matches (dry run, nothing written)
balises.srt: subtitle 1
- <i>Bon</i>jour tout le monde
+ <i>Salut</i> tout le monde
balises.srt: subtitle 2
- Bonjour <i>Marie</i>, ca va
+ Salut <i>Marie</i>, ca va
```

A lista alakját a [Megnézés írás előtt](invocation.md#megnézés-írás-előtt) írja le;
JSON-ban ez a `changes`.

## Kimenet

**Szabványos kimenet** — semmi: az eredmény a megírt fájl. A `--dry-run` esetén a fenti
változáslista.

| Szint | Mi kerül még a hibakimenetre |
| :----- | :------------------------------------- |
| 1 | `<path>: replaced N matches -> <destination>`, vagy `"<pattern>" not found`, vagy `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — a beolvasott fájl formátuma, kódolása, jelölője és sorvége, úgy visszaírva, ahogyan volt |
| 3 | `<path>: N bytes read, M written`, majd **minden olvasási diagnosztika** — lásd [Indítás](invocation.md#olvasási-diagnosztika) |

Az `N` **a szövegükben megváltozott feliratsorokban lecserélt egyezéseket** számolja — ezt
a számot az ablak is kimondja.

### JSON-ban

A [`--format json`](invocation.md#szkript-által-olvasható-kimenet) esetén minden fájl objektuma tartalmazza:

| Kulcs | Tartalom |
| :-- | :------ |
| `counts.replaced` | a lecserélt egyezések a szövegükben megváltozott feliratsorokban: a mondat `N` száma |
| `counts.matched` | a talált egyezések, **akár változtatta meg a cseréjük a szöveget, akár nem** — ez különbözteti meg a „semmi sem található” esetet (`0`) a „semmi sem változik” esettől (a `replaced` értéke `0`, a `matched` pozitív) |
| `changes` | a megváltozott feliratsorok: `{"subtitle", "document", "before", "after"}`, **üres**, ha semmi sem változott |

## Kilépési kódok

Az eszköz általános kódjai: `0`, ha minden fájl feldolgozásra került — az a fájl is, amelyben semmi sem található
—, `2`, ha egy sem, `3`, ha csak néhány, `1` használati hiba esetén.

## Hibák

| Mi váltja ki | Üzenet |
| :------------------ | :------ |
| üres minta | `pattern: nothing to look for` |
| olvashatatlan kifejezés | `pattern: not a regular expression (<ICU's reason>)` |
| rosszul írt vagy a fájlon kívüli tartomány | lásd [`adjust`](adjust.md#--range) |

**A mintát az első fájl előtt olvassa be**: az olvashatatlan kifejezés használati hiba
(`1` kód), semmit sem olvas be és nem ír ki.

A cél hibái közösek az íráshoz használt alparancsokkal:
lásd [Indítás](invocation.md#a-cél).
