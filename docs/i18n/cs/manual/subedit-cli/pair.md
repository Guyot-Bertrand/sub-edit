# `pair`

<!-- machine-translated -->
> Tato stránka je strojovým překladem a dosud ji nezkontroloval žádný čtenář tohoto jazyka.

```
subedit-cli pair (--output FILE | --output-dir DIR | --in-place)
                 -t FILE [--align-method position|number]
                 [--dry-run]
                 <main>
```

Zapíše **překlad přečasovaný na pozice hlavního souboru** a řekne, jak jeho řádky našly svůj
titulek. Jde o `Open Translation…` okna — [totéž párování](../subedit-gui/files.md#otevření-překladu),
táž věta — následované zápisem výsledku, který okno provádí přes `Save Translation`.

**Je to jediná věc, kterou párování dělá a kterou samostatně zpracovaný soubor nedělá.** Ostatní podpříkazy,
které přijímají `-t`, mění překlad, který čtou — [`case`](case.md), [`replace`](replace.md)… —; `pair` v textu nic
nemění: překlad **položí** na hlavní soubor a zapíše, co položení vytvořilo. Hlavní soubor se
čte, aby přijal řádky, a **nikdy se nezapisuje**.

- **Zapisuje se překlad**, na **jeho vlastní cestu a v jeho vlastním formátu**: s formátem, kódováním a
  konci řádků souboru překladu. Jeho titulky jsou **na pozicích hlavního souboru**: řádek, který
  připadl vedle, dostane zpět meze titulku, který překládá.
- **`-t` je povinné.** `pair` bez překladu nemá co položit: je to chyba použití, kód `1`.
- **Jediný hlavní soubor** a žádné `--recursive`: pár tvoří dva soubory a strom nemá
  jak říct, který překlad patří ke kterému hlavnímu souboru. Pro dávku jedno spuštění na každý pár.
- **`--output` jmenuje soubor zapsaného překladu, `--output-dir` do něj umístí jeho základní název, `--in-place`
  ho přepíše**, jako u každého podpříkazu, který přijímá `-t` — viz [Překlad](invocation.md#překlad).
- **`--document` neexistuje**: cílem je jediný dokument, překlad.

<!-- example: subedit-cli pair --help -->
```console
$ subedit-cli pair --help
Write a translation at the positions of its main file, and say how it lined up
Usage: subedit-cli pair [OPTIONS] main

Positionals:
  main TEXT REQUIRED          The main subtitle file

Options:
  -h,--help                   Print this help message and exit
  -t,--translation-file FILE  Translation file to lay over the subtitle file, for a single input
  --align-method position|number
                              How the lines of the translation find their subtitles
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Argumenty a volby

| Volba | Povinná | Hodnoty | Výchozí |
| :----- | :----- | :------ | :----- |
| `<main>` | ano | **jediná** cesta | — |
| `-t`, `--translation-file` | **ano** | soubor překladu | — |
| `--align-method` | ne | `position` nebo `number` — viz [Překlad](invocation.md#překlad) | `position` |
| `--output` / `--output-dir` / `--in-place` | **jedna ze tří**, kromě `--dry-run` | viz [Spuštění](invocation.md#cíl-zápisu) | — |
| `--dry-run` | ne | přepínač: spočítá a vypíše, nic nezapíše — viz [Náhled před zápisem](invocation.md#náhled-před-zápisem) | vypnuto |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,300 --> 00:00:02,300\nBonjour.\n\n2\n00:00:03,300 --> 00:00:04,300\nAu revoir.\n\n' > film.fr.srt; subedit-cli pair film.srt -t film.fr.srt --output recale.fr.srt; cat recale.fr.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,300 --> 00:00:02,300\nBonjour.\n\n2\n00:00:03,300 --> 00:00:04,300\nAu revoir.\n\n' > film.fr.srt; subedit-cli pair film.srt -t film.fr.srt --output recale.fr.srt; cat recale.fr.srt
film.fr.srt: translation: 2 lines attached -> recale.fr.srt
1
00:00:01,000 --> 00:00:02,000
Bonjour.

2
00:00:03,000 --> 00:00:04,000
Au revoir.
```

Překlad byl o 0,3 s pozadu: každý řádek našel titulek, který překrývá jeho střed, a
zapsaný soubor nese pozice hlavního souboru.

## Co říká zarovnání

Věta je **táž, kterou říká okno**, když otevírá překlad, a každá její část se objeví, jen když není
nulová — tabulku jejích částí viz [`inspect -t`](inspect.md#překlad--t). **Vypisuje se
už od úrovně 1**, a ne na úrovni 2 jako u ostatních podpříkazů: zarovnání tu není
podrobností zpracování, je výsledkem.

**Co zarovnání dělá se zapsaným souborem**:

- **titulek, který nikdo nepřeložil, se zapíše jako blok bez textu**, který se přečte bez anomálie;
- **titulek zrozený z řádku** — řádku, k němuž nic nedosáhlo — se zapíše s **jeho vlastními pozicemi**, ať je
  metoda jakákoli: zapsaný soubor tedy může mít více titulků než hlavní soubor;
- **cokoli dokument nesl jako překlad, se před položením vymaže**, což se na příkazovém řádku neuplatní:
  hlavní soubor se čte samostatně.

**Obě metody** dají stejný soubor, když každý řádek připadne na svůj titulek, a nedají,
když řádek chybí nebo je překlad posunutý — proto je metoda volbou a proto
je výchozí pozice. Osm případů v `src/test/data/paires/` je vyjmenovává, s očekávaným souborem pro
každou metodu.

## Výstup

**Standardní výstup** — nic: výsledkem je zapsaný soubor.

| Úroveň | Co se přidá na chybový výstup |
| :----- | :------------------------------------- |
| 1 | `<translation>: translation: N lines attached; … -> <destination>` — **cesta je cestou překladu**, což je zapisovaný soubor; s `--dry-run` místo `-> <destination>` `(dry run, nothing written)` |
| 2 | `<translation>: SubRip, UTF-8, no BOM, LF line endings kept` — formát, kódování, značka a konce řádků **překladu**, vrácené tak, jak byly |
| 3 | `<translation>: N bytes read, M written`, pak **každé hlášení při čtení** hlavního souboru a překladu — viz [Spuštění](invocation.md#hlášení-při-čtení) |

### Ve formátu JSON

S [`--format json`](invocation.md#výstup-čitelný-pro-skript) **jediný objekt**, jehož `file` je překlad a
`destination` zapsaný soubor. `counts.subtitles` počítá **titulky zapsaného souboru**, včetně těch, které
zrodil řádek. Klíč `alignment` — `{"file", "method", "attached", "born", "untranslated", "out_of_order"}` —
nese tytéž počty jako věta, stejně jako klíč `translation` podpříkazu [`inspect`](inspect.md#ve-formátu-json).

## Návratové kódy

Kódy nástroje: `0`, pokud je překlad zapsán — nebo spočítán, s `--dry-run` —, `2`, pokud jeden z obou souborů nešlo
přečíst nebo pokud cíl zápisu nešlo zapsat, `1` při chybě použití. **Špatně zarovnaný překlad není
selhání**: zapíše se a věta řekne, co nenašlo své místo.

## Chyby

| Co je způsobí | Hlášení |
| :------------------- | :------ |
| chybí `-t` | `pair needs the translation file to lay over the main one: use -t` |
| `--document` | `The following argument was not expected: --document` — pro `pair` neexistuje |
| více než jeden hlavní soubor | `The following argument was not expected: <path>` |
| překlad je hlavním souborem | `<translation>: the file is already open as the main document`, kód `2` |
| jeden ze souborů chybí, je nečitelný nebo v neznámém formátu | totéž co všude, s cestou vinného souboru |
