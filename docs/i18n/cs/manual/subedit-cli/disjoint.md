# `disjoint`

<!-- machine-translated -->
> Tato stránka je strojovým překladem a dosud ji nezkontroloval žádný čtenář tohoto jazyka.

```
subedit-cli disjoint (--output FILE | --output-dir DIR | --in-place)
                     [--minimum TIME] [--max-lines N] [--dashes] [--sort]
                     [--dry-run] [--recursive]
                     <file>...
```

Rozdělí překrývající se titulky tak, aby se **žádný rozsah `[start, end]` už nepřekrýval
s jiným**. Jde o `Make Ranges Disjoint…` okna — [týž mechanismus](../subedit-gui/lines.md#převedení-rozsahů-na-disjunktní).

**Nic se nezkracuje ani neprodlužuje**: v každém okamžiku soubor zobrazuje tytéž texty jako dříve. Mění
se způsob, jakým je rozdělena časová osa:

- dělí se při **každém začátku a každém konci** titulků překrývající se skupiny;
- interval pokrytý **jediným** titulkem zůstane tímto titulkem;
- interval pokrytý **několika** se stane **jedním** titulkem, jehož řádky jsou jejich řádky, **v
  pořadí jejich začátků** — nejdřívější na řádku 1, další na řádku 2;
- titulku, který nic nepřekrývá, se nikdo nedotkne, a pořadí ostatních se zachová.

<!-- example: subedit-cli disjoint --help -->
```console
$ subedit-cli disjoint --help
Cut overlapping subtitles so that no range overlaps another, merging what overlaps
Usage: subedit-cli disjoint [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to make disjoint

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --minimum TIME              Shortest interval kept; a shorter one is taken over by a neighbour (default 0.08)
  --max-lines N               Most lines of a merged subtitle; more are shown in windows, 0 for no limit (default 2)
  --dashes                    Put a dash before the text of each merged subtitle
  --sort                      Put the subtitles in order of their start first; without it, a file out of order is refused
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Argumenty a volby

| Volba | Povinná | Hodnoty | Výchozí |
| :----- | :----- | :------ | :----- |
| `<file>...` | ano | jedna nebo více cest | — |
| `--minimum` | ne | doba trvání: `0.08`, `1.5`, `00:00:00,080`. Nula ponechá všechny intervaly | `0.08` (asi dva snímky) |
| `--max-lines` | ne | celé číslo: nejvyšší počet řádků sloučeného titulku; `0` omezení zruší | `2` |
| `--dashes` | ne | přepínač: pomlčka před textem, který každý titulek přináší do sloučeného titulku | vypnuto |
| `--sort` | ne | přepínač: **nejprve** seřadí titulky podle jejich začátku — viz [Je třeba seřazený soubor](#je-třeba-seřazený-soubor) | vypnuto |
| `--recursive`, `-r` | ne | přepínač | vypnuto — viz [Zpracování stromu](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **jedna ze tří**, kromě `--dry-run` | viz [Spuštění](invocation.md#cíl-zápisu) | — |
| `--dry-run` | ne | přepínač: spočítá a vypíše, nic nezapíše — viz [Náhled před zápisem](invocation.md#náhled-před-zápisem) | vypnuto |

<!-- example: printf '1\n00:00:00,000 --> 00:00:05,000\nA\n\n2\n00:00:03,000 --> 00:00:08,000\nB\n\n' > chevauche.srt; subedit-cli disjoint --output disjoint.srt chevauche.srt; cat disjoint.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:05,000\nA\n\n2\n00:00:03,000 --> 00:00:08,000\nB\n\n' > chevauche.srt; subedit-cli disjoint --output disjoint.srt chevauche.srt; cat disjoint.srt
chevauche.srt: 2 subtitles split, 1 merged -> disjoint.srt
1
00:00:00,000 --> 00:00:03,000
A

2
00:00:03,000 --> 00:00:05,000
A
B

3
00:00:05,000 --> 00:00:08,000
B
```

`A [0, 5]` a `B [3, 8]` dávají tři disjunktní titulky: `A` samotný, `A` a `B` spolu, `B` samotný. Žádný
okamžik se neztratí ani nepřibude.

## O čem operace rozhoduje

**Řádek se nikdy nezdvojí.** Dva titulky se stejným textem dají tam, kde se překrývají, jediný řádek,
a dva sousední kusy, které ukazují totéž, tvoří jen jeden titulek. Text
titulku rozděleného na několik kusů se jinak **opakuje v každém z nich**: je to to, co je v daném
okamžiku na obrazovce.

**Velmi krátký interval převezme soused.** Pod hodnotou `--minimum` interval zmizí: soused
**rozšíří svůj rozsah tak, aby ho pokryl**, takže žádný okamžik nezůstane prázdný a nic se neposune o víc než
tento práh. Vybraným sousedem je ten, jehož řádky **už obsahují** řádky intervalu, jinak
ten nejdelší. Text intervalu se ztratí jen tehdy, když ho žádný soused nenese, a pak na méně než práh.
**Celý titulek kratší než práh se nikdy nepřevezme**: ponechá se a započítá.

**Více řádků, než je limit: okna.** Sloučený titulek, který by měl víc než `--max-lines` řádků,
se zobrazí **v postupných oknech** této velikosti, ve stejných dílech doby trvání. Tři řádky na tři
sekundy:

<!-- example: printf '1\n00:00:00,000 --> 00:00:03,000\nA\n\n2\n00:00:00,000 --> 00:00:03,000\nB\n\n3\n00:00:00,000 --> 00:00:03,000\nC\n\n' > trois.srt; subedit-cli disjoint --output fenetres.srt trois.srt; cat fenetres.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:03,000\nA\n\n2\n00:00:00,000 --> 00:00:03,000\nB\n\n3\n00:00:00,000 --> 00:00:03,000\nC\n\n' > trois.srt; subedit-cli disjoint --output fenetres.srt trois.srt; cat fenetres.srt
trois.srt: 0 subtitles split, 1 merged; 1 shown in windows for holding too many lines -> fenetres.srt
1
00:00:00,000 --> 00:00:01,500
A
B

2
00:00:01,500 --> 00:00:03,000
B
C
```

Čtyři řádky dají tři okna a tak dále: okno se posouvá vždy o jeden řádek. Nic se
nezkracuje ani znovu nedělí.

**Překlad sleduje tatáž dělení**, protože titulek nese oba texty. **Co formát nese
navíc** — styl, pozice — je to, co má první pokrývající titulek; ztráta se započítá, když
se sloučené titulky liší.

**Titulek, jehož konec přichází před jeho začátkem**, se nedomýšlí: ponechá se, kde je, a započítá se.

## Je třeba seřazený soubor

Skupiny jsou série sousedů: bez seřazení nejsou sousedy ty titulky, které se překrývají. Soubor,
jehož začátky nejsou seřazené, se proto **odmítne** a hlášení to řekne:

<!-- example: printf '1\n00:00:03,000 --> 00:00:08,000\nB\n\n2\n00:00:00,000 --> 00:00:05,000\nA\n\n' > desordre.srt; subedit-cli disjoint --output x.srt desordre.srt; echo "code $?" -->
```console
$ printf '1\n00:00:03,000 --> 00:00:08,000\nB\n\n2\n00:00:00,000 --> 00:00:05,000\nA\n\n' > desordre.srt; subedit-cli disjoint --output x.srt desordre.srt; echo "code $?"
desordre.srt: The subtitles are not in the order of their start: sort them first. Use --sort to do that here.
code 2
```

`--sort` nejprve provede seřazení — totéž co [`sort`](sort.md) — a řekne to. Jádro nikdy neřadí samo od sebe.

Druhý průchod nic nezmění: jakmile jsou rozsahy disjunktní, není už co dělit, a
věta říká `already disjoint`.

## Výstup

**Standardní výstup** — nic: výsledkem je zapsaný soubor.

| Úroveň | Co se přidá na chybový výstup |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles split, M merged -> <destination>`, za nímž po `;` následuje, co nešlo provést úplně: `K shown in windows for holding too many lines`, `K very short taken over by a neighbour`, `K subtitles shorter than the limit left whole`, `K merged with styles or positions that differ, the first kept`, `K subtitles ending before the start left alone` — nebo `<path>: already disjoint -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, pak **každé hlášení při čtení** |

`N subtitles split` počítá původní titulky rozdělené na několik kusů, `M merged` kusy, které
ukazují několik titulků spolu.

### Ve formátu JSON

S [`--format json`](invocation.md#výstup-čitelný-pro-skript) jsou `counts` každého souboru:

| Počet | Co říká |
| :----- | :----------- |
| `subtitles` | titulky zapsaného souboru |
| `split`, `merged` | jako ve větě |
| `windowed` | kusy zobrazené v oknech |
| `absorbed` | intervaly převzaté sousedem |
| `too_short` | titulky kratší než práh, ponechané celé |
| `ending_before_start` | titulky, jejichž konec přichází před začátkem, ponechané být |
| `extras_dropped` | sloučené kusy, jejichž styl nebo pozice se liší |
| `moved` | jen s `--sort`: místa, která se změnila |

## Návratové kódy

Kódy nástroje: `0`, pokud byly zpracovány všechny soubory — soubor bez překryvů zpracovaný je —, `2`, pokud
žádný, `3`, pokud jen některé, `1` při chybě použití.

## Chyby

| Co je způsobí | Hlášení |
| :------------------- | :------ |
| neseřazený soubor bez `--sort` | `<path>: The subtitles are not in the order of their start: sort them first. Use --sort to do that here.` |
| `--minimum` není doba trvání nebo je záporné | `--minimum: …` |
| `--max-lines` není celé číslo | `--max-lines: "…" is not a number of lines: expected a whole number, 0 for no limit` |

Nečitelný soubor selže jako u každého podpříkazu; chyby cíle zápisu jsou společné
podpříkazům, které zapisují: viz [Spuštění](invocation.md#cíl-zápisu).

## V ostatních příkazech

[`convert`](convert.md#převedení-rozsahů-na-disjunktní), [`append`](append.md#převedení-rozsahů-na-disjunktní) a
[`split-file`](split-file.md#převedení-rozsahů-na-disjunktní) přijímají `--disjoint`, které udělá totéž
s výchozím nastavením, před zápisem.

U špatně segmentovaného souboru — text rozdělený na kusy, které po sobě následují, zadané v nesprávném pořadí — dá samotné seřazení a rozdělení
řádky, které z jednoho kusu na další přeskakují: nejprve [`join-repeats`](join-repeats.md), nebo
[`repair`](repair.md), který řetězí všechny tři.
