# `convert`

<!-- machine-translated -->
> Tämä sivu on konekäännös, eikä kukaan tämän kielen lukija ole vielä tarkistanut sitä.

```
subedit-cli convert --to <format>
                    [--line-endings unix|windows|mac] [--to-encoding NAME]
                    [--bom | --no-bom] [--sort] [--disjoint]
                    [--join-repeats] [--repair]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    <file>...
```

Kirjoittaa kunkin tiedoston pyydettyyn muotoon ja rakenteeseen. **Ei koskaan muuta
syötettä**, ellei `--in-place` nimenomaisesti pyydä sitä.

<!-- example: subedit-cli convert --help -->
```console
$ subedit-cli convert --help
Write a subtitle file out in another format or shape
Usage: subedit-cli convert [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to convert

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --to TEXT:{srt,vtt,subviewer2,ssa,ass,microdvd,mpl2,tmplayer,lrc} REQUIRED
                              Format to write
  --line-endings TEXT:{unix,windows,mac}
                              Line endings to write; the source's by default
  --to-encoding NAME          Encoding to write; the source's by default
  --frame-rate RATE           Frame rate of a file counted in frames: 25, 23.976
  --bom                       Write a byte order mark
  --no-bom                    Write no byte order mark
  --sort                      Put the subtitles in order of their start before writing
  --disjoint                  Cut overlapping subtitles so that no range overlaps another, before writing
  --join-repeats              Join the subtitles that repeat each other into one, before writing
  --repair                    Join the repeats, sort, then make the ranges disjoint, before writing
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
| `--to` | **kyllä** | nimi alla olevasta taulukosta, eikä mitään muuta | — |
| `--line-endings` | ei | `unix`, `windows` tai `mac` | luetun tiedoston rivinvaihdot |
| `--to-encoding` | ei | mikä tahansa ICU:n kirjoittama merkistökoodaus, paitsi ne, jotka kirjoittavat oman merkkinsä | luetun tiedoston merkistökoodaus |
| `--bom` / `--no-bom` | ei | liput, jotka sulkevat toisensa pois | mitä luetussa tiedostossa oli |
| `--sort` | ei | lippu: järjestää kappaleet alkukohtiensa järjestykseen **ennen kirjoittamista**, katso [Järjestäminen kirjoitettaessa](#järjestäminen-kirjoitettaessa) | pois |
| `--join-repeats` | ei | lippu: yhdistää saman tekstin peräkkäiset palat yhdeksi kappaleeksi **ennen mitään muuta**, katso [Aikavälien päällekkäisyyksien poistaminen](#aikavälien-päällekkäisyyksien-poistaminen) | pois |
| `--repair` | ei | lippu: sama kuin `--join-repeats --sort --disjoint`, tässä järjestyksessä | pois |
| `--disjoint` | ei | lippu: leikkaa päällekkäiset kappaleet **ennen kirjoittamista**, katso [Aikavälien päällekkäisyyksien poistaminen](#aikavälien-päällekkäisyyksien-poistaminen) | pois |
| `--output` / `--output-dir` / `--in-place` | **yksi kolmesta**, paitsi valitsimen `--dry-run` kanssa | katso [Käynnistys](invocation.md#kohde) | — |
| `--dry-run` | ei | lippu: laskee ja kertoo, ei kirjoita mitään — katso [Esikatselu ennen kirjoittamista](invocation.md#esikatselu-ennen-kirjoittamista) | pois |

`mac` tarkoittaa pelkkää rivinpalautusmerkkiä (`\r`), klassisen Mac OS:n rivinvaihtoa.

## Muodot, jotka `--to` hyväksyy

| Nimi | Muoto | Kirjoitettava tiedostopääte |
| :-- | :----- | :--------------- |
| `srt` | SubRip | `.srt` |
| `vtt` | WebVTT | `.vtt` |
| `subviewer2` | SubViewer 2.0 | `.sub` |
| `ssa` | Sub Station Alpha | `.ssa` |
| `ass` | Advanced SSA | `.ass` |
| `mpl2` | MPL2 | `.txt` |
| `microdvd` | MicroDVD | `.sub` |
| `tmplayer` | TMPlayer | `.txt` |
| `lrc` | LRC | `.lrc` |

**Nimi eikä tiedostopääte.** Kaksi päätettä nimeää kumpikin kaksi
muotoa — `.sub` on myös MicroDVD:n, `.txt` sekä MPL2:n että
TMPlayerin — joten päätteitä hyväksyvä valitsin ei tietäisi, mitä
niistä pyydetään. Yllä olevat nimet ovat yksiselitteisiä ja vastaavat
päätettä aina, kun päätekin on yksiselitteinen.

**Luettelo on suljettu.** Se on Gaupolin luettelo, ja kaikki yhdeksän ovat siinä.

## Kaksi muotoa, joissa ei ole loppukohtaa

**TMPlayer ja LRC kirjoittavat vain yhden kohdan riviä kohden**, sen alun. Tämä
näkyy molemmissa suunnissa.

**Luettaessa** loput päätellään: rivin loppu on seuraavan rivin alku,
ja viimeinen saa viisi sekuntia. Näin Gaupol tekee.
Me myös kerromme sen huomautuksella, jonka `--verbose` näyttää kolmella
alikomennolla:

```
file.lrc: carries no end times; each one was taken from the next start, settled by the reader
```

**Kirjoitettaessa** loppuja ei kirjoiteta, koska niille ei ole paikkaa.
Muuntaminen jompaankumpaan näistä muodoista menettää ne siis, eikä edestakainen muunnos
löydä niitä uudelleen — se päättelee ne uudelleen.

**LRC menettää vielä yhden asian**: sillä ei ole tapaa ilmaista rivinvaihtoa.
Kahdelle riville jaettu repliikki kirjoitetaan yhdelle, ja rivit yhdistetään
välilyönnillä. Tätäkään menetystä ei voi korjata.

Kummallakaan ei ole sanastoa kursiiville.

## `--frame-rate` ja ainoa ruutuina laskeva muoto

**MicroDVD ei laske sekunteina**: sen kohdat *ovat* ruutujen
numeroita, eikä yksikään MicroDVD-tiedosto ilmoita nopeutta, jolla ne
laskettiin. `--frame-rate` antaa sen, ja se toimii molempiin suuntiin: se on
nopeus, jolla ruutuina laskettu `.sub` luetaan, ja nopeus, jolla
muunnos MicroDVD-muotoon laskee ruudut.

| Valitsin | Pakollinen | Arvot | Oletus |
| :----- | :----- | :------ | :----- |
| `--frame-rate` | ei | nopeus ruutuina sekunnissa: `25`, `23.976`, `29.97` | katso alla |

**Luettaessa ilman valitsinta `--frame-rate` tiedosto luetaan nopeudella 23,976** — Gaupolin
arvo — ja taso `-vvv` kertoo sen:
`counts in frames and states no rate; it was read at`. Se on työkalun ainoa kohta, jossa jokainen näytetty kohta
perustuu oletukseen, jota tiedosto ei voi vahvistaa.

**Kirjoitettaessa ilman valitsinta `--frame-rate` on kolme tapausta ja kolme vastausta:**

| Lähtötiedosto | Mitä tapahtuu |
| :------------------- | :-------------- |
| on jo MicroDVD-muodossa | nopeus, jolla se luettiin, kirjoittaa sen takaisin, ja ruudut palaavat identtisinä |
| on aikaperusteinen, ja sen kohdat osuvat ruudukolle | ruudukko otetaan, ja `-vv` kertoo, mikä |
| on aikaperusteinen ilman ruudukkoa | **hylkäys**, koodi 2: `writing frames needs a frame rate, and the positions fall on no grid to take one from — give --frame-rate` |

Hylkäys on tarkoituksellinen. Ainoa jäljellä oleva keino olisi keksiä luku, ja
tiedoston jokainen rivi siirtyisi sen verran kuin luku olisi väärin.

## Mitä muunnos menettää

**Muuntaminen toiseen muotoon ei ole koskaan häviötöntä**, ja se, mikä ei säily,
riippuu muotoparista:

| Mikä ei pääse läpi | Miksi |
| :--------------------- | :------- |
| otsake | sillä on merkitystä vain omassa muodossaan: `[INFORMATION]` ei ole WebVTT-otsake |
| muodolle ominaiset tiedot | SubRipin koordinaateilla, WebVTT-vihjeen tunnisteella ja asetuksilla ei ole vastinetta muualla |
| tarkkuus | SubViewer 2, kumpikin Sub Station Alpha ja LRC kirjoittavat sadasosan tarkkuudella, MPL2 kymmenesosan, TMPlayer sekunnin ja MicroDVD ruudun tarkkuudella; `01:00:00,017` palaa muodossa `01:00:00,020` |
| rivin loppu | TMPlayerissa ja LRC:ssä ei ole sille kenttää; seuraava lukukerta päättelee sen uudelleen |
| rivinvaihdot | vain LRC: sen rivit mahtuvat yhdelle riville, ja rivit liitetään yhteen välilyönnillä |
| tagit, joita kohdemuoto ei osaa kirjoittaa | WebVTT:ssä ei ole tagia `<font color>`, TMPlayerissa ja LRC:ssä ei ole mitään |
| asettelu | `{\pos(x,y)}`, `{\an8}`, `<v Marie>`, `<ruby>`: ne säilyvät edestakaisessa muunnoksessa omassa muodossaan ja katoavat muunnettaessa |

**Tiedoston kirjoittaminen takaisin omaan muotoonsa ei menetä mitään** — se on takuu,
jonka varassa kaikki muu on, ja `--to srt` antaa tiedostolle, jonka pääte on `.srt`, takaisin samat tavut. Mitään
ei edes pureta: teksti pysyy luettuna raakamerkkijonona.

## Tagit muunnetaan, niitä ei kopioida

**Kukin muoto kirjoittaa kursiivin omalla tavallaan**, ja muuntaminen kääntämättä
jättäisi tagin `{\i1}` tiedostoon, jonka pääte on `.srt`: tekstiä, jonka käyttäjä näkee,
tiedostossa, joka ei koskaan osaa tulkita sitä. Läpi pääsee **kuusi
asiaa** — lihavointi, kursiivi, alleviivaus, väri, fontti ja koko:

| Muoto | Lihavointi | Kursiivi | Alleviivaus | Väri | Fontti | Koko |
| :----- | :--: | :------: | :------: | :-----: | :----: | :----: |
| SubRip | `<b>` | `<i>` | `<u>` | `<font color="#RRGGBB">` | — | — |
| WebVTT | `<b>` | `<i>` | `<u>` | — | — | — |
| SubViewer 2 | `<b>` | `<i>` | `<u>` | `<font color="#RRGGBB">` | — | — |
| Sub Station Alpha | `{\b1}` | `{\i1}` | — | `{\c&HBBGGRR&}` | `{\fnNAME}` | `{\fsN}` |
| Advanced SSA | `{\b1}` | `{\i1}` | `{\u1}` | `{\c&HBBGGRR&}` | `{\fnNAME}` | `{\fsN}` |
| MicroDVD | `{Y:b}` | `{Y:i}` | `{Y:u}` | `{C:$BBGGRR}` | `{F:NAME}` | `{S:N}` |
| MPL2 | `\` | `/` | `_` | `{C:$BBGGRR}` | `{F:NAME}` | `{S:N}` |
| TMPlayer | — | — | — | — | — | — |
| LRC | — | — | — | — | — | — |

**Viiva on ilmoitettu menetys.** Värillisen tiedoston `.srt` muuntaminen WebVTT-muotoon poistaa
värin ja kertoo sen; tuotettu tiedosto on WebVTT eikä naamioitu SubRip.

**MicroDVD ja MPL2 eivät voi muotoilla rivin osaa.** Niiden tageilla
ei ole loppua: `{y:i}` kallistaa kaiken sitä seuraavan rivin loppuun asti, ja
`{Y:i}` kappaleen loppuun asti. Rivi, josta vain puolet oli
lihavoitu, kirjoitetaan siis kokonaan ilman lihavointia eikä yli valuvalla lihavoinnilla.

## Mitä muunnos kertoo menettäneensä

**Se kertoo, kun se menettää jotain, ja on hiljaa, kun se ei menetä mitään.** Raportti,
joka ilmestyy joka kutsulla, on raportti, jota kukaan ei lue.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,500\n<i>sur deux</i>\nlignes\n\n' > a.srt; subedit-cli convert --to lrc --output b.lrc a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,500\n<i>sur deux</i>\nlignes\n\n' > a.srt; subedit-cli convert --to lrc --output b.lrc a.srt
a.srt: 1 subtitle written as LRC -> b.lrc
a.srt: ends are not carried by LRC, line breaks were joined in 1 subtitle, 1 tag dropped
```

