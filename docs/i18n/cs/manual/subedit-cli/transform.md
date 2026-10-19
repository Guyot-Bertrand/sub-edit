# `transform`

<!-- machine-translated -->
> Tato stránka je strojovým překladem a dosud ji nezkontroloval žádný čtenář tohoto jazyka.

```
subedit-cli transform --first <index>=<time> --last <index>=<time>
                      (--output FILE | --output-dir DIR | --in-place)
                      [--dry-run]
                      [--recursive]
                      <file>...
```

Opraví **každou** pozici souboru podle dvou bodů, jejichž správná hodnota
je známa. Je to operace, kterou si žádá ujíždějící titulkování
— na začátku správné, na konci chybné —, kde [`shift`](shift.md), který posouvá vše o
stejnou hodnotu, nic nezmůže.

Formát přečteného souboru se **zachová**: změna formátu je práce podpříkazu
[`convert`](convert.md).

<!-- example: subedit-cli transform --help -->
```console
$ subedit-cli transform --help
Correct every position from two points known to be right
Usage: subedit-cli transform [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to transform

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --first TEXT REQUIRED       Earlier reference, as <index>=<time>: 1=00:00:01.000
  --last TEXT REQUIRED        Later reference, as <index>=<time>: 3=00:00:10.000
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
| `--first` | **ano** | referenční bod, `<index>=<time>` | — |
| `--last` | **ano** | referenční bod, `<index>=<time>` | — |
| `--output` / `--output-dir` / `--in-place` | **jedna ze tří**, kromě `--dry-run` | viz [Spuštění](invocation.md#cíl-zápisu) | — |
| `--dry-run` | ne | přepínač: spočítá a vypíše, nic nezapíše — viz [Náhled před zápisem](invocation.md#náhled-před-zápisem) | vypnuto |

Krátký tvar neexistuje: tyto dvě volby se zapisují jen celé.

## Zápis referenčního bodu

Referenční bod jmenuje titulek a říká, kam má nyní připadnout jeho **začátek**:

```
<index>=<time>
```

**Index se počítá od 1**, tak jak se zobrazuje a jak ho čísluje soubor
SubRip. `0` se odmítne, stejně jako index za posledním
titulkem.

**Čas se zapisuje jako u podpříkazu [`shift`](shift.md)** — sekundy nebo
časový údaj, desetinným oddělovačem je tečka, rozlišením milisekunda:

| Tvar | Příklady |
| :---- | :------- |
| sekundy | `10`, `2.999` |
| časový údaj | `00:00:10.000`, `01:39:37.040` |

`--first 3=10` a `--first 3=00:00:10.000` říkají totéž.

`--first` a `--last` jmenují dva body, ne interval. Jejich názvy vyjadřují
očekávané použití — jeden referenční bod na začátku souboru, druhý na konci —, nikoli
omezení: jejich prohození dá stejný výsledek a **vše mimo
oba referenční body se řídí stejně jako to, co je mezi nimi**.

## Co se stane s ostatními pozicemi

Oba referenční body určují měřítko a každá pozice se jím řídí:

```
t′ = y₁ + (t − x₁) × r      with r = (y₂ − y₁) / (x₂ − x₁)
```

`x₁`, `x₂` jsou současné začátky obou jmenovaných titulků, `y₁`, `y₂`
požadované časy. Měřítko se uchovává jako přesné racionální číslo a **zaokrouhluje
se jen jednou**, na milisekundu; díky tomu oba referenční body připadnou
**přesně** na požadované časy, a ne s přesností na tisícinu
(viz [ADR 0013](../../../../adr/0013-mise-a-l-echelle-exacte-des-positions.md)).

Každý titulek zůstane na obrazovce **úměrně** tomu, jak dlouho tam zůstával:
konec se řídí stejným měřítkem jako začátek. Transformace není
posun a nemá jeho vlastnost.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,001 --> 00:00:07,000\nDeux.\n\n3\n00:00:09,000 --> 00:00:11,000\nTrois.\n\n' > a.srt; subedit-cli transform --first 1=00:00:01.000 --last 3=00:00:10.000 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,001 --> 00:00:07,000\nDeux.\n\n3\n00:00:09,000 --> 00:00:11,000\nTrois.\n\n' > a.srt; subedit-cli transform --first 1=00:00:01.000 --last 3=00:00:10.000 --output b.srt a.srt; cat b.srt
a.srt: 3 subtitles transformed onto 1=00:00:01.000 and 3=00:00:10.000 -> b.srt
1
00:00:01,000 --> 00:00:03,250
Un.

2
00:00:05,501 --> 00:00:07,750
Deux.

3
00:00:10,000 --> 00:00:12,250
Trois.
```

