# `correct`

<!-- machine-translated -->
> Tämä sivu on konekäännös, eikä kukaan tämän kielen lukija ole vielä tarkistanut sitä.

```
subedit-cli correct --tasks LIST --code CODE
                    [--classes human|ocr|human,ocr]
                    [--enable NAME]... [--disable NAME]...
                    [--language CODE]
                    [--max-length N] [--max-lines N]
                    [--skip-length N|off] [--skip-lines N|off]
                    [--keep-blank-subtitles | --remove-blank-subtitles]
                    [--range N-M|N-]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    <file>...
```

Korjaa tekstit **Gaupolin lausekkeilla** kuten ikkunan
korjausavustaja [`Correct Texts…`](../subedit-gui/correct-texts.md) — samat lausekkeet,
sama moottori, samat laskurit. Tehtäviä on kuusi: **kuulovammaisille tarkoitetut
maininnat**, **sanojen yhdistäminen** ja **jakaminen**, **yleiset virheet**,
**isot alkukirjaimet** ja **rivinvaihto**.

<!-- example: subedit-cli correct --help -->
```console
$ subedit-cli correct --help
Correct the texts with the patterns of the correction assistant
Usage: subedit-cli correct [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to correct

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --tasks LIST                The tasks to run, in a list: none is chosen beforehand
  --code CODE                 The cascade of patterns the tasks read: Zyyy, Latn, Latn-en, Latn-en-US
  --classes human|ocr|human,ocr
                              The classes of common errors to apply: both by default
  --enable NAME               Switch a pattern on, by its English name or type:name; repeatable
  --disable NAME              Switch a pattern off, by its English name or type:name; repeatable
  --language CODE             The dictionary join-words and split-words read: fr_FR, en_US, en...
  --max-length N              Longest line of line-break, in characters; no default, Gaupol's is in ems
  --max-lines N               Most lines of line-break: 3, as Gaupol's
  --skip-length N|off         Leave alone a subtitle whose longest line is within this, or off: --max-length
  --skip-lines N|off          Leave alone a subtitle whose line count is within this, or off: --max-lines
  --keep-blank-subtitles      Leave the subtitles the correction empties, empty, instead of removing them
  --remove-blank-subtitles    Remove the subtitles the correction empties, whatever the settings file says
  --range N-M|N-              Act only on subtitles N to M, or N to the end
  --document main|translation The document to change: the main one, or the translation given by -t
  -t,--translation-file FILE  Translation file to lay over the subtitle file, for a single input
  --align-method position|number
                              How the lines of the translation find their subtitles
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Mitä valitaan ja mitä ei

**Mitään ei ole valittu etukäteen.** Ikkuna avaa ruutunsa sellaisina kuin käyttäjä ne jätti;
komentorivi ottaa tehtävänsä, koodinsa, rivipituutensa ja kielensä omalta riviltään,
ei koskaan asetustiedostosta: ikkunassa joskus valittuja tehtäviä, siellä kytkettyjä
lausekeruutuja (`correction.activations`) tai oikoluvun korvausluetteloa ei
lueta, eikä sille puolelle kirjoiteta mitään.

**Neljä asetusta tulee oletuksena asetustiedostosta** — arvot, jotka `Correct Texts…`
muistaa tai jotka [`config set`](config.md) kirjoitti. **Komentorivin valitsin voittaa aina**, sitten
tiedosto ja sitten sisäänrakennettu oletusarvo; katso [Asetustiedosto](invocation.md#asetustiedosto):
missä se on, `--config` ja `--no-config`.

| Avain | Mitä se antaa | Sen ohittava valitsin |
| :-- | :--------------- | :----------------------- |
| `correction.human`, `correction.ocr` | sovellettavat yleisten virheiden luokat | `--classes` |
| `correction.remove-blank` | poistetaanko kappaleet, jotka korjaus tyhjentää | `--keep-blank-subtitles`, `--remove-blank-subtitles` |
| `correction.line-break.max-lines` | tehtävän `line-break` suurin rivimäärä | `--max-lines` |

- **Jokaisella neljästä on valitsin, joka kumoaa sen**, sanoipa tiedosto mitä tahansa: `--classes human,ocr`
  soveltaa molempia luokkia, `--remove-blank-subtitles` poistaa tyhjennetyt kappaleet, `--max-lines 3`
  palauttaa Gaupolin rivimäärän. `--keep-blank-subtitles` ja `--remove-blank-subtitles` yhdessä ovat
  käyttövirhe.
- **Jos tiedostossa molemmat luokat ovat pois päältä, se ohitetaan ja kerrotaan** tasolla 1 —
  `settings: correction.human and correction.ocr are both false, ignored` —: `--classes` ei voisi kirjoittaa sitä, eikä se jättäisi
  tehtävälle common-errors mitään tehtävää. Sen sijaan sovelletaan molempia luokkia.
- **Rivimäärää, joka ei ole väliltä 1–1 000, ei lueta tiedostosta lainkaan**: tiedoston lukija
  hylkää sen — `settings: correction.line-break.max-lines: cannot read 'N', default kept` tasolla
  1 — ja käytetään sisäänrakennettua arvoa 3.

Tämän sivun esimerkit ajetaan valitsimella `--no-config`, jotta ne kertovat, mitä sisäänrakennetut
oletusarvot tekevät, riippumatta niitä ajavan koneen asetuksista.

| Valitsin | Pakollinen | Arvot | Oletus |
| :----- | :----- | :------ | :----- |
| `<file>...` | kyllä | yksi tai useampi polku | — |
| `--tasks` | **kyllä** | pilkuin erotettu luettelo: `mentions`, `join-words`, `split-words`, `common-errors`, `capitalization`, `line-break` | ei tehtävää |
| `--code` | **kyllä** lausekkeita lukevan tehtävän kanssa, muuten **ei sovellu** | `Script[-language[-COUNTRY]]`: `Zyyy`, `Latn`, `Latn-en`, `Latn-en-US` | — |
| `--language` | **kyllä** tehtävien `join-words` tai `split-words` kanssa, muuten **ei sovellu** | kielikoodi: `fr`, `fr_FR`, `en_US` | — |
| `--classes` | ei | `human`, `ocr` tai `human,ocr` | molemmat — tai mitä asetustiedosto sanoo |
| `--enable`, `--disable` | ei, toistettavissa | lausekkeen englanninkielinen nimi tai `type:name` | toimitettujen tiedostojen `.conf` asetukset |
| `--max-length` | **kyllä** tehtävän `line-break` kanssa | nollaa suurempi määrä **merkkejä** | **ei ole** — katso alla |
| `--max-lines` | ei | nollaa suurempi kokonainen rivimäärä | 3 — tai mitä asetustiedosto sanoo |
| `--skip-length`, `--skip-lines` | ei | luku tai `off` | samat rajat kuin valitsimilla `--max-length` ja `--max-lines` |
| `--keep-blank-subtitles` | ei | lippu: tyhjennetyt kappaleet jäävät tyhjinä | tyhjennetyt kappaleet poistetaan — ellei asetustiedosto sano muuta |
| `--remove-blank-subtitles` | ei | lippu: tyhjennetyt kappaleet poistetaan, sanoipa asetustiedosto mitä tahansa; hylätään yhdessä valitsimen `--keep-blank-subtitles` kanssa | pois |
| `--range` | ei | `N-M`, tai `N-` loppuun asti — katso [`adjust`](adjust.md#--range) | koko tiedosto |
| `--document` | ei | `main` tai `translation` — katso [Käännös](invocation.md#käännös) | `main` |
| `-t`, `--translation-file` | valitsimen `--document translation` kanssa | käännöstiedosto, **yhdelle** syötteelle | — |
| `--align-method` | ei, ja vain valitsimen `-t` kanssa | `position` tai `number` | `position` |
| `--recursive`, `-r` | ei | lippu | pois — katso [Hakemistopuun käsittely](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **yksi kolmesta**, paitsi valitsimen `--dry-run` kanssa | katso [Käynnistys](invocation.md#kohde) | — |
| `--dry-run` | ei | lippu: laskee ja kertoo, ei kirjoita mitään — katso [Esikatselu ennen kirjoittamista](invocation.md#esikatselu-ennen-kirjoittamista) | pois |

### Tehtävät

**Luettelon järjestyksellä ei ole merkitystä**: tehtäviä sovelletaan
Gaupolin järjestyksessä — maininnat, sitten sanojen yhdistäminen ja jakaminen, sitten yleiset virheet, sitten isot alkukirjaimet ja lopuksi rivinvaihto —, kukin
tekstiin, jonka edellinen jätti. Kappale, jonka maininnat tyhjensivät, ei osallistu
siihen, mitä seuraa.

| Tehtävä | Mitä se tekee |
| :---- | :-------------- |
| `mentions` | poistaa hakasulkeissa olevat äänet, ja kaarisulkeissa olevat, kun `Sound in parentheses` on kytketty päälle (alikomennon [`hearing-impaired`](hearing-impaired.md) siivous), sitten laulujen sanojen ja puhujien lausekkeet |
| `join-words` | liittää takaisin kahtia katkenneen sanan valitsimen `--language` sanakirjan perusteella |
| `split-words` | erottaa kaksi yhteen liimautunutta sanaa saman sanakirjan perusteella |
| `common-errors` | korjaa yleiset virheet: välilyönnit, välimerkit, ligatuurit, tekstintunnistuksen virheet |
| `capitalization` | panee ison alkukirjaimen sinne, minne kieli sen haluaa |
| `line-break` | jakaa kappaleen rivit uudelleen niin, ettei mikään ylitä arvoa `--max-length`, punniten koodin rivinvaihtolausekkeita |

### Lausekekoodi

**`--code` nimeää kaskadin**: `Zyyy` mille tahansa kirjoitusjärjestelmälle, sitten kirjoitusjärjestelmä, kieli
ja maa — `Latn-en-US` lukee koodien `Zyyy`, `Latn`, `Latn-en` ja
`Latn-en-US` lausekkeet. **Se on pakollinen**, koska komentorivillä ei ole ympäröivää kieltä:
yleinen oletus soveltaisi *vähemmän* kuin luullaan, kertomatta siitä. Se pätee
saman ajon jokaiseen tehtävään; tehtävä, joka haluaa oman koodinsa, on toinen
ajo. Väärin kirjoitettu koodi on käyttövirhe:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code francais --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code francais --output b.srt a.srt; echo "code=$?"
--code: "francais" is not a pattern code: expected Script[-language[-COUNTRY]], like Zyyy, Latn or Latn-en-US
code=1
```

