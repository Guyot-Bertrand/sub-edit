# `hearing-impaired`

<!-- machine-translated -->
> Bu sayfa makine çevirisidir ve henüz dili bilen bir okuyucu tarafından gözden geçirilmemiştir.

```
subedit-cli hearing-impaired
                  (--output FILE | --output-dir DIR | --in-place)
                  [--dry-run]
                  [--recursive] [--parentheses]
                  [--document main|translation] [-t FILE] [--align-method position|number]
                  <file>...
```

Dosyanın tamamından, köşeli ayraç arasında anlatılan sesleri, yani **işitme engelli notlarını**
kaldırır. **Parantezler varsayılan olarak kalır**: fısıldanan bir
konuşma için de, anlatılan bir ses için de aynı sıklıkta kullanılırlar ve kaldırılmaları diyalogu silerdi. `--parentheses`
onları da kaldırır; seslerini parantez içinde anlatan dosyalar için.

Yapmadıkları: kare işaretleri arasındaki şarkı sözleri, iki noktadan önceki konuşmacı
adı, büyük harflerin yeniden düzenlenmesi, OCR hatalarının düzeltilmesi. Bu desenler,
pencerenin [`Correct Texts…`](../subedit-gui/correct-texts.md) sihirbazının desenleridir; komut
satırı bunu [`correct`](correct.md) adıyla sunar. Burada hiçbir şey ayarlanamaz: dönüşüm
kesindir, yapılandırılabilir değildir.

