# Časování podle videa

<!-- machine-translated -->
> Tato stránka je strojovým překladem a dosud ji nezkontroloval žádný čtenář tohoto jazyka.

*Předpokládá se, že film je otevřený a přehrávač známý: viz [Přehrávač](player.md).*

Sledujete film, zastavíte se na snímku, kde má začít replika, a **tato pozice se stane
začátkem titulku**. Pět úkonů v nabídce **Video**, pod názvy, které jim dává Gaupol. Opírají
se o to, co už existuje: nastavení pozice je **tentýž příkaz jako zápis hodnoty do
buňky** a vložení je **totéž jako `Úpravy ▸ Vložit titulky…`**.

| Příkaz | Zkratka | Co dělá |
| :------ | :------- | :----------- |
| Video ▸ Nastavit začátek podle pozice videa | `Alt+U` | nastaví **začátek** vybraného titulku na pozici filmu |
| Video ▸ Nastavit konec podle pozice videa | `Alt+K` | nastaví jeho **konec** na pozici filmu |
| Video ▸ Vložit titulek na pozici videa | `Alt+J` | vloží titulek, který **začíná** na pozici, a vybere ho |
| Video ▸ Vybrat předchozí od pozice videa | `Alt+Up` | vybere poslední titulek, který **začíná před** pozicí |
| Video ▸ Vybrat následující od pozice videa | `Alt+Down` | vybere první, který **začíná po** pozici |

**Všechny jsou nedostupné, není-li otevřeno žádné video**, stejně jako úkony přehrávače. `Set Start…` a `Set End…`
jsou navíc nedostupné, dokud není nic vybráno: působí na **první řádek**
výběru.

**Zkratky jsou zkratkami Gaupolu, s `Alt`**: tam jsou to `U`, `K` a `J` — samotná písmena, která
by okno zachytilo dříve než upravovaná buňka, takže by napsání `k` do textu
nastavilo konec. `Select Previous…` a `Select Next…` jsou v Gaupolu `Ctrl+Y` a `Ctrl+U`; `Ctrl+Y`
je v některých systémech *znovu* a šipky s `Alt` říkají *před* a *po* lépe než
dvě písmena. Upravovaná buňka se **potvrdí** před `Set Start…`, `Set End…` nebo
`Insert Subtitle…`, stejně jako před každou úpravou.

## Nastavení okraje

- **Jedna položka historie na úkon**: `Úpravy ▸ Zpět` vrátí předchozí pozici, a nic jiného.
- **Stejná pravidla jako u buňky.** Začátek nastavený za konec titulku nebo konec před jeho
  začátek **se neodmítne**: nastaví se a tabulka titulek označí, jako označí ten,
  jehož hodnota byla zapsána ručně — viz [Anomálie](table.md#anomálie). Nic se tiše neopravuje.
- **Pozice, která už okrajem je, nic neudělá** a do historie nic nepřidá.
- **Pozice je vždy začátkem snímku**: přehrávač nikdy nehlásí pozici
  uvnitř snímku. Nastavení okraje a následný návrat na něj — například pomocí `Seek Selection Start` —
  **ukáže tentýž snímek**.

## Vložení na pozici

Nový titulek **začíná na pozici** a trvá **tři sekundy**, nebo do začátku následujícího,
pokud ten přijde dříve. Jeho text je prázdný. Zaujme své místo v pořadí souboru: za
všemi, které začínají na této pozici nebo před ní. Vložení je **jedna položka historie**
a vložený titulek je vybraný: můžete ho hned psát.

## Výběr od pozice

Oba se řídí pořadím souboru, ne aktuálním výběrem. **Bez souseda na této straně**
přejde výběr u `Select Next…` na poslední titulek souboru, u
`Select Previous…` na první — chování Gaupolu. Tyto dva úkony filmem nepohnou.

## Snímek po snímku

Posun vpřed a vzad **o jeden snímek** — nebo o N, viz níže —, abyste nastavili okraj **přesně na snímek**.
Dva úkony a dvě tlačítka na liště přehrávání, po obou stranách tlačítka přehrávání:

| Příkaz | Zkratka | Co dělá |
| :------ | :------- | :----------- |
| Video ▸ O snímek zpět | `Alt+Left` | vrátí film o krok ve snímcích |
| Video ▸ O snímek vpřed | `Alt+Right` | posune ho vpřed o krok ve snímcích |

- **Krok je počet snímků, nejméně jeden**: je to nastavení `Frame step` v
  [předvolbách](preferences.md#přehrávač). Krok o N snímcích ukáže snímek **o N dál**, při 25 snímcích za
  sekundu stejně jako při 23,976; **nikdy se nevyjadřuje v milisekundách**.
- **Film se zastaví na svých mezích**: krok zpět z prvního snímku ho tam ponechá, krok vpřed z posledního
  také. Po kroku je přehrávání zastaveno.
- **Tlačítka lišty** — dvě ikony po obou stranách přehrát/pozastavit — **spouštějí tytéž akce jako nabídka**: stejný krok, nedostupná
  bez videa jako položky nabídky a se stejnou bublinovou nápovědou, která uvádí zkratku. **Prst, který zůstane na
  tlačítku nebo na klávese, řetězí kroky.**
- **Jeden krok po druhém.** Držená klávesa se opakuje rychleji, než se krok provede: první opakování odejde
  ihned, ta, která přijdou během provádění kroku, **se nehromadí** — sdílejí jedno místo, to
  poslední, které odejde potom. Uvolnění klávesy tedy zastaví obraz tam, kde jste, a nenechá film
  pokračovat setrvačností.

## Posun okraje o jeden snímek

K úpravě okraje z klávesnice: **začátek nebo konec vybraného titulku**, o jeden krok, dříve nebo později.

| Příkaz | Zkratka | Co dělá |
| :------ | :------- | :----------- |
| Video ▸ Posunout začátek dříve | `Alt+Q` | titulek **začne o krok dříve** |
| Video ▸ Posunout začátek později | `Alt+W` | **začne o krok později** |
| Video ▸ Posunout konec dříve | `Alt+Shift+Q` | **skončí o krok dříve** |
| Video ▸ Posunout konec později | `Alt+Shift+W` | **skončí o krok později** |

`Q` a `W` jsou v Gaupolu pro *dříve* a *později*, s `Alt` z výše uvedeného důvodu; konec
dostává `Alt+Shift`, protože `E` z Gaupolu je zde písmenem `Edit`.

- **Jedna položka historie na každý posun**, kterou lze vrátit zpět. Sousední titulek a překryv se hlásí
  **stejně jako při psaní**: okraj se nastaví a tabulka označí, co už neplatí — viz
  [Anomálie](table.md#anomálie).
- **Stejný krok jako u přehrávače**, tedy ve výchozím stavu jeden snímek. **Kolik snímek trvá**, se čte v tomto pořadí
  ze **snímkové frekvence, kterou uvádí video**, ze snímkové frekvence **dokumentu, který počítá ve snímcích** (MicroDVD), a pak
  z **mřížky, kterou tvoří pozice**. **Bez žádné z těchto tří úkon odmítne a řekne to**: zvolit
  snímkovou frekvenci nahodile by posunulo okraj o něco, co snímkem není.
- **Nevyžaduje video**, jen výběr: použije se také u souboru počítaného ve snímcích, bez
  filmu. Když je otevřen film, **přehrávání přejde na novou pozici**, aby ukázalo snímek, který okraj
  právě zaujal.
- Okraj nejde před počátek: krok zpět z méně než jednoho snímku ho nastaví na nulu.
