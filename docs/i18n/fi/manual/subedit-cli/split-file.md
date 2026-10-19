# `split-file`

<!-- machine-translated -->
> Tämä sivu on konekäännös, eikä kukaan tämän kielen lukija ole vielä tarkistanut sitä.

```
subedit-cli split-file --at N --head FILE --tail FILE
                       [--dry-run] [--sort] [--disjoint]
                       <file>
```

Leikkaa tiedoston **kahtia**: alkuosaksi ja loppuosaksi. Se on ikkunan `Split Project…` — [sama moottori](../subedit-gui/operations.md#split-project),
samat laskurit, sama hylkäys — ja **alikomennon [`append`](append.md) täsmällinen käänteistoiminto**: loppuosan lisääminen
alkuosan perään palauttaa lähtötiedoston.

- **`--at N` on loppuosan ensimmäisen kappaleen numero** sellaisena kuin ikkunan taulukko
  sen numeroi: ykkösestä alkaen. `--at 3` laittaa kappaleet 1 ja 2 alkuosaan ja kolmannen sekä loput
  loppuosaan. Arvo on **toisesta viimeiseen**: leikkaus tarvitsee kappaleen kummallekin puolelle.
- **Loppuosa tuodaan takaisin alkukohtaan**: sitä siirretään alkuosaan jäävän viimeisen kappaleen lopun verran taaksepäin,
  ja sen kohdat säilyttävät järjestyksensä ja välinsä. Juuri se tekee alikomennosta `split-file` alikomennon `append` käänteistoiminnon.
- **Molemmat puolikkaat säilyttävät luetun tiedoston muodon, merkistökoodauksen, tavujärjestysmerkin ja rivinvaihdot**, ja
  kirjoitettaessa ne numeroidaan uudelleen ykkösestä alkaen, kuten mikä tahansa takaisin kirjoitettu tiedosto.
- **Kaksi tulostetta, siis oma kohde**: `--head` ja `--tail`, **molemmat pakollisia**. Valitsimia `--output`,
  `--output-dir`, `--in-place` ja `--recursive` ei ole alikomennolla `split-file`, ja ne hylätään kuten mikä tahansa tuntematon
  valitsin, koodi `1`: ei ole hakemistoa, johon kaksi nimettyä tiedostoa jaettaisiin, eikä syötettä, jonka päälle kirjoitettaisiin.

<!-- example: subedit-cli split-file --help -->
```console
$ subedit-cli split-file --help
Cut a subtitle file in two, a head and a tail
Usage: subedit-cli split-file [OPTIONS] file

Positionals:
  file TEXT REQUIRED          Subtitle file to cut

Options:
  -h,--help                   Print this help message and exit
  --at N                      Number of the first subtitle of the tail, as the table numbers them
  --head FILE                 File to write the subtitles before the cut to
  --tail FILE                 File to write the subtitles from the cut on to
  --dry-run                   Work out and say what would be written, and write nothing
  --sort                      Put the subtitles in order of their start before cutting; --at counts in that order
  --disjoint                  Cut overlapping subtitles so that no range overlaps another, before cutting
```

## Argumentit ja valitsimet

| Valitsin | Pakollinen | Arvot | Oletus |
| :----- | :----- | :------ | :----- |
| `<file>` | kyllä | **yksi ainoa** polku | — |
| `--at` | kyllä | kokonaisluku kahdesta tiedoston kappaleiden määrään | — |
| `--head`, `--tail` | **kyllä, molemmat**, paitsi valitsimen `--dry-run` kanssa | tiedostopolut; niiden hakemisto luodaan, jos se puuttuu | — |
| `--dry-run` | ei | lippu: laskee ja kertoo, ei kirjoita mitään — katso [Esikatselu ennen kirjoittamista](invocation.md#esikatselu-ennen-kirjoittamista) | pois |
| `--sort` | ei | lippu: järjestää kappaleet alkujensa mukaan **ennen leikkaamista**; `--at` laskee tässä järjestyksessä | pois |
| `--disjoint` | ei | lippu: leikkaa päällekkäiset kappaleet **ennen leikkaamista**; `--at` laskee jäljelle jääneistä, katso [Aikavälien päällekkäisyyksien poistaminen](#aikavälien-päällekkäisyyksien-poistaminen) | pois |

**Valitsimella `--dry-run` kohdetta ei vaadita**: kumpaakaan ei tarvitse antaa, mutta ei myöskään vain toista —
`--head` ja `--tail` kuuluvat yhteen. Annetut arvioidaan samoin kuin ilman valitsinta `--dry-run`.

**Kumpikaan kohde ei voi olla syöte, eivätkä ne voi olla sama tiedosto**: luettavana olevan päälle kirjoittaminen
tai loppuosan kirjoittaminen alkuosan päälle hylätään ennen kuin mitään luetaan, koodi `1`.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n3\n00:00:06,000 --> 00:00:07,000\ntrois\n\n4\n00:00:08,000 --> 00:00:09,000\nquatre\n\n' > film.srt; subedit-cli split-file --at 3 --head tete.srt --tail queue.srt film.srt; cat queue.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n3\n00:00:06,000 --> 00:00:07,000\ntrois\n\n4\n00:00:08,000 --> 00:00:09,000\nquatre\n\n' > film.srt; subedit-cli split-file --at 3 --head tete.srt --tail queue.srt film.srt; cat queue.srt
film.srt: split at subtitle 3: 2 subtitles in the head, 2 subtitles in the tail -> tete.srt, queue.srt
1
00:00:02,000 --> 00:00:03,000
trois

2
00:00:04,000 --> 00:00:05,000
quatre
```

Toinen kappale, joka jää alkuosaan, päättyy kohtaan 4 s: loppuosa siirtyy 4 s taaksepäin, ja kolmas kappale,
joka alkoi kohdasta 6 s, alkaa kohdasta 2 s.

## Leikkaaminen aikajärjestyksessä

`--at` numeroi kappaleet **siinä järjestyksessä kuin tiedosto ne antaa**. Tiedostossa, jonka järjestys on rikki, alkuosa ei siis ole ajallisesti
aikaisempi puolisko. `--sort` järjestää kappaleet alkujensa mukaan **ennen leikkauksen arviointia** — vakaasti, kuten [`sort`](sort.md) tekee —, ja
`--at` laskee sitten tässä järjestyksessä.

Ei koskaan oletuksena. Valitsimen kanssa tiedosto kertoo kerronnan ensimmäisellä tasolla, kuinka monta kappaletta siirtyi (`<path>: N subtitles moved`
tai `<path>: already in order`), ja `--format json` -tietueessa on laskuri `moved`, **joka puuttuu ilman valitsinta**.

## Aikavälien päällekkäisyyksien poistaminen

Ei koskaan oletuksena. Valitsin tekee saman kuin [`disjoint`](disjoint.md) **sen oletusasetuksin** ennen leikkauksen arviointia, järjestämisen jälkeen: `--at` laskee siitä, mitä tämä antaa. Se tarvitsee järjestyksessä olevan tiedoston: muuten se hylätään, ja `--sort` — joka tulee ensin — järjestää sen etukäteen. Tiedosto kertoo sen kerronnan ensimmäisellä tasolla (`<path>: N subtitles split, M merged`), ja `--format json` -tietueessa ovat laskurit `split` ja `merged`, **jotka puuttuvat ilman valitsinta**.

## Kun leikkaus hylätään

**Kahta puoliskoa, jotka ovat päällekkäin leikkauskohdassa, ei leikata.** Jos jokin loppuosan kappale alkaisi ennen
jäljelle jäävän viimeisen kappaleen loppua, siirto veisi sen ennen videon alkua — negatiivinen kohta,
jota mikään muoto ei kirjoita. `split-file` kieltäytyy **nimeten kappaleen** ikkunan sanoin:

```text
o.srt: Cannot split at subtitle 3: subtitle 3 would fall before the start of the video. Cut somewhere else.
```

Koodi on `2`, eikä **mitään kirjoiteta**. Leikkaa muualta. Osuminen täsmälleen alkuun on sallittua: nolla on
kohta.

**Tiedoston ulkopuolinen `--at` hylätään samoin**, kun tiedosto on luettu: `--at 1` — ensimmäinen kappale ei voi
aloittaa loppuosaa, koska mitään ei jäisi jäljelle — ja viimeisen kappaleen ylittävä `--at`, joka jättäisi loppuosan
tyhjäksi. Yhden kappaleen tiedostoa ei voi leikata.

## Tuloste

**Vakiotuloste** — ei mitään: tulos on kaksi kirjoitettua tiedostoa.

| Taso | Mitä virhetulosteeseen lisätään |
| :----- | :------------------------------------- |
| 1 | `<path>: split at subtitle N: H subtitles in the head, T subtitles in the tail -> <head>, <tail>`; valitsimella `--dry-run` merkintä `(dry run, nothing written)` merkinnän `-> …` sijaan |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — luetun tiedoston muoto, merkistökoodaus, merkki ja rivinvaihdot palautettuina ennalleen |
| 3 | `<path>: N bytes read, H and T written`, sitten **jokainen huomautus luettaessa** — katso [Käynnistys](invocation.md#huomautukset-luettaessa) |

### JSON-muodossa

Valitsimella [`--format json`](invocation.md#skriptin-luettava-tuloste) **yksi ainoa objekti**, jonka `file` on leikattu tiedosto.
**`destination` on alkuosa** — tiedosto, joksi syöte muuttuu — ja **`tail` loppuosa**, polku tai `null`
valitsimella `--dry-run` kuten `destination`. Laskurit `counts`: `subtitles` (luetun tiedoston kappaleet), `head` ja `tail` (kummankin
puoliskon kappaleet).

## Paluuarvot

| Koodi | Milloin |
| :--- | :---- |
| `0` | molemmat tiedostot on kirjoitettu — tai laskettu, valitsimella `--dry-run` |
| `1` | käyttövirhe: ei valitsimia `--head` ja `--tail`, valitsin, jota alikomennolla `split-file` ei ole, kohde, joka on syöte tai toinen kohde, `--at`, joka ei ole positiivinen kokonaisluku |
| `2` | tiedostoa ei voitu lukea, **leikkaus hylätään** (tiedoston ulkopuolella tai päällekkäisyys), tai kohdetta ei voitu kirjoittaa. Koodia `3` ei ole: käsitellään vain yksi tiedosto |

**Alkuosa kirjoitetaan ensin**: levy, joka kieltäytyy loppuosasta, jättää alkuosan jälkeensä, ja koodi on `2`.

## Virheet

| Mikä ne aiheuttaa | Viesti |
| :------------------- | :------ |
| ei valitsinta `--head` eikä `--tail` | `no destination given: split-file writes two files, use --head and --tail` |
| vain toinen kahdesta, valitsimella `--dry-run` | `--head and --tail go together: give both, or neither with --dry-run` |
| molemmat kohteet ovat sama tiedosto | `<path>: named as both the head and the tail` |
| kohde on syöte | `<path>: would be written over the input <input>` |
| `--at` tiedoston ulkopuolella | `<file>: --at 9: the file holds 3 subtitles, and the tail can start from 2 to 3` |
| yhden kappaleen tiedosto | `<file>: a file of 1 subtitle cannot be split: a cut needs a subtitle on each side` |
| päällekkäisyys | `<file>: Cannot split at subtitle N: subtitle M would fall before the start of the video. Cut somewhere else.` |
| tiedosto puuttuu, on lukukelvoton tai tuntematonta muotoa | `<path>: does not exist`… — samat kuin kaikkialla |