Lausekkeet, jotka käyttäjä panee hakemistoon `$XDG_DATA_HOME/subedit/patterns` (tai
`~/.local/share/subedit/patterns`), **lisätään toimitettuihin lausekkeisiin** kuten
ikkunassa, ja toimitettu lauseke, jota ei voi lukea, kerrotaan tasolla 1: `patterns: <file>, line N (<reason>)`.

### Sanojen yhdistäminen ja jakaminen sekä sanakirja

**Nämä kaksi tehtävää eivät lue lausekkeita**: ne kysyvät **järjestelmän sanakirjalta
Enchantin kautta** — se on pakettien riippuvuus (Debianissa ja Ubuntussa `libenchant-2-2`, Fedorassa
`enchant2`), jonka `subedit-cli` jakaa ikkunan kanssa. Niillä ei siis ole koodia eikä asetettavaa nimeä:
`--code` on merkityksetön ilman lausekkeita lukevaa tehtävää, ja `--language` ilman jompaakumpaa näistä kahdesta
— toinen ilman toista on käyttövirhe, koodi `1`.

- **`join-words`** liittää sanan, jota sanakirja ei tunne, edeltävään tai
  seuraavaan sanaan, **kun vain yksi suunta antaa tunnetun sanan**: `hel lo` muuttuu muotoon `hello`; kaksi mahdollista
  suuntaa tai ei yhtään ei ratkaise mitään.
