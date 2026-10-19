# `framerate`

<!-- machine-translated -->
> Tämä sivu on konekäännös, eikä kukaan tämän kielen lukija ole vielä tarkistanut sitä.

> **Tämä ei ole [`snap`](snap.md).** Tämä komento **skaalaa
> tiedoston**: se on tarkoitettu tilanteeseen, jossa ajoitus on väärä ja valuu useita
> sekunteja elokuvan loppuun mennessä. `snap` palauttaa aikaleimat lähimmälle
> ruudukolle ajoittamatta mitään uudelleen, ja se on tarkoitettu tilanteeseen, jossa ajoitus on jo
> oikea. Molemmilla on sama muoto, yksi tai kaksi ruutunopeutta, eikä niiden sekoittaminen aiheuta
> virhettä.

```
subedit-cli framerate --from <fps> --to <fps>
                      (--output FILE | --output-dir DIR | --in-place)
                      [--dry-run]
                      [--recursive]
                      <file>...
```

Ajoittaa yhdelle ruutunopeudelle ajoitetun tiedoston uudelleen toiselle. Tätä
toimintoa tekstitystiedosto tarvitsee, kun se on haettu kopiolle, joka
ei ole sama kuin käytössä oleva elokuva — esimerkiksi PAL-DVD NTSC-siirtoa vasten.

Luetun tiedoston muoto **säilytetään**: muodon vaihtaminen on alikomennon
[`convert`](convert.md) tehtävä.

<!-- example: subedit-cli framerate --help -->
```console
$ subedit-cli framerate --help
Re-time a file mastered at one frame rate for another
Usage: subedit-cli framerate [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to re-time

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --from TEXT REQUIRED        Frame rate the file is timed at: 25, 23.976
  --to TEXT REQUIRED          Frame rate to time it for: 24, 29.97
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
| `--from` | **kyllä** | ruutunopeus, katso alla | — |
| `--to` | **kyllä** | ruutunopeus, katso alla | — |
| `--output` / `--output-dir` / `--in-place` | **yksi kolmesta**, paitsi valitsimen `--dry-run` kanssa | katso [Käynnistys](invocation.md#kohde) | — |
| `--dry-run` | ei | lippu: laskee ja kertoo, ei kirjoita mitään — katso [Esikatselu ennen kirjoittamista](invocation.md#esikatselu-ennen-kirjoittamista) | pois |

`--from` on ruutunopeus, jolle tiedosto **on** ajoitettu, `--to` se, jolle
se halutaan. Niiden vaihtaminen keskenään siirtää tiedostoa
toiseen suuntaan — noin seitsemän sekuntia pitkässä elokuvassa ruutunopeuksien 25 ja
23,976 välillä — eikä mikään ilmaise sitä ennen kuin elokuva katsotaan.

## Ruutunopeuden kirjoittaminen

Ruutuina sekunnissa, desimaalit mukaan lukien, erottimena piste:

```
25      24      23.976      29.97      60
```

**Nolla tai negatiivinen ruutunopeus hylätään**: se ei ole ruutunopeus.

### Kolmea NTSC-ruutunopeutta ei voi kirjoittaa tarkasti

`23.976`, `29.97` ja `59.94` ovat **nimilappuja**. Ruutunopeudet, joita ne
nimeävät, ovat `24000/1001`, `30000/1001` ja `60000/1001`, eikä yhdelläkään niistä ole
päättyvää desimaaliesitystä. Työkalu tunnistaa ne sellaisiksi:

| Mitä kirjoitetaan | Mitä se nimeää |
| :------------- | :------------------ |
| `23.976` | `24000/1001` — NTSC-videolle siirretty elokuva |
| `24` | `24` — elokuva |
| `25` | `25` — PAL- ja SECAM-video |
| `29.97` | `30000/1001` — NTSC-video |
| `30`, `50`, `60` | itsensä |
| `59.94` | `60000/1001` |

Tunnistus koskee **arvoa eikä merkkejä**: `23.9760`
nimeää saman ruutunopeuden kuin `23.976`.

Mikä tahansa muu desimaaliluku otetaan kirjaimellisesti — `23.9` ei ole kenenkään
standardi, siihen ei ole mitään tulkittavaa. Siksi yllä oleva taulukko on myös
tarpeen: ilman sitä `23.976` olisi `23976/1000`, ja komentorivi
sanoisi samoilla sanoilla eri asian kuin ikkuna.

## Mitä kohdista tulee

```
t′ = round( t × (from / to) )
```

Ruutunopeudelle 23,976 masteroitu ja nopeudella 25 ruutua sekunnissa toistettu elokuva kulkee nopeammin,
joten sen kappaleet saapuvat aikaisemmin; toiseen suuntaan ne saapuvat
myöhemmin.

**Kerroin on tarkka rationaaliluku, ja pyöristys tehdään vain kerran.** Polku,
joka näyttää samanarvoiselta — kohdan muuntaminen ruudun numeroksi
syötteen nopeudella ja sitten takaisin millisekunneiksi tulosteen nopeudella —
ei ole sitä: se kvantisoi jokaisen kohdan syötteen ruudukolle ja erehtyy jopa
puolen ruudun verran, mitattuna 21 ms. Katso
[ADR 0013](../../../../adr/0013-mise-a-l-echelle-exacte-des-positions.md).

Käytännössä ruutunopeudesta 25 nopeuteen 23,976 kerroin on `1001/960`:

| Luettu kohta | Kirjoitettu | Mitä ruutujen kautta kulkeminen antaisi |
| -----------: | -----: | -----------------------------------------: |
| `1 010 ms` | `1 053 ms` | `1 043 ms` |
| `3 600 017 ms` | `3 753 768 ms` | `3 753 750 ms` |

<!-- example: printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 25 --to 23.976 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 25 --to 23.976 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle retimed from 25 to 24000/1001 fps -> b.srt
1
00:00:01,053 --> 00:00:02,106
Un.
```

