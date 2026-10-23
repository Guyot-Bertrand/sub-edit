# Spuštění

<!-- machine-translated -->
> Tato stránka je strojovým překladem a dosud ji nezkontroloval žádný čtenář tohoto jazyka.

```
subedit-cli [global options] <subcommand> [options] <file>...
```

Bez podpříkazu nástroj vypíše svou nápovědu a skončí.

<!-- example: subedit-cli -->
```console
$ subedit-cli
Read, inspect and retime subtitle files.
Usage: subedit-cli [OPTIONS] [SUBCOMMAND]

Options:
  -h,--help                   Print this help message and exit
  --version                   Display program version information and exit
  -v                          Say more: -v is the default, -vv details, -vvv debugs
  -q,--quiet                  Say nothing but errors
  --encoding NAME             Encoding to read the files in; detected by default
  --format text|json          Form of the result on standard output: text or json
  --config FILE               Read the operation settings from FILE instead of the user's settings file
  --no-config Excludes: --config
                              Ignore any settings file: use the built-in defaults
  --list-encodings            Write the encodings that can be read and written, one a line, and stop

Subcommands:
  inspect                     Report what a subtitle file is made of
  convert                     Write a subtitle file out in another format or shape
  shift                       Move every position of a file by a fixed amount
  transform                   Correct every position from two points known to be right
  framerate                   Re-time a file mastered at one frame rate for another
  snap                        Move every position onto the nearest frame of a frame rate (see framerate)
  hearing-impaired            Remove the sounds described between square brackets
  adjust                      Bring the duration of every subtitle within a reading speed and limits
  replace                     Replace a text in the subtitles, without breaking a tag
  case                        Put the texts in title, sentence, upper or lower case, tags intact
  italics                     Put the texts in italics, or take their italics out
  dialogue-dashes             Put dialogue dashes on the lines, or take them off
  sort                        Put the subtitles in the order of their start, keeping ties as they are
  join-repeats                Join the subtitles that repeat each other, one after the other, into one
  disjoint                    Cut overlapping subtitles so that no range overlaps another, merging what overlaps
  repair                      Join the repeated subtitles, put them in order, then make the ranges disjoint
  correct                     Correct the texts with the patterns of the correction assistant
  append                      Put subtitle files one after another into a single file
  split-file                  Cut a subtitle file in two, a head and a tail
  pair                        Write a translation at the positions of its main file, and say how it lined up
  config                      List, show and change the operation settings
```

## Jazyk

Příkazový řádek mluví jazykem svého prostředí: **jeho nápověda, výpis průběhu a
chybová hlášení** se řídí požadovaným jazykem. Příklady v této příručce se spouštějí anglicky —
bloky generuje `make manual`, nikdy se nekopírují ručně.

**Jak se jazyk volí.** Národním prostředím je první z proměnných `LC_ALL`, `LC_MESSAGES` a `LANG`, která je nastavená a
není prázdná; `LANGUAGE`, pokud je nastavená, pak uvádí požadované jazyky oddělené
dvojtečkami (`LANGUAGE=pt_BR:fr`) a čte se jako první. Je to pořadí, které dodržuje gettext, a čte ho
sám program: národní prostředí nemusí být v počítači vygenerované.

| Pravidlo | Účinek |
| :--- | :----- |
| kódování a modifikátor | ignorují se: `fr_FR.UTF-8@euro` hledá `fr_FR`, pak `fr` |
| `LC_ALL=C`, národní prostředí `C` nebo `POSIX`, nebo žádné národní prostředí | angličtina a `LANGUAGE` se nečte |
| položka, která označuje angličtinu (`en`, `en_GB`) | angličtina: to, co za ní v `LANGUAGE` následuje, se nečte |
| jazyk, jehož katalog není nainstalován | další požadovaný jazyk a po posledním angličtina, **bez jediného slova** |
| hlášení, které katalog nepřekládá | zobrazí se anglicky |

**Název opravného vzoru**, o kterém `correct` hlásí, že ho nemohl použít, se také řídí
jazykem, pokud ho katalog zná — jde o vzory dodávané s programem; vlastní vzor se
uvádí tak, jak ho zapisuje jeho soubor. Samotné soubory vzorů se nikdy nepřepisují.

Katalogy se instalují do `<prefix>/share/subedit/locale/<language>/LC_MESSAGES/subedit.mo`
a hledají se od adresáře samotného spustitelného souboru. Angličtina je vestavěná. Tato verze instaluje `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR`, `ru`, `sr`, `tr` a `zh_CN` — katalánštinu,
češtinu, němčinu, španělštinu, finštinu, francouzštinu, galicijštinu, hebrejštinu, maďarštinu, interlingue, islandštinu, italštinu, nizozemštinu, polštinu, portugalštinu, brazilskou portugalštinu, ruštinu, srbštinu, turečtinu a zjednodušenou čínštinu — v úplném překladu.
Francouzštinu zkontroloval čtenář tohoto jazyka; katalánština, čeština, němčina, španělština,
finština, galicijština, hebrejština, maďarština, interlingue, islandština, italština, nizozemština, polština, portugalština, brazilská portugalština, ruština, srbština, turečtina a zjednodušená čínština jsou strojově přeložené a dosud je nezkontroloval žádný čtenář daného jazyka.
<!-- machine-translated: ca -->
<!-- machine-translated: cs -->
<!-- machine-translated: de -->
<!-- machine-translated: es -->
<!-- machine-translated: fi -->
<!-- machine-translated: gl -->
<!-- machine-translated: he -->
<!-- machine-translated: hu -->
<!-- machine-translated: ie -->
<!-- machine-translated: is -->
<!-- machine-translated: it_IT -->
<!-- machine-translated: nl -->
<!-- machine-translated: pl -->
<!-- machine-translated: pt -->
<!-- machine-translated: pt_BR -->
<!-- machine-translated: ru -->
<!-- machine-translated: sr -->
<!-- machine-translated: tr -->
<!-- machine-translated: zh_CN -->
Při `LANG=fr_FR.UTF-8` začíná `subedit-cli --help` větou „Lire,
inspecter et recaler des fichiers de sous-titres.“ Skript, který čte slova textového
výstupu, nastaví `LANGUAGE=en` (nebo `LC_ALL=C`) — nebo čte `--format json`, který se nikdy nemění.

**Co se jazykem nikdy neřídí** — to, co čte skript:

