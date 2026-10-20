# Szövegek kivágása, másolása és beillesztése

<!-- machine-translated -->
> Ez az oldal gépi fordítás, amelyet a nyelv egyik olvasója még nem nézett át.

Az `Edit` menü három eleme, az `Undo` és a `Redo` alatt, valamint a
`Find and Replace…` fölött.

| Elem | Gyorsbillentyű | Mit csinál |
| :--- | :------------- | :--------- |
| Szövegek kivágása | `Ctrl+X` | átmásolja a kijelölés szövegeit, majd kiüríti azokat |
| Szövegek másolása | `Ctrl+C` | átmásolja a kijelölés szövegeit |
| Szövegek beillesztése | `Ctrl+V` | beírja a másolt szövegeket az első kijelölt sortól kezdve |

**Csak a szövegek utaznak, semmi más**: sem kezdet, sem vég, sem teljes
sor. A beillesztés ezért nem mozdít el egyetlen pozíciót sem — a szöveget kapó sor
időzítése a sajátja marad. Ezt teszi a Gaupol is.

## Amire hatnak

**Fordítás esetén az aktuális oszlop szövege**: a fordítás, ha az aktuális
cella a `Translation` oszlopban van, egyébként a fő szöveg —
lásd [A táblázat](table.md#a-szöveg-amelyre-a-műveletek-vonatkoznak). A `Cut Texts` ezért kiüríti
a fordítást, és meghagyja a fő szöveget, a `Paste Texts` pedig abba az oszlopba ír,
amelyben a kurzor áll. **A vágólap nem hordoz dokumentumot**: egy fő szövegű cella
másolható, és beilleszthető a fordításba, és fordítva is.

**A kijelölés, és sosem az egész fájl.** A három elem le van tiltva, amíg
semmi sincs kijelölve. Az ablak más helyein a „semmi sincs kijelölve” azt jelenti, hogy
„az egész fájl”; itt egy szerencsétlen `Ctrl+X` kiürítené a dokumentum
összes szövegét, a beillesztésnek pedig nem lenne sora, ahonnan indulhatna.

Amíg egy cella nyitva van, a `Ctrl+X`, a `Ctrl+C` és a `Ctrl+V` a beviteli mezőhöz
tartozik: karaktereket vágnak, másolnak és illesztenek be, ahogy a `Del` is
karaktert töröl ott, nem feliratsort. A menüből indítva az elemek először jóváhagyják
a bevitelt: a `Copy Texts` ekkor a szöveget úgy másolja, ahogy az imént be lett
gépelve. Lásd [Cella szerkesztése](editing.md).

## Másolás

A `Copy Texts` minden kijelölt sor szövegét másolja, az elsőtől az
utolsóig. **A megszakított kijelölés megtartja az alakját**: a kijelölésből kihagyott sor
lyukká válik, és a beillesztés nem írja felül.

**Az a kijelölt sor is lyukká válik, amelynek a szövege üres**:
beillesztve a célsort érintetlenül hagyja. Egyetlen üres szöveg másolása ezért
semmit sem illeszt be.

A rendszer vágólapja ezeket a szövegeket **egyszerű szövegként, üres sorral elválasztva**
kapja meg — a lyuk ott üres szövegként van leírva, és ezért az üres
szöveg nem lehet más, mint lyuk: a beillesztés ugyanazt az eredményt adja,
akár ebből az ablakból, akár másik programból származik a másolat. Amit itt másolnak,
az ezért bármely programba beilleszthető, és amit máshol másolnak, az ide is.

A másolás nem módosítja a dokumentumot, és nem kerül be az előzményekbe.

## Kivágás

A `Cut Texts` másol, majd **kiüríti a kijelölt sorok szövegeit**. A sorok
megmaradnak a pozícióikkal: a kivágás nem eltávolítás — sorok eltávolítására
a [`Remove Subtitles`](lines.md#eltávolítás) szolgál.

Az olyan szövegek kivágása, amelyek már mind üresek, nem csinál semmit, és nem kerül
be az előzményekbe.

## Beillesztés

A `Paste Texts` a vágólap szövegeit **az első kijelölt sortól kezdve** írja be,
szövegenként egy sort, lefelé haladva.

| Mi van a vágólapon | Mi történik |
| :----------------- | :---------- |
| egy szöveg | felváltja a sor szövegét |
| egy lyuk, vagy egy másolt üres szöveg | a sor megtartja a szövegét |
| több szöveg, mint ahány sor maradt | a végén sorok adódnak hozzá a fogadásukra |

A hozzáadott sorok úgy születnek, mint az utolsó után beszúrt sorok: egyenként
három másodperc, egymás után. **Az ablak ezt kimondja**, egy tájékoztató ablakban:

```text
inserted 2 subtitles to fit the clipboard
```

A beillesztés után **a beírt sorok ki vannak jelölve**.

Az a beillesztés, amely nem változtat semmin — ugyanazok a szövegek ugyanazokon a sorokon —, nem kerül be
az előzményekbe.

### Más formátum címkéit átalakítja a program

A programban végzett másolat **megjegyzi a dokumentuma formátumát**, és
ez az emlék túléli egy másik fájl megnyitását. Egy másik formátumú dokumentumba
beillesztve ezért átalakítja a címkéket, ahogy a `Save As…` is:
a `{\i1}Bonjour{\i0}`, amelyet egy Advanced SSA-ból másoltak, `<i>Bonjour</i>` alakban kerül be egy
SubRipbe.

**Amit a célformátum nem tud leírni, az elvész, és ezt az ablak kimondja**, a
`Save As…` szavaival:

```text
pasting SubRip texts into LRC: 1 tag dropped
```

A `Save As…` ablakkal ellentétben előtte nem kérdez semmit: a beillesztés a
`Ctrl+Z` billentyűvel vonható vissza, amely visszaadja a szövegeket eredeti állapotukban.

**A `subedit` programon kívül másolt szövegnek nincs formátuma**, és úgy kerül be, ahogy van:
nincs mit átalakítani a semmiből. Ugyanez a helyzet az ebből az ablakból származó másolattal is, ha
azóta a rendszer vágólapja felváltotta — a rendszer vágólapjának
tartalma számít.

Ha mindkét üzenet esedékes, egyetlen ablakban férnek el, pontosvesszővel
elválasztva.

## Mit mond erről a visszavonás művelete

Minden mozdulat **egy** előzménybejegyzés, akárhány
sort ír vagy ad hozzá.

| Művelet | Amit az `Undo` kiír |
| :------ | :------------------ |
| kivágás | `Undo: cutting texts` |
| beillesztés | `Undo: pasting texts` |

Lásd [Visszavonás és újra](undo.md).
