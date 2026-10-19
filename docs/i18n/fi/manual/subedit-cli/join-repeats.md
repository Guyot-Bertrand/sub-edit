# `join-repeats`

<!-- machine-translated -->
> Tämä sivu on konekäännös, eikä kukaan tämän kielen lukija ole vielä tarkistanut sitä.

```
subedit-cli join-repeats (--output FILE | --output-dir DIR | --in-place)
                         [--dry-run]
                         [--recursive]
                         <file>...
```

Tekee **yhden ainoan kappaleen** kappaleista, jotka näyttävät saman asian peräkkäin: paloiksi leikatun tekstin,
jonka palat seuraavat toisiaan, kuten jotkin segmentointityökalut sen jättävät. Se on ikkunan `Join Repeated Subtitles` —
[sama moottori](../subedit-gui/lines.md#toistuvien-kappaleiden-yhdistäminen).

**Toisto** on kappale, jolla on **sama teksti, sama käännös ja samat muotoiluasetukset** (tyyli,
sijainti, vihjeasetukset) kuin toisella kappaleella **ja joka alkaa viimeistään silloin, kun toinen päättyy**: ne
koskettavat toisiaan tai menevät päällekkäin. Toistojen jono muuttuu kappaleeksi, **joka alkaa ensimmäisenä**, ja sen loppu
siirretään jonon myöhäisimpään loppuun; muut katoavat.

- **Tiedoston järjestyksellä ei ole merkitystä, eikä sitä muuteta**: säilytettävä kappale on se, joka alkaa ensimmäisenä,
  missä muut sitten ovatkin. Järjestyksessä oleva tiedosto pysyy järjestyksessä.
- **Mitään ei yhdistetä vahingossa**: eri teksti, eri käännös, eri asetukset, väli kahden
  palan välillä, kappale ilman tekstiä tai kappale, jonka loppu on ennen sen alkua — kaikissa näissä tapauksissa ne pysyvät erillään.
- **Tehtävä ennen muuta**: järjestäminen ja [päällekkäisyyksien poistaminen](disjoint.md) lukevat siistin jonon paremmin.
  [`repair`](repair.md) ketjuttaa kaikki kolme, ja [`convert`](convert.md#aikavälien-päällekkäisyyksien-poistaminen) hyväksyy sen valitsimena (`--join-repeats`, `--repair`).

<!-- example: subedit-cli join-repeats --help -->
```console
$ subedit-cli join-repeats --help
Join the subtitles that repeat each other, one after the other, into one
Usage: subedit-cli join-repeats [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to join the repeats of

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

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:04,000 --> 00:00:06,000\nAlpha\n\n4\n00:00:08,000 --> 00:00:09,000\nBeta\n\n' > morceaux.srt; subedit-cli join-repeats --output joint.srt morceaux.srt; cat joint.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:04,000 --> 00:00:06,000\nAlpha\n\n4\n00:00:08,000 --> 00:00:09,000\nBeta\n\n' > morceaux.srt; subedit-cli join-repeats --output joint.srt morceaux.srt; cat joint.srt
morceaux.srt: 2 repeated subtitles joined -> joint.srt
1
00:00:01,000 --> 00:00:06,000
Alpha

2
00:00:08,000 --> 00:00:09,000
Beta
```

Tekstin `Alpha` kolme palaa, jotka seuraavat toisiaan ajasta 1 s aikaan 6 s, muodostavat nyt vain yhden. Välin erottama `Beta`
säilyi. Toinen ajo ei muuta mitään ja kertoo sen: `no repeated subtitles`.

## Tuloste

**Vakiotuloste** — ei mitään: tulos on kirjoitettu tiedosto.

| Taso | Mitä virhetulosteeseen lisätään |
| :----- | :------------------------------------- |
| 1 | `<path>: N repeated subtitles joined -> <destination>`, tai `<path>: no repeated subtitles -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, sitten **jokainen lukemisen huomautus** |

### JSON-muodossa

Valitsimella [`--format json`](invocation.md#skriptin-luettava-tuloste) kunkin tiedoston `counts`: `subtitles` (kirjoitetun
tiedoston kappaleet), `joined` (kappaleet, jotka poistettiin, koska toinen kattaa ne) ja `chains` (yhdistetyt jonot).

## Paluuarvot

Työkalun paluuarvot: `0`, jos jokainen tiedosto käsiteltiin — tiedosto ilman toistoja on käsitelty —, `2`, jos yhtäkään ei käsitelty, `3`, jos
vain osa, `1` käyttövirheessä.

## Virheet

Alikomennolla `join-repeats` ei ole omia virheitä: lukukelvoton tiedosto epäonnistuu kuten millä tahansa alikomennolla, ja
kohteen virheet ovat yhteisiä kaikille kirjoittaville alikomennoille: katso [Käynnistys](invocation.md#kohde).
