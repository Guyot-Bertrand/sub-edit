# `snap`

<!-- machine-translated -->
> Tato stránka je strojovým překladem a dosud ji nezkontroloval žádný čtenář tohoto jazyka.

```
subedit-cli snap --rate <rate>
                 (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 <file>...
```

Umístí **každý časový údaj** — začátek i konec — na nejbližší snímek
zadané snímkové frekvence. Nic se nepřečasuje: každá pozice se posune nejvýše o půl
snímku.

## `snap` není `framerate` a jejich záměna není vidět

Oba příkazy mají stejný tvar — soubory, cíl zápisu, snímková frekvence — a dělají opačné věci.

| | [`framerate`](framerate.md) | `snap` |
| :--- | :-------------------------- | :----- |
| co počítá | `t' = t x input rate / output rate` | nejbližší snímek na mřížce `--rate` |
| o kolik posouvá | úměrně času — u celovečerního filmu **o několik sekund** | nejvýše o půl snímku, **všude** |
| pro jaký soubor | soubor, jehož **časování je chybné** | soubor, jehož časování je **správné** a jehož mřížka je chybná |

**Případ použití podpříkazu `snap`**, rozepsaný: film při 25 snímcích za
sekundu, soubor titulků zapsaný na mřížce 24, repliky už
víceméně na svém místě s přesností na několik milisekund. Není co přečasovat
— jen vrátit časové údaje na správné snímky.

**Případ použití podpříkazu `framerate`** je ten druhý: soubor časovaný pro master při
25, který chcete přehrát na kopii při 23,976 a jehož časování ke konci filmu ujíždí
o několik sekund.

Záměna jednoho za druhý **nezpůsobí žádnou chybu**: oba
soubor přijmou a zapíší výsledek. Rozliší je řádek
výpisu — `snap` vždy řekne, o kolik pozice posunul.

<!-- example: subedit-cli snap --help -->
```console
$ subedit-cli snap --help
Move every position onto the nearest frame of a frame rate (see framerate)
Usage: subedit-cli snap [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to align

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --rate TEXT REQUIRED        Frame rate to align on: 25, 23.976
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
| `--rate` | **ano** | snímková frekvence ve snímcích za sekundu, zapsaná jako u podpříkazu [`framerate`](framerate.md#zápis-snímkové-frekvence) | — |
| `--output` / `--output-dir` / `--in-place` | **jedna ze tří**, kromě `--dry-run` | viz [Spuštění](invocation.md#cíl-zápisu) | — |
| `--dry-run` | ne | přepínač: spočítá a vypíše, nic nezapíše — viz [Náhled před zápisem](invocation.md#náhled-před-zápisem) | vypnuto |

`--rate` přijímá **jakoukoli platnou snímkovou frekvenci**, a ne jen osm
standardních: je to volba uživatele, jako u `framerate`. Nulová nebo záporná
snímková frekvence se odmítne.

Formát přečteného souboru se **zachová**: změna formátu je práce podpříkazu
[`convert`](convert.md).

## Výstup

**Standardní výstup** — nic: výsledkem je zapsaný soubor. S `--dry-run` také nic: žádný soubor neexistuje a řádek výpisu to říká — viz [Náhled před zápisem](invocation.md#náhled-před-zápisem).

| Úroveň | Co se přidá na chybový výstup |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles aligned on 25 fps, M positions moved, by at most 20 ms -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — formát, kódování, značka a konce řádků přečteného souboru, vrácené tak, jak byly |
| 3 | `<path>: N bytes read, M written`, pak **každé hlášení při čtení** — viz [Spuštění](invocation.md#hlášení-při-čtení) |

**Smyslem řádku jsou obě čísla.** Půl snímku při 25 snímcích za
sekundu je dvacet milisekund a nic většího tam zapsáno být nemůže. Kdo
chtěl `framerate` a napsal `snap`, přečte „by at most 20 ms“ tam, kde čekal
sekundy, a hned to ví.

## Příklad

Replika v `00:00:01,010`, zarovnaná na 25 snímků za sekundu — tedy na
násobek čtyřiceti milisekund:

<!-- example: printf '1\n00:00:01,010 --> 00:00:03,020\nUn.\n\n' > a.srt; subedit-cli snap --rate 25 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,010 --> 00:00:03,020\nUn.\n\n' > a.srt; subedit-cli snap --rate 25 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle aligned on 25 fps, 2 positions moved, by at most 20 ms -> b.srt
1
00:00:01,000 --> 00:00:03,040
Un.
```

## Dvě vlastnosti

**Druhé zarovnání už nic nezmění.** Pozice, která už na
snímku je, tam zůstane: jde o tutéž operaci použitou na tentýž výsledek.

**Pořadí titulků se zachová.** Zaokrouhlení na nejbližší snímek je
monotónní, takže dva začátky vzdálené alespoň jeden snímek takové zůstanou. Bližší
začátky mohou splynout — což je anomálie, kterou
[`inspect`](inspect.md#co-hlásí-anomalies) umí pojmenovat, a ne
převrácení pořadí.

### Ve formátu JSON

S [`--format json`](invocation.md#výstup-čitelný-pro-skript) nese objekt každého souboru `counts.subtitles`, `counts.moved` (posunuté pozice) a `counts.furthest_ms` (největší posun v milisekundách). Úplná tabulka je na stránce [Spuštění](invocation.md#co-který-podpříkaz-uvádí-v-counts).

## Návratové kódy

Kódy nástroje: `0`, pokud byly zapsány všechny soubory, `2`, pokud žádný, `3`, pokud
jen některé, `1` při chybě použití.

## Chyby

Chyby snímkové frekvence, slovo od slova ty podpříkazu [`framerate`](framerate.md#chyby)
— oba čtou tutéž hodnotu touž cestou:

| Co to způsobí | Hlášení |
| :------------------ | :------ |
| nulová nebo záporná snímková frekvence | `"…" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed` |
| desetinná čárka | `"…" is not a frame rate: use a decimal point and not a comma, in every language` |
| text, který není číslem | `"…" is not a frame rate: expected frames per second, like 25 or 23.976` |
| nepřiměřená snímková frekvence | `"…" is not a frame rate: no video runs at that many frames per second` |

**Bez jedné ze tří voleb cíle zápisu se nic nezapíše** a jejich chyby
jsou společné podpříkazům, které zapisují:
viz [Spuštění](invocation.md#cíl-zápisu).
