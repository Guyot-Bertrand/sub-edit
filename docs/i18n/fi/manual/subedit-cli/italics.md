# `italics`

<!-- machine-translated -->
> Tämä sivu on konekäännös, eikä kukaan tämän kielen lukija ole vielä tarkistanut sitä.

```
subedit-cli italics (--on | --off) [--range N-M|N-]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    <file>...
```

Muuttaa tekstit **kursiiviksi** tai **poistaa niistä kursiivin** tiedoston muodon tageilla.
Se on [ikkunan](../subedit-gui/operations.md) `Italic`.

**Kaksi sanaa eikä vaihtokytkin**: ikkunan painike kysyy ytimeltä, kumpaan suuntaan se
toimii; tässä suunta sanotaan itse. `--on` ja `--off` sulkevat toisensa pois, ja yksi tarvitaan — sen pois jättäminen tai molempien antaminen
on käyttövirhe (koodi `1`).

- **`--on`** ympäröi kunkin tekstin yhdellä tagiparilla **poistettuaan ne, jotka
  siinä jo olivat**: jo kursivoitua tekstiä ei ympäröidä kahdesti, ja kahtia katkennut kursiivi
  tehdään uudelleen yhtenä kappaleena. Usean rivin yli yksi pari kattaa kokonaisuuden.
- **`--off`** poistaa kursiivitagit; **tagin muu osa säilytetään** (`{\b1\i1}` antaa
  tuloksen `{\b1}`).
- Teksti kirjoitetaan **niin kuin tiedoston muoto kirjoittaa kursiivin**: `<i>…</i>` muodoille SubRip, WebVTT ja
  SubViewer 2, `{\i1}…{\i0}` kahdelle Sub Station Alpha -muodolle, `{Y:i}` muodolle MicroDVD.

<!-- example: subedit-cli italics --help -->
```console
$ subedit-cli italics --help
Put the texts in italics, or take their italics out
Usage: subedit-cli italics [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --on                        Put the texts in italics
  --off                       Take the italics out
  --range N-M|N-              Act only on subtitles N to M, or N to the end
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
| `--recursive`, `-r` | ei | lippu | pois — katso [Hakemistopuun käsittely](lots.md) |
| `--on` / `--off` | **toinen kahdesta** | liput | — |
| `--range` | ei | `N-M` tai `N-` loppuun asti — katso [`adjust`](adjust.md#--range) | koko tiedosto |
| `--document` | ei | `main` tai `translation` — katso [Käännös](invocation.md#käännös) | `main` |
| `-t`, `--translation-file` | valitsimen `--document translation` kanssa | käännöstiedosto, **yhdelle ainoalle** syötteelle | — |
| `--align-method` | ei, ja vain valitsimen `-t` kanssa | `position` tai `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **yksi kolmesta**, paitsi valitsimen `--dry-run` kanssa | katso [Käynnistys](invocation.md#kohde) | — |
| `--dry-run` | ei | lippu: laskee ja kertoo, ei kirjoita mitään — katso [Esikatselu ennen kirjoittamista](invocation.md#esikatselu-ennen-kirjoittamista) | pois |

**Oletuksena pääteksti; käännös valitsimilla `--document translation -t FILE`.** Silloin
muutetaan ja kirjoitetaan käännös, ja vain se — katso [Käännös](invocation.md#käännös).

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour\nMarie\n\n' > a.srt; subedit-cli italics --on --output i.srt a.srt; cat i.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour\nMarie\n\n' > a.srt; subedit-cli italics --on --output i.srt a.srt; cat i.srt
a.srt: 2 subtitles put in italics -> i.srt
1
00:00:01,000 --> 00:00:02,000
<i>bonjour marie</i>

2
00:00:03,000 --> 00:00:04,000
<i>Bonjour
Marie</i>
```

## Muoto, jossa ei ole tyylejä, kieltäytyy

**TMPlayer ja LRC eivät kirjoita tyylejä**: ei ole kursiivia, jota lisätä tai poistaa.
Ikkuna näyttää kohdan harmaana sen piilottamisen sijaan; komentorivi **kieltäytyy tiedosto
kerrallaan ja kertoo syyn**, ja erän muut tiedostot jatkuvat. Ratkaisevaa on se, mitä
muoto pystyy säilyttämään, eikä tapa, jolla se sen kirjoittaa: SubRip ja WebVTT kirjoittavat kursiivin samalla
tavalla, mutta eivät sano samaa väristä.

<!-- example: printf '00:00:01:Hello there\n00:00:03:Bye\n' > a.txt; subedit-cli italics --on --output-dir sortie a.txt; echo "code=$?" -->
```console
$ printf '00:00:01:Hello there\n00:00:03:Bye\n' > a.txt; subedit-cli italics --on --output-dir sortie a.txt; echo "code=$?"
a.txt: TMPlayer writes no style: there are no italics to put on
code=2
```

JSON-muodossa epäonnistumisella on tunniste **`no-style`**.

## Tuloste

**Vakiotuloste** — ei mitään: tulos on kirjoitettu tiedosto. Valitsimella `--dry-run` luettelo
muutoksista, joka kuvataan kohdassa [Esikatselu ennen kirjoittamista](invocation.md#esikatselu-ennen-kirjoittamista).

| Taso | Mitä virhetulosteeseen lisätään |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles put in italics -> <destination>`, tai `N subtitles taken out of italics`, tai `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — luetun tiedoston muoto, merkistökoodaus, merkki ja rivinvaihdot, palautettuina sellaisinaan |
| 3 | `<path>: N bytes read, M written`, sitten **jokainen lukemisen huomautus** — katso [Käynnistys](invocation.md#huomautukset-luettaessa) |

`N` on niiden kappaleiden määrä, **joiden teksti muuttui**, luettuna itse toiminnosta.

### JSON-muodossa

Valitsimella [`--format json`](invocation.md#skriptin-luettava-tuloste) kunkin tiedoston objekti sisältää kentän
`counts.changed` — lauseen `N` — ja kentän `changes`, muuttuneiden kappaleiden luettelon
(`{"subtitle", "document", "before", "after"}`, **tyhjä**, kun mikään ei muuttunut).

## Paluuarvot

Työkalun paluuarvot: `0`, jos jokainen tiedosto käsiteltiin — tiedosto, jossa mikään ei muutu, on käsitelty —,
`2`, jos yhtäkään ei käsitelty, `3`, jos vain osa, `1` käyttövirheessä.

## Virheet

| Mikä sen aiheuttaa | Viesti |
| :------------------ | :------ |
| ei `--on` eikä `--off` | `italics needs --on to put the texts in italics, or --off` |
| molemmat | `--on and --off say opposite things; give one of them` |
| muoto, jossa ei ole tyylejä | `<path>: TMPlayer writes no style: there are no italics to put on` (tai `take out`) |
| väli kirjoitettu väärin tai tiedoston ulkopuolella | katso [`adjust`](adjust.md#--range) |

Kohteen virheet ovat yhteisiä kaikille kirjoittaville alikomennoille:
katso [Käynnistys](invocation.md#kohde).
