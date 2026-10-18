# Zpět a znovu

<!-- machine-translated -->
> Tato stránka je strojovým překladem a dosud ji nezkontroloval žádný čtenář tohoto jazyka.

Každá operace se zapíše do historie a lze ji vrátit zpět. Upravená buňka se počítá
jako **jedna** položka: psaní se nezaznamenává po jednotlivých úhozech, počítá se jen
potvrzení.

| Akce | Zkratka | Kde |
| :----- | :------- | :---- |
| Zpět | `Ctrl+Z` | nabídka **Úpravy**, panel nástrojů |
| Znovu | `Ctrl+Shift+Z` | nabídka **Úpravy**, panel nástrojů |

## Akce pojmenuje, co vrátí

Nabídka neříká „Zpět“, ale **„Zpět: úprava textu“**, podle
operace, která se vrátí. Tlačítko panelu nástrojů si ponechá holé slovo — jeho
šířka by jinak sledovala poslední operaci a posouvalo by se pod
ukazatelem myši —, ale jeho bublinová nápověda nese celý popisek.

| Operace | Co akce ukazuje |
| :-------- | :-------------------- |
| upravená textová buňka | `editing a text` |
| začátek, konec | `editing a start`, `editing an end` |
| doba trvání | `editing a duration` |
| vložení řádků | `inserting` |
| odstranění řádků | `removing` |
| posun, včetně `Shift Whole File onto Grid` | `shifting` |
| transformace | `transforming` |
| převod snímkové frekvence | `converting the frame rate` |
| zarovnání na snímkovou frekvenci | `aligning on the frame rate` |
| odstranění popisků | `removing hearing-impaired mentions` |
| převod na kurzívu | `putting in italics` |
| odebrání kurzívy | `taking italics out` |
| změna velikosti písmen | `changing the case` |
| přidané, odebrané pomlčky | `adding dialogue dashes`, `removing dialogue dashes` |
| sloučení řádků | `merging` |
| rozdělení řádku | `splitting` |
| vyjmuté, vložené texty | `cutting texts`, `pasting texts` |
| úprava dob trvání | `adjusting durations` |
| otevření překladu | `opening a translation` |
| připojení souboru | `appending a file` |
| rozdělení projektu | `splitting the project` |
| oprava přijatá v `Correct Texts…` nebo provedená pomocí `Check Spelling…` | `correcting texts` |
| nahrazení, nahrazení všech | `replacing`, `replacing all` |

**Dvacet sedm popisků, a to je vše, co okno dokáže vytvořit.** Jádro pojmenovává
i dvacátý osmý — `sorting` —, ke kterému se žádná akce okna nedostane: řazení
probíhá jen při přísné politice pořadí, zatímco okno otevírá své dokumenty
při benevolentní politice, která nepořádek hlásí, místo aby ho opravovala.

**`Shift Whole File onto Grid` se od posunu neodlišuje**, a to
správně: posunem je, jen jeho hodnota byla změřena, a ne zadána.

Jsou to tatáž slova jako na příkazovém řádku: zapisují se jednou,
v jádru.

Když není co vrátit, akce říká prostě „Zpět“ **a je
nedostupná**: její klávesová zkratka nic nedělá.

## Značka změny

Titulek okna nese značku, dokud se soubor liší od souboru na
disku — hvězdičku, nebo cokoli, co platforma používá místo ní. **Totéž platí pro popisek
karty**, `premier.srt*`: titulek okna mluví jen o projektu, na který se
díváte, popisek informuje o těch, na které se nedíváte.

**Každá karta má vlastní historii.** Zpět vrací jen to, co dostal projekt
aktuální karty — včetně `Replace All` provedeného přes všechny projekty: to
zanechá jednu položku v každém projektu, kterého se dotklo.

**Značka zmizí, pokud vrátíte změny až k výchozímu bodu.** Nejde o detail
zobrazení: počet změn se počítá, nezaznamenává se jako ano/ne, takže
návrat zpět ho může vrátit na nulu a značka zmizí.

## Dvě věci, které je dobré vědět

**Referenčním bodem je poslední uložení**, nebo otevření souboru, pokud k uložení
nedošlo. Uložení ho posune: značka zmizí a vrácení zpět po něm
ji vrátí.

Operace provedená po vrácení zpět **smaže, co bylo možné provést znovu**: opakování
by přehrálo příkaz, jehož výchozí stav už neexistuje. Tak se
chová každý editor, a je to záměr.

**Dokud je buňka otevřená**, `Undo` a `Redo` — z nabídky nebo z panelu nástrojů —
nejprve potvrdí vstup, stejně jako ostatní úkony z
[Úpravy buňky](editing.md). Co bylo právě napsáno, se tak stane poslední
položkou historie: `Undo` vrátí to, a ne předchozí operaci; `Redo` nemá
co opakovat, protože vstup smaže, co bylo možné provést znovu. Otevřená
buňka, do které se nic nenapsalo, nezanechá žádnou položku: `Undo` pak vrátí předchozí
operaci.

Historie uchovává tisíc položek a nejstarší nad tento počet zapomíná.
