# `correct`

<!-- machine-translated -->
> Tato stránka je strojovým překladem a dosud ji nezkontroloval žádný čtenář tohoto jazyka.

```
subedit-cli correct --tasks LIST --code CODE
                    [--classes human|ocr|human,ocr]
                    [--enable NAME]... [--disable NAME]...
                    [--language CODE]
                    [--max-length N] [--max-lines N]
                    [--skip-length N|off] [--skip-lines N|off]
                    [--keep-blank-subtitles | --remove-blank-subtitles]
                    [--range N-M|N-]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    <file>...
```

Opravuje texty pomocí **vzorů Gaupolu**, stejně jako průvodce
[`Correct Texts…`](../subedit-gui/correct-texts.md) v okně — stejné vzory,
stejný stroj, stejné počty. Existuje zde šest úloh: **popisky pro
neslyšící**, **spojování** a **rozdělování slov**, **běžné chyby**,
**velká písmena** a **zalamování řádků**.

<!-- example: subedit-cli correct --help -->
```console
$ subedit-cli correct --help
Correct the texts with the patterns of the correction assistant
Usage: subedit-cli correct [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to correct

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --tasks LIST                The tasks to run, in a list: none is chosen beforehand
  --code CODE                 The cascade of patterns the tasks read: Zyyy, Latn, Latn-en, Latn-en-US
  --classes human|ocr|human,ocr
                              The classes of common errors to apply: both by default
  --enable NAME               Switch a pattern on, by its English name or type:name; repeatable
  --disable NAME              Switch a pattern off, by its English name or type:name; repeatable
  --language CODE             The dictionary join-words and split-words read: fr_FR, en_US, en...
  --max-length N              Longest line of line-break, in characters; no default, Gaupol's is in ems
  --max-lines N               Most lines of line-break: 3, as Gaupol's
  --skip-length N|off         Leave alone a subtitle whose longest line is within this, or off: --max-length
  --skip-lines N|off          Leave alone a subtitle whose line count is within this, or off: --max-lines
  --keep-blank-subtitles      Leave the subtitles the correction empties, empty, instead of removing them
  --remove-blank-subtitles    Remove the subtitles the correction empties, whatever the settings file says
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

## Co se vybírá a co ne

**Nic není předem zaškrtnuto.** Okno otevírá svá políčka tak, jak je uživatel nechal;
příkazový řádek bere své úlohy, svůj kód, svou délku řádku a svůj jazyk ze svého vlastního řádku,
nikdy ze souboru nastavení: nečtou se ani úlohy, které byly kdysi zaškrtnuty v okně, ani
políčka vzorů, která se tam přepnula (`correction.activations`), ani seznam náhrad kontroly
pravopisu, a na tuto stranu se nic nezapisuje.

**Čtyři nastavení pocházejí ve výchozím stavu ze souboru nastavení** — hodnoty, které si `Correct Texts…`
pamatuje nebo které zapsal [`config set`](config.md). **Volba na řádku vždy vítězí**, pak
soubor, pak vestavěná výchozí hodnota; viz [Soubor nastavení](invocation.md#soubor-nastavení),
kde se nachází, a `--config` a `--no-config`.

| Klíč | Co dodává | Volba, která ho přepíše |
| :-- | :--------------- | :----------------------- |
| `correction.human`, `correction.ocr` | použité třídy běžných chyb | `--classes` |
| `correction.remove-blank` | zda se titulky, které oprava vyprázdní, odstraní | `--keep-blank-subtitles`, `--remove-blank-subtitles` |
| `correction.line-break.max-lines` | nejvyšší počet řádků úlohy `line-break` | `--max-lines` |

- **Každé ze čtyř má volbu, která ho ruší**, ať soubor říká cokoli: `--classes human,ocr`
  použije obě třídy, `--remove-blank-subtitles` odstraní vyprázdněné titulky, `--max-lines 3`
  vrátí počet řádků Gaupolu. `--keep-blank-subtitles` a `--remove-blank-subtitles` současně jsou
  chybou použití.
- **Obě třídy vypnuté v souboru se ignorují, a hlásí se to** na úrovni 1 —
  `settings: correction.human and correction.ocr are both false, ignored` —: `--classes` by to zapsat nedokázala a úloze
  common-errors by to nenechalo nic na práci. Místo toho se použijí obě třídy.
- **Počet řádků mimo rozsah 1 až 1000 se ze souboru vůbec nepřečte**: čtečka souboru
  ho odmítne — `settings: correction.line-break.max-lines: cannot read 'N', default kept`, na
  úrovni 1 — a použije se vestavěná hodnota 3.

Příklady na této stránce se spouštějí s `--no-config`, aby ukazovaly, co dělají vestavěné
výchozí hodnoty, bez ohledu na nastavení počítače, na kterém běží.

| Volba | Povinná | Hodnoty | Výchozí |
| :----- | :----- | :------ | :----- |
| `<file>...` | ano | jedna nebo více cest | — |
| `--tasks` | **ano** | seznam oddělený čárkami: `mentions`, `join-words`, `split-words`, `common-errors`, `capitalization`, `line-break` | žádná úloha |
| `--code` | **ano** s úlohou vzorů, jinak **se nepoužije** | `Script[-language[-COUNTRY]]`: `Zyyy`, `Latn`, `Latn-en`, `Latn-en-US` | — |
| `--language` | **ano** s `join-words` nebo `split-words`, jinak **se nepoužije** | kód jazyka: `fr`, `fr_FR`, `en_US` | — |
| `--classes` | ne | `human`, `ocr` nebo `human,ocr` | obě — nebo to, co říká soubor nastavení |
| `--enable`, `--disable` | ne, opakovatelná | anglický název vzoru nebo `type:name` | dodávané soubory `.conf` |
| `--max-length` | **ano** s `line-break` | počet **znaků** větší než nula | **žádná** — viz níže |
| `--max-lines` | ne | celý počet řádků větší než nula | 3 — nebo to, co říká soubor nastavení |
| `--skip-length`, `--skip-lines` | ne | číslo nebo `off` | stejné meze jako `--max-length` a `--max-lines` |
| `--keep-blank-subtitles` | ne | přepínač: vyprázdněné titulky zůstanou, prázdné | vyprázdněné titulky se odstraní — pokud soubor nastavení neříká jinak |
| `--remove-blank-subtitles` | ne | přepínač: vyprázdněné titulky se odstraní, ať soubor nastavení říká cokoli; odmítne se s `--keep-blank-subtitles` | vypnuto |
| `--range` | ne | `N-M` nebo `N-` až do konce — viz [`adjust`](adjust.md#--range) | celý soubor |
| `--document` | ne | `main` nebo `translation` — viz [Překlad](invocation.md#překlad) | `main` |
| `-t`, `--translation-file` | s `--document translation` | soubor překladu, pro **jediný** vstup | — |
| `--align-method` | ne, a jen s `-t` | `position` nebo `number` | `position` |
| `--recursive`, `-r` | ne | přepínač | vypnuto — viz [Zpracování stromu](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **jedna ze tří**, kromě `--dry-run` | viz [Spuštění](invocation.md#cíl-zápisu) | — |
| `--dry-run` | ne | přepínač: spočítá a vypíše, nic nezapíše — viz [Náhled před zápisem](invocation.md#náhled-před-zápisem) | vypnuto |

### Úlohy

**Na pořadí seznamu nezáleží**: úlohy se použijí v
pořadí Gaupolu — popisky, pak spojování a rozdělování slov, pak běžné chyby, pak velká písmena, pak zalamování řádků —, každá na
text, který zanechala předchozí. Titulek, který popisky vyprázdnily, se dalšího zpracování
neúčastní.

| Úloha | Co dělá |
| :---- | :-------------- |
| `mentions` | odstraní zvuky v hranatých závorkách a v kulatých závorkách, když je zapnutý `Sound in parentheses` (čištění podpříkazu [`hearing-impaired`](hearing-impaired.md)), pak vzory textů písní a mluvčích |
| `join-words` | slepí zpět slovo rozdělené na dvě, podle slovníku `--language` |
| `split-words` | rozdělí dvě slepená slova, podle téhož slovníku |
| `common-errors` | opraví běžné chyby: mezery, interpunkci, slitky, chyby OCR |
| `capitalization` | dá velké písmeno tam, kde ho jazyk chce |
| `line-break` | znovu rozdělí řádky titulku tak, aby žádný nepřesáhl `--max-length`, s ohledem na vzory zalomení daného kódu |

### Kód vzorů

**`--code` pojmenovává kaskádu**: `Zyyy` pro jakékoli písmo, pak písmo, jazyk
a země — `Latn-en-US` čte vzory `Zyyy`, `Latn`, `Latn-en` a
`Latn-en-US`. **Je povinný**, protože příkazový řádek nemá žádný okolní jazyk:
univerzální výchozí hodnota by použila *méně*, než si člověk myslí, a neřekla by to. Platí pro
všechny úlohy téhož běhu; úloha, která chce vlastní kód, je druhý
běh. Špatně zapsaný kód je chybou použití:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code francais --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code francais --output b.srt a.srt; echo "code=$?"
--code: "francais" is not a pattern code: expected Script[-language[-COUNTRY]], like Zyyy, Latn or Latn-en-US
code=1
```

