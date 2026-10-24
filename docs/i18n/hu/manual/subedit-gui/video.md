# A társított videó

<!-- machine-translated -->
> Ez az oldal gépi fordítás, amelyet a nyelv egyik olvasója még nem nézett át.

A **Videó** menü először három parancsot hordoz, majd a filmet vezérlő mozdulatokat
— lásd [A lejátszó](player.md#mozgás-a-filmben).

| Parancs | Gyorsbillentyű | Mit csinál |
| :------ | :------------- | :--------- |
| Videó kiválasztása… | nincs | kiválasztja a videófájlt, amelyhez a dokumentum tartozik |
| Hang leiratozása… | nincs | a film hangját a projekt feliratait felváltó feliratokká iratozza le — lásd [A hang leiratozása](#a-hang-leiratozása) |
| Lejátszás / Szünet | `Ctrl+P` | lejátssza vagy megállítja a videót — lásd [A lejátszó](player.md#lejátszás-és-megállítás) |

A társított videó **a megnyitott dokumentum tulajdonsága**, nem az alkalmazás
beállítása: egy másik feliratfájl megnyitása elölről kezdi. Egyetlen fájlba
sem íródik ki, és az ablakkal együtt eltűnik.

## A hangsáv

Egy film több hangsávot is hordozhat — az eredeti változatot, egy szinkront, egy kommentárt. A
**`Videó ▸ Audió ▸ Nyelv`** almenü felsorolja őket, **sávonként egy elemmel**, és **megjelöli a lejátszottat**:

| Amit a fájl megad | Amit az elem kiír |
| :---------------- | :---------------- |
| egy nyelv és egy cím | `1: fra — Original` |
| csak egy nyelv | `2: eng` |
| csak egy cím | `3: Commentary` |
| egyik sem | `1` |

A szám **a sáv helye a filmben**, egyenként számolva, a fájl sorrendjében: két sáv
is kiírhatja, hogy `eng`, és egy fájlnak, amely hallgat a felirataikról, is van első és második sávja.

**Egy elem kiválasztása azt a sávot játssza le.** A jelölés a lejátszó válasza, nem a kattintás: azt a
sávot mutatja, amely éppen szól.

- **Az almenü le van tiltva**, ha nincs videó vagy nincs hangsáv. Egyetlen sáv esetén engedélyezve marad, és
  bejelölve mutatja azt: a film nyelvét közli. A két hangerő-mozdulat, amelyek mellette vannak, minden esetben
  engedélyezve marad: lásd [A hangerő](player.md#a-hangerő).
- **A sávot nem jegyzi meg a program** egyik videóról a másikra, sem egyik munkamenetről a másikra: a sávszám
  a fájlé, és ha átvinnék a következő filmre, ott azt választaná, ami ezt a számot viseli.
  Egy laphoz visszatérve a film azon a sávon nyílik újra, amelyet a lejátszó maga választ.
- Nincs gyorsbillentyű: a nyelvet ritkán váltják, és a menü elég.

## Videó kiválasztása

A párbeszédablak a feliratfájl könyvtárában nyílik meg, és a felismert
videókiterjesztésekre szűr:

    .avi .flv .m2ts .mkv .mov .mp4 .ogg .ogv .vob .webm

Azt is felkínálja, hogy az összes fájl látsszon. A párbeszédablak megszakítása
semmit sem változtat.

A kiválasztott fájl **azonnal megnyílik**: a kép megjelenik az ablakban,
az első pillanatán szüneteltetve. Lásd [A lejátszó](player.md).

**Egy videófájl ejtése az ablakra ugyanezt teszi** — lásd
[Fájlok ejtése az ablakra](files.md#fájlok-ejtése-az-ablakra).

**A videó az aktuális lap projektjéé.** Minden lap a sajátját őrzi;
a lapváltás filmet vált. **Egy laphoz visszatérve a film ott folytatódik, ahol
hagyták**, szüneteltetve, mint minden megnyitás. Egy másik film, amelyet
időközben választottak ehhez a projekthez, ezzel szemben az elejéről indul.

## Az automatikus javaslat

Egy feliratfájl megnyitásakor, **ha nincs videó kiválasztva**,
az ablak keres egyet ugyanabban a könyvtárban: olyan videófájlt, amelynek kiterjesztés
nélküli neve a miénk előtagja, pontig bezárólag.

| Feliratfájl | Talált videó | Miért |
| :---------- | :----------- | :---- |
| `film.fr.srt` | `film.mkv` | a `film` előtagja a `film.fr` névnek |
| `film.en.forced.srt` | `film.mkv` | ugyanígy, a szakaszok számától függetlenül |
| `film.fr.srt` | `film.fr.mkv` a `film.mkv` helyett | a leghosszabb név nyer |
| `film.fr.srt` | egy sem, ha a `film.mkv` **és** a `film.mp4` is ott van | két jelölt nem különböztethető meg |
| `film.fr.srt` | egy sem, ha csak a `fil.mkv` van ott | a `fil` nem pontnál ér véget |

**A javaslat nem választás.** Hagyja magát egy másik
javaslattal felváltani — másik fájl megnyitásával, másik néven történő mentéssel. Egy kifejezett
választást ezzel szemben soha nem írnak felül: a `Select Video…` használata után az
automatikus javaslat többé nem tér vissza mögötte.

Ettől elfogadható a találgatás: kijavítható, és a javítása megmarad.

## Amit az állapotsor mutat

Az állapotsor jobb oldalán egy sor közli, mihez tartozik a dokumentum. Ez
a három közül az utolsó: először a beolvasott fájl [kódolása](files.md#a-kódolás-az-állapotsorban)
áll, utána a pozíciókból kikövetkeztetett [képkockarács](frame-grid.md).

| Helyzet | Amit kiír |
| :------ | :-------- |
| társítva van egy videó | `Video: film.mkv` |
| … és az `ffprobe` megadja a képkockasebességét | `Video: film.mkv, 24000/1001 fps` |
| nincs | `No video` |

**A képkockasebesség a filmmel együtt jár, nem mellette.** Egyetlen tény — amihez
ez a dokumentum tartozik —, és egy harmadik említés egy rangra tenné
[a pozíciókból kikövetkeztetett ráccsal](frame-grid.md), amely egy másik tény.

## A hang leiratozása

A **`Transcribe Audio…`** a film beszédét írja ki feliratként. Addig nem elérhető, amíg nincs megnyitva film: ez
a **projekt filmjének** hangja, és soha nem egy fájl, amelyet kiválasztana — egy hangfájl leiratozásához
használja a [`transcribe`](../subedit-cli/transcribe.md) parancsot a parancssorban.

A párbeszédablak két dolgot kér, és megmondja, el tud-e indulni:

- **a hangsávot**, úgy felsorolva, ahogy az [`Audio`](#a-hangsáv) menü felsorolja, a fájl által
  megadott nyelvvel és címmel;
- **a nyelvet**, két betű, például `en` vagy `fr`. A kiválasztott hangsáv nyelvével indul, ha a fájl
  megadja, követi a hangsávot, amíg Ön nem írt be egyet, és
  **a mező alatti sor megmondja, honnan származik**. Ha a fájl nem ad meg nyelvet a hangsávhoz,
  a sor ezt mondja: a modell ekkor a hangból fogja felismerni, amit csak a többnyelvű modell
  tud, és ha ismeri, írja be. Csak angolra való modellnél, amely semmit sem ismer fel,
  a sor azt mondja, hogy a nyelvet be kell írni.

**Azt is megmondja, mi hiányzik.** Három sor megmondja, megvan-e a motor, az `ffmpeg` és a modell, és
hol; ha valamelyik nincs meg, az `OK` kikapcsolva marad, és az ablak ezt a következőre mutató hivatkozással mondja meg:
[A leiratozó eszközök telepítése](../subedit-cli/install-transcription.md). A `Preferences…` gomb megnyitja
a [beállításokat](preferences.md#leiratozás), ahol a program és a modell megadható — ezek a gépre
vonatkozó tények, nem olyan választások, amelyeket minden leiratozásnál újra meg kell tenni — és az ablak újra megnézi, amikor bezárulnak.

A nyelv megmarad a következő alkalomra. Maga a motor és a feliratok korlátai
[beállítások](preferences.md) (`transcription.*`).

**Ha a projekt már tartalmaz feliratokat, az ablak a lecserélésük előtt rákérdez**, és megmondja,
hányat érint. A munka előtt kérdez, hogy az elutasítás semmibe se kerüljön. Ha a projekt üres, nincs
mit kérdezni.

A munka **a háttérben** fut, egy ablakkal, amely mutatja, meddig jutott a motor, és egy `Cancel`
gombbal; az ablak körülötte használható marad. Amikor véget ér, a projekt feliratait
a leirat váltja fel **egyetlen műveletként**: egy `Undo` visszaadja, ami volt, a fordítási
szövegekkel együtt. Egy sikertelen vagy általa megszakított leiratozás nem változtat semmin — az okot
egy ablak mondja meg, kivéve a megszakítást, amelyet csak az állapotsor jelez.

**Amire szüksége van, azt nem szállítja a program**: egy motor (whisper.cpp), egy modell, és az `ffmpeg` a hang kinyeréséhez.
Lásd: [A leiratozó eszközök telepítése](../subedit-cli/install-transcription.md).

## Ha nincs film

Az ablak a kép helyén egy **`Select Video…`** gombot mutat, ahelyett hogy
semmit sem mutatna. Ez ugyanaz a parancs, mint a `Video` menüé, az egér
közelébe hozva: egy olyan hiányt, amelyre nem lehet hatni, nehéz megkülönböztetni
a hibától.

A gomb eltűnik, amint egy film megnyílik, és a kép **a teljes** helyét
elfoglalja: azt a magasságot, amelyet a táblázat fölötte meghagy — amelyet a
[`window.table-share`](preferences.md) jegyzett meg egyik munkamenetről a másikra —, és
sosem kisebbet a kép legkisebb magasságánál, amelyből a táblázat szükség esetén enged.
Amikor a film elmegy, a gomb visszaveszi ugyanezt a helyet.

**A fájl neve, sosem az útvonala.** Egy kétszáz karakteres útvonal
kitolna mindent a sávból; a teljes útvonal az, amelyet a
párbeszédablak mutatott.

## Az `ffmpeg` nem kötelező

A `subedit-gui` az `ffprobe`-ot — amely az `ffmpeg` részeként érkezik — egyetlen dologról
kérdezi: **a konténer által megadott képkockasebességről**. Filmenként egyszer
olvassa be, a társításkor.

| `ffprobe` mellett | Nélküle |
| :---------------- | :------ |
| az állapotsor a film neve mellé írja a képkockasebességet | csak a nevet írja |
| az átváltási párbeszédablak az alsó mezőben felkínálja a film képkockasebességét | semmit sem kínál fel, és a korábbiak szerint nyílik meg |
| az igazítási párbeszédablak ezzel nyílik meg | a projekt képkockasebességével nyílik meg |

**Semmi más nem függ tőle, és egyetlen művelet sem tagad meg semmit.** A lejátszónak
viszont egyáltalán nincs szüksége az `ffprobe`-ra: a `libmpv`-re támaszkodik, amely
fordítási függőség. A film időtartama, amellyel a végén túlnyúlót jelzik,
a lejátszótól származik, nem az `ffprobe`-tól.

**A leiratozás az az egyetlen dolog, amelynek valóban szüksége van az `ffmpeg`-re**: ezzel nyeri ki a film hangját. Lásd
[A leiratozó eszközök telepítése](../subedit-cli/install-transcription.md).

Ez tehát nem csökkentett üzemmód, hanem **a normál működés** egy olyan gépen,
amelyre sosem telepítették az `ffmpeg`-et: az adat a felhasználótól jön, és az `ffprobe`
felkínálja, ha ott van.

A beolvasott képkockasebesség **pontos tört** — `24000/1001`, nem `23.976`.
Ezért tartják meg az `ffprobe`-ot erre a válaszra: a
lejátszó ugyanezt lebegőpontos számként ismeri, és egy közelítő képkockasebesség
az egész fájlt kissé mellé időzítené újra.
