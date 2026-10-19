# `join-repeats`

<!-- machine-translated -->
> Tato stránka je strojovým překladem a dosud ji nezkontroloval žádný čtenář tohoto jazyka.

```
subedit-cli join-repeats (--output FILE | --output-dir DIR | --in-place)
                         [--dry-run]
                         [--recursive]
                         <file>...
```

Udělá **jediný titulek** z titulků, které po sobě ukazují totéž: z textu rozděleného na kusy,
které po sobě následují, jak ho zanechávají některé segmentační nástroje. Jde o `Join Repeated Subtitles` okna —
[týž mechanismus](../subedit-gui/lines.md#spojení-opakovaných-titulků).

**Opakování** je titulek, který má **stejný text, stejný překlad a stejná nastavení formátu** (styl,
pozici, nastavení podnětu) jako jiný titulek **a který začíná nejpozději tehdy, když ten druhý končí**:
dotýkají se nebo se překrývají. Série opakování se stane **tím, který začíná první**, jehož konec se
přenese na nejpozdější konec série; ostatní zmizí.

- **Na pořadí souboru nezáleží a nemění se**: ponechaný titulek je ten, který začíná první,
  ať jsou ostatní kdekoli. Seřazený soubor zůstane seřazený.
- **Nic se nespojí omylem**: jiný text, jiný překlad, jiná nastavení, mezera mezi dvěma
  kusy, titulek bez textu nebo titulek, jehož konec přichází před začátkem — ve všech těchto případech zůstanou oddělené.
- **Provádí se před ostatním**: seřazení a [rozdělení překryvů](disjoint.md) čtou čistou sérii lépe.
  [`repair`](repair.md) řetězí všechny tři a [`convert`](convert.md#převedení-rozsahů-na-disjunktní) ho přijímá jako volbu (`--join-repeats`, `--repair`).

<!-- example: subedit-cli join-repeats --help -->
```console
$ subedit-cli join-repeats --help
Join the subtitles that repeat each other, one after the other, into one
Usage: subedit-cli join-repeats [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to join the repeats of

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
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
| `--output` / `--output-dir` / `--in-place` | **jedna ze tří**, kromě `--dry-run` | viz [Spuštění](invocation.md#cíl-zápisu) | — |
| `--dry-run` | ne | přepínač: spočítá a vypíše, nic nezapíše — viz [Náhled před zápisem](invocation.md#náhled-před-zápisem) | vypnuto |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:04,000 --> 00:00:06,000\nAlpha\n\n4\n00:00:08,000 --> 00:00:09,000\nBeta\n\n' > morceaux.srt; subedit-cli join-repeats --output joint.srt morceaux.srt; cat joint.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:04,000 --> 00:00:06,000\nAlpha\n\n4\n00:00:08,000 --> 00:00:09,000\nBeta\n\n' > morceaux.srt; subedit-cli join-repeats --output joint.srt morceaux.srt; cat joint.srt
morceaux.srt: 2 repeated subtitles joined -> joint.srt
1
00:00:01,000 --> 00:00:06,000
Alpha

2
00:00:08,000 --> 00:00:09,000
Beta
```

Tři kusy `Alpha`, které po sobě následují od 1 s do 6 s, nyní tvoří jen jeden. `Beta`, oddělená mezerou,
zůstala. Druhý průchod nic nezmění a řekne to: `no repeated subtitles`.

## Výstup

**Standardní výstup** — nic: výsledkem je zapsaný soubor.

| Úroveň | Co se přidá na chybový výstup |
| :----- | :------------------------------------- |
| 1 | `<path>: N repeated subtitles joined -> <destination>`, nebo `<path>: no repeated subtitles -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, pak **každé hlášení při čtení** |

### Ve formátu JSON

S [`--format json`](invocation.md#výstup-čitelný-pro-skript) jsou `counts` každého souboru: `subtitles` (titulky zapsaného
souboru), `joined` (titulky odstraněné, protože je pokrývá jiný) a `chains` (spojené série).

## Návratové kódy

Kódy nástroje: `0`, pokud byly zpracovány všechny soubory — soubor bez opakování zpracovaný je —, `2`, pokud žádný, `3`, pokud
jen některé, `1` při chybě použití.

## Chyby

Žádné vlastní podpříkazu `join-repeats`: nečitelný soubor selže jako u každého podpříkazu a chyby
cíle zápisu jsou společné podpříkazům, které zapisují: viz [Spuštění](invocation.md#cíl-zápisu).
