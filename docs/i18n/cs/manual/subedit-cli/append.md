# `append`

<!-- machine-translated -->
> Tato stránka je strojovým překladem a dosud ji nezkontroloval žádný čtenář tohoto jazyka.

```
subedit-cli append --output FILE
                   [--dry-run] [--sort] [--disjoint]
                   <base> <file>...
```

Připojí soubory **za první z nich**, do **jediného** souboru: film ve dvou částech se změní
v jedinou sadu titulků. Jde o `Append File…` okna — [týž mechanismus](../subedit-gui/operations.md#append-file),
tytéž počty — použitý tolikrát, kolik je souborů k připojení.

- **První soubor je základ.** Dává výsledku **svůj formát, své kódování, svou značku pořadí
  bajtů a své konce řádků**.
- **Mezi dva soubory se nic nevkládá**: každý se posune o **konec posledního titulku toho, co
  mu předchází**, a jeho pozice si zachovají pořadí i rozestupy. Třetí následuje za druhým, ne za
  základem: soubory jdou po sobě v pořadí, v jakém jsou zadány.
- **Každý připojený soubor prochází převodem** do formátu základu: jeho značky se překládají
  a to, co tento formát nedokáže zapsat, se ohlásí soubor po souboru slovy okna.
- **Připojené soubory nemají překlad**: soubor otevřený samostatně žádný nemá, a nemůže ho tedy dát.
- **Soubor lze zadat několikrát**, nebo připojit sám k sobě: nic tomu nebrání.

**`append` není dávka.** N vstupů, **jeden** výstup: smyčka ostatních podpříkazů — jeden výsledek
na soubor a selhání, které ostatní nechá pokračovat — tu nemá smysl. Plynou z toho tři věci:

- **cílem zápisu je `--output`, a nic jiného.** `--output-dir` a `--in-place` pro `append`
  neexistují a odmítnou se jako každá neznámá volba, kód `1`; stejně tak `--recursive`: pořadí
  souborů je to zadané, a procházený strom žádné nemá;
- **všechno, nebo nic**: první soubor, který nelze přečíst, běh zastaví a **nic se nezapíše**.
  Film ve třech částech, z nichž jedna chybí, není film ve dvou;
- **jediný řádek na běh** pro výsledek a ve formátu JSON **jediný objekt**.

<!-- example: subedit-cli append --help -->
```console
$ subedit-cli append --help
Put subtitle files one after another into a single file
Usage: subedit-cli append [OPTIONS] files...

Positionals:
  files BASE FILE...          The base file, then the files to put after it, in order

Options:
  -h,--help                   Print this help message and exit
  --output FILE               File to write
  --dry-run                   Work out and say what would be written, and write nothing
  --sort                      Put the subtitles in order of their start before writing
  --disjoint                  Cut overlapping subtitles so that no range overlaps another, before writing
```

## Argumenty a volby

| Volba | Povinná | Hodnoty | Výchozí |
| :----- | :----- | :------ | :----- |
| `<base> <file>...` | ano, **alespoň dva** | cesty: základ, pak to, co se za něj připojí, v pořadí | — |
| `--output` | **ano**, kromě `--dry-run` | cesta k souboru; jeho adresář se vytvoří, pokud chybí | — |
| `--dry-run` | ne | přepínač: spočítá a vypíše, nic nezapíše — viz [Náhled před zápisem](invocation.md#náhled-před-zápisem) | vypnuto |
| `--sort` | ne | přepínač: seřadí celek podle jeho začátků, **jakmile je vše připojeno**, viz [Seřazení výsledku](#seřazení-výsledku) | vypnuto |
| `--disjoint` | ne | přepínač: rozdělí překrývající se titulky, **jakmile je vše připojeno a seřazeno**, viz [Převedení rozsahů na disjunktní](#převedení-rozsahů-na-disjunktní) | vypnuto |

**Cílem zápisu nemůže být žádný ze vstupů**, včetně základu: zápis přes soubor, který se
čte, se odmítne dříve, než se cokoli přečte, kód `1`. Existující `--output` odlišný od vstupů se
přepíše, jako všude.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n' > partie1.srt; printf '1\n00:00:00,500 --> 00:00:01,500\ntrois\n\n' > partie2.srt; subedit-cli append --output film.srt partie1.srt partie2.srt; cat film.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n' > partie1.srt; printf '1\n00:00:00,500 --> 00:00:01,500\ntrois\n\n' > partie2.srt; subedit-cli append --output film.srt partie1.srt partie2.srt; cat film.srt
partie2.srt: appended 1 subtitle
partie1.srt: 3 subtitles from 2 files -> film.srt
1
00:00:01,000 --> 00:00:02,000
un

2
00:00:03,000 --> 00:00:04,000
deux

3
00:00:04,500 --> 00:00:05,500
trois
```

Poslední titulek `partie1.srt` končí ve 4 s; titulek `partie2.srt`, který začínal v 0,5 s, začíná ve
4,5 s.

## Seřazení výsledku

Každý soubor se posune **od konce toho, co mu předchází**: pořadí, které se může porušit, je tedy pořadí samotných souborů. `--sort`
seřadí výsledek podle začátků, **najednou, po posledním připojení**, jako by to udělal [`sort`](sort.md) — stabilně a beze změny čehokoli
v obsahu.

Nikdy ve výchozím stavu. S volbou to základ řekne na první úrovni výpisu, `<base>: N subtitles moved` nebo `<base>: already in order`, a
záznam `--format json` nese počet `moved`, **který bez volby chybí**.

## Převedení rozsahů na disjunktní

Nikdy ve výchozím stavu. Volba udělá to, co dělá [`disjoint`](disjoint.md), **s jeho výchozím nastavením**, jakmile je vše připojeno, na celku. Potřebuje seřazený soubor: jinak se odmítne, a `--sort` — který přichází první — ho předem seřadí. Soubor to řekne na první úrovni výpisu (`<path>: N subtitles split, M merged`) a záznam `--format json` nese počty `split` a `merged`, **které bez volby chybí**.

## Když se formáty liší

Výsledek je ve **formátu základu**, ať jsou ostatní v jakémkoli. Co přechod stojí, se
ohlásí soubor po souboru slovy okna — touž větou jako `Append File…` a jako
[vložení z jiného formátu](../subedit-gui/clipboard.md#značky-jiného-formátu-se-překládají):

```text
d.ass: appended 1 subtitle; Advanced SSA into SubRip: 1 tag dropped, the header was dropped
```

Co se ztratí, závisí jen na formátu **každého** připojeného souboru: základ v Advanced SSA přijme
soubor SubRip, aniž by cokoli ztratil, opačně ne. Chcete-li zvolit formát výsledku, dejte na první místo soubor,
který už v tomto formátu je, nebo ho poté převeďte pomocí [`convert`](convert.md).

## Výstup

**Standardní výstup** — nic: výsledkem je zapsaný soubor.

| Úroveň | Co se přidá na chybový výstup |
| :----- | :------------------------------------- |
| 1 | jeden řádek na každý připojený soubor — `<path>: appended N subtitles`, za nímž, když formát něco stál, následuje `; <source format> into <base format>: <what is lost>` — pak `<base>: N subtitles from M files -> <destination>` (s `--dry-run` místo `-> <destination>` `(dry run, nothing written)`) |
| 2 | `<base>: SubRip, UTF-8, no BOM, LF line endings kept` — formát, kódování, značka a konce řádků základu, vrácené tak, jak byly |
| 3 | `<base>: N bytes read, M written` (všechny soubory dohromady), pak **každé hlášení při čtení** — viz [Spuštění](invocation.md#hlášení-při-čtení) |

### Ve formátu JSON

S [`--format json`](invocation.md#výstup-čitelný-pro-skript) **jediný objekt** pro celý běh,
jehož `file` je **základ**. Jeho `counts`:

| Počet | Co říká |
| :----- | :----------- |
| `files` | soubory běhu, včetně základu |
| `subtitles` | titulky výsledku |
| `appended` | ty, které pocházejí z připojených souborů |
| `lost_ends`, `lost_header` | `0` nebo `1`: některý z připojených souborů je ztratil |
| `joined_lines`, `lost_tags`, `lost_fields` | součet toho, co připojené soubory ztratily |
| `furthest_ms` | největší posun pozice, všechny soubory dohromady |

**Pole `inputs`** doprovází `counts`, mimo ně: jeden prvek na každý připojený soubor, v pořadí,
`{"file", "counts"}` — `counts` nese `appended` a pak tytéž ztráty, **jen pro tento soubor**. To
říká, který soubor v běhu deseti souborů co stál. Varování při čtení všech souborů
se shromažďují ve `warnings`.

Selhání je také objekt, **jediný**: objekt prvního souboru, který nešlo přečíst.

## Návratové kódy

| Kód | Kdy |
| :--- | :---- |
| `0` | výsledek je zapsán — nebo spočítán, s `--dry-run` |
| `1` | chyba použití: méně než dva soubory, žádný `--output`, volba, kterou `append` nemá, cíl zápisu, který je jedním ze vstupů |
| `2` | **soubor nešlo přečíst** nebo cíl zápisu nešlo zapsat: nic se nezapíše. Kód `3` neexistuje: běh, který zapisuje jen jeden soubor, buď uspěje, nebo ne |

## Chyby

| Co je způsobí | Hlášení |
| :------------------- | :------ |
| zadán jediný soubor | `append needs a base file and at least one file to put after it` |
| ani `--output`, ani `--dry-run` | `no destination given: append writes one file, use --output` |
| `--output` jmenuje vstup | `<destination>: would be written over the input <input>` |
| soubor chybí, je nečitelný nebo v neznámém formátu | `<path>: does not exist`, `<path>: is in no format this tool knows`… — totéž co všude |

**Prázdný soubor se nepřipojí**: žádný formát nečte bez alespoň jednoho titulku a soubor se
odmítne jako každý soubor, který nelze přečíst — `<path>: holds nothing recognisable as a subtitle` —, což
běh zastaví a nic nezapíše.
