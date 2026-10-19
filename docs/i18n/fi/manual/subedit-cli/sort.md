# `sort`

<!-- machine-translated -->
> Tämä sivu on konekäännös, eikä kukaan tämän kielen lukija ole vielä tarkistanut sitä.

```
subedit-cli sort (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 <file>...
```

Palauttaa kappaleet **alkukohtiensa järjestykseen**. Mikään muu ei muutu: eivät tekstit, eivät
kohdat eikä tiedoston muoto. Kirjoitettaessa kappaleet **numeroidaan uudelleen** numerosta 1 alkaen,
kuten mikä tahansa takaisin kirjoitettu tiedosto.

**Ydin ei koskaan järjestä omin päin**: tiedoston lukeminen jättää sen järjestyksen ennalleen, ja
[`inspect`](inspect.md#mitä-anomalies-kertoo) ilmoittaa järjestyksen katkeamisesta viestillä
`subtitle N starts before the previous one starts`. `sort` on **tarkoituksella tehty toimenpide** tiedostolle
tai erälle, jonka järjestys on rikki — ja sen voi kumota ikkunassa kuten minkä tahansa toiminnon.

- **Järjestäminen on vakaa**: kaksi kappaletta, jotka alkavat samalla hetkellä, **säilyttävät järjestyksen,
  jonka tiedosto niille antoi**. Kumpikaan ei ole toisen edellä; niiden siirtäminen olisi päätös, jota kukaan
  ei pyytänyt.
- **`sort` ei ota väliä eikä asiakirjaa**: ei ole mitään valittavaa, järjestys on alkukohtien järjestys.

<!-- example: subedit-cli sort --help -->
```console
$ subedit-cli sort --help
Put the subtitles in the order of their start, keeping ties as they are
Usage: subedit-cli sort [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to sort

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
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
| `--output` / `--output-dir` / `--in-place` | **yksi kolmesta**, paitsi valitsimen `--dry-run` kanssa | katso [Käynnistys](invocation.md#kohde) | — |
| `--dry-run` | ei | lippu: laskee ja kertoo, ei kirjoita mitään — katso [Esikatselu ennen kirjoittamista](invocation.md#esikatselu-ennen-kirjoittamista) | pois |

<!-- example: printf '1\n00:00:05,000 --> 00:00:06,000\ncinq\n\n2\n00:00:01,000 --> 00:00:02,000\nun\n\n3\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > desordre.srt; subedit-cli sort --output ordre.srt desordre.srt; cat ordre.srt -->
```console
$ printf '1\n00:00:05,000 --> 00:00:06,000\ncinq\n\n2\n00:00:01,000 --> 00:00:02,000\nun\n\n3\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > desordre.srt; subedit-cli sort --output ordre.srt desordre.srt; cat ordre.srt
desordre.srt: 3 subtitles moved -> ordre.srt
1
00:00:01,000 --> 00:00:02,000
un

2
00:00:03,000 --> 00:00:04,000
trois

3
00:00:05,000 --> 00:00:06,000
cinq
```

## Mitä lasketaan

**Lukumäärä on niiden paikkojen määrä, joiden kappale vaihtui**, kuten ydin sen ilmoittaa — mitä
ikkunan historia kutsuisi siirretyiksi. Yksi väärässä paikassa oleva kappale siirtää useita:
yllä olevassa esimerkissä yksikään kolmesta paikasta ei säilytä kappalettaan, ja lause kertoo `3 subtitles moved`.

**Jo järjestyksessä oleva tiedosto kirjoitetaan silti**, ja se kerrotaan: annettu kohde on
kirjoitettu kohde, ja jos `sort` olisi poikkeus, skriptin pitäisi tietää, mitkä
alikomennot tuottavat joskus tiedoston ja joskus eivät mitään.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > ordre.srt; subedit-cli sort --output copie.srt ordre.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > ordre.srt; subedit-cli sort --output copie.srt ordre.srt
ordre.srt: already in order -> copie.srt
```

## Tuloste

**Vakiotuloste** — ei mitään: tulos on kirjoitettu tiedosto. Valitsimella `--dry-run` ei myöskään mitään: tiedostoa
ei ole, ja kerrontarivi kertoo sen — katso [Esikatselu ennen kirjoittamista](invocation.md#esikatselu-ennen-kirjoittamista).

| Taso | Mitä virhetulosteeseen lisätään |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles moved -> <destination>`, tai `<path>: already in order -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — luetun tiedoston muoto, merkistökoodaus, merkki ja rivinvaihdot, palautettuina sellaisinaan |
| 3 | `<path>: N bytes read, M written`, sitten **jokainen lukemisen huomautus** — katso [Käynnistys](invocation.md#huomautukset-luettaessa) |

### JSON-muodossa

Valitsimella [`--format json`](invocation.md#skriptin-luettava-tuloste) kunkin tiedoston objekti sisältää kentät
`counts.subtitles` (tiedoston kaikki kappaleet) ja `counts.moved` (muuttuneet paikat;
`0` jo järjestyksessä olevalle tiedostolle).

## Paluuarvot

Työkalun paluuarvot: `0`, jos jokainen tiedosto käsiteltiin — jo järjestyksessä oleva tiedosto on käsitelty —, `2`, jos
yhtäkään ei käsitelty, `3`, jos vain osa, `1` käyttövirheessä.

## Virheet

Alikomennolla `sort` ei ole omia virheitä: lukukelvoton tiedosto epäonnistuu kuten millä tahansa alikomennolla, ja
kohteen virheet ovat yhteisiä kaikille kirjoittaville alikomennoille:
katso [Käynnistys](invocation.md#kohde).
