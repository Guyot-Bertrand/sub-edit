# Videoya göre zamanlama

<!-- machine-translated -->
> Bu sayfa makine çevirisidir ve henüz dili bilen bir okuyucu tarafından gözden geçirilmemiştir.

*Filmin açık ve oynatıcının bilindiği varsayılır: bkz. [Oynatıcı](player.md).*

Filmi izlersiniz, bir satırın başlaması gereken karede durursunuz ve **bu konum altyazının
başlangıcı olur**. **Video** menüsünde, Gaupol'un onlara verdiği adlarla beş hareket. Zaten var
olana dayanırlar: bir konumu ayarlamak **değeri hücreye yazmakla aynı komuttur**, eklemek ise
**`Edit ▸ Insert Subtitles…` ile aynıdır**.

| Komut | Kısayol | Ne yapar |
| :---- | :------ | :------- |
| Video ▸ Başlangıcı Video Konumundan Ayarla | `Alt+U` | seçili altyazının **başlangıcını** filmin konumuna ayarlar |
| Video ▸ Bitişi Video Konumundan Ayarla | `Alt+K` | **bitişini** filmin konumuna ayarlar |
| Video ▸ Video Konumuna Altyazı Ekle | `Alt+J` | konumda **başlayan** bir altyazı ekler ve onu seçer |
| Video ▸ Video Konumundan Öncekini Seç | `Alt+Up` | konumdan **önce başlayan** son altyazıyı seçer |
| Video ▸ Video Konumundan Sonrakini Seç | `Alt+Down` | konumdan **sonra başlayan** ilk altyazıyı seçer |

**Hepsi, oynatıcının hareketleri gibi, video açık değilken devre dışıdır.** `Set Start…` ve
`Set End…` ayrıca hiçbir şey seçili olmadığı sürece devre dışıdır: seçimin **ilk satırı** üzerinde
etki ederler.

**Kısayollar Gaupol'unkilerdir, `Alt` ile**: orada `U`, `K` ve `J` — pencerenin, düzenlenen
hücreden önce yakalayacağı yalın harfler; bu yüzden bir metne `k` yazmak bir bitiş ayarlardı.
Gaupol'da `Select Previous…` için `Ctrl+Y`, `Select Next…` için `Ctrl+U` kullanılır; `Ctrl+Y` bazı
sistemlerde bir *yineleme* kısayoludur ve `Alt` ile oklar, “önce” ile “sonra” kavramlarını iki harften
daha iyi anlatır.
Düzenlenen bir hücre, `Set Start…`, `Set End…` ya da `Insert Subtitle…` öncesinde, her
değişiklikten önce olduğu gibi **onaylanır**.

## Bir ucu ayarlama

