# `transform`

<!-- machine-translated -->
> Bu sayfa makine çevirisidir ve henüz dili bilen bir okuyucu tarafından gözden geçirilmemiştir.

```
subedit-cli transform --first <index>=<time> --last <index>=<time>
                      (--output FILE | --output-dir DIR | --in-place)
                      [--dry-run]
                      [--recursive]
                      <file>...
```

Doğru değeri bilinen iki noktadan yola çıkarak dosyanın **tüm** konumlarını düzeltir.
Kayan bir altyazılamanın gerektirdiği işlemdir
— başta doğru, sonda yanlış — ve her şeyi aynı miktarda kaydıran [`shift`](shift.md)
bu durumda hiçbir şey yapamaz.

Okunan dosyanın biçimi **korunur**: biçimi değiştirmek
[`convert`](convert.md) komutunun işidir.

<!-- example: subedit-cli transform --help -->
```console
$ subedit-cli transform --help
Correct every position from two points known to be right
Usage: subedit-cli transform [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to transform

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --first TEXT REQUIRED       Earlier reference, as <index>=<time>: 1=00:00:01.000
  --last TEXT REQUIRED        Later reference, as <index>=<time>: 3=00:00:10.000
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
| `--first` | **evet** | bir referans, `<index>=<time>` | — |
| `--last` | **evet** | bir referans, `<index>=<time>` | — |
| `--output` / `--output-dir` / `--in-place` | `--dry-run` dışında **üçünden biri** | bkz. [Çalıştırma](invocation.md#hedef) | — |
| `--dry-run` | hayır | bir bayrak: hesaplar ve bildirir, hiçbir şey yazmaz — bkz. [Yazmadan önce görme](invocation.md#yazmadan-önce-görme) | kapalı |

Kısa biçim yoktur: bu iki seçenek yalnızca
uzun biçimiyle yazılır.

## Referans yazma

Bir referans bir altyazıyı adlandırır ve **başlangıcının** artık nereye düşmesi gerektiğini söyler:

```
<index>=<time>
```

**Dizin 1'den başlayarak sayılır**; gösterildiği gibi ve bir SubRip
dosyasının numaraladığı gibi. `0` reddedilir, son altyazıyı
aşan bir dizin de.

**Süre, [`shift`](shift.md) komutundaki gibi yazılır** — saniye ya da
zaman damgası, ondalık ayırıcı nokta, çözünürlük milisaniye:

| Biçim | Örnekler |
| :---- | :------- |
| saniye | `10`, `2.999` |
| zaman damgası | `00:00:10.000`, `01:39:37.040` |

`--first 3=10` ve `--first 3=00:00:10.000` aynı şeyi söyler.

`--first` ve `--last` bir aralığı değil, iki noktayı adlandırır. Adları
beklenen kullanımı söyler — biri dosyanın başında, öbürü sonunda olan iki referans — ve bu bir
kısıt değildir: yerlerini değiştirmek aynı sonucu verir ve **iki referansın dışında kalan her şey,
aralarındakiler gibi izler**.

## Öbür konumların neye dönüştüğü

İki referans bir ölçek çarpanı belirler ve her konum onu izler:

```
t′ = y₁ + (t − x₁) × r      with r = (y₂ − y₁) / (x₂ − x₁)
```

`x₁`, `x₂` adlandırılan iki altyazının geçerli başlangıçlarıdır, `y₁`, `y₂` ise
istenen sürelerdir. Çarpan tam bir rasyonel sayı olarak tutulur ve **yuvarlama
yalnızca bir kez**, milisaniyeye yapılır; iki referansın istenen sürelere binde bir
farkla değil **tam olarak** düşmesini sağlayan budur
(bkz. [ADR 0013](../../../../adr/0013-mise-a-l-echelle-exacte-des-positions.md)).

Her altyazı, ekranda kaldığı süreyle **orantılı** olarak ekranda kalır:
bitiş, başlangıçla aynı ölçeği izler. Bir dönüştürme kaydırma değildir ve
onun özelliğine sahip değildir.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,001 --> 00:00:07,000\nDeux.\n\n3\n00:00:09,000 --> 00:00:11,000\nTrois.\n\n' > a.srt; subedit-cli transform --first 1=00:00:01.000 --last 3=00:00:10.000 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,001 --> 00:00:07,000\nDeux.\n\n3\n00:00:09,000 --> 00:00:11,000\nTrois.\n\n' > a.srt; subedit-cli transform --first 1=00:00:01.000 --last 3=00:00:10.000 --output b.srt a.srt; cat b.srt
a.srt: 3 subtitles transformed onto 1=00:00:01.000 and 3=00:00:10.000 -> b.srt
1
00:00:01,000 --> 00:00:03,250
Un.

2
00:00:05,501 --> 00:00:07,750
Deux.

3
00:00:10,000 --> 00:00:12,250
Trois.
```

İlk referans yerinden oynamaz, üçüncü 9 sn'den 10 sn'ye gider: çarpan
`9/8`'dir ve adı verilmeyen ikinci altyazı
`5,001` konumundan `5,501` konumuna gider; bu, bir kez yuvarlanmış `1000 + 4001 × 9/8` değeridir.

