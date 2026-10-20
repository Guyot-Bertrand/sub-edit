# Asennus

<!-- machine-translated -->
> Tämä sivu on konekäännös, eikä kukaan tämän kielen lukija ole vielä tarkistanut sitä.

**Tämä sivu koskee molempia ohjelmia** — `subedit-cli` ja `subedit-gui`.
Ne käännetään ja asennetaan yhdessä.

Tapoja on kaksi: **natiivi paketti**, `.deb` tai `.rpm`, tai **kääntäminen
lähdekoodista**, jolloin ohjelman voi asentaa minne haluaa.

## Paketit

Molemmat tuottaa CPack **samoista asennussäännöistä**:
ne sijoittavat samat tiedostot samoihin paikkoihin ja eroavat vain
niiden pakettien nimissä, joista ne riippuvat — samoilla kirjastoilla on
eri nimet Debianissa ja Fedorassa.

```bash
sudo apt install ./subedit_<version>_amd64.deb      # Debian, Ubuntu
sudo dnf install ./subedit-<version>.x86_64.rpm     # Fedora, and relatives
```

| Mitä paketti asentaa | Minne |
| :---------------------- | :-- |
| `subedit-cli`, `subedit-gui` | `/usr/bin` |
| valikkomerkintä, kuvake, sovelluskeskuksen metatiedot | `/usr/share/applications`, `/usr/share/icons`, `/usr/share/metainfo` |
| tämä käyttöopas Markdown-muodossa | `/usr/share/subedit/manual` |
| sen käännös kullekin asennetulle kielelle — katalaani, tšekki, saksa, espanja, suomi, ranska, galicia, heprea, unkari, interlingue | `/usr/share/subedit/manual/<language>` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie` |
| Gaupolin korjauslausekkeet | `/usr/share/subedit/patterns` |
| ohjelman `subedit-cli` man-sivut, yksi kutakin alikomentoa kohden, sekä ohjelman `subedit-gui` man-sivu — katso [Man-sivut](invocation.md#man-sivut) | `/usr/share/man/man1` |
| ohjelman `subedit-cli` sivut kullakin asennetulla kielellä — katalaani, tšekki, saksa, espanja, suomi, ranska, galicia, heprea, unkari, interlingue | `/usr/share/man/<language>/man1` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie` |

