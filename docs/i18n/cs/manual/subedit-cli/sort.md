# `sort`

<!-- machine-translated -->
> Tato stránka je strojovým překladem a dosud ji nezkontroloval žádný čtenář tohoto jazyka.

```
subedit-cli sort (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 <file>...
```

Vrátí titulky do **pořadí podle jejich začátku**. Nic jiného se nezmění: ani texty, ani
pozice, ani formát souboru. Titulky se při zápisu **přečíslují** od 1,
jako každý soubor zapsaný zpět.

**Jádro nikdy neřadí samo od sebe**: čtení souboru ponechá jeho pořadí, jak bylo, a
[`inspect`](inspect.md#co-hlásí-anomalies) ohlásí porušení pořadí hlášením
`subtitle N starts before the previous one starts`. `sort` je **úkon provedený záměrně**, pro soubor
nebo dávku s porušeným pořadím — a v okně ho lze vrátit zpět jako každou operaci.

- **Řazení je stabilní**: dva titulky, které začínají ve stejném okamžiku, si **zachovají pořadí, které
  jim dal soubor**. Žádný není před druhým; jejich přesunutí by bylo rozhodnutím, o které nikdo
  nežádal.
- **`sort` nepřijímá ani rozsah, ani dokument**: není co vybírat, pořadí je pořadím začátků.

<!-- example: subedit-cli sort --help -->
```console
$ subedit-cli sort --help
Put the subtitles in the order of their start, keeping ties as they are
Usage: subedit-cli sort [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to sort

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

<!-- example: printf '1\n00:00:05,000 --> 00:00:06,000\ncinq\n\n2\n00:00:01,000 --> 00:00:02,000\nun\n\n3\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > desordre.srt; subedit-cli sort --output ordre.srt desordre.srt; cat ordre.srt -->
```console
$ printf '1\n00:00:05,000 --> 00:00:06,000\ncinq\n\n2\n00:00:01,000 --> 00:00:02,000\nun\n\n3\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > desordre.srt; subedit-cli sort --output ordre.srt desordre.srt; cat ordre.srt
desordre.srt: 3 subtitles moved -> ordre.srt
1
00:00:01,000 --> 00:00:02,000
un

2
00:00:03,000 --> 00:00:04,000
trois

3
00:00:05,000 --> 00:00:06,000
cinq
```

## Co se počítá

**Počítají se místa, na nichž se titulek změnil**, tak jak to hlásí jádro — to, co by
historie okna nazvala přesunutým. Jediný titulek na nesprávném místě posune několik dalších:
ve výše uvedeném příkladu si žádné ze tří míst nezachová svůj titulek a věta říká `3 subtitles moved`.

**Soubor, který už seřazený je, se přesto zapíše** a řekne to: zadaný cíl zápisu je
cíl, do kterého se zapisuje, a udělat ze `sort` výjimku by nutilo skript vědět, které
podpříkazy někdy vytvoří soubor a někdy nic.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > ordre.srt; subedit-cli sort --output copie.srt ordre.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > ordre.srt; subedit-cli sort --output copie.srt ordre.srt
ordre.srt: already in order -> copie.srt
```

## Výstup

**Standardní výstup** — nic: výsledkem je zapsaný soubor. S `--dry-run` také nic: žádný
soubor neexistuje a řádek výpisu to říká — viz [Náhled před zápisem](invocation.md#náhled-před-zápisem).

| Úroveň | Co se přidá na chybový výstup |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles moved -> <destination>`, nebo `<path>: already in order -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — formát, kódování, značka a konce řádků přečteného souboru, vrácené tak, jak byly |
| 3 | `<path>: N bytes read, M written`, pak **každé hlášení při čtení** — viz [Spuštění](invocation.md#hlášení-při-čtení) |

### Ve formátu JSON

S [`--format json`](invocation.md#výstup-čitelný-pro-skript) nese objekt každého souboru
`counts.subtitles` (všechny titulky souboru) a `counts.moved` (místa, která se změnila;
`0` u souboru, který už seřazený je).

## Návratové kódy

Kódy nástroje: `0`, pokud byly zpracovány všechny soubory — soubor, který už seřazený je, zpracovaný je —, `2`, pokud
žádný, `3`, pokud jen některé, `1` při chybě použití.

## Chyby

Žádné vlastní podpříkazu `sort`: nečitelný soubor selže jako u každého podpříkazu a
chyby cíle zápisu jsou společné podpříkazům, které zapisují:
viz [Spuštění](invocation.md#cíl-zápisu).
