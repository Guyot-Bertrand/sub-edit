# `adjust`

<!-- machine-translated -->
> Bu sayfa makine çevirisidir ve henüz dili bilen bir okuyucu tarafından gözden geçirilmemiştir.

```
subedit-cli adjust [--speed CPS|off] [--shorten | --no-shorten] [--lengthen | --no-lengthen]
                   [--minimum TIME|off] [--maximum TIME|off] [--gap TIME|off]
                   [--range N-M|N-]
                   (--output FILE | --output-dir DIR | --in-place)
                   [--dry-run]
                   [--recursive]
                   <file>...
```

Her altyazının **süresini** bir okuma hızına ve sınırlara göre ayarlar: okunamayacak kadar kısa bir
altyazı uzatılır, ekrandan çok geç çıkan bir altyazı kısaltılabilir ve iki altyazı
çakışmaz. **Yalnızca bitiş oynar**: başlangıcı oynatmak altyazıyı oynatmak olurdu; bu da
[`shift`](shift.md) komutunun işidir. Aynı varsayılanlarla, [pencerenin](../subedit-gui/operations.md#adjust-durations)
`Adjust Durations…` işlemidir.

## Dört kısıt ve sıraları

**Bu sırayla** uygulanırlar; her biri bitişi ayarlar, sonuncusu kazanır:

1. **okuma hızı** — metnin, saniyede belirli sayıda karakter hızında
   ihtiyaç duyduğu süre (etiketler sayılmaz, satır sonları sayılır);
2. **asgari süre** — bunun altındaki bir süre asgariye çıkarılır;
3. **azami süre** — bunun üstündeki bir süre azamiye indirilir;
4. **aralık** — sonraki altyazıdan önce bırakılan süre. En son gelir ve
   **kazanır**: birlikte gösterilen iki altyazı izleyicinin gördüğü bir kusurdur,
   biraz kısa kalan bir altyazı değildir.

Dosyanın son altyazısının sonrakisi yoktur, dolayısıyla korunacak bir aralığı da yoktur. Bir
altyazının sonrakisi, `--range` tarafından **seçilmiş olsun olmasın**, dosyadaki sonraki altyazıdır.

| Kısıt | Seçenek | Varsayılan | Kapatma |
| :--------- | :----- | :----- | :------- |
| hız | `--speed CPS` | saniyede **15** karakter | `--speed off` |
| hızın yönü | `--shorten`, `--no-shorten`, `--lengthen`, `--no-lengthen` | yalnızca **uzatır** | — |
| asgari süre | `--minimum TIME` | **1,5 sn** | `--minimum off` |
| azami süre | `--maximum TIME` | **yok** | `--maximum off` |
| aralık | `--gap TIME` | **0** | `--gap off` |

Bunlar Gaupol’un ve pencerenin ilk açılışındaki ayarlardır. **`off` bir kısıtı kapatır:
artık denetlenmez.** Bu, onu sıfıra ayarlamak değildir — `0` asgari süre
sıfır asgari süredir, `0` aralık her çakışmayı yasaklar.

## Ayar dosyasının sağladıkları

**Yukarıdaki varsayılanlar yerleşik olanlardır.** `adjust` varsayılan olarak ayar dosyasının
işlem ayarlarını okur — `Adjust Durations…` işleminin pencerede hatırladığı veya
[`config set`](config.md) komutunun yazdığı değerler — ve bunlar yerleşik varsayılanların yerini alır. **Satırda
verilen bir seçenek her zaman kazanır**, sonra dosya, sonra yerleşik varsayılan. Dosyanın nerede olduğu, `--config` ve
`--no-config` için bkz.
[Ayar dosyası](invocation.md#ayar-dosyası).

| Anahtar | Ne sağlar | Onu geçersiz kılan seçenek |
| :-- | :--------------- | :----------------------- |
| `duration-adjust.speed` | okuma hızı, saniyedeki karakter sayısı olarak | `--speed CPS` |
| `duration-adjust.lengthen`, `duration-adjust.shorten` | hızın bir bitişi hangi yöne oynatabileceği; ikisi de `false` ise hız kapanır | `--lengthen`, `--no-lengthen`, `--shorten`, `--no-shorten`, `--speed off`; `--speed CPS` veya bir yön vermek, dosyanın kapattığı bir hızı uzatma yönüyle yeniden açar |
| `duration-adjust.minimum-enabled`, `duration-adjust.minimum-ms` | en kısa süre veya hiçbiri | `--minimum TIME`, `--minimum off` |
| `duration-adjust.maximum-enabled`, `duration-adjust.maximum-ms` | en uzun süre veya hiçbiri | `--maximum TIME`, `--maximum off` |
| `duration-adjust.gap-enabled`, `duration-adjust.gap-ms` | en az aralık veya hiçbiri | `--gap TIME`, `--gap off` |

**Her anahtarın onu geri alan bir seçeneği vardır.** Her yönün, dosya ne derse desin onu ayarlayan bir
bayrak çifti vardır: kısaltma için `--shorten` ve `--no-shorten`, uzatma için `--lengthen` ve
`--no-lengthen`; verilmeyen bir bayrak dosyanın yönünü olduğu gibi bırakır.
`--maximum off`, dosyanın açtığı bir azami süreyi kaldırır. `--format json` ile `constraints`, nereden
gelirse gelsin gerçekten kullanılan ayarları, `settings` ise okunan dosyayı söyler:

<!-- example: printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine/settings.conf config set duration-adjust.speed 12 2>/dev/null; subedit-cli --config mine/settings.conf --format json adjust --dry-run a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine/settings.conf config set duration-adjust.speed 12 2>/dev/null; subedit-cli --config mine/settings.conf --format json adjust --dry-run a.srt
settings: read mine/settings.conf
a.srt: adjusted the durations of 1 subtitle (dry run, nothing written)
{"schema":1,"command":"adjust","file":"a.srt","ok":true,"dry_run":true,"destination":null,"counts":{"subtitles":1,"adjusted":1,"sacrificed":{"speed":0,"minimum":0,"gap":0}},"constraints":{"speed":{"cps":"12","lengthen":true,"shorten":false},"minimum_ms":1500,"maximum_ms":null,"gap_ms":0},"settings":"mine/settings.conf","warnings":[]}
```

Bu sayfadaki diğer örnekler `--no-config` ile çalıştırılır; böylece onları çalıştıran makinenin
ayarları ne olursa olsun yerleşik varsayılanların ne yaptığını söylerler.

<!-- example: subedit-cli adjust --help -->
```console
$ subedit-cli adjust --help
Bring the duration of every subtitle within a reading speed and limits
Usage: subedit-cli adjust [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to adjust

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --speed CPS|off             Reading speed in characters per second, or off (default 15)
  --shorten                   Let the reading speed bring an end earlier
  --no-shorten                Do not let the reading speed bring an end earlier
  --lengthen                  Let the reading speed move an end later
  --no-lengthen               Do not let the reading speed move an end later
  --minimum TIME|off          Shortest duration, or off (default 1.5)
  --maximum TIME|off          Longest duration, or off (default: none)
  --gap TIME|off              Least time left before the next subtitle, or off (default 0)
  --range N-M|N-              Act only on subtitles N to M, or N to the end
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
| `--speed` | hayır | saniyedeki karakter sayısı, **sıfırdan büyük** (`15`, `12.5`) veya `off` | `15` |
| `--shorten` | hayır | bir bayrak: hız bir bitişi **ayrıca öne de çekebilir** | kapalı |
| `--no-shorten` | hayır | bir bayrak: ayar dosyası ne derse desin hız artık bir bitişi **öne çekmez** | kapalı |
| `--lengthen` | hayır | bir bayrak: ayar dosyası ne derse desin hız bir bitişi **sonraya iter** | kapalı |
| `--no-lengthen` | hayır | bir bayrak: hız artık bir bitişi **sonraya itmez** | kapalı |
| `--minimum` | hayır | sıfır veya daha büyük bir süre, ya da `off` | `1.5` |
| `--maximum` | hayır | **sıfırdan büyük** bir süre, ya da `off` | yok |
| `--gap` | hayır | sıfır veya daha büyük bir süre, ya da `off` | `0` |
| `--range` | hayır | `N-M` veya sona kadar `N-` — aşağıya bakın | dosyanın tamamı |
| `--output` / `--output-dir` / `--in-place` | **üçünden biri**, `--dry-run` hariç | bkz. [Çalıştırma](invocation.md#hedef) | — |
| `--dry-run` | hayır | bir bayrak: hesaplar ve söyler, hiçbir şey yazmaz — bkz. [Yazmadan önce görme](invocation.md#yazmadan-önce-görme) | kapalı |

Bir süre, [`shift`](shift.md#süre-yazma) komutundaki gibi yazılır: `2.5`, `00:00:02.500`.

**Etkin hiçbir kısıt yoksa `adjust` reddedilir** (kod `1`, hiçbir şey yazılmaz): uyarlanacak bir
şey kalmazdı ve başarılı olduğunu söyleyerek hiçbir şey yapmayan bir komut en kötü
yanıttır. Bu, bir azami süresi olmayan `--speed off --minimum off --gap off` durumunun ve asgari süre
ile aralığı kapatılmış tek başına `--no-lengthen` durumunun halidir: hiçbir
bitişi oynatmayan bir hız hiçbir şeyi kısıtlamaz.

`--shorten`, `--no-shorten`, `--lengthen` ve `--no-lengthen`, hızın bir bitişi hangi yöne
oynattığını söyler; yalnızca bir hızla anlamlıdır ve **`--speed off` bunları reddeder**. Karşıtıyla birlikte verilen bir bayrak
— `--shorten --no-shorten`, `--lengthen --no-lengthen` — da reddedilir.

### `--range`

`--range 3-7`, ayarlamayı 3 ile 7 arasındaki altyazılarla sınırlar; **1 numaralı altyazıdan başlayarak sayılır, sınırlar
dahildir**; `--range 3-` sonuncuya kadar gider. Tek bir altyazı `4-4` olarak yazılır.

- **Bitmeden başlayan** bir aralık, bir `0`, bir işaret veya yalnız bir sayı
  kullanım hatası olarak reddedilir (kod `1`).
- **Toplu işlemin her dosyası için geçerlidir**; bu yalnızca dosyalar birbirine
  benziyorsa anlamlıdır. **Her dosyaya göre** değerlendirilir: o dosyayı aşan bir aralık,
  dosyaya **dokunulmadan önce** sınırı adlandırarak reddedilir —
  `range 3-9 ends after the last subtitle: the file has 4` —, o dosya başarısız olur (kod `2` veya `3`, tanımlayıcı
  `range-out-of-bounds`) ve diğerleri devam eder.
- Raporun sayıları yalnızca seçilen altyazıları ilgilendirir.

## Nelerden vazgeçilir

İki kısıt aynı altyazıda birbiriyle çelişebilir: asgari süre 1,5 sn iken sonraki altyazının bıraktığı
süre 0,8 sn ise ikisi birlikte sağlanamaz. **Sıra karar verir ve rapor
neyin pes ettiğini söyler** — Gaupol’un yapmadığı şey budur: onları sessizce bozar.

Her kısıt yalnızca, diğerleri uygulandıktan sonra **en az bir altyazı onu sağlayamadıysa**
adlandırılır. Azami sürenin sayısı yoktur: aralıktan önce uygulanır ve aralık yalnızca
kısaltabildiğinden her zaman geçerlidir. Azami süreyi aşan bir asgari süre azami süreye yol verir ve
asgari süre olarak sayılır.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --output ajuste.srt courts.srt; grep -e '-->' ajuste.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --output ajuste.srt courts.srt; grep -e '-->' ajuste.srt
courts.srt: adjusted the durations of 3 subtitles; could not satisfy the minimum duration in 1 subtitle -> ajuste.srt
00:00:00,000 --> 00:00:02,000
00:00:10,000 --> 00:00:14,000
00:00:14,200 --> 00:00:15,000
00:00:15,000 --> 00:00:16,500
```

Üçüncü altyazının 1,5 sn sürmesi için bitişi 15,7 sn olmalıdır, oysa dördüncünün
başlangıcı 15,0 sn: **aralık kazanır**, bitiş 15,0 sn olur ve rapor asgari süreyi adlandırır.

Aralığı kapatmak asgari süreyi kazandırır — ve iki altyazı çakışır:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --gap off --output libre.srt courts.srt; grep -e '-->' libre.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --gap off --output libre.srt courts.srt; grep -e '-->' libre.srt
courts.srt: adjusted the durations of 3 subtitles -> libre.srt
00:00:00,000 --> 00:00:02,000
00:00:10,000 --> 00:00:14,000
00:00:14,200 --> 00:00:15,700
00:00:15,000 --> 00:00:16,500
```

Dosya zaten sınırlarının içine sığıyorsa hiçbir şeyden vazgeçilmez ve hiçbir şey ayarlanmaz:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --speed off --minimum off --output intact.srt courts.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --speed off --minimum off --output intact.srt courts.srt
courts.srt: no duration to adjust -> intact.srt
```

Dosya olduğu gibi yazılır: **verilen bir hedef yazılan bir hedeftir**.

## Çıktı

**Standart çıktı** — boş: sonuç, yazılan dosyadır.

| Düzey | Hata çıktısına eklenenler |
| :----- | :------------------------------------- |
| 1 | `<path>: adjusted the durations of N subtitles; could not satisfy …  -> <destination>`, veya `<path>: no duration to adjust -> <destination>` — ve en başta, okunan ayar dosyası — `settings: read <path>` — bkz. [Ayar dosyası](invocation.md#ayar-dosyası) |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — okunan dosyanın biçimi, kodlaması, işareti ve satır sonları, olduğu gibi geri konur |
| 3 | `<path>: N bytes read, M written`, ardından **her okuma tanılaması** — bkz. [Çalıştırma](invocation.md#tanılama-iletilerini-okuma) |

Cümle pencerenindir: `N`, bitişi oynayan altyazıların sayısıdır; geri kalanı ise,
hiçbir şey pes etmediğinde bulunmaz, korunamayanları — `the reading speed`,
`the minimum duration`, `the gap` — ilgili altyazı sayısıyla birlikte sıralar.

### JSON biçiminde

[`--format json`](invocation.md#bir-betiğin-okuyabileceği-çıktı) ile her dosyanın nesnesi, zarfın yanında
şunları taşır:

| Anahtar | İçerik |
| :-- | :------ |
| `counts.subtitles` | **hedeflenen** altyazılar: dosyanın tamamı veya aralık |
| `counts.adjusted` | bitişi oynayanlar |
| `counts.sacrificed.speed`, `.minimum`, `.gap` | diğerleri uygulandıktan sonra bu kısıtı hâlâ sağlamayan altyazılar, **bitişi oynamayanlar dahil** |
| `constraints.speed` | `{"cps", "lengthen", "shorten"}` — saniyedeki karakter sayısı **bir dizge olarak** (`"15"`, `"12.5"`) — veya kapatılmışsa `null` |
| `constraints.minimum_ms`, `.maximum_ms`, `.gap_ms` | kullanılan süreler, **tam milisaniye** olarak, veya kapatılmışsa `null` |
| `settings` | okunan ayar dosyası, hiçbiri okunmadıysa `null` |

`constraints`, varsayılanlar dahil **gerçekten kullanılan ayarları** söyler: aynı altyazılar
üzerindeki iki hesaplama böylece komut satırı okunmadan karşılaştırılır.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config --format json adjust --dry-run --maximum 1.8 courts.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config --format json adjust --dry-run --maximum 1.8 courts.srt
courts.srt: adjusted the durations of 4 subtitles; could not satisfy the reading speed in 1 subtitle, the minimum duration in 1 subtitle (dry run, nothing written)
{"schema":1,"command":"adjust","file":"courts.srt","ok":true,"dry_run":true,"destination":null,"counts":{"subtitles":4,"adjusted":4,"sacrificed":{"speed":1,"minimum":1,"gap":0}},"constraints":{"speed":{"cps":"15","lengthen":true,"shorten":false},"minimum_ms":1500,"maximum_ms":1800,"gap_ms":0},"settings":null,"warnings":[]}
```

## Çıkış kodları

Aracın genel kodları: her dosya işlendiyse `0`, hiçbiri işlenmediyse `2`, yalnızca
bazıları işlendiyse `3`, kullanım hatasında `1`. Zaten sınırlarının içine sığan bir dosya işlenmiş sayılır.

## Hatalar

| Neyin tetiklediği | İleti |
| :------------------ | :------ |
| `--config` ile adlandırılan var olmayan bir dosya — kullanıcının kendi dosyası eksikse varsayılanlar kullanılır | `settings file not found: <FILE>` |
| etkin hiçbir kısıt yok | `adjust has nothing to adjust to: every constraint is off, or the reading speed is told to move no end` |
| hız olmayan bir hız | `--speed: "…" is not a reading speed: expected a number of characters per second above zero, like 15 or 12.5, or off` |
| `--speed off` ile bir yön bayrağı | `--lengthen, --no-lengthen, --shorten and --no-shorten say which way the reading speed may move an end, and --speed off switches the reading speed off` |
| karşıtıyla birlikte bir yön bayrağı | `--shorten and --no-shorten say opposite things; give one of them` (`--lengthen` ve `--no-lengthen` için de aynı) |
| süre olmayan bir süre | `--minimum: "…" is not a time: …` (`--maximum`, `--gap` için de aynı) |
| negatif süre | `--minimum: "-1" is not a duration of zero or more` (`--gap` için de aynı) |
| sıfır veya negatif azami süre | `--maximum: "0" is not a duration above zero` |
| kötü yazılmış aralık | `--range: "…" is not a range: write it N-M, or N- to go to the end, counted from 1 and inclusive` |
| bitmeden başlayan aralık | `--range: "5-3" is not a range: it ends before it starts` |
| dosyanın dışında kalan aralık | `<path>: range N-M starts after the last subtitle: the file has K`, veya `… ends after …` |

Hedef hataları, yazan alt komutlar için ortaktır:
bkz. [Çalıştırma](invocation.md#hedef).
