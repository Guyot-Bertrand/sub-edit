# Çalıştırma

<!-- machine-translated -->
> Bu sayfa makine çevirisidir ve henüz dili bilen bir okuyucu tarafından gözden geçirilmemiştir.

```
subedit-gui [--version] [<file>]
```

| Bağımsız değişken | Zorunlu | Değer | Varsayılan |
| :------- | :------- | :---- | :------ |
| `<file>` | hayır | açılacak bir altyazı dosyasının yolu | yok — pencere boş açılır |
| `--version` | hayır | — | — |

**Tek bir dosya.** Sonraki bağımsız değişkenler yok sayılır: pencere aynı anda yalnızca bir proje tutar.

## `--version`

Komut satırının `--version` seçeneğinin yazdığı bloğu standart çıktıya yazar ve pencere açmadan hemen döner. Çıkış kodu `0`.

**İlk satırda tek başına `subedit <version>`**, ardından pencerenin birlikte çalıştığı her kitaplık için bir satır: çalışma zamanında bulunan sürüm ve parantez içinde derlendiği sürüm; sonra kurulu arayüz dilleri, koduyla. Qt 6.4.2 bulunan bir makinede:

```console
$ subedit-gui --version
subedit <version>
  ICU      74.2 (built with 74.2)
  Enchant  2.3.3 (built with 2.3.3)
  Qt       6.4.2 (built with 6.4.2)
  libmpv   2.2 (built with 2.2)
  Languages: en, fr
```

