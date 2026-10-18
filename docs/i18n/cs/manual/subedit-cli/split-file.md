# `split-file`

<!-- machine-translated -->
> Tato stránka je strojovým překladem a dosud ji nezkontroloval žádný čtenář tohoto jazyka.

```
subedit-cli split-file --at N --head FILE --tail FILE
                       [--dry-run] [--sort] [--disjoint]
                       <file>
```

Rozdělí soubor **na dvě části**: první a druhou. Jde o `Split Project…` okna — [týž mechanismus](../subedit-gui/operations.md#split-project),
tytéž počty, totéž odmítnutí — a o **přesnou inverzi podpříkazu [`append`](append.md)**: připojení druhé části k
první vrátí výchozí soubor.

- **`--at N` je číslo prvního titulku druhé části**, číslované tak, jak čísluje tabulka
  okna: od 1. `--at 3` dá titulky 1 a 2 do první části a třetí a zbytek do druhé
  části. Jde **od druhého po poslední**: dělení potřebuje titulek na každé straně.
- **Druhá část se vrátí k počátku**: posune se o minus konec posledního titulku, který zůstal v první části,
  a její pozice si zachovají pořadí i rozestupy. Právě tím je `split-file` inverzí podpříkazu `append`.
- **Obě poloviny si zachovají formát, kódování, značku pořadí bajtů a konce řádků přečteného souboru** a
  při zápisu se přečíslují od 1, jako každý soubor zapsaný zpět.
- **Dva výstupy, tedy vlastní cíl zápisu**: `--head` a `--tail`, **oba povinné**. `--output`,
  `--output-dir`, `--in-place` a `--recursive` pro `split-file` neexistují a odmítnou se jako každá neznámá
  volba, kód `1`: není adresář, do něhož by se rozdělily dva pojmenované soubory, ani vstup, který by se přepsal.

<!-- example: subedit-cli split-file --help -->
```console
$ subedit-cli split-file --help
Cut a subtitle file in two, a head and a tail
Usage: subedit-cli split-file [OPTIONS] file

Positionals:
  file TEXT REQUIRED          Subtitle file to cut

Options:
  -h,--help                   Print this help message and exit
  --at N                      Number of the first subtitle of the tail, as the table numbers them
  --head FILE                 File to write the subtitles before the cut to
  --tail FILE                 File to write the subtitles from the cut on to
  --dry-run                   Work out and say what would be written, and write nothing
  --sort                      Put the subtitles in order of their start before cutting; --at counts in that order
  --disjoint                  Cut overlapping subtitles so that no range overlaps another, before cutting
```

## Argumenty a volby

| Volba | Povinná | Hodnoty | Výchozí |
| :----- | :----- | :------ | :----- |
| `<file>` | ano | **jediná** cesta | — |
| `--at` | ano | celé číslo od 2 do počtu titulků souboru | — |
| `--head`, `--tail` | **ano, obě**, kromě `--dry-run` | cesty k souborům; jejich adresář se vytvoří, pokud chybí | — |
| `--dry-run` | ne | přepínač: spočítá a vypíše, nic nezapíše — viz [Náhled před zápisem](invocation.md#náhled-před-zápisem) | vypnuto |
| `--sort` | ne | přepínač: seřadí titulky podle jejich začátku **před dělením**; `--at` počítá v tomto pořadí | vypnuto |
| `--disjoint` | ne | přepínač: rozdělí překrývající se titulky **před dělením**; `--at` počítá v tom, co zbude, viz [Převedení rozsahů na disjunktní](#převedení-rozsahů-na-disjunktní) | vypnuto |

**S `--dry-run` není žádný cíl zápisu povinný**: nemusí být zadán žádný, ale ne jen jeden ze dvou —
`--head` a `--tail` patří k sobě. Ty, které zadány jsou, se posuzují jako bez `--dry-run`.

**Oba cíle zápisu nemohou být ani vstupem, ani týmž souborem**: zápis přes to, co se
čte, nebo zápis druhé části přes první se odmítne dříve, než se cokoli přečte, kód `1`.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n3\n00:00:06,000 --> 00:00:07,000\ntrois\n\n4\n00:00:08,000 --> 00:00:09,000\nquatre\n\n' > film.srt; subedit-cli split-file --at 3 --head tete.srt --tail queue.srt film.srt; cat queue.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n3\n00:00:06,000 --> 00:00:07,000\ntrois\n\n4\n00:00:08,000 --> 00:00:09,000\nquatre\n\n' > film.srt; subedit-cli split-file --at 3 --head tete.srt --tail queue.srt film.srt; cat queue.srt
film.srt: split at subtitle 3: 2 subtitles in the head, 2 subtitles in the tail -> tete.srt, queue.srt
1
00:00:02,000 --> 00:00:03,000
trois

2
00:00:04,000 --> 00:00:05,000
quatre
```

Druhý titulek, který zůstal v první části, končí ve 4 s: druhá část se vrátí o 4 s a třetí titulek,
který začínal v 6 s, začíná ve 2 s.

## Dělení v časovém pořadí

`--at` čísluje titulky **tak, jak je soubor podává**. U souboru s porušeným pořadím proto první část není ta polovina, která je časově
dřívější. `--sort` seřadí titulky podle jejich začátků **před posouzením dělení** — stabilně, jako to dělá [`sort`](sort.md) — a
`--at` pak počítá v tomto pořadí.

Nikdy ve výchozím stavu. S volbou soubor řekne na první úrovni výpisu, kolik titulků se přesunulo (`<path>: N subtitles moved`
nebo `<path>: already in order`), a záznam `--format json` nese počet `moved`, **který bez volby chybí**.

## Převedení rozsahů na disjunktní

Nikdy ve výchozím stavu. Volba udělá to, co dělá [`disjoint`](disjoint.md), **s jeho výchozím nastavením**, před posouzením dělení, po seřazení: `--at` počítá v tom, co z toho vzejde. Potřebuje seřazený soubor: jinak se odmítne, a `--sort` — který přichází první — ho předem seřadí. Soubor to řekne na první úrovni výpisu (`<path>: N subtitles split, M merged`) a záznam `--format json` nese počty `split` a `merged`, **které bez volby chybí**.

## Když se dělení odmítne

**Dvě poloviny, které se v místě dělení překrývají, se nerozdělí.** Kdyby titulek druhé části začínal před
koncem posledního titulku, který zůstal vzadu, posun by ho přenesl před začátek videa — záporná pozice,
kterou žádný formát nezapíše. `split-file` odmítne a **jmenuje titulek**, slovy okna:

```text
o.srt: Cannot split at subtitle 3: subtitle 3 would fall before the start of the video. Cut somewhere else.
```

Kód je `2` a **nic se nezapíše**. Rozdělte soubor jinde. Dopadnout přesně na začátek je dovoleno: nula je
pozice.

**`--at` mimo soubor se odmítne stejně**, jakmile je soubor přečten: `--at 1` — první titulek nemůže
začínat druhou část, nic by nezůstalo — a `--at` za posledním titulkem, které by ponechalo prázdnou
druhou část. Soubor s jediným titulkem rozdělit nelze.

## Výstup

**Standardní výstup** — nic: výsledkem jsou oba zapsané soubory.

| Úroveň | Co se přidá na chybový výstup |
| :----- | :------------------------------------- |
| 1 | `<path>: split at subtitle N: H subtitles in the head, T subtitles in the tail -> <head>, <tail>`; s `--dry-run` místo `-> …` `(dry run, nothing written)` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — formát, kódování, značka a konce řádků přečteného souboru, vrácené tak, jak byly |
| 3 | `<path>: N bytes read, H and T written`, pak **každé hlášení při čtení** — viz [Spuštění](invocation.md#hlášení-při-čtení) |

### Ve formátu JSON

S [`--format json`](invocation.md#výstup-čitelný-pro-skript) **jediný objekt**, jehož `file` je rozdělený soubor.
**`destination` je první část** — soubor, kterým se vstup stává — a **`tail` druhá část**, cesta, nebo `null` s
`--dry-run`, stejně jako `destination`. `counts`: `subtitles` (titulky přečteného souboru), `head` a `tail` (titulky každé
poloviny).

## Návratové kódy

| Kód | Kdy |
| :--- | :---- |
| `0` | oba soubory jsou zapsány — nebo spočítány, s `--dry-run` |
| `1` | chyba použití: chybí `--head` a `--tail`, volba, kterou `split-file` nemá, cíl zápisu, který je vstupem nebo druhým cílem, `--at`, které není kladné celé číslo |
| `2` | soubor nešlo přečíst, **dělení se odmítlo** (mimo soubor nebo překryv), nebo cíl zápisu nešlo zapsat. Kód `3` neexistuje: zpracovává se jediný soubor |

**První část se zapisuje jako první**: disk, který odmítne druhou část, za sebou první část ponechá, a kód je `2`.

## Chyby

| Co je způsobí | Hlášení |
| :------------------- | :------ |
| ani `--head`, ani `--tail` | `no destination given: split-file writes two files, use --head and --tail` |
| jen jedna ze dvou, s `--dry-run` | `--head and --tail go together: give both, or neither with --dry-run` |
| oba cíle zápisu jsou týž soubor | `<path>: named as both the head and the tail` |
| cíl zápisu je vstupem | `<path>: would be written over the input <input>` |
| `--at` mimo soubor | `<file>: --at 9: the file holds 3 subtitles, and the tail can start from 2 to 3` |
| soubor s jedním titulkem | `<file>: a file of 1 subtitle cannot be split: a cut needs a subtitle on each side` |
| překryv | `<file>: Cannot split at subtitle N: subtitle M would fall before the start of the video. Cut somewhere else.` |
| soubor chybí, je nečitelný nebo v neznámém formátu | `<path>: does not exist`… — totéž co všude |
