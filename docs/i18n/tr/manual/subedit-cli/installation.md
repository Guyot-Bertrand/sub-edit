# Kurulum

<!-- machine-translated -->
> Bu sayfa makine çevirisidir ve henüz dili bilen bir okuyucu tarafından gözden geçirilmemiştir.

**Bu sayfa her iki program için de geçerlidir** — `subedit-cli` ve `subedit-gui`.
Birlikte derlenir ve birlikte kurulurlar.

İki yol vardır: **yerel bir paket**, `.deb` ya da `.rpm`, veya istenen yere
kurulan **kaynaktan derleme**.

## Paketler

İkisi de CPack ile **aynı kurulum kurallarından** üretilir:
aynı dosyaları aynı yerlere koyarlar ve yalnızca bağlı oldukları
paketlerin adlarıyla ayrılırlar — aynı kütüphaneler Debian'da ve Fedora'da
farklı adlandırılır.

```bash
sudo apt install ./subedit_<version>_amd64.deb      # Debian, Ubuntu
sudo dnf install ./subedit-<version>.x86_64.rpm     # Fedora, and relatives
```

| Paketin kurduğu | Nereye |
| :---------------------- | :-- |
| `subedit-cli`, `subedit-gui` | `/usr/bin` |
| menü girdisi, simge, yazılım merkezi üst verisi | `/usr/share/applications`, `/usr/share/icons`, `/usr/share/metainfo` |
| bu kılavuz, Markdown biçiminde | `/usr/share/subedit/manual` |
| kurulu her dile çevirisi — Katalanca, Çekçe, Almanca, İspanyolca, Fince, Fransızca, Galiçyaca, İbranice, Macarca, Interlingue, İzlandaca, İtalyanca, Felemenkçe, Lehçe, Portekizce, Brezilya Portekizcesi, Rusça, Sırpça, Türkçe, Çince (Basitleştirilmiş) | `/usr/share/subedit/manual/<language>` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR`, `ru`, `sr`, `tr`, `zh_CN` |
| Gaupol'un düzeltme desenleri | `/usr/share/subedit/patterns` |
| `subedit-cli` man sayfaları, alt komut başına bir tane, ve `subedit-gui` sayfası — bkz. [Man sayfaları](invocation.md#man-sayfaları) | `/usr/share/man/man1` |
| `subedit-cli` sayfalarının kurulu her dildeki karşılığı — Katalanca, Çekçe, Almanca, İspanyolca, Fince, Fransızca, Galiçyaca, İbranice, Macarca, Interlingue, İzlandaca, İtalyanca, Felemenkçe, Lehçe, Portekizce, Brezilya Portekizcesi, Rusça, Sırpça, Türkçe, Çince (Basitleştirilmiş) | `/usr/share/man/<language>/man1` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR`, `ru`, `sr`, `tr`, `zh_CN` |