## Tuloste

**Vakiotuloste** — ei mitään: tulos on kirjoitettu tiedosto. Valitsimella `--dry-run` ei myöskään mitään: tiedostoa ei ole, ja kerrontarivi kertoo sen — katso [Esikatselu ennen kirjoittamista](invocation.md#esikatselu-ennen-kirjoittamista).

| Taso | Mitä virhetulosteeseen lisätään |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles retimed from 25 to 24000/1001 fps -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — luetun tiedoston muoto, merkistökoodaus, merkki ja rivinvaihdot palautettuina ennalleen |
| 3 | `<path>: N bytes read, M written`, sitten **jokainen huomautus luettaessa** — katso [Käynnistys](invocation.md#huomautukset-luettaessa) |

**Kerronta nimeää tarkan ruutunopeuden, ei kirjoitettua nimilappua.** Jos tuolle riville
kirjoitettaisiin `23.976`, se ilmoittaisi muunnoksesta, jota ei tapahtunut; se on
ainoa paikka, josta näkee, kumpaa käytettiin. Ruutunopeus, jonka
desimaaliluku ilmaisee tarkasti, näytetään siellä desimaalina, muut murtolukuna.

### JSON-muodossa

Valitsimella [`--format json`](invocation.md#skriptin-luettava-tuloste) kunkin tiedoston objekti sisältää kentän `counts.subtitles` (tiedoston kaikki kappaleet). Koko taulukko on sivulla [Käynnistys](invocation.md#mitä-kukin-alikomento-kirjoittaa-kenttään-counts).

## Paluuarvot

Työkalun yleiset: `0`, jos jokainen tiedosto kirjoitettiin, `2`, jos yhtäkään ei, `3`, jos
vain osa, `1` käyttövirheessä.

Mikään hylkäys ei riipu tiedoston sisällöstä: kaksi ruutunopeutta määrittää aina
kertoimen. Se on kirjoittavista alikomennoista ainoa, jonka kaikki
hylkäykset tulevat ennen ensimmäistä lukemista.

<!-- example: printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 0 --to 25 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 0 --to 25 --output b.srt a.srt; echo "code=$?"
"0" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed
code=1
```

## Virheet

| Mikä sen aiheuttaa | Viesti |
| :------------------ | :------ |
| nolla tai negatiivinen ruutunopeus | `"…" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed` |
| desimaalipilkku | `"…" is not a frame rate: use a decimal point and not a comma, in every language` |
| teksti, joka ei ole luku | `"…" is not a frame rate: expected frames per second, like 25 or 23.976` |
| täysin kohtuuton ruutunopeus | `"…" is not a frame rate: no video runs at that many frames per second` |

**Mitään ei kirjoiteta ilman jotakin kolmesta kohdevalitsimesta**, ja niiden
virheet ovat yhteisiä kaikille kirjoittaville alikomennoille:
katso [Käynnistys](invocation.md#kohde).
