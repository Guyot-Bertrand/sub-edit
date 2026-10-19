# `join-repeats`

<!-- machine-translated -->
> Ez az oldal gépi fordítás, amelyet a nyelv egyik olvasója még nem nézett át.

```
subedit-cli join-repeats (--output FILE | --output-dir DIR | --in-place)
                         [--dry-run]
                         [--recursive]
                         <file>...
```

Egyetlen **feliratsorrá** egyesíti az egymás után ugyanazt mutató feliratsorokat: az egymást követő darabokra vágott szöveget, ahogy egyes szegmentálóeszközök hagyják. Ez az ablak `Join Repeated Subtitles` művelete — [ugyanaz a motor](../subedit-gui/lines.md#ismétlődő-feliratsorok-egyesítése).

**Ismétlődő feliratsor** az a feliratsor, amelynek **ugyanaz a szövege, ugyanaz a fordítása és ugyanazok a formátumbeállításai** (stílus, pozíció, jelenetbeállítások) vannak, mint egy másiknak, **és legkésőbb akkor kezdődik, amikor az a másik véget ér**: érintkeznek vagy átfedik egymást. Az ismétlődők sorozatából **az először kezdődő** feliratsor marad meg, amelynek a vége a sorozat legkésőbbi végére tolódik; a többi eltűnik.

- **A fájlbeli sorrend nem számít, és nem is változik**: a megmaradó feliratsor az, amelyik először kezdődik, bárhol is vannak a többiek. A rendezett fájl rendezett marad.
- **Tévedésből semmi sem egyesül**: más szöveg, más fordítás, más beállítások, rés a két darab között, szöveg nélküli feliratsor vagy olyan, amelynek a vége a kezdete előtt van — ezekben az esetekben különállók maradnak.
- **A többi előtt végzendő**: a rendezés és [az átfedések levágása](disjoint.md) tisztább sorozatból jobban dolgozik. A [`repair`](repair.md) mindhármat láncba fűzi, a [`convert`](convert.md#a-tartományok-átfedésmentessé-tétele) pedig kapcsolóként fogadja el (`--join-repeats`, `--repair`).

<!-- example: subedit-cli join-repeats --help -->
```console
$ subedit-cli join-repeats --help
Join the subtitles that repeat each other, one after the other, into one
Usage: subedit-cli join-repeats [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to join the repeats of

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
| `<file>...` | igen | egy vagy több elérési út | — |
| `--recursive`, `-r` | nem | kapcsoló érték nélkül | ki — lásd [Könyvtárfa feldolgozása](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **a három közül egy**, a `--dry-run` kapcsoló kivételével | lásd [Indítás](invocation.md#a-cél) | — |
| `--dry-run` | nem | kapcsoló érték nélkül: kiszámolja és közli, nem ír semmit — lásd [Megnézés írás előtt](invocation.md#megnézés-írás-előtt) | ki |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:04,000 --> 00:00:06,000\nAlpha\n\n4\n00:00:08,000 --> 00:00:09,000\nBeta\n\n' > morceaux.srt; subedit-cli join-repeats --output joint.srt morceaux.srt; cat joint.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:04,000 --> 00:00:06,000\nAlpha\n\n4\n00:00:08,000 --> 00:00:09,000\nBeta\n\n' > morceaux.srt; subedit-cli join-repeats --output joint.srt morceaux.srt; cat joint.srt
morceaux.srt: 2 repeated subtitles joined -> joint.srt
1
00:00:01,000 --> 00:00:06,000
Alpha

2
00:00:08,000 --> 00:00:09,000
Beta
```

Az `Alpha` szöveg három darabja, amelyek 1 s-tól 6 s-ig követik egymást, most már csak egyet alkot. A `Beta`, amelyet rés választ el, megmaradt. A második menet nem változtat semmit, és ezt közli is: `no repeated subtitles`.

## Kimenet

**Szabványos kimenet** — semmi: az eredmény a megírt fájl.

| Szint | Mi kerül még a hibakimenetre |
| :----- | :------------------------------------- |
| 1 | `<path>: N repeated subtitles joined -> <destination>`, vagy `<path>: no repeated subtitles -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, majd **minden olvasási diagnosztika** |

### JSON-ban

A [`--format json`](invocation.md#szkript-által-olvasható-kimenet) kapcsolóval az egyes fájlok `counts` mezője: `subtitles` (a megírt fájl feliratsorai), `joined` (az eltávolított feliratsorok, mert egy másik lefedi őket) és `chains` (az egyesített sorozatok).

## Kilépési kódok

Az eszköz kódjai: `0`, ha minden fájl feldolgozása megtörtént — az ismétlődők nélküli fájl is feldolgozottnak számít —, `2`, ha egyé sem, `3`, ha csak egyeseké, `1` használati hiba esetén.

## Hibák

A `join-repeats` alparancsnak nincsenek saját hibái: az olvashatatlan fájl úgy hibázik, mint bármelyik alparancsnál, a cél hibái pedig az író alparancsokban közösek: lásd [Indítás](invocation.md#a-cél).
