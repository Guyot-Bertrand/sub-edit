# `hearing-impaired`

<!-- machine-translated -->
> Tato stránka je strojovým překladem a dosud ji nezkontroloval žádný čtenář tohoto jazyka.

```
subedit-cli hearing-impaired
                  (--output FILE | --output-dir DIR | --in-place)
                  [--dry-run]
                  [--recursive] [--parentheses]
                  [--document main|translation] [-t FILE] [--align-method position|number]
                  <file>...
```

Odstraní z celého souboru **popisky pro neslyšící** — zvuky popsané v hranatých závorkách. **Kulaté závorky
ve výchozím stavu zůstávají**: používají se stejně často pro šeptanou
repliku jako pro popsaný zvuk a jejich odstranění by smazalo dialog. `--parentheses` je odstraní
také, u souboru, který v nich své zvuky popisuje.

Co nedělá: text písní mezi znaky mřížky, jméno mluvčího
před dvojtečkou, opětovné psaní velkých písmen, opravu chyb OCR. Tyto
vzory patří průvodci
[`Correct Texts…`](../subedit-gui/correct-texts.md) okna, kterého příkazový
řádek nabízí pod názvem [`correct`](correct.md); a nic tu nelze nastavit:
transformace je daná, nikoli konfigurovatelná.

