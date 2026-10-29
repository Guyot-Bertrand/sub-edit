# subedit — příručka

<!-- machine-translated -->
> Tato stránka je strojovým překladem a dosud ji nezkontroloval žádný čtenář tohoto jazyka.

subedit čte, kontroluje a přečasovává soubory titulků. Dodává se jako dva
programy, které sdílejí stejné jádro, a tedy i stejná pravidla a stejná slova.

| Program | K čemu slouží | Jeho příručka |
| :-------- | :-------------- | :--------- |
| `subedit-gui` | otevřít soubor, prohlížet ho v tabulce, upravovat ho, sledovat ho spolu s filmem | [příručka okna](subedit-gui/index.md) |
| `subedit-cli` | zpracovávat soubory v dávkách, ze skriptu, bez rozhraní | [příručka příkazového řádku](subedit-cli/index.md) |

Oba čtou a zapisují **devět formátů** — SubRip, WebVTT, SubViewer 2,
Sub Station Alpha, Advanced SSA, MicroDVD, MPL2, TMPlayer, LRC — a
rozpoznávají formát podle obsahu, nikoli podle přípony. **Také kódování
se rozpoznává podle obsahu**: soubor v kódování Latin-1, CP1252 nebo UTF-16
se otevře, program sdělí, jak byl přečten, a zapíše ho zpět v jeho vlastním kódování —
soubor otevřený a uložený beze změny se nezmění ani o bajt.

## Kde začít

