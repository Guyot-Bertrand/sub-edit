# `inspect`

<!-- machine-translated -->
> Ez az oldal gépi fordítás, amelyet a nyelv egyik olvasója még nem nézett át.

```
subedit-cli inspect [--frame-rate RATE] [--recursive] [--detail]
                   [-t FILE] [--align-method position|number] <file>...
```

Jelenti, miből áll az egyes fájl. **Semmit sem módosít, és nem ír fájlt.**

<!-- example: subedit-cli inspect --help -->
```console
$ subedit-cli inspect --help
Report what a subtitle file is made of
Usage: subedit-cli inspect [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to report on

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --detail                    List every anomaly, subtitle by subtitle, instead of counting them
  --frame-rate RATE           Frame rate of a file counted in frames: 25, 23.976
  -t,--translation-file FILE  Translation file to lay over the subtitle file, for a single input
  --align-method position|number
                              How the lines of the translation find their subtitles
```

## Argumentumok

| Argumentum | Kötelező | Érték | Alapértelmezés |
| :------- | :----- | :----- | :----- |
| `<file>...` | igen | egy vagy több feliratfájl elérési útja | — |
| `--recursive`, `-r` | nem | kapcsoló érték nélkül | ki — lásd [Könyvtárfa feldolgozása](lots.md) |
| `--detail` | nem | kapcsoló érték nélkül: megnevezi az egyes anomáliákat, ahelyett hogy megszámolná őket — lásd [Amit az `anomalies` jelez](#amit-az-anomalies-jelez) | ki |

Az elérési út hiánya használati hiba, tehát `1` kód.

## Kapcsolók

| Kapcsoló | Kötelező | Érték | Alapértelmezés |
| :----- | :------ | :----- | :----- |
| `--frame-rate` | nem | sebesség képkocka/másodpercben: `25`, `23.976`, `29.97` | 23.976 |

**Csak a képkockákban számolt fájlokra vonatkozik**, vagyis a MicroDVD-re, a kilenc formátum közül az egyetlenre. A MicroDVD-fájl csak képkockaszámokat hordoz; az a sebesség, amellyel olvassák őket, dönti el **a jelentés minden pozícióját**, a `span` mezővel kezdve. Máshol megadva semmit sem változtat, és nem hiba.

Az az érték, amely semmit sem nevez meg, használati hiba, tehát `1` kód:

<!-- example: subedit-cli inspect --frame-rate douze film.sub; echo $? -->
```console
$ subedit-cli inspect --frame-rate douze film.sub; echo $?
--frame-rate: "douze" is not a frame rate: expected frames per second, like 25 or 23.976
1
```

Ez ugyanaz a kapcsoló, mint a [`convert`](convert.md) parancsnál, szóról szóra, mert ugyanarról a sebességről van szó. A `convert` **íráskor** is használja.

## Fordítás: `-t`

| Kapcsoló | Kötelező | Érték | Alapértelmezés |
| :----- | :------ | :----- | :----- |
| `-t`, `--translation-file` | nem | fordításfájl, **egyetlen** bemenethez | — |
| `--align-method` | nem, és csak a `-t` kapcsolóval | `position` vagy `number` | `position` |

A `-t` kapcsolóval a fordítás **úgy kerül a fájlra, ahogy az ablak megnyitáskor teszi** — lásd [Fordítás](invocation.md#fordítás) —, és a jelentés közli, **hogyan találták meg a sorai a feliratsorukat**. A szkript így tudja, hogy a fordítás illeszkedik-e, mielőtt javítaná.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour.\n\n' > film.fr.srt; subedit-cli --quiet inspect -t film.fr.srt film.srt | tail -3 -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour.\n\n' > film.fr.srt; subedit-cli --quiet inspect -t film.fr.srt film.srt | tail -3
  anomalies: none
  translation file: film.fr.srt, matched by position
  translation: 1 line attached; 1 subtitle left without a translation
```

Az első hozzáadott sor megnevezi a fájlt és a módszert; a második **az a mondat, amelyet az ablak kimond**, amikor fordítást nyit meg, minden része csak akkor, ha nem nulla:

| Rész | Mit számol |
| :----- | :---------------- |
| `N lines attached` | a fordítás azon sorai, amelyek találtak feliratsort a főfájlban |
| `N subtitles born of a line` | azok a sorok, amelyek egyet sem találtak, és a saját pozícióikon létrehozták a sajátjukat |
| `N subtitles left without a translation` | a főfájl azon feliratsorai, amelyeket egyik sor sem ért el |
| `N lines out of order` | azok a sorok, amelyek egy későbbi után következtek a fájlban |

Az a fordítás, amely egyetlen sort sem hordoz, ezt mondja: `translation: the file holds no line`. **Maga a fájl jelentése változatlanul csak a fájlé marad**: a sorból létrejött feliratsort nem számolja a `subtitles` mező.

A `-t` hibái az olvasás hibái, a fordítás elérési útjával kimondva, és ez: `<translation>: the file is already open as the main document`, ha a megadott fájl a főfájl. Az `--align-method` a `-t` nélkül használati hiba, és az is a `-t` több bemenettel — lásd [Fordítás](invocation.md#fordítás).

## Kimenet

A jelentés a **szabványos kimenetre** kerül — ez a parancs eredménye, és minden szinten megíródik, a `--quiet` kapcsolóval is. Fájlonként egy blokk, az elérési úttal kezdve:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,500\nLe canot dérive.\n\n2\n00:00:04,000 --> 00:00:06,200\nPersonne n a vu la côte.\n' > exemple.srt; subedit-cli --quiet inspect exemple.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,500\nLe canot dérive.\n\n2\n00:00:04,000 --> 00:00:06,200\nPersonne n a vu la côte.\n' > exemple.srt; subedit-cli --quiet inspect exemple.srt
exemple.srt
  format: SubRip
  encoding: UTF-8, detected
  byte order mark: absent
  line endings: LF
  subtitles: 2
  span: 00:00:01.000 -> 00:00:06.200
  frame rate grid: none (too few subtitles to tell)
  anomalies: none
```

| Mező | Mit mond |
| :---- | :----------- |
| `format` | a `--to` kapcsoló által elfogadott formátumok egyike, teljes névvel: `SubRip`, `WebVTT`, `SubViewer 2`, `Sub Station Alpha`, `Advanced SSA`, `MicroDVD`, `MPL2`, `TMPlayer`, `LRC` |
| `encoding` | az olvasott kódolás, utána **az, honnan származik a válasz**: `, from its byte order mark`, ha a fájl kijelenti, `, as asked for`, ha az `--encoding` kapcsoló írta elő, `, detected`, ha mérlegelték |
| `byte order mark` | `present` vagy `absent` |
| `line endings` | `LF`, `CRLF` vagy `CR`, utána `, mixed from line N`, ha a fájl keveri őket |
| `subtitles` | az olvasott feliratsorok száma |
| `span` | a legkorábbi kezdettől a legkésőbbi végéig, `HH:MM:SS.mmm` |
| `frame rate grid` | az a képkockasebesség, amelyen a pozíciókat számították, **kikövetkeztetve** — lásd alább |
| `frame rate` | a képkockákban számolt fájl esetén az a sebesség, amellyel olvasták, és **honnan származik** — az előző sor helyére lép, lásd alább |
| `anomalies` | `none` ugyanabban a sorban, vagy ami hibás: **fajtánként egy sor a számával**, az `anomalies:` alatt behúzva; a `--detail` kapcsolóval **anomáliánként egy sor**, feliratsoronként |

**A `span` nem „az elsőtől az utolsóig”**, hanem a legkorábbitól a legkésőbbiig: ha a fájl sorrendje felborult, a kettő eltér, és csak a második mondja meg az igazat arról, mit fed le a fájl.

**Az `anomalies` a `--detail` kapcsolóval a feliratsorokat 1-től számolja**, ahogy a fájl számozza őket — és nem a fájl sorait. A feliratsor száma túléli a módosítást; a sor száma nem. Lásd alább.

## Amit a `frame rate grid` jelez

**A feliratfájl nem jelenti ki a képkockasebességét.** A SubRipnek nincs fejléce, a WebVTT fejléce szabad szöveg. Ez a sor tehát semmit sem olvas ki: **következtet**, azzal, hogy megméri, a pozíciók képkockarácsra esnek-e.

A **kezdeteket** mérik. Ahol van rács, ott kivétel nélkül rajta vannak, a végek viszont gyakran letérnek róla — a véget gyakran olvasási sebesség szabálya számítja, nem képkockára teszik.

Az első sor mindig ugyanazt mondja: a választott sebességet, az ítéletet és az azt alátámasztó koncentrációt.

| Ítélet | Mit jelent |
| :------ | :----------------- |
| `clean` | a pozíciók rácson vannak |
| `partial` | egy részük rajta van, a többi nem |
| `none` | egyetlen jelölt sem magyaráz meg semmit |

A `none` **két okból fordulhat elő, és ezek nem ugyanazt a választ adják**: vagy egyetlen rács sem illik — a sor ilyenkor megadja a legjobb jelöltet és a pontszámát, hogy látsszon, mennyivel hibázik —, vagy a fájl túl kevés feliratsort számlál ahhoz, hogy bármit is lehessen mondani róla. Két kezdet mindig tökéletesen illeszkedőnek látszik; ez semmit sem bizonyít.

**A nem szabványos sebességű szabályos fájl `none` értéket ad, és ez szándékos.** A jelöltek halmaza zárt — az átváltás által kínált nyolc sebesség —, és a legkevésbé téves jelentése hibás választ adna ott, ahol az „nem tudom” a helyes.

### A következő sorok, ha van mondanivalójuk

| Sor | Mikor jelenik meg |
| :---- | :------------------ |
| `grid offset` | a pozíciók a rácson vannak **egy állandó eltéréssel**: a fájlt eltolták, és íme, mennyivel |
| `also fits` | egy másik jelölt ugyanúgy illik, mert a választott egész számú többszöröse |
| `too short a span to separate` | a fájl időtartama nem teszi lehetővé, hogy a választott sebességet megkülönböztessék a megnevezettektől |
| `off the grid` | hány kezdet tér le a rácsról, és hány **futamban** |

**Az `also fits` nem habozás.** A 25 képkocka/másodperces rács *benne van* az 50-esben: a 25-re időzített fájl tehát mindkettőre illik. A fordítottja nem igaz — a valódi 50 összeomlik a 25-ön —, ezért a legalacsonyabbat választják, a másikat pedig megnevezik, ahelyett hogy elhallgatnák. Az érintett párok csak a 25 és az 50, a 29,97 és az 59,94, a 30 és a 60.

**A `too short a span to separate` a túlzottan magabiztos válasz ellen véd.** A 23,976 és a 24 filmmásodpercenként egy ezredmásodperccel távolodik egymástól: egy egész estés filmen kétséget kizáróan elválnak, tíz másodperc alatt nem különböztethetők meg. A sor megmondja, melyik marad lehetséges.

**Az `off the grid` két történetet választ szét, amelyeket a százalék önmagában összekever.** Sok, egyetlen kezdetből álló futam kézzel, egyenként javított pozíció. Néhány hosszú futam újraidőzített szakasz vagy két másikból összeállított fájl. A futamok száma választja el őket.

### Példa

Tizenkét sor 25 képkocka/másodperces rácson — vagyis 40 ezredmásodperc többszöröseinek megfelelő pozíciókon —, közel kilenc percre elosztva:

<!-- example: k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > grille.srt; subedit-cli --quiet inspect grille.srt | tail -3 -->
```console
$ k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > grille.srt; subedit-cli --quiet inspect grille.srt | tail -3
  frame rate grid: 25 fps, clean (100.0%)
  also fits: 50 fps, of which this rate is a whole divisor
  anomalies: none
```

A példa maga állítja elő a fájlját, ahelyett hogy a tárolóból olvasna egyet, és ez szándékos: úgy, ahogy van, újrajátszható, és a ciklus jobban megmutatja, mi a rács, mint egy mondat — egy képkockahossz többszöröseinek megfelelő pozíciók.

## Amit a `frame rate` jelez, és miért nem rács

**A kilenc formátum közül csak egy számol képkockákban**: a MicroDVD, amelynek minden sora két képkockaszámot hordoz, és semmi mást. Ilyen fájlon az `inspect` nem a `frame rate grid`, hanem a `frame rate` sort írja, és a két sor sosem jelenik meg együtt.

**Az ok egy mondatban elmondható.** A MicroDVD-fájl pozícióit a képkockáiból számították, adott sebességgel; ha rácsból következtetnének rájuk, éppen azt a sebességet kapnák vissza, amelyet az imént megadtak. Ez megadott érték lenne, mérésnek öltöztetve.

Helyette **az olvasási sebesség és az, honnan származik** íródik ki — ugyanabban az alakban, mint az `encoding` sor:

| Mit mond a sor | Mikor |
| :------------------ | :---- |
| `frame rate: 25 fps, as asked for` | a `--frame-rate` kapcsoló írta elő |
| `frame rate: 24000/1001 fps, assumed` | senki sem mondta meg; ez az alapértelmezés |

**Két válasz, és nem három**, ahol az `encoding` háromat ismer: a MicroDVD-fájl semmit sem jelent ki. Ez az egész probléma — a sebesség szükségszerűen kívülről jön, és a jelentés megmondja, a két külső forrás közül melyikből.

**A sebesség dönti el a jelentés egész többi részét.** A `span` is rajta megy át, és ugyanazon fájl két sebességű két olvasata két időtartamot ad:

<!-- example: printf '{25}{75}Première.\n{100}{150}Deuxième.\n{200}{260}Troisième.\n' > images.sub; subedit-cli --quiet inspect images.sub | tail -3; subedit-cli --quiet inspect --frame-rate 25 images.sub | tail -3 -->
```console
$ printf '{25}{75}Première.\n{100}{150}Deuxième.\n{200}{260}Troisième.\n' > images.sub; subedit-cli --quiet inspect images.sub | tail -3; subedit-cli --quiet inspect --frame-rate 25 images.sub | tail -3
  span: 00:00:01.043 -> 00:00:10.844
  frame rate: 24000/1001 fps, assumed
  anomalies: none
  span: 00:00:01.000 -> 00:00:10.400
  frame rate: 25 fps, as asked for
  anomalies: none
```

## Amit az `anomalies` jelez

A dokumentumban három dolog lehet hibás, és mindegyiket másképp javítják. **Alapértelmezés szerint az `inspect` megszámolja őket**, fajtánként egy sor, a táblázat sorrendjében: a rosszul készült fájl százával hordozza őket, és az dönti el, mit kell futtatni, hogy fajtánként hányról van szó. **A `--detail` egyenként megnevezi őket.**

| Ami kiíródik | A `--detail` kapcsolóval | Mit jelent |
| :--------------- | :-------------- | :------------------ |
| `subtitle ends before it starts: N` | `subtitle N ends before it starts` | a vége a kezdete előtt van |
| `subtitle starts before the previous one ends: N` | `subtitle N starts before the previous one ends` | átfedi az előtte lévőt |
| `subtitle starts before the previous one starts: N` | `subtitle N starts before the previous one starts` | felborítja a fájl sorrendjét — a [`sort`](sort.md) helyreállítja |

A hiányzó fajta nem íródik ki. A bal oldali oszlopban az `N` a feliratsorok száma; a következőben egy feliratsor száma.

**Ugyanaz a feliratsor kétszer is megjelenhet**, és ez szándékos: az, amelyik azelőtt kezdődik, hogy az előző elkezdődött volna, azelőtt is kezdődik, hogy az véget ért volna. Az elsőt a feliratsor mozgatása, a másodikat az időtartam módosítása oldja meg.

**A megnevezett feliratsor az, amelyik rossz helyen van**, nem az, amelyikhez képest. A `0`, `4 s`, `2 s`, `3 s` kezdeteknél: a harmadik töri meg a sorrendet, a negyedik követi a harmadikat, ezért nem kerül megnevezésre — vele nincs teendő.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect desordre.srt | tail -3 -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect desordre.srt | tail -3
  anomalies:
    subtitle starts before the previous one ends: 1
    subtitle starts before the previous one starts: 1
```

És a `--detail` kapcsolóval a megnevezett feliratsor:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect --detail desordre.srt | tail -3 -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect --detail desordre.srt | tail -3
  anomalies:
    subtitle 3 starts before the previous one ends
    subtitle 3 starts before the previous one starts
```

**A `--detail` kapcsolóval anomáliánként egy sor**, mert egy sorba zsúfolt száz megállapítás sem olvasható, sem kereshető. A hibátlan dokumentum megtartja az egyetlen sorát, az `anomalies: none` sort, amelyet a szkript keres. **A `--format json` rekord mindig felsorolja az egyes anomáliákat**: a szkript a részletet olvassa, és a `--detail` csak a szöveget változtatja.

**Ez nem olvasási diagnosztika.** A diagnosztika azt mondja meg, mibe ütközött az olvasás, és a **fájl egy sorára** mutat; az anomáliák azt mondják meg, mi *a dokumentum*, és egy **feliratsorra** mutatnak. Az a fájl, amelynek a sorrendje felborult, ettől még nem hibás szerkezetű: az olvasásnak nincs mit mondania róla.

## JSON-ban

A [`--format json`](invocation.md#szkript-által-olvasható-kimenet) kapcsolóval az `inspect` a fenti blokkok helyett **fájlonként egy objektumot** ír; a beszámoló a hibakimeneten marad. A kulcsok azt írják le, amit a szöveg mond, és **egyiket sem szabad mondatból visszaolvasni**:

| Kulcs | Tartalom |
| :-- | :------ |
| `format` | a formátum azonosítója, az, amelyet a `convert --to` elfogad: `srt`, `vtt`… |
| `encoding` | `{"charset", "origin", "byte_order_mark"}` — az `origin` értéke `byte-order-mark`, `asked` vagy `detected` |
| `line_endings` | `{"kind", "mixed_from_line"}` — `lf`, `crlf` vagy `cr`; az a sor, ahol egy második fajta megjelenik, vagy `null` |
| `subtitles` | a feliratsorok száma |
| `span_ms` | `{"start", "end"}` — az első kezdet és az utolsó vég, ezredmásodpercben |
| `frame_rate` | a képkockákban számolt fájl esetén: `{"rate", "origin"}` (`asked` vagy `assumed`); egyébként `null` |
| `grid` | a többi esetén: a kikövetkeztetett rács, vagy `null` a képkockákban számolt fájl esetén |
| `anomalies` | `{"subtitle", "kind"}` elemek tömbje — `end-before-start`, `overlapping-subtitles`, `out-of-order` |
| `translation` | **csak a `-t` kapcsolóval**: `{"file", "method", "attached", "born", "untranslated", "out_of_order"}` — a `method` értéke `position` vagy `number`, a négy számláló pedig a fenti mondaté |
| `warnings` | amibe az olvasás ütközött, minden szinten |

A `grid` tartalmazza a `verdict` (`clean`, `partial` vagy `silent`), az `enough_starts`, a `rate` (szöveg, vagy `null`, ha semmit sem neveznek meg), a `concentration_permille` (ezrelék, `null` elég kezdet nélkül), az `offset_ms`, az `also_fits` (az a sebesség, amelynek ez egész osztója, vagy `null`), a `not_separated` (azok a sebességek, amelyeket a túl rövid időtartam nem választ szét), a `strays` (a rácson kívüli kezdetek) és a `starts` mezőt.

<!-- example: printf '{25}{50}Un.\n{75}{100}Deux.\n' > f.sub; subedit-cli --format json inspect --frame-rate 25 f.sub 2>/dev/null -->
```console
$ printf '{25}{50}Un.\n{75}{100}Deux.\n' > f.sub; subedit-cli --format json inspect --frame-rate 25 f.sub 2>/dev/null
{"schema":1,"command":"inspect","file":"f.sub","ok":true,"format":"microdvd","encoding":{"charset":"UTF-8","origin":"detected","byte_order_mark":false},"line_endings":{"kind":"lf","mixed_from_line":null},"subtitles":2,"span_ms":{"start":1000,"end":4000},"frame_rate":{"rate":"25","origin":"asked"},"grid":null,"anomalies":[],"warnings":[]}
```

## Beszámoló

A hibakimeneten, a kért szinttől függően:

| Szint | Mi kerül hozzá |
| :----- | :-------------- |
| 1 | `<path>: N subtitles` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings` |
| 3 | `<path>: N bytes read`, `<path>: N diagnostic(s) while reading`, majd **mindegyikük** — lásd [Indítás](invocation.md#olvasási-diagnosztika) |

## Kilépési kódok

Az eszköz kódjai: `0`, ha minden fájl beolvasása megtörtént, `2`, ha egyé sem, `3`, ha csak egyeseké, `1` használati hiba esetén.

## Hibák

Az olvasás hibái — a fájlé, és a fordításé, ha a `-t` meg van adva —, valamint a `-t` hibái: lásd fent. Az `inspect` semmit sem ír, ami elutasítható lenne.

| Mi váltja ki | Üzenet |
| :------------------ | :------ |
| nincs megadott elérési út | `files is required`, utána a `--help` kapcsolóra utaló megjegyzés |
| hiányzó fájl | `<path>: does not exist` |
| olvashatatlan fájl | `<path>: cannot be opened: permission denied` |
| nem dekódolható bájtok | `<path>: cannot be decoded in the chosen encoding` |
| fel nem ismert formátum | `<path>: is in no format this tool knows` |
| semmi, ami feliratsorra hasonlítana | `<path>: holds nothing recognisable as a subtitle` |

Ezek ugyanazok az üzenetek mind a tizenhét alparancsnál: a megnyitás receptje egyszer van megírva, a magban.

## Példa, amely hibázik

Az üres fájl egyetlen formátumnak sem felel meg. Nem üresként jelentik: elutasításra kerül, és megnevezik.

<!-- example: : > vide.srt; subedit-cli inspect vide.srt; echo "code=$?" -->
```console
$ : > vide.srt; subedit-cli inspect vide.srt; echo "code=$?"
vide.srt: is in no format this tool knows
code=2
```
