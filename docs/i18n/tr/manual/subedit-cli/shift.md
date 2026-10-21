# `shift`

<!-- machine-translated -->
> Bu sayfa makine çevirisidir ve henüz dili bilen bir okuyucu tarafından gözden geçirilmemiştir.

```
subedit-cli shift (--by <time> | --to-grid)
                  (--output FILE | --output-dir DIR | --in-place)
                  [--dry-run]
                  [--recursive]
                  <file>...
```

Dosyanın **tüm** konumlarını, bir yöne ya da ötekine, aynı miktarda
kaydırır. Hiçbir altyazı ekranda daha uzun ya da daha kısa kalmaz:
bir kaydırmayı kaydırma yapan budur.

<!-- example: subedit-cli shift --help -->
```console
$ subedit-cli shift --help
Move every position of a file by a fixed amount
Usage: subedit-cli shift [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to shift

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --by TEXT                   Amount to move by: 2.999, -7.001, or 00:00:07.001
  --to-grid                   Move by the amount that puts the positions back on their frame grid
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
| `--by` | **ikisinden biri** | işaretli bir süre, aşağıya bakın | — |
| `--to-grid` | **ikisinden biri** | bayrak: miktar ölçülür, aşağıya bakın | — |
| `--output` / `--output-dir` / `--in-place` | `--dry-run` dışında **üçünden biri** | bkz. [Çalıştırma](invocation.md#hedef) | — |
| `--dry-run` | hayır | bir bayrak: hesaplar ve bildirir, hiçbir şey yazmaz — bkz. [Yazmadan önce görme](invocation.md#yazmadan-önce-görme) | kapalı |

Okunan dosyanın biçimi **korunur**: biçimi değiştirmek
[`convert`](convert.md) komutunun işidir.

## `--to-grid`: verilen değil, ölçülen miktar

Konumları bir kare ızgarasının **sabit bir miktar
dışında** üzerinde olan bir dosya kaydırılmıştır ve bu sabit, konumların kendisinden ölçülebilir.
`--to-grid` onu ölçer ve uygular; kimsenin onu bilmesi gerekmez.

Miktar **her dosyaya özgüdür**: aynı ızgaradan iki farklı miktarda
kaymış iki dosya, her biri kendi miktarıyla geri döner. Bulunan kare hızıyla birlikte
anlatımda yazılır.

**Yol en kısa olandır.** Yarım karenin ötesinde en yakın ızgara çizgisi
bir sonrakidir: düzeltme, neredeyse bütün bir kare geri gitmek yerine
ileri gider.

<!-- example: k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40 + 7)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > decale.srt; subedit-cli shift --to-grid --output recale.srt decale.srt -->
```console
$ k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40 + 7)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > decale.srt; subedit-cli shift --to-grid --output recale.srt decale.srt
decale.srt: 12 subtitles shifted by -0.007 s onto their 25 fps grid -> recale.srt
```

**Bilinen hiçbir ızgarada olmayan bir dosya reddedilir ve nedeni söylenir.**
Yeniden birleştirilecek bir şey yoktur ve gürültü üzerinde ölçülmüş bir evre, dosyayı
rastgele kaydırırdı. İşlemi denemeden önce bir dosyanın ne verdiğini öğrenmek için
bkz. [`inspect`](inspect.md#frame-rate-grid-ne-bildirir).

`--by` ve `--to-grid` ikisi de ne kadar kaydırılacağını söyler: ikisini birlikte
vermek bir kullanım hatasıdır, hiçbirini vermemek de öyle.

**`--to-grid`, [`snap`](snap.md) değildir.** Tüm dosyayı
aynı miktarda kaydırır ve göreli zamanlamayı **tam olarak** korur; `snap`
her konumu bağımsız olarak taşır ve yol boyunca elle
düzeltilmiş olanları da içine alır. Temiz bir dosyada ikisi aynı sonucu verir;
kısmi bir dosyada vermez.

## Süre yazma

İki biçim vardır, yalnızca iki:

| Biçim | Örnekler |
| :---- | :------- |
| saniye | `30`, `2.999`, `-7.001`, `+1.5` |
| zaman damgası | `00:00:07.001`, `01:39:37.040`, `-00:00:07.001` |

İkisi de aynı şeyi söyler: `--by 7.001` ve `--by 00:00:07.001` aynı
dosyayı üretir.

**Ondalık ayırıcı noktadır**, çünkü komut satırı İngilizce konuşur.
Virgül yorumlanmaz, reddedilir: dünyanın yarısı `7,001` değerini
yedi bin bir olarak yazar ve onu yedi saniye diye okumak bir tahmin olurdu —
o yarı için yanlış bir tahmin.

**Çözünürlük milisaniyedir.** Daha ince bir değer sessizce
yuvarlanmak yerine reddedilir: örtük bir yuvarlama, istenenle hiç
uyuşmayan bir sonuç verirdi.

<!-- example: printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle shifted by -7.001 s -> b.srt
1
00:00:02,999 --> 00:00:04,999
Bonjour.
```

