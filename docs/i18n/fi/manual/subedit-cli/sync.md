# `sync`

<!-- machine-translated -->
> Tämä sivu on konekäännös, eikä kielen lukija ole vielä tarkistanut sitä.

```
subedit-cli sync (--video FILE | --audio FILE)
                 [--audio-track N]
                 [--language CODE]
                 [--start TIME] [--end TIME]
                 [--model FILE]
                 [--vad-model FILE]
                 [--engine NAME]
                 [--keep-audio FILE]
                 [--method METHOD]
                 [--min-match PERCENT]
                 [--doubt-gap MS]
                 [--report FILE]
                 [-r]
                 [--output FILE | --output-dir DIR | --in-place] [--dry-run]
                 <files>...
```

**Ajoittaa tiedoston tekstitykset uudelleen elokuvan äänen mukaan.** Komento on tehty tapausta varten, jossa verkosta haettu `.srt` on
siirtynyt alussa ja lopussa eikä palaa kohdalleen yhdellä siirrolla. Kaksi menetelmää hoitaa sen, ja `--method` (tai asetus
`alignment.method`) valitsee niiden välillä:

- **`speech`, oletus, asettaa tiedoston sen päälle, milloin puhutaan.** Se kysyy äänestä vain sitä, missä on puhetta, ja
  löytää siirron, jolla tekstitykset täsmäävät siihen, elokuvan pätkä kerrallaan: vakiosiirto, ajautuma, myöhässä oleva toinen
  puolisko ja toiselle ruutunopeudelle ajoitettu tiedosto (25 eikä 23.976 ja vastaavat) korjautuvat kaikki.
  **Se ei riipu kielestä** — tekstitykset voivat olla ranskaksi ja elokuva englanniksi —, se ei tarvitse litterointia ja se
  vie sekunteja. Se tarvitsee `ffmpeg`in, ohjelman `whisper-vad-speech-segments`, joka tulee whisper.cpp:n mukana, ja
  puheaktiivisuusmallin (`--vad-model`, asetus `transcription.vad-model`).
- **`words` vertaa jokaisen tekstityksen sanoja sanoihin, jotka moottori kuuli.** Se kuuntelee litterointimoottorilla —
  samalla kuin komento [`transcribe`](transcribe.md) — ja jokainen tekstitys saa paikan, jossa sen sanat sanotaan. Mitään
  ei käännetä: tekstitysten on oltava puhutulla kielellä. Se osaa sijoittaa yksittäisen väärässä paikassa olevan tekstityksen
  muuten oikeassa tiedostossa, ja se vie litteroinnin ajan.

**Se, mitä `speech` ei tee, sanotaan joka kerta**, virhetulosteessa ja kirjauksessa: se korjaa tiedoston
**kokonaisuutena**, ei koskaan yhtä tekstitystä kerrallaan. Tekstitys, joka on yksinään väärässä paikassa muuten oikeassa tiedostossa,
tai hiljaisuuden päällä näytetty teksti (otsikkokortti, ruudulla näkyvää selittävä teksti) ei saa omaa etsintäänsä
ja voi yhä olla väärin, ja ensimmäiset minuutit voivat jäädä pois paikaltaan, kun elokuva alkaa musiikilla tai
tekstityksillä. Tarkista tulos tai käytä valitsinta `--method words`, kun teksti on puhutulla kielellä.

**Mitään tarvittavista osista ei toimiteta ohjelman mukana**: ohjelmat, malli ja `ffmpeg` ovat samat kuin sivulla
[Litterointityökalujen asentaminen](install-transcription.md), ja niiden puuttuminen ei riko mitään — komento kertoo,
mikä puuttuu, viittaa tuohon sivuun, kirjoittaa ei mitään ja päättyy nollasta poikkeavaan koodiin.

- **Vain alut ja loput muuttuvat.** Ei koskaan teksti, tyyli, tekstitysten määrä tai niiden järjestys.
- **Tekstitys, jota ääni ei anna, säilyttää aikansa** — tekijätiedot, joita kukaan ei sano, laulu, rivi toisella kielellä.
  **Mitään paikkaa ei koskaan keksitä eikä arvata naapureista**: rivi jätetään sinne, missä tiedosto sen piti,
  ja komento kertoo sen. Se *ei* ole epäonnistuminen, ja paluuarvo on `0`.