## Geçersiz iki referans

| Durum | Ne zaman görülür | Kod |
| :-------- | :------------- | :--- |
| `--first` ve `--last` aynı dizini adlandırır | herhangi bir okumadan önce | `1` |
| bir dizin son altyazıyı aşar | dosya açıldığında | `2` ya da `3` |
| adı verilen iki altyazı aynı anda başlar | dosya açıldığında | `2` ya da `3` |

Üçü de aynı nedenle reddedilir: `x₂ − x₁` sıfırdır ve çakışan iki
nokta bir doğru tanımlamaz. Sonuç gibi görünen bir sıfıra bölmeyi geri vermek,
reddetmekten daha kötü olurdu.

İlk durum bir **kullanım hatasıdır** — iki seçenek dosyalar ne olursa olsun
hatalıdır, bu yüzden dosya başına değil, tek bir ileti gerekir. Öbür
ikisi açılan dosyaya bağlıdır ve bu yüzden bir işleme
başarısızlığı sayılır.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n' > a.srt; subedit-cli transform --first 2=1.000 --last 2=4.000 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n' > a.srt; subedit-cli transform --first 2=1.000 --last 2=4.000 --output b.srt a.srt; echo "code=$?"
--first and --last both name subtitle 2, and two references on one subtitle define no transform
code=1
```

Sınırın dışındaki bir dizin **sınır adıyla** reddedilir; bu, aynı zamanda
ne yazılabileceğini de söyler:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,000 --> 00:00:07,000\nDeux.\n\n' > a.srt; subedit-cli transform --first 1=1.000 --last 9=10.000 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,000 --> 00:00:07,000\nDeux.\n\n' > a.srt; subedit-cli transform --first 1=1.000 --last 9=10.000 --output b.srt a.srt; echo "code=$?"
a.srt: subtitle 9 is past the end: the file holds 2 subtitles
code=2
```

## Başlangıçtan önceki bir konum reddedilir

[`shift`](shift.md) komutunda olduğu gibi ve aynı nedenle: negatif bir konum
`-00:00:01,000` diye yazılır; bu, dokuz biçimin hiçbirinin tanımlamadığı ve hiçbir
okuyucunun kabul etmediği bir biçimdir. Bu durum burada, bir altyazı **ilk referanstan
önce** durduğunda ve ölçek onu sıfırın ötesine ittiğinde ortaya çıkar. Hiçbir şey
yazılmaz, kod `2`'dir ve ileti hatalı altyazıyı adıyla bildirir.

## Çıktı

**Standart çıktı** — hiçbir şey: sonuç, yazılan dosyadır. `--dry-run` ile de hiçbir şey yoktur: dosya yoktur ve anlatım satırı bunu söyler — bkz. [Yazmadan önce görme](invocation.md#yazmadan-önce-görme).

| Düzey | Hata çıktısına eklenenler |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles transformed onto 1=00:00:01.000 and 3=00:00:10.000 -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — okunan dosyanın biçimi, kodlaması, işareti ve satır sonları, olduğu gibi geri konarak |
| 3 | `<path>: N bytes read, M written`, ardından **her okuma tanılama iletisi** — bkz. [Çalıştırma](invocation.md#tanılama-iletilerini-okuma) |

Referanslar, nasıl yazılmış olurlarsa olsunlar ondalık noktayla geri yazılır.

### JSON biçiminde

[`--format json`](invocation.md#bir-betiğin-okuyabileceği-çıktı) ile her dosyanın nesnesi `counts.subtitles` (dosyanın tüm altyazıları) alanını taşır. Tablonun tamamı [Çalıştırma](invocation.md#her-alt-komutun-counts-içine-koyduğu) sayfasındadır.

## Çıkış kodları

Aracın kodlarıdır: her dosya yazıldıysa `0`, hiçbiri yazılmadıysa `2`, yalnızca
bazıları yazıldıysa `3`, kullanım hatasında `1`.

## Hatalar

| Neyin tetiklediği | İleti |
| :------------------ | :------ |
| `=` içermeyen referans | `"…" is not a reference: write it <index>=<time>, as in 3=00:00:10.000` |
| iki yarısından biri eksik referans | `"…" is missing one of its two halves: write it <index>=<time>, as in 3=00:00:10.000` |
| `0` dizini | `"0" is not a subtitle number: subtitles are counted from 1, as the file shows them` |
| tam sayı olmayan dizin | `"…" is not a subtitle number: expected a whole number, counted from 1 as the file shows them` |
| geçersiz süre | [`shift`](shift.md) komutundakiler |
| aynı dizinde `--first` ve `--last` | `--first and --last both name subtitle N, and two references on one subtitle define no transform` |
| sınırın dışında dizin | `<path>: subtitle N is past the end: the file holds M subtitles` |
| birlikte başlayan iki altyazı | `<path>: subtitles N and M start at the same moment, so they define no transform` |
| başlangıçtan önceki konum | `<path>: subtitle N would land before the origin, which no subtitle file can hold` |

**Üç hedef seçeneğinden biri olmadan hiçbir şey yazılmaz** ve bunların
hataları yazan alt komutlar için ortaktır:
bkz. [Çalıştırma](invocation.md#hedef).
