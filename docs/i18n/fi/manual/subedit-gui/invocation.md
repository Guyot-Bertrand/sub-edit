# Käynnistys

<!-- machine-translated -->
> Tämä sivu on konekäännös, eikä kukaan tämän kielen lukija ole vielä tarkistanut sitä.

```
subedit-gui [--version] [<file>]
```

| Argumentti | Pakollinen | Arvo | Oletus |
| :------- | :------- | :---- | :------ |
| `<file>` | ei | avattavan tekstitystiedoston polku | ei ole — ikkuna avautuu tyhjänä |
| `--version` | ei | — | — |

**Yksi ainoa tiedosto.** Seuraavat argumentit ohitetaan: ikkunassa on
kerrallaan vain yksi projekti.

## `--version`

Kirjoittaa vakiotulosteeseen saman lohkon, jonka komentorivin `--version` kirjoittaa, ja
palaa heti avaamatta ikkunaa. Paluuarvo `0`.

**`subedit <version>` yksinään ensimmäisellä rivillä**, sitten yksi rivi kutakin kirjastoa kohden,
jonka varassa ikkuna toimii, ajon aikana löydetyn version kera ja suluissa versio,
jota vasten se on käännetty; sitten asennetut käyttöliittymän kielet koodeina. Koneella,
jossa on Qt 6.4.2:

```console
$ subedit-gui --version
subedit <version>
  ICU      74.2 (built with 74.2)
  Enchant  2.3.3 (built with 2.3.3)
  Qt       6.4.2 (built with 6.4.2)
  libmpv   2.2 (built with 2.2)
  Languages: en, fr
```

