# `case`

<!-- machine-translated -->
> Tämä sivu on konekäännös, eikä kukaan tämän kielen lukija ole vielä tarkistanut sitä.

```
subedit-cli case --to title|sentence|upper|lower [--range N-M|N-]
                 (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 [--document main|translation] [-t FILE] [--align-method position|number]
                 <file>...
```

Muuttaa tekstit **muotoon, jossa jokainen sana alkaa isolla, lauseen kirjainkokoon, isoiksi tai pieniksi kirjaimiksi** koskematta
yhteenkään tagiin. Se on [ikkunan](../subedit-gui/operations.md) `Työkalut ▸ Kirjainkoko` sääntöineen.

| `--to` | Vaikutus | `bonjour marie` muuttuu muotoon |
| :----- | :---- | :---------------------- |
| `title` | jokainen sana saa ison alkukirjaimen, muut kirjaimet muuttuvat pieniksi | `Bonjour Marie` |
| `sentence` | tekstin ensimmäinen kirjain, ja vain se; muut kirjaimet muuttuvat pieniksi | `Bonjour marie` |
| `upper` | kaikki isoilla kirjaimilla | `BONJOUR MARIE` |
| `lower` | kaikki pienillä kirjaimilla | `bonjour marie` |

- **Ensimmäistä kirjainta tai ensimmäistä numeroa edeltävään ei kosketa**: vuoropuheluviiva,
  aloittava lainausmerkki tai sulje säilyvät (`- bonjour` antaa tuloksen `- Bonjour`).
- **Tagit eivät ole tekstiä**: niiden kirjainkoko ei muutu — `<I>` pysyy muodossa `<I>` — eivätkä ne
  vaihda paikkaa: sanan katkaiseva tagi katkaisee sen myös jälkeenpäin.
- **ICU päättää, mistä sana alkaa**: `l'été sera chaud` antaa tuloksen `L'été Sera Chaud`, ja aksentit
  säilyvät (`éléonore` antaa tuloksen `Éléonore`).

<!-- example: subedit-cli case --help -->
```console
$ subedit-cli case --help
Put the texts in title, sentence, upper or lower case, tags intact
Usage: subedit-cli case [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --to title|sentence|upper|lower
                              The case to put the texts in
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
| `--to` | **kyllä** | `title`, `sentence`, `upper`, `lower` | — |
| `--range` | ei | `N-M` tai `N-` loppuun asti — katso [`adjust`](adjust.md#--range) | koko tiedosto |
| `--document` | ei | `main` tai `translation` — katso [Käännös](invocation.md#käännös) | `main` |
| `-t`, `--translation-file` | valitsimen `--document translation` kanssa | käännöstiedosto, **yhdelle ainoalle** syötteelle | — |
| `--align-method` | ei, ja vain valitsimen `-t` kanssa | `position` tai `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **yksi kolmesta**, paitsi valitsimen `--dry-run` kanssa | katso [Käynnistys](invocation.md#kohde) | — |
| `--dry-run` | ei | lippu: laskee ja kertoo, ei kirjoita mitään — katso [Esikatselu ennen kirjoittamista](invocation.md#esikatselu-ennen-kirjoittamista) | pois |

**Oletuksena pääteksti; käännös valitsimilla `--document translation -t FILE`.** Silloin
muutetaan ja kirjoitetaan käännös, ja vain se — katso [Käännös](invocation.md#käännös).

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\n- BONJOUR MARIE\n\n' > a.srt; subedit-cli case --to title --output titre.srt a.srt; cat titre.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\n- BONJOUR MARIE\n\n' > a.srt; subedit-cli case --to title --output titre.srt a.srt; cat titre.srt
a.srt: 2 subtitles recased -> titre.srt
1
00:00:01,000 --> 00:00:02,000
<i>Bon</i>jour Marie

2
00:00:03,000 --> 00:00:04,000
- Bonjour Marie
```

Tiedosto, jossa **mikään teksti ei muutu** — kaikki on jo pyydetyssä kirjainkoossa — kirjoitetaan sellaisenaan, ja
se kerrotaan:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBONJOUR\n\n' > a.srt; subedit-cli case --to upper --output b.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBONJOUR\n\n' > a.srt; subedit-cli case --to upper --output b.srt a.srt
a.srt: nothing to change -> b.srt
```

## Tuloste

**Vakiotuloste** — ei mitään: tulos on kirjoitettu tiedosto. Valitsimella `--dry-run` luettelo
muutoksista, joka kuvataan kohdassa [Esikatselu ennen kirjoittamista](invocation.md#esikatselu-ennen-kirjoittamista).

| Taso | Mitä virhetulosteeseen lisätään |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles recased -> <destination>`, tai `nothing to change` |
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
| `--to` puuttuu | `--to is required` |
| arvo, joka ei ole yksi neljästä | `--to: … not in {title,sentence,upper,lower}` |
| väli kirjoitettu väärin tai tiedoston ulkopuolella | katso [`adjust`](adjust.md#--range) |

Kohteen virheet ovat yhteisiä kaikille kirjoittaville alikomennoille:
katso [Käynnistys](invocation.md#kohde).