Vzory, které uživatel uloží do `$XDG_DATA_HOME/subedit/patterns` (nebo
`~/.local/share/subedit/patterns`), **se přidávají k dodávaným vzorům**, stejně jako v
okně, a dodávaný vzor, který nelze přečíst, se ohlásí na úrovni 1: `patterns: <file>, line N (<reason>)`.

### Spojování a rozdělování slov a slovník

**Tyto dvě úlohy nečtou žádný vzor**: dotazují se **systémového slovníku prostřednictvím
Enchant** — závislosti balíčků (`libenchant-2-2` v Debianu a Ubuntu, `enchant2` ve
Fedoře), kterou `subedit-cli` sdílí s oknem. Nemají tedy ani kód, ani název k nastavení:
`--code` nemá smysl bez úlohy vzorů a `--language` žádný bez jedné z těchto dvou
— jedno zadané bez druhého je chybou použití, kód `1`.

- **`join-words`** slepí slovo, které slovník nezná, se slovem, které mu předchází nebo
  následuje, **když známé slovo dává jen jediný směr**: z `hel lo` se stane `hello`; dva možné
  směry, nebo žádný, nerozhodují nic.
- **`split-words`** rozdělí slovo, které slovník nezná, **když právě jeden z jeho
  návrhů je totéž slovo s mezerou**: z `hellothere` se stane `hello there`. Slovo s velkým
  počátečním písmenem, za nímž následují malá písmena, se nechá být: nejčastěji jde o jméno.

