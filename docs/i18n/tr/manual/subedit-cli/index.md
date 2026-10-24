# subedit-cli — kılavuz

<!-- machine-translated -->
> Bu sayfa makine çevirisidir ve henüz dili bilen bir okuyucu tarafından gözden geçirilmemiştir.

subedit’in komut satırı aracı: altyazı dosyalarını grafik arayüz olmadan,
toplu olarak veya bir betikten işlemek. Diğer program için ve nereden başlanacağını
öğrenmek için
[kılavuza](../index.md) bakın.

> **Güncel durum.** `subedit-cli` programının on yedi alt komutu vardır: inceleme,
> dönüştürme, kaydırma, dönüşüm, kare hızını yeniden zamanlama, bir kare ızgarasına oturtma,
> işitme engelli notlarını kaldırma, süreleri ayarlama, bir metni değiştirme,
> büyük/küçük harfi, italikleri ve diyalog çizgilerini değiştirme, altyazıları yeniden
> sıraya sokma, metinleri Gaupol’un desenleriyle düzeltme, dosyaları art arda ekleme,
> bir dosyayı ikiye bölme ve bir çeviriyi ana dosyasının konumlarına yazma. Bu kılavuz
> var olanı anlatır, planlananı asla; sonraki adımlar
> [yol haritasında](../../../../feuille-de-route.md) yer alır.

## Bölümler

| Bölüm | İçerik |
| :------ | :------ |
| [Kurulum](installation.md) | aracı derleme ve kurma |
| [Çalıştırma](invocation.md) | genel seçenekler, çıktılar, çıkış kodları |
| [Ayar dosyası](invocation.md#ayar-dosyası) | `adjust` ve `correct` alt komutlarının pencerenin ayar dosyasından okudukları, `--config`, `--no-config` |
| [Dizin ağacını işleme](lots.md) | birkaç dosya, bir dizinin tamamı, `--recursive` |
| [`inspect`](inspect.md) | bir dosyanın neyden oluştuğunu bildirme |
| [`convert`](convert.md) | bir dosyayı başka bir biçimde veya başka bir düzende yazma |
| [`shift`](shift.md) | her konumu aynı miktarda kaydırma |
| [`transform`](transform.md) | doğru olduğu bilinen iki noktadan yola çıkarak her konumu düzeltme |
| [`framerate`](framerate.md) | bir dosyayı bir kare hızından diğerine yeniden zamanlama |
| [`snap`](snap.md) | zaman damgalarını bir kare hızının karelerine geri oturtma |
| [`hearing-impaired`](hearing-impaired.md) | işitme engelli notlarını kaldırma |
| [`adjust`](adjust.md) | süreleri bir okuma hızına ve sınırlara göre ayarlama |
| [`replace`](replace.md) | bir metni etiketi bozmadan değiştirme |
| [`case`](case.md) | metinleri başlık, cümle, büyük veya küçük harf düzenine getirme |
| [`italics`](italics.md) | metinleri italik yapma veya italiklerini kaldırma |
| [`dialogue-dashes`](dialogue-dashes.md) | diyalog çizgilerini koyma veya kaldırma |
| [`sort`](sort.md) | altyazıları başlangıçlarının sırasına geri koyma |
| [`join-repeats`](join-repeats.md) | aynı metnin art arda gelen parçalarından tek bir altyazı yapma |
| [`disjoint`](disjoint.md) | çakışan altyazıları, hiçbir aralık artık bir diğeriyle çakışmayacak biçimde kesme |
| [`repair`](repair.md) | kötü bölünmüş, sırasız ve çakışan bir dosyada `join-repeats`, `sort` ve `disjoint` işlemlerini art arda çalıştırma |
| [`correct`](correct.md) | metinleri Gaupol’un desenleriyle düzeltme: notlar, sözcükleri birleştirme ve bölme, yaygın hatalar, büyük harf kullanımı, satır bölme |
| [`append`](append.md) | dosyaları ilkinin ardına, tek bir dosyada toplama |
| [`split-file`](split-file.md) | bir dosyayı ikiye kesme, kuyruğu başlangıca getirme |
| [`pair`](pair.md) | bir çeviriyi ana dosyasının konumlarına yazma ve satırlarının nasıl hizalandığını söyleme |
| [`transcribe`](transcribe.md) | bir videonun ya da ses dosyasının konuşmasını altyazı olarak yazma |
| [Yazıya dökme araçlarını kurma](install-transcription.md) | programın birlikte getirmediği motor, model ve `ffmpeg` |
| [`config`](config.md) | pencereyle paylaşılan işlem ayarlarını listeleme, gösterme ve değiştirme |
