# `framerate`

<!-- machine-translated -->
> Bu sayfa makine çevirisidir ve henüz dili bilen bir okuyucu tarafından gözden geçirilmemiştir.

> **Bu, [`snap`](snap.md) değildir.** Bu komut **dosyayı
> ölçekler**: zamanlama yanlışsa ve filmin sonuna doğru birkaç
> saniye kayıyorsa kullanılır. `snap`, zaman damgalarını hiçbir şeyi yeniden
> zamanlamadan en yakın ızgaraya geri koyar ve zamanlama zaten
> doğruysa kullanılır. İkisinin biçimi aynıdır, bir ya da iki kare hızı; birbirine karıştırmak
> hata vermez.

```
subedit-cli framerate --from <fps> --to <fps>
                      (--output FILE | --output-dir DIR | --in-place)
                      [--dry-run]
                      [--recursive]
                      <file>...
```

Bir kare hızına göre zamanlanmış bir dosyayı başka bir kare hızına göre yeniden zamanlar. Bir altyazı dosyası,
eldeki filmin kopyasından farklı bir kopya için alındığında gereken işlemdir —
örneğin bir NTSC aktarımına karşı bir PAL DVD.

Okunan dosyanın biçimi **korunur**: biçimi değiştirmek
[`convert`](convert.md) komutunun işidir.

<!-- example: subedit-cli framerate --help -->
```console
$ subedit-cli framerate --help
Re-time a file mastered at one frame rate for another
Usage: subedit-cli framerate [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to re-time

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --from TEXT REQUIRED        Frame rate the file is timed at: 25, 23.976
  --to TEXT REQUIRED          Frame rate to time it for: 24, 29.97
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Bağımsız değişkenler ve seçenekler

| Seçenek | Zorunlu | Değerler | Varsayılan |
| :----- | :----- | :------ | :----- |
| `<file>...` | evet | bir ya da daha fazla yol | — |
| `--recursive`, `-r` | hayır | bir bayrak | kapalı — bkz. [Dizin ağacını işleme](lots.md) |
| `--from` | **evet** | bir kare hızı, aşağıya bakın | — |
| `--to` | **evet** | bir kare hızı, aşağıya bakın | — |
| `--output` / `--output-dir` / `--in-place` | **üçünden biri**, `--dry-run` dışında | bkz. [Çalıştırma](invocation.md#hedef) | — |
| `--dry-run` | hayır | bir bayrak: hesaplar ve söyler, hiçbir şey yazmaz — bkz. [Yazmadan önce görme](invocation.md#yazmadan-önce-görme) | kapalı |

`--from`, dosyanın **zamanlandığı** kare hızıdır, `--to` ise
istenen kare hızı. Bunları ters yazmak dosyayı
öbür yöne kaydırır — uzun metrajlı bir filmde 25 ile
23,976 arasında yaklaşık yedi saniye — ve film izlenene kadar hiçbir şey bunu bildirmez.

## Kare hızı yazma

Saniyedeki kare olarak, ondalıklar dahil, ayırıcı olarak nokta ile:

```
25      24      23.976      29.97      60
```

**Sıfır ya da negatif bir kare hızı reddedilir**: bir kare hızı değildir.

### Üç NTSC kare hızı tam olarak yazılamaz

`23.976`, `29.97` ve `59.94` birer **etikettir**. Adını verdikleri kare hızları
`24000/1001`, `30000/1001` ve `60000/1001` değerleridir; hiçbirinin sonlu bir ondalık
yazımı yoktur. Araç bunları böyle tanır:

| Yazılan | Adını verdiği |
| :------------- | :------------------ |
| `23.976` | `24000/1001` — NTSC videoya aktarılan sinema |
| `24` | `24` — sinema |
| `25` | `25` — PAL ve SECAM video |
| `29.97` | `30000/1001` — NTSC video |
| `30`, `50`, `60` | kendileri |
| `59.94` | `60000/1001` |

Tanıma **harflere değil, değere** bakar: `23.9760`,
`23.976` ile aynı kare hızını adlandırır.

Başka her ondalık sayı olduğu gibi alınır — `23.9` kimsenin
standardı değildir, ondan bir anlam çıkarılamaz. Yukarıdaki tabloyu
gerekli kılan da budur: o olmasa `23.976`, `23976/1000` olurdu ve komut satırı
aynı sözler için pencereden başka bir şey söylerdi.

## Konumlar ne olur

```
t′ = round( t × (from / to) )
```

23,976 hızında hazırlanmış ve saniyede 25 karede oynatılan bir film daha hızlı akar,
dolayısıyla altyazıları daha erken gelir; tersi yönde ise daha
geç gelir.

**Çarpan tam bir kesirdir ve yuvarlama yalnızca bir kez yapılır.** Eşdeğer
görünen yol — konumu girdi hızında bir kare numarasına çevirmek, sonra çıktı
hızında yeniden milisaniyeye — eşdeğer değildir: her konumu girdi ızgarasında nicemler ve
yarım kareye kadar, ölçülen 21 ms kadar yanılır. Bkz.
[ADR 0013](../../../../adr/0013-mise-a-l-echelle-exacte-des-positions.md).

Uygulamada, 25 fps hızından 23,976 fps hızına geçerken çarpan `1001/960` değeridir:

| Okunan konum | Yazılan | Karelerden geçmek ne verirdi |
| -----------: | -----: | -----------------------------------------: |
| `1 010 ms` | `1 053 ms` | `1 043 ms` |
| `3 600 017 ms` | `3 753 768 ms` | `3 753 750 ms` |

<!-- example: printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 25 --to 23.976 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 25 --to 23.976 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle retimed from 25 to 24000/1001 fps -> b.srt
1
00:00:01,053 --> 00:00:02,106
Un.
```