- **Hareket başına bir geçmiş kaydı**: `Edit ▸ Undo` önceki konumu geri verir, başka bir şey değil.
- **Hücreyle aynı kurallar.** Altyazının bitişinden sonra ayarlanan bir başlangıç ya da başlangıcından
  önce ayarlanan bir bitiş **reddedilmez**: ayarlanır ve tablo altyazıyı, değeri elle yazılmış olanı
  işaretlediği gibi işaretler — bkz. [Anomaliler](table.md#anomaliler). Hiçbir şey sessizce düzeltilmez.
- **Zaten ucun konumu olan bir konum hiçbir şey yapmaz** ve geçmişe hiçbir şey eklemez.
- **Konum her zaman bir karenin başlangıcıdır**: oynatıcı hiçbir zaman bir karenin içinden bir konum
  bildirmez. Bir ucu ayarlamak, sonra ona geri dönmek — örneğin `Seek Selection Start` ile —
  **aynı kareyi gösterir**.

## Konumda ekleme

Yeni altyazı **konumda başlar** ve **üç saniye** sürer, ya da sonrakinin başlangıcı daha önceyse
onun başlangıcına kadar. Metni boştur. Dosyanın sırasında yerini alır: o konumda ya da ondan önce
başlayanların hepsinden sonra. Ekleme **tek bir geçmiş kaydıdır** ve eklenen altyazı seçilidir:
hemen yazabilirsiniz.

## Konumdan seçme

İkisi de dosyanın sırasını izler, geçerli seçimi değil. O yönde **komşu yoksa** seçim, `Select Next…`
için dosyanın son altyazısına, `Select Previous…` için ilkine gider — Gaupol'un davranışı. Bu iki
hareket filmi oynatmaz.

## Kare kare

**Bir kare**, ya da N kare — aşağıya bakın —, ileri ve geri gitmek, bir ucu **tam kareye** ayarlamak
için. İki hareket ve oynatma çubuğunda, oynat düğmesinin iki yanında iki düğme:

| Komut | Kısayol | Ne yapar |
| :---- | :------ | :------- |
| Video ▸ Geri Adım | `Alt+Left` | filmi kare adımı kadar geri alır |
| Video ▸ İleri Adım | `Alt+Right` | filmi kare adımı kadar ileri alır |

- **Adım bir kare sayısıdır, en az bir**: [tercihlerin](preferences.md#oynatıcı) `Frame step`
  ayarıdır. N karelik bir adım, saniyede 25 karede de 23,976 karede de **N kare ilerideki** kareyi
  gösterir; **asla milisaniye cinsinden ifade edilmez**.
- **Film sınırlarında durur**: ilk kareden geri adım atmak onu orada bırakır, son kareden ileri adım
  atmak da. Oynatma bir adımdan sonra durdurulur.
- **Çubuğun düğmeleri** — oynat/duraklat düğmesinin iki yanında iki simge — **menüyle aynı eylemleri
  tetikler**: aynı adım, menünün öğeleri gibi video yokken devre dışı ve kısayolu veren aynı ipucu.
  **Bir düğmede ya da tuşta kalan bir parmak adımları zincirler.**
- **Aynı anda tek adım.** Basılı tutulan bir tuş, bir adımın yapılmasından daha hızlı tekrar eder:
  ilk tekrar hemen gider, adım yapılırken gelenler **birikmez** — tek bir yeri, sonuncununkini
  paylaşırlar ve o sonra gider. Tuşu bırakmak bu nedenle görüntüyü bulunduğunuz yerde durdurur ve
  filmin ivmesiyle sürmesine izin vermez.

## Bir ucu bir kare kaydırma

Bir ucu klavyeyle ayarlamak için: **seçili altyazının başlangıcı ya da bitişi**, bir adım, daha
erkene ya da daha geriye.

| Komut | Kısayol | Ne yapar |
| :---- | :------ | :------- |
| Video ▸ Başlangıcı Öne Al | `Alt+Q` | altyazı **bir adım daha erken başlar** |
| Video ▸ Başlangıcı Geriye Al | `Alt+W` | **bir adım daha geç başlar** |
| Video ▸ Bitişi Öne Al | `Alt+Shift+Q` | **bir adım daha erken biter** |
| Video ▸ Bitişi Geriye Al | `Alt+Shift+W` | **bir adım daha geç biter** |

`Q` ve `W`, yukarıda verilen nedenle `Alt` ile, Gaupol'da *daha erken* ve *daha geç* içindir;
bitiş `Alt+Shift` alır, çünkü Gaupol'un `E` tuşu burada `Edit` menüsünün harfidir.

- **Kaydırma başına bir geçmiş kaydı**, geri alınabilir. Komşu altyazı ve çakışma **yazarken olduğu
  gibi** bildirilir: uç ayarlanır ve tablo artık tutmayanı işaretler — bkz.
  [Anomaliler](table.md#anomaliler).
- **Oynatıcıyla aynı adım**, yani varsayılan olarak bir kare. **Bir karenin değeri** şu sırayla
  okunur: **videonun bildirdiği kare hızından**, **belge karelerle sayıyorsa** onunkinden
  (MicroDVD), sonra **konumların çizdiği ızgaradan**. **Üçünden hiçbiri yoksa hareket reddeder ve
  bunu söyler**: gelişigüzel bir kare hızı seçmek her ucu bir kare olmayan bir miktarla kaydırırdı.
- **Video gerektirmez**, yalnızca bir seçim gerektirir: filmi olmayan, karelerle sayılan bir dosyada
  da kullanılır. Bir film açıkken **oynatma yeni konuma geçer**, ucun az önce aldığı kareyi göstermek
  için.
- Bir uç başlangıç noktasından öteye gitmez: bir kareden az olan bir konumdan bir adım geri gitmek
  onu sıfıra koyar.
