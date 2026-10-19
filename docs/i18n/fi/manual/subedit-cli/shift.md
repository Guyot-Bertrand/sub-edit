# `shift`

<!-- machine-translated -->
> Tämä sivu on konekäännös, eikä kukaan tämän kielen lukija ole vielä tarkistanut sitä.

```
subedit-cli shift (--by <time> | --to-grid)
                  (--output FILE | --output-dir DIR | --in-place)
                  [--dry-run]
                  [--recursive]
                  <file>...
```

Siirtää tiedoston **jokaista** kohtaa saman verran, jompaankumpaan
suuntaan. Mikään kappale ei pysy ruudulla pidempään eikä lyhyempään:
juuri se tekee siirrosta siirron.

<!-- example: subedit-cli shift --help -->
```console
$ subedit-cli shift --help
Move every position of a file by a fixed amount
Usage: subedit-cli shift [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to shift

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --by TEXT                   Amount to move by: 2.999, -7.001, or 00:00:07.001
  --to-grid                   Move by the amount that puts the positions back on their frame grid
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
| `--by` | **jompikumpi kahdesta** | etumerkillinen kesto, katso alla | — |
| `--to-grid` | **jompikumpi kahdesta** | lippu: määrä mitataan, katso alla | — |
| `--output` / `--output-dir` / `--in-place` | **yksi kolmesta**, paitsi valitsimen `--dry-run` kanssa | katso [Käynnistys](invocation.md#kohde) | — |
| `--dry-run` | ei | lippu: laskee ja kertoo, ei kirjoita mitään — katso [Esikatselu ennen kirjoittamista](invocation.md#esikatselu-ennen-kirjoittamista) | pois |

Luetun tiedoston muoto **säilytetään**: muodon vaihtaminen on alikomennon
[`convert`](convert.md) tehtävä.

## `--to-grid`: määrä mitataan eikä anneta

Tiedosto, jonka kohdat ovat ruudukolla **vakiota lukuun
ottamatta**, on siirretty, ja tuon vakion voi mitata itse kohdista.
`--to-grid` mittaa sen ja käyttää sitä ilman, että kenenkään tarvitsee tietää sitä.

Määrä on **kullekin tiedostolle oma**: kaksi tiedostoa, jotka on siirretty samalta
ruudukolta kahdella eri määrällä, palaavat kumpikin omallaan. Se
kirjoitetaan kerrontaan yhdessä löydetyn ruutunopeuden kanssa.

**Reitti on lyhin.** Yli puolen ruudun päässä lähin ruudukon
viiva on seuraava: korjaus menee eteenpäin sen sijaan, että se siirtyisi
taaksepäin lähes kokonaisen ruudun verran.

<!-- example: k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40 + 7)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > decale.srt; subedit-cli shift --to-grid --output recale.srt decale.srt -->
```console
$ k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40 + 7)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > decale.srt; subedit-cli shift --to-grid --output recale.srt decale.srt
decale.srt: 12 subtitles shifted by -0.007 s onto their 25 fps grid -> recale.srt
```

**Tiedosto, joka ei ole millään tunnetulla ruudukolla, hylätään, ja syy kerrotaan.**
Mitään ei ole palautettavissa, ja kohinasta mitattu vaihe siirtäisi tiedostoa
sattumanvaraisesti. Katso [`inspect`](inspect.md#mitä-frame-rate-grid-kertoo)
nähdäksesi, mitä tiedostosta saadaan, ennen kuin kokeilet toimintoa.

`--by` ja `--to-grid` kertovat kumpikin, kuinka paljon siirretään: niiden antaminen yhdessä
on käyttövirhe, ja niin on kummankin pois jättäminenkin.

**`--to-grid` ei ole [`snap`](snap.md).** Se siirtää koko tiedostoa
saman verran ja säilyttää suhteellisen ajoituksen **täsmälleen**; `snap`
siirtää kutakin kohtaa erikseen ja sulauttaa matkan varrella ne, jotka oli
korjattu käsin. Siistissä tiedostossa molemmat antavat saman
tuloksen; osittaisessa tiedostossa eivät.

## Keston kirjoittaminen

Kaksi muotoa, ja vain kaksi:

| Muoto | Esimerkit |
| :---- | :------- |
| sekunnit | `30`, `2.999`, `-7.001`, `+1.5` |
| aikaleima | `00:00:07.001`, `01:39:37.040`, `-00:00:07.001` |

Molemmat sanovat saman: `--by 7.001` ja `--by 00:00:07.001` tuottavat
saman tiedoston.

**Desimaalierotin on piste**, koska komentorivi puhuu englantia.
Pilkku hylätään eikä sitä tulkita: puolet maailmasta kirjoittaa `7,001`
tarkoittaessaan seitsemäätuhatta yhtä, ja sen lukeminen seitsemäksi sekunniksi olisi arvaus —
väärä arvaus tuolle puolelle.

**Tarkkuus on millisekunti.** Hienojakoisempi hylätään eikä sitä pyöristetä
hiljaa: äänetön pyöristys antaisi tuloksen, joka ei vastaa mitään
pyydetystä.

<!-- example: printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle shifted by -7.001 s -> b.srt
1
00:00:02,999 --> 00:00:04,999
Bonjour.
```

