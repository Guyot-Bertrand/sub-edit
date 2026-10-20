# Indítás

<!-- machine-translated -->
> Ez az oldal gépi fordítás, amelyet a nyelv egyik olvasója még nem nézett át.

```
subedit-gui [--version] [<file>]
```

| Argumentum | Kötelező | Érték | Alapértelmezés |
| :------- | :------- | :---- | :------ |
| `<file>` | nem | a megnyitandó feliratfájl útvonala | nincs — az ablak üresen nyílik meg |
| `--version` | nem | — | — |

**Egyetlen fájl.** A további argumentumokat a program figyelmen kívül hagyja: az ablak
egyszerre csak egy projektet tart.

## `--version`

A szabványos kimenetre írja azt a blokkot, amelyet a parancssor `--version` kapcsolója ír, és
azonnal visszatér, ablak megnyitása nélkül. Kilépési kód: `0`.

**Az első sorban egyedül a `subedit <version>` áll**, majd egy-egy sor minden könyvtárhoz,
amellyel az ablak fut, a futásidőben talált verzióval és zárójelben azzal, amellyel
lefordították; végül a telepített felületi nyelvek, kód szerint. Qt 6.4.2-es
gépen:

```console
$ subedit-gui --version
subedit <version>
  ICU      74.2 (built with 74.2)
  Enchant  2.3.3 (built with 2.3.3)
  Qt       6.4.2 (built with 6.4.2)
  libmpv   2.2 (built with 2.2)
  Languages: en, fr
```

