# Çalıştırma

<!-- machine-translated -->
> Bu sayfa makine çevirisidir ve henüz dili bilen bir okuyucu tarafından gözden geçirilmemiştir.

```
subedit-cli [global options] <subcommand> [options] <file>...
```

Alt komut verilmezse araç yardımını yazar ve durur.

<!-- example: subedit-cli -->
```console
$ subedit-cli
Read, inspect and retime subtitle files.
Usage: subedit-cli [OPTIONS] [SUBCOMMAND]

Options:
  -h,--help                   Print this help message and exit
  --version                   Display program version information and exit
  -v                          Say more: -v is the default, -vv details, -vvv debugs
  -q,--quiet                  Say nothing but errors
  --encoding NAME             Encoding to read the files in; detected by default
  --format text|json          Form of the result on standard output: text or json
  --config FILE               Read the operation settings from FILE instead of the user's settings file
  --no-config Excludes: --config
                              Ignore any settings file: use the built-in defaults
  --list-encodings            Write the encodings that can be read and written, one a line, and stop

Subcommands:
  inspect                     Report what a subtitle file is made of
  convert                     Write a subtitle file out in another format or shape
  shift                       Move every position of a file by a fixed amount
  transform                   Correct every position from two points known to be right
  framerate                   Re-time a file mastered at one frame rate for another
  snap                        Move every position onto the nearest frame of a frame rate (see framerate)
  hearing-impaired            Remove the sounds described between square brackets
  adjust                      Bring the duration of every subtitle within a reading speed and limits
  replace                     Replace a text in the subtitles, without breaking a tag
  case                        Put the texts in title, sentence, upper or lower case, tags intact
  italics                     Put the texts in italics, or take their italics out
  dialogue-dashes             Put dialogue dashes on the lines, or take them off
  sort                        Put the subtitles in the order of their start, keeping ties as they are
  join-repeats                Join the subtitles that repeat each other, one after the other, into one
  disjoint                    Cut overlapping subtitles so that no range overlaps another, merging what overlaps
  repair                      Join the repeated subtitles, put them in order, then make the ranges disjoint
  correct                     Correct the texts with the patterns of the correction assistant
  append                      Put subtitle files one after another into a single file
  split-file                  Cut a subtitle file in two, a head and a tail
  pair                        Write a translation at the positions of its main file, and say how it lined up
  transcribe                  Write the speech of a video or an audio file as subtitles
  config                      List, show and change the operation settings
```

## Dil

Komut satırı ortamının dilini konuşur: **yardımı, anlatımı ve hataları**
istenen dili izler. Bu kılavuzun örnekleri İngilizce çalıştırılır — bloklar
`make manual` tarafından üretilir, asla elle kopyalanmaz.

**Dil nasıl seçilir.** `LC_ALL`, `LC_MESSAGES` ve `LANG` içinden ayarlı olan ve
boş olmayan ilki yerel ayardır; `LANGUAGE` ayarlıysa istenen dilleri iki nokta üst üste ile ayırarak
sıralar (`LANGUAGE=pt_BR:fr`) ve ilk o okunur. Bu, gettext'in izlediği sıradır ve
programın kendisi tarafından okunur: yerel ayarın makinede üretilmiş olması gerekmez.

| Kural | Etki |
| :--- | :----- |
| kodlama ve değiştirici | yok sayılır: `fr_FR.UTF-8@euro` önce `fr_FR`, sonra `fr` arar |
| `LC_ALL=C`, `C` veya `POSIX` yerel ayarı, ya da hiç yerel ayar yok | İngilizce ve `LANGUAGE` okunmaz |
| İngilizceyi adlandıran bir girdi (`en`, `en_GB`) | İngilizce: `LANGUAGE` içinde ondan sonra gelenler okunmaz |
| kataloğu kurulu olmayan bir dil | istenen sonraki dil, sonuncudan sonra İngilizce, **tek söz etmeden** |
| kataloğun çevirmediği bir ileti | İngilizce gösterilir |

`correct` komutunun uygulayamadığını bildirdiği **bir düzeltme deseninin adı** da,
katalog biliyorsa dili izler — programla gelen desenler için; kendi deseniniz
dosyasının yazdığı gibi adlandırılır. Desen dosyalarının kendisi asla yeniden yazılmaz.

Kataloglar yürütülebilir dosyanın kendi dizininden bulunur ve
`<prefix>/share/subedit/locale/<language>/LC_MESSAGES/subedit.mo` altına kurulur. İngilizce yerleşiktir. Bu sürüm `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR`, `ru`, `sr`, `tr` ve `zh_CN` — Katalanca,
Çekçe, Almanca, İspanyolca, Fince, Fransızca, Galiçyaca, İbranice, Macarca, Interlingue, İzlandaca, İtalyanca, Felemenkçe, Lehçe, Portekizce, Brezilya Portekizcesi, Rusça, Sırpça, Türkçe ve Çince (Basitleştirilmiş) — dillerini tam çevrilmiş olarak kurar. Fransızca, dili bilen bir okuyucu
tarafından gözden geçirilmiştir; Katalanca, Çekçe, Almanca, İspanyolca, Fince, Galiçyaca, İbranice, Macarca, Interlingue, İzlandaca, İtalyanca, Felemenkçe, Lehçe, Portekizce, Brezilya Portekizcesi, Rusça, Sırpça, Türkçe ve Çince (Basitleştirilmiş)
makine çevirisidir ve henüz dili bilen bir okuyucu tarafından gözden geçirilmemiştir.
<!-- machine-translated: ca -->
<!-- machine-translated: cs -->
<!-- machine-translated: de -->
<!-- machine-translated: es -->
<!-- machine-translated: fi -->
<!-- machine-translated: gl -->
<!-- machine-translated: he -->
<!-- machine-translated: hu -->
<!-- machine-translated: ie -->
<!-- machine-translated: is -->
<!-- machine-translated: it_IT -->
<!-- machine-translated: nl -->
<!-- machine-translated: pl -->
<!-- machine-translated: pt -->
<!-- machine-translated: pt_BR -->
<!-- machine-translated: ru -->
<!-- machine-translated: sr -->
<!-- machine-translated: tr -->
<!-- machine-translated: zh_CN -->
`LANG=fr_FR.UTF-8` altında `subedit-cli --help` çıktısı “Lire,
inspecter et recaler des fichiers de sous-titres.” ile başlar. Metin çıktısının sözcüklerini okuyan bir betik
`LANGUAGE=en` (ya da `LC_ALL=C`) ayarlar — ya da hiç değişmeyen `--format json` çıktısını okur.

**Dili asla izlemeyenler** — bir betiğin okudukları:

