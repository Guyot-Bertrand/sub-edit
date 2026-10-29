# subedit — kılavuz

<!-- machine-translated -->
> Bu sayfa makine çevirisidir ve henüz dili bilen bir okuyucu tarafından gözden geçirilmemiştir.

subedit altyazı dosyalarını okur, inceler ve zamanlamalarını yeniden ayarlar. Aynı çekirdeği,
dolayısıyla aynı kuralları ve aynı sözcükleri paylaşan iki programdan oluşur.

| Program | Ne için | Kılavuzu |
| :-------- | :-------------- | :--------- |
| `subedit-gui` | bir dosyayı açmak, tabloda görmek, düzenlemek, filmle karşılaştırarak izlemek | [pencere kılavuzu](subedit-gui/index.md) |
| `subedit-cli` | dosyaları toplu olarak, bir betikten, arayüz olmadan işlemek | [komut satırı kılavuzu](subedit-cli/index.md) |

İkisi de **dokuz biçimi** — SubRip, WebVTT, SubViewer 2,
Sub Station Alpha, Advanced SSA, MicroDVD, MPL2, TMPlayer, LRC — okur ve yazar;
biçimi uzantısından değil içeriğinden tanır. **Kodlama da
içerikten tanınır**: Latin-1, CP1252 veya UTF-16 kodlamalı bir dosya
açılır, hangi kodlamayla okunduğu bildirilir ve dosya kendi kodlamasıyla geri yazılır —
açılıp değişiklik yapılmadan kaydedilen bir dosya tek bir bayt bile değişmez.

## Nereden başlamalı

