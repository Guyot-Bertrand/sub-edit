# subedit — kézikönyv

<!-- machine-translated -->
> Ez az oldal gépi fordítás, amelyet a nyelv egyik olvasója még nem nézett át.

A subedit feliratfájlokat olvas, vizsgál és időzít újra. Két programból áll, amelyek
ugyanazt a magot használják, így ugyanazok a szabályaik és ugyanazok a szavaik.

| Program | Mire való | Kézikönyve |
| :-------- | :-------------- | :--------- |
| `subedit-gui` | fájl megnyitása, megtekintése táblázatban, szerkesztése, összevetése a filmmel | [az ablak kézikönyve](subedit-gui/index.md) |
| `subedit-cli` | fájlok kötegelt feldolgozása szkriptből, felület nélkül | [a parancssori kézikönyv](subedit-cli/index.md) |

Mindkettő **a kilenc formátumot** olvassa és írja — SubRip, WebVTT, SubViewer 2,
Sub Station Alpha, Advanced SSA, MicroDVD, MPL2, TMPlayer, LRC — és
a formátumot a tartalmából ismeri fel, nem a kiterjesztéséből. **A kódolást is
a tartalom alapján ismeri fel a program**: a Latin-1, CP1252 vagy UTF-16 kódolású
fájl megnyílik, a program közli, milyen kódolásúként olvasta, és a fájl a saját kódolásában íródik vissza — a
megnyitott, majd változtatás nélkül mentett fájl egyetlen bájtot sem változik.

## Hol érdemes kezdeni

