# `transform`

<!-- machine-translated -->
> Tämä sivu on konekäännös, eikä kukaan tämän kielen lukija ole vielä tarkistanut sitä.

```
subedit-cli transform --first <index>=<time> --last <index>=<time>
                      (--output FILE | --output-dir DIR | --in-place)
                      [--dry-run]
                      [--recursive]
                      <file>...
```

Korjaa tiedoston **jokaisen** kohdan kahden pisteen avulla, joiden oikea arvo
tiedetään. Tätä toimintoa tarvitaan, kun tekstitys valuu
— oikein alussa, väärin lopussa — eikä [`shift`](shift.md), joka siirtää kaikkea
saman verran, voi tehdä mitään.

Luetun tiedoston muoto **säilytetään**: muodon vaihtaminen on alikomennon
[`convert`](convert.md) tehtävä.

<!-- example: subedit-cli transform --help -->
```console
$ subedit-cli transform --help
Correct every position from two points known to be right
Usage: subedit-cli transform [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to transform

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --first TEXT REQUIRED       Earlier reference, as <index>=<time>: 1=00:00:01.000
  --last TEXT REQUIRED        Later reference, as <index>=<time>: 3=00:00:10.000
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
| `--first` | **kyllä** | viitekohta, `<index>=<time>` | — |
| `--last` | **kyllä** | viitekohta, `<index>=<time>` | — |
| `--output` / `--output-dir` / `--in-place` | **yksi kolmesta**, paitsi valitsimen `--dry-run` kanssa | katso [Käynnistys](invocation.md#kohde) | — |
| `--dry-run` | ei | lippu: laskee ja kertoo, ei kirjoita mitään — katso [Esikatselu ennen kirjoittamista](invocation.md#esikatselu-ennen-kirjoittamista) | pois |

Lyhyttä muotoa ei ole: nämä kaksi valitsinta kirjoitetaan aina
kokonaan.

## Viitekohdan kirjoittaminen

Viitekohta nimeää kappaleen ja kertoo, mihin sen **alun** on nyt osuttava:

```
<index>=<time>
```

**Indeksi lasketaan ykkösestä**, kuten se näytetään ja kuten SubRip-tiedosto
sen numeroi. `0` hylätään, samoin viimeisen kappaleen ylittävä
indeksi.

**Aika kirjoitetaan kuten alikomennossa [`shift`](shift.md)** — sekunteina tai
aikaleimana, desimaalierottimena piste, tarkkuutena millisekunti:

| Muoto | Esimerkit |
| :---- | :------- |
| sekunnit | `10`, `2.999` |
| aikaleima | `00:00:10.000`, `01:39:37.040` |

`--first 3=10` ja `--first 3=00:00:10.000` sanovat saman.

`--first` ja `--last` nimeävät kaksi pistettä, eivät väliä. Niiden nimet kertovat
odotetun käytön — yksi viitekohta tiedoston alussa, toinen lopussa — eivätkä
rajoitusta: niiden vaihtaminen keskenään antaa saman tuloksen, ja **kaikki
kahden viitekohdan ulkopuolella seuraa samoin kuin se, mikä on niiden välissä**.

## Mitä muista kohdista tulee

Kaksi viitekohtaa määräävät skaalauskertoimen, ja jokainen kohta seuraa sitä:

```
t′ = y₁ + (t − x₁) × r      with r = (y₂ − y₁) / (x₂ − x₁)
```

`x₁`, `x₂` ovat kahden nimetyn kappaleen nykyiset alut, `y₁`, `y₂`
pyydetyt ajat. Kerroin pidetään tarkkana rationaalilukuna, ja **pyöristys
tehdään vain kerran**, millisekuntiin; juuri se saa kaksi
viitekohtaa osumaan **täsmälleen** pyydettyihin aikoihin eikä vain tuhannesosan
tarkkuudella (katso [ADR 0013](../../../../adr/0013-mise-a-l-echelle-exacte-des-positions.md)).

Kukin kappale pysyy ruudulla **suhteessa** siihen, kuinka kauan se siellä oli:
loppu seuraa samaa skaalaa kuin alku. Muunnos ei ole
siirto, eikä sillä ole siirron ominaisuutta.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,001 --> 00:00:07,000\nDeux.\n\n3\n00:00:09,000 --> 00:00:11,000\nTrois.\n\n' > a.srt; subedit-cli transform --first 1=00:00:01.000 --last 3=00:00:10.000 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,001 --> 00:00:07,000\nDeux.\n\n3\n00:00:09,000 --> 00:00:11,000\nTrois.\n\n' > a.srt; subedit-cli transform --first 1=00:00:01.000 --last 3=00:00:10.000 --output b.srt a.srt; cat b.srt
a.srt: 3 subtitles transformed onto 1=00:00:01.000 and 3=00:00:10.000 -> b.srt
1
00:00:01,000 --> 00:00:03,250
Un.

2
00:00:05,501 --> 00:00:07,750
Deux.

3
00:00:10,000 --> 00:00:12,250
Trois.
```

Ensimmäinen viitekohta ei liiku, kolmas siirtyy kohdasta 9 s kohtaan 10 s: kerroin on
`9/8`, ja toinen kappale — jota kukaan ei nimennyt — siirtyy arvosta
`5,001` arvoon `5,501`, eli `1000 + 4001 × 9/8` kerran pyöristettynä.