**Bez slovníku pro daný jazyk se běh odmítne**, kód `1`, **dříve, než se přečte jakýkoli
soubor**, a slovy okna, které funkci zašedí a řekne to — zde příkazový řádek
odmítne a řekne to, místo aby mlčel:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nhel lo\n\n' > a.srt; subedit-cli --no-config correct --tasks join-words --language qq_QQ --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nhel lo\n\n' > a.srt; subedit-cli --no-config correct --tasks join-words --language qq_QQ --output b.srt a.srt; echo "code=$?"
no dictionary for qq_QQ
code=1
```

**Seznam náhrad okna se nečte ani nezapisuje.** *Plní se*
volbami uživatele při kontrole pravopisu; příkazový řádek takovou
volbu nemá a čtení seznamu by způsobilo, že skript závisí na tom, co do něj jeho autor kdysi vložil.

### Zalamování řádků

**Měří se ve znacích, a v tom je rozdíl oproti oknu.** Okno měří v *em* —
šířce kalibrované podle písma, 0,55 em na písmeno —, a jeho výchozí hodnota 24 je hodnota
v em. Příkazový řádek žádné písmo nemá: počítá znaky, znaky
textu. **Převzetí hodnoty 24 jako počtu znaků by rozdělilo téměř všechno, a neřeklo by
to**; proto **neexistuje žádná výchozí délka** a `--max-length` je s
`line-break` povinná:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn texte\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn texte\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --output b.srt a.srt; echo "code=$?"
--max-length is required by the task line-break: Gaupol's 24 is a width in ems, and has no value in characters
code=1
```

