# `replace`

<!-- machine-translated -->
> Tämä sivu on konekäännös, eikä kukaan tämän kielen lukija ole vielä tarkistanut sitä.

```
subedit-cli replace [--regex] [--case-sensitive] [--range N-M|N-]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    <pattern> <replacement> <file>...
```

Korvaa tekstin kappaleissa **rikkomatta yhtäkään tagia**. Se on
ikkunan [haun](../subedit-gui/find-and-replace.md) toiminto `Replace All` sääntöineen:

- **haku tehdään näkyvästä tekstistä** — siitä, mitä katsoja lukee. Tagit eivät kuulu
  siihen: haku `<i>` ei löydä mitään, ja haku `Bonjour` löytää ”Bonjour”-sanan, josta puolet
  on kursiivilla;
- **korvaus tehdään lähdetekstiin**, tiedoston tekstiin, ja tagit pysyvät siellä, missä ne
  olivat. Haetun sanan katkaiseva tagi ympäröi koko korvauksen; osumaa
  koskettava tyyli kattaa koko korvauksen; mikä ei koske osumaan,
  ei liiku. Säännöt tapaus kerrallaan ovat samat kuin ikkunan haussa.

**Alikomentoa `find` ei ole**: kun haluat nähdä, missä lauseke esiintyy, muuttamatta mitään,
[`--dry-run`](invocation.md#esikatselu-ennen-kirjoittamista) luettelee jokaisen kyseisen kappaleen.

<!-- example: subedit-cli replace --help -->
```console
$ subedit-cli replace --help
Replace a text in the subtitles, without breaking a tag
Usage: subedit-cli replace [OPTIONS] pattern replacement files...

Positionals:
  pattern TEXT REQUIRED       Text to look for in what is shown
  replacement TEXT REQUIRED   What replaces it, tags allowed
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --regex                     Read the text to look for as a regular expression
  --case-sensitive            Tell capitals from small letters
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
| `<pattern>` | kyllä | näkyvästä tekstistä haettava teksti; **ei tyhjä** | — |
| `<replacement>` | kyllä | sen korvaava teksti, tagit sallittu; voi olla tyhjä | — |
| `<file>...` | kyllä | yksi tai useampi polku | — |
| `--recursive`, `-r` | ei | lippu | pois — katso [Hakemistopuun käsittely](lots.md) |
| `--regex` | ei | lippu: lauseke on ICU-säännöllinen lauseke | pelkkä teksti |
| `--case-sensitive` | ei | lippu: erottaa isot ja pienet kirjaimet | kirjainkokoa ei huomioida |
| `--range` | ei | `N-M` tai `N-` loppuun asti — katso [`adjust`](adjust.md#--range) | koko tiedosto |
| `--document` | ei | `main` tai `translation` — katso [Käännös](invocation.md#käännös) | `main` |
| `-t`, `--translation-file` | valitsimen `--document translation` kanssa | käännöstiedosto **yhdelle** syötteelle | — |
| `--align-method` | ei, ja vain valitsimen `-t` kanssa | `position` tai `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **yksi kolmesta**, paitsi valitsimen `--dry-run` kanssa | katso [Käynnistys](invocation.md#kohde) | — |
| `--dry-run` | ei | lippu: laskee ja kertoo, ei kirjoita mitään — katso [Esikatselu ennen kirjoittamista](invocation.md#esikatselu-ennen-kirjoittamista) | pois |

**Viivalla alkava lauseke** kirjoitetaan merkkien `--` jälkeen: `subedit-cli replace -- -- "—" film.srt`.

**Oletuksena pääteksti; käännös valitsimilla `--document translation -t FILE`.** Silloin
muutetaan ja kirjoitetaan käännös, ja vain se — katso [Käännös](invocation.md#käännös).

### Pelkkä teksti vai säännöllinen lauseke

Oletuksena lauseke **luetaan sellaisenaan**: `a.b` hakee merkkiä `a`, pistettä ja merkkiä `b`.
Kirjainkokoa ei huomioida yhtä hienovaraisesti kuin ikkunassa — `É` ja `é` ovat yksi ja sama kirjain.

Valitsimella `--regex` lauseke on **ICU**-säännöllinen lauseke, joka luetaan kuten Gaupol sen lukee: `.` ylittää
rivinvaihdon, `^` ja `$` pätevät kappaleen jokaisella rivillä. Korvauksessa, ja
vain siinä:

| Kirjoita | Tarkoittaaksesi |
| :----- | :-------- |
| `$1`–`$9` | osuman ryhmät |
| `$0` | koko osuma |
| `\n` | rivinvaihto |
| `\$`, `\\` | merkki `$`, kenoviiva |

Huomaa komentotulkin lainausmerkit: `'$1 EUR'` eikä `"$1 EUR"`, jolloin komentotulkki lukisi itse viittauksen `$1`.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nPrix: 12 euros, et 7 euros.\n\n' > prix.srt; subedit-cli replace --regex '([0-9]+) euros' '$1 EUR' --output euros.srt prix.srt; cat euros.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nPrix: 12 euros, et 7 euros.\n\n' > prix.srt; subedit-cli replace --regex '([0-9]+) euros' '$1 EUR' --output euros.srt prix.srt; cat euros.srt
prix.srt: replaced 2 matches -> euros.srt
1
00:00:01,000 --> 00:00:02,000
Prix: 12 EUR, et 7 EUR.
```

## Tagit

Teksti, jossa tagi katkaisee haetun sanan:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace Bonjour Salut --output salut.srt balises.srt; cat salut.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace Bonjour Salut --output salut.srt balises.srt; cat salut.srt
balises.srt: replaced 2 matches -> salut.srt
1
00:00:01,000 --> 00:00:02,000
<i>Salut</i> tout le monde

2
00:00:03,000 --> 00:00:04,000
Salut <i>Marie</i>, ca va
```

Ensimmäisessä kappaleessa tagi katkaisi sanan ”Bonjour”: nyt se ympäröi sanaa ”Salut”.
Toisessa siihen ei kosketa, ja se pysyy paikallaan. **Avoimeksi jäänyt tagi
jää avoimeksi**: horjuvan merkinnän korjaaminen olisi eri toiminto.

Korvaus luetaan **tiedoston muodon tageina**: `<i>…</i>` SubRip-muodossa,
`{\i1}…` Advanced SSA -muodossa. Se on ainoa tapa muotoilla kirjoitettavaa tekstiä.

## Mitään ei löytynyt, mitään ei muuteta

Kaksi vastausta, jotka eivät ole epäonnistumisia — paluuarvo on `0` ja **tiedosto kirjoitetaan
silti**, sillä annettu kohde on kirjoitettu kohde:

| Tilanne | Lause |
| :-------- | :----- |
| lauseketta ei ole missään | `"zzz" not found` |
| se löytyy, mutta jokainen osuma korvataan itsellään | `nothing to change` |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli replace zzz x --output b.srt a.srt; subedit-cli replace Bonjour Bonjour --output c.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli replace zzz x --output b.srt a.srt; subedit-cli replace Bonjour Bonjour --output c.srt a.srt
a.srt: "zzz" not found -> b.srt
a.srt: nothing to change -> c.srt
```

## `--dry-run`

Vakiotuloste luettelee **jokaisen kappaleen, jonka teksti muuttuisi**, ennen ja jälkeen, sellaisina
kuin ne ovat tiedostossa — tagit mukaan lukien:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace --dry-run Bonjour Salut balises.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace --dry-run Bonjour Salut balises.srt
balises.srt: replaced 2 matches (dry run, nothing written)
balises.srt: subtitle 1
- <i>Bon</i>jour tout le monde
+ <i>Salut</i> tout le monde
balises.srt: subtitle 2
- Bonjour <i>Marie</i>, ca va
+ Salut <i>Marie</i>, ca va
```

Luettelon muoto kuvataan kohdassa [Esikatselu ennen kirjoittamista](invocation.md#esikatselu-ennen-kirjoittamista);
JSON-muodossa se on kenttä `changes`.

## Tuloste

**Vakiotuloste** — ei mitään: tulos on kirjoitettu tiedosto. Valitsimella `--dry-run` yllä oleva
muutosluettelo.

| Taso | Mitä virhetulosteeseen lisätään |
| :----- | :------------------------------------- |
| 1 | `<path>: replaced N matches -> <destination>` tai `"<pattern>" not found` tai `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — luetun tiedoston muoto, merkistökoodaus, merkki ja rivinvaihdot palautettuina ennalleen |
| 3 | `<path>: N bytes read, M written`, sitten **jokainen huomautus luettaessa** — katso [Käynnistys](invocation.md#huomautukset-luettaessa) |

`N` laskee **korvatut osumat niissä kappaleissa, joiden teksti muuttui** — sama
luku, jonka myös ikkuna ilmoittaa.

### JSON-muodossa

Valitsimella [`--format json`](invocation.md#skriptin-luettava-tuloste) kunkin tiedoston objekti sisältää:

| Avain | Sisältö |
| :-- | :------ |
| `counts.replaced` | korvatut osumat niissä kappaleissa, joiden teksti muuttui: lauseen `N` |
| `counts.matched` | löydetyt osumat, **muuttiko niiden korvaus tekstiä tai ei** — siitä erottaa tilanteen ”mitään ei löytynyt” (`0`) tilanteesta ”mitään ei muuteta” (`replaced` on `0`, `matched` positiivinen) |
| `changes` | muuttuneet kappaleet: `{"subtitle", "document", "before", "after"}`, **tyhjä**, kun mikään ei muuttunut |

## Paluuarvot

Työkalun yleiset: `0`, jos jokainen tiedosto käsiteltiin — myös tiedosto, josta ei löytynyt mitään,
on käsitelty —, `2`, jos yhtäkään ei, `3`, jos vain osa, `1` käyttövirheessä.

## Virheet

| Mikä sen aiheuttaa | Viesti |
| :------------------ | :------ |
| tyhjä lauseke | `pattern: nothing to look for` |
| lukukelvoton säännöllinen lauseke | `pattern: not a regular expression (<ICU's reason>)` |
| väärin kirjoitettu tai tiedoston ulkopuolinen väli | katso [`adjust`](adjust.md#--range) |

**Lauseke luetaan ennen ensimmäistä tiedostoa**: lukukelvoton säännöllinen lauseke on käyttövirhe
(koodi `1`), mitään ei lueta eikä kirjoiteta.

Kohteen virheet ovat yhteisiä kaikille kirjoittaville alikomennoille:
katso [Käynnistys](invocation.md#kohde).
