# `inspect`

<!-- machine-translated -->
> Tato stránka je strojovým překladem a dosud ji nezkontroloval žádný čtenář tohoto jazyka.

```
subedit-cli inspect [--frame-rate RATE] [--recursive] [--detail]
                   [-t FILE] [--align-method position|number] <file>...
```

Hlásí, z čeho se každý soubor skládá. **Nic nemění a nezapisuje žádný
soubor.**

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

## Argumenty

| Argument | Povinný | Hodnota | Výchozí |
| :------- | :----- | :----- | :----- |
| `<file>...` | ano | jedna nebo více cest k souborům titulků | — |
| `--recursive`, `-r` | ne | přepínač | vypnuto — viz [Zpracování stromu](lots.md) |
| `--detail` | ne | přepínač: pojmenuje každou anomálii, místo aby je počítal — viz [Co hlásí `anomalies`](#co-hlásí-anomalies) | vypnuto |

Žádná cesta je chybou použití, tedy kód `1`.

## Volby

| Volba | Povinná | Hodnota | Výchozí |
| :----- | :------ | :----- | :----- |
| `--frame-rate` | ne | frekvence ve snímcích za sekundu: `25`, `23.976`, `29.97` | 23,976 |

**Slouží jen souborům počítaným ve snímcích**, tedy formátu MicroDVD, jedinému
z devíti. Soubor MicroDVD nenese nic než čísla snímků;
frekvence, kterou se čtou, rozhoduje o **každé pozici zprávy**,
počínaje `span`. Zadaná jinde nic nemění a není to
chyba.

Hodnota, která nic neoznačuje, je chybou použití, tedy kód `1`:

<!-- example: subedit-cli inspect --frame-rate douze film.sub; echo $? -->
```console
$ subedit-cli inspect --frame-rate douze film.sub; echo $?
--frame-rate: "douze" is not a frame rate: expected frames per second, like 25 or 23.976
1
```

Je to tatáž volba jako u podpříkazu [`convert`](convert.md), slovo od slova, protože
jde o tutéž frekvenci. `convert` ji navíc používá k **zápisu**.

## Překlad: `-t`

| Volba | Povinná | Hodnota | Výchozí |
| :----- | :------ | :----- | :----- |
| `-t`, `--translation-file` | ne | soubor překladu, pro **jediný** vstup | — |
| `--align-method` | ne, a jen s `-t` | `position` nebo `number` | `position` |

S `-t` se překlad položí na soubor **stejně, jako to dělá okno při jeho otevření** — viz
[Překlad](invocation.md#překlad) — a zpráva říká, **jak jeho řádky našly svůj
titulek**. Skript tak ví, zda je překlad zarovnaný, dříve než ho opraví.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour.\n\n' > film.fr.srt; subedit-cli --quiet inspect -t film.fr.srt film.srt | tail -3 -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour.\n\n' > film.fr.srt; subedit-cli --quiet inspect -t film.fr.srt film.srt | tail -3
  anomalies: none
  translation file: film.fr.srt, matched by position
  translation: 1 line attached; 1 subtitle left without a translation
```

První přidaný řádek uvádí soubor a metodu; druhý je **věta, kterou okno
říká**, když otevírá překlad, každá část jen tehdy, když není nulová:

| Část | Co počítá |
| :----- | :---------------- |
| `N lines attached` | řádky překladu, které našly titulek hlavního souboru |
| `N subtitles born of a line` | řádky, které žádný nenašly a daly vzniknout vlastnímu, na vlastních pozicích |
| `N subtitles left without a translation` | titulky hlavního souboru, ke kterým žádný řádek nedošel |
| `N lines out of order` | řádky, které v souboru přišly až po pozdějším |

Překlad, který nenese žádný řádek, hlásí `translation: the file holds no line`. **Zpráva o
samotném souboru zůstává zprávou o souboru samotném**: titulek vzniklý z řádku se nezapočítává do
`subtitles`.

Chyby `-t` jsou chyby čtení, hlášené s cestou k překladu, a tato:
`<translation>: the file is already open as the main document`, když je zadaný soubor hlavním souborem.
`--align-method` bez `-t` je chybou použití, stejně jako `-t` s více vstupy — viz
[Překlad](invocation.md#překlad).

## Výstup

Zpráva jde na **standardní výstup** — je výsledkem příkazu a
zapisuje se na každé úrovni, včetně `--quiet`. Jeden blok na soubor, uvozený
cestou:

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

| Pole | Co říká |
| :---- | :----------- |
| `format` | jeden z formátů, které přijímá `--to`, vypsaný celý: `SubRip`, `WebVTT`, `SubViewer 2`, `Sub Station Alpha`, `Advanced SSA`, `MicroDVD`, `MPL2`, `TMPlayer`, `LRC` |
| `encoding` | přečtené kódování, za nímž následuje, **odkud odpověď pochází**: `, from its byte order mark`, pokud ho soubor deklaruje, `, as asked for`, pokud ho vynutilo `--encoding`, `, detected`, pokud bylo odhadnuto |
| `byte order mark` | `present` nebo `absent` |
| `line endings` | `LF`, `CRLF` nebo `CR`, za nímž následuje `, mixed from line N`, pokud je soubor míchá |
| `subtitles` | počet přečtených titulků |
| `span` | od nejdřívějšího začátku po nejpozdější konec, `HH:MM:SS.mmm` |
| `frame rate grid` | snímková frekvence, na které byly pozice vypočteny, **odvozená** — viz níže |
| `frame rate` | u souboru počítaného ve snímcích frekvence, kterou byl přečten, a **odkud pochází** — nahrazuje předchozí řádek, viz níže |
| `anomalies` | `none` na témže řádku, nebo co je špatně: **jeden druh na řádek s jeho počtem**, odsazený pod `anomalies:`; s `--detail` **jedna anomálie na řádek**, titulek po titulku |

**`span` neznamená „od prvního po poslední“**, ale od nejdřívějšího po nejpozdější:
u souboru s porušeným pořadím se obojí liší a jen to druhé říká
pravdu o tom, co soubor pokrývá.

**`anomalies` s `--detail` čísluje titulky od 1**, tak jak je soubor
čísluje — a ne řádky souboru. Číslo titulku přežije
změnu; číslo řádku ne. Viz níže.

## Co hlásí `frame rate grid`

**Soubor titulků svou snímkovou frekvenci nedeklaruje.** SubRip nemá žádnou
hlavičku, hlavička WebVTT je volný text. Tento řádek tedy nic nečte:
**odvozuje**, a to měřením, zda pozice padají na mřížku snímků.

Měří se **začátky**. Všude, kde mřížka existuje, na ní leží
bez výjimky, kdežto konce z ní často vybočují — konec se
často počítá pravidlem rychlosti čtení, místo aby se položil na
snímek.

První řádek říká vždy totéž: zvolenou frekvenci, verdikt
a koncentraci, která ho podporuje.

| Verdikt | Co znamená |
| :------ | :----------------- |
| `clean` | pozice leží na mřížce |
| `partial` | část z nich ano, zbytek ne |
| `none` | žádný kandidát nic nevysvětluje |

`none` má **dvě příčiny, a nejde o stejnou odpověď**: buď žádná
mřížka nesedí — řádek pak uvede nejlepšího kandidáta a jeho skóre,
aby bylo vidět, o kolik selhává — nebo soubor obsahuje příliš málo
titulků na to, aby se o něm dalo něco říct. Dva začátky vypadají vždy
dokonale zarovnané; to nic nedokazuje.

**Pravidelný soubor na nestandardní frekvenci dává `none`, a je to
záměr.** Množina kandidátů je uzavřená — osm frekvencí, které
převod nabízí — a hlásit nejméně chybnou z nich by znamenalo dát
chybnou odpověď tam, kde správná je „nevím“.

### Řádky, které následují, když mají co říct

| Řádek | Kdy se objeví |
| :---- | :------------------ |
| `grid offset` | pozice leží na mřížce **až na konstantu**: soubor byl posunut, a tady je o kolik |
| `also fits` | jiný kandidát sedí stejně dobře, protože je celým násobkem zvoleného |
| `too short a span to separate` | rozpětí souboru nedovoluje odlišit zvolenou frekvenci od uvedených |
| `off the grid` | kolik začátků vybočuje z mřížky, a v kolika **sériích** |

**`also fits` není váhání.** Mřížka 25 snímků za sekundu je
*obsažena* v mřížce 50: soubor časovaný na 25 tedy sedí na obě.
Opak neplatí — skutečných 50 se na 25 zhroutí — proto se volí ta
nižší a ta druhá se uvede, místo aby se zamlčela. Jediné
dotčené dvojice jsou 25 a 50, 29,97 a 59,94, 30 a 60.

**`too short a span to separate` chrání před příliš sebejistou odpovědí.** 23,976
a 24 se od sebe vzdalují o jednu milisekundu za sekundu filmu: u celovečerního filmu se
oddělí bez pochyb, na deseti sekundách je nelze rozlišit. Řádek říká,
která zůstává možná.

**`off the grid` rozlišuje dva příběhy, které samotné procento plete.**
Mnoho sérií po jediném začátku jsou pozice opravené ručně,
jedna po druhé. Několik dlouhých sérií je přečasovaný úsek nebo soubor
složený ze dvou jiných. Počet sérií je to, co je odlišuje.

### Příklad

Dvanáct replik položených na mřížku 25 snímků za sekundu — tedy na
pozice, které jsou násobky 40 milisekund — rozložených na téměř devět minut:

<!-- example: k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > grille.srt; subedit-cli --quiet inspect grille.srt | tail -3 -->
```console
$ k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > grille.srt; subedit-cli --quiet inspect grille.srt | tail -3
  frame rate grid: 25 fps, clean (100.0%)
  also fits: 50 fps, of which this rate is a whole divisor
  anomalies: none
```

Příklad si vyrábí vlastní soubor, místo aby jeden četl z repozitáře, a je to
záměrné: lze ho přehrát tak, jak je, a smyčka ukazuje, co je mřížka,
lépe než věta — pozice, které jsou násobky doby trvání snímku.

## Co hlásí `frame rate` a proč to není mřížka

**Jen jeden z devíti formátů počítá ve snímcích**: MicroDVD, jehož každý řádek
nese dvě čísla snímků a nic jiného. U takového souboru `inspect`
nezapíše `frame rate grid`, ale `frame rate`, a oba řádky se
nikdy nezobrazí společně.

**Důvod se vejde do jedné věty.** Pozice souboru MicroDVD byly
vypočteny *z* jeho snímků při dané frekvenci; odvozovat je z
mřížky by vrátilo právě tuto frekvenci, tak jak byla právě dodána. Bylo by to
zadané číslo převlečené za měření.

Místo toho se zapíše **frekvence čtení a odkud pochází** —
ve stejném tvaru jako řádek `encoding`:

| Co řádek říká | Kdy |
| :------------------ | :---- |
| `frame rate: 25 fps, as asked for` | vynutilo ji `--frame-rate` |
| `frame rate: 24000/1001 fps, assumed` | nikdo ji neuvedl; je to výchozí hodnota |

**Dvě odpovědi, a ne tři**, kde `encoding` má tři: soubor MicroDVD
nedeklaruje nic. V tom je celý problém — frekvence nutně pochází
zvenčí a zpráva říká, z kterého ze dvou „zvenčí“.

**Frekvence rozhoduje o celém zbytku zprávy.** `span` jí prochází a dvě
čtení téhož souboru při dvou frekvencích dávají dvě rozpětí:

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

## Co hlásí `anomalies`

V dokumentu mohou být špatně tři věci a každá se opravuje jinak.
**Ve výchozím stavu je `inspect` počítá**, jeden řádek na druh, v pořadí tabulky:
špatně vyrobený soubor jich nese stovky a vědět, kolik je kterého druhu,
je to, co říká, co spustit. **`--detail` je pojmenuje jednu po druhé.**

| Co se zapíše | S `--detail` | Co znamená |
| :--------------- | :-------------- | :------------------ |
| `subtitle ends before it starts: N` | `subtitle N ends before it starts` | konec přichází před začátkem |
| `subtitle starts before the previous one ends: N` | `subtitle N starts before the previous one ends` | překrývá se s předchozím |
| `subtitle starts before the previous one starts: N` | `subtitle N starts before the previous one starts` | porušuje pořadí souboru — [`sort`](sort.md) ho vrátí na místo |

Druh, který chybí, se nezapíše. V levém sloupci je `N` počet
titulků; v dalším číslo titulku.

**Tentýž titulek se může objevit dvakrát**, a je to záměr: titulek, který
začíná dříve, než začal předchozí, začíná také dříve, než předchozí
skončil. První bod se řeší přesunutím titulku, druhý
změnou doby trvání.

**Uvedený titulek je ten, který je na špatném místě**, ne ten, vůči kterému
je na špatném místě. U začátků `0`, `4 s`, `2 s`, `3 s`: třetí porušuje pořadí,
čtvrtý třetí opravdu následuje, a proto se neuvádí — není s ním co
dělat.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect desordre.srt | tail -3 -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect desordre.srt | tail -3
  anomalies:
    subtitle starts before the previous one ends: 1
    subtitle starts before the previous one starts: 1
```

A s `--detail` uvedený titulek:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect --detail desordre.srt | tail -3 -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect --detail desordre.srt | tail -3
  anomalies:
    subtitle 3 starts before the previous one ends
    subtitle 3 starts before the previous one starts
```

**S `--detail` jeden řádek na anomálii**, protože sto zjištění
nalepených na jednom řádku nelze ani přečíst, ani prohledat. Dokument bez vady si ponechá
svůj jediný řádek, `anomalies: none`, který skript hledá. **Záznam
`--format json` vždy vypisuje každou anomálii**: skript čte podrobnosti a
`--detail` mění jen text.

**Nejde o hlášení při čtení.** Hlášení říkají, na co
čtení narazilo, a ukazují na **řádek souboru**; anomálie říkají,
čím dokument *je*, a ukazují na **titulek**. Soubor s porušeným
pořadím tím ještě není chybně utvořený: čtení k němu nemá co říct.

## Ve formátu JSON

S [`--format json`](invocation.md#výstup-čitelný-pro-skript) zapíše `inspect` **jeden
objekt na soubor** místo výše uvedených bloků; výpis průběhu zůstává na chybovém výstupu.
Klíče popisují to, co říká text, a **žádný se nemusí zpětně vyčítat z věty**:

| Klíč | Obsah |
| :-- | :------ |
| `format` | identifikátor formátu, ten, který přijímá `convert --to`: `srt`, `vtt`… |
| `encoding` | `{"charset", "origin", "byte_order_mark"}` — `origin` je `byte-order-mark`, `asked` nebo `detected` |
| `line_endings` | `{"kind", "mixed_from_line"}` — `lf`, `crlf` nebo `cr`; řádek, kde se objeví druhý druh, nebo `null` |
| `subtitles` | počet titulků |
| `span_ms` | `{"start", "end"}` — první začátek a poslední konec, v milisekundách |
| `frame_rate` | u souboru počítaného ve snímcích: `{"rate", "origin"}` (`asked` nebo `assumed`); jinak `null` |
| `grid` | u ostatních: odvozená mřížka, nebo `null` u souboru počítaného ve snímcích |
| `anomalies` | pole `{"subtitle", "kind"}` — `end-before-start`, `overlapping-subtitles`, `out-of-order` |
| `translation` | **jen s `-t`**: `{"file", "method", "attached", "born", "untranslated", "out_of_order"}` — `method` je `position` nebo `number` a čtyři počty jsou počty z výše uvedené věty |
| `warnings` | na co čtení narazilo, na každé úrovni |

`grid` nese `verdict` (`clean`, `partial` nebo `silent`), `enough_starts`, `rate` (řetězec,
nebo `null`, když se nic neuvádí), `concentration_permille` (tisíciny, `null` bez dostatku
začátků), `offset_ms`, `also_fits` (frekvence, jejímž celým dělitelem tato je, nebo
`null`), `not_separated` (frekvence, které příliš krátké rozpětí neoddělí), `strays`
(začátky mimo mřížku) a `starts`.

<!-- example: printf '{25}{50}Un.\n{75}{100}Deux.\n' > f.sub; subedit-cli --format json inspect --frame-rate 25 f.sub 2>/dev/null -->
```console
$ printf '{25}{50}Un.\n{75}{100}Deux.\n' > f.sub; subedit-cli --format json inspect --frame-rate 25 f.sub 2>/dev/null
{"schema":1,"command":"inspect","file":"f.sub","ok":true,"format":"microdvd","encoding":{"charset":"UTF-8","origin":"detected","byte_order_mark":false},"line_endings":{"kind":"lf","mixed_from_line":null},"subtitles":2,"span_ms":{"start":1000,"end":4000},"frame_rate":{"rate":"25","origin":"asked"},"grid":null,"anomalies":[],"warnings":[]}
```

## Výpis průběhu

Na chybovém výstupu, podle požadované úrovně:

| Úroveň | Co se přidá |
| :----- | :-------------- |
| 1 | `<path>: N subtitles` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings` |
| 3 | `<path>: N bytes read`, `<path>: N diagnostic(s) while reading`, pak **každé z nich** — viz [Spuštění](invocation.md#hlášení-při-čtení) |

## Návratové kódy

Kódy nástroje: `0`, pokud byly přečteny všechny soubory, `2`, pokud žádný, `3`, pokud
jen některé, `1` při chybě použití.

## Chyby

Chyby čtení — souboru, a překladu, když je zadáno `-t` —, a
chyby `-t`: viz výše. `inspect` nezapisuje nic, co by mohlo být odmítnuto.

| Co to způsobí | Hlášení |
| :------------------ | :------ |
| není zadána žádná cesta | `files is required`, následované odkazem na `--help` |
| chybějící soubor | `<path>: does not exist` |
| nečitelný soubor | `<path>: cannot be opened: permission denied` |
| bajty, které nelze dekódovat | `<path>: cannot be decoded in the chosen encoding` |
| nerozpoznaný formát | `<path>: is in no format this tool knows` |
| nic, co by vypadalo jako titulek | `<path>: holds nothing recognisable as a subtitle` |

Pro všech sedmnáct podpříkazů jsou to stejná hlášení: postup otevírání
je zapsán jednou, v jádře.

## Příklad, který selže

Prázdný soubor neodpovídá žádnému formátu. Nehlásí se jako prázdný:
odmítne se a pojmenuje.

<!-- example: : > vide.srt; subedit-cli inspect vide.srt; echo "code=$?" -->
```console
$ : > vide.srt; subedit-cli inspect vide.srt; echo "code=$?"
vide.srt: is in no format this tool knows
code=2
```