- **`split-words`** jakaa sanan, jota sanakirja ei tunne, **kun täsmälleen yksi sen
  ehdotuksista on sama sana välilyönnin kera**: `hellothere` muuttuu muotoon `hello there`. Sana, jossa on iso
  alkukirjain ja sen perässä pieniä kirjaimia, jätetään rauhaan: se on useimmiten nimi.

**Ilman kielen sanakirjaa ajo hylätään**, koodi `1`, **ennen kuin yhtäkään tiedostoa
luetaan**, ja ikkunan sanoin; ikkuna näyttää toiminnon harmaana ja kertoo sen — tässä komentorivi
hylkää ja kertoo sen sen sijaan, että vaikenisi:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nhel lo\n\n' > a.srt; subedit-cli --no-config correct --tasks join-words --language qq_QQ --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nhel lo\n\n' > a.srt; subedit-cli --no-config correct --tasks join-words --language qq_QQ --output b.srt a.srt; echo "code=$?"
no dictionary for qq_QQ
code=1
```

**Ikkunan korvausluetteloa ei lueta eikä kirjoiteta.** Se *täyttyy* käyttäjän
oikoluvussa tekemistä valinnoista; komentorivillä ei ole sellaista
valintaa, ja luettelon lukeminen tekisi skriptistä riippuvaisen siitä, mitä sen tekijä joskus pani luetteloon.

### Rivinvaihto

**Se mitataan merkkeinä, ja siinä on ero ikkunaan.** Ikkuna mittaa *em-yksiköissä* —
fontin mukaan kalibroitu leveys, 0,55 em kirjainta kohden —, ja sen oletusarvo 24 on arvo
em-yksiköissä. Komentorivillä ei ole fonttia: se laskee merkkejä, tekstin
merkkejä. **Luvun 24 kopioiminen merkeiksi katkaisisi lähes kaiken
kertomatta siitä**; siksi **oletuspituutta ei ole**, ja `--max-length` on pakollinen
tehtävän `line-break` kanssa:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn texte\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn texte\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --output b.srt a.srt; echo "code=$?"
--max-length is required by the task line-break: Gaupol's 24 is a width in ems, and has no value in characters
code=1
```