**`--max-lines` je 3**, hodnota Gaupolu, kterou jednotka nemění; zalamování řádků si
vezme o jeden víc, když tři nestačí. **Přeskočení** — titulek, který už je v mezích,
se nechá tak, jak je — bere ve výchozím stavu **stejné meze** (`--skip-length` = `--max-length`,
`--skip-lines` = `--max-lines`), stejně jako Gaupol; `--skip-length off` a `--skip-lines off`
ho vypnou a text se pak znovu přečte jako celek a znovu rozdělí:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nWhen the rain stopped, we walked home under the dark sky of the old town.\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --max-length 24 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nWhen the rain stopped, we walked home under the dark sky of the old town.\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --max-length 24 --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:03,000
When the rain
stopped, we walked
home under the dark
sky of the old town.
```

Vzory zalomení jsou vzory kódu: `Latn` nezalamuje mezi číslem a jeho jednotkou ani
za dialogovou pomlčkou, `Latn-en` nezalamuje za členem, předložkou, titulem
ani přivlastňovacím determinantem. **Pod `Zyyy` žádný vzor zalomení neexistuje**: řádky se
jen vyvažují. **Tato úloha se nikdy neodmítne kvůli chybějícímu aktivnímu vzoru**: její vzory
váží, kde zalomit, zalomení proběhne i bez nich. Čtyři volby mají smysl jen s
`line-break`; zadané bez ní jsou chybou použití.

### Třídy a přepínání podle názvu

**`--classes` filtruje běžné chyby natrvalo**: zrušení zaškrtnutí `ocr` vyřadí jeho
vzory z použití — slitek `ﬁ` pak zůstane `ﬁ`. Ostatní tři typy vzorů
žádnou třídu nemají.

**`--enable` a `--disable` nastavují vzor podle jeho anglického názvu**, přičemž název je klíčem
(ADR 0037); jeden název může označovat několik záznamů, které pak sdílejí jediné
políčko. Je to nutné a právě to odkládá výchozí hodnoty okna stranou: **dodávané
soubory `.conf` vzory popisků vypínají**, takže samotné `--tasks mentions`
by nepoužilo nic. Dva vzory „Sound in brackets“ a „Sound in parentheses“
ovládají každý svůj oddělovač — hranaté závorky, kulaté závorky — a nastavují se týmiž
volbami: zapnutí jen jednoho z nich neodstraní druhý.

Tři věci jsou **chybami použití**, kód `1`, ohlášenými dříve, než se přečte jakýkoli soubor:

| Co to způsobí | Hlášení |
| :------------------ | :------ |
| název, který neoznačuje žádný vzor zadaných úloh pod zadaným kódem | `--enable: "<name>" names no pattern of the tasks given, under the code <code>` |
| název, který označuje vzory dvou typů — pak se píše `type:name`, kde typ je `common-error`, `capitalization`, `hearing-impaired` nebo `line-break` | `--enable: "<name>" names patterns of several types: write type:name, …` |
| název zadaný pro `--enable` i pro `--disable` | `"<name>" is given to --enable and to --disable: choose one` |

**Překlep je hlasitý**: okno zastaralé nastavení ignoruje, protože
nastavení může přežít vzor, který zmizel; spuštění žádného přeživšího nemá.

**Úloha bez jediného aktivního vzoru se odmítne**, kód `1` — „nic na práci“ je
odpověď, která se vysloví, ne tichý úspěch:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --output b.srt a.srt; echo "code=$?"
mentions: no pattern is active under the code Latn; switch one on with --enable
code=1
```

Tentýž soubor se zapnutým čištěním hranatých závorek:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --enable "Sound in brackets" --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --enable "Sound in brackets" --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:02,000
Bonjour
```

## Co se zapíše a co se ohlásí

Formát, kódování a konce řádků přečteného souboru se **zachovají**. **Titulky,
které oprava vyprázdní, se odstraní**, jako ve výchozím stavu políčka
okna; `--keep-blank-subtitles` je ponechá prázdné a `--remove-blank-subtitles` je odstraní
i tehdy, když je soubor nastavení ponechává. **Vyprázdněný překlad zůstane vyprázdněný**
a svůj titulek si ponechá.

**Počet je stejný jako v okně** — „Edited N and removed M subtitles“: změněné
texty a odstraněné titulky, **nikdy shody**. Soubor, ve kterém se nic
nezmění, se zapíše i tak: zadaný cíl zápisu je zapsaný cíl zápisu.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\neﬀet  de ﬁn\n\n2\n00:00:03,000 --> 00:00:04,000\nPropre.\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code Latn --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\neﬀet  de ﬁn\n\n2\n00:00:03,000 --> 00:00:04,000\nPropre.\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code Latn --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:02,000
effet de fin

2
00:00:03,000 --> 00:00:04,000
Propre.
```

### Vzor, který nelze použít

**Vzor, který nelze přečíst, nelze přeložit nebo který neskončí, se pojmenuje spolu s
titulkem, u něhož to vzdal**, a zde příkazový řádek říká víc než
okno: stránka potvrzení vzor pojmenuje jen jednou, bez textu.

| Důvod | Co znamená |
| :----- | :------------------- |
| `cannot be translated` | jeho výraz používá něco, co překlad do ICU odmítá |
| `will not compile` | stroj jeho výraz odmítl |
| `has an invalid replacement` | jeho náhrada se nečte tak, jak ji čte Python |
| `timed out` | stroj to u daného textu vzdal |
| `never settled` | `Repeat` text po sto průchodech stále měnil |
| `grew the text too long` | text přesáhl to, co titulek dokáže pojmout |