## Kaksi viitekohtaa, jotka eivät kelpaa

| Tilanne | Milloin se havaitaan | Koodi |
| :-------- | :------------- | :--- |
| `--first` ja `--last` nimeävät saman indeksin | ennen mitään lukemista | `1` |
| indeksi ylittää viimeisen kappaleen | kun tiedosto avataan | `2` tai `3` |
| kaksi nimettyä kappaletta alkavat samalla hetkellä | kun tiedosto avataan | `2` tai `3` |

Kaikki kolme hylätään samasta syystä: `x₂ − x₁` on nolla, eivätkä kaksi
yhtyvää pistettä määritä suoraa. Tulokseksi puettu nollalla jakaminen
olisi hylkäämistä huonompi.

Ensimmäinen tapaus on **käyttövirhe** — vika on kahdessa valitsimessa tiedostoista
riippumatta, joten tarvitaan yksi viesti eikä yksi tiedostoa kohden. Kaksi
muuta riippuvat avatusta tiedostosta, ja siksi ne lasketaan käsittelyn
epäonnistumisiksi.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n' > a.srt; subedit-cli transform --first 2=1.000 --last 2=4.000 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n' > a.srt; subedit-cli transform --first 2=1.000 --last 2=4.000 --output b.srt a.srt; echo "code=$?"
--first and --last both name subtitle 2, and two references on one subtitle define no transform
code=1
```

Rajojen ulkopuolinen indeksi hylätään **rajan nimeten**, mikä kertoo
samalla, mitä olisi voinut kirjoittaa:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,000 --> 00:00:07,000\nDeux.\n\n' > a.srt; subedit-cli transform --first 1=1.000 --last 9=10.000 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,000 --> 00:00:07,000\nDeux.\n\n' > a.srt; subedit-cli transform --first 1=1.000 --last 9=10.000 --output b.srt a.srt; echo "code=$?"
a.srt: subtitle 9 is past the end: the file holds 2 subtitles
code=2
```

## Alkukohtaa edeltävä kohta hylätään

Kuten alikomennossa [`shift`](shift.md) ja samasta syystä: negatiivinen kohta
kirjoitettaisiin `-00:00:01,000`, muoto, jota mikään yhdeksästä muodosta ei määrittele eikä mikään
lukija hyväksy. Tapaus syntyy tässä, kun kappale on **ennen
ensimmäistä viitekohtaa** ja skaalaus työntää sen nollan ohi. Mitään ei
kirjoiteta, koodi on `2`, ja viesti nimeää virheellisen kappaleen.

## Tuloste

**Vakiotuloste** — ei mitään: tulos on kirjoitettu tiedosto. Valitsimella `--dry-run` ei myöskään mitään: tiedostoa ei ole, ja kerrontarivi kertoo sen — katso [Esikatselu ennen kirjoittamista](invocation.md#esikatselu-ennen-kirjoittamista).

| Taso | Mitä virhetulosteeseen lisätään |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles transformed onto 1=00:00:01.000 and 3=00:00:10.000 -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — luetun tiedoston muoto, merkistökoodaus, merkki ja rivinvaihdot palautettuina ennalleen |
| 3 | `<path>: N bytes read, M written`, sitten **jokainen huomautus luettaessa** — katso [Käynnistys](invocation.md#huomautukset-luettaessa) |

Viitekohdat kirjoitetaan takaisin desimaalipisteellä, kirjoitettiinpa ne miten tahansa.

### JSON-muodossa

Valitsimella [`--format json`](invocation.md#skriptin-luettava-tuloste) kunkin tiedoston objekti sisältää kentän `counts.subtitles` (tiedoston kaikki kappaleet). Koko taulukko on sivulla [Käynnistys](invocation.md#mitä-kukin-alikomento-kirjoittaa-kenttään-counts).

## Paluuarvot

Työkalun yleiset: `0`, jos jokainen tiedosto kirjoitettiin, `2`, jos yhtäkään ei, `3`, jos
vain osa, `1` käyttövirheessä.

## Virheet

| Mikä sen aiheuttaa | Viesti |
| :------------------ | :------ |
| viitekohta ilman merkkiä `=` | `"…" is not a reference: write it <index>=<time>, as in 3=00:00:10.000` |
| viitekohdasta puuttuu toinen puolisko | `"…" is missing one of its two halves: write it <index>=<time>, as in 3=00:00:10.000` |
| indeksi `0` | `"0" is not a subtitle number: subtitles are counted from 1, as the file shows them` |
| indeksi, joka ei ole kokonaisluku | `"…" is not a subtitle number: expected a whole number, counted from 1 as the file shows them` |
| virheellinen aika | samat kuin alikomennolla [`shift`](shift.md) |
| `--first` ja `--last` samalla indeksillä | `--first and --last both name subtitle N, and two references on one subtitle define no transform` |
| rajojen ulkopuolinen indeksi | `<path>: subtitle N is past the end: the file holds M subtitles` |
| kaksi yhtä aikaa alkavaa kappaletta | `<path>: subtitles N and M start at the same moment, so they define no transform` |
| alkukohtaa edeltävä kohta | `<path>: subtitle N would land before the origin, which no subtitle file can hold` |

**Mitään ei kirjoiteta ilman jotakin kolmesta kohdevalitsimesta**, ja niiden
virheet ovat yhteisiä kaikille kirjoittaville alikomennoille:
katso [Käynnistys](invocation.md#kohde).
