# Időzítés a videóból

<!-- machine-translated -->
> Ez az oldal gépi fordítás, amelyet a nyelv egyik olvasója még nem nézett át.

*Feltételezett, hogy a film meg van nyitva, és a lejátszó ismert: lásd [A lejátszó](player.md).*

A filmet nézik, megállnak azon a képkockán, ahol egy sornak kezdődnie kell, és **ez a pozíció lesz
a feliratsor kezdete**. Öt mozdulat a **Videó** menüben, azokon a neveken, amelyeken a Gaupol adja őket. A
már meglévőre épülnek: egy pozíció beállítása **ugyanaz a parancs, mint az érték begépelése a
cellába**, a beszúrás pedig **ugyanaz, mint a `Szerkesztés ▸ Feliratok beszúrása…`**.

| Parancs | Gyorsbillentyű | Mit csinál |
| :------ | :------------- | :--------- |
| Videó ▸ Kezdet beállítása a videó pozíciójából | `Alt+U` | a kijelölt feliratsor **kezdetét** a film pozíciójára állítja |
| Videó ▸ Vég beállítása a videó pozíciójából | `Alt+K` | a **végét** a film pozíciójára állítja |
| Videó ▸ Felirat beszúrása a videó pozíciójánál | `Alt+J` | beszúr egy feliratsort, amely a pozíciónál **kezdődik**, és kijelöli azt |
| Videó ▸ Az előző kijelölése a videó pozíciójától | `Alt+Up` | kijelöli az utolsó feliratsort, amely a pozíció **előtt kezdődik** |
| Videó ▸ A következő kijelölése a videó pozíciójától | `Alt+Down` | kijelöli az elsőt, amely a pozíció **után kezdődik** |

**Mind le van tiltva, ha nincs megnyitott videó**, mint a lejátszó mozdulatai. A `Set Start…` és a `Set End…`
akkor is le van tiltva, ha semmi sincs kijelölve: a kijelölés **első során**
hatnak.

**A gyorsbillentyűk a Gaupolé, `Alt`-tal**: ott `U`, `K` és `J` — puszta betűk, amelyeket
az ablak a szerkesztett cella előtt venne át, így egy `k` begépelése egy szövegbe
beállítana egy véget. A `Select Previous…` és a `Select Next…` a Gaupolban `Ctrl+Y` és `Ctrl+U`; a `Ctrl+Y`
egyes rendszereken *újra*, és az `Alt`-os nyilak jobban kifejezik az *előtt* és az *után* fogalmát, mint
két betű. A szerkesztett cella **jóvá van hagyva** a `Set Start…`, a `Set End…` vagy az
`Insert Subtitle…` előtt, mint minden módosítás előtt.

## Egy szél beállítása

