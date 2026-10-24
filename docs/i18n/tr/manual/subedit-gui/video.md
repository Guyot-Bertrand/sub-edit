# Bağlı video

<!-- machine-translated -->
> Bu sayfa makine çevirisidir ve henüz dili bilen bir okuyucu tarafından gözden geçirilmemiştir.

**Video** menüsü önce üç komut ve filmi yöneten hareketleri taşır
— bkz. [Oynatıcı](player.md#filmde-gezinme).

| Komut | Kısayol | Ne yapar |
| :---- | :------ | :------- |
| Video Seç… | yok | belgenin eşlik ettiği video dosyasını seçer |
| Sesi Yazıya Dök… | yok | filmin sesini, projenin altyazılarının yerine geçen altyazılara dönüştürür — bkz. [Sesi yazıya dökme](#sesi-yazıya-dökme) |
| Oynat / Duraklat | `Ctrl+P` | videoyu oynatır ya da durdurur — bkz. [Oynatıcı](player.md#oynatma-ve-durdurma) |

Bağlı video **açık belgenin bir özelliğidir**, uygulamanın bir ayarı değil: başka bir altyazı
dosyası açmak sıfırdan başlar. Hiçbir dosyaya yazılmaz ve pencereyle birlikte kaybolur.

## Ses izi

Bir film birkaç ses izi taşıyabilir — özgün sürüm, bir dublaj, bir yorum. **`Video ▸ Ses ▸ Dil`**
alt menüsü bunları listeler, **iz başına bir öğe**, ve **çalanı işaretler**:

| Dosyanın verdiği | Öğenin söylediği |
| :--------------- | :--------------- |
| bir dil ve bir başlık | `1: fra — Original` |
| yalnızca bir dil | `2: eng` |
| yalnızca bir başlık | `3: Commentary` |
| hiçbiri | `1` |

Numara, **izin filmdeki yeridir**, birer birer, dosyanın sırasıyla: iki iz `eng` diyebilir ve
etiketleri hakkında sessiz kalan bir dosyanın da yine bir birinci ve bir ikinci izi vardır.

**Bir öğeyi seçmek o izi çalar.** İşaret, tıklamanın değil oynatıcının yanıtıdır: çalan izi söyler.

- **Alt menü devre dışıdır**, video ya da ses izi yoksa. Tek iz varsa etkin kalır ve onu işaretli
  olarak gösterir: filmin dilini söyler. Yanında bulunan iki ses düzeyi hareketi her durumda etkin
  kalır: bkz. [Ses düzeyi](player.md#ses-düzeyi).
- **İz hatırlanmaz**, bir videodan ötekine ya da bir oturumdan ötekine: bir iz numarası kendi
  dosyasına özgüdür ve onu sonraki filme taşımak orada o numarayı taşıyanı seçerdi. Bir sekmeye
  dönmek filmini, oynatıcının kendi seçtiği izle yeniden açar.
- Kısayol yok: dili seyrek değiştirirsiniz ve menü yeter.

## Video seçme

İletişim kutusu altyazı dosyasının dizininde açılır ve tanınan video uzantılarına göre süzer:

    .avi .flv .m2ts .mkv .mov .mp4 .ogg .ogv .vob .webm

Tüm dosyaları göstermeyi de önerir. İletişim kutusunu iptal etmek hiçbir şeyi değiştirmez.

Seçilen dosya **hemen açılır**: görüntü pencerede belirir, ilk anında duraklatılmış olarak.
Bkz. [Oynatıcı](player.md).

**Bir video dosyasını pencereye bırakmak da aynı şeyi yapar** — bkz.
[Dosyaları pencereye bırakma](files.md#dosyaları-pencereye-bırakma).

**Video, geçerli sekmenin projesinin videosudur.** Her sekme kendisininkini tutar; sekme
değiştirmek filmi değiştirir. **Bir sekmeye dönmek filmine bıraktığınız yerden devam eder**,
duraklatılmış olarak, her açılış gibi. Bu arada o proje için seçilen başka bir film ise baştan
başlar.

## Otomatik öneri

Bir altyazı dosyası açıldığında, **hiçbir video seçilmemişse**, pencere aynı dizinde bir tane arar:
uzantısız adı, bir noktaya kadar, bizimkinin bir öneki olan bir video dosyası.

| Altyazı dosyası | Bulunan video | Neden |
| :-------------- | :------------ | :---- |
| `film.fr.srt` | `film.mkv` | `film`, `film.fr` adının önekidir |
| `film.en.forced.srt` | `film.mkv` | aynı şekilde, bölüm sayısı ne olursa olsun |
| `film.fr.srt` | `film.mkv` yerine `film.fr.mkv` | en uzun ad kazanır |
| `film.fr.srt` | yok, `film.mkv` **ve** `film.mp4` varsa | iki aday birbirinden ayırt edilemez |
| `film.fr.srt` | yok, yalnızca `fil.mkv` varsa | `fil` bir noktada bitmez |

**Bir öneri bir seçim değildir.** Başka bir öneriyle değiştirilmesine izin verir — başka bir dosya
açmak, başka bir adla kaydetmek. Açık bir seçim ise asla üzerine yazılmaz: `Select Video…` bir kez
kullanıldığında otomatik öneri artık arkasından geri gelmez.

Tahmini kabul edilebilir kılan budur: düzeltilebilir ve düzeltmesi kalır.

## Durum çubuğunun gösterdikleri

Durum çubuğunun sağında bir satır, belgenin neye eşlik ettiğini söyler. Üçün sonuncusudur: önce
okunan dosyanın [kodlaması](files.md#durum-çubuğundaki-kodlama) gelir, ardından konumlardan çıkarılan
[kare ızgarası](frame-grid.md).

| Durum | Ne yazılır |
| :---- | :--------- |
| bir video bağlıdır | `Video: film.mkv` |
| … ve `ffprobe` kare hızını verir | `Video: film.mkv, 24000/1001 fps` |
| hiçbiri | `No video` |

**Kare hızı filmin yanında değil, filmle birlikte gider.** Bunlar tek bir olgudur — bu belgenin neye
eşlik ettiği — ve üçüncü bir ifade onu, başka bir şey olan
[konumlardan çıkarılan ızgara](frame-grid.md) ile aynı sıraya koyardı.

## Sesi yazıya dökme

**`Transcribe Audio…`** filmin konuşmasını altyazı olarak yazar. Bir film açılana kadar devre dışıdır: bu,
**projenin filminin** sesidir ve asla seçeceğiniz bir dosya değildir — bir ses dosyasını yazıya dökmek için
komut satırında [`transcribe`](../subedit-cli/transcribe.md) komutunu kullanın.

İletişim kutusu üç şey ister ve başlayıp başlayamayacağını söyler:

- **ses izi**, [`Audio`](#ses-izi) menüsünün listelediği gibi listelenir, dosyanın bildirdiği dil
  ve başlıkla birlikte;
- **dil**, `en` ya da `fr` gibi iki harf. Dosya bildirdiğinde seçilen izin diliyle
  başlar, siz kendiniz bir tane yazmadığınız sürece ize uyar ve
  **alanın altındaki bir satır nereden geldiğini söyler**. Dosya iz için bir dil bildirmediğinde
  satır bunu söyler: model dili o zaman sesten algılar, bunu yalnızca çok dilli bir model yapabilir ve
  biliyorsanız dili yazmalısınız. Hiçbir şeyi algılamayan, yalnızca İngilizce bir modelle satır,
  dilin yazılması gerektiğini söyler;
- **filmin bölümü**, bir zamandan bir başkasına — bkz. [Önce bir bölümü denemek](#önce-bir-bölümü-denemek). İki alan da boşsa filmin tamamı demektir.

**Neyin eksik olduğunu da söyler.** Üç satır motorun, `ffmpeg`'in ve modelin orada olup olmadığını ve
nerede olduğunu söyler; biri yoksa `OK` kapalı kalır ve kutu bunu
[Yazıya dökme araçlarını kurma](../subedit-cli/install-transcription.md) sayfasına yönlendirerek söyler. Bir `Preferences…` düğmesi
[tercihleri](preferences.md#yazıya-dökme) açar; program ve model orada ayarlanır — bunlar makineyle ilgili
bilgilerdir, her yazıya dökmede yeniden yapılacak seçimler değil — ve kutu, tercihler kapanınca yeniden bakar.

Dil bir sonraki sefer için saklanır. Motorun kendisi ve altyazıların sınırları
[ayarlardır](preferences.md) (`transcription.*`).

**Proje zaten altyazı içeriyorsa, pencere onları değiştirmeden önce sorar** ve kaç tanesinin gideceğini
söyler. İşten önce sorar, böylece vazgeçmenin bir maliyeti olmaz. Proje boşsa
sorulacak bir şey yoktur.

İş **arka planda** çalışır, bir kutu ve bir `Cancel` düğmesiyle; pencere çevresinde kullanılabilir kalır. **Kutu işin nerede olduğunu söyler**, böylece canlı olduğunu ve ne kadar süreceğini bilirsiniz:

- **adım**: önce “Ses çıkarılıyor…” — kutu o sırada meşguldür, rakam göstermez, çünkü çıkarma ne kadar ilerlediğini söylemez —, sonra “Ses yazıya dökülüyor…”, bir çubukla;
- **konum**: “0:12:30 / 0:50:00”, motorun filmde ne kadar ilerlediği; ilk metnini üretir üretmez görünür;
- **kalan süre**, “yaklaşık 38 dk kaldı”: başta yoktur, tahmin yürütmeye yetecek kadar iş yapıldığında görünür ve son dakikanın hızından hesaplanır; birkaç dakikalık bir doğruluktur, bir söz değil;
- **elde edilen son metin**, işin anlamlı gittiğini görmeniz için.

Büyük bir modelle işlemcide çalışan uzun bir filmde ilk dakikalar uzun olabilir: motor modeli yüklüyor ve ilk otuz saniyelik pencereleri üzerinde çalışıyor. Konum ve son metin, motorun yalnızca filmin %5'lik adımlarıyla gelen kendi yüzdesinden önce hareket eder. Bittiğinde projenin altyazıları
yazıya dökmeyle **tek bir işlem olarak** değiştirilir: bir `Undo` orada olanı geri verir, çeviri
metinleri dahil. Başarısız olan ya da iptal ettiğiniz bir yazıya dökme hiçbir şeyi değiştirmez — neden
bir kutuda söylenir, yalnızca iptal hariç; iptal sadece durum çubuğunda iptal edildiğini söyler.

### Önce bir bölümü denemek

Bir aralık, yazıya dökmeyi bir **denemeye** çevirir: iki saate girişmeden önce bir modeli ya da bir dili iki dakikada değerlendirmek için. “Filmin bir bölümü” alanları bir başlangıç ve bir bitiş alır, her biri `2:00`, `1:02:03`, `2:00.5` ya da bir saniye sayısı, `90` olarak yazılır; yalnızca biri de olabilir (oradan sona kadar ya da baştan oraya kadar). Bir alan süre olmadığı ya da bitiş başlangıçtan sonra gelmediği sürece `OK` kapalı kalır ve alanların altındaki bir satır hangisi olduğunu söyler.

- **Altyazılar filmin zamanını korur**: 2:10'da söylenen şey sonuçta `0:02:10` konumundadır.
- **Hiçbir şey değiştirilmez ve hiçbir şey sorulmaz.** Sonuç, dosyasız, yeni bir sekmede **kendi başına bir proje** olarak açılır; çıkarıldığı proje ve altyazıları olduğu gibi kalır. Kaydedin ya da kapatın.

**İhtiyaç duyduğu şey birlikte gelmez**: bir motor (whisper.cpp), bir model ve sesi çıkarmak için `ffmpeg`.
Bkz. [Yazıya dökme araçlarını kurma](../subedit-cli/install-transcription.md).

## Film olmadığında

Pencere, görüntünün yerine hiçbir şey koymak yerine bir **`Select Video…`** düğmesi gösterir. Bu,
`Video` menüsündekiyle aynı komuttur, fare erişiminde: üzerinde işlem yapamadığınız bir yokluğu bir
arızadan ayırmak zordur.

Düğme bir film açılır açılmaz kaybolur ve görüntü yerinin **tamamını** alır: tablonun onun
üstünde bıraktığı yükseklik — [`window.table-share`](preferences.md) değerinin bir oturumdan
ötekine hatırladığı — ve görüntünün en küçük yüksekliğinden asla az olmamak üzere; gerekirse tablo
yer verir. Film gittiğinde düğme aynı yeri geri alır.

**Dosyanın adı, yolu asla.** İki yüz karakterlik bir yol, çubuktaki her şeyi dışarı iterdi; tam yol,
iletişim kutusunun gösterdiğidir.

## `ffmpeg` gerekmez

`subedit-gui`, `ffprobe` programına — `ffmpeg` ile birlikte gelir — yalnızca bir şey sorar:
**kapsayıcının bildirdiği kare hızı**. Her film için, bağlandığında bir kez okunur.

| `ffprobe` ile | Olmadan |
| :------------ | :------ |
| durum çubuğu kare hızını filmin adının yanına yazar | yalnızca adı yazar |
| dönüştürme iletişim kutusu filmin kare hızını alttaki alanda önerir | hiçbir şey önermez ve eskisi gibi açılır |
| hizalama iletişim kutusu onunla açılır | projenin kare hızıyla açılır |

**Başka hiçbir şey ona bağlı değildir ve hiçbir işlem reddetmez.** Oynatıcı ise `ffprobe`
programına hiç ihtiyaç duymaz: bir derleme bağımlılığı olan `libmpv` kitaplığına dayanır. Filmin
süresi, sonun ötesine geçeni bildirmek için kullanılan süre, `ffprobe` değil oynatıcıdan gelir.

**Yazıya dökme, `ffmpeg` programına gerçekten ihtiyaç duyan tek şeydir**: filmin sesini çıkarır. Bkz.
[Yazıya dökme araçlarını kurma](../subedit-cli/install-transcription.md).

Bu nedenle bir bozulmuş kip değil, `ffmpeg` kurmamış bir makinenin **normal işleyişidir**: veri
kullanıcıdan gelir ve `ffprobe` varsa onu önerir.

Okunan kare hızı **kesin bir orandır** — `24000/1001`, `23.976` değil. `ffprobe` programının bu
yanıt için tutulmasının nedeni budur: oynatıcı aynı şeyi kayan noktalı bir sayı olarak bilir ve
yaklaşık bir kare hızı bütün dosyayı biraz yanlış yeniden zamanlardı.