Na úrovni 1, na chybovém výstupu: 
`<path>: pattern "<name>" (<reason>) was not applied to subtitle N` — bez konce, když se vzor odloží pro všechny texty. **Číslo je číslem
souboru**, ať úlohy předtím odstranily jakékoli titulky. Ve formátu JSON jde o
varování souboru `pattern-failed`, jehož `detail` nese tuto větu. **Vzor,
který to vzdal, neznamená neúspěšný soubor**: soubor se zapíše a návratový kód je `0`.

## Výstup

**Standardní výstup** — nic, protože výsledkem je zapsaný soubor. **S `--dry-run` jde o
navrhované změny**, jeden blok na titulek: jeho číslo, text před, text po —
a `(removed)` bez řádku `+` u titulku, který by oprava odstranila; s
`--keep-blank-subtitles` zůstane, prázdný. Viz [Náhled před zápisem](invocation.md#náhled-před-zápisem).

| Úroveň | Co se přidá na chybový výstup |
| :----- | :------------------------------------- |
| 1 | `<path>: Edited N and removed M subtitles -> <destination>`, vzory, které nebylo možné použít, soubory vzorů, které nelze přečíst — a jako první přečtený soubor nastavení — `settings: read <path>` — a co z něj bylo ignorováno |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, pak každé hlášení při čtení |

Ve formátu JSON nese `counts` hodnoty `corrected` (změněné texty) a `removed` (odstraněné
titulky) a `changes` dotčené titulky. S `-t` klíč `alignment` říká, jak byl
překlad položen.
`settings` uvádí soubor nastavení, který byl přečten, nebo je `null`, když žádný přečten nebyl.

## Návratové kódy

Kódy nástroje: `0`, pokud byly zapsány všechny soubory, `2`, pokud žádný, `3`, pokud jen
některé, `1` při chybě použití.

## Chyby

| Co to způsobí | Co se zapíše na chybový výstup |
| :------------------ | :--------------------------------------- |
| soubor zadaný přes `--config`, který neexistuje — chybějící vlastní soubor uživatele dává výchozí hodnoty | `settings file not found: <FILE>` |
| chybí `--tasks` | `--tasks is required`, následované odkazem na `--help` |
| neznámá úloha | `--tasks: "<name>" is not a task: expected mentions, join-words, split-words, common-errors, capitalization or line-break` |
| chybí `--code` | `--code is required by the tasks that read patterns: <tasks>` |
| `--code` bez úlohy vzorů | `--code is for the tasks that read patterns, and none was asked for` |
| chybí `--language` s `join-words` nebo `split-words` | `--language is required by the tasks that check words: <tasks>` |
| `--language`, které není kódem jazyka | `--language: "<value>" is not a language: expected a locale code, like fr, en_US or sr@Latn` |
| `--language` bez jedné z těchto dvou úloh | `--language is for the tasks join-words and split-words, which were not asked for` |
| jazyk bez slovníku | `no dictionary for <language>` |
| špatně zapsaný kód | `--code: "<code>" is not a pattern code: expected Script[-language[-COUNTRY]], like Zyyy, Latn or Latn-en-US` |
| chybí `--max-length` s `line-break` | `--max-length is required by the task line-break: Gaupol's 24 is a width in ems, and has no value in characters` |
| `--max-length`, které není číslem větším než nula | `--max-length: "<value>" is not a length: expected a number of characters greater than zero` |
| `--max-lines`, `--skip-lines`, které nejsou celým počtem řádků | `--max-lines: "<value>" is not a number of lines: expected a whole number greater than zero` |
| nečitelné `--skip-length` nebo `--skip-lines` | `--skip-length: "<value>" is not a bound: expected a number greater than zero, or off` |
| jedna ze čtyř voleb bez `line-break` | `--max-length is for the task line-break, which was not asked for` |
| neznámá třída | `--classes: "<name>" is not a class: expected human, ocr or human,ocr` |
| `--keep-blank-subtitles` s `--remove-blank-subtitles` | `--keep-blank-subtitles and --remove-blank-subtitles say opposite things; give one of them` |
| chybný název vzoru | viz tabulku výše |
| úloha bez aktivního vzoru | `<task>: no pattern is active under the code <code>; switch one on with --enable` |
| `--range`, `-t`, `--document`, cíl zápisu | chyby ze stránky [Spuštění](invocation.md#chyby) a z oddílu [Překlad](invocation.md#překlad) |
