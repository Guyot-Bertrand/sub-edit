# `convert`

<!-- machine-translated -->
> Tato stránka je strojovým překladem a dosud ji nezkontroloval žádný čtenář tohoto jazyka.

```
subedit-cli convert --to <format>
                    [--line-endings unix|windows|mac] [--to-encoding NAME]
                    [--bom | --no-bom] [--sort] [--disjoint]
                    [--join-repeats] [--repair]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    <file>...
```

Zapíše každý soubor v požadovaném formátu a tvaru. **Vstup nikdy
nemění**, pokud o to výslovně nežádá `--in-place`.

<!-- example: subedit-cli convert --help -->
```console
$ subedit-cli convert --help
Write a subtitle file out in another format or shape
Usage: subedit-cli convert [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to convert

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --to TEXT:{srt,vtt,subviewer2,ssa,ass,microdvd,mpl2,tmplayer,lrc} REQUIRED
                              Format to write
  --line-endings TEXT:{unix,windows,mac}
                              Line endings to write; the source's by default
  --to-encoding NAME          Encoding to write; the source's by default
  --frame-rate RATE           Frame rate of a file counted in frames: 25, 23.976
  --bom                       Write a byte order mark
  --no-bom                    Write no byte order mark
  --sort                      Put the subtitles in order of their start before writing
  --disjoint                  Cut overlapping subtitles so that no range overlaps another, before writing
  --join-repeats              Join the subtitles that repeat each other into one, before writing
  --repair                    Join the repeats, sort, then make the ranges disjoint, before writing
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Argumenty a volby

| Volba | Povinná | Hodnoty | Výchozí |
| :----- | :----- | :------ | :----- |
| `<file>...` | ano | jedna nebo více cest | — |
| `--recursive`, `-r` | ne | přepínač | vypnuto — viz [Zpracování stromu](lots.md) |
| `--to` | **ano** | název z tabulky níže, a nic jiného | — |
| `--line-endings` | ne | `unix`, `windows` nebo `mac` | ty přečteného souboru |
| `--to-encoding` | ne | jakékoli kódování, které ICU umí zapsat, kromě těch, která zapisují vlastní značku | kódování přečteného souboru |
| `--bom` / `--no-bom` | ne | přepínače, které se navzájem vylučují | to, co nesl přečtený soubor |
| `--sort` | ne | přepínač: seřadí titulky podle jejich začátku **před zápisem**, viz [Řazení při zápisu](#řazení-při-zápisu) | vypnuto |
| `--join-repeats` | ne | přepínač: z po sobě jdoucích částí jednoho textu udělá jediný titulek, **před vším ostatním**, viz [Převedení rozsahů na disjunktní](#převedení-rozsahů-na-disjunktní) | vypnuto |
| `--repair` | ne | přepínač: totéž co `--join-repeats --sort --disjoint`, v tomto pořadí | vypnuto |
| `--disjoint` | ne | přepínač: rozdělí překrývající se titulky **před zápisem**, viz [Převedení rozsahů na disjunktní](#převedení-rozsahů-na-disjunktní) | vypnuto |
| `--output` / `--output-dir` / `--in-place` | **jedna ze tří**, kromě `--dry-run` | viz [Spuštění](invocation.md#cíl-zápisu) | — |
| `--dry-run` | ne | přepínač: spočítá a vypíše, nic nezapíše — viz [Náhled před zápisem](invocation.md#náhled-před-zápisem) | vypnuto |

`mac` označuje samotný návrat vozíku (`\r`), konec řádku klasického systému Mac OS.

## Formáty, které přijímá `--to`

| Název | Formát | Zapisovaná přípona |
| :-- | :----- | :--------------- |
| `srt` | SubRip | `.srt` |
| `vtt` | WebVTT | `.vtt` |
| `subviewer2` | SubViewer 2.0 | `.sub` |
| `ssa` | Sub Station Alpha | `.ssa` |
| `ass` | Advanced SSA | `.ass` |
| `mpl2` | MPL2 | `.txt` |
| `microdvd` | MicroDVD | `.sub` |
| `tmplayer` | TMPlayer | `.txt` |
| `lrc` | LRC | `.lrc` |

**Název, a ne přípona.** Dvě přípony označují každá dva
formáty — `.sub` patří také formátu MicroDVD, `.txt` formátům MPL2 a
TMPlayer — takže volba, která by přijímala přípony, by nevěděla, který
se požaduje. Výše uvedené názvy jsou jednoznačné a shodují se s
příponou všude, kde je jednoznačná i přípona.

**Seznam je uzavřený.** Je to seznam Gaupolu a je v něm všech devět.

## Dva formáty, které nenesou konec

**TMPlayer a LRC zapisují jen jednu pozici na řádek**, jeho začátek. To
je vidět na obou stranách.

**Při čtení** se konce odvozují: konec řádku je začátkem
následujícího a poslední dostane pět sekund. Tak to dělá Gaupol.
Říkáme to také, hlášením, které `--verbose` zobrazí u tří
podpříkazů:

```
file.lrc: carries no end times; each one was taken from the next start, settled by the reader
```

**Při zápisu** se konce nezapisují, protože pro ně není místo.
Převod do jednoho z těchto dvou formátů je tedy ztratí a zpáteční cesta je
znovu nenajde — znovu je odvodí.

**LRC ztrácí ještě jednu věc**: nemá jak nést zalomení řádku.
Replika na dvou řádcích se zapíše na jeden, oba řádky spojené
mezerou. Ani tuto ztrátu nelze napravit.

Ani jeden z nich nemá slovník pro kurzívu.

## `--frame-rate` a jediný formát, který počítá ve snímcích

**MicroDVD nepočítá v sekundách**: jeho pozice *jsou* čísla
snímků a žádný soubor MicroDVD nedeklaruje frekvenci, při které byly
počítány. `--frame-rate` ji zadává a slouží na obou stranách: je to
frekvence, při které se čte `.sub` ve snímcích, a ta, při které
převod do MicroDVD počítá snímky.

| Volba | Povinná | Hodnoty | Výchozí |
| :----- | :----- | :------ | :----- |
| `--frame-rate` | ne | frekvence ve snímcích za sekundu: `25`, `23.976`, `29.97` | viz níže |

**Při čtení bez `--frame-rate` se soubor čte při 23,976** — hodnotě
Gaupolu — a úroveň `-vvv` to říká:
`counts in frames and states no rate; it was read at`. Je to jediné místo v nástroji, kde každá zobrazená pozice
spočívá na předpokladu, který soubor nemůže potvrdit.

**Při zápisu bez `--frame-rate` tři případy a tři odpovědi:**

| Výchozí soubor | Co se stane |
| :------------------- | :-------------- |
| už je MicroDVD | frekvence, která ho přečetla, ho zapíše zpět a snímky se vrátí shodné |
| je založený na čase a jeho pozice padají na mřížku | převezme se mřížka a `-vv` řekne, která |
| je založený na čase, bez mřížky | **odmítnutí**, kód 2: `writing frames needs a frame rate, and the positions fall on no grid to take one from — give --frame-rate` |

Odmítnutí je záměrné. Jediné, co by zbývalo, by bylo vymyslet si číslo, a
každý řádek souboru by se posunul o tolik, o kolik by bylo chybné.

## Co převod ztrácí

**Převod do jiného formátu nikdy není bezeztrátový**, a co nepřežije,
závisí na dvojici:

| Co neprojde | Proč |
| :--------------------- | :------- |
| hlavička | dává smysl jen ve svém vlastním formátu: `[INFORMATION]` není hlavička WebVTT |
| data specifická pro formát | souřadnice SubRip, identifikátor a nastavení podnětu WebVTT nemají jinde obdobu |
| přesnost | SubViewer 2, obě Sub Station Alpha a LRC zapisují na setiny, MPL2 na desetiny, TMPlayer na sekundy, MicroDVD na snímky; `01:00:00,017` se vrátí jako `01:00:00,020` |
| konec řádku | TMPlayer a LRC pro něj nemají pole; další čtení ho znovu odvodí |
| zalomení řádků | jen LRC: jeho repliky se vejdou na jeden řádek a oba řádky se znovu slepí mezerou |
| značky, které cílový formát neumí zapsat | WebVTT nemá `<font color>`, TMPlayer a LRC nemají vůbec nic |
| rozvržení | `{\pos(x,y)}`, `{\an8}`, `<v Marie>`, `<ruby>`: přežijí zpáteční cestu ve vlastním formátu a při převodu zmizí |

**Zápis souboru zpět v jeho vlastním formátu neztrácí nic** — to je záruka,
na které stojí všechno ostatní, a `--to srt` na `.srt` vrátí tytéž bajty. Nic
se ani nedekóduje: text zůstává surovým řetězcem, který byl přečten.

## Značky se překládají, nekopírují

**Každý formát zapisuje kurzívu po svém** a převod bez překladu
by v `.srt` ponechal `{\i1}`: text, který uživatel vidí, v
souboru, který ho nikdy nebude umět vyložit. Projde **šest
věcí** — tučné písmo, kurzíva, podtržení, barva, písmo a velikost:

| Formát | Tučné | Kurzíva | Podtržení | Barva | Písmo | Velikost |
| :----- | :--: | :------: | :------: | :-----: | :----: | :----: |
| SubRip | `<b>` | `<i>` | `<u>` | `<font color="#RRGGBB">` | — | — |
| WebVTT | `<b>` | `<i>` | `<u>` | — | — | — |
| SubViewer 2 | `<b>` | `<i>` | `<u>` | `<font color="#RRGGBB">` | — | — |
| Sub Station Alpha | `{\b1}` | `{\i1}` | — | `{\c&HBBGGRR&}` | `{\fnNAME}` | `{\fsN}` |
| Advanced SSA | `{\b1}` | `{\i1}` | `{\u1}` | `{\c&HBBGGRR&}` | `{\fnNAME}` | `{\fsN}` |
| MicroDVD | `{Y:b}` | `{Y:i}` | `{Y:u}` | `{C:$BBGGRR}` | `{F:NAME}` | `{S:N}` |
| MPL2 | `\` | `/` | `_` | `{C:$BBGGRR}` | `{F:NAME}` | `{S:N}` |
| TMPlayer | — | — | — | — | — | — |
| LRC | — | — | — | — | — | — |

**Pomlčka je ohlášená ztráta.** Převod barevného `.srt` do WebVTT odstraní
barvu a řekne to; vytvořený soubor je WebVTT, ne převlečený SubRip.

**MicroDVD a MPL2 nedokážou stylovat část řádku.** Jejich značky
nemají konec: `{y:i}` nakloní všechno, co následuje, až do konce řádku, a
`{Y:i}` až do konce titulku. Řádek, který byl jen z poloviny
tučný, se proto zapíše úplně bez tučného písma, místo s tučným písmem, které přetéká.

## Co převod hlásí, že ztratil

**Hlásí to, když ztrácí, a mlčí, když neztrácí nic.** Hlášení,
které se objeví při každém volání, je hlášení, které nikdo nečte.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,500\n<i>sur deux</i>\nlignes\n\n' > a.srt; subedit-cli convert --to lrc --output b.lrc a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,500\n<i>sur deux</i>\nlignes\n\n' > a.srt; subedit-cli convert --to lrc --output b.lrc a.srt
a.srt: 1 subtitle written as LRC -> b.lrc
a.srt: ends are not carried by LRC, line breaks were joined in 1 subtitle, 1 tag dropped
```

