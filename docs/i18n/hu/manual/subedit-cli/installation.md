# Telepítés

<!-- machine-translated -->
> Ez az oldal gépi fordítás, amelyet a nyelv egyik olvasója még nem nézett át.

**Ez az oldal mindkét programra érvényes** — a `subedit-cli` és a `subedit-gui` programra. Együtt készülnek és együtt települnek.

Két út van: **natív csomag**, `.deb` vagy `.rpm`, vagy **fordítás forráskódból**, amely aztán oda telepít, ahová akarják.

## A csomagok

Mindkettőt a CPack állítja elő **ugyanazokból a telepítési szabályokból**: ugyanazokat a fájlokat teszik ugyanazokra a helyekre, és csak azoknak a csomagoknak a nevében különböznek, amelyektől függenek — ugyanazokat a könyvtárakat másként hívják a Debianon és a Fedorán.

```bash
sudo apt install ./subedit_<version>_amd64.deb      # Debian, Ubuntu
sudo dnf install ./subedit-<version>.x86_64.rpm     # Fedora, and relatives
```

| Mit telepít a csomag | Hová |
| :---------------------- | :-- |
| `subedit-cli`, `subedit-gui` | `/usr/bin` |
| a menübejegyzés, az ikon, a szoftverközpont metaadatai | `/usr/share/applications`, `/usr/share/icons`, `/usr/share/metainfo` |
| ez a kézikönyv, Markdown formátumban | `/usr/share/subedit/manual` |
| a fordítása minden telepített nyelvre — katalán, cseh, német, spanyol, finn, francia, galíciai, héber, magyar, interlingue, izlandi, olasz, holland, lengyel, portugál, brazil portugál, orosz, szerb, török, egyszerűsített kínai | `/usr/share/subedit/manual/<language>` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR`, `ru`, `sr`, `tr`, `zh_CN` |
| a Gaupol javítómintái | `/usr/share/subedit/patterns` |
| a `subedit-cli` kézikönyvoldalai alparancsonként egy, és a `subedit-gui` kézikönyvoldala — lásd [A kézikönyvoldalak](invocation.md#a-kézikönyvoldalak) | `/usr/share/man/man1` |
| a `subedit-cli` oldalai minden telepített nyelven — katalán, cseh, német, spanyol, finn, francia, galíciai, héber, magyar, interlingue, izlandi, olasz, holland, lengyel, portugál, brazil portugál, orosz, szerb, török, egyszerűsített kínai | `/usr/share/man/<language>/man1` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR`, `ru`, `sr`, `tr`, `zh_CN` |