**`--max-lines` on 3**, Gaupolin arvo, jota yksikkö ei muuta; rivinvaihto ottaa
yhden rivin lisää, kun kolme ei riitä. **Ohitus** — jo rajojen sisällä oleva kappale
jätetään ennalleen — käyttää oletuksena **samoja rajoja** (`--skip-length` = `--max-length`,
`--skip-lines` = `--max-lines`), kuten Gaupol; `--skip-length off` ja `--skip-lines off`
kytkevät sen pois, ja teksti luetaan silloin uudelleen yhtenä ja jaetaan uudelleen:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nWhen the rain stopped, we walked home under the dark sky of the old town.\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --max-length 24 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nWhen the rain stopped, we walked home under the dark sky of the old town.\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --max-length 24 --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:03,000
When the rain
stopped, we walked
home under the dark
sky of the old town.
```

Rivinvaihtolausekkeet ovat koodin lausekkeita: `Latn` ei katkaise luvun ja sen yksikön välistä eikä
vuoropuheluviivan jälkeen, `Latn-en` ei katkaise artikkelin, preposition, tittelin
tai possessiivisen määreen jälkeen. **Koodilla `Zyyy` rivinvaihtolausekkeita ei ole**: rivit vain
tasapainotetaan. **Tätä tehtävää ei koskaan hylätä voimassa olevan lausekkeen puuttumisen vuoksi**: sen lausekkeet
punnitsevat, mistä katkaista, ja katkaisu tapahtuu ilman niitäkin. Neljällä valitsimella on merkitystä vain
tehtävän `line-break` kanssa; ilman sitä annettuina ne ovat käyttövirhe.

### Luokat ja kytkeminen nimen perusteella

**`--classes` suodattaa yleiset virheet lopullisesti**: luokan `ocr` valinnan poistaminen ottaa sen
lausekkeet pois sovelluksesta — ligatuuri `ﬁ` pysyy silloin muodossa `ﬁ`. Muilla kolmella lausekelajilla
ei ole luokkaa.

**`--enable` ja `--disable` asettavat lausekkeen sen englanninkielisen nimen perusteella**, ja nimi on avain
(ADR 0037); nimi voi tarkoittaa useaa tietuetta, jotka silloin jakavat yhden
ruudun. Se on välttämätöntä, ja juuri se syrjäyttää ikkunan oletusarvot: **toimitetut
tiedostot `.conf` kytkevät mainintojen lausekkeet pois**, joten pelkkä `--tasks mentions`
ei soveltaisi mitään. Kaksi lauseketta ”Sound in brackets” ja ”Sound in parentheses”
(ikkunassa Ääni hakasulkeissa ja Ääni kaarisulkeissa) ohjaavat kumpikin omaa erotintaan — hakasulkeita, kaarisulkeita — ja ne asetetaan samoilla
valitsimilla: toisen kytkeminen yksinään päälle ei poista toista.

Kolme asiaa on **käyttövirheitä**, koodi `1`, ja ne kerrotaan ennen kuin yhtäkään tiedostoa luetaan:

| Mikä sen aiheuttaa | Viesti |
| :------------------ | :------ |
| nimi, joka ei tarkoita annettujen tehtävien yhtäkään lauseketta annetulla koodilla | `--enable: "<name>" names no pattern of the tasks given, under the code <code>` |
| nimi, joka tarkoittaa kahden lajin lausekkeita — se kirjoitetaan silloin `type:name`, jossa laji on `common-error`, `capitalization`, `hearing-impaired` tai `line-break` | `--enable: "<name>" names patterns of several types: write type:name, …` |
| nimi, joka annetaan sekä valitsimelle `--enable` että valitsimelle `--disable` | `"<name>" is given to --enable and to --disable: choose one` |

**Kirjoitusvirhe kuuluu**: ikkuna ohittaa vanhentuneen asetuksen, koska
asetus voi elää kauemmin kuin kadonnut lauseke; käynnistyskerralla ei ole jälkeenjääneitä.

**Tehtävä ilman yhtään voimassa olevaa lauseketta hylätään**, koodi `1` — ”ei mitään tehtävää” on
vastaus, joka sanotaan, ei hiljainen onnistuminen:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --output b.srt a.srt; echo "code=$?"
mentions: no pattern is active under the code Latn; switch one on with --enable
code=1
```

