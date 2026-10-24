# Uppsetning umritunartækjanna

<!-- machine-translated -->
> Þessi síða er vélþýðing sem enginn lesandi tungumálsins hefur enn farið yfir.

Til að umrita hljóð í skjátexta — [`transcribe`](transcribe.md) á skipanalínunni,
[`Transcribe Audio…`](../subedit-gui/video.md#umritun-hljóðsins) í glugganum — þarf þrennt
sem **subedit fylgir ekki með og sækir ekki**: vél, líkan fyrir þá vél og
`ffmpeg`. Þú setur það upp og segir subedit hvar líkanið er.

**Ekkert bilar án þess.** Restin af forritinu virkar eins og áður; aðeins umritunin
slekkur á sér, með skilaboðum sem vísa hingað.

## Vélin: whisper.cpp

[whisper.cpp](https://github.com/ggml-org/whisper.cpp) er vélin sem subedit keyrir. Hún er forrit,
`whisper-cli`, sem keyrir á venjulegum örgjörva — skjákort gerir hana aðeins hraðari. Smíðaðu hana
eins og skjölin hennar segja (C++ þýðandi og CMake nægja):

```bash
git clone https://github.com/ggml-org/whisper.cpp
cd whisper.cpp
cmake -B build -DBUILD_SHARED_LIBS=OFF
cmake --build build -j --config Release
```

Forritið er þá `build/bin/whisper-cli`. Þegar það er smíðað með `-DBUILD_SHARED_LIBS=OFF`, eins og að ofan, er það ein skrá sem þarf ekkert safn við hlið sér, svo þú getur afritað það hvert sem er. Settu það þar sem `PATH` finnur það, eða **gefðu upp slóð þess** til subedit með stillingunni `transcription.executable` — í glugganum hafa [kjörstillingarnar](../subedit-gui/preferences.md#umritun) reit fyrir það. Smíði fyrir skjákort bætir einum valkosti við fyrstu `cmake`-línuna: `-DGGML_CUDA=1` fyrir NVIDIA-kort, `-DGGML_VULKAN=1` fyrir flest önnur; subedit keyrir það sem þú settir upp og velur ekkert fyrir þig.

Til að halda aðeins forritinu skaltu **setja það upp** og eyða klóninu:

```bash
cmake --install build --prefix ~/.local
```

`--prefix` segir hvert. `~/.local` krefst engra stjórnandaréttinda, og `bin` möppan þar þarf að vera í `PATH` hjá þér; án valkostsins er sjálfgefið `/usr/local`, sem biður um `sudo`. `whisper-cli` lendir í `bin` möppu þess forskeytis, við hlið verkfæra sem subedit notar ekki; þegar það er komið þangað má fjarlægja klónið og `build` möppuna þess.

## Líkan

Vélin les **líkan**, skrá sem er nokkur hundruð megabæti upp í nokkur gígabæti og þú
sækir einu sinni. Opinberu líkönin eru talin upp á
[huggingface.co/ggerganov/whisper.cpp](https://huggingface.co/ggerganov/whisper.cpp), og
geymsla vélarinnar hefur skriftu sem sækir eitt:

```bash
sh ./models/download-ggml-model.sh small
```

Gefðu síðan upp slóð þess með `--model` á skipanalínunni, eða með stillingunni `transcription.model`,
eða í [kjörstillingum](../subedit-gui/preferences.md#umritun) forritsins, sem muna hana.

### Hvaða líkan fyrir hvaða tungumál

Whisper skiptir ekki um líkan eftir að það hefur fundið tungumálið: **líkanið sem þú gefur ræður hvað það
getur gert.**

| Líkan | Tungumál | Greinir tungumálið | Stærð |
| :---- | :-------- | :------------------- | :--- |
| `tiny`, `base`, `small`, `medium`, `large-v3`, `large-v3-turbo` | um hundrað | já | frá um 75 MB upp í um 3 GB |
| `tiny.en`, `base.en`, `small.en`, `medium.en` | aðeins enska | nei | sama stærð og hér að ofan |

- **Fyrir allt nema ensku skaltu taka fjöltyngt líkan** — líkan þar sem heitið hefur ekki `.en`. Líkan
  fyrir ensku eingöngu hafnar öðru tungumáli: skipunin segir það áður en nokkur vinna hefst.
- **Minnstu líkönin eru greinilega verri utan ensku.** Fyrir frönsku, þýsku, spænsku og
  þess háttar er `small` lágmark, og `medium` eða `large-v3-turbo` gefa mun betri niðurstöður gegn
  meira minni og lengri tíma.
- **Tungumálagreiningin er aðeins jafngóð og líkanið.** Lítið líkan getur skjátlast ef byrjunin er
  tónlist eða þögn; segðu tungumálið með `--language` þegar þú veist það.

## ffmpeg

[`ffmpeg`](https://ffmpeg.org/download.html) dregur út hljóðrás myndskeiðs, og `ffprobe`, sem fylgir
með því, telur upp rásirnar. Settu það upp með pakkastjóra dreifingarinnar þinnar. Án þess er hægt að
umrita hljóðskrá sem vélin les eins og hún er (`wav`, `mp3`, `flac`, `ogg`), en myndskeið
ekki.

## Að athuga hvort þetta virkar

```bash
subedit-cli transcribe --model /path/to/ggml-small.bin --output film.srt film.mkv
```

Ef eitthvað vantar segja skilaboðin hvað, og vísa á þessa síðu.
