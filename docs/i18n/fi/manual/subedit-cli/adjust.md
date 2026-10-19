# `adjust`

<!-- machine-translated -->
> Tämä sivu on konekäännös, eikä kukaan tämän kielen lukija ole vielä tarkistanut sitä.

```
subedit-cli adjust [--speed CPS|off] [--shorten | --no-shorten] [--lengthen | --no-lengthen]
                   [--minimum TIME|off] [--maximum TIME|off] [--gap TIME|off]
                   [--range N-M|N-]
                   (--output FILE | --output-dir DIR | --in-place)
                   [--dry-run]
                   [--recursive]
                   <file>...
```

Säätää kunkin kappaleen **keston** lukunopeuden ja rajojen mukaan: liian lyhyt kappale, jota ei
ehdi lukea, pidennetään, liian myöhään ruudulta poistuva kappale voidaan lyhentää, eivätkä kaksi
kappaletta mene päällekkäin. **Vain loppu siirtyy**: alun siirtäminen siirtäisi koko kappaletta,
mikä on alikomennon [`shift`](shift.md) tehtävä. Se on [ikkunan](../subedit-gui/operations.md#adjust-durations)
toiminto `Adjust Durations…` samoine oletusarvoineen.

## Neljä rajoitetta ja niiden järjestys

Niitä sovelletaan **tässä järjestyksessä**, kukin asettaa lopun, ja viimeinen voittaa:

1. **lukunopeus** — kesto, jonka teksti tarvitsee tietyllä merkkimäärällä sekunnissa
   (tageja ei lasketa, rivinvaihdot lasketaan);
2. **vähimmäiskesto** — sitä lyhyempi kesto nostetaan vähimmäiskestoon;
3. **enimmäiskesto** — sitä pidempi kesto lasketaan enimmäiskestoon;
4. **väli** — seuraavaa kappaletta edeltävä aika. Se tulee viimeisenä ja
   **se voittaa**: kaksi yhtä aikaa näkyvää kappaletta on katsojan huomaama virhe,
   hieman liian lyhyt kappale ei ole.

Tiedoston viimeisellä kappaleella ei ole seuraavaa, eikä siis väliä säilytettävänä. Kappaleen
seuraava on tiedostossa seuraava kappale, **valitsiko** `--range` **sen tai ei**.

| Rajoite | Valitsin | Oletus | Poiskytkentä |
| :--------- | :----- | :----- | :------- |
| nopeus | `--speed CPS` | **15** merkkiä sekunnissa | `--speed off` |
| nopeuden suunta | `--shorten`, `--no-shorten`, `--lengthen`, `--no-lengthen` | se vain **pidentää** | — |
| vähimmäiskesto | `--minimum TIME` | **1,5 s** | `--minimum off` |
| enimmäiskesto | `--maximum TIME` | **ei ole** | `--maximum off` |
| väli | `--gap TIME` | **0** | `--gap off` |

Nämä ovat Gaupolin asetukset ja ikkunan ensimmäisen avauskerran asetukset. **`off` kytkee
rajoitteen pois: sitä ei enää tarkisteta.** Se ei ole sama kuin nolla — vähimmäiskesto `0`
on nollan pituinen vähimmäiskesto, ja väli `0` kieltää kaiken päällekkäisyyden.

## Mitä asetustiedosto antaa

**Yllä olevat oletusarvot ovat sisäänrakennettuja.** Oletuksena `adjust` lukee asetustiedoston
toimintojen asetukset — arvot, jotka `Adjust Durations…` muistaa ikkunassa tai jotka
[`config set`](config.md) kirjoitti — ja ne korvaavat sisäänrakennetut oletusarvot. **Komentorivillä
annettu valitsin voittaa aina**, sitten tiedosto ja sitten sisäänrakennettu oletusarvo. Katso
[Asetustiedosto](invocation.md#asetustiedosto): missä tiedosto on, `--config` ja
`--no-config`.

| Avain | Mitä se antaa | Sen ohittava valitsin |
| :-- | :--------------- | :----------------------- |
| `duration-adjust.speed` | lukunopeus merkkeinä sekunnissa | `--speed CPS` |
| `duration-adjust.lengthen`, `duration-adjust.shorten` | mihin suuntaan nopeus saa siirtää loppua; kun molemmat ovat `false`, nopeus on pois päältä | `--lengthen`, `--no-lengthen`, `--shorten`, `--no-shorten`, `--speed off`; valitsin `--speed CPS` tai suunta kytkee tiedoston poiskytkemän nopeuden takaisin päälle pidentävänä |
| `duration-adjust.minimum-enabled`, `duration-adjust.minimum-ms` | lyhin kesto, tai ei mitään | `--minimum TIME`, `--minimum off` |
| `duration-adjust.maximum-enabled`, `duration-adjust.maximum-ms` | pisin kesto, tai ei mitään | `--maximum TIME`, `--maximum off` |
| `duration-adjust.gap-enabled`, `duration-adjust.gap-ms` | pienin väli, tai ei mitään | `--gap TIME`, `--gap off` |

**Jokaisella avaimella on valitsin, joka kumoaa sen.** Kummallakin suunnalla on lippupari, joka
asettaa sen riippumatta tiedostosta: `--shorten` ja `--no-shorten` lyhentämiselle, `--lengthen` ja
`--no-lengthen` pidentämiselle; antamatta jätetty lippu jättää tiedoston suunnan ennalleen.
`--maximum off` poistaa tiedoston päälle kytkemän enimmäiskeston. Valitsimella `--format json` kenttä `constraints`
kertoo todella käytetyt asetukset niiden alkuperästä riippumatta, ja kenttä `settings` luetun tiedoston:

<!-- example: printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine/settings.conf config set duration-adjust.speed 12 2>/dev/null; subedit-cli --config mine/settings.conf --format json adjust --dry-run a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine/settings.conf config set duration-adjust.speed 12 2>/dev/null; subedit-cli --config mine/settings.conf --format json adjust --dry-run a.srt
settings: read mine/settings.conf
a.srt: adjusted the durations of 1 subtitle (dry run, nothing written)
{"schema":1,"command":"adjust","file":"a.srt","ok":true,"dry_run":true,"destination":null,"counts":{"subtitles":1,"adjusted":1,"sacrificed":{"speed":0,"minimum":0,"gap":0}},"constraints":{"speed":{"cps":"12","lengthen":true,"shorten":false},"minimum_ms":1500,"maximum_ms":null,"gap_ms":0},"settings":"mine/settings.conf","warnings":[]}
```

Tämän sivun muut esimerkit ajetaan valitsimella `--no-config`, jotta ne kertovat, mitä sisäänrakennetut
oletusarvot tekevät, riippumatta niitä ajavan koneen asetuksista.

<!-- example: subedit-cli adjust --help -->
```console
$ subedit-cli adjust --help
Bring the duration of every subtitle within a reading speed and limits
Usage: subedit-cli adjust [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to adjust

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --speed CPS|off             Reading speed in characters per second, or off (default 15)
  --shorten                   Let the reading speed bring an end earlier
  --no-shorten                Do not let the reading speed bring an end earlier
  --lengthen                  Let the reading speed move an end later
  --no-lengthen               Do not let the reading speed move an end later
  --minimum TIME|off          Shortest duration, or off (default 1.5)
  --maximum TIME|off          Longest duration, or off (default: none)
  --gap TIME|off              Least time left before the next subtitle, or off (default 0)
  --range N-M|N-              Act only on subtitles N to M, or N to the end
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Argumentit ja valitsimet

| Valitsin | Pakollinen | Arvot | Oletus |
| :----- | :----- | :------ | :----- |
| `<file>...` | kyllä | yksi tai useampi polku | — |
| `--recursive`, `-r` | ei | lippu | pois — katso [Hakemistopuun käsittely](lots.md) |
| `--speed` | ei | merkkimäärä sekunnissa, **nollaa suurempi** (`15`, `12.5`), tai `off` | `15` |
| `--shorten` | ei | lippu: nopeus saa myös **aikaistaa** loppua | pois |
| `--no-shorten` | ei | lippu: nopeus ei enää **aikaista** loppua, sanoipa asetustiedosto mitä tahansa | pois |
| `--lengthen` | ei | lippu: nopeus **myöhentää** loppua, sanoipa asetustiedosto mitä tahansa | pois |
| `--no-lengthen` | ei | lippu: nopeus ei enää **myöhennä** loppua | pois |
| `--minimum` | ei | kesto, joka on nolla tai enemmän, tai `off` | `1.5` |
| `--maximum` | ei | kesto, joka on **nollaa suurempi**, tai `off` | ei ole |
| `--gap` | ei | kesto, joka on nolla tai enemmän, tai `off` | `0` |
| `--range` | ei | `N-M`, tai `N-` loppuun asti — katso alla | koko tiedosto |
| `--output` / `--output-dir` / `--in-place` | **yksi kolmesta**, paitsi valitsimen `--dry-run` kanssa | katso [Käynnistys](invocation.md#kohde) | — |
| `--dry-run` | ei | lippu: laskee ja kertoo, ei kirjoita mitään — katso [Esikatselu ennen kirjoittamista](invocation.md#esikatselu-ennen-kirjoittamista) | pois |

Kesto kirjoitetaan kuten alikomennolle [`shift`](shift.md#keston-kirjoittaminen): `2.5`, `00:00:02.500`.

**Ilman yhtään voimassa olevaa rajoitetta `adjust` hylätään** (koodi `1`, mitään ei kirjoiteta): ei
olisi mitään, minkä mukaan säätää, ja komento, joka ei tee mitään mutta kertoo onnistuneensa, on huonoin
mahdollinen vastaus. Näin käy yhdistelmällä `--speed off --minimum off --gap off` ilman enimmäiskestoa, sekä
pelkällä valitsimella `--no-lengthen`, kun vähimmäiskesto ja väli on kytketty pois: nopeus, joka ei siirrä
yhtään loppua, ei rajoita mitään.

`--shorten`, `--no-shorten`, `--lengthen` ja `--no-lengthen` kertovat, mihin suuntaan nopeus siirtää
loppua; niillä on merkitystä vain nopeuden kanssa, ja **`--speed off` hylkää ne**. Lippu yhdessä
vastakohtansa kanssa — `--shorten --no-shorten`, `--lengthen --no-lengthen` — hylätään myös.

### `--range`

`--range 3-7` rajaa säädön kappaleisiin 3–7, **laskettuna ykkösestä, rajat
mukaan lukien**; `--range 3-` jatkuu viimeiseen asti. Yksittäinen kappale kirjoitetaan `4-4`.

- Aikaväli, **joka päättyy ennen kuin alkaa**, `0`, etumerkki tai pelkkä luku
  hylätään käyttövirheenä (koodi `1`).
- **Se pätee erän jokaiseen tiedostoon**, mikä on järkevää vain, jos tiedostot
  muistuttavat toisiaan. Se arvioidaan **kunkin tiedoston mukaan**: tiedoston ohi menevä aikaväli
  hylätään **ennen kuin tiedostoon kosketaan**, ja raja nimetään —
  `range 3-9 ends after the last subtitle: the file has 4` —, tiedoston käsittely epäonnistuu (koodi `2` tai `3`, tunniste
  `range-out-of-bounds`) ja muiden käsittely jatkuu.
- Raportin laskurit koskevat vain valittuja kappaleita.

## Mistä tingitään

Kaksi rajoitetta voi olla ristiriidassa samassa kappaleessa: 1,5 sekunnin vähimmäiskesto ei mahdu
0,8 sekuntiin, jonka seuraava kappale jättää. **Järjestys ratkaisee, ja raportti kertoo,
mikä joutui joustamaan** — mitä Gaupol ei tee: se rikkoo rajoitteet hiljaa.

Kukin rajoite mainitaan vain, jos **vähintään yksi kappale ei saanut sitä**, kun
muut on sovellettu. Enimmäiskestolla ei ole laskuria: sitä sovelletaan ennen väliä, joka voi
vain lyhentää, joten se pätee aina. Enimmäiskestoa suurempi vähimmäiskesto joustaa enimmäiskeston hyväksi ja
lasketaan vähimmäiskestoksi.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --output ajuste.srt courts.srt; grep -e '-->' ajuste.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --output ajuste.srt courts.srt; grep -e '-->' ajuste.srt
courts.srt: adjusted the durations of 3 subtitles; could not satisfy the minimum duration in 1 subtitle -> ajuste.srt
00:00:00,000 --> 00:00:02,000
00:00:10,000 --> 00:00:14,000
00:00:14,200 --> 00:00:15,000
00:00:15,000 --> 00:00:16,500
```

Kolmas kappale haluaisi päättyä kohdassa 15,7 s kestääkseen 1,5 s, mutta neljäs alkaa kohdassa
15,0 s: **väli voittaa**, kappale päättyy kohdassa 15,0 s, ja raportti mainitsee vähimmäiskeston.

Välin poiskytkentä antaa vähimmäiskeston voittaa — ja kaksi kappaletta menee päällekkäin:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --gap off --output libre.srt courts.srt; grep -e '-->' libre.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --gap off --output libre.srt courts.srt; grep -e '-->' libre.srt
courts.srt: adjusted the durations of 3 subtitles -> libre.srt
00:00:00,000 --> 00:00:02,000
00:00:10,000 --> 00:00:14,000
00:00:14,200 --> 00:00:15,700
00:00:15,000 --> 00:00:16,500
```

Eikä mistään tingitä eikä mitään säädetä, kun tiedosto on jo rajojensa sisällä:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --speed off --minimum off --output intact.srt courts.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --speed off --minimum off --output intact.srt courts.srt
courts.srt: no duration to adjust -> intact.srt
```

Tiedosto kirjoitetaan sellaisenaan: **annettu kohde on kirjoitettu kohde**.

## Tuloste

**Vakiotuloste** — ei mitään: tulos on kirjoitettu tiedosto.

| Taso | Mitä virhetulosteeseen lisätään |
| :----- | :------------------------------------- |
| 1 | `<path>: adjusted the durations of N subtitles; could not satisfy …  -> <destination>` tai `<path>: no duration to adjust -> <destination>` — ja ensin luettu asetustiedosto — `settings: read <path>` — katso [Asetustiedosto](invocation.md#asetustiedosto) |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — luetun tiedoston muoto, merkistökoodaus, merkki ja rivinvaihdot, palautettuina ennalleen |
| 3 | `<path>: N bytes read, M written`, sitten **jokainen huomautus luettaessa** — katso [Käynnistys](invocation.md#huomautukset-luettaessa) |

Lause on ikkunan lause: `N` on niiden kappaleiden määrä, joiden loppu siirtyi, ja
loppuosa, joka puuttuu, kun mikään ei joustanut, luettelee, mitä ei säilytetty — `the reading speed`,
`the minimum duration`, `the gap` — sekä asianomaisten kappaleiden määrän.

### JSON-muodossa

Valitsimella [`--format json`](invocation.md#skriptin-luettava-tuloste) kunkin tiedoston olio
sisältää kirjekuoren lisäksi:

| Avain | Sisältö |
| :-- | :------ |
| `counts.subtitles` | **kohteena olevat** kappaleet: koko tiedosto tai aikaväli |
| `counts.adjusted` | ne, joiden loppu siirtyi |
| `counts.sacrificed.speed`, `.minimum`, `.gap` | kappaleet, jotka eivät edelleenkään täytä tätä rajoitetta, kun muut on sovellettu, **mukaan lukien ne, joiden loppu ei siirtynyt** |
| `constraints.speed` | `{"cps", "lengthen", "shorten"}` — merkit sekunnissa **merkkijonona** (`"15"`, `"12.5"`) — tai `null`, jos pois päältä |
| `constraints.minimum_ms`, `.maximum_ms`, `.gap_ms` | käytetyt kestot **kokonaisina millisekunteina**, tai `null`, jos pois päältä |
| `settings` | luettu asetustiedosto, tai `null`, kun mitään ei luettu |

`constraints` kertoo **todella käytetyt asetukset** oletusarvot mukaan lukien: kahta samoja
kappaleita koskevaa laskentaa voi silloin verrata lukematta komentoriviä.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config --format json adjust --dry-run --maximum 1.8 courts.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config --format json adjust --dry-run --maximum 1.8 courts.srt
courts.srt: adjusted the durations of 4 subtitles; could not satisfy the reading speed in 1 subtitle, the minimum duration in 1 subtitle (dry run, nothing written)
{"schema":1,"command":"adjust","file":"courts.srt","ok":true,"dry_run":true,"destination":null,"counts":{"subtitles":4,"adjusted":4,"sacrificed":{"speed":1,"minimum":1,"gap":0}},"constraints":{"speed":{"cps":"15","lengthen":true,"shorten":false},"minimum_ms":1500,"maximum_ms":1800,"gap_ms":0},"settings":null,"warnings":[]}
```

## Paluuarvot

Työkalun yleiset: `0`, jos jokainen tiedosto käsiteltiin, `2`, jos yhtäkään ei, `3`, jos vain
osa, `1` käyttövirheessä. Tiedosto, joka on jo rajojensa sisällä, on käsitelty.

## Virheet

| Mikä sen aiheuttaa | Viesti |
| :------------------ | :------ |
| valitsimella `--config` nimetty tiedosto, jota ei ole olemassa — käyttäjän oma tiedosto puuttuessaan antaa oletusarvot | `settings file not found: <FILE>` |
| ei yhtään voimassa olevaa rajoitetta | `adjust has nothing to adjust to: every constraint is off, or the reading speed is told to move no end` |
| nopeus, joka ei ole nopeus | `--speed: "…" is not a reading speed: expected a number of characters per second above zero, like 15 or 12.5, or off` |
| suuntalippu yhdessä valitsimen `--speed off` kanssa | `--lengthen, --no-lengthen, --shorten and --no-shorten say which way the reading speed may move an end, and --speed off switches the reading speed off` |
| suuntalippu yhdessä vastakohtansa kanssa | `--shorten and --no-shorten say opposite things; give one of them` (samoin `--lengthen` ja `--no-lengthen`) |
| kesto, joka ei ole kesto | `--minimum: "…" is not a time: …` (samoin `--maximum`, `--gap`) |
| negatiivinen kesto | `--minimum: "-1" is not a duration of zero or more` (samoin `--gap`) |
| nolla tai negatiivinen enimmäiskesto | `--maximum: "0" is not a duration above zero` |
| väärin kirjoitettu aikaväli | `--range: "…" is not a range: write it N-M, or N- to go to the end, counted from 1 and inclusive` |
| aikaväli, joka päättyy ennen kuin alkaa | `--range: "5-3" is not a range: it ends before it starts` |
| tiedoston ulkopuolinen aikaväli | `<path>: range N-M starts after the last subtitle: the file has K` tai `… ends after …` |

Kohteen virheet ovat yhteisiä kaikille kirjoittaville alikomennoille:
katso [Käynnistys](invocation.md#kohde).
