# `dialogue-dashes`

<!-- machine-translated -->
> Tato stránka je strojovým překladem a dosud ji nezkontroloval žádný čtenář tohoto jazyka.

```
subedit-cli dialogue-dashes (--add | --remove) [--range N-M|N-]
                            (--output FILE | --output-dir DIR | --in-place)
                            [--dry-run]
                            [--recursive]
                            [--document main|translation] [-t FILE] [--align-method position|number]
                            <file>...
```

Přidá na začátek řádků **dialogovou pomlčku**, nebo ji odebere. Jde o `Dialogue Dashes`
[okna](../subedit-gui/operations.md).

**Dvě slova, jako u podpříkazu [`italics`](italics.md)**: `--add` a `--remove` se navzájem vylučují a jedno z nich je nutné.

- **`--add`** přidá pomlčku před každý řádek, který žádnou nemá: z `Bonjour` se stane `- Bonjour` a
  z `Bonjour\nMarie` `- Bonjour\n- Marie`. **Pomlčka, která už tam je, se nezdvojí.**
- **Rozpoznávají se tři pomlčky** — spojovník z klávesnice a dvě dlouhé pomlčky sazečů —
  **a zapisuje se spojovník**: z `— Bonjour` se stane `- Bonjour`. Nadbytečné mezery
  za pomlčkou zmizí.
- Pomlčka se umístí **před text, ne před otevírací značku**: `<i>Bonjour</i>` dá
  `<i>- Bonjour</i>`. Žádná značka se nehne.
- **`--remove`** odebere úvodní pomlčky, ve všech třech podobách.

<!-- example: subedit-cli dialogue-dashes --help -->
```console
$ subedit-cli dialogue-dashes --help
Put dialogue dashes on the lines, or take them off
Usage: subedit-cli dialogue-dashes [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --add                       Put a dialogue dash at the head of the lines
  --remove                    Take the dialogue dashes off
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
| `--add` / `--remove` | **jedna ze dvou** | přepínače | — |
| `--range` | ne | `N-M` nebo `N-` až do konce — viz [`adjust`](adjust.md#--range) | celý soubor |
| `--document` | ne | `main` nebo `translation` — viz [Překlad](invocation.md#překlad) | `main` |
| `-t`, `--translation-file` | s `--document translation` | soubor překladu, pro **jediný** vstup | — |
| `--align-method` | ne, a jen s `-t` | `position` nebo `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **jedna ze tří**, kromě `--dry-run` | viz [Spuštění](invocation.md#cíl-zápisu) | — |
| `--dry-run` | ne | přepínač: spočítá a vypíše, nic nezapíše — viz [Náhled před zápisem](invocation.md#náhled-před-zápisem) | vypnuto |

**Ve výchozím stavu hlavní text; překlad s `--document translation -t FILE`.** Mění a zapisuje se
pak překlad, a jen on — viz [Překlad](invocation.md#překlad).

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\nMarie\n\n2\n00:00:03,000 --> 00:00:04,000\n<i>Salut</i>\n\n' > a.srt; subedit-cli dialogue-dashes --add --output t.srt a.srt; cat t.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\nMarie\n\n2\n00:00:03,000 --> 00:00:04,000\n<i>Salut</i>\n\n' > a.srt; subedit-cli dialogue-dashes --add --output t.srt a.srt; cat t.srt
a.srt: 2 subtitles dashed -> t.srt
1
00:00:01,000 --> 00:00:02,000
- Bonjour
- Marie

2
00:00:03,000 --> 00:00:04,000
<i>- Salut</i>
```

## Výstup

**Standardní výstup** — nic: výsledkem je zapsaný soubor. S `--dry-run` výpis
změn popsaný v oddílu [Náhled před zápisem](invocation.md#náhled-před-zápisem).

| Úroveň | Co se přidá na chybový výstup |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles dashed -> <destination>`, nebo `N subtitles undashed`, nebo `nothing to change` |
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
| ani `--add`, ani `--remove` | `dialogue-dashes needs --add to put dashes on, or --remove` |
| obě | `--add and --remove say opposite things; give one of them` |
| rozsah špatně zapsaný nebo mimo soubor | viz [`adjust`](adjust.md#--range) |

Chyby cíle zápisu jsou společné podpříkazům, které zapisují:
viz [Spuštění](invocation.md#cíl-zápisu).
