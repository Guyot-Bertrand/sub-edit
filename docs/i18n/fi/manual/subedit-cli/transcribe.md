# `transcribe`

<!-- machine-translated -->
> Tämä sivu on konekäännös, eikä kielen lukija ole vielä tarkistanut sitä.

```
subedit-cli transcribe --output FILE
                       [--to FORMAT]
                       [--audio-track N]
                       [--language CODE]
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
  --language CODE             Language of the speech: fr, en; detected by default
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
| `--language` | ei | kaksi pientä kirjainta, ISO 639-1 -koodi: `en`, `fr` | asetus `transcription.language`; tyhjä: tunnistetaan |
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

## Tuloste

**Vakiotuloste** — ei mitään: tulos on kirjoitettu tiedosto.

| Taso | Mitä virhetulosteeseen lisätään |
| :---- | :-------------------------------- |
| 1 | `<input>: N subtitles transcribed, language: xx -> <output>` |
| 2 | moottorin eteneminen, `transcribing: 25%`, sellaisena kuin se sen ilmoittaa — noin kolmenkymmenen äänisekunnin välein |

### JSON-muodossa

Valitsimella [`--format json`](invocation.md#skriptin-luettava-tuloste) (yleinen valitsin, kirjoitetaan ennen
alikomentoa) tiedoston objektissa on käytetty kieli kentässä `language`, `counts.subtitles` (kirjoitetut
tekstitykset) ja `counts.segments` (moottorin palat, joista ne tehtiin) sekä `audio`, kun
`--keep-audio` säilytti äänen.

## Paluuarvot

Työkalun paluuarvot: `0`, jos tiedosto kirjoitettiin, `2`, jos sitä ei voitu kirjoittaa — työkalu tai malli puuttuu, kieli,
jota malli ei voi käsitellä, epäonnistunut purku tai litterointi —, `1` käyttövirheessä, kuten
moottorissa, jota kukaan ei ole rekisteröinyt.

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
| mikä tahansa muu virhe | `the transcription failed`, ja se, mitä moottori sanoi |

Viittaus oppaaseen on `See the manual page "Installing the transcription tools"
(install-transcription).`
