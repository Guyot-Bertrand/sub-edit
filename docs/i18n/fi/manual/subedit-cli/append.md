# `append`

<!-- machine-translated -->
> Tämä sivu on konekäännös, eikä kukaan tämän kielen lukija ole vielä tarkistanut sitä.

```
subedit-cli append --output FILE
                   [--dry-run] [--sort] [--disjoint]
                   <base> <file>...
```

Lisää tiedostoja **ensimmäisen perään** **yhdeksi** tiedostoksi: kaksiosainen elokuva muutettuna
yhdeksi tekstitykseksi. Se on ikkunan `Append File…` — [sama moottori](../subedit-gui/operations.md#append-file),
samat laskurit — toistettuna niin monta kertaa kuin perään lisättäviä tiedostoja on.

- **Ensimmäinen tiedosto on pohja.** Se antaa tulokselle **muotonsa, merkistökoodauksensa,
  tavujärjestysmerkkinsä ja rivinvaihtonsa**.
- **Kahden tiedoston väliin ei lisätä mitään**: kutakin siirretään **sitä edeltävän osan viimeisen kappaleen
  lopun verran**, ja sen kohdat säilyttävät järjestyksensä ja välinsä. Kolmas seuraa toista, ei
  pohjaa: tiedostot seuraavat toisiaan siinä järjestyksessä, jossa ne annetaan.
- **Jokainen perään lisätty tiedosto kulkee muunnoksen läpi** pohjan muotoon: sen tagit muunnetaan,
  ja se, mitä tuo muoto ei pysty kirjoittamaan, kerrotaan tiedosto kerrallaan ikkunan sanoin.
- **Perään lisätyillä tiedostoilla ei ole käännöstä**: yksin avatulla tiedostolla ei ole käännöstä annettavana.
- **Tiedoston voi antaa useita kertoja** tai lisätä itsensä perään: mikään ei kiellä sitä.

**`append` ei ole erä.** N syötettä, **yksi** tuloste: muiden alikomentojen silmukka — yksi tulos
tiedostoa kohden ja epäonnistuminen, joka antaa muiden jatkaa — ei ole tässä järkevä. Siitä seuraa kolme asiaa:

- **kohde on `--output` eikä mikään muu.** Valitsimia `--output-dir` ja `--in-place` ei ole
  alikomennolla `append`, ja ne hylätään kuten mikä tahansa tuntematon valitsin, koodi `1`; samoin `--recursive`: tiedostojen
  järjestys on annettu järjestys, eikä läpikäydyllä hakemistopuulla ole järjestystä;
- **kaikki tai ei mitään**: ensimmäinen tiedosto, jota ei voi lukea, pysäyttää ajon, eikä **mitään kirjoiteta**.
  Kolmiosainen elokuva, josta yksi osa puuttuu, ei ole kaksiosainen elokuva;
- **yksi ainoa rivi ajoa kohden** tulokselle ja JSON-muodossa **yksi ainoa objekti**.

<!-- example: subedit-cli append --help -->
```console
$ subedit-cli append --help
Put subtitle files one after another into a single file
Usage: subedit-cli append [OPTIONS] files...

Positionals:
  files BASE FILE...          The base file, then the files to put after it, in order

Options:
  -h,--help                   Print this help message and exit
  --output FILE               File to write
  --dry-run                   Work out and say what would be written, and write nothing
  --sort                      Put the subtitles in order of their start before writing
  --disjoint                  Cut overlapping subtitles so that no range overlaps another, before writing
```

## Argumentit ja valitsimet

| Valitsin | Pakollinen | Arvot | Oletus |
| :----- | :----- | :------ | :----- |
| `<base> <file>...` | kyllä, **vähintään kaksi** | polut: pohja ja sitten sen perään tulevat, järjestyksessä | — |
| `--output` | **kyllä**, paitsi valitsimen `--dry-run` kanssa | tiedostopolku; sen hakemisto luodaan, jos se puuttuu | — |
| `--dry-run` | ei | lippu: laskee ja kertoo, ei kirjoita mitään — katso [Esikatselu ennen kirjoittamista](invocation.md#esikatselu-ennen-kirjoittamista) | pois |
| `--sort` | ei | lippu: järjestää kokonaisuuden alkujen mukaan **kun kaikki on lisätty perään**, katso [Tuloksen järjestäminen](#tuloksen-järjestäminen) | pois |
| `--disjoint` | ei | lippu: leikkaa päällekkäiset kappaleet **kun kaikki on lisätty ja järjestetty**, katso [Aikavälien päällekkäisyyksien poistaminen](#aikavälien-päällekkäisyyksien-poistaminen) | pois |

**Kohde ei voi olla mikään syötteistä**, ei myöskään pohja: luettavana olevan tiedoston päälle kirjoittaminen
hylätään ennen kuin mitään luetaan, koodi `1`. Olemassa oleva, syötteistä erillinen `--output`
kirjoitetaan yli, kuten kaikkialla.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n' > partie1.srt; printf '1\n00:00:00,500 --> 00:00:01,500\ntrois\n\n' > partie2.srt; subedit-cli append --output film.srt partie1.srt partie2.srt; cat film.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n' > partie1.srt; printf '1\n00:00:00,500 --> 00:00:01,500\ntrois\n\n' > partie2.srt; subedit-cli append --output film.srt partie1.srt partie2.srt; cat film.srt
partie2.srt: appended 1 subtitle
partie1.srt: 3 subtitles from 2 files -> film.srt
1
00:00:01,000 --> 00:00:02,000
un

2
00:00:03,000 --> 00:00:04,000
deux

3
00:00:04,500 --> 00:00:05,500
trois
```

Tiedoston `partie1.srt` viimeinen kappale päättyy kohtaan 4 s; tiedoston `partie2.srt` kappale, joka alkoi kohdasta 0,5 s, alkaa kohdasta
4,5 s.

## Tuloksen järjestäminen

Kutakin tiedostoa siirretään **sitä edeltävän osan lopusta**: järjestys, joka voi rikkoutua, on siis itse tiedostojen järjestys. `--sort`
järjestää tuloksen alkujen mukaan **yhdellä kertaa viimeisen perään lisäyksen jälkeen**, kuten [`sort`](sort.md) tekisi — vakaasti ja muuttamatta mitään
sisällöstä.

Ei koskaan oletuksena. Valitsimen kanssa pohja kertoo sen kerronnan ensimmäisellä tasolla, `<base>: N subtitles moved` tai `<base>: already in order`, ja
`--format json` -tietueessa on laskuri `moved`, **joka puuttuu ilman valitsinta**.

## Aikavälien päällekkäisyyksien poistaminen

Ei koskaan oletuksena. Valitsin tekee saman kuin [`disjoint`](disjoint.md) **sen oletusasetuksin**, kun kaikki on lisätty perään, koko tulokselle. Se tarvitsee järjestyksessä olevan tiedoston: muuten se hylätään, ja `--sort` — joka tulee ensin — järjestää sen etukäteen. Tiedosto kertoo sen kerronnan ensimmäisellä tasolla (`<path>: N subtitles split, M merged`), ja `--format json` -tietueessa ovat laskurit `split` ja `merged`, **jotka puuttuvat ilman valitsinta**.

## Kun muodot eroavat

Tulos on **pohjan muodossa**, olivatpa muiden muodot mitä tahansa. Se, mitä muodon ylitys maksaa,
kerrotaan tiedosto kerrallaan ikkunan sanoin — sama lause kuin komennolla `Append File…` ja
[toisesta muodosta liitettäessä](../subedit-gui/clipboard.md#toisen-muodon-tagit-muunnetaan):

```text
d.ass: appended 1 subtitle; Advanced SSA into SubRip: 1 tag dropped, the header was dropped
```

Se, mitä menetetään, riippuu vain **kunkin** perään lisätyn tiedoston muodosta: Advanced SSA -muotoinen pohja ottaa vastaan
SubRip-tiedoston menettämättä mitään, mutta ei päinvastoin. Valitaksesi tuloksen muodon laita ensimmäiseksi tiedosto,
joka on jo tuossa muodossa, tai muunna jälkikäteen alikomennolla [`convert`](convert.md).

## Tuloste

**Vakiotuloste** — ei mitään: tulos on kirjoitettu tiedosto.

| Taso | Mitä virhetulosteeseen lisätään |
| :----- | :------------------------------------- |
| 1 | yksi rivi kutakin perään lisättyä tiedostoa kohden — `<path>: appended N subtitles`, jonka perään, kun muoto maksoi jotain, `; <source format> into <base format>: <what is lost>` — sitten `<base>: N subtitles from M files -> <destination>` (valitsimella `--dry-run` merkintä `(dry run, nothing written)` merkinnän `-> <destination>` sijaan) |
| 2 | `<base>: SubRip, UTF-8, no BOM, LF line endings kept` — pohjan muoto, merkistökoodaus, merkki ja rivinvaihdot palautettuina ennalleen |
| 3 | `<base>: N bytes read, M written` (kaikki tiedostot yhteensä), sitten **jokainen huomautus luettaessa** — katso [Käynnistys](invocation.md#huomautukset-luettaessa) |

### JSON-muodossa

Valitsimella [`--format json`](invocation.md#skriptin-luettava-tuloste) **yksi ainoa objekti** koko ajolle,
ja sen `file` on **pohja**. Sen `counts`:

| Laskuri | Mitä se kertoo |
| :----- | :----------- |
| `files` | ajon tiedostot pohja mukaan lukien |
| `subtitles` | tuloksen kappaleet |
| `appended` | perään lisätyistä tiedostoista tulevat kappaleet |
| `lost_ends`, `lost_header` | `0` tai `1`: jokin perään lisätyistä tiedostoista menetti sen |
| `joined_lines`, `lost_tags`, `lost_fields` | perään lisättyjen tiedostojen menetysten summa |
| `furthest_ms` | kohdan suurin siirtymä kaikki tiedostot yhteensä |

**Taulukko `inputs`** kulkee laskurien `counts` rinnalla, niiden ulkopuolella: yksi alkio kutakin perään lisättyä tiedostoa kohden, järjestyksessä,
`{"file", "counts"}` — `counts` sisältää laskurin `appended` ja sitten samat menetykset **vain tuon tiedoston osalta**. Siitä
näkee kymmenen tiedoston ajossa, mikä niistä maksoi mitäkin. Kaikkien tiedostojen lukuvaroitukset
kootaan kenttään `warnings`.

Myös epäonnistuminen on objekti, **yksi ainoa**: ensimmäisen tiedoston, jota ei voitu lukea.

## Paluuarvot

| Koodi | Milloin |
| :--- | :---- |
| `0` | tulos on kirjoitettu — tai laskettu, valitsimella `--dry-run` |
| `1` | käyttövirhe: vähemmän kuin kaksi tiedostoa, ei valitsinta `--output`, valitsin, jota alikomennolla `append` ei ole, kohde, joka on yksi syötteistä |
| `2` | **jotakin tiedostoa ei voitu lukea**, tai kohdetta ei voitu kirjoittaa: mitään ei kirjoiteta. Koodia `3` ei ole: ajo, joka kirjoittaa vain yhden tiedoston, joko onnistuu tai ei |

## Virheet

| Mikä ne aiheuttaa | Viesti |
| :------------------- | :------ |
| vain yksi tiedosto annettu | `append needs a base file and at least one file to put after it` |
| ei valitsinta `--output` eikä `--dry-run` | `no destination given: append writes one file, use --output` |
| `--output` nimeää syötteen | `<destination>: would be written over the input <input>` |
| tiedosto puuttuu, on lukukelvoton tai tuntematonta muotoa | `<path>: does not exist`, `<path>: is in no format this tool knows`… — samat kuin kaikkialla |

**Tyhjää tiedostoa ei lisätä perään**: mikään muoto ei lue ilman vähintään yhtä kappaletta, ja tiedosto
hylätään kuten mikä tahansa tiedosto, jota ei voi lukea — `<path>: holds nothing recognisable as a subtitle` —, mikä
pysäyttää ajon eikä kirjoita mitään.
