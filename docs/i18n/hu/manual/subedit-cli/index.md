# subedit-cli — kézikönyv

<!-- machine-translated -->
> Ez az oldal gépi fordítás, amelyet a nyelv egyik olvasója még nem nézett át.

A subedit parancssori eszköze: munka feliratfájlokkal grafikus felület
nélkül, kötegelve vagy szkriptből. A másik programról és a kezdéshez
lásd
[a kézikönyvet](../index.md).

> **Jelenlegi állapot.** A `subedit-cli` tizenhét alparancsa létezik: vizsgálat,
> átalakítás, eltolás, átalakítás két referenciapontból, a képkockasebesség újraidőzítése, igazítás a képkockarácsra, a
> hallássérülteknek szóló utalások eltávolítása, az időtartamok módosítása, szöveg cseréje, a kis- és nagybetű,
> a dőlt betűk és a párbeszéd-kötőjelek módosítása, a feliratsorok sorrendbe állítása, a
> szövegek javítása a Gaupol mintáival, fájlok egymás után fűzése, egy
> fájl kettévágása, és fordítás írása a főfájl pozícióira. Ez a kézikönyv
> azt írja le, ami létezik, soha nem azt, ami tervben van; hogy mi jön ezután, azt
> az [ütemterv](../../../../feuille-de-route.md) tartalmazza.

## Fejezetek

| Fejezet | Tartalom |
| :------ | :------ |
| [Telepítés](installation.md) | az eszköz fordítása és telepítése |
| [Indítás](invocation.md) | globális kapcsolók, kimenetek, kilépési kódok |
| [A beállításfájl](invocation.md#a-beállításfájl) | amit az `adjust` és a `correct` az ablak beállításfájljából olvas, `--config`, `--no-config` |
| [Könyvtárfa feldolgozása](lots.md) | több fájl, egy egész könyvtár, `--recursive` |
| [`inspect`](inspect.md) | megmutatja, miből áll egy fájl |
| [`convert`](convert.md) | fájl írása más formátumban vagy más alakban |
| [`shift`](shift.md) | minden pozíció eltolása ugyanannyival |
| [`transform`](transform.md) | minden pozíció javítása két ismerten helyes pontból |
| [`framerate`](framerate.md) | fájl újraidőzítése egyik képkockasebességről a másikra |
| [`snap`](snap.md) | az időbélyegek visszaigazítása egy képkockasebesség képkockáira |
| [`hearing-impaired`](hearing-impaired.md) | a hallássérülteknek szóló utalások eltávolítása |
| [`adjust`](adjust.md) | az időtartamok módosítása egy olvasási sebességhez és korlátokhoz |
| [`replace`](replace.md) | szöveg cseréje a címkék megrongálása nélkül |
| [`case`](case.md) | a szövegek címszerű, mondatszerű, nagybetűs vagy kisbetűs írásra állítása |
| [`italics`](italics.md) | a szövegek dőlt betűssé tétele, vagy a dőlt betűk eltávolítása |
| [`dialogue-dashes`](dialogue-dashes.md) | párbeszéd-kötőjelek felvétele vagy eltávolítása |
| [`sort`](sort.md) | a feliratsorok visszaállítása a kezdetük szerinti sorrendbe |
| [`join-repeats`](join-repeats.md) | egymás után következő, azonos szöveg darabjaiból egyetlen feliratsor készítése |
| [`disjoint`](disjoint.md) | az átfedő feliratsorok levágása, hogy egyetlen tartomány se fedjen át egy másikat |
| [`repair`](repair.md) | a `join-repeats`, a `sort` és a `disjoint` láncolása egy rosszul tagolt, rendezetlen és átfedő fájlon |
| [`correct`](correct.md) | a szövegek javítása a Gaupol mintáival: utalások, szavak egyesítése és szétválasztása, gyakori hibák, nagybetűs írás, sortörés |
| [`append`](append.md) | fájlok az első után fűzése egyetlen fájlba |
| [`split-file`](split-file.md) | fájl kettévágása, a vége visszavezetve a kezdőpontra |
| [`pair`](pair.md) | fordítás írása a főfájl pozícióira, és annak jelzése, hogyan illeszkedtek a sorai |
| [`config`](config.md) | az ablakkal megosztott műveleti beállítások listázása, megjelenítése és módosítása |