Řádek se objeví na výchozí úrovni výpisu a na každé vyšší.
Položky, v tomto pořadí:

| Položka | Co říká |
| :---- | :----------- |
| `ends are not carried by <format>` | cílový formát nezapisuje žádný konec |
| `line breaks were joined in N subtitles` | jen LRC: N titulků bylo na více řádcích |
| `N tags dropped` | N značek nebylo možné zapsat, včetně rozvržení |
| `the header was dropped` | hlavička nepřekročí hranici formátu |
| `the <format> fields of N subtitles were dropped` | N titulků neslo data specifická pro svůj formát |
| `positions moved by up to N ms` | největší vzdálenost, o kterou se pozice posunula |

## Změna kódování

**`--to-encoding` volí zapisované kódování; `--encoding` uvádí to, které se
čte.** Obě společně soubor překódují:

```console
$ subedit-cli convert --to srt --encoding cp1252 --to-encoding utf-8 \
      --output-dir sortie film.srt
```

Bez `--to-encoding` se soubor zapíše v **kódování, ve kterém byl přečten**,
stejně jako s jeho konci řádků a jeho značkou: soubor převedený bez
pokynu ohledně kódování vrátí tytéž znaky, v týchž bajtech.

**Znak, který cílové kódování neumí zapsat, soubor zastaví**, a
nic se nezapíše. `ł` nemá v Latin-1 místo; nahradit ho znakem `?`
by ztratilo text mezi čtením a zápisem, aniž by to kdo viděl.