## Çıktı

**Standart çıktı** — boş: sonuç, yazılan dosyadır. `--dry-run` ile de boş: dosya yoktur ve anlatım satırı bunu söyler — bkz. [Yazmadan önce görme](invocation.md#yazmadan-önce-görme).

| Düzey | Hata çıktısına eklenenler |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles retimed from 25 to 24000/1001 fps -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — okunan dosyanın biçimi, kodlaması, işareti ve satır sonları, olduğu gibi geri konur |
| 3 | `<path>: N bytes read, M written`, ardından **her okuma tanılama iletisi** — bkz. [Çalıştırma](invocation.md#tanılama-iletilerini-okuma) |

**Anlatım, yazılan etiketi değil, tam kare hızını adlandırır.** O satırda
`23.976` yazmak, gerçekleşmemiş bir dönüştürmeyi bildirmek olurdu; iki
etiketten hangisinin kullanıldığının görüldüğü tek yer orasıdır. Bir ondalık sayının tam olarak
yazabildiği kare hızı orada ondalık olarak, diğerleri kesir olarak gösterilir.

### JSON biçiminde

[`--format json`](invocation.md#bir-betiğin-okuyabileceği-çıktı) ile her dosyanın nesnesi `counts.subtitles` (dosyanın tüm altyazıları) değerini taşır. Tam tablo [Çalıştırma](invocation.md#her-alt-komutun-counts-içine-koyduğu) sayfasındadır.

## Çıkış kodları

Aracın kodları: her dosya yazıldıysa `0`, hiçbiri yazılmadıysa `2`,
yalnızca bazıları yazıldıysa `3`, kullanım hatasında `1`.

Hiçbir ret bir dosyanın içeriğine bağlı değildir: iki kare hızı her zaman bir
çarpan tanımlar. Yazan alt komutlar arasında, tüm retleri ilk okumadan önce
gelen tek alt komut budur.

<!-- example: printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 0 --to 25 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 0 --to 25 --output b.srt a.srt; echo "code=$?"
"0" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed
code=1
```

## Hatalar

| Neyin tetiklediği | İleti |
| :------------------ | :------ |
| sıfır ya da negatif kare hızı | `"…" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed` |
| ondalık virgül | `"…" is not a frame rate: use a decimal point and not a comma, in every language` |
| sayı olmayan metin | `"…" is not a frame rate: expected frames per second, like 25 or 23.976` |
| aşırı büyük kare hızı | `"…" is not a frame rate: no video runs at that many frames per second` |

**Üç hedef seçeneğinden biri olmadan hiçbir şey yazılmaz** ve bunların
hataları yazan alt komutlar için ortaktır:
bkz. [Çalıştırma](invocation.md#hedef).