Rivi näkyy oletustasolla ja jokaisella sitä korkeammalla tasolla.
Kohdat tässä järjestyksessä:

| Kohta | Mitä se kertoo |
| :---- | :----------- |
| `ends are not carried by <format>` | kohdemuoto ei kirjoita loppua |
| `line breaks were joined in N subtitles` | vain LRC: N kappaletta oli usealla rivillä |
| `N tags dropped` | N tagia ei voitu kirjoittaa, asettelu mukaan lukien |
| `the header was dropped` | otsake ei ylitä muodon rajaa |
| `the <format> fields of N subtitles were dropped` | N kappaletta sisälsi muodolleen ominaisia tietoja |
| `positions moved by up to N ms` | suurin matka, jonka kohta siirtyi |

## Merkistökoodauksen vaihtaminen

**`--to-encoding` valitsee kirjoitettavan merkistökoodauksen; `--encoding` kertoo luettavan.**
Yhdessä ne koodaavat tiedoston uudelleen:

```console
$ subedit-cli convert --to srt --encoding cp1252 --to-encoding utf-8 \
      --output-dir sortie film.srt
```

Ilman valitsinta `--to-encoding` tiedosto kirjoitetaan **samalla merkistökoodauksella, jolla se luettiin**,
kuten myös sen rivinvaihdot ja merkki: tiedosto, joka muunnetaan ilman
merkistökoodausta koskevaa ohjetta, antaa takaisin samat merkit samoina tavuina.