`ffmpeg`, her iki paket tarafından da **önerilir, zorunlu değildir**: o olmadan pencere
yalnızca filmin bildirdiği kare hızını sunmayı bırakır — bkz.
[`ffmpeg` neyi değiştirir](../subedit-gui/video.md#ffmpeg-gerekmez).

### Paketlerin dağıtımdan istedikleri

**`apt` ya da `dnf` ile kurun, asla `dpkg -i` ya da `rpm -i` ile değil.** Bu iki
komut paketi bağımlılıklarını çözmeden yerleştirir: yalnızca eksik olanları
listeler ve durur. Gereken her şeyin `dnf` erişiminde olduğu bir
Fedora'da `rpm -i` şunu yanıtlar:

```console
$ rpm -i subedit-0.13.0-1.fc43.x86_64.rpm
error: Failed dependencies:
        libicui18n.so.77()(64bit) is needed by subedit-0.13.0-1.fc43.x86_64
        libicuuc.so.77()(64bit) is needed by subedit-0.13.0-1.fc43.x86_64
```

`sudo dnf install ./subedit-<version>-1.fc43.x86_64.rpm` komutu, varsa, bu
kütüphaneleri getirir.

**Alınacak `.rpm`, kendi Fedora sürümünüzünkidir**: dosya adı
sürümü taşır, `fc42`, `fc43`, `fc44`. Her biri o Fedora üzerinde, onun
ICU'suna karşı derlenir — hangisini alacağınızı `cat /etc/fedora-release` söyler. Başka bir
Fedora'nın `.rpm` dosyası, sizde bulunmayan bir ICU ister.

| Bağımlılık | `.deb` | `.rpm` | Kısıt |
| :--------- | :----- | :----- | :--------- |
| Qt 6: `Widgets`, `Gui`, `Core` | `libqt6widgets6`, `libqt6gui6`, `libqt6core6` | `qt6-qtbase-gui` | pencere için **≥ 6.4** |
| libmpv | `libmpv2` veya `libmpv1` | `mpv-libs` | ikili dosya `libmpv.so.2` ile bağlanır |
| Enchant 2 | `libenchant-2-2` | `enchant2` | yazım denetimi ve `join-words` ile `split-words` görevleri ([`correct`](correct.md)) |
| **ICU** | **`libicu74`** | `libicu`, yani **onu derleyen Fedora'nın** `libicuuc.so.N` ve `libicui18n.so.N` dosyaları | **`.rpm` dosyasının Fedora'sındaki sürüm**, tam olarak — aşağıya bakın |
| glibc ve libstdc++ | — | — | **yeterince yeni**: `.deb` için Ubuntu 24.04 sürümünün kütüphaneleri, `.rpm` için onu derleyen Fedora'nınkiler |

**ICU, önemli olan kısıttır ve paketin bir satırı değiştirilerek
gevşetilemez.** ICU'nun simgeleri ana sürüm numarasını taşır
(`u_strToUTF8_74`): ICU 74 ile derlenmiş bir ikili dosya, ICU 76 ya da 77 ile
bağlanamaz. Daha geniş yazılmış bir bağımlılık paketin kurulmasına izin verir,
sonra açılışta başarısız olurdu — tam olarak bildirmek, dürüst kalan
yoldur. **`.rpm` dosyasının bir kez değil, yayımlanan her Fedora için
ayrı ayrı, onun ICU'suna karşı derlenmesinin nedeni budur**; `.deb`, Ubuntu 24.04 üzerinde derlenir.

| Dağıtım | Ne sağlar |
| :----------- | :---------------- |
| Ubuntu 24.04 | `.deb` kurulur: orada ICU sürümü 74 olarak bulunur |
| Fedora 42, 43, 44 | sürümünün `.rpm` dosyası (`…fc42…`, `…fc43…`, `…fc44…`) **`dnf` aracılığıyla** kurulur, o Fedora'nın ICU'suna karşı derlenmiştir |
| Fedora rawhide (bir sonraki sürüm), diğer `.rpm` dağıtımları | **hayır**: onlar için `.rpm` derlenmez — kaynaktan derleyin |
| ICU sürümü 74 olmayan her Debian ya da Ubuntu dağıtımı | **hayır**: `.deb`, `libicu74` ister — kaynaktan derleyin |

**Kaynaktan derleme yedek çözümdür ve bu kısıtı taşımaz**: sürümü ne olursa olsun
makinenin ICU'suna bağlanır.
glibc'si paketlerinkinden daha eski olan bir dağıtımda da gereken budur.

### Her birinde neyin sınandığı ve neyin sınanmadığı

**İkisi de aynı ölçüde denetlenmez ve olmayan bir eşitliğe
inandırmaktansa bunu söylemek daha iyidir.** Geliştirme Ubuntu üzerinde yapılır; orada derlenen bir
`.rpm` orada kurulamaz. Başka yerde kurulur: bir kapsayıcıdaki bir
Fedora'da, haftada bir.

| | `.deb` | `.rpm` |
| :--- | :----- | :----- |
| dosya listesi | denetlenir | denetlenir ve **`.deb` listesiyle karşılaştırılır** |
| bildirilen bağımlılıklar | var olduğu denetlenir | var olduğu denetlenir |
| bağımlılık adlarının dağıtımda var olduğu | evet, Ubuntu'nunkilerdir | **evet**, `dnf` ile çözülür |
| paketin kurulduğu | hayır — yönetici hakları gerekir | **evet**, bir kapsayıcıdaki Fedora'da |
| kurulu ikili dosyaların başladığı | evet, geçici bir önekten | **evet**, kurulu paketten |

**Fedora denetimi, yayımlananları değil, Ubuntu üzerinde derlenen `.rpm` dosyasını sınar.** O imajda kendi
ICU sürümü (74) yoktur: paket orada, Fedora bir uyumluluk `libicu74` paketi tuttuğu için
kurulur ve denetim bu paket var olduğu sürece yeşil olurdu. Yayımlanan `.rpm` dosyaları kendi Fedora'larında derlenir ve artık
bu uyumluluğa bağlı değildir. Bkz.
[paketlerin istedikleri](#paketlerin-dağıtımdan-istedikleri).

**İki listeyi karşılaştırmak en önemli denetimdir.** İki
paket aynı kurulumdan çıkar: aralarındaki bir fark, biçimin değil,
kurulum kurallarının bir kusuru olurdu.

**`.deb` dosyasının hâlâ kanıtlamadığı şey kurulduğudur** ve nedeni
değişmedi: `dpkg -i` yönetici hakları ister; bir kalite kapısının bunlara
sahip olması da, istemesi de doğru olmaz. `.rpm` ise kimseye ait olmayan bir
kapsayıcıda kurulur. Bkz.
[ADR 0023](../../../../adr/0023-deb-et-rpm-pour-la-premiere-livraison.md).

**Flatpak ve AppImage sunulmaz** ve bu bir tercihtir — bir ihmal değil.
Aynı ADR'de açıklanır ve **kesindir**: Debian ailesinden de Fedora ailesinden de olmayan bir dağıtım
kaynaktan derler; bir sonraki bölüm bunu eksiksiz anlatır.

## Kaynaktan derleme

### Önkoşullar

| Gerekenler | Neden | Debian, Ubuntu |
| :------------ | :------- | :------------- |
| CMake ≥ 3.28 | derleme | `cmake` |
| bir C++23 derleyicisi — GCC 13 yeter | — | `g++` |
| `make`, `pkg-config` | derleme | `make`, `pkg-config` |
| CLI11 | komut satırını okuma | `libcli11-dev` |
| Qt 6 ≥ 6.4, `Widgets` modülü | pencere | `qt6-base-dev` |
| `libmpv` | yerleşik oynatıcı | `libmpv-dev` |
| ICU | kodlamaları okuma | `libicu-dev` |
| Enchant 2 | yazım denetiminin kütüphanesi | `libenchant-2-dev` |

**Son dördü yalnızca `subedit-cli` derlemek için bile
gereklidir**: CMake yapılandırması bunları tüm proje için arar ve
eksikse durur. ICU ve Enchant iki kez gereklidir — yürütülebilir dosya bunları yükler, biri
kodlamaları dönüştürmek, öbürü yazım denetimi için — ve
çalışma zamanında yüklediği tek üçüncü taraf kütüphanelerdir.

**İlk `cmake` sırasında bir ağ bağlantısı gerekir**: Catch2 test kütüphanesi
yapılandırma sırasında GitHub'dan indirilir. Sonrasında yeniden indirilmez.
Aracı tek başına, ağsız derlemek için aşağıya bakın.

#### Gerekli olmayanlar

| İsteğe bağlı olan | O olmadan kaybedilen |
| :-------------------- | :--------------------- |
| `ffmpeg`, `ffprobe` aracı için | pencere artık filmin bildirdiği kare hızını sunmaz — [ayrıntılar](../subedit-gui/video.md#ffmpeg-gerekmez) |
| bir görüntü sunucusu | `subedit-cli` için hiçbir şey; `subedit-gui` kendini göstermek için birine ihtiyaç duyar |

### Derleme

```bash
git clone git@github.com:Guyot-Bertrand/sub-edit.git subedit
cd subedit
make build
```

İkili dosyalar `build/dev/bin/` içinde üretilir — `subedit-cli` ve
`subedit-gui`.

Eniyilenmiş bir sürüm için:

```bash
make release
```

Her iki ikili dosya da o zaman `build/release/bin/` içindedir — `subedit-cli` ve
`subedit-gui`. Hedef başka bir şey derlemez: ne kıyaslama aracını ne de
kendi hedefleri olan uçtan uca test düzeneğini.

**Bu hedefi, eşdeğer olan iki `cmake` komutuna tercih edin**; onlar,
bağlama zamanı eniyilemesinin kaç işlem başlatabileceğini bilemez:
her bağlamada çekirdek sayısı kadar işlem başlatır.
`make release` ona, derlemenin geri kalanına verdiği gibi `JOBS` değerini geçirir.

### Kurulumu yapma

```bash
make release
cmake --install build/release --prefix ~/.local
```

Seçilen önek için `GNUInstallDirs` tarafından belirlenen yollarda kurulanlar:

| Dosya | Nereye |
| :------ | :-- |
| `subedit-cli`, `subedit-gui` | `<prefix>/bin` |
| masaüstü menü girdisi | `<prefix>/share/applications` |
| simge | `<prefix>/share/icons/hicolor/scalable/apps` |
| yazılım merkezi üst verisi | `<prefix>/share/metainfo` |
| bu kılavuz, Markdown biçiminde | `<prefix>/share/subedit/manual` |
| kurulu her dile çevirisi — Katalanca, Çekçe, Almanca, İspanyolca, Fince, Fransızca, Galiçyaca, İbranice, Macarca, Interlingue, İzlandaca, İtalyanca, Felemenkçe, Lehçe, Portekizce, Brezilya Portekizcesi, Rusça, Sırpça, Türkçe, Çince (Basitleştirilmiş) | `<prefix>/share/subedit/manual/<language>` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR`, `ru`, `sr`, `tr`, `zh_CN` |
| Gaupol'un düzeltme desenleri | `<prefix>/share/subedit/patterns` |
| her iki ikili dosyanın man sayfaları, `subedit-cli` için alt komut başına bir tane | `<prefix>/share/man/man1` |
| `subedit-cli` sayfalarının kurulu her dildeki karşılığı — Katalanca, Çekçe, Almanca, İspanyolca, Fince, Fransızca, Galiçyaca, İbranice, Macarca, Interlingue, İzlandaca, İtalyanca, Felemenkçe, Lehçe, Portekizce, Brezilya Portekizcesi, Rusça, Sırpça, Türkçe, Çince (Basitleştirilmiş) | `<prefix>/share/man/<language>/man1` — `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR`, `ru`, `sr`, `tr`, `zh_CN` |

**Bunlar paketlerin kurduğu dosyaların aynısıdır**: paketler, ikinci kez
betimlenmek yerine bunlardan çıkar.

`--prefix ~/.local` ile her iki ikili dosya da çoğu dağıtımın `PATH` değişkeninde bulunan
`~/.local/bin` içine yerleşir. Bir sistem öneki —
örneğin `/usr/local` — uygun hakları gerektirir.

**`~/.local` altına konan bir menü girdisi her zaman hemen
görünmez**: masaüstleri önbelleklerini kendi hızlarında yeniden okur. `subedit-gui`
bu arada, her komut gibi bir uçbirimden başlatılabilir.

`DESTDIR` dikkate alınır, bir paketleyicinin beklediği gibi: aşamalı bir kurulum
aşama dizininin dışında hiçbir şeye dokunmaz.

```bash
DESTDIR=/tmp/scene cmake --install build/release --prefix /usr
```

### Aracı tek başına derleme

Testler varsayılan olarak derlenir ve ağı isteyen de onlardır —
`make release` onları derlemez, ama projeyi yapılandırır ve Catch2'yi
getiren yapılandırmadır. Onları bir kez devre dışı bırakmak yeter: ayar
CMake önbelleğinde kalır ve sonraki derlemeler onu korur.

```bash
cmake --preset release -DSUBEDIT_BUILD_TESTS=OFF
make release
```

Bu, bir katkıcıyı hiçbir şeyden muaf tutmaz: kalite kapısı
onları gerçekten ister.

### Derleme bağlama sırasında başarısız olursa

Geçerli kodda “undefined reference to `std::cout`” yağmuru
genellikle sistemin `c++` alternatifinin `g++` yerine `gcc` gösterdiği anlamına gelir:
C++ derlenir, ama standart kütüphane bağlanmaz.

```bash
ls -l /etc/alternatives/c++                                       # check
sudo update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++ 100   # fix
CXX=g++ make build                                                # work around
```

CMake yapılandırması bu durumu algılar ve bağlamanın
başarısız olmasına izin vermek yerine bu iletiyle durur.

### Paket kurulumu `libicu` hakkında şikâyet ederse

```console
error: Failed dependencies:
        libicuuc.so.74()(64bit) is needed by subedit-<version>.x86_64
```

Yazılan komuttan ve dosya adından ayırt edilen üç neden:

- **`rpm -i`**: hiçbir şeyi çözmez. Şununla yeniden deneyin:
  `sudo dnf install ./subedit-<version>.fcNN.x86_64.rpm`;
- **adında `fcNN` bulunmayan bir `.rpm`** ya da sizinkinden başka bir Fedora'dan olan:
  onu derleyen makinenin ICU'sunu ister. `fcNN` değeri
  `cat /etc/fedora-release` çıktısı olanı alın;
- **sürümünüz için `.rpm` yok**: Fedora rawhide ya da başka bir dağıtım.
  Bu deponun hiçbir paketi orada kurulmaz;
  [kaynaktan derleyin](#kaynaktan-derleme).