| Asla çevrilmeyen | Neden |
| :--------------- | :-- |
| `--format json` çıktısı: anahtarlar, değerler ve hataların ve uyarıların `message` ile `detail` alanları | her dilde aynı baytlar; sözcükler İngilizcedir |
| çıkış kodları | [Çıkış kodları](#çıkış-kodları) bölümündeki dört kod, her dilde |
| yazılan adlar: alt komutlar, seçenekler, değerleri (`off`, `srt`), üst değişkenler (`FILE`, `TEXT`, `OPTIONS`) | bir kez yazılan komut satırı her dilde çalışır |
| `correct --enable` ve `--disable` seçeneklerinin aldığı düzeltme desenlerinin adları | bir desen, dosyasının taşıdığı İngilizce adla çağrılır |
| biçimlerin ve kodlamaların adları | bir biçim adıyla çağrılır |
| CLI11'in kendi tanılama iletileri — eksik zorunlu bağımsız değişken, bilinmeyen seçenek, bir seçeneğin kabul etmediği değer — ve `Run with --help for more information.` | komut satırının üzerine kurulduğu ayrıştırıcının içinde yazılmıştır; İngilizce kalırlar |

Ortam ne olursa olsun İngilizce istemek:

<!-- example: LC_ALL=C LANGUAGE=fr subedit-cli shift --output-dir out a.srt; echo "code=$?" -->
```console
$ LC_ALL=C LANGUAGE=fr subedit-cli shift --output-dir out a.srt; echo "code=$?"
shift needs --by, or --to-grid to work the amount out from the positions
code=1
```

## Genel seçenekler

**Alt komutun adından önce yazılırlar, yalnızca orada.**
`subedit-cli --encoding cp1252 inspect film.srt` doğru biçimdir;
`subedit-cli inspect --encoding cp1252 film.srt`, `The following argument was not expected: --encoding`
iletisiyle reddedilir. Aynısı `--version`, `-v` ve `--quiet` için de geçerlidir.

**`--help` istisnadır ve gerçekte bir istisna değildir**:
`subedit-cli inspect --help` kabul edilir, çünkü her alt komut kendi yardımını taşır ve
yazılan, aracın değil alt komutun yardımıdır.

| Seçenek | Etki |
| :----- | :---- |
| `-h`, `--help` | yardımı yazar ve durur; bir alt komut adlandırılmışsa onunkini |
| `--version` | `subedit <version>`, aracın karşısında çalıştığı kütüphaneleri ve kurulu dilleri yazar ve durur — bkz. [`--version`](#--version) |
| `-q`, `--quiet` | düzey 0 — hiç anlatım yok |
| `-v`, `-vv`, `-vvv` | düzey 1 ile 3 arası; düzey 1 varsayılandır |
| `--encoding NAME` | dosyaları tahmin etmek yerine bu kodlamada okur |
| `--list-encodings` | aracın okuyup yazabildiği kodlamaları, satır başına bir tane yazar ve **durur** (kod `0`) — bkz. [`--list-encodings`](#--list-encodings) |
| `--format text\|json` | standart çıktıdaki sonucun biçimi: metin ya da dosya başına bir JSON nesnesi — bkz. [Bir betiğin okuyabileceği çıktı](#bir-betiğin-okuyabileceği-çıktı) |
| `--config FILE` | işlem ayarlarını kullanıcının ayar dosyası yerine `FILE` dosyasından okur — bkz. [Ayar dosyası](#ayar-dosyası) |
| `--no-config` | hiçbir ayar dosyası okumaz: komut satırında verilmeyen her işlem ayarı yerleşik varsayılanını alır — bkz. [Ayar dosyası](#ayar-dosyası) |

Aynı çağrıda `--quiet` ve `-v` reddedilir: birbirine zıt iki niyet
en son yazılanın lehine çözülmez.

Aynı çağrıda `--config` ve `--no-config` de reddedilir, `--config excludes --no-config`
iletisiyle; boş bir adla `--config` de öyle, `--config needs a file name`
iletisiyle: boş bir ad hiçbir dosyayı adlandırmaz ve kullanıcının kendi dosyası
onun yerine okunmaz.

### Ayar dosyası

**[`adjust`](adjust.md) ve [`correct`](correct.md) işlem ayarlarını varsayılan olarak
ayar dosyasından okur** — `Adjust Durations…` ve `Correct Texts…` işlemlerinin
pencerede hatırladığı ya da [`config set`](config.md) komutunun yazdığı değerler. Başka hiçbir alt komut
onu okumaz. Dosya şudur:

```
$XDG_CONFIG_HOME/subedit/settings.conf
```

ya da `XDG_CONFIG_HOME` ayarsız, boş veya mutlak olmayan bir yolsa `~/.config/subedit/settings.conf` —
pencerenin dosyası, [kılavuzunda](../subedit-gui/preferences.md#dosya) anlatılan.

**Öncelik, en güçlüden başlayarak: komut satırındaki seçenek, sonra dosya, sonra yerleşik
varsayılan.** Seçenek her zaman kazanır: `--speed off`, dosyanın belirlediği bir hızı kapatır. Dosyanın yalnızca
**işlem bloğu** okunur; görüntü tercihleri (geometri, sütunlar, tema…)
asla okunmaz. [`config list`](config.md#config-list) anahtarları ve bunlardan komut satırının
hangilerini okuduğunu adlandırır: dokuz `duration-adjust.*` anahtarı, `correction.human`, `correction.ocr`,
`correction.remove-blank` ve `correction.line-break.max-lines`. **Her birinin, dosya onu nasıl
ayarlarsa ayarlasın, tek bir çağrı için onu geçersiz kılan bir seçeneği vardır** —
[`adjust`](adjust.md#ayar-dosyasının-sağladıkları) ve
[`correct`](correct.md#neyin-seçildiği-ve-neyin-seçilmediği) tabloları bunları adlandırır.

**Kullanıcının dosyasını okumak bir komutu asla başarısız kılmaz.** Eksik bir dosya varsayılanları verir; okunamayan bir
dosya varsayılanları verir ve bunu söyler; okunamayan bir değer varsayılanını korur ve adlandırılır;
bilinmeyen bir anahtar yok sayılır. Dosya `adjust` ya da `correct` tarafından asla yazılmaz.

**Okunanlar söylenir**, varsayılan düzeyde (1), standart hatada, ilk dosyadan önce:

| Durum | Satır |
| :--- | :--- |
| dosya okundu | `settings: read <path>` |
| içindeki bir değer okunamadı | `settings: <key>: cannot read '<value>', default kept` |
| kullanıcının dosyası yok | `settings: <path> not found, defaults used` |
| dosya okunamıyor | `settings: <path> unreadable, defaults used` |
| `--no-config` | hiçbir şey |

[`--format json`](#bir-betiğin-okuyabileceği-çıktı) ile `adjust` ve
`correct` komutlarının her dosya nesnesi `settings` taşır: okunan yol, ya da hiçbiri okunmadıysa `null`.


**`--no-config` bir çalıştırmayı yeniden üretilebilir kılar**: kullanıcı pencerede ne ayarlamış olursa olsun,
aynı satır her makinede aynı sonucu verir. Betiklerde, testlerde ve hata raporlarında kullanın — ya da bunlara eşlik eden bir dosyayla
`--config FILE`.

**`--config FILE` başka bir dosyayı adlandırır**, aynı biçimde okunur — **var olması gerektiği dışında**.
Komut satırında verilen bir ad, varsayılanları istemek değildir: `FILE` yoksa
`adjust` ve `correct` hiçbir girdi okunmadan durur, standart hataya `settings file not found: <FILE>` yazar ve
`1` kullanım koduyla çıkar. Var olan ama okunamayan bir `FILE`
varsayılanları verir ve bunu söyler, tıpkı kullanıcının dosyası gibi. [`config show`](config.md#config-show)
eksik bir `FILE` dosyasını yine `(not found)` olarak gösterir, [`config set`](config.md#config-set-ve-config-unset) ise yine
oluşturur.

<!-- example: printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine.conf adjust --dry-run a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine.conf adjust --dry-run a.srt; echo "code=$?"
settings file not found: mine.conf
code=1
```

### `--version`

**Programın sürümü, ilk satırda tek başına** — `subedit <version>`, çıktının 0.15.38 sürümüne kadar olduğu ve
bir betiğin okuduğu şey. Ardından **aracın karşısında çalıştığı her kütüphane için bir satır**, sonra kurulu arayüz
dilleri, standart çıktıda; araç `0` koduyla durur. Birlikte verilen bir alt komut
çalıştırılmaz; `-q` ve `-v` hiçbir şeyi değiştirmez.

<!-- example: subedit-cli --version | head -n 1 -->
```console
$ subedit-cli --version | head -n 1
subedit 1.1.3
```

ICU 74.2 ve Enchant 2.3.3 bulunan bir makinede tamamı — sayılar makineninkilerdir:

```console
$ subedit-cli --version
subedit <version>
  ICU      74.2 (built with 74.2)
  Enchant  2.3.3 (built with 2.3.3)
  CLI11    2.4.1 (built with 2.4.1)
  Languages: en, fr
```

| Kütüphane | Aracın onu ne için kullandığı |
| :------ | :------------------------ |
| `ICU` | kodlamaları okuma ve yazma, algılama, harflerin büyük/küçük hâlini değiştirme |
| `Enchant` | `correct` komutunun yazım denetimi |
| `CLI11` | bu komut satırını okuma |

- **Her satır iki sürüm verir**: çalışma zamanında bulunan önce, programın
  derlendiği sürüm parantez içinde. Makinenin kütüphanesi program derlendikten sonra
  yükseltildiğinde farklı olurlar — bir hata raporunda verilecek ilk şey.
- **Enchant başlıkları sürüm taşımaz**: parantez içindeki, yapılandırma sırasında
  `pkg-config` tarafından bildirilen sürümdür.
- **CLI11 programın içine derlenir**, ondan geriye yalnızca başlıkları kalır: iki sürümü
  her zaman aynıdır.
- **Ne Qt ne libmpv**: komut satırı hiçbirine bağlanmaz ve yalnızca karşısında çalıştığını listeler.
  Pencere onları listeler — bkz. [kılavuzu](../subedit-gui/invocation.md#--version).
- **`Languages`**, kaynak dil olan ve katalog gerektirmeyen `en` ile başlayıp
  `<prefix>/share/subedit/locale` altında kurulu ve okunabilen her kataloğu koda göre listeler —
  `Düzenle ▸ Tercihler…` öğesinin sunduğu diller.
- **Dili izleyenler**: `built with` ve `Languages`. İlk satır, adlar,
  sayılar ve dil kodları asla izlemez.
- **`--format json` ile**, `--version` seçeneğinden önce ya da sonra, **tek satırda tek nesne**: `schema`
  (`1`), `command` (`"version"`), `version`, `libraries` — metindeki sırayla,
  `{"name":"ICU","loaded":"74.2","built":"74.2"}` nesnelerinin bir dizisi — ve `languages`, kodların bir
  dizisi. İçindeki hiçbir şey çevrilmez.

<!-- example: subedit-cli --version --format json | grep -o '"name":"[^"]*"' -->
```console
$ subedit-cli --version --format json | grep -o '"name":"[^"]*"'
"name":"ICU"
"name":"Enchant"
"name":"CLI11"
```

### `--list-encodings`

**Aracın okuyup yazabildiği kodlamalar, satır başına bir tane, ada göre sıralı**, standart çıktıda —
sonra araç, `--version` gibi `0` koduyla durur: bu bir adım değil, yanıttır. Birlikte verilen bir
alt komut çalıştırılmaz.

<!-- example: subedit-cli --list-encodings | grep '^UTF' | head -4 -->
```console
$ subedit-cli --list-encodings | grep '^UTF' | head -4
UTF-16BE
UTF-16LE
UTF-32BE
UTF-32LE
```

- **Bunlar `--encoding` seçeneğinin kabul ettiği adlardır**, kanonik olanlar; takma adları (`cp1252`) da kabul edilir,
  listelenmeden.
- **Sayı kurulu ICU'ya bağlıdır** ve her ICU'da bulunanların ötesinde hiçbir ad vaat edilmez
  (`UTF-8`, `UTF-16LE`, `windows-1252`…). Bu yüzden kılavuz bir sayı vermez.
- **`UTF-16` ve `UTF-32` listede yoktur**: ICU bunları bilir, ama dönüştürücüleri kendi işaretini yazar
  ve model bunları reddeder — aşağıya bakın. Seçim yapılacak bir liste, bir sonraki alanın
  reddedeceği şeyi sunmaz.
- **`--format json` ile**, **kodlama başına bir nesne**:
  `{"schema":1,"command":"list-encodings","name":"UTF-8"}`. Bir dosyanın raporu değildir: bu
  nesneler **ne `file` ne `ok` taşır**; [biçimin](#biçim) zarfına tek istisnadır. Yine de satır başına bir nesne,
  girdi başına bir tanedir.
- **Anlatım düzeyi hiçbir şeyi değiştirmez**: liste bir sonuçtur ve `-q` onu ortadan kaldırmaz.

### `--encoding` ve zorlayamadığı şey

**Her alt komuta uygulanır**, çünkü hepsi okur: kodlaması yanlış tahmin edilen bir dosya,
üzerinde ne yapılırsa yapılsın yanlış kalır ve yalnızca `inspect` tarafından taşınan bir seçenek
çaresiz bir uyumsuzluk bırakırdı.

**Ad ICU'nundur ve tüm takma adları kabul edilir**: `cp1252`,
`windows-1252` ve `WINDOWS 1252` aynı kodlamayı adlandırır. Küme hiçbir yerde
yazılı değildir: makinenin ICU'sunun dönüştürebildiği kümedir ve
[`--list-encodings`](#--list-encodings) onu listeler. Dönüştüremediği bir ad,
tek bir dosya okunmadan önce reddedilir:

```console
$ subedit-cli --encoding klingon-1 inspect film.srt
no encoding is named "klingon-1"
```

**Bir istisna, yalnızca bir: kodlama bayt sırasını adlandırmalıdır.**
`UTF-16` ve `UTF-32`, ICU'nun bildiği ve dönüştürücüsünün **kendi işaretini** yazdığı
adlardır. Böyle bir kodlama bu modelde yaşayamaz — işaret bir seçim olmaktan
çıkar, `--no-bom` kabul edilir ve dinlenmezdi — bu yüzden ikisi de okuma için de
yazma için de reddedilir:

```console
$ subedit-cli --encoding UTF-16 inspect film.srt
"UTF-16" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do
```

`UTF-16LE` ve `UTF-16BE` aynı baytları, işaret dahil, okur ve yazar; kesin adın
eklediği şey hangisi olduğunun bilinmesidir.

**Bir bayt sırası işareti seçeneğe üstün gelir.** Bir altyazı dosyasının kendisi hakkında
bildirdiği tek şeydir ve onu bildirdiğinden başka türlü okumak,
dosyaya karşı çağıranın sözünü dinlemek olurdu. Uyumsuzluk bir tanılama iletisi
olarak **söylenir**:

```console
$ subedit-cli -vvv --encoding cp1252 inspect avec-bom.srt
avec-bom.srt: a byte order mark that contradicts the encoding asked for ("UTF-16LE"), settled by the reader
```

Gaupol'un yaptığı da budur — okumayı işaretin kodlamasıyla yeniden başlatır —
ama bunu söylemeden; burada istenen ile yapılan arasındaki fark
sessizlik değildir.

## Alt komutlar

| Alt komut | Ne yapar | Yazar mı? |
| :------------ | :-------------- | :----------- |
| [`inspect`](inspect.md) | bir dosyanın neyden oluştuğunu, hiçbir şeyi değiştirmeden bildirir | hayır |
| [`convert`](convert.md) | bir dosyayı başka bir biçimde ya da başka bir şekilde yazar | evet |
| [`shift`](shift.md) | her konumu aynı miktarda kaydırır | evet |
| [`transform`](transform.md) | her konumu doğru olduğu bilinen iki noktadan düzeltir | evet |
| [`framerate`](framerate.md) | bir dosyayı bir kare hızından ötekine yeniden zamanlar | evet |
| [`snap`](snap.md) | her zaman damgasını en yakın kareye taşır | evet |
| [`hearing-impaired`](hearing-impaired.md) | işitme engelli notlarını kaldırır | evet |
| [`adjust`](adjust.md) | süreleri bir okuma hızına ve sınırlara göre ayarlar | evet |
| [`replace`](replace.md) | altyazılardaki bir metni, bir etiketi bozmadan değiştirir | evet |
| [`case`](case.md) | metinleri sözcük başları büyük, cümle düzeni, büyük ya da küçük harfe çevirir | evet |
| [`italics`](italics.md) | metinleri italik yapar ya da italiklerini kaldırır | evet |
| [`dialogue-dashes`](dialogue-dashes.md) | diyalog çizgilerini koyar ya da kaldırır | evet |
| [`sort`](sort.md) | altyazıları başlangıç sırasına geri koyar | evet |
| [`join-repeats`](join-repeats.md) | art arda gelen tek bir metnin parçalarından tek bir altyazı yapar | evet |
| [`disjoint`](disjoint.md) | çakışan altyazıları keser; böylece hiçbir aralık artık bir başkasıyla çakışmaz | evet |
| [`repair`](repair.md) | `join-repeats`, `sort` ve `disjoint` komutlarını zincirler | evet |
| [`correct`](correct.md) | metinleri Gaupol'un desenleriyle düzeltir: notlar, sözcük birleştirme ve bölme, yaygın hatalar, büyük harf kullanımı, satır bölme | evet |
| [`append`](append.md) | ilkinden sonraki dosyaları **tek bir** dosyaya ekler | evet |
| [`split-file`](split-file.md) | bir dosyayı **iki** parçaya böler: baş ve kuyruk | evet |
| [`pair`](pair.md) | bir çeviriyi ana dosyasının konumlarına yazar ve satırlarının nasıl hizalandığını söyler | evet |
| [`transcribe`](transcribe.md) | bir videonun ya da ses dosyasının konuşmasını altyazı olarak yazar | evet — kendi hedefi, `--output` |
| [`config`](config.md) | ayar dosyasının işlem ayarlarını listeler, gösterir ve değiştirir | yalnızca ayar dosyası |

Yirmi ikisinin hepsi buradadır; aracın yardımı onları aynı sırayla listeler.

**Yalnızca biri hiçbir şeye dokunmaz**, o da sağ sütundur: `inspect` okur ve
bildirir. Diğer yirmisi yazar ve izleyenler onlardan on yedisi için geçerlidir:
[`append`](append.md) ve [`split-file`](split-file.md) alt komutlarının her birinin kendi girdi sayısı vardır — **bir** sonuç için N dosya,
**iki** sonuç için bir dosya — ve dolayısıyla kendi hedefleri, sayfalarında anlattıkları;
[`transcribe`](transcribe.md) bir video ya da ses dosyasından yeni bir dosya yapar ve yalnızca `--output` alır.
[`config`](config.md) hiçbir altyazı dosyası yazmaz: ayar dosyasını okur ve değiştirir ve
izleyenlerin hiçbiri ona uygulanmaz.

## Hedef

Yazan on yedi alt komut — [`convert`](convert.md),
[`shift`](shift.md), [`transform`](transform.md), [`framerate`](framerate.md),
[`snap`](snap.md), [`hearing-impaired`](hearing-impaired.md), [`adjust`](adjust.md),
[`replace`](replace.md), [`case`](case.md), [`italics`](italics.md),
[`dialogue-dashes`](dialogue-dashes.md), [`sort`](sort.md), [`join-repeats`](join-repeats.md),
[`disjoint`](disjoint.md), [`repair`](repair.md), [`correct`](correct.md), [`pair`](pair.md) — hedeflerini aynı biçimde alır. ([`append`](append.md) ve
[`split-file`](split-file.md) alt komutlarının kendi hedefleri sayfalarında anlatılır.) [`inspect`](inspect.md) hiçbir dosya yazmaz ve
bu seçeneklerin hiçbirini kabul etmez.

**Var olan bir hedefin üzerine yazılır**, soru sorulmadan ve bundan kaçınmak için
bir seçenek olmadan: yazma atomiktir — dosya asla yarım yazılmaz ve
yarıda kalan bir yazma eskisini bozmadan bırakır —, ama önceki içerik
kaybolur. Bu, bırakılmış bir kusur değildir: kuraldır ve aynı çıktı dizininde bir komutu
yeniden çalıştırmak, onu kendi üzerinde yeniden çalıştırmaktır.

**Açık bir hedef olmadan hiçbir şey yazılmaz.** Onu vermenin üç yolu
birbirini dışlar ve biri gereklidir: o olmadan hiçbir şey yazılmaz ve çıkış
kodu `1` olur — hiçbir şey yazmayan ve bu yüzden hedef istemeyen
[`--dry-run`](#yazmadan-önce-görme) dışında.

| Seçenek | Sonuç nereye gider |
| :----- | :---------------- |
| `--output FILE` | bu dosyanın içine, tam bu adla; tek bir girdi |
| `--output-dir DIR` | bu dizinin içine, girdinin temel adıyla |
| `--in-place` | girdinin üzerine, atomik yazmayla |

Hiçbir şey söylenmediği için girdisinin üzerine yazan bir araç, ikinci seferde
bırakılan bir araçtır. Ret bir satıra mal olur ve dosyayı
kurtarır.

Birden çok girdiyle `--output` reddedilir: son dosya öncekilerin
üzerine yazılırdı. Bir toplu işte `--output-dir`, `--in-place` ile birlikte, üçünden anlam taşıyan
tek seçenektir.

**Çıktı dizini**, eksikse, üst dizinleri dahil **oluşturulur** — `--output-dir` için ve
`--output` seçeneğinin yazdığı dizin için. Bu, komut satırı doğrulandıktan sonra ve
**ilk dosyadan önce** yapılır. Oluşturulamazsa bir kez söylenir, hiçbir dosya okunmaz ve çıkış kodu
`2` olur.

**Toplu iş, yazmadan önce bir bütün olarak değerlendirilir.** Her girdinin hedefi
önce hesaplanır ve bundan iki ret çıkar — kullanım hataları, kod
`1`, **hiçbir dosya yazılmaz** ve hiçbir dizin oluşturulmaz:

- **aynı dosyaya düşecek iki girdi** — `--output-dir` ile `a/film.srt` ve
  `b/film.srt`, ya da ikisi de WebVTT biçimine dönüştürülen `a/film.srt` ve `b/film.vtt`;
  bunların son hedefinin uzantısı değişmiştir. İleti hedefi ve iki girdiyi
  adlandırır; ikincisini yeniden adlandırmak ya da sonuncunun kazanmasına izin vermek, söylemeden bir dosya
  kaybettirirdi ve reddetmek bir satıra mal olur;
- **girdilerden biri olan bir hedef**, `--in-place` olmadan — `--output-dir .`
  aracılığıyla, `--output` aracılığıyla ya da bir sembolik bağ aracılığıyla da. Girdinin üzerine
  yazmak, adı konulan bir harekettir.

İki yol, sistem öyle dediğinde aynı dosyayı adlandırır, benzer yazıldıklarında değil:
`in/../in/film.srt` ve `in/film.srt` tek bir dosyadır.

**Uzantı, yazılan biçimi izler.** Yalnızca biçim değiştiren tek alt komut olan
[`convert`](convert.md) için değişir; diğerleri okunan dosyanın biçimini ve dolayısıyla
uzantısını korur.

| Neyin tetiklediği | İleti |
| :------------------ | :------ |
| `--dry-run` olmadan hedef yok | `no destination given: use --output, --output-dir or --in-place` |
| iki hedef | `--output, --output-dir and --in-place exclude one another` |
| bir toplu işte `--output` | `--output names one file but several were given: use --output-dir instead` |
| iki girdi, tek hedef | `<destination>: would be written by both <input> and <input>` |
| bir girdinin üzerine yazılacak | `<destination>: would be written over the input <input>` — [`append`](append.md) ve [`split-file`](split-file.md) için aynı cümle |
| oluşturulamayan çıktı dizini | `<directory>: cannot be created: permission denied` ya da `: cannot be created` |
| yazılamayan hedef | `<path>: <destination>: cannot be written: permission denied` ya da `: cannot be written` |
| yazılan kodlamada olmayan karakter | `<path>: holds a character the chosen encoding cannot write` |

**Sonuncu bir cümleyi hak eder.** Bir dosya, satır sonları ve işaretiyle birlikte,
**okunduğu kodlamayla** geri yazılır. Bir
değişiklik ona bu kodlamanın bilmediği bir karakter getirdiyse — Latin-1 içinde bir
`ł` —, yazma başarısız olur ve **hiçbir şey yazılmaz**. Onu `?` ile
değiştirmek, metni onu yeni yazanın gözü önünde kaybetmek olurdu.

## Yazmadan önce görme

**`--dry-run`**, yazan on altı alt komutun her biri tarafından kabul edilir ve her yerde aynı
anlama gelir: **oku, hesapla, bildir, hiçbir dosya yazma**. Çalıştırılmak üzere olan
satıra eklenir; uygulamak, aynı satırı onsuz yeniden çalıştırmaktır. Hesaplama
belirlidir: bir `--dry-run` çağrısının gösterdiği, dosya değişmediği sürece bir sonraki
çalıştırmanın yazacağı şeydir.

- **Hiçbir dosya yazmaz ve hiçbir dizin oluşturmaz**, `--in-place` dahil: girdi
  dosyası olduğu gibi kalır.
- **Hiçbir hedef gerektirmez.** Biri verilebilir ve **o zaman `--dry-run` olmadan olduğu gibi
  denetlenir**: aynı hedefe giden iki girdi, üzerine yazılacak bir girdi,
  bir toplu işte `--output`, nereye yazılacağını söylemenin iki yolu — `--dry-run` olmadan reddedilen bir komut satırı
  onunla da reddedilir, aynı cümleyle ve aynı `1` koduyla.
- **Çıkış kodu gerçek bir çalıştırmanınkidir**: her şey hesaplandıysa `0`, dosyalar
  okunamadıysa ya da işlenemediyse `2` veya `3`, kullanım için `1`. Dosyanın kodlamasının
  yazamadığı bir karakter, bir deneme çalıştırmasını gerçek bir çalıştırma gibi başarısız kılar.
  **Değişikliklerin var olması kodu değiştirmez**: dört kod aracın
  başarılı olup olmadığını söyler, dosyanın uygun olup olmadığını değil.
- **Anlatım bunu söyler.** Gerçek bir çalıştırmanın `-> <destination>` yazdığı yerde, bir deneme
  çalıştırması `(dry run, nothing written)` yazar.

<!-- example: printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by 1 --dry-run a.srt; ls -- *.srt -->
```console
$ printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by 1 --dry-run a.srt; ls -- *.srt
a.srt: 1 subtitle shifted by 1.000 s (dry run, nothing written)
a.srt
```

**Metinleri değiştiren bir alt komutta** —
[`hearing-impaired`](hearing-impaired.md), [`replace`](replace.md), [`case`](case.md),
[`italics`](italics.md), [`dialogue-dashes`](dialogue-dashes.md) ve [`correct`](correct.md) —, **standart çıktı önerilen
değişiklikleri taşır**. Değişen her altyazı için bir blok, dosyanın sırasıyla:

```
<path>: subtitle <number>
- <the text before, line by line>
+ <the text after, line by line>
```

- **numara**, altyazının okunan dosyada sahip olduğu numaradır, birden başlayarak sayılır;
- önceki metnin her satırının önünde `- `, sonraki metnin her satırının önünde `+ ` bulunur: birkaç satırlı bir
  metin olduğu gibi okunur ve kendisi bir diyalog
  çizgisiyle başlayan bir satır `- - ` verir;
- **işlemin kaldıracağı bir altyazı** bunu ilk satırında söyler,
  `<path>: subtitle <number> (removed)`, ve `+` satırı yoktur;
- `--document translation` ile bir çeviri altyazısı, aynı yerde `(translation)` diye kendini bildirir — bkz. [Bir çeviri](#bir-çeviri);
- her bloğun ilk satırı yolla başlar; böylece `grep '^film.srt: '`
  bir toplu işte tek bir dosyanın bloklarını bulur;
- hiçbir şey değişmeyecekse **blok yok**.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\n- [Grincement]\n- Qui est là ?\n\n' > mentions.srt; subedit-cli hearing-impaired --dry-run mentions.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\n- [Grincement]\n- Qui est là ?\n\n' > mentions.srt; subedit-cli hearing-impaired --dry-run mentions.srt
mentions.srt: 2 subtitles cleaned, 1 removed (dry run, nothing written)
mentions.srt: subtitle 1 (removed)
- [Bruit de pas]
mentions.srt: subtitle 2
- Attends [il tousse] Marie.
+ Attends Marie.
mentions.srt: subtitle 3
- - [Grincement]
- - Qui est là ?
+ Qui est là ?
```

**Liste JSON'da aynıdır**: dosyanın nesnesi bir `changes` dizisi taşır, bkz.
[biçim](#biçim); kaldırılan bir altyazı için `after` değeri `null` olur. **Yalnızca
`--dry-run` ya da `--format json` istediğinde hesaplanır**: binlerce
altyazılık bir toplu iş onu boşuna oluşturmaz.

**Diğer alt komutlarda** — [`convert`](convert.md), [`shift`](shift.md),
[`transform`](transform.md), [`framerate`](framerate.md), [`snap`](snap.md),
[`adjust`](adjust.md), [`sort`](sort.md), [`append`](append.md), [`split-file`](split-file.md), [`pair`](pair.md) —, standart
çıktı, gerçek bir çalıştırmanınkini taşır, yani metinde hiçbir şey; JSON'da,
her dosyanın nesnesi `"dry_run":true`, `"destination":null` ve aynı
`counts` ile.

## İki çıktı, iki rol

| Çıktı | Ne taşır |
| :----- | :--------------- |
| standart | **sonuç ve yalnızca sonuç** — `inspect` raporu, bir metin [`--dry-run`](#yazmadan-önce-görme) işleminin önerilen değişiklikleri; `--format json` ile dosya başına bir nesne — [`append`](append.md), [`split-file`](split-file.md) ve [`pair`](pair.md) için çalıştırma başına bir tane |
| hata | **geri kalan her şey** — anlatım, uyarılar, hatalar |

Sonucu, hikâyeyi içine karıştırmadan yönlendirmeyi sağlayan bu ayrımdır:

```console
$ subedit-cli inspect *.srt > rapport.txt
```

Rapor dosyaya gider, anlatım ekranda kalır.

## Bir betiğin okuyabileceği çıktı

**`--format json`**, standart çıktının metnini **girdi dosyası başına bir JSON
nesnesi** ile değiştirir, JSON Lines biçiminde: satır başına bir sıkışık nesne, işaretsiz
UTF-8 olarak, satır sonu `\n`. `text` varsayılandır ve değişmez. Genel
bir seçenektir ve bu yüzden alt komuttan önce yazılır; `text` ve `json` dışındaki bir değer
kullanım hatasıdır, kod `1`.

<!-- example: printf '1\n00:00:10,000 --> 00:00:12,000\nUn.\n\n' > a.srt; subedit-cli --format json shift --by 1 --output-dir sortie a.srt absent.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:10,000 --> 00:00:12,000\nUn.\n\n' > a.srt; subedit-cli --format json shift --by 1 --output-dir sortie a.srt absent.srt; echo "code=$?"
a.srt: 1 subtitle shifted by 1.000 s -> sortie/a.srt
{"schema":1,"command":"shift","file":"a.srt","ok":true,"dry_run":false,"destination":"sortie/a.srt","counts":{"subtitles":1,"shifted_by_ms":1000},"warnings":[]}
absent.srt: does not exist
{"schema":1,"command":"shift","file":"absent.srt","ok":false,"error":{"kind":"not-found","message":"absent.srt: does not exist"}}
1 of 2 files shifted, 1 failed
code=3
```

**Girdi başına bir nesne, asla sıfır, asla iki**; girdi başarılı olsun olmasın, girdilerin
verildiği sırayla — ya da [`--recursive`](lots.md) ile bulunduğu sırayla. Biri
satırları sayar, biri dosyalarını bulur. **Üç alt komut istisnadır, çünkü girdi sayıları
bir toplu işinki değildir: [`append`](append.md) (N girdi, bir çıktı), [`split-file`](split-file.md)
(bir girdi, iki çıktı) ve [`pair`](pair.md) (iki dosya, bir çıktı) tüm çalıştırma için
**tek bir nesne** yazar ve bu nesne, sayfalarının söylediği dosyayı adlandırır.** **Yeniden yazan alt komutlar
bu yüzden standart çıktıya da yazar**; metin biçimindeki sözleşmeleri bunu yapmıyordu: dosyaları
sonuçları olarak kalır ve nesne onun raporudur.

**Anlatım değişmez.** `-q`, `-v`, `-vv` ve `-vvv`, `--format` olmadığındaki gibi, metin olarak
hata çıktısına etki eder; **standart çıktı onlarla değişmez**.
Metnin 3. düzeyde tuttuğu okuma tanılama iletileri veridir ve **her zaman**
`warnings` içindedir. Bir kullanım hatası nesne yazmaz: herhangi bir dosyaya dokunulmadan önce
algılanır, bu yüzden ona iliştirilecek bir şey yoktur; hata çıktısına `1` koduyla gider ve standart çıktı
boş kalır.

### Biçim

| Anahtar | İçerik | Bulunma |
| :-- | :------ | :------- |
| `schema` | tamsayı, bu biçimin sürümü — `1` | her zaman |
| `command` | alt komutun adı | her zaman |
| `file` | yol, **verildiği gibi** ya da bir dizin taramasının oluşturduğu gibi | her zaman |
| `ok` | boolean | her zaman |
| `error` | `{"kind", "message"}` | `ok` yanlışsa |
| `dry_run` | boolean: bir [`--dry-run`](#yazmadan-önce-görme) için doğru | `ok` doğruysa, yazan bir alt komutta |
| `destination` | yazılan yol ya da `dry_run` doğruysa `null`: hiçbir şey yazılmadı | aynı şekilde |
| `counts` | alt komuta özgü **tamsayılardan** oluşan bir nesne | aynı şekilde |
| `changes` | `{"subtitle", "document", "before", "after"}` dizisi: değişen metinler, aşağıya bakın | metinleri değiştiren bir alt komutta |
| `warnings` | `{"kind", "line"?, "detail"?, "settled"?}` dizisi | `ok` doğruysa |
| `alignment` | bir çevirinin nasıl yerleştirildiği, bkz. [Bir çeviri](#bir-çeviri) | `-t` ile ve `pair` için |
| `constraints` | kullanılan ayarlar | `adjust` |
| `settings` | okunan ayar dosyası ya da hiçbiri okunmadıysa `null` — bkz. [Ayar dosyası](#ayar-dosyası) | `adjust`, `correct`, `ok` doğruysa |
| `inputs`, `tail` | eklenen dosya başına bir öğe; kuyruğun yolu | `append`; `split-file` |

`line`, dosyanın tamamıyla ilgili bir tanılama iletisinde bulunmaz; eklenecek bir şey olmadığında `detail`
bulunmaz; `settled`, okuma bir şeyi **çözdüğünde** — yeniden üretilen bir
numaralama — `true` olur ve okuma işleri olduğu gibi bıraktığında bulunmaz.

**`changes`**, dosyanın sırasıyla, metni değişen her altyazıyı listeler:
`subtitle` okunan dosyadaki numarasıdır, birden başlayarak sayılır; `document`, `"main"`
ya da `"translation"` olur; `before` önceki metindir; `after` sonraki metin, ya da altyazı kaldırılıyorsa **`null`**.
Bir metnin satır sonları içinde `\n` olarak yazılır. Bir
[`--dry-run`](#yazmadan-önce-görme) için olduğu gibi gerçek bir çalıştırma için de yazılır ve
hiçbir şey değişmediğinde **boştur (`[]`)**: varlığı alt komutun değişikliklerini listelediğini,
yokluğu hiçbirinin olmadığını söyler.

**Hiçbir yerde kayan noktalı sayı yok.** Bir konum ya da süre
**milisaniye** cinsinden bir tamsayıdır (`…_ms`); bir sayaç tamsayıdır; bir kare hızı bir **dizedir**
(`"25"`, `"24000/1001"`), çünkü tamdır, kayan noktalı bir rasyonel sayı onu
tutamaz; bir ızgara yoğunlaşması **binde** cinsinden bir tamsayıdır — `1000` kusursuz
bir ızgaradır. İki çıktı böylece yuvarlama olmadan bayt bayt karşılaştırılır.

**UTF-8 olmayan bir yol** — Linux buna izin verir — JSON'da temsil edilemez:
geçersiz baytların yerine U+FFFD ile yazılır ve nesne `path-not-utf8` uyarısını taşır.
Böyle bir yolun aslına uygunluğu vaat edilmez ve bu söylenir.

### Her alt komutun `counts` içine koyduğu

| Alt komut | `counts` anahtarları |
| :------------ | :--------------- |
| [`shift`](shift.md) | `subtitles`, `shifted_by_ms` (işaretli) |
| [`transform`](transform.md) | `subtitles` |
| [`framerate`](framerate.md) | `subtitles` |
| [`snap`](snap.md) | `subtitles`, `moved` (taşınan konumlar), `furthest_ms` (en büyük taşıma) |
| [`hearing-impaired`](hearing-impaired.md) | `cleaned` (yeniden yazılan metinler), `removed` (kaldırılan altyazılar) |
| [`replace`](replace.md) | `replaced` (değişen altyazılarda değiştirilen eşleşmeler), `matched` (bulunan eşleşmeler) |
| [`case`](case.md), [`italics`](italics.md), [`dialogue-dashes`](dialogue-dashes.md) | `changed` (metni değişen altyazılar) |
| [`correct`](correct.md) | `corrected` (değişen metinler), `removed` (kaldırılan altyazılar) — asla eşleşme yok |
| [`sort`](sort.md) | `subtitles`, `moved` (altyazı değiştiren yerler) |
| [`adjust`](adjust.md) | `subtitles` (hedeflenen altyazılar), `adjusted` (bitişi taşınanlar), ardından `sacrificed` grubu: `speed`, `minimum`, `gap`; ve `counts` dışında, `constraints` — kullanılan ayarlar |
| [`convert`](convert.md) | `subtitles`, ardından dönüştürmenin kaybettikleri: `lost_ends` ve `lost_header` (0 ya da 1), `joined_lines`, `lost_tags`, `lost_fields`, `furthest_ms` |
| [`append`](append.md) | `files`, `subtitles`, `appended`, ardından `lost_ends`, `lost_header`, `joined_lines`, `lost_tags`, `lost_fields`, `furthest_ms` toplanmış olarak; ve `counts` dışında, `inputs` — eklenen dosya başına bir öğe |
| [`split-file`](split-file.md) | `subtitles`, `head`, `tail`; ve `counts` dışında, `tail` — kuyruğun yolu (`destination` baştır) |
| [`pair`](pair.md) | `subtitles` (yazılan dosyanınkiler); ve `counts` dışında, `alignment` |
| [`inspect`](inspect.md) | `counts` yok: dosyanın betimlemesi, sayfasına bakın |

### Hata tanımlayıcıları

Bir `error.kind` bunlardan biridir; **bir okuyucu bilinmeyen bir tanımlayıcıyı
“diğer” sayar**.

| `kind` | Ne olduğu |
| :----- | :----------- |
| `not-found`, `permission-denied`, `io` | sistem dosyayı okumayı — ya da yazmayı — reddetti |
| `unknown-format`, `no-subtitle-found`, `undecodable` | dosya okundu ve kendini okutmuyor |
| `unencodable` | seçilen kodlamanın yazamadığı bir karakter |
| `no-byte-order-mark`, `no-frame-rate` | `convert` komutunun kendisine söylenmeden yazamadığı şey |
| `before-the-origin`, `beyond-the-end`, `no-transform`, `no-grid` | bu dosyaya uygulanamayan bir işlem |
| `range-out-of-bounds` | bu dosyanın içermediği bir `--range` |
| `no-style` | `italics` için, stil yazmayan bir biçim (TMPlayer, LRC) |
| `refused` | bir işlemin başka her reddi |

### Neyin vaat edildiği ve neyin edilmediği

**`schema` değeri `1` olduğu sürece vaat edilenler**: anahtarların adları, türleri ve birimleri; `ok`,
`file` ve `destination` anlamları; **girdi başına bir nesne, girdilerin sırasıyla** — kendi girdi sayısı olan üç alt komut için çalıştırma başına bir tane;
var olan `kind` tanımlayıcıları; belirlilik — aynı girdiler, aynı bağımsız değişkenler,
aynı baytlar.

**Vaat edilmeyenler**: **iletilerin metni** (`message`, `detail`), bir insan için
İngilizcedir ve gerektiğinde düzeltilir — onu karşılaştıran bir betik baştan bozuktur; bir nesnedeki
**anahtarların sırası**, sabittir ama JSON tarafından garanti edilmez; **henüz eklenmemiş
anahtarların** varlığı.

**Bir okuyucu bu yüzden bilinmeyen bir anahtarı yok sayar** ve bilinmeyen bir `kind` değerini “diğer” sayar:
bir anahtar ya da tanımlayıcı eklemek `schema` değerini değiştirmez. Bir anahtarı kaldırmak veya yeniden adlandırmak,
bir türü veya birimi değiştirmek, bir tanımlayıcıyı kaldırmak onu artırır — bu bir kırılmadır,
CHANGELOG'da ve burada duyurulur.

## Anlatım düzeyleri

Her düzey **bir öncekini içerir**: bir kademe yükselmek ekler, asla
yerine geçmez.

| Düzey | Nasıl | Hata çıktısının taşıdığı |
| :----- | :------ | :------------------------------ |
| 0 | `--quiet` | hiçbir şey, **hatalar dışında** |
| 1 | varsayılan olarak, ya da `-v` | işlenen dosya başına bir satır ve birden çok dosya olur olmaz bir özet |
| 2 | `-vv` | ve tanınanlar: biçim, kodlama, BOM, satır sonları |
| 3 | `-vvv` | ve hata ayıklama izi: okunan ve yazılan baytlar ve **her okuma tanılama iletisi** |

**Hatalar asla susturulmaz**, `--quiet` dahil. Sessizce başarısız olan bir komut
çıkış kodundan başka bir şey bırakmazdı.

Özet yalnızca iki dosyadan itibaren görünür: tek bir girdide, kendinden önceki satırı
yinelerdi.

## Tanılama iletilerini okuma

Altyazı biçimleri **elden geldiğince iyi** okunur: bir anomaliyle karşılaşan
okuyucu durmaz — karar verir ya da işleri olduğu gibi bırakır ve bunu söyler. Karşılaştığı
şey **3. düzeyde**, tanılama iletisi başına bir satır olarak, hata
çıktısına yazılır:

```
a.srt: 2 diagnostics while reading
a.srt: line 6: SubRip numbers that do not follow ("7"), settled by the reader
a.srt: line 9: a line that fits nowhere, left as it stands
```

On yedi alt komutun hepsi bunları bildirir, yalnızca [`inspect`](inspect.md) değil:
elden geldiğince iyi okunup sonra geri yazılan bir dosya aynı kararlardan geçmiştir ve bunları sessiz
tutmak hiçbir şey olmadığına inandırırdı.

**Bir tanılama iletisi asla bir başarısızlık değildir.** Dosya okundu, komut
başarılı oldu, çıkış kodu `0`. Bu yüzden en ayrıntılı düzeyde yaşarlar: daha aşağıda,
gerçekte yapılanı söyleyen satırı gömerlerdi.

### Her satırın taşıdıkları

| Parça | Ne söyler |
| :----- | :------------- |
| `line N` | nerede, bir düzenleyicinin gösterdiği gibi 1'den sayılır; tüm dosyayı ilgilendiren şeyler için **bulunmaz** |
| cümle | karşılaşılan şey, aşağıdaki on iki kategoriden |
| `("…")` | kategori yetmediğinde dosyanın hatalı metni; 80 bayta kısaltılır |
| son | bununla ne yapıldığı: `settled by the reader` ya da `left as it stands` |

**İki kategorinin satırı yoktur** ve ikisi de kodlamayla ilgilidir: baytlar tartıldı
ya da işaretleri, dosyanın tek bir satırı var olmadan önce okundu. Satırları bu yüzden cümleyle
başlar, önünde `line N` olmadan — adlandırılacak bir yer yoktur.

**Satırın sonu en çok önem taşıyandır.** `settled by the reader`,
okuyucunun karar verdiği ve yazılan dosyanın kararını taşıdığı anlamına gelir — eksik
bir numaralama yeniden üretilir. `left as it stands`, hiçbir şeye dokunmadığı anlamına gelir, çünkü **yalnızca siz
karar verebilirsiniz**: başlamadan önce biten bir altyazı olduğu gibi kalır.

### Bir bloğun satır numarası

**Tüm bir blokla** ilgili bir anomali, onu tetikleyen kesin satıra değil,
zamanlama satırına bağlanır. Beşinci satıra yazılan tutarsız bir numaralama bu yüzden
altıncı satırda, onu izleyen zamanlama satırında bildirilir — hatalı numara `("…")`
içindedir. Yalnızca **bir satırla** ilgili anomaliler — okunamayan bir zamanlama satırı, ilkinden önceki metin —
kendi satırlarında bildirilir.

### On iki kategori

| Cümle | Neyin tetiklediği |
| :----- | :------------------ |
| `a line that fits nowhere` | hiçbir bloğa girmeyen bir satır |
| `a timing line that could not be read` | okunamayan bir zamanlama satırı |
| `a SubRip block without its number` | eksik numaralama, yazarken yeniden üretilir |
| `SubRip numbers that do not follow` | atlayan numaralama |
| `text before the first timing line` | ilk zamanlama satırından önceki metin |
| `a WebVTT block of an unknown kind` | tanınmayan bir WebVTT bloğu |
| `declares an event column this tool cannot fill` | burada hiçbir şeyin doldurmadığı bir Sub Station Alpha olay sütunu |
| `counts in frames and states no rate; it was read at` | kare hızı belirtmeyen, karelerle sayılan bir dosya — kullanılan hız ayrıntıdır |
| `carries no end times; each one was taken from the next start` | bitişi olmayan bir biçim, TMPlayer ya da LRC: **tablodaki hiçbir bitiş dosyadan gelmez** |
| `more than one kind of line ending` | karışık satır sonları |
| `an encoding nothing declared` | kodlama baytlar tartılarak önerildi ve UTF-8 değil |
| `a byte order mark that contradicts the encoding asked for` | dosya bir işaret taşıyor ve `--encoding` başka birini adlandırdı — işaret kazanır |

**Ortadaki üçü 9. aşamanınkilerdir** ve üçün son ikisi bir satırdan değil
tüm dosyadan söz eder: gösterilen bir değerin okunmadığını söylerler. Bir LRC üzerinde doldurulmuş bir
`End` sütunu, bir MicroDVD üzerinde bir konum — dosya bunların hiçbirini taşımıyordu.

**Bir işlem kendi iletisini ekleyebilir**, bu bir okuma tanılama iletisi değildir:
[`correct`](correct.md#uygulanamayan-bir-desen), alt komutun vazgeçtiği deseni altyazıyla birlikte
söyler. 1. düzeyde söylenir ve dosyanın nesnesinde `pattern-failed` tanımlayıcısını
taşır — asla dosyanın başarısızlığı değildir.

## Bir çeviri

Bir çeviri dosyası **herhangi bir altyazı dosyası gibi** işlenir:
`subedit-cli case --to sentence film.fr.srt` onu yeni bir şey olmadan değiştirir. Eşlemenin eklediği ve
üç seçeneğin söylediği şey dardır: **bir çevirinin satırlarının ana dosyanın
altyazılarına nasıl düştüğünü bilmek** — `inspect -t` budur —, ve **çeviriyi ana dosyasına yerleşik tutarak
değiştirmek**, pencereyle aynı kurallarla.

| Seçenek | Nerede | Değer | Varsayılan |
| :----- | :---- | :----- | :----- |
| `-t`, `--translation-file` | `inspect`, `hearing-impaired`, `replace`, `case`, `italics`, `dialogue-dashes`, `correct`, `pair` | çeviri dosyası | — |
| `--document` | altı metin alt komutu, `inspect` ve `pair` değil | `main` veya `translation` | `main` |
| `--align-method` | `-t` ile aynı | `position` veya `number` | `position` |

- **`position`**, her satırın ortasını altyazıların sınırlarıyla karşılaştırır: eksik bir satır
  ondan sonrakileri kaydırmaz. **`number`**, *n* numaralı satırı *n* numaralı altyazıya, konumlara
  bakmadan gönderir. Varsayılan konumdur, pencerenin sahip olduğu nedenle: ortadaki eksik bir satır, numarayla eşlenmiş bir çevirinin
  geri kalanının tamamını kaydırır.
- **`--document translation` için `-t` gerekir ve `-t` için `--document translation` gerekir**; bir metin
  alt komutunda biri diğeri olmadan bir kullanım hatasıdır, kod `1`, çünkü hiçbir hareketin
  kullanmadığı okunmuş bir dosya bir tercih değil, bir ihmaldir. `inspect` üzerindeki `-t` için `--document` yoktur:
  rapor eder. **`-t` olmadan `--align-method` da bir kullanım hatasıdır.**
- **`-t` bir dosyayı adlandırır ve bu yüzden yalnızca tek bir girdi için anlamlıdır**: birden çok girdiyle ya da
  `--recursive` ile alınan bir dizinle, `--output` gibi, **bir dosya
  okunmadan önce** reddedilir. Bir toplu iş için, çift başına bir çağrı çalıştırın.
- **Ana dosyayla aynı olan bir çeviri dosyası reddedilir** — yolu nasıl yazılmış olursa olsun:
  `<translation>: the file is already open as the main document`, kod `2`.
- **Yalnızca hedeflenen belge yazılır**, **kendi yoluna ve kendi biçiminde**: çeviri dosyasının
  biçimi, kodlaması ve satır sonları. `--output` çevirinin dosyasını adlandırır,
  `--output-dir` temel adını oraya koyar, `--in-place` onu yeniden yazar; **ana dosyaya
  asla dokunulmaz**. Raporun yolları çevirinin yollarıdır.
- **Eşlemenin canlandırdığı altyazılar** — hiçbir şeyin ulaşmadığı bir satır — çeviriye,
  yöntem ne olursa olsun **kendi konumlarıyla** yazılır; **kimsenin
  çevirmediği bir altyazı metinsiz bir blok olarak yazılır**, bu da anomalisiz okunur.
- **`--encoding` ve `--frame-rate` her iki dosyaya da uygulanır.**
- **Konum** alt komutları — `shift`, `transform`, `framerate`, `snap`, `adjust` — bu
  seçeneklere sahip değildir: onlara her iki dosyayı da verin, bir toplu işte olduğu gibi,
  `subedit-cli shift --by 2 film.srt film.fr.srt --output-dir out/`.
- **Çeviriyi ana dosyaya göre yeniden zamanlanmış yazmak** — eşlemenin yaptığı ve tek başına bir dosyanın
  yapmadığı tek şey — ne `--document` ne `--recursive` seçeneği olan [`pair`](pair.md) alt komutunun işidir.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\n[Un oiseau] Bonjour.\n\n2\n00:00:03,000 --> 00:00:04,000\nAu revoir.\n\n' > film.fr.srt; subedit-cli hearing-impaired film.srt -t film.fr.srt --document translation --output propre.fr.srt; cat propre.fr.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\n[Un oiseau] Bonjour.\n\n2\n00:00:03,000 --> 00:00:04,000\nAu revoir.\n\n' > film.fr.srt; subedit-cli hearing-impaired film.srt -t film.fr.srt --document translation --output propre.fr.srt; cat propre.fr.srt
film.fr.srt: 1 subtitle cleaned, 0 removed -> propre.fr.srt
1
00:00:01,000 --> 00:00:02,000
Bonjour.

2
00:00:03,000 --> 00:00:04,000
Au revoir.
```

`--format json` ile her dosyanın nesnesi ayrıca, çevirinin nasıl yerleştirildiğini söyleyen `alignment` anahtarını taşır —
`{"file", "method", "attached", "born", "untranslated", "out_of_order"}`, [`inspect`](inspect.md#json-biçiminde) komutunun
`translation` anahtarı gibi — ve **`--dry-run` ile** `changes`, çevirinin altyazılarını,
her biri `"document":"translation"` ile listeler.

## Birden çok dosya

Her alt komut birkaç yolu kabul eder; girdi sayısı kendi sayfalarında anlatılan [`append`](append.md), [`split-file`](split-file.md) ve
[`pair`](pair.md) dışında. Her biri
bağımsız işlenir: birinin başarısızlığı diğerlerini kesmez ve başarısızlıklar
dosya ve neden adlandırılarak bildirilir.

**Bir dizin, `--recursive` (`-r`) ile içerdiği dosyaların yerine geçer**:
bu seçenek olmadan bir dizin vermek bir kullanım hatasıdır, kod `1`, ve hiçbir şey
işlenmez — hiçbir şey işlememek ve `0` döndürmektense reddetmek daha iyidir:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?" -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?"
films: is a directory: use --recursive
code=1
```

Tarama, `--output-dir` altında korunan ağaç ve bir kenara bırakılanlar
[Dizin ağacını işleme](lots.md) sayfasında anlatılır.

## Man sayfaları

Bir kurulum, `subedit-cli(1)` yanına **alt komut başına bir man sayfası** koyar:
`subedit-cli-shift(1)`, `subedit-cli-convert(1)` ve diğerleri, [alt komut tablosunun](#alt-komutlar)
her satırı için bir tane. `subedit-cli(1)` bu bölümü — genel seçenekleri, çıktıları,
çıkış kodlarını — taşır ve her diğer sayfa kendi alt komutunun bölümünü, örnekleri dahil, taşır.
Pencerenin sayfası `subedit-gui(1)` bunların yanındadır.

```console
$ man subedit-cli
$ man subedit-cli-shift
```

**Sayfalar bu kılavuzdan ayrı yazılmaz**: program derlenirken ondan üretilirler,
bu yüzden onun söylediğini söylerler ve ondan uzaklaşamazlar. Her sayfanın DOSYALAR bölümü,
kılavuzun kurulu olduğu dizini adlandırır.

| Sayfalar | Nereye kurulurlar |
| :---- | :----------------------- |
| İngilizce | `share/man/man1` |
| kurulu her dil — Katalanca, Çekçe, Almanca, İspanyolca, Fince, Fransızca, Galiçyaca, İbranice, Macarca, Interlingue, İzlandaca, İtalyanca, Felemenkçe, Lehçe, Portekizce, Brezilya Portekizcesi, Rusça, Sırpça, Türkçe, Çince (Basitleştirilmiş) | `share/man/<language>/man1` — `share/man/fr/man1` |

**`man` dili yerel ayardan seçer**, program gibi: bir Fransızca yerel ayar altında
`man subedit-cli` Fransızca sayfayı gösterir ve sayfaları kurulu olmayan bir dil
İngilizce olanları alır. `-L`, yerel ayar ne olursa olsun bir dil ister:

```console
$ man -L fr subedit-cli-shift
```

## Çıkış kodları

| Kod | Anlam |
| :--- | :------------ |
| `0` | her şey başarılı oldu |
| `1` | kullanım hatası — bilinmeyen seçenek, geçersiz değer, yasak bileşim |
| `2` | hiçbir dosya işlenemedi |
| `3` | bazı dosyalar işlendi, bazıları işlenmedi |

`2` ve `3` ayrı tutulur, böylece bir betik çıktıyı yeniden okumadan nasıl davranacağını bilir:
“hiçbir şey çalışmadı” ve “biri eksik” aynı tepkiyi gerektirmez.

Bir kullanım hatası **herhangi bir işlemden önce** algılanır: bir toplu işi
yarım işlenmiş bırakmaz.

## Hatalar

| Neyin tetiklediği | Hata çıktısına yazılan |
| :------------------ | :--------------------------------------- |
| bilinmeyen seçenek ya da alt komut | hatalı ad ve `--help` işaretçisi |
| `-v` ile `--quiet` | `--quiet and -v ask for opposite things; give one or the other` |
| `--no-config` ile `--config` | `--config excludes --no-config` |
| boş adla `--config` | `--config needs a file name` |
| eksik dosya | `<path>: does not exist` |
| okunamayan dosya | `<path>: cannot be opened: permission denied` |
| kendi işaretini yazan kodlama | `"<name>" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do` |
| kodu çözülemeyen baytlar | `<path>: cannot be decoded in the chosen encoding` |
| tanınmayan biçim | `<path>: is in no format this tool knows` |
| altyazıya benzeyen hiçbir şey yok | `<path>: holds nothing recognisable as a subtitle` |
| `-t` olmadan `--document translation` | `--document translation needs the translation file: use -t` |
| `--document translation` olmadan `-t` | `-t names a translation to change: use --document translation` |
| `-t` olmadan `--align-method` | `--align-method needs a translation file: use -t` |
| birkaç girdi ya da bir dizinle `-t` | `-t names one file but several inputs were given: use one invocation per pair` |
| ana dosyayla aynı olan çeviri | `<translation>: the file is already open as the main document` |
