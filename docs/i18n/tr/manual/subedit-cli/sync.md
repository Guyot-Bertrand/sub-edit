# `sync`

<!-- machine-translated -->
> Bu sayfa makine çevirisidir ve henüz dili bilen bir okuyucu tarafından gözden geçirilmemiştir.

```
subedit-cli sync (--video FILE | --audio FILE)
                 [--audio-track N]
                 [--language CODE]
                 [--start TIME] [--end TIME]
                 [--model FILE]
                 [--vad-model FILE]
                 [--engine NAME]
                 [--keep-audio FILE]
                 [--min-match PERCENT]
                 [--doubt-gap MS]
                 [--report FILE]
                 [-r]
                 [--output FILE | --output-dir DIR | --in-place] [--dry-run]
                 <files>...
```

**Bir altyazı dosyasını bir filmin sesine göre yeniden zamanlar.** Var olma nedeni, çevrimiçi bulunan, başta ve sonda
kaymış `.srt` dosyasıdır; tek bir kaydırma onu düzeltmez: her altyazı söylenenler içinde aranır ve
söylendiği yere yerleşir.

Sesi bir yazıya dökme motoruyla dinler — [`transcribe`](transcribe.md) ile aynı motor — ve **her altyazının
sözcüklerini duyduğu sözcüklerle eşleştirir**. Hiçbir şey çevrilmez: altyazılar konuşulan dilde olmalıdır.

**İhtiyaç duyduğu hiçbir şey programla birlikte gelmez**: motor, bir model ve `ffmpeg`,
[Yazıya dökme araçlarını kurma](install-transcription.md) sayfasındakilerdir ve yokluklarının hiçbir şeyi bozmaz — komut
neyin eksik olduğunu o sayfaya yönlendirerek söyler, hiçbir şey yazmaz ve sıfırdan farklı bir kodla çıkar.

- **Yalnızca başlangıçlar ve bitişler değişir.** Metin, biçem, altyazı sayısı ya da sıraları asla.
- **Sesin yer vermediği bir altyazı zamanlarını korur** — kimsenin söylemediği bir jenerik, bir şarkı, başka bir dilde bir satır.
  **Hiçbir konum uydurulmaz ve komşulardan tahmin edilmez**: satır dosyadaki yerinde bırakılır
  ve komut bunu söyler. Bu bir başarısızlık *değildir* ve çıkış kodu `0`'dır.
- **Bir altyazı, sözcüklerinin en az yarısı duyulduğunda bulunmuş sayılır** (`--min-match`, `alignment.min-match`) ve
  en az biri. Büyük/küçük harf, vurgu işaretleri, noktalama ve biçimlendirme önemli değildir; uzun bir sözcükten bir harf farklı bir sözcük duyulmuş sayılır.
- **Bitiş, son sözcüğün bittiği yerdir.** Motor sözcüklerin zamanını yaklaşık verir, altyazının kendi bitişinden birkaç yüzde saniye önce,
  bu yüzden bir altyazı eskisinden biraz daha kısa çıkabilir.
- **Bir sonrakine taşacak bir altyazı, sonrakinin başladığı yere kadar kısaltılır.** Kısaltmanın çözemediği, hâlâ
  bir sonrakiyle çakışan altyazı olduğu gibi bırakılır ve söylenir.
- **Hiçbir altyazısı bulunmayan bir dosya reddedilir ve yazılmaz**: neredeyse kesinlikle yanlış ses
  ya da yanlış dildir.
