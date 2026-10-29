# subedit — käyttöopas

<!-- machine-translated -->
> Tämä sivu on konekäännös, eikä kukaan tämän kielen lukija ole vielä tarkistanut sitä.

subedit lukee ja tutkii tekstitystiedostoja ja ajoittaa niitä uudelleen. Se koostuu kahdesta
ohjelmasta, joilla on yhteinen ydin ja siksi samat säännöt ja samat sanat.

| Ohjelma | Mihin | Sen käyttöopas |
| :-------- | :-------------- | :--------- |
| `subedit-gui` | tiedoston avaaminen, sen tarkasteleminen taulukossa, muokkaaminen ja katsominen elokuvan rinnalla | [ikkunan käyttöopas](subedit-gui/index.md) |
| `subedit-cli` | tiedostojen käsittely erissä, skriptistä, ilman käyttöliittymää | [komentorivin käyttöopas](subedit-cli/index.md) |

Molemmat lukevat ja kirjoittavat **yhdeksää muotoa** — SubRip, WebVTT, SubViewer 2,
Sub Station Alpha, Advanced SSA, MicroDVD, MPL2, TMPlayer, LRC — ja
tunnistavat muodon sisällöstä eikä tiedostopäätteestä. **Myös merkistökoodaus
tunnistetaan sisällöstä**: Latin-1-, CP1252- tai UTF-16-koodattu tiedosto
avautuu, ohjelma kertoo, minä se luettiin, ja se kirjoitetaan takaisin omassa koodauksessaan — tiedosto,
joka avataan ja tallennetaan muuttamatta, ei muutu tavuakaan.

## Mistä aloittaa