| Ne yapmak istiyorsunuz | Nereye gitmelisiniz |
| :----------------- | :------- |
| aracı kurmak | [Kurulum](subedit-cli/installation.md) — iki program için de geçerlidir |
| bir dosyayı açıp içeriğine bakmak | [Tablo](subedit-gui/table.md) |
| sabit bir miktar kaymış altyazıların zamanlamasını yeniden ayarlamak | [İşlemler](subedit-gui/operations.md#shift-positions) veya [`shift`](subedit-cli/shift.md) |
| başka bir kare hızı için yazılmış bir dosyanın zamanlamasını yeniden ayarlamak | [`framerate`](subedit-cli/framerate.md); zamanlama zaten doğruysa [`snap`](subedit-cli/snap.md) |
| bir dosyanın hangi kare hızına göre yazıldığını öğrenmek | [Kare ızgarası](subedit-gui/frame-grid.md) veya [`inspect`](subedit-cli/inspect.md) |
| işitme engelli notlarını kaldırmak | [İşlemler](subedit-gui/operations.md#remove-hearing-impaired-mentions) veya [`hearing-impaired`](subedit-cli/hearing-impaired.md) |
| notları, yaygın hataları, büyük/küçük harfi ve satır bölmeyi tek geçişte düzeltmek | [`Correct Texts…`](subedit-gui/correct-texts.md) veya [`correct`](subedit-cli/correct.md) |
| yazımı sistemin sözlükleriyle sözcük sözcük denetlemek | [`Check Spelling…`](subedit-gui/check-spelling.md) |
| okunamayacak kadar kısa altyazıları uzatmak | [`Adjust Durations…`](subedit-gui/operations.md#adjust-durations) veya [`adjust`](subedit-cli/adjust.md) |
| altyazıları sırasız olan bir dosyayı yeniden sıraya sokmak | [`sort`](subedit-cli/sort.md) |
| aynı metnin art arda gelen parçalarından tek bir altyazı yapmak | [`Join Repeated Subtitles`](subedit-gui/lines.md#yinelenen-altyazıları-birleştirme) veya [`join-repeats`](subedit-cli/join-repeats.md) |
| kötü bölünmüş, sırasız ve çakışan bir dosyayı tek seferde düzeltmek | [`repair`](subedit-cli/repair.md) |
| altyazılar arasındaki çakışmaları kaldırmak | [`Make Ranges Disjoint…`](subedit-gui/lines.md#aralıkları-ayrık-yapma) veya [`disjoint`](subedit-cli/disjoint.md) |
| bir filmin iki parçasını uç uca eklemek | [`Append File…`](subedit-gui/operations.md#append-file) veya [`append`](subedit-cli/append.md) |
| bir filmi iki parçaya bölmek | [`Split Project…`](subedit-gui/operations.md#split-project) veya [`split-file`](subedit-cli/split-file.md) |
| bir dosyanın tamamının büyük/küçük harfini, italiklerini veya diyalog çizgilerini değiştirmek | [İşlemler](subedit-gui/operations.md#italic) veya [`case`](subedit-cli/case.md), [`italics`](subedit-cli/italics.md), [`dialogue-dashes`](subedit-cli/dialogue-dashes.md) |
| bir sözcüğü bulup etiketleri bozmadan değiştirmek | [Arama ve değiştirme](subedit-gui/find-and-replace.md) veya [`replace`](subedit-cli/replace.md) |
| bir filmin konuşmasını altyazı olarak yazmak | [`Transcribe Audio…`](subedit-gui/video.md#sesi-yazıya-dökme) veya [`transcribe`](subedit-cli/transcribe.md) — [araçları kurduktan](subedit-cli/install-transcription.md) sonra |
| bir altyazı dosyasını bir filmin sesine göre yeniden zamanlamak | [`sync`](subedit-cli/sync.md) — [araçları kurduktan](subedit-cli/install-transcription.md) sonra |
| özgün metnin yanında çeviri yapmak | [Çeviri açma](subedit-gui/files.md#çeviri-açma) veya çeviriyi ana dosyaya göre yeniden zamanlamak için [`pair`](subedit-cli/pair.md) |
| yüz dosyayı bir seferde işlemek | [Dizin ağacını işleme](subedit-cli/lots.md), ardından [Çalıştırma](subedit-cli/invocation.md) |

**`framerate` ile `snap` arasındaki bir hata fark edilmez.**
Tereddüt ediyorsanız [`snap`](subedit-cli/snap.md#snap-framerate-değildir-ve-ikisini-karıştırmak-belli-olmaz)
sayfası ikisinin karşılaştırmasıyla açılır.

## Bu kılavuzun vaatleri

**Var olanı anlatır, planlananı asla.** Bundan sonra neyin geleceği ve
hangi sırayla geleceği [yol haritasında](../../../feuille-de-route.md) yer alır.

**Bu kılavuz İngilizce yazılmıştır ve çevrilir.** Çevirileri sayfa sayfa onu izler;
deponun `docs/i18n/` dizininde, her dil için bir ağaç halinde bulunur. Programlar
ortamlarının dilini konuşur; gösterdikleri adlar — alt komutlar,
seçenekler, menü etiketleri — burada programın İngilizce yazdığı biçimde aktarılır.

**Örnekler ve görüntüler üretilir, metin üretilmez.**
`console` blokları, komut gerçekten çalıştırılarak elde edilir; ekran
görüntüleri ise gerçek pencere kurulup fotoğraflanarak. Bu nedenle ikisi de
artık var olmayan bir aracı anlatamaz. Güvencesiz kalan şey, etraflarındaki
metindir; doğruluğu okuma ve gözden geçirmeye dayanır.

**Ekran görüntüleri iki paleti de gösterir**, `Light` ve `Dark` —
[tercihlerin](subedit-gui/preferences.md#tema) sunduklarını.

## İki arayüz aynı sözcükleri söyler

Birini kullanırken diğerinin kılavuzunu da okumanın bir nedeni: hata
iletileri, biçim adları, ızgara kararları ve geri alma
etiketleri **bir kez**, çekirdekte yazılır ve iki program da
bunları aktarır. Reddedilen bir dosya her iki tarafta da aynı cümleyi verir.

İki arayüzün farkı, her birinin neler yapabildiğindedir:

| | `subedit-gui` | `subedit-cli` |
| :--- | :------------ | :------------ |
| aynı anda birkaç dosya | evet — proje başına bir sekme | evet |
| ana dosyanın yanında bir çeviri | evet | evet — metin alt komutlarında `-t`, yeniden zamanlamak için de [`pair`](subedit-cli/pair.md) |
| bir dosya eklemek, bir projeyi bölmek | evet | evet — [`append`](subedit-cli/append.md), [`split-file`](subedit-cli/split-file.md) |
| geri alma | evet, bin öğe | uygulanmaz — hiçbir şey sorulmadan yerinde değiştirilmez |
| bir metni veya bir konumu elle düzenlemek | evet | hayır |
| satır eklemek, silmek, birleştirmek ve bölmek | evet | hayır |
| bulmak ve değiştirmek | evet | evet — [`replace`](subedit-cli/replace.md) |
| metinleri kesmek, kopyalamak ve yapıştırmak | evet | hayır |
| süreleri, italikleri, büyük/küçük harfi, çizgileri ayarlamak | evet | evet — [`adjust`](subedit-cli/adjust.md), [`italics`](subedit-cli/italics.md), [`case`](subedit-cli/case.md), [`dialogue-dashes`](subedit-cli/dialogue-dashes.md) |
| metinleri desenlerle düzeltmek | evet — `Correct Texts…` | evet — [`correct`](subedit-cli/correct.md) |
| yazımı sözcük sözcük denetlemek | evet — `Check Spelling…` | hayır |
| zamanlama yaparken filmi izlemek | evet | hayır |
| girdinin üzerine yazmak | `Save` | `--in-place` |