## Siirto ennen alkukohtaa hylätään

Nollan ohi taaksepäin meneminen antaa koodin `2` eikä **kirjoita mitään**. Viesti nimeää
kappaleen, joka ei kestänyt siirtoa, mikä kertoo samalla,
kuinka pitkälle voi mennä.

Syy ei ole periaatteellinen: alkukohtaa edeltävä kohta kirjoitettaisiin
`-00:00:01,000`, muoto, jota mikään yhdeksästä muodosta ei määrittele eikä mikään lukija
hyväksy. Ydin kyllä sallii tällaiset kohdat — niiden hylkääminen tekisi
muokkaustoiminnosta erikoistapauksen — mutta tiedosto on eri asia kuin
muistissa oleva malli.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; echo "code=$?"
a.srt: subtitle 1 would start before the origin, which no subtitle file can hold
code=2
```

## Tuloste

**Vakiotuloste** — ei mitään: tulos on kirjoitettu tiedosto. Valitsimella `--dry-run` ei myöskään mitään: tiedostoa ei ole, ja kerrontarivi kertoo sen — katso [Esikatselu ennen kirjoittamista](invocation.md#esikatselu-ennen-kirjoittamista).

| Taso | Mitä virhetulosteeseen lisätään |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles shifted by 2.999 s -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — luetun tiedoston muoto, merkistökoodaus, merkki ja rivinvaihdot palautettuina ennalleen |
| 3 | `<path>: N bytes read, M written`, sitten **jokainen huomautus luettaessa** — katso [Käynnistys](invocation.md#huomautukset-luettaessa) |

### JSON-muodossa

Valitsimella [`--format json`](invocation.md#skriptin-luettava-tuloste) kunkin tiedoston objekti sisältää kentät `counts.subtitles` (tiedoston kaikki kappaleet) ja `counts.shifted_by_ms` (siirto kokonaisina millisekunteina, **etumerkillisenä**). Koko taulukko on sivulla [Käynnistys](invocation.md#mitä-kukin-alikomento-kirjoittaa-kenttään-counts).

## Paluuarvot

Työkalun yleiset: `0`, jos jokainen tiedosto kirjoitettiin, `2`, jos yhtäkään ei, `3`, jos
vain osa, `1` käyttövirheessä.

## Virheet

| Mikä sen aiheuttaa | Viesti |
| :------------------ | :------ |
| desimaalipilkku | `"…" is not a time: use a decimal point and not a comma, in every language` |
| millisekuntia hienojakoisempi | `"…" is not a time: positions are held to the millisecond, no finer` |
| teksti, joka ei ole kesto | `"…" is not a time: expected seconds like 2.999, or a timestamp like 00:00:07.001` |
| siirto ennen alkukohtaa | `<path>: subtitle N would start before the origin, which no subtitle file can hold` |

**Mitään ei kirjoiteta ilman jotakin kolmesta kohdevalitsimesta**, ja niiden
virheet ovat yhteisiä kaikille kirjoittaville alikomennoille:
katso [Käynnistys](invocation.md#kohde).