První referenční bod se nehne, třetí titulek přejde z 9 s na 10 s: měřítko je
`9/8` a druhý titulek — který nikdo nejmenoval — přejde z
`5,001` na `5,501`, tedy `1000 + 4001 × 9/8` zaokrouhleno jednou.

## Dva referenční body, které jimi nejsou

| Situace | Kdy se to zjistí | Kód |
| :-------- | :------------- | :--- |
| `--first` a `--last` jmenují stejný index | před jakýmkoli čtením | `1` |
| index je za posledním titulkem | při otevření souboru | `2` nebo `3` |
| oba jmenované titulky začínají ve stejném okamžiku | při otevření souboru | `2` nebo `3` |

Všechny tři se odmítnou ze stejného důvodu: `x₂ − x₁` je nula a dva
splývající body neurčují žádnou přímku. Vrátit dělení nulou převlečené za
výsledek by bylo horší než odmítnout.

První případ je **chyba použití** — chybné jsou obě volby bez ohledu na
soubory, a patří se tedy jediné hlášení, ne jedno na soubor. Další
dva závisejí na otevřeném souboru, a počítají se proto jako selhání
zpracování.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n' > a.srt; subedit-cli transform --first 2=1.000 --last 2=4.000 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n' > a.srt; subedit-cli transform --first 2=1.000 --last 2=4.000 --output b.srt a.srt; echo "code=$?"
--first and --last both name subtitle 2, and two references on one subtitle define no transform
code=1
```

Index mimo meze se odmítne **s uvedením meze**, což zároveň říká,
co se dalo zapsat:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,000 --> 00:00:07,000\nDeux.\n\n' > a.srt; subedit-cli transform --first 1=1.000 --last 9=10.000 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,000 --> 00:00:07,000\nDeux.\n\n' > a.srt; subedit-cli transform --first 1=1.000 --last 9=10.000 --output b.srt a.srt; echo "code=$?"
a.srt: subtitle 9 is past the end: the file holds 2 subtitles
code=2
```

## Pozice před počátkem se odmítne

Jako u podpříkazu [`shift`](shift.md), a ze stejného důvodu: záporná pozice
se zapíše `-00:00:01,000`, což je tvar, který nedefinuje žádný z devíti formátů a nepřijme žádná
čtečka. Tento případ tu nastane, když titulek leží **před
prvním referenčním bodem** a měřítko ho posune za nulu. Nic se
nezapíše, kód je `2` a hlášení jmenuje vinný titulek.

## Výstup

**Standardní výstup** — nic: výsledkem je zapsaný soubor. S `--dry-run` také nic: žádný soubor neexistuje a řádek výpisu to říká — viz [Náhled před zápisem](invocation.md#náhled-před-zápisem).

| Úroveň | Co se přidá na chybový výstup |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles transformed onto 1=00:00:01.000 and 3=00:00:10.000 -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — formát, kódování, značka a konce řádků přečteného souboru, vrácené tak, jak byly |
| 3 | `<path>: N bytes read, M written`, pak **každé hlášení při čtení** — viz [Spuštění](invocation.md#hlášení-při-čtení) |

Referenční body se vypíší zpět s desetinnou tečkou, ať byly zadány jakkoli.

### Ve formátu JSON

S [`--format json`](invocation.md#výstup-čitelný-pro-skript) nese objekt každého souboru `counts.subtitles` (všechny titulky souboru). Úplná tabulka je na stránce [Spuštění](invocation.md#co-který-podpříkaz-uvádí-v-counts).

## Návratové kódy

Kódy nástroje: `0`, pokud byly zapsány všechny soubory, `2`, pokud žádný, `3`, pokud
jen některé, `1` při chybě použití.

## Chyby

| Co to způsobí | Hlášení |
| :------------------ | :------ |
| referenční bod bez `=` | `"…" is not a reference: write it <index>=<time>, as in 3=00:00:10.000` |
| referenčnímu bodu chybí jedna polovina | `"…" is missing one of its two halves: write it <index>=<time>, as in 3=00:00:10.000` |
| index `0` | `"0" is not a subtitle number: subtitles are counted from 1, as the file shows them` |
| index, který není celé číslo | `"…" is not a subtitle number: expected a whole number, counted from 1 as the file shows them` |
| neplatný čas | chyby podpříkazu [`shift`](shift.md) |
| `--first` a `--last` na stejném indexu | `--first and --last both name subtitle N, and two references on one subtitle define no transform` |
| index mimo meze | `<path>: subtitle N is past the end: the file holds M subtitles` |
| dva titulky začínající současně | `<path>: subtitles N and M start at the same moment, so they define no transform` |
| pozice před počátkem | `<path>: subtitle N would land before the origin, which no subtitle file can hold` |

**Bez jedné ze tří voleb cíle zápisu se nic nezapíše** a jejich
chyby jsou společné podpříkazům, které zapisují:
viz [Spuštění](invocation.md#cíl-zápisu).