Az `ffmpeg` mindkét csomagnál **ajánlott, és nem kötelező**: nélküle az ablak csak azt nem kínálja fel többé, hogy a film által megadott képkockasebességet használja — lásd [mit változtat az `ffmpeg`](../subedit-gui/video.md#az-ffmpeg-nem-kötelező).

### Amit a csomagok megkövetelnek a disztribúciótól

**Telepítés `apt` vagy `dnf` paranccsal történjen, soha `dpkg -i` vagy `rpm -i` paranccsal.** Ez a két parancs a függőségek feloldása nélkül teszi le a csomagot: csak felsorolja a hiányzókat, és megáll. Ezt feleli az `rpm -i` egy olyan Fedorán, amelyen megvan, amire szüksége van, a `dnf` hatókörében:

```console
$ rpm -i subedit-0.13.0-1.fc43.x86_64.rpm
error: Failed dependencies:
        libicui18n.so.77()(64bit) is needed by subedit-0.13.0-1.fc43.x86_64
        libicuuc.so.77()(64bit) is needed by subedit-0.13.0-1.fc43.x86_64
```

A `sudo dnf install ./subedit-<version>-1.fc43.x86_64.rpm` letölti ezeket a könyvtárakat, ahol léteznek.

**A használandó `.rpm` az adott Fedoráé**: a fájlnév hordozza a verziót, `fc42`, `fc43`, `fc44`. Mindegyik az adott Fedorán készül, az ottani ICU-val szemben — a `cat /etc/fedora-release` megmondja, melyiket kell venni. Egy másik Fedora `.rpm` csomagja olyan ICU-t kér, amilyen a gépen nincs.

| Függőség | `.deb` | `.rpm` | Megkötés |
| :--------- | :----- | :----- | :--------- |
| Qt 6: `Widgets`, `Gui`, `Core` | `libqt6widgets6`, `libqt6gui6`, `libqt6core6` | `qt6-qtbase-gui` | **≥ 6.4**, az ablakhoz |
| libmpv | `libmpv2` vagy `libmpv1` | `mpv-libs` | a bináris a `libmpv.so.2` könyvtárhoz linkel |
| Enchant 2 | `libenchant-2-2` | `enchant2` | a helyesírás-ellenőrző, valamint a `join-words` és a `split-words` feladat a [`correct`](correct.md) parancsban |
| **ICU** | **`libicu74`** | `libicu`, vagyis a `libicuuc.so.N` és a `libicui18n.so.N` **annak a Fedorának, amely lefordította** | **az a verzió, amelyet az `.rpm` Fedorája tartalmaz**, pontosan — lásd alább |
| glibc és libstdc++ | — | — | **elég friss**: az Ubuntu 24.04 könyvtárai a `.deb` esetén, az `.rpm` Fedorájának könyvtárai az utóbbi esetén |

**Az ICU az a megkötés, amely számít, és nem lazítható a csomag egyetlen sorának módosításával.** Az ICU szimbólumai a főverziója számát hordozzák (`u_strToUTF8_74`): az ICU 74-gyel szemben fordított bináris nem linkelhet az ICU 76-hoz vagy 77-hez. A tágabban megírt függőség engedné a csomag telepítését, majd indításkor hibázna — a pontos megadás az, ami őszinte marad. **Ezért az `.rpm` nem egyszer készül, hanem minden kiadott Fedorán**, az ottani ICU-val szemben; a `.deb` Ubuntu 24.04-en készül.

| Disztribúció | Mit kap |
| :----------- | :---------------- |
| Ubuntu 24.04 | a `.deb` települ: az ICU ott a 74-es verzió |
| Fedora 42, 43, 44 | a verziójának `.rpm` csomagja (`…fc42…`, `…fc43…`, `…fc44…`) települ **a `dnf`-en át**, az adott Fedora ICU-jával szemben fordítva |
| Fedora rawhide (a következő), a többi `.rpm`-disztribúció | **nem**: számukra nem készül `.rpm` — fordítás forráskódból |
| bármely Debian- vagy Ubuntu-disztribúció, amelynek az ICU-ja nem a 74-es verzió | **nem**: a `.deb` a `libicu74` csomagot követeli meg — fordítás forráskódból |

**A forráskódból fordítás a tartalék megoldás, és nincs meg ez a megkötése**: a gép ICU-jához linkel, bármilyen verziójú is. Arra a disztribúcióra is ez kell, amelynek a glibc könyvtára régebbi a csomagokénál.

### Mit tesztelnek mindkettőből, és mit nem

**A kettőt nem ellenőrzik ugyanolyan messzire, és jobb kimondani, mint olyan egyenértékűségben hagyni hinni, amely nem létezik.** A fejlesztés Ubuntun folyik; az ott fordított `.rpm` nem telepíthető ott. Máshol települ: Fedorán egy konténerben, hetente egyszer.

| | `.deb` | `.rpm` |
| :--- | :----- | :----- |
| a fájllista | ellenőrzött | ellenőrzött, és **összevetve a `.deb` listájával** |
| a megadott függőségek | meglétük ellenőrzött | meglétük ellenőrzött |
| hogy a függőségek nevei léteznek a disztribúcióban | igen, Ubuntu nevei | **igen**, a `dnf` feloldja |
| hogy a csomag települ | nem — ahhoz rendszergazdai jogok kellenek | **igen**, Fedorán egy konténerben |
| hogy a telepített binárisok elindulnak | igen, ideiglenes előtagból | **igen**, a telepített csomagból |

**A Fedora-ellenőrzés az Ubuntun fordított `.rpm` csomagot teszteli, nem a kiadottakat.** Annak a képnek nincs saját ICU 74-e: a csomag azért települ ott, mert a Fedora kompatibilitási `libicu74` csomagot tart, és az ellenőrzés addig lenne zöld, amíg ez a csomag létezik. A kiadott `.rpm` fájlok a saját Fedorájukon készülnek, és már nem függenek ettől a kompatibilitástól. Lásd [mit követelnek meg a csomagok](#amit-a-csomagok-megkövetelnek-a-disztribúciótól).

**A két lista összevetése az az ellenőrzés, amely a legtöbbet számít.** A két csomag ugyanabból a telepítésből kerül ki: a köztük lévő eltérés a telepítési szabályok hibája lenne, nem a formátumé.

**Amit a `.deb` még mindig nem bizonyít, az az, hogy települ**, és az ok nem változott: a `dpkg -i` rendszergazdai jogokat kíván, amelyekkel a minőségi kapu nem rendelkezik, és nem is kérhet. Az `.rpm` ezzel szemben települ egy olyan konténerben, amely senkié. Lásd [ADR 0023](../../../../adr/0023-deb-et-rpm-pour-la-premiere-livraison.md).

**A Flatpak és az AppImage nem kínált**, és ez döntés — nem mulasztás. Ugyanaz az ADR magyarázza, és **végleges**: az a disztribúció, amely sem a Debian-, sem a Fedora-családba nem tartozik, forráskódból fordít, amelyet a következő szakasz teljesen leír.

## Fordítás forráskódból

### Előfeltételek

| Mi szükséges | Miért | Debian, Ubuntu |
| :------------ | :------- | :------------- |
| CMake ≥ 3.28 | a fordítás | `cmake` |
| C++23-fordító — a GCC 13 megfelel | — | `g++` |
| `make`, `pkg-config` | a fordítás | `make`, `pkg-config` |
| CLI11 | a parancssor olvasása | `libcli11-dev` |
| Qt 6 ≥ 6.4, `Widgets` modul | az ablak | `qt6-base-dev` |
| `libmpv` | a beépített lejátszó | `libmpv-dev` |
| ICU | a kódolások olvasása | `libicu-dev` |
| Enchant 2 | a helyesírás-ellenőrző könyvtára | `libenchant-2-dev` |

**Az utolsó négy még akkor is kell, ha csak a `subedit-cli` programot fordítják**: a CMake-konfiguráció az egész projekthez keresi őket, és megáll, ha hiányoznak. Az ICU és az Enchant kétszeresen is kell — a futtatható fájl betölti őket, az egyik a kódolások átalakítására, a másik a helyesírás-ellenőrzőhöz —, és ezek az egyetlen külső könyvtárak, amelyeket futásidőben betölt.

**Hálózati kapcsolat az első `cmake` futtatásakor**: a Catch2 tesztkönyvtárat a GitHubról töltik le a konfiguráláskor. Utána nem töltik le újra. Az eszköz önmagának fordításához, hálózat nélkül, lásd alább.

#### Ami nem kötelező

| Mi választható | Mi vész el nélküle |
| :-------------------- | :--------------------- |
| az `ffmpeg`, az `ffprobe` miatt | az ablak nem kínálja fel többé a film által megadott képkockasebességet — [a részletek](../subedit-gui/video.md#az-ffmpeg-nem-kötelező) |
| megjelenítőkiszolgáló | semmi a `subedit-cli` esetén; a `subedit-gui` programnak kell egy, hogy megjelenítse magát |

### Fordítás

```bash
git clone git@github.com:Guyot-Bertrand/sub-edit.git subedit
cd subedit
make build
```

A binárisok a `build/dev/bin/` könyvtárban készülnek — `subedit-cli` és `subedit-gui`.

Optimalizált változathoz:

```bash
make release
```

Mindkét bináris ekkor a `build/release/bin/` könyvtárban van — `subedit-cli` és `subedit-gui`. A cél mást nem fordít: sem a teljesítménymérőt, sem a végpontok közötti tesztkörnyezetet, amelyeknek saját céljaik vannak.

**Ezt a célt részesítsék előnyben a két egyenértékű `cmake` parancs helyett**, mert azok nem tudhatják, hány folyamatot indíthat a linkelésidejű optimalizálás: minden linkeléskor annyit indít, ahány mag van. A `make release` átadja neki a `JOBS` értéket, ahogy a fordítás többi részének is.

### Telepítés

```bash
make release
cmake --install build/release --prefix ~/.local
```

Ami települ, a `GNUInstallDirs` által a választott előtaghoz megadott útvonalakon:

| Fájl | Hová |
| :------ | :-- |
| `subedit-cli`, `subedit-gui` | `<prefix>/bin` |
| az asztali menübejegyzés | `<prefix>/share/applications` |
| az ikon | `<prefix>/share/icons/hicolor/scalable/apps` |
| a szoftverközpont metaadatai | `<prefix>/share/metainfo` |
| ez a kézikönyv, Markdown formátumban | `<prefix>/share/subedit/manual` |
| a fordítása minden telepített nyelvre — katalán, cseh, német, spanyol, finn, francia, galíciai, héber, magyar, interlingue, izlandi, olasz, holland, lengyel, portugál, brazil portugál, orosz, szerb, török, egyszerűsített kínai | `<prefix>/share/subedit/manual/<language>` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR`, `ru`, `sr`, `tr`, `zh_CN` |
| a Gaupol javítómintái | `<prefix>/share/subedit/patterns` |
| mindkét bináris kézikönyvoldalai, a `subedit-cli` esetén alparancsonként egy | `<prefix>/share/man/man1` |
| a `subedit-cli` oldalai minden telepített nyelven — katalán, cseh, német, spanyol, finn, francia, galíciai, héber, magyar, interlingue, izlandi, olasz, holland, lengyel, portugál, brazil portugál, orosz, szerb, török, egyszerűsített kínai | `<prefix>/share/man/<language>/man1` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR`, `ru`, `sr`, `tr`, `zh_CN` |

**Ugyanazok a fájlok, amelyeket a csomagok telepítenek**: a csomagok ezekből készülnek, ahelyett hogy másodszor is le kellene írni őket.

A `--prefix ~/.local` kapcsolóval mindkét bináris a `~/.local/bin` könyvtárba kerül, amely a legtöbb disztribúció `PATH` értékében szerepel. A rendszerszintű előtag — például a `/usr/local` — megfelelő jogokat kíván.

**A `~/.local` alá tett menübejegyzés nem mindig jelenik meg azonnal**: az asztali környezetek a saját ütemükben olvassák újra a gyorsítótárukat. A `subedit-gui` közben elindul terminálból, mint bármely más parancs.

A `DESTDIR` figyelembe vett, ahogy azt a csomagkészítő elvárja: az előkészített telepítés semmihez sem nyúl az előkészítő könyvtáron kívül.

```bash
DESTDIR=/tmp/scene cmake --install build/release --prefix /usr
```

### Az eszköz önmagában történő fordítása

A tesztek alapértelmezés szerint épülnek, és ők kérik a hálózatot — a `make release` nem fordítja őket, de konfigurálja a projektet, és a konfigurálás tölti le a Catch2 könyvtárat. Elég egyszer kikapcsolni őket: a beállítás a CMake gyorsítótárában marad, és a következő fordítások megtartják.

```bash
cmake --preset release -DSUBEDIT_BUILD_TESTS=OFF
make release
```

Ez a közreműködőt semmi alól sem menti fel: a minőségi kapu megköveteli őket.

### Ha a fordítás linkeléskor hibázik

Az érvényes kódon megjelenő „undefined reference to `std::cout`” hibaáradat általában azt jelenti, hogy a rendszer `c++` alternatívája a `g++` helyett a `gcc` programra mutat: a C++ lefordul, de a szabványos könyvtár nem linkelődik.

```bash
ls -l /etc/alternatives/c++                                       # check
sudo update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++ 100   # fix
CXX=g++ make build                                                # work around
```

A CMake-konfiguráció felismeri ezt az esetet, és ezzel az üzenettel megáll, ahelyett hogy a linkelést hagyná hibázni.

### Ha a csomag telepítése a `libicu` miatt panaszkodik

```console
error: Failed dependencies:
        libicuuc.so.74()(64bit) is needed by subedit-<version>.x86_64
```

Három ok van, amelyeket a beírt parancs és a fájlnév különböztet meg:

- **`rpm -i`**: semmit sem old fel. Próbálja újra a `sudo dnf install ./subedit-<version>.fcNN.x86_64.rpm` paranccsal;
- **az `.rpm`, amelynek nevében nincs `fcNN`**, vagy a sajátjukétól eltérő Fedoráról való: azon gép ICU-ját kéri, amely lefordította. Azt kell venni, amelynek `fcNN` része a `cat /etc/fedora-release` kimenete;
- **nincs `.rpm` az adott verzióhoz**: Fedora rawhide vagy másik disztribúció. A tároló egyetlen csomagja sem települ ott; [fordítás forráskódból](#fordítás-forráskódból).
