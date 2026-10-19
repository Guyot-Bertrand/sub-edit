# `disjoint`

<!-- machine-translated -->
> Tämä sivu on konekäännös, eikä kukaan tämän kielen lukija ole vielä tarkistanut sitä.

```
subedit-cli disjoint (--output FILE | --output-dir DIR | --in-place)
                     [--minimum TIME] [--max-lines N] [--dashes] [--sort]
                     [--dry-run] [--recursive]
                     <file>...
```

Leikkaa päällekkäiset kappaleet niin, ettei **mikään aikaväli `[start, end]` ole enää päällekkäin
toisen kanssa**. Se on ikkunan `Make Ranges Disjoint…` — [sama moottori](../subedit-gui/lines.md#aikavälien-päällekkäisyyksien-poistaminen).

**Mitään ei lyhennetä eikä pidennetä**: tiedosto näyttää joka hetkellä samat tekstit kuin ennenkin. Muuttuu
vain se, miten aikajana leikataan:

- se leikataan päällekkäisen ryhmän kappaleiden **jokaisesta alusta ja jokaisesta lopusta**;
- väli, jonka kattaa **yksi ainoa** kappale, pysyy tuona kappaleena;
- väli, jonka kattaa **useampi**, muuttuu **yhdeksi** kappaleeksi, jonka rivit ovat niiden rivit **niiden
  alkujen järjestyksessä** — aikaisin rivillä 1, seuraava rivillä 2;
- kappaleeseen, joka ei ole päällekkäin minkään kanssa, ei kosketa, ja muiden järjestys säilyy.

<!-- example: subedit-cli disjoint --help -->
```console
$ subedit-cli disjoint --help
Cut overlapping subtitles so that no range overlaps another, merging what overlaps
Usage: subedit-cli disjoint [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to make disjoint

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --minimum TIME              Shortest interval kept; a shorter one is taken over by a neighbour (default 0.08)
  --max-lines N               Most lines of a merged subtitle; more are shown in windows, 0 for no limit (default 2)
  --dashes                    Put a dash before the text of each merged subtitle
  --sort                      Put the subtitles in order of their start first; without it, a file out of order is refused
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Argumentit ja valitsimet

| Valitsin | Pakollinen | Arvot | Oletus |
| :----- | :----- | :------ | :----- |
| `<file>...` | kyllä | yksi tai useampi polku | — |
| `--minimum` | ei | kesto: `0.08`, `1.5`, `00:00:00,080`. Nolla säilyttää jokaisen välin | `0.08` (noin kaksi ruutua) |
| `--max-lines` | ei | kokonaisluku: yhdistetyn kappaleen rivien enimmäismäärä; `0` poistaa rajan | `2` |
| `--dashes` | ei | lippu: viiva sen tekstin eteen, jonka kukin kappale tuo yhdistettyyn kappaleeseen | pois |
| `--sort` | ei | lippu: järjestää kappaleet alkujensa mukaan **ensin** — katso [Tiedoston on oltava järjestyksessä](#tiedoston-on-oltava-järjestyksessä) | pois |
| `--recursive`, `-r` | ei | lippu | pois — katso [Hakemistopuun käsittely](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **yksi kolmesta**, paitsi valitsimen `--dry-run` kanssa | katso [Käynnistys](invocation.md#kohde) | — |
| `--dry-run` | ei | lippu: laskee ja kertoo, ei kirjoita mitään — katso [Esikatselu ennen kirjoittamista](invocation.md#esikatselu-ennen-kirjoittamista) | pois |

<!-- example: printf '1\n00:00:00,000 --> 00:00:05,000\nA\n\n2\n00:00:03,000 --> 00:00:08,000\nB\n\n' > chevauche.srt; subedit-cli disjoint --output disjoint.srt chevauche.srt; cat disjoint.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:05,000\nA\n\n2\n00:00:03,000 --> 00:00:08,000\nB\n\n' > chevauche.srt; subedit-cli disjoint --output disjoint.srt chevauche.srt; cat disjoint.srt
chevauche.srt: 2 subtitles split, 1 merged -> disjoint.srt
1
00:00:00,000 --> 00:00:03,000
A

2
00:00:03,000 --> 00:00:05,000
A
B

3
00:00:05,000 --> 00:00:08,000
B
```

`A [0, 5]` ja `B [3, 8]` antavat kolme päällekkäisyyksistä vapaata kappaletta: `A` yksin, `A` ja `B` yhdessä, `B` yksin. Yhtäkään
hetkeä ei menetetä eikä lisätä.

## Mitä toiminto päättää

**Riviä ei koskaan monisteta.** Kaksi kappaletta, joilla on sama teksti, antavat yhden rivin siellä, missä ne
ovat päällekkäin, ja kaksi vierekkäistä palaa, jotka näyttävät saman asian, muodostavat vain yhden kappaleen. Useaksi
palaksi leikatun kappaleen teksti muuten **toistetaan jokaisessa palassa**: se on se, mikä ruudulla on tuolla
hetkellä.

**Hyvin lyhyen välin ottaa haltuunsa naapuri.** Valitsimen `--minimum` arvoa lyhyempi väli katoaa: naapuri
**laajentaa aikaväliään kattamaan sen**, jotta mikään hetki ei jää tyhjäksi eikä mikään siirry tuota kynnystä
enempää. Valittu naapuri on se, jonka rivit **jo sisältävät** välin rivit, muuten
pisin. Välin teksti menetetään vain, jos mikään naapuri ei kanna sitä, ja silloinkin alle kynnyksen ajaksi.
**Kynnystä lyhyempää kokonaista kappaletta ei koskaan oteta haltuun**: se jätetään ennalleen ja lasketaan.

**Enemmän rivejä kuin raja sallii: ikkunat.** Yhdistetty kappale, jolla olisi enemmän rivejä kuin valitsimen `--max-lines` arvo,
näytetään **peräkkäisinä ikkunoina**, joiden koko on tuo arvo, kesto jaettuna tasan. Kolme riviä kolmen
sekunnin aikana:

<!-- example: printf '1\n00:00:00,000 --> 00:00:03,000\nA\n\n2\n00:00:00,000 --> 00:00:03,000\nB\n\n3\n00:00:00,000 --> 00:00:03,000\nC\n\n' > trois.srt; subedit-cli disjoint --output fenetres.srt trois.srt; cat fenetres.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:03,000\nA\n\n2\n00:00:00,000 --> 00:00:03,000\nB\n\n3\n00:00:00,000 --> 00:00:03,000\nC\n\n' > trois.srt; subedit-cli disjoint --output fenetres.srt trois.srt; cat fenetres.srt
trois.srt: 0 subtitles split, 1 merged; 1 shown in windows for holding too many lines -> fenetres.srt
1
00:00:00,000 --> 00:00:01,500
A
B

2
00:00:01,500 --> 00:00:03,000
B
C
```

Neljä riviä antaa kolme ikkunaa ja niin edelleen: ikkuna liukuu rivi kerrallaan. Mitään ei
katkaista eikä leikata uudelleen.

**Käännös seuraa samoja leikkauksia**, koska kappale kantaa molempia tekstejä. **Se, mitä muoto kantaa
lisäksi** — tyyli, sijainti — on ensimmäisen kattavan kappaleen; menetys lasketaan, kun
yhdistetyt kappaleet eroavat toisistaan.

**Kappaletta, jonka loppu on ennen sen alkua**, ei yritetä arvata: se jätetään paikalleen ja lasketaan.

## Tiedoston on oltava järjestyksessä

Ryhmät ovat naapurien jonoja: epäjärjestyksessä naapurit eivät ole niitä, jotka ovat päällekkäin. Tiedosto,
jonka alut eivät ole järjestyksessä, siksi **hylätään**, ja viesti kertoo sen:

<!-- example: printf '1\n00:00:03,000 --> 00:00:08,000\nB\n\n2\n00:00:00,000 --> 00:00:05,000\nA\n\n' > desordre.srt; subedit-cli disjoint --output x.srt desordre.srt; echo "code $?" -->
```console
$ printf '1\n00:00:03,000 --> 00:00:08,000\nB\n\n2\n00:00:00,000 --> 00:00:05,000\nA\n\n' > desordre.srt; subedit-cli disjoint --output x.srt desordre.srt; echo "code $?"
desordre.srt: The subtitles are not in the order of their start: sort them first. Use --sort to do that here.
code 2
```

`--sort` tekee järjestämisen ensin — saman kuin [`sort`](sort.md) — ja kertoo sen. Ydin ei koskaan järjestä omin päin.

Toinen ajokerta ei muuta mitään: kun aikavälit ovat erillään, leikattavaa ei enää ole, ja
lause sanoo `already disjoint`.

## Tuloste

**Vakiotuloste** — ei mitään: tulos on kirjoitettu tiedosto.

| Taso | Mitä virhetulosteeseen lisätään |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles split, M merged -> <destination>`, jonka jälkeen merkin `;` perässä se, mitä ei voitu tehdä kokonaan: `K shown in windows for holding too many lines`, `K very short taken over by a neighbour`, `K subtitles shorter than the limit left whole`, `K merged with styles or positions that differ, the first kept`, `K subtitles ending before the start left alone` — tai `<path>: already disjoint -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, sitten **jokainen huomautus luettaessa** |

`N subtitles split` laskee alkuperäiset kappaleet, jotka leikattiin useaksi palaksi, `M merged` palat, jotka
näyttävät useita kappaleita yhdessä.

### JSON-muodossa

Valitsimella [`--format json`](invocation.md#skriptin-luettava-tuloste) kunkin tiedoston `counts`:

| Laskuri | Mitä se kertoo |
| :----- | :----------- |
| `subtitles` | kirjoitetun tiedoston kappaleet |
| `split`, `merged` | kuten lauseessa |
| `windowed` | ikkunoina näytetyt palat |
| `absorbed` | naapurin haltuun ottamat välit |
| `too_short` | kynnystä lyhyemmät kappaleet, jätetty kokonaisiksi |
| `ending_before_start` | kappaleet, joiden loppu on ennen alkua, jätetty ennalleen |
| `extras_dropped` | yhdistetyt palat, joiden tyyli tai sijainti eroaa |
| `moved` | vain valitsimella `--sort`: muuttuneet paikat |

## Paluuarvot

Työkalun yleiset: `0`, jos jokainen tiedosto käsiteltiin — myös tiedosto, jossa ei ole päällekkäisyyksiä, on käsitelty —, `2`, jos
yhtäkään ei, `3`, jos vain osa, `1` käyttövirheessä.

## Virheet

| Mikä ne aiheuttaa | Viesti |
| :------------------- | :------ |
| epäjärjestyksessä oleva tiedosto ilman valitsinta `--sort` | `<path>: The subtitles are not in the order of their start: sort them first. Use --sort to do that here.` |
| `--minimum` ei ole kesto tai on negatiivinen | `--minimum: …` |
| `--max-lines` ei ole kokonaisluku | `--max-lines: "…" is not a number of lines: expected a whole number, 0 for no limit` |

Lukukelvoton tiedosto epäonnistuu kuten minkä tahansa alikomennon kanssa; kohteen virheet ovat yhteisiä
kaikille kirjoittaville alikomennoille: katso [Käynnistys](invocation.md#kohde).

## Muissa komennoissa

[`convert`](convert.md#aikavälien-päällekkäisyyksien-poistaminen), [`append`](append.md#aikavälien-päällekkäisyyksien-poistaminen) ja
[`split-file`](split-file.md#aikavälien-päällekkäisyyksien-poistaminen) hyväksyvät valitsimen `--disjoint`, joka tekee saman
oletusasetuksin ennen kirjoittamista.

Huonosti segmentoidussa tiedostossa — teksti, joka on leikattu peräkkäisiksi paloiksi ja annettu epäjärjestyksessä —
pelkkä järjestäminen ja leikkaaminen antavat rivejä, jotka vaihtelevat palasta toiseen: ensin [`join-repeats`](join-repeats.md) tai
[`repair`](repair.md), joka ketjuttaa kaikki kolme.