- **Filmin bir bölümü tek başına dinlenebilir**, `--start` ve `--end` ile,
  [`transcribe`](transcribe.md#filmin-bir-bölümünü-yazıya-dökme) için olduğu gibi: yalnızca aralıkta **başlayan** altyazılar aranır
  ve değiştirilir, diğerleri olduğu gibi bırakılır ve sayılmaz.
- **Ses, verilen her dosya için bir kez dinlenir.** Aynı filmde birkaç dosyayı yeniden zamanlamak için
  her biri için beklemeyi göze alın.

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
  --min-match PERCENT         Share of a subtitle's words that must be heard, in percent; the alignment.min-match setting by default
  --doubt-gap MS              How far a subtitle may move from its neighbours' shift before it is flagged, in milliseconds; the alignment.doubt-gap setting by default
  --report FILE               Write the subtitles to check to this file
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Bağımsız değişkenler ve seçenekler

| Seçenek | Zorunlu | Değerler | Varsayılan |
| :----- | :------- | :----- | :------ |
| `<files>` | evet | yeniden zamanlanacak altyazı dosyaları; `-r` ile dizinler | — |
| `--recursive`, `-r` | hayır | bir bayrak | kapalı — bkz. [Dizin ağacını işleme](lots.md) |
| `--video` | `--video` ve `--audio` değerlerinden biri | dinlenecek film | — |
| `--audio` | `--video` ve `--audio` değerlerinden biri | bir film yerine dinlenecek bir ses dosyası | — |
| `--audio-track` | hayır | 0'dan başlayan bir tam sayı: filmin ses izleri arasındaki konum | `0`, ilki |
| `--language` | hayır | iki harf, bir ISO 639-1 kodu: `en`, `fr` | `transcription.language` ayarı; boşsa: algılanır |
| `--start`, `--end` | hayır | bir süre: `2:00`, `1:02:03`, `2:00.5` ya da bir saniye sayısı; `--end`, `--start` değerinden sonra gelmelidir | sesin tamamı |
| `--model` | hayır | model dosyası | `transcription.model` ayarı |
| `--vad-model` | hayır | motorun sessizlikleri atlamasını sağlayan bir ses etkinliği modeli; onunla sözcükler konuşmayı daha yakından izler | `transcription.vad-model` ayarı |
| `--engine` | hayır | dinleyen motorun adı: `whisper.cpp` | `transcription.engine` ayarı |
| `--keep-audio` | hayır | çıkarılan sesin saklanacağı dosya; dizini yoksa oluşturulur | ses hiçbir yerde saklanmaz |
| `--min-match` | hayır | 1 ile 100 arasında bir tam sayı: bir altyazının sözcüklerinden duyulması gereken pay, yüzde olarak | `alignment.min-match` ayarı, `50` |
| `--doubt-gap` | hayır | milisaniye cinsinden 0 ya da daha büyük bir tam sayı: bir altyazının kaymasının komşularınınkinden ne kadar farklı olabileceği, işaretlenmeden önce | `alignment.doubt-gap` ayarı, `2000` |
| `--report` | hayır | kontrol edilecek altyazıların yazılacağı bir dosya, satır başına bir tane: altyazının numarası, bir sekme ve neden; her seferinde bir altyazı dosyası | hiçbir şey yazılmaz |
| `--output` / `--output-dir` / `--in-place` | `--dry-run` dışında **üçünden biri** | bkz. [Çalıştırma](invocation.md#hedef) | — |
| `--dry-run` | hayır | bir bayrak: hesaplar ve bildirir, hiçbir şey yazmaz — bkz. [Yazmadan önce görme](invocation.md#yazmadan-önce-görme) | kapalı |

Hedef — `--output`, `--output-dir`, `--in-place`, `--dry-run` — ve `-r`, bir dosyayı yeniden yazan diğer komutlarınkilerdir:
bkz. [Hedef](invocation.md#hedef). Dosya biçimini, kodlamasını ve
satır sonlarını korur.

**Ayarlar** ayar dosyasından okunur — bkz. [Ayar dosyası](invocation.md#ayar-dosyası) — ve her birinin
burada kendi seçeneği vardır. Motor, model ve ses etkinliği modeli `transcribe` komutununkilerdir.

| Anahtar | Anlamı | Varsayılan |
| :-- | :------ | :------ |
| `alignment.engine` | hizalama motoru; bir tane vardır, `transcript-matching`, dinler ve eşleştirir | `transcript-matching` |
| `alignment.min-match` | bir altyazının bulunmuş sayılması için sözcüklerinden duyulması gereken pay, yüzde olarak | `50` |
| `alignment.doubt-gap` | bir altyazının kaymasının komşularınınkinden, işaretlenmeden önce, milisaniye olarak ne kadar ayrılabileceği | `2000` |

## Nelerin işaretlendiği

Komut hangi altyazıların kontrol edilmesi gerektiğini ve nedenini söyler. Bir altyazı, şunlardan **ilk** geçerli olanı için işaretlenir:

| Neden | Anlamı |
| :-- | :------ |
| not found in the audio, left as it was | sözcüklerinin `--min-match` değerinden azı duyuldu: zamanlarını korur |
| only partly heard | bulundu, ama sözcüklerinin %80'inden azı duyuldu |
| moved differently from its neighbours | bulundu, ama kayması, iki yanındaki iki altyazının ortanca kaymasından `--doubt-gap` değerinden fazla ayrılıyor |

Bir altyazı ayrıca **sonraki altyazının başlangıcına kadar kısaltılmış** olabilir ya da **hâlâ sonraki altyazıyla çakışıyor** olabilir.

İki rakam dosyanın bütününün öyküsünü anlatır. **Ortanca kayma**, altyazıların ne kadar kaydığının ortasıdır: yalnızca
geç kalmış bir dosya burada okunur, başka yerde değil. **Sapma**, altyazıların son onda birinin ortanca kaymasından
ilk onda birininkinin çıkarılmasıdır: sabit bir kayma için sıfıra yakındır, film boyunca giderek daha geç kalan bir dosya için ise büyüktür.

## Çıktı

**Standart çıktı** — boş: sonuç yazılan dosyadır.

| Düzey | Hata çıktısına eklenenler |
| :---- | :------------------------------- |
| 1 | `<file>: N of M subtitles placed on the audio, K to check; median shift X s, drift Y s -> <destination>` |
| 2 | adım ve ilerleme, [`transcribe`](transcribe.md#çıktı) için olduğu gibi |

### JSON biçiminde

[`--format json`](invocation.md#bir-betiğin-okuyabileceği-çıktı) ile, dosyanın nesnesi `counts` taşır: `subtitles`
(aranan), `placed` (bir yer verilen), `moved` (zamanları değişen), `partial`, `out_of_line`, `not_found`,
`median_shift_ms` ve `drift_ms`; ve `flagged`, bir `{"subtitle": N, "kind": "…"}` listesi, burada sayı
altyazının dosyadaki yeridir ve `kind`, `not-found`, `partial`, `out-of-line`, `cut-back` ve `overlaps` değerlerinden biridir.
Tanımlayıcılar vaat edilir; cümleler edilmez.

## Örnek

```bash
subedit-cli sync --video film.mkv --model small.bin --vad-model silero.bin \
    --report to-check.txt --output film.synced.srt film.srt
```

## Çıkış kodları

Aracın genel kodları: dosya yazıldıysa, altyazılar işaretlenmiş olsun olmasın `0`, yazılamadıysa `2` — eksik bir araç ya da model, başarısız bir çıkarma ya da dinleme, hiçbir altyazının bulunmaması —, bir kullanım hatasında `1`, örneğin ne `--video` ne `--audio` (ya da ikisi birden), aralığı dışında bir seçenek ya da kullanılamayan bir aralık.

## Hatalar

| Neden | İleti |
| :---- | :------ |
| ne `--video` ne `--audio`, ya da ikisi birden | `sync needs the audio to listen to: give --video or --audio, and not both` |
| hiçbir motor programı bulunamadı ya da yapılandırılan yerinde değil | `the transcription engine was not found`, ardından kılavuza yönlendirme |
| hiçbir model verilmedi ya da orada değil | `no model was given, or the model file is not there`, ardından kılavuza yönlendirme |
| bir film için `ffmpeg` bulunamadı | `ffmpeg was not found, so the audio of a video cannot be extracted`, ardından yönlendirme |
| filmin sahip olmadığı bir iz | `the video has no audio track N`, ardından sahip olduğu izler |
| modelin yazıya dökemediği bir dil | `the model cannot transcribe the language xx` |
| kimsenin kaydetmediği bir motor | `no transcription engine is called NAME` ya da `there is no alignment engine called NAME` |
| hiçbir altyazı bulunamadı | `none of the N subtitles was found in the audio: is it the right audio, and the right language?` |
| 1 ile 100 dışında `--min-match`, negatif `--doubt-gap` | seçeneğin kullanım hatası |
| birden fazla altyazı dosyasıyla `--report` | `--report names one file: give one subtitle file when you use it` |
