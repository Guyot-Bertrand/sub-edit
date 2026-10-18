# Vyjmutí, kopírování a vložení textů

<!-- machine-translated -->
> Tato stránka je strojovým překladem a dosud ji nezkontroloval žádný čtenář tohoto jazyka.

Tři položky nabídky `Edit`, pod `Undo` a `Redo` a nad
`Find and Replace…`.

| Položka | Zkratka | Co dělá |
| :---- | :------- | :----------- |
| Vyjmout texty | `Ctrl+X` | zkopíruje texty výběru a pak je vyprázdní |
| Kopírovat texty | `Ctrl+C` | zkopíruje texty výběru |
| Vložit texty | `Ctrl+V` | zapíše zkopírované texty od prvního vybraného řádku |

**Putují texty, a nic jiného**: žádný začátek, žádný konec, žádný celý
řádek. Vložení proto nepohne žádnou pozicí — časování řádku, který
text přijme, zůstane jeho vlastní. Tak to dělá i Gaupol.

## Na co se vztahují

**S překladem na text aktuálního sloupce**: na překlad, je-li aktuální
buňka ve sloupci `Translation`, jinde na hlavní text —
viz [Tabulka](table.md#text-na-který-operace-míří). `Cut Texts` tedy vyprázdní
překlad a hlavní text ponechá a `Paste Texts` zapisuje do
sloupce, ve kterém jste. **Schránka nenese žádný dokument**:
buňku hlavního textu můžete zkopírovat a vložit do překladu, a
naopak.

**Výběr, a nikdy celý soubor.** Tři položky jsou nedostupné, dokud
není nic vybráno. Jinde v okně znamená „nic nevybráno“
„celý soubor“; zde by nešťastné `Ctrl+X` vyprázdnilo všechny texty
dokumentu a vložení by nemělo řádek, od kterého začít.

Dokud je buňka otevřená, patří `Ctrl+X`, `Ctrl+C` a `Ctrl+V`
vstupnímu poli: vyjímají, kopírují a vkládají znaky, stejně jako `Del`
v něm maže znak, a ne titulek. Spuštěny z nabídky, položky nejprve vstup
potvrdí: `Copy Texts` pak zkopíruje text tak, jak byl právě
napsán. Viz [Úprava buňky](editing.md).

## Kopírování

`Copy Texts` zkopíruje text každého vybraného řádku, od prvního po
poslední. **Nesouvislý výběr si zachová svůj tvar**: řádek vynechaný
z výběru se stane mezerou a vložení ho nepřepíše.

**Vybraný řádek s prázdným textem se také stane mezerou**:
při vložení ponechá cílový řádek, jak je. Zkopírování jediného prázdného textu tedy
nevloží nic.

Systémová schránka dostane tyto texty **jako prostý text, oddělené
prázdným řádkem** — mezera se tam zapíše jako prázdný text, a proto prázdný
text nemůže být ničím jiným než mezerou: vložení dá stejný výsledek,
ať kopie pochází z tohoto okna, nebo z jiného programu. Co zde zkopírujete, lze
tedy vložit do jakéhokoli programu, a co zkopírujete jinde, lze vložit sem.

Kopírování dokument nemění a do historie se nedostane.

## Vyjmutí

`Cut Texts` zkopíruje a pak **vyprázdní texty** vybraných řádků. Řádky
zůstanou i se svými pozicemi: vyjmutí není odstranění — k odstranění
řádků slouží [`Remove Subtitles`](lines.md#odstraňování).

Vyjmutí textů, které jsou už všechny prázdné, nic neudělá a do
historie se nedostane.

## Vložení

`Paste Texts` zapíše texty ze schránky **od prvního vybraného
řádku**, jeden řádek na text, směrem dolů.

| Co schránka obsahuje | Co se stane |
| :-------------------------- | :----------- |
| text | nahradí text řádku |
| mezeru nebo zkopírovaný prázdný text | řádek si ponechá svůj text |
| více textů, než zbývá řádků | na konec se přidají řádky, které je přijmou |

Přidané řádky vznikají jako řádky vložené za poslední: tři
sekundy každý, jeden za druhým. **Okno to řekne** v informačním okně:

```text
inserted 2 subtitles to fit the clipboard
```

Po vložení **jsou zapsané řádky vybrané**.

Vložení, které nic nemění — stejné texty na stejných řádcích —, se do
historie nedostane.

### Značky jiného formátu se překládají

Kopie pořízená v tomto okně **si pamatuje formát svého dokumentu** a
tato paměť přežije otevření jiného souboru. Vložení do dokumentu
jiného formátu proto značky přeloží, stejně jako `Save As…`:
`{\i1}Bonjour{\i0}` zkopírované z Advanced SSA se do
SubRipu vloží jako `<i>Bonjour</i>`.

**Co cílový formát neumí zapsat, se ztratí, a řekne se to**,
slovy `Save As…`:

```text
pasting SubRip texts into LRC: 1 tag dropped
```

Na rozdíl od `Save As…` se předem nic neptá: vložení se vrátí zpět pomocí
`Ctrl+Z`, které texty vrátí tak, jak byly.

**Text zkopírovaný mimo `subedit` nemá formát** a vloží se tak, jak je:
není z čeho překládat. Totéž platí pro kopii z
tohoto okna, kterou systémová schránka mezitím nahradila — rozhoduje
obsah systémové schránky.

Když jsou na místě obě hlášení, vejdou se do jediného okna, oddělená
středníkem.

## Co o tom říká akce Zpět

Každý úkon je **jedna** položka historie, bez ohledu na počet
řádků, které zapíše nebo přidá.

| Operace | Co ukazuje `Undo` |
| :-------- | :---------------- |
| vyjmutí | `Undo: cutting texts` |
| vložení | `Undo: pasting texts` |

Viz [Zpět a znovu](undo.md).
