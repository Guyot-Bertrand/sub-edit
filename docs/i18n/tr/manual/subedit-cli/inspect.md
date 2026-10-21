# `inspect`

<!-- machine-translated -->
> Bu sayfa makine çevirisidir ve henüz dili bilen bir okuyucu tarafından gözden geçirilmemiştir.

```
subedit-cli inspect [--frame-rate RATE] [--recursive] [--detail]
                   [-t FILE] [--align-method position|number] <file>...
```

Her dosyanın neyden oluştuğunu bildirir. **Hiçbir şeyi değiştirmez ve hiçbir dosya
yazmaz.**

<!-- example: subedit-cli inspect --help -->
```console
$ subedit-cli inspect --help
Report what a subtitle file is made of
Usage: subedit-cli inspect [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to report on

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --detail                    List every anomaly, subtitle by subtitle, instead of counting them
  --frame-rate RATE           Frame rate of a file counted in frames: 25, 23.976
  -t,--translation-file FILE  Translation file to lay over the subtitle file, for a single input
  --align-method position|number
                              How the lines of the translation find their subtitles
```

## Bağımsız değişkenler

| Bağımsız değişken | Zorunlu | Değer | Varsayılan |
| :------- | :----- | :----- | :----- |
| `<file>...` | evet | altyazı dosyalarının bir ya da daha fazla yolu | — |
| `--recursive`, `-r` | hayır | bir bayrak | kapalı — bkz. [Dizin ağacını işleme](lots.md) |
| `--detail` | hayır | bir bayrak: anomalileri saymak yerine her birini adlandırır — bkz. [`anomalies` ne bildirir](#anomalies-ne-bildirir) | kapalı |

Yol vermemek bir kullanım hatasıdır, dolayısıyla kod `1`.

## Seçenekler

| Seçenek | Zorunlu | Değer | Varsayılan |
| :----- | :------ | :----- | :----- |
| `--frame-rate` | hayır | saniyedeki kare olarak bir hız: `25`, `23.976`, `29.97` | 23,976 |

**Yalnızca karelerle sayılan dosyalara hizmet eder**, yani dokuz biçimden
yalnızca biri olan MicroDVD biçimindeki dosyalara. Bir MicroDVD dosyası kare numaralarından başka bir şey taşımaz;
okunma hızı, `span` ile başlayarak **raporun her konumunu** belirler. Başka bir yerde verilirse
hiçbir şeyi değiştirmez ve hata değildir.

Hiçbir şeyi adlandırmayan bir değer bir kullanım hatasıdır, dolayısıyla kod `1`:

<!-- example: subedit-cli inspect --frame-rate douze film.sub; echo $? -->
```console
$ subedit-cli inspect --frame-rate douze film.sub; echo $?
--frame-rate: "douze" is not a frame rate: expected frames per second, like 25 or 23.976
1
```

[`convert`](convert.md) üzerindeki seçenekle kelimesi kelimesine aynıdır, çünkü
aynı hızdır. `convert` onu ayrıca **yazmak** için de kullanır.

## Bir çeviri: `-t`

| Seçenek | Zorunlu | Değer | Varsayılan |
| :----- | :------ | :----- | :----- |
| `-t`, `--translation-file` | hayır | bir çeviri dosyası, **tek bir** girdi için | — |
| `--align-method` | hayır, yalnızca `-t` ile | `position` veya `number` | `position` |

`-t` ile çeviri dosyanın üzerine **pencerenin onu açarken yaptığı gibi** yerleştirilir — bkz.
[Bir çeviri](invocation.md#bir-çeviri) — ve rapor, **satırlarının altyazılarını nasıl
bulduğunu** söyler. Böylece bir betik, bir çeviriyi düzeltmeden önce hizalı olup olmadığını bilir.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour.\n\n' > film.fr.srt; subedit-cli --quiet inspect -t film.fr.srt film.srt | tail -3 -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour.\n\n' > film.fr.srt; subedit-cli --quiet inspect -t film.fr.srt film.srt | tail -3
  anomalies: none
  translation file: film.fr.srt, matched by position
  translation: 1 line attached; 1 subtitle left without a translation
```

İlk eklenen satır dosyayı ve yöntemi adlandırır; ikincisi, bir çeviriyi açtığında **pencerenin
söylediği cümledir**, her parça yalnızca sıfır olmadığında yer alır:

| Parça | Neyi sayar |
| :----- | :---------------- |
| `N lines attached` | ana dosyanın bir altyazısını bulan çeviri satırları |
| `N subtitles born of a line` | hiçbirini bulamayan ve kendi konumlarında kendi altyazılarını doğuran satırlar |
| `N subtitles left without a translation` | hiçbir satırın ulaşmadığı ana dosya altyazıları |
| `N lines out of order` | dosyada kendinden sonraki bir satırdan sonra gelen satırlar |

Hiçbir satır taşımayan bir çeviri `translation: the file holds no line` der. **Dosyanın kendi raporu
yalnızca dosyanınki olarak kalır**: bir satırdan doğan altyazı `subtitles` içinde
sayılmaz.

`-t` seçeneğinin hataları, çevirinin yoluyla söylenen bir okumanın hataları ve şudur:
verilen dosya ana dosya olduğunda `<translation>: the file is already open as the main document`.
`-t` olmadan `--align-method` bir kullanım hatasıdır; birden çok girdiyle `-t` de öyle — bkz.
[Bir çeviri](invocation.md#bir-çeviri).

## Çıktı

Rapor **standart çıktıya** gider — komutun sonucudur ve her düzeyde,
`--quiet` dahil, yazılır. Her dosya için bir blok vardır, başında yolu ile:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,500\nLe canot dérive.\n\n2\n00:00:04,000 --> 00:00:06,200\nPersonne n a vu la côte.\n' > exemple.srt; subedit-cli --quiet inspect exemple.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,500\nLe canot dérive.\n\n2\n00:00:04,000 --> 00:00:06,200\nPersonne n a vu la côte.\n' > exemple.srt; subedit-cli --quiet inspect exemple.srt
exemple.srt
  format: SubRip
  encoding: UTF-8, detected
  byte order mark: absent
  line endings: LF
  subtitles: 2
  span: 00:00:01.000 -> 00:00:06.200
  frame rate grid: none (too few subtitles to tell)
  anomalies: none
```

| Alan | Ne söyler |
| :---- | :----------- |
| `format` | `--to` seçeneğinin kabul ettiği biçimlerden biri, tam yazılmış olarak: `SubRip`, `WebVTT`, `SubViewer 2`, `Sub Station Alpha`, `Advanced SSA`, `MicroDVD`, `MPL2`, `TMPlayer`, `LRC` |
| `encoding` | okunan kodlama, ardından **yanıtın nereden geldiği**: dosya bildiriyorsa `, from its byte order mark`, `--encoding` zorladıysa `, as asked for`, tartılarak bulunduysa `, detected` |
| `byte order mark` | `present` veya `absent` |
| `line endings` | `LF`, `CRLF` veya `CR`; dosya bunları karıştırıyorsa ardından `, mixed from line N` |
| `subtitles` | okunan altyazı sayısı |
| `span` | en erken başlangıçtan en geç bitişe, `HH:MM:SS.mmm` |
| `frame rate grid` | konumların hesaplandığı kare hızı, **çıkarılmış** — aşağıya bakın |
| `frame rate` | karelerle sayılan bir dosya için, okunduğu hız ve **nereden geldiği** — bir önceki satırın yerini alır, aşağıya bakın |
| `anomalies` | aynı satırda `none` ya da neyin yanlış olduğu: `anomalies:` altında girintili, **her tür için bir satır ve sayısı**; `--detail` ile **her anomali için bir satır**, altyazı altyazı |

**`span`, “ilkinden sonuncuya” değil**, en erkenden en geçe kadardır:
sırası bozuk bir dosyada ikisi farklıdır ve dosyanın neyi kapsadığı konusunda yalnızca ikincisi
doğruyu söyler.

**`anomalies`, `--detail` ile altyazıları birden başlayarak sayar**, dosyanın onları
numaraladığı gibi — dosyanın satırlarını değil. Bir altyazı numarası bir
değişiklikten sağ çıkar; bir satır numarası çıkmaz. Aşağıya bakın.

## `frame rate grid` ne bildirir

**Bir altyazı dosyası kare hızını bildirmez.** SubRip biçiminin başlığı
yoktur, WebVTT biçiminin başlığı serbest metindir. Bu satır bu yüzden hiçbir şey okumaz:
konumların bir kare ızgarasına oturup oturmadığını ölçerek **çıkarım yapar**.

Ölçülenler **başlangıçlardır**. Bir ızgara olan her yerde başlangıçlar istisnasız
ızgaradadır; bitişler ise çoğu zaman ızgaradan ayrılır — bir bitiş sık sık bir okuma hızı kuralıyla
hesaplanır, bir kareye oturtulmaz.

İlk satır her zaman aynı şeyi söyler: seçilen hız, karar ve
onu destekleyen yoğunlaşma.

| Karar | Ne anlama gelir |
| :------ | :----------------- |
| `clean` | konumlar bir ızgaradadır |
| `partial` | bir kısmı ızgaradadır, geri kalanı değildir |
| `none` | hiçbir aday hiçbir şeyi açıklamaz |

`none` kararının **iki nedeni vardır ve aynı yanıt değildirler**: ya hiçbir
ızgara uymuyordur — satır o zaman en iyi adayı ve puanını verir,
böylece ne kadar tutmadığı görülür — ya da dosya hakkında bir şey söylemek için çok az
altyazısı vardır. İki başlangıç her zaman kusursuz hizalı görünür; bu hiçbir şeyi kanıtlamaz.

**Standart olmayan bir hızdaki düzenli bir dosya `none` verir ve bu
kasıtlıdır.** Adayların kümesi kapalıdır — dönüştürmenin sunduğu sekiz hız — ve
bunlardan en az yanlış olanı bildirmek, “bilmiyorum”un doğru yanıt olduğu yerde
yanlış bir yanıt vermek olurdu.

### Ardından gelen, söyleyecek bir şeyi olan satırlar

| Satır | Ne zaman görünür |
| :---- | :------------------ |
| `grid offset` | konumlar ızgaradadır, **sabit bir fark dışında**: dosya kaydırılmıştır ve ne kadar olduğu burada yazar |
| `also fits` | başka bir aday, seçilenin tam katı olduğu için aynı derecede uyar |
| `too short a span to separate` | dosyanın süresi, seçilen hızı adı geçenlerden ayırt etmeye izin vermez |
| `off the grid` | kaç başlangıcın ızgaradan ayrıldığı ve kaç **diziye** yayıldığı |

**`also fits` bir tereddüt değildir.** Saniyede 25 karelik bir ızgara,
saniyede 50 karelik bir ızgaranın *içinde* yer alır: 25 hızına göre zamanlanmış bir dosya bu yüzden ikisine de uyar.
Tersi yanlıştır — gerçek bir 50 hızı 25 hızına çöker — bu yüzden en düşük olan
seçilir ve diğeri susturulmak yerine adlandırılır. İlgili tek
çiftler şunlardır: 25 ve 50, 29.97 ve 59.94, 30 ve 60.

**`too short a span to separate`, aşırı kendinden emin bir yanıta karşı korur.** 23.976
ve 24, film saniyesi başına bir milisaniye ayrışır: uzun metrajlı bir filmde
kuşkusuz ayrışırlar, on saniyede ayırt edilemezler. Satır, hangisinin
hâlâ mümkün olduğunu söyler.

**`off the grid`, yüzdenin tek başına karıştırdığı iki öyküyü ayırır.**
Tek başlangıçlık çok sayıda dizi, elle, tek tek düzeltilmiş konumlardır.
Birkaç uzun dizi, yeniden zamanlanmış bir bölüm ya da iki dosyadan
derlenmiş bir dosyadır. İkisini ayıran, dizilerin sayısıdır.

### Bir örnek

Saniyede 25 karelik bir ızgaraya — yani 40 milisaniyenin katı olan
konumlara — yerleştirilmiş, neredeyse dokuz dakikaya yayılan on iki satır:

<!-- example: k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > grille.srt; subedit-cli --quiet inspect grille.srt | tail -3 -->
```console
$ k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > grille.srt; subedit-cli --quiet inspect grille.srt | tail -3
  frame rate grid: 25 fps, clean (100.0%)
  also fits: 50 fps, of which this rate is a whole divisor
  anomalies: none
```

Örnek, depodan bir dosya okumak yerine kendi dosyasını üretir ve bu
kasıtlıdır: olduğu gibi yeniden çalıştırılabilir ve döngü, bir ızgaranın ne olduğunu
bir cümleden daha iyi gösterir — bir kare süresinin katı olan konumlar.

## `frame rate` ne bildirir ve neden bir ızgara değildir

**Dokuz biçimden yalnızca biri karelerle sayar**: MicroDVD; her satırı
iki kare numarası taşır, başka bir şey taşımaz. Böyle bir dosyada `inspect`,
`frame rate grid` yazmaz, `frame rate` yazar ve iki satır
hiçbir zaman birlikte gösterilmez.

**Gerekçe bir cümleye sığar.** Bir MicroDVD dosyasının konumları,
belirli bir hızda kareleri *esas alınarak* hesaplanmıştır; onları bir ızgaradan
çıkarmak, az önce verilmiş olan hızın kendisini geri verirdi. Ölçüm gibi
giydirilmiş verilmiş bir rakam olurdu.

Onun yerine yazılan, **okuma hızı ve nereden geldiğidir** —
`encoding` satırıyla aynı biçim:

| Satırın söylediği | Ne zaman |
| :------------------ | :---- |
| `frame rate: 25 fps, as asked for` | `--frame-rate` zorladı |
| `frame rate: 24000/1001 fps, assumed` | kimse söylemedi; varsayılandır |

**Üç değil, iki yanıt**, `encoding` üç tane verir: bir MicroDVD
dosyası hiçbir şey bildirmez. Sorunun tamamı budur — hız zorunlu olarak
dışarıdan gelir ve rapor iki dışarıdan hangisi olduğunu söyler.

**Hız, raporun geri kalanının tamamını belirler.** `span` ondan geçer ve aynı dosyanın
iki hızda iki okunması iki farklı süre verir:

<!-- example: printf '{25}{75}Première.\n{100}{150}Deuxième.\n{200}{260}Troisième.\n' > images.sub; subedit-cli --quiet inspect images.sub | tail -3; subedit-cli --quiet inspect --frame-rate 25 images.sub | tail -3 -->
```console
$ printf '{25}{75}Première.\n{100}{150}Deuxième.\n{200}{260}Troisième.\n' > images.sub; subedit-cli --quiet inspect images.sub | tail -3; subedit-cli --quiet inspect --frame-rate 25 images.sub | tail -3
  span: 00:00:01.043 -> 00:00:10.844
  frame rate: 24000/1001 fps, assumed
  anomalies: none
  span: 00:00:01.000 -> 00:00:10.400
  frame rate: 25 fps, as asked for
  anomalies: none
```

## `anomalies` ne bildirir

Bir belgede üç şey yanlış olabilir ve her biri farklı onarılır.
**Varsayılan olarak `inspect` onları sayar**, tablodaki sırayla her tür için bir satır:
kötü yapılmış bir dosya yüzlercesini taşır ve her türden kaç tane olduğunu bilmek
neyin çalıştırılacağını söyler. **`--detail` onları tek tek adlandırır.**

| Yazılan | `--detail` ile | Ne anlama gelir |
| :--------------- | :-------------- | :------------------ |
| `subtitle ends before it starts: N` | `subtitle N ends before it starts` | bitiş başlangıçtan önce gelir |
| `subtitle starts before the previous one ends: N` | `subtitle N starts before the previous one ends` | öncekiyle çakışır |
| `subtitle starts before the previous one starts: N` | `subtitle N starts before the previous one starts` | dosyanın sırasını bozar — [`sort`](sort.md) yerine koyar |

Bulunmayan bir tür yazılmaz. Soldaki sütunda `N` altyazı sayısıdır; sonraki
sütunda bir altyazının numarasıdır.

**Aynı altyazı iki kez görünebilir** ve bu kasıtlıdır: bir önceki başlamadan
başlayan altyazı, o bitmeden de başlamıştır. İlki altyazıyı
taşıyarak, ikincisi bir süreyi değiştirerek çözülür.

**Adlandırılan altyazı yerinde olmayandır**, karşısında yerinde olmadığı değil.
Başlangıçları `0`, `4 s`, `2 s`, `3 s` olanlarda: üçüncüsü sırayı bozar, dördüncüsü
üçüncüyü gerçekten izler ve bu yüzden adlandırılmaz — onun için yapılacak
bir şey yoktur.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect desordre.srt | tail -3 -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect desordre.srt | tail -3
  anomalies:
    subtitle starts before the previous one ends: 1
    subtitle starts before the previous one starts: 1
```

Ve `--detail` ile, adlandırılan altyazı:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect --detail desordre.srt | tail -3 -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect --detail desordre.srt | tail -3
  anomalies:
    subtitle 3 starts before the previous one ends
    subtitle 3 starts before the previous one starts
```

**`--detail` ile anomali başına bir satır**, çünkü tek satıra yapıştırılmış yüz
bulgu ne okunabilir ne aranabilir. Kusursuz bir belge
tek satırını, yani bir betiğin aradığı `anomalies: none` satırını korur. **`--format json`
kaydı her anomaliyi her zaman listeler**: bir betik ayrıntıyı okur ve
`--detail` yalnızca metni değiştirir.

**Bir okuma tanılama iletisi değildir.** Tanılama iletileri, okumanın
karşılaştığı şeyi söyler ve **dosyanın bir satırını** gösterir; anomaliler,
belgenin *ne olduğunu* söyler ve bir **altyazıyı** gösterir. Sırası
bozuk bir dosya bu yüzden bozuk biçimli değildir: okumanın bu konuda söyleyecek bir şeyi yoktur.

## JSON biçiminde

[`--format json`](invocation.md#bir-betiğin-okuyabileceği-çıktı) ile `inspect`, yukarıdaki bloklar yerine **dosya başına
bir nesne** yazar; anlatım hata çıktısında kalır.
Anahtarlar metnin söylediğini betimler ve **hiçbiri bir cümleden geri okunmaz**:

| Anahtar | İçerik |
| :-- | :------ |
| `format` | biçimin tanımlayıcısı, `convert --to` seçeneğinin kabul ettiği: `srt`, `vtt`… |
| `encoding` | `{"charset", "origin", "byte_order_mark"}` — `origin`, `byte-order-mark`, `asked` veya `detected` olur |
| `line_endings` | `{"kind", "mixed_from_line"}` — `lf`, `crlf` veya `cr`; ikinci bir türün göründüğü satır, ya da `null` |
| `subtitles` | altyazı sayısı |
| `span_ms` | `{"start", "end"}` — ilk başlangıç ve son bitiş, milisaniye olarak |
| `frame_rate` | karelerle sayılan bir dosya için: `{"rate", "origin"}` (`asked` veya `assumed`); aksi halde `null` |
| `grid` | diğerleri için: çıkarılan ızgara, karelerle sayılan bir dosya için `null` |
| `anomalies` | bir `{"subtitle", "kind"}` dizisi — `end-before-start`, `overlapping-subtitles`, `out-of-order` |
| `translation` | **yalnızca `-t` ile**: `{"file", "method", "attached", "born", "untranslated", "out_of_order"}` — `method`, `position` veya `number` olur ve dört sayı yukarıdaki cümlenin sayılarıdır |
| `warnings` | okumanın karşılaştığı şeyler, her düzeyde |

`grid` şunları taşır: `verdict` (`clean`, `partial` veya `silent`), `enough_starts`, `rate` (bir dize,
ya da hiçbir şey adlandırılmadığında `null`), `concentration_permille` (binde, yeterli
başlangıç yoksa `null`), `offset_ms`, `also_fits` (bu hızın tam böleni olduğu hız, ya da
`null`), `not_separated` (çok kısa bir sürenin ayırmadığı hızlar), `strays`
(ızgaradan ayrılan başlangıçlar) ve `starts`.

<!-- example: printf '{25}{50}Un.\n{75}{100}Deux.\n' > f.sub; subedit-cli --format json inspect --frame-rate 25 f.sub 2>/dev/null -->
```console
$ printf '{25}{50}Un.\n{75}{100}Deux.\n' > f.sub; subedit-cli --format json inspect --frame-rate 25 f.sub 2>/dev/null
{"schema":1,"command":"inspect","file":"f.sub","ok":true,"format":"microdvd","encoding":{"charset":"UTF-8","origin":"detected","byte_order_mark":false},"line_endings":{"kind":"lf","mixed_from_line":null},"subtitles":2,"span_ms":{"start":1000,"end":4000},"frame_rate":{"rate":"25","origin":"asked"},"grid":null,"anomalies":[],"warnings":[]}
```

## Anlatım

Hata çıktısında, istenen düzeye göre:

| Düzey | Neler eklenir |
| :----- | :-------------- |
| 1 | `<path>: N subtitles` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings` |
| 3 | `<path>: N bytes read`, `<path>: N diagnostic(s) while reading`, ardından **her biri** — bkz. [Çalıştırma](invocation.md#tanılama-iletilerini-okuma) |

## Çıkış kodları

Aracın kodları: her dosya okunduysa `0`, hiçbiri okunmadıysa `2`,
yalnızca bazıları okunduysa `3`, kullanım hatasında `1`.

## Hatalar

Bir okumanın hataları — dosyanın ve `-t` verildiğinde çevirinin — ve
`-t` seçeneğinin hataları: yukarıya bakın. `inspect` reddedilebilecek hiçbir şey yazmaz.

| Neyin tetiklediği | İleti |
| :------------------ | :------ |
| yol verilmedi | `files is required`, ardından `--help` işaretçisi |
| eksik dosya | `<path>: does not exist` |
| okunamayan dosya | `<path>: cannot be opened: permission denied` |
| kodu çözülemeyen baytlar | `<path>: cannot be decoded in the chosen encoding` |
| tanınmayan biçim | `<path>: is in no format this tool knows` |
| altyazıya benzeyen hiçbir şey yok | `<path>: holds nothing recognisable as a subtitle` |

Bunlar on yedi alt komut için aynı iletilerdir: açma tarifi
çekirdekte bir kez yazılmıştır.

## Başarısız olan bir örnek

Boş bir dosya hiçbir biçimle eşleşmez. Boş olarak bildirilmez:
reddedilir ve adı söylenir.

<!-- example: : > vide.srt; subedit-cli inspect vide.srt; echo "code=$?" -->
```console
$ : > vide.srt; subedit-cli inspect vide.srt; echo "code=$?"
vide.srt: is in no format this tool knows
code=2
```