| Co chcete | Kam jít |
| :----------------- | :------- |
| nainstalovat nástroj | [Instalace](subedit-cli/installation.md) — platí pro oba programy |
| otevřít soubor a podívat se, co obsahuje | [Tabulka](subedit-gui/table.md) |
| přečasovat titulky posunuté o stálou hodnotu | [Operace](subedit-gui/operations.md#shift-positions), nebo [`shift`](subedit-cli/shift.md) |
| přečasovat soubor napsaný pro jinou snímkovou frekvenci | [`framerate`](subedit-cli/framerate.md), a [`snap`](subedit-cli/snap.md), pokud je časování už správné |
| zjistit, pro jakou snímkovou frekvenci byl soubor napsán | [Mřížka snímků](subedit-gui/frame-grid.md), nebo [`inspect`](subedit-cli/inspect.md) |
| odstranit popisky pro neslyšící | [Operace](subedit-gui/operations.md#remove-hearing-impaired-mentions), nebo [`hearing-impaired`](subedit-cli/hearing-impaired.md) |
| opravit popisky, běžné chyby, velikost písmen a zalomení řádků v jediném průchodu | [`Correct Texts…`](subedit-gui/correct-texts.md), nebo [`correct`](subedit-cli/correct.md) |
| zkontrolovat pravopis slovo po slovu, se slovníky systému | [`Check Spelling…`](subedit-gui/check-spelling.md) |
| prodloužit titulky, které jsou příliš krátké na přečtení | [`Adjust Durations…`](subedit-gui/operations.md#adjust-durations), nebo [`adjust`](subedit-cli/adjust.md) |
| znovu seřadit soubor, jehož titulky seřazené nejsou | [`sort`](subedit-cli/sort.md) |
| spojit do jediného titulku po sobě jdoucí kusy téhož textu | [`Join Repeated Subtitles`](subedit-gui/lines.md#spojení-opakovaných-titulků), nebo [`join-repeats`](subedit-cli/join-repeats.md) |
| najednou dát do pořádku soubor, který je špatně rozdělený, neseřazený a s překryvy | [`repair`](subedit-cli/repair.md) |
| odstranit překryvy mezi titulky | [`Make Ranges Disjoint…`](subedit-gui/lines.md#převedení-rozsahů-na-disjunktní), nebo [`disjoint`](subedit-cli/disjoint.md) |
| spojit dvě části filmu za sebe | [`Append File…`](subedit-gui/operations.md#append-file), nebo [`append`](subedit-cli/append.md) |
| rozdělit film na dvě části | [`Split Project…`](subedit-gui/operations.md#split-project), nebo [`split-file`](subedit-cli/split-file.md) |
| změnit velikost písmen, kurzívu nebo dialogové pomlčky celého souboru | [Operace](subedit-gui/operations.md#italic), nebo [`case`](subedit-cli/case.md), [`italics`](subedit-cli/italics.md), [`dialogue-dashes`](subedit-cli/dialogue-dashes.md) |
| vyhledat slovo a nahradit ho, aniž by se porušily značky | [Najít a nahradit](subedit-gui/find-and-replace.md), nebo [`replace`](subedit-cli/replace.md) |
| zapsat řeč filmu jako titulky | [`Transcribe Audio…`](subedit-gui/video.md#přepis-zvuku), nebo [`transcribe`](subedit-cli/transcribe.md) — po [instalaci nástrojů](subedit-cli/install-transcription.md) |
| přečasovat soubor titulků podle zvuku filmu | [`sync`](subedit-cli/sync.md) — po [instalaci nástrojů](subedit-cli/install-transcription.md) |
| překládat vedle původního textu | [Otevření překladu](subedit-gui/files.md#otevření-překladu), nebo [`pair`](subedit-cli/pair.md) k jeho přečasování podle hlavního souboru |
| zpracovat sto souborů najednou | [Zpracování stromu](subedit-cli/lots.md), pak [Spuštění](subedit-cli/invocation.md) |

**Mezi `framerate` a `snap` chyba projde bez povšimnutí.** Pokud
váháte, stránka [`snap`](subedit-cli/snap.md#snap-není-framerate-a-jejich-záměna-není-vidět)
začíná jejich srovnáním.

## Co tato příručka slibuje

**Popisuje to, co existuje, nikdy to, co se plánuje.** Co přijde a
v jakém pořadí, je v [plánu vývoje](../../../feuille-de-route.md).

**Tato příručka je psána anglicky a překládá se.** Její překlady ji sledují
stránku po stránce v adresáři `docs/i18n/` repozitáře, jeden strom na jazyk.
Programy mluví jazykem svého prostředí; názvy, které zobrazují — podpříkazy,
volby, popisky nabídek —, se zde uvádějí tak, jak je program píše anglicky.

**Příklady a obrázky se generují, text ne.** Bloky
`console` vznikají skutečným spuštěním příkazu; snímky
obrazovky sestavením skutečného okna a jeho vyfotografováním. Ani
jedno tedy nemůže popisovat nástroj, který už neexistuje. Bez záchranné sítě
zůstává text kolem nich, jehož správnost závisí na korektuře.

**Snímky obrazovky ukazují obě palety**, `Light` a `Dark` — ty, které
nabízejí [předvolby](subedit-gui/preferences.md#motiv).

## Obě rozhraní říkají stejná slova

Jeden z důvodů, proč číst kteroukoli z příruček při používání druhého programu:
chybová hlášení, názvy formátů, verdikty mřížky a popisky
vrácení zpět jsou napsány **jednou**, v jádře, a oba programy
je citují. Odmítnutý soubor dostane na obou stranách stejnou větu.

Liší se to, co které rozhraní umí:

| | `subedit-gui` | `subedit-cli` |
| :--- | :------------ | :------------ |
| několik souborů najednou | ano — jedna karta na projekt | ano |
| překlad vedle hlavního souboru | ano | ano — `-t` u textových podpříkazů a [`pair`](subedit-cli/pair.md) k jeho přečasování |
| připojit soubor, rozdělit projekt | ano | ano — [`append`](subedit-cli/append.md), [`split-file`](subedit-cli/split-file.md) |
| vrácení zpět | ano, tisíc položek | nepoužije se — nic se nemění na místě, aniž byste o to požádali |
| ručně upravit text nebo pozici | ano | ne |
| vkládat, odstraňovat, slučovat a rozdělovat řádky | ano | ne |
| najít a nahradit | ano | ano — [`replace`](subedit-cli/replace.md) |
| vyjímat, kopírovat a vkládat texty | ano | ne |
| upravit doby trvání, kurzívu, velikost písmen, pomlčky | ano | ano — [`adjust`](subedit-cli/adjust.md), [`italics`](subedit-cli/italics.md), [`case`](subedit-cli/case.md), [`dialogue-dashes`](subedit-cli/dialogue-dashes.md) |
| opravit texty pomocí vzorů | ano — `Correct Texts…` | ano — [`correct`](subedit-cli/correct.md) |
| zkontrolovat pravopis slovo po slovu | ano — `Check Spelling…` | ne |
| sledovat film při časování | ano | ne |
| zapsat přes vstup | `Save` | `--in-place` |
