# Käynnistys

<!-- machine-translated -->
> Tämä sivu on konekäännös, eikä kukaan tämän kielen lukija ole vielä tarkistanut sitä.

```
subedit-cli [global options] <subcommand> [options] <file>...
```

Ilman alikomentoa työkalu kirjoittaa ohjeensa ja pysähtyy.

<!-- example: subedit-cli -->
```console
$ subedit-cli
Read, inspect and retime subtitle files.
Usage: subedit-cli [OPTIONS] [SUBCOMMAND]

Options:
  -h,--help                   Print this help message and exit
  --version                   Display program version information and exit
  -v                          Say more: -v is the default, -vv details, -vvv debugs
  -q,--quiet                  Say nothing but errors
  --encoding NAME             Encoding to read the files in; detected by default
  --format text|json          Form of the result on standard output: text or json
  --config FILE               Read the operation settings from FILE instead of the user's settings file
  --no-config Excludes: --config
                              Ignore any settings file: use the built-in defaults
  --list-encodings            Write the encodings that can be read and written, one a line, and stop

Subcommands:
  inspect                     Report what a subtitle file is made of
  convert                     Write a subtitle file out in another format or shape
  shift                       Move every position of a file by a fixed amount
  transform                   Correct every position from two points known to be right
  framerate                   Re-time a file mastered at one frame rate for another
  snap                        Move every position onto the nearest frame of a frame rate (see framerate)
  hearing-impaired            Remove the sounds described between square brackets
  adjust                      Bring the duration of every subtitle within a reading speed and limits
  replace                     Replace a text in the subtitles, without breaking a tag
  case                        Put the texts in title, sentence, upper or lower case, tags intact
  italics                     Put the texts in italics, or take their italics out
  dialogue-dashes             Put dialogue dashes on the lines, or take them off
  sort                        Put the subtitles in the order of their start, keeping ties as they are
  join-repeats                Join the subtitles that repeat each other, one after the other, into one
  disjoint                    Cut overlapping subtitles so that no range overlaps another, merging what overlaps
  repair                      Join the repeated subtitles, put them in order, then make the ranges disjoint
  correct                     Correct the texts with the patterns of the correction assistant
  append                      Put subtitle files one after another into a single file
  split-file                  Cut a subtitle file in two, a head and a tail
  pair                        Write a translation at the positions of its main file, and say how it lined up
  transcribe                  Write the speech of a video or an audio file as subtitles
  sync                        Re-time the subtitles of a file on the audio of a film
  config                      List, show and change the operation settings
```

## Kieli

Komentorivi puhuu ympäristönsä kieltä: **sen ohje, kerronta ja
virheet** noudattavat pyydettyä kieltä. Tämän käyttöoppaan esimerkit ajetaan englanniksi —
lohkot tuottaa `make manual`, niitä ei koskaan kopioida käsin.

**Miten kieli valitaan.** Kieliasetus on ensimmäinen muuttujista `LC_ALL`, `LC_MESSAGES` ja `LANG`, joka on asetettu
eikä ole tyhjä; muuttuja `LANGUAGE`, kun se on asetettu, luettelee sitten halutut kielet
kaksoispisteillä erotettuina (`LANGUAGE=pt_BR:fr`), ja se luetaan ensin. Tämä on järjestys, jota gettext noudattaa, ja
ohjelma lukee sen itse: kieliasetusta ei tarvitse olla luotu koneelle.

| Sääntö | Vaikutus |
| :--- | :----- |
| merkistökoodaus ja tarkenne | ohitetaan: `fr_FR.UTF-8@euro` etsii ensin `fr_FR`, sitten `fr` |
| `LC_ALL=C`, kieliasetus `C` tai `POSIX` tai ei kieliasetusta lainkaan | englanti, eikä muuttujaa `LANGUAGE` lueta |
| englannin nimeävä merkintä (`en`, `en_GB`) | englanti: sitä seuraavaa muuttujan `LANGUAGE` osaa ei lueta |
| kieli, jonka käännösluetteloa ei ole asennettu | seuraava pyydetty, ja viimeisen jälkeen englanti, **sanaakaan sanomatta** |
| viesti, jota käännösluettelo ei käännä | näytetään englanniksi |

**Korjauslausekkeen nimi**, jota `correct` kertoo, ettei se voinut käyttää, noudattaa myös kieltä,
kun käännösluettelo tuntee sen — ohjelman mukana toimitetut lausekkeet; oma lauseke
nimetään niin kuin sen tiedosto sen kirjoittaa. Itse lauseketiedostoja ei koskaan kirjoiteta uudelleen.

Käännösluettelot asennetaan polkuun `<prefix>/share/subedit/locale/<language>/LC_MESSAGES/subedit.mo`,
ja ne löydetään ohjelman omasta hakemistosta käsin. Englanti on sisäänrakennettu. Tämä versio asentaa kielet `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR`, `ru`, `sr`, `tr` ja `zh_CN` — katalaanin,
tšekin, saksan, espanjan, suomen, ranskan, galician, heprean, unkarin, interlinguen, islannin, italian, hollannin, puolan, portugalin, brasilianportugalin, venäjän, serbian, turkin ja yksinkertaistetun kiinan — kokonaan käännettyinä. Ranskan on tarkistanut
kielen lukija; katalaani, tšekki, saksa, espanja, suomi, galicia, heprea, unkari, interlingue, islanti, italia, hollanti, puola, portugali, brasilianportugali, venäjä, serbia, turkki ja yksinkertaistettu kiina ovat
konekäännöksiä, eikä kukaan kielen lukija ole vielä tarkistanut niitä.
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
<!-- machine-translated: ru -->
<!-- machine-translated: sr -->
<!-- machine-translated: tr -->
<!-- machine-translated: zh_CN -->
Kun `LANG=fr_FR.UTF-8`, ohje `subedit-cli --help` alkaa sanoin ”Lire,
inspecter et recaler des fichiers de sous-titres.” Skripti, joka lukee tekstitulosteen
sanoja, asettaa `LANGUAGE=en` (tai `LC_ALL=C`) — tai lukee muotoa `--format json`, joka ei koskaan muutu.

**Mikä ei koskaan noudata kieltä** — se, mitä skripti lukee:

| Ei koskaan käännetä | Miksi |
| :--------------- | :-- |
| tuloste muodossa `--format json`: avaimet, arvot sekä epäonnistumisten ja varoitusten `message` ja `detail` | samat tavut kaikilla kielillä; sanat ovat englantia |
| paluuarvot | [Paluuarvojen](#paluuarvot) neljä arvoa kaikilla kielillä |
| kirjoitettavat nimet: alikomennot, valitsimet, niiden arvot (`off`, `srt`), metamuuttujat (`FILE`, `TEXT`, `OPTIONS`) | kerran kirjoitettu komentorivi toimii kaikilla kielillä |
| korjauslausekkeiden nimet, jotka `correct --enable` ja `--disable` ottavat | lausekkeeseen viitataan englanninkielisellä nimellä, joka sen tiedostossa on |
| muotojen ja merkistökoodausten nimet | muotoon viitataan sen nimellä |
| CLI11:n omat huomautukset — puuttuva pakollinen argumentti, tuntematon valitsin, arvo, jota valitsin ei hyväksy — ja sen `Run with --help for more information.` | ne on muotoiltu sen jäsentimen sisällä, jonka päälle komentorivi on rakennettu; ne pysyvät englanninkielisinä |

Englannin pyytäminen ympäristöstä riippumatta:

<!-- example: LC_ALL=C LANGUAGE=fr subedit-cli shift --output-dir out a.srt; echo "code=$?" -->
```console
$ LC_ALL=C LANGUAGE=fr subedit-cli shift --output-dir out a.srt; echo "code=$?"
shift needs --by, or --to-grid to work the amount out from the positions
code=1
```

## Yleiset valitsimet

**Ne kirjoitetaan ennen alikomennon nimeä, ja vain siihen.**
`subedit-cli --encoding cp1252 inspect film.srt` on oikea muoto;
`subedit-cli inspect --encoding cp1252 film.srt` hylätään viestillä
`The following argument was not expected: --encoding`. Sama koskee
valitsimia `--version`, `-v` ja `--quiet`.

**`--help` on poikkeus, eikä se oikeastaan ole poikkeus**:
`subedit-cli inspect --help` hyväksytään, koska jokaisella alikomennolla on oma ohjeensa, ja se kirjoittaa
alikomennon ohjeen, ei työkalun.

| Valitsin | Vaikutus |
| :----- | :---- |
| `-h`, `--help` | kirjoittaa ohjeen ja pysähtyy; alikomennon ohjeen, jos alikomento nimetään |
| `--version` | kirjoittaa `subedit <version>`, kirjastot, joiden varassa työkalu toimii, ja asennetut kielet, ja pysähtyy — katso [`--version`](#--version) |
| `-q`, `--quiet` | taso 0 — ei lainkaan kerrontaa |
| `-v`, `-vv`, `-vvv` | tasot 1–3; taso 1 on oletus |
| `--encoding NAME` | lukee tiedostot tällä merkistökoodauksella sen arvaamisen sijaan |
| `--list-encodings` | kirjoittaa merkistökoodaukset, joita työkalu osaa lukea ja kirjoittaa, yhden riville, ja **pysähtyy** (koodi `0`) — katso [`--list-encodings`](#--list-encodings) |
| `--format text\|json` | tuloksen muoto vakiotulosteessa: teksti tai yksi JSON-objekti tiedostoa kohden — katso [Skriptin luettava tuloste](#skriptin-luettava-tuloste) |
| `--config FILE` | lukee toimintojen asetukset tiedostosta `FILE` käyttäjän asetustiedoston sijaan — katso [Asetustiedosto](#asetustiedosto) |
| `--no-config` | ei lue mitään asetustiedostoa: jokainen toiminnon asetus, jota ei anneta komentorivillä, saa sisäänrakennetun oletusarvonsa — katso [Asetustiedosto](#asetustiedosto) |

`--quiet` ja `-v` samassa käynnistyksessä hylätään: kahta vastakkaista aikomusta
ei ratkaista viimeksi kirjoitetun hyväksi.

Myös `--config` ja `--no-config` samassa käynnistyksessä hylätään viestillä
`--config excludes --no-config`, samoin `--config` tyhjällä nimellä viestillä
`--config needs a file name`: tyhjä nimi ei nimeä mitään tiedostoa, eikä käyttäjän omaa tiedostoa lueta
sen sijaan.

### Asetustiedosto

**[`adjust`](adjust.md) ja [`correct`](correct.md) lukevat oletuksena asetustiedoston
toimintojen asetukset** — arvot, jotka `Adjust Durations…` ja `Correct Texts…` muistavat
ikkunassa tai jotka [`config set`](config.md) kirjoitti. Mikään muu alikomento ei lue sitä. Tiedosto on:

```
$XDG_CONFIG_HOME/subedit/settings.conf
```

tai `~/.config/subedit/settings.conf`, kun `XDG_CONFIG_HOME` on asettamatta, tyhjä tai ei ole absoluuttinen
polku — ikkunan tiedosto, joka kuvataan [sen käyttöoppaassa](../subedit-gui/preferences.md#tiedosto).

**Etusijajärjestys vahvimmasta alkaen: komentorivin valitsin, sitten tiedosto, sitten sisäänrakennettu
oletus.** Valitsin voittaa aina: `--speed off` kytkee pois lukunopeuden, jonka tiedosto asettaa. Tiedostosta luetaan vain
**toimintojen lohko**; näytön asetuksia (ikkunan mitat, sarakkeet, teema…)
ei koskaan lueta. [`config list`](config.md#config-list) nimeää avaimet ja kertoo, mitkä niistä komentorivi
lukee: yhdeksän avainta `duration-adjust.*`, sekä `correction.human`, `correction.ocr`,
`correction.remove-blank` ja `correction.line-break.max-lines`. **Jokaisella niistä on valitsin,
joka ohittaa sen yhden käynnistyksen ajaksi**, asetti tiedosto sen miten tahansa — alikomentojen
[`adjust`](adjust.md#mitä-asetustiedosto-antaa) ja
[`correct`](correct.md#mitä-valitaan-ja-mitä-ei) taulukot nimeävät ne.

**Käyttäjän tiedoston lukeminen ei koskaan kaada komentoa.** Puuttuva tiedosto antaa oletukset; tiedosto,
jota ei voi lukea, antaa oletukset ja kertoo sen; arvo, jota ei voi lukea, säilyttää oletuksensa ja nimetään;
tuntematon avain ohitetaan. Alikomennot `adjust` ja `correct` eivät koskaan kirjoita tiedostoon.

**Se, mitä luettiin, kerrotaan** oletustasolla 1 virhetulosteeseen ennen ensimmäistä tiedostoa:

| Tapaus | Rivi |
| :--- | :--- |
| tiedosto luettiin | `settings: read <path>` |
| jotakin sen arvoa ei voitu lukea | `settings: <key>: cannot read '<value>', default kept` |
| käyttäjän tiedostoa ei ole olemassa | `settings: <path> not found, defaults used` |
| tiedostoa ei voi lukea | `settings: <path> unreadable, defaults used` |
| `--no-config` | ei mitään |

Valitsimella [`--format json`](#skriptin-luettava-tuloste) alikomentojen `adjust` ja
`correct` kunkin tiedoston objektissa on kenttä `settings`: luettu polku tai `null`, kun mitään ei luettu.


**`--no-config` tekee ajosta toistettavan**: sama komentorivi antaa saman tuloksen jokaisella
koneella riippumatta siitä, mitä sen käyttäjä on asettanut ikkunassa. Käytä sitä skripteissä, testeissä ja vikailmoituksissa — tai
valitsinta `--config FILE` niiden mukana kulkevan tiedoston kanssa.

**`--config FILE` nimeää toisen tiedoston**, joka luetaan samalla tavalla — **paitsi että sen on oltava olemassa**.
Komentorivillä annettu nimi ei ole pyyntö käyttää oletuksia: kun `FILE` ei ole olemassa,
`adjust` ja `correct` pysähtyvät ennen minkään syötteen lukemista, kirjoittavat virhetulosteeseen
`settings file not found: <FILE>` ja päättyvät käyttövirheen koodilla `1`. Olemassa oleva `FILE`, jota ei voi lukea,
antaa oletukset ja kertoo sen, kuten käyttäjän tiedosto. [`config show`](config.md#config-show)
näyttää puuttuvan tiedoston `FILE` silti muodossa `(not found)`, ja [`config set`](config.md#config-set-ja-config-unset) silti
luo sen.

<!-- example: printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine.conf adjust --dry-run a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine.conf adjust --dry-run a.srt; echo "code=$?"
settings file not found: mine.conf
code=1
```

### `--version`

**Ohjelman versio yksinään ensimmäisellä rivillä** — `subedit <version>`, joka oli koko tuloste versioon
0.15.38 asti ja jonka skripti lukee. Sitten **yksi rivi kutakin kirjastoa kohden, jonka varassa työkalu toimii**, sitten asennetut
käyttöliittymän kielet, vakiotulosteeseen; työkalu pysähtyy koodilla `0`. Samalla annettua alikomentoa
ei ajeta, eivätkä `-q` ja `-v` muuta mitään.

<!-- example: subedit-cli --version | head -n 1 -->
```console
$ subedit-cli --version | head -n 1
subedit 1.1.5
```

Koko tuloste koneella, jossa on ICU 74.2 ja Enchant 2.3.3 — numerot ovat koneen omia:

```console
$ subedit-cli --version
subedit <version>
  ICU      74.2 (built with 74.2)
  Enchant  2.3.3 (built with 2.3.3)
  CLI11    2.4.1 (built with 2.4.1)
  Languages: en, fr
```

| Kirjasto | Mihin työkalu sitä käyttää |
| :------ | :------------------------ |
| `ICU` | merkistökoodausten lukemiseen, kirjoittamiseen ja tunnistamiseen sekä kirjainten koon muuttamiseen |
| `Enchant` | alikomennon `correct` oikolukuun |
| `CLI11` | tämän komentorivin lukemiseen |

- **Jokainen rivi antaa kaksi versiota**: ajon aikana löydetyn ensin ja sen, jota vasten ohjelma
  käännettiin, sulkeissa. Ne eroavat, kun koneen kirjasto on päivitetty ohjelman
  kääntämisen jälkeen — ensimmäinen asia, joka kannattaa antaa vikailmoituksessa.
- **Enchantin otsaketiedostoissa ei ole versiota**: sulkeissa oleva on se, jonka `pkg-config` ilmoitti,
  kun käännös määritettiin.
- **CLI11 on käännetty ohjelman sisään**, sillä se koostuu pelkistä otsaketiedostoista: sen kaksi versiota ovat
  aina samat.
- **Ei Qt:tä eikä libmpv:tä**: komentorivi ei linkitä kumpaakaan, ja se luettelee vain sen, minkä varassa se toimii.
  Ikkuna luettelee ne — katso [sen käyttöopas](../subedit-gui/invocation.md#--version).
- **`Languages`** luettelee koodin `en`, lähdekielen, joka ei tarvitse käännösluetteloa, ja sitten koodeittain jokaisen
  hakemistoon `<prefix>/share/subedit/locale` asennetun käännösluettelon, joka voidaan lukea — ne kielet, joita
  `Muokkaa ▸ Asetukset…` tarjoaa.
- **Mikä noudattaa kieltä**: `built with` ja `Languages`. Ensimmäinen rivi, nimet,
  numerot ja kielikoodit eivät koskaan.
- **Valitsimella `--format json`**, ennen tai jälkeen valitsimen `--version`, **yksi objekti yhdellä rivillä**: `schema`
  (`1`), `command` (`"version"`), `version`, `libraries` — taulukko muotoa
  `{"name":"ICU","loaded":"74.2","built":"74.2"}` tekstin järjestyksessä — ja `languages`,
  koodien taulukko. Mitään siinä ei käännetä.

<!-- example: subedit-cli --version --format json | grep -o '"name":"[^"]*"' -->
```console
$ subedit-cli --version --format json | grep -o '"name":"[^"]*"'
"name":"ICU"
"name":"Enchant"
"name":"CLI11"
```

### `--list-encodings`

**Merkistökoodaukset, joita työkalu osaa lukea ja kirjoittaa, yksi riville, nimen mukaan järjestettyinä**, vakiotulosteeseen —
sitten työkalu pysähtyy koodilla `0`, kuten valitsimella `--version`: se on vastaus, ei vaihe.
Samalla annettua alikomentoa ei ajeta.

<!-- example: subedit-cli --list-encodings | grep '^UTF' | head -4 -->
```console
$ subedit-cli --list-encodings | grep '^UTF' | head -4
UTF-16BE
UTF-16LE
UTF-32BE
UTF-32LE
```

- **Ne ovat nimet, jotka `--encoding` hyväksyy**, kanoniset nimet; niiden vaihtoehtoiset nimet (`cp1252`) hyväksytään myös,
  vaikka niitä ei luetella.
- **Määrä riippuu asennetusta ICU:sta**, eikä mitään nimeä luvata niiden lisäksi, jotka jokaisessa ICU:ssa on
  (`UTF-8`, `UTF-16LE`, `windows-1252`…). Käyttöopas ei siksi anna määrää.
- **`UTF-16` ja `UTF-32` eivät ole mukana**: ICU tuntee ne, mutta niiden muunnin kirjoittaa oman merkkinsä,
  ja malli hylkää ne — katso alla. Luettelo, josta valitaan, ei tarjoa sitä, minkä seuraava kenttä
  hylkäisi.
- **Valitsimella `--format json`** **yksi objekti kutakin merkistökoodausta kohden**:
  `{"schema":1,"command":"list-encodings","name":"UTF-8"}`. Se ei ole tiedoston raportti: näissä
  objekteissa **ei ole kenttää `file` eikä kenttää `ok`**, mikä on ainoa poikkeus
  [rakenteen](#rakenne) kehyksestä. Ne ovat silti yksi objekti riville, yksi kutakin merkintää kohden.
- **Kerronnan taso ei muuta mitään**: luettelo on tulos, eikä `-q` poista sitä.

### `--encoding` ja mitä se ei voi pakottaa

**Se koskee jokaista alikomentoa**, koska ne kaikki lukevat: tiedosto, jonka
merkistökoodaus arvataan väärin, pysyy vääränä, tehtiinpä sille mitä tahansa, ja valitsin,
joka olisi vain alikomennolla `inspect`, jättäisi ristiriidan ilman korjauskeinoa.

**Nimi on ICU:n, ja kaikki sen vaihtoehtoiset nimet hyväksytään**: `cp1252`,
`windows-1252` ja `WINDOWS 1252` nimeävät saman merkistökoodauksen. Joukkoa ei ole kirjoitettu
mihinkään: se on se, minkä koneen ICU osaa muuntaa, ja
[`--list-encodings`](#--list-encodings) luettelee sen. Nimi, jota se ei osaa muuntaa,
hylätään ennen kuin yhtäkään tiedostoa luetaan:

```console
$ subedit-cli --encoding klingon-1 inspect film.srt
no encoding is named "klingon-1"
```

**Yksi poikkeus, ja vain yksi: merkistökoodauksen on nimettävä tavujärjestyksensä.**
`UTF-16` ja `UTF-32` ovat nimiä, jotka ICU tuntee ja joiden muunnin kirjoittaa **oman
merkkinsä**. Tällainen merkistökoodaus ei mahdu tähän malliin — merkki lakkaisi
olemasta valinta, `--no-bom` hyväksyttäisiin mutta sitä ei noudatettaisi — joten molemmat
hylätään, niin luettaessa kuin kirjoitettaessa:

```console
$ subedit-cli --encoding UTF-16 inspect film.srt
"UTF-16" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do
```

`UTF-16LE` ja `UTF-16BE` lukevat ja kirjoittavat samat tavut merkki mukaan lukien; se, mitä
tarkka nimi lisää, on tieto siitä, kumpi on kyseessä.

**Tavujärjestysmerkki voittaa valitsimen.** Se on ainoa asia, jonka tekstitystiedosto
ilmoittaa itsestään, ja sen lukeminen toisin kuin se itsestään ilmoittaa
olisi kutsujan totelemista tiedostoa vastaan. Ristiriita **kerrotaan**
huomautuksena:

```console
$ subedit-cli -vvv --encoding cp1252 inspect avec-bom.srt
avec-bom.srt: a byte order mark that contradicts the encoding asked for ("UTF-16LE"), settled by the reader
```

Niin tekee Gaupol — se aloittaa lukemisen uudelleen merkin
merkistökoodauksella — mutta sanomatta mitään; tässä ero sen välillä, mitä pyydettiin, ja sen,
mitä tehtiin, ei ole vaikenemista.

## Alikomennot

| Alikomento | Mitä se tekee | Kirjoittaako se? |
| :------------ | :-------------- | :----------- |
| [`inspect`](inspect.md) | kertoo, mistä tiedosto koostuu, muuttamatta mitään | ei |
| [`convert`](convert.md) | kirjoittaa tiedoston toiseen muotoon tai toiseen rakenteeseen | kyllä |
| [`shift`](shift.md) | siirtää jokaista kohtaa saman verran | kyllä |
| [`transform`](transform.md) | korjaa jokaisen kohdan kahden oikeaksi tiedetyn pisteen avulla | kyllä |
| [`framerate`](framerate.md) | ajoittaa tiedoston uudelleen ruutunopeudelta toiselle | kyllä |
| [`snap`](snap.md) | siirtää jokaisen aikaleiman lähimpään ruutuun | kyllä |
| [`hearing-impaired`](hearing-impaired.md) | poistaa kuulovammaisille tarkoitetut maininnat | kyllä |
| [`adjust`](adjust.md) | säätää kestot lukunopeuden ja rajojen mukaan | kyllä |
| [`replace`](replace.md) | korvaa tekstin kappaleissa rikkomatta tagia | kyllä |
| [`case`](case.md) | muuttaa tekstit muotoon Jokainen Sana Isolla, lauseen kirjainkokoon, isoiksi tai pieniksi kirjaimiksi | kyllä |
| [`italics`](italics.md) | kursivoi tekstit tai poistaa niiden kursiivin | kyllä |
| [`dialogue-dashes`](dialogue-dashes.md) | lisää tai poistaa vuoropuheluviivat | kyllä |
| [`sort`](sort.md) | palauttaa kappaleet alkukohtiensa järjestykseen | kyllä |
| [`join-repeats`](join-repeats.md) | tekee yhden kappaleen saman tekstin peräkkäisistä palasista | kyllä |
| [`disjoint`](disjoint.md) | leikkaa päällekkäiset kappaleet niin, ettei mikään aikaväli enää ole toisen päällä | kyllä |
| [`repair`](repair.md) | ketjuttaa alikomennot `join-repeats`, `sort` ja `disjoint` | kyllä |
| [`correct`](correct.md) | korjaa tekstit Gaupolin lausekkeilla: maininnat, sanojen yhdistäminen ja jakaminen, yleiset virheet, isot alkukirjaimet, rivinvaihto | kyllä |
| [`append`](append.md) | liittää tiedostot ensimmäisen perään **yhdeksi** tiedostoksi | kyllä |
| [`split-file`](split-file.md) | leikkaa tiedoston **kahtia**: alkuosaan ja loppuosaan | kyllä |
| [`pair`](pair.md) | kirjoittaa käännöksen päätiedostonsa kohtiin ja kertoo, miten sen rivit asettuivat kohdakkain | kyllä |
| [`transcribe`](transcribe.md) | kirjoittaa videon tai äänitiedoston puheen tekstitykseksi | kyllä — oma kohteensa, `--output` |
| [`sync`](sync.md) | ajoittaa tiedoston tekstitykset uudelleen elokuvan äänen mukaan | kyllä |
| [`config`](config.md) | luettelee, näyttää ja muuttaa asetustiedoston toimintojen asetuksia | vain asetustiedostoon |

Kaikki kaksikymmentäkolme ovat mukana; työkalun ohje luettelee ne samassa järjestyksessä.

**Vain yksi ei koske mihinkään**, ja sen kertoo oikeanpuoleinen sarake: `inspect` lukee ja
raportoi. Muut kaksikymmentäyksi kirjoittavat, ja seuraava koskee kahdeksaatoista niistä:
[`append`](append.md) ja [`split-file`](split-file.md) ottavat kumpikin omanlaisensa määrän tiedostoja — N tiedostoa
**yhteen** tulokseen, yksi tiedosto **kahteen** — ja siksi niillä on oma kohteensa, jonka ne kuvaavat sivullaan;
[`transcribe`](transcribe.md) tekee videosta tai äänitiedostosta uuden tiedoston ja ottaa vain `--output`in.
[`config`](config.md) ei kirjoita tekstitystiedostoa: se lukee ja muuttaa asetustiedostoa, eikä
mikään seuraavasta koske sitä.

## Kohde

Kahdeksantoista kirjoittavaa alikomentoa — [`convert`](convert.md),
[`shift`](shift.md), [`transform`](transform.md), [`framerate`](framerate.md),
[`snap`](snap.md), [`hearing-impaired`](hearing-impaired.md), [`adjust`](adjust.md),
[`replace`](replace.md), [`case`](case.md), [`italics`](italics.md),
[`dialogue-dashes`](dialogue-dashes.md), [`sort`](sort.md), [`join-repeats`](join-repeats.md),
[`disjoint`](disjoint.md), [`repair`](repair.md), [`correct`](correct.md), [`pair`](pair.md), [`sync`](sync.md) — ottavat kohteensa samalla tavalla. ([`append`](append.md) ja
[`split-file`](split-file.md) käyttävät omaansa, joka kuvataan niiden sivulla.) [`inspect`](inspect.md) ei kirjoita tiedostoa eikä
hyväksy mitään näistä valitsimista.

**Jo olemassa olevan kohteen päälle kirjoitetaan** kysymättä, eikä sitä voi
välttää millään valitsimella: kirjoittaminen on atominen — tiedostoa ei koskaan kirjoiteta puolittain, ja
keskeytynyt kirjoitus jättää vanhan tiedoston ehjäksi —, mutta aiempi sisältö
menetetään. Tämä ei ole säilytetty vika: se on sääntö, ja komennon ajaminen
uudelleen samaan tulostehakemistoon ajaa sen uudelleen itsensä päälle.

**Mitään ei koskaan kirjoiteta ilman nimenomaista kohdetta.** Kolme tapaa
antaa kohde sulkevat toisensa pois, ja yksi tarvitaan: ilman sitä mitään ei kirjoiteta, ja paluuarvo
on `1` — paitsi valitsimella [`--dry-run`](#esikatselu-ennen-kirjoittamista), joka ei kirjoita mitään
eikä siksi vaadi kohdetta.

| Valitsin | Minne tulos menee |
| :----- | :---------------- |
| `--output FILE` | tähän tiedostoon, täsmälleen tällä nimellä; yksi ainoa syöte |
| `--output-dir DIR` | tähän hakemistoon, syötteen perusnimellä |
| `--in-place` | syötteen päälle, atomisella kirjoituksella |

Työkalu, joka kirjoittaa syötteensä päälle, koska sille ei sanottu mitään, on työkalu, jonka
käyttö lopetetaan toisella kerralla. Kieltäytyminen maksaa yhden rivin ja pelastaa
tiedoston.

`--output` usealla syötteellä hylätään: viimeinen tiedosto kirjoitettaisiin
edellisten päälle. Erälle `--output-dir` on kolmesta ainoa, joka on
järkevä, valitsimen `--in-place` lisäksi.

**Tulostehakemisto luodaan**, jos sitä ei ole, ylähakemistot mukaan lukien — valitsimelle
`--output-dir` ja sille hakemistolle, johon `--output` kirjoittaa. Se tehdään, kun
komentorivi on validoitu, ja **ennen ensimmäistä tiedostoa**. Jos sitä ei voi luoda,
siitä kerrotaan kerran, yhtäkään tiedostoa ei lueta, ja paluuarvo on
`2`.

**Erä arvioidaan kokonaisuutena ennen kirjoittamista.** Kunkin syötteen kohde
selvitetään ensin, ja siitä seuraa kaksi hylkäystä — käyttövirheitä, koodi
`1`, **yhtään tiedostoa ei kirjoiteta** eikä hakemistoa luoda:

- **kaksi syötettä, jotka päätyisivät samaan tiedostoon** — `a/film.srt` ja
  `b/film.srt` valitsimella `--output-dir`, tai `a/film.srt` ja `b/film.vtt`, jotka molemmat
  muunnetaan WebVTT-muotoon ja joiden lopullisen kohteen tiedostopääte muuttui. Viesti
  nimeää kohteen ja molemmat syötteet; toisen nimeäminen uudelleen tai
  viimeisen antaminen voittaa menettäisi tiedoston sanomatta mitään, ja hylkääminen
  maksaa yhden rivin;
- **kohde, joka on yksi syötteistä**, ilman valitsinta `--in-place` — myös
  muodon `--output-dir .`, valitsimen `--output` tai symbolisen linkin kautta. Oman syötteen päälle
  kirjoittaminen on teko, joka nimetään.

Kaksi polkua nimeää saman tiedoston, kun järjestelmä sanoo niin, ei silloin, kun ne
kirjoitetaan samalla tavalla: `in/../in/film.srt` ja `in/film.srt` ovat yksi ja sama tiedosto.

**Tiedostopääte noudattaa kirjoitettua muotoa.** Se muuttuu vain alikomennolla
[`convert`](convert.md), ainoalla alikomennolla, joka vaihtaa muotoa; muut
säilyttävät luetun tiedoston muodon ja siten sen tiedostopäätteen.

| Mikä sen aiheuttaa | Viesti |
| :------------------ | :------ |
| ei kohdetta, ilman valitsinta `--dry-run` | `no destination given: use --output, --output-dir or --in-place` |
| kaksi kohdetta | `--output, --output-dir and --in-place exclude one another` |
| `--output` erälle | `--output names one file but several were given: use --output-dir instead` |
| kaksi syötettä, yksi kohde | `<destination>: would be written by both <input> and <input>` |
| syötteen päälle kirjoitettaisiin | `<destination>: would be written over the input <input>` — sama lause alikomennoille [`append`](append.md) ja [`split-file`](split-file.md) |
| tulostehakemistoa ei voi luoda | `<directory>: cannot be created: permission denied` tai `: cannot be created` |
| kohteeseen ei voi kirjoittaa | `<path>: <destination>: cannot be written: permission denied` tai `: cannot be written` |
| kirjoitettavasta merkistökoodauksesta puuttuva merkki | `<path>: holds a character the chosen encoding cannot write` |

**Viimeinen ansaitsee lauseen.** Tiedosto kirjoitetaan takaisin **samalla merkistökoodauksella,
jolla se luettiin**, kuten rivinvaihtojensa ja merkkinsä kanssa. Jos
muutos toi siihen merkin, jota tämä merkistökoodaus ei tunne — esimerkiksi
`ł` Latin-1-koodauksessa —, kirjoittaminen epäonnistuu eikä **mitään kirjoiteta**. Sen korvaaminen
merkillä `?` olisi tekstin menettämistä juuri sen kirjoittaneen silmien edessä.

## Esikatselu ennen kirjoittamista

**`--dry-run`** hyväksytään jokaisessa kuudessatoista kirjoittavassa alikomennossa, ja se tarkoittaa samaa
kaikkialla: **lue, laske, raportoi, älä kirjoita tiedostoa**. Se lisätään
komentoriviin, joka oli juuri ajettavana; soveltaminen on saman rivin ajamista uudelleen ilman sitä. Laskenta on
deterministinen: se, minkä `--dry-run` näytti, on se, minkä seuraava ajo kirjoittaa, kunhan
tiedosto ei ole muuttunut.

- **Se ei kirjoita tiedostoa eikä luo hakemistoa**, ei myöskään valitsimella `--in-place`: syötetiedosto
  pysyy ennallaan.
- **Se ei vaadi kohdetta.** Kohteen voi antaa, ja **se tarkistetaan silloin kuten ilman
  valitsinta `--dry-run`**: kaksi syötettä samalle kohteelle, syöte, jonka päälle kirjoitettaisiin,
  `--output` erälle, kaksi tapaa sanoa minne — komentorivi, joka hylätään ilman valitsinta `--dry-run`,
  hylätään myös sen kanssa samalla lauseella ja samalla koodilla `1`.
- **Paluuarvo on sama kuin todellisessa ajossa**: `0`, kun kaikki laskettiin, `2` tai `3`,
  kun tiedostoja ei voitu lukea tai käsitellä, `1` käyttövirheelle. Merkki, jota
  tiedoston merkistökoodaus ei voinut kirjoittaa, kaataa koeajon samoin kuin todellisen ajon.
  **Muutosten olemassaolo ei muuta koodia**: neljä koodia kertovat, onnistuiko työkalu,
  eivät sitä, onko tiedosto kunnossa.
- **Kerronta kertoo sen.** Kun todellinen ajo kirjoittaa `-> <destination>`, koeajo
  kirjoittaa `(dry run, nothing written)`.

<!-- example: printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by 1 --dry-run a.srt; ls -- *.srt -->
```console
$ printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by 1 --dry-run a.srt; ls -- *.srt
a.srt: 1 subtitle shifted by 1.000 s (dry run, nothing written)
a.srt
```

**Tekstejä muuttavassa alikomennossa** —
[`hearing-impaired`](hearing-impaired.md), [`replace`](replace.md), [`case`](case.md),
[`italics`](italics.md), [`dialogue-dashes`](dialogue-dashes.md) ja [`correct`](correct.md) — **vakiotuloste sisältää ehdotetut
muutokset**. Yksi lohko kutakin muuttunutta kappaletta kohden tiedoston järjestyksessä:

```
<path>: subtitle <number>
- <the text before, line by line>
+ <the text after, line by line>
```

- **numero** on se, joka kappaleella on luetussa tiedostossa, ykkösestä alkaen laskettuna;
- tekstin jokaisen rivin edessä ennen muutosta on `- `, jokaisen rivin edessä muutoksen jälkeen `+ `: usean
  rivin teksti luetaan sellaisenaan, ja rivi, joka itse alkaa vuoropuheluviivalla,
  antaa `- - `;
- **kappale, jonka toiminto poistaisi**, kertoo sen ensimmäisellä rivillään,
  `<path>: subtitle <number> (removed)`, eikä sillä ole rivejä, jotka alkavat merkillä `+`;
- käännöksen kappale ilmoittaa itsensä valitsimella `--document translation` merkinnällä `(translation)` samassa
  kohdassa — katso [Käännös](#käännös);
- kunkin lohkon ensimmäinen rivi alkaa polulla, jotta `grep '^film.srt: '`
  löytää yhden tiedoston lohkot erästä;
- **ei yhtään lohkoa**, kun mikään ei muuttuisi.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\n- [Grincement]\n- Qui est là ?\n\n' > mentions.srt; subedit-cli hearing-impaired --dry-run mentions.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\n- [Grincement]\n- Qui est là ?\n\n' > mentions.srt; subedit-cli hearing-impaired --dry-run mentions.srt
mentions.srt: 2 subtitles cleaned, 1 removed (dry run, nothing written)
mentions.srt: subtitle 1 (removed)
- [Bruit de pas]
mentions.srt: subtitle 2
- Attends [il tousse] Marie.
+ Attends Marie.
mentions.srt: subtitle 3
- - [Grincement]
- - Qui est là ?
+ Qui est là ?
```

**Luettelo on sama JSON-muodossa**: tiedoston objektissa on taulukko `changes`, katso
[rakenne](#rakenne), ja poistetun kappaleen kentän `after` arvo on `null`. **Se lasketaan
vain, kun `--dry-run` tai `--format json` pyytää sitä**: tuhansien kappaleiden erä
ei rakenna sitä turhaan.

**Muissa alikomennoissa** — [`convert`](convert.md), [`shift`](shift.md),
[`transform`](transform.md), [`framerate`](framerate.md), [`snap`](snap.md),
[`adjust`](adjust.md), [`sort`](sort.md), [`append`](append.md), [`split-file`](split-file.md), [`pair`](pair.md) — vakiotuloste
sisältää sen, minkä todellisen ajon vakiotuloste sisältäisi, eli tekstimuodossa ei mitään;
JSON-muodossa kunkin tiedoston objektin, jossa on `"dry_run":true`, `"destination":null` ja sama kenttä
`counts`.

## Kaksi tulostetta, kaksi tehtävää

| Tuloste | Mitä se sisältää |
| :----- | :--------------- |
| vakiotuloste | **tuloksen ja vain tuloksen** — alikomennon `inspect` raportin, tekstejä muuttavan alikomennon ehdotetut muutokset valitsimella [`--dry-run`](#esikatselu-ennen-kirjoittamista); valitsimella `--format json` yhden objektin tiedostoa kohden — yhden ajoa kohden alikomennoille [`append`](append.md), [`split-file`](split-file.md) ja [`pair`](pair.md) |
| virhetuloste | **kaiken muun** — kerronnan, varoitukset, virheet |

Juuri tämä jako mahdollistaa tuloksen uudelleenohjauksen ilman, että kerronta sekoittuu siihen:

```console
$ subedit-cli inspect *.srt > rapport.txt
```

Raportti menee tiedostoon, kerronta jää näytölle.

## Skriptin luettava tuloste

**`--format json`** korvaa vakiotulosteen tekstin **yhdellä JSON-objektilla kutakin
syötetiedostoa kohden** JSON Lines -muodossa: yksi tiivis objekti riville, UTF-8 ilman
merkkiä, rivinvaihto on `\n`. `text` on oletus eikä muutu. Se on
yleinen valitsin, ja siksi se kirjoitetaan ennen alikomentoa; muu arvo kuin
`text` ja `json` on käyttövirhe, koodi `1`.

<!-- example: printf '1\n00:00:10,000 --> 00:00:12,000\nUn.\n\n' > a.srt; subedit-cli --format json shift --by 1 --output-dir sortie a.srt absent.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:10,000 --> 00:00:12,000\nUn.\n\n' > a.srt; subedit-cli --format json shift --by 1 --output-dir sortie a.srt absent.srt; echo "code=$?"
a.srt: 1 subtitle shifted by 1.000 s -> sortie/a.srt
{"schema":1,"command":"shift","file":"a.srt","ok":true,"dry_run":false,"destination":"sortie/a.srt","counts":{"subtitles":1,"shifted_by_ms":1000},"warnings":[]}
absent.srt: does not exist
{"schema":1,"command":"shift","file":"absent.srt","ok":false,"error":{"kind":"not-found","message":"absent.srt: does not exist"}}
1 of 2 files shifted, 1 failed
code=3
```

**Yksi objekti kutakin syötettä kohden, ei koskaan nolla, ei koskaan kaksi**, onnistuipa syöte tai ei,
syötteiden antojärjestyksessä — tai löytymisjärjestyksessä valitsimella [`--recursive`](lots.md). Rivit
lasketaan, omat tiedostot löydetään. **Kolme alikomentoa on poikkeus, koska niiden
argumenttien määrä ei ole erän: [`append`](append.md) (N syötettä, yksi tuloste), [`split-file`](split-file.md)
(yksi syöte, kaksi tulostetta) ja [`pair`](pair.md) (kaksi tiedostoa, yksi tuloste) kirjoittavat **yhden objektin koko
ajolle**, ja se nimeää sen tiedoston, jonka niiden sivu kertoo.** **Uudelleenkirjoittavat alikomennot kirjoittavat siis myös
vakiotulosteeseen**, mitä niiden sopimus ei tekstimuodossa tehnyt: niiden
tiedosto pysyy niiden tuloksena, ja objekti on raportti siitä.

**Kerronta ei muutu.** `-q`, `-v`, `-vv` ja `-vvv` vaikuttavat
virhetulosteeseen tekstimuodossa kuten ilman valitsinta `--format`; **vakiotuloste ei vaihtele niiden mukaan**.
Lukemisen huomautukset, jotka tekstimuoto pitää tasolla 3, ovat dataa ja ovat
**aina** kentässä `warnings`. Käyttövirhe ei kirjoita objektia: se havaitaan
ennen kuin mihinkään tiedostoon kosketaan, joten sitä ei ole mihin liittää; se menee
virhetulosteeseen koodilla `1`, ja vakiotuloste jää tyhjäksi.

### Rakenne

| Avain | Sisältö | Mukana |
| :-- | :------ | :------- |
| `schema` | kokonaisluku, tämän rakenteen versio — `1` | aina |
| `command` | alikomennon nimi | aina |
| `file` | polku **sellaisena kuin se annettiin** tai sellaisena kuin hakemiston läpikäynti sen muodosti | aina |
| `ok` | totuusarvo | aina |
| `error` | `{"kind", "message"}` | jos `ok` on epätosi |
| `dry_run` | totuusarvo: tosi, kun ajossa on [`--dry-run`](#esikatselu-ennen-kirjoittamista) | jos `ok` on tosi, kirjoittavassa alikomennossa |
| `destination` | kirjoitettu polku tai `null`, kun `dry_run` on tosi: mitään ei kirjoitettu | samoin |
| `counts` | **kokonaislukujen** objekti, alikomennosta riippuva | samoin |
| `changes` | taulukko muotoa `{"subtitle", "document", "before", "after"}`: muuttuneet tekstit, katso alla | tekstejä muuttavassa alikomennossa |
| `warnings` | taulukko muotoa `{"kind", "line"?, "detail"?, "settled"?}` | jos `ok` on tosi |
| `alignment` | miten käännös asetettiin, katso [Käännös](#käännös) | valitsimella `-t` sekä alikomennolle `pair` |
| `constraints` | käytetyt asetukset | `adjust` |
| `settings` | luettu asetustiedosto tai `null`, kun mitään ei luettu — katso [Asetustiedosto](#asetustiedosto) | `adjust`, `correct`, jos `ok` on tosi |
| `inputs`, `tail` | yksi alkio kutakin perään lisättyä tiedostoa kohden; loppuosan polku | `append`; `split-file` |

`line` puuttuu koko tiedostoa koskevasta huomautuksesta; `detail` puuttuu, kun
lisättävää ei ole; `settled` on `true`, kun lukeminen **ratkaisi** jotakin — esimerkiksi
uudelleen luodun numeroinnin —, ja se puuttuu, kun lukeminen jätti asiat ennalleen.

**`changes`** luettelee tiedoston järjestyksessä jokaisen kappaleen, jonka teksti muuttuu:
`subtitle` on sen numero luetussa tiedostossa ykkösestä alkaen laskettuna; `document` on `"main"`
tai `"translation"`; `before` on teksti ennen; `after` teksti jälkeen tai **`null`, kun
kappale poistetaan**. Tekstin rivinvaihdot ovat siinä `\n`. Se
kirjoitetaan todelliselle ajolle kuten valitsimen [`--dry-run`](#esikatselu-ennen-kirjoittamista) ajolle, ja se on **tyhjä
(`[]`), kun mikään ei muutu**: sen läsnäolo kertoo, että alikomento luettelee muutoksensa,
sen puuttuminen, ettei sillä ole niitä.

**Ei liukulukuja missään.** Kohta tai kesto on kokonaisluku
**millisekunteina** (`…_ms`); määrä on kokonaisluku; ruutunopeus on **merkkijono**
(`"25"`, `"24000/1001"`), koska se on tarkka, mitä rationaaliluvulle liukuluku ei voi
taata; ruudukon keskittymä on kokonaisluku **tuhannesosina** — `1000` on
täydellinen ruudukko. Kaksi tulostetta voi silloin verrata tavu tavulta ilman pyöristystä.

**Polkua, joka ei ole UTF-8:aa** — Linux sallii sen —, ei voi esittää JSONissa: se
kirjoitetaan niin, että virheellisten tavujen tilalla on U+FFFD, ja objektissa on varoitus
`path-not-utf8`. Tällaisen polun uskollisuutta ei luvata, ja se sanotaan.

### Mitä kukin alikomento kirjoittaa kenttään `counts`

| Alikomento | Kentän `counts` avaimet |
| :------------ | :--------------- |
| [`shift`](shift.md) | `subtitles`, `shifted_by_ms` (etumerkillinen) |
| [`transform`](transform.md) | `subtitles` |
| [`framerate`](framerate.md) | `subtitles` |
| [`snap`](snap.md) | `subtitles`, `moved` (siirretyt kohdat), `furthest_ms` (suurin siirto) |
| [`hearing-impaired`](hearing-impaired.md) | `cleaned` (uudelleen kirjoitetut tekstit), `removed` (poistetut kappaleet) |
| [`replace`](replace.md) | `replaced` (korvatut osumat muuttuneissa kappaleissa), `matched` (löydetyt osumat) |
| [`case`](case.md), [`italics`](italics.md), [`dialogue-dashes`](dialogue-dashes.md) | `changed` (kappaleet, joiden teksti muuttui) |
| [`correct`](correct.md) | `corrected` (muuttuneet tekstit), `removed` (poistetut kappaleet) — ei koskaan osumia |
| [`sort`](sort.md) | `subtitles`, `moved` (paikat, joiden kappale vaihtui) |
| [`adjust`](adjust.md) | `subtitles` (kohteena olleet kappaleet), `adjusted` (ne, joiden loppu siirtyi), sitten ryhmä `sacrificed`: `speed`, `minimum`, `gap`; ja kentän `counts` ulkopuolella `constraints` — käytetyt asetukset |
| [`convert`](convert.md) | `subtitles`, sitten se, minkä muunnos menetti: `lost_ends` ja `lost_header` (0 tai 1), `joined_lines`, `lost_tags`, `lost_fields`, `furthest_ms` |
| [`append`](append.md) | `files`, `subtitles`, `appended`, sitten `lost_ends`, `lost_header`, `joined_lines`, `lost_tags`, `lost_fields`, `furthest_ms` yhteenlaskettuina; ja kentän `counts` ulkopuolella `inputs` — yksi alkio kutakin perään lisättyä tiedostoa kohden |
| [`split-file`](split-file.md) | `subtitles`, `head`, `tail`; ja kentän `counts` ulkopuolella `tail` — loppuosan polku (`destination` on alkuosa) |
| [`pair`](pair.md) | `subtitles` (kirjoitetun tiedoston kappaleet); ja kentän `counts` ulkopuolella `alignment` |
| [`inspect`](inspect.md) | ei kenttää `counts`: tiedoston kuvaus, katso sen sivu |

### Virhetunnisteet

`error.kind` on jokin näistä; **lukija käsittelee tuntemattoman tunnisteen
arvona ”muu”**.

| `kind` | Mitä se on |
| :----- | :----------- |
| `not-found`, `permission-denied`, `io` | järjestelmä kieltäytyi lukemasta — tai kirjoittamasta — tiedostoa |
| `unknown-format`, `no-subtitle-found`, `undecodable` | tiedosto luetaan, mutta se ei anna lukea itseään |
| `unencodable` | merkki, jota valittu merkistökoodaus ei voi kirjoittaa |
| `no-byte-order-mark`, `no-frame-rate` | mitä `convert` ei voi kirjoittaa, ellei sille kerrota |
| `before-the-origin`, `beyond-the-end`, `no-transform`, `no-grid` | toiminto, jota ei voi soveltaa tähän tiedostoon |
| `range-out-of-bounds` | `--range`, jota tämä tiedosto ei sisällä |
| `no-style` | muoto, joka ei kirjoita tyylejä (TMPlayer, LRC), alikomennolle `italics` |
| `refused` | mikä tahansa muu toiminnon hylkäys |

### Mitä luvataan ja mitä ei

**Luvataan niin kauan kuin `schema` on `1`**: avainten nimet, niiden tyyppi ja yksikkö; kenttien
`ok`, `file` ja `destination` merkitys; **yksi objekti kutakin syötettä kohden syötteiden järjestyksessä** — yksi ajoa kohden niille kolmelle alikomennolle, joilla on oma argumenttimääränsä;
kentän `kind` olemassa olevat tunnisteet; determinismi — samat syötteet, samat argumentit,
samat tavut.

**Ei luvata**: **viestien tekstiä** (`message`, `detail`), joka on ihmiselle tarkoitettua englantia
ja jota korjataan tarvittaessa — skripti, joka vertaa sitä, on rikki jo valmiiksi; **avainten
järjestystä** objektissa, joka on kiinteä mutta jota JSON ei takaa; **vielä lisäämättömien avainten**
läsnäoloa.

**Lukija siis ohittaa tuntemattoman avaimen** ja käsittelee kentän `kind` tuntemattoman arvon arvona ”muu”:
avaimen tai tunnisteen lisääminen ei muuta kenttää `schema`. Avaimen poistaminen tai uudelleennimeäminen,
tyypin tai yksikön muuttaminen tai tunnisteen poistaminen kasvattaa sitä — se on rikkova muutos,
josta ilmoitetaan CHANGELOGissa ja täällä.

## Kerronnan tasot

Jokainen taso **sisältää edellisen**: pykälän nouseminen lisää, ei koskaan
korvaa.

| Taso | Miten | Mitä virhetuloste sisältää |
| :----- | :------ | :------------------------------ |
| 0 | `--quiet` | ei mitään **virheitä lukuun ottamatta** |
| 1 | oletuksena tai `-v` | yksi rivi kutakin käsiteltyä tiedostoa kohden ja yhteenveto heti, kun tiedostoja on useita |
| 2 | `-vv` | ja mitä tunnistettiin: muoto, merkistökoodaus, BOM, rivinvaihdot |
| 3 | `-vvv` | ja vianetsinnän jälki: luetut ja kirjoitetut tavut sekä **jokainen lukemisen huomautus** |

**Virheitä ei koskaan vaienneta, ei myöskään valitsimella `--quiet`.** Komento, joka epäonnistuu
hiljaa, ei jättäisi jälkeensä muuta kuin paluuarvonsa.

Yhteenveto näkyy vasta kahdesta tiedostosta alkaen: yhdellä syötteellä se vain
toistaisi edellisen rivin.

## Huomautukset luettaessa

Tekstitysmuodot luetaan **niin hyvin kuin mahdollista**: poikkeaman kohdatessaan
lukija ei pysähdy — se tekee päätöksen tai jättää asiat ennalleen, ja se kertoo sen. Se,
mihin lukija törmäsi, tulostetaan **tasolla 3**, yksi rivi kutakin huomautusta kohden,
virhetulosteeseen:

```
a.srt: 2 diagnostics while reading
a.srt: line 6: SubRip numbers that do not follow ("7"), settled by the reader
a.srt: line 9: a line that fits nowhere, left as it stands
```

Kaikki kahdeksantoista alikomentoa ilmoittavat niistä, ei vain [`inspect`](inspect.md):
niin hyvin kuin mahdollista luettu ja sitten takaisin kirjoitettu tiedosto kävi läpi samat päätökset, ja niistä vaikeneminen
antaisi uskoa, ettei mitään tapahtunut.

**Huomautus ei ole koskaan epäonnistuminen.** Tiedosto luettiin, komento
onnistui, paluuarvo on `0`. Siksi ne ovat yksityiskohtaisimmalla
tasolla: alemmilla tasoilla ne hautaisivat alleen rivin, joka kertoo, mitä todella
tehtiin.

### Mitä kukin rivi sisältää

| Osa | Mitä se kertoo |
| :----- | :------------- |
| `line N` | missä, ykkösestä alkaen laskettuna, kuten muokkain sen näyttää; **puuttuu**, kun asia koskee koko tiedostoa |
| lause | mihin törmättiin, alla olevista kahdestatoista luokasta |
| `("…")` | tiedoston virheellinen teksti, kun luokka ei riitä; katkaistu 80 tavuun |
| loppu | mitä asialle tehtiin: `settled by the reader` tai `left as it stands` |

**Kahdella luokalla ei ole riviä**, ja ne ovat ne kaksi, jotka koskevat
merkistökoodausta: tavut punnittiin tai niiden merkki luettiin ennen kuin tiedoston yhtäkään
riviä oli olemassa. Niiden rivi alkaa siksi lauseella ilman edeltävää merkintää `line N`
— nimettävää paikkaa ei ole.

**Rivin loppu on tärkein osa.** `settled by the reader` tarkoittaa,
että lukija teki päätöksen ja että kirjoitettu tiedosto sisältää sen — esimerkiksi
puuttuva numerointi luodaan uudelleen. `left as it stands` tarkoittaa, että lukija ei koskenut
mihinkään, koska **vain sinä voit päättää**: kappale, joka päättyy ennen kuin se
alkaa, jää sellaiseksi kuin on.

### Lohkon rivinumero

Poikkeama, joka koskee **koko lohkoa**, kiinnitetään sen ajoitusriviin
eikä täsmälleen siihen riviin, joka sen aiheuttaa. Rivillä 5 oleva epäjohdonmukainen
numerointi ilmoitetaan siksi rivillä 6, joka on sitä seuraava
ajoitusrivi — virheellinen numero on merkinnässä `("…")`. Vain poikkeamat, jotka koskevat
**yhtä riviä** — lukukelvoton ajoitusrivi, teksti ennen ensimmäistä ajoitusriviä —,
ilmoitetaan omalla rivillään.

### Kaksitoista luokkaa

| Lause | Mikä sen aiheuttaa |
| :----- | :------------------ |
| `a line that fits nowhere` | rivi, joka ei kuulu mihinkään lohkoon |
| `a timing line that could not be read` | lukukelvoton ajoitusrivi |
| `a SubRip block without its number` | puuttuva numerointi, luodaan uudelleen kirjoitettaessa |
| `SubRip numbers that do not follow` | numerointi, joka hyppää |
| `text before the first timing line` | tekstiä ennen ensimmäistä ajoitusriviä |
| `a WebVTT block of an unknown kind` | tunnistamaton WebVTT-lohko |
| `declares an event column this tool cannot fill` | Sub Station Alphan tapahtumasarake, jota mikään täällä ei täytä |
| `counts in frames and states no rate; it was read at` | ruutuina laskeva tiedosto, joka ei ilmoita ruutunopeutta — käytetty nopeus on lisätietona |
| `carries no end times; each one was taken from the next start` | muoto ilman loppukohtia, TMPlayer tai LRC: **mikään taulukon loppukohta ei tule tiedostosta** |
| `more than one kind of line ending` | sekalaiset rivinvaihdot |
| `an encoding nothing declared` | merkistökoodaus ehdotettiin tavuja punnitsemalla, eikä se ole UTF-8 |
| `a byte order mark that contradicts the encoding asked for` | tiedostossa on merkki, ja `--encoding` nimesi toisen — merkki voittaa |

**Kolme keskimmäistä ovat vaiheen 9 luokat**, ja niistä kahdesta viimeisestä
kumpikaan ei puhu rivistä vaan koko tiedostosta: ne kertovat, että
näytettyä arvoa ei luettu. LRC-tiedostossa täytetty sarake `End` tai
MicroDVD-tiedoston kohta — tiedostossa ei ollut niistä mitään.

**Toiminto voi lisätä omansa**, joka ei ole lukemisen huomautus:
[`correct`](correct.md#lauseke-jota-ei-voi-käyttää) kertoo kappaleen kanssa lausekkeen,
joka luovutti. Siitä kerrotaan tasolla 1, ja tiedoston objektissa sillä on tunniste
`pattern-failed` — se ei koskaan ole tiedoston epäonnistuminen.

## Käännös

Käännöstiedosto käsitellään **kuten mikä tahansa tekstitystiedosto**:
`subedit-cli case --to sentence film.fr.srt` muuttaa sitä ilman mitään uutta. Se, mitä parittaminen lisää ja mistä
kolme valitsinta kertovat, on suppeaa: **tieto siitä, miten käännöksen rivit osuvat
päätiedoston kappaleisiin** — se on `inspect -t` —, ja **käännöksen muuttaminen niin, että se pysyy
asetettuna päätiedostonsa päälle**, samoin säännöin kuin ikkunassa.

| Valitsin | Missä | Arvo | Oletus |
| :----- | :---- | :----- | :----- |
| `-t`, `--translation-file` | `inspect`, `hearing-impaired`, `replace`, `case`, `italics`, `dialogue-dashes`, `correct`, `pair` | käännöstiedosto | — |
| `--document` | kuusi tekstialikomentoa, ei `inspect` eikä `pair` | `main` tai `translation` | `main` |
| `--align-method` | kuten `-t` | `position` tai `number` | `position` |

- **`position`** vertaa kunkin rivin keskikohtaa kappaleiden rajoihin: puuttuva rivi ei
  siirrä seuraavia. **`number`** lähettää rivin *n* kappaleeseen *n* katsomatta
  kohtia. Kohta on oletus samasta syystä kuin ikkunassa: puuttuva rivi
  keskellä siirtää numeron mukaan paritetun käännöksen koko loppuosaa.
- **`--document translation` vaatii valitsimen `-t`, ja `-t` vaatii valitsimen `--document translation`**
  tekstialikomennossa — toinen ilman toista on käyttövirhe, koodi `1`, koska luettu tiedosto, jota mikään toiminto
  ei käytä, on unohdus eikä valinta. Alikomennon `inspect` valitsin `-t` ei kaipaa valitsinta `--document`: se
  raportoi. **Myös `--align-method` ilman valitsinta `-t` on käyttövirhe.**
- **`-t` nimeää yhden tiedoston, joten se on järkevä vain yhdellä syötteellä**: usealla syötteellä tai
  valitsimella `--recursive` otetulla hakemistolla se hylätään kuten `--output`, **ennen kuin yhtäkään tiedostoa
  luetaan**. Aja erälle yksi käynnistys kutakin paria kohden.
- **Päätiedoston kanssa identtinen käännöstiedosto hylätään** — kirjoitettiinpa sen
  polku miten tahansa: `<translation>: the file is already open as the main document`, koodi `2`.
- **Vain kohteena oleva asiakirja kirjoitetaan**, **sen omaan polkuun ja sen omassa muodossa**: käännöstiedoston
  muoto, merkistökoodaus ja rivinvaihdot. `--output` nimeää käännöksen
  tiedoston, `--output-dir` sijoittaa sinne sen perusnimen, `--in-place` kirjoittaa sen uudelleen; **päätiedostoon
  ei koskaan kosketa**. Raportin polut ovat käännöksen polkuja.
- **Kappaleet, jotka parittaminen synnyttää** — rivi, johon mikään ei osunut —, kirjoitetaan
  käännökseen **niiden omilla kohdilla** menetelmästä riippumatta; **kappale, jota kukaan ei
  kääntänyt, kirjoitetaan lohkona ilman tekstiä**, joka luetaan ilman poikkeamaa.
- **`--encoding` ja `--frame-rate` koskevat molempia tiedostoja.**
- **Kohtia** käsittelevillä alikomennoilla — `shift`, `transform`, `framerate`, `snap`, `adjust` — ei ole
  näitä valitsimia: anna niille molemmat tiedostot kuten erälle,
  `subedit-cli shift --by 2 film.srt film.fr.srt --output-dir out/`.
- **Päätiedoston mukaan uudelleen ajoitetun käännöksen kirjoittaminen** — ainoa asia, jonka parittaminen tekee ja yksittäinen tiedosto
  ei — on alikomennon [`pair`](pair.md) tehtävä, eikä sillä ole valitsinta `--document` eikä valitsinta `--recursive`.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\n[Un oiseau] Bonjour.\n\n2\n00:00:03,000 --> 00:00:04,000\nAu revoir.\n\n' > film.fr.srt; subedit-cli hearing-impaired film.srt -t film.fr.srt --document translation --output propre.fr.srt; cat propre.fr.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\n[Un oiseau] Bonjour.\n\n2\n00:00:03,000 --> 00:00:04,000\nAu revoir.\n\n' > film.fr.srt; subedit-cli hearing-impaired film.srt -t film.fr.srt --document translation --output propre.fr.srt; cat propre.fr.srt
film.fr.srt: 1 subtitle cleaned, 0 removed -> propre.fr.srt
1
00:00:01,000 --> 00:00:02,000
Bonjour.

2
00:00:03,000 --> 00:00:04,000
Au revoir.
```

Valitsimella `--format json` kunkin tiedoston objektissa on myös avain `alignment`, joka kertoo, miten
käännös asetettiin — `{"file", "method", "attached", "born", "untranslated", "out_of_order"}`, kuten
alikomennon [`inspect`](inspect.md#json-muodossa) avain `translation` — ja **valitsimella `--dry-run`** `changes` luettelee
käännöksen kappaleet, kunkin merkinnällä `"document":"translation"`.

## Useita tiedostoja

Jokainen alikomento hyväksyy useita polkuja, paitsi [`append`](append.md), [`split-file`](split-file.md) ja
[`pair`](pair.md), joiden argumenttien määrä kerrotaan niiden sivulla. Kukin käsitellään
erikseen: yhden epäonnistuminen ei keskeytä muita, ja epäonnistumisista
ilmoitetaan nimeämällä tiedosto ja syy.

**Hakemisto edustaa sen sisältämiä tiedostoja valitsimella `--recursive` (`-r`)**:
ilman tätä valitsinta hakemiston antaminen on käyttövirhe, koodi `1`, eikä mitään
käsitellä — sen hylkääminen on parempi kuin olla käsittelemättä mitään ja palauttaa `0`:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?" -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?"
films: is a directory: use --recursive
code=1
```

Läpikäynti, valitsimen `--output-dir` alla säilytettävä puu ja se, mitä jätetään
sivuun, kuvataan sivulla [Hakemistopuun käsittely](lots.md).

## Man-sivut

Asennus sijoittaa **yhden man-sivun kutakin alikomentoa kohden** sivun `subedit-cli(1)` viereen:
`subedit-cli-shift(1)`, `subedit-cli-convert(1)` ja muut, yksi kutakin
[alikomentotaulukon](#alikomennot) riviä kohden. `subedit-cli(1)` sisältää tämän luvun — yleiset valitsimet,
tulosteet, paluuarvot — ja kukin muu sivu sisältää alikomentonsa luvun
esimerkkeineen. Ikkunan sivu, `subedit-gui(1)`, on niiden vieressä.

```console
$ man subedit-cli
$ man subedit-cli-shift
```

**Sivuja ei kirjoiteta erillään tästä käyttöoppaasta**: ne tuotetaan siitä, kun
ohjelma käännetään, joten ne sanovat saman kuin se eivätkä voi erkaantua siitä. Kunkin sivun
osio TIEDOSTOT nimeää hakemiston, johon käyttöopas on asennettu.

| Sivut | Minne ne asennetaan |
| :---- | :----------------------- |
| englanti | `share/man/man1` |
| kukin asennettu kieli — katalaani, tšekki, saksa, espanja, suomi, ranska, galicia, heprea, unkari, interlingue, islanti, italia, hollanti, puola, portugali, brasilianportugali, venäjä, serbia, turkki, yksinkertaistettu kiina | `share/man/<language>/man1` — `share/man/fr/man1` |

**`man` valitsee kielen kieliasetuksen mukaan**, kuten ohjelma: ranskalaisella kieliasetuksella
`man subedit-cli` näyttää ranskankielisen sivun, ja kieli, jonka sivuja ei ole asennettu, saa
englanninkieliset. `-L` pyytää kieltä kieliasetuksesta riippumatta:

```console
$ man -L fr subedit-cli-shift
```

## Paluuarvot

| Koodi | Merkitys |
| :--- | :------------ |
| `0` | kaikki onnistui |
| `1` | käyttövirhe — tuntematon valitsin, virheellinen arvo, kielletty yhdistelmä |
| `2` | yhtään tiedostoa ei voitu käsitellä |
| `3` | osa tiedostoista käsiteltiin, osa ei |

`2` ja `3` erotetaan toisistaan, jotta skripti tietää, miten toimia, lukematta tulostetta uudelleen:
”mikään ei toiminut” ja ”yksi puuttuu” eivät vaadi samaa reaktiota.

Käyttövirhe havaitaan **ennen mitään käsittelyä**: se ei koskaan jätä
erää puolittain käsitellyksi.

## Virheet

| Mikä sen aiheuttaa | Mitä virhetulosteeseen kirjoitetaan |
| :------------------ | :--------------------------------------- |
| tuntematon valitsin tai alikomento | virheellinen nimi ja viittaus valitsimeen `--help` |
| `--quiet` ja `-v` yhdessä | `--quiet and -v ask for opposite things; give one or the other` |
| `--config` ja `--no-config` yhdessä | `--config excludes --no-config` |
| `--config` tyhjällä nimellä | `--config needs a file name` |
| puuttuva tiedosto | `<path>: does not exist` |
| lukukelvoton tiedosto | `<path>: cannot be opened: permission denied` |
| merkistökoodaus, joka kirjoittaa oman merkkinsä | `"<name>" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do` |
| tavut, joita ei voi purkaa | `<path>: cannot be decoded in the chosen encoding` |
| tunnistamaton muoto | `<path>: is in no format this tool knows` |
| ei mitään kappaleelta näyttävää | `<path>: holds nothing recognisable as a subtitle` |
| `--document translation` ilman valitsinta `-t` | `--document translation needs the translation file: use -t` |
| `-t` ilman valitsinta `--document translation` | `-t names a translation to change: use --document translation` |
| `--align-method` ilman valitsinta `-t` | `--align-method needs a translation file: use -t` |
| `-t` usealla syötteellä tai hakemistolla | `-t names one file but several inputs were given: use one invocation per pair` |
| päätiedoston kanssa identtinen käännös | `<translation>: the file is already open as the main document` |