| Mitä haluat | Minne mennä |
| :----------------- | :------- |
| asentaa työkalun | [Asennus](subedit-cli/installation.md) — se koskee molempia ohjelmia |
| avata tiedoston ja katsoa, mitä se sisältää | [Taulukko](subedit-gui/table.md) |
| ajoittaa uudelleen kappaleet, jotka ovat siirtyneet vakiomäärän | [Toiminnot](subedit-gui/operations.md#shift-positions) tai [`shift`](subedit-cli/shift.md) |
| ajoittaa uudelleen toiselle ruutunopeudelle kirjoitetun tiedoston | [`framerate`](subedit-cli/framerate.md), ja [`snap`](subedit-cli/snap.md), jos ajoitus on jo oikea |
| tietää, mille ruutunopeudelle tiedosto on kirjoitettu | [Ruuturuudukko](subedit-gui/frame-grid.md) tai [`inspect`](subedit-cli/inspect.md) |
| poistaa kuulovammaisille tarkoitetut maininnat | [Toiminnot](subedit-gui/operations.md#remove-hearing-impaired-mentions) tai [`hearing-impaired`](subedit-cli/hearing-impaired.md) |
| korjata maininnat, yleiset virheet, kirjainkoon ja rivinvaihdot yhdellä kertaa | [`Correct Texts…`](subedit-gui/correct-texts.md) tai [`correct`](subedit-cli/correct.md) |
| tarkistaa oikeinkirjoituksen sana sanalta järjestelmän sanakirjoilla | [`Check Spelling…`](subedit-gui/check-spelling.md) |
| pidentää kappaleita, jotka ovat liian lyhyitä luettaviksi | [`Adjust Durations…`](subedit-gui/operations.md#adjust-durations) tai [`adjust`](subedit-cli/adjust.md) |
| palauttaa järjestykseen tiedoston, jonka kappaleet eivät ole järjestyksessä | [`sort`](subedit-cli/sort.md) |
| tehdä yksi kappale saman tekstin peräkkäisistä palasista | [`Join Repeated Subtitles`](subedit-gui/lines.md#toistuvien-kappaleiden-yhdistäminen) tai [`join-repeats`](subedit-cli/join-repeats.md) |
| oikaista yhdellä kertaa tiedosto, joka on huonosti pilkottu, epäjärjestyksessä ja päällekkäinen | [`repair`](subedit-cli/repair.md) |
| poistaa kappaleiden väliset päällekkäisyydet | [`Make Ranges Disjoint…`](subedit-gui/lines.md#aikavälien-päällekkäisyyksien-poistaminen) tai [`disjoint`](subedit-cli/disjoint.md) |
| liittää elokuvan kaksi osaa peräkkäin | [`Append File…`](subedit-gui/operations.md#append-file) tai [`append`](subedit-cli/append.md) |
| leikata elokuvan kahteen osaan | [`Split Project…`](subedit-gui/operations.md#split-project) tai [`split-file`](subedit-cli/split-file.md) |
| muuttaa koko tiedoston kirjainkokoa, kursiivia tai vuoropuheluviivoja | [Toiminnot](subedit-gui/operations.md#italic) tai [`case`](subedit-cli/case.md), [`italics`](subedit-cli/italics.md), [`dialogue-dashes`](subedit-cli/dialogue-dashes.md) |
| etsiä sanaa ja korvata sen rikkomatta tageja | [Etsi ja korvaa](subedit-gui/find-and-replace.md) tai [`replace`](subedit-cli/replace.md) |
| kirjoittaa elokuvan puhe tekstitykseksi | [`Transcribe Audio…`](subedit-gui/video.md#äänen-litterointi) tai [`transcribe`](subedit-cli/transcribe.md) — [työkalujen asentamisen](subedit-cli/install-transcription.md) jälkeen |
| ajoittaa tekstitystiedoston uudelleen elokuvan äänen mukaan | [`sync`](subedit-cli/sync.md) — [työkalujen asentamisen](subedit-cli/install-transcription.md) jälkeen |
| kääntää alkuperäisen tekstin rinnalla | [Käännöksen avaaminen](subedit-gui/files.md#käännöksen-avaaminen) tai [`pair`](subedit-cli/pair.md), jolla käännös ajoitetaan uudelleen päätiedoston mukaan |
| käsitellä sata tiedostoa kerralla | [Hakemistopuun käsittely](subedit-cli/lots.md), sitten [Käynnistys](subedit-cli/invocation.md) |

**Alikomentojen `framerate` ja `snap` välillä virhe jää huomaamatta.** Jos
olet epävarma, sivu [`snap`](subedit-cli/snap.md#snap-ei-ole-framerate-eikä-niiden-sekoittaminen-näy)
alkaa näiden kahden vertailulla.

## Mitä tämä käyttöopas lupaa

**Se kuvaa sitä, mikä on olemassa, ei koskaan sitä, mitä suunnitellaan.** Se, mitä on tulossa ja
missä järjestyksessä, on [tiekartassa](../../../feuille-de-route.md).

**Tämä käyttöopas on kirjoitettu englanniksi ja käännetty.** Sen käännökset seuraavat sitä
sivu sivulta, tietovaraston hakemistossa `docs/i18n/`, yksi puu kutakin kieltä kohden. Ohjelmat
puhuvat ympäristönsä kieltä; niiden näyttämät nimet — alikomennot,
valitsimet, valikkojen nimikkeet — lainataan tässä sellaisina kuin ohjelma kirjoittaa ne englanniksi.

**Esimerkit ja kuvat tuotetaan automaattisesti, teksti ei.** Lohkot
`console` syntyvät ajamalla komento todella; kuvakaappaukset
rakentamalla todellinen ikkuna ja valokuvaamalla se. Kumpikaan
ei siis voi kuvata työkalua, jota ei enää ole. Ilman turvaverkkoa jää
niitä ympäröivä teksti, jonka oikeellisuus perustuu oikolukuun.

**Kuvakaappaukset näyttävät molemmat paletit**, `Light` ja `Dark` — ne, jotka
[asetukset](subedit-gui/preferences.md#teema) tarjoavat.

## Molemmat käyttöliittymät sanovat samat sanat

Yksi syy lukea kumpaa tahansa käyttöopasta toista ohjelmaa käytettäessä:
virheilmoitukset, muotojen nimet, ruudukon tulokset ja kumoamisen
nimikkeet kirjoitetaan **kerran**, ytimessä, ja molemmat ohjelmat
lainaavat niitä. Hylätty tiedosto antaa saman lauseen molemmilla puolilla.

Eroa on siinä, mitä kukin käyttöliittymä osaa:

| | `subedit-gui` | `subedit-cli` |
| :--- | :------------ | :------------ |
| useita tiedostoja kerralla | kyllä — yksi välilehti projektia kohden | kyllä |
| käännös päätiedoston rinnalla | kyllä | kyllä — `-t` tekstialikomennoissa, ja [`pair`](subedit-cli/pair.md) sen ajoittamiseen uudelleen |
| tiedoston lisääminen perään, projektin jakaminen | kyllä | kyllä — [`append`](subedit-cli/append.md), [`split-file`](subedit-cli/split-file.md) |
| kumoaminen | kyllä, tuhat merkintää | ei sovellu — mitään ei muuteta paikallaan kysymättä |
| tekstin tai kohdan muokkaaminen käsin | kyllä | ei |
| rivien lisääminen, poistaminen, yhdistäminen ja jakaminen | kyllä | ei |
| etsiminen ja korvaaminen | kyllä | kyllä — [`replace`](subedit-cli/replace.md) |
| tekstien leikkaaminen, kopioiminen ja liittäminen | kyllä | ei |
| kestojen, kursiivin, kirjainkoon ja viivojen säätäminen | kyllä | kyllä — [`adjust`](subedit-cli/adjust.md), [`italics`](subedit-cli/italics.md), [`case`](subedit-cli/case.md), [`dialogue-dashes`](subedit-cli/dialogue-dashes.md) |
| tekstien korjaaminen lausekkeilla | kyllä — `Correct Texts…` | kyllä — [`correct`](subedit-cli/correct.md) |
| oikeinkirjoituksen tarkistaminen sana sanalta | kyllä — `Check Spelling…` | ei |
| elokuvan katsominen ajoitettaessa | kyllä | ei |
| syötteen päälle kirjoittaminen | `Save` | `--in-place` |
