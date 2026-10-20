# `hearing-impaired`

<!-- machine-translated -->
> Ez az oldal gépi fordítás, amelyet a nyelv egyik olvasója még nem nézett át.

```
subedit-cli hearing-impaired
                  (--output FILE | --output-dir DIR | --in-place)
                  [--dry-run]
                  [--recursive] [--parentheses]
                  [--document main|translation] [-t FILE] [--align-method position|number]
                  <file>...
```

A teljes fájlból eltávolítja a **hallássérültek számára készült megjegyzéseket** — a szögletes zárójelek között leírt hangokat. **A kerek zárójelek alapértelmezés szerint megmaradnak**: ugyanolyan gyakran szolgálnak suttogott replikára, mint leírt hangra, és az eltávolításuk párbeszédet törölne. A `--parentheses` kapcsoló ezeket is eltávolítja, olyan fájlhoz, amely bennük írja le a hangjait.

Amit nem végez el: a kettőskeresztek közötti dalszövegek, a kettőspont előtti beszélőnév, az ismételt nagybetűsítés, az OCR-hibák javítása. Ezek a minták az ablak [`Correct Texts…`](../subedit-gui/correct-texts.md) segédjének mintái, amelyet a parancssor a [`correct`](correct.md) néven kínál; itt pedig semmi sem állítható: az átalakítás el van döntve, nem beállítható.

