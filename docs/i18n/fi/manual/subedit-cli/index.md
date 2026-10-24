# subedit-cli — käyttöopas

<!-- machine-translated -->
> Tämä sivu on konekäännös, eikä kukaan tämän kielen lukija ole vielä tarkistanut sitä.

subeditin komentorivityökalu: tekstitystiedostojen käsittely ilman graafista
käyttöliittymää, erissä tai skriptistä. Toisesta ohjelmasta ja siitä,
mistä aloittaa, kertoo
[käyttöopas](../index.md).

> **Nykytila.** Ohjelman `subedit-cli` kaikki seitsemäntoista alikomentoa ovat olemassa: tutkiminen,
> muuntaminen, siirtäminen, muuntaminen kahden pisteen avulla, uudelleenajoitus ruutunopeudelta toiselle, kohdistaminen ruuturuudukkoon,
> kuulovammaisille tarkoitettujen mainintojen poistaminen, kestojen säätäminen, tekstin korvaaminen, kirjainkoon,
> kursiivin ja vuoropuheluviivojen muuttaminen, kappaleiden palauttaminen järjestykseen,
> tekstien korjaaminen Gaupolin lausekkeilla, tiedostojen liittäminen peräkkäin,
> tiedoston leikkaaminen kahtia ja käännöksen kirjoittaminen päätiedostonsa kohtiin. Tämä käyttöopas
> kuvaa sitä, mikä on olemassa, ei koskaan sitä, mitä suunnitellaan; mitä on tulossa seuraavaksi, on
> [tiekartassa](../../../../feuille-de-route.md).

## Osiot

| Osio | Sisältö |
| :------ | :------ |
| [Asennus](installation.md) | työkalun kääntäminen ja asentaminen |
| [Käynnistys](invocation.md) | yleiset valitsimet, tulosteet, paluuarvot |
| [Asetustiedosto](invocation.md#asetustiedosto) | mitä alikomennot `adjust` ja `correct` lukevat ikkunan asetustiedostosta, `--config`, `--no-config` |
| [Hakemistopuun käsittely](lots.md) | useita tiedostoja, kokonainen hakemisto, `--recursive` |
| [`inspect`](inspect.md) | kertoo, mistä tiedosto koostuu |
| [`convert`](convert.md) | kirjoittaa tiedoston toiseen muotoon tai toiseen rakenteeseen |
| [`shift`](shift.md) | siirtää jokaista kohtaa saman verran |
| [`transform`](transform.md) | korjaa jokaisen kohdan kahden oikeaksi tiedetyn pisteen avulla |
| [`framerate`](framerate.md) | ajoittaa tiedoston uudelleen ruutunopeudelta toiselle |
| [`snap`](snap.md) | palauttaa aikaleimat ruutunopeuden ruutuihin |
| [`hearing-impaired`](hearing-impaired.md) | poistaa kuulovammaisille tarkoitetut maininnat |
| [`adjust`](adjust.md) | säätää kestot lukunopeuden ja rajojen mukaan |
| [`replace`](replace.md) | korvaa tekstin rikkomatta tagia |
| [`case`](case.md) | muuttaa tekstit muotoon Jokainen Sana Isolla, lauseen kirjainkokoon, isoiksi tai pieniksi kirjaimiksi |
| [`italics`](italics.md) | kursivoi tekstit tai poistaa niiden kursiivin |
| [`dialogue-dashes`](dialogue-dashes.md) | lisää tai poistaa vuoropuheluviivat |
| [`sort`](sort.md) | palauttaa kappaleet alkukohtiensa järjestykseen |
| [`join-repeats`](join-repeats.md) | tekee yhden kappaleen saman tekstin peräkkäisistä palasista |
| [`disjoint`](disjoint.md) | leikkaa päällekkäiset kappaleet niin, ettei mikään aikaväli enää ole toisen päällä |
| [`repair`](repair.md) | ketjuttaa alikomennot `join-repeats`, `sort` ja `disjoint` tiedostolle, joka on huonosti pilkottu, epäjärjestyksessä ja päällekkäinen |
| [`correct`](correct.md) | korjaa tekstit Gaupolin lausekkeilla: maininnat, sanojen yhdistäminen ja jakaminen, yleiset virheet, isot alkukirjaimet, rivinvaihto |
| [`append`](append.md) | liittää tiedostot ensimmäisen perään yhdeksi tiedostoksi |
| [`split-file`](split-file.md) | leikkaa tiedoston kahtia, ja loppuosa siirretään alkamaan nollasta |
| [`pair`](pair.md) | kirjoittaa käännöksen päätiedostonsa kohtiin ja kertoo, miten sen rivit asettuivat kohdakkain |
| [`transcribe`](transcribe.md) | kirjoittaa videon tai äänitiedoston puheen tekstitykseksi |
| [Litterointityökalujen asentaminen](install-transcription.md) | moottori, malli ja `ffmpeg`, joita ohjelma ei toimita |
| [`config`](config.md) | luettelee, näyttää ja muuttaa ikkunan kanssa yhteisiä toimintojen asetuksia |