<!-- example: subedit-cli hearing-impaired --help -->
```console
$ subedit-cli hearing-impaired --help
Remove the sounds described between square brackets
Usage: subedit-cli hearing-impaired [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to clean

Options:
  -h,--help                   Print this help message and exit
  --parentheses               Also remove the sounds between parentheses, often whispered lines
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --document main|translation The document to change: the main one, or the translation given by -t
  -t,--translation-file FILE  Translation file to lay over the subtitle file, for a single input
  --align-method position|number
                              How the lines of the translation find their subtitles
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Bağımsız değişkenler ve seçenekler

| Seçenek | Zorunlu | Değerler | Varsayılan |
| :----- | :----- | :------ | :----- |
| `<file>...` | evet | bir ya da daha fazla yol | — |
| `--parentheses` | hayır | bir bayrak: parantez arasındaki notları da kaldırır | kapalı |
| `--recursive`, `-r` | hayır | bir bayrak | kapalı — bkz. [Dizin ağacını işleme](lots.md) |
| `--document` | hayır | `main` veya `translation` — bkz. [Bir çeviri](invocation.md#bir-çeviri) | `main` |
| `-t`, `--translation-file` | `--document translation` ile | bir çeviri dosyası, **tek bir** girdi için | — |
| `--align-method` | hayır, yalnızca `-t` ile | `position` veya `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **üçünden biri**, `--dry-run` dışında | bkz. [Çalıştırma](invocation.md#hedef) | — |
| `--dry-run` | hayır | bir bayrak: hesaplar ve söyler, hiçbir şey yazmaz — bkz. [Yazmadan önce görme](invocation.md#yazmadan-önce-görme) | kapalı |

Okunan dosyanın biçimi **korunur**: biçimi değiştirmek
[`convert`](convert.md) komutunun işidir.

**Varsayılan olarak ana metin; çeviri için `--document translation -t FILE`.** O durumda
değiştirilen ve yazılan çeviridir, yalnızca o — bkz. [Bir çeviri](invocation.md#bir-çeviri).

**Bir çeviriyi boşaltacak bir not, çeviriyi boş yazar ve altyazıyı korur**: altyazı
dosyada metinsiz kalır. Ana metin ise, aşağıda anlatıldığı gibi, tümüyle boşalan bir altyazıyı yitirir.

## Neyin kaldırıldığı, neyin kaldığı

Bir not, bir köşeli ayraçtır — `--parentheses` ile bir paranteztir — ve kapsadığı her şeydir; **satır
sonu dahil**: gerçek dosyalarda bir not çoğu zaman satır
sonuyla kesilir.

| Girdi | Çıktı |
| :----- | :----- |
| `Bonjour [il tousse] Marie` | `Bonjour Marie` |
| `Bonjour[il tousse]Marie` | `Bonjour Marie` |
| `[Bruit de pas] Bonjour` | `Bonjour` |
| `A [un] [deux] B` | `A B` |

Kaldırma, **çevresindekiler arasında tam olarak bir boşluk bırakır ve bir satırın
kenarında hiçbir şey bırakmaz**. Dosyanın boşluklarını temizleme işi değildir: metnin
zaten taşıdığı iki boşluk, iki boşluk olarak kalır.

**Not satır sonunu aşıyorsa, kalan satır sonudur** — iki
satır iki satır olarak kalır. Bu durum bir tuhaflık değildir: on beş gerçek
altyazı çalışmasında, yetim görünen her köşeli ayraç bu şekilde
kesilmiş bir nottu.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nReculez ! [Il\nhurle] Tout de suite !\n\n' > a.srt; subedit-cli --quiet hearing-impaired --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nReculez ! [Il\nhurle] Tout de suite !\n\n' > a.srt; subedit-cli --quiet hearing-impaired --output b.srt a.srt; cat b.srt
1
00:00:01,000 --> 00:00:03,000
Reculez !
Tout de suite !
```

**Not olmayanlar ve yerinden oynamayanlar:**

| Durum | Ne olur |
| :-- | :-------------- |
| yalnızca sayılardan oluşan içerik — `[1]`, `(12)`, `[ 156478 ]` | olduğu gibi, ayraçlar dahil korunur: bir gönderme hiçbir zaman bir sesi adlandırmamıştır |
| bir paranteze ve kapsadığına, `--parentheses` olmadan | olduğu gibi korunur: `Venez (chut) ici` değişmez |
| hiçbir şeyin kapatmadığı bir köşeli ayraç ya da parantez | olduğu gibi bırakılır |
| bir biçim etiketi — `<i>`, `</i>` | asla kaldırılmaz |

Rakamlar, bir harf ya da bir işaret **arasındaki** bir boşluk — `[1 2]`, `[1a]`,
`[-1]` — onu bir not yapar. Boş içerik de öyledir, `[]` ya da `( )`: rakam
yok, dolayısıyla gönderme değil; söylenecek bir şey yok, dolayısıyla korunacak bir şey de yok.

## Tümüyle kaybolanlar

**Kaldırmanın boşalttığı bir satır kaybolur** ve **tümüyle boşalan bir altyazı
dosyadan silinir**. Sonrasındaki altyazılar yeniden numaralanır.

Bir satırda yalnızca boşluklar, biçim etiketleri ya da tek bir diyalog çizgisi kaldıysa
satır boş sayılır — `<i>[Musique]</i>` ekranda bir
`<i></i>` bırakmaz.

**Etiketler dosyanın biçiminin etiketleridir.** Bir Advanced SSA içinde `{\i1}[Musique]{\i0}`,
bir MicroDVD içinde `{Y:i}[Musique]` ve bir MPL2 içinde `/[Musique]` de hiçbir şey
bırakmaz. Tersine, etiket yazmayan bir biçimdeki — TMPlayer, LRC — bir `<i>`
izleyicinin okuduğu metindir ve onu taşıyan satır boş değildir.

**Tek bir sese inen bir diyalog artık diyalog değildir** ve geriye kalan çizgi
kaldırılır. İki ses kaldığı sürece çizgiler durur.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\nVoir [1] la note.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\nVoir [1] la note.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle cleaned, 1 removed -> b.srt
1
00:00:04,000 --> 00:00:06,000
Attends Marie.

2
00:00:07,000 --> 00:00:09,000
Voir [1] la note.
```

## Hiç not içermeyen bir dosya yine de yazılır

Aynen kopyalanır, çıkış kodu `0` olur ve rapor kaldırılacak bir şey
olmadığını söyler.

Bu, yazan alt komutların kuralıdır: **verilen bir hedef,
yazılan bir hedeftir**. Burada bir istisna yapmak, bir betiği alt komutlardan hangisinin
bazen dosya ürettiğini bazen üretmediğini bilmeye zorlardı.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nRien à signaler.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nRien à signaler.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt
a.srt: no mention to remove -> b.srt
```

## Çıktı

**Standart çıktı** — boş: sonuç, yazılan dosyadır. **`--dry-run` ile önerilen değişikliklerdir**: metni değişecek her altyazı için numarası, önceki metin ve sonraki metin — bkz. [Yazmadan önce görme](invocation.md#yazmadan-önce-görme).

| Düzey | Hata çıktısına eklenenler |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles cleaned, M removed -> <destination>` ya da `<path>: no mention to remove -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — okunan dosyanın biçimi, kodlaması, işareti ve satır sonları, olduğu gibi geri konur |
| 3 | `<path>: N bytes read, M written`, ardından **her okuma tanılama iletisi** — bkz. [Çalıştırma](invocation.md#tanılama-iletilerini-okuma) |

`N` metni değişen altyazıları, `M` ise kaldırmanın tümüyle
boşalttığı ve bu yüzden dosyadan çıkan altyazıları sayar.

### JSON biçiminde

[`--format json`](invocation.md#bir-betiğin-okuyabileceği-çıktı) ile her dosyanın nesnesi `counts.cleaned` (yeniden yazılan metinler) ve `counts.removed` (kaldırılan altyazılar) değerlerini taşır; **`--dry-run` ile** `changes` anahtarı ayrıca, metin biçimindeki standart çıktı gibi, değişen altyazıları listeler. Tam tablo [Çalıştırma](invocation.md#her-alt-komutun-counts-içine-koyduğu) sayfasındadır.

## Çıkış kodları

Aracın kodları: her dosya yazıldıysa `0`, hiçbiri yazılmadıysa `2`,
yalnızca bazıları yazıldıysa `3`, kullanım hatasında `1`.

## Hatalar

Bu alt komutun kendine özgü bir hatası yoktur: ayrıştırılacak bir
değer almaz ve hiçbir metin onun bir dosyayı reddetmesine yol açamaz. Başarısız
olabilecek olan, diğerlerinde olduğu gibi okuma ya da yazmadır.

**Üç hedef seçeneğinden biri olmadan hiçbir şey yazılmaz** ve bunların hataları
yazan alt komutlar için ortaktır:
bkz. [Çalıştırma](invocation.md#hedef).
