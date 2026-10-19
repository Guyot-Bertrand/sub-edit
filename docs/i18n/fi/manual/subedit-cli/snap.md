# `snap`

<!-- machine-translated -->
> Tämä sivu on konekäännös, eikä kukaan tämän kielen lukija ole vielä tarkistanut sitä.

```
subedit-cli snap --rate <rate>
                 (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 <file>...
```

Sijoittaa **jokaisen aikaleiman** — alun ja lopun — annetun ruutunopeuden lähimpään
ruutuun. Mitään ei ajoiteta uudelleen: kukin kohta siirtyy enintään puolen
ruudun verran.

## `snap` ei ole `framerate`, eikä niiden sekoittaminen näy

Kahdella komennolla on sama muoto — tiedostot, kohde, ruutunopeus — ja ne tekevät päinvastaisia asioita.

| | [`framerate`](framerate.md) | `snap` |
| :--- | :-------------------------- | :----- |
| mitä se laskee | `t' = t x input rate / output rate` | lähimmän ruudun, valitsimen `--rate` ruudukolla |
| kuinka paljon se siirtää | suhteessa aikaan — pitkässä elokuvassa **useita sekunteja** | enintään puoli ruutua, **kaikkialla** |
| millaiselle tiedostolle | tiedostolle, jonka **ajoitus on väärä** | tiedostolle, jonka ajoitus on **oikea** ja ruudukko väärä |

**Alikomennon `snap` käyttötapaus** tarkemmin: elokuva, jonka ruutunopeus on 25 ruutua
sekunnissa, tekstitystiedosto, joka on kirjoitettu ruudukolle 24, ja rivit, jotka ovat jo
suunnilleen paikoillaan muutaman millisekunnin tarkkuudella. Mitään ei tarvitse ajoittaa uudelleen
— aikaleimat on vain palautettava oikeisiin ruutuihin.

**Alikomennon `framerate` käyttötapaus** on toinen: tiedosto, joka on ajoitettu master-kopiolle nopeudella
25 ja joka halutaan toistaa kopiolla nopeudella 23,976, ja jonka ajoitus ajautuu
elokuvan loppuun mennessä useita sekunteja.

Toisen erehtyminen toiseksi **ei aiheuta virhettä**: kumpikin
hyväksyy tiedoston ja kirjoittaa tuloksen. Kerrontarivi on se, joka
erottaa ne — `snap` kertoo aina, kuinka paljon se siirsi kohtia.

