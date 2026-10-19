# Kumoa ja tee uudelleen

<!-- machine-translated -->
> Tämä sivu on konekäännös, eikä kukaan tämän kielen lukija ole vielä tarkistanut sitä.

Jokainen toiminto tallentuu historiaan, ja sen voi kumota. Muokattu solu lasketaan
**yhdeksi** merkinnäksi: kirjoittamista ei tallenneta näppäinpainallus kerrallaan, vaan vain
vahvistus lasketaan.

| Toiminto | Pikanäppäin | Missä |
| :----- | :------- | :---- |
| Kumoa | `Ctrl+Z` | valikko **Muokkaa**, työkalurivi |
| Tee uudelleen | `Ctrl+Shift+Z` | valikko **Muokkaa**, työkalurivi |

## Toiminto nimeää sen, minkä se kumoaa

Valikossa ei lue ”Kumoa” vaan **”Kumoa: tekstin muokkaaminen”**, sen
toiminnon mukaan, joka kumotaan. Työkalurivin painike säilyttää pelkän sanan — muuten sen
leveys seuraisi viimeistä toimintoa, ja painike liikkuisi
osoittimen alla —, mutta sen työkaluvihjeessä on koko nimi.

| Toiminto | Mitä toiminnossa lukee |
| :-------- | :-------------------- |
| muokattu tekstisolu | `editing a text` |
| alku, loppu | `editing a start`, `editing an end` |
| kesto | `editing a duration` |
| rivien lisääminen | `inserting` |
| rivien poistaminen | `removing` |
| siirto, myös `Shift Whole File onto Grid` | `shifting` |
| muunnos | `transforming` |
| ruutunopeuden muunto | `converting the frame rate` |
| kohdistus ruutunopeuteen | `aligning on the frame rate` |
| mainintojen poistaminen | `removing hearing-impaired mentions` |
| kursivointi | `putting in italics` |
| kursiivin poistaminen | `taking italics out` |
| kirjainkoon muuttaminen | `changing the case` |
| viivojen lisääminen, poistaminen | `adding dialogue dashes`, `removing dialogue dashes` |
| rivien yhdistäminen | `merging` |
| rivin jakaminen | `splitting` |
| tekstien leikkaaminen, liittäminen | `cutting texts`, `pasting texts` |
| kestojen säätäminen | `adjusting durations` |
| käännöksen avaaminen | `opening a translation` |
| tiedoston lisääminen perään | `appending a file` |
| projektin jakaminen | `splitting the project` |
| komennossa `Correct Texts…` hyväksytty tai komennon `Check Spelling…` tekemä korjaus | `correcting texts` |
| korvaus, kaikkien korvaaminen | `replacing`, `replacing all` |

**Kaksikymmentäseitsemän nimeä, ja siinä on kaikki, mitä ikkuna voi tuottaa.** Ydin nimeää
kahdennenkymmenennenkahdeksannen — `sorting` —, johon mikään ikkunan toiminto ei yllä: järjestäminen
tapahtuu vain tiukan järjestyskäytännön alaisena, kun taas ikkuna avaa asiakirjansa
sallivan käytännön mukaan, joka ilmoittaa epäjärjestyksestä korjaamatta sitä.

**`Shift Whole File onto Grid` ei erotu siirrosta**, ja se on
täsmällistä: se on siirto, jonka määrä on mitattu eikä kirjoitettu.

Nämä ovat samat sanat kuin komentorivillä: ne kirjoitetaan kerran,
ytimessä.

Kun kumottavaa ei ole, toiminnossa lukee pelkkä ”Kumoa”, **eikä se ole
käytettävissä**: sen pikanäppäin ei tee mitään.

## Muutosmerkki

Ikkunan otsikossa on merkki niin kauan kuin tiedosto eroaa levyllä
olevasta — tähti tai mitä ikinä alusta käyttää sen sijaan. **Samoin välilehden
nimessä**, `premier.srt*`: otsikko kertoo vain projektista, jota katsot,
ja välilehden nimi kertoo niistä, joita et katso.

**Jokaisella välilehdellä on oma historiansa.** Kumoaminen kumoaa vain sen, minkä nykyisen
välilehden projekti sai — myös kaikissa projekteissa tehty `Replace All`: se
jättää yhden merkinnän jokaiseen projektiin, johon se koski.

**Merkki katoaa, jos kumoat lähtöpisteeseen asti.** Tämä ei ole näytön
yksityiskohta: muutosten määrä lasketaan eikä sitä tallenneta kyllä/ei-tietona, joten
takaisin palaaminen voi viedä sen nollaan, ja merkki katoaa.

## Kaksi asiaa, jotka on hyvä tietää

**Viitekohta on viimeisin tallennus**, tai tiedoston avaaminen, jos
tallennusta ei ole tehty. Tallentaminen siirtää sitä: merkki katoaa, ja jälkeenpäin kumoaminen
tuo sen takaisin.

Kumoamisen jälkeen tehty toiminto **tyhjentää sen, mitä oli tehtävissä uudelleen**: uudelleen tekeminen
toistaisi komennon, jonka lähtötilaa ei enää ole. Näin toimii
jokainen muokkausohjelma, ja se on tarkoituksellista.

**Kun solu on auki**, `Undo` ja `Redo` — valikosta tai työkaluriviltä —
vahvistavat ensin syötteen, kuten muutkin sivun
[Solun muokkaaminen](editing.md) toiminnot. Juuri kirjoitettu tulee silloin historian viimeiseksi
merkinnäksi: `Undo` kumoaa sen eikä sitä edeltävää toimintoa; `Redo` ei voi tehdä
mitään uudelleen, koska syöte tyhjentää sen, mitä oli tehtävissä uudelleen. Auki oleva
solu, johon ei kirjoitettu mitään, ei jätä merkintää: `Undo` kumoaa silloin edellisen
toiminnon.

Historia säilyttää tuhat merkintää ja unohtaa sitä vanhemmat.
