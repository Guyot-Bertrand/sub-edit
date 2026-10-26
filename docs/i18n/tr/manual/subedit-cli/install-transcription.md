# Yazıya dökme araçlarını kurma

<!-- machine-translated -->
> Bu sayfa makine çevirisidir ve henüz dili bilen bir okuyucu tarafından gözden geçirilmemiştir.

Sesi altyazıya dökmek — komut satırında [`transcribe`](transcribe.md),
pencerede [`Transcribe Audio…`](../subedit-gui/video.md#sesi-yazıya-dökme) — **subedit'in birlikte
getirmediği ve indirmediği** üç şeye ihtiyaç duyar: bir motor, o motor için bir model ve
`ffmpeg`. Onları siz kurarsınız ve modelin nerede olduğunu subedit'e siz söylersiniz.

**Onlar olmadan hiçbir şey bozulmaz.** Programın geri kalanı eskisi gibi çalışır; yalnızca yazıya dökme
kendini devre dışı bırakır ve buraya yönlendiren bir ileti verir.

## Motor: whisper.cpp

[whisper.cpp](https://github.com/ggml-org/whisper.cpp), subedit'in çalıştırdığı motordur. Sıradan bir işlemcide
çalışan `whisper-cli` adlı bir programdır — bir ekran kartı yalnızca onu hızlandırır. Kendi belgelerinin
dediği gibi derleyin (bir C++ derleyicisi ve CMake yeterlidir):

```bash
git clone https://github.com/ggml-org/whisper.cpp
cd whisper.cpp
cmake -B build -DBUILD_SHARED_LIBS=OFF
cmake --build build -j --config Release
```

Program bundan sonra `build/bin/whisper-cli` olur. Yukarıdaki gibi `-DBUILD_SHARED_LIBS=OFF` ile derlendiğinde, yanında hiçbir kitaplık gerektirmeyen tek bir dosyadır; bu yüzden onu istediğiniz yere kopyalayabilirsiniz. Onu `PATH` içinde bulunacağı bir yere koyun ya da `transcription.executable` ayarıyla subedit'e **yolunu verin** — pencerede bunun için [tercihlerde](../subedit-gui/preferences.md#yazıya-dökme) bir alan vardır. Bir ekran kartı için derleme, ilk `cmake` satırına tek bir seçenek ekler: NVIDIA kartı için `-DGGML_CUDA=1`, diğerlerinin çoğu için `-DGGML_VULKAN=1`; subedit sizin kurduğunuzu çalıştırır ve sizin yerinize hiçbir şey seçmez.

Yalnızca programı tutmak için onu **kurun** ve klonu silin:

```bash
cmake --install build --prefix ~/.local
```

`--prefix` nereye olduğunu söyler. `~/.local` yönetici hakları gerektirmez ve `bin` dizini `PATH` içinde olmalıdır; seçenek olmadan varsayılan `/usr/local`'dir ve `sudo` ister. `whisper-cli`, o önekin `bin` dizinine, subedit'in kullanmadığı araçların yanına yerleşir; oraya geldikten sonra klon ve `build` dizini silinebilir.

## Bir model

Motor, bir kez indirdiğiniz, birkaç yüz megabayttan birkaç gigabayta kadar bir dosya olan bir **model** okur. Resmî
modeller
[huggingface.co/ggerganov/whisper.cpp](https://huggingface.co/ggerganov/whisper.cpp) adresinde listelenir ve
motorun deposunda birini getiren bir betik vardır:

```bash
sh ./models/download-ggml-model.sh small
```

Sonra yolunu komut satırında `--model` ile, ya da `transcription.model` ayarıyla,
ya da pencerenin, onu hatırlayan [tercihlerinde](../subedit-gui/preferences.md#yazıya-dökme) verin.

### Konuşmanın başladığı yerden başlamak için bir ses etkinliği modeli

İsteğe bağlı olarak bir **ses etkinliği modeli** (Silero), motorun kodlamadan önce sessizlikleri atlamasını sağlar. Model yoksa whisper ilk altyazısını filmde ilk ne gelirse gelsin, müzik dahil, 0'da başlatır ve zamanlarını tam pencerelere yuvarlar; model varsa ilk altyazı konuşmanın başladığı yerde başlar, diğerleri onu izler. Motorun deposunda bir tane indiren bir betik vardır:

```bash
sh ./models/download-vad-model.sh silero-v6.2.0
```

Yolunu `--vad-model` ile, `transcription.vad-model` ayarıyla ya da pencerenin [tercihlerinde](../subedit-gui/preferences.md#yazıya-dökme) verin.

### Hangi dil için hangi model

Whisper dili bulduktan sonra model değiştirmez: **verdiğiniz model neyi
yapabileceğini belirler.**

| Model | Diller | Dili algılar | Boyut |
| :---- | :-------- | :------------------- | :--- |
| `tiny`, `base`, `small`, `medium`, `large-v3`, `large-v3-turbo` | yaklaşık yüz | evet | yaklaşık 75 MB'tan yaklaşık 3 GB'a |
| `tiny.en`, `base.en`, `small.en`, `medium.en` | yalnızca İngilizce | hayır | yukarıdaki aynı boyutlar gibi |

- **İngilizce dışındaki her şey için çok dilli bir model alın** — adında `.en` bulunmayan bir model. Yalnızca İngilizce
  bir model başka bir dili reddeder: komut bunu herhangi bir iş yapmadan önce söyler.
- **En küçük modeller İngilizce dışında belirgin biçimde daha kötüdür.** Fransızca, Almanca, İspanyolca ve
  benzerleri için `small` bir asgaridir, `medium` ya da `large-v3-turbo` ise daha fazla bellek ve daha fazla zaman
  pahasına çok daha iyi sonuçlar verir.
- **Dil algılama modelin olduğu kadar iyidir.** Küçük bir model, müzik ya da sessizlikten oluşan bir başlangıçta
  yanılabilir; dili bildiğinizde `--language` ile söyleyin.

## ffmpeg

[`ffmpeg`](https://ffmpeg.org/download.html) bir videonun ses izini çıkarır, onunla gelen `ffprobe` ise
izleri listeler. Dağıtımınızın paket yöneticisiyle kurun. O olmadan, motorun olduğu gibi okuduğu bir ses
dosyası (`wav`, `mp3`, `flac`, `ogg`) yine yazıya dökülebilir, ama bir video dökülemez.

## Çalıştığını denetleme

```bash
subedit-cli transcribe --model /path/to/ggml-small.bin --output film.srt film.mkv
```

Bir şey eksikse ileti neyin eksik olduğunu söyler ve bu sayfaya yönlendirir.
