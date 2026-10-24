# A leiratozó eszközök telepítése

<!-- machine-translated -->
> Ez az oldal gépi fordítás, amelyet a nyelv egyik olvasója még nem nézett át.

A hang feliratokká leiratozása — a [`transcribe`](transcribe.md) a parancssorban,
a [`Transcribe Audio…`](../subedit-gui/video.md#a-hang-leiratozása) az ablakban — három olyan
dolgot igényel, amelyet a **subedit nem szállít és nem tölt le**: egy motort, egy modellt ehhez a motorhoz, és
az `ffmpeg`-et. Ön telepíti őket, és megmondja a subeditnek, hol van a modell.

**Nélkülük semmi sem romlik el.** A program többi része a korábbiak szerint működik; csak a leiratozás
kapcsol ki, egy üzenettel, amely ide mutat.

## A motor: whisper.cpp

A [whisper.cpp](https://github.com/ggml-org/whisper.cpp) az a motor, amelyet a subedit futtat. Ez egy program,
a `whisper-cli`, amely hétköznapi processzoron fut — a videokártya csak gyorsabbá teszi. Építse meg
úgy, ahogy a saját dokumentációja mondja (egy C++ fordító és a CMake elég):

```bash
git clone https://github.com/ggml-org/whisper.cpp
cd whisper.cpp
cmake -B build
cmake --build build -j --config Release
```

A program ezután a `build/bin/whisper-cli`. Tegye oda, ahol a `PATH` megtalálja, vagy **adja meg az
elérési útját** a subeditnek a `transcription.executable` beállítással. A videokártyához készült építés
a dokumentációja által megadott kapcsolókkal történik; a subedit azt futtatja, amit telepített, és semmit
sem választ Ön helyett.

## Egy modell

A motor egy **modellt** olvas be, egy több száz megabájtos vagy néhány gigabájtos fájlt, amelyet
egyszer tölt le. A hivatalos modellek listája itt található:
[huggingface.co/ggerganov/whisper.cpp](https://huggingface.co/ggerganov/whisper.cpp), a motor
tárolójában pedig van egy szkript, amely letölt egyet:

```bash
sh ./models/download-ggml-model.sh small
```

Ezután adja meg az elérési útját az `--model` kapcsolóval a parancssorban, vagy a `transcription.model` beállítással,
vagy az ablak párbeszédablakában, amely megjegyzi.

### Melyik modell melyik nyelvhez

A Whisper nem vált modellt, miután megtalálta a nyelvet: **az Ön által megadott modell dönti el, mit
tud.**

| Modell | Nyelvek | Felismeri a nyelvet | Méret |
| :---- | :-------- | :------------------- | :--- |
| `tiny`, `base`, `small`, `medium`, `large-v3`, `large-v3-turbo` | nagyjából száz | igen | körülbelül 75 MB-tól körülbelül 3 GB-ig |
| `tiny.en`, `base.en`, `small.en`, `medium.en` | csak angol | nem | mint a fenti, azonos méretűeké |

- **Az angolon kívül bármihez többnyelvű modellt válasszon** — olyat, amelynek a nevében nincs `.en`. Egy csak angol
  modell elutasít minden más nyelvet: a parancs ezt még a munka megkezdése előtt megmondja.
- **A legkisebb modellek az angolon kívül érezhetően gyengébbek.** Franciához, némethez, spanyolhoz és
  hasonlókhoz a `small` a minimum, a `medium` vagy a `large-v3-turbo` pedig sokkal jobb eredményt ad
  több memória és több idő árán.
- **A nyelvfelismerés csak annyira jó, mint a modell.** Egy kis modell tévedhet egy zenével vagy
  csenddel kezdődő elejénél; adja meg a nyelvet a `--language` kapcsolóval, ha ismeri.

## ffmpeg

Az [`ffmpeg`](https://ffmpeg.org/download.html) kinyeri egy videó hangsávját, az `ffprobe`, amely vele
együtt érkezik, pedig felsorolja a hangsávokat. Telepítse a disztribúció csomagkezelőjével. Nélküle egy olyan
hangfájlt, amelyet a motor magában is beolvas (`wav`, `mp3`, `flac`, `ogg`), még le lehet iratozni, de videót
nem.

## Annak ellenőrzése, hogy működik

```bash
subedit-cli transcribe --model /path/to/ggml-small.bin --output film.srt film.mkv
```

Ha valami hiányzik, az üzenet megmondja, mi, és erre az oldalra mutat.