- **Menetelmällä `words` tekstitys katsotaan löytyneeksi, kun vähintään puolet sen sanoista kuullaan** (`--min-match`,
  `alignment.min-match`), ja vähintään yksi. Kirjainkoko, aksentit, välimerkit ja merkintä eivät vaikuta; yhden kirjaimen päässä oleva
  pitkä sana lasketaan kuulluksi. `speech` sijoittaa jokaisen tekstityksen eikä ota mitään valitsimista `--language`, `--model`,
  `--engine` tai `--min-match`.
- **Loppu on siellä, missä viimeinen sana päättyy.** Moottori ajoittaa sanat likimääräisesti, muutama sadasosasekunti ennen
  tekstityksen omaa loppua, joten tekstitys voi tulla hieman lyhyempänä kuin se oli.
- **Tekstitys, joka menisi seuraavan päälle, lyhennetään kohtaan, jossa seuraava alkaa.** Tekstitys, joka menee yhä
  seuraavan päälle, mitä lyhentäminen ei korjaa, jätetään ja siitä kerrotaan.
- **Tiedostoa, joka ei sovi ääneen, ei kirjoiteta vaan se hylätään**: menetelmällä `words` yhtäkään tekstitystä ei löytynyt,
  mikä tarkoittaa lähes aina väärää ääntä tai väärää kieltä — `speech` ei riipu kielestä; menetelmällä `speech` tekstityksillä ja
  puheella on liian vähän yhteistä, mikä tarkoittaa lähes aina väärää elokuvaa.
