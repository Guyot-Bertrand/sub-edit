# `transcribe`

<!-- machine-translated -->
> Bu sayfa makine çevirisidir ve henüz dili bilen bir okuyucu tarafından gözden geçirilmemiştir.

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

**Bir videonun ya da ses dosyasının konuşmasını bir altyazı dosyası olarak** yazar. Ses hazırlanır,
söylenenleri ve ne zaman söylendiğini bulan bir motora verilir ve motorun kestiği parçalar, bir süreye
ve bir uzunluğa uyan altyazılar halinde birleştirilir.

**İhtiyaç duyduğu hiçbir şey programla birlikte gelmez**: motoru (whisper.cpp), bir modeli ve `ffmpeg`
programını kullanıcı kurar ve modelin nerede olduğunu o söyler. [Yazıya dökme
araçlarını kurma](install-transcription.md) sayfası her birinin nerede bulunacağını ve hangi modelin seçileceğini anlatır. **Bunların
yokluğu hiçbir şeyi bozmaz**: komut neyin eksik olduğunu o sayfaya yönlendirerek söyler, hiçbir şey yazmaz
ve sıfırdan farklı bir kodla çıkar.

- **Girdi bir video ya da bir ses dosyasıdır.** Bir videodan `ffmpeg` bir ses izi çıkarır; `ffmpeg`
  olmadan, motorun olduğu gibi okuduğu bir ses dosyası (`wav`, `mp3`, `flac`, `ogg`) yine
  yazıya dökülür, ama bir video dökülmez, bir iz istendiğinde de hiçbir dosya dökülmez.
- **Her seferinde yalnızca bir dosya**, ve hedef `--output`'tur: `--output-dir` ya da
  `--in-place` yoktur, çünkü sonuç yeniden yazma değil, yeni bir dosyadır.
- **Dil algılanır**, `--language` zorlamadıkça, ve kullanılan dil söylenir. Yalnızca İngilizce için
  yapılmış bir model hiçbir şey algılamaz ve başka hiçbir şeyi yazıya dökmez: alamayacağı bir dil,
  herhangi bir iş başlamadan *önce*, modelin sınırı belirtilerek reddedilir.
- **Bir dil iki harfle yazılır — `fr`, `en`, `de` — ya da bir dosyanın bildirdiği üç harfli kodla, `fra`,
  `eng`; bu, iki harfli kod olarak okunur.** Geri kalan her şey herhangi bir iş başlamadan önce reddedilir. **Algılama
  yalnızca ilk otuz saniyeyi dinler**: müzikle ya da sessizlikle açılan bir film başka bir dil sanılabilir ve
  motor o zaman konuşmayı o dile *çevrilmiş* olarak yazar — İngilizce sanılan bir Fransızca film İngilizce çıkar ya da
  `[SPEAKING FRENCH]` olarak. Dili biliyorsanız verin.
