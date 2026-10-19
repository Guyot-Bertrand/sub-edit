# `repair`

<!-- machine-translated -->
> Tämä sivu on konekäännös, eikä kukaan tämän kielen lukija ole vielä tarkistanut sitä.

```
subedit-cli repair (--output FILE | --output-dir DIR | --in-place)
                   [--minimum TIME] [--max-lines N] [--dashes]
                   [--dry-run] [--recursive]
                   <file>...
```

Tekee kerralla sen, minkä kolme komentoa tekevät peräkkäin: [`join-repeats`](join-repeats.md), sitten
[`sort`](sort.md), sitten [`disjoint`](disjoint.md). **Tässä järjestyksessä, joka on ainoa toimiva**: leikkaaminen lukee
alkukohtien järjestyksen, ja saman tekstin toistuvat palat on parempi poistaa ennen kumpaakin. Tiedostolle,
joka on huonosti segmentoitu, epäjärjestyksessä ja päällekkäinen, tämä on ajettava komento.

Asetukset ovat alikomennon `disjoint` asetukset, sillä se päättää ketjun; valitsinta `--sort` ei tässä ole, koska järjestäminen tehdään aina.

<!-- example: subedit-cli repair --help -->
```console
$ subedit-cli repair --help
Join the repeated subtitles, put them in order, then make the ranges disjoint
Usage: subedit-cli repair [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to repair

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --minimum TIME              Shortest interval kept; a shorter one is taken over by a neighbour (default 0.08)
  --max-lines N               Most lines of a merged subtitle; more are shown in windows, 0 for no limit (default 2)
  --dashes                    Put a dash before the text of each merged subtitle
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
| `--max-lines` | ei | kokonaisluku; `0` poistaa rajan | `2` |
| `--dashes` | ei | lippu: viiva sen tekstin eteen, jonka kukin kappale tuo yhdistettyyn kappaleeseen | pois |
| `--recursive`, `-r` | ei | lippu | pois — katso [Hakemistopuun käsittely](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **yksi kolmesta**, paitsi valitsimen `--dry-run` kanssa | katso [Käynnistys](invocation.md#kohde) | — |
| `--dry-run` | ei | lippu: laskee ja kertoo, ei kirjoita mitään — katso [Esikatselu ennen kirjoittamista](invocation.md#esikatselu-ennen-kirjoittamista) | pois |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:01,000 --> 00:00:04,000\nBeta\n\n' > segments.srt; subedit-cli repair --output repare.srt segments.srt; cat repare.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:01,000 --> 00:00:04,000\nBeta\n\n' > segments.srt; subedit-cli repair --output repare.srt segments.srt; cat repare.srt
segments.srt: 1 repeated subtitle joined; already in order; 0 subtitles split, 1 merged -> repare.srt
1
00:00:01,000 --> 00:00:04,000
Alpha
Beta
```

Kahdeksi toisiaan seuraavaksi palaksi leikattu `Alpha` muuttuu taas yhdeksi kappaleeksi; sen jälkeen se kattaa saman aikavälin kuin `Beta`,
ja ne muodostavat vain yhden kahden rivin kappaleen, aikaisin ensin.

## Tuloste

**Vakiotuloste** — ei mitään: tulos on kirjoitettu tiedosto. Virhetulosteeseen ensimmäisellä tasolla **lause,
joka ketjuttaa kolme lukumäärää** puolipisteillä erotettuina: mitä alikomennot `join-repeats`, `sort` ja `disjoint` kukin kertovat,
omin sanoin — `N repeated subtitles joined; M subtitles moved; K subtitles split, L merged…`.

### JSON-muodossa

Valitsimella [`--format json`](invocation.md#skriptin-luettava-tuloste) kunkin tiedoston `counts`: `subtitles`, `joined`,
`moved`, sitten alikomennon [`disjoint`](disjoint.md#json-muodossa) kentät (`split`, `merged`, `windowed`, `absorbed`, `too_short`,
`ending_before_start`, `extras_dropped`).

## Paluuarvot ja virheet

Työkalun paluuarvot. Alikomennolla `repair` ei ole omia virheitä asetusten lisäksi, ja ne arvioidaan kuten alikomennon `disjoint` asetukset:
`--minimum`, joka ei ole kesto tai on negatiivinen, `--max-lines`, joka ei ole kokonaisluku, koodi `1`.
