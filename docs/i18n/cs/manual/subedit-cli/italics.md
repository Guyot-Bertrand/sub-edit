# `italics`

<!-- machine-translated -->
> Tato stránka je strojovým překladem a dosud ji nezkontroloval žádný čtenář tohoto jazyka.

```
subedit-cli italics (--on | --off) [--range N-M|N-]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    <file>...
```

Převede texty **na kurzívu**, nebo **jim kurzívu odebere**, ve značkách formátu souboru.
Jde o `Italic` [okna](../subedit-gui/operations.md).

**Dvě slova, a ne přepínač tam a zpět**: tlačítko okna se ptá jádra, kterým směrem
působí; tady to řeknete sami. `--on` a `--off` se navzájem vylučují a jedno z nich je nutné — jeho vynechání nebo zadání obou
je chyba použití (kód `1`).

- **`--on`** obalí každý text jedinou dvojicí značek, **když předtím odstraní ty, které
  v něm byly**: text, který už kurzívou je, se neobalí dvakrát, a kurzíva rozdělená ve dvě
  se udělá znovu v jednom kuse. Na více řádcích pokryje celek jedna dvojice.
- **`--off`** odstraní značky kurzívy; **zbytek značky se zachová** (`{\b1\i1}` dá
  `{\b1}`).
- Text se zapíše **tak, jak kurzívu zapisuje formát souboru**: `<i>…</i>` pro SubRip, WebVTT a
  SubViewer 2, `{\i1}…{\i0}` pro obě Sub Station Alpha, `{Y:i}` pro MicroDVD.

<!-- example: subedit-cli italics --help -->
```console
$ subedit-cli italics --help
Put the texts in italics, or take their italics out
Usage: subedit-cli italics [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --on                        Put the texts in italics
  --off                       Take the italics out
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

## Argumenty a volby

| Volba | Povinná | Hodnoty | Výchozí |
| :----- | :----- | :------ | :----- |
| `<file>...` | ano | jedna nebo více cest | — |
| `--recursive`, `-r` | ne | přepínač | vypnuto — viz [Zpracování stromu](lots.md) |
| `--on` / `--off` | **jedna ze dvou** | přepínače | — |
| `--range` | ne | `N-M` nebo `N-` až do konce — viz [`adjust`](adjust.md#--range) | celý soubor |
| `--document` | ne | `main` nebo `translation` — viz [Překlad](invocation.md#překlad) | `main` |
| `-t`, `--translation-file` | s `--document translation` | soubor překladu, pro **jediný** vstup | — |
| `--align-method` | ne, a jen s `-t` | `position` nebo `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **jedna ze tří**, kromě `--dry-run` | viz [Spuštění](invocation.md#cíl-zápisu) | — |
| `--dry-run` | ne | přepínač: spočítá a vypíše, nic nezapíše — viz [Náhled před zápisem](invocation.md#náhled-před-zápisem) | vypnuto |

**Ve výchozím stavu hlavní text; překlad s `--document translation -t FILE`.** Mění a zapisuje se
pak překlad, a jen on — viz [Překlad](invocation.md#překlad).

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour\nMarie\n\n' > a.srt; subedit-cli italics --on --output i.srt a.srt; cat i.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour\nMarie\n\n' > a.srt; subedit-cli italics --on --output i.srt a.srt; cat i.srt
a.srt: 2 subtitles put in italics -> i.srt
1
00:00:01,000 --> 00:00:02,000
<i>bonjour marie</i>

2
00:00:03,000 --> 00:00:04,000
<i>Bonjour
Marie</i>
```

## Formát bez stylu odmítá

**TMPlayer a LRC nezapisují žádný styl**: není kurzíva, kterou by šlo přidat ani odebrat.
Okno položku zašedí, místo aby ji skrylo; příkazový řádek **odmítne, soubor po
souboru, s uvedením důvodu**, a ostatní soubory dávky pokračují. Rozhoduje to, co
formát dokáže nést, ne způsob, jakým to zapisuje: SubRip a WebVTT zapisují kurzívu stejně,
a o barvě neříkají totéž.

<!-- example: printf '00:00:01:Hello there\n00:00:03:Bye\n' > a.txt; subedit-cli italics --on --output-dir sortie a.txt; echo "code=$?" -->
```console
$ printf '00:00:01:Hello there\n00:00:03:Bye\n' > a.txt; subedit-cli italics --on --output-dir sortie a.txt; echo "code=$?"
a.txt: TMPlayer writes no style: there are no italics to put on
code=2
```

Ve formátu JSON nese selhání identifikátor **`no-style`**.

## Výstup

**Standardní výstup** — nic: výsledkem je zapsaný soubor. S `--dry-run` výpis
změn popsaný v oddílu [Náhled před zápisem](invocation.md#náhled-před-zápisem).

| Úroveň | Co se přidá na chybový výstup |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles put in italics -> <destination>`, nebo `N subtitles taken out of italics`, nebo `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — formát, kódování, značka a konce řádků přečteného souboru, vrácené tak, jak byly |
| 3 | `<path>: N bytes read, M written`, pak **každé hlášení při čtení** — viz [Spuštění](invocation.md#hlášení-při-čtení) |

`N` je počet titulků, **jejichž text se změnil**, převzatý ze samotné operace.

### Ve formátu JSON

S [`--format json`](invocation.md#výstup-čitelný-pro-skript) nese objekt každého souboru
`counts.changed` — `N` z věty — a `changes`, seznam změněných titulků
(`{"subtitle", "document", "before", "after"}`, **prázdný**, když se nic nezměnilo).

## Návratové kódy

Kódy nástroje: `0`, pokud byly zpracovány všechny soubory — soubor, v němž se nic nemění, zpracovaný je —,
`2`, pokud žádný, `3`, pokud jen některé, `1` při chybě použití.

## Chyby

| Co to způsobí | Hlášení |
| :------------------ | :------ |
| ani `--on`, ani `--off` | `italics needs --on to put the texts in italics, or --off` |
| obě | `--on and --off say opposite things; give one of them` |
| formát bez stylu | `<path>: TMPlayer writes no style: there are no italics to put on` (nebo `take out`) |
| rozsah špatně zapsaný nebo mimo soubor | viz [`adjust`](adjust.md#--range) |

Chyby cíle zápisu jsou společné podpříkazům, které zapisují:
viz [Spuštění](invocation.md#cíl-zápisu).