| Cél | Hová érdemes menni |
| :----------------- | :------- |
| az eszköz telepítése | [Telepítés](subedit-cli/installation.md) — mindkét programra érvényes |
| fájl megnyitása és a tartalmának megtekintése | [A táblázat](subedit-gui/table.md) |
| állandó mértékkel eltolódott feliratok újraidőzítése | [A műveletek](subedit-gui/operations.md#shift-positions) vagy [`shift`](subedit-cli/shift.md) |
| más képkockasebességre írt fájl újraidőzítése | [`framerate`](subedit-cli/framerate.md), és [`snap`](subedit-cli/snap.md), ha az időzítés már helyes |
| annak megállapítása, hogy a fájl milyen képkockasebességre készült | [A képkockarács](subedit-gui/frame-grid.md) vagy [`inspect`](subedit-cli/inspect.md) |
| a hallássérülteknek szóló utalások eltávolítása | [A műveletek](subedit-gui/operations.md#remove-hearing-impaired-mentions) vagy [`hearing-impaired`](subedit-cli/hearing-impaired.md) |
| az utalások, a gyakori hibák, a kis- és nagybetű és a sortörés javítása egyetlen menetben | [`Correct Texts…`](subedit-gui/correct-texts.md) vagy [`correct`](subedit-cli/correct.md) |
| a helyesírás ellenőrzése szavanként, a rendszer szótáraival | [`Check Spelling…`](subedit-gui/check-spelling.md) |
| az olvasáshoz túl rövid feliratsorok meghosszabbítása | [`Adjust Durations…`](subedit-gui/operations.md#adjust-durations) vagy [`adjust`](subedit-cli/adjust.md) |
| olyan fájl sorrendbe állítása, amelynek feliratsorai nincsenek sorrendben | [`sort`](subedit-cli/sort.md) |
| egymás után következő, azonos szöveg darabjaiból egyetlen feliratsor készítése | [`Join Repeated Subtitles`](subedit-gui/lines.md#ismétlődő-feliratsorok-egyesítése) vagy [`join-repeats`](subedit-cli/join-repeats.md) |
| rosszul tagolt, rendezetlen és átfedő fájl rendbetétele egy lépésben | [`repair`](subedit-cli/repair.md) |
| a feliratsorok közötti átfedések megszüntetése | [`Make Ranges Disjoint…`](subedit-gui/lines.md#a-tartományok-átfedésmentessé-tétele) vagy [`disjoint`](subedit-cli/disjoint.md) |
| egy film két részének egymás után fűzése | [`Append File…`](subedit-gui/operations.md#append-file) vagy [`append`](subedit-cli/append.md) |
| egy film kettévágása | [`Split Project…`](subedit-gui/operations.md#split-project) vagy [`split-file`](subedit-cli/split-file.md) |
| egy egész fájl kis- és nagybetűinek, dőlt betűinek vagy párbeszéd-kötőjeleinek módosítása | [A műveletek](subedit-gui/operations.md#italic) vagy [`case`](subedit-cli/case.md), [`italics`](subedit-cli/italics.md), [`dialogue-dashes`](subedit-cli/dialogue-dashes.md) |
| szó keresése és cseréje a címkék megrongálása nélkül | [Keresés és csere](subedit-gui/find-and-replace.md) vagy [`replace`](subedit-cli/replace.md) |
| egy film beszédének kiírása feliratként | [`Transcribe Audio…`](subedit-gui/video.md#a-hang-leiratozása) vagy [`transcribe`](subedit-cli/transcribe.md) — [az eszközök telepítése](subedit-cli/install-transcription.md) után |
| egy feliratfájl újraidőzítése egy film hangjára | [`Synchronize with Audio…`](subedit-gui/video.md#a-hanggal-való-szinkronizálás) vagy [`sync`](subedit-cli/sync.md) — [az eszközök telepítése](subedit-cli/install-transcription.md) után |
| fordítás az eredeti szöveg mellett | [Fordítás megnyitása](subedit-gui/files.md#fordítás-megnyitása) vagy [`pair`](subedit-cli/pair.md), amellyel a főfájlhoz időzíthető újra |
| száz fájl feldolgozása egyszerre | [Könyvtárfa feldolgozása](subedit-cli/lots.md), majd [Indítás](subedit-cli/invocation.md) |

**A `framerate` és a `snap` között a tévedés észrevétlen marad.** Ha
kétséges, melyiket kell használni, a [`snap`](subedit-cli/snap.md#a-snap-nem-a-framerate-és-az-összetévesztésük-nem-látszik)
oldala a kettő összehasonlításával kezdődik.

## Amit ez a kézikönyv ígér

**Azt írja le, ami létezik, soha nem azt, ami tervben van.** Hogy mi fog még jönni, és
milyen sorrendben, azt az [ütemterv](../../../feuille-de-route.md) tartalmazza.

**Ez a kézikönyv angolul készült, és le van fordítva.** A fordítások oldalról oldalra
követik, az adattár `docs/i18n/` könyvtárában, nyelvenként egy fával. A
programok a környezetük nyelvén szólnak; az általuk mutatott neveket — alparancsok,
kapcsolók, menücímkék — itt úgy idézzük, ahogyan a program angolul kiírja.

**A példák és a képek generáltak, a szöveg nem.** A
`console` blokkok az adott parancs tényleges lefuttatásából készülnek; a
képernyőképek a valódi ablak felépítésével és lefényképezésével. Egyik
sem írhat le tehát olyan eszközt, amely már nem létezik. Háló nélkül marad
a körülöttük lévő szöveg, amelynek helyessége a lektoráláson múlik.

**A képernyőképek mindkét palettát mutatják**, a `Light` és a `Dark` változatot — azokat,
amelyeket [a beállítások](subedit-gui/preferences.md#a-téma) kínálnak.

## A két felület ugyanazokat a szavakat használja

Egy ok, amiért érdemes az egyik kézikönyvet olvasni a másik program használatakor: a
hibaüzenetek, a formátumok nevei, a rács megállapításai és a visszavonás
címkéi **egyszer** íródnak meg, a magban, és mindkét program
idézi őket. Az elutasított fájl mindkét oldalon ugyanazt a mondatot adja.

Az különbözik, hogy mit tud az egyes felület:

| | `subedit-gui` | `subedit-cli` |
| :--- | :------------ | :------------ |
| több fájl egyszerre | igen — projektenként egy lap | igen |
| fordítás a főfájl mellett | igen | igen — `-t` a szöveges alparancsoknál, és a [`pair`](subedit-cli/pair.md) az újraidőzítéshez |
| fájl hozzáfűzése, projekt felosztása | igen | igen — [`append`](subedit-cli/append.md), [`split-file`](subedit-cli/split-file.md) |
| visszavonás | igen, ezer bejegyzés | nem értelmezhető — kérés nélkül semmi sem módosul helyben |
| szöveg vagy pozíció kézi szerkesztése | igen | nem |
| feliratsorok beszúrása, törlése, egyesítése és felosztása | igen | nem |
| keresés és csere | igen | igen — [`replace`](subedit-cli/replace.md) |
| szövegek kivágása, másolása és beillesztése | igen | nem |
| az időtartamok, a dőlt betűk, a kis- és nagybetűk és a kötőjelek módosítása | igen | igen — [`adjust`](subedit-cli/adjust.md), [`italics`](subedit-cli/italics.md), [`case`](subedit-cli/case.md), [`dialogue-dashes`](subedit-cli/dialogue-dashes.md) |
| a szövegek javítása mintákkal | igen — `Correct Texts…` | igen — [`correct`](subedit-cli/correct.md) |
| a helyesírás ellenőrzése szavanként | igen — `Check Spelling…` | nem |
| a film nézése időzítés közben | igen | nem |
| írás a bemenet fölé | `Save` | `--in-place` |