Sayılar buraya kopyalanmaz: onlar makineye aittir ve bu kılavuzda programın kendi bloğunu güncel tutacak üretilmiş bir blok yoktur. Kitaplıklar, komut satırının kullandığı ICU ve Enchant ile yalnızca pencerenin bağladığı Qt ve oynatıcı libmpv’dir; libmpv sürümü, `mpv.pc` dosyasının duyurduğu istemci API’si sürümüyle belirtilir. Geri kalanı [komut satırı kılavuzunda](../subedit-cli/invocation.md#--version) anlatılır. `built with` ve `Languages` sözcükleri pencerenin diline uyar; başka hiçbir şey uymaz.

`Yardım ▸ subedit Hakkında` aynı kitaplıkları, aynı listeden, bir tabloda gösterir — ad, yüklenen sürüm, derlendiği sürüm, her biri bir satır — ve altında dilleri; başlıkları pencerenin diline uyar.

## Bir dosya açma

```console
$ subedit-gui film.srt
```

Okunan biçimler, açma süzgecinin listelediği **dokuz biçimdir** — bkz. [Açma ve kaydetme](files.md#açma). Biçim içeriğe göre tanınır, uzantıya göre değil, **kodlama da öyle**: dosya taşıyorsa bir BOM, taşımıyorsa baytlarının akla getirdiği kodlama.

## Açma başarısız olduğunda

Eksik bir dosya, okunamayan bir dosya, baytları hiçbir kodlamada çözülemeyen bir dosya ya da içeriğini hiçbir biçimin tanımadığı bir dosya: **pencere yine de açılır, boş**, ve hata, hata akışına yazılır.

**İleti nedeni adlandırır**, komut satırıyla aynı sözcüklerle:

| İleti | Ne oldu |
| :------ | :------------ |
| `does not exist` | yol hiçbir dosyayı göstermiyor |
| `cannot be opened: permission denied` | sistem açmayı reddediyor |
| `cannot be read` | sistem başka bir nedenle reddetti |
| `cannot be decoded in the chosen encoding` | baytlar seçilen kodlamada okunamıyor |
| `is in no format this tool knows` | hiçbir biçim içeriği tanımıyor |
| `holds nothing recognisable as a subtitle` | biçim tanınıyor, ancak içinde altyazı sayılabilecek bir şey yok |

```console
$ subedit-gui notes.txt
subedit-gui: notes.txt: is in no format this tool knows
```

Başlamayı reddetmek yerine boş açılır: araç yine bir işe yarar ve bir bağımsız değişkenin başarısızlığı onu hiç başlatmamak için bir neden değildir.

**Kötü biçimlenmiş ama okunabilir bir dosya açılır**, ve okumanın yol boyunca düzelttikleri durum çubuğundaki bir düğmede okunabilir — bkz. [bir okumanın tanılama iletileri](files.md#bir-okumanın-tanılama-iletileri).

## Çıkış kodları

| Kod | Ne zaman |
| :--- | :------ |
| `0` | pencere açıldı, sonra olağan biçimde kapatıldı ya da `--version` yanıt verdi |
| `2` | açılıştan önce beklenmedik bir hata — Qt eksik, ekran kullanılamıyor |

Bir dosyayı açma başarısızlığı **çıkış kodunu değiştirmez**: pencere açıldı ve kodun bildirdiği budur.

## Kılavuz sayfası

Bir kurulum `share/man/man1` altına `subedit-gui(1)` sayfasını koyar:

```console
$ man subedit-gui
```

Çağırma biçimini, `--version` seçeneğini, açma başarısızlığı iletilerini, çıkış kodlarını, iki ortam değişkenini ve ayar dosyasını verir. **Pencereyi anlatmaz** — onu bu kılavuz anlatır ve sayfa buraya gönderir.

## Qt platformu

`QT_QPA_PLATFORM`, pencerenin üzerinde çalışacağı şeyi seçer. **Değişken tanımlıysa `subedit-gui` ona bağımsız değişken olmadan uyar.**

Tanımlı değilse seçimi Qt’ye bırakır: yerleşik oynatıcı görüntüyü pencerenin kendisinde çizer ve platformdan hiçbir şey istemez — bkz. [Oynatıcı](player.md).

`QT_QPA_PLATFORM=offscreen`, pencereyi grafik sunucusunun bulunmadığı yerde çalıştırır — sürekli tümleştirmenin ve testlerin yaptığı budur.

## Dil

Pencere, ortamının dilini konuşur ve bu dil **başlarken bir kez seçilir**: çalışan bir pencerenin değişkenlerini değiştirmek, bir sonraki başlatmaya kadar hiçbir şeyi değiştirmez. Değişkenler ve kurallar komut satırınınkilerdir — `LANGUAGE`, `LC_ALL`, `LC_MESSAGES`, `LANG`, İngilizce için `LC_ALL=C`, istenen dil kurulu değilse **tek sözcük etmeden** İngilizce — ve [komut satırı kılavuzunda](../subedit-cli/invocation.md#dil) anlatılır. Kataloglar aynı dosyalardır ve `<prefix>/share/subedit/locale` altına kurulur.

**`Düzenle ▸ Tercihler…` içinde seçilen bir dil önce gelir** ve ortam yalnızca `System default` iken okunur; bir seçim yapılıncaya kadar durum budur: orada seçilen kurulu bir dil, `LC_ALL=C` dahil, her değişkenin önüne geçer. Seçim de bir sonraki başlatmada uygulanır. Bkz. [Arayüz dili](preferences.md#arayüz-dili).

| Ne | Dile uyar mı |
| :--- | :------------------- |
| menüler ve öğeleri, altı çizili harfleriyle; araç çubuğu; ipuçları | evet |
| tablonun sütunları ve `Görünüm ▸ Sütunlar`, sekmeler, durum satırı, oynatma çubuğu | evet |
| Qt’nin kendi çizdiği standart düğmeler (`OK`, `Cancel`) | evet, o dilin Qt’ye ait kataloğu kuruluysa (`qtbase_<language>.qm`); aksi halde İngilizce |
| iletişim kutuları, `Correct Texts…` sayfaları, dosya iletişim kutularının başlıkları ve süzgeçleri, ileti kutuları ve söyledikleri | evet |
| pencerenin hata akışına yazdığı satırlar, `subedit-gui: …` | evet, programın adı hariç |
| `Correct Texts…` içindeki düzeltme desenlerinin adları ve ipuçlarındaki açıklamaları | evet, programla gelen desenler için; kendi deseninizi dosyasının yazdığı gibi görürsünüz |
| kodlama, satır sonu ve yazım denetimi dillerinin adları (`UTF-8`, `CRLF`, `en_US`) | asla: bunlar sözcük değil, addır |
| klavye kısayolları | asla: `Ctrl+S` kısayolu her dilde `Ctrl+S` biçimindedir |
| kataloğun çevirmediği bir ileti | İngilizce gösterilir |

Bu sürüm `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR`, `ru`, `sr`, `tr` ve `zh_CN` dillerini tam çevrilmiş olarak kurar — Katalanca, Çekçe, Almanca,
İspanyolca, Fince, Fransızca, Galiçyaca, İbranice, Macarca, Interlingue, İzlandaca, İtalyanca, Felemenkçe, Lehçe, Portekizce, Brezilya Portekizcesi, Rusça, Sırpça, Türkçe ve Çince (Basitleştirilmiş). Fransızca, dili bilen bir okuyucu tarafından gözden geçirilmiştir;
Katalanca, Çekçe, Almanca, İspanyolca, Fince, Galiçyaca, İbranice, Macarca, Interlingue, İzlandaca, İtalyanca, Felemenkçe, Lehçe, Portekizce, Brezilya Portekizcesi, Rusça, Sırpça, Türkçe ve Çince (Basitleştirilmiş) makine çevirisidir ve henüz dili bilen bir okuyucu tarafından
gözden geçirilmemiştir.
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
Fransızca bir masaüstünde menü çubuğu şöyle görünür
`&Fichier`, `É&dition`, `&Affichage`, `&Vidéo`, `&Outils`, `&Projets`, `Aid&e`. Bu kılavuzdaki ekran görüntüleri Türkçe olarak alınmıştır.