| Nikdy se nepřekládá | Proč |
| :--------------- | :-- |
| výstup `--format json`: klíče, hodnoty a `message` a `detail` selhání a varování | stejné bajty v každém jazyce; slova jsou anglická |
| návratové kódy | čtyři kódy z oddílu [Návratové kódy](#návratové-kódy), v každém jazyce |
| zadávané názvy: podpříkazy, volby, jejich hodnoty (`off`, `srt`), metaproměnné (`FILE`, `TEXT`, `OPTIONS`) | jednou napsaný příkazový řádek funguje v každém jazyce |
| názvy opravných vzorů, které přijímají `correct --enable` a `--disable` | vzor se volá anglickým názvem, který obsahuje jeho soubor |
| názvy formátů a kódování | formát se volá svým názvem |
| vlastní hlášení CLI11 — chybějící povinný argument, neznámá volba, hodnota, kterou volba nepřijímá — a jeho `Run with --help for more information.` | jsou formulována uvnitř analyzátoru, na kterém je příkazový řádek postaven; zůstávají anglicky |

Vyžádání angličtiny bez ohledu na prostředí:

<!-- example: LC_ALL=C LANGUAGE=fr subedit-cli shift --output-dir out a.srt; echo "code=$?" -->
```console
$ LC_ALL=C LANGUAGE=fr subedit-cli shift --output-dir out a.srt; echo "code=$?"
shift needs --by, or --to-grid to work the amount out from the positions
code=1
```

## Globální volby

**Píší se před název podpříkazu, a jen tam.**
`subedit-cli --encoding cp1252 inspect film.srt` je správný tvar;
`subedit-cli inspect --encoding cp1252 film.srt` se odmítne hlášením
`The following argument was not expected: --encoding`. Totéž platí pro
`--version`, `-v` a `--quiet`.

**`--help` je výjimka, a vlastně ani není**:
`subedit-cli inspect --help` se přijme, protože každý podpříkaz má vlastní nápovědu, a vypíše se
nápověda podpříkazu, ne nástroje.

| Volba | Účinek |
| :----- | :---- |
| `-h`, `--help` | vypíše nápovědu a skončí; nápovědu podpříkazu, pokud je nějaký uveden |
| `--version` | vypíše `subedit <version>`, knihovny, se kterými nástroj běží, a nainstalované jazyky, a skončí — viz [`--version`](#--version) |
| `-q`, `--quiet` | úroveň 0 — žádný výpis průběhu |
| `-v`, `-vv`, `-vvv` | úrovně 1 až 3; úroveň 1 je výchozí |
| `--encoding NAME` | čte soubory v tomto kódování, místo aby ho odhadoval |
| `--list-encodings` | vypíše kódování, která nástroj umí číst a zapisovat, jedno na řádek, a **skončí** (kód `0`) — viz [`--list-encodings`](#--list-encodings) |
| `--format text\|json` | podoba výsledku na standardním výstupu: text, nebo jeden objekt JSON na soubor — viz [Výstup čitelný pro skript](#výstup-čitelný-pro-skript) |
| `--config FILE` | čte nastavení operací ze souboru `FILE` místo uživatelského souboru nastavení — viz [Soubor nastavení](#soubor-nastavení) |
| `--no-config` | nečte žádný soubor nastavení: každé nastavení operace, které není zadáno na řádku, má svou vestavěnou výchozí hodnotu — viz [Soubor nastavení](#soubor-nastavení) |

`--quiet` a `-v` ve stejném spuštění se odmítnou: dva opačné záměry
se nerozhodují ve prospěch naposledy zapsaného.

`--config` a `--no-config` ve stejném spuštění se také odmítnou, hlášením
`--config excludes --no-config`, a stejně tak `--config` s prázdným názvem, hlášením
`--config needs a file name`: prázdný název neoznačuje žádný soubor a uživatelův vlastní soubor se
místo něj nečte.

### Soubor nastavení

**[`adjust`](adjust.md) a [`correct`](correct.md) ve výchozím stavu čtou nastavení operací ze
souboru nastavení** — hodnoty, které si v okně pamatují `Adjust Durations…` a `Correct Texts…`,
nebo které zapsal [`config set`](config.md). Žádný jiný podpříkaz ho nečte. Soubor je:

```
$XDG_CONFIG_HOME/subedit/settings.conf
```

nebo `~/.config/subedit/settings.conf`, pokud `XDG_CONFIG_HOME` není nastavená, je prázdná nebo není absolutní
cestou — je to soubor okna, popsaný v [jeho příručce](../subedit-gui/preferences.md#soubor).

**Priorita, od nejsilnější: volba na příkazovém řádku, pak soubor, pak vestavěná výchozí
hodnota.** Volba vždy vyhrává: `--speed off` vypne rychlost, kterou nastavuje soubor. Čte se jen
**blok operací** souboru; předvolby zobrazení (geometrie, sloupce, motiv…)
se nečtou nikdy. [`config list`](config.md#config-list) uvádí klíče a které z nich čte příkazový
řádek: devět klíčů `duration-adjust.*`, `correction.human`, `correction.ocr`,
`correction.remove-blank` a `correction.line-break.max-lines`. **Každý z nich má volbu,
která ho pro jedno spuštění přepíše**, ať ho soubor nastavuje jakkoli — uvádějí je tabulky
podpříkazů [`adjust`](adjust.md#co-dodává-soubor-nastavení) a
[`correct`](correct.md#co-se-vybírá-a-co-ne).

**Čtení uživatelova souboru nikdy nezpůsobí selhání příkazu.** Chybějící soubor dá výchozí hodnoty; soubor,
který nelze přečíst, dá výchozí hodnoty a sdělí to; hodnota, kterou nelze přečíst, si ponechá výchozí hodnotu a uvede se;
neznámý klíč se ignoruje. `adjust` ani `correct` soubor nikdy nezapisují.

**Co bylo přečteno, se sdělí** na výchozí úrovni 1, na standardním chybovém výstupu, před prvním souborem:

| Případ | Řádek |
| :--- | :--- |
| soubor byl přečten | `settings: read <path>` |
| hodnotu v něm nebylo možné přečíst | `settings: <key>: cannot read '<value>', default kept` |
| uživatelův soubor neexistuje | `settings: <path> not found, defaults used` |
| soubor nelze přečíst | `settings: <path> unreadable, defaults used` |
| `--no-config` | nic |

S [`--format json`](#výstup-čitelný-pro-skript) nese objekt každého souboru podpříkazů `adjust` a
`correct` klíč `settings`: přečtenou cestu, nebo `null`, pokud se žádná nečetla.


**`--no-config` činí běh reprodukovatelným**: stejný řádek dá stejný výsledek na každém
počítači, ať jeho uživatel v okně nastavil cokoli. Používejte ho ve skriptech, testech a hlášeních chyb — nebo
`--config FILE` se souborem, který je doprovází.

**`--config FILE` určuje jiný soubor**, čtený stejným způsobem — **s tou výjimkou, že musí existovat**.
Název zadaný na příkazovém řádku není žádostí o výchozí hodnoty: když `FILE` neexistuje,
`adjust` a `correct` skončí dřív, než se přečte jakýkoli vstup, vypíšou `settings file not found: <FILE>` na
standardní chybový výstup a skončí s kódem chyby použití `1`. `FILE`, který existuje a nelze ho přečíst,
dá výchozí hodnoty a sdělí to, stejně jako uživatelův soubor. [`config show`](config.md#config-show)
přesto zobrazí chybějící `FILE` jako `(not found)` a [`config set`](config.md#config-set-a-config-unset) ho
přesto vytvoří.

<!-- example: printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine.conf adjust --dry-run a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine.conf adjust --dry-run a.srt; echo "code=$?"
settings file not found: mine.conf
code=1
```

### `--version`

**Verze programu, sama na prvním řádku** — `subedit <version>`, což byl celý výstup až do
verze 0.15.38 a co čte skript. Pak **jeden řádek na každou knihovnu, se kterou nástroj běží**, pak nainstalované
jazyky rozhraní, na standardním výstupu; nástroj skončí s kódem `0`. Podpříkaz zadaný navíc se
nespustí a `-q` ani `-v` nic nemění.

<!-- example: subedit-cli --version | head -n 1 -->
```console
$ subedit-cli --version | head -n 1
subedit 1.0.9
```

Celý výstup na počítači s ICU 74.2 a Enchant 2.3.3 — čísla jsou čísla daného počítače:

```console
$ subedit-cli --version
subedit <version>
  ICU      74.2 (built with 74.2)
  Enchant  2.3.3 (built with 2.3.3)
  CLI11    2.4.1 (built with 2.4.1)
  Languages: en, fr
```

| Knihovna | K čemu ji nástroj používá |
| :------ | :------------------------ |
| `ICU` | čtení a zápis kódování, jejich zjišťování, změna velikosti písmen |
| `Enchant` | kontrola pravopisu v `correct` |
| `CLI11` | čtení tohoto příkazového řádku |

- **Každý řádek uvádí dvě verze**: tu, která byla nalezena za běhu, jako první, a tu, proti které byl program
  zkompilován, v závorkách. Liší se, když byla knihovna počítače aktualizována po
  sestavení programu — to je první věc, kterou uvést v hlášení chyby.
- **Hlavičkové soubory Enchant nenesou žádnou verzi**: ta v závorkách je ta, kterou ohlásil `pkg-config`
  při konfiguraci sestavení.
- **CLI11 je zkompilována přímo do programu**, protože se skládá jen ze svých hlavičkových souborů: její dvě verze jsou
  vždy stejné.
- **Ani Qt, ani libmpv**: příkazový řádek se nelinkuje s žádnou z nich a uvádí jen to, s čím běží.
  Okno je uvádí — viz [jeho příručku](../subedit-gui/invocation.md#--version).
- **`Languages`** uvádí `en`, zdrojový jazyk, který nepotřebuje žádný katalog, pak každý katalog
  nainstalovaný pod `<prefix>/share/subedit/locale`, který lze přečíst, podle kódu — jazyky,
  které nabízí `Úpravy ▸ Předvolby…`.
- **Co se řídí jazykem**: `built with` a `Languages`. První řádek, názvy,
  čísla a kódy jazyků nikdy.
- **S `--format json`**, před `--version` nebo za ním, **jeden objekt na jednom řádku**: `schema`
  (`1`), `command` (`"version"`), `version`, `libraries` — pole objektů
  `{"name":"ICU","loaded":"74.2","built":"74.2"}` v pořadí textu — a `languages`,
  pole kódů. Nic v něm se nepřekládá.

<!-- example: subedit-cli --version --format json | grep -o '"name":"[^"]*"' -->
```console
$ subedit-cli --version --format json | grep -o '"name":"[^"]*"'
"name":"ICU"
"name":"Enchant"
"name":"CLI11"
```

### `--list-encodings`

**Kódování, která nástroj umí číst a zapisovat, jedno na řádek, seřazená podle názvu**, na standardním výstupu —
pak nástroj skončí s kódem `0`, stejně jako u `--version`: je to odpověď, ne krok.
Podpříkaz zadaný navíc se nespustí.

<!-- example: subedit-cli --list-encodings | grep '^UTF' | head -4 -->
```console
$ subedit-cli --list-encodings | grep '^UTF' | head -4
UTF-16BE
UTF-16LE
UTF-32BE
UTF-32LE
```

- **Jsou to názvy, které přijímá `--encoding`**, ty kanonické; jejich aliasy (`cp1252`) se přijímají také,
  aniž by byly uvedeny.
- **Počet závisí na nainstalované ICU** a žádný název se neslibuje kromě těch, které má každá ICU
  (`UTF-8`, `UTF-16LE`, `windows-1252`…). Příručka proto žádný počet neuvádí.
- **`UTF-16` a `UTF-32` v něm nejsou**: ICU je zná, ale jejich převodník zapisuje vlastní značku
  a model je odmítá — viz níže. Seznam k výběru nenabízí to, co by další pole
  odmítlo.
- **S `--format json`** **jeden objekt na každé kódování**:
  `{"schema":1,"command":"list-encodings","name":"UTF-8"}`. Nejde o zprávu o souboru: tyto
  objekty **nenesou `file` ani `ok`**, což je jediná výjimka z obálky popsané v oddílu
  [Struktura](#struktura). Stále jde o jeden objekt na řádek, jeden na položku.
- **Úroveň výpisu nic nemění**: seznam je výsledek a `-q` ho neodebere.

### `--encoding` a co nedokáže vynutit

**Platí pro každý podpříkaz**, protože všechny čtou: soubor, jehož
kódování je odhadnuto špatně, zůstane špatný, ať se s ním dělá cokoli, a volba,
kterou by měl jen `inspect`, by nechala nesoulad bez nápravy.

**Název je název ICU a přijímají se všechny jeho aliasy**: `cp1252`,
`windows-1252` a `WINDOWS 1252` označují stejné kódování. Množina není nikde
zapsána: je to ta, kterou umí převádět ICU počítače, a
[`--list-encodings`](#--list-encodings) ji vypíše. Název, který převádět neumí, se
odmítne dřív, než se přečte jediný soubor:

```console
$ subedit-cli --encoding klingon-1 inspect film.srt
no encoding is named "klingon-1"
```

**Jedna výjimka, a jen jedna: kódování musí uvádět pořadí bajtů.**
`UTF-16` a `UTF-32` jsou názvy, které ICU zná a jejichž převodník zapisuje **vlastní
značku**. Takové kódování v tomto modelu nemůže existovat — značka by přestala
být volbou, `--no-bom` by se přijalo a neuposlechlo —, proto se obě
odmítají, pro čtení i pro zápis:

```console
$ subedit-cli --encoding UTF-16 inspect film.srt
"UTF-16" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do
```

`UTF-16LE` a `UTF-16BE` čtou a zapisují stejné bajty, včetně značky; přesný
název přidává jen to, že víte, které.

**Značka pořadí bajtů má přednost před volbou.** Je to jediná věc, kterou soubor titulků
sám o sobě deklaruje, a číst ho jinak, než sám deklaruje,
by znamenalo poslechnout volajícího proti souboru. Nesoulad se **sdělí** jako
hlášení při čtení:

```console
$ subedit-cli -vvv --encoding cp1252 inspect avec-bom.srt
avec-bom.srt: a byte order mark that contradicts the encoding asked for ("UTF-16LE"), settled by the reader
```

Tak to dělá i Gaupol — začne čtení znovu s kódováním
značky —, ale neříká to; zde rozdíl mezi tím, co bylo požadováno, a
tím, co bylo provedeno, není zamlčen.

## Podpříkazy

| Podpříkaz | Co dělá | Zapisuje? |
| :------------ | :-------------- | :----------- |
| [`inspect`](inspect.md) | vypíše, z čeho se soubor skládá, aniž by cokoli změnil | ne |
| [`convert`](convert.md) | zapíše soubor v jiném formátu nebo v jiné podobě | ano |
| [`shift`](shift.md) | posune každou pozici o stejnou hodnotu | ano |
| [`transform`](transform.md) | opraví každou pozici podle dvou bodů, o kterých se ví, že jsou správně | ano |
| [`framerate`](framerate.md) | přečasuje soubor z jedné snímkové frekvence na jinou | ano |
| [`snap`](snap.md) | přesune každý časový údaj na nejbližší snímek | ano |
| [`hearing-impaired`](hearing-impaired.md) | odstraní popisky pro neslyšící | ano |
| [`adjust`](adjust.md) | upraví doby trvání podle rychlosti čtení a mezí | ano |
| [`replace`](replace.md) | nahradí text v titulcích, aniž by porušil značku | ano |
| [`case`](case.md) | převede texty na velikost písmen jako v nadpisu, ve větě, na velká nebo malá písmena | ano |
| [`italics`](italics.md) | převede texty na kurzívu, nebo kurzívu odebere | ano |
| [`dialogue-dashes`](dialogue-dashes.md) | přidá dialogové pomlčky, nebo je odebere | ano |
| [`sort`](sort.md) | znovu seřadí titulky podle jejich začátku | ano |
| [`join-repeats`](join-repeats.md) | spojí do jediného titulku po sobě jdoucí kusy téhož textu | ano |
| [`disjoint`](disjoint.md) | zkrátí překrývající se titulky tak, aby se už žádný rozsah nepřekrýval s jiným | ano |
| [`repair`](repair.md) | zřetězí `join-repeats`, `sort` a `disjoint` | ano |
| [`correct`](correct.md) | opraví texty pomocí vzorů Gaupolu: popisky, spojování a rozdělování slov, běžné chyby, velká písmena, zalamování řádků | ano |
| [`append`](append.md) | připojí soubory za první, do **jediného** souboru | ano |
| [`split-file`](split-file.md) | rozdělí soubor na **dvě** části: první a druhou | ano |
| [`pair`](pair.md) | zapíše překlad na pozice jeho hlavního souboru a sdělí, jak se jeho řádky zarovnaly | ano |
| [`config`](config.md) | vypíše, zobrazí a změní nastavení operací v souboru nastavení | jen soubor nastavení |

Je jich všech dvacet jedna; nápověda nástroje je uvádí ve stejném pořadí.

**Jen jeden se ničeho nedotkne**, a to ukazuje pravý sloupec: `inspect` čte a
podává zprávu. Ostatních devatenáct zapisuje a to, co následuje, platí pro sedmnáct z nich:
[`append`](append.md) a [`split-file`](split-file.md) mají každý vlastní aritu — N souborů pro
**jeden** výsledek, jeden soubor pro **dva** — a tedy i vlastní cíl zápisu, který popisují na své stránce.
[`config`](config.md) nezapisuje žádný soubor titulků: čte a mění soubor nastavení a
nic z toho, co následuje, se na něj nevztahuje.

## Cíl zápisu

Sedmnáct podpříkazů, které zapisují — [`convert`](convert.md),
[`shift`](shift.md), [`transform`](transform.md), [`framerate`](framerate.md),
[`snap`](snap.md), [`hearing-impaired`](hearing-impaired.md), [`adjust`](adjust.md),
[`replace`](replace.md), [`case`](case.md), [`italics`](italics.md),
[`dialogue-dashes`](dialogue-dashes.md), [`sort`](sort.md), [`join-repeats`](join-repeats.md),
[`disjoint`](disjoint.md), [`repair`](repair.md), [`correct`](correct.md), [`pair`](pair.md) —, přijímají svůj cíl zápisu stejným způsobem. ([`append`](append.md) a
[`split-file`](split-file.md) mají vlastní, popsaný na jejich stránce.) [`inspect`](inspect.md) nezapisuje žádný soubor a
žádnou z těchto voleb nepřijímá.

**Cíl, který už existuje, se přepíše**, bez dotazu a bez volby, jak se tomu
vyhnout: zápis je atomický — soubor nikdy není zapsán napůl a
přerušený zápis ponechá starý soubor nedotčený —, ale předchozí obsah se
ztratí. Není to ponechaná vada: je to pravidlo a opětovné spuštění příkazu
do stejného výstupního adresáře ho spustí znovu přes jeho vlastní výsledek.

**Bez výslovného cíle se nikdy nic nezapíše.** Tři způsoby, jak
ho zadat, se navzájem vylučují a jeden je nutný: bez něj se nic nezapíše a návratový
kód je `1` — kromě [`--dry-run`](#náhled-před-zápisem), který nic nezapisuje,
a proto žádný cíl nevyžaduje.

| Volba | Kam jde výsledek |
| :----- | :---------------- |
| `--output FILE` | do tohoto souboru, přesně pod tímto názvem; jediný vstup |
| `--output-dir DIR` | do tohoto adresáře, pod základním názvem vstupu |
| `--in-place` | přes vstup, atomickým zápisem |

Nástroj, který přepíše svůj vstup, protože mu nikdo nic neřekl, je nástroj, který
podruhé přestanete používat. Odmítnutí stojí jeden řádek a zachrání
soubor.

`--output` s několika vstupy se odmítne: poslední soubor by se zapsal přes
předchozí. U dávky dává z těchto tří smysl jen `--output-dir`,
spolu s `--in-place`.

**Výstupní adresář se vytvoří**, pokud chybí, včetně nadřazených adresářů — pro
`--output-dir` i pro adresář, do kterého zapisuje `--output`. Stane se to, jakmile je
příkazový řádek ověřen, a **před prvním souborem**. Pokud to nejde,
sdělí se to jednou, nepřečte se žádný soubor a návratový kód je
`2`.

**Dávka se před zápisem posuzuje jako celek.** Nejprve se určí cíl
každého vstupu a z toho plynou dvě odmítnutí — chyby použití, kód
`1`, **žádný zapsaný soubor** a žádný vytvořený adresář:

- **dva vstupy, které by skončily ve stejném souboru** — `a/film.srt` a
  `b/film.srt` s `--output-dir`, nebo `a/film.srt` a `b/film.vtt`, oba
  převedené do WebVTT, jejichž konečný cíl změnil příponu. Hlášení
  uvede cíl i oba vstupy; přejmenovat druhý nebo
  nechat vyhrát poslední by bez upozornění ztratilo soubor, a odmítnutí
  stojí jeden řádek;
- **cíl, který je jedním ze vstupů**, bez `--in-place` — včetně případů
  přes `--output-dir .`, přes `--output` nebo přes symbolický odkaz. Zápis přes
  vlastní vstup je úkon, který se výslovně pojmenuje.

Dvě cesty označují stejný soubor, když to řekne systém, ne když jsou
zapsány stejně: `in/../in/film.srt` a `in/film.srt` jsou jeden a tentýž soubor.

**Přípona se řídí zapisovaným formátem.** Mění se jen u
[`convert`](convert.md), jediného podpříkazu, který mění formát;
ostatní zachovávají formát přečteného souboru, a tedy i jeho příponu.

| Co to způsobí | Hlášení |
| :------------------ | :------ |
| žádný cíl, bez `--dry-run` | `no destination given: use --output, --output-dir or --in-place` |
| dva cíle | `--output, --output-dir and --in-place exclude one another` |
| `--output` u dávky | `--output names one file but several were given: use --output-dir instead` |
| dva vstupy, jeden cíl | `<destination>: would be written by both <input> and <input>` |
| vstup by se přepsal | `<destination>: would be written over the input <input>` — stejná věta pro [`append`](append.md) a [`split-file`](split-file.md) |
| výstupní adresář, který nelze vytvořit | `<directory>: cannot be created: permission denied`, nebo `: cannot be created` |
| cíl, do kterého nelze zapisovat | `<path>: <destination>: cannot be written: permission denied`, nebo `: cannot be written` |
| znak, který v zapisovaném kódování chybí | `<path>: holds a character the chosen encoding cannot write` |

**Poslední si zaslouží větu.** Soubor se zapisuje zpět v **kódování, ve kterém
byl přečten**, stejně jako se zachovají jeho konce řádků a jeho značka. Pokud
změna vnesla znak, který toto kódování nezná — například
`ł` v Latin-1 —, zápis selže a **nic se nezapíše**. Nahradit ho
znakem `?` by znamenalo ztratit text před očima toho, kdo ho právě napsal.

## Náhled před zápisem

**`--dry-run`** přijímá každý ze šestnácti podpříkazů, které zapisují, a všude znamená totéž:
**přečíst, spočítat, podat zprávu, nezapsat žádný soubor**. Přidá se na
řádek, který jste se chystali spustit; použít změny znamená spustit stejný řádek znovu bez něj. Výpočet je
deterministický: co `--dry-run` ukázal, to další spuštění zapíše, pokud se
soubor nezměnil.

- **Nezapíše žádný soubor a nevytvoří žádný adresář**, včetně `--in-place`: vstupní
  soubor zůstane, jaký byl.
- **Nevyžaduje žádný cíl.** Cíl lze zadat a **pak se ověří stejně jako bez
  `--dry-run`**: dva vstupy pro stejný cíl, vstup, který by se přepsal,
  `--output` u dávky, dva způsoby, jak říct kam — příkazový řádek odmítnutý bez `--dry-run`
  se odmítne i s ním, stejnou větou a stejným kódem `1`.
- **Návratový kód je stejný jako u skutečného běhu**: `0`, když se vše spočítalo, `2` nebo `3`,
  když soubory nebylo možné přečíst nebo zpracovat, `1` pro chybu použití. Znak, který
  kódování souboru nedokázalo zapsat, způsobí selhání zkušebního běhu stejně jako skutečného.
  **To, že existují změny, kód nemění**: čtyři kódy říkají, zda nástroj
  uspěl, ne zda soubor vyhovuje.
- **Výpis průběhu to sdělí.** Tam, kde skutečný běh vypíše `-> <destination>`, zkušební
  běh vypíše `(dry run, nothing written)`.

<!-- example: printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by 1 --dry-run a.srt; ls -- *.srt -->
```console
$ printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by 1 --dry-run a.srt; ls -- *.srt
a.srt: 1 subtitle shifted by 1.000 s (dry run, nothing written)
a.srt
```

**U podpříkazu, který mění texty** —
[`hearing-impaired`](hearing-impaired.md), [`replace`](replace.md), [`case`](case.md),
[`italics`](italics.md), [`dialogue-dashes`](dialogue-dashes.md) a [`correct`](correct.md) —, **nese standardní výstup navrhované
změny**. Jeden blok na každý změněný titulek, v pořadí souboru:

```
<path>: subtitle <number>
- <the text before, line by line>
+ <the text after, line by line>
```

- **číslo** je to, které má titulek v přečteném souboru, počítáno od jedné;
- každý řádek textu před změnou má na začátku `- `, každý řádek textu po změně `+ `: text
  o několika řádcích se čte tak, jak je, a řádek, který sám začíná dialogovou
  pomlčkou, dá `- - `;
- **titulek, který by operace odstranila**, to uvede na svém prvním řádku,
  `<path>: subtitle <number> (removed)`, a nemá žádné řádky `+`;
- titulek překladu, s `--document translation`, se na stejném místě ohlásí jako `(translation)`
  — viz [Překlad](#překlad);
- první řádek každého bloku začíná cestou, takže `grep '^film.srt: '`
  najde v dávce bloky jednoho souboru;
- **žádný blok**, když by se nic nezměnilo.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\n- [Grincement]\n- Qui est là ?\n\n' > mentions.srt; subedit-cli hearing-impaired --dry-run mentions.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\n- [Grincement]\n- Qui est là ?\n\n' > mentions.srt; subedit-cli hearing-impaired --dry-run mentions.srt
mentions.srt: 2 subtitles cleaned, 1 removed (dry run, nothing written)
mentions.srt: subtitle 1 (removed)
- [Bruit de pas]
mentions.srt: subtitle 2
- Attends [il tousse] Marie.
+ Attends Marie.
mentions.srt: subtitle 3
- - [Grincement]
- - Qui est là ?
+ Qui est là ?
```

**Seznam je stejný v JSON**: objekt souboru nese pole `changes`, viz
[Struktura](#struktura), s `after` nastaveným na `null` u odstraněného titulku. **Počítá se jen
tehdy, když o něj požádá `--dry-run` nebo `--format json`**: dávka s tisíci
titulků ho nesestavuje zbytečně.

U **ostatních podpříkazů** — [`convert`](convert.md), [`shift`](shift.md),
[`transform`](transform.md), [`framerate`](framerate.md), [`snap`](snap.md),
[`adjust`](adjust.md), [`sort`](sort.md), [`append`](append.md), [`split-file`](split-file.md), [`pair`](pair.md) — nese standardní
výstup to, co by nesl výstup skutečného běhu, tedy v textu nic; v
JSON objekt každého souboru s `"dry_run":true`, `"destination":null` a stejnými
`counts`.

## Dva výstupy, dvě role

| Výstup | Co nese |
| :----- | :--------------- |
| standardní | **výsledek, a jen výsledek** — zprávu `inspect`, navrhované změny textového [`--dry-run`](#náhled-před-zápisem); s `--format json` jeden objekt na soubor — jeden na běh u [`append`](append.md), [`split-file`](split-file.md) a [`pair`](pair.md) |
| chybový | **všechno ostatní** — výpis průběhu, varování, chyby |

Právě toto rozdělení umožňuje přesměrovat výsledek, aniž by se do něj dostal popis průběhu:

```console
$ subedit-cli inspect *.srt > rapport.txt
```

Zpráva jde do souboru, výpis průběhu zůstane na obrazovce.

## Výstup čitelný pro skript

**`--format json`** nahradí text standardního výstupu **jedním objektem JSON na každý
vstupní soubor** ve formátu JSON Lines: jeden kompaktní objekt na řádek, v UTF-8 bez
značky, konec řádku je `\n`. `text` je výchozí hodnota a nemění se. Je to
globální volba, a proto se píše před podpříkaz; jiná hodnota než
`text` a `json` je chyba použití, kód `1`.

<!-- example: printf '1\n00:00:10,000 --> 00:00:12,000\nUn.\n\n' > a.srt; subedit-cli --format json shift --by 1 --output-dir sortie a.srt absent.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:10,000 --> 00:00:12,000\nUn.\n\n' > a.srt; subedit-cli --format json shift --by 1 --output-dir sortie a.srt absent.srt; echo "code=$?"
a.srt: 1 subtitle shifted by 1.000 s -> sortie/a.srt
{"schema":1,"command":"shift","file":"a.srt","ok":true,"dry_run":false,"destination":"sortie/a.srt","counts":{"subtitles":1,"shifted_by_ms":1000},"warnings":[]}
absent.srt: does not exist
{"schema":1,"command":"shift","file":"absent.srt","ok":false,"error":{"kind":"not-found","message":"absent.srt: does not exist"}}
1 of 2 files shifted, 1 failed
code=3
```

**Jeden objekt na vstup, nikdy žádný, nikdy dva**, ať vstup uspěl, nebo ne, v
pořadí, v jakém byly vstupy zadány — nebo nalezeny, s [`--recursive`](lots.md). Stačí
spočítat řádky a své soubory najdete. **Tři podpříkazy jsou výjimkou, protože jejich
arita není aritou dávky: [`append`](append.md) (N vstupů, jeden výstup), [`split-file`](split-file.md)
(jeden vstup, dva výstupy) a [`pair`](pair.md) (dva soubory, jeden výstup) zapisují **jeden objekt za celý
běh**, který uvádí soubor určený na jejich stránce.** **Podpříkazy, které přepisují, proto zapisují také
na standardní výstup**, což jejich smlouva v textu nedělala: jejich
soubor zůstává jejich výsledkem a objekt je zprávou o něm.

**Výpis průběhu se nemění.** `-q`, `-v`, `-vv` a `-vvv` působí na chybový
výstup, v textu, stejně jako bez `--format`; **standardní výstup se s nimi nemění**.
Hlášení při čtení, která text ponechává na úrovni 3, jsou data a jsou
**vždy** v `warnings`. Chyba použití nezapíše žádný objekt: zjistí se
dřív, než se sáhne na jakýkoli soubor, takže není k čemu ji připojit; jde na chybový
výstup s kódem `1` a standardní výstup zůstane prázdný.

### Struktura

| Klíč | Obsah | Přítomen |
| :-- | :------ | :------- |
| `schema` | celé číslo, verze této struktury — `1` | vždy |
| `command` | název podpříkazu | vždy |
| `file` | cesta **tak, jak byla zadána**, nebo jak ji sestavilo procházení adresáře | vždy |
| `ok` | logická hodnota | vždy |
| `error` | `{"kind", "message"}` | pokud je `ok` nepravdivé |
| `dry_run` | logická hodnota: pravdivá pro [`--dry-run`](#náhled-před-zápisem) | pokud je `ok` pravdivé, u podpříkazu, který zapisuje |
| `destination` | zapsaná cesta, nebo `null`, když je `dry_run` pravdivé: nic se nezapsalo | stejně |
| `counts` | objekt **celých čísel**, specifický pro podpříkaz | stejně |
| `changes` | pole objektů `{"subtitle", "document", "before", "after"}`: změněné texty, viz níže | u podpříkazu, který mění texty |
| `warnings` | pole objektů `{"kind", "line"?, "detail"?, "settled"?}` | pokud je `ok` pravdivé |
| `alignment` | jak byl překlad položen, viz [Překlad](#překlad) | s `-t` a u `pair` |
| `constraints` | použitá nastavení | `adjust` |
| `settings` | přečtený soubor nastavení, nebo `null`, pokud se žádný nečetl — viz [Soubor nastavení](#soubor-nastavení) | `adjust`, `correct`, pokud je `ok` pravdivé |
| `inputs`, `tail` | jeden prvek na každý připojený soubor; cesta druhé části | `append`; `split-file` |

`line` chybí u hlášení o celém souboru; `detail` chybí, když není
co dodat; `settled` je `true`, když čtení něco **rozhodlo** — například
znovu vygenerované číslování —, a chybí, když věci ponechalo tak, jak jsou.

**`changes`** uvádí v pořadí souboru každý titulek, jehož text se mění:
`subtitle` je jeho číslo v přečteném souboru, počítáno od jedné; `document` je `"main"`
nebo `"translation"`; `before` je text před změnou; `after` text po změně, nebo **`null`, když se
titulek odstraní**. Zalomení řádků textu jsou v něm `\n`. Zapisuje se
při skutečném běhu stejně jako při [`--dry-run`](#náhled-před-zápisem) a **je prázdné
(`[]`), když se nic nemění**: jeho přítomnost říká, že podpříkaz své změny vypisuje,
jeho nepřítomnost, že žádné nemá.

**Nikde žádné číslo s plovoucí desetinnou čárkou.** Pozice nebo doba trvání je celé číslo
v **milisekundách** (`…_ms`); počet je celé číslo; snímková frekvence je **řetězec**
(`"25"`, `"24000/1001"`), protože je přesná, což racionální číslo nemůže v plovoucí čárce
uchovat; koncentrace mřížky je celé číslo v **tisícinách** — `1000` je
dokonalá mřížka. Dva výstupy se pak dají porovnat bajt po bajtu, bez zaokrouhlování.

**Cestu, která není v UTF-8** — Linux to dovoluje —, nelze v JSON vyjádřit:
zapíše se s U+FFFD místo neplatných bajtů a objekt nese varování
`path-not-utf8`. Věrnost takové cesty se neslibuje a říká se to.

### Co který podpříkaz uvádí v `counts`

| Podpříkaz | Klíče `counts` |
| :------------ | :--------------- |
| [`shift`](shift.md) | `subtitles`, `shifted_by_ms` (se znaménkem) |
| [`transform`](transform.md) | `subtitles` |
| [`framerate`](framerate.md) | `subtitles` |
| [`snap`](snap.md) | `subtitles`, `moved` (posunuté pozice), `furthest_ms` (největší posun) |
| [`hearing-impaired`](hearing-impaired.md) | `cleaned` (přepsané texty), `removed` (odstraněné titulky) |
| [`replace`](replace.md) | `replaced` (nahrazené výskyty ve změněných titulcích), `matched` (nalezené výskyty) |
| [`case`](case.md), [`italics`](italics.md), [`dialogue-dashes`](dialogue-dashes.md) | `changed` (titulky, jejichž text se změnil) |
| [`correct`](correct.md) | `corrected` (změněné texty), `removed` (odstraněné titulky) — nikdy výskyty |
| [`sort`](sort.md) | `subtitles`, `moved` (místa, kde se změnil titulek) |
| [`adjust`](adjust.md) | `subtitles` (cílové titulky), `adjusted` (ty, jejichž konec se posunul), pak skupina `sacrificed`: `speed`, `minimum`, `gap`; a mimo `counts` `constraints` — použitá nastavení |
| [`convert`](convert.md) | `subtitles`, pak to, co převod ztratil: `lost_ends` a `lost_header` (0 nebo 1), `joined_lines`, `lost_tags`, `lost_fields`, `furthest_ms` |
| [`append`](append.md) | `files`, `subtitles`, `appended`, pak sečtené `lost_ends`, `lost_header`, `joined_lines`, `lost_tags`, `lost_fields`, `furthest_ms`; a mimo `counts` `inputs` — jeden prvek na každý připojený soubor |
| [`split-file`](split-file.md) | `subtitles`, `head`, `tail`; a mimo `counts` `tail` — cesta druhé části (`destination` je první část) |
| [`pair`](pair.md) | `subtitles` (titulky zapsaného souboru); a mimo `counts` `alignment` |
| [`inspect`](inspect.md) | žádné `counts`: popis souboru, viz jeho stránku |

### Identifikátory chyb

`error.kind` je jeden z těchto identifikátorů; **čtenář zachází s neznámým identifikátorem jako
s „other“**.

| `kind` | Co znamená |
| :----- | :----------- |
| `not-found`, `permission-denied`, `io` | systém odmítl soubor přečíst — nebo zapsat |
| `unknown-format`, `no-subtitle-found`, `undecodable` | soubor se přečte a nedá se zpracovat |
| `unencodable` | znak, který zvolené kódování neumí zapsat |
| `no-byte-order-mark`, `no-frame-rate` | to, co `convert` nemůže zapsat, aniž by mu to někdo řekl |
| `before-the-origin`, `beyond-the-end`, `no-transform`, `no-grid` | operace, kterou na tento soubor nelze použít |
| `range-out-of-bounds` | `--range`, který tento soubor neobsahuje |
| `no-style` | formát, který nezapisuje žádný styl (TMPlayer, LRC), u `italics` |
| `refused` | jakékoli jiné odmítnutí operace |

### Co je slíbeno a co ne

**Slíbeno, dokud je `schema` rovno `1`**: názvy klíčů, jejich typ a jejich jednotka; význam
`ok`, `file` a `destination`; **jeden objekt na vstup, v pořadí vstupů** — jeden na běh u tří podpříkazů s vlastní aritou;
existující identifikátory `kind`; determinismus — stejné vstupy, stejné argumenty,
stejné bajty.

**Neslibuje se**: **text hlášení** (`message`, `detail`), angličtina pro člověka,
která se opraví, když je to potřeba — skript, který ho porovnává, je rozbitý předem; **pořadí
klíčů** v objektu, pevné, ale JSON ho nezaručuje; přítomnost **klíčů, které ještě nebyly
přidány**.

**Čtenář proto neznámý klíč ignoruje** a s neznámým `kind` zachází jako s „other“:
přidání klíče nebo identifikátoru `schema` nemění. Odstranění nebo přejmenování klíče,
změna typu nebo jednotky, odstranění identifikátoru ho zvýší — to je zlom,
oznámený v CHANGELOG a zde.

## Úrovně výpisu

Každá úroveň **obsahuje předchozí**: o stupeň výš se přidává, nikdy
nenahrazuje.

| Úroveň | Jak | Co nese chybový výstup |
| :----- | :------ | :------------------------------ |
| 0 | `--quiet` | nic, **kromě chyb** |
| 1 | ve výchozím stavu, nebo `-v` | jeden řádek na každý zpracovaný soubor a souhrn, jakmile je souborů několik |
| 2 | `-vv` | a co bylo rozpoznáno: formát, kódování, BOM, konce řádků |
| 3 | `-vvv` | a ladicí výpis: přečtené a zapsané bajty a **každé hlášení při čtení** |

**Chyby se nikdy neumlčují, ani s `--quiet`.** Příkaz, který by selhal
potichu, by nezanechal nic než svůj návratový kód.

Souhrn se objeví až od dvou souborů: u jediného vstupu by
opakoval řádek před sebou.

## Hlášení při čtení

Formáty titulků se čtou **tak dobře, jak to jde**: když narazí na anomálii,
čtečka se nezastaví — rozhodne, nebo ponechá věci tak, jak jsou, a sdělí to. Na co
narazila, se vypíše na **úrovni 3**, jeden řádek na hlášení, na chybový
výstup:

```
a.srt: 2 diagnostics while reading
a.srt: line 6: SubRip numbers that do not follow ("7"), settled by the reader
a.srt: line 9: a line that fits nowhere, left as it stands
```

Hlásí je všech sedmnáct podpříkazů, nejen [`inspect`](inspect.md):
soubor přečtený tak dobře, jak to šlo, a pak zapsaný zpět prošel stejnými rozhodnutími a jejich zamlčení
by nechalo věřit, že se nic nestalo.

**Hlášení nikdy není selháním.** Soubor byl přečten, příkaz
uspěl, návratový kód je `0`. Proto patří na nejpodrobnější
úroveň: níže by zasypala řádek, který říká, co se skutečně
provedlo.

### Co nese každý řádek

| Část | Co říká |
| :----- | :------------- |
| `line N` | kde, počítáno od 1, jak to ukazuje editor; **chybí** u toho, co se týká celého souboru |
| věta | na co se narazilo, z dvanácti níže uvedených kategorií |
| `("…")` | chybný text souboru, když kategorie nestačí; zkrácený na 80 bajtů |
| konec | co se s tím udělalo: `settled by the reader`, nebo `left as it stands` |

**Dvě kategorie nemají řádek**, a jsou to ty dvě, které se týkají
kódování: bajty byly zváženy nebo byla přečtena jejich značka dřív, než existoval jediný
řádek souboru. Jejich řádek proto začíná větou, bez `line N`
na začátku — není žádné místo, které by šlo uvést.

**Konec řádku je to nejdůležitější.** `settled by the reader` znamená,
že čtečka rozhodla a že zapsaný soubor nese její rozhodnutí — například
chybějící číslování se znovu vygeneruje. `left as it stands` znamená, že se ničeho
nedotkla, protože **rozhodnout můžete jen vy**: titulek, který končí dřív, než
začne, zůstane, jak je.

### Číslo řádku bloku

Anomálie týkající se **celého bloku** se váže k jeho časovému
řádku, ne k přesnému řádku, který ji způsobí. Nekonzistentní číslování
zapsané na řádku 5 se proto ohlásí na řádku 6, tedy na časovém řádku, který
následuje — chybné číslo je v `("…")`. Jen anomálie týkající se
**jednoho řádku** — nečitelný časový řádek, text před prvním z nich — se
hlásí na nich samotných.

### Dvanáct kategorií

| Věta | Co ji způsobí |
| :----- | :------------------ |
| `a line that fits nowhere` | řádek, který nepatří do žádného bloku |
| `a timing line that could not be read` | nečitelný časový řádek |
| `a SubRip block without its number` | chybějící číslování, znovu vygenerované při zápisu |
| `SubRip numbers that do not follow` | číslování, které přeskakuje |
| `text before the first timing line` | text před prvním časovým řádkem |
| `a WebVTT block of an unknown kind` | nerozpoznaný blok WebVTT |
| `declares an event column this tool cannot fill` | sloupec událostí Sub Station Alpha, který zde nic nevyplní |
| `counts in frames and states no rate; it was read at` | soubor ve snímcích, který neuvádí snímkovou frekvenci — použitá frekvence je v podrobnostech |
| `carries no end times; each one was taken from the next start` | formát bez konců, TMPlayer nebo LRC: **žádný konec v tabulce nepochází ze souboru** |
| `more than one kind of line ending` | smíšené konce řádků |
| `an encoding nothing declared` | kódování bylo navrženo zvážením bajtů a není to UTF-8 |
| `a byte order mark that contradicts the encoding asked for` | soubor nese značku a `--encoding` uvedla jinou — značka vyhrává |

**Tři uprostřed jsou ty z fáze 9** a poslední dvě z těchto
tří nemluví o řádku, ale o celém souboru: říkají, že
zobrazená hodnota nebyla přečtena. Vyplněný sloupec `End` u LRC,
pozice u MicroDVD — soubor nic z toho nenesl.

**Operace může přidat vlastní**, což není hlášení při čtení:
[`correct`](correct.md#vzor-který-nelze-použít) sdělí spolu s titulkem vzor,
který to vzdal. Sdělí se to na úrovni 1 a v objektu souboru to nese identifikátor
`pattern-failed` — nikdy selhání souboru.

## Překlad

Soubor překladu se zpracovává **jako kterýkoli soubor titulků**:
`subedit-cli case --to sentence film.fr.srt` ho změní bez čehokoli nového. To, co přidává párování a co
říkají tři volby, je úzké: **zjistit, jak řádky překladu padají na
titulky hlavního souboru** — to je `inspect -t` —, a **měnit překlad, zatímco zůstává
položený na svém hlavním souboru**, podle stejných pravidel jako v okně.

| Volba | Kde | Hodnota | Výchozí |
| :----- | :---- | :----- | :----- |
| `-t`, `--translation-file` | `inspect`, `hearing-impaired`, `replace`, `case`, `italics`, `dialogue-dashes`, `correct`, `pair` | soubor překladu | — |
| `--document` | šest textových podpříkazů, ne `inspect` ani `pair` | `main` nebo `translation` | `main` |
| `--align-method` | jako `-t` | `position` nebo `number` | `position` |

- **`position`** porovnává střed každého řádku s hranicemi titulků: chybějící řádek
  neposune ty, které následují. **`number`** pošle řádek *n* k titulku *n*, aniž by se díval na
  pozice. Pozice je výchozí hodnota, ze stejného důvodu jako v okně: chybějící řádek
  uprostřed posune celý zbytek překladu spárovaného podle čísla.
- **`--document translation` vyžaduje `-t` a `-t` vyžaduje `--document translation`** u textového
  podpříkazu — jedno bez druhého je chyba použití, kód `1`, protože přečtený soubor, který žádný úkon
  nepoužije, je opomenutí, a ne preference. `-t` u `inspect` nemá `--document`:
  podává zprávu. **`--align-method` bez `-t` je také chyba použití.**
- **`-t` určuje soubor, a proto má smysl jen pro jediný vstup**: s několika vstupy nebo
  s adresářem převzatým přes `--recursive` se odmítne stejně jako `--output`, **dřív, než se přečte
  jakýkoli soubor**. Pro dávku spusťte program jednou pro každý pár.
- **Soubor překladu totožný s hlavním souborem se odmítne** — ať je jeho
  cesta zapsána jakkoli: `<translation>: the file is already open as the main document`, kód `2`.
- **Zapíše se jen cílový dokument**, na **jeho vlastní cestu a v jeho vlastním formátu**: formát,
  kódování a konce řádků souboru překladu. `--output` určuje soubor
  překladu, `--output-dir` do něj umístí jeho základní název, `--in-place` ho přepíše; **hlavního souboru
  se nikdy nic nedotkne**. Cesty ve zprávě jsou cesty překladu.
- **Titulky, které vzniknou při párování** — řádek, ke kterému nic nedosáhlo —, se zapíšou do
  překladu s **vlastními pozicemi**, ať je metoda jakákoli; **titulek, který nikdo
  nepřeložil, se zapíše jako blok bez textu**, který se přečte bez anomálie.
- **`--encoding` a `--frame-rate` platí pro oba soubory.**
- **Poziční** podpříkazy — `shift`, `transform`, `framerate`, `snap`, `adjust` — tyto
  volby nemají: zadejte jim oba soubory, jako pro dávku,
  `subedit-cli shift --by 2 film.srt film.fr.srt --output-dir out/`.
- **Zápis překladu přečasovaného podle hlavního souboru** — jediná věc, kterou párování umí a samotný soubor
  ne — je úkolem podpříkazu [`pair`](pair.md), který nemá ani `--document`, ani `--recursive`.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\n[Un oiseau] Bonjour.\n\n2\n00:00:03,000 --> 00:00:04,000\nAu revoir.\n\n' > film.fr.srt; subedit-cli hearing-impaired film.srt -t film.fr.srt --document translation --output propre.fr.srt; cat propre.fr.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\n[Un oiseau] Bonjour.\n\n2\n00:00:03,000 --> 00:00:04,000\nAu revoir.\n\n' > film.fr.srt; subedit-cli hearing-impaired film.srt -t film.fr.srt --document translation --output propre.fr.srt; cat propre.fr.srt
film.fr.srt: 1 subtitle cleaned, 0 removed -> propre.fr.srt
1
00:00:01,000 --> 00:00:02,000
Bonjour.

2
00:00:03,000 --> 00:00:04,000
Au revoir.
```

S `--format json` nese objekt každého souboru také klíč `alignment`, který říká, jak byl
překlad položen — `{"file", "method", "attached", "born", "untranslated", "out_of_order"}`, stejně jako
klíč `translation` podpříkazu [`inspect`](inspect.md#ve-formátu-json) —, a **s `--dry-run`** uvádí `changes`
titulky překladu, každý s `"document":"translation"`.

## Více souborů

Každý podpříkaz přijímá několik cest, kromě [`append`](append.md), [`split-file`](split-file.md) a
[`pair`](pair.md), jejichž arita je ta, kterou uvádí jejich stránka. Každá cesta se zpracuje
samostatně: selhání jedné nepřeruší ostatní a selhání
se hlásí s uvedením souboru a důvodu.

**Adresář zastupuje soubory, které obsahuje, s `--recursive` (`-r`)**:
bez této volby je jeho zadání chybou použití, kód `1`, a nic se
nezpracuje — odmítnout ho je lepší než nezpracovat nic a vrátit `0`:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?" -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?"
films: is a directory: use --recursive
code=1
```

Procházení, strom zachovaný pod `--output-dir` a to, co se
vynechá, popisuje stránka [Zpracování stromu](lots.md).

## Manuálové stránky

Instalace umístí **jednu manuálovou stránku na podpříkaz** vedle `subedit-cli(1)`:
`subedit-cli-shift(1)`, `subedit-cli-convert(1)` a další, jednu pro každý řádek
[tabulky podpříkazů](#podpříkazy). `subedit-cli(1)` obsahuje tuto kapitolu — globální volby,
výstupy, návratové kódy — a každá další stránka obsahuje kapitolu svého podpříkazu, včetně jejích
příkladů. Stránka okna, `subedit-gui(1)`, je vedle nich.

```console
$ man subedit-cli
$ man subedit-cli-shift
```

**Stránky se nepíší odděleně od této příručky**: generují se z ní při
sestavení programu, takže říkají totéž co ona a nemohou se od ní odchýlit. Oddíl SOUBORY každé
stránky uvádí adresář, do kterého je příručka nainstalována.

| Stránky | Kam se instalují |
| :---- | :----------------------- |
| anglické | `share/man/man1` |
| každý instalovaný jazyk — katalánština, čeština, němčina, španělština, finština, francouzština, galicijština, hebrejština, maďarština, interlingue, islandština, italština, nizozemština, polština, portugalština, brazilská portugalština, ruština, srbština, turečtina, zjednodušená čínština | `share/man/<language>/man1` — `share/man/fr/man1` |

**`man` vybírá jazyk podle národního prostředí**, stejně jako program: při francouzském národním prostředí
zobrazí `man subedit-cli` francouzskou stránku a jazyk, jehož stránky nejsou nainstalovány, dostane
anglické. `-L` si vyžádá jazyk bez ohledu na národní prostředí:

```console
$ man -L fr subedit-cli-shift
```

## Návratové kódy

| Kód | Význam |
| :--- | :------------ |
| `0` | vše uspělo |
| `1` | chyba použití — neznámá volba, neplatná hodnota, zakázaná kombinace |
| `2` | nebylo možné zpracovat žádný soubor |
| `3` | některé soubory byly zpracovány, jiné ne |

`2` a `3` se rozlišují, aby skript věděl, jak jednat, aniž by znovu četl výstup:
„nic nefungovalo“ a „jeden chybí“ nevyžadují stejnou reakci.

Chyba použití se zjistí **před jakýmkoli zpracováním**: nikdy nenechá
dávku zpracovanou napůl.

## Chyby

| Co to způsobí | Co se zapíše na chybový výstup |
| :------------------ | :--------------------------------------- |
| neznámá volba nebo podpříkaz | chybný název a odkaz na `--help` |
| `--quiet` s `-v` | `--quiet and -v ask for opposite things; give one or the other` |
| `--config` s `--no-config` | `--config excludes --no-config` |
| `--config` s prázdným názvem | `--config needs a file name` |
| chybějící soubor | `<path>: does not exist` |
| nečitelný soubor | `<path>: cannot be opened: permission denied` |
| kódování, které zapisuje vlastní značku | `"<name>" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do` |
| bajty, které nelze dekódovat | `<path>: cannot be decoded in the chosen encoding` |
| nerozpoznaný formát | `<path>: is in no format this tool knows` |
| nic, co by vypadalo jako titulek | `<path>: holds nothing recognisable as a subtitle` |
| `--document translation` bez `-t` | `--document translation needs the translation file: use -t` |
| `-t` bez `--document translation` | `-t names a translation to change: use --document translation` |
| `--align-method` bez `-t` | `--align-method needs a translation file: use -t` |
| `-t` s několika vstupy nebo s adresářem | `-t names one file but several inputs were given: use one invocation per pair` |
| překlad totožný s hlavním souborem | `<translation>: the file is already open as the main document` |
