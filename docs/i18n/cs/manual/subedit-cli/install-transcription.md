# Instalace nástrojů pro přepis

<!-- machine-translated -->
> Tato stránka je strojovým překladem a dosud ji nezkontroloval žádný čtenář tohoto jazyka.

Přepis zvuku na titulky — [`transcribe`](transcribe.md) v příkazovém řádku,
[`Transcribe Audio…`](../subedit-gui/video.md#přepis-zvuku) v okně — vyžaduje tři
věci, které **subedit nedodává ani nestahuje**: nástroj, model pro tento nástroj a
`ffmpeg`. Nainstalujete je vy a subeditu řeknete, kde model leží.

**Bez nich se nic nerozbije.** Zbytek programu funguje jako dřív; jen přepis
se vypne a zobrazí hlášení, které odkazuje sem.

## Nástroj: whisper.cpp

[whisper.cpp](https://github.com/ggml-org/whisper.cpp) je nástroj, který subedit spouští. Je to program,
`whisper-cli`, který běží na běžném procesoru — grafická karta ho jen zrychlí. Sestavte ho
podle jeho vlastní dokumentace (stačí překladač C++ a CMake):

```bash
git clone https://github.com/ggml-org/whisper.cpp
cd whisper.cpp
cmake -B build -DBUILD_SHARED_LIBS=OFF
cmake --build build -j --config Release
```

Program pak najdete v `build/bin/whisper-cli`. Sestavený s `-DBUILD_SHARED_LIBS=OFF`, jak je uvedeno výše, je to jediný soubor, který vedle sebe nepotřebuje žádnou knihovnu, takže ho můžete zkopírovat kamkoli. Umístěte ho tam, kde ho najde váš `PATH`, nebo **zadejte jeho cestu** subeditu v nastavení `transcription.executable` — v okně pro něj mají [předvolby](../subedit-gui/preferences.md#přepis) pole. Sestavení pro grafickou kartu přidá jednu volbu do prvního řádku `cmake`: `-DGGML_CUDA=1` pro kartu NVIDIA, `-DGGML_VULKAN=1` pro většinu ostatních; subedit spouští to, co jste nainstalovali, a nic za vás nevolí.

## Model

Nástroj čte **model**, soubor o velikosti několika set megabajtů až několika gigabajtů, který
stáhnete jednou. Oficiální modely jsou uvedeny na
[huggingface.co/ggerganov/whisper.cpp](https://huggingface.co/ggerganov/whisper.cpp) a
repozitář nástroje má skript, který jeden stáhne:

```bash
sh ./models/download-ggml-model.sh small
```

Poté zadejte jeho cestu pomocí `--model` v příkazovém řádku nebo nastavením `transcription.model`,
nebo v [předvolbách](../subedit-gui/preferences.md#přepis) okna, které si ji pamatují.

### Který model pro který jazyk

Whisper po zjištění jazyka model nemění: **model, který zadáte, určuje, co
dokáže.**

| Model | Jazyky | Zjistí jazyk | Velikost |
| :---- | :-------- | :------------------- | :--- |
| `tiny`, `base`, `small`, `medium`, `large-v3`, `large-v3-turbo` | asi sto | ano | zhruba od 75 MB do asi 3 GB |
| `tiny.en`, `base.en`, `small.en`, `medium.en` | jen angličtina | ne | jako stejná velikost výše |

- **Pro cokoli jiného než angličtinu zvolte vícejazyčný model** — takový, jehož název nemá `.en`. Model jen pro
  angličtinu jiný jazyk odmítne: příkaz to řekne dřív, než začne jakákoli práce.
- **Nejmenší modely jsou mimo angličtinu znatelně horší.** Pro francouzštinu, němčinu, španělštinu a
  podobné je `small` minimum a `medium` nebo `large-v3-turbo` dávají mnohem lepší výsledky za cenu
  větší paměti a delšího času.
- **Zjištění jazyka je jen tak dobré jako model.** Malý model se může mýlit u začátku tvořeného
  hudbou nebo tichem; jazyk zadejte pomocí `--language`, když ho znáte.

## ffmpeg

[`ffmpeg`](https://ffmpeg.org/download.html) vyextrahuje zvukovou stopu videa a `ffprobe`, který se
dodává s ním, stopy vypíše. Nainstalujte ho správcem balíčků své distribuce. Bez něj lze zvukový
soubor, který nástroj čte tak, jak je (`wav`, `mp3`, `flac`, `ogg`), přesto přepsat, ale video
ne.

## Ověření, že to funguje

```bash
subedit-cli transcribe --model /path/to/ggml-small.bin --output film.srt film.mkv
```

Pokud něco chybí, hlášení řekne co a odkáže na tuto stránku.
