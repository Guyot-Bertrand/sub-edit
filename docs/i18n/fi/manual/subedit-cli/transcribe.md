# `transcribe`

<!-- machine-translated -->
> Tämä sivu on konekäännös, eikä kielen lukija ole vielä tarkistanut sitä.

```
subedit-cli transcribe --output FILE
                       [--to FORMAT]
                       [--audio-track N]
                       [--language CODE]
                       [--start TIME] [--end TIME]
                       [--model FILE]
                       [--engine NAME]
                       [--keep-audio FILE]
                       [--raw]
                       <input>
```

Kirjoittaa **videon tai äänitiedoston puheen tekstitystiedostoksi**. Ääni valmistellaan,
annetaan moottorille, joka selvittää, mitä sanotaan ja milloin, ja sen leikkaamat palat yhdistetään
tekstityksiksi, jotka noudattavat tiettyä kestoa ja pituutta.

**Mitään tarvittavista osista ei toimiteta ohjelman mukana**: moottorin (whisper.cpp), mallin ja `ffmpeg`in
asentaa käyttäjä, joka kertoo, missä malli on. [Litterointityökalujen
asentaminen](install-transcription.md) kertoo, mistä kukin löytyy ja minkä mallin valitsee. **Niiden puuttuminen ei
riko mitään**: komento kertoo, mikä puuttuu, ja viittaa tähän sivuun, kirjoittaa ei mitään ja
päättyy nollasta poikkeavaan koodiin.

- **Syöte on video tai äänitiedosto.** Videosta `ffmpeg` purkaa yhden ääniraidan; ilman
  `ffmpeg`iä äänitiedosto, jonka moottori lukee sellaisenaan (`wav`, `mp3`, `flac`, `ogg`), litteroidaan silti,
  mutta video ei, eikä mikään tiedosto, kun raitaa pyydetään.
- **Vain yksi tiedosto kerrallaan**, ja kohde on `--output`: `--output-dir`iä tai
  `--in-place`a ei ole, koska tulos on uusi tiedosto eikä uudelleenkirjoitus.
- **Kieli tunnistetaan**, ellei `--language` pakota sitä, ja käytetty kieli kerrotaan. Malli,
  joka on tehty vain englannille, ei tunnista mitään eikä litteroi muuta: kieli, jota se ei voi
  käsitellä, hylätään *ennen* työn alkua, ja mallin raja nimetään.
- **Kieli kirjoitetaan kahdella kirjaimella — `fr`, `en`, `de` — tai kolmikirjaimisella koodilla, jonka tiedosto ilmoittaa, `fra`,
  `eng`, joka luetaan kaksikirjaimisena.** Kaikki muu hylätään ennen työn alkua. **Tunnistus
  kuuntelee vain ensimmäiset kolmekymmentä sekuntia**: elokuva, joka alkaa musiikilla tai hiljaisuudella, voidaan ottaa
  toiseksi kieleksi, ja moottori kirjoittaa silloin puheen *käännettynä* tälle kielelle — englanniksi luultu ranskalainen
  elokuva tulee ulos englanniksi tai muodossa `[SPEAKING FRENCH]`. Kun tiedät kielen, anna se.
