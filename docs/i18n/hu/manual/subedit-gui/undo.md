# Visszavonás és újra

<!-- machine-translated -->
> Ez az oldal gépi fordítás, amelyet a nyelv egyik olvasója még nem nézett át.

Minden művelet bekerül egy előzményekbe, és visszavonható. Egy szerkesztett cella
**egy** bejegyzésnek számít: a gépelés nem billentyűleütésenként kerül rögzítésre, csak a
jóváhagyás számít.

| Művelet | Gyorsbillentyű | Hol |
| :------ | :------------- | :-- |
| Visszavonás | `Ctrl+Z` | **Szerkesztés** menü, eszköztár |
| Újra | `Ctrl+Shift+Z` | **Szerkesztés** menü, eszköztár |

## A művelet megnevezi, mit fog visszavonni

A menü nem azt írja, hogy „Visszavonás”, hanem azt, hogy **„Visszavonás: szöveg szerkesztése”**, a
visszavonandó művelet után. Az eszköztár gombja megtartja a puszta szót — szélessége különben
az utolsó műveletet követné, és az egérmutató alatt elmozdulna —, de a súgószövege a teljes
feliratot hordozza.

| Művelet | Amit a művelet felirata kiír |
| :------ | :--------------------------- |
| egy szerkesztett szövegcella | `editing a text` |
| egy kezdet, egy vég | `editing a start`, `editing an end` |
| egy időtartam | `editing a duration` |
| sorok beszúrása | `inserting` |
| sorok eltávolítása | `removing` |
| egy eltolás, a `Shift Whole File onto Grid` is ideértve | `shifting` |
| egy átalakítás | `transforming` |
| egy képkockasebesség-átváltás | `converting the frame rate` |
| egy igazítás a képkockasebességhez | `aligning on the frame rate` |
| utalások eltávolítása | `removing hearing-impaired mentions` |
| dőlt betűssé tétel | `putting in italics` |
| a dőlt betű eltávolítása | `taking italics out` |
| kis- és nagybetű módosítása | `changing the case` |
| párbeszéd-kötőjelek hozzáadása, eltávolítása | `adding dialogue dashes`, `removing dialogue dashes` |
| sorok egyesítése | `merging` |
| egy sor felosztása | `splitting` |
| kivágott, beillesztett szövegek | `cutting texts`, `pasting texts` |
| időtartamok módosítása | `adjusting durations` |
| egy fordítás megnyitása | `opening a translation` |
| egy fájl hozzáfűzése | `appending a file` |
| a projekt felosztása | `splitting the project` |
| egy elfogadott javítás a `Correct Texts…` ablakban, vagy a `Check Spelling…` által végzett javítás | `correcting texts` |
| egy csere, az összes csere | `replacing`, `replacing all` |

**Huszonhét felirat van, és az ablak ennél többet nem tud előállítani.** A mag egy huszonnyolcadikat is
megnevez — `sorting` —, amelyet az ablak egyetlen művelete sem ér el: a rendezés csak szigorú
sorrendi szabályzat mellett történik, az ablak viszont engedékeny szabályzattal nyitja meg a
dokumentumokat, amely jelzi a rendetlenséget, ahelyett hogy kijavítaná.

**A `Shift Whole File onto Grid` nem különböztethető meg egy eltolástól**, és ez pontos: eltolás
az is, csak a mértékét megmérték, nem begépelték.

Ezek ugyanazok a szavak, mint a parancssorban: egyszer vannak leírva, a magban.

Ha nincs mit visszavonni, a művelet a puszta „Visszavonás” feliratot viseli, **és le van tiltva**: a
gyorsbillentyűje semmit sem csinál.

## A módosításjelző

Az ablak címe jelzést hordoz, amíg a fájl különbözik a lemezen lévőtől — csillagot, vagy amit a
platform helyette használ. **A lap felirata is hordozza**, `premier.srt*`: a cím csak a most
nézett projektről szól, a lap felirata azokról is szól, amelyeket éppen nem néznek.

**Minden lapnak saját előzménye van.** A visszavonás csak azt vonja vissza, amit az aktuális lap
projektje kapott — az összes projektre kiterjedő `Replace All` is ideértve: az minden érintett
projektben egy-egy bejegyzést hagy.

**Eltűnik, ha a kiindulópontig vonják vissza.** Ez nem megjelenítési részlet: a módosítások számát
számolják, nem igen-nem értékként jegyzik, így a visszalépés nullára viheti, és a jelzés eltűnik.

## Két tudnivaló

**A viszonyítási pont az utolsó mentés**, vagy a fájl megnyitása, ha mentés nem volt. A mentés
elmozdítja: a jelzés eltűnik, és a későbbi visszavonás visszahozza.

Egy visszavonás után végzett művelet **törli, ami újra elvégezhető volt**: az újra elvégzés egy olyan
parancsot játszana le, amelynek kiindulási állapota már nem létezik. Ez minden szerkesztő
viselkedése, és szándékos.

**Amíg egy cella nyitva van**, az `Undo` és a `Redo` — a menüből vagy az eszköztárról — előbb
jóváhagyja a bevitelt, mint a többi mozdulat a [Cella szerkesztése](editing.md) oldalon. Ami épp be lett
gépelve, az ezután az előzmények utolsó bejegyzése lesz: az `Undo` művelet azt vonja vissza, nem az előző
műveletet; a `Redo` műveletnek nincs már mit újra elvégeznie, mert egy bevitel törli, ami újra elvégezhető
volt. Egy nyitott cella, amelybe nem gépeltek semmit, nem hagy bejegyzést: az `Undo` művelet ekkor az előző
műveletet vonja vissza.

Az előzmények ezer bejegyzést őriznek, és azon túl elfelejtik a legrégebbit.
