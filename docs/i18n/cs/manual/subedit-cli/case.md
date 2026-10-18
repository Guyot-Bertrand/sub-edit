# `case`

<!-- machine-translated -->
> Tato stránka je strojovým překladem a dosud ji nezkontroloval žádný čtenář tohoto jazyka.

```
subedit-cli case --to title|sentence|upper|lower [--range N-M|N-]
                 (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 [--document main|translation] [-t FILE] [--align-method position|number]
                 <file>...
```

Převede texty na velikost písmen **jako v nadpisu, jako ve větě, na velká nebo na malá písmena**, aniž by se
dotkl značky. Jde o `Nástroje ▸ Velikost písmen` [okna](../subedit-gui/operations.md), s jeho pravidly.

| `--to` | Účinek | Z `bonjour marie` se stane |
| :----- | :---- | :---------------------- |
| `title` | každé slovo dostane velké písmeno, zbytek se zmenší | `Bonjour Marie` |
| `sentence` | první písmeno textu, a jen ono; zbytek se zmenší | `Bonjour marie` |
| `upper` | vše velkými písmeny | `BONJOUR MARIE` |
| `lower` | vše malými písmeny | `bonjour marie` |

- **Čeho před prvním písmenem nebo první číslicí je, se nikdo nedotkne**: dialogová
  pomlčka, otevírací uvozovka nebo závorka přežijí (`- bonjour` dá `- Bonjour`).
- **Značky nejsou text**: nemění velikost písmen — `<I>` zůstane `<I>` — a nemění
  místo: značka, která rozděluje slovo, ho rozděluje i potom.
- **Kde slovo začíná, rozhoduje ICU**: `l'été sera chaud` dá `L'été Sera Chaud` a diakritika
  vydrží (`éléonore` dá `Éléonore`).

<!-- example: subedit-cli case --help -->
```console
$ subedit-cli case --help
Put the texts in title, sentence, upper or lower case, tags intact
Usage: subedit-cli case [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --to title|sentence|upper|lower
                              The case to put the texts in
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
| `--to` | **ano** | `title`, `sentence`, `upper`, `lower` | — |
| `--range` | ne | `N-M` nebo `N-` až do konce — viz [`adjust`](adjust.md#--range) | celý soubor |
| `--document` | ne | `main` nebo `translation` — viz [Překlad](invocation.md#překlad) | `main` |
| `-t`, `--translation-file` | s `--document translation` | soubor překladu, pro **jediný** vstup | — |
| `--align-method` | ne, a jen s `-t` | `position` nebo `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **jedna ze tří**, kromě `--dry-run` | viz [Spuštění](invocation.md#cíl-zápisu) | — |
| `--dry-run` | ne | přepínač: spočítá a vypíše, nic nezapíše — viz [Náhled před zápisem](invocation.md#náhled-před-zápisem) | vypnuto |

**Ve výchozím stavu hlavní text; překlad s `--document translation -t FILE`.** Mění a zapisuje se
pak překlad, a jen on — viz [Překlad](invocation.md#překlad).

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\n- BONJOUR MARIE\n\n' > a.srt; subedit-cli case --to title --output titre.srt a.srt; cat titre.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\n- BONJOUR MARIE\n\n' > a.srt; subedit-cli case --to title --output titre.srt a.srt; cat titre.srt
a.srt: 2 subtitles recased -> titre.srt
1
00:00:01,000 --> 00:00:02,000
<i>Bon</i>jour Marie

2
00:00:03,000 --> 00:00:04,000
- Bonjour Marie
```

Soubor, v němž se **žádný text nezmění** — vše už má požadovanou velikost písmen —, se zapíše, jak je, a
řekne to:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBONJOUR\n\n' > a.srt; subedit-cli case --to upper --output b.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBONJOUR\n\n' > a.srt; subedit-cli case --to upper --output b.srt a.srt
a.srt: nothing to change -> b.srt
```

## Výstup

**Standardní výstup** — nic: výsledkem je zapsaný soubor. S `--dry-run` výpis
změn popsaný v oddílu [Náhled před zápisem](invocation.md#náhled-před-zápisem).

| Úroveň | Co se přidá na chybový výstup |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles recased -> <destination>`, nebo `nothing to change` |
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
| chybí `--to` | `--to is required` |
| hodnota, která není jednou ze čtyř | `--to: … not in {title,sentence,upper,lower}` |
| rozsah špatně zapsaný nebo mimo soubor | viz [`adjust`](adjust.md#--range) |

Chyby cíle zápisu jsou společné podpříkazům, které zapisují:
viz [Spuštění](invocation.md#cíl-zápisu).