Sama tiedosto, kun hakasulkeiden siivous on kytketty päälle:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --enable "Sound in brackets" --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --enable "Sound in brackets" --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:02,000
Bonjour
```

## Mitä kirjoitetaan ja mitä kerrotaan

Luetun tiedoston muoto, merkistökoodaus ja rivinvaihdot **säilytetään**. **Kappaleet,
jotka korjaus tyhjentää, poistetaan** kuten ikkunan oletusruudussa;
`--keep-blank-subtitles` jättää ne tyhjiksi, ja `--remove-blank-subtitles` poistaa ne
silloinkin, kun asetustiedosto säilyttää ne. **Tyhjennetty käännös pysyy tyhjänä**
ja säilyttää kappaleensa.

**Laskuri on ikkunan laskuri** — ”Edited N and removed M subtitles”: muuttuneet
tekstit ja poistetut kappaleet, **ei koskaan osumia**. Tiedosto, jossa mikään ei
muutu, kirjoitetaan silti: annettu kohde on kirjoitettu kohde.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\neﬀet  de ﬁn\n\n2\n00:00:03,000 --> 00:00:04,000\nPropre.\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code Latn --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\neﬀet  de ﬁn\n\n2\n00:00:03,000 --> 00:00:04,000\nPropre.\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code Latn --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:02,000
effet de fin

2
00:00:03,000 --> 00:00:04,000
Propre.
```

### Lauseke, jota ei voi käyttää

**Lauseke, jota ei voi lukea, jota ei voi kääntää tai joka ei pääty, nimetään yhdessä
sen kappaleen kanssa, jossa se luovutti**, ja tässä komentorivi kertoo enemmän kuin
ikkuna: vahvistussivu nimeää lausekkeen vain kerran ilman tekstiä.

| Syy | Mitä se tarkoittaa |
| :----- | :------------------- |
| `cannot be translated` | sen lauseke käyttää jotain, minkä käännös ICU:lle hylkää |
| `will not compile` | moottori hylkäsi sen lausekkeen |
| `has an invalid replacement` | sen korvaus ei lue niin kuin Python sen lukee |
| `timed out` | moottori luovutti tämän tekstin kohdalla |
| `never settled` | `Repeat` muutti yhä tekstiä sadan kierroksen jälkeen |
| `grew the text too long` | teksti ylitti sen, mitä kappale voi sisältää |

Tasolla 1 virhetulosteeseen: 
`<path>: pattern "<name>" (<reason>) was not applied to subtitle N` — ilman loppuosaa, kun lauseke jätetään sivuun jokaisen tekstin osalta. **Numero on
tiedoston numero** riippumatta siitä, mitkä tehtävät poistivat kappaleita aiemmin. JSON-muodossa se on
tiedoston varoitus `pattern-failed`, jonka kenttä `detail` sisältää lauseen. **Lauseke,
josta luovuttiin, ei ole epäonnistunut tiedosto**: tiedosto kirjoitetaan, ja paluuarvo on `0`.

## Tuloste

