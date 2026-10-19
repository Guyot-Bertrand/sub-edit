# `hearing-impaired`

<!-- machine-translated -->
> Tämä sivu on konekäännös, eikä kukaan tämän kielen lukija ole vielä tarkistanut sitä.

```
subedit-cli hearing-impaired
                  (--output FILE | --output-dir DIR | --in-place)
                  [--dry-run]
                  [--recursive] [--parentheses]
                  [--document main|translation] [-t FILE] [--align-method position|number]
                  <file>...
```

Poistaa koko tiedostosta **kuulovammaisille tarkoitetut maininnat** — hakasulkeiden väliin kuvatut äänet.
**Kaarisulkeet jäävät oletuksena**: niitä käytetään yhtä usein kuiskatulle
repliikille kuin kuvatulle äänelle, ja niiden poistaminen pyyhkisi vuoropuhelua. `--parentheses` poistaa
nekin tiedostosta, joka kuvaa äänensä niissä.

Mitä se ei tee: risuaitojen väliin kirjoitettuja laulujen sanoja, kaksoispistettä edeltävää
puhujan nimeä, isojen alkukirjainten palauttamista, tekstintunnistuksen virheiden korjaamista. Ne
lausekkeet kuuluvat ikkunan avustajalle
[`Correct Texts…`](../subedit-gui/correct-texts.md), jota komentorivi
tarjoaa nimellä [`correct`](correct.md); eikä tässä voi asettaa mitään:
muunnos on päätetty, ei määritettävissä.