**Merkki, jota kohdemerkistökoodaus ei osaa kirjoittaa, pysäyttää tiedoston**, eikä
mitään kirjoiteta. Merkillä `ł` ei ole paikkaa Latin-1:ssä; sen korvaaminen merkillä `?`
menettäisi tekstiä lukemisen ja kirjoittamisen välillä kenenkään huomaamatta.

**Tavujärjestysmerkki on olemassa vain Unicode-merkistökoodauksille.**
`--bom` yhdessä sellaisen valitsimen `--to-encoding` kanssa, jossa merkkiä ei ole, hylätään eikä ohiteta
— katso virhetaulukko alla.

**Ja merkistökoodauksen on nimettävä tavujärjestyksensä.** `UTF-16` ja `UTF-32` ovat
ICU:n tuntemia nimiä, joiden muunnin kirjoittaa **oman merkkinsä**, pyydettiinpä
mitä tahansa: `--no-bom` hyväksyttäisiin silloin mutta jätettäisiin noudattamatta. Molemmat
siis hylätään, ja viesti nimeää tulosteen:

```console
$ subedit-cli convert --to srt --to-encoding UTF-16 --output copie.srt film.srt
"UTF-16" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do
```

`UTF-16LE`, `UTF-16BE`, `UTF-32LE` ja `UTF-32BE` kirjoittavat täsmälleen samat
tavut merkki mukaan lukien — mutta valitsimien `--bom` ja `--no-bom` ohjaamina. Hylkäys
ei siis maksa muuta kuin tarkemman nimen, ja se pätee
sekä lukemiseen että kirjoittamiseen: `--encoding UTF-16` saa saman vastauksen.