**Značka pořadí bajtů existuje jen pro kódování Unicode.**
`--bom` s `--to-encoding`, které žádnou nenese, se odmítne, místo aby se ignorovalo
— viz tabulku chyb níže.

**A kódování musí uvádět své pořadí bajtů.** `UTF-16` a `UTF-32` jsou
názvy, které ICU zná a jejichž převodník zapisuje **vlastní značku**, ať se
po něm žádá cokoli: `--no-bom` by se pak přijalo a neuposlechlo. Oba se
proto odmítnou a hlášení uvádí řešení:

```console
$ subedit-cli convert --to srt --to-encoding UTF-16 --output copie.srt film.srt
"UTF-16" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do
```

`UTF-16LE`, `UTF-16BE`, `UTF-32LE` a `UTF-32BE` zapisují přesně tytéž
bajty, včetně značky — ale pod kontrolou `--bom` a `--no-bom`.
Odmítnutí tedy nestojí nic než přesnější název a platí pro
čtení stejně jako pro zápis: `--encoding UTF-16` dostane stejnou odpověď.

## Cíl zápisu

Tři volby a jejich pravidla jsou společné podpříkazům, které
zapisují: viz [Spuštění](invocation.md#cíl-zápisu). Bez jedné z nich se nic
nezapíše.

Specifické pro `convert`: **`--output-dir` mění příponu spolu s
formátem.** `convert --to vtt` na `a.srt` zapíše `a.vtt`, nikdy `a.srt` obsahující
WebVTT — soubor, jehož název lže, podrazí každý jiný nástroj.

## Převod na místě se odmítne

`--in-place` s `--to`, které mění formát, dává kód `1` a nezapíše
nic: na místě neexistuje druhý název, který by nesl nový formát, a
soubor by zůstal pod příponou, kterou už jeho obsah neopravňuje.

`--in-place` lze přesto použít ke změně **tvaru bez formátu** —
například k zápisu `.srt` zpět jako `.srt` s jinými konci řádků.

O odmítnutí se rozhoduje **jen podle přípony**, před jakýmkoli čtením: chyba
použití nesmí nikdy nechat dávku napůl zapsanou.

## Konce řádků a značka pořadí bajtů

Ve výchozím stavu **zapsaný soubor převezme to, co nesl přečtený soubor**. Model
si obojí při čtení uchovává; vnucovat je ve výchozím stavu by při každém
převodu ztratilo informaci, která se uchovává záměrně.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli convert --to vtt --output b.vtt a.srt; cat b.vtt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli convert --to vtt --output b.vtt a.srt; cat b.vtt
a.srt: 1 subtitle written as WebVTT -> b.vtt
WEBVTT

00:01.000 --> 00:03.000
Bonjour.
```

## Řazení při zápisu

`--sort` zapíše soubor **v pořadí začátků**, ať je cílový formát jakýkoli. Řazení probíhá **na kopii**: přečteného souboru se
nikdo nedotkne a zapíše se to, co by z něj udělal [`sort`](sort.md) — stabilně, dva titulky, které začínají současně, si zachovají své pořadí.

**Nikdy ve výchozím stavu**: pořadí souboru patří jeho autorovi, dokud se nežádá nic jiného. S touto volbou to každý soubor ohlásí na
první úrovni výpisu, `<path>: N subtitles moved` nebo `<path>: already in order`, a záznam `--format json` nese
počet `moved` — **bez volby chybí**, aby se nezměnil tvar toho, co čtou existující skripty.

## Převedení rozsahů na disjunktní

`--disjoint` rozdělí překrývající se titulky tak, aby se už žádný rozsah nepřekrýval s jiným, **na téže kopii jako řazení**: přečteného souboru se nikdo nedotkne.

Nikdy ve výchozím stavu. Volba dělá to, co dělá [`disjoint`](disjoint.md), **s jeho výchozím nastavením**, před zápisem, po řazení. Potřebuje seřazený soubor: jinak se odmítne, a `--sort` — které přichází první — ho předem seřadí. Soubor to ohlásí na první úrovni výpisu (`<path>: N subtitles split, M merged`) a záznam `--format json` nese počty `split` a `merged`, **bez volby chybějí**.

**`--join-repeats` přichází před řazením a dělením**: text rozdělený na po sobě jdoucí části se nejprve znovu sestaví (viz
[`join-repeats`](join-repeats.md)), pak se titulky seřadí, pak se rozsahy převedou na disjunktní. **`--repair` je všechno tři najednou**,
jako [`repair`](repair.md), s výchozím nastavením dělení. S `--join-repeats` každý soubor ohlásí
`<path>: N repeated subtitles joined` na první úrovni výpisu a záznam `--format json` nese `counts.joined` — **bez volby chybí**.

## Výstup

**Standardní výstup** — nic: výsledkem je zapsaný soubor. S `--dry-run` také nic: žádný soubor neexistuje a řádek výpisu to říká — viz [Náhled před zápisem](invocation.md#náhled-před-zápisem).

Na chybovém výstupu, podle úrovně:

| Úroveň | Co se přidá |
| :----- | :-------------- |
| 1 | `<path>: N subtitles written as WebVTT -> <destination>` |
| 2 | `<path>: SubRip -> WebVTT, UTF-8, no BOM, LF line endings` — uvedené kódování je to zapsané |
| 3 | `<path>: N bytes read, M written`, pak **každé hlášení při čtení** — viz [Spuštění](invocation.md#hlášení-při-čtení) |

### Ve formátu JSON

S [`--format json`](invocation.md#výstup-čitelný-pro-skript) nese objekt každého souboru `counts.subtitles` a pak to, co převod ztratil: `lost_ends` a `lost_header` (`0` nebo `1`), `joined_lines`, `lost_tags`, `lost_fields` a `furthest_ms` — položky [věty o ztrátách](#co-převod-hlásí-že-ztratil). Úplná tabulka je na stránce [Spuštění](invocation.md#co-který-podpříkaz-uvádí-v-counts).

## Návratové kódy

Kódy nástroje: `0`, pokud byly zapsány všechny soubory, `2`, pokud žádný, `3`, pokud
jen některé, `1` při chybě použití.

## Chyby

| Co to způsobí | Hlášení |
| :------------------ | :------ |
| `--bom` s `--no-bom` | `--bom and --no-bom ask for opposite things; give one or the other` |
| `--in-place`, které mění formát | `--in-place cannot change the format: the file would keep a name its content no longer matches` |
| `--bom` u kódování, které žádnou nemá | `<path>: <encoding> has no byte order mark to write` |
| `--to-encoding` s neznámým kódováním | `no encoding is named "<name>"` |
| `--to-encoding` s kódováním, které zapisuje vlastní značku | `"<name>" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do` |
| znak, který v zapisovaném kódování chybí | `<path>: holds a character the chosen encoding cannot write` |

**Odmítnutí `--bom` u kódování, které žádnou nenese, není
puntičkářství.** Značka pořadí bajtů existuje pro kódování Unicode a
pro žádná jiná: žádat ji u souboru ve Windows-1252 znamená žádat
něco, co neexistuje. Soubor by se zapsal bez ní a nikdo by
nevěděl, že žádosti nebylo vyhověno.

Chyby cíle zápisu jsou společné podpříkazům, které zapisují:
viz [Spuštění](invocation.md#cíl-zápisu).

Chyby čtení jsou chyby podpříkazu [`inspect`](inspect.md): týkají se
vstupního souboru a mají stejná hlášení.