<!-- example: subedit-cli hearing-impaired --help -->
```console
$ subedit-cli hearing-impaired --help
Remove the sounds described between square brackets
Usage: subedit-cli hearing-impaired [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to clean

Options:
  -h,--help                   Print this help message and exit
  --parentheses               Also remove the sounds between parentheses, often whispered lines
  -r,--recursive              Take directories as inputs, and every subtitle file in them
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
| `<file>...` | igen | egy vagy több elérési út | — |
| `--parentheses` | nem | kapcsoló érték nélkül: a kerek zárójelek közötti megjegyzéseket is eltávolítja | ki |
| `--recursive`, `-r` | nem | kapcsoló érték nélkül | ki — lásd [Könyvtárfa feldolgozása](lots.md) |
| `--document` | nem | `main` vagy `translation` — lásd [Fordítás](invocation.md#fordítás) | `main` |
| `-t`, `--translation-file` | a `--document translation` kapcsolóval | fordításfájl, **egyetlen** bemenethez | — |
| `--align-method` | nem, és csak a `-t` kapcsolóval | `position` vagy `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **a három közül egy**, a `--dry-run` kapcsoló kivételével | lásd [Indítás](invocation.md#a-cél) | — |
| `--dry-run` | nem | kapcsoló érték nélkül: kiszámolja és közli, nem ír semmit — lásd [Megnézés írás előtt](invocation.md#megnézés-írás-előtt) | ki |

Az olvasott fájl formátuma **megmarad**: a formátum módosítása a [`convert`](convert.md) dolga.

**Alapértelmezés szerint a fő szöveg; a fordítás a `--document translation -t FILE` kapcsolókkal.** Ilyenkor a fordítás módosul és íródik meg, és csakis az — lásd [Fordítás](invocation.md#fordítás).

**Az a megjegyzés, amely kiürítene egy fordítást, üresen írja meg, és megtartja a feliratsort**: a feliratsor szöveg nélkül marad a fájlban. A fő szöveg ezzel szemben elveszíti a teljesen kiürült feliratsort, lásd alább.

## Mi törlődik, és mi marad

A megjegyzés egy szögletes zárójel — vagy kerek zárójel a `--parentheses` kapcsolóval — és az, amit körülzár, **a sortörést is beleértve**: a valódi fájlokban a megjegyzést gyakran elvágja a sor vége.

| Bemenet | Kimenet |
| :----- | :----- |
| `Bonjour [il tousse] Marie` | `Bonjour Marie` |
| `Bonjour[il tousse]Marie` | `Bonjour Marie` |
| `[Bruit de pas] Bonjour` | `Bonjour` |
| `A [un] [deux] B` | `A B` |

Az eltávolítás **pontosan egy szóközt hagy azok között, amik körülvették, a sor szélén pedig semmit**. Ez nem a fájl szóközeinek tisztítása: a szövegben már meglévő két szóköz két szóköz marad.

**Ha a megjegyzést a sortörés elvágja, a sortörés marad meg** — a két sor két sor marad. Ez az eset nem érdekesség: tizenöt valódi feliratfájlban minden árvának tűnő szögletes zárójel ilyen módon elvágott megjegyzés volt.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nReculez ! [Il\nhurle] Tout de suite !\n\n' > a.srt; subedit-cli --quiet hearing-impaired --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nReculez ! [Il\nhurle] Tout de suite !\n\n' > a.srt; subedit-cli --quiet hearing-impaired --output b.srt a.srt; cat b.srt
1
00:00:01,000 --> 00:00:03,000
Reculez !
Tout de suite !
```

**Ami nem megjegyzés, és nem mozdul:**

| Eset | Mi történik |
| :-- | :-------------- |
| kizárólag számokat tartalmazó tartalom — `[1]`, `(12)`, `[ 156478 ]` | változatlanul marad, a határolókkal együtt: a hivatkozás sosem nevezett meg hangot |
| kerek zárójel és tartalma a `--parentheses` kapcsoló nélkül | változatlanul marad: a `Venez (chut) ici` nem változik |
| zárójel, amelyet semmi sem zár le | érintetlenül marad |
| formátumcímke — `<i>`, `</i>` | sosem törlődik |

A számjegyek, egy betű vagy egy jel **között** álló üres hely — `[1 2]`, `[1a]`, `[-1]` — megjegyzéssé teszi. Ugyanígy az üres tartalom, a `[]` vagy a `( )`: nincs számjegy, tehát nem hivatkozás; nincs mondanivaló, tehát nincs mit megtartani.

## Ami teljesen eltűnik

**A sor, amelyet az eltávolítás kiürített, eltűnik**, és **a teljesen kiürült feliratsor kikerül a fájlból**. A következő feliratsorok újraszámozódnak.

A sor üresnek számít, ha csak szóközök, formátumcímkék vagy egyetlen párbeszéd-kötőjel maradt benne — a `<i>[Musique]</i>` nem hagy egy `<i></i>` címkepárt a képernyőn.

**A címkék a fájl formátumának címkéi.** A `{\i1}[Musique]{\i0}` egy Advanced SSA-ban, a `{Y:i}[Musique]` egy MicroDVD-ben és a `/[Musique]` egy MPL2-ben szintén semmit sem hagy. Fordítva, az `<i>` olyan formátumban, amely nem ír címkét — TMPlayer, LRC —, szöveg, amelyet a néző olvas, és az a sor, amely hordozza, nem üres.

**Az egyetlen hangra szűkült párbeszéd már nem párbeszéd**, és a megmaradt kötőjel eltűnik. Amíg két hang marad, a kötőjelek maradnak.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\nVoir [1] la note.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\nVoir [1] la note.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle cleaned, 1 removed -> b.srt
1
00:00:04,000 --> 00:00:06,000
Attends Marie.

2
00:00:07,000 --> 00:00:09,000
Voir [1] la note.
```

## A megjegyzés nélküli fájl is megíródik

Azonos másolatként íródik meg, a kilépési kód `0`, és a jelentés közli, hogy nem volt mit eltávolítani.

Ez az író alparancsok szabálya: **a megadott cél megírt cél**. Kivételt tenni itt arra kényszerítené a szkriptet, hogy tudja, az alparancsok közül melyik állít elő néha fájlt, néha semmit.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nRien à signaler.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nRien à signaler.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt
a.srt: no mention to remove -> b.srt
```

## Kimenet

**Szabványos kimenet** — semmi: az eredmény a megírt fájl. **A `--dry-run` kapcsolóval a javasolt módosítások**: minden olyan feliratsorhoz, amelynek a szövege változna, a sorszáma, az előtte és az utána álló szöveg — lásd [Megnézés írás előtt](invocation.md#megnézés-írás-előtt).

| Szint | Mi kerül még a hibakimenetre |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles cleaned, M removed -> <destination>`, vagy `<path>: no mention to remove -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — az olvasott fájl formátuma, kódolása, jelölője és sorvége, úgy visszaállítva, ahogy volt |
| 3 | `<path>: N bytes read, M written`, majd **minden olvasási diagnosztika** — lásd [Indítás](invocation.md#olvasási-diagnosztika) |

Az `N` azokat a feliratsorokat számolja, amelyeknek a szövege megváltozott, az `M` azokat, amelyeket az eltávolítás teljesen kiürített, és amelyek így kikerültek a fájlból.

### JSON-ban

A [`--format json`](invocation.md#szkript-által-olvasható-kimenet) kapcsolóval az egyes fájlok objektuma tartalmazza a `counts.cleaned` (az átírt szövegek) és a `counts.removed` (az eltávolított feliratsorok) mezőt; **a `--dry-run` kapcsolóval** a `changes` kulcs a módosított feliratsorokat is felsorolja, mint a szöveges alak szabványos kimenete. A teljes táblázat az [Indítás](invocation.md#amit-az-egyes-alparancsok-a-counts-mezőbe-írnak) oldalán található.

## Kilépési kódok

Az eszköz kódjai: `0`, ha minden fájl megírása megtörtént, `2`, ha egyé sem, `3`, ha csak egyeseké, `1` használati hiba esetén.

## Hibák

Ennek az alparancsnak nincs saját hibája: nem fogad el elemzendő értéket, és semmilyen szöveg nem késztetheti egy fájl elutasítására. Ami hibázhat, az az olvasás vagy az írás, mint a többinél.

**A három célkapcsoló egyike nélkül semmi sem íródik meg**, és ezek hibái közösek az író alparancsokban: lásd [Indítás](invocation.md#a-cél).
