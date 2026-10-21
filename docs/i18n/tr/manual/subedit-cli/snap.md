# `snap`

<!-- machine-translated -->
> Bu sayfa makine çevirisidir ve henüz dili bilen bir okuyucu tarafından gözden geçirilmemiştir.

```
subedit-cli snap --rate <rate>
                 (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 <file>...
```

**Tüm zaman damgalarını** — başlangıç ve bitiş — verilen kare hızının
en yakın karesine oturtur. Hiçbir şey yeniden zamanlanmaz: her konum
en fazla yarım kare kayar.

## `snap`, `framerate` değildir ve ikisini karıştırmak belli olmaz

İki komutun biçimi aynıdır — dosyalar, bir hedef, bir kare hızı — ve zıt şeyler yaparlar.

| | [`framerate`](framerate.md) | `snap` |
| :--- | :-------------------------- | :----- |
| neyi hesaplar | `t' = t x input rate / output rate` | `--rate` ızgarasındaki en yakın kare |
| ne kadar kaydırır | zamanla orantılı olarak — bir uzun metrajlı filmde **birkaç saniye** | en fazla yarım kare, **her yerde** |
| hangi dosya için | **zamanlaması yanlış** olan bir dosya | zamanlaması **doğru**, ızgarası yanlış olan bir dosya |

**`snap` komutunun kullanım durumu**, açıkça: saniyede 25 kareli bir film,
24'lük bir ızgarada yazılmış bir altyazı dosyası, satırlar birkaç milisaniye
farkla zaten aşağı yukarı yerinde. Yeniden zamanlanacak bir şey yoktur
— zaman damgalarını doğru karelere geri koymak yeterlidir.

**`framerate` komutunun kullanım durumu** ise öbürüdür: 25 hızında hazırlanmış bir
kopya için zamanlanmış, 23,976 hızındaki bir kopyada oynatılmak istenen ve zamanlaması
film sonuna doğru birkaç saniye kayan bir dosya.

Birini ötekiyle karıştırmak **hata vermez**: ikisi de
dosyayı kabul eder ve bir sonuç yazar. Aralarındaki farkı
söyleyen anlatım satırıdır — `snap` konumları ne kadar kaydırdığını her zaman bildirir.

<!-- example: subedit-cli snap --help -->
```console
$ subedit-cli snap --help
Move every position onto the nearest frame of a frame rate (see framerate)
Usage: subedit-cli snap [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to align

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --rate TEXT REQUIRED        Frame rate to align on: 25, 23.976
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Bağımsız değişkenler ve seçenekler

| Seçenek | Zorunlu | Değerler | Varsayılan |
| :----- | :----- | :------ | :----- |
| `<file>...` | evet | bir veya daha fazla yol | — |
| `--recursive`, `-r` | hayır | bir bayrak | kapalı — bkz. [Dizin ağacını işleme](lots.md) |
| `--rate` | **evet** | saniyedeki kare olarak bir kare hızı, [`framerate`](framerate.md#kare-hızı-yazma) komutundaki gibi yazılır | — |
| `--output` / `--output-dir` / `--in-place` | `--dry-run` dışında **üçünden biri** | bkz. [Çalıştırma](invocation.md#hedef) | — |
| `--dry-run` | hayır | bir bayrak: hesaplar ve bildirir, hiçbir şey yazmaz — bkz. [Yazmadan önce görme](invocation.md#yazmadan-önce-görme) | kapalı |

`--rate` seçeneği **geçerli her kare hızını** kabul eder, yalnızca sekiz
standart hızı değil: `framerate` komutunda olduğu gibi seçim kullanıcınındır. Sıfır ya da negatif
bir kare hızı reddedilir.

Okunan dosyanın biçimi **korunur**: biçimi değiştirmek
[`convert`](convert.md) komutunun işidir.

## Çıktı

**Standart çıktı** — hiçbir şey: sonuç, yazılan dosyadır. `--dry-run` ile de hiçbir şey yoktur: dosya yoktur ve anlatım satırı bunu söyler — bkz. [Yazmadan önce görme](invocation.md#yazmadan-önce-görme).

| Düzey | Hata çıktısına eklenenler |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles aligned on 25 fps, M positions moved, by at most 20 ms -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — okunan dosyanın biçimi, kodlaması, işareti ve satır sonları, olduğu gibi geri konarak |
| 3 | `<path>: N bytes read, M written`, ardından **her okuma tanılama iletisi** — bkz. [Çalıştırma](invocation.md#tanılama-iletilerini-okuma) |

**Satırın asıl amacı iki sayıdır.** Saniyede 25 karede yarım kare
yirmi milisaniyedir ve oraya daha büyük bir değer yazılamaz. `framerate` isteyip
`snap` yazan kişi, saniyeler beklerken “by at most 20 ms” ifadesini okur ve
hemen anlar.

## Bir örnek

`00:00:01,010` konumundaki bir satır, saniyede 25 kareye oturtulur — yani kırk milisaniyenin
bir katına:

<!-- example: printf '1\n00:00:01,010 --> 00:00:03,020\nUn.\n\n' > a.srt; subedit-cli snap --rate 25 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,010 --> 00:00:03,020\nUn.\n\n' > a.srt; subedit-cli snap --rate 25 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle aligned on 25 fps, 2 positions moved, by at most 20 ms -> b.srt
1
00:00:01,000 --> 00:00:03,040
Un.
```

## İki özellik

**İki kez oturtmak, ikincisinde hiçbir şeyi değiştirmez.** Zaten bir
karede olan bir konum orada kalır: aynı işlem, aynı sonuca uygulanmıştır.

**Altyazıların sırası korunur.** En yakın kareye yuvarlama
monotondur; bu yüzden en az bir kare arayla duran iki başlangıç öyle kalır. Daha
yakın olanlar birleşebilir — bu,
[`inspect`](inspect.md#anomalies-ne-bildirir) komutunun adlandırmayı bildiği bir anomalidir, bir
ters çevrilme değil.

### JSON biçiminde

[`--format json`](invocation.md#bir-betiğin-okuyabileceği-çıktı) ile her dosyanın nesnesi `counts.subtitles`, `counts.moved` (kaydırılan konumlar) ve `counts.furthest_ms` (en büyük kaydırma, milisaniye olarak) alanlarını taşır. Tablonun tamamı [Çalıştırma](invocation.md#her-alt-komutun-counts-içine-koyduğu) sayfasındadır.

## Çıkış kodları

Aracın kodlarıdır: her dosya yazıldıysa `0`, hiçbiri yazılmadıysa `2`, yalnızca
bazıları yazıldıysa `3`, kullanım hatasında `1`.

## Hatalar

Bir kare hızının hataları, [`framerate`](framerate.md#hatalar) komutununkilerle kelimesi kelimesine aynıdır
— ikisi de aynı değeri aynı yoldan okur:

| Neyin tetiklediği | İleti |
| :------------------ | :------ |
| sıfır ya da negatif kare hızı | `"…" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed` |
| ondalık virgül | `"…" is not a frame rate: use a decimal point and not a comma, in every language` |
| sayı olmayan metin | `"…" is not a frame rate: expected frames per second, like 25 or 23.976` |
| aşırı büyük kare hızı | `"…" is not a frame rate: no video runs at that many frames per second` |

**Üç hedef seçeneğinden biri olmadan hiçbir şey yazılmaz** ve bunların hataları
yazan alt komutlar için ortaktır:
bkz. [Çalıştırma](invocation.md#hedef).
