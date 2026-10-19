# `framerate`

<!-- machine-translated -->
> Tato stránka je strojovým překladem a dosud ji nezkontroloval žádný čtenář tohoto jazyka.

> **Toto není [`snap`](snap.md).** Tento příkaz **mění měřítko
> souboru**: slouží pro případ, kdy je časování chybné a ke konci filmu ujíždí
> o několik sekund. `snap` vrací časové údaje na nejbližší
> mřížku, aniž by cokoli přečasoval, a slouží pro případ, kdy je časování už
> správné. Oba mají stejný tvar, jednu nebo dvě snímkové frekvence, a jejich záměna nezpůsobí
> žádnou chybu.

```
subedit-cli framerate --from <fps> --to <fps>
                      (--output FILE | --output-dir DIR | --in-place)
                      [--dry-run]
                      [--recursive]
                      <file>...
```

Přečasuje soubor časovaný pro jednu snímkovou frekvenci na jinou. Je to
operace, kterou si žádá soubor titulků pořízený pro kopii, která
není kopií filmu, kterou máte — například DVD PAL proti přepisu NTSC.

Formát přečteného souboru se **zachová**: změna formátu je práce podpříkazu
[`convert`](convert.md).

<!-- example: subedit-cli framerate --help -->
```console
$ subedit-cli framerate --help
Re-time a file mastered at one frame rate for another
Usage: subedit-cli framerate [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to re-time

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --from TEXT REQUIRED        Frame rate the file is timed at: 25, 23.976
  --to TEXT REQUIRED          Frame rate to time it for: 24, 29.97
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
| `--from` | **ano** | snímková frekvence, viz níže | — |
| `--to` | **ano** | snímková frekvence, viz níže | — |
| `--output` / `--output-dir` / `--in-place` | **jedna ze tří**, kromě `--dry-run` | viz [Spuštění](invocation.md#cíl-zápisu) | — |
| `--dry-run` | ne | přepínač: spočítá a vypíše, nic nezapíše — viz [Náhled před zápisem](invocation.md#náhled-před-zápisem) | vypnuto |

`--from` je snímková frekvence, pro kterou soubor **je** časován, `--to` ta, pro kterou
ho chcete mít. Jejich prohození posune soubor
opačným směrem — u celovečerního filmu mezi 25 a
23,976 zhruba o sedm sekund — a nic na to neupozorní, dokud se film nepustí.

## Zápis snímkové frekvence

Ve snímcích za sekundu, včetně desetinných míst, s tečkou jako oddělovačem:

```
25      24      23.976      29.97      60
```

**Nulová nebo záporná snímková frekvence se odmítne**: snímkovou frekvencí není.

### Tři snímkové frekvence NTSC nelze zapsat přesně

`23.976`, `29.97` a `59.94` jsou **označení**. Snímkové frekvence, které
označují, jsou `24000/1001`, `30000/1001` a `60000/1001`, z nichž žádná nemá
konečný desetinný zápis. Nástroj je jako takové rozpoznává:

| Co se zapíše | Co označuje |
| :------------- | :------------------ |
| `23.976` | `24000/1001` — film přepsaný na video NTSC |
| `24` | `24` — film |
| `25` | `25` — video PAL a SECAM |
| `29.97` | `30000/1001` — video NTSC |
| `30`, `50`, `60` | samy sebe |
| `59.94` | `60000/1001` |

Rozpoznání se týká **hodnoty, a ne znaků**: `23.9760`
označuje stejnou snímkovou frekvenci jako `23.976`.

Jakékoli jiné desetinné číslo se bere doslova — `23.9` není ničí
normou, není v něm co hledat. Právě to také činí výše uvedenou tabulku
nezbytnou: bez ní by `23.976` bylo `23976/1000` a příkazový řádek
by pro stejná slova říkal něco jiného než okno.

## Co se stane s pozicemi

```
t′ = round( t × (from / to) )
```

Film masterovaný na 23,976 a přehrávaný při 25 snímcích za sekundu běží rychleji,
takže jeho titulky přicházejí dříve; opačným směrem přicházejí
později.

**Měřítko je přesné racionální číslo a zaokrouhluje se jen jednou.** Cesta,
která vypadá rovnocenně — převést pozici na číslo snímku při
vstupní frekvenci a pak zpět na milisekundy při výstupní frekvenci —,
rovnocenná není: kvantuje každou pozici na vstupní mřížce a chybuje až o
půl snímku, naměřeno 21 ms. Viz
[ADR 0013](../../../../adr/0013-mise-a-l-echelle-exacte-des-positions.md).

V praxi je z 25 na 23,976 měřítko `1001/960`:

| Přečtená pozice | Zapsaná | Co by dal průchod přes snímky |
| -----------: | -----: | -----------------------------------------: |
| `1 010 ms` | `1 053 ms` | `1 043 ms` |
| `3 600 017 ms` | `3 753 768 ms` | `3 753 750 ms` |

<!-- example: printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 25 --to 23.976 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 25 --to 23.976 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle retimed from 25 to 24000/1001 fps -> b.srt
1
00:00:01,053 --> 00:00:02,106
Un.
```

## Výstup

**Standardní výstup** — nic: výsledkem je zapsaný soubor. S `--dry-run` také nic: žádný soubor neexistuje a řádek výpisu to říká — viz [Náhled před zápisem](invocation.md#náhled-před-zápisem).

| Úroveň | Co se přidá na chybový výstup |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles retimed from 25 to 24000/1001 fps -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — formát, kódování, značka a konce řádků přečteného souboru, vrácené tak, jak byly |
| 3 | `<path>: N bytes read, M written`, pak **každé hlášení při čtení** — viz [Spuštění](invocation.md#hlášení-při-čtení) |

**Výpis jmenuje přesnou snímkovou frekvenci, ne zadané označení.** Zápis
`23.976` na tomto řádku by ohlásil převod, který neproběhl; je to
jediné místo, kde je vidět, která z obou byla použita. Snímková frekvence, kterou
desetinné číslo zapíše přesně, se tam zobrazí jako desetinné číslo, ostatní jako zlomek.

### Ve formátu JSON

S [`--format json`](invocation.md#výstup-čitelný-pro-skript) nese objekt každého souboru `counts.subtitles` (všechny titulky souboru). Úplná tabulka je na stránce [Spuštění](invocation.md#co-který-podpříkaz-uvádí-v-counts).

## Návratové kódy

Kódy nástroje: `0`, pokud byly zapsány všechny soubory, `2`, pokud žádný, `3`, pokud
jen některé, `1` při chybě použití.

Žádné odmítnutí nezávisí na obsahu souboru: dvě snímkové frekvence vždy určují
měřítko. Je to jediný z podpříkazů, které zapisují, jehož všechna
odmítnutí přicházejí před prvním čtením.

<!-- example: printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 0 --to 25 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 0 --to 25 --output b.srt a.srt; echo "code=$?"
"0" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed
code=1
```

## Chyby

| Co to způsobí | Hlášení |
| :------------------ | :------ |
| nulová nebo záporná snímková frekvence | `"…" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed` |
| desetinná čárka | `"…" is not a frame rate: use a decimal point and not a comma, in every language` |
| text, který není číslem | `"…" is not a frame rate: expected frames per second, like 25 or 23.976` |
| nepřiměřená snímková frekvence | `"…" is not a frame rate: no video runs at that many frames per second` |

**Bez jedné ze tří voleb cíle zápisu se nic nezapíše** a jejich
chyby jsou společné podpříkazům, které zapisují:
viz [Spuštění](invocation.md#cíl-zápisu).