<!-- example: subedit-cli hearing-impaired --help -->
```console
$ subedit-cli hearing-impaired --help
Remove the sounds described between square brackets
Usage: subedit-cli hearing-impaired [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to clean

Options:
  -h,--help                   Print this help message and exit
  --parentheses               Also remove the sounds between parentheses, often whispered lines
  -r,--recursive              Take directories as inputs, and every subtitle file in them
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
| `--parentheses` | ne | přepínač: odstraní také popisky v kulatých závorkách | vypnuto |
| `--recursive`, `-r` | ne | přepínač | vypnuto — viz [Zpracování stromu](lots.md) |
| `--document` | ne | `main` nebo `translation` — viz [Překlad](invocation.md#překlad) | `main` |
| `-t`, `--translation-file` | s `--document translation` | soubor překladu, pro **jediný** vstup | — |
| `--align-method` | ne, a jen s `-t` | `position` nebo `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **jedna ze tří**, kromě `--dry-run` | viz [Spuštění](invocation.md#cíl-zápisu) | — |
| `--dry-run` | ne | přepínač: spočítá a vypíše, nic nezapíše — viz [Náhled před zápisem](invocation.md#náhled-před-zápisem) | vypnuto |

Formát přečteného souboru se **zachová**: změna formátu je práce podpříkazu
[`convert`](convert.md).

**Ve výchozím stavu hlavní text; překlad s `--document translation -t FILE`.** Mění a zapisuje se
pak překlad, a jen on — viz [Překlad](invocation.md#překlad).

**Popisek, který by překlad vyprázdnil, ho zapíše prázdný a titulek ponechá**: titulek
zůstane v souboru, bez textu. Hlavní text naopak titulek zcela vyprázdněný ztratí, jako
níže.

## Co se odstraní a co zůstane

Popisek je hranatá závorka — nebo kulatá, s `--parentheses` — a to, co uzavírá, **včetně
zalomení řádku**: ve skutečných souborech je popisek často přerušen
koncem řádku.

| Vstup | Výstup |
| :----- | :----- |
| `Bonjour [il tousse] Marie` | `Bonjour Marie` |
| `Bonjour[il tousse]Marie` | `Bonjour Marie` |
| `[Bruit de pas] Bonjour` | `Bonjour` |
| `A [un] [deux] B` | `A B` |

Odstranění ponechá **přesně jednu mezeru mezi tím, co ho obklopovalo, a nic na
okraji řádku**. Nejde o úklid mezer souboru: dvě mezery,
které text už nesl, zůstanou dvěma mezerami.

**Když popisek přesahuje zalomení řádku, zůstane zalomení** — oba
řádky zůstanou dvěma řádky. Tento případ není kuriozita: v patnácti
skutečných titulkováních byla každá hranatá závorka, která se zdála osiřelá,
popiskem takto přerušeným.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nReculez ! [Il\nhurle] Tout de suite !\n\n' > a.srt; subedit-cli --quiet hearing-impaired --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nReculez ! [Il\nhurle] Tout de suite !\n\n' > a.srt; subedit-cli --quiet hearing-impaired --output b.srt a.srt; cat b.srt
1
00:00:01,000 --> 00:00:03,000
Reculez !
Tout de suite !
```

**Co není popisek a nehne se:**

| Případ | Co se stane |
| :-- | :-------------- |
| čistě číselný obsah — `[1]`, `(12)`, `[ 156478 ]` | ponechá se, jak je, včetně oddělovačů: odkaz nikdy nejmenoval zvuk |
| kulatá závorka a to, co uzavírá, bez `--parentheses` | ponechá se, jak je: `Venez (chut) ici` se nezmění |
| hranatá nebo kulatá závorka, kterou nic neuzavírá | ponechá se, jak je |
| značka formátu — `<i>`, `</i>` | nikdy se neodstraní |

Mezera **mezi** číslicemi, písmeno nebo znak — `[1 2]`, `[1a]`,
`[-1]` — z něj popisek dělají. Stejně tak prázdný obsah, `[]` nebo `( )`: žádná
číslice, tedy žádný odkaz; nic k řečení, tedy nic k ponechání.

## Co zmizí úplně

**Řádek, který odstranění vyprázdnilo, zmizí**, a **titulek zcela
vyprázdněný se ze souboru odstraní**. Následující titulky se přečíslují.

Řádek se počítá jako prázdný, když v něm zbývají jen mezery, značky
formátu nebo osamělá dialogová pomlčka — `<i>[Musique]</i>` nenechá na obrazovce
`<i></i>`.

**Značky jsou značkami formátu souboru.** `{\i1}[Musique]{\i0}` v
Advanced SSA, `{Y:i}[Musique]` v MicroDVD a `/[Musique]` v MPL2 také nezanechají
nic. Naopak `<i>` ve formátu, který žádnou značku
nezapisuje — TMPlayer, LRC —, je text, který divák čte, a řádek, který
ho nese, není prázdný.

**Dialog omezený na jediný hlas už dialogem není**, a pomlčka, která zbyla,
zmizí. Dokud zůstávají dva hlasy, pomlčky zůstávají.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\nVoir [1] la note.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\nVoir [1] la note.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle cleaned, 1 removed -> b.srt
1
00:00:04,000 --> 00:00:06,000
Attends Marie.

2
00:00:07,000 --> 00:00:09,000
Voir [1] la note.
```

## Soubor bez jakéhokoli popisku se přesto zapíše

Zkopíruje se beze změny, návratový kód je `0` a zpráva říká, že
nebylo co odstranit.

Je to pravidlo podpříkazů, které zapisují: **zadaný cíl zápisu je
cíl, do kterého se zapisuje**. Výjimka by tu nutila skript vědět,
který z podpříkazů někdy vytvoří soubor a někdy nic.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nRien à signaler.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nRien à signaler.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt
a.srt: no mention to remove -> b.srt
```

## Výstup

**Standardní výstup** — nic: výsledkem je zapsaný soubor. **S `--dry-run` jsou to navržené změny**: pro každý titulek, jehož text by se změnil, jeho číslo, text před a text po — viz [Náhled před zápisem](invocation.md#náhled-před-zápisem).

| Úroveň | Co se přidá na chybový výstup |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles cleaned, M removed -> <destination>`, nebo `<path>: no mention to remove -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — formát, kódování, značka a konce řádků přečteného souboru, vrácené tak, jak byly |
| 3 | `<path>: N bytes read, M written`, pak **každé hlášení při čtení** — viz [Spuštění](invocation.md#hlášení-při-čtení) |

`N` počítá titulky, jejichž text se změnil, `M` ty, které odstranění
zcela vyprázdnilo a které tedy ze souboru odešly.

### Ve formátu JSON

S [`--format json`](invocation.md#výstup-čitelný-pro-skript) nese objekt každého souboru `counts.cleaned` (přepsané texty) a `counts.removed` (odstraněné titulky); **s `--dry-run`** uvádí klíč `changes` také změněné titulky, jako standardní výstup textové podoby. Úplná tabulka je na stránce [Spuštění](invocation.md#co-který-podpříkaz-uvádí-v-counts).

## Návratové kódy

Kódy nástroje: `0`, pokud byly zapsány všechny soubory, `2`, pokud žádný, `3`, pokud
jen některé, `1` při chybě použití.

## Chyby

Tento podpříkaz nemá žádnou vlastní chybu: nepřijímá žádnou
hodnotu ke zpracování a žádný text ho nemůže přimět soubor odmítnout. Selhat
může čtení nebo zápis, jako u ostatních.

**Bez jedné ze tří voleb cíle zápisu se nic nezapíše** a jejich chyby
jsou společné podpříkazům, které zapisují:
viz [Spuštění](invocation.md#cíl-zápisu).