- **Elokuvan osan voi kuunnella yksinään** valitsimilla `--start` ja `--end`, kuten komennolla
  [`transcribe`](transcribe.md#osan-litterointi): vain tekstityksiä, jotka **alkavat** alueella, etsitään ja
  muutetaan, muut jätetään ennalleen eikä niitä lasketa.
- **Ääni kuunnellaan kerran jokaista annettua tiedostoa kohden.** Jos useita tiedostoja ajoitetaan uudelleen samalla elokuvalla, odotettavissa on
  odotus jokaisen kohdalla.

<!-- example: subedit-cli sync --help -->
```console
$ subedit-cli sync --help
Re-time the subtitles of a file on the audio of a film
Usage: subedit-cli sync [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to re-time

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --video FILE                Film to listen to
  --audio FILE                Audio file to listen to, instead of a film
  --audio-track N             Audio track of a film, counted from 0; the first by default
  --start TIME                Where to begin, as 2:00, 1:02:03 or seconds; the beginning by default
  --end TIME                  Where to stop, as 2:00, 1:02:03 or seconds; the end by default
  --language CODE             Language of the speech, two letters: fr, en, de; detected from the first 30 seconds by default
  --model FILE                Model file; the transcription.model setting by default
  --vad-model FILE            Voice-activity model that skips silences; the transcription.vad-model setting by default
  --engine NAME               Engine that listens; the transcription.engine setting by default
  --keep-audio FILE           Keep the extracted audio in this file
  --method METHOD             How the subtitles are put in line: speech (from when there is speech, whatever the language) or words (from the words heard); the alignment.method setting by default
  --min-match PERCENT         Share of a subtitle's words that must be heard, in percent; the alignment.min-match setting by default
  --doubt-gap MS              How far a subtitle may move from its neighbours' shift before it is flagged, in milliseconds; the alignment.doubt-gap setting by default
  --report FILE               Write the subtitles to check to this file
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Argumentit ja valitsimet

| Valitsin | Pakollinen | Arvot | Oletus |
| :----- | :------- | :----- | :------ |
| `<files>` | kyllä | uudelleen ajoitettavat tekstitystiedostot; valitsimella `-r` hakemistot | — |
| `--recursive`, `-r` | ei | lippu | pois — katso [Hakemistopuun käsittely](lots.md) |
| `--video` | yksi valitsimista `--video` ja `--audio` | kuunneltava elokuva | — |
| `--audio` | yksi valitsimista `--video` ja `--audio` | kuunneltava äänitiedosto elokuvan sijaan | — |
| `--audio-track` | ei | kokonaisluku nollasta: paikka elokuvan ääniraitojen joukossa | `0`, ensimmäinen |
| `--language` | ei | kaksi kirjainta, ISO 639-1 -koodi: `en`, `fr` | asetus `transcription.language`; tyhjä: tunnistetaan |
| `--start`, `--end` | ei | aika: `2:00`, `1:02:03`, `2:00.5` tai sekuntimäärä; `--end`-arvon on oltava `--start`-arvon jälkeen | koko ääni |
| `--model` | ei | mallitiedosto | asetus `transcription.model` |
| `--vad-model` | ei | puheaktiivisuusmalli, joka saa moottorin ohittamaan hiljaisuudet; sen kanssa sanat seuraavat puhetta tarkemmin | asetus `transcription.vad-model` |
| `--engine` | ei | kuuntelevan moottorin nimi: `whisper.cpp` | asetus `transcription.engine` |
| `--keep-audio` | ei | tiedosto, johon purettu ääni säilytetään; sen hakemisto luodaan, jos se puuttuu | ääntä ei säilytetä missään |
| `--method` | ei | `speech`: aseta tiedosto sen päälle, milloin puhutaan, millä kielellä tahansa; `words`: vertaa sanoja kuultuun, puhutulla kielellä | asetus `alignment.method`, `speech` |
| `--min-match` | ei | kokonaisluku väliltä 1-100: osuus tekstityksen sanoista, jotka on kuultava, prosentteina | asetus `alignment.min-match`, `50` |
| `--doubt-gap` | ei | kokonaisluku millisekunteina, 0 tai enemmän: kuinka paljon tekstityksen siirto saa poiketa naapureidensa siirrosta ennen kuin se merkitään | asetus `alignment.doubt-gap`, `2000` |
| `--report` | ei | tiedosto, johon tarkistettavat tekstitykset kirjoitetaan, yksi riviä kohden: tekstityksen numero, sarkain ja syy; yksi tekstitystiedosto kerrallaan | mitään ei kirjoiteta |
| `--output` / `--output-dir` / `--in-place` | **yksi kolmesta**, paitsi valitsimen `--dry-run` kanssa | katso [Käynnistys](invocation.md#kohde) | — |
| `--dry-run` | ei | lippu: laskee ja kertoo, ei kirjoita mitään — katso [Esikatselu ennen kirjoittamista](invocation.md#esikatselu-ennen-kirjoittamista) | pois |

Kohde — `--output`, `--output-dir`, `--in-place`, `--dry-run` — ja `-r` ovat samat kuin muilla tiedostoa
uudelleenkirjoittavilla komennoilla: katso [kohde](invocation.md#kohde). Tiedosto säilyttää muotonsa, koodauksensa ja
rivinvaihtonsa.

**Asetukset** luetaan asetustiedostosta — katso [asetustiedosto](invocation.md#asetustiedosto) — ja jokaisella
on tässä oma valitsimensa. Moottori, malli ja puheaktiivisuusmalli ovat samat kuin komennolla `transcribe`.

| Avain | Merkitys | Oletus |
| :-- | :------ | :------ |
| `alignment.engine` | menetelmän `words` kohdistusmoottori; niitä on yksi, `transcript-matching`, joka kuuntelee ja vertaa | `transcript-matching` |
| `alignment.method` | miten tekstitykset kohdistetaan: `speech` tai `words` | `speech` |
| `alignment.min-match` | osuus tekstityksen sanoista, jotka on kuultava, jotta se katsotaan löytyneeksi, prosentteina | `50` |
| `alignment.doubt-gap` | kuinka paljon, millisekunteina, tekstityksen siirto saa poiketa naapureidensa siirrosta ennen kuin se merkitään | `2000` |

## Mitä merkitään

Komento kertoo, mitkä tekstitykset on tarkistettava ja miksi. Tekstitys merkitään **ensimmäisestä** pätevästä syystä:

| Miksi | Merkitys |
| :-- | :------ |
| not found in the audio, left as it was | alle `--min-match` sen sanoista kuultiin: se säilyttää aikansa |
| only partly heard | löytyi, mutta alle 80 % sen sanoista kuultiin |
| moved differently from its neighbours | löytyi, mutta sen siirto poikkeaa kummallakin puolella olevien kahden tekstityksen keskisiirrosta yli `--doubt-gap`-arvon |

Menetelmällä `speech` jokainen tekstitys sijoitetaan, ja vain viimeinen näistä voi päteä: kun tiedoston siirto muuttuu enemmän kuin
`--doubt-gap` tekstityksestä seuraavaan (myöhässä oleva toinen puolisko), askelman molemmin puolin olevat tekstitykset
merkitään.

Tekstitys voidaan myös **lyhentää seuraavan tekstityksen alkuun** tai se voi **mennä yhä seuraavan päälle**.

Kaksi lukua kertoo tiedoston kokonaistarinan. **Mediaanisiirto** on keskimmäinen arvo siitä, kuinka paljon tekstitykset
siirtyivät: pelkästään myöhässä ollut tiedosto näkyy tässä eikä missään muualla. **Ajautuma** on viimeisen kymmenesosan
tekstitysten mediaanisiirto miinus ensimmäisen kymmenesosan: se on lähellä nollaa vakiosiirrolle ja suuri tiedostolle, joka oli
myöhässä yhä enemmän elokuvan edetessä.

## Tuloste

**Vakiotuloste** — ei mitään: tulos on kirjoitettu tiedosto.

| Taso | Mitä virhetulosteeseen lisätään |
| :---- | :-------------------------------- |
| 1 | `source: <file>`, `media: <audio>`, `result: N of M subtitles placed on the audio, K to check; median shift X s, drift Y s`, `dest: <destination>` — ja kun ruutunopeus korjattiin, suhdeluku; menetelmällä `speech` **varoitus siitä, mitä se ei tee**, omalla rivillään |
| 2 | vaihe ja eteneminen, kuten komennolla [`transcribe`](transcribe.md#tuloste) |

### JSON-muodossa

Valitsimella [`--format json`](invocation.md#skriptin-luettava-tuloste) tiedoston objektissa on `counts`: `subtitles`
(etsityt), `placed` (paikan saaneet), `moved` (joiden ajat muuttuivat), `partial`, `out_of_line`, `not_found`,
`median_shift_ms` ja `drift_ms`; sekä `flagged`, lista alkioita `{"subtitle": N, "kind": "…"}`, joissa numero on
tekstityksen paikka tiedostossa ja `kind` on yksi arvoista `not-found`, `partial`, `out-of-line`, `cut-back` ja `overlaps`.
Menetelmällä `speech` `warnings` sisältää yhden alkion, jonka `kind` on `speech-only`. Tunnisteet on luvattu; lauseita ei.

## Esimerkki

```bash
subedit-cli sync --video film.mkv --vad-model silero.bin \
    --report to-check.txt --output film.synced.srt film.srt
```

Kun jokainen tekstitys halutaan sijoittaa sanojensa mukaan, puhutulla kielellä olevalle tekstille:

```bash
subedit-cli sync --method words --video film.mkv --model small.bin --vad-model silero.bin \
    --output film.synced.srt film.srt
```

## Paluuarvot

Työkalun paluuarvot: `0`, jos tiedosto kirjoitettiin, merkittyjä tekstityksiä tai ei, `2`, jos sitä ei voitu kirjoittaa — työkalu tai malli puuttuu, epäonnistunut purku tai kuuntelu, tiedosto, joka ei sovi ääneen (yhtäkään sen tekstitystä ei löytynyt tai sillä on liian vähän yhteistä puheen kanssa) —, `1` käyttövirheessä, kuten kun ei anneta kumpaakaan valitsimista
`--video` ja `--audio` (tai annetaan molemmat), valitsin on alueensa ulkopuolella tai aluetta ei voi käyttää.

## Virheet

| Syy | Viesti |
| :---- | :------ |
| ei `--video`- eikä `--audio`-valitsinta, tai molemmat | `sync needs the audio to listen to: give --video or --audio, and not both` |
| moottorin ohjelmaa ei löydy, tai määritettyä ei ole | `the transcription engine was not found`, sitten viittaus oppaaseen |
| mallia ei annettu, tai sitä ei ole | `no model was given, or the model file is not there`, sitten viittaus oppaaseen |
| `ffmpeg` ei löydy elokuvaa varten | `ffmpeg was not found, so the audio of a video cannot be extracted`, sitten viittaus |
| raita, jota elokuvalla ei ole | `the video has no audio track N`, sitten raidat, jotka sillä on |
| kieli, jota malli ei osaa litteroida | `the model cannot transcribe the language xx` |
| moottori, jota kukaan ei ole rekisteröinyt | `no transcription engine is called NAME` tai `there is no alignment engine called NAME` |
| yhtäkään tekstitystä ei löytynyt (`words`) | `none of the N subtitles was found in the audio: is it the right audio, and the right language?`, sitten `The method based on speech does not depend on the language.` |
| puheaktiivisuusmallia ei nimetty, tai puheen löytävää ohjelmaa ei löydy (`speech`) | `Finding where there is speech needs a voice-activity model: name one in the transcription.vad-model setting.` tai `Finding where there is speech needs the program whisper-vad-speech-segments, which comes with whisper.cpp, and it was not found.`, sitten viittaus oppaaseen |
| tekstityksillä ja puheella on liian vähän yhteistä (`speech`) | `The sound and the subtitles have too little in common to be put in line: is it the right film?` |
| `--min-match` väliltä 1-100 ulkopuolella, `--doubt-gap` negatiivinen | valitsimen käyttövirhe |
| `--report` useammalla kuin yhdellä tekstitystiedostolla | `--report names one file: give one subtitle file when you use it` |