Molemmat paketit **suosittelevat ohjelmaa `ffmpeg`, mutta eivät vaadi sitä**: ilman sitä
ikkuna ainoastaan lakkaa tarjoamasta elokuvan ilmoittamaa ruutunopeutta — katso
[mitä `ffmpeg` muuttaa](../subedit-gui/video.md#ffmpeg-ei-ole-pakollinen).

### Mitä paketit vaativat jakelulta

**Asenna komennolla `apt` tai `dnf`, älä koskaan komennolla `dpkg -i` tai `rpm -i`.** Nämä kaksi
komentoa asentavat paketin ratkaisematta sen riippuvuuksia: ne vain
luettelevat puuttuvat riippuvuudet ja pysähtyvät. Näin vastaa `rpm -i`
Fedorassa, jossa kaikki tarvittava on kyllä komennon `dnf` ulottuvilla:

```console
$ rpm -i subedit-0.13.0-1.fc43.x86_64.rpm
error: Failed dependencies:
        libicui18n.so.77()(64bit) is needed by subedit-0.13.0-1.fc43.x86_64
        libicuuc.so.77()(64bit) is needed by subedit-0.13.0-1.fc43.x86_64
```

`sudo dnf install ./subedit-<version>-1.fc43.x86_64.rpm` noutaa nämä
kirjastot, jos ne ovat olemassa.

**Valitse oman Fedorasi `.rpm`**: tiedostonimessä on
versio, `fc42`, `fc43`, `fc44`. Kukin käännetään kyseisessä Fedorassa sen
ICU:ta vasten — `cat /etc/fedora-release` kertoo, mikä valita. Toisen
Fedoran `.rpm` vaatii ICU:n, jota sinun järjestelmässäsi ei ole.

| Riippuvuus | `.deb` | `.rpm` | Rajoite |
| :--------- | :----- | :----- | :--------- |
| Qt 6: `Widgets`, `Gui`, `Core` | `libqt6widgets6`, `libqt6gui6`, `libqt6core6` | `qt6-qtbase-gui` | **≥ 6.4**, ikkunaa varten |
| libmpv | `libmpv2` tai `libmpv1` | `mpv-libs` | ohjelma linkittää kirjaston `libmpv.so.2` |
| Enchant 2 | `libenchant-2-2` | `enchant2` | oikoluku sekä alikomennon [`correct`](correct.md) tehtävät `join-words` ja `split-words` |
| **ICU** | **`libicu74`** | `libicu`, eli **sen Fedoran, joka paketin käänsi,** `libicuuc.so.N` ja `libicui18n.so.N` | **täsmälleen** se versio, joka paketin `.rpm` Fedorassa on — katso alla |
| glibc ja libstdc++ | — | — | **riittävän uudet**: Ubuntu 24.04:n versiot paketille `.deb`, paketin `.rpm` Fedoran versiot sille |

**ICU on se rajoite, jolla on merkitystä, eikä sitä voi lieventää
muuttamalla paketista yhtä riviä.** ICU:n symboleissa on sen
pääversion numero (`u_strToUTF8_74`): ICU 74:ää vasten käännetty ohjelma ei voi
linkittyä ICU 76:een tai 77:ään. Väljemmin kirjoitettu riippuvuus antaisi paketin
asentua mutta epäonnistua käynnistettäessä — sen ilmoittaminen täsmällisenä on se, mikä
pysyy rehellisenä. **Siksi pakettia `.rpm` ei käännetä kerran vaan
jokaisessa julkaistussa Fedorassa**, sen omaa ICU:ta vasten; `.deb` käännetään Ubuntu 24.04:ssä.

| Jakelu | Mitä se antaa |
| :----------- | :---------------- |
| Ubuntu 24.04 | `.deb` asentuu: ICU on siinä versiossa 74 |
| Fedora 42, 43, 44 | sen version `.rpm` (`…fc42…`, `…fc43…`, `…fc44…`) asentuu **komennolla `dnf`**, käännettynä kyseisen Fedoran ICU:ta vasten |
| Fedora rawhide (seuraava versio), muut jakelut, joissa käytetään paketteja `.rpm` | **ei**: niille ei käännetä pakettia `.rpm` — käännä lähdekoodista |
| mikä tahansa Debian- tai Ubuntu-jakelu, jonka ICU ei ole versiossa 74 | **ei**: `.deb` vaatii paketin `libicu74` — käännä lähdekoodista |

**Kääntäminen lähdekoodista on varakeino, eikä sillä ole tätä
rajoitetta**: se linkittyy koneen ICU:hun, oli sen versio mikä tahansa.
Sitä tarvitaan myös jakelussa, jonka glibc on vanhempi
kuin pakettien.

### Mitä kummastakin testataan ja mitä ei

**Kumpaakaan ei tarkisteta yhtä pitkälle, ja se on parempi sanoa kuin antaa
uskoa tasavertaisuuteen, jota ei ole.** Kehitys tapahtuu Ubuntussa; siellä
käännettyä pakettia `.rpm` ei voi asentaa sinne. Se asennetaan muualle:
säiliössä ajettavaan Fedoraan, kerran viikossa.

| | `.deb` | `.rpm` |
| :--- | :----- | :----- |
| tiedostoluettelo | tarkistetaan | tarkistetaan ja **verrataan paketin `.deb` luetteloon** |
| ilmoitetut riippuvuudet | tarkistetaan, että ne ovat mukana | tarkistetaan, että ne ovat mukana |
| että riippuvuuksien nimet ovat olemassa jakelussa | kyllä, ne ovat Ubuntun | **kyllä**, `dnf` ratkaisee ne |
| että paketti asentuu | ei — se vaatii pääkäyttäjän oikeudet | **kyllä**, säiliössä ajettavassa Fedorassa |
| että asennetut ohjelmat käynnistyvät | kyllä, väliaikaisesta etuliitteestä | **kyllä**, asennetusta paketista |

**Fedora-tarkistus testaa Ubuntussa käännettyä pakettia `.rpm`, ei
julkaistuja.** Siinä levykuvassa ei ole omaa ICU 74:ää: paketti asentuu siihen, koska
Fedora pitää yllä yhteensopivuuspakettia `libicu74`, ja tarkistus olisi vihreä niin kauan kuin
se paketti on olemassa. Julkaistut tiedostot `.rpm` käännetään omassa Fedorassaan, eivätkä ne enää
riipu tuosta yhteensopivuudesta. Katso
[mitä paketit vaativat](#mitä-paketit-vaativat-jakelulta).

**Kahden luettelon vertailu on tärkein tarkistus.** Molemmat
paketit syntyvät samasta asennuksesta: ero niiden välillä olisi
asennussääntöjen vika, ei paketin muodon.

**Mitä `.deb` ei vieläkään todista, on se, että se asentuu**, eikä syy
ole muuttunut: `dpkg -i` vaatii pääkäyttäjän oikeudet, joita laatuportilla
ei ole eikä se saa niitä pyytää. `.rpm` taas asentuu
säiliöön, joka ei kuulu kenellekään. Katso
[ADR 0023](../../../../adr/0023-deb-et-rpm-pour-la-premiere-livraison.md).

**Flatpakia ja AppImagea ei tarjota**, ja se on valinta — ei unohdus.
Se perustellaan samassa ADR:ssä, ja se on **lopullinen**: jakelu, joka
ei kuulu Debian- eikä Fedora-perheeseen, kääntää ohjelman
lähdekoodista, minkä seuraava osio kuvaa kokonaan.

## Kääntäminen lähdekoodista

### Edellytykset

| Mitä tarvitaan | Miksi | Debian, Ubuntu |
| :------------ | :------- | :------------- |
| CMake ≥ 3.28 | kääntäminen | `cmake` |
| C++23-kääntäjä — GCC 13 riittää | — | `g++` |
| `make`, `pkg-config` | kääntäminen | `make`, `pkg-config` |
| CLI11 | komentorivin lukeminen | `libcli11-dev` |
| Qt 6 ≥ 6.4, moduuli `Widgets` | ikkuna | `qt6-base-dev` |
| `libmpv` | sisäänrakennettu soitin | `libmpv-dev` |
| ICU | merkistökoodausten lukeminen | `libicu-dev` |
| Enchant 2 | oikoluvun kirjasto | `libenchant-2-dev` |

**Neljä viimeistä vaaditaan, vaikka käännettäisiin vain
`subedit-cli`**: CMake-määritys etsii niitä koko projektille ja
pysähtyy, jos ne puuttuvat. ICU ja Enchant ovat pakollisia kahdesti — ohjelma lataa ne, toisen
merkistökoodausten muuntamiseen, toisen oikolukua varten — ja
ne ovat ainoat kolmannen osapuolen kirjastot, jotka se lataa ajon aikana.

**Verkkoyhteys komennon `cmake` ensimmäisellä ajokerralla**: testikirjasto Catch2
noudetaan GitHubista määritysvaiheessa. Sitä ei ladata uudelleen
sen jälkeen. Pelkän työkalun kääntäminen ilman verkkoa: katso alla.

#### Mitä ei vaadita

| Mikä on valinnaista | Mitä ilman sitä menetetään |
| :-------------------- | :--------------------- |
| `ffmpeg`, sen `ffprobe` | ikkuna ei enää tarjoa elokuvan ilmoittamaa ruutunopeutta — [yksityiskohdat](../subedit-gui/video.md#ffmpeg-ei-ole-pakollinen) |
| näyttöpalvelin | ei mitään ohjelmalle `subedit-cli`; `subedit-gui` tarvitsee sellaisen näkyäkseen |

### Kääntäminen

```bash
git clone git@github.com:Guyot-Bertrand/sub-edit.git subedit
cd subedit
make build
```

Ohjelmat syntyvät hakemistoon `build/dev/bin/` — `subedit-cli` ja
`subedit-gui`.

Optimoitu versio:

```bash
make release
```

Molemmat ohjelmat ovat silloin hakemistossa `build/release/bin/` — `subedit-cli` ja
`subedit-gui`. Kohde ei käännä mitään muuta: ei suorituskykytestiä eikä
päästä päähän -testien ajoympäristöä, joilla on omat kohteensa.

**Suosi tätä kohdetta kahden vastaavan komennon `cmake` sijaan**, sillä niillä
ei ole keinoa tietää, montako prosessia linkitysvaiheen optimointi saa
käynnistää: jokaisessa linkityksessä se käynnistää niitä yhtä monta kuin on ytimiä.
`make release` välittää sille muuttujan `JOBS`, kuten muullekin käännökselle.

### Asentaminen

```bash
make release
cmake --install build/release --prefix ~/.local
```

Mitä asennetaan, poluille, jotka `GNUInstallDirs` määrittää valitulle
etuliitteelle:

| Tiedosto | Minne |
| :------ | :-- |
| `subedit-cli`, `subedit-gui` | `<prefix>/bin` |
| työpöydän valikkomerkintä | `<prefix>/share/applications` |
| kuvake | `<prefix>/share/icons/hicolor/scalable/apps` |
| sovelluskeskuksen metatiedot | `<prefix>/share/metainfo` |
| tämä käyttöopas Markdown-muodossa | `<prefix>/share/subedit/manual` |
| sen käännös kullekin asennetulle kielelle — katalaani, tšekki, saksa, espanja, suomi, ranska, galicia, heprea, unkari, interlingue | `<prefix>/share/subedit/manual/<language>` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie` |
| Gaupolin korjauslausekkeet | `<prefix>/share/subedit/patterns` |
| molempien ohjelmien man-sivut, ohjelmalle `subedit-cli` yksi kutakin alikomentoa kohden | `<prefix>/share/man/man1` |
| ohjelman `subedit-cli` sivut kullakin asennetulla kielellä — katalaani, tšekki, saksa, espanja, suomi, ranska, galicia, heprea, unkari, interlingue | `<prefix>/share/man/<language>/man1` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie` |

**Ne ovat samat tiedostot, jotka paketit asentavat**: paketit syntyvät niistä
sen sijaan, että ne kuvattaisiin toiseen kertaan.

Valitsimella `--prefix ~/.local` molemmat ohjelmat päätyvät hakemistoon `~/.local/bin`,
joka on useimmissa jakeluissa muuttujassa `PATH`. Järjestelmän etuliite —
esimerkiksi `/usr/local` — vaatii vastaavat oikeudet.

**Hakemistoon `~/.local` sijoitettu valikkomerkintä ei aina näy
heti**: työpöydät lukevat välimuistinsa uudelleen omaan tahtiinsa. `subedit-gui` käynnistyy
sillä välin päätteestä, kuten mikä tahansa muu komento.

`DESTDIR` otetaan huomioon, kuten paketoija odottaa: välihakemistoon tehty asennus
ei koske mihinkään välihakemiston ulkopuolella.

```bash
DESTDIR=/tmp/scene cmake --install build/release --prefix /usr
```

### Pelkän työkalun kääntäminen

Testit käännetään oletuksena, ja juuri ne vaativat verkkoyhteyden —
`make release` ei käännä niitä, mutta se määrittää projektin, ja
määritysvaihe noutaa Catch2:n. Riittää, että ne poistetaan käytöstä kerran:
asetus säilyy CMaken välimuistissa, ja seuraavat käännökset säilyttävät sen.

```bash
cmake --preset release -DSUBEDIT_BUILD_TESTS=OFF
make release
```

Se ei vapauta osallistujaa mistään: laatuportti
vaatii ne kyllä.

### Jos käännös epäonnistuu linkitysvaiheessa

Vyöry viestejä ”undefined reference to `std::cout`” kelvollisesta koodista
tarkoittaa yleensä, että järjestelmän vaihtoehto `c++` osoittaa ohjelmaan `gcc` eikä
ohjelmaan `g++`: C++ kääntyy, mutta standardikirjastoa ei linkitetä.

```bash
ls -l /etc/alternatives/c++                                       # check
sudo update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++ 100   # fix
CXX=g++ make build                                                # work around
```

CMake-määritys tunnistaa tämän tapauksen ja pysähtyy tähän viestiin sen sijaan, että
antaisi linkityksen epäonnistua.

### Jos paketin asennus valittaa kirjastosta `libicu`

```console
error: Failed dependencies:
        libicuuc.so.74()(64bit) is needed by subedit-<version>.x86_64
```

Syitä on kolme, ja ne erottaa annetusta komennosta ja tiedostonimestä:

- **`rpm -i`**: se ei ratkaise mitään. Yritä uudelleen komennolla
  `sudo dnf install ./subedit-<version>.fcNN.x86_64.rpm`;
- **`.rpm`, jonka nimessä ei ole merkintää `fcNN`**, tai joka on muusta Fedorasta kuin omasi:
  se vaatii sen koneen ICU:n, joka sen käänsi. Valitse se, jonka `fcNN`
  on komennon `cat /etc/fedora-release` tuloste;
- **omalle versiollesi ei ole pakettia `.rpm`**: Fedora rawhide tai jokin muu jakelu.
  Mikään tämän tietovaraston paketti ei asennu sinne;
  [käännä lähdekoodista](#kääntäminen-lähdekoodista).