<!-- example: subedit-cli snap --help -->
```console
$ subedit-cli snap --help
Move every position onto the nearest frame of a frame rate (see framerate)
Usage: subedit-cli snap [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to align

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --rate TEXT REQUIRED        Frame rate to align on: 25, 23.976
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
| `--rate` | **kyllä** | ruutunopeus ruutuina sekunnissa, kirjoitettuna kuten alikomennolle [`framerate`](framerate.md#ruutunopeuden-kirjoittaminen) | — |
| `--output` / `--output-dir` / `--in-place` | **yksi kolmesta**, paitsi valitsimen `--dry-run` kanssa | katso [Käynnistys](invocation.md#kohde) | — |
| `--dry-run` | ei | lippu: laskee ja kertoo, ei kirjoita mitään — katso [Esikatselu ennen kirjoittamista](invocation.md#esikatselu-ennen-kirjoittamista) | pois |

`--rate` hyväksyy **minkä tahansa kelvollisen ruutunopeuden**, eikä vain kahdeksaa
vakionopeutta: valinta on käyttäjän, kuten alikomennolla `framerate`. Nolla tai negatiivinen
ruutunopeus hylätään.

Luetun tiedoston muoto **säilytetään**: muodon vaihtaminen on alikomennon
[`convert`](convert.md) tehtävä.

## Tuloste

**Vakiotuloste** — ei mitään: tulos on kirjoitettu tiedosto. Valitsimella `--dry-run` ei myöskään mitään: tiedostoa ei ole, ja kerrontarivi kertoo sen — katso [Esikatselu ennen kirjoittamista](invocation.md#esikatselu-ennen-kirjoittamista).

| Taso | Mitä virhetulosteeseen lisätään |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles aligned on 25 fps, M positions moved, by at most 20 ms -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — luetun tiedoston muoto, merkistökoodaus, merkki ja rivinvaihdot, palautettuina sellaisinaan |
| 3 | `<path>: N bytes read, M written`, sitten **jokainen lukemisen huomautus** — katso [Käynnistys](invocation.md#huomautukset-luettaessa) |

**Rivin ydin on sen kaksi lukua.** Puoli ruutua nopeudella 25 ruutua
sekunnissa on kaksikymmentä millisekuntia, eikä siihen voi kirjoittaa mitään suurempaa. Joka
halusi alikomennon `framerate` ja kirjoitti `snap`, lukee ”enintään 20 ms” siellä, missä hän odotti
sekunteja, ja tietää sen heti.

## Esimerkki

Rivi kohdassa `00:00:01,010`, kohdistettuna nopeudelle 25 ruutua sekunnissa — siis
neljänkymmenen millisekunnin monikertaan:

<!-- example: printf '1\n00:00:01,010 --> 00:00:03,020\nUn.\n\n' > a.srt; subedit-cli snap --rate 25 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,010 --> 00:00:03,020\nUn.\n\n' > a.srt; subedit-cli snap --rate 25 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle aligned on 25 fps, 2 positions moved, by at most 20 ms -> b.srt
1
00:00:01,000 --> 00:00:03,040
Un.
```

## Kaksi ominaisuutta

**Toinen kohdistus ei muuta mitään.** Kohta, joka on jo
ruudussa, pysyy siinä: sama toiminto sovelletaan samaan tulokseen.

**Kappaleiden järjestys säilyy.** Pyöristys lähimpään ruutuun on
monotoninen, joten kaksi alkua, joiden väli on vähintään yksi ruutu, pysyvät erillään. Lähempänä
toisiaan ne voivat sulautua — mikä on poikkeama, jonka
[`inspect`](inspect.md#mitä-anomalies-kertoo) osaa nimetä, eikä
järjestyksen kääntyminen.

### JSON-muodossa

Valitsimella [`--format json`](invocation.md#skriptin-luettava-tuloste) kunkin tiedoston objekti sisältää kentät `counts.subtitles`, `counts.moved` (siirretyt kohdat) ja `counts.furthest_ms` (suurin siirto millisekunteina). Koko taulukko on sivulla [Käynnistys](invocation.md#mitä-kukin-alikomento-kirjoittaa-kenttään-counts).

## Paluuarvot

Työkalun paluuarvot: `0`, jos jokainen tiedosto kirjoitettiin, `2`, jos yhtäkään ei kirjoitettu, `3`, jos
vain osa, `1` käyttövirheessä.

## Virheet

Ruutunopeuden virheet, sanasta sanaan samat kuin alikomennolla [`framerate`](framerate.md#virheet)
— kumpikin lukee saman arvon samaa reittiä:

| Mikä sen aiheuttaa | Viesti |
| :------------------ | :------ |
| nolla tai negatiivinen ruutunopeus | `"…" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed` |
| desimaalipilkku | `"…" is not a frame rate: use a decimal point and not a comma, in every language` |
| teksti, joka ei ole luku | `"…" is not a frame rate: expected frames per second, like 25 or 23.976` |
| suhteeton ruutunopeus | `"…" is not a frame rate: no video runs at that many frames per second` |

**Mitään ei kirjoiteta ilman jotakin kolmesta kohdevalitsimesta**, ja niiden virheet
ovat yhteisiä kaikille kirjoittaville alikomennoille:
katso [Käynnistys](invocation.md#kohde).