- **Mozdulatonként egy előzménybejegyzés**: a `Szerkesztés ▸ Visszavonás` visszaadja az előző pozíciót, és semmi mást.
- **Ugyanazok a szabályok, mint a celláé.** Az a kezdet, amelyet a feliratsor vége után állítottak be, vagy az a vég, amely
  a kezdete előtt van, **nincs elutasítva**: be van állítva, és a táblázat úgy jelöli a feliratsort, ahogy azt,
  amelynek az értékét kézzel gépelték — lásd [Az anomáliák](table.md#az-anomáliák). Semmi sem javul ki némán.
- **Az a pozíció, amely már a szélé, nem csinál semmit**, és nem ad hozzá semmit az előzményekhez.
- **A pozíció mindig egy képkocka eleje**: a lejátszó sosem jelent pozíciót
  egy képkocka belsejében. Egy szél beállítása, majd visszatérés hozzá — például a `Seek Selection Start` művelettel —
  **ugyanazt a képkockát mutatja**.

## Beszúrás a pozíciónál

Az új feliratsor **a pozíciónál kezdődik**, és **három másodpercig** tart, vagy a következő kezdetéig,
ha az korábban jön. A szövege üres. A fájl sorrendjében foglalja el a helyét: mindazok után,
amelyek ennél a pozíciónál vagy előtte kezdődnek. A beszúrás **egy előzménybejegyzés**,
és a beszúrt feliratsor ki van jelölve: azonnal gépelni lehet bele.

## Kijelölés a pozíciótól

Mindkettő a fájl sorrendjét követi, nem az aktuális kijelölést. **Ha nincs szomszéd az adott oldalon**, a
kijelölés a fájl utolsó feliratsorára ugrik a `Select Next…` esetén, az elsőre a
`Select Previous…` esetén — ez a Gaupol viselkedése. Ez a két mozdulat nem mozdítja a filmet.

## Képkockánként

Előre és hátra **egy képkockával** — vagy N-nel, lásd lejjebb —, hogy egy szél **pontosan a képkockára** kerüljön.
Két mozdulat, és két gomb a lejátszósávban, a lejátszás gomb két oldalán:

| Parancs | Gyorsbillentyű | Mit csinál |
| :------ | :------------- | :--------- |
| Videó ▸ Léptetés hátra | `Alt+Left` | a képkockalépésnyit hátra viszi a filmet |
| Videó ▸ Léptetés előre | `Alt+Right` | a képkockalépésnyit előre viszi |

- **A lépés képkockák száma, legalább egy**: ez a `Frame step` beállítás a
  [beállításokban](preferences.md#a-lejátszó). Az N képkockás lépés az **N-nel arrébb lévő** képkockát mutatja, 25 képkocka/másodpercnél
  éppúgy, mint 23,976-nál; **sosem ezredmásodpercben van kifejezve**.
- **A film megáll a határainál**: az első képkockáról hátralépve ott marad, az utolsóról előrelépve
  szintén. A lejátszás a lépés után megáll.
- **A sáv gombjai** — két ikon, a lejátszás/szünet két oldalán — **ugyanazokat a műveleteket váltják ki, mint a menü**: ugyanaz a lépés, letiltva,
  ha nincs videó, mint a menü elemei, és ugyanaz a súgószöveg, amely megadja a gyorsbillentyűt. **Az az ujj, amely egy
  gombon vagy a billentyűn marad, láncba fűzi a lépéseket.**
- **Egyszerre egy lépés.** A lenyomva tartott billentyű gyorsabban ismétel, mint ahogy a lépés megtörténik: az első ismétlés azonnal
  kimegy, azok, amelyek a lépés közben érkeznek, **nem halmozódnak fel** — egy helyen osztoznak, az
  utolsón, amely utána megy ki. A billentyű felengedése így ott állítja meg a képet, ahol tart, és nem hagyja, hogy a film
  a lendületével továbbmenjen.

## Egy szél tolása egy képkockával

Egy szél beállításához a billentyűzettel: **a kijelölt feliratsor kezdete vagy vége**, egy lépéssel, korábbra vagy későbbre.

| Parancs | Gyorsbillentyű | Mit csinál |
| :------ | :------------- | :--------- |
| Videó ▸ Kezdet korábbra | `Alt+Q` | a feliratsor **egy lépéssel korábban kezdődik** |
| Videó ▸ Kezdet későbbre | `Alt+W` | **egy lépéssel később kezdődik** |
| Videó ▸ Vég korábbra | `Alt+Shift+Q` | **egy lépéssel korábban ér véget** |
| Videó ▸ Vég későbbre | `Alt+Shift+W` | **egy lépéssel később ér véget** |

A `Q` és a `W` a Gaupolé a *korábbra* és a *későbbre* számára, az `Alt`-tal a fent említett okból; a vég
az `Alt+Shift` kombinációt kapja, mert a Gaupol `E` billentyűje itt a `Edit` menü betűje.

- **Tolásonként egy előzménybejegyzés**, visszavonható. A szomszédos feliratsort és az átfedést
  **úgy jelzik, mint gépeléskor**: a szél be van állítva, és a táblázat jelöli, ami már nem áll — lásd
  [Az anomáliák](table.md#az-anomáliák).
- **Ugyanaz a lépés, mint a lejátszóé**, tehát alapértelmezés szerint egy képkocka. **Hogy mennyit ér egy képkocka**, azt ebben a sorrendben
  olvassák, **a videó által megadott képkockasebességből**, a **dokumentuméból, ha az képkockákban számol** (MicroDVD), majd
  **a pozíciók által rajzolt rácsból**. **Ha a háromból egy sincs, a mozdulat elutasít, és ezt kimondja**: egy
  találomra választott képkockasebesség bármely szélt olyasmivel tolna el, ami nem képkocka.
- **Nem igényel videót**, csak kijelölést: képkockákban számolt fájlon is használható, film
  nélkül. Ha egy film meg van nyitva, **a lejátszás az új pozícióra ugrik**, hogy megmutassa azt a képkockát, amelyet a szél
  az imént felvett.
- Egy szél nem megy a kezdőpont elé: egy lépéssel hátratolva, egy képkockánál kisebb értékről nullára kerül.