- **Elokuvan osan voi litteroida yksinään** valitsimilla `--start` ja `--end` — katso
  [Osan litterointi](#osan-litterointi). Näin mallia kokeillaan kahdella minuutilla ennen kuin se ajetaan
  kahdelle tunnille.
- **Äänet eivät kirjoitu muistiinpanoina**: moottorille on annettu käsky olla tekemättä niin (sen oma valitsin on `--suppress-nst`), jotta
  `(Sonnerie de téléphone)` tai `[MUSIC PLAYING]` eivät nouse tekstityksiin, ei sulkeissa eikä hakasulkeissa.
- **Segmentit yhdistetään ja leikataan** asetusten rajojen mukaan (`transcription.merge-gap-ms`,
  `transcription.max-duration-ms`, `transcription.max-characters`): kaksi saman lauseen palaa, jotka ovat
  lähellä toisiaan, muodostavat yhden tekstityksen, ja liian pitkä tai liian leveä pala leikataan sanan kohdalta. **`--raw`
  kirjoittaa moottorin segmentit sellaisina kuin ne tulivat**, yhden tekstitystä kohti, ja siten näkee myös, mitä
  yhdistäminen tekee.

<!-- example: subedit-cli transcribe --help -->
```console
$ subedit-cli transcribe --help
Write the speech of a video or an audio file as subtitles
Usage: subedit-cli transcribe [OPTIONS] input

Positionals:
  input TEXT REQUIRED         Video or audio file to transcribe

Options:
  -h,--help                   Print this help message and exit
  --output FILE               Subtitle file to write
  --to TEXT:{srt,vtt,subviewer2,ssa,ass,microdvd,mpl2,tmplayer,lrc}
                              Format to write; the one the extension of --output says by default
  --audio-track N             Audio track of a video, counted from 0; the first by default
  --start TIME                Where to begin, as 2:00, 1:02:03 or seconds; the beginning by default
  --end TIME                  Where to stop, as 2:00, 1:02:03 or seconds; the end by default
  --language CODE             Language of the speech, two letters: fr, en, de; detected from the first 30 seconds by default
  --model FILE                Model file; the transcription.model setting by default
  --engine NAME               Engine; the transcription.engine setting by default
  --keep-audio FILE           Keep the extracted audio in this file
  --raw                       Write the segments as the engine cut them
```

## Argumentit ja valitsimet

| Valitsin | Pakollinen | Arvot | Oletus |
| :----- | :------- | :----- | :------ |
| `<input>` | kyllä | video tai äänitiedosto | — |
| `--output` | kyllä | kirjoitettava tekstitystiedosto; sen hakemisto luodaan, jos se puuttuu | — |
| `--to` | ei | muodon lyhyt nimi: `srt`, `vtt`, `subviewer2`, `ssa`, `ass`, `microdvd`, `mpl2`, `tmplayer`, `lrc` | muoto, jonka `--output`-tiedoston pääte kertoo, muuten `srt` |
| `--audio-track` | ei | kokonaisluku nollasta: paikka videon ääniraitojen joukossa | `0`, ensimmäinen |
| `--language` | ei | kaksi kirjainta, ISO 639-1 -koodi: `en`, `fr`; kolmikirjaiminen koodi kuten `fra` luetaan muodossa `fr` | asetus `transcription.language`; tyhjä: tunnistetaan |
| `--start` | no | aika: `2:00`, `1:02:03`, `2:00.5` tai sekuntimäärä, `90` | äänen alku |
| `--end` | no | aika, kirjoitettuna kuten valitsimelle `--start`; sen on oltava `--start`-arvon jälkeen | äänen loppu |
| `--model` | ei | mallitiedosto | asetus `transcription.model` |
| `--engine` | ei | moottorin nimi: `whisper.cpp` | asetus `transcription.engine` |
| `--keep-audio` | ei | tiedosto, johon purettu ääni säilytetään 16 kHz:n 16-bittisenä mono-PCM:nä WAV-tiedostossa; sen hakemisto luodaan, jos se puuttuu | ääntä ei säilytetä missään |
| `--raw` | ei | lippu: kirjoittaa moottorin segmentit ilman yhdistämistä ja leikkaamista | pois |

**Asetukset** luetaan asetustiedostosta kuten komennon [`adjust`](adjust.md) asetukset — katso
[asetustiedosto](invocation.md#asetustiedosto) — ja jokaisella on tässä oma valitsimensa:

| Avain | Merkitys | Oletus |
| :-- | :------ | :------ |
| `transcription.engine` | moottori | `whisper.cpp` |
| `transcription.executable` | moottorin ohjelma, absoluuttinen polku; tyhjä: haetaan `PATH`-polusta | tyhjä |
| `transcription.model` | mallitiedosto, absoluuttinen polku | tyhjä |
| `transcription.language` | äänen kieli, kaksi kirjainta; tyhjä: tunnistetaan | tyhjä |
| `transcription.merge-gap-ms` | saman lauseen palat, jotka ovat näin monta millisekuntia lähempänä toisiaan, yhdistetään | `400` |
| `transcription.max-duration-ms` | pisin aika, jonka tekstitys pysyy näytöllä, millisekunteina | `7000` |
| `transcription.max-characters` | tekstityksen merkkien enimmäismäärä, rivinvaihtoja ei lasketa | `84` |

## Äänen valinta

Videolla voi olla useita ääniraitoja — alkuperäinen, selostus, toinen kieli. `--audio-track`
ottaa **paikan tiedoston ääniraitojen joukossa** nollasta alkaen siinä järjestyksessä, jossa tiedosto ne luettelee.
Raita, jota videolla ei ole, hylätään, **ja samalla näytetään raidat, jotka sillä on**, jotta seuraava kutsu
on oikea:

```bash
subedit-cli transcribe --audio-track 4 --output film.srt --model small.bin film.mkv
```

```text
film.mkv: the video has no audio track 4
audio track 0: fra, Original
audio track 1: eng, Commentary
```

## Osan litterointi

`--start` ja `--end` rajaavat työn elokuvan niiden väliseen osaan. Toisen voi antaa yksinään: pelkkä
`--start` jatkuu loppuun asti, pelkkä `--end` alkaa alusta.

```bash
subedit-cli transcribe --start 2:00 --end 3:30 --output test.srt --model large-v3.bin film.mkv
```

- **Aika kirjoitetaan muodossa `M:SS`, `H:MM:SS`, kummassakin desimaaliosa pisteen jälkeen (`2:00.5`), tai
  sekuntimääränä (`90`, `90.5`)**. Pilkku hylätään, kuten kaikkialla työkalussa.
- **Tekstitykset säilyttävät elokuvan ajan**: elokuvan kohdassa 2:10 sanottu rivi kirjoitetaan kohtaan `00:02:10`, ei
  kohtaan `00:00:10`. Tiedoston voi asettaa koko elokuvan päälle sellaisenaan.
- **Reunat ovat likimääräisiä**: moottori työskentelee kolmenkymmenen sekunnin ikkunoissa, joten sen palauttama
  rajataan jälkeenpäin alueeseen; reunan yli menevä lause katkaistaan siihen.
- **Mitään ei aloiteta, jos aluetta ei voi käyttää**: raja, joka ei ole aika, tai `--end`, joka ei ole
  `--start`-arvon jälkeen, on käyttövirhe, joka nimeää ongelman, ei kirjoita mitään eikä käynnistä `ffmpeg`-ohjelmaa eikä
  moottoria.

Koko elokuvan ääni puretaan edelleen: alue säästää litteroinnin, joka kestää paljon pidempään.

## Tuloste

**Vakiotuloste** — ei mitään: tulos on kirjoitettu tiedosto.

| Taso | Mitä virhetulosteeseen lisätään |
| :---- | :-------------------------------- |
| 1 | `<input>: N subtitles transcribed, language: xx -> <output>` |
| 2 | vaihe ja eteneminen, yksi rivi jokaista prosenttia kohti: `extracting the audio…`, sitten `transcribing…`, sitten `transcribing: 25%, 0:12:30 of 0:50:00, about 38 min left, elapsed: 0:05:12` — prosentti, äänessä saavutettu kohta, jäljellä oleva aika ja kulunut aika; **rivi sanotaan uudelleen kymmenen sekunnin välein, kun mitään uutta ei ole tullut**, jotta pitkää odotusta ei luulla jumiutumiseksi |
| 3 | ja jokainen uusi teksti, jonka moottori saa, `last text: …` |

**Prosentti on moottorin kohta äänessä, ei sen omien ikkunoiden laskuri**: moottori kirjoittaa litteroimansa
sitä mukaa kuin se etenee, ja komento lukee, mihin asti se on päässyt, joten luku liikkuu ensimmäisestä
minuutista alkaen. (Moottorin oma eteneminen tulee 5 %:n askelin äänestä, kun kolmenkymmenen sekunnin ikkuna on valmis:
yksinään se voi pysyä 0 %:ssa vartin pitkällä elokuvalla, jossa on suuri malli prosessorilla.) **Jäljellä oleva
aika vaikenee aluksi**: se annetaan, kun työtä on tehty riittävästi ekstrapoloitavaksi, ja se seuraa viimeisen
minuutin nopeutta, ei koko ajon keskiarvoa.

### JSON-muodossa

Valitsimella [`--format json`](invocation.md#skriptin-luettava-tuloste) (yleinen valitsin, kirjoitetaan ennen
alikomentoa) tiedoston objektissa on käytetty kieli kentässä `language`, `counts.subtitles` (kirjoitetut
tekstitykset) ja `counts.segments` (moottorin palat, joista ne tehtiin) sekä `audio`, kun
`--keep-audio` säilytti äänen.

## Huomautukset

Kaksi huomautusta sanotaan ensimmäisellä tasolla, koska ne muuttavat sitä, mitä on odotettavissa:

- **Ennen työtä, kun malli on vain englannille**: `This model is for English only: whatever language the film
  speaks, the result will be English, or notes in brackets. Choose a multilingual model in the preferences.` Malli,
  jonka tiedostonimi päättyy `.en`, on sellainen.
- **Työn jälkeen, kun moottori on kirjoittanut enimmäkseen hakasulkeissa olevia merkintöjä** — `[MUSIC PLAYING]`, `[SPEAKING FRENCH]` —,
  vähintään kaksi kolmasosaa sen paloista: `N of the M pieces are only notes in brackets, such as [MUSIC PLAYING]: the
  language used (xx) is probably not the one spoken, and the engine may have translated the speech. Run it again with
  the language typed.` Tekstitykset kirjoitetaan silti; huomautus kertoo, mitä epäillä.

## Paluuarvot

Työkalun paluuarvot: `0`, jos tiedosto kirjoitettiin, `2`, jos sitä ei voitu kirjoittaa — työkalu tai malli puuttuu, kieli,
jota malli ei voi käsitellä, epäonnistunut purku tai litterointi —, `1` käyttövirheessä, kuten
moottorissa, jota kukaan ei ole rekisteröinyt, tai alueessa, jota ei voi käyttää.

## Virheet

| Syy | Viesti |
| :---- | :------ |
| moottorin ohjelmaa ei löydy, tai määritettyä ei ole | `the transcription engine was not found`, sitten viittaus oppaaseen |
| mallia ei annettu, tai sitä ei ole | `no model was given, or the model file is not there`, sitten viittaus oppaaseen |
| `ffmpeg` ei löydy videota varten | `ffmpeg was not found, so the audio of a video cannot be extracted`, sitten viittaus |
| raita, jota videolla ei ole | `the video has no audio track N`, sitten raidat, jotka sillä on |
| kieli, jota malli ei osaa litteroida | `the model cannot transcribe the language xx` |
| ei kieltä, ja malli, joka ei osaa tunnistaa kieltä | `the model cannot detect the language: give one` |
| moottori, jota kukaan ei ole rekisteröinyt | `no transcription engine is called NAME` |
| kieli, joka ei ole koodi | `"TEXT" is not a language code: write two letters, such as fr, en, de or es` |
| alueen raja, joka ei ole aika | `"TEXT" is not a time: write it as 2:00, 1:02:03 or 2:00.5, or in seconds` |
| `--end` ei ole `--start`-arvon jälkeen | `the end of the range must come after its start` |
| mikä tahansa muu virhe | `the transcription failed`, ja se, mitä moottori sanoi |

Viittaus oppaaseen on `See the manual page "Installing the transcription tools"
(install-transcription).`
