# `disjoint`

<!-- machine-translated -->
> Ez az oldal gépi fordítás, amelyet a nyelv egyik olvasója még nem nézett át.

```
subedit-cli disjoint (--output FILE | --output-dir DIR | --in-place)
                     [--minimum TIME] [--max-lines N] [--dashes] [--sort]
                     [--dry-run] [--recursive]
                     <file>...
```

Levágja az átfedő feliratsorokat, hogy **egyetlen `[start, end]` tartomány se fedjen át többé másikat**. Ez az ablak `Make Ranges Disjoint…` művelete — [ugyanaz a motor](../subedit-gui/lines.md#a-tartományok-átfedésmentessé-tétele).

**Semmi sem rövidül, és semmi sem hosszabbodik**: a fájl minden pillanatban ugyanazokat a szövegeket mutatja, mint előtte. Az idővonal felosztása változik:

- az átfedő csoport feliratsorainak **minden kezdeténél és minden végénél** szétvágja;
- az **egyetlen** feliratsor által lefedett intervallum ugyanaz a feliratsor marad;
- a **több** által lefedett intervallumból **egyetlen** feliratsor lesz, amelynek sorai az övéik, **a kezdetük sorrendjében** — a legkorábbi az 1. sorban, a következő a 2. sorban;
- az a feliratsor, amely semmivel sem fedi át, érintetlen marad, és a többi sorrendje megmarad.

<!-- example: subedit-cli disjoint --help -->
```console
$ subedit-cli disjoint --help
Cut overlapping subtitles so that no range overlaps another, merging what overlaps
Usage: subedit-cli disjoint [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to make disjoint

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --minimum TIME              Shortest interval kept; a shorter one is taken over by a neighbour (default 0.08)
  --max-lines N               Most lines of a merged subtitle; more are shown in windows, 0 for no limit (default 2)
  --dashes                    Put a dash before the text of each merged subtitle
  --sort                      Put the subtitles in order of their start first; without it, a file out of order is refused
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Argumentumok és kapcsolók

| Kapcsoló | Kötelező | Értékek | Alapértelmezés |
| :----- | :----- | :------ | :----- |
| `<file>...` | igen | egy vagy több elérési út | — |
| `--minimum` | nem | időtartam: `0.08`, `1.5`, `00:00:00,080`. A nulla minden intervallumot megtart | `0.08` (nagyjából két képkocka) |
| `--max-lines` | nem | egész szám: az egyesített feliratsor legtöbb sora; a `0` megszünteti a korlátot | `2` |
| `--dashes` | nem | kapcsoló érték nélkül: az egyesített feliratsorba minden feliratsor szövege elé kötőjel kerül | ki |
| `--sort` | nem | kapcsoló érték nélkül: a feliratsorokat **előbb** a kezdetük sorrendjébe rendezi — lásd [Rendezett fájl szükséges](#rendezett-fájl-szükséges) | ki |
| `--recursive`, `-r` | nem | kapcsoló érték nélkül | ki — lásd [Könyvtárfa feldolgozása](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **a három közül egy**, a `--dry-run` kapcsoló kivételével | lásd [Indítás](invocation.md#a-cél) | — |
| `--dry-run` | nem | kapcsoló érték nélkül: kiszámolja és közli, nem ír semmit — lásd [Megnézés írás előtt](invocation.md#megnézés-írás-előtt) | ki |

<!-- example: printf '1\n00:00:00,000 --> 00:00:05,000\nA\n\n2\n00:00:03,000 --> 00:00:08,000\nB\n\n' > chevauche.srt; subedit-cli disjoint --output disjoint.srt chevauche.srt; cat disjoint.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:05,000\nA\n\n2\n00:00:03,000 --> 00:00:08,000\nB\n\n' > chevauche.srt; subedit-cli disjoint --output disjoint.srt chevauche.srt; cat disjoint.srt
chevauche.srt: 2 subtitles split, 1 merged -> disjoint.srt
1
00:00:00,000 --> 00:00:03,000
A

2
00:00:03,000 --> 00:00:05,000
A
B

3
00:00:05,000 --> 00:00:08,000
B
```

Az `A [0, 5]` és a `B [3, 8]` három átfedésmentes feliratsort ad: az `A` egyedül, az `A` és a `B` együtt, a `B` egyedül. Egyetlen pillanat sem vész el, és egy sem kerül hozzá.

## Mit dönt el a művelet

**Egy sor sosem duplázódik.** Két azonos szövegű feliratsor ott, ahol átfedik egymást, egyetlen sort ad, és két szomszédos darab, amely ugyanazt mutatja, csak egy feliratsort alkot. A több darabra vágott feliratsor szövege egyébként **mindegyikben megismétlődik**: ez látszik a képernyőn az adott pillanatban.

**A nagyon rövid intervallumot egy szomszéd veszi át.** A `--minimum` kapcsolóval megadott érték alatt az intervallum eltűnik: a szomszéd **kiterjeszti a tartományát, hogy lefedje**, így egyetlen pillanat sem marad üresen, és semmi sem mozdul el ennél a küszöbnél jobban. A választott szomszéd az, amelynek sorai **már tartalmazzák** az intervallum sorait, ennek hiányában a leghosszabb. Az intervallum szövege csak akkor vész el, ha egyetlen szomszéd sem viszi, és akkor is a küszöbnél rövidebb ideig. **A küszöbnél rövidebb egész feliratsort sosem veszi át senki**: érintetlenül marad, és megszámolja.

**A korlátnál több sor: ablakok.** Az az egyesített feliratsor, amelynek a `--max-lines` kapcsolóval megadott értéknél több sora lenne, **egymást követő ablakokban** jelenik meg ekkora méretben, az időtartam egyenlő részeivel. Három sor három másodpercen át:

<!-- example: printf '1\n00:00:00,000 --> 00:00:03,000\nA\n\n2\n00:00:00,000 --> 00:00:03,000\nB\n\n3\n00:00:00,000 --> 00:00:03,000\nC\n\n' > trois.srt; subedit-cli disjoint --output fenetres.srt trois.srt; cat fenetres.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:03,000\nA\n\n2\n00:00:00,000 --> 00:00:03,000\nB\n\n3\n00:00:00,000 --> 00:00:03,000\nC\n\n' > trois.srt; subedit-cli disjoint --output fenetres.srt trois.srt; cat fenetres.srt
trois.srt: 0 subtitles split, 1 merged; 1 shown in windows for holding too many lines -> fenetres.srt
1
00:00:00,000 --> 00:00:01,500
A
B

2
00:00:01,500 --> 00:00:03,000
B
C
```

Négy sor három ablakot ad, és így tovább: az ablak soronként csúszik. Semmi sem csonkul, és semmi sem vágódik újra.

**A fordítás ugyanezeket a vágásokat követi**, mert a feliratsor mindkét szöveget hordozza. **Amit a formátum ezen felül hordoz** — stílus, pozíció — az első lefedő feliratsoré; a veszteséget megszámolja, ha az egyesített feliratsorok eltérnek.

**Az a feliratsor, amelynek a vége a kezdete előtt van**, nem találgatás tárgya: ott marad, ahol van, és megszámolja.

## Rendezett fájl szükséges

A csoportok szomszédok sorozatai: rendezetlenül a szomszédok nem azok, amelyek átfedik egymást. Az a fájl tehát, amelynek a kezdetei nincsenek sorrendben, **elutasításra kerül**, és az üzenet ezt közli:

<!-- example: printf '1\n00:00:03,000 --> 00:00:08,000\nB\n\n2\n00:00:00,000 --> 00:00:05,000\nA\n\n' > desordre.srt; subedit-cli disjoint --output x.srt desordre.srt; echo "code $?" -->
```console
$ printf '1\n00:00:03,000 --> 00:00:08,000\nB\n\n2\n00:00:00,000 --> 00:00:05,000\nA\n\n' > desordre.srt; subedit-cli disjoint --output x.srt desordre.srt; echo "code $?"
desordre.srt: The subtitles are not in the order of their start: sort them first. Use --sort to do that here.
code 2
```

A `--sort` kapcsoló előbb elvégzi a rendezést — ugyanazt, mint a [`sort`](sort.md) —, és ezt közli. A mag soha nem rendez magától.

A második menet nem változtat semmit: ha a tartományok már átfedésmentesek, nincs mit vágni, és a közlemény így szól: `already disjoint`.

## Kimenet

**Szabványos kimenet** — semmi: az eredmény a megírt fájl.

| Szint | Mi kerül még a hibakimenetre |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles split, M merged -> <destination>`, utána `;` után az, amit nem sikerült teljesen elvégezni: `K shown in windows for holding too many lines`, `K very short taken over by a neighbour`, `K subtitles shorter than the limit left whole`, `K merged with styles or positions that differ, the first kept`, `K subtitles ending before the start left alone` — vagy `<path>: already disjoint -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, majd **minden olvasási diagnosztika** |

Az `N subtitles split` a több darabra vágott eredeti feliratsorokat számolja, az `M merged` azokat a darabokat, amelyek több feliratsort mutatnak együtt.

### JSON-ban

A [`--format json`](invocation.md#szkript-által-olvasható-kimenet) kapcsolóval az egyes fájlok `counts` mezője:

| Számláló | Mit mond |
| :----- | :----------- |
| `subtitles` | a megírt fájl feliratsorai |
| `split`, `merged` | mint a közleményben |
| `windowed` | az ablakokban megjelenített darabok |
| `absorbed` | a szomszéd által átvett intervallumok |
| `too_short` | a küszöbnél rövidebb, érintetlenül hagyott feliratsorok |
| `ending_before_start` | az a feliratsor, amelynek a vége a kezdete előtt van, érintetlenül hagyva |
| `extras_dropped` | az egyesített darabok, amelyek stílusa vagy pozíciója eltér |
| `moved` | csak a `--sort` kapcsolóval: a megváltozott helyek |

## Kilépési kódok

Az eszköz kódjai: `0`, ha minden fájl feldolgozása megtörtént — az átfedések nélküli fájl is feldolgozottnak számít —, `2`, ha egyé sem, `3`, ha csak egyeseké, `1` használati hiba esetén.

## Hibák

| Mi váltja ki | Üzenet |
| :------------------- | :------ |
| rendezetlen fájl a `--sort` kapcsoló nélkül | `<path>: The subtitles are not in the order of their start: sort them first. Use --sort to do that here.` |
| a `--minimum` értéke nem időtartam, vagy negatív | `--minimum: …` |
| a `--max-lines` értéke nem egész szám | `--max-lines: "…" is not a number of lines: expected a whole number, 0 for no limit` |

Az olvashatatlan fájl úgy hibázik, mint bármelyik alparancsnál; a cél hibái az író alparancsokban közösek: lásd [Indítás](invocation.md#a-cél).

## A többi parancsban

A [`convert`](convert.md#a-tartományok-átfedésmentessé-tétele), az [`append`](append.md#a-tartományok-átfedésmentessé-tétele) és a [`split-file`](split-file.md#a-tartományok-átfedésmentessé-tétele) parancs elfogadja a `--disjoint` kapcsolót, amely írás előtt ugyanezt végzi el az alapértelmezett beállításokkal.

A rosszul szegmentált fájlon — az egymást követő darabokra vágott, rendezetlenül megadott szövegen — a rendezés és a vágás önmagában olyan sorokat ad, amelyek egyik darabról a másikra átbillennek: előbb a [`join-repeats`](join-repeats.md), vagy a [`repair`](repair.md), amely mindhármat láncba fűzi.
