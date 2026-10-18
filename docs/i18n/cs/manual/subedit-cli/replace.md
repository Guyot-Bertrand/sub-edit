# `replace`

<!-- machine-translated -->
> Tato stránka je strojovým překladem a dosud ji nezkontroloval žádný čtenář tohoto jazyka.

```
subedit-cli replace [--regex] [--case-sensitive] [--range N-M|N-]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    <pattern> <replacement> <file>...
```

Nahradí text v titulcích, **aniž by porušil značku**. Jde o operaci
`Replace All` [hledání](../subedit-gui/find-and-replace.md) v okně, s jejími pravidly:

- **hledá se ve viditelném textu** — v tom, co čte divák. Značky do něj
  nepatří: hledání `<i>` nic nenajde a hledání `Bonjour` najde „Bonjour“, jehož polovina
  je kurzívou;
- **nahrazuje se ve zdrojovém textu**, tedy v textu souboru, a značky zůstávají tam, kde
  byly. Značka, která rozdělí hledané slovo, obklopí celou náhradu; styl, který
  se dotýká shody, pokryje celou náhradu; čeho se shoda nedotýká,
  to se nehne. Pravidla případ od případu jsou pravidla hledání v okně.

**Žádný podpříkaz `find` neexistuje**: chcete-li vidět, kde se vzor nachází, aniž byste cokoli měnili,
vypíše každý dotčený titulek [`--dry-run`](invocation.md#náhled-před-zápisem).

<!-- example: subedit-cli replace --help -->
```console
$ subedit-cli replace --help
Replace a text in the subtitles, without breaking a tag
Usage: subedit-cli replace [OPTIONS] pattern replacement files...

Positionals:
  pattern TEXT REQUIRED       Text to look for in what is shown
  replacement TEXT REQUIRED   What replaces it, tags allowed
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --regex                     Read the text to look for as a regular expression
  --case-sensitive            Tell capitals from small letters
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
| `<pattern>` | ano | text, který se hledá v tom, co je zobrazeno; **nesmí být prázdný** | — |
| `<replacement>` | ano | co ho nahradí, značky povoleny; může být prázdné | — |
| `<file>...` | ano | jedna nebo více cest | — |
| `--recursive`, `-r` | ne | přepínač | vypnuto — viz [Zpracování stromu](lots.md) |
| `--regex` | ne | přepínač: vzor je regulární výraz ICU | prostý text |
| `--case-sensitive` | ne | přepínač: rozlišuje velká a malá písmena | nerozlišují se |
| `--range` | ne | `N-M` nebo `N-` až do konce — viz [`adjust`](adjust.md#--range) | celý soubor |
| `--document` | ne | `main` nebo `translation` — viz [Překlad](invocation.md#překlad) | `main` |
| `-t`, `--translation-file` | s `--document translation` | soubor překladu, pro **jediný** vstup | — |
| `--align-method` | ne, a jen s `-t` | `position` nebo `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **jedna ze tří**, kromě `--dry-run` | viz [Spuštění](invocation.md#cíl-zápisu) | — |
| `--dry-run` | ne | přepínač: spočítá a vypíše, nic nezapíše — viz [Náhled před zápisem](invocation.md#náhled-před-zápisem) | vypnuto |

**Vzor, který začíná pomlčkou**, se zapíše za `--`: `subedit-cli replace -- -- "—" film.srt`.

**Ve výchozím stavu hlavní text; překlad s `--document translation -t FILE`.** Mění a zapisuje se
pak překlad, a jen on — viz [Překlad](invocation.md#překlad).

### Prostý text, nebo výraz

Ve výchozím stavu se vzor **čte tak, jak je**: `a.b` hledá `a`, tečku a `b`. Velikost
písmen se nerozlišuje se stejnou jemností jako v okně — `É` a `é` jsou jediné písmeno.

S `--regex` je vzor regulárním výrazem **ICU**, čteným tak, jak ho čte Gaupol: `.` přechází
přes zalomení řádku, `^` a `$` platí na každém řádku titulku. V náhradě, a
jen tam:

| Zapište | Chcete-li říct |
| :----- | :-------- |
| `$1` až `$9` | skupiny shody |
| `$0` | celou shodu |
| `\n` | zalomení řádku |
| `\$`, `\\` | znak `$`, zpětné lomítko |

Pozor na uvozovky shellu: `'$1 EUR'`, a ne `"$1 EUR"`, které by shellu dovolily přečíst `$1`.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nPrix: 12 euros, et 7 euros.\n\n' > prix.srt; subedit-cli replace --regex '([0-9]+) euros' '$1 EUR' --output euros.srt prix.srt; cat euros.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nPrix: 12 euros, et 7 euros.\n\n' > prix.srt; subedit-cli replace --regex '([0-9]+) euros' '$1 EUR' --output euros.srt prix.srt; cat euros.srt
prix.srt: replaced 2 matches -> euros.srt
1
00:00:01,000 --> 00:00:02,000
Prix: 12 EUR, et 7 EUR.
```

## Značky

Text, v němž značka rozděluje hledané slovo:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace Bonjour Salut --output salut.srt balises.srt; cat salut.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace Bonjour Salut --output salut.srt balises.srt; cat salut.srt
balises.srt: replaced 2 matches -> salut.srt
1
00:00:01,000 --> 00:00:02,000
<i>Salut</i> tout le monde

2
00:00:03,000 --> 00:00:04,000
Salut <i>Marie</i>, ca va
```

V prvním titulku značka rozdělila „Bonjour“: nyní obklopuje „Salut“.
Ve druhém se jí nic nedotklo a zůstává tam, kde je. **Neuzavřená značka
zůstane neuzavřená**: oprava vratkého značkování by byla jiná operace.

Náhrada se čte ve **značkách formátu souboru**: `<i>…</i>` pro SubRip,
`{\i1}…` pro Advanced SSA. Je to jediný způsob, jak naformátovat, co zapisujete.

## Nic nenalezeno, nic ke změně

Dvě odpovědi, které nejsou selháním — návratový kód je `0` a **soubor se přesto
zapíše**, protože zadaný cíl zápisu je cíl, do kterého se zapisuje:

| Situace | Věta |
| :-------- | :----- |
| vzor se nikde nevyskytuje | `"zzz" not found` |
| vyskytuje se, ale každá shoda se nahradí sama sebou | `nothing to change` |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli replace zzz x --output b.srt a.srt; subedit-cli replace Bonjour Bonjour --output c.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli replace zzz x --output b.srt a.srt; subedit-cli replace Bonjour Bonjour --output c.srt a.srt
a.srt: "zzz" not found -> b.srt
a.srt: nothing to change -> c.srt
```

## `--dry-run`

Standardní výstup vypíše **každý titulek, jehož text by se změnil**, před a po, tak jak
jsou v souboru — včetně značek:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace --dry-run Bonjour Salut balises.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace --dry-run Bonjour Salut balises.srt
balises.srt: replaced 2 matches (dry run, nothing written)
balises.srt: subtitle 1
- <i>Bon</i>jour tout le monde
+ <i>Salut</i> tout le monde
balises.srt: subtitle 2
- Bonjour <i>Marie</i>, ca va
+ Salut <i>Marie</i>, ca va
```

Tvar výpisu popisuje [Náhled před zápisem](invocation.md#náhled-před-zápisem);
ve formátu JSON je to `changes`.

## Výstup

**Standardní výstup** — nic: výsledkem je zapsaný soubor. S `--dry-run` výše uvedený výpis
změn.

| Úroveň | Co se přidá na chybový výstup |
| :----- | :------------------------------------- |
| 1 | `<path>: replaced N matches -> <destination>`, nebo `"<pattern>" not found`, nebo `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — formát, kódování, značka a konce řádků přečteného souboru, vrácené tak, jak byly |
| 3 | `<path>: N bytes read, M written`, pak **každé hlášení při čtení** — viz [Spuštění](invocation.md#hlášení-při-čtení) |

`N` počítá **shody nahrazené v titulcích, jejichž text se změnil** — je to také
číslo, které uvádí okno.

### Ve formátu JSON

S [`--format json`](invocation.md#výstup-čitelný-pro-skript) nese objekt každého souboru:

| Klíč | Obsah |
| :-- | :------ |
| `counts.replaced` | shody nahrazené v titulcích, jejichž text se změnil: `N` z věty |
| `counts.matched` | nalezené shody, **ať jejich náhrada text změnila, nebo ne** — tím se liší „nic nenalezeno“ (`0`) od „nic ke změně“ (`replaced` na `0`, `matched` kladné) |
| `changes` | změněné titulky: `{"subtitle", "document", "before", "after"}`, **prázdné**, když se nic nezměnilo |

## Návratové kódy

Kódy nástroje: `0`, pokud byly zpracovány všechny soubory — soubor, v němž se nic nenašlo,
zpracovaný je —, `2`, pokud žádný, `3`, pokud jen některé, `1` při chybě použití.

## Chyby

| Co to způsobí | Hlášení |
| :------------------ | :------ |
| prázdný vzor | `pattern: nothing to look for` |
| nečitelný výraz | `pattern: not a regular expression (<ICU's reason>)` |
| rozsah špatně zapsaný nebo mimo soubor | viz [`adjust`](adjust.md#--range) |

**Vzor se čte před prvním souborem**: nečitelný výraz je chyba použití
(kód `1`), nic se nepřečte ani nezapíše.

Chyby cíle zápisu jsou společné podpříkazům, které zapisují:
viz [Spuštění](invocation.md#cíl-zápisu).
