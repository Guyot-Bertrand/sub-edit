# `shift`

<!-- machine-translated -->
> Tato stránka je strojovým překladem a dosud ji nezkontroloval žádný čtenář tohoto jazyka.

```
subedit-cli shift (--by <time> | --to-grid)
                  (--output FILE | --output-dir DIR | --in-place)
                  [--dry-run]
                  [--recursive]
                  <file>...
```

Posune **každou** pozici souboru o stejnou hodnotu, jedním nebo
druhým směrem. Žádný titulek nezůstane na obrazovce déle ani kratší dobu:
právě tím je posun posunem.

<!-- example: subedit-cli shift --help -->
```console
$ subedit-cli shift --help
Move every position of a file by a fixed amount
Usage: subedit-cli shift [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to shift

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --by TEXT                   Amount to move by: 2.999, -7.001, or 00:00:07.001
  --to-grid                   Move by the amount that puts the positions back on their frame grid
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
| `--by` | **jedna ze dvou** | doba trvání se znaménkem, viz níže | — |
| `--to-grid` | **jedna ze dvou** | přepínač: hodnota se změří, viz níže | — |
| `--output` / `--output-dir` / `--in-place` | **jedna ze tří**, kromě `--dry-run` | viz [Spuštění](invocation.md#cíl-zápisu) | — |
| `--dry-run` | ne | přepínač: spočítá a vypíše, nic nezapíše — viz [Náhled před zápisem](invocation.md#náhled-před-zápisem) | vypnuto |

Formát přečteného souboru se **zachová**: změna formátu je práce podpříkazu
[`convert`](convert.md).

## `--to-grid`: hodnota změřená, nikoli zadaná

Soubor, jehož pozice leží na mřížce snímků **až na
konstantu**, byl posunut, a tuto konstantu lze změřit na samotných pozicích.
`--to-grid` ji změří a použije, aniž by ji kdokoli musel znát.

Hodnota je **vlastní každému souboru**: dva soubory posunuté ze stejné
mřížky o dvě různé hodnoty se každý vrátí o tu svou. Zapíše
se do výpisu průběhu spolu s nalezenou snímkovou frekvencí.

**Cesta je nejkratší.** Za polovinou snímku je nejbližší čárou mřížky
ta následující: oprava jde dopředu, místo aby se vracela zpět o
téměř celý snímek.

<!-- example: k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40 + 7)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > decale.srt; subedit-cli shift --to-grid --output recale.srt decale.srt -->
```console
$ k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40 + 7)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > decale.srt; subedit-cli shift --to-grid --output recale.srt decale.srt
decale.srt: 12 subtitles shifted by -0.007 s onto their 25 fps grid -> recale.srt
```

**Soubor, který neleží na žádné známé mřížce, se odmítne, a řekne se proč.** Není
k čemu se vrátit, a fáze změřená na šumu by soubor posunula
nahodile. Viz [`inspect`](inspect.md#co-hlásí-frame-rate-grid), abyste
věděli, co soubor dává, dříve než operaci zkusíte.

`--by` i `--to-grid` říkají, o kolik posunout: zadat je spolu
je chyba použití, a stejně tak nezadat ani jednu.

**`--to-grid` není [`snap`](snap.md).** Posune celý soubor
o stejnou hodnotu a zachová relativní časování **přesně**; `snap`
posouvá každou pozici nezávisle a cestou pohltí ty, které byly
opraveny ručně. U čistého souboru dají oba stejný
výsledek; u částečného ne.

## Zápis doby trvání

Dva tvary, a jen dva:

| Tvar | Příklady |
| :---- | :------- |
| sekundy | `30`, `2.999`, `-7.001`, `+1.5` |
| časový údaj | `00:00:07.001`, `01:39:37.040`, `-00:00:07.001` |

Oba říkají totéž: `--by 7.001` a `--by 00:00:07.001` vytvoří
stejný soubor.

**Desetinným oddělovačem je tečka**, protože příkazový řádek mluví anglicky.
Čárka se odmítne, místo aby se vykládala: polovina světa píše `7,001`
pro sedm tisíc jedna, a číst to jako sedm sekund by bylo hádání —
pro tuto polovinu chybné.

**Rozlišení je milisekunda.** Jemnější hodnota se odmítne, místo aby se
potichu zaokrouhlila: tiché zaokrouhlení by dalo výsledek, který neodpovídá ničemu z toho,
co bylo požadováno.

<!-- example: printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle shifted by -7.001 s -> b.srt
1
00:00:02,999 --> 00:00:04,999
Bonjour.
```

## Posun před počátek se odmítne

Návrat za nulu dává kód `2` a **nic nezapíše**. Hlášení jmenuje
titulek, který posun nemohl přijmout, což zároveň říká,
jak daleko lze jít.

Důvodem není zásada: pozice před počátkem se zapíše
`-00:00:01,000`, což je tvar, který nedefinuje žádný z devíti formátů a nepřijme žádná
čtečka. Jádro takové pozice dovoluje — jejich odmítnutí by z editační
operace udělalo zvláštní případ — ale soubor je něco jiného než
model v paměti.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; echo "code=$?"
a.srt: subtitle 1 would start before the origin, which no subtitle file can hold
code=2
```

## Výstup

**Standardní výstup** — nic: výsledkem je zapsaný soubor. S `--dry-run` také nic: žádný soubor neexistuje a řádek výpisu to říká — viz [Náhled před zápisem](invocation.md#náhled-před-zápisem).

| Úroveň | Co se přidá na chybový výstup |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles shifted by 2.999 s -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — formát, kódování, značka a konce řádků přečteného souboru, vrácené tak, jak byly |
| 3 | `<path>: N bytes read, M written`, pak **každé hlášení při čtení** — viz [Spuštění](invocation.md#hlášení-při-čtení) |

### Ve formátu JSON

S [`--format json`](invocation.md#výstup-čitelný-pro-skript) nese objekt každého souboru `counts.subtitles` (všechny titulky souboru) a `counts.shifted_by_ms` (posun v celých milisekundách, **se znaménkem**). Úplná tabulka je na stránce [Spuštění](invocation.md#co-který-podpříkaz-uvádí-v-counts).

## Návratové kódy

Kódy nástroje: `0`, pokud byly zapsány všechny soubory, `2`, pokud žádný, `3`, pokud
jen některé, `1` při chybě použití.

## Chyby

| Co to způsobí | Hlášení |
| :------------------ | :------ |
| desetinná čárka | `"…" is not a time: use a decimal point and not a comma, in every language` |
| jemnější než milisekunda | `"…" is not a time: positions are held to the millisecond, no finer` |
| text, který není dobou trvání | `"…" is not a time: expected seconds like 2.999, or a timestamp like 00:00:07.001` |
| posun před počátek | `<path>: subtitle N would start before the origin, which no subtitle file can hold` |

**Bez jedné ze tří voleb cíle zápisu se nic nezapíše** a jejich
chyby jsou společné podpříkazům, které zapisují:
viz [Spuštění](invocation.md#cíl-zápisu).