- **Filmin yalnızca bir bölümü yazıya dökülebilir**, `--start` ve `--end` ile — bkz.
  [Filmin bir bölümünü yazıya dökme](#filmin-bir-bölümünü-yazıya-dökme). Bir model iki saatte çalıştırılmadan önce iki dakikada böyle denenir.
- **Parçalar birleştirilir ve kesilir**, ayarların sınırlarına göre (`transcription.merge-gap-ms`,
  `transcription.max-duration-ms`, `transcription.max-characters`): bir cümlenin birbirine yakın iki
  parçası tek bir altyazı olur, çok uzun ya da çok geniş bir parça ise bir sözcükten kesilir. **`--raw`
  motorun parçalarını geldikleri gibi yazar**, her altyazıya bir parça; birleştirmenin ne yaptığını
  görmenin yolu da budur.

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

## Bağımsız değişkenler ve seçenekler

| Seçenek | Zorunlu | Değerler | Varsayılan |
| :----- | :----- | :------ | :----- |
| `<input>` | evet | bir video ya da ses dosyası | — |
| `--output` | evet | yazılacak altyazı dosyası; dizini yoksa oluşturulur | — |
| `--to` | hayır | bir biçimin kısa adı: `srt`, `vtt`, `subviewer2`, `ssa`, `ass`, `microdvd`, `mpl2`, `tmplayer`, `lrc` | `--output` uzantısının belirttiği biçim, yoksa `srt` |
| `--audio-track` | hayır | 0'dan başlayan bir tam sayı: videonun ses izleri arasındaki konum | `0`, ilki |
| `--language` | hayır | iki harf, bir ISO 639-1 kodu: `en`, `fr`; `fra` gibi üç harfli bir kod `fr` olarak okunur | `transcription.language` ayarı; boşsa: algılanır |
| `--start` | hayır | bir süre: `2:00`, `1:02:03`, `2:00.5` ya da bir saniye sayısı, `90` | sesin başı |
| `--end` | hayır | `--start` için olduğu gibi yazılmış bir süre; `--start` değerinden sonra gelmelidir | sesin sonu |
| `--model` | hayır | model dosyası | `transcription.model` ayarı |
| `--engine` | hayır | bir motorun adı: `whisper.cpp` | `transcription.engine` ayarı |
| `--keep-audio` | hayır | çıkarılan sesin saklanacağı dosya, bir WAV dosyasında 16 kHz mono 16 bit PCM olarak; dizini yoksa oluşturulur | ses hiçbir yerde saklanmaz |
| `--raw` | hayır | bir bayrak: motorun parçalarını birleştirme ve kesme olmadan yazar | kapalı |

**Ayarlar**, [`adjust`](adjust.md) komutunun ayarları gibi ayar dosyasından okunur — bkz.
[Ayar dosyası](invocation.md#ayar-dosyası) — ve her birinin burada kendi seçeneği vardır:

| Anahtar | Anlamı | Varsayılan |
| :-- | :------ | :------ |
| `transcription.engine` | motor | `whisper.cpp` |
| `transcription.executable` | motorun programı, mutlak bir yol; boşsa: `PATH` içinde aranır | boş |
| `transcription.model` | model dosyası, mutlak bir yol | boş |
| `transcription.language` | sesin dili, iki harf; boşsa: algılanır | boş |
| `transcription.merge-gap-ms` | bir cümlenin bu kadar milisaniyeden daha yakın parçaları birleştirilir | `400` |
| `transcription.max-duration-ms` | bir altyazının ekranda kalabileceği en uzun süre, milisaniye cinsinden | `7000` |
| `transcription.max-characters` | bir altyazının taşıyabileceği en fazla karakter, satır sonları sayılmaz | `84` |

## Sesi seçme

Bir video birkaç ses izi taşıyabilir — özgün ses, bir yorum, başka bir dil. `--audio-track`
dosyanın **ses izleri arasındaki konumu** alır, 0'dan başlayarak, dosyanın listelediği sırayla.
Videoda olmayan bir iz, **sahip olduğu izlerle birlikte** reddedilir, böylece bir sonraki çağrı
doğru olur:

```bash
subedit-cli transcribe --audio-track 4 --output film.srt --model small.bin film.mkv
```

```text
film.mkv: the video has no audio track 4
audio track 0: fra, Original
audio track 1: eng, Commentary
```

## Filmin bir bölümünü yazıya dökme

`--start` ve `--end`, işi aralarındaki film bölümüyle sınırlar. İkisinden biri tek başına verilebilir: tek başına bir `--start` sona kadar gider, tek başına bir `--end` baştan başlar.

```bash
subedit-cli transcribe --start 2:00 --end 3:30 --output test.srt --model large-v3.bin film.mkv
```

- **Bir süre `M:SS` ya da `H:MM:SS` olarak, gerekirse bir noktadan sonra ondalık bölümle (`2:00.5`), ya da bir saniye sayısı olarak (`90`, `90.5`) yazılır**. Virgül, aracın her yerinde olduğu gibi reddedilir.
- **Altyazılar filmin zamanını korur**: filmin 2:10'unda söylenen cümle `00:02:10` olarak yazılır, `00:00:10` olarak değil. Dosya olduğu gibi filmin tamamının üzerine konabilir.
- **Kenarlar yaklaşık değerlerdir**: motor otuz saniyelik pencerelerle çalışır, bu yüzden döndürdüğü şey sonradan aralığa göre kesilir; bir kenara taşan cümle orada kesilir.
- **Aralık kullanılamıyorsa hiçbir şey başlamaz**: bir süre olmayan bir sınır ya da `--start` değerinden sonra gelmeyen bir `--end`, sorunu adlandıran bir kullanım hatasıdır; hiçbir şey yazmaz ve ne `ffmpeg` ne de motoru çalıştırır.

Tüm filmin sesi yine de çıkarılır: aralığın kazandırdığı, çok daha uzun süren yazıya dökmedir.

## Çıktı

**Standart çıktı** — boş: sonuç yazılan dosyadır.

| Düzey | Hata çıktısına eklenenler |
| :----- | :------------------------------------- |
| 1 | `<input>: N subtitles transcribed, language: xx -> <output>` |
| 2 | adım ve ilerleme, her yüzde için bir satır: `extracting the audio…`, sonra `transcribing…`, sonra `transcribing: 25%, 0:12:30 of 0:50:00, about 38 min left, elapsed: 0:05:12` — yüzde, seste ulaşılan konum, kalan süre (hesaplanabilene kadar `estimating the time left…`) ve geçen süre; **yeni bir şey gelmediğinde satır her on saniyede bir yeniden söylenir**, uzun bir bekleme donma sanılmasın diye |
| 3 | ve motorun elde ettiği her yeni metin, `last text: …` |

**Yüzde, motorun seste bulunduğu konumdur, kendi pencerelerinin sayısı değil**: motor yazıya döktüğünü ilerledikçe yazar, komut da ne kadar ilerlediğini okur; böylece rakam ilk dakikadan itibaren hareket eder. (Motorun kendi ilerlemesi, otuz saniyelik bir pencere bittikten sonra, sesin %5'lik adımlarıyla gelir: tek başına, bir işlemcide büyük bir modelle uzun bir filmde çeyrek saat boyunca %0'da kalabilir.) **Kalan süre başlangıçta sessizdir**: tahmin yürütmeye yetecek kadar iş yapıldığında verilir ve tüm çalışmanın ortalamasını değil, son dakikanın hızını izler.

### JSON biçiminde

[`--format json`](invocation.md#bir-betiğin-okuyabileceği-çıktı) ile (alt komuttan önce yazılan genel seçenek),
dosyanın nesnesi `language` içinde kullanılan dili, `counts.subtitles` (yazılan altyazılar) ve
`counts.segments` (bunların yapıldığı motor parçaları) değerlerini taşır; `--keep-audio` sesi sakladıysa
`audio` da bulunur.

## Uyarılar

İki uyarı ilk düzeyde söylenir, çünkü ne beklenmesi gerektiğini değiştirirler:

- **İşten önce, yalnızca İngilizce bir modelle**: `This model is for English only: whatever language the film
  speaks, the result will be English, or notes in brackets. Choose a multilingual model in the preferences.` Dosya adı
  `.en` ile biten bir model böyledir.
- **İşten sonra, motor çoğunlukla köşeli parantez içinde notlar yazdığında** — `[MUSIC PLAYING]`, `[SPEAKING FRENCH]` —,
  en az iki ve parçalarının üçte biri: `N of the M pieces are only notes in brackets, such as [MUSIC PLAYING]: the
  language used (xx) is probably not the one spoken, and the engine may have translated the speech. Run it again with
  the language typed.` Altyazılar yine de yazılır; uyarı neyden şüphelenileceğini söyler.

## Çıkış kodları

Aracın genel kodları: dosya yazıldıysa `0`, yazılamadıysa `2` — eksik bir araç ya da model, modelin alamayacağı bir dil,
başarısız bir çıkarma ya da yazıya dökme —, bir kullanım hatasında `1`, örneğin kimsenin kaydetmediği
bir motor ya da kullanılamayan bir aralık.

## Hatalar

| Neden | İleti |
| :---- | :------ |
| hiçbir motor programı bulunamadı ya da yapılandırılan yerinde değil | `the transcription engine was not found`, ardından kılavuza yönlendirme |
| model verilmedi ya da yerinde değil | `no model was given, or the model file is not there`, ardından kılavuza yönlendirme |
| bir video için `ffmpeg` bulunamadı | `ffmpeg was not found, so the audio of a video cannot be extracted`, ardından yönlendirme |
| videoda olmayan bir iz | `the video has no audio track N`, ardından sahip olduğu izler |
| modelin yazıya dökemediği bir dil | `the model cannot transcribe the language xx` |
| dil yok ve dili algılayamayan bir model | `the model cannot detect the language: give one` |
| kimsenin kaydetmediği bir motor | `no transcription engine is called NAME` |
| bir kod olmayan bir dil | `"TEXT" is not a language code: write two letters, such as fr, en, de or es` |
| bir süre olmayan bir aralık sınırı | `"TEXT" is not a time: write it as 2:00, 1:02:03 or 2:00.5, or in seconds` |
| `--end`, `--start` değerinden sonra değil | `the end of the range must come after its start` |
| diğer her başarısızlık | `the transcription failed`, motorun söylediğiyle birlikte |

Kılavuza yönlendirme şudur: `See the manual page "Installing the transcription tools"
(install-transcription).`