A számok itt nincsenek átmásolva: a gép saját számai, és ez a kézikönyv nem tartalmaz olyan
generált blokkot, amely a program sajátját naprakészen tartaná. A könyvtárak a
parancssor könyvtárai, az ICU és az Enchant, valamint a két könyvtár, amelyet csak az ablak használ — a Qt és
a libmpv, a lejátszó, amelynek verzióját a kliens API-jának verziója adja meg, az, amelyet az `mpv.pc`
közöl. A többi leírása
[a parancssori kézikönyvben](../subedit-cli/invocation.md#--version) található. A `built with` és a `Languages`
szavak az ablak nyelvét követik; semmi más nem.

A `Súgó ▸ A subedit névjegye` ugyanezeket a könyvtárakat mutatja, ugyanabból a listából, egy
táblázatban — a név, a betöltött verzió, a lefordított verzió, soronként egy könyvtár —, alatta pedig a
nyelveket; a fejlécei az ablak nyelvét követik.

## Fájl megnyitása

```console
$ subedit-gui film.srt
```

Az olvasott formátumok **a kilenc**, amelyet a megnyitás szűrője felsorol — lásd:
[Megnyitás és mentés](files.md#megnyitás). A formátumot a tartalom alapján ismeri fel a program,
nem a kiterjesztés alapján, és **a kódolást is így**: a BOM alapján,
ha a fájlban van, egyébként abból a kódolásból, amelyre a bájtjai utalnak.

## Ha a megnyitás sikertelen

Hiányzó fájl, olvashatatlan fájl, olyan fájl, amelynek bájtjai egyetlen
kódolásban sem fejthetők vissza, vagy olyan, amelynek tartalmát egyetlen formátum sem ismeri fel: **az ablak
ettől még megnyílik, üresen**, a hibát pedig a hibakimenetre írja a program.

**Az üzenet megnevezi az okot**, ugyanazokkal a szavakkal, mint a parancssor:

| Üzenet | Mi történt |
| :------ | :------------ |
| `does not exist` | az útvonal nem jelöl ki fájlt |
| `cannot be opened: permission denied` | a rendszer megtagadja a megnyitást |
| `cannot be read` | a rendszer más okból tagadta meg |
| `cannot be decoded in the chosen encoding` | a bájtok nem olvashatók a kiválasztott kódolásban |
| `is in no format this tool knows` | egyetlen formátum sem ismeri fel a tartalmat |
| `holds nothing recognisable as a subtitle` | a formátum felismerhető, de nincs benne egyetlen felirat sem |

```console
$ subedit-gui notes.txt
subedit-gui: notes.txt: is in no format this tool knows
```

Üres megnyitás a megtagadás helyett: az eszköz így is jó valamire, és egy argumentum
hibája nem ok arra, hogy a program ne induljon el.

**A hibásan felépített, de olvasható fájl megnyílik**, és ami az olvasás közben
kiderült, az állapotsor egyik gombjában olvasható — lásd:
[az olvasás diagnosztikája](files.md#az-olvasás-diagnosztikája).

## Kilépési kódok

| Kód | Mikor |
| :--- | :--- |
| `0` | az ablak megnyílt, majd rendesen bezárták, vagy a `--version` válaszolt |
| `2` | váratlan hiba a megnyitás előtt — hiányzó Qt, elérhetetlen kijelző |

A fájl megnyitásának sikertelensége **nem változtatja meg a kilépési kódot**: az
ablak megnyílt, és a kód ezt jelzi.

## A kézikönyvoldal

A telepítés a `share/man/man1` alá helyezi a `subedit-gui(1)` oldalt:

```console
$ man subedit-gui
```

Megadja a hívás alakját, a `--version` kapcsolót, a megnyitási hibák üzeneteit, a
kilépési kódokat, a két környezeti változót és a beállításfájlt.
**Az ablakot nem írja le** — ezt ez a kézikönyv teszi, és az oldal
ide utal.

## A Qt platform

A `QT_QPA_PLATFORM` választja ki, min fut az ablak. **Ha a változó
be van állítva, a `subedit-gui` argumentum nélkül is tiszteletben tartja.**

Ha nincs beállítva, a választást a Qt-ra bízza: a beépített lejátszó maga az
ablakban rajzolja ki a képet, és semmit nem kér a platformtól — lásd:
[A lejátszó](player.md).

A `QT_QPA_PLATFORM=offscreen` ott futtatja az ablakot, ahol nincs
grafikus kiszolgáló — a folyamatos integráció és a tesztek is így járnak el.

## Nyelv

Az ablak a környezete nyelvén szól, **egyszer választva, az induláskor**: egy futó ablak
változóinak módosítása a következő indításig semmit nem változtat. A változók és a szabályok
a parancssoréi — `LANGUAGE`, `LC_ALL`, `LC_MESSAGES`, `LANG`, az angolhoz `LC_ALL=C`, és
angol **egyetlen szó nélkül**, ha a kért nyelv nincs telepítve — ezeket
[a parancssori kézikönyv](../subedit-cli/invocation.md#nyelv) írja le. A katalógusok ugyanazok a
fájlok, a `<prefix>/share/subedit/locale` alá telepítve.

**A `Szerkesztés ▸ Beállítások…` alatt választott nyelv élvezi az elsőbbséget**, a környezetet pedig csak
akkor olvassa a program, ha a beállítás értéke `System default`, és ez addig így van, amíg nem választanak nyelvet: az ott választott telepített nyelv
minden változót felülír, az `LC_ALL=C` beállítást is. A választás is a következő indításkor lép életbe. Lásd:
[A felület nyelve](preferences.md#a-felület-nyelve).

| Mi | Követi a nyelvet |
| :--- | :------------------- |
| a menük és menüpontjaik, aláhúzott betűikkel; az eszköztár; a buboréksúgók | igen |
| a táblázat és a `Nézet ▸ Oszlopok` oszlopai, a lapok, az állapotsor, a lejátszósáv | igen |
| a szabványos gombok, amelyeket a Qt maga rajzol (`OK`, `Cancel`) | igen, ha a Qt saját katalógusa telepítve van az adott nyelvhez (`qtbase_<language>.qm`); egyébként angol |
| a párbeszédablakok, a `Correct Texts…` oldalai, a fájlpárbeszédek címei és szűrői, az üzenetablakok és amit mondanak | igen |
| a sorok, amelyeket az ablak a hibakimenetére ír, `subedit-gui: …` | igen, a program nevének kivételével |
| a javítási minták nevei a `Correct Texts…` ablakban, és leírásuk a buboréksúgókban | igen, a programmal szállított mintáknál; a saját minta úgy jelenik meg, ahogy a fájlja írja |
| a kódolások, sorvégek és helyesírási nyelvek nevei (`UTF-8`, `CRLF`, `en_US`) | soha: nevek, nem szavak |
| a gyorsbillentyűk | soha: a `Ctrl+S` minden nyelven `Ctrl+S` |
| olyan üzenet, amelyet a katalógus nem fordít | angolul jelenik meg |

Ez a verzió a következőket telepíti: `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt` és `pt_BR` — katalán, cseh, német,
spanyol, finn, francia, galíciai, héber, magyar, interlingue, izlandi, olasz, holland, lengyel, portugál és brazil portugál — teljes fordításban. A franciát a
nyelv egyik olvasója átnézte; a katalán, cseh, német, spanyol, finn, galíciai, héber, magyar, interlingue, izlandi, olasz, holland, lengyel, portugál és brazil portugál gépi fordítású, és a
nyelv olvasója még nem nézte át.
<!-- machine-translated: ca -->
<!-- machine-translated: cs -->
<!-- machine-translated: de -->
<!-- machine-translated: es -->
<!-- machine-translated: fi -->
<!-- machine-translated: gl -->
<!-- machine-translated: he -->
<!-- machine-translated: hu -->
<!-- machine-translated: ie -->
<!-- machine-translated: is -->
<!-- machine-translated: it_IT -->
<!-- machine-translated: nl -->
<!-- machine-translated: pl -->
<!-- machine-translated: pt -->
<!-- machine-translated: pt_BR -->
Francia asztali környezetben a menüsor így szól:
`&Fichier`, `É&dition`, `&Affichage`, `&Vidéo`, `&Outils`, `&Projets`, `Aid&e`. A kézikönyv
képernyőképei magyar nyelvű felületen készülnek.
