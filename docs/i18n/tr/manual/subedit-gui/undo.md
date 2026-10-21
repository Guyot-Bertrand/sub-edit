# Geri alma ve yineleme

<!-- machine-translated -->
> Bu sayfa makine çevirisidir ve henüz dili bilen bir okuyucu tarafından gözden geçirilmemiştir.

Her işlem bir geçmişe girer ve geri alınabilir. Düzenlenen bir hücre **tek** kayıt sayılır:
yazılanlar tuş vuruşu başına kaydedilmez, yalnızca onay sayılır.

| Eylem | Kısayol | Nerede |
| :---- | :------ | :----- |
| Geri Al | `Ctrl+Z` | **Düzenle** menüsü, araç çubuğu |
| Yinele | `Ctrl+Shift+Z` | **Düzenle** menüsü, araç çubuğu |

## Eylem neyi geri alacağını söyler

Menü “Geri Al” demez, geri alınacak işlemin ardından **“Geri Al: bir metni düzenleme”** der. Araç çubuğu
düğmesi yalın sözcüğü korur — aksi halde genişliği son işlemi izler ve işaretçinin altında
oynardı —, ancak ipucu etiketin tamamını taşır.

| İşlem | Eylemin okuduğu |
| :---- | :-------------- |
| düzenlenen bir metin hücresi | `editing a text` |
| bir başlangıç, bir bitiş | `editing a start`, `editing an end` |
| bir süre | `editing a duration` |
| satır ekleme | `inserting` |
| satır silme | `removing` |
| bir kaydırma, `Shift Whole File onto Grid` dahil | `shifting` |
| bir dönüştürme | `transforming` |
| bir kare hızı dönüştürmesi | `converting the frame rate` |
| bir kare hızına hizalama | `aligning on the frame rate` |
| notları kaldırma | `removing hearing-impaired mentions` |
| italik yapma | `putting in italics` |
| italiği kaldırma | `taking italics out` |
| büyük/küçük harf değişikliği | `changing the case` |
| eklenen, kaldırılan çizgiler | `adding dialogue dashes`, `removing dialogue dashes` |
| satırların birleştirilmesi | `merging` |
| bir satırın bölünmesi | `splitting` |
| kesilen, yapıştırılan metinler | `cutting texts`, `pasting texts` |
| sürelerin ayarlanması | `adjusting durations` |
| bir çeviri açma | `opening a translation` |
| bir dosya ekleme | `appending a file` |
| projeyi bölme | `splitting the project` |
| `Correct Texts…` içinde kabul edilen ya da `Check Spelling…` ile yapılan bir düzeltme | `correcting texts` |
| bir değiştirme, tüm değiştirmeler | `replacing`, `replacing all` |

**Yirmi yedi etiket var ve pencerenin üretebildiği bunların hepsidir.** Çekirdek yirmi sekizincisini
adlandırır — `sorting` — ancak pencerenin hiçbir eylemi ona ulaşmaz: sıralama yalnızca katı bir sıra
ilkesi altında yapılır, oysa pencere belgelerini, düzeni onarmak yerine bildiren gevşek ilke altında açar.

**`Shift Whole File onto Grid` bir kaydırmadan ayırt edilmez** ve bu doğrudur: miktarı yazılmamış,
ölçülmüş bir kaydırmadır.

Bunlar komut satırındakilerle aynı sözcüklerdir: çekirdekte bir kez yazılırlar.

Geri alınacak bir şey olmadığında eylem yalın “Geri Al” der **ve devre dışıdır**: kısayolu hiçbir şey yapmaz.

## Değişiklik işareti

Dosya diskteki dosyadan farklı olduğu sürece pencere başlığı bir işaret taşır — bir yıldız ya da platformun
onun yerine kullandığı şey. **Sekme etiketi de taşır**, `premier.srt*`: başlık yalnızca baktığınız
projeyi anlatır, etiket bakmadıklarınızı anlatır.

**Her sekmenin kendi geçmişi vardır.** Geri alma yalnızca geçerli sekmenin projesinin aldığını geri alır —
tüm projeler üzerinde yapılan bir `Replace All` dahil: dokunduğu her projede bir kayıt bırakır.

**Başlangıç noktasına kadar geri alırsanız işaret kaybolur.** Bu bir görüntüleme ayrıntısı değildir:
değişiklik sayısı evet/hayır olarak kaydedilmez, sayılır; bu yüzden geri dönmek onu sıfıra
indirebilir ve işaret kalkar.

## Bilinmesi gereken iki şey

**Başvuru noktası son kayıttır**, hiç kayıt yoksa dosyanın açılışıdır. Kaydetmek onu taşır: işaret
kaybolur, ardından geri almak onu geri getirir.

Bir geri almadan sonra yapılan bir işlem **yinelenecekleri siler**: yinelemek, başlangıç durumu artık
var olmayan bir komutu yeniden oynatmak olurdu. Bu her düzenleyicinin davranışıdır ve bilerek böyledir.

**Bir hücre açıkken** `Undo` ve `Redo` — menüden ya da araç çubuğundan — diğer hareketler gibi önce
girdiyi onaylar; bkz. [Hücre düzenleme](editing.md). O an yazılan şey geçmişin son kaydı olur:
`Undo` önceki işlemi değil onu geri alar; `Redo` yinelenecek bir şey bulamaz, çünkü bir girdi
yinelenecekleri siler. Hiçbir şey yazılmamış açık bir hücre kayıt bırakmaz: `Undo` bu durumda önceki
işlemi geri alır.

Geçmiş bin kayıt tutar ve bunun ötesinde en eskileri unutur.