**Vakiotuloste** — ei mitään, koska tulos on kirjoitettu tiedosto. **Valitsimella `--dry-run` se on
ehdotetut muutokset**, yksi lohko kappaletta kohden: sen numero, teksti ennen, teksti jälkeen —
ja `(removed)` ilman riviä `+` kappaleelle, jonka korjaus poistaisi; valitsimella
`--keep-blank-subtitles` se jää tyhjänä. Katso [Esikatselu ennen kirjoittamista](invocation.md#esikatselu-ennen-kirjoittamista).

| Taso | Mitä virhetulosteeseen lisätään |
| :----- | :------------------------------------- |
| 1 | `<path>: Edited N and removed M subtitles -> <destination>`, lausekkeet, joita ei voitu käyttää, lauseketiedostot, joita ei voi lukea — ja ensin luettu asetustiedosto — `settings: read <path>` — ja se, mitä siitä ohitettiin |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, sitten jokainen huomautus luettaessa |

JSON-muodossa `counts` sisältää kentät `corrected` (muuttuneet tekstit) ja `removed` (poistetut
kappaleet), ja `changes` kosketut kappaleet. Valitsimella `-t` avain `alignment` kertoo, miten
käännös asetettiin.
`settings` nimeää luetun asetustiedoston, tai se on `null`, kun mitään ei luettu.

## Paluuarvot

Työkalun yleiset: `0`, jos jokainen tiedosto kirjoitettiin, `2`, jos yhtäkään ei, `3`, jos vain
osa, `1` käyttövirheessä.

## Virheet

| Mikä sen aiheuttaa | Mitä virhetulosteeseen kirjoitetaan |
| :------------------ | :--------------------------------------- |
| valitsimella `--config` nimetty tiedosto, jota ei ole olemassa — käyttäjän oma tiedosto puuttuessaan antaa oletusarvot | `settings file not found: <FILE>` |
| `--tasks` puuttuu | `--tasks is required` ja sen perässä viittaus valitsimeen `--help` |
| tuntematon tehtävä | `--tasks: "<name>" is not a task: expected mentions, join-words, split-words, common-errors, capitalization or line-break` |
| `--code` puuttuu | `--code is required by the tasks that read patterns: <tasks>` |
| `--code` ilman lausekkeita lukevaa tehtävää | `--code is for the tasks that read patterns, and none was asked for` |
| `--language` puuttuu tehtävän `join-words` tai `split-words` kanssa | `--language is required by the tasks that check words: <tasks>` |
| `--language`, joka ei ole kielikoodi | `--language: "<value>" is not a language: expected a locale code, like fr, en_US or sr@Latn` |
| `--language` ilman jompaakumpaa näistä kahdesta tehtävästä | `--language is for the tasks join-words and split-words, which were not asked for` |
| kieli ilman sanakirjaa | `no dictionary for <language>` |
| väärin kirjoitettu koodi | `--code: "<code>" is not a pattern code: expected Script[-language[-COUNTRY]], like Zyyy, Latn or Latn-en-US` |
| `--max-length` puuttuu tehtävän `line-break` kanssa | `--max-length is required by the task line-break: Gaupol's 24 is a width in ems, and has no value in characters` |
| `--max-length`, joka ei ole nollaa suurempi luku | `--max-length: "<value>" is not a length: expected a number of characters greater than zero` |
| `--max-lines`, `--skip-lines`, jotka eivät ole kokonainen rivimäärä | `--max-lines: "<value>" is not a number of lines: expected a whole number greater than zero` |
| `--skip-length` tai `--skip-lines` lukukelvoton | `--skip-length: "<value>" is not a bound: expected a number greater than zero, or off` |
| jokin neljästä valitsimesta ilman tehtävää `line-break` | `--max-length is for the task line-break, which was not asked for` |
| tuntematon luokka | `--classes: "<name>" is not a class: expected human, ocr or human,ocr` |
| `--keep-blank-subtitles` yhdessä valitsimen `--remove-blank-subtitles` kanssa | `--keep-blank-subtitles and --remove-blank-subtitles say opposite things; give one of them` |
| virheellinen lausekkeen nimi | katso yllä oleva taulukko |
| tehtävä ilman voimassa olevaa lauseketta | `<task>: no pattern is active under the code <code>; switch one on with --enable` |
| `--range`, `-t`, `--document`, kohde | sivujen [Käynnistys](invocation.md#virheet) ja [Käännös](invocation.md#käännös) virheet |
