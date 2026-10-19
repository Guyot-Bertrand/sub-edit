# `inspect`

<!-- machine-translated -->
> Tämä sivu on konekäännös, eikä kukaan tämän kielen lukija ole vielä tarkistanut sitä.

```
subedit-cli inspect [--frame-rate RATE] [--recursive] [--detail]
                   [-t FILE] [--align-method position|number] <file>...
```

Kertoo, mistä kukin tiedosto koostuu. **Ei muuta mitään eikä kirjoita
tiedostoa.**

<!-- example: subedit-cli inspect --help -->
```console
$ subedit-cli inspect --help
Report what a subtitle file is made of
Usage: subedit-cli inspect [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to report on

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --detail                    List every anomaly, subtitle by subtitle, instead of counting them
  --frame-rate RATE           Frame rate of a file counted in frames: 25, 23.976
  -t,--translation-file FILE  Translation file to lay over the subtitle file, for a single input
  --align-method position|number
                              How the lines of the translation find their subtitles
```

## Argumentit

| Argumentti | Pakollinen | Arvo | Oletus |
| :------- | :----- | :----- | :----- |
| `<file>...` | kyllä | yksi tai useampi tekstitystiedoston polku | — |
| `--recursive`, `-r` | ei | lippu | pois — katso [Hakemistopuun käsittely](lots.md) |
| `--detail` | ei | lippu: nimeää jokaisen poikkeaman sen sijaan, että laskisi ne — katso [Mitä `anomalies` kertoo](#mitä-anomalies-kertoo) | pois |

Polun puuttuminen on käyttövirhe, joten koodi on `1`.

## Valitsimet

| Valitsin | Pakollinen | Arvo | Oletus |
| :----- | :------ | :----- | :----- |
| `--frame-rate` | ei | nopeus ruutuina sekunnissa: `25`, `23.976`, `29.97` | 23,976 |

**Se palvelee vain ruutuina laskettuja tiedostoja**, eli MicroDVD:tä,
ainoaa yhdeksästä. MicroDVD-tiedosto sisältää pelkkiä ruutujen numeroita; nopeus,
jolla ne luetaan, ratkaisee **raportin jokaisen kohdan**,
alkaen kentästä `span`. Muualla annettuna se ei muuta mitään eikä ole
virhe.

Arvo, joka ei nimeä mitään, on käyttövirhe, joten koodi on `1`:

<!-- example: subedit-cli inspect --frame-rate douze film.sub; echo $? -->
```console
$ subedit-cli inspect --frame-rate douze film.sub; echo $?
--frame-rate: "douze" is not a frame rate: expected frames per second, like 25 or 23.976
1
```

Se on sanasta sanaan sama valitsin kuin alikomennolla [`convert`](convert.md), koska
kyse on samasta nopeudesta. `convert` käyttää sitä myös **kirjoittamiseen**.

## Käännös: `-t`

| Valitsin | Pakollinen | Arvo | Oletus |
| :----- | :------ | :----- | :----- |
| `-t`, `--translation-file` | ei | käännöstiedosto, **yhdelle** syötteelle | — |
| `--align-method` | ei, ja vain valitsimen `-t` kanssa | `position` tai `number` | `position` |

Valitsimella `-t` käännös asetetaan tiedoston päälle **samoin kuin ikkuna tekee sitä avatessaan** — katso
[Käännös](invocation.md#käännös) — ja raportti kertoo, **miten sen rivit löysivät
kappaleensa**. Skripti tietää näin, onko käännös kohdistettu, ennen kuin korjaa sitä.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour.\n\n' > film.fr.srt; subedit-cli --quiet inspect -t film.fr.srt film.srt | tail -3 -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour.\n\n' > film.fr.srt; subedit-cli --quiet inspect -t film.fr.srt film.srt | tail -3
  anomalies: none
  translation file: film.fr.srt, matched by position
  translation: 1 line attached; 1 subtitle left without a translation
```

Ensimmäinen lisätty rivi nimeää tiedoston ja menetelmän; toinen on **lause, jonka ikkuna
sanoo** avatessaan käännöksen, ja kukin osa mainitaan vain, kun se ei ole nolla:

| Osa | Mitä se laskee |
| :----- | :---------------- |
| `N lines attached` | käännöksen rivit, jotka löysivät kappaleen päätiedostosta |
| `N subtitles born of a line` | rivit, jotka eivät löytäneet kappaletta ja synnyttivät oman kappaleen omiin kohtiinsa |
| `N subtitles left without a translation` | päätiedoston kappaleet, joihin mikään rivi ei ulottunut |
| `N lines out of order` | rivit, jotka tulivat tiedostossa myöhemmän rivin jälkeen |

Käännös, jossa ei ole yhtään riviä, sanoo `translation: the file holds no line`. **Itse tiedoston
raportti pysyy pelkän tiedoston raporttina**: rivistä syntynyttä kappaletta ei lasketa
kenttään `subtitles`.

Valitsimen `-t` virheet ovat lukemisen virheitä käännöksen polun kera, sekä tämä:
`<translation>: the file is already open as the main document`, kun annettu tiedosto on päätiedosto.
`--align-method` ilman valitsinta `-t` on käyttövirhe, samoin `-t` usean syötteen kanssa — katso
[Käännös](invocation.md#käännös).

## Tuloste

Raportti menee **vakiotulosteeseen** — se on komennon tulos, ja
se kirjoitetaan jokaisella tasolla, myös valitsimella `--quiet`. Yksi lohko tiedostoa kohden,
polun edeltämänä:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,500\nLe canot dérive.\n\n2\n00:00:04,000 --> 00:00:06,200\nPersonne n a vu la côte.\n' > exemple.srt; subedit-cli --quiet inspect exemple.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,500\nLe canot dérive.\n\n2\n00:00:04,000 --> 00:00:06,200\nPersonne n a vu la côte.\n' > exemple.srt; subedit-cli --quiet inspect exemple.srt
exemple.srt
  format: SubRip
  encoding: UTF-8, detected
  byte order mark: absent
  line endings: LF
  subtitles: 2
  span: 00:00:01.000 -> 00:00:06.200
  frame rate grid: none (too few subtitles to tell)
  anomalies: none
```

| Kenttä | Mitä se kertoo |
| :---- | :----------- |
| `format` | jokin muodoista, jotka `--to` hyväksyy, kokonaan kirjoitettuna: `SubRip`, `WebVTT`, `SubViewer 2`, `Sub Station Alpha`, `Advanced SSA`, `MicroDVD`, `MPL2`, `TMPlayer`, `LRC` |
| `encoding` | luettu merkistökoodaus ja sen perässä **vastauksen lähde**: `, from its byte order mark`, jos tiedosto ilmoittaa sen, `, as asked for`, jos `--encoding` pakotti sen, `, detected`, jos se punnittiin |
| `byte order mark` | `present` tai `absent` |
| `line endings` | `LF`, `CRLF` tai `CR` ja sen perässä `, mixed from line N`, jos tiedosto sekoittaa niitä |
| `subtitles` | luettujen kappaleiden määrä |
| `span` | varhaisimmasta alusta myöhäisimpään loppuun, `HH:MM:SS.mmm` |
| `frame rate grid` | ruutunopeus, jonka mukaan kohdat laskettiin, **päätelty** — katso alla |
| `frame rate` | ruutuina lasketulle tiedostolle nopeus, jolla se luettiin, ja **sen lähde** — korvaa edellisen rivin, katso alla |
| `anomalies` | `none` samalla rivillä tai se, mikä on vialla: **yksi laji riviä kohden laskureineen**, sisennettynä rivin `anomalies:` alle; valitsimella `--detail` **yksi poikkeama riviä kohden**, kappale kappaleelta |

**`span` ei ole ”ensimmäisestä viimeiseen”** vaan varhaisimmasta myöhäisimpään:
tiedostossa, jonka järjestys on rikki, nämä eroavat, ja vain jälkimmäinen kertoo
totuuden siitä, mitä tiedosto kattaa.

**`anomalies` laskee valitsimella `--detail` kappaleet ykkösestä**, kuten tiedosto
ne numeroi — eikä tiedoston rivejä. Kappaleen numero säilyy
muutoksessa; rivinumero ei. Katso alla.

## Mitä `frame rate grid` kertoo

**Tekstitystiedosto ei ilmoita ruutunopeuttaan.** SubRipissä ei ole
otsaketta, WebVTT:n otsake on vapaata tekstiä. Tämä rivi ei siis lue mitään:
se **päättelee** mittaamalla, osuvatko kohdat ruutujen ruudukolle.

Mitataan **alkukohtia**. Missä ruudukko on olemassa, ne ovat sillä
poikkeuksetta, kun taas loppukohdat poikkeavat siitä usein — loppu
lasketaan usein lukunopeussäännöllä sen sijaan, että se asetettaisiin
ruudulle.

Ensimmäinen rivi kertoo aina saman: valitun nopeuden, tuloksen
ja sitä tukevan keskittymän.

| Tulos | Mitä se tarkoittaa |
| :------ | :----------------- |
| `clean` | kohdat ovat ruudukolla |
| `partial` | osa niistä on, loput eivät |
| `none` | mikään ehdokas ei selitä mitään |

Tuloksella `none` on **kaksi syytä, eivätkä ne ole sama vastaus**: joko mikään
ruudukko ei sovi — rivi antaa silloin parhaan ehdokkaan ja sen pistemäärän,
jotta näkee, kuinka paljon se jää vajaaksi — tai tiedostossa on liian vähän
kappaleita, jotta siitä voisi sanoa mitään. Kaksi alkukohtaa näyttää aina
täydellisesti kohdistetuilta; se ei todista mitään.

**Säännöllinen tiedosto epästandardilla nopeudella antaa tuloksen `none`, ja se on
tarkoituksellista.** Ehdokkaiden joukko on suljettu — muunnoksen tarjoamat
kahdeksan nopeutta — ja niistä vähiten väärän ilmoittaminen olisi
väärä vastaus siellä, missä ”en tiedä” on oikea.

### Seuraavat rivit, kun niillä on sanottavaa

| Rivi | Milloin se näkyy |
| :---- | :------------------ |
| `grid offset` | kohdat ovat ruudukolla **vakiota lukuun ottamatta**: tiedostoa on siirretty, ja tässä on siirron määrä |
| `also fits` | toinen ehdokas sopii yhtä hyvin, koska se on valitun kokonaislukumonikerta |
| `too short a span to separate` | tiedoston kattama aika ei riitä erottamaan valittua nopeutta nimetyistä |
| `off the grid` | kuinka monta alkukohtaa poikkeaa ruudukolta ja kuinka monessa **jaksossa** |

**`also fits` ei ole epäröintiä.** Ruudukko, jossa on 25 ruutua sekunnissa, *sisältyy*
50 ruudun ruudukkoon: nopeudelle 25 ajoitettu tiedosto sopii siis molempiin.
Päinvastoin ei päde — todellinen 50 ei sovi ruudukkoon 25 — joten valitaan alin,
ja toinen nimetään sen sijaan, että siitä vaiettaisiin. Ainoat
asianomaiset parit ovat 25 ja 50, 29,97 ja 59,94 sekä 30 ja 60.

**`too short a span to separate` suojaa liian varmalta vastaukselta.** 23,976
ja 24 erkanevat toisistaan millisekunnin elokuvan sekuntia kohden: pitkässä elokuvassa ne
erottuvat kiistatta, kymmenessä sekunnissa niitä ei voi erottaa. Rivi kertoo,
mikä niistä on edelleen mahdollinen.

**`off the grid` erottaa kaksi tarinaa, jotka pelkkä prosenttiluku sekoittaa.**
Monet yhden alkukohdan jaksot ovat käsin korjattuja kohtia,
yksi kerrallaan. Muutama pitkä jakso on uudelleen ajoitettu osuus tai kahdesta muusta
koottu tiedosto. Jaksojen määrä erottaa ne.

### Esimerkki

Kaksitoista riviä, jotka on asetettu 25 ruudun sekuntinopeuden ruudukolle — siis
40 millisekunnin monikertoihin — lähes yhdeksän minuutin ajalle:

<!-- example: k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > grille.srt; subedit-cli --quiet inspect grille.srt | tail -3 -->
```console
$ k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > grille.srt; subedit-cli --quiet inspect grille.srt | tail -3
  frame rate grid: 25 fps, clean (100.0%)
  also fits: 50 fps, of which this rate is a whole divisor
  anomalies: none
```

Esimerkki tekee oman tiedostonsa sen sijaan, että lukisi sellaisen arkistosta, ja se on
tarkoituksellista: sen voi toistaa sellaisenaan, ja silmukka näyttää, mikä ruudukko on,
paremmin kuin lause — kohtia, jotka ovat ruudun keston monikertoja.

## Mitä `frame rate` kertoo ja miksi se ei ole ruudukko

**Vain yksi yhdeksästä muodosta laskee ruutuina**: MicroDVD, jonka jokainen rivi
sisältää kaksi ruudun numeroa eikä mitään muuta. Tällaisessa tiedostossa `inspect`
ei kirjoita riviä `frame rate grid` vaan rivin `frame rate`, eikä näitä kahta riviä
koskaan näytetä yhdessä.

**Syy mahtuu yhteen lauseeseen.** MicroDVD-tiedoston kohdat on
laskettu sen ruuduista *lähtien* tietyllä nopeudella; niiden päätteleminen
ruudukosta antaisi takaisin juuri sen nopeuden, joka on juuri annettu. Se olisi
annettu luku mittaukseksi puettuna.

Sen sijaan kirjoitetaan **lukunopeus ja sen lähde** —
samassa muodossa kuin rivi `encoding`:

| Mitä rivi kertoo | Milloin |
| :------------------ | :---- |
| `frame rate: 25 fps, as asked for` | `--frame-rate` pakotti sen |
| `frame rate: 24000/1001 fps, assumed` | kukaan ei kertonut sitä; se on oletus |

**Kaksi vastausta eikä kolme**, kun rivillä `encoding` on kolme: MicroDVD-tiedosto
ei ilmoita mitään. Siinä on koko ongelma — nopeus tulee välttämättä
ulkopuolelta, ja raportti kertoo, kummalta kahdesta ulkopuolesta.

**Nopeus ratkaisee koko muun raportin.** `span` kulkee sen kautta, ja saman tiedoston kaksi
lukukertaa kahdella nopeudella antaa kaksi aikajännettä:

<!-- example: printf '{25}{75}Première.\n{100}{150}Deuxième.\n{200}{260}Troisième.\n' > images.sub; subedit-cli --quiet inspect images.sub | tail -3; subedit-cli --quiet inspect --frame-rate 25 images.sub | tail -3 -->
```console
$ printf '{25}{75}Première.\n{100}{150}Deuxième.\n{200}{260}Troisième.\n' > images.sub; subedit-cli --quiet inspect images.sub | tail -3; subedit-cli --quiet inspect --frame-rate 25 images.sub | tail -3
  span: 00:00:01.043 -> 00:00:10.844
  frame rate: 24000/1001 fps, assumed
  anomalies: none
  span: 00:00:01.000 -> 00:00:10.400
  frame rate: 25 fps, as asked for
  anomalies: none
```

## Mitä `anomalies` kertoo

Asiakirjassa voi olla vialla kolme asiaa, ja kukin korjataan eri tavalla.
**Oletuksena `inspect` laskee ne**, yksi rivi lajia kohden, taulukon järjestyksessä:
huonosti tehdyssä tiedostossa niitä on satoja, ja tieto siitä, kuinka monta kutakin lajia on,
kertoo, mitä pitää ajaa. **`--detail` nimeää ne yksitellen.**

| Mitä kirjoitetaan | Valitsimella `--detail` | Mitä se tarkoittaa |
| :--------------- | :-------------- | :------------------ |
| `subtitle ends before it starts: N` | `subtitle N ends before it starts` | loppu tulee ennen alkua |
| `subtitle starts before the previous one ends: N` | `subtitle N starts before the previous one ends` | se menee edellisen päälle |
| `subtitle starts before the previous one starts: N` | `subtitle N starts before the previous one starts` | se rikkoo tiedoston järjestyksen — [`sort`](sort.md) palauttaa sen |

Puuttuvaa lajia ei kirjoiteta. Vasemmanpuoleisessa sarakkeessa `N` on kappaleiden
määrä; seuraavassa kappaleen numero.

**Sama kappale voi esiintyä kahdesti**, ja se on tarkoituksellista: kappale, joka
alkaa ennen kuin edellinen on alkanut, alkaa myös ennen kuin edellinen on
päättynyt. Ensimmäinen asia ratkaistaan siirtämällä kappaletta, toinen
muuttamalla kestoa.

**Nimetty kappale on se, joka on väärässä paikassa**, ei se, johon nähden se on
väärässä paikassa. Alkukohdilla `0`, `4 s`, `2 s`, `3 s` kolmas rikkoo järjestyksen, ja
neljäs seuraa kyllä kolmatta eikä siksi tule nimetyksi — sille ei ole mitään
tehtävää.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect desordre.srt | tail -3 -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect desordre.srt | tail -3
  anomalies:
    subtitle starts before the previous one ends: 1
    subtitle starts before the previous one starts: 1
```

Ja valitsimella `--detail` nimetty kappale:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect --detail desordre.srt | tail -3 -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect --detail desordre.srt | tail -3
  anomalies:
    subtitle 3 starts before the previous one ends
    subtitle 3 starts before the previous one starts
```

**Yksi rivi poikkeamaa kohden valitsimella `--detail`**, koska sataa samalle riville
ahdettua havaintoa ei voi lukea eikä hakea. Virheetön asiakirja säilyttää
ainoan rivinsä, `anomalies: none`, jota skripti etsii. **Valitsimen
`--format json` tietue luettelee aina jokaisen poikkeaman**: skripti lukee yksityiskohdat, ja
`--detail` muuttaa vain tekstiä.

**Se ei ole huomautus luettaessa.** Huomautukset kertovat, mihin
lukeminen törmäsi, ja osoittavat **tiedoston riviin**; poikkeamat kertovat,
mitä asiakirja *on*, ja osoittavat **kappaleeseen**. Tiedosto, jonka järjestys
on rikki, ei sen vuoksi ole virheellisesti muodostettu: lukemisella ei ole siitä mitään sanottavaa.

## JSON-muodossa

Valitsimella [`--format json`](invocation.md#skriptin-luettava-tuloste) `inspect` kirjoittaa **yhden
olion tiedostoa kohden** yllä olevien lohkojen sijaan; kerronta pysyy virhetulosteessa.
Avaimet kuvaavat sitä, mitä teksti sanoo, eikä **yhtäkään tarvitse lukea lauseesta**:

| Avain | Sisältö |
| :-- | :------ |
| `format` | muodon tunniste, sama jonka `convert --to` hyväksyy: `srt`, `vtt`… |
| `encoding` | `{"charset", "origin", "byte_order_mark"}` — `origin` on `byte-order-mark`, `asked` tai `detected` |
| `line_endings` | `{"kind", "mixed_from_line"}` — `lf`, `crlf` tai `cr`; rivi, jolla toinen laji ilmestyy, tai `null` |
| `subtitles` | kappaleiden määrä |
| `span_ms` | `{"start", "end"}` — ensimmäinen alku ja viimeinen loppu millisekunteina |
| `frame_rate` | ruutuina lasketulle tiedostolle: `{"rate", "origin"}` (`asked` tai `assumed`); muuten `null` |
| `grid` | muille: päätelty ruudukko, tai `null` ruutuina lasketulle tiedostolle |
| `anomalies` | taulukko olioita `{"subtitle", "kind"}` — `end-before-start`, `overlapping-subtitles`, `out-of-order` |
| `translation` | **vain valitsimella `-t`**: `{"file", "method", "attached", "born", "untranslated", "out_of_order"}` — `method` on `position` tai `number`, ja neljä laskuria ovat yllä olevan lauseen laskurit |
| `warnings` | mihin lukeminen törmäsi, jokaisella tasolla |

`grid` sisältää kentät `verdict` (`clean`, `partial` tai `silent`), `enough_starts`, `rate` (merkkijono,
tai `null`, kun mitään ei nimetä), `concentration_permille` (tuhannesosina, `null` ilman riittävästi
alkukohtia), `offset_ms`, `also_fits` (nopeus, jonka kokonaislukujakaja tämä on, tai
`null`), `not_separated` (nopeudet, joita liian lyhyt aikajänne ei erota), `strays`
(ruudukon ulkopuoliset alkukohdat) ja `starts`.

<!-- example: printf '{25}{50}Un.\n{75}{100}Deux.\n' > f.sub; subedit-cli --format json inspect --frame-rate 25 f.sub 2>/dev/null -->
```console
$ printf '{25}{50}Un.\n{75}{100}Deux.\n' > f.sub; subedit-cli --format json inspect --frame-rate 25 f.sub 2>/dev/null
{"schema":1,"command":"inspect","file":"f.sub","ok":true,"format":"microdvd","encoding":{"charset":"UTF-8","origin":"detected","byte_order_mark":false},"line_endings":{"kind":"lf","mixed_from_line":null},"subtitles":2,"span_ms":{"start":1000,"end":4000},"frame_rate":{"rate":"25","origin":"asked"},"grid":null,"anomalies":[],"warnings":[]}
```

## Kerronta

Virhetulosteeseen pyydetyn tason mukaan:

| Taso | Mitä lisätään |
| :----- | :-------------- |
| 1 | `<path>: N subtitles` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings` |
| 3 | `<path>: N bytes read`, `<path>: N diagnostic(s) while reading`, sitten **jokainen niistä** — katso [Käynnistys](invocation.md#huomautukset-luettaessa) |

## Paluuarvot

Työkalun yleiset: `0`, jos jokainen tiedosto luettiin, `2`, jos yhtäkään ei, `3`, jos
vain osa, `1` käyttövirheessä.

## Virheet

Lukemisen virheet — tiedoston ja, kun `-t` on annettu, käännöksen —, sekä
valitsimen `-t` virheet: katso yllä. `inspect` ei kirjoita mitään, mikä voitaisiin hylätä.

| Mikä sen aiheuttaa | Viesti |
| :------------------ | :------ |
| polkua ei annettu | `files is required` ja sen perässä viittaus valitsimeen `--help` |
| puuttuva tiedosto | `<path>: does not exist` |
| lukukelvoton tiedosto | `<path>: cannot be opened: permission denied` |
| tavut, joita ei voi purkaa | `<path>: cannot be decoded in the chosen encoding` |
| tunnistamaton muoto | `<path>: is in no format this tool knows` |
| ei mitään kappaleelta näyttävää | `<path>: holds nothing recognisable as a subtitle` |

Viestit ovat samat kaikille seitsemälletoista alikomennolle: avaamisen resepti
on kirjoitettu kerran, ytimeen.

## Epäonnistuva esimerkki

Tyhjä tiedosto ei vastaa mitään muotoa. Sitä ei raportoida tyhjänä:
se hylätään ja nimetään.

<!-- example: : > vide.srt; subedit-cli inspect vide.srt; echo "code=$?" -->
```console
$ : > vide.srt; subedit-cli inspect vide.srt; echo "code=$?"
vide.srt: is in no format this tool knows
code=2
```