Numeroita ei kopioida tähän: ne ovat koneen omia, eikä tässä käyttöoppaassa ole
tuotettua lohkoa, joka pitäisi ohjelman omat ajan tasalla. Kirjastot ovat
komentorivin kirjastot, ICU ja Enchant, sekä ne kaksi, jotka vain ikkuna linkittää — Qt ja
libmpv, soitin, jonka versioksi ilmoitetaan sen asiakas-API:n versio, se, jonka `mpv.pc`
ilmoittaa. Loput kuvataan
[komentorivin käyttöoppaassa](../subedit-cli/invocation.md#--version). Sanat
`built with` ja `Languages` seuraavat ikkunan kieltä; mikään muu ei.

`Ohje ▸ Tietoja subeditistä` näyttää samat kirjastot samasta luettelosta taulukkona —
nimi, ladattu versio, versio, jota vasten on käännetty, kukin omalla rivillään — ja sen alla
kielet; sen otsikot seuraavat ikkunan kieltä.

## Tiedoston avaaminen

```console
$ subedit-gui film.srt
```

Luettavia muotoja ovat **ne yhdeksän**, jotka avaamisen suodatin luettelee — katso
[Avaaminen ja tallentaminen](files.md#avaaminen). Muoto tunnistetaan sisällöstä eikä
tiedostopäätteestä, ja **niin myös merkistökoodaus**: BOM-merkistä,
jos tiedostossa on sellainen, muuten koodauksesta, johon sen tavut viittaavat.

## Kun avaaminen epäonnistuu

Puuttuva tiedosto, lukukelvoton tiedosto, tiedosto, jonka tavut eivät pura millään
merkistökoodauksella, tai tiedosto, jonka sisältöä mikään muoto ei tunnista: **ikkuna avautuu
silti, tyhjänä**, ja epäonnistuminen kirjoitetaan virhetulosteeseen.

**Viesti nimeää syyn** samoin sanoin kuin komentorivi:

| Viesti | Mitä tapahtui |
| :------ | :------------ |
| `does not exist` | polku ei osoita mihinkään tiedostoon |
| `cannot be opened: permission denied` | järjestelmä kieltäytyy avaamasta sitä |
| `cannot be read` | järjestelmä kieltäytyi jostain muusta syystä |
| `cannot be decoded in the chosen encoding` | tavuja ei voi lukea valitulla merkistökoodauksella |
| `is in no format this tool knows` | mikään muoto ei tunnista sisältöä |
| `holds nothing recognisable as a subtitle` | muoto tunnistetaan, mutta mikään siinä ei ole kappale |

```console
$ subedit-gui notes.txt
subedit-gui: notes.txt: is in no format this tool knows
```

Tyhjänä avautuminen käynnistyksestä kieltäytymisen sijaan: työkalusta on silti
hyötyä, eikä argumentin epäonnistuminen ole syy olla käynnistymättä.

**Huonosti muodostettu mutta luettava tiedosto avautuu**, ja se, mitä lukeminen
huomasi matkan varrella, voidaan lukea tilarivin painikkeesta — katso
[lukemisen huomautukset](files.md#lukemisen-huomautukset).

## Paluuarvot

| Koodi | Milloin |
| :--- | :--- |
| `0` | ikkuna avautui ja suljettiin sitten normaalisti, tai `--version` vastasi |
| `2` | odottamaton virhe ennen avaamista — Qt puuttuu, näyttö ei ole käytettävissä |

Tiedoston avaamisen epäonnistuminen **ei muuta paluuarvoa**:
ikkuna avautui, ja sen paluuarvo kertoo.

## Man-sivu

Asennus sijoittaa sivun `subedit-gui(1)` hakemistoon `share/man/man1`:

```console
$ man subedit-gui
```

Se kertoo kutsumuodon, valitsimen `--version`, avaamisen epäonnistumisen viestit,
paluuarvot, kaksi ympäristömuuttujaa ja asetustiedoston.
**Se ei kuvaa ikkunaa** — sen tekee tämä käyttöopas, ja sivu
viittaa tänne.

## Qt-alusta

`QT_QPA_PLATFORM` valitsee, millä alustalla ikkuna toimii. **Jos muuttuja on
asetettu, `subedit-gui` noudattaa sitä kyselemättä.**

Jos sitä ei ole asetettu, ohjelma antaa Qt:n valita: sisäänrakennettu soitin piirtää kuvan
itse ikkunaan eikä vaadi alustalta mitään — katso
[Soitin](player.md).

`QT_QPA_PLATFORM=offscreen` ajaa ikkunaa siellä, missä ei ole
graafista palvelinta — niin jatkuva integraatio ja testit tekevät.

## Kieli

Ikkuna puhuu ympäristönsä kieltä, **joka valitaan kerran, käynnistyksen yhteydessä**: käynnissä olevan
ikkunan muuttujien muuttaminen ei muuta mitään ennen seuraavaa käynnistystä. Muuttujat ja säännöt
ovat komentorivin — `LANGUAGE`, `LC_ALL`, `LC_MESSAGES`, `LANG`, `LC_ALL=C`
englantia varten, englanti **sanaakaan sanomatta**, kun pyydettyä kieltä ei ole asennettu — ja ne kuvataan
[komentorivin käyttöoppaassa](../subedit-cli/invocation.md#kieli). Käännösluettelot ovat samat
tiedostot, jotka asennetaan hakemistoon `<prefix>/share/subedit/locale`.

**Kohdassa `Muokkaa ▸ Asetukset…` valittu kieli on etusijalla**, ja ympäristö luetaan vain,
kun valintana on `System default`, kuten on siihen asti, kunnes kieli valitaan: siellä valittu asennettu kieli
voittaa jokaisen muuttujan, myös muuttujan `LC_ALL=C`. Myös valinta tulee voimaan seuraavalla käynnistyskerralla. Katso
[Käyttöliittymän kieli](preferences.md#käyttöliittymän-kieli).

| Mikä | Seuraa kieltä |
| :--- | :------------------- |
| valikot ja niiden kohdat alleviivattuine kirjaimineen; työkalurivi; työkaluvihjeet | kyllä |
| taulukon ja kohdan `Näytä ▸ Sarakkeet` sarakkeet, välilehdet, tilarivi, toistopalkki | kyllä |
| Qt:n itse piirtämät vakiopainikkeet (`OK`, `Cancel`) | kyllä, kun Qt:n oma kyseisen kielen käännösluettelo on asennettu (`qtbase_<language>.qm`); muuten englanniksi |
| valintaikkunat, kohdan `Correct Texts…` sivut, tiedostovalintaikkunoiden otsikot ja suodattimet, viesti-ikkunat ja niiden sisältö | kyllä |
| rivit, jotka ikkuna kirjoittaa virhetulosteeseensa, `subedit-gui: …` | kyllä, ohjelman nimeä lukuun ottamatta |
| korjauslausekkeiden nimet kohdassa `Correct Texts…` ja niiden kuvaukset työkaluvihjeissä | kyllä, ohjelman mukana toimitetuille lausekkeille; oma lauseke näytetään niin kuin sen tiedosto sen kirjoittaa |
| merkistökoodausten, rivinvaihtojen ja oikeinkirjoituskielten nimet (`UTF-8`, `CRLF`, `en_US`) | ei koskaan: ne ovat nimiä, eivät sanoja |
| pikanäppäimet | ei koskaan: `Ctrl+S` on `Ctrl+S` kaikilla kielillä |
| viesti, jota käännösluettelo ei käännä | näytetään englanniksi |

Tämä versio asentaa kielet `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT` ja `nl` — katalaanin, tšekin, saksan, espanjan,
suomen, ranskan, galician, heprean, unkarin, interlinguen, islannin, italian ja hollannin — kokonaan käännettyinä. Ranskan on tarkistanut kielen lukija;
katalaani, tšekki, saksa, espanja, suomi, galicia, heprea, unkari, interlingue, islanti, italia ja hollanti ovat konekäännöksiä, eikä kukaan kielen lukija ole
vielä tarkistanut niitä.
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
Ranskankielisellä työpöydällä valikkorivillä lukee
`&Fichier`, `É&dition`, `&Affichage`, `&Vidéo`, `&Outils`, `&Projets`, `Aid&e`. Tämän käyttöoppaan
kuvakaappaukset on otettu suomenkielisestä käyttöliittymästä.
