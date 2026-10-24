# Liitetty video

<!-- machine-translated -->
> Tämä sivu on konekäännös, eikä kukaan tämän kielen lukija ole vielä tarkistanut sitä.

Valikossa **Video** on ensin kolme komentoa ja sitten toiminnot, joilla elokuvaa ohjataan
— katso [Soitin](player.md#elokuvassa-liikkuminen).

| Komento | Pikanäppäin | Mitä se tekee |
| :------ | :------- | :----------- |
| Valitse video… | ei ole | valitsee videotiedoston, johon asiakirja liittyy |
| Litteroi ääni… | ei ole | litteroi elokuvan äänen tekstityksiksi, jotka korvaavat projektin tekstitykset — katso [Äänen litterointi](#äänen-litterointi) |
| Toista / tauko | `Ctrl+P` | toistaa tai pysäyttää videon — katso [Soitin](player.md#toistaminen-ja-pysäyttäminen) |

Liitetty video on **avoimen asiakirjan ominaisuus** eikä sovelluksen
asetus: toisen tekstitystiedoston avaaminen aloittaa alusta. Sitä
ei kirjoiteta mihinkään tiedostoon, ja se katoaa ikkunan mukana.

## Ääniraita

Elokuvassa voi olla useita ääniraitoja — alkuperäinen versio, jälkiäänitys, kommenttiraita.
Alivalikko **`Video ▸ Ääni ▸ Kieli`** luettelee ne, **yksi kohta kutakin raitaa kohden**, ja **merkitsee soivan raidan**:

| Mitä tiedosto antaa | Mitä kohdassa lukee |
| :------------------ | :------------------ |
| kieli ja nimi | `1: fra — Original` |
| pelkkä kieli | `2: eng` |
| pelkkä nimi | `3: Commentary` |
| ei kumpaakaan | `1` |

Numero on **raidan paikka elokuvassa**, yksi kerrallaan, tiedoston järjestyksessä: kahdessa raidassa
voi lukea `eng`, ja tiedostossa, joka ei kerro raitojensa nimiä, on silti ensimmäinen ja toinen raita.

**Kohdan valitseminen toistaa kyseisen raidan.** Merkintä on soittimen vastaus eikä napsautus: se kertoo,
mikä raita soi.

- **Alivalikko ei ole käytettävissä**, kun videota tai ääniraitaa ei ole. Kun raitoja on vain yksi, alivalikko on käytettävissä ja
  näyttää sen valittuna: se kertoo elokuvan kielen. Sen vieressä olevat kaksi äänenvoimakkuuden toimintoa ovat
  aina käytettävissä: katso [Äänenvoimakkuus](player.md#äänenvoimakkuus).
- **Raitaa ei muisteta** videosta toiseen eikä istunnosta toiseen: raidan numero kuuluu
  omalle tiedostolleen, ja sen siirtäminen seuraavaan elokuvaan valitsisi sattumanvaraisen raidan, jolla on siellä sama numero.
  Välilehdelle palaaminen avaa sen elokuvan uudelleen raidalla, jonka soitin valitsee itse.
- Ei pikanäppäintä: kieltä vaihdetaan harvoin, ja valikko riittää.

## Videon valitseminen

Valintaikkuna avautuu tekstitystiedoston hakemistoon ja suodattaa
tunnetut videotiedostopäätteet:

    .avi .flv .m2ts .mkv .mov .mp4 .ogg .ogv .vob .webm

Se tarjoaa myös mahdollisuuden näyttää kaikki tiedostot. Valintaikkunan peruminen ei muuta
mitään.

Valittu tiedosto **avataan heti**: kuva ilmestyy ikkunaan
pysäytettynä ensimmäiseen hetkeensä. Katso [Soitin](player.md).

**Videotiedoston pudottaminen ikkunaan tekee saman** — katso
[Tiedostojen pudottaminen ikkunaan](files.md#tiedostojen-pudottaminen-ikkunaan).

**Video kuuluu nykyisen välilehden projektille.** Jokainen välilehti säilyttää
omansa; välilehden vaihtaminen vaihtaa elokuvan. **Välilehdelle palaaminen jatkaa sen
elokuvaa siitä, mihin jäit**, pysäytettynä, kuten mikä tahansa avaaminen. Toinen elokuva,
joka on sillä välin valittu kyseiselle projektille, alkaa puolestaan alusta.

## Automaattinen ehdotus

Kun tekstitystiedosto avataan, **jos videota ei ole valittu**,
ikkuna etsii sellaista samasta hakemistosta: videotiedostoa, jonka nimi
ilman päätettä on tekstitystiedoston nimen alkuosa pisteeseen asti.

| Tekstitystiedosto | Löydetty video | Miksi |
| :------------ | :---------- | :-- |
| `film.fr.srt` | `film.mkv` | `film` on nimen `film.fr` alkuosa |
| `film.en.forced.srt` | `film.mkv` | samoin, osien määrästä riippumatta |
| `film.fr.srt` | `film.fr.mkv` eikä `film.mkv` | pisin nimi voittaa |
| `film.fr.srt` | ei mitään, jos sekä `film.mkv` **että** `film.mp4` ovat paikalla | kahta ehdokasta ei voi erottaa toisistaan |
| `film.fr.srt` | ei mitään, jos paikalla on vain `fil.mkv` | `fil` ei pääty pisteeseen |

**Ehdotus ei ole valinta.** Sen voi korvata toinen
ehdotus — toisen tiedoston avaaminen, tallentaminen toisella nimellä. Nimenomaista
valintaa sen sijaan ei koskaan korvata: kun `Select Video…` on kerran käytetty,
automaattinen ehdotus ei enää palaa sen tilalle.

Tämä tekee arvauksesta hyväksyttävän: sen voi korjata, ja
korjaus pysyy.

## Mitä tilarivi näyttää

Tilarivin oikeassa reunassa oleva rivi kertoo, mihin asiakirja liittyy. Se
on viimeinen kolmesta: luetun tiedoston [merkistökoodaus](files.md#merkistökoodaus-tilarivillä)
tulee ensin, kohdista päätelty [ruuturuudukko](frame-grid.md)
sen jälkeen.

| Tilanne | Mitä kirjoitetaan |
| :-------- | :-------------- |
| video on liitetty | `Video: film.mkv` |
| … ja `ffprobe` antaa sen ruutunopeuden | `Video: film.mkv, 24000/1001 fps` |
| ei videota | `No video` |

**Ruutunopeus kulkee elokuvan mukana eikä sen vieressä.** Ne ovat yksi ja sama tieto — mihin
tämä asiakirja liittyy —, ja kolmas maininta asettaisi sen samalle tasolle kuin
[kohdista päätellyn ruudukon](frame-grid.md), joka on eri tieto.

## Äänen litterointi

**`Transcribe Audio…`** kirjoittaa elokuvan puheen tekstitykseksi. Komento on pois käytöstä, kunnes elokuva on avattu: se koskee
**projektin elokuvan** ääntä eikä koskaan tiedostoa, jonka valitsisit — litteroidaksesi
äänitiedoston käytä komentoriviltä komentoa [`transcribe`](../subedit-cli/transcribe.md).

Valintaikkuna kysyy kolme asiaa ja kertoo, voiko se aloittaa:

- **ääniraidan**, luetteloituna samoin kuin valikko [`Audio`](#ääniraita) ne luettelee, kielineen
  ja nimineen sellaisina kuin tiedosto ne ilmoittaa;
- **kielen**, kaksi kirjainta kuten `en` tai `fr` (`fra` luetaan muodossa `fr`). Se alkaa valitun raidan kielellä,
  kun tiedosto ilmoittaa sellaisen, seuraa raitaa niin kauan kuin et ole kirjoittanut omaa, ja
  **kentän alla oleva rivi kertoo, mistä se tulee**. Kun tiedosto ei ilmoita raidalle kieltä,
  rivi kertoo sen ja varoittaa: malli tunnistaa kielen silloin äänen ensimmäisten kolmenkymmenen sekunnin perusteella, minkä vain monikielinen malli
  osaa, ja **elokuva, joka alkaa musiikilla tai hiljaisuudella, voidaan ottaa toiseksi kieleksi ja sen puhe käännetään
  tälle kielelle**. Kirjoita kieli, jos tiedät sen. Pelkälle englannille tarkoitetun mallin kohdalla kenttä on
  lukittu arvoon `en` eikä sitä voi muuttaa, ja rivi kertoo, että elokuva tulee ulos englanniksi riippumatta siitä, mitä kieltä se puhuu;
- **elokuvan osan**, ajasta toiseen — katso [Kokeile ensin osaa](#kokeile-ensin-osaa). Molemmat kentät
  tyhjinä tarkoittaa koko elokuvaa.

**Se kertoo myös, mitä puuttuu.** Kolme riviä kertoo, ovatko moottori, `ffmpeg` ja malli paikalla, ja
missä; kun jokin ei ole, `OK` pysyy harmaana, ja valintaikkuna kertoo sen viittauksella sivuun
[Litterointityökalujen asentaminen](../subedit-cli/install-transcription.md). Painike `Preferences…` avaa
[asetukset](preferences.md#litterointi), joissa ohjelma ja malli asetetaan — ne ovat tietoja
koneesta, eivät valintoja, jotka pitäisi tehdä uudelleen jokaisella litteroinnilla — ja valintaikkuna katsoo uudelleen, kun ne suljetaan.

Kieli muistetaan seuraavaa kertaa varten. Moottori itse ja tekstitysten rajat ovat
[asetuksia](preferences.md) (`transcription.*`).

**Kun projektissa on jo tekstityksiä, ikkuna kysyy ennen niiden korvaamista** ja kertoo,
montako poistuu. Se kysyy ennen työtä, jotta kieltäytyminen ei maksa mitään. Kun projekti on tyhjä,
ei ole mitään kysyttävää.

Työ tehdään **taustalla**, ja ruudussa on painike `Cancel`; ikkuna pysyy käytettävänä sen ympärillä. **Ruutu
kertoo, missä työ on**, jotta tiedät sen elävän ja kuinka kauan se kestää:

- **vaihe**: ensin “Puretaan ääntä…” — ruutu on silloin varattu, ilman lukua, sillä purku ei kerro, kuinka pitkällä se
  on —, sitten “Litteroidaan ääntä…”, palkin kanssa;
- **jäljellä oleva aika**, “noin 38 min jäljellä”: aluksi puuttuu, näytetään, kun moottori on raportoinut riittävästi edistymistä ekstrapoloitavaksi, ja laskettu sen nopeudesta alusta viimeiseen minuuttiin asti; se on muutaman minuutin tarkkuudella, ei lupaus;
- **kulunut aika**, “kulunut: 0:05:12”, joka kulkee silloinkin, kun mikään muu ei liiku;
- **kohta**, “0:12:30 / 0:50:00”: kuinka pitkälle elokuvaa moottori on päässyt, heti kun se on tuottanut ensimmäisen tekstinsä, näytetään **heti sen viimeisimmän tekstin yläpuolella**, jota se vastaa;
- **viimeisin saatu teksti**, jotta näet työn olevan järkevää.

Pitkällä elokuvalla, jossa on suuri malli prosessorilla, ensimmäiset minuutit voivat olla pitkiä: moottori lataa mallia ja työstää ensimmäisiä
kolmenkymmenen sekunnin ikkunoitaan. Kohta ja viimeisin teksti liikkuvat ennen moottorin omaa prosenttia, joka tulee vain 5 %:n
askelin elokuvasta. Kun työ päättyy, projektin tekstitykset korvataan
litteroinnilla **yhtenä toimintona**: yksi `Undo` palauttaa sen, mitä siellä oli, käännöstekstit
mukaan lukien. Litterointi, joka epäonnistuu tai jonka perut, ei muuta mitään — syy kerrotaan
ruudussa, paitsi peruutuksen kohdalla, joka vain kertoo tilarivillä, että se peruttiin.

**Kun suurin osa siitä, mitä moottori kirjoitti, on hakasulkeissa olevia merkintöjä** — `[MUSIC PLAYING]`, `[SPEAKING FRENCH]` —, ruutu
kertoo sen työn jälkeen: käytetty kieli ei todennäköisesti ole puhuttu kieli, ja moottori on voinut kääntää puheen. Aja uudelleen
kirjoittamalla kieli. Tekstitykset ovat silti projektissa, ja yksi `Undo` palauttaa sen, mikä siinä oli.

### Kokeile ensin osaa

Alueen avulla voit **arvioida mallia tai kieltä kahdella minuutilla ennen kuin sitoudut kahteen tuntiin**. Kentät “Elokuvan osa” ottavat alun ja lopun, kumpikin kirjoitettuna `2:00`, `1:02:03`, `2:00.5` tai sekuntimääränä,
`90`; yksi yksinään on sallittu (sieltä loppuun tai alusta sinne). `OK` pysyy harmaana, kunnes kenttä
on aika ja loppu on alun jälkeen, ja kenttien alla oleva rivi kertoo, mikä on vialla.

- **Tekstitykset säilyttävät elokuvan ajan**: se, mikä sanotaan kohdassa 2:10, on tuloksessa kohdassa `0:02:10`.
- **Se korvaa projektin tekstitykset kuten mikä tahansa litterointi**, saman kysymyksen jälkeen, kun tekstityksiä on, ja yksi
  `Undo` palauttaa ne. Jos haluat kokeilla aluetta koskematta työhösi, tallenna ensin tai työskentele kopiolla.

**Sitä, mitä se tarvitsee, ei toimiteta**: moottori (whisper.cpp), malli ja `ffmpeg` äänen purkamiseen.
Katso [Litterointityökalujen asentaminen](../subedit-cli/install-transcription.md).

## Kun elokuvaa ei ole

Ikkuna näyttää kuvan paikalla painikkeen **`Select Video…`** sen sijaan, että
näyttäisi ei mitään. Se on sama komento kuin valikon `Video` komento, hiiren
ulottuvilla: puutetta, jolle ei voi tehdä mitään, on vaikea erottaa
viasta.

Painike katoaa heti, kun elokuva on avattu, ja kuva vie **koko** sen
tilan: korkeuden, jonka taulukko jättää sen yläpuolelle — sen, jonka
[`window.table-share`](preferences.md) muistaa istunnosta toiseen —, eikä
koskaan vähempää kuin kuvan vähimmäiskorkeus, jonka taulukko tarvittaessa luovuttaa.
Kun elokuva poistuu, painike saa takaisin saman tilan.

**Tiedoston nimi, ei koskaan sen polkua.** Kahdensadan merkin polku
työntäisi kaiken muun pois riviltä; koko polku on se, jonka
valintaikkuna näytti.

## `ffmpeg` ei ole pakollinen

`subedit-gui` kysyy ohjelmalta `ffprobe` — joka toimitetaan paketin `ffmpeg` mukana — yhtä ja vain yhtä
asiaa: **säiliön ilmoittamaa ruutunopeutta**. Se luetaan kerran
kutakin elokuvaa kohden, kun elokuva liitetään.

| Ohjelman `ffprobe` kanssa | Ilman |
| :------------- | :------ |
| tilarivi kirjoittaa ruutunopeuden elokuvan nimen viereen | se kirjoittaa vain nimen |
| muunnosikkuna ehdottaa elokuvan ruutunopeutta alemmassa kentässä | se ei ehdota mitään ja avautuu kuten ennenkin |
| kohdistusikkuna avautuu siihen | se avautuu projektin ruutunopeuteen |

**Mikään muu ei riipu siitä, eikä mikään toiminto kieltäydy.** Soitin puolestaan
ei tarvitse ohjelmaa `ffprobe` lainkaan: se käyttää kirjastoa `libmpv`, joka on
käännösaikainen riippuvuus. Elokuvan kesto, jota käytetään ilmoittamaan siitä, mikä
menee lopun yli, tulee soittimelta eikä ohjelmalta `ffprobe`.

**Litterointi on ainoa asia, joka tarvitsee `ffmpeg`in**: se purkaa elokuvan äänen. Katso
[Litterointityökalujen asentaminen](../subedit-cli/install-transcription.md).
Kyse ei siis ole heikennetystä tilasta vaan **normaalista toiminnasta** koneella,
jolle `ffmpeg` ei ole koskaan asennettu: tieto tulee käyttäjältä, ja `ffprobe`
ehdottaa sitä, kun se on saatavilla.

Luettu ruutunopeus on **tarkka rationaaliluku** — `24000/1001`, ei `23.976`.
Siksi `ffprobe` säilytetään tätä vastausta varten:
soitin tietää saman asian liukulukuna, ja likimääräinen ruutunopeus
ajoittaisi koko tiedoston uudelleen hieman pieleen.
