# Hakemistopuun käsittely

<!-- machine-translated -->
> Tämä sivu on konekäännös, eikä kukaan tämän kielen lukija ole vielä tarkistanut sitä.

```
subedit-cli <subcommand> --recursive [options] <directory>...
```

Jokainen alikomento, joka lukee tiedostoerän — `inspect` mukaan lukien, mutta ei
[`append`](append.md), [`split-file`](split-file.md) eikä [`pair`](pair.md), joiden argumentit ovat pari tai
yksi elokuva — hyväksyy tiedoston sijaan **hakemiston** valitsimella `--recursive` (`-r`).
Tämä sivu kertoo, mitä läpikäynti ottaa mukaan, missä järjestyksessä ja minne se kirjoittaa.

**Mikään seuraavasta ei muuta sitä, mitä tiedostolle tehdään**: kukin luetaan,
käsitellään ja kirjoitetaan erikseen, yhden epäonnistuminen ei pysäytä muita, ja
neljä paluuarvoa ovat [käynnistyksen](invocation.md#paluuarvot) paluuarvot.
Läpikäynti tuottaa luettelon poluista, jonka erän loppuosa käsittelee ikään kuin ne
olisi kirjoitettu komentoriville.

## Ilman valitsinta `--recursive` hakemisto hylätään

Käyttövirhe, koodi `1`, yhtään tiedostoa ei käsitellä eikä kirjoiteta:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?" -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?"
films: is a directory: use --recursive
code=1
```

## Mitä läpikäynti ottaa mukaan

Läpikäynti on **rekursiivinen, deterministinen eikä yllätä**:

- **järjestys** — kunkin hakemiston merkinnät otetaan nimiensä
  järjestyksessä **tavu tavulta** (isot kirjaimet ennen pieniä koneen
  kielestä riippumatta), ja alihakemisto käydään läpi omalla paikallaan, syvyys
  ensin. Kaksi ajoa antaa samat tavut jokaisella
  koneella;
- **symbolisia linkkejä ei seurata** — ei tiedostoon eikä
  hakemistoon: puulla, joka sisältää linkin johonkin ylähakemistoonsa, ei olisi
  loppua;
- **piilotetut tiedostot ja hakemistot ohitetaan** — ne, joiden nimi alkaa
  pisteellä;
- **vain tunnettujen muotojen tiedostopäätteet otetaan mukaan**: `.srt`, `.vtt`,
  `.ssa`, `.ass`, `.lrc`, `.sub`, kirjainkoosta riippumatta. **`.txt` on suljettu pois**: se
  tarkoittaa kahta muotoa, mutta ennen kaikkea maailman jokaista tiedostoa nimeltä `LISEZMOI.txt`, ja erä, joka
  epäonnistuisi jokaisen kohdalla, olisi käyttökelvoton.

**Komentorivillä nimettyä tiedostoa ei koskaan suodateta.** Sen nimeäminen
on sen haluamista: `subedit-cli shift --by 1 --in-place notes.txt` käsittelee sen sen
tiedostopäätteestä riippumatta ja valittaa vain, jos sen sisältö ei ole missään muodossa.

**Sivuun jätetyt lasketaan**, ja niistä kerrotaan tasolla `-vv`:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli -vv inspect --recursive films 2>&1 | head -1 -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli -vv inspect --recursive films 2>&1 | head -1
films: 1 file left aside, with no extension of a known format
```

Hakemisto, jossa ei ole mitään läpikäytävää, kertoo sen oletustasolla
(`<directory>: holds no file in a format this tool walks for`), eikä se ole virhe.

## Minne läpikäynti kirjoittaa

**Puu säilytetään valitsimen `--output-dir` alla**: tiedoston polku
**suhteessa annettuun hakemistoon** toistetaan tulostehakemiston alle. Annetun
hakemiston nimi ei kuulu siihen — tämä on komennon `rsync` semantiikka
kauttaviiva lopussa, ei komennon `cp -r`: kohde nimetään, ja siihen lisätään
haluttu taso (`--output-dir sortie/films`).

<!-- example: mkdir -p films/a/b; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/x.srt; cp films/x.srt films/a/x.srt; cp films/x.srt films/a/b/y.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli shift --by 1 --recursive --output-dir sortie films; find sortie -type f | sort -->
```console
$ mkdir -p films/a/b; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/x.srt; cp films/x.srt films/a/x.srt; cp films/x.srt films/a/b/y.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli shift --by 1 --recursive --output-dir sortie films; find sortie -type f | sort
films/a/b/y.srt: 1 subtitle shifted by 1.000 s -> sortie/a/b/y.srt
films/a/x.srt: 1 subtitle shifted by 1.000 s -> sortie/a/x.srt
films/x.srt: 1 subtitle shifted by 1.000 s -> sortie/x.srt
3 of 3 files shifted
sortie/a/b/y.srt
sortie/a/x.srt
sortie/x.srt
```

Tulostehakemisto ja puun hakemistot **luodaan**, jos niitä ei ole. Valitsimella
`--in-place` jokainen mukaan otettu tiedosto kirjoitetaan takaisin paikalleen. Tiedostopääte, jonka
`convert` muuttaa, muuttuu myös puussa: WebVTT-muotoon muunnettu
`films/a/x.srt` kirjoitetaan nimellä `sortie/a/x.vtt`.

**Tulostehakemisto suljetaan läpikäynnin ulkopuolelle, jos se on läpikäytävän puun sisällä**:
toinen ajo ei lue uudelleen sitä, minkä ensimmäinen kirjoitti.

## Kaksi hakemistoa ja törmäykset

[Kohteen](invocation.md#kohde) sääntö ratkaisee asian: kaksi tiedostoa,
jotka päätyisivät samaan polkuun, hylätään **ennen kuin mitään kirjoitetaan**, koodi `1`, ja
molemmat syötteet ja kohde nimetään.

<!-- example: mkdir -p un/a deux/a; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > un/a/x.srt; cp un/a/x.srt deux/a/x.srt; subedit-cli shift --by 1 -r --output-dir sortie un deux; echo "code=$?" -->
```console
$ mkdir -p un/a deux/a; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > un/a/x.srt; cp un/a/x.srt deux/a/x.srt; subedit-cli shift --by 1 -r --output-dir sortie un deux; echo "code=$?"
sortie/a/x.srt: would be written by both un/a/x.srt and deux/a/x.srt
code=1
```

## Mitä se maksaa

Erä pysyy peräkkäisenä: sen kesto on sen tiedostojen kestojen summa.
Suorituskykytesti `make bench` mittaa läpikäynnin ja saman työn yhdessä tiedostossa; luvut
ovat [suorituskykylokissa](../../../../mesures/performances.md).