<!-- example: subedit-cli hearing-impaired --help -->
```console
$ subedit-cli hearing-impaired --help
Remove the sounds described between square brackets
Usage: subedit-cli hearing-impaired [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to clean

Options:
  -h,--help                   Print this help message and exit
  --parentheses               Also remove the sounds between parentheses, often whispered lines
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --document main|translation The document to change: the main one, or the translation given by -t
  -t,--translation-file FILE  Translation file to lay over the subtitle file, for a single input
  --align-method position|number
                              How the lines of the translation find their subtitles
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Argumentit ja valitsimet

| Valitsin | Pakollinen | Arvot | Oletus |
| :----- | :----- | :------ | :----- |
| `<file>...` | kyllä | yksi tai useampi polku | — |
| `--parentheses` | ei | lippu: poistaa myös kaarisulkeiden väliset maininnat | pois |
| `--recursive`, `-r` | ei | lippu | pois — katso [Hakemistopuun käsittely](lots.md) |
| `--document` | ei | `main` tai `translation` — katso [Käännös](invocation.md#käännös) | `main` |
| `-t`, `--translation-file` | valitsimen `--document translation` kanssa | käännöstiedosto **yhdelle** syötteelle | — |
| `--align-method` | ei, ja vain valitsimen `-t` kanssa | `position` tai `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **yksi kolmesta**, paitsi valitsimen `--dry-run` kanssa | katso [Käynnistys](invocation.md#kohde) | — |
| `--dry-run` | ei | lippu: laskee ja kertoo, ei kirjoita mitään — katso [Esikatselu ennen kirjoittamista](invocation.md#esikatselu-ennen-kirjoittamista) | pois |

Luetun tiedoston muoto **säilytetään**: muodon vaihtaminen on alikomennon
[`convert`](convert.md) tehtävä.

**Oletuksena pääteksti; käännös valitsimilla `--document translation -t FILE`.** Silloin
muutetaan ja kirjoitetaan käännös, ja vain se — katso [Käännös](invocation.md#käännös).

**Maininta, joka tyhjentäisi käännöksen, kirjoittaa sen tyhjänä ja säilyttää kappaleen**: kappale
jää tiedostoon ilman tekstiä. Päätekstistä sen sijaan poistuu kokonaan tyhjentynyt kappale, kuten
alla.

## Mitä poistetaan ja mitä jää

Maininta on hakasulje — tai kaarisulje valitsimella `--parentheses` — ja se, mitä se ympäröi, **rivinvaihto
mukaan lukien**: todellisissa tiedostoissa maininta katkeaa usein
rivin loppuun.

| Syöte | Tuloste |
| :----- | :----- |
| `Bonjour [il tousse] Marie` | `Bonjour Marie` |
| `Bonjour[il tousse]Marie` | `Bonjour Marie` |
| `[Bruit de pas] Bonjour` | `Bonjour` |
| `A [un] [deux] B` | `A B` |

Poisto jättää **täsmälleen yhden välilyönnin sen ympärillä olleiden väliin eikä mitään
rivin reunaan**. Se ei siivoa tiedoston välilyöntejä: kaksi välilyöntiä,
jotka teksti jo sisälsi, pysyvät kahtena välilyöntinä.

**Kun maininta ulottuu rivinvaihdon yli, rivinvaihto jää** — kaksi
riviä pysyvät kahtena rivinä. Tapaus ei ole kuriositeetti: viidessätoista
todellisessa tekstityksessä jokainen orvolta näyttänyt hakasulje oli
tällä tavoin katkennut maininta.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nReculez ! [Il\nhurle] Tout de suite !\n\n' > a.srt; subedit-cli --quiet hearing-impaired --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nReculez ! [Il\nhurle] Tout de suite !\n\n' > a.srt; subedit-cli --quiet hearing-impaired --output b.srt a.srt; cat b.srt
1
00:00:01,000 --> 00:00:03,000
Reculez !
Tout de suite !
```

**Mikä ei ole maininta eikä liiku:**

| Tapaus | Mitä tapahtuu |
| :-- | :-------------- |
| pelkästään numeerinen sisältö — `[1]`, `(12)`, `[ 156478 ]` | säilytetään sellaisenaan erottimineen: viite ei koskaan nimennyt ääntä |
| kaarisulje ja sen sisältö ilman valitsinta `--parentheses` | säilytetään sellaisenaan: `Venez (chut) ici` ei muutu |
| haka- tai kaarisulje, jota mikään ei sulje | jätetään ennalleen |
| muodon tagi — `<i>`, `</i>` | ei koskaan poisteta |

Tyhjä merkki numeroiden **välissä**, kirjain tai merkki — `[1 2]`, `[1a]`,
`[-1]` — tekevät siitä maininnan. Samoin tyhjä sisältö, `[]` tai `( )`: ei
numeroita, siis ei viite; ei mitään sanottavaa, siis ei mitään säilytettävää.

## Mikä katoaa kokonaan

**Rivi, jonka poisto tyhjensi, katoaa**, ja **kokonaan tyhjentynyt kappale
poistetaan tiedostosta**. Seuraavat kappaleet numeroidaan uudelleen.

Rivi lasketaan tyhjäksi, kun siihen jää pelkkiä tyhjiä merkkejä, muodon
tageja tai yksinäinen vuoropuheluviiva — `<i>[Musique]</i>` ei jätä
ruudulle merkintää `<i></i>`.

**Tagit ovat tiedoston muodon tageja.** `{\i1}[Musique]{\i0}`
Advanced SSA -tiedostossa, `{Y:i}[Musique]` MicroDVD-tiedostossa ja `/[Musique]` MPL2-tiedostossa eivät nekään jätä
mitään. Toisaalta `<i>` muodossa, joka ei kirjoita yhtäkään
tagia — TMPlayer, LRC — on tekstiä, jonka katsoja lukee, eikä sen sisältävä
rivi ole tyhjä.

**Yhteen ääneen supistunut vuoropuhelu ei enää ole vuoropuhelu**, ja jäljelle jäänyt viiva
poistuu. Niin kauan kuin kaksi ääntä on jäljellä, viivat pysyvät.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\nVoir [1] la note.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\nVoir [1] la note.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle cleaned, 1 removed -> b.srt
1
00:00:04,000 --> 00:00:06,000
Attends Marie.

2
00:00:07,000 --> 00:00:09,000
Voir [1] la note.
```

## Tiedosto, jossa ei ole yhtään mainintaa, kirjoitetaan silti

Se kopioidaan identtisenä, paluuarvo on `0`, ja raportti kertoo, ettei
poistettavaa ollut.

Tämä on kirjoittavien alikomentojen sääntö: **annettu kohde on
kirjoitettu kohde**. Poikkeus tässä pakottaisi skriptin tietämään,
mikä alikomennoista tuottaa joskus tiedoston ja joskus ei mitään.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nRien à signaler.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nRien à signaler.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt
a.srt: no mention to remove -> b.srt
```

## Tuloste

**Vakiotuloste** — ei mitään: tulos on kirjoitettu tiedosto. **Valitsimella `--dry-run` se on ehdotetut muutokset**: jokaisesta kappaleesta, jonka teksti muuttuisi, sen numero, teksti ennen ja teksti jälkeen — katso [Esikatselu ennen kirjoittamista](invocation.md#esikatselu-ennen-kirjoittamista).

| Taso | Mitä virhetulosteeseen lisätään |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles cleaned, M removed -> <destination>` tai `<path>: no mention to remove -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — luetun tiedoston muoto, merkistökoodaus, merkki ja rivinvaihdot palautettuina ennalleen |
| 3 | `<path>: N bytes read, M written`, sitten **jokainen huomautus luettaessa** — katso [Käynnistys](invocation.md#huomautukset-luettaessa) |

`N` laskee kappaleet, joiden teksti muuttui, `M` ne, jotka poisto
tyhjensi kokonaan ja jotka siksi poistuivat tiedostosta.

### JSON-muodossa

Valitsimella [`--format json`](invocation.md#skriptin-luettava-tuloste) kunkin tiedoston objekti sisältää kentät `counts.cleaned` (uudelleen kirjoitetut tekstit) ja `counts.removed` (poistetut kappaleet); **valitsimella `--dry-run`** avain `changes` luettelee lisäksi muuttuneet kappaleet, kuten tekstimuodon vakiotuloste. Koko taulukko on sivulla [Käynnistys](invocation.md#mitä-kukin-alikomento-kirjoittaa-kenttään-counts).

## Paluuarvot

Työkalun yleiset: `0`, jos jokainen tiedosto kirjoitettiin, `2`, jos yhtäkään ei, `3`, jos
vain osa, `1` käyttövirheessä.

## Virheet

Tällä alikomennolla ei ole omia virheitä: se ei ota jäsennettävää
arvoa, eikä mikään teksti voi saada sitä hylkäämään tiedostoa. Epäonnistua
voi lukeminen tai kirjoittaminen, kuten muillakin.

**Mitään ei kirjoiteta ilman jotakin kolmesta kohdevalitsimesta**, ja niiden virheet
ovat yhteisiä kaikille kirjoittaville alikomennoille:
katso [Käynnistys](invocation.md#kohde).