## Başlangıçtan önceye kaydırma reddedilir

Sıfırın ötesine gitmek kod `2` verir ve **hiçbir şey yazmaz**. İleti,
kaydırmayı kaldıramayan altyazıyı adıyla bildirir; bu da aynı zamanda
ne kadar gidilebileceğini söyler.

Nedeni bir ilke değildir: başlangıçtan önceki bir konum
`-00:00:01,000` diye yazılır; bu, dokuz biçimin hiçbirinin tanımlamadığı ve hiçbir okuyucunun
kabul etmediği bir biçimdir. Çekirdek bu tür konumlara izin verir — onları reddetmek bir
düzenleme işlemini özel bir duruma çevirirdi — ama bir dosya, bellekteki
bir modelden başka bir şeydir.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; echo "code=$?"
a.srt: subtitle 1 would start before the origin, which no subtitle file can hold
code=2
```

## Çıktı

**Standart çıktı** — hiçbir şey: sonuç, yazılan dosyadır. `--dry-run` ile de hiçbir şey yoktur: dosya yoktur ve anlatım satırı bunu söyler — bkz. [Yazmadan önce görme](invocation.md#yazmadan-önce-görme).

| Düzey | Hata çıktısına eklenenler |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles shifted by 2.999 s -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — okunan dosyanın biçimi, kodlaması, işareti ve satır sonları, olduğu gibi geri konarak |
| 3 | `<path>: N bytes read, M written`, ardından **her okuma tanılama iletisi** — bkz. [Çalıştırma](invocation.md#tanılama-iletilerini-okuma) |

### JSON biçiminde

[`--format json`](invocation.md#bir-betiğin-okuyabileceği-çıktı) ile her dosyanın nesnesi `counts.subtitles` (dosyanın tüm altyazıları) ve `counts.shifted_by_ms` (kaydırma, tam milisaniye olarak, **işaretli**) alanlarını taşır. Tablonun tamamı [Çalıştırma](invocation.md#her-alt-komutun-counts-içine-koyduğu) sayfasındadır.

## Çıkış kodları

Aracın kodlarıdır: her dosya yazıldıysa `0`, hiçbiri yazılmadıysa `2`, yalnızca
bazıları yazıldıysa `3`, kullanım hatasında `1`.

## Hatalar

| Neyin tetiklediği | İleti |
| :------------------ | :------ |
| ondalık virgül | `"…" is not a time: use a decimal point and not a comma, in every language` |
| milisaniyeden daha ince | `"…" is not a time: positions are held to the millisecond, no finer` |
| süre olmayan metin | `"…" is not a time: expected seconds like 2.999, or a timestamp like 00:00:07.001` |
| başlangıçtan önceye kaydırma | `<path>: subtitle N would start before the origin, which no subtitle file can hold` |

**Üç hedef seçeneğinden biri olmadan hiçbir şey yazılmaz** ve bunların
hataları yazan alt komutlar için ortaktır:
bkz. [Çalıştırma](invocation.md#hedef).
