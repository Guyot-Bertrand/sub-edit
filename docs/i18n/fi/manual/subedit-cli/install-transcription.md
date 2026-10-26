# Litterointityökalujen asentaminen

<!-- machine-translated -->
> Tämä sivu on konekäännös, eikä kielen lukija ole vielä tarkistanut sitä.

Äänen litterointi tekstitykseksi — [`transcribe`](transcribe.md) komentorivillä,
[`Transcribe Audio…`](../subedit-gui/video.md#äänen-litterointi) ikkunassa — tarvitsee kolme
asiaa, joita **subedit ei toimita eikä lataa**: moottorin, mallin tälle moottorille ja
`ffmpeg`in. Asennat ne itse ja kerrot subeditille, missä malli on.

**Ilman niitä mikään ei mene rikki.** Ohjelman muu osa toimii kuten ennenkin; vain litterointi
poistuu käytöstä ja näyttää viestin, joka viittaa tänne.

## Moottori: whisper.cpp

[whisper.cpp](https://github.com/ggml-org/whisper.cpp) on moottori, jota subedit ajaa. Se on ohjelma,
`whisper-cli`, joka toimii tavallisella suorittimella — näytönohjain vain nopeuttaa sitä. Rakenna se
sen oman dokumentaation mukaan (C++-kääntäjä ja CMake riittävät):

```bash
git clone https://github.com/ggml-org/whisper.cpp
cd whisper.cpp
cmake -B build -DBUILD_SHARED_LIBS=OFF
cmake --build build -j --config Release
```

Ohjelma on tämän jälkeen `build/bin/whisper-cli`. Kun se koostetaan valinnalla `-DBUILD_SHARED_LIBS=OFF`, kuten yllä, se on yksittäinen tiedosto, joka ei tarvitse rinnalleen mitään kirjastoa, joten sen voi kopioida minne tahansa. Aseta se paikkaan, josta `PATH` sen löytää, tai **anna sen polku** subeditille asetuksella `transcription.executable` — ikkunassa sitä varten on [asetuksissa](../subedit-gui/preferences.md#litterointi) kenttä. Näytönohjainta varten tehty koonti lisää ensimmäiselle `cmake`-riville yhden valinnan: `-DGGML_CUDA=1` NVIDIA-kortille, `-DGGML_VULKAN=1` useimmille muille; subedit ajaa sen, minkä olet asentanut, eikä valitse mitään puolestasi.

Jos haluat säilyttää vain ohjelman, **asenna** se ja poista klooni:

```bash
cmake --install build --prefix ~/.local
```

`--prefix` kertoo, minne. `~/.local` ei vaadi järjestelmänvalvojan oikeuksia, ja sen `bin`-hakemiston on oltava `PATH`-polullasi; ilman valitsinta oletus on `/usr/local`, joka vaatii `sudo`:n. `whisper-cli` päätyy tämän etuliitteen `bin`-hakemistoon muiden työkalujen viereen, joita subedit ei käytä; kun se on siellä, klooni ja sen `build`-hakemisto voi poistaa.

## Malli

Moottori lukee **mallin**, tiedoston, jonka koko on muutamasta sadasta megatavusta muutamaan gigatavuun ja jonka
lataat kerran. Viralliset mallit on lueteltu osoitteessa
[huggingface.co/ggerganov/whisper.cpp](https://huggingface.co/ggerganov/whisper.cpp), ja
moottorin tietovarastossa on komentosarja, joka hakee yhden:

```bash
sh ./models/download-ggml-model.sh small
```

Anna sitten sen polku valitsimella `--model` komentorivillä tai asetuksella `transcription.model`
tai ikkunan [asetuksissa](../subedit-gui/preferences.md#litterointi), jotka muistavat sen.

### Puheaktiivisuusmalli, jotta alku osuu puheen kohdalle

Valinnaisesti **puheaktiivisuusmalli** (Silero) antaa moottorin ohittaa hiljaisuudet ennen dekoodausta. Ilman sitä whisper aloittaa ensimmäisen tekstityksen kohdasta 0 riippumatta siitä, mitä elokuvassa tulee ensin, musiikki mukaan lukien, ja pyöristää aikansa kokonaisiin ikkunoihin; sen kanssa ensimmäinen tekstitys alkaa siitä, mistä puhe alkaa, ja muut seuraavat sitä. Moottorin repositoriossa on skripti, joka hakee sellaisen:

```bash
sh ./models/download-vad-model.sh silero-v6.2.0
```

Anna sen polku valitsimella `--vad-model`, asetuksella `transcription.vad-model` tai ikkunan [asetuksissa](../subedit-gui/preferences.md#litterointi).

### Mikä malli mille kielelle

Whisper ei vaihda mallia sen jälkeen, kun se on löytänyt kielen: **antamasi malli ratkaisee, mihin
se pystyy.**

| Malli | Kielet | Tunnistaa kielen | Koko |
| :---- | :-------- | :------------------- | :--- |
| `tiny`, `base`, `small`, `medium`, `large-v3`, `large-v3-turbo` | noin sata | kyllä | noin 75 Mt:sta noin 3 Gt:hen |
| `tiny.en`, `base.en`, `small.en`, `medium.en` | vain englanti | ei | sama kuin yllä samankokoisella |

- **Muulle kuin englannille valitse monikielinen malli** — sellainen, jonka nimessä ei ole `.en`. Vain
  englannille tehty malli hylkää toisen kielen: komento kertoo sen ennen kuin mitään työtä tehdään.
- **Pienimmät mallit ovat selvästi huonompia englannin ulkopuolella.** Ranskalle, saksalle, espanjalle ja
  vastaaville `small` on vähimmäisvaatimus, ja `medium` tai `large-v3-turbo` antavat paljon parempia tuloksia
  suuremman muistin ja pidemmän ajan hinnalla.
- **Kielen tunnistus on yhtä hyvä kuin malli.** Pieni malli voi erehtyä alussa, joka koostuu
  musiikista tai hiljaisuudesta; kerro kieli valitsimella `--language`, kun tiedät sen.

## ffmpeg

[`ffmpeg`](https://ffmpeg.org/download.html) purkaa videon ääniraidan, ja `ffprobe`, joka tulee
sen mukana, luettelee raidat. Asenna se jakelusi paketinhallinnalla. Ilman sitä äänitiedosto,
jonka moottori lukee sellaisenaan (`wav`, `mp3`, `flac`, `ogg`), voidaan silti litteroida, mutta video
ei.

## Toimivuuden tarkistus

```bash
subedit-cli transcribe --model /path/to/ggml-small.bin --output film.srt film.mkv
```

Jos jokin puuttuu, viesti kertoo mikä, ja viittaa tähän sivuun.
