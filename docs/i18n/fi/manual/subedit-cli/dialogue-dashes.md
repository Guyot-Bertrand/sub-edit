# `dialogue-dashes`

<!-- machine-translated -->
> Tämä sivu on konekäännös, eikä kukaan tämän kielen lukija ole vielä tarkistanut sitä.

```
subedit-cli dialogue-dashes (--add | --remove) [--range N-M|N-]
                            (--output FILE | --output-dir DIR | --in-place)
                            [--dry-run]
                            [--recursive]
                            [--document main|translation] [-t FILE] [--align-method position|number]
                            <file>...
```

Lisää rivien alkuun **vuoropuheluviivan** tai poistaa sen. Se on
[ikkunan](../subedit-gui/operations.md) `Dialogue Dashes`.

**Kaksi sanaa, kuten alikomennolla [`italics`](italics.md)**: `--add` ja `--remove` sulkevat toisensa pois, ja yksi tarvitaan.

- **`--add`** lisää viivan jokaisen sellaisen rivin eteen, jossa sitä ei ole: `Bonjour` antaa tuloksen `- Bonjour`, ja
  `Bonjour\nMarie` antaa tuloksen `- Bonjour\n- Marie`. **Jo olemassa olevaa viivaa ei kahdenneta.**
- **Kolme viivaa tunnistetaan** — näppäimistön yhdysmerkki ja typografin kaksi pitkää viivaa —
  **ja kirjoitettava on yhdysmerkki**: `— Bonjour` muuttuu muotoon `- Bonjour`. Ylimääräiset välilyönnit
  viivan jälkeen katoavat.
- Viiva tulee **tekstin eteen eikä aloittavan tagin eteen**: `<i>Bonjour</i>` antaa tuloksen
  `<i>- Bonjour</i>`. Mikään tagi ei siirry.
- **`--remove`** poistaa rivien alun viivat, kaikissa kolmessa muodossa.

<!-- example: subedit-cli dialogue-dashes --help -->
```console
$ subedit-cli dialogue-dashes --help
Put dialogue dashes on the lines, or take them off
Usage: subedit-cli dialogue-dashes [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --add                       Put a dialogue dash at the head of the lines
  --remove                    Take the dialogue dashes off
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
| `--add` / `--remove` | **toinen kahdesta** | liput | — |
| `--range` | ei | `N-M` tai `N-` loppuun asti — katso [`adjust`](adjust.md#--range) | koko tiedosto |
| `--document` | ei | `main` tai `translation` — katso [Käännös](invocation.md#käännös) | `main` |
| `-t`, `--translation-file` | valitsimen `--document translation` kanssa | käännöstiedosto, **yhdelle ainoalle** syötteelle | — |
| `--align-method` | ei, ja vain valitsimen `-t` kanssa | `position` tai `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **yksi kolmesta**, paitsi valitsimen `--dry-run` kanssa | katso [Käynnistys](invocation.md#kohde) | — |
| `--dry-run` | ei | lippu: laskee ja kertoo, ei kirjoita mitään — katso [Esikatselu ennen kirjoittamista](invocation.md#esikatselu-ennen-kirjoittamista) | pois |

**Oletuksena pääteksti; käännös valitsimilla `--document translation -t FILE`.** Silloin
muutetaan ja kirjoitetaan käännös, ja vain se — katso [Käännös](invocation.md#käännös).

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\nMarie\n\n2\n00:00:03,000 --> 00:00:04,000\n<i>Salut</i>\n\n' > a.srt; subedit-cli dialogue-dashes --add --output t.srt a.srt; cat t.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\nMarie\n\n2\n00:00:03,000 --> 00:00:04,000\n<i>Salut</i>\n\n' > a.srt; subedit-cli dialogue-dashes --add --output t.srt a.srt; cat t.srt
a.srt: 2 subtitles dashed -> t.srt
1
00:00:01,000 --> 00:00:02,000
- Bonjour
- Marie

2
00:00:03,000 --> 00:00:04,000
<i>- Salut</i>
```

## Tuloste

**Vakiotuloste** — ei mitään: tulos on kirjoitettu tiedosto. Valitsimella `--dry-run` luettelo
muutoksista, joka kuvataan kohdassa [Esikatselu ennen kirjoittamista](invocation.md#esikatselu-ennen-kirjoittamista).

| Taso | Mitä virhetulosteeseen lisätään |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles dashed -> <destination>`, tai `N subtitles undashed`, tai `nothing to change` |
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
| ei `--add` eikä `--remove` | `dialogue-dashes needs --add to put dashes on, or --remove` |
| molemmat | `--add and --remove say opposite things; give one of them` |
| väli kirjoitettu väärin tai tiedoston ulkopuolella | katso [`adjust`](adjust.md#--range) |

Kohteen virheet ovat yhteisiä kaikille kirjoittaville alikomennoille:
katso [Käynnistys](invocation.md#kohde).