## Kohde

Kolme valitsinta ja niiden säännöt ovat yhteisiä kaikille kirjoittaville
alikomennoille: katso [Käynnistys](invocation.md#kohde). Mitään ei kirjoiteta
ilman jotakin niistä.

Alikomennolle `convert` ominaista: **`--output-dir` vaihtaa tiedostopäätteen
muodon mukana.** `convert --to vtt` tiedostolle `a.srt` kirjoittaa tiedoston `a.vtt`, ei koskaan WebVTT:tä
sisältävää tiedostoa `a.srt` — tiedosto, jonka nimi valehtelee, kompastuttaa jokaisen muun työkalun.

## Paikallaan muuntaminen hylätään

`--in-place` yhdessä muotoa vaihtavan valitsimen `--to` kanssa antaa koodin `1` eikä kirjoita
mitään: paikallaan ei ole toista nimeä, joka kantaisi uutta muotoa, ja
tiedosto jäisi tiedostopäätteen alle, jota sen sisältö ei enää oikeuta.

Valitsinta `--in-place` voi silti käyttää **rakenteen vaihtamiseen ilman muotoa** —
esimerkiksi tiedoston `.srt` kirjoittamiseen takaisin samaan muotoon `.srt` toisin rivinvaihdoin.

Hylkäys ratkaistaan **pelkän tiedostopäätteen perusteella** ennen mitään lukemista: käyttövirhe
ei saa koskaan jättää erää puoliksi kirjoitetuksi.

## Rivinvaihdot ja tavujärjestysmerkki

Oletuksena **kirjoitettu tiedosto ottaa sen, mitä luetussa tiedostossa oli**. Malli
säilyttää molemmat luettaessa; niiden pakottaminen oletuksena menettäisi joka
muunnoksessa tarkoituksella säilytetyn tiedon.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli convert --to vtt --output b.vtt a.srt; cat b.vtt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli convert --to vtt --output b.vtt a.srt; cat b.vtt
a.srt: 1 subtitle written as WebVTT -> b.vtt
WEBVTT

00:01.000 --> 00:03.000
Bonjour.
```

## Järjestäminen kirjoitettaessa

`--sort` kirjoittaa tiedoston **alkukohtien järjestyksessä** kohdemuodosta riippumatta. Järjestäminen tehdään **kopiolle**: luettuun tiedostoon ei
kosketa, ja kirjoitettu on sitä, mitä alikomento [`sort`](sort.md) olisi siitä tehnyt — vakaa järjestys: kaksi yhtä aikaa alkavaa kappaletta säilyttää järjestyksensä.

**Ei koskaan oletuksena**: tiedoston järjestys on sen tekijän, niin kauan kuin muuta ei pyydetä. Valitsimen kanssa kukin tiedosto kertoo
ensimmäisellä kerronnan tasolla `<path>: N subtitles moved` tai `<path>: already in order`, ja valitsimen `--format json` tietueessa on
laskuri `moved` — **puuttuu ilman valitsinta**, jotta olemassa olevien skriptien lukeman tiedon rakenne ei muutu.

## Aikavälien päällekkäisyyksien poistaminen

`--disjoint` leikkaa päällekkäiset kappaleet niin, ettei mikään aikaväli enää mene toisen päälle, **samalle kopiolle kuin järjestäminen**: luettuun tiedostoon ei kosketa.

Ei koskaan oletuksena. Valitsin tekee saman kuin [`disjoint`](disjoint.md) **oletusasetuksillaan** ennen kirjoittamista, järjestämisen jälkeen. Se vaatii järjestyksessä olevan tiedoston: muuten se hylätään, ja `--sort` — joka tulee ensin — järjestää tiedoston etukäteen. Tiedosto kertoo sen ensimmäisellä kerronnan tasolla (`<path>: N subtitles split, M merged`), ja valitsimen `--format json` tietueessa ovat laskurit `split` ja `merged`, jotka **puuttuvat ilman valitsinta**.

**`--join-repeats` tulee ennen järjestämistä ja leikkaamista**: peräkkäisiin paloihin leikattu teksti kootaan ensin uudelleen (katso
[`join-repeats`](join-repeats.md)), sitten kappaleet järjestetään ja sitten aikavälien päällekkäisyydet poistetaan. **`--repair` on kaikki kolme kerralla**,
kuten [`repair`](repair.md), leikkaamisen oletusasetuksin. Valitsimella `--join-repeats` kukin tiedosto kertoo
`<path>: N repeated subtitles joined` ensimmäisellä kerronnan tasolla, ja valitsimen `--format json` tietueessa on `counts.joined` — **puuttuu ilman valitsinta**.

## Tuloste

**Vakiotuloste** — ei mitään: tulos on kirjoitettu tiedosto. Valitsimella `--dry-run` ei myöskään mitään: tiedostoa ei ole, ja kerrontarivi kertoo sen — katso [Esikatselu ennen kirjoittamista](invocation.md#esikatselu-ennen-kirjoittamista).

Virhetulosteeseen tason mukaan:

| Taso | Mitä lisätään |
| :----- | :-------------- |
| 1 | `<path>: N subtitles written as WebVTT -> <destination>` |
| 2 | `<path>: SubRip -> WebVTT, UTF-8, no BOM, LF line endings` — nimetty merkistökoodaus on kirjoitettu |
| 3 | `<path>: N bytes read, M written`, sitten **jokainen huomautus luettaessa** — katso [Käynnistys](invocation.md#huomautukset-luettaessa) |

### JSON-muodossa

Valitsimella [`--format json`](invocation.md#skriptin-luettava-tuloste) kunkin tiedoston olio sisältää kentän `counts.subtitles` ja sitten sen, mitä muunnos menetti: `lost_ends` ja `lost_header` (`0` tai `1`), `joined_lines`, `lost_tags`, `lost_fields` ja `furthest_ms` — [menetyslauseen](#mitä-muunnos-kertoo-menettäneensä) kohdat. Täydellinen taulukko on sivulla [Käynnistys](invocation.md#mitä-kukin-alikomento-kirjoittaa-kenttään-counts).

## Paluuarvot

Työkalun yleiset: `0`, jos jokainen tiedosto kirjoitettiin, `2`, jos yhtäkään ei, `3`, jos
vain osa, `1` käyttövirheessä.

## Virheet

| Mikä sen aiheuttaa | Viesti |
| :------------------ | :------ |
| `--bom` yhdessä valitsimen `--no-bom` kanssa | `--bom and --no-bom ask for opposite things; give one or the other` |
| `--in-place`, joka vaihtaa muodon | `--in-place cannot change the format: the file would keep a name its content no longer matches` |
| `--bom` merkistökoodaukselle, jossa sitä ei ole | `<path>: <encoding> has no byte order mark to write` |
| `--to-encoding`, joka nimeää tuntemattoman merkistökoodauksen | `no encoding is named "<name>"` |
| `--to-encoding`, joka nimeää oman merkkinsä kirjoittavan merkistökoodauksen | `"<name>" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do` |
| merkki, joka puuttuu kirjoitettavasta merkistökoodauksesta | `<path>: holds a character the chosen encoding cannot write` |

**Valitsimen `--bom` hylkääminen merkistökoodaukselle, jossa sitä ei ole, ei ole
pikkutarkkuutta.** Tavujärjestysmerkki on olemassa Unicode-merkistökoodauksille eikä
millekään muulle: sen pyytäminen Windows-1252-tiedostolle on sellaisen pyytämistä,
mitä ei ole olemassa. Tiedosto kirjoitettaisiin ilman sitä, eikä kukaan
tietäisi, ettei pyyntöä noudatettu.

Kohteen virheet ovat yhteisiä kaikille kirjoittaville alikomennoille:
katso [Käynnistys](invocation.md#kohde).

Lukuvirheet ovat samat kuin alikomennolla [`inspect`](inspect.md): ne koskevat
syötetiedostoa, ja niillä on samat viestit.
