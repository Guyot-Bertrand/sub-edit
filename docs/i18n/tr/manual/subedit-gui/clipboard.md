# Metinleri kesme, kopyalama ve yapıştırma

<!-- machine-translated -->
> Bu sayfa makine çevirisidir ve henüz dili bilen bir okuyucu tarafından gözden geçirilmemiştir.

`Edit` menüsünün üç öğesi; `Undo` ve `Redo` öğelerinin altında, `Find and Replace…`
öğesinin üstündedir.

| Öğe | Kısayol | Ne yapar |
| :-- | :------ | :------- |
| Metinleri Kes | `Ctrl+X` | seçimin metinlerini kopyalar, sonra boşaltır |
| Metinleri Kopyala | `Ctrl+C` | seçimin metinlerini kopyalar |
| Metinleri Yapıştır | `Ctrl+V` | kopyalanan metinleri seçili ilk satırdan başlayarak yazar |

**Taşınan yalnızca metinlerdir**, başka hiçbir şey değil: ne başlangıç, ne bitiş, ne de bütün bir
satır. Yapıştırma bu yüzden hiçbir konumu oynatmaz — bir metni alan satırın zamanlaması kendisinin
kalır. Gaupol da böyle yapar.

## Neye etki ettikleri

**Bir çeviri varsa, geçerli sütunun metnine**: geçerli hücre `Translation` sütunundaysa çeviriye,
başka yerdeyse ana metne — bkz. [Tablo](table.md#metin-işlemlerin-hedefi). `Cut Texts` bu
nedenle çeviriyi boşaltır ve ana metni bırakır, `Paste Texts` ise bulunduğunuz sütuna yazar.
**Pano hiçbir belge taşımaz**: ana metnin bir hücresini kopyalayıp çeviriye yapıştırabilir,
ters yönde de yapabilirsiniz.

**Seçime etki ederler, dosyanın tamamına asla.** Üç öğe, hiçbir şey seçili olmadığı sürece devre dışıdır.
Pencerenin başka yerlerinde “hiçbir şey seçili değil” “dosyanın tamamı” demektir; burada yanlışlıkla
basılan bir `Ctrl+X` belgenin bütün metinlerini boşaltırdı ve bir yapıştırmanın başlayacağı
satır olmazdı.

Bir hücre açıkken `Ctrl+X`, `Ctrl+C` ve `Ctrl+V` girdi alanına aittir: tıpkı `Del` tuşunun orada bir
altyazıyı değil bir karakteri silmesi gibi, karakterleri keser, kopyalar ve yapıştırır. Menüden
başlatıldığında öğeler önce girdiyi onaylar: `Copy Texts` bu durumda metni yeni yazıldığı haliyle
kopyalar. Bkz. [Hücre düzenleme](editing.md).

## Kopyalama

`Copy Texts`, seçili her satırın metnini, ilkinden sonuncusuna kadar kopyalar. **Kesintili bir seçim
biçimini korur**: seçimin dışında kalan bir satır bir boşluk olur ve yapıştırma onun üzerine yazmaz.

**Metni boş olan seçili bir satır da boşluk olur**: yapıştırıldığında hedef satırı olduğu gibi bırakır.
Tek bir boş metni kopyalamak bu nedenle hiçbir şey yapıştırmaz.

Sistem panosu bu metinleri **düz metin olarak, bir boş satırla ayrılmış** alır — bir boşluk orada boş
bir metin olarak yazılır ve bir boş metnin boşluktan başka bir şey olamamasının nedeni budur:
yapıştırma, kopya bu pencereden de gelse başka bir programdan da gelse aynı sonucu verir. Burada
kopyaladığınız şey bu nedenle herhangi bir programa yapıştırılabilir, başka yerde kopyaladığınız
şey de buraya yapıştırılabilir.

Kopyalama belgeyi değiştirmez ve geçmişe girmez.

## Kesme

`Cut Texts` kopyalar, sonra seçili satırların **metinlerini boşaltır**. Satırlar konumlarıyla
birlikte kalır: kesmek silmek değildir — satırları silmek için [`Remove Subtitles`](lines.md#silme)
vardır.

Zaten hepsi boş olan metinleri kesmek hiçbir şey yapmaz ve geçmişe girmez.

## Yapıştırma

`Paste Texts`, panonun metinlerini **seçili ilk satırdan başlayarak**, metin başına bir satır olmak
üzere aşağı doğru yazar.

| Panonun içeriği | Ne olur |
| :-------------- | :------ |
| bir metin | satırınkinin yerine geçer |
| bir boşluk ya da kopyalanmış boş bir metin | satır metnini korur |
| kalan satırlardan daha çok metin | bunları almak için sonuna satır eklenir |

Eklenen satırlar, sonuncudan sonra eklenen satırlar gibi doğar: her biri üç saniye, art arda.
**Pencere bunu söyler**, bir bilgi kutusunda:

```text
inserted 2 subtitles to fit the clipboard
```

Yapıştırmadan sonra **yazılan satırlar seçilidir**.

Hiçbir şeyi değiştirmeyen bir yapıştırma — aynı satırlarda aynı metinler — geçmişe girmez.

### Başka bir biçimin etiketleri çevrilir

Bu pencerede yapılan bir kopya **belgesinin biçimini hatırlar** ve bu bellek başka bir dosyanın
açılmasından sonra da sürer. Başka biçimdeki bir belgeye yapıştırmak bu nedenle, `Save As…` gibi,
etiketleri çevirir: bir Advanced SSA belgesinden kopyalanan `{\i1}Bonjour{\i0}`, bir SubRip
belgesine `<i>Bonjour</i>` olarak yapıştırılır.

**Hedef biçimin yazamadığı şey kaybolur ve bu söylenir**, `Save As…` ile aynı sözcüklerle:

```text
pasting SubRip texts into LRC: 1 tag dropped
```

`Save As…` işleminden farklı olarak önceden hiçbir şey sorulmaz: yapıştırma `Ctrl+Z` ile geri alınır
ve bu, metinleri eski haline getirir.

**`subedit` dışında kopyalanan bir metnin biçimi yoktur** ve olduğu gibi yapıştırılır: hiçbir yerden
çeviri yapılamaz. Bu pencereden yapılmış olup sistem panosunun sonradan değiştirdiği bir kopya için
de durum aynıdır — sayılan, sistem panosunun içeriğidir.

İki ileti birden gerektiğinde tek bir kutuya sığarlar, bir noktalı virgülle ayrılmış olarak.

## Geri alma eylemi bu konuda ne söyler

Her hareket, yazdığı ya da eklediği satır sayısı ne olursa olsun, geçmişin **tek** kaydıdır.

| İşlem | `Undo` neyi okur |
| :---- | :---------------- |
| kesme | `Undo: cutting texts` |
| yapıştırma | `Undo: pasting texts` |

Bkz. [Geri alma ve yineleme](undo.md).
